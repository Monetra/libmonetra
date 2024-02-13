#include "monetra_api.h"
#include "monetra_trans.h"
#include "monetra_conn.h"

/* conn should be locked when this is called. Needed to be split out for internal
 * to cover the 'hack' that is a ping request on connect. */
static LM_trans_t *LM_trans_new_internal(LM_conn_t *conn)
{
    LM_trans_t *trans;

    trans           = M_malloc_zero(sizeof(*trans));
    trans->lock     = M_thread_mutex_create(M_THREAD_MUTEXATTR_NONE);
    trans->conn     = conn;

    /* Assign transaction id, don't allow duplicates */
    while (1) {
        trans->trans_id = M_rand_range(conn->rnd, 1, M_UINT32_MAX);
        if (M_hash_u64vp_get_direct(conn->trans_id_map, trans->trans_id) == NULL)
            break;
    }

    trans->status   = LM_TRANS_STATUS_NEW;
    return trans;
}


LM_trans_t * LM_SPEC LM_trans_new(LM_conn_t *conn)
{
    LM_trans_t *trans;

    if (conn == NULL)
        return NULL;

    M_thread_mutex_lock(conn->lock);

    trans = LM_trans_new_internal(conn);

    M_queue_insert(conn->trans_new, trans);
    M_hash_u64vp_insert(conn->trans_id_map, trans->trans_id, trans);

    M_thread_mutex_unlock(conn->lock);

    return trans;
}

void LM_trans_free(LM_trans_t *trans)
{
    if (trans == NULL)
        return;

    M_thread_mutex_destroy(trans->lock);
    M_hash_dict_destroy(trans->request_params);
    M_hash_dict_destroy(trans->response_params);
    M_free(trans->response_raw);
    M_csv_destroy(trans->response_csv);
    M_free(trans);
}

static M_queue_t *LM_trans_get_conn_queue(LM_trans_t *trans)
{
    switch (trans->status) {
        case LM_TRANS_STATUS_NEW:
            return trans->conn->trans_new;
        case LM_TRANS_STATUS_READY:
            return trans->conn->trans_ready;
        case LM_TRANS_STATUS_PENDING:
            return trans->conn->trans_pending;
        case LM_TRANS_STATUS_DONE:
            return trans->conn->trans_done;
        default:
            break;
    }
    return NULL;
}


void LM_trans_delete_unlocked(LM_trans_t *trans)
{
    M_queue_t *queue;

    if (trans == NULL)
        return;

    /* Detach from connection */
    queue = LM_trans_get_conn_queue(trans);
    M_queue_take(queue, trans);
    M_hash_u64vp_remove(trans->conn->trans_id_map, trans->trans_id, M_TRUE);

    if (trans->conn->status == LM_CONN_STATUS_CONNECTED && M_queue_len(trans->conn->trans_pending) == 0) {
        M_event_timer_reset(trans->conn->timer, trans->conn->idle_timeout * 1000);
    }

    LM_trans_free(trans);

}


void LM_SPEC LM_trans_delete(LM_trans_t *trans)
{
    LM_conn_t *conn;

    if (trans == NULL)
        return;

    conn = trans->conn;

    M_thread_mutex_lock(conn->lock);

    /* Kill user data as a signal its been deleted incase signals are delivered */
    trans->user_data = NULL;

    /* Kill idle timer */
    if (trans->timeout_timer) {
        M_event_timer_remove(trans->timeout_timer);
        trans->timeout_timer = NULL;
    }

    /* If we are processing events, we must *delay* cleanup */
    if (conn->in_use) {
        M_llist_insert(conn->trans_delay_rm, trans);
        M_thread_mutex_unlock(conn->lock);
        return;
    }

    LM_trans_delete_unlocked(trans);

    M_thread_mutex_unlock(conn->lock);
}


M_bool LM_SPEC LM_trans_set_param(LM_trans_t *trans, const char *key, const char *value)
{
    M_bool retval = M_FALSE;
    if (trans == NULL || M_str_isempty(key))
        return M_FALSE;

    M_thread_mutex_lock(trans->lock);
    if (trans->status == LM_TRANS_STATUS_NEW) {
        if (trans->request_params == NULL)
            trans->request_params = M_hash_dict_create(16, 75, M_HASH_DICT_CASECMP);
        M_hash_dict_insert(trans->request_params, key, value);
        retval = M_TRUE;
    }
    M_thread_mutex_unlock(trans->lock);
    return retval;
}


M_bool LM_SPEC LM_trans_set_param_binary(LM_trans_t *trans, const char *key, const unsigned char *value, size_t value_len)
{
    char  *b64;
    M_bool retval;

    if (trans == NULL || M_str_isempty(key))
        return M_FALSE;

    b64 = M_bincodec_encode_alloc(value, value_len, 0, M_BINCODEC_BASE64);

    retval = LM_trans_set_param(trans, key, b64);
    M_free(b64);

    return retval;
}


M_bool LM_SPEC LM_trans_set_timeout(LM_trans_t *trans, M_uint64 timeout_s)
{
    M_bool rv = M_TRUE;

    if (trans == NULL)
        return M_FALSE;

    M_thread_mutex_lock(trans->lock);

    if (trans->status != LM_TRANS_STATUS_NEW) {
        rv = M_FALSE;
    } else {
        trans->timeout_s = timeout_s;
    }
    M_thread_mutex_unlock(trans->lock);

    return rv;
}


M_bool LM_SPEC LM_trans_send(LM_trans_t *trans)
{
    if (trans == NULL)
        return M_FALSE;

    /* Always lock connection object first when we move queues */
    M_thread_mutex_lock(trans->conn->lock);
    M_thread_mutex_lock(trans->lock);

    /* Check for misuse */
    if (trans->status != LM_TRANS_STATUS_NEW || M_hash_dict_num_keys(trans->request_params) == 0) {
        M_thread_mutex_unlock(trans->lock);
        M_thread_mutex_unlock(trans->conn->lock);
        return M_FALSE;
    }

    /* Move to READY queue */
    trans->status = LM_TRANS_STATUS_READY;
    M_queue_take(trans->conn->trans_new, trans);
    M_queue_insert(trans->conn->trans_ready, trans);

    M_thread_mutex_unlock(trans->lock);

    /* Initiate a connect if not yet connected, if connected, enqueue the message on the wire if connected */
    LM_trans_send_messages(trans->conn);

    M_thread_mutex_unlock(trans->conn->lock);
    return M_TRUE;
}


LM_trans_t * LM_SPEC LM_trans_run(LM_conn_t *conn, const M_hash_dict_t *params)
{
    LM_trans_t *trans;

    if (conn == NULL || params == NULL || M_hash_dict_num_keys(params) == 0)
        return NULL;

    trans = LM_trans_new(conn);
    if (trans == NULL)
        return NULL;

    M_thread_mutex_lock(trans->lock);
    trans->request_params = M_hash_dict_duplicate(params);
    M_thread_mutex_unlock(trans->lock);

    if (!LM_trans_send(trans)) {
        LM_trans_delete(trans);
        return NULL;
    }

    return trans;
}

/*! Private! expects conn to already be locked. This function is really a hack! */
LM_trans_t *LM_trans_ping_request(LM_conn_t *conn)
{
    LM_trans_t *trans = LM_trans_new_internal(conn);
    LM_trans_set_param(trans, "action", "ping");
    /* Force onto PENDING queue */
    trans->status = LM_TRANS_STATUS_PENDING;
    M_queue_insert(trans->conn->trans_pending, trans);
    M_hash_u64vp_insert(conn->trans_id_map, trans->trans_id, trans);
    /* Structure the message onto the wire */
    LM_trans_structure(conn, trans);
    /* Force a write onto the write */
    M_io_write_from_buf(conn->io, conn->outbuf);
    return trans;
}


M_bool LM_SPEC LM_trans_set_userdata(LM_trans_t *trans, void *user_data)
{
    if (trans == NULL)
        return M_FALSE;
    trans->user_data = user_data;
    return M_TRUE;
}


void * LM_SPEC LM_trans_get_userdata(LM_trans_t *trans)
{
    if (trans == NULL)
        return NULL;
    return trans->user_data;
}


M_uint64 LM_SPEC LM_trans_internal_id(const LM_trans_t *trans)
{
    if (trans == NULL)
        return 0;

    /* NOTE: no lock here as this won't change as long as the pointer is valid */
    return trans->trans_id;
}


LM_trans_status_t LM_SPEC LM_trans_status(LM_trans_t *trans)
{
    LM_trans_status_t status;

    if (trans == NULL)
        return 0;

    /* Pretty sure these locks are useless */

    M_thread_mutex_lock(trans->lock);
    status = trans->status;
    M_thread_mutex_unlock(trans->lock);

    return status;
}


LM_trans_response_type_t LM_SPEC LM_trans_response_type(LM_trans_t *trans)
{
    LM_trans_response_type_t resptype;

    if (trans == NULL)
        return 0;

    /* Pretty sure these locks are useless */

    M_thread_mutex_lock(trans->lock);
    resptype = trans->response_type;
    M_thread_mutex_unlock(trans->lock);

    return resptype;
}


const M_hash_dict_t * LM_SPEC LM_trans_response_dict(LM_trans_t *trans)
{
    M_hash_dict_t *response = NULL;

    if (trans == NULL)
        return NULL;

    M_thread_mutex_lock(trans->lock);
    if (trans->status == LM_TRANS_STATUS_DONE && trans->response_type == LM_TRANS_RESPONSE_KV) {
        response = trans->response_params;
    }
    M_thread_mutex_unlock(trans->lock);

    return response;
}


M_list_str_t * LM_SPEC LM_trans_response_keys(LM_trans_t *trans)
{
    M_list_str_t *list = NULL;

    if (trans == NULL)
        return NULL;

    M_thread_mutex_lock(trans->lock);
    if (trans->status == LM_TRANS_STATUS_DONE && trans->response_type == LM_TRANS_RESPONSE_KV) {
        M_hash_dict_enum_t *hashenum = NULL;
        const char         *key      = NULL;

        M_hash_dict_enumerate(trans->response_params, &hashenum);
        list = M_list_str_create(M_LIST_STR_NONE);
        while (M_hash_dict_enumerate_next(trans->response_params, hashenum, &key, NULL)) {
            M_list_str_insert(list, key);
        }
        M_hash_dict_enumerate_free(hashenum);
    }
    M_thread_mutex_unlock(trans->lock);

    return list;
}


const char * LM_SPEC LM_trans_response_param(LM_trans_t *trans, const char *key)
{
    const char *value = NULL;

    if (trans == NULL || M_str_isempty(key))
        return NULL;

    M_thread_mutex_lock(trans->lock);
    if (trans->status == LM_TRANS_STATUS_DONE && trans->response_type == LM_TRANS_RESPONSE_KV) {
        value = M_hash_dict_get_direct(trans->response_params, key);
    }
    M_thread_mutex_unlock(trans->lock);

    return value;
}


unsigned char * LM_SPEC LM_trans_response_param_binary(LM_trans_t *trans, const char *key, size_t *len)
{
    const char *value;

    if (trans == NULL || M_str_isempty(key) || len == NULL)
        return NULL;

    *len = 0;

    value = LM_trans_response_param(trans, key);
    if (value == NULL)
        return NULL;

    return M_bincodec_decode_alloc(value, M_str_len(value), len, M_BINCODEC_BASE64);
}


const char * LM_SPEC LM_trans_response_raw(LM_trans_t *trans)
{
    const char *raw = NULL;

    if (trans == NULL)
        return NULL;

    M_thread_mutex_lock(trans->lock);
    if (trans->status == LM_TRANS_STATUS_DONE && trans->response_type != LM_TRANS_RESPONSE_KV) {
        /* NOTE: this will be NULL if CSV parser is called */
        raw = trans->response_raw;
    }
    M_thread_mutex_unlock(trans->lock);

    return raw;
}


const M_csv_t * LM_SPEC LM_trans_response_csv(LM_trans_t *trans)
{
    M_csv_t *csv = NULL;
    if (trans == NULL)
        return NULL;

    M_thread_mutex_lock(trans->lock);
    if (trans->status == LM_TRANS_STATUS_DONE && trans->response_type == LM_TRANS_RESPONSE_CSV) {
        if (trans->response_csv == NULL && trans->response_raw != NULL) {
            trans->response_csv = M_csv_parse_inplace(trans->response_raw, M_str_len(trans->response_raw), ',', '"', M_CSV_FLAG_TRIM_WHITESPACE);
            trans->response_raw = NULL; /* CSV took ownership! */
        }
        csv = trans->response_csv;
    }
    M_thread_mutex_unlock(trans->lock);
    return csv;
}
