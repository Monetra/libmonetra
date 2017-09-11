lib_LTLIBRARIES = libmonetra.la

AM_CFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS = -I$(top_srcdir)/include

libmonetra_la_LDFLAGS = -export-dynamic -version-info @LIBMONETRA_VERSION@
libmonetra_la_SOURCES =          \
	monetra_conn.c           \
	monetra_conn_event.c     \
	monetra_init.c           \
	monetra_deprecated.c     \
	monetra_legacy.c         \
	monetra_parse.c          \
	monetra_trans.c          \
	monetra_trans_structure.c 
