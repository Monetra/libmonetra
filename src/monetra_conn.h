#ifndef __LIBMONETRA_CONN_H__
#define __LIBMONETRA_CONN_H__

#include "monetra_api.h"
#include "mstdlib/mstdlib_tls.h"

struct LM_conn {
	M_thread_mutex_t          *lock;            /*!< Protect concurrent access */
	M_rand_t                  *rnd;             /*!< Random number generator instance for internal transaction id generation */

	/* Transactions in various states */
	M_queue_t                 *trans_new;      /*!< List of transactions being structured */
	M_queue_t                 *trans_ready;    /*!< List of transactions ready to be sent
	                                     * (only real reason not sent is probably because connection isn't established yet) */
	M_queue_t                 *trans_pending;  /*!< List of transactions waiting on a response */
	M_queue_t                 *trans_done;     /*!< Transactions in a DONE or ERROR state */
	M_hash_u64vp_t            *trans_id_map;   /*!< Cross reference of transaction ids to pointers */

	/* State tracking data */
	LM_conn_status_t           status;         /*!< Whether or not the connection is established and ready for transactions.
	                                             *   The underlying IO object may be connected, but we may not have finished
	                                             *   all initialization steps until this is set appropriately */
	M_io_t                     *io;             /*!< Base communications object */
	M_tls_clientctx_t          *tlsctx;         /*!< TLS client CTX/state */
	M_dns_t                    *dns;            /*!< DNS instance */
	M_bool                      in_use;         /*!< Connection object is in use processing events, cannot be destroyed */
	M_llist_t                  *trans_delay_rm; /*!< List of transactions to delay deleting due to "in_use" */

	M_bool                      destroy;        /*!< A deferred destroy request while processing events */

	M_buf_t                    *outbuf;         /*!< Data meant to go on the wire to Monetra */
	M_parser_t                 *inbuf;          /*!< Data from Monetra to us */
	M_event_timer_t            *timer;          /*!< Timer for PING request */
	M_event_trigger_t          *write_trigger;  /*!< Trigger for sending fake WRITE events when aggregating transactions */
	LM_trans_t                 *pingtxn;        /*!< Pointer to ping transaction so we can delete it */

	char                        error[256];     /*!< Error message buffer   */

	/* Configuration Data */
	char                       *host;           /*!< Host to connect to     */
	M_uint16                    port;           /*!< Port to connect to     */
	LM_mode_t                   mode;           /*!< Set mode TLS/IP        */
	M_bool                      disable_ping;   /*!< Disable PING on connect */
	M_event_t                  *event;          /*!< Event handle to bind io object to */
	size_t                      conn_timeout;   /*!< Set connection timeout */
	size_t                      idle_timeout;   /*!< Idle connection timeout */
	LM_event_callback_t         event_callback; /*!< Registered callback to send events */
	void                       *user_data;      /*!< User data for custom tracking */
	LM_conn_iocreate_callback_t iocreate_cb;    /*!< Callback for io object creation */
	void                       *iocreate_cbarg; /*!< Callback for io object creation user argument */
};

/*! General event handler */
void LM_conn_event_handler(M_event_t *event, M_event_type_t type, M_io_t *io, void *user_arg);
void LM_trans_event_handler(M_event_t *event, M_event_type_t type, M_io_t *io, void *user_arg);


/*! Event handler specific to requesting a fake write event used for aggregating multiple
 *  transaction writes into one */
void LM_conn_event_handler_writetrigger(M_event_t *event, M_event_type_t type, M_io_t *io, void *user_arg);


enum LM_conn_parse_error {
	LM_CONN_PARSE_ERROR_SUCCESS = 0, /* Extracted a transaction */
	LM_CONN_PARSE_ERROR_WAIT    = 1, /* Need more data (may have processed data such as orphaned transaction) */
	LM_CONN_PARSE_ERROR_ERROR   = 2  /* Error during parsing */
};
typedef enum LM_conn_parse_error LM_conn_parse_error_t;

LM_conn_parse_error_t LM_conn_parse_trans_buffer(LM_conn_t *conn, LM_trans_t **trans);

/* Same as LM_conn_disconnect(), but assumes you're already holding the lock */
M_bool LM_conn_disconnect_connlocked(LM_conn_t *conn);

#endif
