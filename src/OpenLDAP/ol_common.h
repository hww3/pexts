/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2002 The Caudium Group
 */

/* Copyright (C) 2000-2002 The Caudium Group
 * Copyright (C) 2000-2002 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The OpenLDAP Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The OpenLDAP Pexts Module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Pike Extensions; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 *
 * $Id$
 */
#ifndef __ol_common_h
#define __ol_common_h

#include "ol_config.h"

#ifdef HAVE_LBER_H
#include <lber.h>
#endif
#ifdef HAVE_LDAP_H
#include <ldap.h>
#endif

#if SIZEOF_LONG >= 8
#define push_longint(_x_) push_int64((_x_))
#else
#define push_longint(_x_) push_int((_x_))
#endif

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

#define OL_DEF_FILTER     "dn=*"

#ifdef HAVE_LDAP_H
typedef struct 
{
    LDAP                *conn;
    LDAPURLDesc         *server_url;

    struct pike_string  *basedn;
    int                  scope;
    
    int                  lerrno;
    int                  bound:1;
    int                  caching:1;

    void                *data;
} OLSTORAGE;

typedef struct
{
    LDAPMessage         *msg;
    LDAPMessage         *cur;
    LDAP                *conn;
    int                  num_entries;
} OLRSTORAGE;
#endif

void _ol_ldap_program_init(void);
void _ol_result_program_init(void);

struct array* make_pike_array(char **carr);

extern struct program   *ldap_program;
extern struct program   *result_program;

#endif
