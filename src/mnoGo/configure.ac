AC_INIT(mnogo.c)
AC_CONFIG_HEADER(mnogo_config.h)

CAUDIUM_MODULE_INIT([mnoGo])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AC_ARG_WITH(mnoGo,[  --with-mnoGo[=DIR]      Include mnoGoSearch support.  DIR is the mnoGoSearch 
                          base install directory, defaults to 
                          /usr/local/mnogosearch.], [
  if test "$withval" != no; then 
		if test "$withval" = "yes" ; then
			MNOGOSEARCH_BINDIR=/usr/local/mnogosearch/bin
			MNOGOSEARCH_INCDIR=/usr/local/mnogosearch/include
			MNOGOSEARCH_LIBDIR=/usr/local/mnogosearch/lib
		else
			MNOGOSEARCH_BINDIR=$withval/bin
			MNOGOSEARCH_INCDIR=$withval/include
			MNOGOSEARCH_LIBDIR=$withval/lib
		fi
			
		AC_MSG_CHECKING(for mnoGoSearch version)    
			
		if test -x "$MNOGOSEARCH_BINDIR/udm-config"; then
			MNOGOSEARCH_VERSION=`$MNOGOSEARCH_BINDIR/udm-config --version`
			MNOGOSEARCH_VERSION_ID=`$MNOGOSEARCH_BINDIR/udm-config --version-id`
	
			if test $? -ne 0; then
				AC_MSG_RESULT(<= 3.1.9)
				AC_MSG_ERROR(mnoGoSearch 3.1.10 at least required)
			fi
		
			if test "$MNOGOSEARCH_VERSION_ID" -lt 30110; then
				AC_MSG_RESULT(<= 3.1.9)
				AC_MSG_ERROR(mnoGoSearch 3.1.10 at least required)
			fi
		
			AC_MSG_RESULT($MNOGOSEARCH_VERSION)
			LIBS="-L$MNOGOSEARCH_LIBDIR -ludmsearch `$MNOGOSEARCH_BINDIR/udm-config --libs`"
		else
			AC_MSG_ERROR(udm-config script not found in $MNOGOSEARCH_BINDIR)
		fi
			
		CFLAGS="-I$MNOGOSEARCH_INCDIR"
		AC_SUBST(CFLAGS)
    AC_SUBST(LIBS)
		AC_DEFINE(HAVE_MNOGOSEARCH,1,[Define if you have mnogosearch on your system])
	fi
], [echo "*** mnoGoSearch support not enabled!"])

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 2
dnl ## End:
