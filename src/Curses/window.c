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

#ifdef HAVE_CURSES

struct window {
  WINDOW *win;
};

#define THISWIN ((struct window *)(fp->current_storage))

#define W_VOID_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  if (args>0) \
    Pike_error("Too many arguments to " #f "()\n"); \
  if (!curses_inited) \
    Pike_error("Can't use " #f "() before init()\n"); \
  push_int(w##f(THISWIN->win)); \
}

#define VOID_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  if (args>0) \
    Pike_error("Too many arguments to " #f "()\n"); \
  if (!curses_inited) \
    Pike_error("Can't use " #f "() before init()\n"); \
  push_int(f(THISWIN->win)); \
}

#define INT_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  int i; \
  check_all_args(#f, args, BIT_INT, 0); \
  i = sp[-1].u.integer; \
  pop_n_elems(args); \
  push_int(f(THISWIN->win, i)); \
}

#define INT_INT_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  int i,j; \
  check_all_args(#f, args, BIT_INT, BIT_INT, 0); \
  i = sp[-2].u.integer; \
  j = sp[-1].u.integer; \
  pop_n_elems(args); \
  push_int(f(THISWIN->win, i, j)); \
}

#define INT_VOID_FN(f) \
static void f_##f(INT32 args) \
{ \
  int i; \
  check_all_args(#f, args, BIT_INT, 0); \
  i = sp[-1].u.integer; \
  pop_n_elems(args); \
  f(THISWIN->win, i); \
}

#define W_INT_INT_FN(f) \
static void f_##f(INT32 args) \
{ \
  int i; \
  check_all_args(#f, args, BIT_INT, 0); \
  i = sp[-1].u.integer; \
  pop_n_elems(args); \
  push_int(w##f(THISWIN->win, i)); \
}

#define W_INT_VOID_FN(f) \
static void f_##f(INT32 args) \
{ \
  int i; \
  check_all_args(#f, args, BIT_INT, 0); \
  i = sp[-1].u.integer; \
  pop_n_elems(args); \
  w##f(THISWIN->win, i); \
}

static void init_window_struct(struct object *o)
{
  THISWIN->win = NULL;
}

static void exit_window_struct(struct object *o)
{
  if(THISWIN->win && (THISWIN->win!=stdscr))
    delwin(THISWIN->win);
  THISWIN->win = 0;
}

static void f_window_create(INT32 args)
{
  if (args == 0)
  {
    if (curses_rootwin || !curses_inited)
      Pike_error("Too few arguments to Window->create()\n");
    THISWIN->win = stdscr;
/*     fprintf(stderr,"stdscr = %p\n",stdscr); */
    if (!THISWIN->win)
      Pike_error("stdscr = 0 in window::create()\n");
  }
  else
  {
    check_all_args("create", args, BIT_INT, BIT_INT, BIT_INT, BIT_INT, 0);
    if (!curses_inited)
      Pike_error("Can't create window before Curses.init() is called\n");
    THISWIN->win = newwin(sp[-1].u.integer, /* nlines */
			  sp[-2].u.integer, /* ncols */
			  sp[-3].u.integer, /* begin_y */
			  sp[-4].u.integer); /* begin_x */
    /* scrollok(THISWIN->win, TRUE); */
  }
  pop_n_elems(args);
}

#define VOID_Y_X_FN(f) \
static void f_##f(INT32 args) \
{ \
  int x, y; \
  if (args > 0) \
    Pike_error("Too many arguments to "#f"\n"); \
  f(THISWIN->win, y, x); \
  push_int(y); \
  push_int(x); \
  push_array(aggregate_array(2)); \
}

VOID_Y_X_FN(getyx)
VOID_Y_X_FN(getparyx)

static void f_getbegyx(INT32 args)
{
  int x, y;
  if (args > 0)
    Pike_error("Too many arguments to getbegyx\n");
  getbegyx(THISWIN->win, y, x);
  push_int(y);
  push_int(x);
  push_array(aggregate_array(2));
}

static void f_getmaxyx(INT32 args)
{
  int x, y;
  if (args > 0)
    Pike_error("Too many arguments to getmaxyx\n");
  getmaxyx(THISWIN->win, y, x);
  push_int(y);
  push_int(x);
  push_array(aggregate_array(2));
}

static void f_xsize(INT32 args)
{
  int x, y;
  check_all_args("xsize", args, 0);
  getmaxyx(THISWIN->win, y, x);
  push_int(x);
}

static void f_ysize(INT32 args)
{
  int x, y;
  check_all_args("ysize", args, 0);
  getmaxyx(THISWIN->win, y, x);
  push_int(y);
}

W_INT_INT_FN(bkgd)

static void f_bkgdset(INT32 args)
{
  check_all_args("bkgdset", args, BIT_INT, 0);
  wbkgdset(THISWIN->win, sp[-1].u.integer);
  pop_n_elems(args);
}

INT_INT_FN(clearok)
INT_INT_FN(idlok)
INT_VOID_FN(idcok)
INT_VOID_FN(immedok)
INT_INT_FN(leaveok)

static void f_setscrreg(INT32 args)
{
  check_all_args("setscrreg", args, BIT_INT, BIT_INT, 0);
  push_int(wsetscrreg(THISWIN->win, sp[-2].u.integer, sp[-1].u.integer));
  pop_n_elems(args);
}

INT_INT_FN(scrollok)
INT_INT_FN(intrflush)
INT_INT_FN(keypad)
INT_INT_FN(meta)
INT_INT_FN(nodelay)
W_INT_VOID_FN(timeout)
INT_INT_FN(notimeout)

static void f_move(INT32 args)
{
  check_all_args("move", args, BIT_INT, BIT_INT,0 );
  /* THREADS_ALLOW(); */
  wmove(THISWIN->win, sp[-2].u.integer, sp[-1].u.integer);
  /* THREADS_DISALLOW(); */
}

static void f_addstr(INT32 args)
{
  int r;
  check_all_args("addstr", args, BIT_STRING, 0);
  /* THREADS_ALLOW(); */
  r = waddstr(THISWIN->win, sp[-1].u.string->str);
  /* THREADS_DISALLOW(); */
  pop_n_elems(args);
  push_int(r);
}

static void f_mvaddstr(INT32 args)
{
  int r;
  check_all_args("mvaddstr", args, BIT_INT, BIT_INT, BIT_STRING, 0);
  /* THREADS_ALLOW(); */
  r = mvwaddstr(THISWIN->win, sp[-3].u.integer, sp[-2].u.integer,
		sp[-1].u.string->str);
  /* THREADS_DISALLOW(); */
  pop_n_elems(args);
  push_int(r);
}

W_INT_INT_FN(addch)

static void f_mvaddch(INT32 args)
{
  int r;
  check_all_args("mvaddch", args, BIT_INT, BIT_INT, BIT_INT, 0);
  /* THREADS_ALLOW(); */
  r = mvwaddch(THISWIN->win, sp[-3].u.integer, sp[-2].u.integer,
	       sp[-1].u.integer);
  /* THREADS_DISALLOW(); */
  pop_n_elems(args);
  push_int(r);
}

W_INT_INT_FN(insch)
static void f_mvinsch(INT32 args)
{
  int r;
  check_all_args("mvinsch", args, BIT_INT, BIT_INT, BIT_INT, 0);
  r = mvwinsch(THISWIN->win, sp[-3].u.integer, sp[-2].u.integer,
	       sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

W_INT_INT_FN(echochar)

W_INT_INT_FN(attroff)
W_INT_INT_FN(attron)
W_INT_INT_FN(attrset)
W_VOID_INT_FN(standend)
W_VOID_INT_FN(standout)

static void f_window_reverse(INT32 args)
{
  int r;
  check_all_args("reverse", args, BIT_INT, 0);
  if (sp[-1].u.integer)
    r = wattrset(THISWIN->win, A_REVERSE);
  else
    r = wattrset(THISWIN->win, 0);
  pop_n_elems(args);
  push_int(r);
}

static void f_bold(INT32 args)
{
  int r;
  check_all_args("bold", args, BIT_INT, 0);
  if (sp[-1].u.integer)
    r = wattrset(THISWIN->win, A_BOLD);
  else
    r = wattrset(THISWIN->win, 0);
  pop_n_elems(args);
  push_int(r);
}

VOID_INT_FN(touchwin)
INT_INT_INT_FN(touchline)
VOID_INT_FN(untouchwin)

static void f_touchln(INT32 args)
{
  int i, j, k;
  check_all_args("touchln", args, BIT_INT, BIT_INT, BIT_INT, 0);
  i = sp[-3].u.integer;
  j = sp[-2].u.integer;
  k = sp[-1].u.integer;
  pop_n_elems(args);
  push_int(wtouchln(THISWIN->win, i, j, k));
}

INT_INT_FN(is_linetouched)
VOID_INT_FN(is_wintouched)
     
W_VOID_INT_FN(erase)
W_VOID_INT_FN(clear)
W_VOID_INT_FN(clrtobot)
W_VOID_INT_FN(clrtoeol)

VOID_INT_FN(scroll)
W_INT_INT_FN(scrl)

W_VOID_INT_FN(refresh)
W_VOID_INT_FN(noutrefresh)
VOID_INT_FN(redrawwin)

static void f_redrawln(INT32 args)
{
  int r;
  check_all_args("redrawln", args, BIT_INT, BIT_INT, 0);
  r = wredrawln(THISWIN->win, sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

static void f_getch(INT32 args)
{
  int c;
  if (args != 0)
    Pike_error("Too many arguments to getch()\n");
  /* THREADS_ALLOW(); */
  c = wgetch(THISWIN->win);
  /* THREADS_DISALLOW(); */
  push_int(c);
}

static void f_border(INT32 args)
{
  int i;
  int a[8];
  if (args > 8)
    Pike_error("Too many arguments to border()\n");
  for (i=0;i<8;i++)
    if (i<args)
    {
      if (sp[-args+i].type != T_INT)
      {
	char s[100];
	sprintf(s, "Bad argumend %d in border(), expected int\n", i);
	Pike_error(s);
      }
      a[i] = sp[-args+i].u.integer;
    }
    else
      a[i] = 0;
  
  i = wborder(THISWIN->win,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
  pop_n_elems(args);
  push_int(i);
}

static void f_box(INT32 args)
{
  int i;
  int a[2];
  if (args > 2)
    Pike_error("Too many arguments to box()\n");
  for (i=0;i<2;i++)
    if (i<args)
    {
      if (sp[-args+i].type != T_INT)
      {
	char s[100];
	sprintf(s, "Bad argumend %d in box(), expected int\n", i);
	Pike_error(s);
      }
      a[i] = sp[-args+i].u.integer;
    }
    else
      a[i] = 0;
  
  i = box(THISWIN->win,a[0],a[1]);
  pop_n_elems(args);
  push_int(i);
}

static void f_hline(INT32 args)
{
  int r;
  check_all_args("hline", args, BIT_INT, BIT_INT, 0);
  r = whline(THISWIN->win, sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

static void f_mvhline(INT32 args)
{
  int r;
  check_all_args("mvhline", args, BIT_INT, BIT_INT, BIT_INT, BIT_INT, 0);
  r = mvwhline(THISWIN->win, sp[-4].u.integer, sp[-3].u.integer,
                             sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

static void f_vline(INT32 args)
{
  int r;
  check_all_args("vline", args, BIT_INT, BIT_INT, 0);
  r = wvline(THISWIN->win, sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

static void f_mvvline(INT32 args)
{
  int r;
  check_all_args("mvvline", args, BIT_INT, BIT_INT, BIT_INT, BIT_INT, 0);
  r = mvwvline(THISWIN->win, sp[-4].u.integer, sp[-3].u.integer,
                             sp[-2].u.integer, sp[-1].u.integer);
  pop_n_elems(args);
  push_int(r);
}

void init_window_program(void)
{
  start_new_program();

  ADD_STORAGE(struct window);

  set_init_callback(init_window_struct);
  set_exit_callback(exit_window_struct);

  ADD_FUNCTION("create", f_window_create,
	       tFunc(tOr(tInt, tVoid) tOr(tInt, tVoid)
               tOr(tInt, tVoid) tOr(tInt, tVoid), tVoid), 0);

  /* (curs_getyx) */
  ADD_FUNCTION("getyx", f_getyx, tFunc(tVoid, tArr(tInt)), OPT_NOT_CONST);
  ADD_FUNCTION("getparyx", f_getparyx, tFunc(tVoid, tArr(tInt)), OPT_NOT_CONST);
  ADD_FUNCTION("getbegyx", f_getbegyx, tFunc(tVoid, tArr(tInt)), OPT_NOT_CONST);
  ADD_FUNCTION("getmaxyx", f_getmaxyx, tFunc(tVoid, tArr(tInt)), OPT_NOT_CONST);
  /* Derived functions */
  ADD_FUNCTION("xsize", f_xsize, tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("ysize", f_ysize, tFunc(tVoid, tInt), OPT_NOT_CONST);
  
  /* (curs_bkgd) */
  ADD_FUNCTION("bkgd", f_bkgd, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("bkgdset", f_bkgdset, tFunc(tInt, tVoid), 0);

  /* Output options (curs_outopts) */
  ADD_FUNCTION("clearok", f_clearok, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("idlok", f_idlok, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("idcok", f_idcok, tFunc(tInt, tVoid), 0);
  ADD_FUNCTION("immedok", f_immedok, tFunc(tInt, tVoid), 0);
  ADD_FUNCTION("leaveok", f_leaveok, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("setscrreg", f_setscrreg, tFunc(tInt tInt, tInt), 0);
  ADD_FUNCTION("scrollok", f_scrollok, tFunc(tInt, tInt), 0);

  /* Input options (curs_inopts) */
  ADD_FUNCTION("intrflush", f_intrflush, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("keypad", f_keypad, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("meta", f_meta, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("nodelay", f_nodelay, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("timeout", f_timeout, tFunc(tInt, tVoid), 0);
  ADD_FUNCTION("notimeout", f_notimeout, tFunc(tInt, tInt), 0);

  ADD_FUNCTION("move", f_move, tFunc(tInt tInt, tVoid), 0);
  ADD_FUNCTION("addstr", f_addstr, tFunc(tString, tInt), 0);
  ADD_FUNCTION("mvaddstr", f_mvaddstr, tFunc(tInt tInt tString, tInt), 0);
  ADD_FUNCTION("addch", f_addch, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("mvaddch", f_mvaddch, tFunc(tInt tInt tInt, tInt), 0);
  ADD_FUNCTION("echochar", f_echochar, tFunc(tInt, tInt), 0);

  /* Inserting characters (curs_insch) */
  ADD_FUNCTION("insch", f_insch, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("mvinsch", f_mvinsch, tFunc(tInt tInt tInt, tInt), 0);

  /* (curs_touch) */
  ADD_FUNCTION("touchwin", f_touchwin, tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("touchline", f_touchline,tFunc(tInt tInt, tInt),OPT_NOT_CONST);
  ADD_FUNCTION("untouchwin", f_untouchwin, tFunc(tVoid, tInt), OPT_NOT_CONST);
  ADD_FUNCTION("touchln", f_touchln,tFunc(tInt tInt tInt, tInt),OPT_NOT_CONST);
  ADD_FUNCTION("is_linetouched", f_is_linetouched,tFunc(tInt, tInt),
	      OPT_NOT_CONST);
  ADD_FUNCTION("is_wintouched",f_is_wintouched,tFunc(tVoid, tInt),OPT_NOT_CONST);

  /* (curs_clear) */
  ADD_FUNCTION("erase", f_erase, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("clear", f_clear, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("clrtobot", f_clrtobot, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("clrtoeol", f_clrtoeol, tFunc(tVoid, tInt), 0);
  
  /* (curs_scroll */
  ADD_FUNCTION("scroll", f_scroll, tFunc(tVoid, tInt),
	       OPT_SIDE_EFFECT|OPT_NOT_CONST);
  ADD_FUNCTION("scrl", f_scrl, tFunc(tInt, tInt),
	       OPT_SIDE_EFFECT|OPT_NOT_CONST);

  /* (curs_refresh) */
  ADD_FUNCTION("refresh", f_refresh, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("noutrefresh", f_noutrefresh, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("redrawwin", f_refresh, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("redrawln", f_refresh, tFunc(tInt tInt, tVoid), 0);

  /* Border functions (curs_border) */
  ADD_FUNCTION("border", f_border,
	       tFunc(tOr(tInt, tVoid) tOr(tInt, tVoid) 
               tOr(tInt, tVoid) tOr(tInt, tVoid) tOr(tInt, tVoid) 
	       tOr(tInt, tVoid) tOr(tInt, tVoid) tOr(tInt, tVoid), tInt), 0);
  ADD_FUNCTION("box", f_box, tFunc(tOr(tInt, tVoid) tOr(tInt, tVoid), tInt), 0);
  ADD_FUNCTION("hline", f_hline, tFunc(tInt tInt, tInt), 0);
  ADD_FUNCTION("vline", f_vline, tFunc(tInt tInt, tInt), 0);
  ADD_FUNCTION("mvhline", f_mvhline, tFunc(tInt tInt tInt tInt, tInt), 0);
  ADD_FUNCTION("mvvline", f_mvvline, tFunc(tInt tInt tInt tInt, tInt), 0);

  /* Char attributes (curs_attr) */
  ADD_FUNCTION("attron", f_attron, tFunc(tInt, tInt), OPT_SIDE_EFFECT);
  ADD_FUNCTION("attrset", f_attrset, tFunc(tInt, tInt), OPT_SIDE_EFFECT);
  ADD_FUNCTION("attroff", f_attroff, tFunc(tInt, tInt), OPT_SIDE_EFFECT);
  ADD_FUNCTION("standout", f_standout, tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("standend", f_standend, tFunc(tVoid, tInt), 0);

  ADD_FUNCTION("reverse", f_window_reverse, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("bold", f_bold, tFunc(tInt, tInt), 0);
  ADD_FUNCTION("getch", f_getch, tFunc(tVoid, tInt), 0);

  curses_window_program = end_program();
  add_program_constant("Window", curses_window_program, 0);
}
#endif /* HAVE_CURSES */
