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
 *
 * $Id$
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
    if (!THIS_OBJ(obj)->id || THIS_OBJ(obj)->id > CLASS_COUNT)
        return 0;
    else
        return THIS_OBJ(obj)->id;
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
    THIS->component = NULL;
}

static void
exit_base(struct object *o)
{
    /*
     * If the Form isn't destroyed find whether it has a 'destroy'
     * function defined. If found, call it (should always be found :P)
     */
    if (THIS->id == CLASS_FORM && THIS->created && !THIS->destroyed) {
        int       fun_offset;

        fun_offset = find_identifier("destroy", fp->current_object->prog);
        if (fun_offset >= 0) {
            push_int(THIS->id);
            apply_low(fp->current_object, fun_offset, 1);
            pop_stack();
        }
    }
}

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
            newtFormDestroy(THIS->component);
            THIS->created = 0;
            THIS->destroyed = 1;
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
            break;

        default:
            ERROR("create", "Unknown class identifier 0x%02X", ARG(1).u.integer);
            break;
    }

    THIS->destroyed = 0;
    THIS->created = 0;

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

    /*
     * Functions
     */
    ADD_FUNCTION("create", f_create, tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("destroy", f_destroy, tFunc(tInt, tVoid), 0);
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
 
