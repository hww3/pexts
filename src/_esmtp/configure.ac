AC_INIT(esmtp_main.c)
AC_CONFIG_HEADER(esmtp_config.h)

CAUDIUM_MODULE_INIT([ESMTP])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_LIB(esmtp, smtp_create_session,,)
AC_CHECK_HEADERS(libesmtp.h)
ACX_PATH_LIBESMTP(0.8.8, AC_DEFINE(HAVE_LIBESMTP))

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
