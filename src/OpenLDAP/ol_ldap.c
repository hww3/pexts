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

#ifdef HAVE_UNISTD
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <time.h>

#define THIS ((OLSTORAGE*)get_storage(fp->current_object, ldap_program))

static struct program       *ldap_program;
static struct program       *result_program;

static struct pike_string   *base_str;
static struct pike_string   *scope_str;
static struct pike_string   *filter_str;
static struct pike_string   *attrs_str;
static struct pike_string   *attrsonly_str;
static struct pike_string   *timeout_str;
static struct pike_string   *empty_str;

static void
f_create(INT32 args)
{
    if (args != 1)
        Pike_error("OpenLDAP.Client->create():: wrong number of arguments\n");

    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        Pike_error("OpenLDAP.Client->create():: expecting an 8-bit string as the first argument\n");

    if ((THIS->lerrno = ldap_url_parse(ARG(1).u.string->str, &THIS->server_url)))
        Pike_error("OpenLDAP.Client->create():: badly formed server URL\n");

    THIS->conn = ldap_init(THIS->server_url->lud_host,
                           THIS->server_url->lud_port);

    if (!THIS->conn)
        Pike_error("OpenLDAP.Client->create():: error initializing OpenLDAP: '%s'\n",
                   strerror(errno));

    pop_n_elems(args);
}

static void
f_ldap_bind(INT32 args)
{
    char    *who = "", *cred = "";
    int      auth = LDAP_AUTH_SIMPLE, ret;

    get_all_args("OpenLDAP.Client->bind()", args, "%s%s%i",
                   &who, &cred, &auth);

    ret = ldap_bind_s(THIS->conn, who, cred, auth);
    THIS->bound = ret ? 0 : 1;
    
    pop_n_elems(args);
    
    push_int(ret);
}

static void
f_ldap_unbind(INT32 args)
{
    if (THIS->bound) {
        THIS->lerrno = ldap_unbind_s(THIS->conn);
        THIS->bound = 0;
    } else
        THIS->lerrno = 0;
    
    pop_n_elems(args);

    push_int(THIS->lerrno);
}

static void
f_ldap_enable_cache(INT32 args)
{
    long timeout = OL_CACHE_TIMEOUT;
    long maxmem = OL_CACHE_MAX;
    int  ret;
    
    if (THIS->caching) {
        pop_n_elems(args);
        push_int(0);
        return;
    }

    get_all_args("OpenLDAP.Client->enable_cache()", args, "%i%i",
                 &timeout, &maxmem);
    
    ret = ldap_enable_cache(THIS->conn,
                            timeout,
                            maxmem);

    if (!ret)
        THIS->caching = 1;
    
    pop_n_elems(args);
    push_int(ret);
}

static void
f_ldap_disable_cache(INT32 args)
{
    pop_n_elems(args);
    
    if (!THIS->caching)
        return;
    
    ldap_disable_cache(THIS->conn);
}

static void
f_ldap_destroy_cache(INT32 args)
{
    pop_n_elems(args);    
    if (!THIS->caching)
        return;
    
    ldap_destroy_cache(THIS->conn);
}

static void
f_ldap_flush_cache(INT32 args)
{
    pop_n_elems(args);
    if (!THIS->caching)
        return;

    ldap_flush_cache(THIS->conn);
}

static void
f_ldap_uncache_entry(INT32 args)
{
    char   *dn = "";
    
    if (!THIS->caching) {
        pop_n_elems(args);
        return;
    }

    if (args != 1)
        Pike_error("OpenLDAP.Client->uncache_entry() requires a single 8-bit string argument\n");

    get_all_args("OpenLDAP.Client->uncache_entry()", args, "%s", &dn);

    ldap_uncache_entry(THIS->conn, dn);
    pop_n_elems(args);
}

static void
f_ldap_set_cache_options(INT32 args)
{
    int   opts = 0; /* LDAP_CACHE_OPT_CACHENOERRS; */

    get_all_args("OpenLDAP.Client->set_cache_options", args, "%i", &opts);

    pop_n_elems(args);
    ldap_set_cache_options(THIS->conn, opts);
}

static void
f_ldap_err2string(INT32 args)
{
    int    err;
    char  *str;
    
    if (args != 1)
        Pike_error("OpenLDAP.Client->err2string() requires a single integer argument\n");

    get_all_args("OpenLDAP.Client->err2string()", args, "%i", &err);

    pop_n_elems(args);
    
    str = ldap_err2string(err);
    push_string(make_shared_string(str));
}

static void
f_set_base_dn(INT32 args)
{
    struct pike_string  *olddn;
    
    if (args != 1)
        Pike_error("OpenLDAP.Client->set_base_dn() requires a single 8-bit string parameter\n");

    olddn = THIS->basedn;
    get_all_args("OpenLDAP.Client->set_base_dn()", args, "%S", &THIS->basedn);
    pop_n_elems(args);

    if (!olddn)
        push_string(make_shared_string(""));
    else
        push_string(olddn);
}

static void
f_set_scope(INT32 args)
{
    int    scope = -1;

    get_all_args("OpenLDAP.Client->set_scope()", args, "%i", &THIS->scope);
    pop_n_elems(args);

    THIS->scope ^= scope;
    scope ^= THIS->scope;
    THIS->scope ^= scope;

    push_int(scope);
}

static char**
make_c_array(struct svalue *val)
{
    char          **car;
    int             i;
    struct svalue  *sv;
    struct array   *arr = val->u.array;
    
    if (!arr || !arr->size)
        return NULL;

    car = (char**)malloc((arr->size + 1) * sizeof(char*));
    memset(car, 0, arr->size);
    
    if (!arr)
        Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

    for (i = 0; i < arr->size; i++) {
        if (arr->item[i].type != T_STRING)
            continue;

        if (arr->item[i].u.string->size_shift > 0)
            continue;

        car[i] = arr->item[i].u.string->str;
    }
    
    return car;
}

static struct timeval *
make_timeout(long val)
{
    struct timeval   *tv;

    tv = (struct timeval*)malloc(sizeof(struct timeval));
    if (!tv)
        Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

    tv->tv_sec = val;
    tv->tv_usec = 0;
    
    return tv;
}

static void
f_ldap_search(INT32 args)
{
    char                *filter;
    int                  scope;
    struct pike_string  *base;
    char               **attrs;
    int                  attrsonly;
    struct timeval      *timeout;
    LDAPMessage         *res;
    int                  ret;
    struct object       *obj;
    
    if (!args)
        Pike_error("OpenLDAP.Client->search() requires at least one argument\n");

    if (args == 1) {
        switch(ARG(1).type) {
            case T_MAPPING:
            {
                /* the new style case */
                struct svalue  *val;
                struct mapping *m = ARG(1).u.mapping;

                /*
                 * m->base
                 */
                val = low_mapping_string_lookup(m,
                                                base_str);
                if (!val || val->type != T_STRING)
                    base = THIS->basedn;
                else
                    base = val->u.string;

                /*
                 * m->scope
                 */
                val = low_mapping_string_lookup(m,
                                                scope_str);
                if (!val || val->type != T_INT)
                    scope = THIS->scope;
                else
                    scope = val->u.integer;

                /*
                 * m->filter
                 */
                val = low_mapping_string_lookup(m,
                                                filter_str);
                if (!val || val->type != T_STRING)
                    filter = OL_DEF_FILTER;
                else
                    filter = val->u.string->str;

                /*
                 * m->attrs
                 */
                val = low_mapping_string_lookup(m,
                                                attrs_str);
                if (!val || val->type != T_ARRAY)
                    attrs = NULL;
                else
                    attrs = make_c_array(val);

                /*
                 * m->attrsonly
                 */
                val = low_mapping_string_lookup(m,
                                                attrsonly_str);
                if (!val || val->type != T_INT)
                    attrsonly = 0;
                else
                    attrsonly = val->u.integer != 0;

                /*
                 * m->timeout
                 */
                val = low_mapping_string_lookup(m,
                                                timeout_str);
                if (!val || val->type != T_INT)
                    timeout = NULL;
                else
                    timeout = make_timeout(val->u.integer); /*TEMPORARY*/
                
                break;
            }

            case T_STRING:
                /* the old style case */
                base = THIS->basedn;
                scope = THIS->scope;
                filter = ARG(1).u.string->str;
                attrs = NULL;
                attrsonly = 0;
                timeout = NULL;
                break;

            default:
                Pike_error("OpenLDAP.Client->search() with single argument requires either a mapping or a string\n");
                break;
        }
    } else switch(args) {
        case 4: /* timeout */
            if (ARG(4).type != T_INT)
                Pike_error("OpenLDAP.Client->search(): argument 4 must be an integer\n");
            else
                timeout = make_timeout(ARG(4).u.integer);
            /* fall through */

        case 3: /* attrsonly */
            if (ARG(3).type != T_INT)
                Pike_error("OpenLDAP.Client->search(): argument 3 must be an integer\n");
            else
                attrsonly = ARG(3).u.integer != 0;
            /* fall through */

        case 2: /* attrs */
            if (ARG(2).type != T_INT)
                Pike_error("OpenLDAP.Client->search(): argument 2 must be an array\n");
            else
                attrs = make_c_array(&ARG(3));

            base = THIS->basedn;
            scope = THIS->scope;
            filter = ARG(1).u.string->str;
            attrs = NULL;
            attrsonly = 0;
            timeout = NULL;
            break;

        default:
            Pike_error("OpenLDAP.Client->search(): incorrect number of arguments\n");
            break;
    }

    ret = timeout ?
        ldap_search_st(THIS->conn,
                       base->str,
                       scope,
                       filter,
                       attrs,
                       attrsonly,
                       timeout,
                       &res) :
        ldap_search_s(THIS->conn,
                      base->str,
                      scope,
                      filter,
                      attrs,
                      attrsonly,
                      &res);
        
    pop_n_elems(args);
    if (ret) {
        push_int(ret);
        return;
    }
    
    obj = clone_program(result_program, 2);
    push_object(obj);

    if (attrs)
        free(attrs);
}

static void
init_ldap(struct object *o)
{
    base_str = make_shared_string("base");
    add_ref(base_str);

    scope_str = make_shared_string("scope");
    add_ref(scope_str);

    filter_str = make_shared_string("filter");
    add_ref(filter_str);

    attrs_str = make_shared_string("attrs");
    add_ref(attrs_str);

    attrsonly_str = make_shared_string("attrsonly");
    add_ref(attrsonly_str);

    timeout_str = make_shared_string("timeout");
    add_ref(timeout_str);

    empty_str = make_shared_string("");
    add_ref(empty_str);
    
    THIS->conn = NULL;
    THIS->server_url = NULL;
    THIS->basedn = empty_str;
    THIS->scope = LDAP_SCOPE_DEFAULT;
    THIS->bound = 0;
    THIS->lerrno = 0;
    THIS->caching = 0;
}

static void
exit_ldap(struct object *o)
{
    if (THIS->server_url) {
        ldap_free_urldesc(THIS->server_url);
        THIS->server_url = NULL;
    }

    free_string(base_str);
    free_string(scope_str);
    free_string(filter_str);
    free_string(attrs_str);
    free_string(attrsonly_str);
    free_string(timeout_str);
    free_string(empty_str);
}

struct program*
_ol_ldap_program_init(void)
{
    start_new_program();
    ADD_STORAGE(OLSTORAGE);

    set_init_callback(init_ldap);
    set_exit_callback(exit_ldap);
    
    ADD_FUNCTION("create", f_create,
                 tFunc(tString, tVoid), 0);
    ADD_FUNCTION("bind", f_ldap_bind,
                 tFunc(tOr(tString, tVoid) tOr(tString, tVoid) tOr(tInt, tVoid),
                       tInt), 0);
    ADD_FUNCTION("unbind", f_ldap_unbind,
                 tFunc(tVoid, tInt), 0);

    ADD_FUNCTION("enable_cache", f_ldap_enable_cache,
                 tFunc(tOr(tInt, tVoid) tOr(tInt, tVoid), tInt), 0);
    ADD_FUNCTION("disable_cache", f_ldap_disable_cache,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("destroy_cache", f_ldap_destroy_cache,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("flush_cache", f_ldap_flush_cache,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("uncache_entry", f_ldap_uncache_entry,
                 tFunc(tString, tVoid), 0);
    ADD_FUNCTION("set_cache_options", f_ldap_set_cache_options,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("err2string", f_ldap_err2string,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("set_base_dn", f_set_base_dn,
                 tFunc(tString, tString), 0);
    ADD_FUNCTION("set_scope", f_set_scope,
                 tFunc(tInt, tInt), 0);
    ADD_FUNCTION("search", f_ldap_search,
                 tFunc(tOr(tMapping,
                           tString tOr(tArray, tVoid) tOr(tInt, tVoid) tOr(tInt, tVoid)),
                       tOr(tObj, tInt)), 0);

    result_program = _ol_result_program_init();
    
    ldap_program = end_program();
    add_program_constant("Client", ldap_program, 0);
    
    return ldap_program;
}
#else /* !HAVE_LIBLDAP */
struct program*
_ol_ldap_program_init(void)
{
    return NULL;
}
#endif
