/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2005 The Caudium Group
 */

/* Copyright (C) 2002-2005 The Caudium Group
 * Copyright (C) 2002-2005 Zsolt Varga
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
/* generic headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>

/* function specific headers */
#include "libmutt/src/mutt.h"
#include "libmutt/src/mutt_regex.h"
#include <pcreposix.h>
#include "libmutt/src/mailbox.h"
#include "libmutt/src/rfc822.h"
#include "libmutt/src/mutt_socket.h"
#include "libmutt/src/mime.h"
#include "libmutt/src/md5.h"


/* pike headers */
#include "global.h"
#include "pike_macros.h"
#include "program.h"
#include "interpret.h"
#include "mapping.h"
#include "object.h"
#include "svalue.h"
#include "builtin_functions.h"
#include "module_support.h"
#include "stralloc.h"
#include "fdlib.h"

/* local headers */
#include "mailstore.h"
#include "mailstore_config.h"

#ifdef fp
#undef fp
#endif

#ifdef USE_SSL
#include "libmutt/src/keymap_defs.h"
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
int libmutt_ask_for_cert_acceptance(X509 *);
#endif

#ifdef DEBUG
FILE *debugfile;
int debuglevel;
#endif

/*
**! file: mailstore.c
**!  Generic access to Mail Storage, like Maidir, mbox, IMAP.
**! cvs_version: $Id$
*/

RCSID("$Id$");
/* Mailbox */
typedef struct {
	short debug;
	CONTEXT *ctx;
	HEADER *header;
	char *folder;
} MAILSTORE_STORAGE;

/* Mailbox.Message */
typedef struct {
	int msgno;
	MESSAGE *msg;
	HEADER *header;
	CONTEXT *ctx;
} MESSAGE_STORAGE;

#define THIS ((MAILSTORE_STORAGE *)Pike_fp->current_storage)
#define THISMSG ((MESSAGE_STORAGE *)Pike_fp->current_storage)

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

/*
 * Not very elegant, but effective. We have to reference
 * those routines so that they're pulled in from libmutta.
 * Otherwise we'll end up with a .so that doesn't load.
 */
static void *__dummy_variable[] = {
    NULL,
    mutt_socket_readln_d,
    mutt_socket_close,
    mutt_to_base64,
    mutt_socket_write_d,
    MD5Final,
    mutt_socket_readchar,
    mutt_socket_head,
    really_free_svalue,
    mutt_socket_free,
    MD5Init,
    MD5Update,
    mutt_from_base64,
    mutt_socket_open,
    mutt_conn_find,
    mutt_account_getuser
};

/* forward declarations */
void init_mailstore(struct object *o);
void exit_mailstore(struct object *o);
void f_create(INT32 args);
void f_debug(INT32 args);
void f__sizeof(INT32 args);
void f_get_header(INT32 args);
void f_stat(INT32 args);
void push_headers(HEADER *header);
void f_set_flag(INT32 args);
void f_reset_flag(INT32 args);
void f_check_mailbox(INT32 args);

/* LibMutt.Message */
void init_message_storage(struct object *);
void exit_message_storage(struct object *);
void f_msg_create(INT32 args);
void f_msg_get_header(INT32 args);
void f_msg_getfd(INT32 args);


void mutt_message(const char *fmt, ...);
static void libmutt_error (const char *s, ...);


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

	THIS->debug=0;
	THIS->folder=strdup(path);

	if( !FileMask.rx ) {
		FileMask.rx = (regex_t *) malloc (sizeof (regex_t));
		REGCOMP(FileMask.rx,"!^\\.[^.]",0);
	}
	if( !ReplyRegexp.rx ) {
		ReplyRegexp.rx = (regex_t *) malloc (sizeof (regex_t));
		REGCOMP(ReplyRegexp.rx,"^(re([\\[0-9\\]+])*|aw):[ \t]*",0);
	}
	
	THIS->ctx=mx_open_mailbox(THIS->folder, M_QUIET ,NULL);
	if( THIS->ctx == NULL ) {
		Pike_error("Context not open for folder '%s'\n",THIS->folder);
	}
	pop_n_elems(args);
}


void f_debug(INT32 args) {
#ifdef DEBUG
	time_t t;
	char buf[_POSIX_PATH_MAX];
#endif
	int debug;
	get_all_args("debug",args,"%i",&debug);
	THIS->debug=(debug!=0);
#ifdef DEBUG
	t = time (0);
	debuglevel=THIS->debug;
	debugfile=stderr;
//	snprintf(buf, sizeof(buf), "%s/Mailstore_debug", NONULL(Tempdir));
//	fprintf(stderr,"debug: opening '%s'\n",buf);
//	if( (debugfile=safe_fopen(buf, "w")) != NULL ) {
	fprintf(debugfile,"Mailstore debug started at %s",asctime (localtime (&t)));
//	}
#endif
	pop_n_elems(args);
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
	PUSH_MAPPING_si("total", THIS->ctx->msgcount);
	PUSH_MAPPING_si("new",   THIS->ctx->new);
	PUSH_MAPPING_si("unread",THIS->ctx->unread);
	PUSH_MAPPING_si("tagged",THIS->ctx->tagged);
	PUSH_MAPPING_si("seen",  THIS->ctx->msgcount-THIS->ctx->new );
	f_aggregate_mapping(10);
}

void f_set_flag(INT32 args) {
	int idx;
	HEADER *header=0;
	int flag;
	
	get_all_args("set_flag",args,"%i%i",&idx,&flag);
	
	header=THIS->ctx->hdrs[idx];
	mutt_set_flag(THIS->ctx, header, flag,1);
	pop_n_elems(args);
}

void f_reset_flag(INT32 args) {
	int idx;
	HEADER *header=0;
	int flag;
	
	get_all_args("reset_flag",args,"%i%i",&idx,&flag);
	
	header=THIS->ctx->hdrs[idx];
	mutt_set_flag(THIS->ctx, header, flag,0);
	pop_n_elems(args);
}

void f_check_mailbox(INT32 args) {
	int index_hint;
	int ret;

	index_hint=THIS->ctx->msgcount-1;
	ret=mx_check_mailbox(THIS->ctx, &index_hint, 0);
	push_int(ret);
}

void f_get_header(INT32 args) {
	int idx;
	HEADER *header=0;
	
	get_all_args("get_header",args,"%i",&idx);
	pop_n_elems(args);
	
	if( idx >= THIS->ctx->msgcount || idx < 0 ) {
		Pike_error("get_header: header index '%d' not present!\n",idx);
	}
		
	header=THIS->ctx->hdrs[idx];
	if( !header ) {
		Pike_error("get_header: header pointer is NULL for index '%d'\n",idx);
	}
	push_headers(header);
}

void push_headers(HEADER *header) {
	int num;
	ADDRESS *adr;
	
	push_text("subject");		PUSH_TEXT(header->env->real_subj);
	push_text("message_id");	PUSH_TEXT(header->env->message_id);
	push_text("type"); PUSH_TEXT( TYPE(header->content) );
	push_text("subtype"); PUSH_TEXT(header->content->subtype);
	push_text("encoding"); PUSH_TEXT( ENCODING(header->content->encoding) );
	push_text("length"); push_int(header->content->length);
	
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
	f_aggregate_mapping(30);
}

/* Mailstore.Mailbox.Message */
void f_msg_create(INT32 args) {
	int msgno;
	MAILSTORE_STORAGE *ps;
	//ps=(MESSAGE_STORAGE *) parent_storage(1);
	ps=(MAILSTORE_STORAGE *) parent_storage(1);
	if( !ps ) {
		Pike_error("LibMutt.Message: Parent lost\n");
	}
	THISMSG->ctx=(CONTEXT *) ps->ctx;
	get_all_args("create",args,"%d",&msgno);
	THISMSG->msgno=msgno;
	fprintf(stderr,"Message.create: tring to open msg '%d'\n",THISMSG->msgno);
	if ((THISMSG->msg=mx_open_message (THISMSG->ctx, THISMSG->msgno))) {
		THISMSG->header=THISMSG->ctx->hdrs[THISMSG->msgno]; 
		mutt_parse_part (THISMSG->msg->fp, THISMSG->header->content);
		fprintf(stderr,"MUTT_PARSE_PART finished!\n");
	} else {
		Pike_error("LibMutt.Message: no message at id '%d'\n",THISMSG->msgno);
	}
	pop_n_elems(args);
}

void f_msg_getfd(INT32 args) {
	MESSAGE *message;
	struct object *obj;
	
	message=THISMSG->msg;
	
	pop_n_elems(args);
	if( message->fp ) {
		obj=file_make_object_from_fd(fileno(message->fp), 0x1000,0);
		push_object(obj);
	} else 
		push_int(-1);
}

void f_msg_get_header(INT32 args) {
	HEADER *header=0;
	int idx=THISMSG->msgno;
	
	pop_n_elems(args);
	if( idx >= THISMSG->ctx->msgcount || idx < 0 ) {
		Pike_error("get_header: header index '%d' not present!\n",idx);
	}
		
	header=THISMSG->ctx->hdrs[idx];
	if( !header ) {
		Pike_error("get_header: header pointer is NULL for index '%d'\n",idx);
	}
	push_headers(header);
}



void init_mailstore(struct object *o) {
	if( !THIS ) return;
	MEMSET(THIS, 0, sizeof(MAILSTORE_STORAGE));
}

void exit_mailstore(struct object *o) {
	if( THIS->ctx )
		mx_close_mailbox( THIS->ctx,NULL);
	if( THIS->folder )
		free(THIS->folder);
	if( FileMask.rx ) safe_free((void**)&FileMask.rx);
	if( ReplyRegexp.rx ) safe_free((void**)&ReplyRegexp.rx);
}

void init_message_storage(struct object *o) {
	if( !THISMSG ) return;
	MEMSET(THISMSG, 0, sizeof(MESSAGE_STORAGE));
}

void exit_message_storage(struct object *o) {
	if( THISMSG->msg ) {
		mx_close_message( &THISMSG->msg);
		fprintf(stderr,"mx_close_message CALLED!\n");
	} else {
		fprintf(stderr,"mx_close_message __NOT__ CALLED!\n");
	}
}

/* pike module API */

void pike_module_init() {
	struct utsname utsname;
	char *p;
	uname(&utsname);
	if ((p = strchr (utsname.nodename, '.')))
		*p = 0;
	Hostname=strdup(utsname.nodename);
	Tempdir=(char *) LIBMUTT_TEMPDIR;

	ADD_INT_CONSTANT("M_READ",M_READ,0);
	ADD_INT_CONSTANT("M_REPLIED",M_REPLIED,0);
	ADD_INT_CONSTANT("M_OLD",M_OLD,0);
	ADD_INT_CONSTANT("M_FLAG",M_FLAG,0);
	ADD_INT_CONSTANT("M_DELETE",M_DELETE,0);
	ADD_INT_CONSTANT("M_TAG",M_TAG,0);
	ADD_INT_CONSTANT("M_NEW",M_NEW,0);

	/* constants for mx_check_mailbox() */
	ADD_INT_CONSTANT("M_NEW_MAIL",M_NEW_MAIL,0);
	ADD_INT_CONSTANT("M_REOPENED",M_REOPENED,0);
	ADD_INT_CONSTANT("M_FLAGS",M_FLAGS,0);

	start_new_program();
	 ADD_STORAGE(MAILSTORE_STORAGE);
	
	 set_init_callback(init_mailstore);
	 set_exit_callback(exit_mailstore);


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
		tFunc(tInt, tInt), 0);
	ADD_FUNCTION("set_flag", f_set_flag,
		tFunc(tInt tInt, tVoid), 0);
	ADD_FUNCTION("reset_flag", f_reset_flag,
		tFunc(tInt tInt, tVoid), 0);
	ADD_FUNCTION("check_mailbox", f_check_mailbox,
		tFunc(tVoid, tInt), 0);

	/* Mailbox.Message */
	start_new_program();
 	  set_init_callback(init_message_storage);
	  set_exit_callback(exit_message_storage);
	  ADD_STORAGE( MESSAGE_STORAGE );
	  ADD_FUNCTION("create", f_msg_create,
			tFunc(tInt, tVoid), 0);
	  ADD_FUNCTION("getFD", f_msg_getfd,   
			tFunc(tVoid, tInt), 0);
	  ADD_FUNCTION("get_header",f_msg_get_header, 
			tFunc(tVoid, tMapping), 0);
	 end_class("Message",0);

	end_class("Mailbox",0);
	
	mutt_error=libmutt_error;
	/* reference it so that it's not optimized out */
	__dummy_variable[0] = (void*)NULL;
}



void pike_module_exit() 
{ 
  /* reference it so that it's not optimized out */
  __dummy_variable[0] = (void*)NULL;
}

/*---------------------------------------------------------------------*/
/* libmutt callbacks */
void mutt_exit(int code) {
}

void mutt_message(const char *fmt, ...) {
	va_list va_args;
	if( !THIS->debug ) return;
	va_start(va_args, fmt);
	vfprintf(stderr,  fmt, va_args);
	va_end(va_args);
	fprintf(stderr,"\n");
}

void mutt_clear_error(void) {

}

int mutt_yesorno(const char *msg, int def) {
	fprintf(stderr,"YES/NO: %s (%d)", msg, def);
	return 1;
}

static void libmutt_error (const char *s,...) {
	va_list va_args;
	va_start(va_args, s);
	if( THIS->debug ) {
		if( *s ) {
			vfprintf(stderr,  s, va_args);
		}
	}
	va_end(va_args);
	fprintf(stderr,"\n");
}


void mutt_perror (const char *s) {
	char *p = strerror (errno);
	fprintf(stderr,"%s: %s (errno = %d)\n",s,p ? p : "unknown error", errno ); 
}

#ifdef USE_SSL
int libmutt_ask_for_cert_acceptance(X509 *cert) {
	        return OP_SAVE;
}
#endif

/* vim: set ts=3: */
