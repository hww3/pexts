AC_INIT(geoip_main.c)
AC_CONFIG_HEADER(geoip_config.h)

CAUDIUM_MODULE_INIT()

AC_SUBST(PIKE)

AC_CHECK_LIB(GeoIP, GeoIP_country_code_by_name)

AC_CHECK_FUNCS(GeoIP_new)
AC_CHECK_HEADERS(GeoIP.h)

AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
