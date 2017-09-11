#include "monetra_api.h"
#include "monetra.h"


int LM_SPEC M_ReturnCode(M_CONN *conn, M_uintptr identifier)
{
	const char *v;

	if (M_IsCommaDelimited(conn, identifier)) {
		return M_SUCCESS;
	}

	v = M_ResponseParam(conn, identifier, "code");

	if (v == NULL) {
		return M_ERROR;
	}

	if (M_str_eq(v, "AUTH")) {
		return M_AUTH;
	}

	if (M_str_eq(v, "DENY")) {
		return M_DENY;
	}

	if (M_str_eq(v, "DUPL")) {
		return M_DUPL;
	}

	if (M_str_eq(v, "CALL")) {
		return M_CALL;
	}

	if (M_str_eq(v, "PKUP")) {
		return M_PKUP;
	}

	if (M_str_eq(v, "RETRY")) {
		return M_RETRY;
	}

	if (M_str_eq(v, "SETUP")) {
		return M_SETUP;
	}

	if (M_str_eq(v, "SUCCESS")) {
		return M_SUCCESS;
	}

	if (M_str_eq(v, "FAIL")) {
		return M_FAIL;
	}

	if (M_str_eq(v, "TIMEOUT")) {
		return M_TIMEOUT;
	}

	return M_DENY;
}

long LM_SPEC M_TransactionItem(M_CONN *conn, M_uintptr identifier)
{
	const char *v = M_ResponseParam(conn, identifier, "item");

	if (v == NULL)
		return -1;

	return M_str_to_int32(v);
}

long LM_SPEC M_TransactionBatch(M_CONN *conn, M_uintptr identifier)
{
	const char *v = M_ResponseParam(conn, identifier, "batch");

	if (v == NULL)
		return -1;

	return M_str_to_int32(v);
}

M_int64 LM_SPEC M_TransactionID(M_CONN *conn, M_uintptr identifier)
{
	const char *v = M_ResponseParam(conn, identifier, "ttid");

	if (v == NULL)
		return -1;

	return M_str_to_int64(v);
}

const char * LM_SPEC M_TransactionAuth(M_CONN *conn, M_uintptr identifier)
{
	return M_ResponseParam(conn, identifier, "auth");
}

const char * LM_SPEC M_TransactionText(M_CONN *conn, M_uintptr identifier)
{
	return M_ResponseParam(conn, identifier, "verbiage");
}

int LM_SPEC M_TransactionAVS(M_CONN *conn, M_uintptr identifier)
{
	const char *v;

	v = M_ResponseParam(conn, identifier, "avs");

	if (v == NULL) {
		return -1;
	}

	if (M_str_eq(v, "GOOD")) {
		return M_GOOD;
	}

	if (M_str_eq(v, "BAD")) {
		return M_BAD;
	}

	if (M_str_eq(v, "STREET")) {
		return M_STREET;
	}

	if (M_str_eq(v, "ZIP")) {
		return M_ZIP;
	}

	return M_UNKNOWN;
}


int LM_SPEC M_TransactionCV(M_CONN *conn, M_uintptr identifier)
{
	const char *v;

	v = M_ResponseParam(conn, identifier, "cv");

	if (v == NULL) {
		return -1;
	}

	if (M_str_eq(v, "GOOD")) {
		return M_GOOD;
	}

	if (M_str_eq(v, "BAD")) {
		return M_BAD;
	}

	return M_UNKNOWN;
}


const char * LM_SPEC M_TEXT_Code(int code)
{
	const char *text = NULL;

	switch (code) {
		case M_AUTH:
			text = "AUTH";
			break;
		case M_DENY:
			text = "DENY";
			break;
		case M_DUPL:
			text = "DUPLICATE TRANS";
			break;
		case M_CALL:
			text = "CALL PROCESSOR";
			break;
		case M_PKUP:
			text = "PICKUP CARD";
			break;
		case M_RETRY:
			text = "RETRY";
			break;
		case M_SETUP:
			text = "SETUP";
			break;
		case M_TIMEOUT:
			text = "TIMEOUT";
			break;
		case M_SUCCESS:
			text = "SUCCESS";
			break;
		case M_FAIL:
			text = "FAIL";
			break;
		case M_ERROR:
			text = "ERROR";
			break;
		default:
			text = "UNKNOWN";
			break;
	}

	return text;
}

const char * LM_SPEC M_TEXT_AVS(int code)
{
	const char *text = NULL;

	switch (code) {
		case M_STREET:
			text = "STREET FAILED";
			break;
		case M_ZIP:
			text = "ZIP FAILED";
			break;
		case M_GOOD:
			text = "GOOD";
			break;
		case M_BAD:
			text = "BAD";
			break;
		case M_UNKNOWN:
			text = "UNKNOWN";
			break;
		default:
			text = "UNKNOWN";
			break;
	}

	return text;
}

const char * LM_SPEC M_TEXT_CV(int code)
{
	return M_TEXT_AVS(code);
}


#ifndef LIBMONETRA_SPEC_STDCALL


static char *M_tp_transform_action(int action)
{
	switch (action) {
		case MC_TRAN_SALE:
			return M_strdup("SALE");
		case MC_TRAN_PREAUTH:
			return M_strdup("PREAUTH");
		case MC_TRAN_VOID:
			return M_strdup("VOID");
		case MC_TRAN_PREAUTHCOMPLETE:
			return M_strdup("FORCE");
		case MC_TRAN_FORCE:
			return M_strdup("FORCE");
		case MC_TRAN_OVERRIDE:
			return M_strdup("OVERRIDE");
		case MC_TRAN_RETURN:
			return M_strdup("RETURN");
		case MC_TRAN_SETTLE:
			return M_strdup("SETTLE");
		case MC_TRAN_ADMIN:
			return M_strdup("ADMIN");
		case MC_TRAN_CHKPWD:
			return M_strdup("CHKPWD");
		case MC_TRAN_INCREMENTAL:
			return M_strdup("INCREMENTAL");
		case MC_TRAN_REVERSAL:
			return M_strdup("REVERSAL");
		case MC_TRAN_ACTIVATE:
			return M_strdup("ACTIVATE");
		case MC_TRAN_BALANCEINQ:
			return M_strdup("BALANCEINQ");
		case MC_TRAN_CASHOUT:
			return M_strdup("CASHOUT");
		case MC_TRAN_TOREVERSAL:
			return M_strdup("TOREVERSAL");
		case MC_TRAN_SETTLERFR:
			return M_strdup("SETTLERFR");
		case MC_TRAN_ISSUE:
			return M_strdup("ISSUE");
		case MC_TRAN_TIP:
			return M_strdup("TIP");
		case MC_TRAN_MERCHRETURN:
			return M_strdup("MERCHRETURN");
		case MC_TRAN_IVRREQ:
			return M_strdup("IVRREQ");
		case MC_TRAN_IVRRESP:
			return M_strdup("IVRRESP");
		case MC_TRAN_CHNGPWD:
			return M_strdup("CHNGPWD");
		case MC_TRAN_LISTSTATS:
			return M_strdup("LISTSTATS");
		case MC_TRAN_LISTUSERS:
			return M_strdup("LISTUSERS");
		case MC_TRAN_GETUSERINFO:
			return M_strdup("GETUSERINFO");
		case MC_TRAN_ADDUSER:
			return M_strdup("ADDUSER");
		case MC_TRAN_EDITUSER:
			return M_strdup("EDITUSER");
		case MC_TRAN_DELUSER:
			return M_strdup("DELUSER");
		case MC_TRAN_ENABLEUSER:
			return M_strdup("ENABLEUSER");
		case MC_TRAN_DISABLEUSER:
			return M_strdup("DISABLEUSER");
		case MC_TRAN_IMPORT:
			return M_strdup("IMPORT");
		case MC_TRAN_EXPORT:
			return M_strdup("EXPORT");
		case MC_TRAN_ERRORLOG:
			return M_strdup("ERRORLOG");
		case MC_TRAN_CLEARERRORLOG:
			return M_strdup("CLEARERRORLOG");
		case MC_TRAN_GETSUBACCTS:
			return M_strdup("GETSUBACCTS");
	}
	return NULL;
}

static char *M_tp_transform_admin(int admin)
{
	switch (admin) {
		case MC_ADMIN_GUT:
			return M_strdup("GUT");
		case MC_ADMIN_GL:
			return M_strdup("GL");
		case MC_ADMIN_GFT:
			return M_strdup("GFT");
		case MC_ADMIN_QC:
			return M_strdup("QC");
		case MC_ADMIN_RS:
			return M_strdup("RS");
		case MC_ADMIN_BT:
			return M_strdup("BT");
		case MC_ADMIN_CTH:
			return M_strdup("CTH");
		case MC_ADMIN_CFH:
			return M_strdup("CFH");
		case MC_ADMIN_FORCESETTLE:
			return M_strdup("FORCESETTLE");
		case MC_ADMIN_SETBATCHNUM:
			return M_strdup("SETBATCHNUM");
		case MC_ADMIN_RENUMBERBATCH:
			return M_strdup("RENUMBERBATCH");
		case MC_ADMIN_FIELDEDIT:
			return M_strdup("FIELDEDIT");
		case MC_ADMIN_CLOSEBATCH:
			return M_strdup("CLOSEBATCH");
	}
	return NULL;
}

static char *M_tp_transform_excharges(int excharges)
{
	switch (excharges) {
		case MC_EXCHARGES_REST:
			return M_strdup("REST");
		case MC_EXCHARGES_GIFT:
			return M_strdup("GIFT");
		case MC_EXCHARGES_MINI:
			return M_strdup("MINI");
		case MC_EXCHARGES_TELE:
			return M_strdup("TELE");
		case MC_EXCHARGES_OTHER:
			return M_strdup("OTHER");
		case MC_EXCHARGES_LAUND:
			return M_strdup("LAUND");
		case MC_EXCHARGES_NONE:
			return M_strdup("NONE");
		case MC_EXCHARGES_GAS:
			return M_strdup("GAS");
		case MC_EXCHARGES_MILE:
			return M_strdup("MILE");
		case MC_EXCHARGES_LATE:
			return M_strdup("LATE");
		case MC_EXCHARGES_1WAY:
			return M_strdup("1WAY");
		case MC_EXCHARGES_VIOL:
			return M_strdup("VIOL");
	}
	return NULL;
}

static char *M_tp_transform_cardtypes(int types)
{
	M_buf_t *buf = M_buf_create();

	if (types == MC_CARD_ALL) {
		M_buf_add_str(buf, "ALL");
		return M_buf_finish_str(buf, NULL);
	}

	if (types & MC_CARD_VISA) {
		M_buf_add_str(buf, "+VISA");
	}

	if (types & MC_CARD_MC) {
		M_buf_add_str(buf, "+MC");
	}

	if (types & MC_CARD_AMEX) {
		M_buf_add_str(buf, "+AMEX");
	}

	if (types & MC_CARD_DISC) {
		M_buf_add_str(buf, "+DISC");
	}

	if (types & MC_CARD_DC) {
		M_buf_add_str(buf, "+DC");
	}

	if (types & MC_CARD_JCB) {
		M_buf_add_str(buf, "+JCB");
	}

	if (types & MC_CARD_CB) {
		M_buf_add_str(buf, "+CB");
	}

	if (types & MC_CARD_GIFT) {
		M_buf_add_str(buf, "+GIFT");
	}

	if (types & MC_CARD_OTHER) {
		M_buf_add_str(buf, "+OTHER");
	}

	return M_buf_finish_str(buf, NULL);
}

static char *M_tp_transform_priority(int priority)
{
	switch (priority) {
		case MC_PRIO_HIGH:
			return M_strdup("HIGH");
		case MC_PRIO_NORMAL:
			return M_strdup("NORMAL");
		case MC_PRIO_LOW:
			return M_strdup("LOW");
	}
	return NULL;
}

static char *M_tp_transform_mode(int mode)
{
	M_buf_t *buf = M_buf_create();

	if (mode == MC_MODE_BOTH) {
		M_buf_add_str(buf, "BOTH");
		return M_buf_finish_str(buf, NULL);
	}

	if (mode & MC_MODE_AUTH) {
		M_buf_add_str(buf, "+AUTH");
	}

	if (mode & MC_MODE_SETTLE) {
		M_buf_add_str(buf, "+SETTLE");
	}

	return M_buf_finish_str(buf, NULL);
}

typedef enum {
	M_TP_TYPE_STRING,
	M_TP_TYPE_INT,
	M_TP_TYPE_LONG,
	M_TP_TYPE_DOUBLE,
	M_TP_TYPE_INT64,
	M_TP_TYPE_CUSTOM
} M_tp_type_t;

static const struct {
	int         key;
	const char *strkey;
	M_tp_type_t valtype;
	char       *(*int_transform)(int val);
} M_tp_list[] = {
	{ MC_TRANTYPE,       "action",         M_TP_TYPE_INT,    M_tp_transform_action    },
	{ MC_ADMIN,          "admin",          M_TP_TYPE_INT,    M_tp_transform_admin     },
	{ MC_USERNAME,       "username",       M_TP_TYPE_STRING, NULL                     },
	{ MC_PASSWORD,       "password",       M_TP_TYPE_STRING, NULL                     },
	{ MC_ACCOUNT,        "account",        M_TP_TYPE_STRING, NULL                     },
	{ MC_TRACKDATA,      "trackdata",      M_TP_TYPE_STRING, NULL                     },
	{ MC_EXPDATE,        "expdate",        M_TP_TYPE_STRING, NULL                     },
	{ MC_STREET,         "street",         M_TP_TYPE_STRING, NULL                     },
	{ MC_ZIP,            "zip",            M_TP_TYPE_STRING, NULL                     },
	{ MC_CV,             "cv",             M_TP_TYPE_STRING, NULL                     },
	{ MC_COMMENTS,       "comments",       M_TP_TYPE_STRING, NULL                     },
	{ MC_CLERKID,        "clerkid",        M_TP_TYPE_STRING, NULL                     },
	{ MC_STATIONID,      "stationid",      M_TP_TYPE_STRING, NULL                     },
	{ MC_APPRCODE,       "apprcode",       M_TP_TYPE_STRING, NULL                     },
	{ MC_AMOUNT,         "amount",         M_TP_TYPE_DOUBLE, NULL                     },
	{ MC_PTRANNUM,       "ptrannum",       M_TP_TYPE_LONG,   NULL                     },
	{ MC_TIMESTAMP,      "timestamp",      M_TP_TYPE_LONG,   NULL                     },
	{ MC_TTID,           "ttid",           M_TP_TYPE_INT64,  NULL                     },
	{ MC_ACCT,           "acct",           M_TP_TYPE_STRING, NULL                     },
	{ MC_BDATE,          "bdate",          M_TP_TYPE_STRING, NULL                     },
	{ MC_EDATE,          "edate",          M_TP_TYPE_STRING, NULL                     },
	{ MC_BATCH,          "batch",          M_TP_TYPE_STRING, NULL                     },
	{ MC_FILE,           "file",           M_TP_TYPE_STRING, NULL                     },
	{ MC_AUDITTYPE,      "type",           M_TP_TYPE_INT,    M_tp_transform_action    },
	{ MC_CUSTOM,         "",               M_TP_TYPE_CUSTOM, NULL                     },
	{ MC_EXAMOUNT,       "examount",       M_TP_TYPE_DOUBLE, NULL                     },
	{ MC_EXCHARGES,      "excharges",      M_TP_TYPE_INT,    M_tp_transform_excharges },
	{ MC_RATE,           "rate",           M_TP_TYPE_DOUBLE, NULL                     },
	{ MC_RENTERNAME,     "rentername",     M_TP_TYPE_STRING, NULL                     },
	{ MC_RETURNCITY,     "returncity",     M_TP_TYPE_STRING, NULL                     },
	{ MC_RETURNSTATE,    "returnstate",    M_TP_TYPE_STRING, NULL                     },
	{ MC_RETURNLOCATION, "returnlocation", M_TP_TYPE_STRING, NULL                     },
	{ MC_INQUIRY,        "inquiry",        M_TP_TYPE_INT,    NULL                     },
	{ MC_PRIORITY,       "priority",       M_TP_TYPE_INT,    M_tp_transform_priority  },
	{ MC_CARDTYPES,      "cardtypes",      M_TP_TYPE_INT,    M_tp_transform_cardtypes },
	{ MC_SUB,            "sub",            M_TP_TYPE_INT,    NULL                     },
	{ MC_MARKER,         "marker",         M_TP_TYPE_LONG,   NULL                     },
	{ MC_DEVICETYPE,     "devicetype",     M_TP_TYPE_STRING, NULL                     },
	{ MC_ERRORCODE,      "errorcode",      M_TP_TYPE_STRING, NULL                     },
	{ MC_NEWBATCH,       "newbatch",       M_TP_TYPE_STRING, NULL                     },
	{ MC_CURR,           "curr",           M_TP_TYPE_STRING, NULL                     },
	{ MC_DESCMERCH,      "descmerch",      M_TP_TYPE_STRING, NULL                     },
	{ MC_DESCLOC,        "descloc",        M_TP_TYPE_STRING, NULL                     },
	{ MC_ORIGTYPE,       "origtype",       M_TP_TYPE_INT,    M_tp_transform_action    },
	{ MC_VOIDORIGTYPE,   "voidorigtype",   M_TP_TYPE_INT,    M_tp_transform_action    },
	{ MC_PIN,            "pin",            M_TP_TYPE_STRING, NULL                     },

	{ MC_USER_PROC,      "proc",           M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_USER,      "user",           M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_PWD,       "pwd",            M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_INDCODE,   "indcode",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_MERCHID,   "merchid",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_BANKID,    "bankid",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_TERMID,    "termid",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_CLIENTNUM, "clientnum",      M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_STOREID,   "storeid",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_AGENTID,   "agentid",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_CHAINID,   "chainid",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_ZIPCODE,   "zipcode",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_TIMEZONE,  "timezone",       M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_MERCHCAT,  "merchcat",       M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_MERNAME,   "mername",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_MERCHLOC,  "merchloc",       M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_STATECODE, "statecode",      M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_PHONE,     "phone",          M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_SUB,       "sub",            M_TP_TYPE_INT,    NULL                     },
	{ MC_USER_CARDTYPES, "cardtypes",      M_TP_TYPE_INT,    M_tp_transform_cardtypes },
	{ MC_USER_MODE,      "mode",           M_TP_TYPE_STRING, M_tp_transform_mode      },
	{ MC_USER_VNUMBER,   "vnumber",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_ROUTINGID, "routingid",      M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_PPROPERTY, "pproperty",      M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_PID,       "pid",            M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_PIDPWD,    "pidpwd",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_SMID,      "smid",           M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_SMIDPWD,   "smidpwd",        M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_USDDIV,    "usddiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_AUDDIV,    "auddiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_DKKDIV,    "dkkdiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_GBPDIV,    "gbpdiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_HKDDIV,    "hkddiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_JPYDIV,    "jpydiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_NZDDIV,    "nzddiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_NOKDIV,    "nokdiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_SGDDIV,    "sgddiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_ZARDIV,    "zardiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_SEKDIV,    "sekdiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_CHFDIV,    "chfdiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_CADDIV,    "caddiv",         M_TP_TYPE_STRING, NULL                     },
	{ MC_USER_DIVNUM,    "divnum",         M_TP_TYPE_STRING, NULL                     },
	{ 0,                 NULL,             0,                NULL                     } 
};

int M_TransParam(M_CONN *conn, M_uintptr identifier, int key, ...)
{
	va_list     ap;
	const char *strkey;
	char       *val     = NULL;
	int         val_int;
	size_t      i;
	int         rv;

	if (conn == NULL || *conn == NULL || identifier == 0)
		return 0;

	for (i=0; M_tp_list[i].strkey != NULL; i++) {
		if (M_tp_list[i].key == key)
			break;
	}

	strkey = M_tp_list[i].strkey;

	/* Not found */
	if (strkey == NULL)
		return 0;

	va_start(ap, key);

	switch (M_tp_list[i].valtype) {
		case M_TP_TYPE_STRING:
			val = M_strdup(va_arg(ap, const char *));
			break;
		case M_TP_TYPE_INT:
			val_int = va_arg(ap, int);
			if (M_tp_list[i].int_transform) {
				val = M_tp_list[i].int_transform(val_int);
			} else {
				M_asprintf(&val, "%d", val_int);
			}
			break;
		case M_TP_TYPE_LONG:
			M_asprintf(&val, "%ld", va_arg(ap, long));
			break;
		case M_TP_TYPE_DOUBLE:
			M_asprintf(&val, "%.2f", va_arg(ap, double));
			break;
		case M_TP_TYPE_CUSTOM:
			strkey = va_arg(ap, const char *);
			val    = M_strdup(va_arg(ap, const char *));
			break;
		case M_TP_TYPE_INT64:
			M_asprintf(&val, "%lld", va_arg(ap, M_int64));
			break;
	}
	va_end(ap);

	rv = M_TransKeyVal(conn, identifier, strkey, val);
	M_free(val);
	return rv;
}
#endif


