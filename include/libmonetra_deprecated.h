#ifndef __LIBMONETRA_DEPRECATED_H__
#define __LIBMONETRA_DEPRECATED_H__

#include <libmonetra_exp.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! \addtogroup LM_deprecated Deprecated API
 *
 * Deprecated defines, data structures, and functions.
 *
 * @{
 */

#define   M_AUTH       2
#define   M_DENY       3
#define   M_CALL       4
#define   M_DUPL       5
#define   M_PKUP       6
#define   M_RETRY      7
#define   M_SETUP      8
#define   M_TIMEOUT    9

/* AVS/CVV2 response codes */
#define   M_GOOD       1
#define   M_BAD        0
#define   M_STREET     2
#define   M_ZIP        3
#define   M_UNKNOWN   -1



/* Key definitions for Transaction Parameters */
#define MC_TRANTYPE  1
#define MC_USERNAME  2
#define MC_PASSWORD  3
#define MC_ACCOUNT   4
#define MC_TRACKDATA 5
#define MC_EXPDATE   6
#define MC_STREET    7
#define MC_ZIP       8
#define MC_CV        9
#define MC_COMMENTS  10
#define MC_CLERKID   11
#define MC_STATIONID 12
#define MC_APPRCODE  13
#define MC_AMOUNT    14
#define MC_PTRANNUM  15
#define MC_TTID      16
#define MC_USER      17
#define MC_PWD       18
#define MC_ACCT      19
#define MC_BDATE     20
#define MC_EDATE     21
#define MC_BATCH     22
#define MC_FILE      23
#define MC_ADMIN     24
#define MC_AUDITTYPE 25
#define MC_CUSTOM    26

/* restaurant, lodging, auto-rental */
#define MC_EXAMOUNT       27
#define MC_EXCHARGES      28
#define MC_RATE           29
#define MC_RENTERNAME     30
#define MC_RETURNCITY     31
#define MC_RETURNSTATE    32
#define MC_RETURNLOCATION 33
#define MC_PRIORITY       34
#define MC_INQUIRY        35

#define MC_CARDTYPES    36
#define MC_SUB          37
#define MC_MARKER       38
#define MC_DEVICETYPE   39
#define MC_ERRORCODE    40
#define MC_NEWBATCH     41
#define MC_CURR         42
#define MC_DESCMERCH    43
#define MC_DESCLOC      44
#define MC_ORIGTYPE     45
#define MC_PIN          46
#define MC_VOIDORIGTYPE 47
#define MC_TIMESTAMP    48

/* Priorities */
#define MC_PRIO_HIGH   1
#define MC_PRIO_NORMAL 2
#define MC_PRIO_LOW    3

/* Excharges for lodging and auto-rental*/
#define MC_EXCHARGES_REST  1
#define MC_EXCHARGES_GIFT  2
#define MC_EXCHARGES_MINI  3
#define MC_EXCHARGES_TELE  4
#define MC_EXCHARGES_OTHER 5
#define MC_EXCHARGES_LAUND 6
#define MC_EXCHARGES_NONE  7

#define MC_EXCHARGES_GAS  8
#define MC_EXCHARGES_MILE 9
#define MC_EXCHARGES_LATE 10
#define MC_EXCHARGES_1WAY 11
#define MC_EXCHARGES_VIOL 12

/* Args for adding a user */
#define MC_USER_PROC      2000
#define MC_USER_USER      2001
#define MC_USER_PWD       2002
#define MC_USER_INDCODE   2003
#define MC_USER_MERCHID   2004
#define MC_USER_BANKID    2005
#define MC_USER_TERMID    2006
#define MC_USER_CLIENTNUM 2007
#define MC_USER_STOREID   2008
#define MC_USER_AGENTID   2009
#define MC_USER_CHAINID   2010
#define MC_USER_ZIPCODE   2011
#define MC_USER_TIMEZONE  2012
#define MC_USER_MERCHCAT  2013
#define MC_USER_MERNAME   2014
#define MC_USER_MERCHLOC  2015
#define MC_USER_STATECODE 2016
#define MC_USER_PHONE     2017
#define MC_USER_SUB       2018
#define MC_USER_CARDTYPES 2019
#define MC_USER_MODE      2020
#define MC_USER_VNUMBER   2021
#define MC_USER_ROUTINGID 2022
#define MC_USER_PPROPERTY 2023
#define MC_USER_PID       2024
#define MC_USER_PIDPWD    2025
#define MC_USER_SMID      2026
#define MC_USER_SMIDPWD   2027
#define MC_USER_USDDIV    2028
#define MC_USER_AUDDIV    2029
#define MC_USER_DKKDIV    2030
#define MC_USER_GBPDIV    2031
#define MC_USER_HKDDIV    2032
#define MC_USER_JPYDIV    2033
#define MC_USER_NZDDIV    2034
#define MC_USER_NOKDIV    2035
#define MC_USER_SGDDIV    2036
#define MC_USER_ZARDIV    2037
#define MC_USER_SEKDIV    2038
#define MC_USER_CHFDIV    2039
#define MC_USER_CADDIV    2040
#define MC_USER_DIVNUM    2041

/* Card Types */
#define MC_CARD_VISA  2
#define MC_CARD_MC    4
#define MC_CARD_AMEX  8
#define MC_CARD_DISC  16
#define MC_CARD_JCB   32
#define MC_CARD_CB    64
#define MC_CARD_DC    128
#define MC_CARD_GIFT  256
#define MC_CARD_OTHER 512
#define MC_CARD_ALL   (MC_CARD_VISA | MC_CARD_MC | MC_CARD_AMEX | MC_CARD_DISC | MC_CARD_JCB | MC_CARD_CB | MC_CARD_DC | MC_CARD_GIFT | MC_CARD_OTHER)

/* Modes */
#define MC_MODE_AUTH   2
#define MC_MODE_SETTLE 4
#define MC_MODE_BOTH  (MC_MODE_AUTH | MC_MODE_SETTLE)
#define MC_MODE_ALL   MC_MODE_BOTH

/* Value definitions for Transaction Types */
#define MC_TRAN_SALE            1
#define MC_TRAN_REDEMPTION      MC_TRAN_SALE
#define MC_TRAN_PREAUTH         2
#define MC_TRAN_VOID            3
#define MC_TRAN_PREAUTHCOMPLETE 4
#define MC_TRAN_FORCE           5
#define MC_TRAN_OVERRIDE        6
#define MC_TRAN_RETURN          7
#define MC_TRAN_RELOAD          MC_TRAN_RETURN
#define MC_TRAN_CREDIT          MC_TRAN_RETURN
#define MC_TRAN_SETTLE          8
#define MC_TRAN_INCREMENTAL     9
#define MC_TRAN_REVERSAL        10
#define MC_TRAN_ACTIVATE        11
#define MC_TRAN_BALANCEINQ      12
#define MC_TRAN_CASHOUT         13
#define MC_TRAN_TOREVERSAL      14
#define MC_TRAN_SETTLERFR       15
#define MC_TRAN_ISSUE           16
#define MC_TRAN_TIP             17
#define MC_TRAN_MERCHRETURN     18
#define MC_TRAN_IVRREQ          19
#define MC_TRAN_IVRRESP         20
#define MC_TRAN_ADMIN           50
#define MC_TRAN_PING            100
#define MC_TRAN_CHKPWD          200

/* Engine Admin Transaction Types */
#define MC_TRAN_CHNGPWD       1001
#define MC_TRAN_LISTSTATS     1002
#define MC_TRAN_LISTUSERS     1003
#define MC_TRAN_GETUSERINFO   1004
#define MC_TRAN_ADDUSER       1005
#define MC_TRAN_EDITUSER      1006
#define MC_TRAN_DELUSER       1007
#define MC_TRAN_ENABLEUSER    1008
#define MC_TRAN_DISABLEUSER   1009
#define MC_TRAN_IMPORT        1010
#define MC_TRAN_EXPORT        1011
#define MC_TRAN_ERRORLOG      1012
#define MC_TRAN_CLEARERRORLOG 1013
#define MC_TRAN_GETSUBACCTS   1014

/* Value definitions for Admin Types */
#define MC_ADMIN_GUT           1
#define MC_ADMIN_GL            2
#define MC_ADMIN_GFT           3
#define MC_ADMIN_BT            4
#define MC_ADMIN_UB            MC_ADMIN_BT
#define MC_ADMIN_QC            5
#define MC_ADMIN_RS            6
#define MC_ADMIN_CTH           7
#define MC_ADMIN_CFH           8
#define MC_ADMIN_FORCESETTLE   9
#define MC_ADMIN_SETBATCHNUM   10
#define MC_ADMIN_RENUMBERBATCH 11
#define MC_ADMIN_FIELDEDIT     12
#define MC_ADMIN_CLOSEBATCH    13

#define M_SALE     MC_TRAN_SALE
#define M_PREAUTH  MC_TRAN_PREAUTH
#define M_FORCE    MC_TRAN_FORCE
#define M_OVERRIDE MC_TRAN_OVERRIDE
#define M_RETURN   MC_TRAN_RETURN
#define M_VOID     MC_TRAN_VOID
#define M_SETTLE   MC_TRAN_SETTLE

/* USER SETUP INFORMATION */
#define M_PROC         MC_USER_PROC
#define M_USER         MC_USER_USER
#define M_PWD          MC_USER_PWD
#define M_INDCODE      MC_USER_INDCODE
#define M_MERCHID      MC_USER_MERCHID
#define M_BANKID       MC_USER_BANKID
#define M_TERMID       MC_USER_TERMID
#define M_CLIENTNUM    MC_USER_CLIENTNUM
#define M_STOREID      MC_USER_STOREID
#define M_AGENTID      MC_USER_AGENTID
#define M_CHAINID      MC_USER_CHAINID
#define M_ZIPCODE      MC_USER_ZIPCODE
#define M_TIMEZONE     MC_USER_TIMEZONE
#define M_MERCHCAT     MC_USER_MERCHCAT
#define M_MERNAME      MC_USER_MERNAME
#define M_MERCHLOC     MC_USER_MERCHLOC
#define M_STATECODE    MC_USER_STATECODE
#define M_SERVICEPHONE MC_USER_PHONE


/* ---------- Deprecated ------------ */

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_ReturnCode(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT long LM_SPEC M_TransactionItem(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT long LM_SPEC M_TransactionBatch(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT M_int64 LM_SPEC M_TransactionID(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT const char *LM_SPEC M_TransactionAuth(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT const char *LM_SPEC M_TransactionText(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_TransactionAVS(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_TransactionCV(M_CONN *conn, M_uintptr identifier);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT const char *LM_SPEC M_TEXT_Code(int code);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT const char *LM_SPEC M_TEXT_AVS(int code);

/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT const char *LM_SPEC M_TEXT_CV(int code);

#ifndef LIBMONETRA_SPEC_STDCALL
/*
 *
 * \param[in,out] conn #M_CONN passed by reference, or allocated #M_CONN pointer
 * \param[in]
 * \return
 */
LM_EXPORT int LM_SPEC M_TransParam(M_CONN *conn, M_uintptr identifier, int key, ...);
#endif


/* MCVE compatibility */
#define MCVE_uint64 M_uint64
#define MCVE_int64  M_int64

#define MCVE_UNUSED  M_UNUSED
#define MCVE_NEW     M_NEW
#define MCVE_PENDING M_PENDING
#define MCVE_DONE    M_DONE

#define MCVE_ERROR   M_ERROR
#define MCVE_FAIL    M_FAIL
#define MCVE_SUCCESS M_SUCCESS
#define MCVE_AUTH    M_AUTH
#define MCVE_DENY    M_DENY
#define MCVE_CALL    M_CALL
#define MCVE_DUPL    M_DUPL
#define MCVE_PKUP    M_PKUP
#define MCVE_RETRY   M_RETRY
#define MCVE_SETUP   M_SETUP
#define MCVE_TIMEOUT M_TIMEOUT

#define MCVE_GOOD    M_GOOD
#define MCVE_BAD     M_BAD
#define MCVE_STREET  M_STREET
#define MCVE_ZIP     M_ZIP
#define MCVE_UNKNOWN M_UNKNOWN

#define MCVE_CONN M_CONN

#define MCVE_InitEngine(a)         M_InitEngine(a)
#define MCVE_DestroyEngine         M_DestroyEngine
#define MCVE_InitConn(a)           M_InitConn(a)
#define MCVE_SetBlocking(a,b)      M_SetBlocking(a,b)
#define MCVE_SetTimeout(a,b)       M_SetTimeout(a,b)
#define MCVE_SetDropFile(a,b)      M_SetDropFile(a,b)
#define MCVE_SetIP(a,b,c)          M_SetIP(a,b,c)
#define MCVE_SetSSL(a,b,c)         M_SetSSL(a,b,c)
#define MCVE_SetSSL_Files(a,b,c)   M_SetSSL_Files(a,b,c)
#define MCVE_SetSSL_CAfile(a,b)    M_SetSSL_CAfile(a,b)
#define MCVE_VerifyConnection(a,b) M_VerifyConnection(a,b)
#define MCVE_VerifySSLCert(a,b)    M_VerifySSLCert(a,b)
#define MCVE_Connect(a)            M_Connect(a)
#define MCVE_MaxConnTimeout(a,b)   M_MaxConnTimeout(a,b)
#define MCVE_ConnectionError(a)    M_ConnectionError(a)
#define MCVE_DestroyConn(a)        M_DestroyConn(a)
#define MCVE_Monitor(a)            M_Monitor(a)
#define MCVE_TransactionsSent(a)   M_TransactionsSent(a)
#define MCVE_Ping(a)               M_Ping(a)
#define MCVE_DeleteResponse(a, b)  M_DeleteTrans(a, b)
#define MCVE_DeleteTrans(a,b)      M_DeleteTrans(a,b)
#define MCVE_TransNew(a)           M_TransNew(a)
#define MCVE_TransParam            M_TransParam
#define MCVE_TransSend(a,b)        M_TransSend(a,b)
#define MCVE_ResponseParam(a,b,c)  M_ResponseParam(a,b,c)
#define MCVE_ResponseKeys(a,b,c)   M_ResponseKeys(a,b,c)
#define MCVE_FreeResponseKeys(a,b) M_FreeResponseKeys(a,b)

#define MCVE_Sale              M_Sale
#define MCVE_PreAuth           M_PreAuth
#define MCVE_Void              M_Void
#define MCVE_PreAuthCompletion M_PreAuthCompletion
#define MCVE_Force             M_Force
#define MCVE_Return            M_Return
#define MCVE_Override          M_Override
#define MCVE_Settle            M_Settle

#define MCVE_ReturnStatus(a,b)           M_ReturnStatus(a,b)
#define MCVE_ReturnCode(a,b)             M_ReturnCode(a,b)
#define MCVE_TransactionItem(a,b)        M_TransactionItem(a,b)
#define MCVE_TransactionBatch(a,b)       M_TransactionBatch(a,b)
#define MCVE_TransactionID(a,b)          M_TransactionID(a,b)
#define MCVE_TransactionAuth(a,b)        M_TransactionAuth(a,b)
#define MCVE_TransactionText(a,b)        M_TransactionText(a,b)
#define MCVE_TransactionAVS(a,b)         M_TransactionAVS(a,b)
#define MCVE_TransactionCV(a,b)          M_TransactionCV(a,b)
#define MCVE_TransInQueue(a)             M_TransInQueue(a)
#define MCVE_CheckStatus(a,b)            M_CheckStatus(a,b)
#define MCVE_CompleteAuthorizations(a,b) M_CompleteAuthorizations(a,b)
#define MCVE_Gut                         M_Gut
#define MCVE_Gl                          M_Gl
#define MCVE_Gft                         M_Gft
#define MCVE_Rs                          M_Rs
#define MCVE_Qc                          M_Qc
#define MCVE_Chkpwd                      M_Chkpwd
#define MCVE_Bt                          M_Bt
#define MCVE_Ub(a,b,c)                   M_Bt(a,b,c)

#define MCVE_Chngpwd         M_Chngpwd
#define MCVE_ListUsers       M_ListUsers
#define MCVE_EnableUser      M_EnableUser
#define MCVE_DisableUser     M_DisableUser
#define MCVE_GetUserInfo     M_GetUserInfo
#define MCVE_DelUser         M_DelUser
#define MCVE_ListStats       M_ListStats
#define MCVE_Export          M_Export
#define MCVE_Import          M_Import
#define MCVE_InitUserSetup   M_InitUserSetup
#define MCVE_AddUserArg      M_AddUserArg
#define MCVE_DeleteUserSetup M_DeleteUserSetup
#define MCVE_AddUser         M_AddUser
#define MCVE_GetUserArg      M_GetUserArg
#define MCVE_EditUser        M_EditUser
#define MCVE_GetUserParam    M_GetUserParam


#define MCVE_IsCommaDelimited(a,b)    M_IsCommaDelimited(a,b)
#define MCVE_ParseCommaDelimited(a,b) M_ParseCommaDelimited(a,b)
#define MCVE_GetCommaDelimited(a,b)   M_GetCommaDelimited(a,b)
#define MCVE_GetCell(a,b,c,d)         M_GetCell(a,b,c,d)
#define MCVE_GetCellByNum(a,b,c,d)    M_GetCellByNum(a,b,c,d)
#define MCVE_NumColumns(a,b)          M_NumColumns(a,b)
#define MCVE_NumRows(a,b)             M_NumRows(a,b)
#define MCVE_GetHeader(a,b,c)         M_GetHeader(a,b,c)

#define MCVE_SALE         MC_TRAN_SALE
#define MCVE_PREAUTH      MC_TRAN_PREAUTH
#define MCVE_FORCE        MC_TRAN_FORCE
#define MCVE_OVERRIDE     MC_TRAN_OVERRIDE
#define MCVE_RETURN       MC_TRAN_RETURN
#define MCVE_VOID         MC_TRAN_VOID
#define MCVE_SETTLE       MC_TRAN_SETTLE
#define MCVE_PROC         MC_USER_PROC
#define MCVE_USER         MC_USER_USER
#define MCVE_PWD          MC_USER_PWD
#define MCVE_INDCODE      MC_USER_INDCODE
#define MCVE_MERCHID      MC_USER_MERCHID
#define MCVE_BANKID       MC_USER_BANKID
#define MCVE_TERMID       MC_USER_TERMID
#define MCVE_CLIENTNUM    MC_USER_CLIENTNUM
#define MCVE_STOREID      MC_USER_STOREID
#define MCVE_AGENTID      MC_USER_AGENTID
#define MCVE_CHAINID      MC_USER_CHAINID
#define MCVE_ZIPCODE      MC_USER_ZIPCODE
#define MCVE_TIMEZONE     MC_USER_TIMEZONE
#define MCVE_MERCHCAT     MC_USER_MERCHCAT
#define MCVE_MERNAME      MC_USER_MERNAME
#define MCVE_MERCHLOC     MC_USER_MERCHLOC
#define MCVE_STATECODE    MC_USER_STATECODE
#define MCVE_SERVICEPHONE MC_USER_PHONE

#define MCVE_uwait(a)        M_uwait(a)
#define MCVE_TEXT_Code(a)    M_TEXT_Code(a)
#define MCVE_TEXT_AVS(a)     M_TEXT_AVS(a)
#define MCVE_TEXT_CV(a)      M_TEXT_CV(a)

#define MCVE_UserSetup       M_UserSetup

/*! @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBMONETRA_DEPRECATED_H__ */
