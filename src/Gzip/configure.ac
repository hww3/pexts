AC_INIT(gzip_main.c)
AC_CONFIG_HEADER(gzip_config.h)

CAUDIUM_MODULE_INIT([Gzip])

AC_CHECK_LIB(z, zlibVersion)
AC_CHECK_HEADERS(zlib.h)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
