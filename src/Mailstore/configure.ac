AC_INIT(mailstore_main.c)
AC_CONFIG_HEADER(mailstore_config.h)

CAUDIUM_MODULE_INIT([Mailstore])

AH_BOTTOM([
void pike_module_init(void);
void pike_module_exit(void);
])

AC_PATH_PROG(PCRE_CONFIG, pcre-config,[none])
if test -z "$PCRE_CONFIG" -o "x$PCRE_CONFIG" = "xnone"; then
    AC_MSG_ERROR([PCRE is required to compile Mailstore])
fi
PCRE_LIBS="`$PCRE_CONFIG --libs-posix`"
PCRE_CFLAGS="`$PCRE_CONFIG --cflags`"

AC_SUBST(PCRE_LIBS)
AC_SUBST(PCRE_CFLAGS)

AC_CONFIG_SUBDIRS(libmutt)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
dnl vi: set ts=3:
