/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
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
 */

/* This file should be included by ALL source code. It includes various
 * stuff to maintain source code compatibility between Pike versions.
 *
 * $Id$
 */

#ifndef HAVE_CAUDIUM_UTIL_H
#define HAVE_CAUDIUM_UTIL_H

/* Standard Pike include files. */
#include "bignum.h"
#include "array.h"
#include "builtin_functions.h"
#include "constants.h"
#include "interpret.h"
#include "mapping.h"
#include "multiset.h"
#include "module_support.h"
#include "object.h"
#include "pike_macros.h"
#include "pike_types.h"
#include "program.h"
#include "stralloc.h"
#include "svalue.h"
#include "threads.h"
#include "version.h"
#include "operators.h"

#if (PIKE_MAJOR_VERSION == 7 && PIKE_MINOR_VERSION == 1 && PIKE_BUILD_VERSION >= 12) || PIKE_MAJOR_VERSION > 7 || (PIKE_MAJOR_VERSION == 7 && PIKE_MINOR_VERSION > 1)
# include "pike_error.h"
#  ifdef fp
#   undef fp
#  endif
#else
# include "error.h"
# ifndef Pike_error
#  define Pike_error error
# endif
#endif

/* Pexts version */
#define PEXTS_VERSION "0.1.1"
#define PEXTS_MAJOR   0
#define PEXTS_MINOR   1
#define PEXTS_BUILD   1

#ifndef MODULE_MAJOR
#define MODULE_MAJOR PEXTS_MAJOR
#endif

#ifndef MODULE_MINOR
#define MODULE_MINOR PEXTS_MINOR
#endif

#ifndef MODULE_BUILD
#define MODULE_BUILD PEXTS_BUILD
#endif

/*
 * Report module version. 
 * Returns a mapping:
 *
 *  retval->major - module major version
 *  retval->minor - module minor version
 *  retval->build - module build number
 */
static void
f_pexts_module_version(INT32 args)
{
    struct mapping   *retval;
    struct svalue    skey, sval;
    
    pop_n_elems(args);
    
    retval = allocate_mapping(3);
    
    skey.type = PIKE_T_STRING;
    sval.type = PIKE_T_INT;
    
    skey.u.string = make_shared_string("major");
    sval.u.integer = MODULE_MAJOR;
    mapping_insert(retval, &skey, &sval);
    
    skey.u.string = make_shared_string("minor");
    sval.u.integer = MODULE_MINOR;
    mapping_insert(retval, &skey, &sval);
    
    skey.u.string = make_shared_string("build");
    sval.u.integer = MODULE_BUILD;
    mapping_insert(retval, &skey, &sval);
    
    push_mapping(retval);
}

/*
 * Report pexts version. 
 * Returns a mapping:
 *
 *  retval->major - module major version
 *  retval->minor - module minor version
 *  retval->build - module build number
 */
static void
f_pexts_version(INT32 args)
{
    struct mapping   *retval;
    struct svalue    skey, sval;
    
    pop_n_elems(args);
    
    retval = allocate_mapping(3);
    
    skey.type = PIKE_T_STRING;
    sval.type = PIKE_T_INT;
    
    skey.u.string = make_shared_string("major");
    sval.u.integer = PEXTS_MAJOR;
    mapping_insert(retval, &skey, &sval);
    
    skey.u.string = make_shared_string("minor");
    sval.u.integer = PEXTS_MINOR;
    mapping_insert(retval, &skey, &sval);
    
    skey.u.string = make_shared_string("build");
    sval.u.integer = PEXTS_BUILD;
    mapping_insert(retval, &skey, &sval);
    
    push_mapping(retval);
}

/*
 * THis function merely registers the above two
 * calls. It must be invoked by the module.
 */
static void
pexts_init(void)
{
    ADD_FUNCTION("module_version", f_pexts_module_version,
                 tFunc(tVoid, tMap(tString, tInt)), 0);
    ADD_FUNCTION("pexts_version", f_pexts_version,
                 tFunc(tVoid, tMap(tString, tInt)), 0);
}

#ifndef ARG
/* Get argument # _n_ */
#define ARG(_n_) Pike_sp[-((args - _n_) + 1)]
#endif

#endif /* HAVE_CAUDIUM_UTIL_H */
