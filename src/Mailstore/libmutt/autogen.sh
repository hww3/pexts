#!/bin/sh

if test ! -f install-sh ; then touch install-sh ; fi

MAKE=
if test ! -x "$MAKE" ; then MAKE=`which gmake` ; fi
if test ! -x "$MAKE" ; then MAKE=`which make` ; fi
if test ! -x "$MAKE" ; then MAKE=`which gnumake` ; fi
HAVE_GNU_MAKE=`$MAKE --version|grep -c "Free Software Foundation"`

if test ! -x `which aclocal`  
then echo you need autoconfig and automake to generate the Makefile
fi
if test ! -x `which automake`  
then echo you need automake to generate the Makefile
fi

aclocal
libtoolize --force --copy --automake
aclocal
autoheader
#automake --foreign --copy
automake --gnu --copy --add-missing
autoconf

