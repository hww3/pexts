AC_INIT(newt_global.h)
AC_CONFIG_HEADER(newt_config.h)

CAUDIUM_MODULE_INIT()

AH_BOTTOM([
#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)
#define HAVE_NEWT
#include <newt.h>
#endif
])

AC_CHECK_HEADERS( newt.h )
AC_CHECK_LIB(newt, newtInit)

if test $ac_cv_lib_newt_newtInit = yes; then
    AC_CHECK_FUNCS(newtCursorOn newtCursorOff newtListitem newtListitemSet newtListitemGetData)
    AC_CHECK_FUNCS(newtCheckboxTreeSetEntry newtCheckboxTreeGetEntryValue newtCheckboxTreeSetEntryValue)
    AC_CHECK_FUNCS(newtSetThreeD newtFormSetTimer)
fi

AC_SUBST(PIKE)

AC_OUTPUT(Makefile.pre test, [${MAKE:-make} -f Makefile.pre depend; chmod 755 test])


dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
