/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
 */

/* Copyright (C) 2002 The Caudium Group
 * Copyright (C) 2002 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The Esmtp Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The Esmtp Pexts Module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Pike Extensions; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA. 
 *
 * $Id$
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "esmtp_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_LIBESMTP
#include <libesmtp.h>

typedef struct 
{
    smtp_session_t   *sessions;
    unsigned int      nsessions;
    unsigned int      sessionsize;
    smtp_message_t    message;
    smtp_recipient_t *recipients;
    unsigned int      nrecipients;
    unsigned int      recipientsize;
    int               error;
} ESMTP_STORAGE;

#define TABLE_GROWTH_FACTOR 1
#define THIS ((ESMTP_STORAGE*)get_storage(Pike_fp->current_object, esmtp_program))

static struct program   *esmtp_program;

/* Glue */
static void
f_smtp_version(INT32 args)
{
    char   buf[256];
    
    if (args != 0)
        generic_error("version", Pike_sp-args, args,
                      "Too many arguments. None required.\n");

    if (!smtp_version(buf, sizeof(buf), 0))
        generic_error("version", Pike_sp, args,
                      "libesmtp: 'smtp_version' returned 'buffer too small' - "
                      "please report it to grendel@caudium.net\n");

    pop_n_elems(args);
    push_string(make_shared_string(buf));
}

static void
f_smtp_errno(INT32 args)
{
    if (args != 0)
        generic_error("errno", Pike_sp-args, args,
                      "Too many arguments. None required.\n");

    pop_n_elems(args);
    push_int(smtp_errno());
}

static void
f_smtp_strerror(INT32 args)
{
    int    err;
    char   buf[512], *str;
    
    if (args != 1)
        generic_error("strerror", Pike_sp-args, args,
                      "Incorrect number of parameters - required 1.\n");

    get_all_args("strerror", args, "%i", &err);
    pop_n_elems(args);
    
    str = smtp_strerror(err, buf, sizeof(buf));
    if (!str)
        push_string(make_shared_string("Unknown error code."));
    else
        push_string(make_shared_string(str));
}

static void
f_smtp_create_session(INT32 args)
{
    if (args != 0)
        generic_error("create_session", Pike_sp-args, args,
                      "Too many arguments. None required.\n");

    if (!THIS->sessions) {
        THIS->nsessions = 0;
        THIS->sessionsize = 1;
        THIS->sessions = (smtp_session_t*)malloc(sizeof(smtp_session_t));
        if (!THIS->sessions)
            generic_error("create_session", Pike_sp-args, args,
                          "Failed to allocate memory for session storage.\n");
    } else {
        THIS->sessionsize <<= TABLE_GROWTH_FACTOR;
        THIS->sessions = (smtp_session_t*)realloc(THIS->sessions, THIS->sessionsize);
        if (!THIS->sessions) {
            THIS->sessionsize >>= TABLE_GROWTH_FACTOR;
            generic_error("create_session", Pike_sp-args, args,
                          "Failed to reallocate memory for session storage.\n");
        }
    }

    THIS->sessions[THIS->nsessions]  = smtp_create_session();
    if (!THIS->sessions[THIS->nsessions])
        generic_error("create_session", Pike_sp-args, args,
                      "Failed to create session.\n");

    pop_n_elems(args);
    push_int(++THIS->nsessions);
}

static void
f_smtp_destroy_session(INT32 args)
{
    int  sidx;
    
    if (args != 1)
        generic_error("destroy_session", Pike_sp-args, args,
                      "Too many arguments - 1 required.\n");

    get_all_args("destroy_session", args, "%i", &sidx);
    if ((unsigned)sidx > THIS->nsessions)
        generic_error("destroy_session", Pike_sp-args, args,
                      "Passed index exceedes the number of opened sessions.\n");

    sidx = smtp_destroy_session(THIS->sessions[sidx - 1]);
    THIS->sessions[sidx - 1] = NULL;
    THIS->nsessions--;

    pop_n_elems(args);
    push_int(sidx);
}

/* Pike interface */
static void init_esmtp(struct object *o)
{
    THIS->recipients = NULL;
    THIS->nrecipients = 0;
    THIS->error = 0;
    THIS->sessions = NULL;
    THIS->nsessions = 0;
}

static void exit_esmtp(struct object *o)
{
    if (THIS->recipients) {
        free(THIS->recipients);
        THIS->nrecipients = 0;
        THIS->recipientsize = 0;
        THIS->recipients = NULL;
    }

    if (THIS->sessions) {
        unsigned int i;

        for(i = 0; i < THIS->nsessions; i++)
            smtp_destroy_session(THIS->sessions[i]);
        
        free(THIS->sessions);
        THIS->nsessions = 0;
        THIS->sessionsize = 0;
        THIS->sessions = NULL;
    }
}

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
    pexts_init();
#endif

    start_new_program();
    ADD_STORAGE(ESMTP_STORAGE);

    set_init_callback(init_esmtp);
    set_exit_callback(exit_esmtp);

    ADD_FUNCTION("version", f_smtp_version,
                 tFunc(tVoid, tString), 0);
    ADD_FUNCTION("errno", f_smtp_errno,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("strerror", f_smtp_strerror,
                 tFunc(tInt, tString), 0);
    ADD_FUNCTION("create_session", f_smtp_create_session,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("destroy_session", f_smtp_destroy_session,
                 tFunc(tInt, tInt), 0);
    
    esmtp_program = end_program();
    add_program_constant("Esmtp", esmtp_program, 0);

        /* Error constants */
    add_integer_constant("ERR_NOTHING_TO_DO", SMTP_ERR_NOTHING_TO_DO, 0);
    add_integer_constant("ERR_DROPPED_CONNECTION", SMTP_ERR_DROPPED_CONNECTION, 0);
    add_integer_constant("ERR_INVALID_RESPONSE", SMTP_ERR_INVALID_RESPONSE_SYNTAX, 0);
    add_integer_constant("ERR_STATUS_MISMATCH", SMTP_ERR_STATUS_MISMATCH, 0);
    add_integer_constant("ERR_INVALID_RESPONSE_STATUS", SMTP_ERR_INVALID_RESPONSE_STATUS, 0);    
    add_integer_constant("ERR_INVAL", SMTP_ERR_INVAL, 0);
    add_integer_constant("ERR_EXTENSION_NOT_AVAILABLE", SMTP_ERR_EXTENSION_NOT_AVAILABLE, 0);
    add_integer_constant("ERR_EAI_AGAIN", SMTP_ERR_EAI_AGAIN, 0);
    add_integer_constant("ERR_EAI_FAIL", SMTP_ERR_EAI_FAIL, 0);
    add_integer_constant("ERR_EAI_MEMORY", SMTP_ERR_EAI_MEMORY, 0);
    add_integer_constant("ERR_EAI_ADDRFAMILY", SMTP_ERR_EAI_ADDRFAMILY, 0);
    add_integer_constant("ERR_EAI_NODATA", SMTP_ERR_EAI_NODATA, 0);
    add_integer_constant("ERR_EAI_FAMILY", SMTP_ERR_EAI_FAMILY, 0);
    add_integer_constant("ERR_EAI_BADFLAGS", SMTP_ERR_EAI_BADFLAGS, 0);
    add_integer_constant("ERR_EAI_NONAME", SMTP_ERR_EAI_NONAME, 0);
    add_integer_constant("ERR_EAI_SERVICE", SMTP_ERR_EAI_SERVICE, 0);
    add_integer_constant("ERR_EAI_SOCKTYPE", SMTP_ERR_EAI_SOCKTYPE, 0);

    /* Protocol monitor callback.  Values for writing */
    add_integer_constant("CB_READING", SMTP_CB_READING, 0);
    add_integer_constant("CB_WRITING", SMTP_CB_WRITING, 0);
    add_integer_constant("CB_HEADERS", SMTP_CB_HEADERS, 0);
}

void pike_module_exit(void)
{
    free_program(esmtp_program);
}
#else /* !HAVE_LIBESMTP */
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif
