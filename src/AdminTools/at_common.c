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
 * Simple glue for more advanced Unix functions.
 * 
 * Common stuff for the entire module
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include <string.h>

/*
 * Pike includes
 */
#include "caudium_util.h"

#include "at_common.h"

#define THIS_OBJ ((ATSTORAGE*)Pike_fp->current_storage)

static char *module_name;

/*
 * Displays a function error for the current object->function()
 */
void FERROR(char *fn, char *format, ...)
{
    va_list      args;
    static char  *efmt = "%s.%s->%s(): ";
    char         myformat[1024];
    int          freebuf;
    
    snprintf(myformat, sizeof(myformat), efmt, 
             module_name, 
	     THIS_OBJ->object_name ? THIS_OBJ->object_name : "",
	     fn ? fn : "UnknownFunction");
	     
    va_start(args, format);
    vsnprintf(myformat + strlen(myformat), sizeof(myformat) - strlen(myformat) - 1,
              format, args);
    strcat(myformat, "\n");
    Pike_error(myformat);
    va_end(args);
}

/*
 * Displays a function error for the current object
 */
void OPERROR(char *fn, char *format, ...)
{
    va_list      args;
    static char  *efmt = "%s%s: ";
    char         myformat[1024];
    int          freebuf;
    
    snprintf(myformat, sizeof(myformat), efmt, 
             module_name, 
	     THIS_OBJ->object_name ? THIS_OBJ->object_name : "",
	     fn ? fn : "<UnknownOperator>");
	     
    va_start(args, format);
    vsnprintf(myformat + strlen(myformat), sizeof(myformat) - strlen(myformat) - 1,
              format, args);
    strcat(myformat, "\n");
    Pike_error(myformat);
    va_end(args);
}

void init_common(char *modname)
{
    if (modname)
	module_name = strdup(modname);
    else
	module_name = strdup("UnknownModule");
}
