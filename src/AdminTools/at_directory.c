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

#include "pexts.h"

#include "at_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "at_common.h"

/*
 * This is the recommended safe way of calling readdir_r
 */
typedef union {
    struct dirent   d;
    char b[offsetof(struct dirent, d_name) + NAME_MAX + 1];
} DIRENT;

struct dir_struct 
{
    DIR                 *dir;
    struct pike_string  *path;
    struct dirent       *dent; /* cache */
#ifdef HAVE_READDIR_R
    DIRENT              dent2;
#endif
    off_t               offset;
    
    struct svalue      select_cb;
    struct svalue      compare_cb;    
};

static struct program *dir_program;

static char *_object_name = "Directory";

static struct pike_string *s_ino;
static struct pike_string *s_off;
static struct pike_string *s_reclen;
static struct pike_string *s_name;

#define THIS_LOW ((ATSTORAGE*)get_storage(fp->current_object, dir_program))
#define THIS ((struct dir_struct*)THIS_LOW->object_data)

/*
 * opendir() and friends support
 *
 * Note that opendir/readdir is used by get_dir as well, we just provide 
 * a more fine-grained interface here with some more bells and whistles.
 */
#ifdef HAVE_OPENDIR
inline static DIR*
do_opendir(char *dirpath)
{
    DIR   *dirent;
    
    dirent = opendir(dirpath);
    if (!dirent)
        return NULL;
    
    return dirent;
}
#endif

#if defined(HAVE_READDIR) || defined(HAVE_READDIR_R)
inline static void
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

inline static struct dirent*
do_readdir(DIR *dir)
{
#if defined(_REENTRANT) && defined(HAVE_READDIR_R)
    if (readdir_r(dir, &THIS->dent2.d, &THIS->dent) != 0)
        FERROR("read", "error reading directory");
#else
    THIS->dent = readdir(dir);
#endif

#ifdef HAVE_TELLDIR
    THIS->offset = telldir(dir);
#endif
    
    return THIS->dent;
}
#endif

#ifdef HAVE_OPENDIR
static void
f_opendir(INT32 args)
{
    if (THIS->dir)
        FERROR("open", "directory already opened. Close it first");

    if (args > 1)
        FERROR("open", "too many arguments. Expected 0 or 1");
    
    if (args == 1) {
        if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
            FERROR("open", "wrong type of argument 1; expected 8-bit string");
        THIS->path = make_shared_string(ARG(1).u.string->str);
    } else
        THIS->path = make_shared_string("./");

    add_ref(THIS->path);
    THIS->dir = do_opendir(THIS->path->str);
    if (!THIS->dir)
        FERROR("open", "error opening directory");

#ifdef HAVE_TELLDIR
    THIS->offset = telldir(THIS->dir);
#endif
    
    pop_n_elems(args);
}
#endif

#ifdef HAVE_CLOSEDIR
static void
f_closedir(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->dir) {
        push_int(-1);
        return;
    }
    push_int(closedir(THIS->dir));
}
#endif

#if defined(HAVE_READDIR) || defined(HAVE_READDIR_R)
static void
f_readdir(INT32 args)
{
    struct dirent   *dent;
    
    if (!THIS->dir) {
        push_int(0);
        return;
    }
    
    dent = do_readdir(THIS->dir);
    pop_n_elems(args);
    
    if (dent)
        push_dirent(dent);
    else
        push_int(0);
}
#endif

#ifdef HAVE_REWINDDIR
static void
f_rewinddir(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->dir)
        return;
	
    rewinddir(THIS->dir);
}
#endif

#ifdef HAVE_SEEKDIR
static void
f_seekdir(INT32 args)
{
    off_t     soff = 0;
    
    if (!THIS->dir) {
        FERROR("seek", "Directory not opened");
        pop_n_elems(args);
        return;
    }
    
    if (args == 1) {
        if (ARG(1).type != T_INT)
            FERROR("seek", "Wrong argument type for argument 1 - expected int");
        soff = ARG(1).u.integer;
    } else
        FERROR("seek", "Wrong number of arguments. Expected 1 (int)");
    
    pop_n_elems(args);
    seekdir(THIS->dir, soff);

#ifdef HAVE_TELLDIR
    THIS->offset = telldir(THIS->dir);
#endif
}
#endif

#ifdef HAVE_TELLDIR
static void
f_telldir(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->dir) {
        FERROR("tell", "Directory not opened");
        push_int(-1);
        return;
    }
    
    THIS->offset = telldir(THIS->dir);
    push_int(THIS->offset);
}
#endif

#ifdef HAVE_SCANDIR
static void
f_scandir(INT32 args)
{}
#endif

static void
f_dir_create(INT32 args)
{
    /* Some health checks */
#if !defined(HAVE_OPENDIR) || !defined(HAVE_CLOSEDIR) || !defined(HAVE_READDIR)
    FERROR( "create", "OS directory interfaces not fully functional. Cannot work.");
#endif
    if (args == 1) {
        if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
            FERROR("create", "Wrong argument type for argument 1. Expected 8-bit string");

        THIS->path = make_shared_string(ARG(1).u.string->str);
        add_ref(THIS->path);

        THIS->dir = do_opendir(THIS->path->str);
        if (!THIS->dir)
            FERROR("create", "Error opening directory");
    } else if (args > 1) {
        FERROR("create", "too many arguments");
    } else
    THIS->dir = NULL;
    
    pop_n_elems(args);
}

static void
f_dir_index(INT32 args)
{
    long int        pos;

    if (!THIS->dir)
        OPERROR("[]", "directory not opened yet");
    
    if (args != 1)
        OPERROR("[]", "wrong number of arguments. Expected 1");

    switch (ARG(1).type) {
        case T_STRING:
            if (!THIS->dent)
                do_readdir(THIS->dir);
            
            if (ARG(1).u.string == s_ino) {
                pop_n_elems(args);                
                push_int(THIS->dent->d_ino);
            } else if (ARG(1).u.string == s_off) {
                pop_n_elems(args);                
                push_int(THIS->dent->d_off);
            } else if (ARG(1).u.string == s_reclen) {
                pop_n_elems(args);                
                push_int(THIS->dent->d_reclen);
            } else if (ARG(1).u.string == s_name) {
                pop_n_elems(args);
                push_text(THIS->dent->d_name);
            } else {
                Pike_error("AdminTools.%s[]: unknown index '%s'\n", _object_name, ARG(1).u.string->str);
                pop_n_elems(args);
            }
            break;
        
        default:
            OPERROR("[]", "wrong type of argument 1. Expected string");
            pop_n_elems(args);
            break;
    }
}

static void
init_directory(struct object *o)
{
    THIS_LOW->object_name = _object_name;
    THIS_LOW->object_data = malloc(sizeof(struct dir_struct));
    if (!THIS_LOW->object_data)
	Pike_error("Out of memory in AdminTools.Directory init!\n");
	
    THIS->dir = NULL;
    THIS->path = NULL;
    THIS->dent = NULL;
    THIS->offset = 0;
    THIS->select_cb.type = T_INT;
    THIS->select_cb.u.integer = 0;
    THIS->compare_cb.type = T_INT;
    THIS->compare_cb.u.integer = 0;
}

static void
exit_directory(struct object *o)
{
    if (THIS->dir)
        closedir(THIS->dir);
    
    if (THIS_LOW->object_data)
	free(THIS_LOW->object_data);

    free_string(s_ino);
    free_string(s_off);
    free_string(s_reclen);
    free_string(s_name);
}

struct program*
_at_directory_init(void)
{
    start_new_program();
    ADD_STORAGE(ATSTORAGE);

    set_init_callback(init_directory);
    set_exit_callback(exit_directory);

    s_ino = make_shared_string("d_ino");
    s_off = make_shared_string("d_off");
    s_reclen = make_shared_string("d_reclen");
    s_name = make_shared_string("d_name");
    
    add_function("create", f_dir_create,
                 "function(void|string:void)", 0);
    add_function("open", f_opendir,
                 "function(void|string:void)", 0);
    add_function("close", f_closedir,
                 "function(void:int)", 0);
    add_function("read", f_readdir,
                 "function(void:array)", 0);
    add_function("rewind", f_rewinddir,
                 "function(void:void)", 0);
    add_function("seek", f_seekdir,
                 "function(void|int:void)", 0);
    add_function("tell", f_telldir,
                 "function(void:int)", 0);

    /* Operators */
    add_function("`[]", f_dir_index,
                 "function(string|int:string)", 0);
    
    dir_program = end_program();
    add_program_constant("Directory", dir_program, 0);
    
    return dir_program;
}
