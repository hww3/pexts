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

#define MODULE_MAJOR 0
#define MODULE_MINOR 1
#define MODULE_BUILD 1

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "ol_config.h"
#include "ol_common.h"

#ifdef HAVE_UNISTD
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>

#define THIS ((OLSTORAGE*)get_storage(fp->current_object, ldap_program))

static struct program *ldap_program;

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
        THIS->lerror = ldap_unbind_s(THIS->conn);
        THIS->bound = 0;
    } else
        THIS->lerror = 0;
    
    pop_n_elems(args);

    push_int(THIS->lerror);
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

    if (!args || args > 1)
        Pike_error("OpenLDAP.Client->uncache_entry() requires a single string argument\n");

    get_all_args("OpenLDAP.Client->uncache_entry()", args, "%s", &dn);

    ldap_uncache_entry(THIS->conn, dn);
    pop_n_elems(args);
}

static void
f_ldap_set_cache_options(INT32 args)
{
    int   opts = LDAP_CACHE_OPT_CACHENOERRS;

    get_all_args("OpenLDAP.Client->set_cache_options", args, "%i", &opts);

    pop_n_elems(args);
    ldap_set_cache_options(THIS->conn, opts);
}

static void
init_ldap(struct object *o)
{
    THIS->conn = NULL;
    THIS->server_url = NULL;
    THIS->bound = 0;
    THIS->lerror = 0;
    THIS->caching = 0;
}

static void
exit_ldap(struct object *o)
{
    if (THIS->server_url) {
        ldap_free_urldesc(THIS->server_url);
        THIS->server_url = NULL;
    }
}

struct program*
_ol_ldap_program_init(void)
{
    start_new_program();
    ADD_STORAGE(OLSTORAGE);

    set_init_callback(init_ldap);
    set_exit_callback(exit_ldap);

    /* LDAP constants */
    add_integer_constant("LDAP_AUTH_NONE", LDAP_AUTH_NONE, 0);
    add_integer_constant("LDAP_AUTH_SIMPLE", LDAP_AUTH_SIMPLE, 0);
    add_integer_constant("LDAP_AUTH_SASL", LDAP_AUTH_SASL, 0);
    add_integer_constant("LDAP_AUTH_KRBV4", LDAP_AUTH_KRBV4, 0);
    add_integer_constant("LDAP_AUTH_KRBV41", LDAP_AUTH_KRBV41, 0);
    add_integer_constant("LDAP_AUTH_KRBV42", LDAP_AUTH_KRBV42, 0);
    add_integer_constant("LDAP_CACHE_OPT_CACHENOERRS", LDAP_CACHE_OPT_CACHENOERRS, 0);
    add_integer_constant("LDAP_CACHE_OPT_CACHEALLERRS", LDAP_CACHE_OPT_CACHEALLERRS, 0);
    
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
    
    ldap_program = end_program();
    add_program_constant("Client", ldap_program, 0);
    
    return ldap_program;
}
