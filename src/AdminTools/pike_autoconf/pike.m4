dnl $Id$
define([AP_CHECK_PIKE],
[
### Pike Detection ###
######################

AC_ARG_WITH(pike, 
[  --with-pike[=binary]    Use the specified Pike. ],
[
    if test ! -z "$withval" -a "$withval" != "yes"; then 
		if test -f "$withval" -a  ! -x "$withval" ; then
			echo "$withval is not an executable file"
 			exit 1
		elif test -x "$withval" -a -f "$withval"; then
			DEFPIKE="$withval"
		else
			echo "$withval doesn't exist or isn't an executable file."
 			exit 1
		fi
	fi
])
RESULT=no
AC_MSG_CHECKING(for a working Pike)
AC_MSG_RESULT( )
pathpike="`type  pike |sed 's/pike is//' 2>/dev/null`"
if test "$prefix" != "NONE"; then
  PREFIXPIKE="$prefix/bin/pike"
fi
for a in $DEFPIKE $PREFIXPIKE $pathpike /usr/local/bin/pike /opt/pike/bin/pike \
  /sw/local/bin/pike /opt/local/bin/pike /usr/gnu/bin/pike /usr/bin/pike ; do
  if test  "x$PIKE" != "x" ; then
    break;
  fi
  AC_MSG_CHECKING(${a})
  if test -x ${a}; then
    PIKE="${a}"
    if $PIKE -e 'float v; int rel;sscanf(version(), "Pike v%f release %d", v, rel);v += rel/10000.0; if(v < 0.6116) exit(1); exit(0);'; then
		PIKE_MODULE_DIR="`$PIKE --show-paths 2>&1| grep lib|grep modules|head -1 | sed -e 's/.*: //'`"
		PIKE_INCLUDE_DIRS="-I`echo "$PIKE_MODULE_DIR" | sed -e 's,lib/pike/modules,include/pike,' -e 's,lib/modules,include/pike,'`"

		if test -z "$PIKE_INCLUDE_DIRS" -o -z "$PIKE_MODULE_DIR"; then
			AC_MSG_RESULT(no dirs found)
			PIKE=""
		else
		   AC_MSG_RESULT(ok)
		   PIKE_C_INCLUDE=/usr/include/`basename ${PIKE}`
		   AC_MSG_CHECKING(for C includes in ${PIKE_C_INCLUDE})
		   if test -d $PIKE_C_INCLUDE; then
			PIKE_INCLUDE_DIRS="-I$PIKE_C_INCLUDE $PIKE_INCLUDE_DIRS"
			AC_MSG_RESULT(found)
		   else
			AC_MSG_RESULT(not found)
		   fi
		fi
	else
		AC_MSG_RESULT(too old)
		PIKE=""
	fi
  else
    AC_MSG_RESULT(no)
  fi
done
if test "$PIKE" != ""; then 
  tmppike=`ls -l $PIKE | awk -F ' -> ' '{ print $2 }' 2>/dev/null`
  if test -x "$tmppike"; then
 	PIKE=$tmppike
  fi
  PIKE_VERSION=`$PIKE -e 'string v; int rel;sscanf(version(), "Pike v%s release %d", v, rel); write(v+"."+rel);'`
else
  echo "Failed to find a suitable pike!"
  exit 1;
fi

export PIKE PIKE_INCLUDE_DIRS PIKE_VERSION
AC_SUBST(PIKE)
AC_SUBST(PIKE_VERSION)
AC_SUBST(PIKE_INCLUDE_DIRS)
AC_SUBST(PIKE_MODULE_DIR)
#############################################################################
])
