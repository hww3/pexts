AC_INIT(xslt.c)

AC_SUBST(PIKE_INCLUDE_PREFIX)
AC_SUBST(PIKE)
AC_SUBST(PIKE_VERSION)


# check for libxml2 include directories.
AC_PROG_CPP
AC_MSG_CHECKING(Looking for libxml2 includes)
xmlhdrdir=no
for a in /sw/include /sw/include/libxml2 /usr/include /usr/local/include /opt/include /usr/include/libxml2 /usr/sfw/include /opt/sfw/include /usr/local/include/libxml2; do
    if test -d ${a}; then
	LIBXML2_INCLUDES="${a}"
	AC_CHECK_HEADER(${a}/libxml/xmlversion.h,xmlhdrdir=${a})
    fi
done
if test $xmlhdrdir = no ; then
   AC_MSG_ERROR(Failed to find libxml2 include directory)
fi
AC_MSG_RESULT(XML2 includes in $xmlhdrdir)

LIBXML2_INCLUDE="${xmlhdrdir}"
AC_SUBST(LIBXML2_INCLUDE)


#check for libxslt include directories
AC_MSG_CHECKING(Looking for libxlt includes)
xslhdrdir=no
for a in /sw/include /usr/include /usr/local/include /opt/include /opt/sfw/include; do
    if test -d ${a}; then
	LIBXSL_INCLUDES="${a}"
    AC_CHECK_HEADER(${a}/libxslt/xsltconfig.h,xslhdrdir=${a})
    fi
done

if test $xslhdrdir = no ; then
   AC_MSG_ERROR(Failed to find libxslt include directory)
fi
AC_MSG_RESULT(XSL includes in $xslhdrdir)

LIBXSL_INCLUDE="${xslhdrdir}"
AC_SUBST(LIBXSL_INCLUDE)

AC_MSG_CHECKING(Looking for libxlt library)
xsllibdir=no
AC_ARG_WITH(xslt-libdir,
AC_HELP_STRING([--with-libxslt-includes[=include-dir]],[Use path for xslt includes]),
[
		if test -f $withval/libxslt.so; then
		   xsllibdir=$withval;
		fi
])
if test $xsllibdir=no; then
  for a in /sw/lib /usr/lib /usr/local/lib /opt/lib /lib /opt/sfw/lib /usr/sfw/lib; do
	if test -f ${a}/libxslt.so; then
	   xsllibdir=${a}
	   LIBS="-lxslt"
    fi
  done
fi
AC_SUBST(LIBS)
AC_MSG_RESULT(Found libxslt library in $xsllibdir)

AC_MSG_CHECKING(Looking for libml2 library)
xmllibdir=no
for a in /sw/lib /usr/lib /usr/local/lib /opt/lib /lib /opt/sfw/lib /usr/sfw/lib; do
	if test -f ${a}/libxml2.so; then
	   xmllibdir=${a}
    fi
done
AC_MSG_RESULT(Found libxml2 library in $xmllibdir)
LIBDIR="${xsllibdir}:${xmllibdir}"
AC_SUBST(LIBDIR)

AC_MSG_CHECKING(install Script)
installprg=none
for a in /sw/bin /usr/bin /usr/local/bin /opt/bin /bin /opt/sfw/bin /usr/sfw/bin; do
	if test -x ${a}/install; then
	    installprg=${a}/install
	fi
	if test -x ${a}/ginstall; then
	    installprg="${a}/ginstall"
	fi
done
AC_MSG_RESULT(Found $installprg)
AC_SUBST(installprg)

AC_SUBST(LDSHARED)
AC_SUBST(LDFLAGS)

AC_OUTPUT(Makefile)


dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
