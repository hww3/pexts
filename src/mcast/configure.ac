AC_INIT(mcast.c)
AC_CONFIG_HEADER(mcast_config.h)

CAUDIUM_MODULE_INIT()

AC_CHECK_HEADERS(sys/socket.h arpa/inet.h errno.h)

AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
