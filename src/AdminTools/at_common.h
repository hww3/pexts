/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2005 The Caudium Group
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
 * $Id$
 */

/*
 * File licensing and authorship information block.
 *
 * Version: MPL 1.1/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *
 * Marek Habersack <grendel@caudium.net>
 *
 * Portions created by the Initial Developer are Copyright (C) Marek Habersack
 * & The Caudium Group. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of the LGPL, and not to allow others to use your version
 * of this file under the terms of the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL or the LGPL.
 *
 * Significant Contributors to this file are:
 *
 */

/*
 *
 * Simple glue for more advanced Unix functions.
 * Common macros and declarations.
 *
 * $Id$
 */
#ifndef __at_common_h
#define __at_common_h

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
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

#if !defined(NAME_MAX) && defined(_PC_NAME_MAX)
#define NAME_MAX _PC_NAME_MAX
#elif !defined(NAME_MAX)
#define NAME_MAX 1024
#endif

#if defined(HAVE_FUNCTION_ATTRIBUTES)
#undef ATTRIBUTE
#define ATTRIBUTE(_x_) __attribute__ (_x_)
#else
#undef ATTRIBUTE
#define ATTRIBUTE(x)
#endif

/*
 * Common storage structure for the entire module.
 * Every object should allocate their own structures
 * dynamically and store the pointer here.
 */
typedef struct
{
    char      *object_name;
    void      *object_data;
} ATSTORAGE;

/*
 * Common functions
 */
void init_common(char *);
void OPERROR(char *on, char *format, ...) ATTRIBUTE((noreturn,format (printf, 2, 3)));
void FERROR(char *on, char *format, ...) ATTRIBUTE((noreturn,format (printf, 2, 3)));

struct program* _at_shadow_init(void);
struct program* _at_directory_init(void);
struct program* _at_quota_init(void);
struct program* _at_system_init(void);

#ifdef HAVE_PAM
struct program* _at_pam_init(void);
#endif

void pike_module_init(void);
void pike_module_exit(void);
#endif
