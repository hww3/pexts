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

#ifndef PDF_CONFIG_H
#define PDF_CONFIG_H

@TOP@

@BOTTOM@

#if defined(HAVE_PDFLIB_H) && defined(HAVE_LIBPDF)
#define HAVE_PDFLIB
#include <pdflib.h>
#define THIS ((PDF_storage *)fp->current_object->storage)

typedef struct
{
  PDF *pdf;
  char *storage;
  char *tail;
  size_t size;
  char *filename;
} PDF_storage;

#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif
#endif
size_t writeproc( PDF *p, void *data, size_t size );
void f_pdf_create(INT32 args);
void f_set_info(INT32 args);
void f_begin_page(INT32 args);
void f_end_page(INT32 args);
void f_close(INT32 args);
void f_findfont(INT32 args);
void f_setfont(INT32 args);
void f_show(INT32 args);
void f_continue_text(INT32 args);
void f_set_text_pos(INT32 args);
void f_generate(INT32 args);
void f_stringwidth(INT32);
// Graphics Functions
void f_setdash(INT32);
void f_setlinewidth(INT32);
void f_moveto(INT32);
void f_lineto(INT32);
void f_curveto(INT32);
void f_circle(INT32);
void f_arc(INT32);
void f_rect(INT32);
void f_stroke(INT32);

// pike module functions
void pike_module_init(void);
void pike_module_exit(void);
#endif
