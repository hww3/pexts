/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000 The Caudium Group
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

/* PCRE Support - This module adds PCRE support to Pike. */

#include "global.h"
RCSID("$Id$");

#include "stralloc.h"
#include "pike_macros.h"
#include "module_support.h"
#include "program.h"
#include "error.h"
#include "threads.h"
#include "array.h"
#include "pdf_config.h"

#ifdef HAVE_PDFLIB
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>


void f_pdf_create(INT32 args)
{
	char *filename;
	THIS->pdf = PDF_new();

	if( !args ) { 
		THIS->filename=NULL;
		PDF_open_mem(THIS->pdf, writeproc );
	} else {
		get_all_args("create",args,"%s",&filename);
		THIS->filename=strdup( filename );
		if( PDF_open_file(THIS->pdf,THIS->filename) == -1 ) {
			error("PDF.pdf->create: Can't open file: %s.\n",filename);
		}
	}
	pop_n_elems(args);
}

size_t writeproc( PDF *p, void *data, size_t size ) {

	THIS->storage=realloc(THIS->storage,THIS->size+size);

	THIS->tail=THIS->storage+THIS->size;
	memcpy( THIS->tail , data,size);
	THIS->size += size;
	return size;
}

// Set document information fields
// Keys: 'Subject', 'Title', Creator', 'Author', 'Keywords'
void f_set_info(INT32 args) {
	char *key,*value;

	get_all_args("set_info",args,"%s%s", &key, &value );
	PDF_set_info(THIS->pdf,key,value);
	
	pop_n_elems( args );
}

// start a new page in the PDF file
// width/height are in `pt'
void f_begin_page(INT32 args) {
	float width, height;
	get_all_args("begin_page",args,"%f%f",&width,&height);
	
	PDF_begin_page( THIS->pdf, width, height );

	pop_n_elems( args );
}

void f_end_page(INT32 args) {
	
	PDF_end_page( THIS->pdf );
	pop_n_elems( args );
}

void f_close(INT32 args) {
	PDF_close(THIS->pdf);
	pop_n_elems(args);
}

void f_generate(INT32 args) {
	if( !THIS->filename && THIS->storage ) {
		push_string(make_shared_binary_string(THIS->storage,THIS->size));
	} else {
		error("PDF.pdf.generate: File method used.\n");
	} 
	pop_n_elems(args);
}

void f_findfont(INT32 args) {
	char *fontname, *encoding;
	int embed;
	int font;
	
	get_all_args("findfont",args,"%s%s%d",&fontname, &encoding, &embed );
	font=PDF_findfont(THIS->pdf, fontname, encoding, embed );

	pop_n_elems(args);
	push_int(font);
}

void f_setfont(INT32 args) {
	int font, size;

	get_all_args("setfont",args,"%d%d",&font,&size);
	PDF_setfont(THIS->pdf, font, size );
	pop_n_elems(args);
}

void f_show(INT32 args) {
	char *text;
	int len;
	
	if( args == 1 ) {
		get_all_args("show",args,"%s",&text);
		PDF_show(THIS->pdf, text);
	} else {
		get_all_args("show",args,"%s%d",&text,&len);
		PDF_show2(THIS->pdf, text,len);
	}
	pop_n_elems(args);
}

void f_continue_text(INT32 args) {
	char *text;
	int len;
	
	if( args == 1 ) {
		get_all_args("continue_text",args,"%s",&text);
		PDF_continue_text(THIS->pdf, text);
	} else {
		get_all_args("show",args,"%s%d",&text,&len);
		PDF_continue_text2(THIS->pdf, text,len);
	}
	pop_n_elems(args);
}

void f_set_text_pos(INT32 args) {
	float x,y;
	get_all_args("set_text_pos",args,"%f%f",&x,&y);
	PDF_set_text_pos(THIS->pdf,x,y);
	pop_n_elems(args);
}

static struct program *pdf_program;
static void free_pdf(struct object *o)
{
  MEMSET(THIS, 0, sizeof(PDF_storage));
}

static void init_pdf(struct object *o)
{
  MEMSET(THIS, 0, sizeof(PDF_storage));
  THIS->storage=NULL;
  THIS->tail=NULL;
  THIS->size=0;
  THIS->filename=NULL;
}

/* Init the module */
void pike_module_init(void)
{
  start_new_program();
  ADD_STORAGE( PDF_storage  );
  add_function( "create", f_pdf_create,
		"function(string|void:void)", 0 ); 
  add_function( "set_info", f_set_info,
		"function(string,string:void)",0);
  add_function( "begin_page", f_begin_page,
		"function(float,float:void)",0);
  add_function( "end_page", f_end_page,
		"function(void:void)",0);
  add_function( "close", f_close,
		"function(void:void)",0);
  add_function( "generate", f_generate,
		"function(void:string)", 0);
  add_function( "findfont", f_findfont,
		"function(string,string,int:int)",0);
  add_function( "setfont", f_setfont,
		"function(int,int:void)",0);
  add_function( "show", f_show,
		"function(string,int|void:void)",0);
  add_function( "continue_text", f_continue_text,
		"function(string,int|void:void)",0);
  add_function( "set_text_pos", f_set_text_pos,
		"function(float,float:void)",0);

  set_init_callback(init_pdf);
  set_exit_callback(free_pdf);
  pdf_program = end_program();
  add_program_constant("pdf", pdf_program, 0);
}


/* Restore and exit module */
void pike_module_exit( void )
{
  free_program(pdf_program);
}


#endif /* HAVE_PCRE */

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
