/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001 The Caudium Group
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* Curses/Ncurses Support - This module adds Curses support to Pike. */

#include "global.h"
#include "caudium_util.h"

#include "curses_config.h"

#ifdef HAVE_CURSES

struct screen {
  SCREEN *scr;
};

#define THISSCR ((struct screen *)(fp->current_storage))

static void init_screen_struct(struct object *o)
{
  THISSCR->scr = stdterm;
}

static void exit_screen_struct(struct object *o)
{
  SCREEN *prev;
  if (THISSCR->scr != NULL)
  {
    prev = set_term(THISSCR->scr);
    endwin();
    set_term(prev);
    delscreen(THISSCR->scr);
  }
  else
  {
    endwin();
    fprintf(stderr,"ENDWIN (exit-screen)\n");
  }
}

static void f_screen_create(INT32 args)
{
  if (args > 0)
    Pike_error("Can't create screens yet\n");
  pop_n_elems(args);
}

void init_screen_program(void)
{
  start_new_program();

  ADD_STORAGE(struct screen);

  set_init_callback(init_screen_struct);
  set_exit_callback(exit_screen_struct);

  ADD_FUNCTION("create", f_screen_create, tFunc(tOr(tString, tVoid), tVoid), 0);

  curses_screen_program = end_program();
  add_program_constant("Screen", curses_screen_program, 0);
}
#endif /* HAVE_CURSES */

