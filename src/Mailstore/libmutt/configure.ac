dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.52)
AC_INIT(src/muttlib.c)

MUTT_MAJOR=0
MUTT_REVISION=94
MUTT_PATCHLEVEL=5
MUTT_VERSION="$MUTT_MAJOR.$MUTT_REVISION.$MUTT_PATCHLEVEL"
VERSION=$MUTT_VERSION

AM_CONFIG_HEADER(src/muttconfig.h)
AM_INIT_AUTOMAKE(libmutt, $VERSION)

AM_MAINTAINER_MODE

AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

AC_MSG_CHECKING(for prefix)
if test x$prefix = xNONE; then
	mutt_cv_prefix=$ac_default_prefix
else
	mutt_cv_prefix=$prefix
fi
AC_MSG_RESULT($mutt_cv_prefix)

case "$host" in
*-*-hpux*)
	if test -z "$GCC" ; then
		CFLAGS="$CFLAGS -Ae -D_HPUX_SOURCE"
	fi
	;;
esac

dnl AC_PATH_PROG(SENDMAIL, sendmail, no, `echo $PATH | sed "s/:/ /"` /usr/sbin /usr/lib)
dnl no need to replace ':' by ' ' 
AC_PATH_PROG(SENDMAIL, sendmail, no, $PATH:/usr/sbin:/usr/lib)
AC_DEFINE_UNQUOTED(SENDMAIL, "$ac_cv_path_SENDMAIL",
                   [Path to sendmail program.])

AC_PATH_PROG(ISPELL, ispell, no)
if test $ISPELL != no; then
	AC_DEFINE_UNQUOTED(ISPELL, "$ISPELL",[Path to ispell program.])
fi

AC_HEADER_STDC

AC_CHECK_HEADERS(stdarg.h sys/ioctl.h sysexits.h)

AC_TYPE_SIGNAL

AC_DECL_SYS_SIGLIST

dnl need this for DEC alpha
AC_CHECK_SIZEOF(long)

AC_TYPE_PID_T

AC_CHECK_FUNCS(fgetpos memmove setegid srand48 strerror)

AC_REPLACE_FUNCS(strcasecmp)

# otherwise test as usual
  mutt_cv_snprintf=no
  #AC_CHECK_FUNC(snprintf, [AC_DEFINE(HAVE_SNPRINTF,1,[])], [mutt_cv_snprintf=yes])
  #AC_CHECK_FUNC(vsnprintf, [AC_DEFINE(HAVE_VSNPRINTF,1,[])], [mutt_cv_snprintf=yes])
  if test $mutt_cv_snprintf = yes; then
    AC_LIBOBJ([snprintf])
  fi

dnl SCO uses chsize() instead of ftruncate()
AC_CHECK_FUNCS(ftruncate, break, [AC_CHECK_LIB(x, chsize)])

dnl SCO has strftime() in libintl
AC_CHECK_FUNCS(strftime, break, [AC_CHECK_LIB(intl, strftime)])

dnl AIX may not have fchdir()
AC_CHECK_FUNCS(fchdir, [AC_DEFINE(HAVE_FCHDIR)], [mutt_cv_fchdir=no])

AC_ARG_WITH(homespool, [  --with-homespool[=FILE]    file in user's directory where new mail is spooled], with_homespool=${withval})
if test x$with_homespool != x; then
	if test $with_homespool = yes; then
		with_homespool=mailbox
	fi
	AC_DEFINE_UNQUOTED(MAILPATH, "$with_homespool",
                           [Define to the path to the home spool.])
	AC_DEFINE(HOMESPOOL,1,[Define if Home spool is to be used.])
	AC_DEFINE(USE_DOTLOCK,1,[Define if dotlock is to be used.])
	mutt_cv_setgid=no
else
	AC_ARG_WITH(mailpath, [  --with-mailpath=DIR        directory where spool mailboxes are located],
		[mutt_cv_mailpath=$withval],
		[ AC_CACHE_CHECK(where new mail is stored, mutt_cv_mailpath,
			[mutt_cv_mailpath=no
			if test -d /var/mail; then
				mutt_cv_mailpath=/var/mail
			elif test -d /var/spool/mail; then
				mutt_cv_mailpath=/var/spool/mail
			elif test -d /usr/spool/mail; then
				mutt_cv_mailpath=/usr/spool/mail
			elif test -d /usr/mail; then
				mutt_cv_mailpath=/usr/mail
			fi])
		])
	if test $mutt_cv_mailpath = no; then
		AC_MSG_ERROR("Could not determine where new mail is stored.")
	fi
	AC_DEFINE_UNQUOTED(MAILPATH, "$mutt_cv_mailpath")

	AC_CACHE_CHECK(if $mutt_cv_mailpath is world writable, mutt_cv_worldwrite, AC_TRY_RUN([#include <sys/types.h>
#include <sys/stat.h>

int main (int argc, char **argv)
{
	struct stat s;

	stat ("$mutt_cv_mailpath", &s);
	if (s.st_mode & S_IWOTH) exit (0);
	exit (1);
}], [mutt_cv_worldwrite=yes], [mutt_cv_worldwrite=no], 
	[ AC_MSG_ERROR("Cannot cross compile libmutt. Yet")] ))

	mutt_cv_setgid=no
	if test $mutt_cv_worldwrite = yes; then
		AC_DEFINE(USE_DOTLOCK)
	else

		AC_CACHE_CHECK(if $mutt_cv_mailpath is group writable, mutt_cv_groupwrite, AC_TRY_RUN([#include <sys/types.h>
#include <sys/stat.h>

int main (int argc, char **argv)
{
	struct stat s;

	stat ("$mutt_cv_mailpath", &s);
	if (s.st_mode & S_IWGRP) exit (0);
	exit (1);
}], [mutt_cv_groupwrite=yes], [mutt_cv_groupwrite=no],
	[ AC_MSG_ERROR("Cannot cross compile libmutt. Yet")] ))

dnl		if test $mutt_cv_groupwrite = yes; then
dnl			AC_DEFINE(USE_DOTLOCK)
dnl			AC_DEFINE(USE_SETGID)
dnl			mutt_cv_setgid=yes
dnl		fi
	fi
fi

AC_ARG_WITH(sharedir, [  --with-sharedir=PATH       specify where to put arch independent files],
	[mutt_cv_sharedir=$withval],
	[ AC_CACHE_CHECK(where to put architecture-independent data files,
		       mutt_cv_sharedir,
		[if test x$prefix = xNONE; then
			mutt_cv_prefix=$ac_default_prefix
		else
			mutt_cv_prefix=$prefix
		fi
		if test -d ${mutt_cv_prefix}/share; then
			if test -d ${mutt_cv_prefix}/share/misc; then
				mutt_cv_sharedir='${prefix}/share/misc'
			else
				mutt_cv_sharedir='${prefix}/share'
			fi
		else
			mutt_cv_sharedir='${libdir}'
		fi])
	])

sharedir=$mutt_cv_sharedir
AC_SUBST(sharedir)

AC_DEFINE(USE_IMAP,1,[Always compile IMAP support.])
need_imap=yes
need_socket=yes

AC_CHECK_FUNC(setsockopt, , AC_CHECK_LIB(socket, setsockopt))
AC_CHECK_FUNC(gethostent, , AC_CHECK_LIB(nsl, gethostent))
AC_CHECK_FUNCS(getaddrinfo)
AC_DEFINE(USE_SOCKET,1,[Always use flexible socket code.])
LIBIMAP="-Limap -limap"
LIBIMAPDEPS="\$(top_srcdir)/imap/imap.h imap/libimap.a"
AM_CONDITIONAL(BUILD_IMAP, test x$need_imap = xyes)
AC_LIBOBJ([account])
AC_LIBOBJ([mutt_socket])

dnl ##########################################################################
dnl Enable GSS?
dnl ##########################################################################

AC_ARG_WITH(gss, [  --with-gss            Enable GSS [default=no]],
    [with_gss=$withval],[with_gss=no])

AC_MSG_CHECKING([whether to build GSS support])
if test "x$with_gss" != xno  ; then
    AC_MSG_RESULT(yes)
    if test x"$with_gss" != xyes ; then
	dirs="$with_gss default /usr/local /usr/kerberos /usr/lib /usr"
    else
	dirs="default /usr/local /usr/kerberos /usr/lib /usr"
    fi
    gssdir=""
    save_CPP="$CPP"
    for dir in $dirs ; do
	test x"$dir" != xdefault && CPP="$save_CPP -I$dir/include"
	AC_CHECK_HEADERS(gssapi.h gssapi/gssapi.h,  break,
        [ unset ac_cv_header_gssapi_gssapi_h ac_cv_header_gssapi_h ])
        if test x"$ac_cv_header_gssapi_gssapi_h" = xyes -o \
                x"$ac_cv_header_gssapi_h" = xyes; then
           gssdir="$dir"; break
        fi
    done
    if test x"$dir" != xdefault; then
        # keep CPP set for subsequent CHECK_HEADERS
        CPP="$save_CPP -I$dir/include"
        CFLAGS="$CFLAGS -I$dir/include"
    fi
    echo "dir='$dir' gssdir='$gssdir'"
    if test -z "$gssdir"; then
	AC_MSG_ERROR([GSS requested but no GSS headers found])
    else
	AC_CHECK_HEADERS(gssapi/gssapi_generic.h)
	if test "x$gssdir" = xdefault ; then
	    extra_libs=""
	else
	    extra_libs="-L$gssdir/lib"
	fi
	save_LIBS="$LIBS"
	for lib in krb5 des asn1 roken crypt com_err ; do
	    LIBS="$LIBS -l$lib"
            AC_CACHE_CHECK([for -l$lib],[ac_cv_lib_$lib],
                   AC_TRY_LINK(, [main()], [eval ac_cv_lib_$lib=yes],
                           [eval ac_cv_lib_$lib=no]))
           eval "test x\"\$ac_cv_lib_$lib\" = xyes" && extra_libs="$extra_libs -l$lib"
	    LIBS="$save_LIBS"
	done

	for lib in gssapi gssapi_krb5 gss ; do
	    AC_CHECK_LIB($lib, gss_init_sec_context, [gsslib="$lib";break], , 
                         $extra_libs)
	done

	if test x"$gsslib" = x ; then
	    AC_MSG_ERROR([GSS requested but no GSS libraries found])
	else
	    LIBS="$LIBS -l$gsslib $extra_libs"
	    test x"$gsslib" = xgssapi && x"$av_cv_lib_krb5" = xyes &&
		LIBS="$LIBS -lkrb5"
	    if test "$ac_cv_header_gssapi_h" = "yes"; then
		AC_EGREP_HEADER(GSS_C_NT_HOSTBASED_SERVICE, gssapi.h,
		    AC_DEFINE(HAVE_GSS_C_NT_HOSTBASED_SERVICE,1,
		    [Define if GSS C NT hostbased service is available.]))
	    else
		AC_EGREP_HEADER(GSS_C_NT_HOSTBASED_SERVICE, gssapi/gssapi.h,
		    AC_DEFINE(HAVE_GSS_C_NT_HOSTBASED_SERVICE,1,
		    [Define if GSS C NT hostbased service is available.]))
	    fi
	fi
    fi
else 
    AC_MSG_RESULT(no)
    gssdir=no
fi
AM_CONDITIONAL(USE_GSS, test x$gssdir != xno)


dnl ##########################################################################
dnl Enable SSL ?
dnl ##########################################################################

AC_ARG_WITH(ssl, [  --with-ssl[=PFX]         Compile in SSL socket support for IMAP],
[	if test "$with_ssl" != "no"
	then
	  if test "$need_imap" != "yes"; then
	   AC_MSG_ERROR([SSL support is only for IMAP, but IMAP is not enabled])
	  fi
          if pkg-config openssl >& /dev/null; then
	     echo here
             LDFLAGS="$LIBS `pkg-config --libs openssl`"
             CPPFLAGS="`pkg-config --cflags-only-I openssl` $CPPFLAGS"
          else
             echo failed
	     AC_CACHE_CHECK(for ssldir, ac_cv_path_openssl, [
	       if test "$with_ssl" != "yes"; then
			   ac_cv_path_openssl=$withval
	       fi
	     ])
	     if test "x$ac_cv_path_openssl" != "x"; then
	        LDFLAGS="$LDFLAGS -L$ac_cv_path_openssl/lib"
	        CPPFLAGS="$CPPFLAGS -I$ac_cv_path_openssl/include"
	     fi	
	     saved_LIBS="$LIBS"
          fi
	  AC_CHECK_LIB(crypto, X509_new,, AC_MSG_ERROR([Unable to find SSL library]))
	  AC_CHECK_LIB(ssl, SSL_new,, AC_MSG_ERROR([Unable to find SSL library]), -lcrypto)

	  AC_CHECK_FUNCS(RAND_status RAND_egd)

	  AC_DEFINE(USE_SSL,1,[Define if SSL support is to be compiled.])
	  LIBS="$saved_LIBS"
	  MUTTLIBS="$MUTTLIBS -lssl -lcrypto"
	  need_ssl=yes
          
          dnl Some systems avoid certain patent-encumbered SSL routines
          AC_CHECK_HEADER(openssl/rc5.h,,AC_DEFINE(NO_RC5,1,[No RC5.]))
          AC_CHECK_HEADER(openssl/idea.h,,AC_DEFINE(NO_IDEA,1,[No idea.]))
          AC_CHECK_HEADER(openssl/rsa.h,,AC_DEFINE(NO_RSA,1,[No RSA.]))
	fi
])
AM_CONDITIONAL(USE_SSL, test x$need_ssl = xyes)

AC_ARG_WITH(sasl, [  --with-sasl[=DIR]          Use Cyrus SASL library for POP/IMAP authentication],
	[
	if test "$need_socket" != "yes"
	then
	  AC_MSG_ERROR([SASL support is only useful with POP or IMAP support])
	fi

	if test "$with_sasl" != "no"
	then
	  if test "$with_sasl" != "yes"
	  then
	    CPPFLAGS="$CPPFLAGS -I$with_sasl/include"
	    LDFLAGS="$LDFLAGS -L$with_sasl/lib"
	  fi

	  saved_LIBS="$LIBS"

	  AC_CHECK_LIB(sasl, sasl_client_init,,
	    AC_MSG_ERROR([could not find libsasl]),)

	  MUTT_LIB_OBJECTS="$MUTT_LIB_OBJECTS mutt_sasl.o"
	  MUTTLIBS="$MUTTLIBS -lsasl"
	  LIBS="$saved_LIBS"
	  AC_DEFINE(USE_SASL,1,[Defined if SASL library is to be used.])
	  need_sasl=yes
	  need_md5=no
	fi
	])
AM_CONDITIONAL(USE_SASL, test x$need_sasl = xyes)

dnl SSL support via NSS
AC_ARG_WITH(nss, [  --with-nss[=PFX]           Compile in SSL support for POP/IMAP via NSS],
[       if test "$with_nss" != no
        then
          if test "$need_socket" != "yes"; then
           AC_MSG_ERROR([SSL support is only useful with POP or IMAP support])
          fi

          if test "$with_nss" != "yes"
          then
           LDFLAGS="$LDFLAGS -L$withval/lib"
           CPPFLAGS="$CPPFLAGS -I$withval/include -I$withval/public/security"
          fi

          AC_DEFINE(USE_NSS,1,[ Define if you want support for SSL via the NSS library. ])
          MUTTLIBS="$MUTTLIBS -lssl -lnss -lcertdb -lcerthi -lcryptohi"
          MUTTLIBS="$MUTTLIBS -lpk11wrap -lsoftoken -lsecutil -ldbm -lplds4 -lplc4 -lfreebl"
          MUTTLIBS="$MUTTLIBS -lnspr4"

          MUTT_LIB_OBJECTS="$MUTT_LIB_OBJECTS mutt_ssl_nss.o"
          need_ssl=yes
        fi
])
AM_CONDITIONAL(USE_NSS, test x$need_nss = xyes)

mutt_cv_flock=yes
AC_ARG_ENABLE(flock, [  --disable-fcntl            Do NOT use flock() to lock files ],
	[if test $enableval = no; then mutt_cv_flock=no; fi])

if test $mutt_cv_flock = yes; then
	AC_DEFINE(USE_FLOCK,1,[Define if flock locking should be used.])
fi

AC_ARG_ENABLE(fcntl, [  --enable-fcntl             Use fcntl() to lock files],
	[if test $enableval = yes; then
		AC_DEFINE(USE_FCNTL,1,[Define if fcntl locking should be used.])
	fi])

AC_ARG_ENABLE(nfs-fix, [  --enable-nfs-fix           Work around an NFS with broken attributes caching ],
	[if test x$enableval = xyes; then
                AC_DEFINE(NFS_ATTRIBUTE_HACK,1, 
		[Define if libmutt should try to work around NFS issues.])
        fi])

AC_ARG_ENABLE(buffy-size, [  --enable-buffy-size        Use file size attribute instead of access time ],
	[if test x$enableval = xyes; then
                AC_DEFINE(BUFFY_SIZE,1,
		[Defined when file size is to be used to discover changes to mailbox.])
        fi])

AC_ARG_ENABLE(locales-fix, [  --enable-locales-fix       The result of isprint() is unreliable ],
	[if test x$enableval = xyes; then
                AC_DEFINE(LOCALES_HACK,1,[Enable locales hack.])
        fi])

AC_ARG_WITH(exec-shell, [  --with-exec-shell=SHELL    Specify alternate shell (ONLY if /bin/sh is broken)],
	[if test $withval != yes; then
		AC_DEFINE_UNQUOTED(EXECSHELL, "$withval",
		   [Define to path to an alternate shell.])
	fi])

AC_ARG_ENABLE(exact-address, [  --enable-exact-address     enable regeneration of email addresses],
	[if test $enableval = yes; then
		AC_DEFINE(EXACT_ADDRESS,1,
		   [Defined if email adresses can be regenerated.]
		)
	fi])


dnl -----------------------------------
dnl ICONV
dnl -----------------------------------
  dnl Some systems have iconv in libc, some have it in libiconv (OSF/1 and
  dnl those with the standalone portable GNU libiconv installed).

  AC_ARG_WITH([iconv],
[  --with-iconv[=DIR]  search for libiconv in DIR/include and DIR/lib], [
    if test "$withval" != no ; then
    for dir in `echo "$withval" | tr : ' '`; do
      if test -d $dir/include; then CPPFLAGS="$CPPFLAGS -I$dir/include"; fi
      if test -d $dir/lib; then LDFLAGS="$LDFLAGS -L$dir/lib"; fi
    done
    else
      use_iconv=no
    fi
   ],use_iconv=yes)

  if test "$use_iconv" = yes ; then
  AC_CACHE_CHECK(for iconv, am_cv_func_iconv, [
    am_cv_func_iconv="no, consider installing GNU libiconv"
    am_cv_lib_iconv=no
    AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
      [iconv_t cd = iconv_open("","");
       iconv(cd,NULL,NULL,NULL,NULL);
       iconv_close(cd);],
      am_cv_func_iconv=yes)
    if test "$am_cv_func_iconv" != yes; then
      am_save_LIBS="$LIBS"
      LIBS="$LIBS -liconv"
      AC_TRY_LINK([#include <stdlib.h>
#include <iconv.h>],
        [iconv_t cd = iconv_open("","");
         iconv(cd,NULL,NULL,NULL,NULL);
         iconv_close(cd);],
        am_cv_lib_iconv=yes
        am_cv_func_iconv=yes)
      LIBS="$am_save_LIBS"
    fi
  ])
  if test "$am_cv_func_iconv" = yes; then
    AC_DEFINE(HAVE_ICONV, 1, [Define if you have the iconv() function.])
    AC_MSG_CHECKING([for iconv declaration])
    AC_CACHE_VAL(am_cv_proto_iconv, [
      AC_TRY_COMPILE([
#include <stdlib.h>
#include <iconv.h>
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
size_t iconv (iconv_t cd, char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);
#else
size_t iconv();
#endif
], [], am_cv_proto_iconv_arg1="", am_cv_proto_iconv_arg1="const")
      am_cv_proto_iconv="extern size_t iconv (iconv_t cd, $am_cv_proto_iconv_arg1 char * *inbuf, size_t *inbytesleft, char * *outbuf, size_t *outbytesleft);"])
    am_cv_proto_iconv=`echo "[$]am_cv_proto_iconv" | tr -s ' ' | sed -e 's/( /(/'`
    AC_MSG_RESULT([$]{ac_t:-
         }[$]am_cv_proto_iconv)
    AC_DEFINE_UNQUOTED(ICONV_CONST, $am_cv_proto_iconv_arg1,
      [Define as const if the declaration of iconv() needs const.])
  fi
  LIBICONV=
  if test "$am_cv_lib_iconv" = yes; then
    LIBICONV="-liconv"
  fi
  AC_SUBST(LIBICONV)

  fi # use_iconv



AC_CHECK_HEADERS(iconv.h,
        [AC_MSG_CHECKING(whether iconv.h defines iconv_t)
         AC_EGREP_HEADER([typedef.*iconv_t],iconv.h,
                [AC_MSG_RESULT(yes)
                 AC_DEFINE(HAVE_ICONV_T_DEF, 1,
                        [Define if <iconv.h> defines iconv_t.])],
                 AC_MSG_RESULT(no))])

if test "$am_cv_func_iconv" != "yes"
then
#  AC_MSG_ERROR([Unable to find an iconv function. See INSTALL for help])
  AC_MSG_WARN([Unable to find an iconv function. See INSTALL for help])
else

dnl (1) Some implementations of iconv won't convert from UTF-8 to UTF-8.
dnl (2) In glibc-2.1.2 and earlier there is a bug that messes up ob and
dnl     obl when args 2 and 3 are 0 (fixed in glibc-2.1.3).
AC_CACHE_CHECK([whether this iconv is good enough], mutt_cv_iconv_good,
        mutt_save_LIBS="$LIBS"
        LIBS="$LIBS $LIBICONV"
        AC_TRY_RUN([
#include <iconv.h>
int main()
{
  iconv_t cd;
changequote(, )dnl
  char buf[4];
changequote([, ])dnl
  char *ob;
  size_t obl;
  ob = buf, obl = sizeof(buf);
  return ((cd = iconv_open("UTF-8", "UTF-8")) != (iconv_t)(-1) &&
          (iconv(cd, 0, 0, &ob, &obl) ||
           !(ob == buf && obl == sizeof(buf)) ||
           iconv_close(cd)));
}
                ],
                mutt_cv_iconv_good=yes,
                mutt_cv_iconv_good=no,
                mutt_cv_iconv_good=yes)
        LIBS="$mutt_save_LIBS")
if test "$mutt_cv_iconv_good" = no; then
  AC_MSG_ERROR(Try using libiconv instead)
fi

dnl This is to detect implementations such as the one in glibc-2.1,
dnl which always convert exactly but return the number of characters
dnl converted instead of the number converted inexactly.
AC_CACHE_CHECK([whether iconv is non-transcribing], mutt_cv_iconv_nontrans,
        mutt_save_LIBS="$LIBS"
        LIBS="$LIBS $LIBICONV"
        AC_TRY_RUN([
#include <iconv.h>
#include <string.h>
int main()
{
  iconv_t cd;
  const char *ib;
  char *ob;
  size_t ibl, obl;
  const char *s = "\304\211";
changequote(, )dnl
  char t[3];
changequote([, ])dnl
  ib = s, ibl = 2, ob = t, obl = 3;
  return ((cd = iconv_open("UTF-8", "UTF-8")) == (iconv_t)(-1) ||
          iconv(cd, &ib, &ibl, &ob, &obl));
}
                ],
                mutt_cv_iconv_nontrans=no,
                mutt_cv_iconv_nontrans=yes,
                mutt_cv_iconv_nontrans=no)
        LIBS="$mutt_save_LIBS")
if test "$mutt_cv_iconv_nontrans" = yes; then
  AC_DEFINE(ICONV_NONTRANS, 1,[iconv parameter list, v.1])
else
  AC_DEFINE(ICONV_NONTRANS, 0,[iconv parameter list, v.2])
fi

fi # libiconv

CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/intl"

if test "$BUILD_INCLUDED_LIBINTL" = "yes"; then
  AC_DEFINE(HAVE_BIND_TEXTDOMAIN_CODESET,1,
          [ Define if your gettext has bind_textdomain_codeset. ])
else
  AC_CHECK_FUNCS(bind_textdomain_codeset)
fi




dnl ##########################################################################
dnl BALSA: Check for PCRE if available
dnl ##########################################################################

AC_ARG_ENABLE([pcre], 
	[  --enable-pcre           Use Perl-compatible regular expressions if available [default=yes]],[
	check_pcre=$enableval],[
	check_pcre=yes
])

AC_MSG_CHECKING(whether to use PCRE)
if test x"$check_pcre" = xyes ; then
	if pcre-config --libs-posix > /dev/null 2>&1; then
        	vers=`pcre-config --version`
	        case $vers
        	in
                       2.*)
                               have_pcre=no ;;
                       *) 
                               have_pcre=yes ;;
        	esac
	else
        	have_pcre=no
	fi

	if test x"$have_pcre" = xyes ; then
	       	AC_MSG_RESULT(yes)
		CFLAGS="$CFLAGS `pcre-config --cflags-posix`"
		AC_DEFINE(HAVE_PCRE,1,[Define if PCRE library is available.]a<)
	else
	        AC_MSG_RESULT(not found)
	fi
else
	have_pcre=no
	AC_MSG_RESULT(no)
fi

AC_SUBST(MUTT_MAJOR)
AC_SUBST(MUTT_REVISION)
AC_SUBST(MUTT_PATCHLEVEL)
AC_SUBST(MUTT_VERSION)
AC_SUBST(LIBIMAP)
AC_SUBST(LIBIMAPDEPS)

AC_OUTPUT(Makefile src/Makefile imap/Makefile)
