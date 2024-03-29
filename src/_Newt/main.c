/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000-2005 The Caudium Group
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
 * Main code for the _Newt module
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#define MODULE_MAJOR 0
#define MODULE_MINOR 0
#define MODULE_BUILD 1

#include "caudium_util.h"


#include "newt_config.h"

#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)
#include "newt_global.h"

#ifdef __DEBUG__
#include <stdio.h>

void LOG_TO_FILE(char *s);

static FILE*  lf = NULL;

void LOG_TO_FILE(char *s)
{
    fprintf(lf, s);
}
#endif

/*
 * Displays a function error for the current object->function()
 */
void ERROR(char *fn, char *format, ...)
{
    va_list      args;
    static char  *efmt = "%s.%s->%s(): ";
    char         myformat[1024];
    int          freebuf;
    
    snprintf(myformat, sizeof(myformat), efmt, 
             "_Newt", 
             THIS->name ? THIS->name : "UnnamedClass",
             fn ? fn : "UnknownFunction");
	     
    va_start(args, format);
    vsnprintf(myformat + strlen(myformat), sizeof(myformat) - strlen(myformat) - 1,
              format, args);
    strcat(myformat, "\n");

#ifdef __DEBUG__
    LOG_TO_FILE(myformat);
#endif     

	Pike_error(myformat);
    va_end(args);
}

void FERROR(char *fn, char *format, ...)
{
    va_list      args;
    static char  *efmt = "%s.%s(): ";
    char         myformat[1024];
    
    snprintf(myformat, sizeof(myformat), efmt, 
             "_Newt",
             fn ? fn : "UnknownFunction");
	     
    va_start(args, format);
    vsnprintf(myformat + strlen(myformat), sizeof(myformat) - strlen(myformat) - 1,
              format, args);
    strcat(myformat, "\n");

#ifdef __DEBUG__
    LOG_TO_FILE(myformat);
#endif

    Pike_error(myformat);
    va_end(args);
}

void pike_module_init(void)
{
#ifdef __DEBUG__
	lf = fopen("Newt.log", "w+");
#endif    
    init_dictionary();

    init_component_base();
    init_functions();
}

void pike_module_exit(void)
{
#ifdef __DEBUG__
    if (lf)
        fclose(lf);
#endif
}
#else
void pike_module_init(void)
{}

void pike_module_exit(void)
{}
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * fill-column: 75
 * End:
 */
 
