# $Id$
AC_INIT(configure.ac)

CAUDIUM_LOW_MODULE_INIT()

dirs=

# need to specifically enable these modules
if test "$with_AVS" = ""; then
  with_AVS="no"
fi
if test "$with_mnoGo" = ""; then
  with_mnoGo="no"
fi

for a in `(cd $srcdir ; echo *)`
do
  if test "$a" != "CVS" -a "$a" != "RCS" ; then
    if test -d "$srcdir/$a" ; then
      yesno=`eval echo \\$with_$a`
      if test "x${yesno-}" != xno ; then
        dirs="$dirs $a"
        module_names="$module_names $a"
      fi
    fi
  fi
done

AC_CONFIG_SUBDIRS($dirs)

AC_OUTPUT(Makefile module_makefile,[],dirs="$module_names")





