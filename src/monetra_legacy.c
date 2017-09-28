#include "monetra_api.h"
#include "monetra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _M_CONN {
	LM_conn_t         *conn;
	M_event_t         *event;
	M_tls_clientctx_t *tlsctx;
	M_bool             trust_loaded;
	M_bool             blocking;
	M_uint64           txntimeout;
};

static char *M_global_capath = NULL;

int LM_SPEC M_InitEngine_ex(enum m_ssllock_style lockstyle)
{
	return LM_init((lockstyle == M_SSLLOCK_EXTERNAL)?LM_INIT_SSLLOCK_EXTERNAL:LM_INIT_NORMAL);
}

int LM_SPEC M_InitEngine(const char *location)
{
	if (M_InitEngine_ex(M_SSLLOCK_INTERNAL) && !M_str_isempty(location)) {
		M_global_capath = M_strdup(location);
		return 1;
	}
	return 0;
}

void LM_SPEC M_DestroyEngine(void)
{
	M_library_cleanup();
	M_free(M_global_capath);
	M_global_capath = NULL;
}

static void M_legacy_event_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans)
{
	(void)trans;
	(void)conn;
	switch (type) {
		case LM_EVENT_CONN_DISCONNECT:
		case LM_EVENT_CONN_ERROR:
		case LM_EVENT_CONN_CONNECTED:
		case LM_EVENT_TRANS_DONE:
		case LM_EVENT_TRANS_ERROR:
		case LM_EVENT_TRANS_NOCONNECT:
			/* We just need to be able to break out of the event loop.  Everything else
			 * just polls for status */
			M_event_return(event);
			break;
	}
}

void LM_SPEC M_InitConn(M_CONN *conn)
{
	if (conn == NULL)
		return;

	*conn           = M_malloc_zero(sizeof(**conn));
	(*conn)->event  = M_event_create(M_EVENT_FLAG_NONE);
	(*conn)->conn   = LM_conn_init((*conn)->event, M_legacy_event_callback, "localhost", 8665);
	(*conn)->tlsctx = M_tls_clientctx_create();

	/* Old library by default did not verify certs */
	M_tls_clientctx_set_verify_level((*conn)->tlsctx, M_TLS_VERIFY_NONE);
	if (M_global_capath) {
		M_tls_clientctx_set_trust_ca_file((*conn)->tlsctx, M_global_capath);
	}
	LM_conn_set_tls_clientctx((*conn)->conn, (*conn)->tlsctx);
}

int LM_SPEC M_SetProxyHost(M_CONN *conn, const char *host, unsigned short port, const char *type)
{
	/* XXX: Implement me when mstdlib supports it */
	(void)conn;
	(void)host;
	(void)port;
	(void)type;
	return 0;
}

int LM_SPEC M_SetProxyTimeout(M_CONN *conn, int timeout)
{
	/* XXX: Implement me when mstdlib supports it */
	(void)conn;
	(void)timeout;
	return 0;
}

int LM_SPEC M_SetProxyUser(M_CONN *conn, const char *username, const char *password)
{
	/* XXX: Implement me when mstdlib supports it */
	(void)conn;
	(void)username;
	(void)password;
	return 0;
}

int LM_SPEC M_SetProxyLocalNets(M_CONN *conn, const char *localnets, char *error, int errlen)
{
	/* XXX: Implement me when mstdlib supports it */
	(void)conn;
	(void)localnets;
	(void)error;
	(void)errlen;
	return 0;
}

int LM_SPEC M_SetBlocking(M_CONN *conn, int tf)
{
	if (conn == NULL || *conn == NULL)
		return 0;
	(*conn)->blocking = tf?M_TRUE:M_FALSE;
	return 1;
}

int LM_SPEC M_SetLogging(M_CONN *conn, int level)
{
	/* XXX: Implement me when mstdlib supports it */
	(void)conn;
	(void)level;
	return 0;
}

int LM_SPEC M_SetTimeout(M_CONN *conn, long timeout)
{
	if (conn == NULL || *conn == NULL || timeout < 0)
		return 0;
	(*conn)->txntimeout = (M_uint64)timeout;
	return 1;
}

int LM_SPEC M_SetDropFile(M_CONN *conn, const char *df_location)
{
	(void)conn;
	(void)df_location;
	/* Not supported anymore, too insecure */
	return 0;
}

int LM_SPEC M_SetIP(M_CONN *conn, const char *host, unsigned short port)
{
	if (conn == NULL || *conn == NULL || M_str_isempty(host) || port == 0)
		return 0;

	if (!LM_conn_change_server((*conn)->conn, host, port) ||
	    !LM_conn_change_mode((*conn)->conn, LM_MODE_IP))
		return 0;

	return 1;
}

int LM_SPEC M_SetSSL(M_CONN *conn, const char *host, unsigned short port)
{
	if (conn == NULL || *conn == NULL || M_str_isempty(host) || port == 0)
		return 0;

	if (!LM_conn_change_server((*conn)->conn, host, port) ||
	    !LM_conn_change_mode((*conn)->conn, LM_MODE_TLS))
		return 0;

	return 1;
}

int LM_SPEC M_SetSSL_CAfile(M_CONN *conn, const char *path)
{
	if (conn == NULL || *conn == NULL || M_str_isempty(path))
		return 0;
	if (M_tls_clientctx_set_trust_ca_file((*conn)->tlsctx, path)) {
		(*conn)->trust_loaded = M_TRUE;
		return 1;
	}
	return 0;
}

int LM_SPEC M_SetSSL_CAdir(M_CONN *conn, const char *path)
{
	if (conn == NULL || *conn == NULL || M_str_isempty(path))
		return 0;
	if (M_tls_clientctx_set_trust_ca_dir((*conn)->tlsctx, path)) {
		(*conn)->trust_loaded = M_TRUE;
		return 1;
	}
	return 0;
}

int LM_SPEC M_SetSSL_Files(M_CONN *conn, const char *sslkeyfile, const char *sslcertfile)
{
	if (conn == NULL || *conn == NULL || M_str_isempty(sslkeyfile) || M_str_isempty(sslcertfile))
		return 0;
	return LM_conn_set_tls_cert_files((*conn)->conn, sslkeyfile, sslcertfile);
}

void LM_SPEC M_VerifyConnection(M_CONN *conn, int tf)
{
	if (tf)
		return;

	/* Can only be disabled */
	LM_conn_disable_ping((*conn)->conn);
}

void LM_SPEC M_VerifySSLCert(M_CONN *conn, int level)
{
	M_tls_verify_level_t mlevel = M_TLS_VERIFY_FULL;

	if (conn == NULL || *conn == NULL || level < 0 || level > 3)
		return;

	switch (level) {
		case 0:
			mlevel = M_TLS_VERIFY_NONE;
			break;
		case 1:
			mlevel = M_TLS_VERIFY_FULL;
			break;
		case 2:
			mlevel = M_TLS_VERIFY_CERT_FUZZY;
			break;
		case 3:
			mlevel = M_TLS_VERIFY_CERT_ONLY;
			break;
	}

	M_tls_clientctx_set_verify_level((*conn)->tlsctx, mlevel);
}

int LM_SPEC M_ValidateIdentifier(M_CONN *conn, int tf)
{
	(void)tf;

	if (conn == NULL || *conn == NULL)
		return 0;

	/* Can't be disabled anymore */
	return 1;
}

int LM_SPEC M_Connect(M_CONN *conn)
{
	if (conn == NULL || *conn == NULL)
		return 0;

	/* Load default trust if one not loaded */
	if (!(*conn)->trust_loaded) {
		(*conn)->trust_loaded = M_tls_clientctx_set_default_trust((*conn)->tlsctx);
	}

	/* Call connect */
	if (!LM_conn_connect((*conn)->conn))
		return 0;

	/* Block in event loop until either the connection finishes or fails */
	M_event_loop((*conn)->event, M_TIMEOUT_INF);

	/* See if we're connected or not */
	if (LM_conn_status((*conn)->conn) != LM_CONN_STATUS_CONNECTED)
		return 0;

	return 1;
}

void LM_SPEC M_MaxConnTimeout(M_CONN *conn, int maxtime)
{
	if (conn == NULL || *conn == NULL)
		return;
	LM_conn_set_conn_timeout((*conn)->conn, (size_t)maxtime);
}

const char * LM_SPEC M_ConnectionError(M_CONN *conn)
{
	if (conn == NULL || *conn == NULL)
		return NULL;
	return LM_conn_error((*conn)->conn);
}

int LM_SPEC M_Disconnect(M_CONN *conn)
{
	if (conn == NULL || *conn == NULL)
		return 0;

	/* If connected, initiate a disconnect and wait for that to finish */
	if (LM_conn_status((*conn)->conn) == LM_CONN_STATUS_CONNECTED) {
		LM_conn_disconnect((*conn)->conn);

		while (LM_conn_status((*conn)->conn) == LM_CONN_STATUS_DISCONNECTING) {
			/* Block in event loop until either the connection finishes or fails */
			M_event_loop((*conn)->event, M_TIMEOUT_INF);
		}
	}
	return 1;
}

void LM_SPEC M_DestroyConn(M_CONN *conn)
{
	if (conn == NULL || *conn == NULL)
		return;

	M_Disconnect(conn);

	/* Cleanup */
	LM_conn_destroy((*conn)->conn);
	M_event_destroy((*conn)->event);
	M_tls_clientctx_destroy((*conn)->tlsctx);
	M_free(*conn);
	*conn = NULL;
}

int LM_SPEC M_Monitor(M_CONN *conn)
{
	return M_Monitor_ex(conn, 0);
}

int LM_SPEC M_Monitor_ex(M_CONN *conn, long wait_us)
{
	if (conn == NULL || *conn == NULL)
		return 0;

	/* NOTE: With the legacy API, people would use M_Monitor() to determine if
	 *       the connection was still up even before enqueuing a connection.
	 *       Recent versions of Monetra have idle timeouts, and this API now
	 *       supports auto-reconnects ... so don't return an error if there are
	 *       no pending transactions as we don't want someone to actually detect
	 *       this as an error condition since it could be mishandled. */
	if (LM_conn_trans_count((*conn)->conn, LM_TRANS_STATUS_PENDING) == 0 &&
		LM_conn_trans_count((*conn)->conn, LM_TRANS_STATUS_READY) == 0) {
		return 1;
	}

	if (LM_conn_status((*conn)->conn) != LM_CONN_STATUS_CONNECTED &&
		LM_conn_status((*conn)->conn) != LM_CONN_STATUS_CONNECTING) {
		return 0;
	}

	/* We need to handle the case where the event loop is already running.  This
	 * would happen with this legacy API if in blocking mode and multiple threads
	 * are using the same connection object simultaneously (which is odd to do
	 * and was never "officially" condoned, but know people did it).  There's
	 * really not an easy way to gracefully handle this.
	 *
	 * We could work up some thread conditional based solution with timedwaits
	 * and whatnot for 100% synchronization and responsiveness, but at the end,
	 * its not worth it for the legacy API which was always poll-based anyhow
	 * so worst case is not worse than the original implementation ever was */
	if (M_event_get_status((*conn)->event) == M_EVENT_STATUS_RUNNING) {
		/* Wait some pre-determined amount of time so we don't spin CPU cycles */
		if (wait_us < 0 || wait_us > 20000)
			wait_us = 20000;
		if (wait_us != 0)
			M_thread_sleep((M_uint64)wait_us);
	} else {
		/* Run event loop for specified time if connected.  The event handler we
		 * registered will simply signal if there is an event. */
		M_event_loop((*conn)->event, (wait_us == -1)?M_TIMEOUT_INF:(M_uint64)(wait_us / 1000));
	}

	/* If we're not connected or still in the process of connecting at the end of the event loop, return error */
	if (LM_conn_status((*conn)->conn) != LM_CONN_STATUS_CONNECTED &&
		LM_conn_status((*conn)->conn) != LM_CONN_STATUS_CONNECTING) {
		return 0;
	}

	return 1;
}

int LM_SPEC M_TransactionsSent(M_CONN *conn)
{
	if (conn == NULL || *conn == NULL)
		return 0;

	return (int)LM_conn_trans_count((*conn)->conn, LM_TRANS_STATUS_PENDING) +
	       (int)LM_conn_trans_count((*conn)->conn, LM_TRANS_STATUS_DONE);
}


void LM_SPEC M_DeleteTrans(M_CONN *conn, M_uintptr identifier)
{
	LM_trans_t *trans;

	if (conn == NULL || *conn == NULL)
		return;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return;
	LM_trans_delete(trans);
}

M_uintptr LM_SPEC M_TransNew(M_CONN *conn)
{
	LM_trans_t *trans;

	if (conn == NULL || *conn == NULL)
		return 0;

	trans = LM_trans_new((*conn)->conn);
	if (trans == NULL)
		return 0;

	return (M_uintptr)LM_trans_internal_id(trans);
}

int LM_SPEC M_TransKeyVal(M_CONN *conn, M_uintptr identifier, const char *key, const char *value)
{
	LM_trans_t *trans;

	if (conn == NULL || *conn == NULL)
		return 0;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return 0;

	return LM_trans_set_param(trans, key, value);
}

int LM_SPEC M_TransBinaryKeyVal(M_CONN *conn, M_uintptr identifier, const char *key, const char *value, int value_len)
{
	LM_trans_t *trans;

	if (conn == NULL || *conn == NULL || M_str_isempty(key) || value_len < 0)
		return 0;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return 0;

	return LM_trans_set_param_binary(trans, key, (const unsigned char *)value, (size_t)value_len);
}

int LM_SPEC M_TransSend(M_CONN *conn, M_uintptr identifier)
{
	LM_trans_t *trans;

	if (conn == NULL || *conn == NULL)
		return 0;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return 0;

	/* Before enqueing a transaction, lets make sure we poll the connection so
	 * we know if it has been terminated that way it can initiate a fresh
	 * connection.
	 * If not connected (either because we lost the connection or it was never
	 * established in the first place), we should force a connect and block until
	 * it is connected rather than relying on implicit connection as the legacy
	 * API can't really report all errors the new API can ... and the old API
	 * never previously supported implicit connecting or reconnecting */

	M_event_loop((*conn)->event, 0);
	if (LM_conn_status((*conn)->conn) != LM_CONN_STATUS_CONNECTED) {
		if (!M_Connect(conn))
			return 0;
	}

	/* Send the transaction.  If not in blocking mode, this will also attempt
	 * to implicitly connect */
	if (!LM_trans_send(trans))
		return 0;

	/* If blocking mode was requested, wait for a response (or disconnect) */
	if ((*conn)->blocking) {
		/* Loop in a blocking M_Monitor_ex(), each time a signal comes in it will unblock
		 * because there might be something to do */
		while (LM_trans_status(trans) != LM_TRANS_STATUS_DONE) {
			if (!M_Monitor_ex(conn, -1))
				return 0;
		}
	}
	return 1;
}

const char * LM_SPEC M_ResponseParam(M_CONN *conn, M_uintptr identifier, const char *key)
{
	LM_trans_t *trans;

	if (conn == NULL || *conn == NULL || M_str_isempty(key))
		return NULL;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return NULL;

	return LM_trans_response_param(trans, key);
}

unsigned char * LM_SPEC M_ResponseBinaryParam(M_CONN *conn, M_uintptr identifier, const char *key, int *out_length)
{
	LM_trans_t    *trans;
	unsigned char *out;
	unsigned char *temp;
	size_t         temp_len = 0;

	if (conn == NULL || *conn == NULL || M_str_isempty(key) || out_length == NULL)
		return NULL;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return NULL;

	*out_length = 0;

	/* Store to temporary variable since we have to map it to non-mstdlib malloc */
	temp = LM_trans_response_param_binary(trans, key, &temp_len);
	if (temp == NULL)
		return NULL;

	/* Copy using non-mstdlib malloc */
	*out_length = (int)temp_len;

M_BEGIN_IGNORE_DEPRECATIONS
	out         = malloc(temp_len);
M_END_IGNORE_DEPRECATIONS

	M_mem_copy(out, temp, temp_len);
	M_free(temp);
	return out;
}

char ** LM_SPEC M_ResponseKeys(M_CONN *conn, M_uintptr identifier, int *num_keys)
{
	LM_trans_t   *trans;
	char        **out   = NULL;
	M_list_str_t *keys;
	size_t        i;

	if (conn == NULL || *conn == NULL || num_keys == NULL)
		return NULL;

	*num_keys = 0;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return NULL;

	keys = LM_trans_response_keys(trans);
	if (keys == NULL)
		return NULL;

	/* Return as character array using standard system malloc */
	if (M_list_str_len(keys)) {
M_BEGIN_IGNORE_DEPRECATIONS
		out = malloc(sizeof(*out) * M_list_str_len(keys));
		for (i=0; i<M_list_str_len(keys); i++) {
			const char *key = M_list_str_at(keys, i);
			out[i] = strdup(key);
		}
M_END_IGNORE_DEPRECATIONS
		*num_keys = (int)M_list_str_len(keys);
	}
	M_list_str_destroy(keys);

	return out;
}

const char * LM_SPEC M_ResponseKeys_index(char **keys, int num_keys, int idx)
{
	if (keys == NULL || num_keys <= 0 || idx < 0 || idx > num_keys)
		return NULL;
	return keys[idx];
}

int LM_SPEC M_FreeResponseKeys(char **keys, int num_keys)
{
	int i;
	if (keys == NULL || num_keys == 0)
		return 0;

M_BEGIN_IGNORE_DEPRECATIONS
	for (i=0; i<num_keys; i++)
		free(keys[i]);
	free(keys);
M_END_IGNORE_DEPRECATIONS

	return 1;
}


long LM_SPEC M_TransInQueue(M_CONN *conn)
{
	if (conn == NULL || *conn == NULL)
		return 0;

	return (int)LM_conn_trans_count((*conn)->conn, LM_TRANS_STATUS_ALL);
}


int LM_SPEC M_CheckStatus(M_CONN *conn, M_uintptr identifier)
{
	LM_trans_t       *trans;

	if (conn == NULL || *conn == NULL)
		return M_UNUSED;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return M_UNUSED;

	switch (LM_trans_status(trans)) {
		case LM_TRANS_STATUS_NEW:
			return M_NEW;
		case LM_TRANS_STATUS_READY:
		case LM_TRANS_STATUS_PENDING:
			return M_PENDING;
		case LM_TRANS_STATUS_DONE:
			return M_DONE;
		default:
			break;
	}
	return M_UNUSED;
}


int LM_SPEC M_ReturnStatus(M_CONN *conn, M_uintptr identifier)
{
	const char *code;

	if (M_CheckStatus(conn, identifier) != M_DONE)
		return M_ERROR;
	if (M_IsCommaDelimited(conn, identifier))
		return M_SUCCESS;
	code = M_ResponseParam(conn, identifier, "code");
	if (M_str_caseeq(code, "SUCCESS") || M_str_caseeq(code, "AUTH"))
		return M_SUCCESS;
	return M_FAIL;
}


long LM_SPEC M_CompleteAuthorizations(M_CONN *conn, M_uintptr **listings)
{
	M_list_t   *list;
	size_t      i;
	size_t      cnt;

	if (conn == NULL || *conn == NULL)
		return 0;

	if (listings != NULL)
		*listings = NULL;

	/* If they didn't pass a pointer, just return a count */
	if (listings == NULL) {
		return (long)LM_conn_trans_count((*conn)->conn, LM_TRANS_STATUS_DONE);
	}

	list = LM_conn_trans_list((*conn)->conn, LM_TRANS_STATUS_DONE);
	if (list == NULL)
		return 0;

	cnt       = M_list_len(list);
M_BEGIN_IGNORE_DEPRECATIONS
	*listings = malloc(sizeof(**listings) * cnt);
M_END_IGNORE_DEPRECATIONS
	for (i=0; i<cnt; i++) {
		(*listings)[i]=LM_trans_internal_id(M_list_at(list, i));
	}
	M_list_destroy(list, M_TRUE);

	return (long)cnt;
}


M_uintptr LM_SPEC M_CompleteAuthorizations_index(M_uintptr *listings, int num_listings, int idx)
{
	if (listings == NULL || num_listings <= 0 || idx <= 0 || idx >= num_listings)
		return 0;
	return listings[idx];
}

int LM_SPEC M_FreeCompleteAuthorizations(M_uintptr *listings, int num_listings)
{
	(void)num_listings;
M_BEGIN_IGNORE_DEPRECATIONS
	if (listings != NULL)
		free(listings);
M_END_IGNORE_DEPRECATIONS

	return 1;
}

int LM_SPEC M_IsCommaDelimited(M_CONN *conn, M_uintptr identifier)
{
	LM_trans_t       *trans;

	if (conn == NULL || *conn == NULL)
		return 0;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return 0;

	if (LM_trans_response_type(trans) == LM_TRANS_RESPONSE_KV)
		return 0;

	/* Legacy API treats CSV and BULK as the same */

	return 1;
}

int LM_SPEC M_ParseCommaDelimited(M_CONN *conn, M_uintptr identifier)
{
	/* No need to explicitly call parser on new API */
	if (M_IsCommaDelimited(conn, identifier))
		return 1;
	return 0;
}

const char * LM_SPEC M_GetCommaDelimited(M_CONN *conn, M_uintptr identifier)
{
	LM_trans_t       *trans;

	if (conn == NULL || *conn == NULL)
		return NULL;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return NULL;

	return LM_trans_response_raw(trans);
}

static const M_csv_t * LM_SPEC M_GetCSVHandle(M_CONN *conn, M_uintptr identifier)
{
	LM_trans_t       *trans;

	if (conn == NULL || *conn == NULL)
		return NULL;

	trans = LM_conn_get_trans_by_internal_id((*conn)->conn, identifier);
	if (trans == NULL)
		return NULL;

	return LM_trans_response_csv(trans);
}

const char * LM_SPEC M_GetCell(M_CONN *conn, M_uintptr identifier, const char *column, long row)
{
	const M_csv_t *csv;

	if (M_str_isempty(column) || row < 0)
		return NULL;

	csv = M_GetCSVHandle(conn, identifier);
	if (csv == NULL)
		return NULL;

	return M_csv_get_cell(csv, (size_t)row, column);
}

const char * LM_SPEC M_GetCellByNum(M_CONN *conn, M_uintptr identifier, int column, long row)
{
	const M_csv_t *csv;

	if (column < 0 || row < 0)
		return NULL;

	csv = M_GetCSVHandle(conn, identifier);
	if (csv == NULL)
		return NULL;

	return M_csv_get_cellbynum(csv, (size_t)row, (size_t)column);
}

char * LM_SPEC M_GetBinaryCell(M_CONN *conn, M_uintptr identifier, const char *column, long row, int *out_length)
{
	const char    *cell;
	unsigned char *data;
	size_t         data_len;
	char          *out;

	if (column == NULL || row < 0 || out_length == NULL)
		return NULL;

	(*out_length) = 0;

	cell = M_GetCell(conn, identifier, column, row);
	if (cell == NULL)
		return NULL;

	data = M_bincodec_decode_alloc(cell, M_str_len(cell), &data_len, M_BINCODEC_BASE64);
	if (data == NULL)
		return NULL;

	/* Use system's malloc */
	*out_length   = (int)data_len;
M_BEGIN_IGNORE_DEPRECATIONS
	out           = malloc(data_len+1);
M_END_IGNORE_DEPRECATIONS
	M_mem_copy(out, data, data_len);
	out[data_len] = 0;
	M_free(data);

	return out;
}

void LM_SPEC M_FreeBinaryCell(char *cell)
{
M_BEGIN_IGNORE_DEPRECATIONS
	if (cell != NULL)
		free(cell);
M_END_IGNORE_DEPRECATIONS

}

int LM_SPEC M_NumColumns(M_CONN *conn, M_uintptr identifier)
{
	const M_csv_t *csv;

	csv = M_GetCSVHandle(conn, identifier);
	if (csv == NULL)
		return 0;

	return (int)M_csv_get_numcols(csv);
}

long LM_SPEC M_NumRows(M_CONN *conn, M_uintptr identifier)
{
	const M_csv_t *csv;

	csv = M_GetCSVHandle(conn, identifier);
	if (csv == NULL)
		return 0;

	return (long)M_csv_get_numrows(csv);
}

const char * LM_SPEC M_GetHeader(M_CONN *conn, M_uintptr identifier, int column_num)
{
	const M_csv_t *csv;

	if (column_num < 0)
		return NULL;

	csv = M_GetCSVHandle(conn, identifier);
	if (csv == NULL)
		return NULL;

	return M_csv_get_header(csv, (size_t)column_num);
}

int LM_SPEC M_uwait(unsigned long length)
{
	M_thread_sleep(length);
	return 1;
}

char * LM_SPEC M_SSLCert_gen_hash(const char *filename)
{
	/* XXX: Implement me when mstdlib supports this */
	(void)filename;
	return NULL;
}

int LM_SPEC M_Register_mutexdestroy(M_CONN *conn, M_Unregister_Mutex reg)
{
	(void)conn;
	(void)reg;
	/* No longer needed */
	return 1;
}

int LM_SPEC M_Register_mutexinit(M_CONN *conn, M_Register_Mutex reg)
{
	(void)conn;
	(void)reg;
	/* No longer needed */
	return 1;
}

int LM_SPEC M_Register_mutexlock(M_CONN *conn, M_Mutex_Lock reg)
{
	(void)conn;
	(void)reg;
	/* No longer needed */
	return 1;
}

int LM_SPEC M_Register_mutexunlock(M_CONN *conn, M_Mutex_Unlock reg)
{
	(void)conn;
	(void)reg;
	/* No longer needed */
	return 1;
}

int LM_SPEC M_Register_threadid(M_CONN *conn, M_ThreadID reg)
{
	(void)conn;
	(void)reg;
	/* No longer needed */
	return 1;
}

int LM_SPEC M_EnableThreadSafety(M_CONN *conn)
{
	(void)conn;
	/* Always threadsafe now */
	return 1;
}

