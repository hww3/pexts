/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000 The Caudium Group
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
 *
 * Interface to some PAM calls.
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "pexts.h"

#include "at_config.h"

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

#define THIS_LOW ((ATSTORAGE*)get_storage(fp->current_object, pam_program))
#define THIS ((void*)THIS_LOW->object_data)

static char *_object_name = "PAM";
static struct program *pam_program;

typedef struct
{
  char    *oldpass;
  char    *newpass;
} MY_CONV_DATA;

static int
chpass_conv_func(int num_msg, const struct pam_message **msg,
                 struct pam_response **resp, void *data)
{
    struct pam_response  *myresp;
    *resp = myresp = (struct pam_response*)calloc(num_msg, sizeof(struct pam_response));

    if (strstr(msg[0]->msg, "Password:") ||
        strstr(msg[0]->msg, "current")) {
        myresp->resp = (char*)strdup(((MY_CONV_DATA*)data)->oldpass);
    } else if (strstr(msg[0]->msg, "New") || strstr(msg[0]->msg, "new")) {
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

static void
f_create(INT32 args)
{}

static void
init_pam(struct object *o)
{
    THIS_LOW->object_name = _object_name;
    THIS_LOW->object_data = NULL;
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
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("chpass", f_chpass,
                 tFunc(tString tString tString tString, tInt), 0);
    
    pam_program = end_program();
    add_program_constant("PAM", pam_program, 0);
    
    return pam_program;
}
