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
 * Common macros and declarations.
 *
 * $Id$
 */
#ifndef __ol_common_h
#define __ol_common_h

#include "ol_config.h"

#ifdef HAVE_LDAP_H
#include <ldap.h>
#endif

#if SIZEOF_LONG >= 8
#define push_longint(_x_) push_int64((_x_))
#else
#define push_longint(_x_) push_int((_x_))
#endif

#define ARG(_n_) sp[-((args - _n_) + 1)]

#if defined(__GNUC__)
#define LOCAL_BUF(_n_, _s_) char _n_[_s_]
#define LOCAL_FREE(_n_)
#define LOCAL_CHECK(_n_, _msg_)
#elif defined(HAVE_ALLOCA)
#define LOCAL_BUF(_n_, _s_) char *_n_ = alloca(_s_)
#define LOCAL_FREE(_n_)
#define LOCAL_CHECK(_n_, _fn_) if (!_n_) FERROR("out of memory creating local buffer with alloca", _fn_)
#else
#define LOCAL_BUF(_n_, _s_) char *_n_ = malloc(_s_)
#define LOCAL_FREE(_n_) if (_n_) free(_n_)
#define LOCAL_CHECK(_n_, _fn_) if (!_n_) FERROR("out of memory creating local buffer with malloc", _fn_)
#endif

#if defined(HAVE_FUNCTION_ATTRIBUTES)
#undef ATTRIBUTE
#define ATTRIBUTE(_x_) __attribute__ (_x_)
#else
#undef ATTRIBUTE
#define ATTRIBUTE(x)
#endif

#define OL_CACHE_MAX      8192
#define OL_CACHE_TIMEOUT  300

typedef struct 
{
    LDAP                *conn;
    LDAPURLDesc         *server_url;

    int                  lerrno;
    int                  bound:1;
    int                  caching:1;
} OLSTORAGE;

struct program* _ol_ldap_program_init(void);

#endif
