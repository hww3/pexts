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
 * Simple glue for more advanced Unix functions.
 */
#define _GNU_SOURCE
#define _POSIX_PTHREAD_SEMANTICS

#define MODULE_MAJOR 0
#define MODULE_MINOR 1
#define MODULE_BUILD 1

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"

#include "at_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "at_common.h"

/*
 * This module contains glue for the various Unix administrative calls
 * not supported by the mainstream Pike modules. Currently supported
 * calls are:
 *
 * *** Shadow passwords support ***
 *
 *  PORTABILITY: Linux
 *
 *  Shadow passwords are used by Pike, but only to get the shadow (see
 *  modules/system/passwords.c) password. I want more :). Note that
 *  this code currently supports only Linux shadow passwords. Where
 *  possible, the module uses thread-safe versions of the library
 *  calls.
 *
 *  void setspent(void);
 *    Open the shadow database for reading.
 *
 *  void endspent(void);
 *    Close the shadow database after reading.
 *
 *  int|array getspent(void);
 *    Return a mapping filled with the next shadow entry read from the
 *    shadow database. If this function invocation wasn't preceeded
 *    with a call to setspent() the database will be opened by this
 *    function. The caller must take care to call endspent() after all
 *    the required entries have been read.
 *    Returns:
 *
 *      0 if error ocurred,
 *      array of data otherwise:
 *        [0] - login name
 *        [1] - encrypted password
 *        [2] - date of last change
 *        [3] - min. nr of days between passwd changes
 *        [4] - max. nr of days between passwd changes
 *        [5] - nr of days to warn the user b4 the passwd expires
 *        [6] - nr of days the account may be inactive
 *        [7] - nr of days since the Epoch until the account expires
 *
 *  int|array(array) getallspents(void);
 *    Return an array of arrays as defined in getspent() containing all
 *    the accounts found in the /etc/shadow database. Returns 0 if it is
 *    impossible to retrieve the data.
 *
 *
 * *** QUOTACTL support ***
 */

DEFINE_IMUTEX(at_shadow_mutex);

static struct program   *shadow_program;
static struct program   *dir_program;
static struct program   *quota_program;
static struct program   *system_program;
#ifdef HAVE_PAM
static struct program   *pam_program;
#endif

void pike_module_init(void)
{
    init_interleave_mutex(&at_shadow_mutex);

    init_common("AdminTools");

  #ifdef PEXTS_VERSION
  pexts_init();
#endif
    
    /* Shadow stuff */
    shadow_program = _at_shadow_init();
    
    /* Dir stuff */
    dir_program = _at_directory_init();
    
    /* Quota stuff */
    quota_program = _at_quota_init();
    
    /* System stuff */
    system_program = _at_system_init();

    /* PAM stuff */
#ifdef HAVE_PAM
    pam_program = _at_pam_init();
#endif
}

void pike_module_exit(void)
{
  free_program(shadow_program);
  free_program(dir_program);
  free_program(quota_program);
#ifdef HAVE_PAM
  free_program(pam_program);
#endif
}
