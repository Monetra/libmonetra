#include <mstdlib/mstdlib.h>
#include <mstdlib/mstdlib_io.h>
#include <monetra_api.h>

#include <assert.h>
#include <stdio.h> /* Needed for fflush() and getchar(). */

/* Handling for VT100 support. */
#define M_HAS_VT100
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
   /*  Virtual terminal processing is only available on recent versions of Windows 10.
    *  If the symbol isn't provided, it's not supported, so undefine M_HAS_VT100.
    */
#  ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#    undef M_HAS_VT100
#  endif
#endif

#define VERSION "1.0.0"


/* Configuration Settings */
static char        config_hostname[128]      = "localhost";
static M_uint16    config_port               = 8665;
static char        config_username[128]      = "loopback";
static char        config_password[128]      = "test123";
static size_t      config_num_conns          = 100;
static size_t      config_trans_per_conn     = 2;
static size_t      config_delay_per_trans_ms = 1000;

/* State Tracking */
static M_timeval_t        track_starttime;
static M_timeval_t        track_lasttime;
static size_t             track_num_connections   = 0;
static size_t             track_num_transactions  = 0;
static size_t             track_last_transactions = 0;
static M_llist_t         *track_conns             = 0;
static M_tls_clientctx_t *track_tlsctx            = NULL;


static M_bool cli_integer_cb(char short_opt, const char *long_opt, M_int64 *integer, void *thunk)
{
	M_uint64       val;
	(void)thunk;
	(void)long_opt;
	
	/* Note: all integer args to this program are required to be non-negative. */
	if (integer == NULL || *integer < 0) {
		return M_FALSE;
	}
	val = (M_uint64)*integer;
	
	switch (short_opt) {
		case 'p':
			if (val > M_UINT16_MAX) {
				return M_FALSE;
			}
			config_port = (M_uint16)val;
			break;
		case 'c':
			if (val > SIZE_MAX) {
				return M_FALSE;
			}
			config_num_conns = (size_t)val;
			break;
		case 't':
			if (val > SIZE_MAX) {
				return M_FALSE;
			}
			config_trans_per_conn = (size_t)val;
			break;
		case 'd':
			if (val > SIZE_MAX) {
				return M_FALSE;
			}
			config_delay_per_trans_ms = (size_t)val;
			break;
		default:
			return M_FALSE;
	}

	return M_TRUE;
}


static M_bool cli_string_cb(char short_opt, const char *long_opt, const char *string, void *thunk)
{	
	(void)long_opt;
	(void)thunk;
	if (M_str_isempty(string)) {
		return M_FALSE;
	}
	
	switch (short_opt) {
		case 'U':
			M_str_cpy(config_username, sizeof(config_username), string);
			break;
		case 'P':
			M_str_cpy(config_password, sizeof(config_password), string);
			break;
		case 'H':
			M_str_cpy(config_hostname, sizeof(config_hostname), string);
			break;
		default:
			return M_FALSE;
	}
	
	return M_TRUE;
}


static void usage(const char *argv0, M_getopt_t *g)
{
	char *bname = M_fs_path_basename(argv0, M_FS_SYSTEM_AUTO);
	char *help  = M_getopt_help(g);
	
	M_printf("Usage:\r\n");
	M_printf("  %s [options]\r\n\r\n", bname);
	M_printf("Parameter Descriptions:\r\n");
	M_printf("%s\r\n", help);
	M_printf("Example:\r\n");
	M_printf("  %s -U loopback -P test123 -H testbox.monetra.com -p 8665 -t 100 -d 1000 -c 1000\r\n\r\n", bname);
	
	M_free(help);
	M_free(bname);
}


static M_bool read_cmdline(int argc, const char *const *argv)
{
	M_getopt_t       *g        = M_getopt_create(NULL);
	const char       *fail     = "?";
	M_getopt_error_t  ret;
	char              tmp[512];
	
	/* Add individual options and help descriptions. */
	/*     -- string parameters -- */
	M_snprintf(tmp, sizeof(tmp), "username          : defaults to '%s'", config_username);
	M_getopt_addstring(g, 'U', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), "password          : defaults to '%s'", config_password);
	M_getopt_addstring(g, 'P', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), "hostname          : defaults to '%s'", config_hostname);
	M_getopt_addstring(g, 'H', NULL, M_TRUE, tmp, cli_string_cb);

	/*     -- integer parameters -- */
	M_snprintf(tmp, sizeof(tmp), "port              : defaults to '%d'", (int)config_port);
	M_getopt_addinteger(g, 'p', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "trans per conn    : defaults to '%zu'", config_trans_per_conn);
	M_getopt_addinteger(g, 't', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "num connections   : defaults to '%zu'", config_num_conns);
	M_getopt_addinteger(g, 'c', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "delay per trans ms: defaults to '%zu'", config_delay_per_trans_ms);
	M_getopt_addinteger(g, 'd', NULL, M_TRUE, tmp, cli_integer_cb);

	//M_getopt_addboolean(g, 'h', NULL, M_FALSE, ": displays help message", cli_bool_cb);

	/* Parse the command line options. */
	ret = M_getopt_parse(g, argv, argc, &fail, NULL);
	
	/* Handle parsing errors or a request to print help. */
	if (ret != M_GETOPT_ERROR_SUCCESS) {
		const char *err_type_desc = NULL;
		
		switch (ret) {
			case M_GETOPT_ERROR_SUCCESS:
				break;
			case M_GETOPT_ERROR_INVALIDOPT:
				err_type_desc = "Invalid option -- ";
				break;
			case M_GETOPT_ERROR_INVALIDDATATYPE:
				err_type_desc = "Invalid option value -- ";
				break;
			case M_GETOPT_ERROR_MISSINGVALUE:
				err_type_desc = "Invalid option value missing -- ";
				break;
			case M_GETOPT_ERROR_NONOPTION:
				err_type_desc = "Invalid action -- ";
				break;
			default:
				err_type_desc = "Error -- ";
				break;
		}
		
		/* If we're aborting due to an error, print error message. */
		if (err_type_desc != NULL) {
			M_printf("Aborted - problem with command line arguments:\n");
			M_printf("   %s%s\n\n", err_type_desc, fail);
		}
		
		/* Print help. */
		usage(argv[0], g);
		
		M_getopt_destroy(g);
		return M_FALSE;
	}

	return M_TRUE;
}


/* Other helpers for main function. */

static void print_config(void)
{
	M_printf(
		"Config Summary:\r\n"
		"  Engine           : %s:%d\r\n"
		"  User             : %s\r\n"
		"  Connections      : %zu\r\n"
		"  Trans Per Conn   : %zu\r\n"
		"  Trans Delay ms   : %zu\r\n"
		"\r\n",
		config_hostname, (int)config_port, config_username, config_num_conns, config_trans_per_conn, config_delay_per_trans_ms);
}

static void monetra_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans);

typedef struct {
	M_llist_node_t *node;
	size_t          num_txns;
} conn_private_t;


static void enqueue_tran(LM_conn_t *conn)
{
	char            temp[256];
	static size_t   txn_enqueued = 0;
	LM_trans_t     *trans        = LM_trans_new(conn);
	conn_private_t *priv         = LM_conn_get_userdata(conn);
	priv->num_txns++;

	LM_trans_set_param(trans, "username", config_username);
	LM_trans_set_param(trans, "password", config_password);
	LM_trans_set_param(trans, "action",   "sale");
	LM_trans_set_param(trans, "account",  "4012888888881881");
	LM_trans_set_param(trans, "expdate",  "0549");
	LM_trans_set_param(trans, "amount",   "1.00");
	LM_trans_set_param(trans, "zip",      "32606");
	LM_trans_set_param(trans, "comments", "conn load script");
	M_snprintf(temp, sizeof(temp), "%zu", ++txn_enqueued);
	LM_trans_set_param(trans, "ptrannum", temp);
	LM_trans_send(trans);
}


static void trans_delay_cb(M_event_t *event, M_event_type_t type, M_io_t *io, void *arg)
{
	(void)event;
	(void)type;
	(void)io;

	enqueue_tran(arg);
}

static void destroy_connection(LM_conn_t *conn)
{
	conn_private_t *priv = LM_conn_get_userdata(conn);
	(void)M_llist_take_node(priv->node);
	M_free(priv);
	LM_conn_destroy(conn);
}


static void create_connection(M_event_t *event)
{
	LM_conn_t      *conn = LM_conn_init(event, monetra_callback, config_hostname, config_port);
	conn_private_t *priv = NULL;

	if (conn == NULL) {
		M_printf("\r\nFailed to create new LM_conn_t object\r\n");
		exit(1);
	}

	priv       = M_malloc_zero(sizeof(*priv));
	priv->node = M_llist_insert(track_conns, conn);

	LM_conn_set_userdata(conn, priv);
	LM_conn_set_tls_clientctx(conn, track_tlsctx);
	LM_conn_connect(conn);
}


static void monetra_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans)
{
	conn_private_t *priv = LM_conn_get_userdata(conn);
	
	switch (type) {
		case LM_EVENT_CONN_CONNECTED:
			track_num_connections++;

			/* Enqueue transaction */
			enqueue_tran(conn);

			/* Check to see if we need more connections and start another */
			if (M_llist_len(track_conns) < config_num_conns)
				create_connection(event);
			break;
		case LM_EVENT_CONN_DISCONNECT:
			track_num_connections--;

			/* Destroy conn object */
			destroy_connection(conn);

			/* Start new connection */
			create_connection(event);
			break;
		case LM_EVENT_CONN_ERROR:
			M_printf("\r\nDisconnect or Error received: %s\n", LM_conn_error(conn));
			exit(1);
			break;
		case LM_EVENT_TRANS_DONE:
			track_num_transactions++;

			/* Clean up transction */
			LM_trans_delete(trans);

			/* Start disconnect if max trans per connection hit, otherwise enqueue new transaction */
			if (priv->num_txns == config_trans_per_conn) {
				LM_conn_disconnect(conn);
			} else {
				if (config_delay_per_trans_ms) {
					M_event_timer_oneshot(event, config_delay_per_trans_ms, M_TRUE, trans_delay_cb, conn);
				} else {
					enqueue_tran(conn);
				}
			}
			break;
		case LM_EVENT_TRANS_ERROR:
		case LM_EVENT_TRANS_NOCONNECT:
			/* We should still get an LM_EVENT_CONN_ERROR after this event for additional cleanup. */
			break;
	}
}


#ifdef M_HAS_VT100
typedef enum {
	VT100_CMD_SAVEPOS,
	VT100_CMD_RESTOREPOS,
	VT100_CMD_CLEAREOL
} VT100_CMDS;

static void vt100_cmd(VT100_CMDS cmd)
{
	static const char ascii_esc = 27;
	
	switch (cmd) {
		case VT100_CMD_SAVEPOS:
			M_printf("%c7", ascii_esc);
			break;
		case VT100_CMD_RESTOREPOS:
			M_printf("%c8", ascii_esc);
			break;
		case VT100_CMD_CLEAREOL:
			M_printf("%c[0K", ascii_esc);
			break;
	}
	fflush(stdout);
}
#endif /* M_HAS_VT100 */


static void status_callback(M_event_t *event, M_event_type_t type, M_io_t *iodummy, void *arg)
{
	M_uint64 elapsed_ms      = M_time_elapsed(&track_starttime);
	M_uint64 last_elapsed_ms = M_time_elapsed(&track_lasttime);
	size_t   trans_diff      = track_num_transactions - track_last_transactions;

	(void)event;
	(void)type;
	(void)iodummy;
	(void)arg;

	/* Prevent divide by zero */
	if (elapsed_ms == 0)
		elapsed_ms = 1;
	if (last_elapsed_ms == 0)
		last_elapsed_ms = 1;

#ifdef M_HAS_VT100
	vt100_cmd(VT100_CMD_CLEAREOL);
	vt100_cmd(VT100_CMD_SAVEPOS);
#endif

	M_printf("%5llu.%03llus: %zu conns, %7zu trans [current %.02f/sec] [overall %.02f/sec]",
	         elapsed_ms / 1000, elapsed_ms % 1000,
	         track_num_connections,
	         track_num_transactions,
	         (double)trans_diff / ((double)last_elapsed_ms / 1000.0),
	         (double)track_num_transactions / ((double)elapsed_ms / 1000.0));
#ifdef M_HAS_VT100
	vt100_cmd(VT100_CMD_RESTOREPOS);
#else
	M_printf("\n");
	fflush(stdout);
#endif

	M_time_elapsed_start(&track_lasttime);
	track_last_transactions = track_num_transactions;
}


int main(int argc, const char *const *argv)
{
	M_event_t       *event;
	M_event_timer_t *timer;

	/* If we're running on Windows 10, enable VT100 commands in console output. */
#if defined(_WIN32) && defined(M_HAS_VT100)
	HANDLE          h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD           dw_mode  = 0;
	
	GetConsoleMode(h_stdout, &dw_mode);
	SetConsoleMode(h_stdout, dw_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif /* _WIN32 && M_HAS_VT100 */
	
	M_printf("Connection Load Script v%s\r\n", VERSION);

	if (!read_cmdline(argc, argv))
		return EXIT_FAILURE;

	print_config();

	M_time_elapsed_start(&track_starttime);
	M_time_elapsed_start(&track_lasttime);
	track_tlsctx    = M_tls_clientctx_create();
	M_tls_clientctx_set_default_trust(track_tlsctx);
	M_tls_clientctx_set_verify_level(track_tlsctx, M_TLS_VERIFY_NONE);
	track_conns     = M_llist_create(NULL, M_LLIST_NONE);
	event           = M_event_create(M_EVENT_FLAG_NONE);

	/* Start status callback */
	timer = M_event_timer_add(event, status_callback, NULL);
	M_event_timer_set_mode(timer, M_EVENT_TIMER_MODE_MONOTONIC);
	M_event_timer_start(timer, 1000);

	/* Start first connection */
	create_connection(event);

	/* Loop forever */
	M_event_loop(event, M_TIMEOUT_INF);

	/* Cleanup. */
	M_tls_clientctx_destroy(track_tlsctx);
	M_event_destroy(event);
	M_library_cleanup();

	return 0;
}
