/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001, 2002 The Caudium Group
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

#include "bdb_config.h"
#include "bdb_proto.h"

#ifdef HAVE_BDB
#include <db.h>

#define BDB ((BDB_Storage *)Pike_fp->current_storage)
#define BDBERR( msg) Pike_error("%s\n", msg)
#define CHECKBDB() DB *db; DB_TXN *txnid; if(!(BDB->state)) BDBERR("Database not initialized."); else if((BDB->state) == -1) BDBERR("Database is closed."); db = BDB->db; txnid = BDB->txnid;
typedef struct
{
  DB *db;
  int state;
  DB_TXN *txnid;
  DBTYPE type;
} BDB_Storage;

#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif /* ADD_STORAGE */

#endif /* HAVE_BDB */
