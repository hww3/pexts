AC_INIT(curses.c)
AC_CONFIG_HEADER(curses_config.h)

CAUDIUM_MODULE_INIT([Curses])

AH_BOTTOM([
#if defined(HAVE_CURSES_H) && defined(HAVE_LIBCURSES)
#define HAVE_CURSES
#include <curses.h>

#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif

#endif
/* pike module functions */

extern struct program *curses_screen_program;
extern struct program *curses_window_program;
extern struct object *curses_mainscr;
extern struct object *curses_rootwin;
extern int curses_inited;
extern SCREEN *stdterm;

void init_window_program(void);
void init_screen_program(void);
void pike_module_init(void);
void pike_module_exit(void);
])

AC_CHECK_HEADERS( curses.h )
AC_CHECK_LIB(curses, initscr)

AC_HAVE_HEADERS(curses.h ncurses.h ncurses/curses.h ncurses/ncurses.h)

AC_SUBST(makefile_rules)
AC_OUTPUT(Makefile)

dnl ## Local Variables:
dnl ## tab-width: 4
dnl ## End:
