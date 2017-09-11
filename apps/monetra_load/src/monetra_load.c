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

#define VERSION "2.0.0"

static const char *create_user_pwd = "Test123$";
static const char *def_accts       = "4012888888881881,5454545454545454,371449635398431,6011000995500000";
static M_rand_t   *randstate       = NULL;

/* State tracking */

static M_timeval_t lasttime;
static M_timeval_t starttime;
static size_t      last_transactions_done  = 0;
static size_t      connections_established = 0;
static size_t      transactions_done       = 0;
static size_t      expected_trans          = 0;


/* Helpers for handling command-line options. */

typedef enum {
	TRANTYPE_SALE,
	TRANTYPE_PREAUTH,
	TRANTYPE_FORCE
} trantype_t;


typedef enum {
	METHOD_IP,
	METHOD_SSL
} methods_t;


typedef struct {
	char      *username;
	char      *password;
	char      *hostname;
	size_t     num_trans;
	size_t     num_conns;
	M_uint64   max_amount;
	M_uint64   exact_amount;
	char     **accounts;
	size_t     num_accounts;
	char      *expdate;
	char      *trackdata;
	char      *ksn;
	char      *street;
	char      *zip;
	size_t     user_count;
	M_bool     user_create;
	M_bool     user_cleanup;
	M_bool     user_settle;
	M_bool     auto_confirm;
	M_bool     help;
	methods_t  method;
	trantype_t trantype;
	M_uint16   port;
	M_uint16   admin_port;

	/* Runtime state */
	M_bool     delete_txn_handle;
	M_bool     expect_approved;
	size_t     expected_connections;
} cli_options_t;

static cli_options_t *cli_options_create(void)
{
	cli_options_t *opt = M_malloc_zero(sizeof(*opt));
	
	opt->username     = M_strdup("loopback");
	opt->password     = M_strdup("test123");
	opt->hostname     = M_strdup("localhost");
	opt->num_trans    = 100;
	opt->num_conns    = 10;
	opt->max_amount   = 10000;
	opt->accounts     = M_str_explode_str(',', def_accts, &opt->num_accounts);
	opt->expdate      = M_strdup("0249");
	opt->street       = M_strdup("580039");
	opt->zip          = M_strdup("32606");
	opt->user_create  = M_TRUE;
	opt->user_cleanup = M_TRUE;
	opt->user_settle  = M_FALSE;
	opt->auto_confirm = M_FALSE;
	opt->help         = M_FALSE;
	opt->method       = METHOD_SSL;
	opt->trantype     = TRANTYPE_SALE;
	opt->port         = 8665;
	opt->admin_port   = 8666;

	return opt;
}

static void cli_options_destroy(cli_options_t *opt)
{
	M_free(opt->username);
	M_free(opt->password);
	M_free(opt->hostname);
	M_str_explode_free(opt->accounts, opt->num_accounts);
	M_free(opt->expdate);
	M_free(opt->trackdata);
	M_free(opt->ksn);
	M_free(opt->street);
	M_free(opt->zip);
}


static M_uint64 GetAmount(const cli_options_t *opt)
{
	M_uint64 amnt = 0;

	if (opt->exact_amount != 0)
		return opt->exact_amount;

	while (M_TRUE) {
		amnt = M_rand_range(randstate, 1, opt->max_amount + 1);

		/* Avoid amounts in the $8.00 range as they are delay amounts which would
		 * negatively impact performance testing */
		if (amnt <= 800 || amnt > 899)
			break;
	}

	return amnt;
}


static char *GetCCNumber(cli_options_t *opt)
{
	return opt->accounts[M_rand_range(randstate, 0, opt->num_accounts)];
}


static M_bool cli_bool_cb(char short_opt, const char *long_opt, M_bool boolean, void *thunk)
{
	cli_options_t *opt = (cli_options_t*)thunk;
	
	(void)long_opt;
	(void)boolean;
	
	if (opt == NULL) {
		return M_FALSE;
	}

	switch (short_opt) {
		case 'f':
			opt->trantype = TRANTYPE_FORCE;
			break;
		case 'r':
			opt->trantype = TRANTYPE_PREAUTH;
			break;
		case 'C':
			opt->auto_confirm = M_TRUE;
			break;
		case 'n':
			opt->user_create = M_FALSE;
			break;
		case 'N':
			opt->user_cleanup = M_FALSE;
			break;
		case 'S':
			opt->user_settle = M_TRUE;
			break;
		case 'h':
			opt->help = M_TRUE;
			break;
		default:
			return M_FALSE;
	}
	
	return M_TRUE;
}


static M_bool cli_integer_cb(char short_opt, const char *long_opt, M_int64 *integer, void *thunk)
{
	cli_options_t *opt = (cli_options_t*)thunk;
	M_uint64       val;
	
	(void)long_opt;
	
	/* Note: all integer args to this program are required to be non-negative. */
	if (opt == NULL || integer == NULL || *integer < 0) {
		return M_FALSE;
	}
	val = (M_uint64)*integer;
	
	switch (short_opt) {
		case 'p':
			if (val > M_UINT16_MAX) {
				return M_FALSE;
			}
			opt->port = (M_uint16)val;
			break;
		case 't':
			if (val > SIZE_MAX) {
				return M_FALSE;
			}
			opt->num_trans = (size_t)val;
			break;
		case 'x':
			if (val > SIZE_MAX) {
				return M_FALSE;
			}
			opt->num_conns = (size_t)val;
			break;
		case 'u':
			if (val > SIZE_MAX) {
				return M_FALSE;
			}
			opt->user_count = (size_t)val;
			M_free(opt->username);
			opt->username = M_strdup("MADMIN");
			break;
		case 'a':
			if (val > M_UINT16_MAX) {
				return M_FALSE;
			}
			opt->admin_port = (M_uint16)val;
			break;
		default:
			return M_FALSE;
	}

	return M_TRUE;
}


static M_bool cli_decimal_cb(char short_opt, const char *long_opt, M_decimal_t *decimal, void *thunk)
{
	cli_options_t *opt    = (cli_options_t*)thunk;
	M_int64        decval;
	
	(void)long_opt;
	
	if (opt == NULL || decimal == NULL) {
		return M_FALSE;
	}
	
	switch (short_opt) {
		case 'A':
			decval = M_decimal_to_int(decimal, 2);
			if (decval < 0) {
				return M_FALSE;
			}
			opt->max_amount = (M_uint64)decval;
			/* max_amount+1 used as upper end of random amount range, make sure it won't overflow. */
			if (opt->max_amount >= RAND_MAX) {
				return M_FALSE;
			}
			break;
		case 'e':
			decval = M_decimal_to_int(decimal, 2);
			if (decval < 0) {
				return M_FALSE;
			}
			opt->exact_amount = (M_uint64)decval;
			break;
		default:
			return M_FALSE;
	}
	
	return M_TRUE;
}


static M_bool cli_string_cb(char short_opt, const char *long_opt, const char *string, void *thunk)
{
	cli_options_t *opt = (cli_options_t*)thunk;
	
	(void)long_opt;
	
	if (opt == NULL || M_str_isempty(string)) {
		return M_FALSE;
	}
	
	switch (short_opt) {
		case 'U':
			M_free(opt->username);
			opt->username = M_strdup(string);
			break;
		case 'P':
			M_free(opt->password);
			opt->password = M_strdup(string);
			break;
		case 'm':
			if (M_str_caseeq(string, "IP")) {
				opt->method = METHOD_IP;
			}
			else if (M_str_caseeq(string, "SSL")) {
				opt->method = METHOD_SSL;
			}
			else {
				return M_FALSE;
			}
			break;
		case 'H':
			M_free(opt->hostname);
			opt->hostname = M_strdup(string);
			break;
		case 'c':
			M_str_explode_free(opt->accounts, opt->num_accounts);
			opt->accounts = M_str_explode_str(',', string, &opt->num_accounts);
			if (opt->num_accounts == 0) {
				return M_FALSE;
			}
			break;
		case 'd':
			M_free(opt->expdate);
			opt->expdate = M_strdup(string);
			break;
		case 'w':
			M_free(opt->trackdata);
			opt->trackdata = M_strdup(string);
			break;
		case 'k':
			M_free(opt->ksn);
			opt->ksn = M_strdup(string);
			break;
		case 's':
			M_free(opt->street);
			opt->street = M_strdup(string);
			break;
		case 'z':
			M_free(opt->zip);
			opt->zip = M_strdup(string);
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
	M_printf("  %s -U loopback -P test123 -H testbox.monetra.com -p 8444 -t 10000 -x 50\r\n\r\n", bname);
	
	M_free(help);
	M_free(bname);
}


static cli_options_t *read_cmdline(int argc, const char *const *argv)
{
	cli_options_t    *opt      = cli_options_create();
	M_getopt_t       *g        = M_getopt_create(NULL);
	const char       *fail     = "?";
	M_getopt_error_t  ret;
	char              tmp[512];
	
	/* Add individual options and help descriptions. */
	/*     -- string parameters -- */
	M_snprintf(tmp, sizeof(tmp), " username     : defaults to '%s'", opt->username);
	M_getopt_addstring(g, 'U', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " password     : defaults to '%s'", opt->password);
	M_getopt_addstring(g, 'P', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " method       : IP or SSL, defaults to %s", (opt->method == METHOD_IP)? "IP" : "SSL");
	M_getopt_addstring(g, 'm', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " hostname     : defaults to '%s'", opt->hostname);
	M_getopt_addstring(g, 'H', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " cc1,cc2,...  : comma-delimited list of cc account numbers to use\r\n"
	         "                                  : default '%s'", def_accts);
	M_getopt_addstring(g, 'c', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " expdate      : card expiration date (MMYY): defaults to '%s'", opt->expdate);
	M_getopt_addstring(g, 'd', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " trackdata    : use specified trackdata instead of keyed account number");
	M_getopt_addstring(g, 'w', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " ksn          : KSN to use with trackdata when Cardshield encrypted");
	M_getopt_addstring(g, 'k', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " street       : defaults to '%s'", opt->street);
	M_getopt_addstring(g, 's', NULL, M_TRUE, tmp, cli_string_cb);
	M_snprintf(tmp, sizeof(tmp), " zipcode      : defaults to '%s'", opt->zip);
	M_getopt_addstring(g, 'z', NULL, M_TRUE, tmp, cli_string_cb);
	
	/*     -- decimal parameters -- */
	M_snprintf(tmp, sizeof(tmp), "max amount   : defaults to '%llu.%02llu", opt->max_amount/100, opt->max_amount%100);
	M_getopt_adddecimal(g, 'A', NULL, M_TRUE, tmp, cli_decimal_cb);
	M_snprintf(tmp, sizeof(tmp), "exact amount : use only a single amount, instead of a random one");
	M_getopt_adddecimal(g, 'e', NULL, M_TRUE, tmp, cli_decimal_cb);
	
	/*     -- integer parameters -- */
	M_snprintf(tmp, sizeof(tmp), "port         : defaults to '%d'", (int)opt->port);
	M_getopt_addinteger(g, 'p', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "num trans    : defaults to '%zu'", opt->num_trans);
	M_getopt_addinteger(g, 't', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "num connects : defaults to '%zu'", opt->num_conns);
	M_getopt_addinteger(g, 'x', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "user count   : if specified, creates the given number of users and disperses\r\n"
	        "                                    transactions across the accounts. Uses loopback as the processor.");
	M_getopt_addinteger(g, 'u', NULL, M_TRUE, tmp, cli_integer_cb);
	M_snprintf(tmp, sizeof(tmp), "admin port   : defaults to '%d'\r\n", (int)opt->admin_port);
	M_getopt_addinteger(g, 'a', NULL, M_TRUE, tmp, cli_integer_cb);
	
	/*     -- option flags -- */
	M_getopt_addboolean(g, 'f', NULL, M_FALSE, ": issue forces instead of sales", cli_bool_cb);
	M_getopt_addboolean(g, 'r', NULL, M_FALSE, ": issue preauths instead of sales", cli_bool_cb);
	M_getopt_addboolean(g, 'C', NULL, M_FALSE, ": autoconfirm start rather than waiting for key press", cli_bool_cb);
	M_getopt_addboolean(g, 'n', NULL, M_FALSE, ": if user count set, do not create users (they already exist)",
		cli_bool_cb);
	M_getopt_addboolean(g, 'N', NULL, M_FALSE, ": if user count set, do not cleanup users (to reuse on future run)",
		cli_bool_cb);
	M_getopt_addboolean(g, 'S', NULL, M_FALSE, ": settle outstanding transactions (for all users if usercount is set)",
		cli_bool_cb);
	M_getopt_addboolean(g, 'h', NULL, M_FALSE, ": displays help message", cli_bool_cb);
	
	/* Parse the command line options. */
	ret = M_getopt_parse(g, argv, argc, &fail, opt);
	
	/* Handle parsing errors or a request to print help. */
	if (ret != M_GETOPT_ERROR_SUCCESS || opt->help) {
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
		
		cli_options_destroy(opt);
		M_getopt_destroy(g);
		return NULL;
	}
	
	/* If trackdata is set, use it instead of the listed accounts. */
	if (!M_str_isempty(opt->trackdata)) {
		M_str_explode_free(opt->accounts, opt->num_accounts);
		opt->accounts     = NULL;
		opt->num_accounts = 0;
	}
	
	M_getopt_destroy(g);
	return opt;
}


/* Other helpers for main function. */

static void print_config(const cli_options_t *opt)
{
	size_t i;

	M_printf(
		"Config Summary:\r\n"
		"  Engine           : %s:%d\r\n"
		"  User Count       : %zu\r\n"
		"  User Create      : %s\r\n"
		"  User Cleanup     : %s\r\n"
		"  User Settle      : %s\r\n"
		"  User             : %s\r\n"
		"  Method           : %s\r\n"
		"  Trans            : %zu\r\n"
		"  Connections      : %zu\r\n"
		"  Exact Amount     : %.2f\r\n"
		"  Max Tran Amount  : %.2f\r\n"
		"  AVS Street       : %s\r\n"
		"  AVS Zip          : %s\r\n"
		"  Auto Confirm     : %s\r\n"
		"  Trackdata        : %s\r\n"
		"  KSN              : %s\r\n"
		"  ExpDate          : %s\r\n"
		"  Card Numbers     : ",
		opt->hostname, (int)opt->port, opt->user_count,
		(opt->user_count > 0 && opt->user_create)?  "yes" : "no",
		(opt->user_count > 0 && opt->user_cleanup)? "yes" : "no", 
		(opt->user_count > 0 && opt->user_settle)?  "yes" : "no",
		opt->username, (opt->method == METHOD_IP)? "IP" : "SSL", opt->num_trans, opt->num_conns,
		(double)opt->exact_amount / 100.00, (double)opt->max_amount / 100.00, opt->street, opt->zip,
		(opt->auto_confirm)? "yes" : "no", M_str_safe(opt->trackdata), M_str_safe(opt->ksn), opt->expdate
	);

	for (i=0; i<opt->num_accounts; i++) {
		if (i != 0) {
			M_printf(":");
		}
		M_printf("%s", opt->accounts[i]);
	}
	M_printf("\r\n\r\n");
}


static void monetra_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans)
{
	cli_options_t *opt = LM_conn_get_userdata(conn);
	
	switch (type) {
		case LM_EVENT_CONN_CONNECTED:
			connections_established++;

			/* Let the event handler break out so it can enqueue transactions */
			if (connections_established == opt->expected_connections)
				M_event_done(event);
			break;
		case LM_EVENT_CONN_DISCONNECT:
		case LM_EVENT_CONN_ERROR:
			M_printf("Disconnect or Error received: %s\n", LM_conn_error(conn));
			exit(1);
			break;
		case LM_EVENT_TRANS_DONE:
			transactions_done++;

			if (opt->expect_approved) {
				if (LM_trans_response_type(trans) != LM_TRANS_RESPONSE_CSV && !M_str_caseeq(LM_trans_response_param(trans, "code"), "AUTH")) {
					M_printf("Transaction denied: %s\n", LM_trans_response_param(trans, "verbiage"));
					exit(1);
				}
			}

			/* Exit event handler when all transactions are done */
			if (transactions_done == expected_trans)
				M_event_done(event);

			if (opt->delete_txn_handle)
				LM_trans_delete(trans);
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
	M_uint64 elapsed_ms      = M_time_elapsed(&starttime);
	M_uint64 last_elapsed_ms = M_time_elapsed(&lasttime);
	size_t   trans_diff      = transactions_done - last_transactions_done;

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
	if (arg == NULL) {
		M_printf("%5llu.%03llus: %7zu transactions done [current %.02f/sec] [overall %.02f/sec]",
		         elapsed_ms / 1000, elapsed_ms % 1000,
		         transactions_done,
		         (double)trans_diff / ((double)last_elapsed_ms / 1000.0),
		         (double)transactions_done / ((double)elapsed_ms / 1000.0));
	} else {
		M_printf("%5llu.%03llus: %7zu requests complete", elapsed_ms / 1000, elapsed_ms % 1000, transactions_done);
	}
#ifdef M_HAS_VT100
	vt100_cmd(VT100_CMD_RESTOREPOS);
#else
	M_printf("\n");
	fflush(stdout);
#endif

	M_time_elapsed_start(&lasttime);
	last_transactions_done = transactions_done;
}


static void enqueue_tran(LM_conn_t *conn, M_uint64 ptrannum, cli_options_t *opt)
{
	char     temp[256];
	M_uint64 amt;

	LM_trans_t *trans = LM_trans_new(conn);
	if (opt->user_count > 0) {
		M_snprintf(temp, sizeof(temp), "u%010llu", M_rand_range(randstate, 0, opt->user_count));
		LM_trans_set_param(trans, "username", temp);
		LM_trans_set_param(trans, "password", create_user_pwd);
	} else {
		LM_trans_set_param(trans, "username", opt->username);
		LM_trans_set_param(trans, "password", opt->password);
	}

	switch (opt->trantype) {
		case TRANTYPE_FORCE:
			LM_trans_set_param(trans, "action", "force");
			LM_trans_set_param(trans, "apprcode", "999999");
			break;
		case TRANTYPE_PREAUTH:
			LM_trans_set_param(trans, "action", "preauth");
			break;
		case TRANTYPE_SALE:
			LM_trans_set_param(trans, "action", "sale");
			break;
	}
	if (opt->num_accounts > 0) {
		LM_trans_set_param(trans, "account", GetCCNumber(opt));
		LM_trans_set_param(trans, "expdate", opt->expdate);
	}
	if (!M_str_isempty(opt->trackdata)) {
		LM_trans_set_param(trans, (M_str_isempty(opt->ksn)) ? "trackdata" : "e_trackdata", opt->trackdata); 
	}
	if (!M_str_isempty(opt->ksn)) {
		LM_trans_set_param(trans, "e_id", opt->ksn);
	}
	amt = GetAmount(opt);
	M_snprintf(temp, sizeof(temp), "%llu.%02llu", amt / 100, amt % 100);
	LM_trans_set_param(trans, "amount", temp);
	LM_trans_set_param(trans, "street", opt->street);
	LM_trans_set_param(trans, "zip", opt->zip);
	LM_trans_set_param(trans, "comments", "push script");
	M_snprintf(temp, sizeof(temp), "%llu", ptrannum);
	LM_trans_set_param(trans, "ptrannum", temp);
	LM_trans_set_param(trans, "priority", "low");

	LM_trans_send(trans);
}


static LM_conn_t **start_connections(M_event_t *event, size_t num, M_bool is_admin, cli_options_t *opt)
{
	LM_conn_t        **conns;
	M_tls_clientctx_t *tlsctx;
	size_t             i;

	conns     = M_malloc_zero(sizeof(*conns) * num);
	tlsctx    = M_tls_clientctx_create();
	
	M_tls_clientctx_set_default_trust(tlsctx);
	M_tls_clientctx_set_verify_level(tlsctx, M_TLS_VERIFY_NONE);
	opt->expected_connections = num;
	connections_established = 0;
	if (!is_admin)
		M_printf("Starting %zu connections...\r\n", num);
	for (i=0; i<num; i++) {
		conns[i] = LM_conn_init(event, monetra_callback, opt->hostname, is_admin? opt->admin_port : opt->port);
		LM_conn_set_userdata(conns[i], opt);
		LM_conn_set_tls_clientctx(conns[i], tlsctx);
		if (opt->method == METHOD_IP)
			LM_conn_change_mode(conns[i], LM_MODE_IP);
		LM_conn_connect(conns[i]);
	}
	M_tls_clientctx_destroy(tlsctx);

	/* Run event loop until all connections are established, will return when
	 * they are */
	M_event_loop(event, M_TIMEOUT_INF);

	if (!is_admin)
		M_printf("Connected\r\n");
	return conns;
}


static void destroy_connections(LM_conn_t **conns, size_t num)
{
	size_t i;

	/* XXX: Should really do graceful disconnects and wait */
	for (i=0; i<num; i++)
		LM_conn_destroy(conns[i]);
	M_free(conns);
}

enum REQUEST_FLAGS {
	REQUEST_FLAG_NONE          = 0,
	REQUEST_FLAG_REQ_APPROVALS = 1 << 0,
	REQUEST_FLAG_MIN_STATUS    = 1 << 1,
	REQUEST_FLAG_TXN_NO_DELETE = 1 << 2
};

static void send_requests(M_event_t *event, size_t expected_response_cnt, cli_options_t *opt, enum REQUEST_FLAGS request_flags)
{
	M_event_timer_t   *timer;

	if (expected_response_cnt == 0) {
		if (!(request_flags & REQUEST_FLAG_MIN_STATUS)) {
			M_printf("Transactions Done\r\n");
		}
		return;
	}

	opt->delete_txn_handle = (request_flags & REQUEST_FLAG_TXN_NO_DELETE)?M_FALSE:M_TRUE;

	/* Start timer for throughput status info */
	if (!(request_flags & REQUEST_FLAG_MIN_STATUS))
		M_printf("Sending %zu Transactions\r\n", expected_response_cnt);

	M_time_elapsed_start(&starttime);
	M_time_elapsed_start(&lasttime);
	last_transactions_done = 0;
	transactions_done = 0;

	timer = M_event_timer_add(event, status_callback, (request_flags & REQUEST_FLAG_MIN_STATUS)?(void *)1:NULL);
	M_event_timer_set_mode(timer, M_EVENT_TIMER_MODE_MONOTONIC);
	M_event_timer_start(timer, 1000);

	/* Run event loop until all transactions are done */
	expected_trans       = expected_response_cnt;
	opt->expect_approved = (request_flags & REQUEST_FLAG_REQ_APPROVALS)?M_TRUE:M_FALSE;
	M_event_loop(event, M_TIMEOUT_INF);

	/* Call status update one last time to print the final result */
	status_callback(event, M_EVENT_TYPE_OTHER, NULL, (request_flags & REQUEST_FLAG_MIN_STATUS)?(void *)1:NULL);
	M_event_timer_remove(timer);

	M_printf("\r\n");
	if (!(request_flags & REQUEST_FLAG_MIN_STATUS))
		M_printf("Transactions Done\r\n");
}


static void enqueue_transactions(LM_conn_t **conns, size_t nconns, size_t ntxns, cli_options_t *opt)
{
	size_t i;
	/* Enqueue transactions */
	for (i=0; i<ntxns; i++) {
		enqueue_tran(conns[i % nconns], ((M_uint64)(i % nconns) * 1000000) + (i / nconns), opt);
	}
}


static void enqueue_delete_users(LM_conn_t **conns, size_t nconns, size_t nusers, cli_options_t *opt)
{
	size_t i;
	/* Enqueue transactions */
	for (i=0; i<nusers; i++) {
		char temp[256];
		LM_trans_t *trans = LM_trans_new(conns[i % nconns]);
		LM_trans_set_param(trans, "username", opt->username);
		LM_trans_set_param(trans, "password", opt->password);
		LM_trans_set_param(trans, "action", "deluser");
		M_snprintf(temp, sizeof(temp), "u%010zu", i);
		LM_trans_set_param(trans, "user", temp);
		LM_trans_send(trans);
	}
}


static void enqueue_add_users(LM_conn_t **conns, size_t nconns, size_t nusers, cli_options_t *opt)
{
	size_t i;
	/* Enqueue transactions */
	for (i=0; i<nusers; i++) {
		char temp[256];
		LM_trans_t *trans = LM_trans_new(conns[i % nconns]);
		LM_trans_set_param(trans, "username", opt->username);
		LM_trans_set_param(trans, "password", opt->password);
		LM_trans_set_param(trans, "action", "adduser");
		M_snprintf(temp, sizeof(temp), "u%010zu", i);
		LM_trans_set_param(trans, "user", temp);
		LM_trans_set_param(trans, "pwd", create_user_pwd);
		LM_trans_set_param(trans, "proc", "loopback");
		LM_trans_set_param(trans, "indcode", "R");
		LM_trans_set_param(trans, "mode", "BOTH");
		LM_trans_set_param(trans, "cardtypes", "VISA+MC+AMEX+DISC+JCB+CB+DINERS+GIFT");
		LM_trans_set_param(trans, "geninterchange", "yes");
		LM_trans_set_param(trans, "zipcode", "32606");
		LM_trans_set_param(trans, "settledelay", "0");
		LM_trans_send(trans);
	}
}


static void create_users(M_event_t *event, cli_options_t *opt)
{
	LM_conn_t **conns;
	size_t num_conns = opt->num_conns;
	if (num_conns > (opt->user_count / 10) + 1)
		num_conns = (opt->user_count / 10) + 1;

	M_printf("Adding %zu users...\r\n", opt->user_count);
	M_printf("Connecting to MADMIN port (%zu conns)\r\n", num_conns);
	conns = start_connections(event, num_conns, M_TRUE, opt);
	M_printf("Connected, clearing conflicting user accounts (use -n if trying to preserve)...\r\n");
	enqueue_delete_users(conns, num_conns, opt->user_count, opt);
	send_requests(event, opt->user_count, opt, REQUEST_FLAG_MIN_STATUS);

	M_printf("Adding new user accounts...\r\n");
	enqueue_add_users(conns, num_conns, opt->user_count, opt);
	send_requests(event, opt->user_count, opt, REQUEST_FLAG_REQ_APPROVALS | REQUEST_FLAG_MIN_STATUS);

	destroy_connections(conns, num_conns);
	M_printf("Done\r\n");
}


static void cleanup_users(M_event_t *event, cli_options_t *opt)
{
	LM_conn_t **conns;
	size_t num_conns = opt->num_conns;
	if (num_conns > (opt->user_count / 10) + 1)
		num_conns = (opt->user_count / 10) + 1;

	M_printf("Cleaning up %zu users...\r\n", opt->user_count);
	M_printf("Connecting to MADMIN port\r\n");
	conns = start_connections(event, num_conns, M_TRUE, opt);
	M_printf("Connected, deleting user accounts (use -N to prevent)...\r\n");
	enqueue_delete_users(conns, num_conns, opt->user_count, opt);
	send_requests(event, opt->user_count, opt, REQUEST_FLAG_REQ_APPROVALS | REQUEST_FLAG_MIN_STATUS);

	destroy_connections(conns, num_conns);
	M_printf("Done\r\n");

}

static void settle_batches(LM_conn_t **conns, cli_options_t *opt, M_event_t *event)
{
	size_t       i;
	size_t       nusers  = opt->user_count;
	size_t       batches = 0;
	LM_trans_t **bttrans = NULL;
	if (nusers == 0)
		nusers = 1;

	bttrans = M_malloc_zero(sizeof(*bttrans) * nusers);

	M_printf("Settling %zu users...\r\n", nusers);
	M_printf("  * Requesting batch totals...\r\n");
	/* Enqueue transactions */
	for (i=0; i<nusers; i++) {
		bttrans[i] = LM_trans_new(conns[i % opt->num_conns]);
		if (opt->user_count) {
			char temp[256];
			M_snprintf(temp, sizeof(temp), "u%010zu", i);
			LM_trans_set_param(bttrans[i], "username", temp);
			LM_trans_set_param(bttrans[i], "password", create_user_pwd);
		} else {
			LM_trans_set_param(bttrans[i], "username", opt->username);
			LM_trans_set_param(bttrans[i], "password", opt->password);
		}
		LM_trans_set_param(bttrans[i], "action", "admin");
		LM_trans_set_param(bttrans[i], "admin", "BT");
		LM_trans_send(bttrans[i]);
	}
	send_requests(event, nusers, opt, REQUEST_FLAG_REQ_APPROVALS | REQUEST_FLAG_MIN_STATUS | REQUEST_FLAG_TXN_NO_DELETE);

	for (i=0; i<nusers; i++) {
		const M_csv_t *csv = LM_trans_response_csv(bttrans[i]);
		if (csv != NULL) {
			size_t j;
			for (j=0; j<M_csv_get_numrows(csv); j++) {
				LM_trans_t *trans = LM_trans_new(conns[batches % opt->num_conns]);
				if (opt->user_count) {
					char temp[256];
					M_snprintf(temp, sizeof(temp), "u%010zu", i);
					LM_trans_set_param(trans, "username", temp);
					LM_trans_set_param(trans, "password", create_user_pwd);
				} else {
					LM_trans_set_param(trans, "username", opt->username);
					LM_trans_set_param(trans, "password", opt->password);
				}
				LM_trans_set_param(trans, "action", "settle");
				LM_trans_set_param(trans, "batch", M_csv_get_cell(csv, j, "BatchNum"));
				LM_trans_set_param(trans, "sub", M_csv_get_cell(csv, j, "Sub"));
				LM_trans_send(trans);
				batches++;
			}
		}
		LM_trans_delete(bttrans[i]);
	}
	M_free(bttrans);

	M_printf("  * Settling %zu batches...\r\n", batches);
	send_requests(event, batches, opt, REQUEST_FLAG_REQ_APPROVALS | REQUEST_FLAG_MIN_STATUS);
	M_printf("Batch settlements complete\r\n");
}


int main(int argc, const char *const *argv)
{
	M_event_t      *event;
	LM_conn_t     **conns;
	cli_options_t  *opt;

	/* If we're running on Windows 10, enable VT100 commands in console output. */
#if defined(_WIN32) && defined(M_HAS_VT100)
	HANDLE          h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD           dw_mode  = 0;
	
	GetConsoleMode(h_stdout, &dw_mode);
	SetConsoleMode(h_stdout, dw_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif /* _WIN32 && M_HAS_VT100 */
	
	M_printf("Load Script v%s\r\n", VERSION);

	opt = read_cmdline(argc, argv);
	if (opt == NULL) {
		return EXIT_FAILURE;
	}

	print_config(opt);

	randstate = M_rand_create(0);
	event     = M_event_create(M_EVENT_FLAG_NONE);

	if (opt->user_count > 0 && opt->user_create) {
		create_users(event, opt);
	}

	conns = start_connections(event, opt->num_conns, M_FALSE, opt);

	enqueue_transactions(conns, opt->num_conns, opt->num_trans, opt);

	M_printf("Transactions Ready\r\n");

	if (!opt->auto_confirm) {
		M_printf("Press Enter to process\r\n");
		fflush(stdout);
		getchar();
	}

	/* Send the requests and wait for all responses */
	send_requests(event, opt->num_trans, opt, REQUEST_FLAG_NONE);

	/* Handle batch settlements here */
	if (opt->user_settle) {
		settle_batches(conns, opt, event);
	}

	/* Make sure we destroy the connections prior to closing the event loop */
	destroy_connections(conns, opt->num_conns);

	if (opt->user_count > 0 && opt->user_cleanup) {
		cleanup_users(event, opt);
	}

	/* Cleanup. */
	M_event_destroy(event);
	M_rand_destroy(randstate);
	M_library_cleanup();
	cli_options_destroy(opt);

	return 0;
}
