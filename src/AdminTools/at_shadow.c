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
 * Simple glue for more advanced Unix functions.
 */
#define _GNU_SOURCE
#define _POSIX_PTHREAD_SEMANTICS

#include "global.h"
RCSID("$Id$");

/*
 * Pike includes
 */
#include "stralloc.h"
#include "pike_macros.h"
#include "module_support.h"
#include "program.h"
#include "error.h"
#include "threads.h"
#include "array.h"

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

#define THIS_LOW ((ATSTORAGE*)get_storage(fp->current_object, shadow_program))
#define THIS ((struct shadow_struct*)THIS_LOW->object_data)

struct shadow_struct
{
    int      db_opened;
    long     sp_buf_max;
};

static char *_object_name = "Shadow";

static struct program *shadow_program;



static void
push_spent(struct spwd *spent)
{
    struct array   *arr;
    
    if (!spent) {
        push_int(0);
        return;
    }
    
    /* [0] - login name */
    push_text(spent->sp_namp);
    
    /* [1] - encrypted password */
    push_text(spent->sp_pwdp);
    
    /* [2] - date of last change */
    push_longint(spent->sp_lstchg);
    
    /* [3] - min. nr of days between passwd changes */
    push_longint(spent->sp_min);
    
    /* [4] - max. nr of days between passwd changes */
    push_longint(spent->sp_max);
    
    /* [5] - nr of days to warn the user b4 the passwd expires */
    push_longint(spent->sp_warn);
    
    /* [6] - nr of days the account may be inactive */
    push_longint(spent->sp_inact);
    
    /* [7] - nr of days since the Epoch until the account expires */
    push_longint(spent->sp_expire);

    arr = aggregate_array(8);
    push_array(arr);
}

/*
 * Exported APIs
 */

#ifdef HAVE_SETSPENT 
static void
f_setspent(INT32 args)
{
    pop_n_elems(args);
    if (THIS->db_opened)
        return;

    setspent();
    THIS->db_opened = 1;
}
#endif

#ifdef HAVE_ENDSPENT
static void
f_endspent(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->db_opened) {
        FERROR("endspent", "database not opened");
        return;
    }
    endspent();
    THIS->db_opened = 0;
}
#endif

#if defined(HAVE_GETSPENT) || defined(HAVE_GETSPENT_R)
static void
f_getspent(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->db_opened)
        f_setspent(0);

#if defined(_REENTRANT) && defined(HAVE_GETSPENT_R)
    {
	LOCAL_BUF(buf, THIS->sp_buf_max);
        struct spwd    spbuf, *spent;

        LOCAL_CHECK(buf, "getspent");

	/*
	 * There is _no_ way to tell the difference between an error
	 * and the end of DB using this function...
	 * /grendel 22-09-2000
	 */
#ifdef HAVE_SOLARIS_GETSPENT_R
	if (getspent_r(&spbuf, buf, THIS->sp_buf_max) != 0)
#else
        if (getspent_r(&spbuf, buf, THIS->sp_buf_max, &spent) != 0)
#endif
	{
        push_int(0);
        return;
	}
    push_spent(spent);

	LOCAL_FREE(buf);
    }
#else
    push_spent(getspent());
#endif
}
#endif

#if defined(HAVE_GETSPNAM) || defined(HAVE_GETSPNAM_R)
static void
f_getspnam(INT32 args)
{
    char           *name;
#ifdef _REENTRANT
    LOCAL_BUF(buf, THIS->sp_buf_max);
    struct spwd    spbuf;
    struct spwd    *spent;
#endif

    get_all_args("AdminTools.Shadow->getspnam", args, "%S", &name);
    if (args != 1)
        FERROR("getspnam", "Invalid number of arguments. Expected 1");
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("getspnam", "Wrong argument type for argument 1. Expected 8-bit string");

    name = ARG(1).u.string->str;
    
#if defined(_REENTRANT) && defined(HAVE_GETSPNAM_R)
    LOCAL_CHECK(buf, "getspnam");
    pop_n_elems(args);
    
#ifdef HAVE_SOLARIS_GETSPNAM_R
    if (!(spent = getspnam_r(name, &spbuf, buf, THIS->sp_buf_max))) {
#else
    if (getspnam_r(name, &spbuf, buf, THIS->sp_buf_max, &spent) != 0) {
#endif
        push_int(0);
        return;
    }
    push_spent(spent);
    LOCAL_FREE(buf);
#else
    pop_n_elems(args);
    push_spent(getspnam(name));
#endif
}
#endif

static void
f_getallspents(INT32 args)
{
    struct spwd    *spent;
    INT32          nents;
    struct array   *sp_arr;
#ifdef _REENTRANT
    LOCAL_BUF(buf, THIS->sp_buf_max);
    struct spwd    spbuf;
#endif

    if(THIS->db_opened)
        f_endspent(0);
    f_setspent(0);

    nents = 0;

#if defined(_REENTRANT) && defined(HAVE_GETSPENT_R)
    LOCAL_CHECK(buf, "getallspents");
    pop_n_elems(args);

#ifdef HAVE_SOLARIS_GETSPENT_R
    if (!(spent = getspent_r(&spbuf, buf, THIS->sp_buf_max))) {
#else
    if (getspent_r(&spbuf, buf, THIS->sp_buf_max, &spent) != 0) {
#endif
        push_int(0);
        return;
    }
#else
    pop_n_elems(args);
    spent = getspent();
#endif
    
    while(spent) {
        push_spent(spent);
        nents++;

#if defined(_REENTRANT) && defined(HAVE_GETSPENT_R)
#ifdef HAVE_SOLARIS_GETSPENT_R
	spent = getspent_r(&spbuf, buf, THIS->sp_buf_max);
#else
	getspent_r(&spbuf, buf, THIS->sp_buf_max, &spent);
#endif
#else
        spent = getspent();
#endif
    }
    
    sp_arr = aggregate_array(nents);
    if (sp_arr)
        push_array(sp_arr);
    else
        push_int(0);
#ifdef _REENTRANT
    LOCAL_FREE(buf);
#endif
}

static void
f_shadow_create(INT32 args)
{
    pop_n_elems(args);
#ifdef HAVE_SYSCONF
    THIS->sp_buf_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (THIS->sp_buf_max < 0)
#endif
      THIS->sp_buf_max = 2048;
}

static void
init_shadow(struct object *o)
{
    THIS_LOW->object_name = _object_name;
    THIS_LOW->object_data = malloc(sizeof(struct shadow_struct));
    if (!THIS_LOW->object_data)
	error("Out of memory in AdminTools.Shadow init!\n");
	
    THIS->db_opened = 0;
    THIS->sp_buf_max = 0;
}

static void
exit_shadow(struct object *o)
{
    if (THIS_LOW->object_data)
	free(THIS_LOW->object_data);
}

struct program*
_at_shadow_init()
{
    start_new_program();
    ADD_STORAGE(ATSTORAGE);

    set_init_callback(init_shadow);
    set_exit_callback(exit_shadow);
    
    add_function("create", f_shadow_create, "function(void:void)", 0);    
    add_function("setspent", f_setspent, "function(void:void)", 0);
    add_function("endspent", f_endspent, "function(void:void)", 0);
    add_function("getspent", f_getspent, "function(void:array)", 0);
    add_function("getspnam", f_getspnam, "function(string:array)", 0);
    add_function("getallspents", f_getallspents,
                 "function(void:array(array))", 0);
    
    shadow_program = end_program();
    add_program_constant("Shadow", shadow_program, 0);
    
    return shadow_program;
}
