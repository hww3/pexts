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

/*
 *
 * Simple glue for more advanced Unix functions.
 *
 * Interface to some PAM calls.
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"

#include "at_config.h"

#ifdef HAVE_PAM

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif

#include "at_common.h"

#define THIS_LOW ((ATSTORAGE*)Pike_fp->current_storage)
#define THIS ((PAM_OBJECT_DATA*)THIS_LOW->object_data)

static char *_object_name = "PAM";
static struct program *pam_program;

typedef struct
{
  char    *oldpass;
  char    *newpass;
} MY_CONV_DATA;

typedef struct
{
    char             *appname;
    pam_handle_t     *pamh;
    struct pam_conv   conv;
    struct program   *pike_conv;
} PAM_OBJECT_DATA;

static int
chpass_conv_func(int num_msg, const struct pam_message **msg,
                 struct pam_response **resp, void *data)
{
    struct pam_response  *myresp;
    *resp = myresp = (struct pam_response*)calloc(num_msg, sizeof(struct pam_response));

    if (strstr(msg[0]->msg, "Password:") ||
        strstr(msg[0]->msg, "current")) {
        myresp->resp = (char*)strdup(((MY_CONV_DATA*)data)->oldpass);
    } else if (strstr(msg[0]->msg, "New") ||
               strstr(msg[0]->msg, "new")) {
        myresp->resp = (char*)strdup(((MY_CONV_DATA*)data)->newpass);
        myresp[1].resp = (char*)strdup(((MY_CONV_DATA*)data)->newpass);
    }

    myresp->resp_retcode = 0;

    return PAM_SUCCESS;
}

static void
f_chpass(INT32 args)
{
    char            *appname, *username, *oldpass, *newpass;
    int              ret;
    struct pam_conv  convs;
    pam_handle_t    *pamh = NULL;
    MY_CONV_DATA     convdata;
    
    get_all_args("AdminTools.PAM->chpass", args,
                 "%s%s%s%s", &appname, &username, &oldpass, &newpass);

    if (!appname || !username || !oldpass || !newpass) {
        FERROR("chpass", "All arguments must be non-empty strings.");
        pop_n_elems(args);
        push_int(-1);
        return;
    }

    pop_n_elems(args);
    
    convs.conv = chpass_conv_func;
    convs.appdata_ptr = (void*)&convdata;
    
    convdata.oldpass = oldpass;
    convdata.newpass = newpass;

    if (pam_start(appname, username, &convs, &pamh) != PAM_SUCCESS) {
        push_int(-2);
        return;
    }

    ret = pam_authenticate(pamh, PAM_UPDATE_AUTHTOK);

    switch(ret) {
        case PAM_USER_UNKNOWN:
            push_int(-3);
            return;

        default:
            push_int(-4);
            return;
    }

    if (pam_chauthtok(pamh, PAM_SILENT) != PAM_SUCCESS) {
        push_int(-5);
        return;
    }

    if (pam_end(pamh, PAM_SUCCESS) != PAM_SUCCESS) {
        push_int(-6);
        return;
    }

    push_int(0);
}

/****
 *
 * Low-level PAM interface
 *
 ****/
static int
pike_glue_conv(int num_msg, const struct pam_message **msg,
               struct pam_response **resp, void *data)
{
    return PAM_SUCCESS;
}

static void
f_pam_start(INT32 args)
{
    char            *username;
    
    get_all_args("AdminTools.PAM->start", args,
                 "%s%p", &username, THIS->pike_conv);
    THIS->conv.conv = pike_glue_conv;
    THIS->conv.appdata_ptr = THIS;
}

static void
f_pam_end(INT32 args)
{}

static void
f_pam_set_item(INT32 args)
{}

static void
f_pam_get_item(INT32 args)
{}

static void
f_pam_strerror(INT32 args)
{}

static void
f_pam_fail_delay(INT32 args)
{}

static void
f_pam_authenticate(INT32 args)
{}

static void
f_pam_setcred(INT32 args)
{}

static void
f_pam_acct_mgmt(INT32 args)
{}

static void
f_pam_chauthtok(INT32 args)
{}

static void
f_pam_open_session(INT32 args)
{}

static void
f_pam_close_session(INT32 args)
{}

static void
f_pam_putenv(INT32 args)
{}

static void
f_pam_getenv(INT32 args)
{}

static void
f_pam_getenvlist(INT32 args)
{}


static void
f_create(INT32 args)
{
    get_all_args("AdminTools.PAM->create", args,
                 "%s", &THIS->appname);
    pop_n_elems(args);
}

static void
init_pam(struct object *o)
{
    PAM_OBJECT_DATA   *dta;

    dta = (PAM_OBJECT_DATA*)malloc(sizeof(PAM_OBJECT_DATA));
    if (!dta)
        FERROR("init_pam", "Out of memory!");

    dta->appname = NULL;
    dta->pamh = NULL;
    dta->pike_conv = NULL;
    THIS_LOW->object_name = _object_name;
    THIS_LOW->object_data = dta;
}

static void
exit_pam(struct object *o)
{}

struct program*
_at_pam_init(void)
{
    start_new_program();
    ADD_STORAGE(ATSTORAGE);

    set_init_callback(init_pam);
    set_exit_callback(exit_pam);

    ADD_FUNCTION("create", f_create, 
                 tFunc(tString, tVoid), 0);
    ADD_FUNCTION("chpass", f_chpass,
                 tFunc(tString tString tString tString, tInt), 0);
    ADD_FUNCTION("start", f_pam_start,
                 tFunc(tString tFunction, tInt), 0);
    
    pam_program = end_program();
    add_program_constant("PAM", pam_program, 0);
    
    return pam_program;
}
#endif /* HAVE_PAM */ 
