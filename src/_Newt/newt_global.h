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
#ifndef __newt_global_h
#define __newt_global_h

/*
 * No debug by default
 */
#ifdef __DEBUG__
#undef __DEBUG__
#endif

#if defined(__GNUC__ ) && defined(__DEBUG__)
#include <stdio.h>

#define INFUN()  fprintf(stderr, "%s:%d %s <--\n", __FILE__, __LINE__, __FUNCTION__)
#define OUTFUN() fprintf(stderr, "%s:%d %s -->\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define INFUN()
#define OUTFUN()
#endif

/*
 * Structure to hold all the component data not directly accessible from
 * within Pike and also all stuff we need to know to appropriately manage
 * forms and components in OOP manner.
 */
#define WHOAMI_DUNNO     0x00
#define WHOAMI_COMPONENT 0x01
#define WHOAMI_GRID      0x02

typedef struct
{
    union 
    {
        newtComponent      component;
        newtGrid           grid;
    } u;
    
    char               *name; /* class name, e.g. "Form" */
    unsigned           id;    /* class ID -> see below */
    int                whoami; /* WHOAMI_GRID || WHOAMI_COMPONENT || WHOAMI_DUNNO*/
    int                created;
    int                destroyed;
    unsigned           pflags; /* Private flags for the class */
    void               *pdata; /* Private data for the class */
} NEWT_DATA;

/* Class IDs */
#define CLASS_FORM                0x000001
#define CLASS_GRID                0x000002
#define CLASS_BUTTON              0x000003
#define CLASS_CHECKBOX            0x000004
#define CLASS_RADIOBUTTON         0x000005
#define CLASS_LISTBOX             0x000006
#define CLASS_TEXTBOX             0x000007
#define CLASS_TEXTBOXREFLOWED     0x000008
#define CLASS_LABEL               0x000009
#define CLASS_SCALE               0x00000A
#define CLASS_ENTRY               0x00000B
#define CLASS_SCREEN              0x00000C
#define CLASS_RADIOGROUP          0x00000D
#define CLASS_RADIOBAR            0x00000E
#define CLASS_BUTTONBAR           0x00000F
#define CLASS_CHECKBOXTREE        0x000010
#define CLASS_VSCROLLBAR          0x000011

/* Individual class flags */
#define BUTTON_COMPACT            0x000001

#define THIS_OBJ(_o_) ((NEWT_DATA*)get_storage((_o_), (_o_)->prog))
#define THIS ((NEWT_DATA*)get_storage(Pike_fp->current_object, Pike_fp->current_object->prog))

#define DICT_OK                    0
#define DICT_NULL_DATA            -1
#define DICT_NULL_OBJECT          -2
#define DICT_NULL_DICT            -3
#define DICT_UNKNOWN_TYPE         -4
#define DICT_EXISTS               -5

typedef void (*dict_cb_fn)(struct object *);

typedef struct _DICT
{
    struct mapping       *dict;
    char                 *name;
    INT32                 used;

    /*
     * Functions
     */
    int                  (*insert)(struct _DICT*, struct object *, void *data);
    struct object*       (*lookup)(struct _DICT*, void *data);
    void                 (*foreach)(struct _DICT*, dict_cb_fn);
} DICT;

/* Initialization functions */
void init_functions(void);
void init_component_base(void);
void init_dictionary(void);

/* Dictionary functions */
DICT* dict_create(char *fn, char *name);

/* Common functions */
void ERROR(char *fn, char *format, ...);
void FERROR(char *fn, char *format, ...);

char *get_class_name(struct object *obj);
unsigned is_known_class(struct object *obj);

/* pike module functions */
void pike_module_init(void);
void pike_module_exit(void);

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * fill-column: 75
 * End:
 */
