AC_INIT(bz2main.c)
AC_CONFIG_HEADER(bz2_config.h)

CAUDIUM_MODULE_INIT()

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

AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
