/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001 The Caudium Group
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

#include "caudium_util.h"

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
#include <sys/stat.h>

#if defined(HAVE_GETLOADAVG) && defined(HAVE_SYS_LOADAVG_H)
#include <sys/loadavg.h>
#endif

#include "at_common.h"

#define THIS_LOW ((ATSTORAGE*)get_storage(fp->current_object, system_program))
#define THIS ((void*)THIS_LOW->object_data)

static char *_object_name = "System";
static struct program *system_program;
static struct program *mkstemp_program;

typedef struct
{
    struct pike_string    *fname;
    int                    fd;
} MKSTEMP_STORAGE;

#define THIS_MKSTEMP ((MKSTEMP_STORAGE*)get_storage(fp->current_object, mkstemp_program))

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

#ifdef HAVE_MKSTEMP
static void
f_temp_create(INT32 args)
{
    // No need to check the arguments - we create the object here
    THIS_MKSTEMP->fname = ARG(1).u.string;
    add_ref(THIS_MKSTEMP->fname);

    THIS_MKSTEMP->fd = ARG(2).u.integer;

    pop_n_elems(args);
}

static void 
f_temp_name(INT32 args)
{
    pop_n_elems(args);

    push_string(THIS_MKSTEMP->fname);
}

static void
f_temp_fd(INT32 args)
{
    pop_n_elems(args);

    push_int(THIS_MKSTEMP->fd);
}

static void
f_temp_truncate(INT32 args)
{
    int   ret;
    
    pop_n_elems(args);
    ret = ftruncate(THIS_MKSTEMP->fd, 0);

    push_int(ret);
}

static void
f_mkstemp(INT32 args)
{
    char            *buf;
    int              ret;
    struct object   *obj;
    
    if (args != 1)
	FERROR("mkstemp", "not enough arguments. Expected exactly 1.");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("mkstemp", "Wrong argument type for argument 1. Expected 8-bit string");
	
    buf = strdup(ARG(1).u.string->str);
    if (!buf)
	FERROR("mkstemp", "Out of memory (allocating %d bytes)", 
	       ARG(1).u.string->len);
    
    if ((ret = mkstemp(buf)) < 0)
	FERROR("mkstemp", "Error creating a temporary file");
	
    /*
     * Some libc libraries set the permissions to 0666,
     * let's change it here
     */
    fchmod(ret, 0600);
    
    pop_n_elems(args);

    push_string(make_shared_string(buf));
    push_int(ret);
    obj = clone_object(mkstemp_program, 2);

    push_object(obj);
    free(buf);
}
#endif

#ifdef HAVE_MKDTEMP
static void
f_mkdtemp(INT32 args)
{
    char  *dir, *ret;
    
    if (args < 1 || args > 2)
	FERROR("mkdtemp", "not enough arguments. Expected 1 or 21.");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("mkdtemp", "Wrong argument type for argument 1. Expected 8-bit string");

    dir = strdup(ARG(1).u.string->str);
    if (!dir)
	FERROR("mkdtemp", "Error allocating memory (requested %d bytes)",
	       ARG(1).u.string->len);
	       
    ret = mkdtemp(dir);
    
    if (!ret)
	FERROR("mkdtemp", "Error creating temporary directory");
	
    pop_n_elems(args);
    
    push_string(make_shared_string(ret));
    free(dir);
}
#endif

#if defined(HAVE_GETLOADAVG)
static void
f_getloadavg(INT32 args)
{

}
#endif

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
		 
#ifdef HAVE_MKSTEMP
    ADD_FUNCTION("mkstemp", f_mkstemp,
	         tFunc(tString, tObj), 0);
#endif

#ifdef HAVE_MKDTEMP
    ADD_FUNCTION("mkdtemp", f_mkdtemp,
	         tFunc(tString tOr(tString, tVoid), tString), 0);
#endif

    system_program = end_program();
    add_program_constant("System", system_program, 0);

    start_new_program();
    ADD_STORAGE(MKSTEMP_STORAGE);
    
    ADD_FUNCTION("create", f_temp_create,
                 tFunc(tString tInt, tVoid), 0);
    ADD_FUNCTION("name", f_temp_name,
                 tFunc(tVoid, tString), 0);
    ADD_FUNCTION("fd", f_temp_fd,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("truncate", f_temp_truncate,
                 tFunc(tVoid, tInt), 0);
#if defined(HAVE_GETLOADAVG)
    ADD_FUNCTION("getloadavg",f_getloadavg,
                 tFunc(tVoid, tString),0);
#endif

    mkstemp_program = end_program();
    
    return system_program;
}
