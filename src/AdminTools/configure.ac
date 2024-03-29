AC_INIT(atmain.c)
AC_CONFIG_HEADER(at_config.h)

CAUDIUM_MODULE_INIT([AdminTools])

AH_TOP([#include "../../config.h"])
AH_BOTTOM([
#if defined(HAVE_SECURITY_PAM_APPL_H) && defined(HAVE_PAM_END)
#define HAVE_PAM
#endif
])

AC_CHECK_HEADERS(shadow.h dirent.h sys/quota.h mntent.h security/pam_appl.h)
AC_CHECK_FUNCS(getspent getspent_r getspnam getspnam_r opendir closedir \
			   readdir readdir_r rewinddir seekdir telldir scandir      \
			   alphasort versionsort sysconf setspent endspent \
			   quotactl hasmntopt getmntent setmntent \
			   mkstemp mkdtemp) 
AC_CHECK_LIB(pam, pam_start)
AC_CHECK_FUNCS(pam_authenticate pam_chauthtok pam_end pam_start)

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
  AC_DEFINE(HAVE_STRERROR, 1, [Define if you have a failsafe strerror])
else
  AC_MSG_RESULT(no)
fi

if test "$ac_cv_header_dirent_h" = yes; then
    AC_MSG_CHECKING(for struct dirent with d_type)
    AC_CACHE_VAL(atools_cv_struct_dirent_d_type,
    [
    AC_TRY_RUN([
    #include <dirent.h>
    int main()
    {
	struct dirent   dent;
	
	dent.d_type = 1;
	return 0;
    }
    ],atools_cv_struct_dirent_d_type=yes,atools_cv_struct_dirent_d_type=no,
    atools_cv_struct_dirent_d_type=no)
    ])
    
    if test "$atools_cv_struct_dirent_d_type" = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_DIRENT_D_TYPE, 1, [Define if your system has the d_type member in struct dirent])
    else
	AC_MSG_RESULT(no)
    fi
    
    AC_MSG_CHECKING(for struct dirent with d_off)
    AC_CACHE_VAL(atools_cv_struct_dirent_d_off,
    [
    AC_TRY_RUN([
    #include <dirent.h>
    int main()
    {
	struct dirent   dent;
	
	dent.d_off = 1;
	return 0;
    }
    ],atools_cv_struct_dirent_d_off=yes,atools_cv_struct_dirent_d_off=no,
    atools_cv_struct_dirent_d_off=no)
    ])
    
    if test "$atools_cv_struct_dirent_d_off" = yes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_DIRENT_D_OFF, 1, [Define if your system has the d_off member in struct dirent])
    else
	AC_MSG_RESULT(no)
    fi
fi

if test "$ac_cv_func_getspnam_r" = "yes"; then
  AC_MSG_CHECKING([if getspnam_r takes 4 (Solaris) or 5 (Linux) arguments])
  AC_CACHE_VAL(atools_cv_have_solaris_getspnam_r, [
    AC_TRY_LINK([
#define _REENTRANT
#define _THREAD_SAFE
#ifdef HAVE_PASSWD_H
# include <passwd.h>
# include <group.h>
#endif /* HAVE_PASSWD_H */

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif /* HAVE_PWD_H */

#ifdef HAVE_GRP_H
# include <grp.h>
#endif /* HAVE_GRP_H */

#ifdef HAVE_SHADOW_H
# include <shadow.h>
#endif /* HAVE_SHADOW_H */
    ], [
      struct spwd sbuf;
      char buf[2048];
      char *foo = getspnam_r("root", &sbuf, buf, sizeof(buf))->sp_pwdp;
      return 0;
    ], [
      atools_cv_have_solaris_getspnam_r="yes"
    ], [
      atools_cv_have_solaris_getspnam_r="no"
    ])
  ])
  if test "$atools_cv_have_solaris_getspnam_r" = "yes"; then
    AC_MSG_RESULT([4 - Solaris])
    AC_DEFINE(HAVE_SOLARIS_GETSPNAM_R, 1, [Define if you run solaris and have this func])
  else
    AC_MSG_RESULT([5 - Linux])
  fi
else :; fi

if test "$ac_cv_func_getspent_r" = "yes"; then
  AC_MSG_CHECKING([if getsent_r takes 3 (Solaris) or 4 (Linux) arguments])
  AC_CACHE_VAL(atools_cv_have_solaris_getspent_r, [
    AC_TRY_LINK([
#define _REENTRANT
#define _THREAD_SAFE
#ifdef HAVE_PASSWD_H
# include <passwd.h>
# include <group.h>
#endif /* HAVE_PASSWD_H */

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif /* HAVE_PWD_H */

#ifdef HAVE_GRP_H
# include <grp.h>
#endif /* HAVE_GRP_H */

#ifdef HAVE_SHADOW_H
# include <shadow.h>
#endif /* HAVE_SHADOW_H */
    ], [
      struct spwd sbuf;
      char buf[2048];
      char *foo = getspent_r(&sbuf, buf, sizeof(buf))->sp_pwdp;
      return 0;
    ], [
      atools_cv_have_solaris_getspent_r="yes"
    ], [
      atools_cv_have_solaris_getspent_r="no"
    ])
  ])
  if test "$atools_cv_have_solaris_getspnam_r" = "yes"; then
    AC_MSG_RESULT([3 - Solaris])
    AC_DEFINE(HAVE_SOLARIS_GETSPENT_R, 1, [Define if you have solaris and have this func])
  else
    AC_MSG_RESULT([4 - Linux])
  fi
else :; fi

AC_MSG_CHECKING(for known quotactl interface)
if test "`uname`" = "Linux"; then
  if test "$ac_cv_func_quotactl" = "yes" -a \
     "$ac_cv_header_sys_quota_h" = "yes" -a \
     "$ac_cv_header_mntent_h" = "yes"; then
    AC_DEFINE(HAVE_QUOTA_SUPPORT, 1, [Define if your system has known quotactl support])
    AC_MSG_RESULT(Linux)
  else
    AC_MSG_RESULT(unsupported Linux)
  fi
else
    AC_MSG_RESULT(not found)
fi

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
