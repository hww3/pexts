/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
 */

/* Copyright (C) 2002-2003 The Caudium Group
 * Copyright (C) 2002-2003 Zsolt Varga
 * 
 * This file is part of the Pike Extensions package.
 *
 * This Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This Pexts Module is distributed in the hope that it will be useful,
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

/* pike headers */
#include "global.h"
RCSID("$Id$");
#include "global.h"
#include "pike_macros.h"
#include "program.h"
#include "interpret.h"
#include "mapping.h"
#include "svalue.h"
#include "builtin_functions.h"
#include "module_support.h"
#include "stralloc.h"

/* generic headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/* function specific headers */
#include <mutt.h>
#include <mutt_regex.h>
#include <pcreposix.h>
#include <mailbox.h>
#include <rfc822.h>

/* local headers */
#include "mailstore.h"
#include "mailstore_config.h"

/*
**! file: mailstore.c
**!  Generic access to Mail Storage, like Maidir, mbox, IMAP.
**! cvs_version: $Id$
*/

typedef struct {
	CONTEXT *ctx;
	HEADER *header;
	char *folder;
} MAILSTORE_STORAGE;


#define THIS ((MAILSTORE_STORAGE *)Pike_fp->current_storage)

#define PUSH_ADDRESS(KEY,METHOD)  push_text(KEY); \
	push_text( ( header->env->METHOD && header->env->METHOD->personal \
		? header->env->METHOD->personal \
		: "" )); \
	push_text( ( header->env->METHOD && header->env->METHOD->mailbox \
		? header->env->METHOD->mailbox \
		: "" )); \
	f_aggregate(2);\
	f_aggregate(1)

#define PUSH_ADDRESS_LIST(KEY,METHOD)  push_text(KEY); \
	for( num=0,adr=header->env->METHOD ; adr ; adr=adr->next,num++ ) { \
		push_text( ( adr->personal \
			? adr->personal \
			: "" )); \
		push_text( ( adr->mailbox \
			? adr->mailbox \
			: "" )); \
		f_aggregate(2); \
	}\
	f_aggregate(num)

#define PUSH_TEXT(X)  if( X ) push_text(X); else push_text("")

// one mapping entry of mapping(string:int)
#define PUSH_MAPPING_si(I, V) \
	push_text(I); \
	push_int(V);


/* forward declarations */
void init_mailstore(struct object *o);
void f_create(INT32 args);
void f_debug(INT32 args);
void f__sizeof(INT32 args);
void f_get_header(INT32 args);
void f_stat(INT32 args);

void mutt_message(const char *fmt, ...);

/* pike methods */


/*
**! method: create(string uri)
**!  Append data to the decode buffer.
**! arg: string uri
**!  A URI pointing to the mailstore.
**!   Examples:
**!    imap://user:pass@host/INBOX
**!	 /path/to/mbox
**!    /path/to/Maildir
**!    /path/to/mh
**! returns:
**!  nothing (for now)
**! name: Mailstore() - class constructor for Maistore
*/

void f_create(INT32 args) {
	char *path;

	get_all_args("create",args,"%s",&path);
	pop_n_elems(args);

	THIS->folder=strdup(path);
	FileMask.rx = (regex_t *) safe_malloc (sizeof (regex_t));
	REGCOMP(FileMask.rx,"!^\\.[^.]",0);
	ReplyRegexp.rx = (regex_t *) safe_malloc (sizeof (regex_t));
	REGCOMP(ReplyRegexp.rx,"^(re([\\[0-9\\]+])*|aw):[ \t]*",0);

	THIS->ctx=mx_open_mailbox(THIS->folder, M_QUIET ,NULL);
}

void f_debug(INT32 args) {
	pop_n_elems(args);
	fprintf(stderr,"\nmsgs: %d\nnew: %d\nunread: %d\ntagged: %d\n",
		THIS->ctx->msgcount, THIS->ctx->new, THIS->ctx->unread, 
		THIS->ctx->tagged );
	push_string(make_shared_string(THIS->folder));
}

void f__sizeof(INT32 args) {
	pop_n_elems(args);
	push_int(THIS->ctx->msgcount);
}

/*
**! method: stat(void)
**!  Gives statistics for current mailbox
**! returns:
**!  mapping(string:int) with following indices:
**!  total, new, unread, tagges, seen
*/

void f_stat(INT32 args) {
	pop_n_elems(args);
	push_text("total");	push_int(THIS->ctx->msgcount);
	push_text("new");		push_int(THIS->ctx->new);
	push_text("unread");	push_int(THIS->ctx->unread);
	push_text("tagged");	push_int(THIS->ctx->tagged);
	push_text("seen");	push_int(THIS->ctx->msgcount-THIS->ctx->new);
	f_aggregate_mapping(10);
}

void f_get_header(INT32 args) {
	int idx, num;
	HEADER *header=0;
	ADDRESS *adr;
	
	get_all_args("get_header", args, "%i", &idx);
	pop_n_elems(args);
	
	if(idx >= THIS->ctx->msgcount || idx < 0) {
		Pike_error("get_header: header index '%d' not present.\n", idx);
	}

	header=THIS->ctx->hdrs[idx];
	if(!header) {
		Pike_error("get_header: reading header for message '%d' failed.\n", idx);
	}

	
	push_text("subject");		PUSH_TEXT(header->env->real_subj);
	push_text("message_id");	PUSH_TEXT(header->env->message_id);
	PUSH_MAPPING_si("lines", header->lines);
	PUSH_MAPPING_si("mime", header->mime);
	PUSH_MAPPING_si("flagged", header->flagged);
	PUSH_MAPPING_si("tagged", header->tagged);
	PUSH_MAPPING_si("deleted", header->deleted);
	PUSH_MAPPING_si("replied", header->replied);
	PUSH_MAPPING_si("date_sent", header->date_sent);
	PUSH_MAPPING_si("date_received", header->received);
	push_text("env");
		PUSH_ADDRESS("from",from);
		PUSH_ADDRESS_LIST("to",to);
		PUSH_ADDRESS_LIST("cc",cc);
		PUSH_ADDRESS_LIST("bcc",bcc);
		PUSH_ADDRESS("sender",sender);
		PUSH_ADDRESS("reply_to",reply_to);
		PUSH_ADDRESS("return_path",return_path);
		f_aggregate_mapping(14);
	f_aggregate_mapping(22);
}


void init_mailstore(struct object *o) {
	MEMSET(THIS, 0, sizeof(MAILSTORE_STORAGE));
}


/* pike module API */

void pike_module_init() {
	start_new_program();

	set_init_callback(init_mailstore);
	//set_exit_callback(free_hash_storage);

	ADD_STORAGE(MAILSTORE_STORAGE);

	// public pike methods
	ADD_FUNCTION("create", f_create,
		tFunc(tString, tVoid), 0);
	ADD_FUNCTION("_sizeof", f__sizeof,
		tFunc(tVoid, tInt), 0);
	ADD_FUNCTION("stat", f_stat,
		tFunc(tVoid, tMapping), 0); 
	ADD_FUNCTION("get_header", f_get_header,
		tFunc(tInt, tMapping), 0);
	ADD_FUNCTION("debug", f_debug,
		tFunc(tVoid, tString), 0);

	end_class("Mailbox",0);
}



void pike_module_exit() { }


/* libmutt callbacks */

void mutt_exit(int code) { }

void mutt_clear_error(void) { }

void mutt_message(const char *fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	vfprintf(stderr,  fmt, va_args);
	va_end(va_args);
}

int mutt_yesorno(const char *msg, int def) {
	fprintf(stderr,"YES/NO: %s (%d)", msg, def);
	return 1;
}

void mutt_perror (const char *s) {
	char *p = strerror (errno);
	fprintf(stderr,"%s: %s (errno = %d)\n",s,p ? p : "unknown error", errno ); 
	// mutt_error ("%s: %s (errno = %d)", s, p ? p : _("unknown error"), errno);
}

/* vim: set ts=3: */

