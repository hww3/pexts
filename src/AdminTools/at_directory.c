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
 * Directory navigation interface.
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

/*
 * opendir() and friends support
 *
 * Note that opendir/readdir is used by get_dir as well, we just provide 
 * a more fine-grained interface here with some more bells and whistles.
 */
#ifdef HAVE_OPENDIR
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
	error("AdminTools.Directory[do_opendir()]: %s\n", err);
    }
    
    return dirent;
}

static void
f_opendir(INT32 args)
{
    get_all_args("AdminTools.Directory->open", args, "%S", &THIS->dir.dirname);
    if (args != 1)
	error("AdminTools.Directory->open(): Invalid number of arguments. Expected 1.\n");
    
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
	error("AdminTools.Directory->open(): Wrong argument type for argument 1. Expected 8-bit string.\n");

    THIS->dir.dirname = make_shared_string(ARG(1).u.string->str);
    add_ref(THIS->dir.dirname);
    
    if (THIS->dir.dir)
	error("AdminTools.Directory->open(): previous directory not closed.\n");
	
    pop_n_elems(args);
    
    THIS->dir.dir = do_opendir(THIS->dir.dirname->str);
}
#else
static void
f_opendir(INT32 args)
{
    error("AdminTools.Directory->open(): function not supported\n");
}
#endif

#ifdef HAVE_CLOSEDIR
static void
f_closedir(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->dir.dir) {
	push_int(-1);
	return;
    }
    push_int(closedir(THIS->dir.dir));
}
#else
static void
f_closedir(INT32 args)
{
    error("AdminTools.Directory->close(): function not supported\n");
}
#endif

#if defined(HAVE_READDIR) || defined(HAVE_READDIR_R)
static void
push_dirent(struct dirent *dent)
{
    struct array    *arr;
    
    /* [0] - entry inode number (d_ino; POSIX) */
    push_int(dent->d_ino);
    
    /* 
     * [1] - offset of disk directory entry (d_off) 
     *  On systems that don't have this member, this is set
     *  to 0;
     */
#ifdef HAVE_DIRENT_D_OFF
    push_int(dent->d_off);
#else
    push_int(0);
#endif

    /* [2] - name of file (d_name; POSIX) */
    push_text(dent->d_name);
    
#ifdef HAVE_DIRENT_D_TYPE
    /* 
     * [3] - type of file (not all systems) (d_type) 
     *  This is fully supported only on BSD-compliant
     *  systems that define this field. If it is not
     *  supported by the system, it will be set to 'U' 
     *  which corresponds to DT_UNKNOWN.
     */
     
     switch(dent->d_type) {
        case DT_UNKNOWN:
	    push_text("U");
	    break;
	    
	case DT_REG:
	    push_text("R");
	    break;
	    
	case DT_DIR:
	    push_text("D");
	    break;
	    
	case DT_FIFO:
	    push_text("F");
	    break;
	    
	case DT_SOCK:
	    push_text("S");
	    break;
	    
	case DT_CHR:
	    push_text("C");
	    break;
	    
	case DT_BLK:
	    push_text("B");
	    break;
	    
	default:
	    push_text("?");
	    break;
     };
#else
    push_text("U");
#endif

    arr = aggregate_array(4);
    push_array(arr);
}

/*
 * This is the recommended safe way of calling readdir_r
 */
typedef union {
    struct dirent   d;
    char b[offsetof(struct dirent, d_name) + NAME_MAX + 1];
} DIRENT;

static struct dirent*
my_readdir(DIR *dir)
{
    struct dirent   *dent;
    
#if defined(_REENTRANT) && defined(HAVE_READDIR_R)
    DIRENT          dent2;
    
    if (readdir_r(dir, &dent2.d, &dent) != 0)
	error("AdminTools.Directory->read(): error reading directory.\n");
#else
    dent = readdir(dir);
#endif

    return dent;
}
#endif

#if defined(HAVE_READDIR) || defined(HAVE_READDIR_R)
static void
f_readdir(INT32 args)
{
    struct dirent   *dent;
    
    if (!THIS->dir.dir) {
	push_int(0);
	return;
    }
    
    dent = my_readdir(THIS->dir.dir);
    pop_n_elems(args);
    
    if (dent)
	push_dirent(dent);
    else
	push_int(0);
}
#else
static void
f_readdir(INT32 args)
{
    error("AdminTools.Directory->read(): function not supported\n");
}
#endif

#ifdef HAVE_REWINDDIR
static void
f_rewinddir(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->dir.dir)
	return;
	
    rewinddir(THIS->dir.dir);
}
#else
static void
f_rewinddir(INT32 args)
{
    error("AdminTools.Directory->rewind(): function not supported\n");
}
#endif

#ifdef HAVE_SEEKDIR
static void
f_seekdir(INT32 args)
{
    off_t     soff = 0;
    
    if (!THIS->dir.dir) {
	pop_n_elems(args);
	return;
    }
    
    if (args == 1) {
	if (ARG(1).type != T_INT)
	    error("AdminTools.Directory->seek(): Wrong argument type for argument 1 - expected int.\n");
	soff = ARG(1).u.integer;
    } else
	error("AdminTools.Directory->seek(): Wrong number of arguments. Expected 1 (int).\n");

    pop_n_elems(args);
    seekdir(THIS->dir.dir, soff);
}
#else
static void
f_seekdir(INT32 args)
{
    error("AdminTools.Directory->seek(): function not supported\n");
}
#endif

#ifdef HAVE_TELLDIR
static void
f_telldir(INT32 args)
{
    off_t     pos;
    
    pop_n_elems(args);
    if (!THIS->dir.dir) {

	push_int(-1);
	return;
    }
    
    pos = telldir(THIS->dir.dir);
    push_int(pos);
}
#else
static void
f_telldir(INT32 args)
{
    error("AdminTools.Directory->tell(): function not supported\n");
}
#endif

#ifdef HAVE_SCANDIR
static void
f_scandir(INT32 args)
{}
#else
static void
f_scandir(INT32 args)
{
    error("AdminTools.Directory->scandir(): function not supported\n");
}
#endif

static void
f_dir_create(INT32 args)
{
#ifdef HAVE_OPENDIR
    if (args > 1)
	error("AdminTools.Directory->create(): Invalid number of arguments. Expected 0 or 1.\n");

    if (args == 1) {
	if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
	    error("AdminTools.Directory->create(): Wrong argument type for argument 1. Expected 8-bit string.\n");

	THIS->dir.dirname = make_shared_string(ARG(1).u.string->str);
	add_ref(THIS->dir.dirname);

	THIS->dir.dir = do_opendir(THIS->dir.dirname->str);
    } else if (args > 1) {
	error("AdminTools.Directory->create(): too many arguments\n");
    } else
	THIS->dir.dir = NULL;
#else
    error("AdminTools.Directory->open(): function not supported\n");
#endif

    pop_n_elems(args);
    THIS->dir.select_cb.type = T_INT;
    THIS->dir.select_cb.u.integer = 0;
    THIS->dir.compare_cb.type = T_INT;
    THIS->dir.compare_cb.u.integer = 0;
    THIS->dir.dirname = NULL;
}

struct program*
_at_directory_init(void)
{
    struct program *dir_program;
    
    start_new_program();
    ADD_STORAGE(struct INSTANCE);
    
    add_function("create", f_dir_create, "function(void|string:void)", 0);
    add_function("open", f_opendir, "function(string:void)", 0);
    add_function("close", f_closedir, "function(void:int)", 0);
    add_function("read", f_readdir, "function(void:array)", 0);
    add_function("rewind", f_rewinddir, "function(void:void)", 0);
    add_function("seek", f_seekdir, "function(void|int:void)", 0);
    add_function("tell", f_telldir, "function(void:int)", 0);
    
    dir_program = end_program();
    add_program_constant("Directory", dir_program, 0);
    
    return dir_program;
}
