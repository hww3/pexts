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

#define _GNU_SOURCE

#define MODULE_MAJOR 0
#define MODULE_MINOR 1
#define MODULE_BUILD 1

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "ol_config.h"
#include "ol_common.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_LIBLDAP

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
    pexts_init();
#endif    

    /* LDAP constants */
    /* AUTH stuff */
    add_integer_constant("LDAP_AUTH_NONE", LDAP_AUTH_NONE, 0);
    add_integer_constant("LDAP_AUTH_SIMPLE", LDAP_AUTH_SIMPLE, 0);
    add_integer_constant("LDAP_AUTH_SASL", LDAP_AUTH_SASL, 0);
    add_integer_constant("LDAP_AUTH_KRBV4", LDAP_AUTH_KRBV4, 0);
    add_integer_constant("LDAP_AUTH_KRBV41", LDAP_AUTH_KRBV41, 0);
    add_integer_constant("LDAP_AUTH_KRBV42", LDAP_AUTH_KRBV42, 0);

    /* Cache stuff
    add_integer_constant("LDAP_CACHE_OPT_CACHENOERRS", LDAP_CACHE_OPT_CACHENOERRS, 0);
    add_integer_constant("LDAP_CACHE_OPT_CACHEALLERRS", LDAP_CACHE_OPT_CACHEALLERRS, 0);
    */
    
    /* Scope stuff */
    add_integer_constant("LDAP_SCOPE_DEFAULT", LDAP_SCOPE_DEFAULT, 0);
    add_integer_constant("LDAP_SCOPE_BASE", LDAP_SCOPE_BASE, 0);
    add_integer_constant("LDAP_SCOPE_ONELEVEL", LDAP_SCOPE_ONELEVEL, 0);
    add_integer_constant("LDAP_SCOPE_SUBTREE", LDAP_SCOPE_SUBTREE, 0);

    /* modification ops */
    add_integer_constant("LDAP_MOD_ADD", LDAP_MOD_ADD, 0);
    add_integer_constant("LDAP_MOD_DELETE", LDAP_MOD_DELETE, 0);
    add_integer_constant("LDAP_MOD_REPLACE", LDAP_MOD_REPLACE, 0);

    /* error constants */
    add_integer_constant("LDAP_SUCCESS", LDAP_SUCCESS, 0);
    add_integer_constant("LDAP_OPERATIONS_ERROR", LDAP_OPERATIONS_ERROR, 0);
    add_integer_constant("LDAP_PROTOCOL_ERROR", LDAP_PROTOCOL_ERROR, 0);
    add_integer_constant("LDAP_TIMELIMIT_EXCEEDED", LDAP_TIMELIMIT_EXCEEDED, 0);
    add_integer_constant("LDAP_SIZELIMIT_EXCEEDED", LDAP_SIZELIMIT_EXCEEDED, 0);
    add_integer_constant("LDAP_COMPARE_FALSE", LDAP_COMPARE_FALSE, 0);
    add_integer_constant("LDAP_COMPARE_TRUE", LDAP_COMPARE_TRUE, 0);
    add_integer_constant("LDAP_AUTH_METHOD_NOT_SUPPORTED", LDAP_AUTH_METHOD_NOT_SUPPORTED, 0);
    add_integer_constant("LDAP_STRONG_AUTH_NOT_SUPPORTED", LDAP_STRONG_AUTH_NOT_SUPPORTED, 0);
    add_integer_constant("LDAP_STRONG_AUTH_REQUIRED", LDAP_STRONG_AUTH_REQUIRED, 0);
    add_integer_constant("LDAP_PARTIAL_RESULTS", LDAP_PARTIAL_RESULTS, 0);
    add_integer_constant("LDAP_REFERRAL", LDAP_REFERRAL, 0);
    add_integer_constant("LDAP_ADMINLIMIT_EXCEEDED", LDAP_ADMINLIMIT_EXCEEDED, 0);
    add_integer_constant("LDAP_UNAVAILABLE_CRITICAL_EXTENSION", LDAP_UNAVAILABLE_CRITICAL_EXTENSION, 0);
    add_integer_constant("LDAP_CONFIDENTIALITY_REQUIRED", LDAP_CONFIDENTIALITY_REQUIRED, 0);
    add_integer_constant("LDAP_SASL_BIND_IN_PROGRESS", LDAP_SASL_BIND_IN_PROGRESS, 0);
    add_integer_constant("LDAP_NO_SUCH_ATTRIBUTE", LDAP_NO_SUCH_ATTRIBUTE, 0);
    add_integer_constant("LDAP_UNDEFINED_TYPE", LDAP_UNDEFINED_TYPE, 0);
    add_integer_constant("LDAP_INAPPROPRIATE_MATCHING", LDAP_INAPPROPRIATE_MATCHING, 0);
    add_integer_constant("LDAP_CONSTRAINT_VIOLATION", LDAP_CONSTRAINT_VIOLATION, 0);
    add_integer_constant("LDAP_TYPE_OR_VALUE_EXISTS", LDAP_TYPE_OR_VALUE_EXISTS, 0);
    add_integer_constant("LDAP_INVALID_SYNTAX", LDAP_INVALID_SYNTAX, 0);
    add_integer_constant("LDAP_NO_SUCH_OBJECT", LDAP_NO_SUCH_OBJECT, 0);
    add_integer_constant("LDAP_ALIAS_PROBLEM", LDAP_ALIAS_PROBLEM, 0);
    add_integer_constant("LDAP_INVALID_DN_SYNTAX", LDAP_INVALID_DN_SYNTAX, 0);
    add_integer_constant("LDAP_IS_LEAF", LDAP_IS_LEAF, 0);
    add_integer_constant("LDAP_ALIAS_DEREF_PROBLEM", LDAP_ALIAS_DEREF_PROBLEM, 0);
    add_integer_constant("LDAP_INAPPROPRIATE_AUTH", LDAP_INAPPROPRIATE_AUTH, 0);
    add_integer_constant("LDAP_INVALID_CREDENTIALS", LDAP_INVALID_CREDENTIALS, 0);
    add_integer_constant("LDAP_INSUFFICIENT_ACCESS", LDAP_INSUFFICIENT_ACCESS, 0);
    add_integer_constant("LDAP_BUSY", LDAP_BUSY, 0);
    add_integer_constant("LDAP_UNAVAILABLE", LDAP_UNAVAILABLE, 0);
    add_integer_constant("LDAP_UNWILLING_TO_PERFORM", LDAP_UNWILLING_TO_PERFORM, 0);
    add_integer_constant("LDAP_LOOP_DETECT", LDAP_LOOP_DETECT, 0);
    add_integer_constant("LDAP_NAMING_VIOLATION", LDAP_NAMING_VIOLATION, 0);
    add_integer_constant("LDAP_OBJECT_CLASS_VIOLATION", LDAP_OBJECT_CLASS_VIOLATION, 0);
    add_integer_constant("LDAP_NOT_ALLOWED_ON_NONLEAF", LDAP_NOT_ALLOWED_ON_NONLEAF, 0);
    add_integer_constant("LDAP_NOT_ALLOWED_ON_RDN", LDAP_NOT_ALLOWED_ON_RDN, 0);
    add_integer_constant("LDAP_ALREADY_EXISTS", LDAP_ALREADY_EXISTS, 0);
    add_integer_constant("LDAP_NO_OBJECT_CLASS_MODS", LDAP_NO_OBJECT_CLASS_MODS, 0);
    add_integer_constant("LDAP_RESULTS_TOO_LARGE", LDAP_RESULTS_TOO_LARGE, 0);
    add_integer_constant("LDAP_AFFECTS_MULTIPLE_DSAS", LDAP_AFFECTS_MULTIPLE_DSAS, 0);
    add_integer_constant("LDAP_OTHER", LDAP_OTHER, 0);
    add_integer_constant("LDAP_SERVER_DOWN", LDAP_SERVER_DOWN, 0);
    add_integer_constant("LDAP_LOCAL_ERROR", LDAP_LOCAL_ERROR, 0);
    add_integer_constant("LDAP_ENCODING_ERROR", LDAP_ENCODING_ERROR, 0);
    add_integer_constant("LDAP_DECODING_ERROR", LDAP_DECODING_ERROR, 0);
    add_integer_constant("LDAP_TIMEOUT", LDAP_TIMEOUT, 0);
    add_integer_constant("LDAP_AUTH_UNKNOWN", LDAP_AUTH_UNKNOWN, 0);
    add_integer_constant("LDAP_FILTER_ERROR", LDAP_FILTER_ERROR, 0);
    add_integer_constant("LDAP_USER_CANCELLED", LDAP_USER_CANCELLED, 0);
    add_integer_constant("LDAP_PARAM_ERROR", LDAP_PARAM_ERROR, 0);
    add_integer_constant("LDAP_NO_MEMORY", LDAP_NO_MEMORY, 0);
    add_integer_constant("LDAP_CONNECT_ERROR", LDAP_CONNECT_ERROR, 0);
    add_integer_constant("LDAP_NOT_SUPPORTED", LDAP_NOT_SUPPORTED, 0);
    add_integer_constant("LDAP_CONTROL_NOT_FOUND", LDAP_CONTROL_NOT_FOUND, 0);
    add_integer_constant("LDAP_NO_RESULTS_RETURNED", LDAP_NO_RESULTS_RETURNED, 0);
    add_integer_constant("LDAP_MORE_RESULTS_TO_RETURN", LDAP_MORE_RESULTS_TO_RETURN, 0);
    add_integer_constant("LDAP_CLIENT_LOOP", LDAP_CLIENT_LOOP, 0);
    add_integer_constant("LDAP_REFERRAL_LIMIT_EXCEEDED", LDAP_REFERRAL_LIMIT_EXCEEDED, 0);
    
    _ol_ldap_program_init();
}

void pike_module_exit(void)
{
  free_program(ldap_program);
  free_program(result_program);
}
#else /* !HAVE_LIBLDAP */
void pike_module_init()
{
#ifdef PEXTS_VERSION
    pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif
