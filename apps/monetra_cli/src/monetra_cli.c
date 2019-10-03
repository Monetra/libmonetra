#include "monetra_cli.h"

static void io_trace(void *cb_arg, M_io_trace_type_t type, M_event_type_t event_type, const unsigned char *data, size_t data_len)
{
	const char *io_type = "UNKNOWN";
	char       *dump    = NULL;

	(void)cb_arg;
	(void)event_type;

	switch (type) {
		case M_IO_TRACE_TYPE_READ:
			io_type = "READ";
			break;
		case M_IO_TRACE_TYPE_WRITE:
			io_type = "WRITE";
			break;
		case M_IO_TRACE_TYPE_EVENT:
			return;
	}

	dump = M_str_hexdump(M_STR_HEXDUMP_NONE, 0, "\t", data, data_len);
	if (M_str_isempty(dump)) {
		M_free(dump);
		dump = M_strdup("\t<No Data>\n");
	}

	M_printf("%s\n%s\n", io_type, dump);
	M_free(dump);
}

static void conn_iocreate_cb(LM_conn_t *conn, M_io_t *io, void *arg)
{
	(void)arg;
	(void)conn;
	M_io_add_trace(io, NULL, io_trace, arg, NULL, NULL);
}

/* Print KVS with the = sign aligned. */
static void print_kvs(const M_hash_dict_t *kvs)
{
	M_hash_dict_enum_t *he;
	const char         *key;
	const char         *val;
	size_t              longest = 0;
	size_t              len;

	/* First iteration gets the longest key so we can
 	 * print the values justified. */
	M_hash_dict_enumerate(kvs, &he);
	while (M_hash_dict_enumerate_next(kvs, he, &key, &val)) {
		len = M_str_len(key);
		if (len > longest) {
			longest = len;
		}
	}
	M_hash_dict_enumerate_free(he);

	/* Always print the identifier first so it's easy to match requests to responses. */
	key = "identifier";
	if (M_hash_dict_get(kvs, key, &val)) {
		len = longest - M_str_len(key)+1;
		M_printf("%s%*s= %s\n", key, (int)len, " ", val);
	}

	/* The dict should have been created ordered so we don't need
 	 * to do anything with the keys here other than print them. */
	M_hash_dict_enumerate(kvs, &he);
	while (M_hash_dict_enumerate_next(kvs, he, &key, &val)) {
		if (M_str_caseeq(key, "identifier")) {
			continue;
		}
		len = longest - M_str_len(key)+1;
		M_printf("%s%*s= %s\n", key, (int)len, " ", val);
	}
	M_hash_dict_enumerate_free(he);

	M_printf("\n");
}

/* Print the KVS for a list of KVS. */
static void list_trans(cli_trans_t *trans)
{
	const M_hash_dict_t *kvs;
	size_t               len;
	size_t               i;

	len = M_list_len(trans->kvs);
	for (i=0; i<len; i++) {
		kvs = M_list_at(trans->kvs, i);
		print_kvs(kvs);
	}
	M_printf("\n");
}

/* Take KVS that represent a monetra transaction and put it
 * into libmonetra to be sent out. */
static M_bool queue_lm_trans(cli_trans_t *trans, LM_conn_t *conn)
{
	LM_trans_t         *lmtrans;
	M_hash_dict_t      *kvs;
	M_hash_dict_enum_t *he;
	const char         *key;
	const char         *val;

	if (trans == NULL)
		return M_FALSE;

	kvs = M_list_take_first(trans->kvs);
	if (kvs == NULL)
		return M_FALSE;

	lmtrans = LM_trans_new(conn);

	M_hash_dict_enumerate(kvs, &he);
	while (M_hash_dict_enumerate_next(kvs, he, &key, &val)) {
		LM_trans_set_param(lmtrans, key, val);
	}
	M_hash_dict_enumerate_free(he);

	/* We associate the identifier to the transaction so we can put it
 	 * in the response KVS. */
	LM_trans_set_userdata(lmtrans, M_strdup(M_hash_dict_get_direct(kvs, "identifier")));
	LM_trans_send(lmtrans);

	M_hash_dict_destroy(kvs);
	trans->outstanding_cnt++;
	return M_TRUE;
}

static void trans_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans)
{
	cli_trans_t   *cli_trans  = NULL;
	M_hash_dict_t *kvs;
	char          *identifier = NULL;
	M_bool         ret;

	if (conn != NULL)
		cli_trans = LM_conn_get_userdata(conn);
	if (trans != NULL)
		identifier = LM_trans_get_userdata(trans);

	switch (type) {
		case LM_EVENT_CONN_CONNECTED:
			do {
				ret = queue_lm_trans(cli_trans, conn);
			} while (ret && !cli_trans->send_serial);
			break;
		case LM_EVENT_CONN_DISCONNECT:
			LM_conn_destroy(conn);
			M_event_done(event);
			break;
		case LM_EVENT_CONN_ERROR:
			M_printf("Error received: %s\n", LM_conn_error(conn));
			LM_conn_destroy(conn);
			M_event_done(event);
			break;
		case LM_EVENT_TRANS_DONE:
			kvs = M_hash_dict_create(8, 75, M_HASH_DICT_CASECMP|M_HASH_DICT_KEYS_ORDERED|M_HASH_DICT_KEYS_SORTASC);
			if (LM_trans_response_type(trans) == LM_TRANS_RESPONSE_CSV || LM_trans_response_type(trans) == LM_TRANS_RESPONSE_BULK) {
				M_hash_dict_insert(kvs, "datablock", LM_trans_response_raw(trans));
			} else {
				/* Merge into our KVS instead of just copying the response KVS because
 				 * we want the keys to be ordered for printing. */
				M_hash_dict_merge(&kvs, M_hash_dict_duplicate(LM_trans_response_dict(trans)));
			}
			/* Add the identifier uses with the request so the user can tell which request
 			 * this response is for. */
			M_hash_dict_insert(kvs, "identifier", identifier);

			print_kvs(kvs);
			M_hash_dict_destroy(kvs);

			LM_trans_delete(trans);
			if (cli_trans != NULL) {
				cli_trans->outstanding_cnt--;

				queue_lm_trans(cli_trans, conn);
				if (cli_trans->outstanding_cnt == 0) {
					/* This is the last transaction. */
					LM_conn_disconnect(conn);
				}
			}
			break;
		case LM_EVENT_TRANS_ERROR:
		case LM_EVENT_TRANS_NOCONNECT:
			M_printf("Transaction %s error (connectivity): %s\n", identifier, LM_conn_error(conn));
			LM_trans_delete(trans);
			break;
		case LM_EVENT_TRANS_TIMEOUT:
			/* ignore */
			break;
	}

	M_free(identifier);
}

static M_bool setup_ssl_ctx(cli_trans_t *trans, M_tls_clientctx_t *ctx, char *error, size_t errlen)
{
	if (!M_str_isempty(trans->keyfile)) {
		if (!M_tls_clientctx_set_cert_files(ctx, trans->keyfile, trans->certfile, NULL)) {
			M_snprintf(error, errlen, "Could not set restrictions key and certificate\n");
			return M_FALSE;
		}
	}

	M_tls_clientctx_set_default_trust(ctx);
	if (!M_str_isempty(trans->cadir)) {
		if (!M_tls_clientctx_set_trust_ca_dir(ctx, trans->cadir)) {
			M_snprintf(error, errlen, "Could not add trust CA directory\n");
			return M_FALSE;
		}
	}

	if (!M_tls_clientctx_set_verify_level(ctx, trans->certvalidation)) {
		M_snprintf(error, errlen, "Could not set certificate verification level\n");
		return M_FALSE;
	}

	return M_TRUE;
}

int main(int argc, char **argv)
{
	M_tls_clientctx_t *ctx    = NULL;
	M_event_t         *el     = NULL;
	LM_conn_t         *conn   = NULL;
	cli_trans_t       *trans;
	char               error[256];
	int                ret = 0;

	trans = cli_parse_args(argc, (const char *const *)argv);
	if (trans == NULL)
		return 1;

	ctx = M_tls_clientctx_create();
	if (!setup_ssl_ctx(trans, ctx, error, sizeof(error))) {
		M_printf("%s\n", error);
		ret = 2;
		goto done;
	}

	el   = M_event_create(M_EVENT_FLAG_NONE);

	conn = LM_conn_init(el, trans_callback, trans->host, trans->port);
	LM_conn_disable_ping(conn);
	LM_conn_set_tls_clientctx(conn, ctx);
	/* User data is the cli trans so transactions can be pulled out of it
 	 * in the event loop. */
	LM_conn_set_userdata(conn, trans);

	if (trans->enable_trace)
		LM_conn_set_iocreate_callback(conn, conn_iocreate_cb, NULL);

	M_printf("-- Request --\n");
	list_trans(trans);

	/* Responses will be printed as they come in via the event loop. */
	M_printf("-- Response --\n");

	LM_conn_connect(conn);
	/* Event loop will be exited either by connection error or once all
 	 * transactions have been processed. */
	M_event_loop(el, M_TIMEOUT_INF);

done:
	M_event_destroy(el);
	/* NOTE: we cleaned up 'conn' within the trans_callback.
 	 * We can't have exited the event loop otherwise. */
	conn = NULL;
	M_tls_clientctx_destroy(ctx);
	cli_trans_destroy(trans);
	M_library_cleanup();

	return ret;
}
