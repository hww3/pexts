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

#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "ol_config.h"
#include "ol_common.h"

#ifdef HAVE_LIBLDAP
struct program  *result_program;

#define THIS ((OLRSTORAGE*)get_storage(Pike_fp->current_object, result_program))
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
add_attr_to_mapping(struct mapping *m, char *attr, struct berval **bvals)
{
    struct svalue   val, key;
    int             len, i;
    
    key.type = T_STRING;
    key.u.string = make_shared_string(attr);
    
    val.type = T_ARRAY;
    if (!bvals) {
        val.u.array = 0;
        mapping_insert(m, &key, &val);
        return;
    }
    
    len = ldap_count_values_len(bvals);
    for (i = 0; i < len; i++)
        push_string(make_shared_binary_string(bvals[i]->bv_val, bvals[i]->bv_len));

    val.u.array = aggregate_array(len);
    mapping_insert(m, &key, &val);
}

static struct mapping *
make_attr_mapping(LDAP *conn, LDAPMessage *entry)
{
    int              cnt = 0;
    char           **values;
    char            *attr;
    BerElement      *ber;
    struct berval  **bervals;
    struct mapping  *ret = NULL;
    
    attr = ldap_first_attribute(THIS->conn, entry, &ber);
    while (attr) {
        if (!ret)
            ret = allocate_mapping(1);
        bervals = ldap_get_values_len(THIS->conn, entry, attr);
/*        if (!bervals && THIS->ld_errno != LDAP_SUCCESS)
            Pike_error("OpenLDAP.Client.Result->fetch(): %s",
                       ldap_err2string(THIS->ld_errno));
*/
        add_attr_to_mapping(ret, attr, bervals);
        if (bervals)
            ldap_value_free_len(bervals);
        attr = ldap_next_attribute(THIS->conn, entry, ber);
        cnt++;
    }
    
    if (!cnt) {
        free_mapping(ret);
        return NULL;
    }
    
    return ret;
}

static void
f_fetch(INT32 args)
{
    int              idx = 0;
    struct mapping  *ret;
    
    if (args > 1)
        Pike_error("OpenLDAP.Client.Result->fetch(): requires at most one argument\n");
    
    get_all_args("OpenLDAP.Client.Result->fetch()", args, "%i", &idx);

    if (idx > THIS->num_entries || idx < 0) {
        push_int(0);
        return;
    }

    ret = make_attr_mapping(THIS->conn, THIS->cur);

    pop_n_elems(args);

    if (!ret)
        push_int(0);
    else
        push_mapping(ret);
}

static void
f_fetch_all(INT32 args)
{
    int             cnt = 0;
    BerElement     *ber;
    struct berval **bervals;
    struct mapping *tmp;
    LDAPMessage    *cur;
    
    pop_n_elems(args);
    
    cur = ldap_first_entry(THIS->conn, THIS->msg);
    while (cur) {
        tmp = make_attr_mapping(THIS->conn, cur);
        push_mapping(tmp);
        cur = ldap_next_entry(THIS->conn, cur);
        cnt++;
    }

    if (cnt) {
        struct array  *arr;
        
        arr = aggregate_array(cnt);
        push_array(arr);
    } else
        push_int(0);
}

static void
f_ldap_first_entry(INT32 args)
{
    if (!THIS_PARENT->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    THIS->cur = ldap_first_entry(THIS->conn, THIS->msg);
    pop_n_elems(args);
}

static void
f_ldap_next_entry(INT32 args)
{
    if (!THIS_PARENT->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    THIS->cur = ldap_next_entry(THIS->conn, THIS->cur);
    pop_n_elems(args);
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
    ADD_FUNCTION("fetch", f_fetch,
                 tFunc(tInt, tMap(tString, tArr(tString))), 0);
    ADD_FUNCTION("fetch_all", f_fetch_all,
                 tFunc(tVoid, tArr(tMap(tString, tArr(tString)))), 0);
    ADD_FUNCTION("first", f_ldap_first_entry,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("next", f_ldap_next_entry,
                 tFunc(tVoid, tVoid), 0);
    
    result_program = end_program();
    add_program_constant("Result", result_program, 0);
}
#endif
