/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
 */

/* Copyright (C) 2000-2003 The Caudium Group
 * Copyright (C) 2000-2002 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The AdminTools Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The AdminTools Pexts Module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Pike Extensions; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 *
 * $Id$
 */

/*
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
