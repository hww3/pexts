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
 * Common macros and declarations.
 */
#ifndef __at_common_h
#define __at_common_h

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

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

struct program* _at_shadow_init(void);
struct program* _at_directory_init(void);
#endif
