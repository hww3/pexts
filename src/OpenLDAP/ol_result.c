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
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "ol_config.h"
#include "ol_common.h"

#ifdef HAVE_LIBLDAP
struct program  *result_program;

#define THIS ((OLSRTORAGE*)get_storage(fp->current_object, result_program))
#define THIS_PARENT ((OLSTORAGE*)get_storage(Pike_fp->next->current_object, ldap_program))

static void
f_create(INT32 args)
{
    THIS->conn = THIS_PARENT->conn;
    THIS->msg = (LDAPMessage*)THIS_PARENT->data;

    pop_n_elems(args);
}

static void
f_num_entries(INT32 args)
{
    int  ret;
    
    pop_n_elems(args);
    ret = ldap_count_entries(THIS->conn, THIS->msg);

    push_int(ret);
}

static void
init_result(struct object *o)
{
    THIS->conn = NULL;
    THIS->msg = NULL;
}

static void
exit_result(struct object *o)
{
    if (THIS->msg)
        free(THIS->msg);
}

void
_ol_result_program_init(void)
{
    start_new_program();
    ADD_STORAGE(OLSRTORAGE);

    set_init_callback(init_result);
    set_exit_callback(exit_result);

    ADD_FUNCTION("create", f_create,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("num_entries", f_num_entries,
                 tFunc(tVoid, tInt), 0);
    
    result_program = end_program();
    add_program_constant("Result", result_program, 0);
}
#endif
