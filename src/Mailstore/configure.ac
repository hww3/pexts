AC_INIT(mailstore_main.c)
AC_CONFIG_HEADER(mailstore_config.h)

CAUDIUM_MODULE_INIT()

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CONFIG_SUBDIRS(libmutt)

AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])


dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
dnl vi: set ts=3:
	
