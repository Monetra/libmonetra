#include "monetra_api.h"
#include "monetra_conn.h"
#include "monetra_trans.h"
#include "mstdlib/mstdlib_tls.h"

static void LM_conn_trans_destructor(void *arg)
{
    LM_trans_free((LM_trans_t *)arg);
}


LM_conn_t * LM_SPEC LM_conn_init(M_event_t *event, LM_event_callback_t event_callback, const char *host, M_uint16 port)
{
    LM_conn_t                     *conn;
    const struct M_llist_callbacks trans_delay_rm_cb = {
        NULL, /* equality */
        NULL, /* duplicate_insert */
        NULL, /* duplicate_copy */
        (M_llist_free_func)LM_trans_delete_unlocked
    };

    LM_init(LM_INIT_NORMAL);

    if (event == NULL || event_callback == NULL || M_str_isempty(host) || port == 0)
        return NULL;

    conn                 = M_malloc_zero(sizeof(*conn));
    conn->event          = event;
    conn->event_callback = event_callback;
    conn->status         = LM_CONN_STATUS_DISCONNECTED;

    conn->lock           = M_thread_mutex_create(M_THREAD_MUTEXATTR_NONE);
    conn->rnd            = M_rand_create(0);
    conn->trans_new      = M_queue_create(NULL, LM_conn_trans_destructor);
    conn->trans_ready    = M_queue_create(NULL, LM_conn_trans_destructor);
    conn->trans_pending  = M_queue_create(NULL, LM_conn_trans_destructor);
    conn->trans_done     = M_queue_create(NULL, LM_conn_trans_destructor);
    conn->trans_id_map   = M_hash_u64vp_create(16, 75, M_HASH_U64VP_NONE, NULL);
    conn->trans_delay_rm = M_llist_create(&trans_delay_rm_cb, M_LLIST_NONE);
    conn->outbuf         = M_buf_create();
    conn->inbuf          = M_parser_create(M_PARSER_FLAG_NONE);

    conn->dns            = M_dns_create(event);

    conn->mode           = LM_MODE_TLS;
    conn->conn_timeout   = 10;

    LM_conn_change_server(conn, host, port);

    return conn;
}


void LM_SPEC LM_conn_destroy(LM_conn_t *conn)
{
    if (conn == NULL)
        return;

    M_thread_mutex_lock(conn->lock);
    /* We cannot actually complete the destroy while in an event handler, defer,
     * the event system is responsible for detecting and cleaning this up later */
    if (conn->in_use) {
        conn->destroy = M_TRUE;
        M_thread_mutex_unlock(conn->lock);
        return;
    }
    M_thread_mutex_unlock(conn->lock);

    /* Ok, safe to destroy */
    M_event_timer_remove(conn->timer);
    M_event_trigger_remove(conn->write_trigger);
    M_io_destroy(conn->io);
    M_tls_clientctx_destroy(conn->tlsctx);
    M_dns_destroy(conn->dns);
    M_rand_destroy(conn->rnd);
    M_thread_mutex_destroy(conn->lock);
    M_queue_destroy(conn->trans_new);
    M_queue_destroy(conn->trans_ready);
    M_queue_destroy(conn->trans_pending);
    M_queue_destroy(conn->trans_done);
    M_hash_u64vp_destroy(conn->trans_id_map, M_FALSE);
    M_buf_cancel(conn->outbuf);
    M_parser_destroy(conn->inbuf);
    M_llist_destroy(conn->trans_delay_rm, M_TRUE);
    M_free(conn->host);
    M_free(conn);
}


M_bool LM_SPEC LM_conn_set_userdata(LM_conn_t *conn, void *user_data)
{
    if (conn == NULL)
        return M_FALSE;

    conn->user_data = user_data;
    return M_TRUE;
}


void * LM_SPEC LM_conn_get_userdata(LM_conn_t *conn)
{
    if (conn == NULL)
        return NULL;

    return conn->user_data;
}


M_bool LM_SPEC LM_conn_change_server(LM_conn_t *conn, const char *host, M_uint16 port)
{
    M_bool retval = M_FALSE;

    if (conn == NULL || M_str_isempty(host) || port == 0)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        if (conn->host)
            M_free(conn->host);
        conn->host = M_strdup(host);
        conn->port = port;
        retval = M_TRUE;
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;
}


M_bool LM_SPEC LM_conn_change_mode(LM_conn_t *conn, LM_mode_t mode)
{
    M_bool retval = M_FALSE;

    if (conn == NULL)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        conn->mode = mode;
        retval = M_TRUE;
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;
}


void LM_SPEC LM_conn_disable_ping(LM_conn_t *conn)
{
    if (conn == NULL)
        return;

    conn->disable_ping = M_TRUE;
}


M_bool LM_SPEC LM_conn_set_conn_timeout(LM_conn_t *conn, size_t to_secs)
{
    M_bool retval = M_FALSE;

    if (conn == NULL || to_secs == 0)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        conn->conn_timeout = to_secs;
        retval = M_TRUE;
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;
}

#define LM_CAST_OFF_CONST(type, var) ((type)((M_uintptr)var))
M_bool LM_SPEC LM_conn_set_tls_clientctx(LM_conn_t *conn, const M_tls_clientctx_t *clientctx)
{
    M_bool retval = M_FALSE;
    M_tls_clientctx_t *ctx = LM_CAST_OFF_CONST(M_tls_clientctx_t *, clientctx);

    if (conn == NULL)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        /* Destroy current */
        if (conn->tlsctx) {
            M_tls_clientctx_destroy(conn->tlsctx);
            conn->tlsctx = NULL;
        }

        conn->tlsctx = ctx;
        if (ctx) {
            /* Increase reference count so when we destroy it we don't kill the parent's
             * unexpectedly */
            M_tls_clientctx_upref(ctx);
        }

        retval = M_TRUE;
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;
}


M_bool LM_SPEC LM_conn_set_tls_cert(LM_conn_t *conn, const char *key, size_t key_len, const char *crt, size_t crt_len)
{
    M_bool retval = M_FALSE;

    if (conn == NULL || key == NULL || crt == NULL || key_len == 0 || crt_len == 0)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        if (M_tls_clientctx_set_cert(conn->tlsctx, (const M_uint8 *)key, key_len, (const M_uint8 *)crt, crt_len, NULL, 0)) {
            retval = M_TRUE;
        }
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;
}


M_bool LM_SPEC LM_conn_set_tls_cert_files(LM_conn_t *conn, const char *keypath, const char *crtpath)
{
    M_bool retval = M_FALSE;

    if (conn == NULL || M_str_isempty(keypath) || M_str_isempty(crtpath))
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        if (M_tls_clientctx_set_cert_files(conn->tlsctx, keypath, crtpath, NULL)) {
            retval = M_TRUE;
        }
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;
}


M_bool LM_SPEC LM_conn_set_iocreate_callback(LM_conn_t *conn, LM_conn_iocreate_callback_t cb, void *cb_arg)
{
    M_bool retval = M_FALSE;

    if (conn == NULL)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    /* Allow change if not in the process of connecting */
    if (conn->status != LM_CONN_STATUS_CONNECTING && conn->status != LM_CONN_STATUS_CONNECTED) {
        conn->iocreate_cb    = cb;
        conn->iocreate_cbarg = cb_arg;
        retval = M_TRUE;
    }
    M_thread_mutex_unlock(conn->lock);

    return retval;

}


M_bool LM_SPEC LM_conn_set_idle_timeout(LM_conn_t *conn, size_t to_secs)
{
    if (conn == NULL)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    conn->idle_timeout = to_secs;

    /* Only mess with the timer object while connected.  The timer is used for
     * other purposes in other states */
    if (conn->status == LM_CONN_STATUS_CONNECTED) {
        /* If disabling the timeout while connected, destroy it */
        if (conn->idle_timeout == 0 && conn->timer != NULL) {
            M_event_timer_remove(conn->timer);
            conn->timer = NULL;
        }
        /* If previously didn't have a timer, create one */
        if (conn->idle_timeout != 0 && conn->timer == NULL) {
            conn->timer = M_event_timer_add(conn->event, LM_conn_event_handler, conn);
            M_event_timer_set_firecount(conn->timer, 1);
        }
        /* If no pending transactions, reset the idle timeout to the new value */
        if (conn->idle_timeout && M_queue_len(conn->trans_pending) == 0) {
            M_event_timer_reset(conn->timer, conn->idle_timeout * 1000);
        }
    }
    M_thread_mutex_unlock(conn->lock);

    return M_TRUE;
}



const char * LM_SPEC LM_conn_error(LM_conn_t *conn)
{
    if (conn == NULL)
        return NULL;

    /* XXX: probably not smart, no locking */
    return conn->error;
}

LM_conn_status_t LM_SPEC LM_conn_status(LM_conn_t *conn)
{
    if (conn == NULL)
        return LM_CONN_STATUS_DISCONNECTED;

    return conn->status;
}


/*#define DEBUG_COMMS*/

#ifdef DEBUG_COMMS

static const char *event_type_str(M_event_type_t type)
{
    switch (type) {
        case M_EVENT_TYPE_CONNECTED:
            return "CONNECTED";
        case M_EVENT_TYPE_ACCEPT:
            return "ACCEPT";
        case M_EVENT_TYPE_READ:
            return "READ";
        case M_EVENT_TYPE_WRITE:
            return "WRITE";
        case M_EVENT_TYPE_DISCONNECTED:
            return "DISCONNECT";
        case M_EVENT_TYPE_ERROR:
            return "ERROR";
        case M_EVENT_TYPE_OTHER:
            return "OTHER";
    }
    return "UNKNOWN";
}


static void trace(void *cb_arg, M_io_trace_type_t type, M_event_type_t event_type, const unsigned char *data, size_t data_len)
{
    char       *buf;
    M_timeval_t tv;
    (void)cb_arg;

    M_time_gettimeofday(&tv);
    if (type == M_IO_TRACE_TYPE_EVENT) {
        M_printf("%lld.%06lld: TRACE: event %s\n", tv.tv_sec, tv.tv_usec, event_type_str(event_type));
        return;
    }

    M_printf("%lld.%06lld: TRACE: %s\n", tv.tv_sec, tv.tv_usec, (type == M_IO_TRACE_TYPE_READ)?"READ":"WRITE");
    buf = M_str_hexdump(M_STR_HEXDUMP_DECLEN, 0, NULL, data, data_len);
    M_printf("%s\n", buf);
    M_free(buf);
}
#endif

M_bool LM_SPEC LM_conn_connect(LM_conn_t *conn)
{
    M_bool       retval = M_FALSE;
    M_io_error_t err;

    if (conn == NULL)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);

    /* Clear connection error */
    M_mem_set(conn->error, 0, sizeof(conn->error));

    /* Already connected */
    if (conn->io) {
        M_snprintf(conn->error, sizeof(conn->error), "Already connected");
        goto end;
    }

    /* If we don't already have a clientctx, create one */
    if (!conn->tlsctx) {
        conn->tlsctx = M_tls_clientctx_create();
        M_tls_clientctx_set_default_trust(conn->tlsctx);
        M_tls_clientctx_set_verify_level(conn->tlsctx, M_TLS_VERIFY_FULL);
        M_tls_clientctx_set_negotiation_timeout_ms(conn->tlsctx, conn->conn_timeout * 1000);
        M_tls_clientctx_set_protocols(conn->tlsctx, M_TLS_PROTOCOL_TLSv1_2|M_TLS_PROTOCOL_TLSv1_3);
    }

    /* Create the IO object for the network connection */
    err = M_io_net_client_create(&conn->io, conn->dns, conn->host, conn->port, M_IO_NET_ANY);
    if (err != M_IO_ERROR_SUCCESS) {
        M_snprintf(conn->error, sizeof(conn->error), "%s", M_io_error_string(err));
        goto end;
    }

    /* TCP Keepalives */
    M_io_net_set_keepalives(conn->io, 4 /* idle_s */, 15 /* retry_s */, 3 /* retry_cnt */);

    /* Connect Timeout */
    M_io_net_set_connect_timeout_ms(conn->io, conn->conn_timeout * 1000);

    /* Wrap network connection in TLS if tls is in use */
    if (conn->mode == LM_MODE_TLS) {
        err = M_io_tls_client_add(conn->io, conn->tlsctx, conn->host, NULL);
        if (err != M_IO_ERROR_SUCCESS) {
            M_io_destroy(conn->io);
            conn->io = NULL;
            M_snprintf(conn->error, sizeof(conn->error), "%s", M_io_error_string(err));
            goto end;
        }
    }

#ifdef DEBUG_COMMS
    M_io_add_trace(conn->io, NULL, trace, NULL);
#endif

    /* Allow user to do their own additional low-level wrappers (really, we're thinking
     * debugging or bwshaping here ... ) */
    if (conn->iocreate_cb) {
        M_thread_mutex_unlock(conn->lock);
        conn->iocreate_cb(conn, conn->io, conn->iocreate_cbarg);
        M_thread_mutex_lock(conn->lock);
    }

    /* Register the connection with the event subsystem */
    if (!M_event_add(conn->event, conn->io, LM_conn_event_handler, conn)) {
        M_io_destroy(conn->io);
        conn->io = NULL;
        /* WTF? */
        M_snprintf(conn->error, sizeof(conn->error), "Failed to register connection with event subsystem");
        goto end;
    }

    conn->status = LM_CONN_STATUS_CONNECTING;

    /* Well we initiated the connection sequence anyhow, we'll know when we get a
     * connected event that we ... connected */
    retval = M_TRUE;

end:
    M_thread_mutex_unlock(conn->lock);
    return retval;
}


M_bool LM_conn_disconnect_connlocked(LM_conn_t *conn)
{
    M_bool retval = M_FALSE;

    /* conn is already locked here */

    if (conn->io && conn->status == LM_CONN_STATUS_CONNECTED) {
        /* This should cause a disconnect signal to be generated at some point
         * in the not too distant future */
        M_io_disconnect(conn->io);
        conn->status = LM_CONN_STATUS_DISCONNECTING;
        retval = M_TRUE;
    }

    return retval;
}


M_bool LM_SPEC LM_conn_disconnect(LM_conn_t *conn)
{
    M_bool retval = M_FALSE;

    if (conn == NULL)
        return M_FALSE;

    M_thread_mutex_lock(conn->lock);
    retval = LM_conn_disconnect_connlocked(conn);
    M_thread_mutex_unlock(conn->lock);

    return retval;
}


size_t LM_SPEC LM_conn_trans_count(LM_conn_t *conn, LM_trans_status_t status)
{
    size_t cnt = 0;

    if (conn == NULL)
        return cnt;

    M_thread_mutex_lock(conn->lock);
    if (status == LM_TRANS_STATUS_NEW     || status == LM_TRANS_STATUS_ALL)
        cnt += M_queue_len(conn->trans_new);
    if (status == LM_TRANS_STATUS_READY   || status == LM_TRANS_STATUS_ALL)
        cnt += M_queue_len(conn->trans_ready);
    if (status == LM_TRANS_STATUS_PENDING || status == LM_TRANS_STATUS_ALL)
        cnt += M_queue_len(conn->trans_pending);
    if (status == LM_TRANS_STATUS_DONE    || status == LM_TRANS_STATUS_ALL)
        cnt += M_queue_len(conn->trans_done);
    M_thread_mutex_unlock(conn->lock);

    return cnt;
}


static void LM_conn_trans_list_append(M_list_t *list, M_queue_t *queue)
{
    M_queue_foreach_t *q_foreach = NULL;
    void              *member    = NULL;
    while (M_queue_foreach(queue, &q_foreach, &member)) {
        M_list_insert(list, member);
    }
}


M_list_t * LM_SPEC LM_conn_trans_list(LM_conn_t *conn, LM_trans_status_t status)
{
    M_list_t *list;

    if (conn == NULL)
        return NULL;

    M_thread_mutex_lock(conn->lock);

    list = M_list_create(NULL, M_LIST_NONE);
    if (status == LM_TRANS_STATUS_NEW     || status == LM_TRANS_STATUS_ALL)
        LM_conn_trans_list_append(list, conn->trans_new);
    if (status == LM_TRANS_STATUS_READY   || status == LM_TRANS_STATUS_ALL)
        LM_conn_trans_list_append(list, conn->trans_ready);
    if (status == LM_TRANS_STATUS_PENDING || status == LM_TRANS_STATUS_ALL)
        LM_conn_trans_list_append(list, conn->trans_pending);
    if (status == LM_TRANS_STATUS_DONE    || status == LM_TRANS_STATUS_ALL)
        LM_conn_trans_list_append(list, conn->trans_done);
    M_thread_mutex_unlock(conn->lock);

    return list;
}


LM_trans_t * LM_SPEC LM_conn_get_trans_by_internal_id(LM_conn_t *conn, M_uint64 id)
{
    LM_trans_t *trans;

    if (conn == NULL)
        return NULL;

    M_thread_mutex_lock(conn->lock);

    trans = M_hash_u64vp_get_direct(conn->trans_id_map, id);

    M_thread_mutex_unlock(conn->lock);

    return trans;
}

