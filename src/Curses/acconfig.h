/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001, 2002 The Caudium Group
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

#ifndef CURSES_CONFIG_H
#define CURSES_CONFIG_H

@TOP@

@BOTTOM@

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
#endif
