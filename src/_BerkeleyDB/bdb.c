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
 */

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "bdb.h"

/* Initialize and start module */

void pike_module_init( void )
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
  
#ifdef HAVE_BDB
  f_bdb_init_db_program();
 
  /* Open flags */
  add_integer_constant("DB_CREATE", DB_CREATE, 0);
  add_integer_constant("DB_EXCL", DB_EXCL, 0);
  add_integer_constant("DB_NOMMAP", DB_NOMMAP, 0);
  add_integer_constant("DB_RDONLY", DB_RDONLY, 0);
  add_integer_constant("DB_TRUNCATE", DB_TRUNCATE, 0);

  /* Database types */
  add_integer_constant("DB_BTREE", DB_BTREE, 0);
  add_integer_constant("DB_HASH", DB_HASH, 0);
  add_integer_constant("DB_QUEUE", DB_QUEUE, 0);
  add_integer_constant("DB_RECNO", DB_RECNO, 0);
  add_integer_constant("DB_UNKNOWN", DB_UNKNOWN, 0);
#endif
}

/* Restore and exit module */
void pike_module_exit( void )
{
}

