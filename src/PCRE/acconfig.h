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

#ifndef PCRE_CONFIG_H
#define PCRE_CONFIG_H

@TOP@

@BOTTOM@

#include "pcrelib/pcre.h"
#define THIS ((PCRE_Regexp *)fp->current_storage)

typedef struct
{
  pcre *regexp;
  pcre_extra *extra;
  struct pike_string *pattern;
} PCRE_Regexp;

void f_pcre_create(INT32);
void f_pcre_match(INT32);
void f_pcre_split(INT32);
void pike_module_init(void);
void pike_module_exit(void);
static void free_regexp(struct object *);

#endif
