#ifndef AT_CONFIG_H
#define AT_CONFIG_H

@TOP@

@BOTTOM@

/* Define if you have a failsafe strerror */
#undef HAVE_STRERROR

/* Define if you run solaris and have this func */
#undef HAVE_SOLARIS_GETSPNAM_R

/* Define if you have solaris and have this func */
#undef HAVE_SOLARIS_GETSPENT_R

/* Define if your system has the d_type member in struct dirent */
#undef HAVE_DIRENT_D_TYPE

/* Define if your system has the d_off member in struct dirent */
#undef HAVE_DIRENT_D_OFF

/* Define if your system has known quotactl support */
#undef HAVE_QUOTA_SUPPORT

#if defined(HAVE_SECURITY_PAM_APPL_H) && defined(HAVE_PAM_END)
#define HAVE_PAM
#endif

#endif
