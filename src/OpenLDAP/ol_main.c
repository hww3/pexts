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
static struct program   *ldap_program;

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
    
    ldap_program = _ol_ldap_program_init();
}

void pike_module_exit(void)
{
  free_program(ldap_program);
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
