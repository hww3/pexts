AC_INIT(index.c)
AC_CONFIG_HEADER(avs_config.h)

CAUDIUM_MODULE_INIT([AVS])

AC_ARG_WITH(avs,  [  --with-avs=path       Enable support for the Altavista search engine],[
  AC_MSG_CHECKING(for AVS)

  if test x$withval = xyes; then
	AC_MSG_ERROR("You need to specify a location of the AVS SDK");
    exit 1
  else
    dir="$withval"
  fi
  AC_MSG_RESULT($dir)

  CVTOBJ="convert.o"
  CVTSRC="convert.c"
  SDKDIR="$dir"
  OBJS="index.o search.o count.o phrase.o spell.o stem.o thesaurus.o $CVTOBJ"
  SOURCES="index.c search.c count.c phrase.c spell.c stem.c thesaurus.c $CVTSRC"
  SDKINCLUDE="$SDKDIR/include"
  SDKLIBDIR="$SDKDIR/bin"
  CVTLIBS="$SDKLIBDIR/libavscvt30.a -lsc_da -lsc_ta"
  LINGLIBS="$SDKLIBDIR/libavsl30_r.a"
  SDKLIBS="$SDKLIBDIR/libavs30_r.a $CVTLIBS $LINGLIBS"
  MODULE_LDFLAGS="-R$SDKLIBDIR -L$SDKLIBDIR -R$SDKLIBDIR/doc_converters -L$SDKLIBDIR/doc_converters"
  MODULE_LIBS="$SDKLIBS $CVTLIBS $LINGLIBS -lpthread -lm -u lstat -u stat -u fstat"
  MODULE_CFLAGS="-DPTHREADS -DDOC_CONVERTERS -I$SDKINCLUDE"
  AC_DEFINE(HAVE_AVS_SDK, 1, [Define if the AVS SDK is installed.])
], [
  AC_MSG_RESULT(
*** Not compiling Altavista SDK module. Use --with-avs=/dir/
*** to specify the location of the SDK (if available).)
  OBJS=dummy.o
  SOURCES=dummy.c
  SDKDIR=.
  MODULE_LDFLAGS=""
  MODULE_LIBS=""
  MODULE_CFLAGS=""
])
AC_SUBST(OBJS)
AC_SUBST(SOURCES)
AC_SUBST(MODULE_LDFLAGS)
AC_SUBST(MODULE_LIBS)
AC_SUBST(MODULE_CFLAGS)
AC_SUBST(makefile_rules)

AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
