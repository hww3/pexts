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
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

/*
 * This module contains glue for the various Unix administrative calls
 * not supported by the mainstream Pike modules. Currently supported
 * calls are:
 *
 * *** Shadow passwords support ***
 *
 *  PORTABILITY: Linux
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
 *
 *
 * *** QUOTACTL support ***
 */

struct SHADOWPWDB
{
    int      db_opened;
    long     sp_buf_max;
};

struct DIRDATA
{
    DIR                *dir;
    struct pike_string *dirname;
    struct svalue      select_cb;
    struct svalue      compare_cb;
};

struct INSTANCE
{
    struct SHADOWPWDB    shadow;
    struct DIRDATA       dir;
};

DEFINE_IMUTEX(at_shadow_mutex);

struct program   *shadow_program;
struct program   *dir_program;

#define THIS ((struct INSTANCE*) fp->current_storage)

#if SIZEOF_LONG >= 8
#define push_longint(_x_) push_int64((_x_))
#else
#define push_longint(_x_) push_int((_x_))
#endif

#define ARG(_n_) sp[-(_n_)]

#if defined(__GNUC__)
#define LOCAL_BUF(_n_, _s_) char _n_[_s_]
#define LOCAL_FREE(_n_)
#define LOCAL_CHECK(_n_, _msg_)
#elif defined(HAVE_ALLOCA)
#define LOCAL_BUF(_n_, _s_) char *_n_ = alloca(_s_)
#define LOCAL_FREE(_n_)
#define LOCAL_CHECK(_n_, _msg_) if (!_n_) error(_msg)
#else
#define LOCAL_BUF(_n_, _s_) char *_n_ = malloc(_s_)
#define LOCAL_FREE(_n_) if (_n_) free(_n_)
#define LOCAL_CHECK(_n_, _msg_) if (!_n_) error(_msg)
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

#ifdef HAVE_SETSPENT 
static void
f_setspent(INT32 args)
{
    pop_n_elems(args);
    if (THIS->shadow.db_opened)
        return;

    setspent();
    THIS->shadow.db_opened = 1;
}
#else
static void
f_setspent(INT32 args)
{
    error("AdminTools.Shadow->setspent(): function not supported\n");
}
#endif

#ifdef HAVE_ENDSPENT
static void
f_endspent(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->shadow.db_opened) {
      return;
    }
    endspent();
    THIS->shadow.db_opened = 0;
}
#else
static void
f_endspent(INT32 args)
{
    error("AdminTools.Shadow->endspent(): function not supported\n");
}
#endif

#if defined(HAVE_GETSPENT) || defined(HAVE_GETSPENT_R)
static void
f_getspent(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->shadow.db_opened)
        f_setspent(0);

#if defined(_REENTRANT) && defined(HAVE_GETSPENT_R)
    {
	LOCAL_BUF(buf, THIS->shadow.sp_buf_max);
        struct spwd    spbuf, *spent;

        LOCAL_CHECK(buf, "AdminTools.Shadow->getspent(): out of memory\n");

	/*
	 * There is _no_ way to tell the difference between an error
	 * and the end of DB using this function...
	 * /grendel 22-09-2000
	 */
#ifdef HAVE_SOLARIS_GETSPENT_R
	if (getspent_r(&spbuf, buf, THIS->shadow.sp_buf_max) != 0)
#else
        if (getspent_r(&spbuf, buf, THIS->shadow.sp_buf_max, &spent) != 0)
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
#else
static void
f_getspent(INT32 args)
{
    error("AdminTools.Shadow->getspent(): function not supported.\n");
}
#endif

static void
f_getallspents(INT32 args)
{
    struct spwd    *spent;
    INT32          nents;
    struct array   *sp_arr;
#ifdef _REENTRANT
    LOCAL_BUF(buf, THIS->shadow.sp_buf_max);
    struct spwd    spbuf;
#endif

    pop_n_elems(args);
    if(THIS->shadow.db_opened)
        f_endspent(0);
    f_setspent(0);

    nents = 0;

#if defined(_REENTRANT) && defined(HAVE_GETSPENT_R)
    LOCAL_CHECK(buf, "AdminTools.Shadow->getallspents(): out of memory\n");

#ifdef HAVE_SOLARIS_GETSPENT_R
    if (getspent_r(&spbuf, buf, THIS->shadow.sp_buf_max) != 0) {
#else
    if (getspent_r(&spbuf, buf, THIS->shadow.sp_buf_max, &spent) != 0) {
#endif
        push_int(0);
        return;
    }
#else
    spent = getspent();
#endif
    
    while(spent) {
        push_spent(spent);
        nents++;

#if defined(_REENTRANT) && defined(HAVE_GETSPENT_R)
#ifdef HAVE_SOLARIS_GETSPENT_R
	getspent_r(&spbuf, buf, THIS->shadow.sp_buf_max);
#else
	getspent_r(&spbuf, buf, THIS->shadow.sp_buf_max, &spent);
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
    THIS->shadow.sp_buf_max = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (THIS->shadow.sp_buf_max < 0)
        THIS->shadow.sp_buf_max = 2048;
}

/*
 * opendir() and friends support
 *
 * Note that opendir/readdir is used by get_dir as well, we just provide 
 * a more fine-grained interface here with some more bells and whistles.
 */
static DIR*
do_opendir(char *dirpath)
{
    DIR   *dirent;
    
    dirent = opendir(THIS->dir.dirname->str);
    if (!dirent) {
	char  *err;
	
#ifdef HAVE_STRERROR
	err = strerror(errno);
#else
	switch(errno) {
	    case EACCESS: 
		err = "access denied";
		break;
		
	    case EMFILE: 
		err = "Too many file descriptors in use by process";
		break;
		
	    case ENFILE:
		err = "Too many files are currently open in the system";
		break;
		
	    case ENOENT:
		err = "Directory doesn't exist";
		break;
		
	    case ENOMEM:
		err = "Out of memory";
		break;
		
	    case ENOTDIR:
		err = "Trying to open a non-directory";
		break;
		
	    default:
		err = "Unknown error";
	}
#endif
	/*
	 * Shouldn't we leave interpreting errno to the calling
	 * program rather than use error() here?
	 * /grendel 26-09-2000
	 */
	error("AdminTools.Directory->opendir(): %s\n", err);
    }
    
    return dirent;
}

static void
f_opendir(INT32 args)
{
#ifdef HAVE_OPENDIR
    get_all_args("AdminTools.Directory->opendir", args, "%S", &THIS->dir.dirname);
    if (args != 1)
	error("AdminTools.Directory->opendir(): Invalid number of arguments. Expected 1.\n");
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
	error("AdminTools.Directory->opendir(): Wrong argument type for argument 1. Expected 8-bit string.\n");

    /*
     * Do I have to make a copy of the string that's on the
     * stack to preserve it for later use?? Guess not...
     * /grendel 26-09-2000
     */	
    THIS->dir.dirname = ARG(1).u.string;
    pop_n_elems(args);

    if (THIS->dir.dir)
	error("AdminTools.Directory->opendir(): previous directory not closed.\n");

    THIS->dir.dir = do_opendir(THIS->dir.dirname->str);
#else
    error("AdminTools.Directory->opendir(): function not supported\n");
#endif
}

static void
f_closedir(INT32 args)
{
#ifdef HAVE_CLOSEDIR
    pop_n_elems(args);
    if (!THIS->dir.dir) {
	push_int(-1);
	return;
    }
    push_int(closedir(THIS->dir.dir));
#else
    error("AdminTools.Directory->closedir(): function not supported\n");
#endif
}

#if defined(HAVE_READDIR) || defined(HAVE_READDIR_R)
static struct dirent*
my_readdir(DIR *dir)
{
#if defined(_REENTRANT) && defined(HAVE_READDIR_R)    
#else
#endif
}
#endif

static void
f_readdir(INT32 args)
{
#if defined(HAVE_READDIR_R) || defined(HAVE_READDIR)
    struct dirent   *dent;
    
    if (!THIS->dir.dir) {
	push_int(0);
	return;
    }
    
    dent = my_readdir(THIS->dir.dir);
#else
    error("AdminTools.Directory->readdir(): function not supported\n");
#endif
}

static void
f_rewinddir(INT32 args)
{}

static void
f_seekdir(INT32 args)
{}

static void
f_telldir(INT32 args)
{}

static void
f_scandir(INT32 args)
{}

static void
f_dir_create(INT32 args)
{
#ifdef HAVE_OPENDIR
    if (args > 1)
	error("AdminTools.Directory->create(): Invalid number of arguments. Expected 0 or 1.\n");

    if (args == 1) {
	if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
	    error("AdminTools.Directory->create(): Wrong argument type for argument 1. Expected 8-bit string.\n");

	THIS->dir.dirname = ARG(1).u.string;
	pop_n_elems(args);

	THIS->dir.dir = do_opendir(THIS->dir.dirname->str);
    } else if (args > 1)
	error("AdminTools.Directory->create(): too many arguments\n");
    else
	THIS->dir.dir = NULL;

#else
    error("AdminTools.Directory->opendir(): function not supported\n");
#endif

    THIS->dir.select_cb.type = T_INT;
    THIS->dir.select_cb.u.integer = 0;
    THIS->dir.compare_cb.type = T_INT;
    THIS->dir.compare_cb.u.integer = 0;
    THIS->dir.dirname = NULL;
}

static void
f_dir_destroy(INT32 args)
{
#ifdef HAVE_CLOSEDIR
    if (THIS->dir.dir)
	closedir(THIS->dir.dir);
    error("destroy called\n");
#endif
    pop_n_elems(args);
}

void pike_module_init(void)
{
    init_interleave_mutex(&at_shadow_mutex);

    /* Shadow stuff */
    start_new_program();
    ADD_STORAGE(struct INSTANCE);

    add_function("create", f_shadow_create, "function(void:void)", 0);
    
    add_function("setspent", f_setspent, "function(void:void)", 0);

    add_function("endspent", f_endspent, "function(void:void)", 0);
		 
    add_function("getspent", f_getspent, 
                 "function(void:array)", 0);
		 
    add_function("getallspents", f_getallspents,
                 "function(void:array(array))", 0);
    
    shadow_program = end_program();
    add_program_constant("Shadow", shadow_program, 0);
    
    /* Dir stuff */
    start_new_program();
    ADD_STORAGE(struct INSTANCE);
    
    add_function("create", f_dir_create, "function(void|string:void)", 0);
    add_function("open", f_opendir, "function(string:void)", 0);
    add_function("close", f_closedir, "function(void:int)", 0);
    add_function("read", f_readdir, "function(void:array)", 0);
    add_function("destroy", f_dir_destroy, "function(void:void)", 0);

    dir_program = end_program();
    add_program_constant("Directory", dir_program, 0);
}

void pike_module_exit(void)
{
  free_program(shadow_program);
  free_program(dir_program);
}
