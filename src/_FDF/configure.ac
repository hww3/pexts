AC_INIT(fdf_main.c)
AC_CONFIG_HEADER(fdf_config.h)

CAUDIUM_MODULE_INIT([FDF])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_HEADERS(fdftk.h)
AC_CHECK_LIB(fdftk, FDFOpen)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
