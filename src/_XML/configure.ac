AC_INIT(xml_main.c)
AC_CONFIG_HEADER(xml_config.h)

CAUDIUM_MODULE_INIT([_XML])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AM_PATH_XML2(2.5.1, AC_DEFINE(HAVE_XML2, 1, [Define if you have XML2 installed]))

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
