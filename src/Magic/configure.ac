AC_INIT(magic_main.c)
AC_CONFIG_HEADER(magic_config.h)

CAUDIUM_MODULE_INIT([magic])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_LIB(magic, magic_file,,)
AC_CHECK_HEADERS(magic.h)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
