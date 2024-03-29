dnl Process this file with autoconf to produce a configure script.

dnl This is required at the start; the name is the name of a file
dnl it should be seeing, to verify it is in the same directory.

AC_INIT(dftables.c)

dnl Arrange to build config.h from config.in. Note that pcre.h is
dnl built differently, as it is just a "substitution" file.
dnl Manual says this macro should come right after AC_INIT.
AC_CONFIG_HEADER(config.h:config.in)

dnl Provide the current PCRE version information. Do not use numbers
dnl with leading zeros for the minor version, as they end up in a C
dnl macro, and may be treated as octal constants. Stick to single
dnl digits for minor numbers less than 10. There are unlikely to be
dnl that many releases anyway.

PCRE_MAJOR=3
PCRE_MINOR=4
PCRE_DATE=22-Aug-2000
PCRE_VERSION=${PCRE_MAJOR}.${PCRE_MINOR}

dnl Checks for programs.

AC_PROG_CC
AC_PROG_RANLIB

dnl Checks for header files.

AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h)

dnl Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.

AC_CHECK_FUNCS(bcopy memmove strerror)

dnl Handle --enable-utf8

AC_ARG_ENABLE(utf8,
[  --enable-utf8           enable UTF8 support (incomplete)],
if test "$enableval" = "yes"; then
  UTF8=-DSUPPORT_UTF8
fi
)

dnl "Export" these variables

AC_SUBST(UTF8)
AC_SUBST(PCRE_MAJOR)
AC_SUBST(PCRE_MINOR)
AC_SUBST(PCRE_DATE)
AC_SUBST(PCRE_VERSION)
AC_SUBST(PCRE_LIB_VERSION)
AC_SUBST(PCRE_POSIXLIB_VERSION)

dnl This must be last; it determines what files are written
AC_OUTPUT(Makefile pcre.h:pcre.in)
