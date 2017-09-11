#include <stdlib.h>
#include <check.h>

#include <monetra_api.h>

static void trans_callback(LM_conn_t *conn, M_event_t *event, LM_event_type_t type, LM_trans_t *trans)
{
	switch (type) {
		case LM_EVENT_CONN_CONNECTED:
			M_printf("Connected successfully\n");
			break;
		case LM_EVENT_CONN_DISCONNECT:
		case LM_EVENT_CONN_ERROR:
			M_printf("%s received: %s\n", (type == LM_EVENT_CONN_DISCONNECT)?"Disconnect":"Connection Error", LM_conn_error(conn));
			LM_conn_destroy(conn);
			M_event_done(event);
			break;
		case LM_EVENT_TRANS_DONE:
			M_printf("Transaction %p complete:\n", trans);
			if (LM_trans_response_type(trans) == LM_TRANS_RESPONSE_KV) {
				M_list_str_t *params = LM_trans_response_keys(trans);
				size_t        i;
				for (i=0; i<M_list_str_len(params); i++) {
					const char *key = M_list_str_at(params, i);
					M_printf("\t'%s' = '%s'\n", key, LM_trans_response_param(trans, key));
				}
				M_list_str_destroy(params);
			} else {
				const M_csv_t *csv = LM_trans_response_csv(trans);
				size_t         i;
				M_printf("\t%zu rows, %zu cols\n", M_csv_get_numrows(csv), M_csv_get_numcols(csv));
				/* Print headers */
				for (i=0; i<M_csv_get_numcols(csv); i++) {
					if (i != 0)
						M_printf("|");
					M_printf("%s", M_csv_get_header(csv, i));
				}
				M_printf("\n");

				/* Print data */
				for (i=0; i<M_csv_get_numrows(csv); i++) {
					size_t j;
					for (j=0; j<M_csv_get_numcols(csv); j++) {
						if (j != 0)
							M_printf("|");
						M_printf("%s", M_csv_get_cellbynum(csv, i, j));
					}
					M_printf("\n");
				}

			}
			LM_trans_delete(trans);
			// We're only running a single transaction, issue a graceful disconnect.
			// Actually, wait, we set a 5s idle timeout, lets do that instead :)
			//M_printf("Initiating disconnect\n");
			//LM_conn_disconnect(conn);
			break;
		case LM_EVENT_TRANS_ERROR:
		case LM_EVENT_TRANS_NOCONNECT:
			M_printf("Transaction %p error (connectivity): %s\n", trans, LM_conn_error(conn));
			LM_trans_delete(trans);
			// We should still get an LM_EVENT_CONN_ERROR after this event for additional cleanup. 
			break;
	}
}

static M_bool check_monetra_test(void)
{
	M_event_t *event    = M_event_create(M_EVENT_FLAG_NONE);
	LM_conn_t *conn     = LM_conn_init(event, trans_callback, "testbox.monetra.com", 8665);
	LM_trans_t *trans;

	/* Set 2s idle timeout.  This is how we want to exit the script to verify this works */
	LM_conn_set_idle_timeout(conn, 2);

	/* NOT required, implicitly done by LM_trans_send() if not connected! */
	//LM_conn_connect(conn);

	trans = LM_trans_new(conn);
	LM_trans_set_param(trans, "username", "test_retail:public");
	LM_trans_set_param(trans, "password", "publ1ct3st");
	LM_trans_set_param(trans, "action", "sale");
	LM_trans_set_param(trans, "account", "4012888888881881");
	LM_trans_set_param(trans, "expdate", "1220");
	LM_trans_set_param(trans, "zip", "32606");
	LM_trans_set_param(trans, "cv", "999");
	LM_trans_set_param(trans, "amount", "12.00");
	LM_trans_send(trans);
	M_printf("Enqueued transaction %p\n", trans);

	M_event_loop(event, M_TIMEOUT_INF);

	// NOTE: we cleaned up 'conn' within the trans_callback.  We can't have
	//       exited the event loop otherwise.
	conn = NULL;

	M_event_destroy(event);

	M_library_cleanup();

	return M_TRUE;
}

static M_bool check_monetra_report_test(void)
{
	M_event_t *event    = M_event_create(M_EVENT_FLAG_NONE);
	LM_conn_t *conn     = LM_conn_init(event, trans_callback, "testbox.monetra.com", 8665);
	LM_trans_t *trans;

	/* Set 2s idle timeout.  This is how we want to exit the script to verify this works */
	LM_conn_set_idle_timeout(conn, 2);

	/* NOT required, implicitly done by LM_trans_send() if not connected! */
	//LM_conn_connect(conn);

	trans = LM_trans_new(conn);
	LM_trans_set_param(trans, "username", "test_retail:public");
	LM_trans_set_param(trans, "password", "publ1ct3st");
	LM_trans_set_param(trans, "action", "admin");
	LM_trans_set_param(trans, "admin", "gut");
	LM_trans_send(trans);
	M_printf("Enqueued transaction %p\n", trans);

	M_event_loop(event, M_TIMEOUT_INF);

	// NOTE: we cleaned up 'conn' within the trans_callback.  We can't have
	//       exited the event loop otherwise.
	conn = NULL;

	M_event_destroy(event);

	M_library_cleanup();

	return M_TRUE;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

START_TEST(check_monetra)
{
	check_monetra_test();
	check_monetra_report_test();
}
END_TEST

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static Suite *monetra_suite(void)
{
	Suite *suite;
	TCase *tc;

	suite = suite_create("monetra");

	tc = tcase_create("monetra");
	tcase_add_test(tc, check_monetra);
	tcase_set_timeout(tc, 60);
	suite_add_tcase(suite, tc);

	return suite;
}

int main(int argc, char **argv)
{
	SRunner *sr;
	int      nf;

	(void)argc;
	(void)argv;

	sr = srunner_create(monetra_suite());
	srunner_set_log(sr, "check_monetra.log");

	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);

	return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
