/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000 The Caudium Group
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
 * Interface to some system calls that are missing from the Pike proper.
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "pexts.h"

#include "at_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "at_common.h"

#define THIS_LOW ((ATSTORAGE*)get_storage(fp->current_object, system_program))
#define THIS ((void*)THIS_LOW->object_data)

static char *_object_name = "System";
static struct program *system_program;

static void
f_ln(INT32 args)
{
    int  ret, symbolic = 0;
    
    if (args < 2)
	FERROR("ln", "not enough arguments. Expected at least 2.");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("ln", "Wrong argument type for argument 1. Expected 8-bit string");
	
    if (ARG(2).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("ln", "Wrong argument type for argument 2. Expected 8-bit string");
	
    if (args > 2) {
        if (ARG(3).type != T_INT)
	    FERROR("ln", "Wrong argument type for argument 3. Expected integer");
	else
	    symbolic = ARG(3).u.integer;
    }

    if (!symbolic)
	ret = link(ARG(1).u.string->str, ARG(2).u.string->str);
    else
	ret = symlink(ARG(1).u.string->str, ARG(2).u.string->str);
	
    pop_n_elems(args);
    push_int(ret);
}

static void
f_unlink(INT32 args)
{
    int  ret;
    
    if (args < 1)
	FERROR("ln", "not enough arguments. Expected exactly 1.");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("ln", "Wrong argument type for argument 1. Expected 8-bit string");
	
    ret = unlink(ARG(1).u.string->str);
	
    pop_n_elems(args);
    push_int(ret);
}

static void
f_create(INT32 args)
{
}

static void
init_system(struct object *o)
{
    THIS_LOW->object_name = _object_name;
    THIS_LOW->object_data = NULL;
}

static void
exit_system(struct object *o)
{}

struct program*
_at_system_init(void)
{
    start_new_program();
    ADD_STORAGE(ATSTORAGE);

    set_init_callback(init_system);
    set_exit_callback(exit_system);

    ADD_FUNCTION("create", f_create, 
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("ln", f_ln, 
                 tFunc(tString tString tOr(tInt, tVoid), tInt), 0);
    ADD_FUNCTION("unlink", f_unlink,
                 tFunc(tString, tInt), 0);
    
    system_program = end_program();
    add_program_constant("System", system_program, 0);
    
    return system_program;
}