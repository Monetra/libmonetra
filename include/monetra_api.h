#ifndef __MONETRA_API_H__
#define __MONETRA_API_H__

#include <mstdlib/mstdlib.h>
#include <mstdlib/mstdlib_formats.h>
#include <mstdlib/mstdlib_io.h>
#include <mstdlib/mstdlib_tls.h>
#include <libmonetra_exp.h>

#define MONETRAAPI_VERSION 0x080000

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*! \defgroup LM_api Monetra API
 *  Monetra API based on mstdlib.  The supported next-generation library.
 *  Use \#include <monetra_api.h>.
 *
 *  Simple usage example below:
 *
 *  \code{.c}
 *  #include <monetra_api.h>
 *
 *  static void trans_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans)
 *  {
 *  	M_list_str_t *params;
 *  	size_t        i;
 *
 *  	switch (type) {
 *  		case LM_EVENT_CONN_CONNECTED:
 *  			M_printf("Connected successfully\n");
 *  			break;
 *
 *  		case LM_EVENT_CONN_DISCONNECT:
 *  		case LM_EVENT_CONN_ERROR:
 *  			M_printf("Disconnect or Error received: %s\n", LM_conn_error(conn));
 *  			LM_conn_destroy(conn);
 *  			M_event_done(event);
 *  			break;
 *
 *  		case LM_EVENT_TRANS_DONE:
 *  			M_printf("Transaction %p complete:\n", trans);
 *  			params = LM_trans_response_keys(trans);
 *  			for (i=0; i<M_list_str_len(params); i++) {
 *  				const char *key = M_list_str_at(params, i);
 *  				M_printf("\t'%s' = '%s'\n", key, LM_trans_response_param(trans, key));
 *  			}
 *  			M_list_str_destroy(params);
 *  			LM_trans_delete(trans);
 *  			// We're only running a single transaction, issue a graceful disconnect
 *  			LM_conn_disconnect(conn);
 *  			break;
 *
 *  		case LM_EVENT_TRANS_TIMEOUT:
 *  			M_printf("\tTransaction %p timed out\n", trans);
 *  			// Transaction will actually stay enqueued and may later return LM_EVENT_TRANS_DONE.
 *  			break;
 *
 *  		case LM_EVENT_TRANS_ERROR:
 *  		case LM_EVENT_TRANS_NOCONNECT:
 *  			M_printf("Transaction %p error (connectivity): %s\n", trans, LM_conn_error(conn));
 *  			LM_trans_delete(trans);
 *  			// We should still get an LM_EVENT_CONN_ERROR after this event for additional cleanup.
 *  			break;
 *  	}
 *  }
 *
 *  int main(int argc, char **argv)
 *  {
 *  	M_event_t *event    = M_event_create(M_EVENT_FLAG_NONE);
 *  	LM_conn_t *conn     = LM_conn_init(event, trans_callback, "testbox.monetra.com", 8665);
 *  	LM_trans_t *trans;
 *
 *  	LM_conn_connect(conn);
 *
 *  	trans = LM_trans_new(conn);
 *  	LM_trans_set_param(trans, "username", "test_retail:public");
 *  	LM_trans_set_param(trans, "password", "publ1ct3st");
 *  	LM_trans_set_param(trans, "action", "sale");
 *  	LM_trans_set_param(trans, "account", "4012888888881881");
 *  	LM_trans_set_param(trans, "expdate", "1220");
 *  	LM_trans_set_param(trans, "zip", "32606");
 *  	LM_trans_set_param(trans, "cv", "999");
 *  	LM_trans_set_param(trans, "amount", "12.00");
 *  	LM_trans_send(trans);
 *
 *  	M_event_loop(event, M_TIMEOUT_INF);
 *
 *  	// NOTE: we cleaned up 'conn' within the trans_callback.  We can't have
 *  	//       exited the event loop otherwise.
 *  	conn = NULL;
 *
 *  	M_event_destroy(event);
 *
 *  	M_library_cleanup();
 *
 *  	return 0;
 *  }
 *  \endcode
 *
 *
 *  \defgroup LM_init Library Initialization
 *  \ingroup LM_api
 *  Data types and functions specific to library initialization.
 *
 *
 *  \defgroup LM_conn Connection Management
 *  \ingroup LM_api
 *  Data types and functions related to connection management.
 *
 *
 *  \defgroup LM_trans Transaction Management
 *  \ingroup LM_api
 *  Data types and functions related to transaction management.
 *
 */


/*! \addtogroup LM_init
 * @{
 */

/*! Library initialization flags used with LM_init() */
enum LM_init_flags {
	LM_INIT_NORMAL           = 0,      /*!< No special flags                              */
	LM_INIT_SSLLOCK_EXTERNAL = 1 << 0, /*!< Force SSL locking to be controlled externally */
};

/*! @} */


/*! \addtogroup LM_conn
 *  @{
 */

/*! Opaque connection structure */
struct LM_conn;
typedef struct LM_conn LM_conn_t;


/*! Events triggered.  Contains both connection-specific events and transaction-specific
 *  events. */
enum LM_event_type {
	/* Communications-related events */
	LM_EVENT_CONN_CONNECTED  = 0x01,  /*!< Connection established successfully           */
	LM_EVENT_CONN_DISCONNECT = 0x02,  /*!< Graceful remote disconnect. No more events will
	                                   *   arrive after this unless the user requests
	                                   *   connection re-establishment                   */
	LM_EVENT_CONN_ERROR      = 0x03,  /*!< Communications error.  No more events will
	                                   *   arrive after this unless the user requests
	                                   *   connection re-establishment                   */
	/* Transaction-related events */
	LM_EVENT_TRANS_DONE      = 0x10,  /*!< Transaction Complete                          */
	LM_EVENT_TRANS_ERROR     = 0x11,  /*!< Transaction Error (e.g. comms failed).        *
	                                   *    Guaranteed to receive a LM_EVENT_CONN_ERROR  *
	                                   *    after all transactions are notified          */
	LM_EVENT_TRANS_NOCONNECT = 0x12,  /*!< Notification that the transaction is still
	                                   *   ready to be sent, but there was an attempt to
	                                   *   connect that failed.  This transaction is still
	                                   *   eligible to be sent if/when a successful connection
	                                   *   is established.  Integrators may wish to use this
	                                   *   event to destroy the transaction object if they
	                                   *   consider this a critical failure.             */
	LM_EVENT_TRANS_TIMEOUT   = 0x13   /*!< Notification that the user-specified transaction
	                                   *   timeout has elapsed.  The user may choose to
	                                   *   ignore this and continue waiting for a final
	                                   *   event, or clean up the transaction. */
};
typedef enum LM_event_type LM_event_type_t;


/*! Method used to establish a connection and communicate with the remote server */
enum LM_mode {
	LM_MODE_TLS = 0, /*!< Default. SSL/TLS mode */
	LM_MODE_IP  = 1, /*!< Unencrypted IP mode */
	/* Future? XML? */
};
typedef enum LM_mode LM_mode_t;


/*! States indicating the current connection status */
enum LM_conn_status {
	LM_CONN_STATUS_CONNECTING    = 1, /*!< In the process of connecting */
	LM_CONN_STATUS_CONNECTED     = 2, /*!< Actively connected */
	LM_CONN_STATUS_DISCONNECTING = 3, /*!< In the process of disconnecting */
	LM_CONN_STATUS_IDLE_TIMEOUT  = 4, /*!< In the process of disconnecting due to idle timeout */
	LM_CONN_STATUS_DISCONNECTED  = 5  /*!< Connection failed or not yet attempted */
};
typedef enum LM_conn_status LM_conn_status_t;

/*! @} */


/*! \addtogroup LM_trans
 * @{
 */

/*! Opaque transaction structure */
struct LM_trans;
typedef struct LM_trans LM_trans_t;

/*! States indicating the current condition of a transaction */
enum LM_trans_status {
	LM_TRANS_STATUS_ALL     = 1, /*!< All transactions (only relevant for count)           */
	LM_TRANS_STATUS_NEW     = 2, /*!< Transaction not yet sent (being structured)          */
	LM_TRANS_STATUS_READY   = 3, /*!< Transaction in ready state (means not connected yet) */
	LM_TRANS_STATUS_PENDING = 4, /*!< Transaction waiting on response                      */
	LM_TRANS_STATUS_DONE    = 5  /*!< Transaction Finished                                 */
};
typedef enum LM_trans_status LM_trans_status_t;

/*! Possible response types as returned by LM_trans_response_type() */
enum LM_trans_response_type {
	LM_TRANS_RESPONSE_UNKNOWN = 0, /*!< Unknown response type (e.g. response not back yet) */
	LM_TRANS_RESPONSE_KV      = 1, /*!< Response is key/value pairs                        */
	LM_TRANS_RESPONSE_CSV     = 2, /*!< Response is CSV data                               */
	LM_TRANS_RESPONSE_BULK    = 3  /*!< Other bulk data format                             */
};
typedef enum LM_trans_response_type LM_trans_response_type_t;

/*! @} */

/* -------------------------------------------------------------------------- */

/*! \addtogroup LM_init
 * @{
 */

/*! Library Initialization.
 *
 *  Optional, only use if need to make changes from default.
 *  Must be called before any other LM_*() calls if used.  Can be cleaned up using
 *  the mstdlib library destructor M_library_cleanup().
 *
 *  \param[in] flags One ore more enum LM_init_flags
 *  \return M_TRUE if successfully initialized, M_FALSE if previously initialized.
 */
LM_EXPORT M_bool LM_SPEC LM_init(M_uint64 flags /*!< enum LM_init_flags */);

/*! @} */


/* -------------------------------------------------------------------------- */


/*! \addtogroup LM_conn
 *
 * @{
 */

/*! Callback that is called for each event, both connection-related and transaction related.
 *
 *  On connection error, each pending transaction will trigger an ERROR event, _followed_
 *  by the connection error or disconnect event itself.
 *
 *  It is safe to call LM_conn_destroy() or LM_trans_delete() within the callback, however if
 *  there are events still enqueued for a connection when destroyed, they will not be delivered
 *  so it is recommended to only LM_conn_destroy() on LM_EVENT_CONN_DISCONNECT or LM_EVENT_CONN_ERROR.
 *
 *  It is also possible that if LM_trans_delete() is called in the callback that events
 *  for that transaction may still be delivered for a short time after, but it is guaranteed
 *  that if that occurs, the passed in trans pointer will still be valid as actual destruction
 *  will be delayed, but LM_trans_get_userdata() will return NULL (even if LM_trans_set_userdata()
 *  had previously set it to a valid pointer).
 *
 *  \param[in] conn  Connection handle associated with the event
 *  \param[in] event mstdlib event handle that generated the event
 *  \param[in] type  Type of event generated
 *  \param[in] trans Transaction pointer affected if type is LM_EVENT_TRANS_DONE,
 *                   LM_EVENT_TRANS_ERROR, or LM_EVENT_TRANS_NOCONECT.  NULL if type
 *                   is LM_EVENT_CONN_*.
 */
typedef void (*LM_event_callback_t)(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans);

/*! Initialize a connection object.
 *
 *  A connection object must be created before any transactions can be created.  It requires
 *  an initialized mstdlib event handle to be passed to it, which it will use for all
 *  communication/io events.  It also requies a user-written event callback to be registered
 *  that will be called any time there are connection state or transaction state changes.
 *
 *  The default connection mode is SSL/TLS and will create a private TLS client ctx unless
 *  one is provided via LM_conn_set_tls_clientctx().  The default ctx will perform strict
 *  server certificate validation.
 *
 *  \param[in] event           Initialized mstdlib event handle
 *  \param[in] event_callback  User-supplied callback to call on connection and transaction
 *                             state changes.
 *  \param[in] host            Host or IP address of server
 *  \param[in] port            Port of remote server
 *  \return Initialized connection object or NULL on failure.
 */
LM_EXPORT LM_conn_t * LM_SPEC LM_conn_init(M_event_t *event, LM_event_callback_t event_callback, const char *host, M_uint16 port);

/*! Store a user-provided pointer that can be used to easily bind the connection object
 *  to user-provided data.
 *
 *  This may be useful for tracking purposes, especially when in an event handler.
 *  Only one user-supplied pointer may be provided, if this function is called again,
 *  it will replace the existing pointer.
 *
 *  \param[in] conn      Initialized connection object.
 *  \param[in] user_data User-supplied pointer, or NULL to remove.
 *  \return M_TRUE on success, M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_userdata(LM_conn_t *conn, void *user_data);

/*! Retrieve a previously stored user-provided pointer.
 *
 *  see LM_conn_set_userdata() for more info.
 *
 *  \param[in] conn  Initialized connection object.
 *  \return User-supplied data, or NULL on failure.
 */
LM_EXPORT void * LM_SPEC LM_conn_get_userdata(LM_conn_t *conn);


/*! Disable sending a PING request after a connection has been opened to verify
 *  it is really connected to Monetra before allowing transactions to be sent.
 *  The default is to send a PING.
 *
 *  \param[in] conn  Initialized (but not connected) connection object.
 */
LM_EXPORT void LM_SPEC LM_conn_disable_ping(LM_conn_t *conn);


/*! Destroy the connection object.
 *
 *  Destroying a connection object will also destroy any transaction objects associated
 *  with the connection.  Those transaction object pointers MUST NOT be used after the
 *  connection has disappeared.
 *
 *  This function will also ungracefully close the connection if it is still open.  If
 *  the connection is still open, it is recommended to call LM_conn_disconnect() instead
 *  and destroy the connection from within the registered event handler when the disconnect
 *  is complete.
 *
 *  \param[in] conn Initialized connection object.
 */
LM_EXPORT void LM_SPEC LM_conn_destroy(LM_conn_t *conn);

/*! Change the server associated with the connection.
 *
 *  The connection is only allowed to be changed when is in the DISCONNECTED or
 *  DISCONNECTING state.
 *
 *  \param[in] conn Initialized connection object.
 *  \param[in] host Host or IP address of server
 *  \param[in] port Port of remote server
 *  \return M_TRUE on success or M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_change_server(LM_conn_t *conn, const char *host, M_uint16 port);

/*! Change the mode of communication with the server.
 *
 *  The connection is only allowed to be changed when is in the DISCONNECTED or
 *  DISCONNECTING state.
 *
 *  \param[in] conn Initialized connection object.
 *  \param[in] mode Mode to be used, one of LM_mode_t.
 *  \return M_TRUE on success or M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_change_mode(LM_conn_t *conn, LM_mode_t mode);

/*! Change the timeout on the connection.
 *
 *  This is how long to wait to establish a connection with the remote server.
 *  Default is 10 seconds.
 *
 *  \param[in] conn    Initialized connection object.
 *  \param[in] to_secs Timeout in seconds.
 *  \return M_TRUE on success or M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_conn_timeout(LM_conn_t *conn, size_t to_secs);

/*! Set a specific TLS client ctx object to use for communication.
 *
 *  It may be desirable to use a TLS client ctx created externally for setting
 *  specific ciphers and protocols, server trust lists, and verification levels.
 *  Using an external client ctx would also enable the use of session resumption.
 *
 *  If a TLS client ctx is not set, a new clientctx object private to the connection
 *  will be created.  It will use the default ciphers and protocols as defined by
 *  mstdlib, and attempt to load the Certificate Trust list from the OS.  The default
 *  will also perform full certificate validation.
 *
 *  This may only be changed when the connection is in the DISCONNECTED or DISCONNECTING
 *  state.
 *
 *  \param[in] conn      Initialized connection object.
 *  \param[in] clientctx Initialized clientctx object, or NULL to unset the existing.
 *  \return M_TRUE on success or M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_tls_clientctx(LM_conn_t *conn, const M_tls_clientctx_t *clientctx);

/*! Configure a client certificate to present to the server during negotiation from a memory
 *  buffer.
 *
 *  Setting a certificate allows for mutual authentication before a connection can be
 *  established.  The server must be configured to support this.
 *
 *  \param[in] conn    Initialized connection object.
 *  \param[in] key     Memory buffer containing PEM-encoded key data.
 *  \param[in] key_len Length of key memory buffer in bytes.
 *  \param[in] crt     Memory buffer containing PEM-encoded certificate data.
 *  \param[in] crt_len Length of crt memory buffer in bytes.
 *  \return M_TRUE on success or M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_tls_cert(LM_conn_t *conn, const char *key, size_t key_len, const char *crt, size_t crt_len);

/*! Configure a client certificate to present to the server during negotiation from a
 *  file.
 *
 *  Setting a certificate allows for mutual authentication before a connection can be
 *  established.  The server must be configured to support this.
 *
 *  \param[in] conn    Initialized connection object.
 *  \param[in] keypath Path to file containing PEM-encoded key data.
 *  \param[in] crtpath Path to file containing PEM-encoded certificate data.
 *  \return M_TRUE on success or M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_tls_cert_files(LM_conn_t *conn, const char *keypath, const char *crtpath);

/*! Callback registered via LM_conn_set_iocreate_callback() that is called when a new
 *  mstdlib io object is created so the user may manipulate the object.
 *
 *  \param[in]  conn  Pointer to the LM_conn_t object
 *  \param[in]  io    Pointer to the M_io_t internal mstdlib object
 *  \param[in]  arg   User-defined argument registered with callback */
typedef void (*LM_conn_iocreate_callback_t)(LM_conn_t *conn, M_io_t *io, void *arg);

/*! Register a user-defined callback to be called when an mstdlib io object is created
 *  so that the caller may inspect the internals of the io object or add additional
 *  layers such as tracing or bandwidth shaping
 *
 *  This may only be changed when the connection is in the DISCONNECTED or DISCONNECTING
 *  state.
 *
 *  \param[in] conn   Initialized connection object.
 *  \param[in] cb     User-supplied callback (or NULL to unset)
 *  \param[in] cb_arg Argument to provide to user-supplied callback (NULL if unused)
 *  \return M_TRUE on success or M_FALSE on error.
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_iocreate_callback(LM_conn_t *conn, LM_conn_iocreate_callback_t cb, void *cb_arg);


/*! Set an idle connection timer.  This will automatically disconnect if there are no
 *  outstanding transactions in the given timeframe.
 *
 *  The user-registered event handler will be called with a normal disconnect event type
 *  when the timer expires.
 *
 *  \param[in] conn    Initialized connection object.
 *  \param[in] to_secs Idle timeout in seconds, or 0 to disable.
 *  \return M_TRUE on success or M_FALSE on error
 */
LM_EXPORT M_bool LM_SPEC LM_conn_set_idle_timeout(LM_conn_t *conn, size_t to_secs);


/*! Textual reason for a connectivity-related failure.
 *
 *  This function provides a pointer to an internal memory buffer that may
 *  change, and is destroyed with the connection object.  It should not be
 *  kept for long term access.
 *
 *  \param[in] conn  Initialized connection object.
 *  \return string representing error, or NULL if unable to retrieve message.
 */
LM_EXPORT const char * LM_SPEC LM_conn_error(LM_conn_t *conn);

/*! Start a connection sequence.
 *
 *  This function is non-blocking, it will return success immediately and initiate a
 *  background connection process.  The user-supplied callback during initialization
 *  will be called when the connection is established.
 *
 *  This function will be implicitly called by either LM_trans_send() or LM_trans_run().
 *  It is however, recommended that this function be called as it may be necessary for
 *  an application to know if the connection will be successful prior to enqueing
 *  transactions.  The the connection is not successful, any enqueud transactions will
 *  remain in the "READY" state so the connectivity information can be updated and
 *  the connection establishment be attempted again.  Once a connection is established,
 *  any "READY" transactions will be put on the wire.
 *
 *  \param[in] conn  Initialized connection object.
 *  \return M_TRUE if successfully started a connection sequence, M_FALSE on failure
 *          (possibly already connected)
 */
LM_EXPORT M_bool LM_SPEC LM_conn_connect(LM_conn_t *conn);

/*! Obtain the current connection status.
 *
 *  \param[in] conn  Initialized connection object.
 *  \return One of the possible LM_conn_status_t states.
 */
LM_EXPORT LM_conn_status_t LM_SPEC LM_conn_status(LM_conn_t *conn);

/*! Issues a graceful disconnect from the server.
 *
 *  This is a non-blocking function call which will start a background disconnection
 *  sequence which will notify the user-registered callback (set during initialization)
 *  when complete passing an event type of LM_EVENT_CONN_DISCONECT.  Any PENDING
 *  transactions will be abandoned and set into an error state.
 *
 *  This should be called when closing the connection prior to calling LM_conn_destroy(),
 *  When the user has been notified the disconnect sequence is complete, the connection
 *  may be destroyed from within the user callback.
 *
 *  \param[in] conn  Initialized connection object.
 *  \return M_TRUE if sequence successfully started, M_FALSE if connection not eligible
 *          (such as already disconnected)
 */
LM_EXPORT M_bool LM_SPEC LM_conn_disconnect(LM_conn_t *conn);

/*! @} */


/* -------------------------------------------------------------------------- */


/*! \addtogroup LM_trans
 *
 * @{
 */

/*! Retrieves the count of the number of transactions in the queue for the given state(s).
 *
 *  \param[in] conn   Initialized connection object
 *  \param[in] status One of the LM_trans_status_t states
 *  \return Count of matching transactions
 */
LM_EXPORT size_t LM_SPEC LM_conn_trans_count(LM_conn_t *conn, LM_trans_status_t status);

/*! Retrieves the list of the number of transactions in the queue for the given state(s).
 *
 *  \param[in] conn  Initialized connection object.
 *  \param[in] status One of the LM_trans_status_t states.
 *  \return mstdlib M_list_t list of LM_trans_t pointers.  It is the user's responsibility
 *          to clean up this list.
 */
LM_EXPORT M_list_t * LM_SPEC LM_conn_trans_list(LM_conn_t *conn, LM_trans_status_t status);

/*! Start a new transaction.
 *
 *  A new transaction object is initialized and put into the "NEW" state.  A transaction is
 *  not sent until parameters have been added and the LM_trans_send() function is called.
 *
 *  The connection does not need to be in an established state to create a transaction.
 *
 *  \param[in] conn Initialized connection object.
 *  \return Initialized LM_trans_t object
 */
LM_EXPORT LM_trans_t * LM_SPEC LM_trans_new(LM_conn_t *conn);

/*! Adds a string key/value pair to a transaction object.
 *
 *  Transactions are made up of a series of string-based key/value pairs.  The key must be
 *  a non-zero length string.  The value may be zero-length or NULL, such as when un-setting
 *  a parameter.
 *
 *  The transaction must be in the NEW state.
 *
 *  \param[in] trans Initialized transaction object by LM_trans_new()
 *  \param[in] key   NULL-terminated string representing the key, must be non-zero length.
 *                   The key will be fully duplicated into the transaction object.
 *  \param[in] value NULL-terminated string representing the value (may be zero-length or NULL).
 *                   The value will be fully duplicated into the transaction object.
 *  \return M_TRUE on success, M_FALSE on error.
 */
LM_EXPORT M_bool LM_SPEC LM_trans_set_param(LM_trans_t *trans, const char *key, const char *value);

/*! Adds a string key with a binary value to the transaction object.
 *
 *  Transactions are made up of a series of string-based key/value pairs.  The key must be
 *  a non-zero length string.  The value may be zero-length or NULL, such as when un-setting
 *  a parameter.  Binary values are converted into string-based representation by base64
 *  encoding them, however only values expected to be base64 encoded must be.  Please reference
 *  the key/value pair manual to determine which values are expected to be base64 encoded.
 *
 *  The transaction must be in the NEW state.
 *
 *  \param[in] trans     Initialized transaction object by LM_trans_new()
 *  \param[in] key       NULL-terminated string representing the key, must be non-zero length.
 *                       The key will be fully duplicated into the transaction object.
 *  \param[in] value     Binary value data. The value will be fully duplicated into the transaction
 *                       object.
 *  \param[in] value_len Length in bytes of binary value data.
 *  \return M_TRUE on success, M_FALSE on error.
 */
LM_EXPORT M_bool LM_SPEC LM_trans_set_param_binary(LM_trans_t *trans, const char *key, const unsigned char *value, size_t value_len);

/*! Set a user-specified timeout in seconds for notification purposes.
 *
 *  Sets a notification timeout on the transaction.  When the timeout elapses, an
 *  LM_EVENT_TRANS_TIMEOUT event will be delivered.  This event does NOT however
 *  cancel the transaction and instead is simply a notification that the user-specified
 *  timer has elapsed.  The user may choose to delete the transaction, or continue
 *  waiting for another signal such as LM_EVENT_TRANS_DONE.
 *
 *  Only transactions that have not had LM_trans_send() called are eligible to be
 *  set for timeout.
 *
 *  The timeout timer starts once the transaction has been put on the wire, so if
 *  there are errors connecting, the timer will not start until a connection has
 *  been established.  The timer is bound to the same event loop as the established
 *  connection so it is guaranteed they will signal using the same thread.
 *
 *  \param[in] trans     Initialized transaction object by LM_trans_new()
 *  \param[in] timeout_s Seconds after sending for the timeout signal to be delivered.
 *  \return M_TRUE if successfully set, M_FALSE on misuse.
 */
LM_EXPORT M_bool LM_SPEC LM_trans_set_timeout(LM_trans_t *trans, M_uint64 timeout_s);

/*! Send the transaction.
 *
 *  Marks the transaction as being ready to be sent.  If the connection is not yet established,
 *  the transaction will move to the READY state, if the transaction is already established,
 *  the transaction will be put on the wire and moved to the PENDING state.  The transaction
 *  must have at least one parameter associated with it.
 *
 *  If the connection is not already established, this function will implicitly start a
 *  connection sequence,  this includes re-connecting when a connection has been lost.
 *
 *  \param[in] trans Initialized transaction object by LM_trans_new()
 *  \return M_TRUE on success, M_FALSE on failure
 */
LM_EXPORT M_bool LM_SPEC LM_trans_send(LM_trans_t *trans);

/*! Run a transaction based on a Dictionary object.
 *
 *  Equivalent of LM_trans_new(), followed by enumerating all members of the Dictionary
 *  and inserting them with LM_trans_set_param(), followed by LM_trans_send().  This is
 *  a convenience function if transactional data is already stored within a Dictionary.
 *  The Dictionary will be duplicated.
 *
 *  If the connection is not already established, this function will implicitly start a
 *  connection sequence,  this includes re-connecting when a connection has been lost.
 *
 *  \param[in] conn Initialized connection object
 *  \param[in] params Dictionary with members.  Will be duplicated by transaction object.
 *  \return Initialized LM_trans_t object in either the READY or PENDING state.
 */
LM_EXPORT LM_trans_t * LM_SPEC LM_trans_run(LM_conn_t *conn, const M_hash_dict_t *params);

/*! Store a user-provided pointer that can be used to easily bind the transaction object
 *  to user-provided data.
 *
 *  This may be useful for tracking purposes, especially when in an event handler.
 *  Only one user-supplied pointer may be provided, if this function is called again,
 *  it will replace the existing pointer.
 *
 *  \param[in] trans     Initialized transaction object.
 *  \param[in] user_data User-supplied pointer, or NULL to remove.
 *  \return M_TRUE on success, M_FALSE on failure.
 */
LM_EXPORT M_bool LM_SPEC LM_trans_set_userdata(LM_trans_t *trans, void *user_data);

/*! Retrieve a previously stored user-provided pointer.
 *
 *  see LM_trans_set_userdata() for more info.
 *
 *  \param[in] trans  Initialized transaction object.
 *  \return User-supplied data, or NULL on failure.
 */
LM_EXPORT void * LM_SPEC LM_trans_get_userdata(LM_trans_t *trans);


/*! Retrieve an integer representing the internal id of a transaction.
 *
 *  This is the same internal identifier that gets sent to the server and is unique
 *  across all transactions associated with the same connection object.  It is
 *  randomly chosen an unlikely to conflict with other connection objects.
 *
 *  Typically used by the legacy wrappers, but may be used by integrators rather
 *  than tracking the pointers returned for safety purposes.  In particular,
 *  since transactions are cleaned up by a connection destroy, this can be used
 *  to prevent dangling pointers by using LM_conn_get_trans_by_internal_id()
 *  to retrieve the transaction object pointer.
 *
 *  \param[in] trans  Initialized transaction object.
 *  \return Integer representing internal id of transaction, or 0 on error
 */
LM_EXPORT M_uint64 LM_SPEC LM_trans_internal_id(const LM_trans_t *trans);

/*! Retrieve the transaction object based on the internal transaction id
 *  returned by LM_trans_internal_id().
 *
 *  \param[in] conn  Initialized connection object.
 *  \param[in] id    Internal transaction id as returned by LM_trans_internal_id().
 *  \return Transaction object or NULL if not found.
 */
LM_EXPORT LM_trans_t * LM_SPEC LM_conn_get_trans_by_internal_id(LM_conn_t *conn, M_uint64 id);

/*! Delete the transaction
 *
 *  Clears all resources associated with a transaction.  If the transaction is not
 *  yet complete, any response will be abandoned.  It is recommended that transactions
 *  be deleted when no longer needed.
 *
 *  \param[in] trans Initialized transaction object
 */
LM_EXPORT void LM_SPEC LM_trans_delete(LM_trans_t *trans);

/*! Retrieve the current transaction status in the lifecycle.
 *
 *  \param[in] trans Initialized transaction object.
 *  \return One of LM_trans_status_t states.
 */
LM_EXPORT LM_trans_status_t LM_SPEC LM_trans_status(LM_trans_t *trans);


/*! Retrieve the response type/format.
 *
 *  Useful for determining if the response is Key/Value pair data or CSV.
 *
 *  Only valid to be called on transactions in the "DONE" state.  Otherwise this
 *  will return LM_TRANS_RESPONSE_TYPE_UNKNOWN.
 *
 *  \param[in] trans Initialized transaction object.
 *  \return One of LM_trans_response_type_t types.
 */
LM_EXPORT LM_trans_response_type_t LM_SPEC LM_trans_response_type(LM_trans_t *trans);


/*! Retrieve a pointer to the internal dictionary of response key/value pairs.
 *
 *  This pointer is automatically cleaned up when the transaction is destroyed and
 *  should be duplicated results are necessary past the lifecycle of a transaction.
 *  NOTE: Destroying a connection object will also destroy a transaction object.
 *
 *  \param[in] trans Initialized transaction object.
 *  \return Dictionary of response key/value pairs or NULL on error (transaction not
 *          done or not a KV response)
 */
LM_EXPORT const M_hash_dict_t * LM_SPEC LM_trans_response_dict(LM_trans_t *trans);

/*! Retrieve a list of response string keys for Key/Value pair responses.
 *
 *  The string list returned must be cleaned up by the caller.
 *
 *  \param[in] trans  Initialized transaction object.
 *  \return String List of response keys that must be cleaned up by the caller, or NULL
 *          on failure (transaction not done or not a KV response)
 */
LM_EXPORT M_list_str_t * LM_SPEC LM_trans_response_keys(LM_trans_t *trans);

/*! Retrieve a string value associated with a response key for Key/Value pair responses.
 *
 *  This pointer is automatically cleaned up when the transaction is destroyed and
 *  should be duplicated results are necessary past the lifecycle of a transaction.
 *  NOTE: Destroying a connection object will also destroy a transaction object.
 *
 *  \param[in] trans  Initialized transaction object.
 *  \param[in] key    String key to retrieve value for.
 *  \return String value associated with key, or NULL on failure (transaction not done,
 *          key not found, or not a KV response).
 */
LM_EXPORT const char * LM_SPEC LM_trans_response_param(LM_trans_t *trans, const char *key);

/*! Retrieve a binary value associated with a response key for Key/Value pair responses.
 *
 *  Binary values are returned by the server in Base64 format, this is a convenience function
 *  to automatically decode the data.  It can only be used on values that were returned in
 *  base64 format by the server.
 *
 * The returned pointer must be cleaned up by the caller.
 *
 *  \param[in]  trans  Initialized transaction object.
 *  \param[in]  key    String key to retrieve value for.
 *  \param[out] len    Pointer to store length of result.
 *  \return Binary decoded value associated with key (caller must free), or NULL on failure
 *         (transaction not done, key not found, not a KV response, not base64 encoded).
 */
LM_EXPORT unsigned char * LM_SPEC LM_trans_response_param_binary(LM_trans_t *trans, const char *key, size_t *len);

/*! Retrieve a pointer to the raw response for CSV or BLOB response data.
 *
 *  When not Key/Value pair response data, the raw response may be retrieved as long
 *  as LM_trans_response_csv() has not yet been called.  It is no longer available
 *  after CSV parsing has occurred due to the data being modified by the parser.
 *
 *  This pointer is automatically cleaned up when the transaction is destroyed and
 *  should be duplicated results are necessary past the lifecycle of a transaction.
 *  NOTE: Destroying a connection object will also destroy a transaction object.
 *
 * \param[in] trans  Initialized transaction object.
 * \return Pointer to CSV or BLOB raw data, or NULL on failure (transaction not done,
 *         or not CSV or BLOB data)
 */
LM_EXPORT const char * LM_SPEC LM_trans_response_raw(LM_trans_t *trans);


/*! Retrieve a pointer to the Parsed csv response data.
 *
 *  If the CSV data has not yet been parsed, it will be parsed when this is called
 *  the first time.  Subsequent calls will simply return the internal cached
 *  pointer.
 *
 *  This pointer is automatically cleaned up when the transaction is destroyed and
 *  should be duplicated results are necessary past the lifecycle of a transaction.
 *  NOTE: Destroying a connection object will also destroy a transaction object.
 *
 * \param[in] trans  Initialized transaction object.
 * \return Pointer to parsed CSV data, or NULL on failure (transaction not done, or
 *         not CSV data).
 */
LM_EXPORT const M_csv_t * LM_SPEC LM_trans_response_csv(LM_trans_t *trans);

/*! @} */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBMONETRA_H__ */
