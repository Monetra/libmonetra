AC_INIT([libmonetra],[8.0.0],[support@monetra.com])
AC_PREREQ(2.60)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/monetra_conn.c])
dnl AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects 1.9.6])
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_PROG_INSTALL
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([aix-soname=svr4])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile test/Makefile Doxyfile])

m4_include([m4/ax_append_flag.m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_append_compile_flags.m4])

dnl Doxygen
m4_include([m4/ax_prog_doxygen.m4])
DX_INIT_DOXYGEN([libmonetra], Doxyfile, doc/api)
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_HTML_FEATURE(ON)

dnl tests
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(CHECK, [check >= 0.9.4],,[AC_MSG_RESULT([no, testing is disabled])])
AM_CONDITIONAL(HAVE_CHECK, [test "x$CHECK_LIBS" != "x"])
AC_SUBST(CHECK_CFLAGS)
AC_SUBST(CHECK_LIBS)

dnl VERSIONING
dnl  On new release, edit m_version.h
dnl   LIBMONETRA_MICRO_VERSION += 1;
dnl   LIBMONETRA_INTERFACE_VERSION += 1;
dnl   LIBMONETRA_BINARY_AGE += 1;
dnl  if new functions were added
dnl   LIBMONETRA_INTERFACE_AGE=0
dnl  if backward compat is broken
dnl   LIBMONETRA_BINARY_AGE=0
dnl   LIBMONETRA_INTERFACE_AGE=0
LIBMONETRA_MAJOR_VERSION=8
LIBMONETRA_MINOR_VERSION=0
LIBMONETRA_MICRO_VERSION=0
LIBMONETRA_INTERFACE_AGE=0
LIBMONETRA_BINARY_AGE=0
LIBMONETRA_VERSION=$LIBMONETRA_MAJOR_VERSION:$LIBMONETRA_MINOR_VERSION:$LIBMONETRA_MICRO_VERSION

AC_SUBST(LIBMONETRA_MAJOR_VERSION)
AC_SUBST(LIBMONETRA_MINOR_VERSION)
AC_SUBST(LIBMONETRA_MICRO_VERSION)
AC_SUBST(LIBMONETRA_INTERFACE_AGE)
AC_SUBST(LIBMONETRA_BINARY_AGE)
AC_SUBST(LIBMONETRA_VERSION)

AC_C_CONST
AC_HEADER_STDC
AC_SYS_LARGEFILE

dnl ----- Check for Position Independent Executable flags -----
AC_MSG_CHECKING([whether $CC accepts PIE flags])
save_CFLAGS=$CFLAGS
save_LDFLAGS=$LDFLAGS
CFLAGS="$CFLAGS -fPIE"
LDFLAGS="$LDFLAGS -pie"
AC_TRY_RUN([static int foo[30000]; int main () { return 0; }],
           [pie_supported=yes], [pie_supported=no], [pie_supported=no])
AC_MSG_RESULT($pie_supported)
if test "$pie_supported" = "no" -o "$use_hardening" = "no" ; then
  CFLAGS=$save_CFLAGS
  LDFLAGS=$save_LDFLAGS
fi

dnl ----- Check for RELRO linker flags -----
AC_MSG_CHECKING([whether $LD accepts RELRO flags])
save_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -Wl,-z,relro -Wl,-z,now"
AC_TRY_RUN([static int foo[30000]; int main () { return 0; }],
           [relro_supported=yes], [relro_supported=no], [relro_supported=no])
AC_MSG_RESULT($relro_supported)
if test "$relro_supported" = "no" -o "$use_hardening" = "no"; then
  LDFLAGS=$save_LDFLAGS
fi

dnl ----- Check for noexecstack linker flags -----
AC_MSG_CHECKING([whether $LD accepts noexecstack flags])
save_LDFLAGS=$LDFLAGS
LDFLAGS="$LDFLAGS -Wl,-z,noexecstack"
AC_TRY_RUN([static int foo[30000]; int main () { return 0; }],
           [noexecstack_supported=yes], [noexecstack_supported=no], [noexecstac_supported=no])
AC_MSG_RESULT($noexecstack_supported)
if test "$noexecstack_supported" = "no" -o "$use_hardening" = "no"; then
  LDFLAGS=$save_LDFLAGS
fi

dnl ----- Check for visibility flags -----
AC_MSG_CHECKING([for visibility declarations])
save_CFLAGS="$CFLAGS"
CFLAGS="-fvisibility=hidden -Werror"
AC_TRY_COMPILE([extern __attribute__((__visibility__("hidden"))) int hiddenvar;
                extern __attribute__((__visibility__("default"))) int exportedvar;
                extern __attribute__((__visibility__("hidden"))) int hiddenfunc (void);
                extern __attribute__((__visibility__("default"))) int exportedfunc (void);],
               [],
               visibility_supported=yes,
               visibility_supported=no)
CFLAGS=$save_CFLAGS
AC_MSG_RESULT(${visibility_supported})
if test "$visibility_supported" = "yes" ; then
        CFLAGS="$CFLAGS -fvisibility=hidden"
fi


AX_APPEND_COMPILE_FLAGS([\
	-W -Wall -Wextra -Wchar-subscripts -Wcomment -Wno-coverage-mismatch \
	-Wdouble-promotion -Wformat -Wnonnull -Winit-self -Wimplicit-int \
	-Wimplicit-function-declaration -Wimplicit -Wignored-qualifiers -Wmain \
	-Wmissing-braces -Wmissing-include-dirs -Wparentheses -Wsequence-point \
	-Wreturn-type -Wswitch -Wtrigraphs -Wunused-but-set-parameter \
	-Wunused-but-set-variable -Wunused-function -Wunused-label \
	-Wunused-local-typedefs -Wunused-parameter -Wunused-variable -Wunused-value \
	-Wunused -Wuninitialized -Wmaybe-uninitialized -Wunknown-pragmas \
	-Wmissing-format-attribute -Warray-bounds \
	-Wtrampolines -Wfloat-equal \
	-Wdeclaration-after-statement -Wundef -Wshadow -Wunsafe-loop-optimizations \
	-Wpointer-arith -Wtype-limits -Wbad-function-cast -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wclobbered -Wempty-body \
	-Wenum-compare -Wjump-misses-init -Wsign-compare -Wsizeof-pointer-memaccess \
	-Waddress -Wlogical-op -Waggregate-return \
	-Wstrict-prototypes -Wold-style-declaration -Wold-style-definition \
	-Wmissing-parameter-type -Wmissing-prototypes -Wmissing-declarations \
	-Wmissing-field-initializers -Woverride-init -Wpacked -Wredundant-decls \
	-Wnested-externs -Winline -Winvalid-pch -Wvariadic-macros -Wvarargs \
	-Wvector-operation-performance -Wvla -Wpointer-sign \
	-Wdisabled-optimization -Wendif-labels -Wpacked-bitfield-compat \
	-Wformat-security -Woverlength-strings -Wsign-conversion -Wstrict-aliasing \
	-Wstrict-overflow -Wsync-nand -Wvolatile-register-var -Wconversion \
], [CFLAGS], [-Werror])


AC_ARG_ENABLE(valgrind-testing,
	AC_HELP_STRING([--enable-valgrind-testing],
       [enable running of tests inside Valgrind (disabled by default)]),
	[ if test "$enable_valgrind_testing" = "yes"; then
		VALGRIND_DIR="$PWD/valgrind"
		AC_CHECK_PROG(CHECK_VALGRIND, valgrind, yes, no)
		VALGRIND_ENVIRONMENT="$VALGRIND_ENVIRONMENT valgrind -q"
		VALGRIND_ENVIRONMENT+=" --memcheck:leak-check=full"
		VALGRIND_ENVIRONMENT+=" --show-reachable=yes"
		VALGRIND_ENVIRONMENT+=" --num-callers=100"
		VALGRIND_ENVIRONMENT+=" --track-fds=no"
		VALGRIND_ENVIRONMENT+=" --error-exitcode=1"
		VALGRIND_ENVIRONMENT+=" --suppressions=$VALGRIND_DIR/check.suppressions"
		AC_SUBST(VALGRIND_ENVIRONMENT)
	fi
	]
)

dnl ----------- MSTDLIB -------------
dnl   Need to add search path criteria before running these checks, along with --with-mstdlib flags
AC_CHECK_HEADER([mstdlib/mstdlib.h], [ ], [ AC_MSG_ERROR([unable to find mstdlib/mstdlib.h]) ], [ ])
AC_CHECK_HEADER([mstdlib/mstdlib_thread.h], [ ], [ AC_MSG_ERROR([unable to find mstdlib/mstdlib_thread.h]) ], [ ])
AC_CHECK_HEADER([mstdlib/mstdlib_io.h], [ ], [ AC_MSG_ERROR([unable to find mstdlib/mstdlib_io.h]) ], [ ])
AC_CHECK_HEADER([mstdlib/mstdlib_tls.h], [ ], [ AC_MSG_ERROR([unable to find mstdlib/mstdlib_tls.h]) ], [ ])

AC_ARG_ENABLE(internal-build, [ Disables checks for precompiled dependencies like mstdlib ], [ INTERNAL_BUILD=yes ], [ INTERNAL_BUILD=no ])

if test "${INTERNAL_BUILD}" = "no" ; then
	AC_SEARCH_LIBS([M_buf_create], [mstdlib], [ ], [ AC_MSG_ERROR([unable to find mstdlib]) ], [ ])
	AC_SEARCH_LIBS([M_thread_create], [mstdlib_thread], [ ], [ AC_MSG_ERROR([unable to find mstdlib_thread]) ], [ -lmstdlib ])
	AC_SEARCH_LIBS([M_event_create], [mstdlib_io], [ ], [ AC_MSG_ERROR([unable to find mstdlib_io]) ], [ -lmstdlib -lmstdlib_thread ])
	AC_SEARCH_LIBS([M_tls_clientctx_create], [mstdlib_tls], [ ], [ AC_MSG_ERROR([unable to find mstdlib_tls]) ], [ -lmstdlib -lmstdlib_thread -lmstdlib_io ])
else
	dnl Expects -I and -L flags to have been set appropriately by chain building
	LIBS="${LIBS} -lmstdlib -lmstdlib_thread -lmstdlib_io -lmstdlib_tls"
fi

AC_DEFINE([LIBMONETRA_INTERNAL], 1, [Let config know we are internal])

AC_OUTPUT


