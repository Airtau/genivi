AC_PREREQ([2.59])
AC_INIT([checkafbus], [1.0], [javier.martinez@collabora.co.uk])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 tests/Makefile
])
AC_OUTPUT
