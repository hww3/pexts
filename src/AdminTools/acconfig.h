#ifndef PCRE_CONFIG_H
#define PCRE_CONFIG_H

@TOP@

@BOTTOM@

#ifndef ADD_STORAGE
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif

/* Define if you have a failsafe strerror */
#undef HAVE_STRERROR

/* Define if you run solaris and have this func */
#undef HAVE_SOLARIS_GETSPNAM_R

/* Define if you have solaris and have this func */
#undef HAVE_SOLARIS_GETSPENT_R

#endif
