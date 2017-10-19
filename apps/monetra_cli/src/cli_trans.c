#include "monetra_cli.h"

cli_trans_t *cli_trans_create(cli_opts_t *opts, const char **fail)
{
	cli_trans_t             *trans;
	M_hash_dict_t           *kvs;
	M_rand_t                *rand;
	char                     id[32];
	M_uint64                 ramount;
	struct M_list_callbacks  lcbs;
	size_t                   num;
	size_t                   len;
	size_t                   i;
	size_t                   j;

	M_mem_set(&lcbs, 0, sizeof(lcbs));

	/* Verify there's really a transaction that can be sent. */
	if (M_hash_dict_num_keys(opts->kvs) == 0) {
		*fail = "An ACTION and/or kvs must be specified";
		return NULL;
	}

	trans = M_malloc_zero(sizeof(*trans));

	/* Copy the host and port from the command line options. If the port was not explicit set switch
 	 * to the admin port if a madmin user was specified. */
	trans->host = M_strdup(opts->host);
	trans->port = opts->port;
	if (!opts->port_set && (M_str_caseeq(opts->username, "madmin") || M_str_caseeq_max(opts->username, "madmin:", 7)))
		trans->port = DEFAULT_PORT_ADMIN;

	/* If a cert restriction is in use check that we have both a cert and key. */
	if ((M_str_isempty(opts->keyfile) && !M_str_isempty(opts->certfile)) ||
		(!M_str_isempty(opts->keyfile) && M_str_isempty(opts->certfile)))
	{
		*fail = "Key and cert paths must be specified together";
		cli_trans_destroy(trans);
		return NULL;
	}

	if (!M_str_isempty(opts->keyfile)) {
		trans->keyfile  = M_strdup(opts->keyfile);
		trans->certfile = M_strdup(opts->certfile);
	}

	if (!M_str_isempty(opts->cadir))
		trans->cadir = M_strdup(opts->cadir);

	trans->certvalidation = opts->certvalidation;

	/* Create the list that we'll put the KVS transactions into. */
	lcbs.value_free = (M_list_free_func)M_hash_dict_destroy;
	trans->kvs      = M_list_create(&lcbs, M_LIST_NONE);

	trans->send_serial = opts->send_serial;

	/* Create the rand object outside of the loop because we want to reuse it
 	 * if we're selecting random amounts. It's not a big deal if we create it
	 * when we're not using a random amount. We could put a check in here to
	 * see if we're using random amounts but it's not really a big deal. */
	rand = M_rand_create(0);

	/* Determine how may KVS transactions we're going to run. We need to run
 	 * at least one. */
	num  = opts->dup;
	if (opts->dup == 0)
		num = 1;

	for (i=0; i<num; i++) {
		kvs = M_hash_dict_create(8, 75, M_HASH_DICT_CASECMP|M_HASH_DICT_KEYS_ORDERED|M_HASH_DICT_KEYS_SORTASC);

		/* Fill in the username and password. Since each transaction KVS set needs this
		 * we're going to fill it here and not pass the username and password along. */
		if (!M_str_isempty(opts->username))
			M_hash_dict_insert(kvs, "username", opts->username);
		if (!M_str_isempty(opts->password))
			M_hash_dict_insert(kvs, "password", opts->password);

		/* Add in the KVS that was parsed from the command line. The command line parser
 		 * already merged the manual and action KVS into ->kvs. */
		M_hash_dict_merge(&kvs, M_hash_dict_duplicate(opts->kvs));

		/* Remove KVS that we don't want sent. */
		len = M_list_str_len(opts->remove_keys);
		for (j=0; j<len; j++) {
			M_hash_dict_remove(kvs, M_list_str_at(opts->remove_keys, j));
		}

		/* If a random amount is used we want to add it to the KVS. Overriding an amount if
 		 * it was already specified. It is possible that random amount is specified with
		 * a non-amount using action but the user could specify any number of KVS that doesn't
		 * apply manually. So if they want a random amount we'll give them a random amount. */
		if (opts->min_amount != 0 && opts->max_amount != 0) {
			/* Ensure the random amount is never a slow trigger value. */
			do {
				ramount = M_rand_range(rand, opts->min_amount, opts->max_amount);
			} while (ramount >= 800 && ramount <= 899);
			
			M_snprintf(id, sizeof(id), "%llu.%02llu", ramount/100, ramount%100);
			M_hash_dict_insert(kvs, "amount", id);
		}

		/* The identifier is set last because it is internal to this app and
		 * we don't want someone sending it in as a KV from overriding it. */
		M_snprintf(id, sizeof(id), "%zu", i+1);
		M_hash_dict_insert(kvs, "identifier", id);

		M_list_insert(trans->kvs, kvs);
	}

	M_rand_destroy(rand);
	return trans;
}

void cli_trans_destroy(cli_trans_t *trans)
{
	if (trans == NULL)
		return;

	M_free(trans->host);
	M_free(trans->keyfile);
	M_free(trans->certfile);
	M_free(trans->cadir);

	M_list_destroy(trans->kvs, M_TRUE);

	M_free(trans);
}
