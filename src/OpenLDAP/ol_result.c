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

#define THIS ((OLRSTORAGE*)get_storage(fp->current_object, result_program))
#define THIS_PARENT ((OLSTORAGE*)get_storage(Pike_fp->next->current_object, ldap_program))

static void
f_create(INT32 args)
{
    THIS->conn = THIS_PARENT->conn;
    THIS->msg = (LDAPMessage*)THIS_PARENT->data;
    THIS->cur = ldap_first_entry(THIS->conn, THIS->msg);
    THIS->num_entries = ldap_count_entries(THIS->conn, THIS->msg);
    
    pop_n_elems(args);
}

static void
f_num_entries(INT32 args)
{
    pop_n_elems(args);
    push_int(THIS->num_entries);
}

static void
f_ldap_get_dn(INT32 args)
{
    char *dn;
    
    pop_n_elems(args);
    if (!THIS->cur) {
        push_int(0);
    } else {  
        dn = ldap_get_dn(THIS->conn, THIS->cur);
        if (!dn) {
            push_int(0);
        } else {
            push_string(make_shared_string(dn));
            ldap_memfree(dn);
        }
    }
}

static void
f_ldap_dn2ufn(INT32 args)
{
    struct pike_string   *dn = NULL;
    char                 *ufn;
    
    if (args != 1)
        Pike_error("OpenLDAP.Client.Result->dn2ufn(): requires exactly one 8-bit string argument\n");

    get_all_args("OpenLDAP.Client.Result->dn2ufn()", args, "%S", &dn);
    if (!dn) {
        push_int(0);
        return;
    }

    ufn = ldap_dn2ufn(dn->str);
    if (!ufn) {
        push_int(0);
    } else {
        push_string(make_shared_string(ufn));
        ldap_memfree(ufn);
    }
}

static void
init_result(struct object *o)
{
    THIS->conn = NULL;
    THIS->msg = NULL;
    THIS->cur = NULL;
    THIS->num_entries = 0;
}

static void
exit_result(struct object *o)
{
    if (THIS->msg)
        free(THIS->msg);
    if (THIS->cur)
        free(THIS->cur);
}

void
_ol_result_program_init(void)
{
    start_new_program();
    ADD_STORAGE(OLRSTORAGE);

    set_init_callback(init_result);
    set_exit_callback(exit_result);

    ADD_FUNCTION("create", f_create,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("num_entries", f_num_entries,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("get_dn", f_ldap_get_dn,
                 tFunc(tVoid, tString), 0);
    ADD_FUNCTION("dn2ufn", f_ldap_dn2ufn,
                 tFunct(tString, tString), 0);
    
    result_program = end_program();
    add_program_constant("Result", result_program, 0);
}
#endif
