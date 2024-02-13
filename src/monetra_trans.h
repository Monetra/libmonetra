#ifndef __LIBMONETRA_TRANS_H__
#define __LIBMONETRA_TRANS_H__

#include "monetra_api.h"

struct LM_trans {
    M_thread_mutex_t         *lock;            /*!< Protect concurrent access */

    LM_conn_t                *conn;            /*!< Pointer to parent connection */
    M_uint64                  trans_id;        /*!< Unique transaction id for connection */
    LM_trans_status_t         status;          /*!< Current status of transaction */

    M_hash_dict_t            *request_params;  /*!< Dictionary of request parameters */
    LM_trans_response_type_t  response_type;   /*!< Type of response (if status == LM_TRANS_STATUS_DONE) */
    M_hash_dict_t            *response_params; /*!< Dictionary of response parameters (if response_type==LM_TRANS_RESPONSE_KV) */
    char                     *response_raw;    /*!< Raw response data if not key/value pair */
    M_csv_t                  *response_csv;    /*!< Parsed CSV response data */

    M_uint64                  timeout_s;       /*!< Seconds to wait until LM_EVENT_TRANS_TIMEOUT is emitted */
    M_event_timer_t          *timeout_timer;   /*!< Timer tracking timeout */

    void                     *user_data;       /*!< User specified data, custom tracking parameter */
};

/* Free the transaction only, don't try to de-register it (used in destructors) */
void LM_trans_free(LM_trans_t *trans);
void LM_trans_send_messages(LM_conn_t *conn);
void LM_trans_delete_unlocked(LM_trans_t *trans);

/* Takes a transaction and writes it to outbuf. Expects conn and trans to be locked. */
void LM_trans_structure(LM_conn_t *conn, LM_trans_t *trans);

/* Expects connection to be locked, hack to handle ping request before fully connected */
LM_trans_t *LM_trans_ping_request(LM_conn_t *conn);

#endif
