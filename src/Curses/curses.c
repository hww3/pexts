/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000 The Caudium Group
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
#include "pexts.h"

#include "curses_config.h"

int curses_inited = 0;

#ifdef HAVE_CURSES

struct program *curses_screen_program = NULL;
struct program *curses_window_program = NULL;
struct object *curses_mainscr = NULL;
struct object *curses_rootwin = NULL;
SCREEN *stdterm = NULL;


static void f_init(INT32 args)
{
  if (args != 0)
    Pike_error("Too many arguments to init()\n");
  pop_n_elems(args);

  if (curses_mainscr != NULL)
    destruct(curses_mainscr);
  if (curses_rootwin != NULL)
    destruct(curses_rootwin);
  curses_mainscr = NULL;
  curses_rootwin = NULL;
  if(stdterm!=NULL)
  {
    endwin();   
    delscreen(stdterm);
  }

  stdterm = newterm(0,stdout,stdin);
  if (!stdterm)
    Pike_error("newterm failed\n");
  set_term(stdterm);
  curses_inited = 1;

  {
    int x,y;
    getmaxyx(stdscr,y,x);
/*     fprintf(stderr,"(%d,%d)\n",x,y); */
  }

  curses_mainscr = clone_object(curses_screen_program, 0);
  curses_rootwin = clone_object(curses_window_program, 0);
}

static void f_root(INT32 args)
{
  if (args > 0)
    Pike_error("Too many arguments to root()\n");
  if (!curses_inited)
    Pike_error("Can't use root() before init()\n");
  pop_n_elems(args);
  curses_rootwin->refs++;
  push_object(curses_rootwin);
}

static void f_endwin(INT32 args)
{
  if (args > 0)
    Pike_error("Too many arguments to endwin()\n");
  if (!curses_inited)
    Pike_error("Can't use endwin() before init()\n");
  pop_n_elems(args);
  endwin();
}

#define VOID_VOID_FN(f) \
static void f_##f(INT32 args) \
{ \
  if (args>0) \
    Pike_error("Too many arguments to " #f "()\n"); \
  if (!curses_inited) \
    Pike_error("Can't use " #f "() before init()\n"); \
  f(); \
}

#define VOID_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  if (args>0) \
    Pike_error("Too many arguments to " #f "()\n"); \
  if (!curses_inited) \
    Pike_error("Can't use " #f "() before init()\n"); \
  push_int(f()); \
}

#define VOID_INT_VAR_FN(f) \
static void f_##f(INT32 args) \
{ \
  if (args>0) \
    Pike_error("Too many arguments to " #f "()\n"); \
  if (!curses_inited) \
    Pike_error("Can't use " #f "() before init()\n"); \
  push_int(f); \
}

#define INT_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  int i; \
  check_all_args(#f, args, BIT_INT, 0); \
  if (!curses_inited) \
    Pike_error("Can't use " #f "() before init()\n"); \
  i = sp[-1].u.integer; \
  pop_n_elems(args); \
  push_int(f(i)); \
}

INT_INT_FN(COLOR_PAIR)

VOID_INT_VAR_FN(ACS_ULCORNER)
VOID_INT_VAR_FN(ACS_LLCORNER)
VOID_INT_VAR_FN(ACS_URCORNER)
VOID_INT_VAR_FN(ACS_LRCORNER)
VOID_INT_VAR_FN(ACS_LTEE)
VOID_INT_VAR_FN(ACS_RTEE)
VOID_INT_VAR_FN(ACS_BTEE)
VOID_INT_VAR_FN(ACS_TTEE)
VOID_INT_VAR_FN(ACS_HLINE)
VOID_INT_VAR_FN(ACS_VLINE)
VOID_INT_VAR_FN(ACS_PLUS)
/* VOID_INT_VAR_FN(ACS_S1) */
/* VOID_INT_VAR_FN(ACS_S9) */
/* VOID_INT_VAR_FN(ACS_DIAMOND) */
/* VOID_INT_VAR_FN(ACS_CKBOARD) */
/* VOID_INT_VAR_FN(ACS_DEGREE) */
/* VOID_INT_VAR_FN(ACS_PLMINUS) */
/* VOID_INT_VAR_FN(ACS_BULLET) */
/* VOID_INT_VAR_FN(ACS_LARROW) */
/* VOID_INT_VAR_FN(ACS_RARROW) */
/* VOID_INT_VAR_FN(ACS_DARROW) */
/* VOID_INT_VAR_FN(ACS_UARROW) */
/* VOID_INT_VAR_FN(ACS_BOARD) */
/* VOID_INT_VAR_FN(ACS_LANTERN) */
/* VOID_INT_VAR_FN(ACS_BLOCK) */
/* VOID_INT_VAR_FN(ACS_S3) */
/* VOID_INT_VAR_FN(ACS_S7) */
/* VOID_INT_VAR_FN(ACS_LEQUAL) */
/* VOID_INT_VAR_FN(ACS_GEQUAL) */
/* VOID_INT_VAR_FN(ACS_PI) */
/* VOID_INT_VAR_FN(ACS_NEQUAL) */
/* VOID_INT_VAR_FN(ACS_STERLING) */
VOID_INT_VAR_FN(ACS_BSSB)
VOID_INT_VAR_FN(ACS_SSBB)
VOID_INT_VAR_FN(ACS_BBSS)
VOID_INT_VAR_FN(ACS_SBBS)
VOID_INT_VAR_FN(ACS_SBSS)
VOID_INT_VAR_FN(ACS_SSSB)
VOID_INT_VAR_FN(ACS_SSBS)
VOID_INT_VAR_FN(ACS_BSSS)
VOID_INT_VAR_FN(ACS_BSBS)
VOID_INT_VAR_FN(ACS_SBSB)
VOID_INT_VAR_FN(ACS_SSSS)

VOID_INT_FN(nl)
VOID_INT_FN(nonl)

VOID_INT_FN(cbreak)
VOID_INT_FN(nocbreak)
VOID_INT_FN(echo)
VOID_INT_FN(noecho)
INT_INT_FN(halfdelay)
VOID_INT_FN(raw)
VOID_INT_FN(noraw)
VOID_VOID_FN(qiflush)
VOID_VOID_FN(noqiflush)
INT_INT_FN(typeahead)

VOID_INT_FN(doupdate)

VOID_VOID_FN(beep)
VOID_VOID_FN(flash)

static void f_getsyx(INT32 args)
{
  struct array *a;
  struct svalue i;
  int x,y;
  if (args > 0)
    Pike_error("Too many arguments to getsyx()\n");
  a = allocate_array(2);
  i.type = T_INT;
  i.subtype = NUMBER_NUMBER;
  getsyx(y,x);
  i.u.integer = y;
  array_set_index(a, 0, &i);
  i.u.integer = x;
  array_set_index(a, 1, &i);
  push_array(a);
}

static void f_setsyx(INT32 args)
{
  int x,y;
  if (args == 0)
    Pike_error("Too few arguments to setsyx\n");
  if (args == 1)
  {
    struct array *a;
    if (sp[-1].type != T_ARRAY)
      Pike_error("Bad argument 1 to setsyx\n");
    a = sp[-1].u.array;
    if (a->size != 2)
      Pike_error("An array argument to setsyze must be of size 2\n");
    if (a->item[0].type != T_INT)
      Pike_error("Element 0 of argument is not an integer\n");
    if (a->item[1].type != T_INT)
      Pike_error("Element 1 of argument is not an integer\n");
    y = a->item[0].u.integer;
    x = a->item[1].u.integer;
    setsyx(y,x);
  }
  else
  {
    check_all_args("setsyx", args, BIT_INT, BIT_INT, 0);
    y = sp[-2].u.integer;
    x = sp[-1].u.integer;
    setsyx(y,x);
  }
  pop_n_elems(args);
}

INT_INT_FN(curs_set)

VOID_INT_FN(start_color)

static void f_init_pair(INT32 args)
{
  int r;
  check_all_args("init_pair", args, BIT_INT, BIT_INT, BIT_INT, 0);
  r = init_pair(sp[-3].u.integer, sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

static void f_init_color(INT32 args)
{
  int r;
  check_all_args("init_color", args, BIT_INT, BIT_INT, BIT_INT, BIT_INT, 0);
  r = init_color(sp[-4].u.integer, sp[-3].u.integer, sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

VOID_INT_FN(has_colors)
VOID_INT_FN(can_change_color)
#endif /* HAVE_CURSES */

#include "pexts_ver.c"
		
void pike_module_init(void)
{
  pexts_init();

#ifdef HAVE_CURSES
  init_screen_program();
  init_window_program();
  
  ADD_FUNCTION("init", f_init, tFunc(tVoid, tVoid), OPT_SIDE_EFFECT);
  ADD_FUNCTION("root", f_root, tFunc(tVoid, tObj), OPT_NOT_CONST);
  ADD_FUNCTION("endwin", f_endwin, tFunc(tVoid, tVoid),
			OPT_SIDE_EFFECT);

  ADD_INT_CONSTANT("ERR", ERR, 0);

  ADD_INT_CONSTANT("A_STANDOUT", A_STANDOUT, 0);
  ADD_INT_CONSTANT("A_UNDERLINE", A_UNDERLINE, 0);
  ADD_INT_CONSTANT("A_REVERSE", A_REVERSE, 0);
  ADD_INT_CONSTANT("A_BLINK", A_BLINK, 0);
  ADD_INT_CONSTANT("A_DIM", A_DIM, 0);
  ADD_INT_CONSTANT("A_BOLD", A_BOLD, 0);
  ADD_INT_CONSTANT("A_ALTCHARSET", A_ALTCHARSET, 0);
  ADD_INT_CONSTANT("A_CHARTEXT", A_CHARTEXT, 0);
  ADD_FUNCTION("COLOR_PAIR", f_COLOR_PAIR, tFunc(tInt, tInt),
			OPT_NOT_CONST);

  ADD_FUNCTION("ACS_ULCORNER", f_ACS_ULCORNER,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_LLCORNER", f_ACS_LLCORNER,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_URCORNER", f_ACS_URCORNER,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_LRCORNER", f_ACS_LRCORNER,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_LTEE", f_ACS_LTEE,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_RTEE", f_ACS_RTEE,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_BTEE", f_ACS_BTEE,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_TTEE", f_ACS_TTEE,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_HLINE", f_ACS_HLINE,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_VLINE", f_ACS_VLINE,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_PLUS", f_ACS_PLUS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
/*   ADD_FUNCTION("ACS_S1", f_ACS_S1, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_S9", f_ACS_S9, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_DIAMOND", f_ACS_DIAMOND, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_CKBOARD", f_ACS_CKBOARD, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_DEGREE", f_ACS_DEGREE, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_PLMINUS", f_ACS_PLMINUS, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_BULLET", f_ACS_BULLET, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_LARROW", f_ACS_LARROW, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_RARROW", f_ACS_RARROW, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_DARROW", f_ACS_DARROW, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_UARROW", f_ACS_UARROW, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_BOARD", f_ACS_BOARD, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_LANTERN", f_ACS_LANTERN, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_BLOCK", f_ACS_BLOCK, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_S3", f_ACS_S3, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_S7", f_ACS_S7, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_LEQUAL", f_ACS_LEQUAL, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_GEQUAL", f_ACS_GEQUAL, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_PI", f_ACS_PI, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_NEQUAL", f_ACS_NEQUAL, */
/* 			"function(:int)", OPT_NOT_CONST); */
/*   ADD_FUNCTION("ACS_STERLING", f_ACS_STERLING, */
/* 			"function(:int)", OPT_NOT_CONST); */
  ADD_FUNCTION("ACS_BSSB", f_ACS_BSSB,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SSBB", f_ACS_SSBB,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_BBSS", f_ACS_BBSS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SBBS", f_ACS_SBBS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SBSS", f_ACS_SBSS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SSSB", f_ACS_SSSB,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SSBS", f_ACS_SSBS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_BSSS", f_ACS_BSSS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_BSBS", f_ACS_BSBS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SBSB", f_ACS_SBSB,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ACS_SSSS", f_ACS_SSSS,
			tFunc(tVoid, tInt), OPT_NOT_CONST);

  /* Output options (curs_outopts) */
  ADD_FUNCTION("nl", f_nl, tFunc(tVoid, tInt), OPT_SIDE_EFFECT);
  ADD_FUNCTION("nonl", f_nl, tFunc(tVoid, tInt), OPT_SIDE_EFFECT);
  
  /* Input options (curs_inopts) */
  ADD_FUNCTION("cbreak", f_cbreak, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("nocbreak", f_nocbreak, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("echo", f_echo, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("noecho", f_noecho, tFunc(tVoid, tInt), OPT_SIDE_EFFECT);
  ADD_FUNCTION("halfdelay", f_halfdelay, tFunc(tInt, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("raw", f_raw, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("noraw", f_noraw, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("qiflush", f_qiflush, tFunc(tVoid, tVoid),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("noqiflush", f_noqiflush, tFunc(tVoid, tVoid),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("typeahead", f_typeahead, tFunc(tInt, tInt),
			OPT_SIDE_EFFECT);

  /* (curs_refresh) */
  ADD_FUNCTION("doupdate", f_doupdate, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);

  /* (curs_beep) */
  ADD_FUNCTION("beep", f_beep, tFunc(tVoid, tVoid), OPT_SIDE_EFFECT);
  ADD_FUNCTION("flash", f_flash, tFunc(tVoid, tVoid), OPT_SIDE_EFFECT);

  /* (curs_kernel) */
  ADD_FUNCTION("getsyx", f_getsyx, tFunc(tVoid, tArr(tInt)),
			OPT_NOT_CONST);
  ADD_FUNCTION("setsyx", f_setsyx,
			tFunc(tOr(tArr(tInt), tInt) tOr(tVoid, tInt), tVoid),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("curs_set", f_curs_set, tFunc(tInt, tInt),
			OPT_SIDE_EFFECT);

  /* Color (curs_color) */
  ADD_FUNCTION("start_color", f_start_color, tFunc(tVoid, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("init_pair", f_init_pair, tFunc(tInt tInt tInt, tInt),
			OPT_SIDE_EFFECT);
  ADD_FUNCTION("init_color", f_init_color,
			tFunc(tInt tInt tInt tInt, tInt), OPT_SIDE_EFFECT);
  ADD_FUNCTION("has_colors", f_has_colors, tFunc(tVoid, tInt),
			OPT_NOT_CONST);
  ADD_FUNCTION("can_change_color", f_can_change_color,
			tFunc(tVoid, tInt), OPT_NOT_CONST);
#endif /* HAVE_CURSES */
}

void pike_module_exit(void)
{
#ifdef HAVE_CURSES
  if (curses_rootwin)
    free_object(curses_rootwin);
  if (curses_mainscr)
    free_object(curses_mainscr);
  endwin();
#endif /* HAVE_CURSES */
}

