AM_CFLAGS   = @CHECK_CPPFLAGS@ @MSTDLIB_CPPFLAGS@ -I$(top_srcdir)/include
AM_CPPFLAGS = $(AM_CFLAGS)
AM_LDFLAGS  = @CHECK_LDFLAGS@ @MSTDLIB_LDFLAGS@
AM_CFLAGS  += -DTEST
LDADD       = $(top_builddir)/src/libmonetra.la @CHECK_LIBADD@ @MSTDLIB_LIBADD@

######################################################################

TESTS_ENVIRONMENT = $(VALGRIND_ENVIRONMENT)

######################################################################

TESTS = \
    check_monetra \
    check_monetra_legacy

######################################################################

all = check

check_PROGRAMS = $(TESTS)

