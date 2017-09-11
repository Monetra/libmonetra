if HAVE_CHECK

AM_CFLAGS   = @CHECK_CFLAGS@ -I$(top_srcdir)/include
AM_CPPFLAGS = $(AM_CFLAGS)
AM_LDFLAGS  = @CHECK_LIBS@
AM_CFLAGS  += -DTEST
LDADD       = $(top_builddir)/src/libmonetra.la

######################################################################

TESTS_ENVIRONMENT = $(VALGRIND_ENVIRONMENT)

######################################################################

TESTS = \
    check_monetra \
    check_monetra_legacy

######################################################################

all = check

check_PROGRAMS = $(TESTS)

endif
