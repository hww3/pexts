/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
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
 * $Id$
 */

/*
 * File licensing and authorship information block.
 *
 * Version: MPL 1.1/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *
 * Marek Habersack <grendel@caudium.net>
 *
 * Portions created by the Initial Developer are Copyright (C) Marek Habersack
 * & The Caudium Group. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of the LGPL, and not to allow others to use your version
 * of this file under the terms of the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL or the LGPL.
 *
 * Significant Contributors to this file are:
 *
 */

/*
 * Glue for all Newt functions
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include <unistd.h>
#include <stdio.h>

#include "caudium_util.h"

#include "newt_config.h"

#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)
#include "newt_global.h"

static char *dictname = "NewtFunctions";
static DICT *dict;

#define dict_insert(_obj_, _data_) dict->insert(dict, _obj_, _data_)
#define dict_lookup(_data_) dict->lookup(dict, _data_)
#define dict_foreach(_cb_) dict->foreach(dict, _cb_)

#ifdef HAVE_NEWTSETTHREED
static void
f_setThreeD(INT32 args)
{
    INFUN();
    
    if (args != 1) {
        OUTFUN();
        FERROR("setThreeD", "Wrong number of arguments. Expected %d got %d instead", 1, args);
    }
    

    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("setThreeD", "Wrong argument type for argument %d. Expected an integer.", 1);
    }

    if (!ARG(1).u.integer)
        newtSetThreeD(0);
    else
        newtSetThreeD(1);

    pop_n_elems(args);

    OUTFUN();
}
#endif

static void
f_cls(INT32 args)
{
    newtCls();
    pop_n_elems(args);
}

static void
f_resizeScreen(INT32 args)
{
    if (args != 1)
        FERROR("resizeScreen", "Expected one argument, got %d instead", args);

    if (ARG(1).type != T_INT)
        FERROR("resizeScreen", "Wrong argument type for argument %d. Expected an integer.", 1);

    if (!ARG(1).u.integer)
        newtResizeScreen(0);
    else
        newtResizeScreen(1);

    pop_n_elems(args);
}

static void
f_waitForKey(INT32 args)
{
    newtWaitForKey();
    pop_n_elems(args);
}

static void
f_clearKeyBuffer(INT32 args)
{
    newtClearKeyBuffer();
    pop_n_elems(args);
}

static void
f_delay(INT32 args)
{
    if (args != 1)
        FERROR("delay", "Expected one argument, got %d instead", args);
    
    if (ARG(1).type != T_INT)
        FERROR("delay", "Wrong argument type for argument %d. Expected an integer.", 1);

    newtDelay(ARG(1).u.integer);

    pop_n_elems(args);
}

static void
f_openWindow(INT32 args)
{
    if (args != 5)
        FERROR("openWindow", "Expected 5 arguments, got %d instead", args);

    if (ARG(1).type != T_INT)
        FERROR("openWindow", "Wrong argument type for argument %d. Expected an integer.", 1);

    if (ARG(2).type != T_INT)
        FERROR("openWindow", "Wrong argument type for argument %d. Expected an integer.", 2);

    if (ARG(3).type != T_INT)
        FERROR("openWindow", "Wrong argument type for argument %d. Expected an integer.", 3);

    if (ARG(4).type != T_INT)
        FERROR("openWindow", "Wrong argument type for argument %d. Expected an integer.", 4);

    if (ARG(5).type != T_STRING || ARG(5).u.string->size_shift > 0)
        FERROR("openWindow", "Wrong argument type for argument %d. Expected an 8-bit string.", 5);

    (void)newtOpenWindow(ARG(1).u.integer, /* left */
                         ARG(2).u.integer, /* top */
                         ARG(3).u.integer, /* width */
                         ARG(4).u.integer, /* height */
                         ARG(5).u.string->str);

    pop_n_elems(args);
}

static void
f_centeredWindow(INT32 args)
{
    if (args != 3)
        FERROR("centeredWindow", "Expected 3 arguments, got %d instead", args);

    if (ARG(1).type != T_INT)
        FERROR("centeredWindow", "Wrong argument type for argument %d. Expected an integer.", 1);

    if (ARG(2).type != T_INT)
        FERROR("centeredWindow", "Wrong argument type for argument %d. Expected an integer.", 2);

    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0)
        FERROR("centeredWindow", "Wrong argument type for argument %d. Expected an 8-bit string.", 3);

    (void)newtCenteredWindow(ARG(1).u.integer, /* width */
                             ARG(2).u.integer, /* height */
                             ARG(3).u.string->str);

    pop_n_elems(args);    
}

static void
f_popWindow(INT32 args)
{
    newtPopWindow();
    pop_n_elems(args);
}

static void
make_color(char *name, struct svalue *key, struct svalue *val, char *fg, char *bg)
{
    struct mapping     *color;

    color = allocate_mapping(2);
    if (!color)
        ERROR("defaultColors", "Error while allocating the component color mapping");

    key->type = T_STRING;
    key->u.string = make_shared_string("fg");
    val->type = T_STRING;
    val->u.string = make_shared_string(fg);
    mapping_insert(color, key, val);

    key->u.string = make_shared_string("bg");
    val->u.string = make_shared_string(bg);
    mapping_insert(color, key, val);

    key->u.string = make_shared_string(name);
    val->type = T_MAPPING;
    val->u.mapping = color;
}

static void
f_defaultColors(INT32 args)
{
    struct mapping       *colors;
    struct svalue        key, val;

    colors = allocate_mapping(23);
    if (!colors)
        FERROR("defaultCollors", "Error while allocating the colors mapping");
    
    make_color("root", &key, &val, newtDefaultColorPalette.rootFg, newtDefaultColorPalette.rootBg);
    mapping_insert(colors, &key, &val);

    make_color("border", &key, &val, newtDefaultColorPalette.borderFg, newtDefaultColorPalette.borderBg);
    mapping_insert(colors, &key, &val);

    make_color("window", &key, &val, newtDefaultColorPalette.windowFg, newtDefaultColorPalette.windowBg);
    mapping_insert(colors, &key, &val);

    make_color("shadow", &key, &val, newtDefaultColorPalette.shadowFg, newtDefaultColorPalette.shadowBg);
    mapping_insert(colors, &key, &val);

    make_color("title", &key, &val, newtDefaultColorPalette.titleFg, newtDefaultColorPalette.titleBg);
    mapping_insert(colors, &key, &val);

    make_color("button", &key, &val, newtDefaultColorPalette.buttonFg, newtDefaultColorPalette.buttonBg);
    mapping_insert(colors, &key, &val);

    make_color("actButton", &key, &val, newtDefaultColorPalette.actButtonFg, newtDefaultColorPalette.actButtonBg);
    mapping_insert(colors, &key, &val);

    make_color("checkbox", &key, &val, newtDefaultColorPalette.checkboxFg, newtDefaultColorPalette.checkboxBg);
    mapping_insert(colors, &key, &val);

    make_color("actCheckbox", &key, &val, newtDefaultColorPalette.actCheckboxFg, newtDefaultColorPalette.actCheckboxBg);
    mapping_insert(colors, &key, &val);

    make_color("entry", &key, &val, newtDefaultColorPalette.entryFg, newtDefaultColorPalette.entryBg);
    mapping_insert(colors, &key, &val);

    make_color("label", &key, &val, newtDefaultColorPalette.labelFg, newtDefaultColorPalette.labelBg);
    mapping_insert(colors, &key, &val);

    make_color("listbox", &key, &val, newtDefaultColorPalette.listboxFg, newtDefaultColorPalette.listboxBg);
    mapping_insert(colors, &key, &val);

    make_color("actListbox", &key, &val, newtDefaultColorPalette.actListboxFg, newtDefaultColorPalette.actListboxBg);
    mapping_insert(colors, &key, &val);

    make_color("textbox", &key, &val, newtDefaultColorPalette.textboxFg, newtDefaultColorPalette.textboxBg);
    mapping_insert(colors, &key, &val);

    make_color("actTextbox", &key, &val, newtDefaultColorPalette.actTextboxFg, newtDefaultColorPalette.actTextboxBg);
    mapping_insert(colors, &key, &val);

    make_color("helpLine", &key, &val, newtDefaultColorPalette.helpLineFg, newtDefaultColorPalette.helpLineBg);
    mapping_insert(colors, &key, &val);

    make_color("rootText", &key, &val, newtDefaultColorPalette.rootTextFg, newtDefaultColorPalette.rootTextBg);
    mapping_insert(colors, &key, &val);

    make_color("emptyScale", &key, &val, newtDefaultColorPalette.emptyScale, newtDefaultColorPalette.fullScale);
    mapping_insert(colors, &key, &val);

    make_color("disabledEntry", &key, &val, newtDefaultColorPalette.disabledEntryFg, newtDefaultColorPalette.disabledEntryBg);
    mapping_insert(colors, &key, &val);

    make_color("compactButton", &key, &val, newtDefaultColorPalette.compactButtonFg, newtDefaultColorPalette.compactButtonBg);
    mapping_insert(colors, &key, &val);

    make_color("actSelListbox", &key, &val, newtDefaultColorPalette.actSelListboxFg, newtDefaultColorPalette.actSelListboxBg);
    mapping_insert(colors, &key, &val);

    make_color("selListbox", &key, &val, newtDefaultColorPalette.selListboxFg, newtDefaultColorPalette.selListboxBg);
    mapping_insert(colors, &key, &val);

#if 0
    make_color("threeDbox", &key, &val, newtDefaultColorPalette.threeDboxFg, newtDefaultColorPalette.threeDboxBg);
    mapping_insert(colors, &key, &val);
#endif

    pop_n_elems(args);
    
    push_mapping(colors);
}

static void
get_color(struct mapping *m, char *name, char **pfg, char **pbg)
{
    struct svalue  *val;
    struct svalue  *fg, *bg;

    *pfg = NULL;
    *pbg = NULL;
    
    val = simple_mapping_string_lookup(m, name);
    if (!val)
        return;
        
    if(val->type != T_MAPPING)
        FERROR("setColors", "Incorrect colors mapping element type. Expected a mapping.");
    fg = simple_mapping_string_lookup(val->u.mapping, "fg");
    bg = simple_mapping_string_lookup(val->u.mapping, "bg");

    if (fg) {
        if (fg->type != T_STRING || fg->u.string->size_shift > 0) {
            FERROR("setColors", "Incorrect FG component type. Expected an 8-bit string.");
        } else {
            *pfg = fg->u.string->str;
        }
    }
    
    if (bg) {
        if (bg->type != T_STRING || bg->u.string->size_shift > 0) {
            FERROR("setColors", "Incorrect BG component type. Expected an 8-bit string.");
        } else {
            *pbg = bg->u.string->str;
        }
    }
}

#define SET_COLOR(sel) \
    if (!fg) \
        colors.sel##Fg = newtDefaultColorPalette.sel##Fg; \
    else \
        colors.sel##Fg = fg; \
    if (!bg) \
        colors.sel##Bg = newtDefaultColorPalette.sel##Bg; \
    else \
        colors.sel##Bg = newtDefaultColorPalette.sel##Bg;

static void
f_setColors(INT32 args)
{
    struct newtColors    colors;
    struct mapping       *sc;
    char                 *fg, *bg;
    
    if (args != 1)
        FERROR("setColors", "Expected 1 argument1, got %d instead", args);

    if (ARG(1).type != T_MAPPING)
        FERROR("setColors", "Wrong argument type for argument %d. Expected a mapping.", 1);

    sc = ARG(1).u.mapping;
    
    get_color(sc, "root", &fg, &bg);
    SET_COLOR(root);

    get_color(sc, "border", &fg, &bg);
    SET_COLOR(border);

    get_color(sc, "window", &fg, &bg);
    SET_COLOR(window);

    get_color(sc, "shadow", &fg, &bg);
    SET_COLOR(shadow);

    get_color(sc, "title", &fg, &bg);
    SET_COLOR(title);

    get_color(sc, "button", &fg, &bg);
    SET_COLOR(button);

    get_color(sc, "actButton", &fg, &bg);
    SET_COLOR(actButton);

    get_color(sc, "checkbox", &fg, &bg);
    SET_COLOR(checkbox);

    get_color(sc, "actCheckbox", &fg, &bg);
    SET_COLOR(actCheckbox);

    get_color(sc, "entry", &fg, &bg);
    SET_COLOR(entry);

    get_color(sc, "label", &fg, &bg);
    SET_COLOR(label);

    get_color(sc, "listbox", &fg, &bg);
    SET_COLOR(listbox);

    get_color(sc, "actListbox", &fg, &bg);
    SET_COLOR(actListbox);

    get_color(sc, "textbox", &fg, &bg);
    SET_COLOR(textbox);

    get_color(sc, "actTextbox", &fg, &bg);
    SET_COLOR(actTextbox);

    get_color(sc, "helpLine", &fg, &bg);
    SET_COLOR(helpLine);

    get_color(sc, "rootText", &fg, &bg);
    SET_COLOR(rootText);

    get_color(sc, "disabledEntry", &fg, &bg);
    SET_COLOR(disabledEntry);

    get_color(sc, "compactButton", &fg, &bg);
    SET_COLOR(compactButton);

    get_color(sc, "actSelListbox", &fg, &bg);
    SET_COLOR(actSelListbox);

    get_color(sc, "selListbox", &fg, &bg);
    SET_COLOR(selListbox);

    get_color(sc, "emptyScale", &fg, &bg);
    if (!fg)
        colors.emptyScale = newtDefaultColorPalette.emptyScale;
    else
        colors.emptyScale = fg;
    if (!bg)
        colors.fullScale = newtDefaultColorPalette.fullScale;
    else 
        colors.fullScale = newtDefaultColorPalette.fullScale;

    pop_n_elems(args);
    
    newtSetColors(colors);
}

static void
f_refresh(INT32 args)
{
    INFUN();
    
    newtRefresh();
    pop_n_elems(args);

    OUTFUN();
}

static void
f_suspend(INT32 args)
{
    INFUN();
    
    newtSuspend();
    pop_n_elems(args);

    OUTFUN();
}

/* IMPLEMENT */
static void
f_setSuspendCallback(INT32 args)
{}

static void
f_resume(INT32 args)
{
    INFUN();
        
    newtResume();
    pop_n_elems(args);

    OUTFUN();
}

static void
f_pushHelpLine(INT32 args)
{
    INFUN();
    
    if (args != 1) {
        OUTFUN();
        FERROR("pushHelpLine", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("pushHelpLine", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    
    newtPushHelpLine(ARG(1).u.string->str);
    pop_n_elems(args);

    OUTFUN();
}

static void
f_redrawHelpLine(INT32 args)
{
    INFUN();
    
    newtRedrawHelpLine();
    pop_n_elems(args);

    OUTFUN();
}

static void
f_popHelpLine(INT32 args)
{
    INFUN();
    
    newtPopHelpLine();
    pop_n_elems(args);

    OUTFUN();
}

static void
f_drawRootText(INT32 args)
{
    INFUN();
    
    if (args != 3) {
        OUTFUN();
        FERROR("drawRootText", "Wrong number of arguments. Expected %d got %d.", 3, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("drawRootText", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("drawRootText", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    
    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("drawRootText", "Wrong argument type for argument %d. Expected an 8-bit string.", 3);
    }
    
    newtDrawRootText(ARG(1).u.integer, ARG(2).u.integer, ARG(3).u.string->str);
    pop_n_elems(args);

    OUTFUN();
}

static void
f_bell(INT32 args)
{
    INFUN();
    
    newtBell();
    pop_n_elems(args);

    OUTFUN();
}

#ifdef HAVE_NEWTCURSOROFF
static void
f_cursorOff(INT32 args)
{
    INFUN();
    
    newtCursorOff();
    pop_n_elems(args);

    OUTFUN();
}
#endif

#ifdef HAVE_NEWTCURSORON
static void
f_cursorOn(INT32 args)
{
    INFUN();
    
    newtCursorOn();
    pop_n_elems(args);

    OUTFUN();
}
#endif

/* COMPONENTS */
/* A few component notes.
 * All the Newt functions that create some component can be called only
 * from their corresponding wrapper class - e.g. compactButton and button
 * can be called only by an object that belongs to the Button class. This
 * is done so because those functions return component structure that has
 * to be stored in correct object.
 */

static void
func_prolog(char *fn,
            unsigned *classids,
            struct object *obj,
            unsigned *id,
            int is_create)
{
    unsigned myId;

    INFUN();

    myId = is_known_class(obj);
    
    if (!(myId)) {
        OUTFUN();
        FERROR(fn, "Unknown class ID");
    }
    
    if (!classids) {
        OUTFUN();
        return; /* Any class is OK */
    }
    
    /* Search the array */
    {
        unsigned *tmp = classids;
        
        while(tmp && *tmp) {
            if (*tmp == myId) {
                if (id)
                    *id = myId;                

                if (!is_create &&
                    (!THIS_OBJ(obj)->u.component || !THIS_OBJ(obj)->created || THIS_OBJ(obj)->destroyed)) {
                    OUTFUN();
                    FERROR(fn, "Caller object hasn't got the associated component created yet!");
                }
                
                OUTFUN();
                return;
            }
            
            tmp++;
        }
    }

    OUTFUN();
    FERROR(fn, "Function called from an incorrect class instance '%s'",
           get_class_name(obj));
}

static void
button_create(char *fn, int isCompact, INT32 args)
{
    static unsigned  ids[] = {CLASS_BUTTON, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog(fn, ids, caller, NULL, 1);

    if (args != 3) {
        OUTFUN();
        FERROR(fn, "Wrong number of arguments. Expected %d got %d.", 3, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR(fn, "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR(fn, "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    
    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0) {
        OUTFUN();
        FERROR(fn, "Wrong argument type for argument %d. Expected an 8-bit string.", 3);
    }
    
    THIS_OBJ(caller)->u.component =
        isCompact ? newtCompactButton(ARG(1).u.integer,
                                      ARG(2).u.integer,
                                      ARG(3).u.string->str)
        : newtButton(ARG(1).u.integer,
                     ARG(2).u.integer,
                     ARG(3).u.string->str);
    
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);

    OUTFUN();
}

static void
f_compactButton(INT32 args)
{
    INFUN();
    
    button_create("compactButton", 1, args);
    pop_n_elems(args);

    OUTFUN();
}

static void
f_button(INT32 args)
{
    INFUN();
    
    button_create("button", 0, args);
    pop_n_elems(args);

    OUTFUN();
}

static void
f_checkbox(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int             left, top;
    char            *text, *seq = NULL;
    char            defValue;

    INFUN();
    
    func_prolog("checkbox", ids, caller, NULL, 1);

    if (args < 3 || args > 5) {
        OUTFUN();
        FERROR("checkbox", "Wrong number of arguments. Expected %d-%d got %d.", 3, 5, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checkbox", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("checkbox", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;
    
    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkbox", "Wrong argument type for argument %d. Expected an 8-bit string.", 3);
    }
    text = ARG(3).u.string->str;
    
    if (args > 3) {
        if (ARG(4).type != T_STRING || ARG(4).u.string->size_shift > 0) {
            OUTFUN();
            FERROR("checkbox", "Wrong argument type for argument %d. Expected an 8-bit string.", 4);
        }
        defValue = ARG(4).u.string->len ? ARG(4).u.string->str[0] : '\x0';
        
        if (args > 4) {
            if (ARG(5).type != T_STRING || ARG(5).u.string->size_shift > 0)
            {
                OUTFUN();
                FERROR("checkbox", "Wrong argument type for argument %d. Expected an 8-bit string.", 5);
            }
            seq = ARG(5).u.string->len ? ARG(5).u.string->str : NULL;
        } else {
            seq = NULL;
        }
    } else {
        defValue = '\x0';
    }
    
    THIS_OBJ(caller)->u.component = newtCheckbox(left, top, text, defValue, seq, NULL);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_checkboxGetValue(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *result = " ";

    INFUN();
    
    func_prolog("checkboxGetValue", ids, caller, NULL, 0);

    pop_n_elems(args);

    result[0] = newtCheckboxGetValue(THIS_OBJ(caller)->u.component);
    push_string(make_shared_string(result));

    OUTFUN();
}

static void
f_checkboxSetValue(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("checkboxSetValue", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("checkboxSetValue", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxSetValue", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    
    if (!ARG(1).u.string->len) {
        OUTFUN();
        FERROR("checkboxSetValue", "Cannot set value from an empty string");
    }
    
    newtCheckboxSetValue(THIS_OBJ(caller)->u.component, ARG(1).u.string->str[0]);
    pop_n_elems(args);

    OUTFUN();
}

/* IMPLEMENT */
static void
f_checkboxSetFlags(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_radiobutton(INT32 args)
{
    static unsigned  ids[] = {CLASS_RADIOBUTTON, CLASS_RADIOBAR, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int             left, top;
    char            *text;
    newtComponent   prev = NULL;
    char            isDefault;
    int             id;

    INFUN();
    
    func_prolog("checkbox", ids, caller, NULL, 1);

    if (args < 3 || args > 5) {
        OUTFUN();
        FERROR("radtiobutton", "Wrong number of arguments. Expected %d-%d got %d.", 3, 5, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("radiobutton", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("radiobutton", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;
    
    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("radiobutton", "Wrong argument type for argument %d. Expected an 8-bit string.", 3);
    }
    text = ARG(3).u.string->str;
    
    if (args > 3) {
        if (ARG(4).type != T_INT) {
            OUTFUN();
            FERROR("radiobutton", "Wrong argument type for argument %d. Expected an integer.", 4);
        }
        isDefault = ARG(4).u.integer;

        if (args > 4) {
            if (ARG(5).type != T_OBJECT) {
                OUTFUN();
                FERROR("radiobutton", "Wrong argument type for argument %d. Expected an object.", 5);
            }
            id = is_known_class(ARG(5).u.object);
        
            if (!id || id != CLASS_RADIOBUTTON) {
                OUTFUN();
                FERROR("radiobutton", "Incorrect object type in argument %d", 5);
            }
            
            prev = THIS_OBJ(ARG(5).u.object)->u.component;
        } else {
            prev = NULL;
        }
    } else {
        isDefault = 0;
    }
    
    THIS_OBJ(caller)->u.component = newtRadiobutton(left, top, text, isDefault, prev);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_radioGetCurrent(INT32 args)
{
    static unsigned  ids[] = {CLASS_RADIOGROUP, CLASS_RADIOBAR, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              id;
    newtComponent    cur;

    INFUN();
    
    func_prolog("radioGetCurrent", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("radioGetCurrent", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_OBJECT) {
        OUTFUN();
        FERROR("radioGetCurrent", "Wrong argument type for argument %d. Expected an object.", 1);
    }
    id = is_known_class(ARG(1).u.object);
    
    if (!id || (id != CLASS_RADIOBUTTON || id != CLASS_RADIOBAR)) {
        OUTFUN();
        FERROR("radioGetCurrent", "Incorrect object type in argument %d", 1);
    }
    cur = newtRadioGetCurrent(THIS_OBJ(ARG(1).u.object)->u.component);

    /*
     * TODO: Implement searching of a button group/bar for the object that
     * contains 'cur' retrieved above and return that object.
     */

    pop_n_elems(args);
    
    push_int(0);

    OUTFUN();
}

#ifdef HAVE_NEWTLISTITEM
static void
f_listitem(INT32 args) /* Not implemented in the Newt library */
{}
#endif

#ifdef HAVE_NEWTLISTITEMSET
static void
f_listitemSet(INT32 args) /* Not implemented in the Newt library */
{}
#endif

#ifdef HAVE_NEWTLISTITEMGETDATA
static void
f_listitemGetData(INT32 args) /* Not implemented in the Newt library */
{}
#endif

static void
f_getScreenSize(INT32 args)
{
    int             cols, rows;
    struct mapping  *ret;
    struct svalue   skey, sval;

    INFUN();
    
    pop_n_elems(args);
    
    newtGetScreenSize(&cols, &rows);

    ret = allocate_mapping(2);
    skey.type = T_STRING;
    sval.type = T_INT;
    
    skey.u.string = make_shared_string("cols");
    sval.u.integer = cols;
    mapping_insert(ret, &skey, &sval);

    skey.u.string = make_shared_string("rows");
    sval.u.integer = rows;
    mapping_insert(ret, &skey, &sval);

    push_mapping(ret);

    OUTFUN();
}

static void
f_label(INT32 args)
{
    static unsigned  ids[] = {CLASS_LABEL, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int             left, top;
    char            *text;

    INFUN();
    
    func_prolog("label", ids, caller, NULL, 1);

    if (args != 3) {
        OUTFUN();
        FERROR("label", "Wrong number of arguments. Expected %d got %d.", 3, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("label", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("label", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;
    
    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("label", "Wrong argument type for argument %d. Expected an 8-bit string.", 3);
    }
    text = ARG(3).u.string->str;

    THIS_OBJ(caller)->u.component = newtLabel(left, top, text);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_labelSetText(INT32 args)
{
    static unsigned  ids[] = {CLASS_LABEL, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *text;

    INFUN();
    
    func_prolog("label", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("labelSetText", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("labelSetText", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    text = ARG(1).u.string->str;

    newtLabelSetText(THIS_OBJ(caller)->u.component, text);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_verticalScrollbar(INT32 args)
{
    static unsigned  ids[] = {CLASS_VSCROLLBAR, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, height;
    int              normalColor = 0, thumbColor = 0;

    INFUN();
    
    func_prolog("verticalScrollbar", ids, caller, NULL, 1);

    if (args < 3 || args > 5) {
        OUTFUN();
        FERROR("verticalScrollbar", "Wrong number of arguments. Expected %d-%d got %d.", 3, 5, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("verticalScrollbar", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("verticalScrollbar", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("verticalScrollbar", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    height = ARG(3).u.integer;

    if (args > 3) {
        if (ARG(4).type != T_INT) {
            OUTFUN();
            FERROR("verticalScrollbar", "Wrong argument type for argument %d. Expected an integer.", 4);
        }
        normalColor = ARG(4).u.integer;

        if (args > 4) {
            if (ARG(5).type != T_INT) {
                OUTFUN();
                FERROR("verticalScrollbar", "Wrong argument type for argument %d. Expected an integer.", 5);
            }
            thumbColor = ARG(5).u.integer;
        }
    }

    THIS_OBJ(caller)->u.component = newtVerticalScrollbar(left, top, height, normalColor, thumbColor);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_scrollbarSet(INT32 args)
{
    static unsigned  ids[] = {CLASS_VSCROLLBAR, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              where, total;

    INFUN();
    
    func_prolog("scrollbarSet", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("scrollbarSet", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("scrol   lbarSet", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    where = ARG(1).u.integer;
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("scrollbarSet", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    total = ARG(2).u.integer;

    newtScrollbarSet(THIS_OBJ(caller)->u.component, where, total);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listbox(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, height;
    int              flags;

    INFUN();
    
    func_prolog("listbox", ids, caller, NULL, 1);

    if (args < 3 || args > 4) {
        OUTFUN();
        FERROR("listbox", "Wrong number of arguments. Expected %d-%d got %d.", 3, 4, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listbox", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("listbox", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("listbox", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    height = ARG(3).u.integer;

    if (args > 3) {
        if (ARG(4).type != T_INT) {
            OUTFUN();
            FERROR("listbox", "Wrong argument type for argument %d. Expected an integer.", 4);
        }
        flags = ARG(4).u.integer;
    } else {
        flags = 0;
    }

    THIS_OBJ(caller)->u.component = newtListbox(left, top, height, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxGetCurrent(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    void            *cur;

    INFUN();
    
    func_prolog("listboxGetCurrent", ids, caller, NULL, 0);

    cur = newtListboxGetCurrent(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    push_int((int)cur);

    OUTFUN();
}

static void
f_listboxSetCurrent(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              cur;

    INFUN();
    
    func_prolog("listboxSetCurrent", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("listboxSetCurrent", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxSetCurrent", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    cur = ARG(1).u.integer;

    newtListboxSetCurrent(THIS_OBJ(caller)->u.component, cur);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxSetCurrentByKey(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    void            *cur;

    INFUN();
    
    func_prolog("listboxSetCurrentByKey", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("listboxSetCurrentByKey", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxSetCurrentByKey", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    cur = (void*)(ARG(1).u.integer);

    newtListboxSetCurrentByKey(THIS_OBJ(caller)->u.component, cur);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxSetEntry(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              num;
    char             *text;

    INFUN();
    
    func_prolog("listboxSetEntry", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("listboxSetEntry", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxSetEntry", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    num = ARG(1).u.integer;
    
    if (ARG(2).type != T_STRING || ARG(2).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("listboxSetEntry", "Wrong argument type for argument %d. Expected an 8-bit string.", 2);
    }
    text = ARG(2).u.string->str;

    newtListboxSetEntry(THIS_OBJ(caller)->u.component, num, text);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxSetWidth(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              width;

    INFUN();
    
    func_prolog("listboxSetWidth", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("listboxSetWidth", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxSetWidth", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    width = ARG(1).u.integer;

    newtListboxSetWidth(THIS_OBJ(caller)->u.component, width);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxSetData(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              num, key;

    INFUN();
    
    func_prolog("listboxSetData", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("listboxSetData", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxSetData", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    num = ARG(1).u.integer;
    
    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("listboxSetData", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    key = ARG(2).u.integer;

    newtListboxSetData(THIS_OBJ(caller)->u.component, num, (void*)key);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxAppendEntry(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key;
    char            *text;
    int              ret;

    INFUN();
    
    func_prolog("listboxAppendEntry", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("listboxAppendEntry", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("listboxAppendEntry", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    text = ARG(1).u.string->str;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("listboxAppendEntry", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    key = ARG(2).u.integer;

    /*
     * Return value seems to be unused in the Newt sources, but we will
     * return it to the caller anyway.
     */
    ret = newtListboxAppendEntry(THIS_OBJ(caller)->u.component, text, (void*)key);

    pop_n_elems(args);

    push_int(ret);

    OUTFUN();
}

static void
f_listboxInsertEntry(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key, keyAfter;
    char            *text;
    int              ret;

    INFUN();
    
    func_prolog("listboxInsertEntry", ids, caller, NULL, 0);

    if (args != 3) {
        OUTFUN();
        FERROR("listboxInsertEntry", "Wrong number of arguments. Expected %d got %d.", 3, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("listboxInsertEntry", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    text = ARG(1).u.string->str;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("listboxInsertEntry", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    key = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("listboxInsertEntry", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    keyAfter = ARG(3).u.integer;

    ret = newtListboxInsertEntry(THIS_OBJ(caller)->u.component, text, (void*)key, (void*)keyAfter);

    pop_n_elems(args);

    push_int(ret);

    OUTFUN();
}

static void
f_listboxDeleteEntry(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key;
    int              ret;

    INFUN();
    
    func_prolog("listboxDeleteEntry", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("listboxDeleteEntry", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxDeleteEntry", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    key = ARG(1).u.integer;

    ret = newtListboxDeleteEntry(THIS_OBJ(caller)->u.component, (void*)key);

    pop_n_elems(args);

    push_int(ret);

    OUTFUN();
}

static void
f_listboxClear(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("listboxClear", ids, caller, NULL, 0);

    newtListboxClear(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxGetEntry(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              num;
    void            *key;
    char            *text;
    struct mapping  *ret;
    struct svalue    skey, sval;

    INFUN();
    
    func_prolog("listboxGetEntry", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("listboxDeleteEntry", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxDeleteEntry", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    num = ARG(1).u.integer;

    pop_n_elems(args);
    
    newtListboxGetEntry(THIS_OBJ(caller)->u.component, num, &text, &key);

    ret = allocate_mapping(2);
    skey.type = T_STRING;
    skey.u.string = make_shared_string("text");
    sval.type = T_STRING;
    sval.u.string = make_shared_string(text);

    skey.u.string = make_shared_string("key");
    sval.type = T_INT;
    sval.u.integer = (int)key;
    
    mapping_insert(ret, &skey, &sval);

    push_mapping(ret);

    OUTFUN();
}

static void
f_listboxGetSelection(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              numitems, i;
    int            **items;
    struct array    *ret;

    INFUN();
    
    func_prolog("listboxGetSelection", ids, caller, NULL, 0);

    (void**)items = newtListboxGetSelection(THIS_OBJ(caller)->u.component, &numitems);

    pop_n_elems(args);
    
    /* Use numitems, as item can also be NULL... */
    for(i = 0; i < numitems; i++)
        push_int((int)(*(items + i)));

    ret = aggregate_array(numitems);

    if (ret)
        push_array(ret);
    else
        push_int(0);

    OUTFUN();
}

static void
f_listboxClearSelection(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("listboxClearSelection", ids, caller, NULL, 0);

    newtListboxClearSelection(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_listboxSelectItem(INT32 args)
{
    static unsigned  ids[] = {CLASS_LISTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key;
    int              flags;

    INFUN();
    
    func_prolog("listboxSelectItem", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("listboxSelectItem", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("listboxSelectItem", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    key = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("listboxSelectItem", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    flags = ARG(1).u.integer;

    newtListboxSelectItem(THIS_OBJ(caller)->u.component, (void*)key, flags);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_checkboxTree(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, height, flags;

    INFUN();
    
    func_prolog("checkboxTree", ids, caller, NULL, 1);

    if (args != 4) {
        OUTFUN();
        FERROR("checkboxTree", "Wrong number of arguments. Expected %d got %d.", 4, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checboxTree", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("checboxTree", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("checboxTree", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    height = ARG(3).u.integer;

    if (ARG(4).type != T_INT) {
        OUTFUN();
        FERROR("checboxTree", "Wrong argument type for argument %d. Expected an integer.", 4);
    }
    flags = ARG(4).u.integer;

    THIS_OBJ(caller)->u.component = newtCheckboxTree(left, top, height, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_checkboxTreeMulti(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, height, flags;
    char             *seq;

    INFUN();
    
    func_prolog("checkboxTreeMulti", ids, caller, NULL, 1);

    if (args != 4) {
        OUTFUN();
        FERROR("checkboxTreeMulti", "Wrong number of arguments. Expected %d got %d.", 4, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeMulti", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeMulti", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeMulti", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    height = ARG(3).u.integer;

    if (ARG(4).type != T_STRING || ARG(4).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxTreeMulti", "Wrong argument type for argument %d. Expected an 8-bit string.", 4);
    }
    seq = ARG(4).u.string->str;
    
    if (ARG(5).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeMulti", "Wrong argument type for argument %d. Expected an integer.", 5);
    }
    flags = ARG(5).u.integer;

    THIS_OBJ(caller)->u.component = newtCheckboxTreeMulti(left, top, height, seq, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_checkboxTreeGetSelection(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              numitems, i;
    int            **items;
    struct array    *ret;

    INFUN();
    
    func_prolog("checkboxTreeGetSelection", ids, caller, NULL, 0);

    (void**)items = newtCheckboxTreeGetSelection(THIS_OBJ(caller)->u.component, &numitems);

    pop_n_elems(args);
    
    /* Use numitems, as item can also be NULL... */
    for(i = 0; i < numitems; i++)
        push_int((int)(*(items + i)));

    ret = aggregate_array(numitems);

    if (ret)
        push_array(ret);
    else
        push_int(0);

    OUTFUN();
}

static void
f_checkboxTreeGetCurrent(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              ret;

    INFUN();
    
    func_prolog("checkboxTreeGetCurrent", ids, caller, NULL, 0);

    (void*)ret = newtCheckboxTreeGetCurrent(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    push_int(ret);

    OUTFUN();
}

static void
f_checkboxTreeGetMultiSelection(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              numitems, i;
    char             seqnum;
    int            **items;
    struct array    *ret;

    INFUN();
    
    func_prolog("checkboxTreeGetMultiSelection", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("checkboxTreeGetMultiSelection", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxTreeGetMultiSelection", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    
    if (!ARG(1).u.string->len) {
        OUTFUN();
        FERROR("checkboxTreeGetMultiSelection", "Cannot use an empty string.");
    }
    
    seqnum = ARG(1).u.string->str[0];
    
    pop_n_elems(args);

    (void**)items = newtCheckboxTreeGetMultiSelection(THIS_OBJ(caller)->u.component, &numitems, seqnum);
    
    /* Use numitems, as item can also be NULL... */
    for(i = 0; i < numitems; i++)
        push_int((int)(*(items + i)));

    ret = aggregate_array(numitems);

    if (ret)
        push_array(ret);
    else
        push_int(0);

    OUTFUN();
}

static void
f_checkboxTreeAddArray(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *text;
    int              key, flags, *indexes, i;
    struct array    *arr;

    INFUN();
    
    func_prolog("checkboxTreeAddArray", ids, caller, NULL, 0);

    if (args != 4) {
        OUTFUN();
        FERROR("checkboxTreeAddArray", "Wrong number of arguments. Expected %d got %d.", 4, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxTreeAddArray", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    text = ARG(1).u.string->str;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeAddArray", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    key = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeAddArray", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    flags = ARG(3).u.integer;

    if (ARG(4).type != T_ARRAY) {
        OUTFUN();
        FERROR("checboxTreeAddArray", "Wrong argument type for argument %d. Expected an array.", 4);
    }
    arr = ARG(4).u.array;

    indexes = (int*)malloc((arr->size + 1) * sizeof(int));
    if (!arr) {
        OUTFUN();
        FERROR("checkboxTreeAddArray", "Out of memory allocating indexes array (%u bytes)",
               arr->size);
    }
    
    for(i = 0; i < arr->size; i++) {
        if (arr->item[0].type == T_INT)
            indexes[i] = arr->item[i].u.integer;
        else
            indexes[i] = 0; /* Or should we shout?? */
    }
    indexes[i] = NEWT_ARG_LAST;
    
    i = newtCheckboxTreeAddArray(THIS_OBJ(caller)->u.component, text, (void*)key, flags, indexes);

    free(indexes);
    
    pop_n_elems(args);

    push_int(i);

    OUTFUN();
}

static void
f_checkboxTreeFindItem(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key, i;
    int              *ret;
    struct array     *arr;

    INFUN();
    
    func_prolog("checkboxTreeFindItem", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("checkboxTreeFindItem", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeFindItem", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    key = ARG(1).u.integer;

    pop_n_elems(args);
    
    ret = newtCheckboxTreeFindItem(THIS_OBJ(caller)->u.component, (void*)key);

    if (!ret) {
        push_int(0);
        OUTFUN();
        return;
    }

    i = 0;
    while(*ret != NEWT_ARG_LAST)
        push_int((i++, *ret++));

    arr = aggregate_array(i);

    push_array(arr);

    OUTFUN();
}

#ifdef HAVE_NEWTCHECKBOXTREESETENTRY
static void
f_checkboxTreeSetEntry(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *text;
    int              key;

    INFUN();
    
    func_prolog("checkboxTreeSetEntry", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("checkboxTreeSetEntry", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeSetEntry", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    key = ARG(1).u.integer;
    
    if (ARG(2).type != T_STRING || ARG(2).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxTreeSetEntry", "Wrong argument type for argument %d. Expected an 8-bit string.", 2);
    }
    text = ARG(2).u.string->str;

    newtCheckboxTreeSetEntry(THIS_OBJ(caller)->u.component, (void*)key, text);

    pop_n_elems(args);

    OUTFUN();
}
#endif

#ifdef HAVE_NEWTCHECKBOXTREEGETENTRYVALUE
static void
f_checkboxTreeGetEntryValue(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key;
    char             *ret = " ";

    INFUN();
    
    func_prolog("checkboxTreeGetEntryValue", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("checkboxTreeGetEntryValue", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeGetEntryValue", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    
    key = ARG(1).u.integer;

    ret[0] = newtCheckboxTreeGetEntryValue(THIS_OBJ(caller)->u.component, (void*)key);

    pop_n_elems(args);

    push_string(make_shared_string(ret));

    OUTFUN();
}
#endif

#ifdef HAVE_NEWTCHECKBOXTREESETENTRYVALUE
static void
f_checkboxTreeSetEntryValue(INT32 args)
{
    static unsigned  ids[] = {CLASS_CHECKBOXTREE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key;
    char             val;

    INFUN();
    
    func_prolog("checkboxTreeSetEntryValue", ids, caller, NULL, 0);

    if (args != 2) {
        OUTFUN();
        FERROR("checkboxTreeSetEntryValue", "Wrong number of arguments. Expected %d got %d.", 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("checboxTreeSetEntryValue", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    key = ARG(1).u.integer;

    if (ARG(2).type != T_STRING || ARG(2).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxTreeSetEntryValue", "Wrong argument type for argument %d. Expected an 8-bit string.", 2);
    }
    
    if (!ARG(2).u.string->len) {
        OUTFUN();
        FERROR("checkboxTreeSetEntryValue", "Cannot set value from an empty string.");
    }
    
    val = ARG(2).u.string->str[0];

    newtCheckboxTreeSetEntryValue(THIS_OBJ(caller)->u.component, (void*)key, val);

    pop_n_elems(args);

    OUTFUN();
}
#endif

static void
f_textboxReflowed(INT32 args)
{
    static unsigned  ids[] = {CLASS_TEXTBOXREFLOWED, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, width, flexDown = 5, flexUp = 5, flags = 0;
    char            *text;

    INFUN();
    
    func_prolog("textboxReflowed", ids, caller, NULL, 1);

    if (args < 4 || args > 7) {
        OUTFUN();
        FERROR("textboxReflowed", "Wrong number of arguments. Expected %d-%d got %d.", 4, 7, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("textboxReflowed", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("textboxReflowed", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_STRING || ARG(3).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("checkboxTreeSetEntryValue", "Wrong argument type for argument %d. Expected an 8-bit string.", 3);
    }
    text = ARG(3).u.string->str;
    
    if (ARG(4).type != T_INT) {
        OUTFUN();
        FERROR("textboxReflowed", "Wrong argument type for argument %d. Expected an integer.", 4);
    }
    width = ARG(4).u.integer;

    switch (args) {
        case 7: 
            if (ARG(7).type != T_INT) {
                OUTFUN();
                FERROR("textboxReflowed", "Wrong argument type for argument %d. Expected an integer.", 7);
            }
            flags = ARG(7).u.integer;
            /* Fall through */
            
        case 6:
            if (ARG(6).type != T_INT) {
                OUTFUN();
                FERROR("textboxReflowed", "Wrong argument type for argument %d. Expected an integer.", 6);
            }
            flexUp = ARG(6).u.integer;
            /* Fall through */

        case 5:
            if (ARG(5).type != T_INT) {
                OUTFUN();
                FERROR("textboxReflowed", "Wrong argument type for argument %d. Expected an integer.", 5);
            }
            flexDown = ARG(5).u.integer;
    }

    THIS_OBJ(caller)->u.component = newtTextboxReflowed(left, top, text, width, flexDown, flexUp, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_textbox(INT32 args)
{
    static unsigned  ids[] = {CLASS_TEXTBOX, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, width, height, flags = 0;

    INFUN();
    
    func_prolog("textbox", ids, caller, NULL, 1);

    if (args < 4 || args > 5) {
        OUTFUN();
        FERROR("textbox", "Wrong number of arguments. Expected %d-%d got %d.", 4, 5, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("textbox", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("textbox", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("textbox", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    width = ARG(3).u.integer;

    if (ARG(4).type != T_INT) {
        OUTFUN();
        FERROR("textbox", "Wrong argument type for argument %d. Expected an integer.", 4);
    }
    height = ARG(4).u.integer;

    if (args > 4) {
        if (ARG(5).type != T_INT) {
            OUTFUN();
            FERROR("textbox", "Wrong argument type for argument %d. Expected an integer.", 5);
        }
        height = ARG(5).u.integer;
    }

    THIS_OBJ(caller)->u.component = newtTextbox(left, top, width, height, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_textboxSetText(INT32 args)
{
    static unsigned  ids[] = {CLASS_TEXTBOX, CLASS_TEXTBOXREFLOWED, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *text;

    INFUN();
    
    func_prolog("textboxSetText", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("textboxSetText", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("textboxSetText", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    text = ARG(1).u.string->str;

    newtTextboxSetText(THIS_OBJ(caller)->u.component, text);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_textboxSetHeight(INT32 args)
{
    static unsigned  ids[] = {CLASS_TEXTBOX, CLASS_TEXTBOXREFLOWED, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              height;

    INFUN();
    
    func_prolog("textboxSetHeight", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("textboxSetHeight", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("textboxSetHeight", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    height = ARG(1).u.integer;

    newtTextboxSetHeight(THIS_OBJ(caller)->u.component, height);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_textboxGetNumLines(INT32 args)
{
    static unsigned  ids[] = {CLASS_TEXTBOX, CLASS_TEXTBOXREFLOWED, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              ret;

    INFUN();
    
    func_prolog("textboxGetNumLines", ids, caller, NULL, 0);

    ret = newtTextboxGetNumLines(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    push_int(ret);

    OUTFUN();
}

static void
f_reflowText(INT32 args)
{
    char            *text;
    int              width, flexDown = 5, flexUp = 5;
    char            *reflown;
    int              actualWidth, actualHeight;
    struct mapping  *ret;
    struct svalue    skey, sval;

    INFUN();
    
    if (args < 2 || args > 4) {
        OUTFUN();
        FERROR("reflowText", "Wrong number of arguments. Expected %d-%d got %d.", 2, 4, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("reflowText", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    text = ARG(1).u.string->str;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("reflowText", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    width = ARG(2).u.integer;

    switch(args) {
        case 4:
            if (ARG(4).type != T_INT) {
                OUTFUN();
                FERROR("reflowText", "Wrong argument type for argument %d. Expected an integer.", 4);
            }
            flexUp = ARG(4).u.integer;
            /* Fall through */

        case 3:
            if (ARG(3).type != T_INT) {
                OUTFUN();
                FERROR("reflowText", "Wrong argument type for argument %d. Expected an integer.", 3);
            }
            flexUp = ARG(3).u.integer;
    }

    reflown = newtReflowText(text, width, flexDown, flexUp, &actualWidth, &actualHeight);
    if (!reflown) {
        push_int(0);
        OUTFUN();
        return;
    }
    
    pop_n_elems(args);

    ret = allocate_mapping(3);
    skey.type = T_STRING;
    skey.u.string = make_shared_string("text");
    sval.type = T_STRING;
    sval.u.string = make_shared_string(reflown);
    mapping_insert(ret, &skey, &sval);
    free(reflown);

    skey.u.string = make_shared_string("actualWidth");
    sval.type = T_INT;
    sval.u.integer = actualWidth;
    mapping_insert(ret, &skey, &sval);

    skey.u.string = make_shared_string("actualHeight");
    sval.u.integer = actualHeight;
    mapping_insert(ret, &skey, &sval);

    push_mapping(ret);

    OUTFUN();
}

static void
f_form(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              flags = 0; /* To shut up the stupid compiler warning */
    char            *help = NULL;
    newtComponent    vertBar = NULL;

    INFUN();
    
    func_prolog("form", ids, caller, NULL, 1);

    if (args > 3) {
        OUTFUN();
        FERROR("form", "Wrong number of arguments. Expected %d-%d got %d.", 0, 3, args);
    }
    
    switch(args) {
        case 3:
            if (ARG(3).type != T_INT) {
                OUTFUN();
                FERROR("form", "Wrong argument type for argument %d. Expected an integer.", 3);
            }
            flags = ARG(3).u.integer;
            /* Fall through */

        case 2:
            if (ARG(2).type != T_STRING || ARG(2).u.string->size_shift > 0)
            {
                OUTFUN();
                FERROR("form", "Wrong argument type for argument %d. Expected an 8-bit string.", 2);
            }
            help = ARG(2).u.string->str;
            /* fall through */

        case 1:
            if (ARG(1).type == T_INT)
                vertBar = NULL;
            else if (ARG(1).type == T_OBJECT) {
                unsigned id = is_known_class(ARG(1).u.object);

                if (!id || id != CLASS_VSCROLLBAR) {
                    OUTFUN();
                    FERROR("form", "Incorrect object type in argument %d. Expected a VScrollBar.", 1);
                }
                
                if (THIS_OBJ(ARG(1).u.object)->u.component)
                    vertBar = THIS_OBJ(ARG(1).u.object)->u.component;
                else
                    vertBar = NULL; /* Or should we yell? */
            } else {
                OUTFUN();
                FERROR("form", "Wrong argument type for argument %d. Expected an object", 1);
            }
            break;
            
        default:
            vertBar = NULL;
            flags = 0;
            help = NULL;
            break;
    }

    THIS_OBJ(caller)->u.component = newtForm(vertBar, help, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

#ifdef HAVE_NEWTFORMSETTIMER
static void
f_formSetTimer(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              msecs;

    INFUN();
    
    func_prolog("formSetTimer", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("formSetTimer", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("formSetTimer", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    msecs = ARG(1).u.integer;

    newtFormSetTimer(THIS_OBJ(caller)->u.component, msecs);

    pop_n_elems(args);

    OUTFUN();
}
#endif

/* IMPLEMENT */
/* We need to take a Stdio.File or an int as the arg (?)*/
static void
f_formWatchFd(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_formSetSize(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("formSetSize", ids, caller, NULL, 0);

    newtFormSetSize(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_formGetCurrent(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    struct object   *obj;
    newtComponent    comp;

    INFUN();
    
    func_prolog("formGetCurrent", ids, caller, NULL, 0);

    comp = newtFormGetCurrent(THIS_OBJ(caller)->u.component);
    
    obj = dict_lookup(comp);

    pop_n_elems(args);

    push_object(obj);

    OUTFUN();
}

static void
f_formSetBackground(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              color;

    INFUN();
    
    func_prolog("formSetBackground", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("formSetBackground", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("formSetBackground", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    color = ARG(1).u.integer;

    newtFormSetBackground(THIS_OBJ(caller)->u.component, color);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_formSetCurrent(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              color;
    unsigned         id;

    INFUN();
    
    func_prolog("formSetCurrent", ids, caller, NULL, 0);
    
    if (args != 1) {
        OUTFUN();
        FERROR("formSetCurrent", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_OBJECT) {
        OUTFUN();
        FERROR("formSetCurrent", "Wrong argument type for argument %d. Expected an object.", 1);
    }
    
    id = is_known_class(ARG(1).u.object);

    if (!id) {
        OUTFUN();
        FERROR("formSetCurrent", "Wrong object type for argument %d. Expected a Newt class.", 1);
    }
    
    if (!THIS_OBJ(ARG(1).u.object)->u.component) {
        OUTFUN();
        FERROR("formSetCurrent", "Cannot use a destroyed object for argument %d.", 1);
    }
    
    newtFormSetCurrent(THIS_OBJ(caller)->u.component, THIS_OBJ(ARG(1).u.object)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_formAddComponent(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    unsigned         id;

    INFUN();
    
    func_prolog("formAddComponent", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("formAddComponent", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_OBJECT) {
        OUTFUN();
        FERROR("formAddComponent", "Wrong argument type for argument %d. Expected an object.", 1);
    }
    
    id = is_known_class(ARG(1).u.object);

    if (!id) {
        OUTFUN();
        FERROR("formAddComponent", "Wrong object type for argument %d. Expected a Newt class.", 1);
    }
    
    if (!THIS_OBJ(ARG(1).u.object)->u.component) {
        OUTFUN();
        FERROR("formAddComponent", "Cannot use a destroyed object for argument %d.", 1);
    }
    
    newtFormAddComponent(THIS_OBJ(caller)->u.component, THIS_OBJ(ARG(1).u.object)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_formSetHeight(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              height;

    INFUN();
    
    func_prolog("formSetHeight", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("formSetHeight", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("formSetHeight", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    height = ARG(1).u.integer;

    newtFormSetHeight(THIS_OBJ(caller)->u.component, height);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_formSetWidth(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              width;

    INFUN();
    
    func_prolog("formSetWidth", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("formSetWidth", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("formSetWidth", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    width = ARG(1).u.integer;

    newtFormSetWidth(THIS_OBJ(caller)->u.component, width);

    pop_n_elems(args);

    OUTFUN();
}

/* THIS IS OBSOLETE */
static void
f_runForm(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    struct object   *obj;
    newtComponent    comp;

    INFUN();
    
    func_prolog("runForm", ids, caller, NULL, 0);

    comp = newtRunForm(THIS_OBJ(caller)->u.component);

    obj = dict_lookup(comp);

    pop_n_elems(args);

    push_object(obj);

    OUTFUN();
}

static void
f_formRun(INT32 args)
{
    static unsigned         ids[] = {CLASS_FORM, 0};
    struct object          *caller = Pike_fp->next->current_object;
    struct object          *obj;
    struct newtExitStruct   es;

    INFUN();
    
    func_prolog("formRun", ids, caller, NULL, 0);

    newtFormRun(THIS_OBJ(caller)->u.component, &es);

    pop_n_elems(args);
    
    switch(es.reason) {
        case NEWT_EXIT_HOTKEY:
            push_int(es.u.key);
            break;

        case NEWT_EXIT_COMPONENT:
            obj = dict_lookup(es.u.co);
            push_object(obj);
            break;

        case NEWT_EXIT_FDREADY:
            push_string(make_shared_string("EXIT_FDREADY"));
            break;

#ifdef HAVE_NEWTFORMSETTIMER
        case NEWT_EXIT_TIMER:
            push_string(make_shared_string("EXIT_TIMER"));
            break;
#endif

        default:
            push_int(0);
            break;
    }

    OUTFUN();
}

static void
f_drawForm(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("drawForm", ids, caller, NULL, 0);

    newtDrawForm(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_formAddHotKey(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              key;

    INFUN();
    
    func_prolog("formAddHotKey", ids, caller, NULL, 0);

    if (args != 1) {
        OUTFUN();
        FERROR("formAddHotKey", "Wrong number of arguments. Expected %d got %d.", 1, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("formAddHotKey", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    
    key = ARG(1).u.integer;

    newtFormAddHotKey(THIS_OBJ(caller)->u.component, key);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_entry(INT32 args)
{
    static unsigned  ids[] = {CLASS_ENTRY, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, width, flags = 0;
    char            *initVal = NULL;

    INFUN();
    
    func_prolog("entry", ids, caller, NULL, 1);

    if (args < 3 || args > 5) {
        OUTFUN();
        FERROR("entry", "Wrong number of arguments. Expected %d-%d got %d.", 3, 5, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("entry", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("entry", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("entry", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    width = ARG(3).u.integer;

    switch(args) {
        case 5:
            if (ARG(5).type != T_INT) {
                OUTFUN();
                FERROR("entry", "Wrong argument type for argument %d. Expected an integer.", 5);
            }
            flags = ARG(5).u.integer;
            /* Fall through */
            
        case 4:
            if (ARG(4).type != T_STRING || ARG(4).u.string->size_shift > 0)
            {
                OUTFUN();
                FERROR("entry", "Wrong argument type for argument %d. Expected an 8-bit string.", 4);
            }
            initVal = ARG(4).u.string->str;
    }

    /*
     * Newt will create the result buffer in the pdata member of the
     * NEWT_DATA structure.
     */
    THIS_OBJ(caller)->u.component = newtEntry(left, top, initVal, width,
                                              (char**)&THIS_OBJ(caller)->pdata, flags);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_entrySet(INT32 args)
{
    static unsigned  ids[] = {CLASS_ENTRY, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *value;
    int              cursorAtEnd = 1;

    INFUN();
    
    func_prolog("entrySet", ids, caller, NULL, 0);

    if (args < 1 || args > 2) {
        OUTFUN();
        FERROR("entrySet", "Wrong number of arguments. Expected %d-%d got %d.", 1, 2, args);
    }
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0) {
        OUTFUN();
        FERROR("entrySet", "Wrong argument type for argument %d. Expected an 8-bit string.", 1);
    }
    value = ARG(1).u.string->str;

    if (args > 1) {
        if (ARG(2).type != T_INT) {
            OUTFUN();
            FERROR("entrySet", "Wrong argument type for argument %d. Expected an integer.", 2);
        }
        cursorAtEnd = ARG(2).u.integer;
    }

    newtEntrySet(THIS_OBJ(caller)->u.component, value, cursorAtEnd);

    pop_n_elems(args);

    OUTFUN();
}

static int
c_entryFilterWrapper(newtComponent entry, void *data, int ch, int cursor)
{
    struct object   *caller = dict_lookup(entry);
    struct svalue   *filter;
    char             buf[2] = {0, 0};
    int              ret;

    INFUN();
    
    if (!caller) {
        OUTFUN();
        return ch;
    }
    
    filter = (struct svalue*)THIS_OBJ(caller)->pdata;
    if (!filter) {
        OUTFUN();
        return ch;
    }

    if (data)
        push_string(data);
    else
        push_int(0);

    /*
     * If the keypress is a functional or navigational key, then we pass an
     * integer - the filter must know that the value is > 256 and is NOT a
     * single-byte character.
     */
    if (ch >= NEWT_KEY_EXTRA_BASE)
        push_int(ch);
    else {
        buf[0] = (char)ch;
        push_string(make_shared_string(buf));
    }
    
    push_int(cursor);
    apply_svalue(filter, 3);

    switch(Pike_sp[-1].type) {
        case T_INT:
            ret = Pike_sp[-1].u.integer;
            break;

        case T_STRING:
            ret = Pike_sp[-1].u.string->str[0];
            break;

        default:
            ret = ch;
            break;
    }
    
    pop_stack();

    OUTFUN();
    
    return ret;
}

static void
f_entrySetFilter(INT32 args)
{
    static unsigned     ids[] = {CLASS_ENTRY, 0};
    struct object      *caller = Pike_fp->next->current_object;
    struct svalue      *filter = NULL;
    struct pike_string *data = NULL;
    
    INFUN();

    func_prolog("entrySetFilter", ids, caller, NULL, 0);

    check_all_args("entrySetFilter", args,
                   BIT_FUNCTION, /* filter */
                   BIT_STRING|BIT_VOID /* data */);

    if (args > 1)
        data = ARG(2).u.string;

    filter = malloc(sizeof(struct svalue)); /* FIXME: this has to be freed */
    if (!filter) 
        FERROR("entrySetFilter", "Out of memory allocating %u bytes", sizeof(struct svalue));
    
    assign_svalue(filter, &ARG(1));
    THIS_OBJ(caller)->pdata = filter;

    newtEntrySetFilter(THIS_OBJ(caller)->u.component, c_entryFilterWrapper,
                       (void*)data);

    pop_n_elems(args);
    
    OUTFUN();
}

static void
f_entryGetValue(INT32 args)
{
    static unsigned  ids[] = {CLASS_ENTRY, 0};
    struct object   *caller = Pike_fp->next->current_object;
    char            *value;

    INFUN();
    
    func_prolog("entryGetValue", ids, caller, NULL, 0);

    value = newtEntryGetValue(THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    push_string(make_shared_string(value));

    OUTFUN();
}

static void
f_entrySetFlags(INT32 args)
{
    static unsigned  ids[] = {CLASS_ENTRY, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              flags, sense;
    
    INFUN();

    func_prolog("entrySetFlags", ids, caller, NULL, 0);

    check_all_args("entrySetFlags", args,
                   BIT_INT, /* flags */
                   BIT_INT /* sense */);

    flags = ARG(1).u.integer;
    sense = ARG(2).u.integer;

    newtEntrySetFlags(THIS_OBJ(caller)->u.component, flags, sense);

    pop_n_elems(args);
    
    OUTFUN();
}

static void
f_scale(INT32 args)
{
    static unsigned  ids[] = {CLASS_SCALE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top, width, fvLow, fvHigh = 0;
    long long        fullValue;

    INFUN();
    
    func_prolog("scale", ids, caller, NULL, 1);

    if (args < 4 || args > 5) {
        OUTFUN();
        FERROR("scale", "Wrong number of arguments. Expected %d-%d got %d.", 4, 5, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("scale", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    left = ARG(1).u.integer;

    if (ARG(2).type != T_INT) {
        OUTFUN();
        FERROR("scale", "Wrong argument type for argument %d. Expected an integer.", 2);
    }
    top = ARG(2).u.integer;

    if (ARG(3).type != T_INT) {
        OUTFUN();
        FERROR("scale", "Wrong argument type for argument %d. Expected an integer.", 3);
    }
    width = ARG(3).u.integer;

    if (ARG(4).type != T_INT) {
        OUTFUN();
        FERROR("scale", "Wrong argument type for argument %d. Expected an integer.", 4);
    }
    fvLow = ARG(4).u.integer;

    if (args > 4) {
        if (ARG(5).type != T_INT) {
            OUTFUN();
            FERROR("scale", "Wrong argument type for argument %d. Expected an integer.", 5);
        }
        fvHigh = ARG(5).u.integer;
    }

    fullValue = (fvHigh << 31) | fvLow;

    THIS_OBJ(caller)->u.component = newtScale(left, top, width, fullValue);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_scaleSet(INT32 args)
{
    static unsigned  ids[] = {CLASS_SCALE, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              amLow, amHigh = 0;
    long long        amount;

    INFUN();
    
    func_prolog("scaleSet", ids, caller, NULL, 0);

    if (args < 1 || args > 2) {
        OUTFUN();
        FERROR("scaleSet", "Wrong number of arguments. Expected %d-%d got %d.", 1, 2, args);
    }
    
    if (ARG(1).type != T_INT) {
        OUTFUN();
        FERROR("scaleSet", "Wrong argument type for argument %d. Expected an integer.", 1);
    }
    amLow = ARG(1).u.integer;

    if (args > 1) {
        if (ARG(2).type != T_INT) {
            OUTFUN();
            FERROR("scaleSet", "Wrong argument type for argument %d. Expected an integer.", 2);
        }
        amHigh = ARG(2).u.integer;
    }

    amount = (amHigh << 31) | amLow;

    newtScaleSet(THIS_OBJ(caller)->u.component, amount);

    pop_n_elems(args);

    OUTFUN();
}

/* IMPLEMENT */
static void
f_componentAddCallback(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_componentTakesFocus(INT32 args)
{
    struct object   *caller = Pike_fp->next->current_object;
    int              val = 1;

    INFUN();
    
    func_prolog("componentTakesFocus", NULL, caller, NULL, 0);

    if (args > 1) {
        OUTFUN();
        FERROR("componentTakesFocus", "Wrong number of arguments. Expected %d-%d got %d.", 0, 1, args);
    }
    
    if (args != 0) {
        if (ARG(1).type != T_INT) {
            OUTFUN();
            FERROR("componentTakesFocus", "Wrong argument type for argument %d. Expected an integer.", 1);
        }
        val = ARG(1).u.integer ? 1 : 0;
    }

    newtComponentTakesFocus(THIS_OBJ(caller)->u.component, val);

    pop_n_elems(args);

    OUTFUN();
}

static void
dict_foreach_cb(struct object *obj)
{
    INFUN();
    
    THIS_OBJ(obj)->u.component = NULL;
    THIS_OBJ(obj)->destroyed = 0;

    OUTFUN();
}

static void
f_formDestroy(INT32 args)
{
    static unsigned  ids[] = {CLASS_FORM, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("formDestroy", ids, caller, NULL, 0);

    newtFormDestroy(THIS_OBJ(caller)->u.component);

    dict_foreach(dict_foreach_cb);

    pop_n_elems(args);

    OUTFUN();
}

static void
f_createGrid(INT32 args)
{
    static unsigned  ids[] = {CLASS_GRID, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              cols, rows;
    
    INFUN();

    func_prolog("createGrid", ids, caller, NULL, 1);

    check_all_args("createGrid", args, BIT_INT, BIT_INT);

    cols = ARG(1).u.integer;
    rows = ARG(2).u.integer;

    THIS_OBJ(caller)->u.grid = newtCreateGrid(rows, cols);
    THIS_OBJ(caller)->created = 1;
    THIS_OBJ(caller)->destroyed = 0;

    dict_insert(caller, THIS_OBJ(caller)->u.component);

    pop_n_elems(args);
    
    OUTFUN();
}

static void
f_gridSetField(INT32 args)
{
    static unsigned  ids[] = {CLASS_GRID, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              col, row, type = NEWT_GRID_EMPTY;
    int              padLeft, padTop, padRight;
    int              padBottom, anchor, flags;
    struct object   *child;
    
    INFUN();

    func_prolog("gridSetField", ids, caller, NULL, 0);

    check_all_args("gridSetField", args,
                   BIT_INT, /* col */
                   BIT_INT, /* row */
                   BIT_INT, /* padLeft */
                   BIT_INT, /* padTop */
                   BIT_INT, /* padRight */
                   BIT_INT, /* padBottom */
                   BIT_INT, /* anchor */
                   BIT_INT, /* flags */
                   BIT_OBJECT|BIT_VOID /* val */);

    if (args == 9) {
        child = ARG(9).u.object;
        switch(THIS_OBJ(child)->id) {
            case CLASS_GRID:
                type = NEWT_GRID_SUBGRID;
                break;
                
            case CLASS_BUTTON:
            case CLASS_CHECKBOX:
            case CLASS_RADIOBUTTON:
            case CLASS_LISTBOX:
            case CLASS_TEXTBOX:
            case CLASS_TEXTBOXREFLOWED:
            case CLASS_LABEL:
            case CLASS_SCALE:
            case CLASS_ENTRY:
            case CLASS_SCREEN:
            case CLASS_RADIOGROUP:
            case CLASS_RADIOBAR:
            case CLASS_BUTTONBAR:
            case CLASS_CHECKBOXTREE:
            case CLASS_VSCROLLBAR:
                type = NEWT_GRID_COMPONENT;
                break;

            default:
                FERROR("gridSetField", "Trying to ad an object of invalid type to the grid");
                break;
        }
    } else {
        child = NULL;
    }

    col = ARG(1).u.integer;
    row = ARG(2).u.integer;
    padLeft = ARG(3).u.integer;
    padTop = ARG(4).u.integer;
    padRight = ARG(5).u.integer;
    padBottom = ARG(6).u.integer;
    anchor = ARG(7).u.integer;
    flags = ARG(8).u.integer;

    newtGridSetField(THIS_OBJ(caller)->u.grid, col, row,
                     type, child, padLeft,
                     padTop, padRight, padBottom,
                     anchor, flags);
    
    pop_n_elems(args);
    
    OUTFUN();
}

static void
f_gridPlace(INT32 args)
{
    static unsigned  ids[] = {CLASS_GRID, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              left, top;
    
    INFUN();

    func_prolog("gridPlace", ids, caller, NULL, 0);

    check_all_args("gridPlace", args,
                   BIT_INT, /* left */
                   BIT_INT /* top */);

    left = ARG(1).u.integer;
    top = ARG(2).u.integer;

    newtGridPlace(THIS_OBJ(caller)->u.grid, left, top);
    
    pop_n_elems(args);
    
    OUTFUN();
}

static void
f_gridDestroy(INT32 args)
{
    static unsigned  ids[] = {CLASS_GRID, 0};
    struct object   *caller = Pike_fp->next->current_object;
    int              recurse = 1;
    
    INFUN();

    func_prolog("gridDestroy", ids, caller, NULL, 0);

    check_all_args("gridDestroy", args,
                   BIT_INT | BIT_VOID /* left */);

    if (args == 1)
        recurse = ARG(1).u.integer;

    newtGridDestroy(THIS_OBJ(caller)->u.grid, recurse);

    dict_foreach(dict_foreach_cb);
    
    pop_n_elems(args);
    
    OUTFUN();
}

static void
f_gridGetSize(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_gridWrappedWindow(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_gridWrappedWindowAt(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_gridAddComponentsToForm(INT32 args)
{
    INFUN();

    OUTFUN();
}

static void
f_init(INT32 args)
{
    static unsigned  ids[] = {CLASS_SCREEN, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("init", ids, caller, NULL, 1);

    newtInit();
    
    pop_n_elems(args);

    OUTFUN();
}

static void
f_finished(INT32 args)
{
    static unsigned  ids[] = {CLASS_SCREEN, 0};
    struct object   *caller = Pike_fp->next->current_object;

    INFUN();
    
    func_prolog("finished", ids, caller, NULL, 1);

    newtFinished();

    pop_n_elems(args);

    OUTFUN();
}

void init_functions()
{
    INFUN();
    
    dict = dict_create("init_functions", dictname);
    
//    start_new_program();
    
    /*
     * Color constants
     */
    add_integer_constant("COLORSET_ROOT", NEWT_COLORSET_ROOT, 0);
    add_integer_constant("COLORSET_BORDER", NEWT_COLORSET_BORDER, 0);
    add_integer_constant("COLORSET_WINDOW", NEWT_COLORSET_WINDOW, 0);
    add_integer_constant("COLORSET_SHADOW", NEWT_COLORSET_SHADOW, 0);
    add_integer_constant("COLORSET_TITLE", NEWT_COLORSET_TITLE, 0);
    add_integer_constant("COLORSET_BUTTON", NEWT_COLORSET_BUTTON, 0);
    add_integer_constant("COLORSET_ACTBUTTON", NEWT_COLORSET_ACTBUTTON, 0);
    add_integer_constant("COLORSET_CHECKBOX", NEWT_COLORSET_CHECKBOX, 0);
    add_integer_constant("COLORSET_ACTCHECKBOX", NEWT_COLORSET_ACTCHECKBOX, 0);
    add_integer_constant("COLORSET_ENTRY", NEWT_COLORSET_ENTRY, 0);
    add_integer_constant("COLORSET_LABEL", NEWT_COLORSET_LABEL, 0);
    add_integer_constant("COLORSET_LISTBOX", NEWT_COLORSET_LISTBOX, 0);
    add_integer_constant("COLORSET_ACTLISTBOX", NEWT_COLORSET_ACTLISTBOX, 0);
    add_integer_constant("COLORSET_TEXTBOX", NEWT_COLORSET_TEXTBOX, 0);
    add_integer_constant("COLORSET_ACTTEXTBOX", NEWT_COLORSET_ACTTEXTBOX, 0);
    add_integer_constant("COLORSET_HELPLINE", NEWT_COLORSET_HELPLINE, 0);
    add_integer_constant("COLORSET_ROOTTEXT", NEWT_COLORSET_ROOTTEXT, 0);
    add_integer_constant("COLORSET_EMPTYSCALE", NEWT_COLORSET_EMPTYSCALE, 0);
    add_integer_constant("COLORSET_FULLSCALE", NEWT_COLORSET_FULLSCALE, 0);
    add_integer_constant("COLORSET_DISENTRY", NEWT_COLORSET_DISENTRY, 0);
    add_integer_constant("COLORSET_COMPACTBUTTON", NEWT_COLORSET_COMPACTBUTTON, 0);
    add_integer_constant("COLORSET_ACTSELLISTBOX", NEWT_COLORSET_ACTSELLISTBOX, 0);
    add_integer_constant("COLORSET_SELLISTBOX", NEWT_COLORSET_SELLISTBOX, 0);

    /*
     * Flag constants
     */
    add_integer_constant("FLAG_RETURNEXIT", NEWT_FLAG_RETURNEXIT, 0);
    add_integer_constant("FLAG_HIDDEN", NEWT_FLAG_HIDDEN, 0);
    add_integer_constant("FLAG_SCROLL", NEWT_FLAG_SCROLL, 0);
    add_integer_constant("FLAG_DISABLED", NEWT_FLAG_DISABLED, 0);
    add_integer_constant("FLAG_BORDER", NEWT_FLAG_BORDER, 0);
    add_integer_constant("FLAG_WRAP", NEWT_FLAG_WRAP, 0);
    add_integer_constant("FLAG_NOF12", NEWT_FLAG_NOF12, 0);
    add_integer_constant("FLAG_MULTIPLE", NEWT_FLAG_MULTIPLE, 0);
    add_integer_constant("FLAG_SELECTED", NEWT_FLAG_SELECTED, 0);
    add_integer_constant("FLAG_CHECKBOX", NEWT_FLAG_CHECKBOX, 0);
#ifdef NEWT_FLAG_PASSWORD    
    add_integer_constant("FLAG_PASSWORD", NEWT_FLAG_PASSWORD, 0);
#endif

    /*
     * Listbox item sense flags
     */
    add_integer_constant("FLAGS_SET", NEWT_FLAGS_SET, 0);
    add_integer_constant("FLAGS_RESET", NEWT_FLAGS_RESET, 0);
    add_integer_constant("FLAGS_TOGGLE", NEWT_FLAGS_TOGGLE, 0);

    /*
     * Key codes
     */
    add_integer_constant("KEY_TAB", NEWT_KEY_TAB, 0);
    add_integer_constant("KEY_ENTER", NEWT_KEY_ENTER, 0);
    add_integer_constant("KEY_SUSPEND", NEWT_KEY_SUSPEND, 0);
    add_integer_constant("KEY_RETURN", NEWT_KEY_RETURN, 0);
    add_integer_constant("KEY_UP", NEWT_KEY_UP, 0);
    add_integer_constant("KEY_DOWN", NEWT_KEY_DOWN, 0);
    add_integer_constant("KEY_LEFT", NEWT_KEY_LEFT, 0);
    add_integer_constant("KEY_RIGHT", NEWT_KEY_RIGHT, 0);
    add_integer_constant("KEY_BKSPC", NEWT_KEY_BKSPC, 0);
    add_integer_constant("KEY_DELETE", NEWT_KEY_DELETE, 0);
    add_integer_constant("KEY_HOME", NEWT_KEY_HOME, 0);
    add_integer_constant("KEY_END", NEWT_KEY_END, 0);
    add_integer_constant("KEY_UNTAB", NEWT_KEY_UNTAB, 0);
    add_integer_constant("KEY_PGUP", NEWT_KEY_PGUP, 0);
    add_integer_constant("KEY_PGDN", NEWT_KEY_PGDN, 0);
    add_integer_constant("KEY_INSERT", NEWT_KEY_INSERT, 0);
    add_integer_constant("KEY_F1", NEWT_KEY_F1, 0);
    add_integer_constant("KEY_F2", NEWT_KEY_F2, 0);
    add_integer_constant("KEY_F3", NEWT_KEY_F3, 0);
    add_integer_constant("KEY_F4", NEWT_KEY_F4, 0);
    add_integer_constant("KEY_F5", NEWT_KEY_F5, 0);
    add_integer_constant("KEY_F6", NEWT_KEY_F6, 0);
    add_integer_constant("KEY_F7", NEWT_KEY_F7, 0);
    add_integer_constant("KEY_F8", NEWT_KEY_F8, 0);
    add_integer_constant("KEY_F9", NEWT_KEY_F9, 0);
    add_integer_constant("KEY_F10", NEWT_KEY_F10, 0);
    add_integer_constant("KEY_F11", NEWT_KEY_F11, 0);
    add_integer_constant("KEY_F12", NEWT_KEY_F12, 0);
    add_integer_constant("KEY_RESIZE", NEWT_KEY_RESIZE, 0);

    add_integer_constant("GRID_EMPTY", NEWT_GRID_EMPTY, 0);
    add_integer_constant("GRID_COMPONENT", NEWT_GRID_COMPONENT, 0);
    add_integer_constant("GRID_SUBGRID", NEWT_GRID_SUBGRID, 0);
    
#ifdef HAVE_NEWTSETTHREED    
    ADD_FUNCTION("setThreeD", f_setThreeD,
                 tFunc(tInt, tVoid), 0);
#endif
    ADD_FUNCTION("cls", f_cls,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("resizeScreen", f_resizeScreen,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("waitForKey", f_waitForKey,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("clearKeyBuffer", f_clearKeyBuffer,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("delay", f_delay,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("openWindow", f_openWindow,
                 tFunc(tInt tInt tInt tInt tString, tVoid), 0);
    ADD_FUNCTION("centeredWindow", f_centeredWindow,
                 tFunc(tInt tInt tString, tVoid), 0);
    ADD_FUNCTION("popWindow", f_popWindow,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("defaultColors", f_defaultColors,
                 tFunc(tVoid, tMap(tString, tMap(tString, tString))), 0);
    ADD_FUNCTION("setColors", f_setColors,
                 tFunc(tMap(tString, tMap(tString, tString)), tVoid), 0);
    ADD_FUNCTION("refresh", f_refresh,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("suspend", f_suspend,
                 tFunc(tVoid, tVoid), 0);
    /* ADD_FUNCTION("setSuspendCallback", f_setSuspendCallback, */
    ADD_FUNCTION("resume", f_resume,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("pushHelpLine", f_pushHelpLine,
                 tFunc(tString, tVoid), 0);
    ADD_FUNCTION("redrawHelpLine", f_redrawHelpLine,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("popHelpLine", f_popHelpLine,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("drawRootText", f_drawRootText,
                 tFunc(tInt tInt tString, tVoid), 0);
    ADD_FUNCTION("bell", f_bell,
                 tFunc(tVoid, tVoid), 0);

#ifdef HAVE_NEWTCURSOROFF
    ADD_FUNCTION("cursorOff", f_cursorOff,
                 tFunc(tVoid, tVoid), 0);
#endif

#ifdef HAVE_NEWTCURSORON
    ADD_FUNCTION("cursorOn", f_cursorOn,
                 tFunc(tVoid, tVoid), 0);
#endif

    ADD_FUNCTION("compactButton", f_compactButton,
                 tFunc(tInt tInt tString, tVoid), 0);
    ADD_FUNCTION("button", f_button,
                 tFunc(tInt tInt tString, tVoid), 0);
    
    ADD_FUNCTION("checkbox", f_checkbox,
                 tFuncV(tInt tInt tString, tOr(tString, tVoid) tOr(tString, tVoid), tVoid), 0);
    ADD_FUNCTION("checkboxGetValue", f_checkboxGetValue,
                 tFunc(tVoid, tString), 0);
    ADD_FUNCTION("checkboxSetValue", f_checkboxSetValue,
                 tFunc(tString, tVoid), 0);

    ADD_FUNCTION("radiobutton", f_radiobutton,
                 tFuncV(tInt tInt tString, tOr(tInt, tVoid) tOr(tObj, tVoid), tVoid), 0);
    ADD_FUNCTION("radioGetCurrent", f_radioGetCurrent,
                 tFunc(tObj, tObj), 0);
    
    ADD_FUNCTION("getScreenSize", f_getScreenSize,
                 tFunc(tVoid, tMap(tString, tInt)), 0);
    
    ADD_FUNCTION("label", f_label,
                 tFunc(tInt tInt tString, tVoid), 0);
    ADD_FUNCTION("labelSetText", f_labelSetText,
                 tFunc(tString, tVoid), 0);

    ADD_FUNCTION("verticalScrollBar", f_verticalScrollbar,
                 tFunc(tInt tInt tInt tInt tInt, tVoid), 0);
    ADD_FUNCTION("scrollbarSet", f_scrollbarSet,
                 tFunc(tInt tInt, tVoid), 0);

    ADD_FUNCTION("listbox", f_listbox,
                 tFuncV(tInt tInt tInt, tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("listboxGetCurrent", f_listboxGetCurrent,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("listboxSetCurrent", f_listboxSetCurrent,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("listboxSetCurrentByKey", f_listboxSetCurrentByKey,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("listboxSetEntry", f_listboxSetEntry,
                 tFunc(tInt tString, tVoid), 0);
    ADD_FUNCTION("listboxSetWidth", f_listboxSetWidth,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("listboxSetData", f_listboxSetData,
                 tFunc(tInt tInt, tVoid), 0);
    ADD_FUNCTION("listboxAppendEntry", f_listboxAppendEntry,
                 tFunc(tString tInt, tInt), 0);
    ADD_FUNCTION("listboxInsertEntry", f_listboxInsertEntry,
                 tFunc(tString tInt tInt, tInt), 0);
    ADD_FUNCTION("listboxDeleteEntry", f_listboxDeleteEntry,
                 tFunc(tInt, tInt), 0);
    ADD_FUNCTION("listboxClear", f_listboxClear,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("listboxGetEntry", f_listboxGetEntry,
                 tFunc(tInt, tMap(tString, tOr(tInt, tString))), 0);
    ADD_FUNCTION("listboxGetSelection", f_listboxGetSelection,
                 tFunc(tVoid, tOr(tArr(tInt), tInt)), 0);
    ADD_FUNCTION("listboxClearSelection", f_listboxClearSelection,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("listboxSelectItem", f_listboxSelectItem,
                 tFunc(tInt tInt, tVoid), 0);

    ADD_FUNCTION("checkboxTree", f_checkboxTree,
                 tFunc(tInt tInt tInt tInt, tVoid), 0);
    ADD_FUNCTION("checkboxTreeMulti", f_checkboxTreeMulti,
                 tFunc(tInt tInt tInt tString tInt, tVoid), 0);
    ADD_FUNCTION("checkboxTreeGetSelecion", f_checkboxTreeGetSelection,
                 tFunc(tVoid, tOr(tArr(tInt), tInt)), 0);
    ADD_FUNCTION("checkboxTreeGetCurrent", f_checkboxTreeGetCurrent,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("checkboxTreeGeMultitSelecion", f_checkboxTreeGetMultiSelection,
                 tFunc(tString, tOr(tArr(tInt), tInt)), 0);

    /*
     * Those two functions are actually the same. They differ in that the
     * first one uses varargs in C, while the other requires an array of
     * integers. Since there's no easy and portable way to construct a
     * varargs list from the Pike params, we map both of those functions to
     * one.
     */
    ADD_FUNCTION("checkboxTreeAddItem", f_checkboxTreeAddArray,
                 tFunc(tString tInt tInt tArr(tInt), tInt), 0);
    ADD_FUNCTION("checkboxTreeAddArray", f_checkboxTreeAddArray,
                 tFunc(tString tInt tInt tArr(tInt), tInt), 0);
    ADD_FUNCTION("checkboxTreeFindItem", f_checkboxTreeFindItem,
                 tFunc(tInt, tOr(tArr(tInt), tInt)), 0);

#ifdef HAVE_NEWTCHECKBOXTREESETENTRY
    ADD_FUNCTION("checkboxTreeSetEntry", f_checkboxTreeSetEntry,
                 tFunc(tInt tString, tVoid), 0);
#endif

#ifdef HAVE_NEWTCHECKBOXTREEGETENTRYVALUE
    ADD_FUNCTION("checkboxTreeGetEntryValue", f_checkboxTreeGetEntryValue,
                 tFunc(tInt, tString), 0);
#endif

#ifdef HAVE_NEWTCHECKBOXTREESETENTRYVALUE
    ADD_FUNCTION("checkboxTreeSetEntryValue", f_checkboxTreeSetEntryValue,
                 tFunc(tInt tString, tVoid), 0);
#endif

    ADD_FUNCTION("textboxReflowed", f_textboxReflowed,
                 tFuncV(tInt tInt tString tInt, tOr(tInt, tVoid) tOr(tInt, tVoid) tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("textbox", f_textbox,
                 tFuncV(tInt tInt tInt tInt, tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("textboxSetText", f_textboxSetText,
                 tFunc(tString, tVoid), 0);
    ADD_FUNCTION("textboxSetHeight", f_textboxSetHeight,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("textboxGetNumLines", f_textboxGetNumLines,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("reflowText", f_reflowText,
                 tFuncV(tString tInt, tOr(tInt, tVoid) tOr(tInt, tVoid),
                        tOr(tMap(tString, tOr(tString, tInt)), tInt)), 0);

    ADD_FUNCTION("form", f_form,
                 tFunc(tOr(tObj, tInt) tString tInt, tVoid), 0);
#ifdef HAVE_NEWTFORMSETTIMER
    ADD_FUNCTION("formSetTimer", f_formSetTimer,
                 tFunc(tInt, tVoid), 0);
#endif
    /* formWatchFd here */
    ADD_FUNCTION("formSetSize", f_formSetSize,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("formGetCurrent", f_formGetCurrent,
                 tFunc(tVoid, tObj), 0);
    ADD_FUNCTION("formSetBackground", f_formSetBackground,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("formSetCurrent", f_formSetCurrent,
                 tFunc(tObj, tVoid), 0);
    ADD_FUNCTION("formAddComponent", f_formAddComponent,
                 tFunc(tObj, tVoid), 0);
    ADD_FUNCTION("formSetHeight", f_formSetHeight,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("formSetWidth", f_formSetWidth,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("runForm", f_runForm,
                 tFunc(tVoid, tObj), 0);
    ADD_FUNCTION("formRun", f_formRun,
                 tFunc(tVoid, tOr(tObj, tOr(tInt, tString))), 0);
    ADD_FUNCTION("drawForm", f_drawForm,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("formAddHotKey", f_formAddHotKey,
                 tFunc(tInt, tVoid), 0);

    ADD_FUNCTION("createGrid", f_createGrid,
                 tFunc(tInt tInt, tVoid), 0);
    ADD_FUNCTION("gridSetField", f_gridSetField,
                 tFuncV(tInt tInt tInt tInt tInt tInt tInt tInt,
                        tOr(tObj, tVoid), tVoid), 0);
    ADD_FUNCTION("gridPlace", f_gridPlace,
                 tFunc(tInt tInt, tVoid), 0);
    ADD_FUNCTION("gridDestroy", f_gridDestroy,
                 tFuncV(tVoid, tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("gridFree", f_gridDestroy,
                 tFuncV(tVoid, tOr(tInt, tVoid), tVoid), 0);
    
    ADD_FUNCTION("entry", f_entry,
                 tFunc(tInt tInt tInt tOr(tString, tVoid) tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("entrySet", f_entrySet,
                 tFunc(tString tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("entrySetFilter", f_entrySetFilter,
                 tFuncV(tFunction, tString, tVoid), 0);
    ADD_FUNCTION("entryGetValue", f_entryGetValue,
                 tFunc(tVoid, tString), 0);
    ADD_FUNCTION("entrySetFlags", f_entrySetFlags,
                 tFunc(tInt tInt, tVoid), 0);

    ADD_FUNCTION("scale", f_scale,
                 tFunc(tInt tInt tInt tInt tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("scaleSet", f_scaleSet,
                 tFunc(tInt tOr(tInt, tVoid), tVoid), 0);

    ADD_FUNCTION("componentTakesFocus", f_componentTakesFocus,
                 tFunc(tOr(tInt, tVoid), tVoid), 0);

    ADD_FUNCTION("formDestroy", f_formDestroy,
                 tFunc(tVoid, tVoid), 0);

    ADD_FUNCTION("init", f_init,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("finished", f_finished,
                 tFunc(tVoid, tVoid), 0);
    
//    end_class("Newt", 0);

    OUTFUN();
}
#else
void init_functions()
{}
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * fill-column: 75
 * End:
 */
