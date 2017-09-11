#include "monetra_api.h"
#include "monetra_conn.h"
#include "monetra_trans.h"

/*! Convert a dictionary into a key/value pair string */
static M_bool M_hash_dict_serialize_buf(M_hash_dict_t *dict, M_buf_t *buf, unsigned char key_delim, unsigned char value_delim, unsigned char quote_char, unsigned char escape_char)
{
	M_hash_dict_enum_t *hashenum = NULL;
	const char         *key;
	const char         *val;

	if (dict == NULL || M_hash_dict_num_keys(dict) == 0)
		return M_FALSE;

	M_hash_dict_enumerate(dict, &hashenum);
	while (M_hash_dict_enumerate_next(dict, hashenum, &key, &val)) {
		M_buf_add_str(buf, key);
		M_buf_add_byte(buf, key_delim);
		if (val != NULL) {
			M_buf_add_byte(buf, quote_char);
			if (M_mem_chr(val, quote_char, M_str_len(val)) != NULL || M_mem_chr(val, escape_char, M_str_len(val)) != NULL) {
				/* If string contains a quote or escape char, we need to escape it */
				for ( ; *val != '\0'; val++) {
					if (*val == quote_char || *val == escape_char)
						M_buf_add_byte(buf, escape_char);
					M_buf_add_byte(buf, (unsigned char)*val);
				}
			} else {
				/* Value doesn't contain a double quote, just output it */
				M_buf_add_str(buf, val);
			}
			M_buf_add_byte(buf, quote_char);
		} else {
			/* Value is NULL, don't even put quotes around it so when we parse it
			 * back out we know to insert NULL as the value */
		}
		M_buf_add_byte(buf, value_delim);
	}

	M_hash_dict_enumerate_free(hashenum);
	return M_TRUE;
}

void LM_trans_structure(LM_conn_t *conn, LM_trans_t *trans)
{
	/* STX */
	M_buf_add_byte(conn->outbuf, 0x02);

	/* Message ID */
	M_buf_add_uint(conn->outbuf, trans->trans_id);

	/* FS */
	M_buf_add_byte(conn->outbuf, 0x1C);

	/* Data */
	if (M_hash_dict_num_keys(trans->request_params) == 1 && M_str_caseeq(M_hash_dict_get_direct(trans->request_params, "action"), "ping")) {
		/* Rewrite for proper ping message format */
		M_buf_add_str(conn->outbuf, "PING");
	} else {
		M_hash_dict_serialize_buf(trans->request_params, conn->outbuf, '=', '\n', '"', '"');
	}
	/* Clear dictionary for security purposes, may contain passwords or sensitive
	 * account information */
	M_hash_dict_destroy(trans->request_params);
	trans->request_params = NULL;

	/* ETX */
	M_buf_add_byte(conn->outbuf, 0x03);
}


/* If connected, move all transactions from 'ready' queue to 'pending' queue
 * and structure request message and place in outbuf.  If outbuf was not empty
 * prior to calling, also write out the io, otherwise assume we'll get a signal.
 * 
 * NOTE: LM_conn_t must be locked before calling this
 */
void LM_trans_send_messages(LM_conn_t *conn)
{
	LM_trans_t *trans;
	M_bool      orig_outbuf_empty = M_FALSE;

	/* If disconnected, implicitly start a connection.  When a connected signal
	 * comes in that the connection is established, will re-run this function */
	if (conn->status == LM_CONN_STATUS_DISCONNECTED) {
		M_thread_mutex_unlock(conn->lock);
		LM_conn_connect(conn);
		M_thread_mutex_lock(conn->lock);
		return;
	}

	if (conn->status != LM_CONN_STATUS_CONNECTED)
		return;

	if (!M_buf_len(conn->outbuf))
		orig_outbuf_empty = M_TRUE;

	while ((trans = M_queue_take_first(conn->trans_ready)) != NULL) {
		M_thread_mutex_lock(trans->lock);
		trans->status = LM_TRANS_STATUS_PENDING;
		LM_trans_structure(conn, trans);
		M_thread_mutex_unlock(trans->lock);
		M_queue_insert(conn->trans_pending, trans);
	}

	/* Make sure the idle timer is not running if we have pending transactions */
	if (M_queue_len(conn->trans_pending))
		M_event_timer_stop(conn->timer);

	/* If outbuf was empty on entry, request a fake write event to put this on the
	 * wire as we probably won't get one otherwise.
	 * NOTE: We don't actually want to write here, because someone may be
	 *       enqueuing thousands of transactions, and we don't want to incur
	 *       both the SSL/TLS packet overhead of shorter-than-max packets, nor
	 *       the syscall overhead of the send() */
	if (orig_outbuf_empty && M_buf_len(conn->outbuf))
		M_event_trigger_signal(conn->write_trigger);
}
