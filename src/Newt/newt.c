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

/* Newt Support - This module adds Newt support to Pike. */

#include "global.h"
RCSID("$Id$");

#include "stralloc.h"
#include "pike_types.h"
#include "pike_macros.h"
#include "object.h"
#include "constants.h"
#include "interpret.h"
#include "svalue.h"
#include "mapping.h"
#include "builtin_functions.h"
#include "error.h"
#include "module_support.h"

/*   struct formobj *foo = (struct formobj *)get_storage(obj,Form_program);
 *   ..
 *   start_new_program();
 *    add_storage(...);
 *    ...
 *    add_ref(Form_program = Pike_compiler->new_program);
 *   end_class("Form");
 *
 *   exit_pike_module(...)
 *    free_program(Form_program);
 */
#include "newt_config.h"

#ifdef HAVE_NEWT
struct program * Newt_program;
struct program * Form_program;

int newt_inited = 0;

static void f_init(INT32 args)
{
   if (args != 0)
     error("Too many arguments to init()\n");
   pop_n_elems(args);
   newt_inited = 1;
   newtInit();
}



static void f_cls(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");
  pop_n_elems(args);
  newtCls();
}

static void f_refresh(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");
  pop_n_elems(args);
  newtRefresh();
}

static void f_finished(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");
  pop_n_elems(args);
  newtFinished();
  newt_inited = 0;
}

static void f_pop_window(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");
  pop_n_elems(args);
  newtPopWindow();
}

static void f_open_window(INT32 args)
{
  int x,y,xd,yd;

  if ( ! newt_inited )
    error("newt: Use init first!\n");

  if (args == 0)
    error("Too few arguments to open_window\n");

  if (args == 2)
  {
    struct array *a;
    if (sp[-2].type != T_ARRAY)
      error("Bad argument 1 to gotorc\n");
    a = sp[-2].u.array;
    if (a->size != 4)
      error("An array argument to open_window must be of size 4\n");
    if (a->item[0].type != T_INT)
      error("Element 0 of argument is not an integer\n");
    if (a->item[1].type != T_INT)
      error("Element 1 of argument is not an integer\n");
    if (a->item[2].type != T_INT)
      error("Element 2 of argument is not an integer\n");
    if (a->item[3].type != T_INT)
      error("Element 3 of argument is not an integer\n");
    y = a->item[0].u.integer;
    x = a->item[1].u.integer;
    yd= a->item[2].u.integer;
    xd= a->item[3].u.integer;
    
    newtOpenWindow(y,x,yd,xd,sp[-1].u.string->str);
  }
  else
  {
    check_all_args("open_window", args, BIT_INT, BIT_INT, BIT_INT, BIT_INT, BIT_STRING, 0);
    y = sp[-5].u.integer;
    x = sp[-4].u.integer;
    yd= sp[-3].u.integer;
    xd= sp[-2].u.integer;
    newtOpenWindow(x,y,xd,yd, sp[-1].u.string->str );
  }
  pop_n_elems(args);
}

static void f_open_centered_window(INT32 args)
{
  int xd,yd;

  if ( ! newt_inited )
    error("newt: Use init first!\n");

  if (args == 0)
    error("Too few arguments to open_window\n");

  if (args == 2)
  {
    struct array *a;
    if (sp[-2].type != T_ARRAY)
      error("Bad argument 1 to open_centered_window\n");
    a = sp[-2].u.array;
    if (a->size != 2)
      error("An array argument to open_centered_window must be of size 2\n");
    if (a->item[0].type != T_INT)
      error("Element 0 of argument is not an integer\n");
    if (a->item[1].type != T_INT)
      error("Element 1 of argument is not an integer\n");
    yd= a->item[0].u.integer;
    xd= a->item[1].u.integer;
    
    newtCenteredWindow(yd,xd,sp[-1].u.string->str);
  }
  else
  {
    check_all_args("open_centered_window", args, BIT_INT, BIT_INT, BIT_STRING, 0);
    xd= sp[-3].u.integer;
    yd= sp[-2].u.integer;
    newtCenteredWindow(xd,yd, sp[-1].u.string->str );
  }
  pop_n_elems(args);
}

static void f_draw_root_text(INT32 args)
{
  int x,y,o;

  if ( ! newt_inited )
    error("newt: Use init first!\n");

  if (args == 0)
    error("Too few arguments to draw_root_text\n");

  check_all_args("draw_root_text", args, BIT_INT, BIT_INT, BIT_STRING, 0);
  y  = sp[-3].u.integer;
  x  = sp[-2].u.integer;
  newtDrawRootText( x, y, sp[-1].u.string->str );
  pop_n_elems(args);
}


static void f_winmessage(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  if (args == 0)
    error("Too few arguments to winmessage\n");

  check_all_args("winmessage", args, BIT_STRING, BIT_STRING, BIT_STRING, 0);
  newtWinMessage( sp[-3].u.string->str, 
                  sp[-2].u.string->str,
                  sp[-1].u.string->str );
  pop_n_elems(args);
}

static void f_winchoice(INT32 args)
{
  int s;
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  if (args == 0)
    error("Too few arguments to winmessage\n");

  check_all_args("winchoice", args, BIT_STRING, BIT_STRING, BIT_STRING, BIT_STRING, 0);
  s=newtWinChoice( sp[-4].u.string->str, 
                  sp[-3].u.string->str,
                  sp[-2].u.string->str,
                  sp[-1].u.string->str );
  pop_n_elems(args);
  push_int(s);
}

static void f_winmenu(INT32 args)
{
  struct svalue sor;
  int		selected;
  int           x, y, xd, yd, i;
  char          *title, *text;
  int           suggestedWidth, flexDown, flexUp, maxListHeight;
  int           needScroll;
  newtComponent	textbox, listbox, result, form;
  newtComponent Buttons[50];
  newtGrid      grid, buttonBar;
  struct array *items;
  struct array *buttons;
  

  if ( ! newt_inited )
    error("newt: Use init first!\n");

  if (args == 0)
    error("Too few arguments to menuwindow\n");


   /*
    *   Makes a menu, returns the selected menu.
    *
    *   int winmenu( string title, string text, int suggestedWidth,
    *            int flexDown, int flexUp, int maxListHeight, 
    *            array items, array buttons );
    *
    *   returns the selected button, or 0 if F12 or return-on-exit pressed
    */

    if( args < 8 )
      error("too few arguments to winmenu()\n"); 

    
    if (sp[-1].type != T_ARRAY || sp[-2].type != T_ARRAY )
      error("Bad argument to winmenu (items or buttons must be arrays)\n");
          
    items   = sp[-2].u.array;
    buttons = sp[-1].u.array;
    if ( items->size < 1 || buttons->size < 1 )
      error("too few items in items or buttons array.\n");
      
//    check_all_args("winmenu", args, BIT_STRING,
//                             BIT_STRING, BIT_INT, BIT_INT, BIT_INT, BIT_INT,
//                             BIT_ARRAY,BIT_ARRAY, 0 );

    maxListHeight  = sp[-3].u.integer;
    flexUp         = sp[-4].u.integer;
    flexDown       = sp[-5].u.integer;
    suggestedWidth = sp[-6].u.integer;
    text           = sp[-7].u.string->str;
    title          = sp[-8].u.string->str;
    
    textbox = newtTextboxReflowed( -1,-1,text,suggestedWidth, flexDown,
                                   flexUp,0 );
                                   
    if (items->size < maxListHeight) maxListHeight=items->size;
    needScroll = items->size > maxListHeight;
    
    listbox = newtListbox(-1,-1,maxListHeight,
                        (needScroll ? NEWT_FLAG_SCROLL : 0 ) | NEWT_FLAG_RETURNEXIT );
    
    for (i = 0; i < items->size; i++) {  
        newtListboxAddEntry(listbox, items->item[i].u.string->str, (void *) i);
    }
    
//    newtListboxSetCurrent(listbox, 

    for ( i = 0; i< buttons->size; i++) {
	Buttons[i] = newtButton( -1, -1, buttons->item[i].u.string->str );
    }


    buttonBar = newtCreateGrid( buttons->size, 1 );
    for ( i = 0; i< buttons->size; i++) {
         newtGridSetField( buttonBar, i, 0, NEWT_GRID_COMPONENT,
                         Buttons[i],
                         i ? 1 : 0, 0, 0, 0, 0, 0 );
    }
    
    grid = newtGridSimpleWindow( textbox, listbox, buttonBar);
    newtGridWrappedWindow(grid, title);
    
    form = newtForm(NULL, 0,0 );
    newtGridAddComponentsToForm( grid, form, 1 );
    newtGridFree(grid, 1);
    
    result = newtRunForm( form );
    
    selected = ((long) newtListboxGetCurrent(listbox));
    
    newtFormDestroy(form);
    newtPopWindow();
    
    pop_n_elems(args);
    push_int(selected);
}

static void f_push_help(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  check_all_args("push_help",args, BIT_STRING, 0);
  newtPushHelpLine( sp[-1].u.string->str );
  pop_n_elems(args);
}

static void f_pop_help(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");
  pop_n_elems(args);
  newtPopHelpLine();
}

static void f_get_screensize(INT32 args)
{
  int x,y;
  
  if ( ! newt_inited )
    error("newt: Use init first!\n");
  
  newtGetScreenSize( &y, &x );
  pop_n_elems( args );
  
  push_int(y);
  push_int(x);
  f_aggregate(2);
}

static void f_waitforkey(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  newtWaitForKey();
  pop_n_elems(args);
}

static void f_clearkeybuffer(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  newtClearKeyBuffer();
  pop_n_elems(args);
}

static void f_bell(INT32 args)
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  newtBell();
  pop_n_elems(args);
}

static void f_init_form( INT32 args )
{
  if ( ! newt_inited )
    error("newt: Use init first!\n");

  THIS->obj = newtForm( NULL, NULL, 0 );
  THIS->first = NULL;
  THIS->last = NULL;
  
  pop_n_elems(args);
}



static void f_label(INT32 args ) {
  newtComponent elem;
  char *labeltext;
  char *nev;
  int x,y;
  
  get_all_args("Label",args,"%s%i%i%s",&nev,&x,&y,&labeltext);

  elem=newtLabel(x,y,labeltext);
  if( !store_component(elem,nev,NULL) ) {
     error("Label: not enough memory\n");
  }
  newtFormAddComponent(THIS->obj, elem);
  pop_n_elems(args);
}

static void f_button(INT32 args ) {
  newtComponent elem;
  char *labeltext;
  char *nev;
  int x,y;
  
  get_all_args("Button",args,"%s%i%i%s",&nev,&x,&y,&labeltext);

  elem=newtButton(x,y,labeltext);
  if( !store_component(elem,nev,NULL) ) {
     error("Button: not enough memory\n");
  }
  newtFormAddComponent(THIS->obj, elem);
  pop_n_elems(args);
}

static void f_compactbutton(INT32 args ) {
  newtComponent elem;
  char *labeltext;
  char *nev;
  int x,y;
  
  get_all_args("CompactButton",args,"%s%i%i%s",&nev,&x,&y,&labeltext);

  elem=newtCompactButton(x,y,labeltext);
  if( !store_component(elem,nev,NULL) ) {
     error("CompactButton: not enough memory\n");
  }
  newtFormAddComponent(THIS->obj, elem);
  pop_n_elems(args);
}

static void f_entry(INT32 args ) {
  newtComponent elem;
  int x,y,width;
  char *value;
  char *nev;
  
  get_all_args("Entry",args,"%s%i%i%i",&nev,&x,&y,&width);

  elem=newtEntry(x,y,"",width, &value,0);
  if( !store_component(elem,nev,value) ) {
     error("Entry: not enough memory\n");
  }
  newtFormAddComponent(THIS->obj, elem);
  pop_n_elems(args);
}
/*
 * Vicces kis menu ahova akarja az ember
 */
static void f_menu(INT32 args )
{
   newtComponent f,lb,answer;
   struct array *a;
   int max,i;
   int selected;
   int x,y;

   if( args < 3 && sp[-1].type != T_ARRAY 
         && sp[-2].type != T_INT 
	 && sp[-3].type != T_INT ) {
       error("too few or bad argument to Menu\n");
   }

   a = sp[-1].u.array;
   y = sp[-2].u.integer;
   x = sp[-3].u.integer;
   
   f = newtForm(NULL,NULL,0);
   lb = newtListbox(0,0,(a->size < 15 ? a->size : 15 ),
		  ( a->size < 15 ? 0 : NEWT_FLAG_SCROLL ) 
		                     | NEWT_FLAG_RETURNEXIT );
 
   for(i=0,max=0; i < a->size; i++ ) {
	if( max < a->item[i].u.string->len ) 
	     max = a->item[i].u.string->len;
 	newtListboxAppendEntry(lb, a->item[i].u.string->str, 
	                          (void *) i );
   }
   newtOpenWindow(x,y,max+2,(a->size < 15 ? a->size : 15 ),"Menu");
   newtFormAddComponent(f,lb);
   answer = newtRunForm(f);
   selected=((long) newtListboxGetCurrent(lb));
   newtFormDestroy(f);
   newtPopWindow();
   pop_n_elems(args);
    
   push_int(selected);
}

static void f_runform(INT32 args) 
{
 FormData *p,*pb;
 newtComponent answer;
 int i;

 answer=newtRunForm( THIS->obj );
 
 p=THIS->first;
 i=0;
 pop_n_elems(args);
 while( p ) {
   /* ha az adott Componens okozta a kilepest akkor a neve
    * belekerul a mint 'exit' mapping 
    */
   if( p->elem == answer ) {
     push_text("exit");
     push_text(p->nev);
     i++;
   }
   if(p->nev && p->value ) {
     push_text( p->nev );
     push_text( p->value );
     i++;
   }
   pb=p;
   p=p->next;
   free(pb);
 }
 if ( i>0 )
   f_aggregate_mapping(i*2);
 
 newtFormDestroy( THIS->obj );
 return;
 
}

int store_component( newtComponent elem, char *nev, char *value )
{
  FormData *p;    

  if( !(p=malloc(sizeof(FormData))) ) 
      return (0);
  
  p->next = NULL;
  p->elem = elem;
  p->nev = nev;
  p->value= value;
  
  if( THIS->last ) {
    THIS->last->next = p; 
    THIS->last=p;
  } else {
     THIS->last=THIS->first=p;  
  }
  return (1);
}
#endif /* HAVE_NEWT */
//----------------------------- - - --- - -  - -

void pike_module_init(void)
{
#ifdef HAVE_NEWT
  start_new_program();
  //ADD_FUNCTION("create", f_init, "function(void:void)", ID_PUBLIC );
  ADD_FUNCTION("create", f_init, tFunc(tVoid,tVoid), ID_PUBLIC);
//  add_integer_constant("ERR", ERR, 0);

  ADD_FUNCTION("cls",f_cls,tFunc(tNone,tVoid),0);
  ADD_FUNCTION("refresh",f_refresh,tFunc(tNone,tVoid),0);
  ADD_FUNCTION("Bell",f_bell,tFunc(tVoid,tVoid),0);
  ADD_FUNCTION("WaitForKey",f_waitforkey,tFunc(tVoid,tVoid),0);
  ADD_FUNCTION("ClearKeyBuffer",f_clearkeybuffer,tFunc(tVoid,tVoid),0);

//  ADD_FUNCTION("open_window",f_open_window,"function(:void)",0);
//  ADD_FUNCTION("open_centered_window",f_open_centered_window,"function(:void)",0);
//  ADD_FUNCTION("pop_window",f_pop_window,"function(:void)",0);
  ADD_FUNCTION("draw_root_text",f_draw_root_text,tFunc(tNone,tVoid),0);
  ADD_FUNCTION("screensize",f_get_screensize,tFunc(tVoid,tArray),0);

  ADD_FUNCTION("finished",f_finished,tFunc(tNone,tVoid),0);
  ADD_FUNCTION("winmessage",f_winmessage,tFunc(tNone,tVoid),0);
  ADD_FUNCTION("winchoice",f_winchoice,tFunc(tNone,tInt),0);
  ADD_FUNCTION("winmenu",f_winmenu,tFunc(tNone,tInt),0);
  ADD_FUNCTION("menu",f_menu,tFunc(tInt tInt tArr(tString),tInt),OPT_RETURN);
  
  start_new_program();
    ADD_FUNCTION("push",f_push_help,tFunc(tNone,tVoid), 0);
    ADD_FUNCTION("pop",f_pop_help,tFunc(tVoid,tVoid), 0);
  end_class("Status",0);

  start_new_program();
    ADD_FUNCTION("Centered", f_open_centered_window,tFunc(tNone,tVoid),0);
    ADD_FUNCTION("Normal",f_open_window,tFunc(tNone,tVoid),0);
    ADD_FUNCTION("pop",f_pop_window,tFunc(tNone,tVoid),0);
    start_new_program();
       ADD_STORAGE(struct formobj);
       ADD_FUNCTION("create",f_init_form,tFunc(tVoid,tVoid),0);
       ADD_FUNCTION("Label",f_label,tFunc(tInt tInt tString,tVoid),0);
       ADD_FUNCTION("Button",f_button,tFunc(tInt tInt tString,tVoid),0);
       ADD_FUNCTION("CompactButton",f_compactbutton,tFunc(tInt tInt tString,tVoid),0);
       ADD_FUNCTION("Entry",f_entry,tFunc(tInt tInt tInt,tVoid),0);
       ADD_FUNCTION("runform",f_runform,tFunc(tVoid,tOr(tVoid,tArr(tMap(tString,tMixed)))),OPT_RETURN);
    end_class("Form",0);
  end_class("Window",0);

  end_class("form",0);
#endif /* HAVE_NEWT */
}

void pike_module_exit(void)
{
#ifdef HAVE_NEWT
   if( newt_inited ) {
      newt_inited=0;
      newtFinished();
   }
#endif
}
