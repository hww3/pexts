AC_INIT(ol_main.c)
AC_CONFIG_HEADER(ol_config.h)

CAUDIUM_MODULE_INIT()

AC_CHECK_HEADERS(unistd.h lber.h ldap.h)
AC_CHECK_LIB(ldap, ldap_init)

AC_FUNC_ALLOCA

AC_MSG_CHECKING(for working (and failsafe) strerror)
AC_CACHE_VAL(atools_cv_func_failsafe_strerror,
[
AC_TRY_RUN([
#include <stdio.h>
#include <string.h>
int main()
{
  int e;
  char *s;
  for(e=0;e<256;e++) if(strerror(e)) if(strerror(e)[0]=='\b') exit(1);
  exit(0);
}
],atools_cv_func_failsafe_strerror=yes,atools_cv_func_failsafe_strerror=no,
  atools_cv_func_failsafe_strerror=no)
])

if test "$atools_cv_func_failsafe_strerror" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_STRERROR, 1, [Define if you have strerror])
else
  AC_MSG_RESULT(no)
fi

AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
