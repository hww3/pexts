AC_INIT(xslt.c)
AC_CONFIG_HEADER(xslt_config.h)

CAUDIUM_MODULE_INIT([_XSLT])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AM_PATH_XML2(2.5.1, AC_DEFINE(HAVE_XML2, 1, [Define if you have XML2 installed]))
AC_CHECK_PROG(XSLT_CONFIG, xslt-config, xslt-config, none)
AC_MSG_CHECKING([for the libxslt1 libs])
if test "$XSLT_CONFIG" = "xslt-config"; then
  XSLT_CFLAGS="`$XSLT_CONFIG --cflags`"
  XSLT_LIBS="`$XSLT_CONFIG --libs`"
  AC_SUBST(XSLT_CFLAGS)
  AC_SUBST(XSLT_LIBS)
  AC_DEFINE([HAVE_XSLT],[],[Define if libxslt1 is installed])
  AC_MSG_RESULT([found])
else
  AC_MSG_RESULT([not found])
fi

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
