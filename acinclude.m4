AC_DEFUN([CAUDIUM_CONFIGURE_PART],[
  AC_MSG_RESULT()
  AC_MSG_RESULT([${T_MD}$1${T_ME}])
])

define([CAUDIUM_LOW_MODULE_INIT],
[
# $Id$

AC_PROG_CC

# The AC_PROG_INSTALL test is broken if $INSTALL is specified by hand.
# The FreeBSD ports system does this...
# Workaround:
if test "x$INSTALL" = "x"; then :; else
  # $INSTALL overrides ac_cv_path_install anyway...
  ac_cv_path_install="$INSTALL"
fi

AC_PROG_INSTALL

AC_SUBST(CLIBRARY_LINK)
AC_SUBST(PIKE_INCLUDE_DIRS)
AC_SUBST(PIKE_MODULE_DIR)
AC_SUBST(PIKE_VERSION)
AC_SUBST(PIKE)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(CC)
AC_SUBST(LDFLAGS)
AC_SUBST(LDSHARED)
AC_SUBST(SO)
AC_SUBST(LIBGCC)

AC_DEFINE(POSIX_SOURCE)
])


AC_DEFUN([CAUDIUM_MODULE_INIT],
[
  if test -z "$1"; then
    MODNAME="Unspecified"
  else
    MODNAME="$1"
  fi
  CAUDIUM_CONFIGURE_PART([$MODNAME])
  CAUDIUM_LOW_MODULE_INIT()

  AC_MSG_CHECKING([for the Pike Extension Package base directory])

  makefile_rules=../Makefile.rules

  counter=.

  while test ! -f "$makefile_rules"
  do
    counter=.$counter
    if test $counter = .......... ; then
      AC_MSG_RESULT(failed)
      exit 1
    else
      :
    fi
    makefile_rules=../$makefile_rules
  done
  AC_MSG_RESULT(found)
])

])

dnl Autoconf macros for libmcrypt
dnl $id$

# This script detects libmcrypt version and defines
# LIBMCRYPT_CFLAGS, LIBMCRYPT_LIBS
# and LIBMCRYPT24 or LIBMCRYPT22 depending on libmcrypt version
# found.

# Modified for LIBMCRYPT -- nmav
# Configure paths for LIBGCRYPT
# Shamelessly stolen from the one of XDELTA by Owen Taylor
# Werner Koch   99-12-09

dnl AM_PATH_LIBMCRYPT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libmcrypt, and define LIBMCRYPT_CFLAGS and LIBMCRYPT_LIBS
dnl
AC_DEFUN(AM_PATH_LIBMCRYPT,
[dnl
dnl Get the cflags and libraries from the libmcrypt-config script
dnl
AC_ARG_WITH(libmcrypt-prefix,
          [  --with-libmcrypt-prefix=PFX   Prefix where libmcrypt is installed (optional)],
          libmcrypt_config_prefix="$withval", libmcrypt_config_prefix="")

  if test x$libmcrypt_config_prefix != x ; then
     libmcrypt_config_args="$libmcrypt_config_args --prefix=$libmcrypt_config_prefix"
     if test x${LIBMCRYPT_CONFIG+set} != xset ; then
        LIBMCRYPT_CONFIG=$libmcrypt_config_prefix/bin/libmcrypt-config
     fi
  fi

  AC_PATH_PROG(LIBMCRYPT_CONFIG, libmcrypt-config, no)
  min_libmcrypt_version=ifelse([$1], ,2.4.0,$1)
  AC_MSG_CHECKING(for libmcrypt - version >= $min_libmcrypt_version)
  no_libmcrypt=""
  if test "$LIBMCRYPT_CONFIG" = "no" ; then
dnl libmcrypt-config was not found (pre 2.4.11 versions)
dnl Try to detect libmcrypt version
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>

int
main ()
{
#if MCRYPT_API_VERSION <= 19991015 
/* version 2.2 */
    return 0;
#else
/* version 2.4 */
    return 1;
#endif /* 19991015 */
}
],  libmcrypt_config_version="2.2.0"
    if test x$libmcrypt_config_prefix != x ; then
	TTLIBS="-L${libmcrypt_config_prefix}/libs"
	TTINCLUDE="-I${libmcrypt_config_prefix}/include"
    fi
    LIBMCRYPT_CFLAGS="${TTINCLUDE}"
    LIBMCRYPT_LIBS="${TTLIBS} -lmcrypt"
    AC_DEFINE(LIBMCRYPT22)

,   libmcrypt_config_version="2.4.0"
    if test x$libmcrypt_config_prefix != x ; then
	TTLIBS="-L${libmcrypt_config_prefix}/libs"
	TTINCLUDE="-I${libmcrypt_config_prefix}/include"
    fi
    LIBMCRYPT_CFLAGS="${TTINCLUDE}"
    LIBMCRYPT_LIBS="${TTLIBS} -lmcrypt -lltdl"
    AC_DEFINE(LIBMCRYPT24))
  else
dnl libmcrypt-config was found
    LIBMCRYPT_CFLAGS=`$LIBMCRYPT_CONFIG $libmcrypt_config_args --cflags`
    LIBMCRYPT_LIBS=`$LIBMCRYPT_CONFIG $libmcrypt_config_args --libs`
    libmcrypt_config_version=`$LIBMCRYPT_CONFIG $libmcrypt_config_args --version`
    AC_DEFINE(LIBMCRYPT24)
  fi

  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"
  CFLAGS="$CFLAGS $LIBMCRYPT_CFLAGS"
  LIBS="$LIBS $LIBMCRYPT_LIBS"

dnl
dnl Now check if the installed libmcrypt is sufficiently new. Also sanity
dnl checks the results of libmcrypt-config to some extent
dnl
      rm -f conf.libmcrypttest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>

#define TWO "2.2"

int
main ()
{
#if MCRYPT_API_VERSION <= 20010201

#if MCRYPT_API_VERSION <= 19991015 
/* version 2.2 */
    int x = mcrypt_get_key_size(MCRYPT_TWOFISH_128);
    system ("touch conf.libmcrypttest");

    if( strncmp( TWO, "$min_libmcrypt_version", strlen(TWO))) {
      printf("\n*** Requested libmcrypt %s, but LIBMCRYPT (%s)\n",
             "$min_libmcrypt_version", TWO );
      printf("*** was found!\n"); 
      return 1;
    }
    return 0;
#else
/* version 2.4 before 11 */
    MCRYPT td = mcrypt_module_open("twofish", NULL, "cbc", NULL);
    system ("touch conf.libmcrypttest");
    mcrypt_module_close(td);

    return 0;
#endif /* 19991015 */

#else

    system ("touch conf.libmcrypttest");

    if( strcmp( mcrypt_check_version(NULL), "$libmcrypt_config_version" ) )
    {
      printf("\n*** 'libmcrypt-config --version' returned %s, but LIBMCRYPT (%s)\n",
             "$libmcrypt_config_version", mcrypt_check_version(NULL) );
      printf("*** was found! If libmcrypt-config was correct, then it is best\n");
      printf("*** to remove the old version of LIBMCRYPT. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If libmcrypt-config was wrong, set the environment variable LIBMCRYPT_CONFIG\n");
      printf("*** to point to the correct copy of libmcrypt-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    }
    else if ( strcmp(mcrypt_check_version(NULL), LIBMCRYPT_VERSION ) )
    {
      printf("\n*** LIBMCRYPT header file (version %s) does not match\n", LIBMCRYPT_VERSION);
      printf("*** library (version %s)\n", mcrypt_check_version(NULL) );
    }
    else
    {
      if ( mcrypt_check_version( "$min_libmcrypt_version" ) )
      {
        return 0;
      }
     else
      {
        printf("no\n*** An old version of LIBMCRYPT (%s) was found.\n",
                mcrypt_check_version(NULL) );
        printf("*** You need a version of LIBMCRYPT newer than %s. The latest version of\n",
               "$min_libmcrypt_version" );
        printf("*** LIBMCRYPT is always available from ftp://mcrypt.hellug.gr/pub/mcrypt.\n");
        printf("*** \n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the libmcrypt-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of LIBMCRYPT, but you can also set the LIBMCRYPT_CONFIG environment to point to the\n");
        printf("*** correct copy of libmcrypt-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;

#endif /* 20010201 */

}
],, no_libmcrypt=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"


  if test "x$no_libmcrypt" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     if test -f conf.libmcrypttest ; then
        :
     else
        AC_MSG_RESULT(no)
     fi
     
     if test -f conf.libmcrypttest ; then
        :
     else
          echo "*** Could not run libmcrypt test program, checking why..."
          CFLAGS="$CFLAGS $LIBMCRYPT_CFLAGS"
          LIBS="$LIBS $LIBMCRYPT_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>
],      [ 
#if MCRYPT_API_VERSION <= 20010201

#if MCRYPT_API_VERSION <= 19991015 
/* version 2.2 */
    int x = mcrypt_get_key_size(MCRYPT_TWOFISH_128);
    return 0;
#else
/* version 2.4 before 11 */
    MCRYPT td = mcrypt_module_open("twofish", NULL, "cbc", NULL);
    mcrypt_module_close(td);
    return 0;
#endif /* 19991015 */
#else

return !!mcrypt_check_version(NULL); 

#endif /* 20010201 */

],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding LIBMCRYPT or finding the wrong"
          echo "*** version of LIBMCRYPT. If it is not finding LIBMCRYPT, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means LIBMCRYPT was incorrectly installed"
          echo "*** or that you have moved LIBMCRYPT since it was installed. In the latter case, you"
          echo "*** may want to edit the libmcrypt-config script: $LIBMCRYPT_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
     fi
     
     LIBMCRYPT_CFLAGS=""
     LIBMCRYPT_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBMCRYPT_CFLAGS)
  AC_SUBST(LIBMCRYPT_LIBS)
])

dnl ACX_PATH_LIBESMTP([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for LIBESMTP, and define LIBESMTP_CFLAGS and LIBESMTP_LIBS
dnl
AC_DEFUN(ACX_PATH_LIBESMTP,
[dnl 
dnl Get the cflags and libraries from the libesmtp-config script
dnl
AC_ARG_WITH(libesmtp-prefix,[  --with-libesmtp-prefix=PFX   Prefix where LIBESMTP is installed (optional)],
            libesmtp_config_prefix="$withval", libesmtp_config_prefix="")
AC_ARG_WITH(libesmtp-exec-prefix,[  --with-libesmtp-exec-prefix=PFX Exec prefix where LIBESMTP is installed (optional)],
            libesmtp_config_exec_prefix="$withval", libesmtp_config_exec_prefix="")
AC_ARG_ENABLE(libesmtptest, [  --disable-libesmtptest       Do not try to compile and run a test LIBESMTP program],
		    , enable_libesmtptest=yes)

  if test x$libesmtp_config_exec_prefix != x ; then
     libesmtp_config_args="$libesmtp_config_args --exec-prefix=$libesmtp_config_exec_prefix"
     if test x${LIBESMTP_CONFIG+set} != xset ; then
        LIBESMTP_CONFIG=$libesmtp_config_exec_prefix/bin/libesmtp-config
     fi
  fi
  if test x$libesmtp_config_prefix != x ; then
     libesmtp_config_args="$libesmtp_config_args --prefix=$libesmtp_config_prefix"
     if test x${LIBESMTP_CONFIG+set} != xset ; then
        LIBESMTP_CONFIG=$libesmtp_config_prefix/bin/libesmtp-config
     fi
  fi

  AC_PATH_PROG(LIBESMTP_CONFIG, libesmtp-config, no)
  min_libesmtp_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for LIBESMTP - version >= $min_libesmtp_version)
  no_libesmtp=""
  if test "$LIBESMTP_CONFIG" = "no" ; then
    no_libesmtp=yes
  else
    LIBESMTP_CFLAGS=`$LIBESMTP_CONFIG $libesmtp_config_args --cflags`
    LIBESMTP_LIBS=`$LIBESMTP_CONFIG $libesmtp_config_args --libs`
    libesmtp_config_major_version=`$LIBESMTP_CONFIG $libesmtp_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libesmtp_config_minor_version=`$LIBESMTP_CONFIG $libesmtp_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    libesmtp_config_micro_version=`$LIBESMTP_CONFIG $libesmtp_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_libesmtptest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $LIBESMTP_CFLAGS"
      LIBS="$LIBESMTP_LIBS $LIBS"
dnl
dnl Now check if the installed LIBESMTP is sufficiently new. (Also sanity
dnl checks the results of libesmtp-config to some extent
dnl
      rm -f conf.libesmtptest
      AC_TRY_RUN([
#include <libesmtp/libesmtp.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.libesmtptest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_libesmtp_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_libesmtp_version");
     exit(1);
   }

  if ((libesmtp_major_version != $libesmtp_config_major_version) ||
      (libesmtp_minor_version != $libesmtp_config_minor_version) ||
      (libesmtp_micro_version != $libesmtp_config_micro_version))
    {
      printf("\n*** 'libesmtp-config --version' returned %d.%d.%d, but LIBESMTP+ (%d.%d.%d)\n", 
             $libesmtp_config_major_version, $libesmtp_config_minor_version, $libesmtp_config_micro_version,
             libesmtp_major_version, libesmtp_minor_version, libesmtp_micro_version);
      printf ("*** was found! If libesmtp-config was correct, then it is best\n");
      printf ("*** to remove the old version of LIBESMTP+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If libesmtp-config was wrong, set the environment variable LIBESMTP_CONFIG\n");
      printf("*** to point to the correct copy of libesmtp-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (LIBESMTP_MAJOR_VERSION) && defined (LIBESMTP_MINOR_VERSION) && defined (LIBESMTP_MICRO_VERSION)
  else if ((libesmtp_major_version != LIBESMTP_MAJOR_VERSION) ||
	   (libesmtp_minor_version != LIBESMTP_MINOR_VERSION) ||
           (libesmtp_micro_version != LIBESMTP_MICRO_VERSION))
    {
      printf("*** LIBESMTP+ header files (version %d.%d.%d) do not match\n",
	     LIBESMTP_MAJOR_VERSION, LIBESMTP_MINOR_VERSION, LIBESMTP_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     libesmtp_major_version, libesmtp_minor_version, libesmtp_micro_version);
    }
#endif /* defined (LIBESMTP_MAJOR_VERSION) ... */
  else
    {
      if ((libesmtp_major_version > major) ||
        ((libesmtp_major_version == major) && (libesmtp_minor_version > minor)) ||
        ((libesmtp_major_version == major) && (libesmtp_minor_version == minor) && (libesmtp_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of LIBESMTP+ (%d.%d.%d) was found.\n",
               libesmtp_major_version, libesmtp_minor_version, libesmtp_micro_version);
        printf("*** You need a version of LIBESMTP+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** LIBESMTP+ is always available from ftp://ftp.libesmtp.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the libesmtp-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of LIBESMTP+, but you can also set the LIBESMTP_CONFIG environment to point to the\n");
        printf("*** correct copy of libesmtp-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_libesmtp=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_libesmtp" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LIBESMTP_CONFIG" = "no" ; then
       echo "*** The libesmtp-config script installed by LIBESMTP could not be found"
       echo "*** If LIBESMTP was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LIBESMTP_CONFIG environment variable to the"
       echo "*** full path to libesmtp-config."
     else
       if test -f conf.libesmtptest ; then
        :
       else
          echo "*** Could not run LIBESMTP test program, checking why..."
          CFLAGS="$CFLAGS $LIBESMTP_CFLAGS"
          LIBS="$LIBS $LIBESMTP_LIBS"
          AC_TRY_LINK([
#include <libesmtp/libesmtp.h>
#include <stdio.h>
],      [ return ((libesmtp_major_version) || (libesmtp_minor_version) || (libesmtp_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding LIBESMTP or finding the wrong"
          echo "*** version of LIBESMTP. If it is not finding LIBESMTP, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the LIBESMTP package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps libesmtp libesmtp-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means LIBESMTP was incorrectly installed"
          echo "*** or that you have moved LIBESMTP since it was installed. In the latter case, you"
          echo "*** may want to edit the libesmtp-config script: $LIBESMTP_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LIBESMTP_CFLAGS=""
     LIBESMTP_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBESMTP_CFLAGS)
  AC_SUBST(LIBESMTP_LIBS)
  rm -f conf.libesmtptest
])

dnl Autoconf macros for libgpgme

# Configure paths for GPGME
# Shamelessly stolen from the one of XDELTA by Owen Taylor
# Werner Koch  2000-11-17

dnl AM_PATH_GPGME([MINIMUM-VERSION,
dnl               [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for gpgme, and define GPGME_CFLAGS and GPGME_LIBS
dnl
AC_DEFUN(AM_PATH_GPGME,
[dnl
dnl Get the cflags and libraries from the gpgme-config script
dnl
  AC_ARG_WITH(gpgme-prefix,
   [  --with-gpgme-prefix=PFX   Prefix where gpgme is installed (optional)],
          gpgme_config_prefix="$withval", gpgme_config_prefix="")
  AC_ARG_ENABLE(gpgmetest,
   [  --disable-gpgmetest    Do not try to compile and run a test gpgme program],
          , enable_gpgmetest=yes)

  if test x$gpgme_config_prefix != x ; then
     gpgme_config_args="$gpgme_config_args --prefix=$gpgme_config_prefix"
     if test x${GPGME_CONFIG+set} != xset ; then
        GPGME_CONFIG=$gpgme_config_prefix/bin/gpgme-config
     fi
  fi

  AC_PATH_PROG(GPGME_CONFIG, gpgme-config, no)
  min_gpgme_version=ifelse([$1], ,1.0.0,$1)
  AC_MSG_CHECKING(for GPGME - version >= $min_gpgme_version)
  no_gpgme=""
  if test "$GPGME_CONFIG" = "no" ; then
    no_gpgme=yes
  else
    GPGME_CFLAGS=`$GPGME_CONFIG $gpgme_config_args --cflags`
    GPGME_LIBS=`$GPGME_CONFIG $gpgme_config_args --libs`
    gpgme_config_version=`$GPGME_CONFIG $gpgme_config_args --version`
    if test "x$enable_gpgmetest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GPGME_CFLAGS"
      LIBS="$LIBS $GPGME_LIBS"
dnl
dnl Now check if the installed gpgme is sufficiently new. Also sanity
dnl checks the results of gpgme-config to some extent
dnl
      rm -f conf.gpgmetest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gpgme.h>

int
main ()
{
 system ("touch conf.gpgmetest");

 if( strcmp( gpgme_check_version(NULL), "$gpgme_config_version" ) )
 {
   printf("\n"
"*** 'gpgme-config --version' returned %s, but GPGME (%s) was found!\n",
              "$gpgme_config_version", gpgme_check_version(NULL) );
   printf(
"*** If gpgme-config was correct, then it is best to remove the old\n"
"*** version of GPGME.  You may also be able to fix the error\n"
"*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n"
"*** /etc/ld.so.conf.  Make sure you have run ldconfig if that is\n"
"*** required on your system.\n"
"*** If gpgme-config was wrong, set the environment variable GPGME_CONFIG\n"
"*** to point to the correct copy of gpgme-config, \n"
"*** and remove the file config.cache before re-running configure\n"
        );
 }
 else if ( strcmp(gpgme_check_version(NULL), GPGME_VERSION ) )
 {
   printf("\n*** GPGME header file (version %s) does not match\n",
            GPGME_VERSION);
   printf("*** library (version %s)\n", gpgme_check_version(NULL) );
 }
 else
 {
        if ( gpgme_check_version( "$min_gpgme_version" ) )
             return 0;
  printf("no\n"
"*** An old version of GPGME (%s) was found.\n", gpgme_check_version(NULL) );
  printf(
"*** You need a version of GPGME newer than %s.\n", "$min_gpgme_version" );
  printf(
"*** The latest version of GPGME is always available at\n"
"***      ftp://ftp.gnupg.org/pub/gcrypt/alpha/gpgme/\n"
"*** \n"
"*** If you have already installed a sufficiently new version, this error\n"
"*** probably means that the wrong copy of the gpgme-config shell script is\n"
"*** being found. The easiest way to fix this is to remove the old version\n"
"*** of GPGME, but you can also set the GPGME_CONFIG environment to point to\n"
"*** the correct copy of gpgme-config. (In this case, you will have to\n"
"*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n"
"*** so that the correct libraries are found at run-time).\n"
      );
    }
  return 1;
}
],, no_gpgme=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gpgme" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     if test -f conf.gpgmetest ; then
        :
     else
        AC_MSG_RESULT(no)
     fi
     if test "$GPGME_CONFIG" = "no" ; then
       echo "*** The gpgme-config script installed by GPGME could not be found"
       echo "*** If GPGME was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GPGME_CONFIG environment variable to the"
       echo "*** full path to gpgme-config."
     else
       if test -f conf.gpgmetest ; then
        :
       else
          echo "*** Could not run gpgme test program, checking why..."
          CFLAGS="$CFLAGS $GPGME_CFLAGS"
          LIBS="$LIBS $GPGME_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gpgme.h>
],      [ gpgme_check_version(NULL); return 0 ],
        [ 
echo "*** The test program compiled, but did not run. This usually means"
echo "*** that the run-time linker is not finding GPGME or finding the wrong"
echo "*** version of GPGME. If it is not finding GPGME, you'll need to set your"
echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
echo "*** to the installed location  Also, make sure you have run ldconfig if"
echo "*** that is required on your system"
echo "***"
echo "*** If you have an old version installed, it is best to remove it,"
echo "*** although you may also be able to get things to work by"
echo "*** modifying LD_LIBRARY_PATH"
echo "***"
        ],
        [
echo "*** The test program failed to compile or link. See the file config.log"
echo "*** for the exact error that occured. This usually means GPGME was"
echo "*** incorrectly installed or that you have moved GPGME since it was"
echo "*** installed. In the latter case, you may want to edit the"
echo "*** gpgme-config script: $GPGME_CONFIG" 
        ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GPGME_CFLAGS=""
     GPGME_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GPGME_CFLAGS)
  AC_SUBST(GPGME_LIBS)
  AC_DEFINE(HAVE_LIBGPGME)
  rm -f conf.gpgmetest
])

# Configure paths for LIBXML2
# Toshio Kuratomi 2001-04-21
# Adapted from:
# Configure paths for GLIB
# Owen Taylor     97-11-3

dnl AM_PATH_XML([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for XML, and define XML_CFLAGS and XML_LIBS
dnl
AC_DEFUN(AM_PATH_XML,[ 
AC_ARG_WITH(xml-prefix,
            [  --with-xml-prefix=PFX   Prefix where libxml is installed (optional)],
            xml_config_prefix="$withval", xml_config_prefix="")
AC_ARG_WITH(xml-exec-prefix,
            [  --with-xml-exec-prefix=PFX Exec prefix where libxml is installed (optional)],
            xml_config_exec_prefix="$withval", xml_config_exec_prefix="")
AC_ARG_ENABLE(xmltest,
              [  --disable-xmltest       Do not try to compile and run a test LIBXML program],,
              enable_xmltest=yes)

  if test x$xml_config_exec_prefix != x ; then
     xml_config_args="$xml_config_args --exec-prefix=$xml_config_exec_prefix"
     if test x${XML_CONFIG+set} != xset ; then
        XML_CONFIG=$xml_config_exec_prefix/bin/xml-config
     fi
  fi
  if test x$xml_config_prefix != x ; then
     xml_config_args="$xml_config_args --prefix=$xml_config_prefix"
     if test x${XML_CONFIG+set} != xset ; then
        XML_CONFIG=$xml_config_prefix/bin/xml-config
     fi
  fi

  AC_PATH_PROG(XML_CONFIG, xml-config, no)
  min_xml_version=ifelse([$1], ,1.0.0,[$1])
  AC_MSG_CHECKING(for libxml - version >= $min_xml_version)
  no_xml=""
  if test "$XML_CONFIG" = "no" ; then
    no_xml=yes
  else
    XML_CFLAGS=`$XML_CONFIG $xml_config_args --cflags`
    XML_LIBS=`$XML_CONFIG $xml_config_args --libs`
    xml_config_major_version=`$XML_CONFIG $xml_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    xml_config_minor_version=`$XML_CONFIG $xml_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    xml_config_micro_version=`$XML_CONFIG $xml_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_xmltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $XML_CFLAGS"
      LIBS="$XML_LIBS $LIBS"
dnl
dnl Now check if the installed libxml is sufficiently new.
dnl (Also sanity checks the results of xml-config to some extent)
dnl
      rm -f conf.xmltest
      AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libxml/tree.h>

int 
main()
{
  int xml_major_version, xml_minor_version, xml_micro_version;
  int major, minor, micro;
  char *tmp_version;
  int tmp_int_version;

  system("touch conf.xmltest");

  /* Capture xml-config output via autoconf/configure variables */
  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = (char *)strdup("$min_xml_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string from xml-config\n", "$min_xml_version");
     exit(1);
   }
   free(tmp_version);

   /* Capture the version information from the header files */
   tmp_int_version = LIBXML_VERSION;
   xml_major_version=tmp_int_version / 10000;
   xml_minor_version=(tmp_int_version - xml_major_version * 10000) / 100;
   xml_micro_version=(tmp_int_version - xml_minor_version * 100 - xml_major_version * 10000);

 /* Compare xml-config output to the libxml headers */
  if ((xml_major_version != $xml_config_major_version) ||
      (xml_minor_version != $xml_config_minor_version)
#if 0
      ||
/* The last released version of libxml-1.x has an incorrect micro version in
 * the header file so neither the includes nor the library will match the
 * micro_version to the output of xml-config
 */
      (xml_micro_version != $xml_config_micro_version)
#endif 
	  )
	  
    {
      printf("*** libxml header files (version %d.%d.%d) do not match\n",
         xml_major_version, xml_minor_version, xml_micro_version);
      printf("*** xml-config (version %d.%d.%d)\n",
         $xml_config_major_version, $xml_config_minor_version, $xml_config_micro_version);
      return 1;
    } 
/* Compare the headers to the library to make sure we match */
  /* Less than ideal -- doesn't provide us with return value feedback, 
   * only exits if there's a serious mismatch between header and library.
   */
    LIBXML_TEST_VERSION;

    /* Test that the library is greater than our minimum version */
    if (($xml_config_major_version > major) ||
        (($xml_config_major_version == major) && ($xml_config_minor_version > minor)) ||
        (($xml_config_major_version == major) && ($xml_config_minor_version == minor) &&
        ($xml_config_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of libxml (%d.%d.%d) was found.\n",
               xml_major_version, xml_minor_version, xml_micro_version);
        printf("*** You need a version of libxml newer than %d.%d.%d. The latest version of\n",
           major, minor, micro);
        printf("*** libxml is always available from ftp://ftp.xmlsoft.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the xml-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of LIBXML, but you can also set the XML_CONFIG environment to point to the\n");
        printf("*** correct copy of xml-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
    }
  return 1;
}
],, no_xml=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi

  if test "x$no_xml" = x ; then
     AC_MSG_RESULT(yes (version $xml_config_major_version.$xml_config_minor_version.$xml_config_micro_version))
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$XML_CONFIG" = "no" ; then
       echo "*** The xml-config script installed by LIBXML could not be found"
       echo "*** If libxml was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the XML_CONFIG environment variable to the"
       echo "*** full path to xml-config."
     else
       if test -f conf.xmltest ; then
        :
       else
          echo "*** Could not run libxml test program, checking why..."
          CFLAGS="$CFLAGS $XML_CFLAGS"
          LIBS="$LIBS $XML_LIBS"
          AC_TRY_LINK([
#include <libxml/tree.h>
#include <stdio.h>
],      [ LIBXML_TEST_VERSION; return 0;],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding LIBXML or finding the wrong"
          echo "*** version of LIBXML. If it is not finding LIBXML, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means LIBXML was incorrectly installed"
          echo "*** or that you have moved LIBXML since it was installed. In the latter case, you"
          echo "*** may want to edit the xml-config script: $XML_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi

     XML_CFLAGS=""
     XML_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(XML_CFLAGS)
  AC_SUBST(XML_LIBS)
  rm -f conf.xmltest
])

# Configure paths for LIBXML2
# Toshio Kuratomi 2001-04-21
# Adapted from:
# Configure paths for GLIB
# Owen Taylor     97-11-3

dnl AM_PATH_XML2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for XML, and define XML_CFLAGS and XML_LIBS
dnl
AC_DEFUN(AM_PATH_XML2,[ 
AC_ARG_WITH(xml-prefix,
            [  --with-xml-prefix=PFX   Prefix where libxml is installed (optional)],
            xml_config_prefix="$withval", xml_config_prefix="")
AC_ARG_WITH(xml-exec-prefix,
            [  --with-xml-exec-prefix=PFX Exec prefix where libxml is installed (optional)],
            xml_config_exec_prefix="$withval", xml_config_exec_prefix="")
AC_ARG_ENABLE(xmltest,
              [  --disable-xmltest       Do not try to compile and run a test LIBXML program],,
              enable_xmltest=yes)

  if test x$xml_config_exec_prefix != x ; then
     xml_config_args="$xml_config_args --exec-prefix=$xml_config_exec_prefix"
     if test x${XML2_CONFIG+set} != xset ; then
        XML2_CONFIG=$xml_config_exec_prefix/bin/xml2-config
     fi
  fi
  if test x$xml_config_prefix != x ; then
     xml_config_args="$xml_config_args --prefix=$xml_config_prefix"
     if test x${XML2_CONFIG+set} != xset ; then
        XML2_CONFIG=$xml_config_prefix/bin/xml2-config
     fi
  fi

  AC_PATH_PROG(XML2_CONFIG, xml2-config, no)
  min_xml_version=ifelse([$1], ,2.0.0,[$1])
  AC_MSG_CHECKING(for libxml - version >= $min_xml_version)
  no_xml=""
  if test "$XML2_CONFIG" = "no" ; then
    no_xml=yes
  else
    XML_CFLAGS=`$XML2_CONFIG $xml_config_args --cflags`
    XML_LIBS=`$XML2_CONFIG $xml_config_args --libs`
    xml_config_major_version=`$XML2_CONFIG $xml_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    xml_config_minor_version=`$XML2_CONFIG $xml_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    xml_config_micro_version=`$XML2_CONFIG $xml_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_xmltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $XML_CFLAGS"
      LIBS="$XML_LIBS $LIBS"
dnl
dnl Now check if the installed libxml is sufficiently new.
dnl (Also sanity checks the results of xml2-config to some extent)
dnl
      rm -f conf.xmltest
      AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libxml/xmlversion.h>

int 
main()
{
  int xml_major_version, xml_minor_version, xml_micro_version;
  int major, minor, micro;
  char *tmp_version;

  system("touch conf.xmltest");

  /* Capture xml2-config output via autoconf/configure variables */
  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = (char *)strdup("$min_xml_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string from xml2-config\n", "$min_xml_version");
     exit(1);
   }
   free(tmp_version);

   /* Capture the version information from the header files */
   tmp_version = (char *)strdup(LIBXML_DOTTED_VERSION);
   if (sscanf(tmp_version, "%d.%d.%d", &xml_major_version, &xml_minor_version, &xml_micro_version) != 3) {
     printf("%s, bad version string from libxml includes\n", "LIBXML_DOTTED_VERSION");
     exit(1);
   }
   free(tmp_version);

 /* Compare xml2-config output to the libxml headers */
  if ((xml_major_version != $xml_config_major_version) ||
      (xml_minor_version != $xml_config_minor_version) ||
      (xml_micro_version != $xml_config_micro_version))
    {
      printf("*** libxml header files (version %d.%d.%d) do not match\n",
         xml_major_version, xml_minor_version, xml_micro_version);
      printf("*** xml2-config (version %d.%d.%d)\n",
         $xml_config_major_version, $xml_config_minor_version, $xml_config_micro_version);
      return 1;
    } 
/* Compare the headers to the library to make sure we match */
  /* Less than ideal -- doesn't provide us with return value feedback, 
   * only exits if there's a serious mismatch between header and library.
   */
    LIBXML_TEST_VERSION;

    /* Test that the library is greater than our minimum version */
    if ((xml_major_version > major) ||
        ((xml_major_version == major) && (xml_minor_version > minor)) ||
        ((xml_major_version == major) && (xml_minor_version == minor) &&
        (xml_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of libxml (%d.%d.%d) was found.\n",
               xml_major_version, xml_minor_version, xml_micro_version);
        printf("*** You need a version of libxml newer than %d.%d.%d. The latest version of\n",
           major, minor, micro);
        printf("*** libxml is always available from ftp://ftp.xmlsoft.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the xml2-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of LIBXML, but you can also set the XML2_CONFIG environment to point to the\n");
        printf("*** correct copy of xml2-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
    }
  return 1;
}
],, no_xml=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi

  if test "x$no_xml" = x ; then
     AC_MSG_RESULT(yes (version $xml_config_major_version.$xml_config_minor_version.$xml_config_micro_version))
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$XML2_CONFIG" = "no" ; then
       echo "*** The xml2-config script installed by LIBXML could not be found"
       echo "*** If libxml was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the XML2_CONFIG environment variable to the"
       echo "*** full path to xml2-config."
     else
       if test -f conf.xmltest ; then
        :
       else
          echo "*** Could not run libxml test program, checking why..."
          CFLAGS="$CFLAGS $XML_CFLAGS"
          LIBS="$LIBS $XML_LIBS"
          AC_TRY_LINK([
#include <libxml/xmlversion.h>
#include <stdio.h>
],      [ LIBXML_TEST_VERSION; return 0;],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding LIBXML or finding the wrong"
          echo "*** version of LIBXML. If it is not finding LIBXML, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means LIBXML was incorrectly installed"
          echo "*** or that you have moved LIBXML since it was installed. In the latter case, you"
          echo "*** may want to edit the xml2-config script: $XML2_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi

     XML_CFLAGS=""
     XML_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(XML_CFLAGS)
  AC_SUBST(XML_LIBS)
  rm -f conf.xmltest
])

dnl *-*wedit:notab*-*  Please keep this as the last line.
