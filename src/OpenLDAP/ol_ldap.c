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

/*
 **| file: OpenLDAP/ol_ldap.c
 **|  Implementation of the OpenLDAP glue.
 **
 **| cvs_version: $Id$
 */

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

#define THIS ((OLSTORAGE*)get_storage(Pike_fp->current_object, ldap_program))

struct program       *ldap_program;

static struct pike_string   *base_str;
static struct pike_string   *scope_str;
static struct pike_string   *filter_str;
static struct pike_string   *attrs_str;
static struct pike_string   *attrsonly_str;
static struct pike_string   *timeout_str;
static struct pike_string   *empty_str;
static struct pike_string   *modify_op;
static struct pike_string   *modify_type;
static struct pike_string   *modify_values;

#define MOD_ADD_STR          "add"
#define MOD_DELETE_STR       "delete"
#define MOD_REPLACE_STR      "replace"

static char*
type2string(int type)
{
    static char casual[128];
    
    switch(type) {
        case T_ARRAY:
            return "T_ARRAY";

        case T_MAPPING:
            return "T_MAPPING";

        case T_MULTISET:
            return "T_MULTISET";

        case T_OBJECT:
            return "T_OBJECT";

        case T_FUNCTION:
            return "T_FUNCTION";

        case T_PROGRAM:
            return "T_PROGRAM";

        case T_STRING:
            return "T_STRING";

        case T_TYPE:
            return "T_TYPE";

        case T_FLOAT:
            return "T_FLOAT";

        case T_INT:
            return "T_INT";

        case T_ZERO:
            return "T_ZERO";

        case T_TUPLE:
            return "T_TUPLE";

        case T_SCOPE:
            return "T_SCOPE";

        default:
            snprintf(casual, sizeof(casual), "T_HOW_SHOULD_I_KNOW (value == %u)",
                     type);
            return casual;
    }
}

/*
 **| method: void create ( string uri );
 **
 **|  Create a new OpenLDAP client connection object.
 **
 **| name: create - create the object
 **
 **| arg: string uri
 **|  An URI pointing to the LDAP host to connect to as described in
 **|  RFC 2255. The LDAP URI has the general format:
 **|
 **|   # ldap://hostport/dn[?attrs[?scope[?filter[?exts]]]]
 **|
 **|  where
 **|    hostport is a host name with an optional ":portnumber"
 **|    dn is the search base
 **|    attrs is a comma separated list of attributes to request
 **|    scope is one of these three strings:
 **|    base one sub (default=base)
 **|    filter is filter
 **|    exts are recognized set of LDAP and/or API extensions.
 **|
 **|  Documentation to this function has been written based on the
 **|  OpenLDAP v2 ldap_url_parse(3) manual page.
 **
 **| see_also: ldap_url_parse(3), RFC 2255
 */
static void
f_create(INT32 args)
{
    if (args != 1)
        Pike_error("OpenLDAP.Client->create():: wrong number of arguments\n");

    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        Pike_error("OpenLDAP.Client->create():: expecting an 8-bit string as the first argument (%u)\n",
                   ARG(1).u.string->size_shift);

    if ((THIS->lerrno = ldap_url_parse(ARG(1).u.string->str, &THIS->server_url)))
        Pike_error("OpenLDAP.Client->create():: badly formed server URL\n");

    if (!THIS)
        Pike_error("Serious problem - no THIS...\n");
    
    THIS->conn = ldap_init(THIS->server_url->lud_host,
                           THIS->server_url->lud_port);

    if (!THIS->conn)
        Pike_error("OpenLDAP.Client->create():: error initializing OpenLDAP: '%s'\n",
                   strerror(errno));

    pop_n_elems(args);
}

/*
 **| method: int bind();
 **|  Bind to the server in the connectionless mode (LDAP v3).
 **
 **| alt: int bind(string whodn);
 **|  Bind to the server without authentication.
 **
 **| alt: int bind(string whodn, string cred);
 **|  Bind to the server with authentication.
 **
 **| alt: int bind(string whodn, string cred, int auth_type);
 **|  Bind to the server with authentication and use the specified
 **|  authentication type.
 **
 **| general:
 **|  This is the interface to various methods of binding to the LDAP
 **|  server. An LDAP bind operation must be performed before any
 **|  other operations can be performed over the connection. There are
 **|  several authentication methods available, as described below.
 **
 **| arg: string whodn
 **|  The DN to use for authentication.
 **
 **| arg: string cred
 **|  The authenticating DN's credentials (usually a password).
 **
 **| arg: int auth_type
 **|  The authentication type to use for this call. The following
 **|  types are defined:
 **|
 **|    - OpenLDAP.LDAP_AUTH_NONE
 **|      No authentication is performed.
 **|
 **|    - OpenLDAP.LDAP_AUTH_SIMPLE
 **|      The 'cred' parameter is the DN's password as listed in the
 **|      userPassword field associated with the entry.
 **|
 **|    - OpenLDAP.LDAP_AUTH_SASL
 **|      Use the SASL identity to authenticate the user. The OpenLDAP
 **|      server must be configured properly to work with SASL.
 **|
 **|    - OpenLDAP.LDAP_AUTH_KRBV4
 **|    - OpenLDAP.LDAP_AUTH_KRBV41
 **|    - OpenLDAP.LDAP_AUTH_KRBV42
 **|      If the LDAP library and LDAP server being  contacted  have
 **|      been  compiled  with the KERBEROS option defined, Kerberos
 **|      version 4 authentication can be  accomplished  by  calling
 **|      the  ldap_kerberos_bind_s()  routine.  It assumes the user
 **|      already has obtained a ticket granting ticket.   It  takes
 **|      whodn,  the  DN  of the entry to bind as.
 **|
 **| returns: the LDAP error code
 **
 **| note: documentation written based on the ldap_bind(3) manual
 **|       page.
 **| note: this method uses the OpenLDAP synchronous interface.
 **
 **| see_also: OpenLDAP v2 ldap_bind(3) manual page.
 */
static void
f_ldap_bind(INT32 args)
{
    char    *who = "", *cred = "";
    int      auth  = LDAP_AUTH_SIMPLE, ret;

    if (!THIS->conn)
        Pike_error("Connection not opened");

    printf("binding...\n");
    
    switch (args) {
        case 3:
            if (ARG(3).type != T_INT)
                Pike_error("OpenLDAP.Client->bind(): argument 3 must be an integer\n");
            auth = ARG(3).u.integer;
            /* fall through */

        case 2:
            if (ARG(2).type != T_STRING || ARG(2).u.string->size_shift > 0)
                Pike_error("OpenLDAP.Client->bind(): argument 2 must be an 8-bit string\n");
            cred = ARG(2).u.string->str;
            /* fall through */
            
        case 1:
            if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
                Pike_error("OpenLDAP.Client->bind(): argument 1 must be an 8-bit string\n");
            who = ARG(1).u.string->str;
            break;
            
        default:
            if (args)
                Pike_error("OpenLDAP.Client->bind(): expects at most 3 arguments\n");
            break;
    }

    switch(auth) {
        case LDAP_AUTH_NONE:
            ret = ldap_bind_s(THIS->conn, "", "", LDAP_AUTH_NONE);
            break;

        case LDAP_AUTH_SIMPLE:
            ret = ldap_simple_bind_s(THIS->conn, who, cred);
            break;

        case LDAP_AUTH_SASL:
            ret = ldap_bind_s(THIS->conn, who, cred, LDAP_AUTH_SASL);
            break;

        case LDAP_AUTH_KRBV4:
        case LDAP_AUTH_KRBV41:
        case LDAP_AUTH_KRBV42:
            ret = ldap_kerberos_bind_s(THIS->conn, who);
            break;
            
        default:
            ret = ldap_bind_s(THIS->conn, who, cred, auth);
            break;
    }
    THIS->bound = (ret == LDAP_SUCCESS) ? 1 : 0;
    
    pop_n_elems(args);
    
    push_int(ret);
}

/*
 **| method: int unbind();
 **|  Unbind from the bound server.
 **
 **| returns: the LDAP error code
 **
 **| note: this method uses the OpenLDAP synchronous interface.
 **
 **| see_also: the OpenLDAP v2 ldap_unbind(3) manual page.
 */
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

/*
 **| method: int enable_cache ( );
 **| alt: int enable_cache ( int timeout );
 **| alt: int enable_cache ( int timeout, int maxmem );
 **|  The first time this method is called, it enables (creates) the
 **|  internal OpenLDAP library cache. Further calls modify the
 **|  parameters of the cache.
 **
 **| arg: int timeout
 **|  Maximum time, in secods, a given entry is kept in the cache.
 **
 **| arg: int maxmem
 **|  The size, in bytes, of the memory that can be used for cache.
 **
 **| returns: 0 for success, -1 if failed to create/modify the cache.
 */
static void
f_ldap_enable_cache(INT32 args)
{
    long timeout = OL_CACHE_TIMEOUT;
    long maxmem = OL_CACHE_MAX;
    int  ret;

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    if (THIS->caching) {
        pop_n_elems(args);
        push_int(0);
        return;
    }

    switch (args) {
        case 2:
            if (ARG(2).type != T_INT)
                Pike_error("OpenLDAP.Client->enable_cache(): argument 2 must be an integer\n");
            maxmem = ARG(2).u.integer;
            /* fall through */
            
        case 1:
            if (ARG(1).type != T_INT)
                Pike_error("OpenLDAP.Client->enable_cache(): argument 1 must be an integer\n");
            timeout = ARG(1).u.integer;
            break;
            
        default:
            if (args)
                Pike_error("OpenLDAP.Client->enable_cache(): expects at most 2 arguments\n");
            break;
    }
    
    ret = ldap_enable_cache(THIS->conn,
                            timeout,
                            maxmem);

    if (!ret)
        THIS->caching = 1;
    
    pop_n_elems(args);
    push_int(ret);
}

/*
 **| method: void disable_cache ( );
 **|  temporarily disables use of the cache
 **|  (new requests are not cached and the cache is not  checked
 **|  when  returning  results).   It  does not delete the cache
 **|  contents.
 */
static void
f_ldap_disable_cache(INT32 args)
{
    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");

    pop_n_elems(args);

    if (!THIS->caching)
        return;
    
    ldap_disable_cache(THIS->conn);
}

/*
 **| method: void destroy_cache ( );
 **|  turns  off  caching  and  completely
 **|  removes the cache from memory.
 */
static void
f_ldap_destroy_cache(INT32 args)
{
    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    pop_n_elems(args);
    
    if (!THIS->caching)
        return;
    
    ldap_destroy_cache(THIS->conn);
}

/*
 **| method: void flush_cache ( );
 **|  deletes  the  cache contents, but does
 **|  not effect it in any other way
 */
static void
f_ldap_flush_cache(INT32 args)
{
    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    pop_n_elems(args);
    if (!THIS->caching)
        return;

    ldap_flush_cache(THIS->conn);
}

/*
 **| method: void uncache_entry ( string dn );
 **|  removes all requests that make reference  to  the
 **|  distinguished  name  dn from the cache.  It
 **|  should be used, for example, after doing a modify
 **|  operation involving dn.
 **
 **| arg: string dn
 **|  The distinguished name to remove from the cache.
 */
static void
f_ldap_uncache_entry(INT32 args)
{
    char   *dn = "";

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
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

/*
 **| method: void set_cache_options ( int opts );
 **|  Set the cache options. The manual page defines two options:
 **|  LDAP_CACHE_OPT_CACHENOERRS and LDAP_CACHE_OPT_CACHEALLERRS, but
 **|  those constants are not defined in the ldap include file,
 **|  therefore they are not currently present in the OpenLDAP module.
 **
 **| arg: int opts
 **|  options to set on the cache.
 */
static void
f_ldap_set_cache_options(INT32 args)
{
    int   opts = 0; /* LDAP_CACHE_OPT_CACHENOERRS; */

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    get_all_args("OpenLDAP.Client->set_cache_options", args, "%i", &opts);

    pop_n_elems(args);
    ldap_set_cache_options(THIS->conn, opts);
}

/*
 **| method: string err2string ( int lerrno );
 **|  Converts the specified error code into the corresponding
 **|  message.
 **
 **| arg: int lerrno
 **|  LDAP error code.
 */
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

/*
 **| method: string dn2ufn ( string dn );
 **|  Convert the given DN to an user friendly form thereof. This will
 **|  strip the type names from the passed dn. See RFC 1781 for more
 **|  details. 
 **
 **| arg: string dn
 **|  an UTF-8 string with the dn to convert.
 **
 **| returns: the user friendly form of the DN.
 */
static void
f_ldap_dn2ufn(INT32 args)
{
    struct pike_string   *dn = NULL;
    char                 *ufn;
    
    if (args != 1)
        Pike_error("OpenLDAP.Client->dn2ufn(): requires exactly one 8-bit string argument\n");

    get_all_args("OpenLDAP.Client->dn2ufn()", args, "%S", &dn);
    
    pop_n_elems(args);
    
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

/*
 **| method: array(string) explode_dn ( string dn );
 **| alt: array(string) explode_dn ( string dn, int notypes );
 **|  Takes a DN and converts it into an array of its components,
 **|  called RDN (Relative Distinguished Name).
 **
 **| arg: string dn
 **|  The DN to explode.
 **
 **| arg: int notypes
 **|  If != 0 then the types of the DN components will be ignored and
 **|  *not present in the output. Defaults to 1.
 **
 **| returns: an array of RDN entries.
 */
static void
f_ldap_explode_dn(INT32 args)
{
    struct pike_string       *dn;
    char                    **edn;
    int                       notypes = 1;    

    switch (args) {
        case 2:
            if (ARG(2).type != T_INT)
                Pike_error("OpenLDAP.Client->explode_dn(): argument 2 must be an integer\n");
            notypes = ARG(2).u.integer;
            /* fall through */
            
        case 1:
            if (ARG(1).type != T_STRING)
                Pike_error("OpenLDAP.Client->explode_dn(): argument 1 must be an integer\n");
            dn = ARG(1).u.string;
            break;
            
        default:
            Pike_error("OpenLDAP.Client->explode_dn(): expects at most 2 and at least 1 argument\n");
            break;
    }

    pop_n_elems(args);
    edn = ldap_explode_dn(dn->str, notypes);
    if (!edn) {
        push_int(0);
        return;
    }

    push_array(make_pike_array(edn));
    ldap_value_free(edn);
}

/*
 **| method: string set_base_dn ( string basedn );
 **|  Set the base DN to be used in all subsequent operations over
 **|  this connection.
 **
 **| arg: string basedn
 **|  The new base DN.
 **
 **| returns: previous base DN.
 */
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
        push_string(empty_str);
    else
        push_string(olddn);
}

/*
 **| method: void set_scope ( int scope );
 **|  Set the operation scope for this connection.
 **
 **| arg: int scope
 **|  One of the following scopes: OpenLDAP.LDAP_SCOPE_BASE (search
 **|  the selected object itself only), OpenLDAP.LDAP_SCOPE_ONELEVEL
 **|  (search the object's immediate children),
 **|  OpenLDAP.LDAP_SCOPE_SUBTREE (search object and all its
 **|  descendants). The 'object' here is the base DN you set with the
 **|  set_base_dn function.
 */ 
static void
f_set_scope(INT32 args)
{
    int    scope;

    get_all_args("OpenLDAP.Client->set_scope()", args, "%i", &scope);
    pop_n_elems(args);

    THIS->scope = scope;
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

    car = (char**)calloc((arr->size + 1), sizeof(char*));
    if (!car)
        Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

    for (i = 0; i < arr->size; i++) {
        if (arr->item[i].type != T_STRING)
            Pike_error("OpenLDAP.Client: expecting a string\n");

        if (arr->item[i].u.string->size_shift > 0)
            Pike_error("OpenLDAP.Client: expecting an 8-bit string\n");

        car[i] = arr->item[i].u.string->str;
    }
    
    return car;
}

static struct berval **
make_berval_array(struct svalue *val)
{
    struct berval     **barr = NULL;
    int                 i = 0;
    size_t              size;
    
    if (!val)
        return NULL;

    if (val->type == T_STRING && val->u.string->size_shift == 0) {
        size = 2;
    } else if (val->type == T_ARRAY) {
        size = val->u.array->size + 1;
    } else {
        Pike_error("OpenDAP.Client: expecting a string or an array\n");
    }
    
    barr = (struct berval**)calloc(size, sizeof(struct berval*));
    if (!barr)
        Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

    if (val->type == T_STRING) {
        barr[0] = (struct berval*)calloc(1, sizeof(struct berval));
        if (!barr[0])
            Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");
        
        barr[0]->bv_len = val->u.string->len;
        barr[0]->bv_val = val->u.string->str;
    } else if (val->type == T_ARRAY) {
        struct array  *arr = val->u.array;
        
        for (i = 0; i < arr->size; i++) {
            if (arr->item[i].type != T_STRING)
                Pike_error("OpenLDAP.Client: expecting a string, got %s\n",
                           type2string(arr->item[i].type));

            if (arr->item[i].u.string->size_shift > 0)
                Pike_error("OpenLDAP.Client: expecting an 8-bit string\n");

            barr[i] = (struct berval*)calloc(1, sizeof(struct berval));
            if (!barr[i])
                Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

            barr[i]->bv_len = arr->item[i].u.string->len;
            barr[i]->bv_val = arr->item[i].u.string->str;
        }
    } else {
        Pike_error("OpenLDAP.Client: unsupported type for making the berval array\n");
    }
    
    return barr;
}

static void free_berval_array(struct berval** arr)
{
    struct berval   *val;
    int              i = 1;
    
    if (!arr)
        return;

    val = arr[0];
    while (val) {
        free(val);
        val = arr[i++];
    }

    free(arr);
}

static void free_mods(LDAPMod** mods)
{
    LDAPMod   *mod;
    int        i = 1;

    if (!mods)
        return;
    
    mod = mods[0];
    while (mod) {
        free_berval_array(mod->mod_bvalues);
        free(mod);
        mod = mods[i++];
    }

    free(mods);
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

/*
 **| method: object search ( mapping params );
 **|   This is the preferred way of calling this method.
 **
 **| alt: object search ( string filter );
 **| alt: object search ( string filter, array attrs );
 **| alt: object search ( string filter, array attrs, int attrsonly );
 **| alt: object search ( string filter, array attrs, int attrsonly, int timeout );
 **
 **| general: search the LDAP directory for the specified entry
 **|   (entries). The first form is the preferred one as it
 **|   encompasses all the current and future parameters as expected
 **|   by the underlying API. The following fields are read from the
 **|   mapping:@nl
 **|
 **|   @list
 **|    * base - the base DN to use in this search. Overrides the
 **|      default base DN (as set with set_base_dn)
 **|    * scope - the search scope for this search. Overrides the
 **|      default scope (as set with set_scope)
 **|    * filter - the search filter. The BNF for the filter is as
 **|      follows:
 **|      @pre
 **|        <filter> ::= `(' <filtercomp> `)'
 **|        <filtercomp> ::= <and> | <or> | <not> | <simple>
 **|        <and> ::= `&' <filterlist>
 **|        <or> ::= `|' <filterlist>
 **|        <not> ::= `!' <filter>
 **|        <filterlist> ::= <filter> | <filter> <filterlist>
 **|        <simple> ::= <attributetype> <filtertype> <attributevalue>
 **|        <filtertype> ::= `=' | `~=' | `<=' | `>='
 **|      @pre_end
 **|    * attrs - search attributes. An array of attribute types to
 **|      return in the result. If absent, all types will be returned.
 **|    * attrsonly - if != then the result will contain attribute
 **|      types only - no values shall be returned.
 **|    * timeout - the timeout, in seconds, after which the call
 **|      should return if the search isn't finished.
 **|   @list_end
 */
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

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
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
            if (ARG(2).type != T_ARRAY)
                Pike_error("OpenLDAP.Client->search(): argument 2 must be an array\n");
            else
                attrs = make_c_array(&ARG(2));

            base = THIS->basedn;
            scope = THIS->scope;
            filter = ARG(1).u.string->str;
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
    
    if (ret)
        Pike_error("OpenLDAP.Client->search(): %s\n",
                   ldap_err2string(ret));

    pop_n_elems(args);
    
    THIS->data = res;
    obj = clone_object(result_program, 0);
    push_object(obj);
    
    if (attrs)
        free(attrs);
}

/*
 * Expects the following structure to modify an entry:
 *
 * An array of mappings. Each mapping has the following fields:
 *
 *  string|int op
 *    modop is one of:
 *       "add" (LDAP_MOD_ADD),
 *       "delete" (LDAP_MOD_DELETE),
 *       "replace" (LDAP_MOD_REPLACE)
 *
 *  string type
 *    The type name of the attribute to modify (e.g. userPassword).
 *
 *  string|array(string) values
 *    Value(s) to be used in this operation.
 */
static void
f_ldap_modify(INT32 args)
{
    struct array         *arr;
    struct pike_string   *dn;
    struct mapping       *m;
    LDAPMod             **mods;
    int                   i, ret;
    struct svalue        *val;

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    get_all_args("OpenLDAP.Client->modify()", args, "%S%a", &dn, &arr);

    mods = (LDAPMod**)calloc((arr->size + 1), sizeof(LDAPMod*));
    if (!mods)
        Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

    for (i = 0; i < arr->size; i++) {
        mods[i] = (LDAPMod*)calloc(1, sizeof(LDAPMod));
        if (!mods[i])
            Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

        if (arr->item[i].type != T_MAPPING)
            Pike_error("OpenLDAP.Client->modify(): array member is not a mapping.\n");
        m = arr->item[i].u.mapping;
        
        val = low_mapping_string_lookup(m, modify_op);
        if (!val)
            Pike_error("OpenLDAP.Client->modify(): invalid modification mapping. "
                       "Missing the '%s' field\n", "op");
        
        if (val->type == T_INT) {
            mods[i]->mod_op = val->u.integer;
        } else if (val->type == T_STRING && val->u.string->size_shift == 0) {
            if (c_compare_string(val->u.string, MOD_ADD_STR, sizeof(MOD_ADD_STR)))
                mods[i]->mod_op = LDAP_MOD_ADD;
            else if (c_compare_string(val->u.string, MOD_DELETE_STR, sizeof(MOD_DELETE_STR)))
                mods[i]->mod_op = LDAP_MOD_DELETE;
            else if (c_compare_string(val->u.string, MOD_REPLACE_STR, sizeof(MOD_REPLACE_STR)))
                mods[i]->mod_op = LDAP_MOD_REPLACE;
        } else {
            Pike_error("OpenLDAP.Client->modify(): invalid 'op' value in modification mapping.\n");
        }

        mods[i]->mod_op |= LDAP_MOD_BVALUES;
        
        val = low_mapping_string_lookup(m, modify_type);
        if (!val)
            Pike_error("OpenLDAP.Client->modify(): invalid modification mapping. "
                       "Missing the '%s' field\n", "type");
        if (val->type != T_STRING || val->u.string->size_shift > 0)
            Pike_error("OpenLDAP.Client->modify(): invalid modification mapping. "
                       "The '%s' field is not an 8-bit string\n", "type");
        mods[i]->mod_type = val->u.string->str;

        val = low_mapping_string_lookup(m, modify_values);
        if (!val)
            Pike_error("OpenLDAP.Client->modify(): invalid modification mapping. "
                       "Missing the '%s' field\n", "values");
        if (val->type != T_ARRAY)
            Pike_error("OpenLDAP.Client->modify(): invalid modification mapping. "
                       "The '%s' field is not an array\n", "values");
        mods[i]->mod_bvalues = make_berval_array(val);
    }

    ret = ldap_modify_s(THIS->conn, dn->str, mods);
    if (ret != LDAP_SUCCESS)
        Pike_error("OpenLDAP.Client->modify(): %s\n",
                   ldap_err2string(ret));

    free_mods(mods);
    
    pop_n_elems(args);
}

/*
 * Takes an array of mappings, similar to modify above, with the
 * exception that the 'op' field is ignored and not used at all.
 */
static void
f_ldap_add(INT32 args)
{
    struct pike_string     *dn;
    struct array           *arr;
    struct mapping         *m;
    struct svalue          *val;
    LDAPMod               **mods;
    int                     i, ret;

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    get_all_args("OpenLDAP.Client->add()", args, "%S%a", &dn, &arr);
    mods = (LDAPMod**)calloc((arr->size + 1), sizeof(LDAPMod*));
    if (!mods)
        Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");

    for (i = 0; i < arr->size; i++) {
        mods[i] = (LDAPMod*)calloc(1, sizeof(LDAPMod));
        if (!mods[i])
            Pike_error("OpenLDAP.Client: OUT OF MEMORY!\n");
        mods[i]->mod_op = LDAP_MOD_BVALUES;

        if (arr->item[i].type != T_MAPPING)
            Pike_error("OpenLDAP.Client->add(): array member is not a mapping.\n");
        m = arr->item[i].u.mapping;
        
        val = low_mapping_string_lookup(m, modify_type);
        if (!val)
            Pike_error("OpenLDAP.Client->add(): invalid modification mapping. "
                       "Missing the '%s' field\n", "type");
        if (val->type != T_STRING || val->u.string->size_shift > 0)
            Pike_error("OpenLDAP.Client->add(): invalid modification mapping. "
                       "The '%s' field is not an 8-bit string\n", "type");
        mods[i]->mod_type = val->u.string->str;

        val = low_mapping_string_lookup(m, modify_values);
        if (!val)
            Pike_error("OpenLDAP.Client->add(): invalid modification mapping. "
                       "Missing the '%s' field\n", "values");
        if (val->type != T_ARRAY)
            Pike_error("OpenLDAP.Client->add(): invalid modification mapping. "
                       "The '%s' field is not an array\n", "values");
        
        mods[i]->mod_bvalues = make_berval_array(val);
    }
    
    ret = ldap_add_s(THIS->conn, dn->str, mods);
    if (ret != LDAP_SUCCESS)
        Pike_error("OpenLDAP.Client->add(): %s\n",
                   ldap_err2string(ret));
    
    free_mods(mods);
    
    pop_n_elems(args);
}

static void
f_ldap_delete(INT32 args)
{
    struct pike_string   *dn;
    int                   ret;

    if (!THIS->bound)
        Pike_error("OpenLDAP.Client: attempting operation on an unbound connection\n");
    
    get_all_args("OpenLDAP.Client->delete()", args, "%S", &dn);
    ret = ldap_delete_s(THIS->conn, dn->str);
    if (ret != LDAP_SUCCESS)
        Pike_error("OpenLDAP.Client->delete(): %s\n",
                   ldap_err2string(ret));

    pop_n_elems(args);
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

    modify_op = make_shared_string("op");
    add_ref(modify_op);

    modify_type = make_shared_string("type");
    add_ref(modify_type);

    modify_values = make_shared_string("values");
    add_ref(modify_values);
    
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
    free_string(modify_op);
    free_string(modify_type);
    free_string(modify_values);
}

void
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
    ADD_FUNCTION("set_basedn", f_set_base_dn,
                 tFunc(tString, tString), 0);
    ADD_FUNCTION("set_scope", f_set_scope,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("dn2ufn", f_ldap_dn2ufn,
                 tFunc(tString, tString), 0);
    ADD_FUNCTION("explode_dn", f_ldap_explode_dn,
                 tFunc(tString tOr(tInt, tVoid), tArr(tString)), 0);
    ADD_FUNCTION("search", f_ldap_search,
                 tFunc(tOr(tMapping,
                           tString tOr(tArray, tVoid) tOr(tInt, tVoid) tOr(tInt, tVoid)),
                       tOr(tObj, tInt)), 0);
    ADD_FUNCTION("modify", f_ldap_modify,
                 tFunc(tString tArr(tMap(tString, tMixed)), tVoid), 0);
    ADD_FUNCTION("add", f_ldap_add,
                 tFunc(tString tArr(tMap(tString, tMixed)), tVoid), 0);
    ADD_FUNCTION("delete", f_ldap_delete,
                 tFunc(tString, tVoid), 0);
    
    _ol_result_program_init();
    
    ldap_program = end_program();
    add_program_constant("Client", ldap_program, 0);
    add_program_constant("client", ldap_program, 0);
}
#else /* !HAVE_LIBLDAP */
void _ol_ldap_program_init(void) { }
#endif
