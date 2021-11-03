m4_define([libmonetra_major_version],   [8])
m4_define([libmonetra_minor_version],   [0])
m4_define([libmonetra_release_version], [0])
m4_define([libmonetra_version], [libmonetra_major_version.libmonetra_minor_version.libmonetra_release_version])

AC_INIT([libmonetra],[libmonetra_version],[support@monetra.com])
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

dnl Setting this prevents showing the full compile/link command line.
dnl Use --disable-silent-rules  or  make V=1  to re-enable verbosity.
AM_SILENT_RULES([yes])
AC_C_CONST
AC_HEADER_STDC
AC_SYS_LARGEFILE

m4_include([m4/ax_append_flag.m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_append_compile_flags.m4])

dnl Doxygen
m4_include([m4/ax_prog_doxygen.m4])
DX_INIT_DOXYGEN([libmonetra], Doxyfile, doc/api)
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_HTML_FEATURE(ON)


dnl ------- LIBCHECK ---------

check_dep_libs=""
AX_FIND_LIBRARY([check_math],    [m],       [floor],              [], ${check_dep_libs})
if test "$have_lib_check_math" = "yes" ; then
  check_dep_libs="-l${have_lib_check_math_lib} ${check_dep_libs}"
fi
AX_FIND_LIBRARY([check_pthread], [pthread], [pthread_mutex_init], [], ${check_dep_libs})
if test "$have_lib_check_pthread" = "yes" ; then
  check_dep_libs="-l${have_lib_check_pthread_lib} ${check_dep_libs}"
fi
AX_FIND_LIBRARY([check_rt],      [rt],      [timer_delete],       [], ${check_dep_libs})
if test "$have_lib_check_rt" = "yes" ; then
  check_dep_libs="-l${have_lib_check_rt_lib} ${check_dep_libs}"
fi
AX_FIND_LIBRARY([check_subunit], [subunit], [subunit_test_start], [], ${check_dep_libs})
if test "$have_lib_check_subunit" = "yes" ; then
  check_dep_libs="-l${have_lib_check_subunit_lib} ${check_dep_libs}"
fi

AX_FIND_PACKAGE(check,
	[/usr/local/check /usr/local/check32 /usr/local/check64 /usr/local],
	[check.h],
	[suite_create],
	[check_pic check],
	${check_dep_libs},
	[/include/check /include /],
	[/lib/check /lib /])

AM_CONDITIONAL([HAVE_CHECK],        [ test $CHECK_FOUND = yes ])


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
	-Wall \
	-Wextra \
	-Wcast-align \
	-Wconversion \
	-Wdeclaration-after-statement \
	-Wdouble-promotion \
	-Wfloat-equal \
	-Wformat-security \
	-Winit-self \
	-Wjump-misses-init \
	-Wlogical-op \
	-Wmissing-braces \
	-Wmissing-declarations \
	-Wmissing-format-attribute \
	-Wmissing-include-dirs \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wno-coverage-mismatch \
	-Wold-style-definition \
	-Wpacked \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wshadow \
	-Wsign-conversion \
	-Wstrict-overflow \
	-Wstrict-prototypes \
	-Wtrampolines \
	-Wundef \
	-Wunused \
	-Wvariadic-macros \
	-Wvla \
	-Wwrite-strings \
	-Werror=implicit-int \
	-Werror=implicit-function-declaration \
	-Werror=partial-availability \
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
AC_ARG_ENABLE(internal-build,
	AC_HELP_STRING([--enable-internal-build],
		[ Disables checks for precompiled dependencies like mstdlib.  Instead it assumes the include paths and library paths have already been set up appropriately ]),
	[ INTERNAL_BUILD=yes ],
	[ INTERNAL_BUILD=no ])

dnl chain build if thirdparty/mstdlib exists, otherwise search for it
if test -d "${srcdir}/thirdparty/mstdlib" ; then
	ac_configure_args="${ac_configure_args} --disable-tests"
	AC_CONFIG_SUBDIRS([thirdparty/mstdlib])
	BUILD_SUBDIRS="${BUILD_SUBDIRS} thirdparty/mstdlib"
	MSTDLIB_CPPFLAGS='-I$(top_srcdir)/thirdparty/mstdlib/include -I$(top_builddir)/thirdparty/mstdlib/include'
	MSTDLIB_LIBADD='$(top_builddir)/thirdparty/mstdlib/tls/.libs/libmstdlib_tls.la $(top_builddir)/thirdparty/mstdlib/io/.libs/libmstdlib_io.la $(top_builddir)/thirdparty/mstdlib/thread/.libs/libmstdlib_thread.la $(top_builddir)/thirdparty/mstdlib/formats/.libs/libmstdlib_formats.la $(top_builddir)/thirdparty/mstdlib/base/.libs/libmstdlib.la   '
	MSTDLIB_LDFLAGS=""
	AC_SUBST(MSTDLIB_CPPFLAGS)
	AC_SUBST(MSTDLIB_LIBADD)
	AC_SUBST(MSTDLIB_LDFLAGS)
	LIBMONETRA_BUILD_SUBDIRS="thirdparty/mstdlib"
else
	AX_FIND_PACKAGE(mstdlib,
		[/usr/local/mstdlib /usr/local],
		[mstdlib/mstdlib.h],
		[M_buf_create],
		[mstdlib])

	if test "${MSTDLIB_FOUND}" != "yes" ; then
		if test "${INTERNAL_BUILD}" = "no" ; then
			AC_MSG_ERROR(unable to locate mstdlib)
		fi
		MSTDLIB_LIBADD="-lmstdlib"
	fi

	MSTDLIB_LIBADD="-lmstdlib_tls -lmstdlib_io -lmstdlib_thread -lmstdlib_formats ${MSTDLIB_LIBADD}"
	AC_SUBST(MSTDLIB_LIBADD)
fi

LIBMONETRA_BUILD_SUBDIRS="${LIBMONETRA_BUILD_SUBDIRS} include src"

dnl
dnl tests
dnl
AC_ARG_ENABLE(tests,
	AC_HELP_STRING([--disable-tests], [Disable building of tests]),
	[ build_tests=${enableval} ],
	[ build_tests=yes ])


if test "${build_tests}" = "yes" ; then
	if test "${CHECK_FOUND}" != "yes" ; then
		AC_MSG_ERROR([libcheck not found, requried for tests])
	fi
	LIBMONETRA_BUILD_SUBDIRS="${LIBMONETRA_BUILD_SUBDIRS} test"
fi

AC_SUBST(LIBMONETRA_BUILD_SUBDIRS)

AC_DEFINE([LIBMONETRA_INTERNAL], 1, [Let config know we are internal])

dnl Libtool versioning is weird, they use a 'current', 'age', and 'revision'
dnl methodology to determine the ABI version.  We actually are guaranteeing
dnl it based on our versions.  Major means ABI breaks, minor means addition
dnl of new functions, and release means bugfixes.  We need to map our version
dnl to the libtool version.  The forward method for .so versioning is
dnl (current-age).age.revision, so we'll do the opposite here:
LIBMONETRA_LIBTOOL_VERSION="`expr libmonetra_major_version + libmonetra_minor_version`:`echo libmonetra_release_version`:`echo libmonetra_minor_version`"
AC_SUBST(LIBMONETRA_LIBTOOL_VERSION)

AC_OUTPUT


