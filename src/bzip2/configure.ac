AC_INIT(bz2main.c)
AC_CONFIG_HEADER(bz2_config.h)

CAUDIUM_MODULE_INIT([bzip2])

AH_BOTTOM([
#undef HAVE_OLD_LIBBZ2

void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_LIB(bz2, BZ2_bzCompressInit,,[
	AC_CHECK_LIB(bz2, bzCompressInit,[
		AC_DEFINE(HAVE_OLD_LIBBZ2)
		LIBS="$LIBS -lbz2"
	])
])
AC_CHECK_HEADERS(bzlib.h)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
