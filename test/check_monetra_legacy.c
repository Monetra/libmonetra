#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <monetra.h>

static int check_monetra_legacy_test(void)
{
	const char    *host = "test.transafe.com";
	unsigned short port = 443;
	M_CONN         conn;
	M_uintptr      identifier;

	M_InitEngine(NULL);
	M_InitConn(&conn);
	M_SetSSL(&conn, host, port);
	M_SetBlocking(&conn, 1);
	printf("Connecting to %s:%d...\r\n", host, (int)port);

	/* Implicit now!

	if (!M_Connect(&conn)) {
		M_printf("Connection Failed: %s\r\n", M_ConnectionError(&conn));
		return M_FALSE;
	}

	M_printf("Connected\r\n");
	*/
	identifier=M_TransNew(&conn);
	M_TransKeyVal(&conn, identifier, "username", "test_ecomm:public");
	M_TransKeyVal(&conn, identifier, "password", "publ1ct3st");
	M_TransKeyVal(&conn, identifier, "action",   "sale");
	M_TransKeyVal(&conn, identifier, "account",  "4012888888881881");
	M_TransKeyVal(&conn, identifier, "expdate",  "0549");
	M_TransKeyVal(&conn, identifier, "amount",   "12.00");
	M_TransKeyVal(&conn, identifier, "zip",      "32606");

	printf("Sending Transaction\r\n");
	ck_assert_msg(M_TransSend(&conn, identifier), "Could not send trans: %s\r\n", M_ConnectionError(&conn));

	printf("Transaction response received\r\n");
	if (M_ReturnStatus(&conn, identifier) == M_SUCCESS) {
		printf("Transaction Authorized: %s -- %s\r\n", M_ResponseParam(&conn, identifier, "code"),
		M_ResponseParam(&conn, identifier, "verbiage"));
	} else {
		printf("Transaction Denied: %s -- %s\r\n", M_ResponseParam(&conn, identifier, "code"),
		M_ResponseParam(&conn, identifier, "verbiage"));
	}
	M_DestroyConn(&conn);
	M_DestroyEngine();

	return 1;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

START_TEST(check_monetra_legacy)
{
	check_monetra_legacy_test();
}
END_TEST

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static Suite *monetra_legacy_suite(void)
{
	Suite *suite;
	TCase *tc;

	suite = suite_create("monetra_legacy");

	tc = tcase_create("monetra_legacy");
	tcase_add_test(tc, check_monetra_legacy);
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

	sr = srunner_create(monetra_legacy_suite());
	srunner_set_log(sr, "check_monetra_legacy.log");

	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);

	return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
