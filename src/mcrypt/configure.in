AC_INIT(mcrypt.c)

CAUDIUM_MODULE_INIT()
AM_PATH_LIBMCRYPT(2.4.15)
if test "$no_libmcrypt" != "yes"; then
  CFLAGS="$CFLAGS -DHAVE_LIBMCRYPT"
fi

AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
