AC_INIT(mhash.c)
AC_CONFIG_HEADER(mhash_config.h)

CAUDIUM_MODULE_INIT([Mhash])

AH_BOTTOM([
#if defined(HAVE_MHASH_H) && defined(HAVE_LIBMHASH)
#define HAVE_MHASH
#include <mhash.h>
#include "mhash_defs.h"
#endif

void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_HEADERS( mhash.h )
AC_CHECK_LIB(mhash, mhash)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
