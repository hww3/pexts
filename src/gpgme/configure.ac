AC_INIT(gpgme_main.c)
AC_CONFIG_HEADER(gpgme_config.h)

CAUDIUM_MODULE_INIT([GPGME])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AM_PATH_GPGME(0.3.4)
AC_DEFINE(GPGME_MIN_VER, "0.3.4", [Minimal GPGME version])
AC_CHECK_HEADERS(gpgme.h)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
