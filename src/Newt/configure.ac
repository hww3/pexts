AC_INIT(newt.c)
AC_CONFIG_HEADER(newt_config.h)

CAUDIUM_MODULE_INIT()

AH_BOTTOM([
#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)
#define HAVE_NEWT
#include <newt.h>
#define THIS ((struct formobj *)Pike_fp->current_storage)

typedef struct formdata {
	newtComponent elem;
	char *nev;
	char *value;
	struct formdata *next;
} FormData;


struct formobj {
	newtComponent   obj;
	FormData *first;
	FormData *last;
};


#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif

int store_component( newtComponent elem, char *, char *);
#endif
/* pike module functions */
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_HEADERS( newt.h )
AC_CHECK_LIB(newt, newtInit)


AC_OUTPUT(Makefile.pre, [${MAKE:-make} -f Makefile.pre depend])


dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
	
