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

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

/*
 * This module contains glue for the various Unix administrative calls
 * not supported by the mainstream Pike modules. Currently supported
 * calls are:
 *
 * *** Shadow passwords support ***
 *
 *  Shadow passwords are used by Pike, but only to get the shadow (see
 *  modules/system/passwords.c) password. I want more :). Note that
 *  this code currently supports only Linux shadow passwords. Where
 *  possible, the module uses thread-safe versions of the library
 *  calls.
 *
 *  void setspent(void);
 *    Open the shadow database for reading.
 *
 *  void endspent(void);
 *    Close the shadow database after reading.
 *
 *  int|array getspent(void);
 *    Return a mapping filled with the next shadow entry read from the
 *    shadow database. If this function invocation wasn't preceeded
 *    with a call to setspent() the database will be opened by this
 *    function. The caller must take care to call endspent() after all
 *    the required entries have been read.
 *    Returns:
 *
 *      0 if error ocurred,
 *      array of data otherwise:
 *        [0] - login name
 *        [1] - encrypted password
 *        [2] - date of last change
 *        [3] - min. nr of days between passwd changes
 *        [4] - max. nr of days between passwd changes
 *        [5] - nr of days to warn the user b4 the passwd expires
 *        [6] - nr of days the account may be inactive
 *        [7] - nr of days since the Epoch until the account expires
 *
 *  int|array(array) getallspents(void);
 *    Return an array of arrays as defined in getspent() containing all
 *    the accounts found in the /etc/shadow database. Returns 0 if it is
 *    impossible to retrieve the data.
 */

struct SHADOWPWDB
{
    int      db_opened;
    long     sp_buf_max;
};

struct INSTANCE
{
    struct SHADOWPWDB    shadow;
};

DEFINE_IMUTEX(at_shadow_mutex);

struct program   *shadow_program;

#define THIS ((struct INSTANCE*) fp->current_storage)

#if SIZEOF_LONG >= 8
#define push_longint(_x_) push_int64((_x_))
#else
#define push_longint(_x_) push_int((_x_))
#endif

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
 
static void
f_setspent(INT32 args)
{
    if (THIS->shadow.db_opened)
        return;

    setspent();
    THIS->shadow.db_opened = 1;
}

static void
f_endspent(INT32 args)
{
    if (!THIS->shadow.db_opened)
        return;

    endspent();
    THIS->shadow.db_opened = 0;
}

static void
f_getspent(INT32 args)
{
    if (!THIS->shadow.db_opened)
        f_setspent(0);

#ifndef     _REENTRANT
    push_spent(getspent());
#else
    {
#ifdef __GNUC__
        char           buf[THIS->shadow.sp_buf_max];
#else
        char           *buf;
#endif
        struct spwd    sp, *spent;

#if !defined(__GNUC__) && defined(HAVE_ALLOCA)
        buf = (char*)alloca(THIS->shadow.sp_buf_max);
#elif !defined(__GNUC__) && !defined(HAVE_ALLOCA)
        buf = (char*)malloc(THIS->shadow.sp_buf_max * sizeof(char));
#endif
        
        if (getspent_r(&sp, buf, THIS->shadow.sp_buf_max, &spent) != 0)
            error("AdminTools.Shadow->getspent(): error retrieving next shadow entry\n");
        push_spent(spent);

#if !defined(__GNUC__) && !defined(HAVE_ALLOCA)
        free(buf);
#endif
    }
#endif
}

static void
f_getallspents(INT32 args)
{
    struct spwd    *spent;
    INT32          nents;
    struct array   *sp_arr;
    char           buf[1024];
    struct spwd    sp;
    
    if(THIS->shadow.db_opened)
        f_endspent(0);
    f_setspent(0);

    nents = 0;

    spent = getspent();
    if (getspent_r(&sp, buf, sizeof(buf), &spent) != 0)
        return;
    
    while(spent) {
        push_spent(spent);
        nents++;

        spent = getspent();
    }
    
    sp_arr = aggregate_array(nents);
    if (sp_arr)
        push_array(sp_arr);
}

static void
f_create(INT32 args)
{
    THIS->shadow.sp_buf_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (THIS->shadow.sp_buf_max < 0)
        THIS->shadow.sp_buf_max = 2048;
}

void pike_module_init(void)
{
    init_interleave_mutex(&at_shadow_mutex);

    start_new_program();
    ADD_STORAGE(struct INSTANCE);

    ADD_FUNCTION("create", f_create,
                 tFunc(tVoid, tVoid), 0);
    
    ADD_FUNCTION("setspent", f_setspent, 
                 tFunc(tVoid, tVoid), 0);

    ADD_FUNCTION("endspent", f_endspent, 
                 tFunc(tVoid, tVoid), 0);
		 
    ADD_FUNCTION("getspent", f_getspent, 
                 tFunc(tVoid,tOr(tInt,tArray)), 0);
		 
    ADD_FUNCTION("getallspents", f_getallspents,
                 tFunc(tVoid,tOr(tInt,tArr(tArray))), 0);
    
    end_class("Shadow", 0);
}

void pike_module_exit(void)
{
}
