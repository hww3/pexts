/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
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
/* Function protos for bdb */


/* These always exist */
void pike_module_init(void);
void pike_module_exit(void);

/* These only exist when bdb is available. */
#ifdef HAVE_BDB

/* DB functions */
void free_bdb_struct(struct object *o);
void init_bdb_struct(struct object *o);
void f_bdb_open(INT32 args);
void f_bdb_create(INT32 args);
void f_bdb_put(INT32 args);
void f_bdb_get(INT32 args);
void f_bdb_del(INT32 args);
void f_bdb_sync(INT32 args);

void f_bdb_init_db_program(void);

#endif
