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
 * Newt Support - This module adds Newt support to Pike. 
 */

/*
 * Implementation of the component object base
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "pexts.h"

#include "newt_config.h"
#include "newt_global.h"

#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)

/* Class names */
static char*  class_names[] = {
    "WrongName",
    "Form",
    "Grid",
    "Button",
    "CheckBox",
    "RadioButton",
    "ListBox",
    "TextBox",
    "TextBoxReflowed",
    "Label",
    "Scale",
    "Entry",
    "Screen",
    "RadioGroup",
    "RadioBar",
    "ButtonBar",
    "CheckBoxTree",
    "VScrollBar"
};

#define CLASS_COUNT (sizeof(class_names) / sizeof(char*))

/*
 * Global functions
 */
unsigned
is_known_class(struct object *obj)
{
    if (!THIS_OBJ(obj)->id || THIS_OBJ(obj)->id > CLASS_COUNT) {
        return 0;
    } else {
        return THIS_OBJ(obj)->id;
    }
}

char *
get_class_name(struct object *obj)
{
    unsigned   id;
    
    if (!(id = is_known_class(obj)))
        return NULL;

    return class_names[id];
}

static void
init_base(struct object *o)
{
    THIS->created = 0;
    THIS->name = NULL;
    THIS->id = 0;
    THIS->destroyed = 0;
    THIS->u.component = NULL;
    THIS->whoami = WHOAMI_DUNNO;
}

static void
exit_base(struct object *o)
{}

/*
 * This is the default destroy function. It destroys the associated form
 * component. Can be overriden in the child classes, but should always be
 * called!
 */
static void
f_destroy(INT32 args)
{
    if (!THIS->created || THIS->destroyed) {
        pop_n_elems(args);
        return;
    }
        
    switch(THIS->id) {
        case CLASS_FORM:
            newtFormDestroy(THIS->u.component);
            THIS->created = 0;
            THIS->destroyed = 1;
            THIS->whoami = WHOAMI_DUNNO;
            break;
    }

    pop_n_elems(args);
}

/*
 * Base create function. Takes the class ID constant as argument. Note that
 * it doesn't create the associated newt component - it just sets what we
 * use internally to manage the class. The component should be created by
 * the class by calling appropriate newt function.
 */
static void
f_create(INT32 args)
{
    THIS->name = "ComponentBase";
    
    if (args != 1)
        ERROR("create", "Expected exactly one argument (integer) got %d instead.", args);

    if (ARG(1).type != T_INT)
        ERROR("create", "Wrong argument type for argument 1 - expected an integer");
    
    switch(ARG(1).u.integer) {
        case CLASS_FORM:
            THIS->name = class_names[CLASS_FORM];
            THIS->id = CLASS_FORM;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_GRID:
            THIS->name = class_names[CLASS_GRID];
            THIS->id = CLASS_GRID;
            THIS->whoami = WHOAMI_GRID;
            break;

        case CLASS_BUTTON:
            THIS->name = class_names[CLASS_BUTTON];
            THIS->id = CLASS_BUTTON;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_CHECKBOX:
            THIS->name = class_names[CLASS_CHECKBOX];
            THIS->id = CLASS_CHECKBOX;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_RADIOBUTTON:
            THIS->name = class_names[CLASS_RADIOBUTTON];
            THIS->id = CLASS_RADIOBUTTON;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_LISTBOX:
            THIS->name = class_names[CLASS_LISTBOX];
            THIS->id = CLASS_LISTBOX;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_TEXTBOX:
            THIS->name = class_names[CLASS_TEXTBOX];
            THIS->id = CLASS_TEXTBOX;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_TEXTBOXREFLOWED:
            THIS->name = class_names[CLASS_TEXTBOXREFLOWED];
            THIS->id = CLASS_TEXTBOXREFLOWED;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_LABEL:
            THIS->name = class_names[CLASS_LABEL];
            THIS->id = CLASS_LABEL;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_SCALE:
            THIS->name = class_names[CLASS_SCALE];
            THIS->id = CLASS_SCALE;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_ENTRY:
            THIS->name = class_names[CLASS_ENTRY];
            THIS->id = CLASS_ENTRY;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_SCREEN:
            THIS->name = class_names[CLASS_SCREEN];
            THIS->id = CLASS_SCREEN;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_RADIOGROUP:
            THIS->name = class_names[CLASS_RADIOGROUP];
            THIS->id = CLASS_RADIOGROUP;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_RADIOBAR:
            THIS->name = class_names[CLASS_RADIOBAR];
            THIS->id = CLASS_RADIOBAR;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_BUTTONBAR:
            THIS->name = class_names[CLASS_BUTTONBAR];
            THIS->id = CLASS_BUTTONBAR;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_CHECKBOXTREE:
            THIS->name = class_names[CLASS_CHECKBOXTREE];
            THIS->id = CLASS_CHECKBOXTREE;
            THIS->whoami = WHOAMI_COMPONENT;
            break;

        case CLASS_VSCROLLBAR:
            THIS->name = class_names[CLASS_VSCROLLBAR];
            THIS->id = CLASS_VSCROLLBAR;
            THIS->whoami = WHOAMI_COMPONENT;
            break;
            
        default:
            ERROR("create", "Unknown class identifier 0x%02X", ARG(1).u.integer);
            break;
    }

    THIS->destroyed = 0;
    THIS->created = 0;
    THIS->u.component = NULL;
    THIS->pdata = NULL;
    
    pop_n_elems(args);
}

/*
 * Returns the name of this class.
 */
static void
f_myname(INT32 args)
{
    struct pike_string   *myName;

    myName = make_shared_string(THIS->name);

    pop_n_elems(args);
    push_string(myName);
}

void init_component_base()
{
    start_new_program();

    ADD_STORAGE(NEWT_DATA);
    
    set_init_callback(init_base);
    set_exit_callback(exit_base);
    
    /*
     * Constants
     */
    add_integer_constant("CLASS_FORM", CLASS_FORM, 0);
    add_integer_constant("CLASS_GRID", CLASS_GRID, 0);
    add_integer_constant("CLASS_BUTTON", CLASS_BUTTON, 0);
    add_integer_constant("CLASS_CHECKBOX", CLASS_CHECKBOX, 0);
    add_integer_constant("CLASS_RADIOBUTTON", CLASS_RADIOBUTTON, 0);
    add_integer_constant("CLASS_LISTBOX", CLASS_LISTBOX, 0);
    add_integer_constant("CLASS_TEXTBOX", CLASS_TEXTBOX, 0);
    add_integer_constant("CLASS_TEXTBOXREFLOWED", CLASS_TEXTBOXREFLOWED, 0);
    add_integer_constant("CLASS_LABEL", CLASS_LABEL, 0);
    add_integer_constant("CLASS_SCALE", CLASS_SCALE, 0);
    add_integer_constant("CLASS_ENTRY", CLASS_ENTRY, 0);
    add_integer_constant("CLASS_SCREEN", CLASS_SCREEN, 0);
    add_integer_constant("CLASS_RADIOGROUP", CLASS_RADIOGROUP, 0);
    add_integer_constant("CLASS_RADIOBAR", CLASS_RADIOBAR, 0);
    add_integer_constant("CLASS_BUTTONBAR", CLASS_BUTTONBAR, 0);
    add_integer_constant("CLASS_CHECKBOXTREE", CLASS_CHECKBOXTREE, 0);
    add_integer_constant("CLASS_VSCROLLBAR", CLASS_VSCROLLBAR, 0);
    
    /*
     * Functions
     */
    ADD_FUNCTION("create", f_create, tFunc(tInt, tVoid), 0);
//    ADD_FUNCTION("destroy", f_destroy, tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("myName", f_myname, tFunc(tVoid, tString), 0);
    
    end_class("ComponentBase", 0);
}
#else
void init_component_base()
{}
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * fill-column: 75
 * End:
 */
 
