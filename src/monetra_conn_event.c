#include "monetra_api.h"
#include "monetra_conn.h"
#include "monetra_trans.h"

static M_bool LM_conn_destroy_is_set(LM_conn_t *conn)
{
	M_bool destroy_set;

	if (conn == NULL)
		return M_FALSE;

	M_thread_mutex_lock(conn->lock);
	destroy_set = conn->destroy;
	M_thread_mutex_unlock(conn->lock);

	return destroy_set;
}


typedef struct {
	LM_event_type_t  type;
	LM_trans_t      *trans;
} LM_conn_events_entry_t;


static void LM_conn_event_add(M_list_t **events, LM_event_type_t type, LM_trans_t *trans)
{
	static const struct M_list_callbacks cb = {
		NULL,
		NULL,
		NULL,
		M_free
	};
	LM_conn_events_entry_t *entry;

	if (events == NULL)
		return;

	/* If a list hasn't been created yet, create it */
	if (*events == NULL)
		*events = M_list_create(&cb, M_LIST_NONE);

	entry        = M_malloc_zero(sizeof(*entry));
	entry->type  = type;
	entry->trans = trans;
	M_list_insert(*events, entry);
}


static void LM_conn_event_handle_conn_error(M_list_t **events, LM_conn_t *conn, M_event_type_t type)
{
	LM_trans_t        *trans;
	M_queue_foreach_t *q_foreach           = NULL;
	M_bool             disconnect_was_idle = (conn->status == LM_CONN_STATUS_IDLE_TIMEOUT);

	/* If we requested a disconnect, but got an error for some reason, rewrite to a
	 * disconnect */
	if (type == M_EVENT_TYPE_ERROR && conn->status == LM_CONN_STATUS_DISCONNECTING) {
		type = M_EVENT_TYPE_DISCONNECTED;
	}

	/* Destroy the io object and any associated unprocessed data */
	M_io_destroy(conn->io);
	M_event_trigger_remove(conn->write_trigger);
	conn->io            = NULL;
	conn->write_trigger = NULL;
	conn->status        = LM_CONN_STATUS_DISCONNECTED;
	M_buf_truncate(conn->outbuf, 0);
	M_parser_truncate(conn->inbuf, 0);

	/* Kill any timers that might be hanging around */
	if (conn->timer != NULL) {
		M_event_timer_remove(conn->timer);
		conn->timer = NULL;
	}

	/* Send events for any "ready" transactions letting them know the connection
	 * did not succeed.  Don't do this if there was an idle timeout as that means
	 * a new transaction was enqueued when an idle timeout was already triggered,
	 * so this isn't a failure to connect but rather the actual disconnect got
	 * delivered for part of the graceful disconnect sequence.  The connection will
	 * be brought back online and these requests will be sent out. */
	if (!disconnect_was_idle) {
		while (M_queue_foreach(conn->trans_ready, &q_foreach, (void **)&trans)) {
			LM_conn_event_add(events, LM_EVENT_TRANS_NOCONNECT, trans);
		}
	}

	/* Mark any pending transactions as an error, send error events for each txn */
	while ((trans=M_queue_take_first(conn->trans_pending)) != NULL) {
		M_thread_mutex_lock(trans->lock);
		M_queue_insert(conn->trans_done, trans);

		/* XXX: We really need to have a real ERROR state we can tag a transaction with */
		trans->status = LM_TRANS_STATUS_DONE;

		/* Fake a short response indicating the failure for compatibility for integrators
		 * not properly relying on the event callback state */
		trans->response_type = LM_TRANS_RESPONSE_KV;
		trans->response_params = M_hash_dict_create(16, 75, M_HASH_DICT_CASECMP);
		M_hash_dict_insert(trans->response_params, "code", "DENY");
		M_hash_dict_insert(trans->response_params, "verbiage", conn->error);

		/* In error state, kill timer */
		if (trans->timeout_timer) {
			M_event_timer_remove(trans->timeout_timer);
			trans->timeout_timer = NULL;
		}

		M_thread_mutex_unlock(trans->lock);
		LM_conn_event_add(events, LM_EVENT_TRANS_ERROR, trans);
	}

	/* Send error or disconnect event as appropriate */
	LM_conn_event_add(events, (type == M_EVENT_TYPE_DISCONNECTED)?LM_EVENT_CONN_DISCONNECT:LM_EVENT_CONN_ERROR, NULL);
}


/* NOTE: must be holding the connection lock before entering */
static void LM_conn_cleanup_ping(LM_conn_t *conn)
{
	/* Kill the timer */
	M_event_timer_remove(conn->timer);
	conn->timer = NULL;

	/* Remove it from the queue.
	 * Need to be in a lock and need to take because remove will destroy the
	 * txn and we can't have that happen here. */
	M_queue_take(conn->trans_pending, conn->pingtxn);

	/* Kill the ping response (if any), must not be holding the conn lock. */
	M_thread_mutex_unlock(conn->lock);
	LM_trans_delete(conn->pingtxn);
	M_thread_mutex_lock(conn->lock);
	conn->pingtxn = NULL;
}


static void LM_conn_event_set_connected(M_event_t *event, LM_conn_t *conn, M_list_t **events)
{
	LM_conn_cleanup_ping(conn);

	/* Enqueue connected event */
	conn->status = LM_CONN_STATUS_CONNECTED;
	LM_conn_event_add(events, LM_EVENT_CONN_CONNECTED, NULL);

	/* Move any transactions from the READY state to the PENDING state (and put them on the wire) */
	LM_trans_send_messages(conn);

	/* Create timer object or idle connection timeout */
	if (conn->idle_timeout) {
		conn->timer = M_event_timer_add(event, LM_conn_event_handler, conn);
		M_event_timer_set_firecount(conn->timer, 1);
	}
}


static void LM_conntrans_event_handler(M_event_t *event, M_event_type_t type, LM_conn_t *conn, LM_trans_t *event_trans)
{
	M_list_t               *events = NULL;
	LM_conn_events_entry_t *entry;
	M_bool                  request_connect    = M_FALSE;

	M_thread_mutex_lock(conn->lock);
	conn->in_use = M_TRUE;

	switch (type) {
		case M_EVENT_TYPE_CONNECTED:
			if (conn->disable_ping) {
				LM_conn_event_set_connected(event, conn, &events);
			} else {
				/* Start 5 second timer for ping request, call back into self as the only time
				 * an event type of OTHER will be triggered is by this timer! */
				conn->timer   = M_event_timer_oneshot(event, 5 * 1000, M_FALSE, LM_conn_event_handler, conn);
				conn->pingtxn = LM_trans_ping_request(conn);
			}

			conn->write_trigger = M_event_trigger_add(event, LM_conn_event_handler_writetrigger, conn);
			break;

		case M_EVENT_TYPE_DISCONNECTED:
		case M_EVENT_TYPE_ERROR:
			/* Save the error message received */
			if (conn->status == LM_CONN_STATUS_IDLE_TIMEOUT) {
				M_snprintf(conn->error, sizeof(conn->error), "Idle connection timeout");
			} else {
				M_io_get_error_string(conn->io, conn->error, sizeof(conn->error));
			}
			/* Handle the connectivity failure */
			LM_conn_event_handle_conn_error(&events, conn, type);
			break;

		case M_EVENT_TYPE_READ:
			/* Read into parser */
			if (M_io_read_into_parser(conn->io, conn->inbuf) == M_IO_ERROR_SUCCESS) {
				/* Parse off completed messages */
				LM_trans_t            *trans = NULL;
				LM_conn_parse_error_t  err;

				while ((err = LM_conn_parse_trans_buffer(conn, &trans)) == LM_CONN_PARSE_ERROR_SUCCESS) {
					/* Enqueue events for each new transaction complete (except ping response) */
					if (trans == conn->pingtxn) {
						LM_conn_event_set_connected(event, conn, &events);
					} else {
						LM_conn_event_add(&events, LM_EVENT_TRANS_DONE, trans);

						M_thread_mutex_lock(trans->lock);
						/* Can't fire timeout, transaction is done */
						if (trans->timeout_timer) {
							M_event_timer_remove(trans->timeout_timer);
							trans->timeout_timer = NULL;
						}
						M_thread_mutex_unlock(trans->lock);

					}
				}

				/* If no pending transactions, re-activate the timer */
				if (M_queue_len(conn->trans_pending) == 0) {
					M_event_timer_reset(conn->timer, conn->idle_timeout * 1000);
				}

				/* If error, mark any pending transactions as an error, send error events for each txn,
				 * just like we had received a DISCONNECTED or ERROR event */
				if (err == LM_CONN_PARSE_ERROR_ERROR) {
					M_snprintf(conn->error, sizeof(conn->error), "Invalid response format received. Disconnect.");
					LM_conn_event_handle_conn_error(&events, conn, M_EVENT_TYPE_ERROR);
				}
			}
			break;

		case M_EVENT_TYPE_WRITE:
			/* Write from buf if not empty, we can ignore errors here as we'll get an
			 * event if an error actually occurred */
			if (M_buf_len(conn->outbuf)) {
				M_io_write_from_buf(conn->io, conn->outbuf);
			}
			break;

		case M_EVENT_TYPE_OTHER:
			/* Used for Timers */

			/* If event_trans is not NULL, then the only thing this could be is a transaction timeout */
			if (event_trans != NULL) {
				LM_conn_event_add(&events, LM_EVENT_TRANS_TIMEOUT, event_trans);
				M_thread_mutex_lock(event_trans->lock);
				M_event_timer_remove(event_trans->timeout_timer);
				event_trans->timeout_timer = NULL;
				M_thread_mutex_unlock(event_trans->lock);
			} else {
				if (conn->status == LM_CONN_STATUS_CONNECTING) {
					/* When in the connecting state, the timer only gets fired on a timeout of a PING operation.
					 * That means we need to treat this as a connection error  */
					LM_conn_cleanup_ping(conn);
					M_snprintf(conn->error, sizeof(conn->error), "Failed to receive reply to PING request");
					LM_conn_event_handle_conn_error(&events, conn, M_EVENT_TYPE_ERROR);
				} else if (conn->status == LM_CONN_STATUS_CONNECTED) {
					/* If we're connected an we get a timer event, that means we hit an idle connection timeout.
					 * we need to trigger a disconnect (but verify there are no pending transactions, the
					 * timer shouldn't be running if there are) */
					if (M_queue_len(conn->trans_pending) == 0) {
						/* Request a graceful disconnect */
						LM_conn_disconnect_connlocked(conn);
						conn->status = LM_CONN_STATUS_IDLE_TIMEOUT;
					}
				}

				/* Clean up the timer if it hasn't been cleaned up already */
				if (conn->timer != NULL) {
					M_event_timer_remove(conn->timer);
					conn->timer = NULL;
				}
			}
			break;

		default:
			/* We'll never get these: M_EVENT_TYPE_ACCEPT */
			break;
	}

	M_thread_mutex_unlock(conn->lock);

	/* Write out all events, but check to see if "conn" was attempted to be destroyed
	 * before processing the next */
	while ((entry = M_list_take_first(events)) != NULL) {
		conn->event_callback(conn, event, entry->type, entry->trans);
		M_free(entry);
		if (LM_conn_destroy_is_set(conn))
			break;
	}
	M_list_destroy(events, M_TRUE);

	/* Untag the connection as in-use. Check to see if we need to reconnect. */
	M_thread_mutex_lock(conn->lock);

	/* Now we need to kill transaction pointers that were LM_trans_delete()'d while we were
	 * processing events, that we had to delay. */
	if (M_llist_len(conn->trans_delay_rm)) {
		M_llist_node_t *next;
		while ((next = M_llist_first(conn->trans_delay_rm)) != NULL)
			M_llist_remove_node(next);
	}


	conn->in_use = M_FALSE;
	/* Looks like we had disconnected due to an idle timeout, then before the connection was
	 * fully brought down (since we do this gracefully which could take a little time),
	 * we enqueued a new transaction.  We need to detect this and start to reconnect so
	 * that transaction can be sent. */
	if (conn->status == LM_CONN_STATUS_DISCONNECTED && M_queue_len(conn->trans_ready)) {
		request_connect = M_TRUE;
	}
	M_thread_mutex_unlock(conn->lock);

	/* Should only happen if new request is enqueued while a disconnect due to idle timeout is processing */
	if (request_connect) {
		LM_conn_connect(conn);
	}

	/* Check to see if the user tried to kill our conn object, if so, let it clean
	 * up now */
	if (LM_conn_destroy_is_set(conn)) {
		LM_conn_destroy(conn);
	}
}


void LM_conn_event_handler(M_event_t *event, M_event_type_t type, M_io_t *io, void *user_arg)
{
	LM_conn_t *conn   = user_arg;
	(void)io;
	LM_conntrans_event_handler(event, type, conn, NULL);
}


void LM_trans_event_handler(M_event_t *event, M_event_type_t type, M_io_t *io, void *user_arg)
{
	LM_trans_t *trans = user_arg;
	LM_conn_t  *conn  = trans->conn;
	(void)io;
	LM_conntrans_event_handler(event, type, conn, trans);
}


void LM_conn_event_handler_writetrigger(M_event_t *event, M_event_type_t type, M_io_t *io, void *user_arg)
{
	LM_conn_t *conn = user_arg;
	(void)type;
	(void)io;
	/* Just use our normal event handler, but rewrite to a WRITE event */
	LM_conntrans_event_handler(event, M_EVENT_TYPE_WRITE, conn, NULL);
}
