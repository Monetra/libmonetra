#ifndef __MONETRA_CLI_H__
#define __MONETRA_CLI_H__

#include <mstdlib/mstdlib.h>
#include <mstdlib/mstdlib_thread.h>
#include <mstdlib/mstdlib_io.h>
#include <mstdlib/mstdlib_tls.h>
#include <monetra_api.h>

extern const char *DEFAULT_USERNAME;
extern const char *DEFAULT_PASSWORD;
extern const char *DEFAULT_HOST;
extern M_uint16    DEFAULT_PORT_USER;
extern M_uint16    DEFAULT_PORT_ADMIN;

/*! A comment line transaction with connectivity and KVS
 * information. */
typedef struct {
	char                 *host;            /*!< Monetra hose */
	M_uint16              port;            /*!< Monetra port. */

	char                 *keyfile;         /*!< Restriction key file (path). */
	char                 *certfile;        /*!< Restriction cert file (path). */

	char                 *cadir;           /*!< Certificate CA verification directory. Used to verify the
	                                            connection to Monetra. */
	M_tls_verify_level_t  certvalidation;  /*!< Level of valiation that should be performed on the
	                                            connection to Monetra */

	M_list_t             *kvs;             /*!< List of M_hash_dict_t KVS that are to be sent. */
	M_bool                send_serial;     /*!< Should the KVS be sent serially. If false the KVS transactions will
	                                            be sent in parallel. */

	size_t                outstanding_cnt; /*!< Numer of KVS transactions that have not been queued. Used to track
	                                            allow the last and only the last transaction to finish to issue
	                                            a disconnect. */

	M_bool                enable_trace;    /*!< Enable I/O tracing. */
} cli_trans_t;


/*! Parsed and collected command line arguments. These will be used to
 * create a cli_trans_t */
typedef struct {
	char                 *username;       /*!< Username. */
	char                 *password;       /*!< Password. */

	char                 *host;           /*!< Monetra host. */
	M_uint16              port;           /*!< Monetra port. */

	char                 *keyfile;        /*!< Restriction key file (path). */
	char                 *certfile;       /*!< Restriction cert file (path). */
	char                 *cadir;          /*!< Certificate CA verification directory. Used to verify the
	                                           connection to Monetra. */
	M_tls_verify_level_t  certvalidation; /*!< Certificate CA verification directory. Used to verify the
	                                            connection to Monetra */

	char                 *action_kvs;     /*!< KVS from an action if one was specified. */
	M_bool                send_pinksn;    /*!< Send a generic pin and ksn with kvs. */
	M_hash_dict_t        *card;           /*!< Test card kvs to use. Overrides kvs and actions. */
	M_hash_dict_t        *man_kvs;        /*!< KVS specified manually. */
	M_list_str_t         *remove_keys;    /*!< KVS keys that should not be sent. */
	M_hash_dict_t        *kvs;            /*!< KVS merged from all input. */

	M_uint64              dup;            /*!< The number of times a transaction will be run. Value is number of time
	                                           run. 1 = one time. 2 = two times. */
	M_uint64              min_amount;     /*!< The minimum amount that a random amount can use. */
	M_uint64              max_amount;     /*!< The minimum amount that a random amount can use. Use of random amounts
	                                           key off of the max_amount and min_amount being 0 or set to something. */

	M_bool                send_serial;    /*!< Should the KVS be sent serially. If false the KVS transactions will
	                                           be sent in parallel. */

	M_bool                help;           /*!< Was help explicitly requested. This will supress any errors with passed
	                                           command line arguments. */
	M_bool                port_set;       /*!< Was the port explicitly set. This will disable auto port (user vs admin)
	                                           selection. */

	M_bool                enable_trace;   /*!< Enable I/O tracing. */
} cli_opts_t;

cli_opts_t *cli_opts_create(void);
void cli_opts_destroy(cli_opts_t *opts);
cli_trans_t *cli_parse_args(int argc, const char *const *argv);

cli_trans_t *cli_trans_create(cli_opts_t *opts, const char **fail);
void cli_trans_destroy(cli_trans_t *tran);

#endif /* __MONETRA_CLI_H__ */
