#ifndef __LIBMONETRA_LEGACY_H__
#define __LIBMONETRA_LEGACY_H__

#include <libmonetra_exp.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! \addtogroup LM_legacy Legacy API
 *
 * Legacy defines, data structures, and functions.  Use \#include <monetra.h>.
 * If used with mstdlib, mstdlib must be included prior to <monetra.h>
 *
 * @{
 */

/* Transaction States */
#define   M_UNUSED    0
#define   M_NEW       100
#define   M_PENDING   1
#define   M_DONE      2

/* Transaction Results */
#ifdef M_ERROR
#    undef M_ERROR
#endif
#define   M_ERROR     -1
#define   M_FAIL       0
#define   M_SUCCESS    1


/* make M_CONN opaque */
struct _M_CONN;
typedef struct _M_CONN * M_CONN;

enum m_ssllock_style {
    M_SSLLOCK_NONE = 0,     /*!< No locking method defined. Should not be set by a user. */
    M_SSLLOCK_EXTERNAL = 1, /*!< User is responsible for initializing OpenSSL and its locks. */
    M_SSLLOCK_INTERNAL = 2  /*!< libmonetra is responsible for all OpenSSL code. OpenSSL should
                             *   not be used outside of libmonetra calls. */
};

/* ---------------------------------------- */

/*! Must be called before any other API calls. Mainly used to initialize SSL
 * calls; on Windows, it also calls WSAStartup() to initialize sockets.
 *
 * \param[in] lockstyle
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_InitEngine_ex(enum m_ssllock_style lockstyle);

/*! \deprecated Use M_InitEngine_ex() instead.
 *
 * Must be called before any other API calls. Mainly used to initialize SSL
 * calls; on Windows, it also calls WSAStartup() to initialize sockets.
 *
 * \param[in] location Should always be NULL. Use M_SetSSL_CAfile() to set location.
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_InitEngine(const char *location);

/*! Frees memory allocated by M_InitEngine() or M_InitEngine_ex(). Should be
 * called just before a program terminates.
 */
LM_EXPORT void LM_SPEC M_DestroyEngine(void);

/*! Allocates memory for, and initializes, an #M_CONN.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 */
LM_EXPORT void LM_SPEC M_InitConn(M_CONN *conn);

/* ---------------------------------------- */

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_SetProxyHost(M_CONN *conn, const char *host, unsigned short port, const char *type);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_SetProxyTimeout(M_CONN *conn, int timeout);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_SetProxyUser(M_CONN *conn, const char *username, const char *password);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_SetProxyLocalNets(M_CONN *conn, const char *localnets, char *error, int errlen);

/* ---------------------------------------- */

/*! Specifies whether to wait for a transaction to finish before returning from
 * an M_TransSend() or (legacy) M_Sale(), etc. (blocking), or to return
 * immediately and require client to check status (non-blocking).
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] tf 1 if blocking is desired, 0 if blocking is not desired
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_SetBlocking(M_CONN *conn, int tf);

/*! Set log level. Logs are kept in /tmp/libmonetra-%d.log where %d is the PID
 * of the running process.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] level debug level. Currently only '1' is used.
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_SetLogging(M_CONN *conn, int level);

/*! Sets the maximum amount of time a transaction can take before timing out.
 * This value gets sent to the Monetra server, causing it to start sending a TIMEOUT
 * response. libmonetra itself does not time out.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] timeout maximum number of seconds to wait for completion of transaction
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_SetTimeout(M_CONN *conn, long timeout);

/*! \deprecated DropFile support has been removed.
 *
 * DropFile support has been removed; M_SetDropFile() does not do anything.
 *
 * \param conn ignored
 * \param df_location ignored
 * \return always 1
 */
LM_EXPORT int LM_SPEC M_SetDropFile(M_CONN *conn, const char *df_location);

/*! Sets hostname and port to use for IP connections.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] host hostname or IP address to which to connect
 * \param[in] port port associated with hostname or IP address
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_SetIP(M_CONN *conn, const char *host, unsigned short port);

/*! Sets an #M_CONN to use SSL.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] host hostname or IP address to which to connect
 * \param[in] port port associated with hostname or IP address
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_SetSSL(M_CONN *conn, const char *host, unsigned short port);

/*! Sets the CA file location. Use before establishing a connection to a
 * Monetra server. Used to verify the server's SSL certificate.
 *
 * If both dir and file are set only dir will be used.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] path path of CA file
 * \return 1 on success, 0 on failure
 *
 * \see M_SetSSL_CAdir
 */
LM_EXPORT int LM_SPEC M_SetSSL_CAfile(M_CONN *conn, const char *path);

/*! Sets the CA directory location. Use before establishing a connection to a
 * Monetra server. Used to verify the server's SSL certificate. All .pem
 * files within this directory and any subdirectories will be used.
 *
 * If both dir and file are set only dir will be used.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] path path of CA directory.
 * \return 1 on success, 0 on failure
 *
 * \see M_SetSSL_CAfile
 */
LM_EXPORT int LM_SPEC M_SetSSL_CAdir(M_CONN *conn, const char *path);

/*! Sets the client certificate and key used for verification if the Monetra
 * server has client SSL certificate verification enabled.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] sslkeyfile path of SSL key file
 * \param[in] sslcertfile path of SSL certificate file
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_SetSSL_Files(M_CONN *conn, const char *sslkeyfile, const char *sslcertfile);

/*! Tells the library whether or not to send a PING request to the server once
 * a connection has been established.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] tf 1 if connection verification is desired, 0 otherwise
 */
LM_EXPORT void LM_SPEC M_VerifyConnection(M_CONN *conn, int tf);

/*! Tells the library whether or not to verify that the SSL certificate
 * provided by the Monetra server has been signed by a proper CA.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]     level 0 for no validation,
 *                      1 for SSL certificate validation plus full domain matching,
 *                      2 for SSL certificate validation plus fuzzy domain matching,
 *                      3 for SSL certificate validation only
 */
LM_EXPORT void LM_SPEC M_VerifySSLCert(M_CONN *conn, int level);

/*! Tells the library whether or not the identifiers used for transactions
 * should be validated from within the connection structure before assuming they
 * are correct. Since the transaction identifier is actually a pointer, passing an
 * incorrect address without validation can cause segmentation faults. This is
 * usually not necessary for C programs, but is helpful for writing modules for
 * other languages.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] tf 1 if verification desired, 0 if verification not desired
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_ValidateIdentifier(M_CONN *conn, int tf);

/*! Once all connection parameters have been set, this function establishes the
 * connection to the Monetra server.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Connect(M_CONN *conn);

/*! Issue a disconnect.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Disconnect(M_CONN *conn);

/*! Sets the maximum amount of time to spend trying to connect to the Monetra
 * server. This only has an effect when there are network problems; sets the
 * socket non-blocking.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] maxtime maximum number of seconds to wait to establish IP/SSL connection
 */
LM_EXPORT void LM_SPEC M_MaxConnTimeout(M_CONN *conn, int maxtime);

/*! If M_Connect() fails, this function may provide some text about what went wrong.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return error message string
 */
LM_EXPORT const char *LM_SPEC M_ConnectionError(M_CONN *conn);

/*! Disconnects from Monetra server and deallocates any memory associated with
 *  the #M_CONN.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 */
LM_EXPORT void LM_SPEC M_DestroyConn(M_CONN *conn);

/*! Performs all communication with the Monetra server. If this function never
 * gets called, no transactions will be processed. This function is
 * non-blocking.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return 1 on success (connection alive), 0 on disconnect, -1 on critical error
 */
LM_EXPORT int LM_SPEC M_Monitor(M_CONN *conn);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_Monitor_ex(M_CONN *conn, long wait_us);

/* ---------------------------------------- */

/*! Returns the number of transactions sent to the Monetra server from this
 * connection that have not already been deleted.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return number of undeleted transactions sent to Monetra server from this connection
 */
LM_EXPORT int LM_SPEC M_TransactionsSent(M_CONN *conn);

/*! Removes a transaction from the queue that was initialized with M_TransNew().
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 */
LM_EXPORT void LM_SPEC M_DeleteTrans(M_CONN *conn, M_uintptr identifier);
#define M_DeleteResponse(a, b) M_DeleteTrans(a, b)

/*! Starts a new transaction. This must be called to obtain an identifier
 * before any transaction parameters may be added.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return identifier for transaction
 */
LM_EXPORT M_uintptr LM_SPEC M_TransNew(M_CONN *conn);

/*! Adds a key/value pair to a transaction.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] key key as per Monetra Client Interface Protocol Specification
 * \param[in] value value as per Monetra Client Interface Protocol Specification
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_TransKeyVal(M_CONN *conn, M_uintptr identifier, const char *key, const char *value);

/*! Adds a key/value pair to a transaction. The value must be a binary format
 * such as an image for a key which expects binary data. The value with be base64
 * encoded before being sent to the Monetra server.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] key key as per Monetra Client Interface Protocol Specification
 * \param[in] value value as per Monetra Client Interface Protocol Specification
 * \param[in] value_len length of value
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_TransBinaryKeyVal(M_CONN *conn, M_uintptr identifier, const char *key, const char *value, int value_len);

/*! Finalizes a transaction and sends it to the Monetra server.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_TransSend(M_CONN *conn, M_uintptr identifier);

/* ---------------------------------------- */

/*! Used to retrieve the response key/value pairs from the Monetra server.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] key key as per Monetra Client Interface Protocol Specification
 * \return If key found, value associated with key, otherwise NULL.
 */
LM_EXPORT const char *LM_SPEC M_ResponseParam(M_CONN *conn, M_uintptr identifier, const char *key);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT unsigned char *LM_SPEC M_ResponseBinaryParam(M_CONN *conn, M_uintptr identifier, const char *key, int *out_length);

/*! Retrieves response keys from the Monetra server for a particular
 * transaction. Useful so you can pull the value using M_ResponseParam() for each
 * key.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[out] num_keys number of keys returned
 * \return keys as an array of strings
 */
LM_EXPORT char **LM_SPEC M_ResponseKeys(M_CONN *conn, M_uintptr identifier, int *num_keys);

/*! This function is primarily for languages such as VB.Net or Delphi that do
 * not easily support direct C arrays. It is typically more efficient to directly
 * access the array member. The index starts at 0.
 *
 * \param[in] keys returned from M_ResponseKeys()
 * \param[in] num_keys number of keys in array (set by M_ResponseKeys())
 * \param[in] idx index of element to retrieve
 * \return the string stored at the index specified, or NULL if invalid
 */
LM_EXPORT const char *LM_SPEC M_ResponseKeys_index(char **keys, int num_keys, int idx);

/*! Frees the memory allocated by M_ResponseKeys().
 *
 * \param[in] keys returned from M_ResponseKeys()
 * \param[in] num_keys number of keys in array (set by M_ResponseKeys())
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_FreeResponseKeys(char **keys, int num_keys);

/* ---------------------------------------- */

/*! Returns whether a particular transaction succeeded or failed. If a
 * detailed code is needed, please use M_ResponseParam() to retrieve the "code"
 * response key/value pair.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return 1 if transaction succeeded (authorization), 0 if transaction failed (denial)
 */
LM_EXPORT int LM_SPEC M_ReturnStatus(M_CONN *conn, M_uintptr identifier);


/*! Returns the total number of transactions in the queue. It does not matter
 * what state the transactions are in or if they have been sent or not.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return number of transactions in the connection queue
 */
LM_EXPORT long LM_SPEC M_TransInQueue(M_CONN *conn);

/*! Returns the state of the transaction, whether or not processing has been
 * completed or is still pending.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return M_PENDING (1) if still being processed, M_DONE (2) if complete, <= 0 on critical error
 */
LM_EXPORT int LM_SPEC M_CheckStatus(M_CONN *conn, M_uintptr identifier);

/*! Returns the number of transactions that have been completed and loads the
 * list of identifiers into listings.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[out] listings identifier for each listing that is complete
 * \return number of transactions in the queue that have been completed
 */
LM_EXPORT long LM_SPEC M_CompleteAuthorizations(M_CONN *conn, M_uintptr **listings);

/*! This function is primarily for languages such as VB.Net or Delphi that do
 * not easily support direct C arrays. It is typically more efficient to directly
 * access the array member. The index starts at 0.
 *
 * \param[in] listings returned from M_CompleteAuthorizations()
 * \param[in] num_listings number of elements in array (returned by M_CompleteAuthorizations())
 * \param[in] idx index of element to retrieve
 * \return data stored at the indexed position
 */
LM_EXPORT M_uintptr LM_SPEC M_CompleteAuthorizations_index(M_uintptr *listings, int num_listings, int idx);

/*! Frees memory allocated by M_CompleteAuthorizations(). Equivalent to
 * calling free(listings) in C.
 *
 * \param[in] listings returned from M_CompleteAuthorizations()
 * \param[in] num_listings number of elements in array (returned by M_CompleteAuthorizations())
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_FreeCompleteAuthorizations(M_uintptr *listings, int num_listings);

/* ---------------------------------------- */

/*! Returns whether a response is comma-delimited.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return 1 if response is comma-delimited, 0 if not
 */
LM_EXPORT int LM_SPEC M_IsCommaDelimited(M_CONN *conn, M_uintptr identifier);

/*! Tells the library to use its internal parsing commands to parse a comma
 * delimited response. Must be called before calling M_GetCell(),
 * M_GetCellByNum(), M_GetHeader(), M_NumRows(), or M_NumColumns().
 *
 * \warning If you call M_ParseCommaDelimited(), you can no longer call
 * M_GetCommaDelimited() because M_ParseCommaDelimited() destroys the data.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_ParseCommaDelimited(M_CONN *conn, M_uintptr identifier);

/*! Returns raw comma-delimited data.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return raw transaction data
 */
LM_EXPORT const char *LM_SPEC M_GetCommaDelimited(M_CONN *conn, M_uintptr identifier);

/*! Returns a single cell from comma-delimited data. M_ParseCommaDelimited()
 * must be called beforehand.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] column text key name for cell
 * \param[in] row row number
 * \return data for particular cell
 */
LM_EXPORT const char *LM_SPEC M_GetCell(M_CONN *conn, M_uintptr identifier, const char *column, long row);

/*! Returns a single cell from comma-delimited data. M_ParseCommaDelimited()
 * must be called beforehand.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] column column number
 * \param[in] row row number
 * \return data for particular cell
 */
LM_EXPORT const char *LM_SPEC M_GetCellByNum(M_CONN *conn, M_uintptr identifier, int column, long row);

/*! Returns a single cell from comma-delimited data. M_ParseCommaDelimited() *
 * must be called beforehand. If the cell exists, an attempt will be made to
 * base64 the data within. This function should only be used on cells known to
 * contain binary data.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] column text key name for cell
 * \param[in] row row number
 * \param[out] out_length length of data
 * \return data for particular cell. Must be freed using free() or M_FreeBinaryCell().
 */
LM_EXPORT char *LM_SPEC M_GetBinaryCell(M_CONN *conn, M_uintptr identifier, const char *column, long row, int *out_length);

/*! Frees memory allocated by M_GetBinaryCell(). Equivalent to calling free(cell).
 *
 * \param[in] cell cell data returned by M_GetBinaryCell()
 */
LM_EXPORT void LM_SPEC M_FreeBinaryCell(char *cell);

/*! Returns the number of columns in a comma-delimited response.
 * M_ParseCommaDelimited() must be called beforehand.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return number of columns in comma-delimited data
 */
LM_EXPORT int LM_SPEC M_NumColumns(M_CONN *conn, M_uintptr identifier);

/*! Returns the number of rows in a comma-delimited response.
 * M_ParseCommaDelimited() must be called beforehand.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \return number of rows in comma-delimited data
 */
LM_EXPORT long LM_SPEC M_NumRows(M_CONN *conn, M_uintptr identifier);

/*! Returns the text name for a column header in comma-delimited data.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] identifier reference for transaction as returned by M_TransNew()
 * \param[in] column_num column number
 * \return text name for column header
 */
LM_EXPORT const char *LM_SPEC M_GetHeader(M_CONN *conn, M_uintptr identifier, int column_num);

/* ---------------------------------------- */

/*! Sleep for a given number of microseconds. Uses select(). Threadsafe.
 *
 * \param[in] length number of microseconds to sleep
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_uwait(unsigned long length);

/* ---------------------------------------- */


/*! Returns hash of client SSL certificate.
 *
 * \param[in] filename path of client SSL certificate file
 * \return hash, or NULL on error. Must be freed using free().
 */
LM_EXPORT char *LM_SPEC M_SSLCert_gen_hash(const char *filename);

/* ---------------------------------------- */

#ifdef _WIN32
/* Specifically make sure callbacks are __cdecl, not stdcall */
typedef void * (__cdecl * M_Register_Mutex)(void);
typedef int (__cdecl * M_Mutex_Lock)(void *);
typedef int (__cdecl * M_Mutex_Unlock)(void *);
typedef int (__cdecl * M_Unregister_Mutex)(void *);
typedef unsigned long (__cdecl * M_ThreadID)(void);
#else
typedef void * (LM_SPEC * M_Register_Mutex)(void);
typedef int (LM_SPEC * M_Mutex_Lock)(void *);
typedef int (LM_SPEC * M_Mutex_Unlock)(void *);
typedef int (LM_SPEC * M_Unregister_Mutex)(void *);
typedef unsigned long (LM_SPEC * M_ThreadID)(void);
#endif /* ifdef _WIN32 */

/*! Register a callback to destroy a mutex.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] reg pointer to function to call when doing a mutex destroy
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Register_mutexdestroy(M_CONN *conn, M_Unregister_Mutex reg);

/*! Register a callback to initialize a mutex.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] reg pointer to function to call when doing a mutex initialization
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Register_mutexinit(M_CONN *conn, M_Register_Mutex reg);

/*! Register a callback to lock a mutex.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] reg pointer to function to call when doing a mutex lock
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Register_mutexlock(M_CONN *conn, M_Mutex_Lock reg);

/*! Register a callback to unlock a mutex.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] reg pointer to function to call when doing a mutex unlock
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Register_mutexunlock(M_CONN *conn, M_Mutex_Unlock reg);

/*! Register a callback to look up a thread ID.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in] reg pointer to function to call when doing a thread ID lookup
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_Register_threadid(M_CONN *conn, M_ThreadID reg);

/*! After registering mutex callbacks, you must enable their use by calling
 * this function. If you call this function, it must be called before M_Connect().
 * Enabling thread safety is important if you have more than one thread that
 * wishes to act upon a single connection at the same time. Most programs do not
 * have this need, but if you do, make sure you register the callbacks and then
 * call this function.
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \return 1 on success, 0 on failure
 */
LM_EXPORT int LM_SPEC M_EnableThreadSafety(M_CONN *conn);

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBMONETRA_LEGACY_H__ */
