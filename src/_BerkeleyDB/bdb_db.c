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

/* Main DB class functions */

#include "global.h"
RCSID("$Id$");

#include "pexts.h"
#include "threads.h"

#include "bdb.h"

#ifdef HAVE_BDB


/* Free the storage */
void free_bdb_struct(struct object *o)
{
  if(BDB->db != NULL) {
    BDB->db->close(BDB->db, 0);
    BDB->db = NULL;
  }
  BDB->state = -1; /* Destructed */
}
/* init storage */
void init_bdb_struct(struct object *o)
{
  BDB->db = NULL;
  BDB->state = 0;
  BDB->txnid = NULL;
}

/* Initialize program */
void f_bdb_init_db_program(void) {
  start_new_program();
  ADD_STORAGE( BDB_Storage  );
  ADD_FUNCTION( "create", f_bdb_create, tFunc(tVoid, tVoid), 0 );
  ADD_FUNCTION( "open", f_bdb_open, tFunc(tStr tStr tInt tInt tInt, tInt), 0);
  ADD_FUNCTION( "put", f_bdb_put, tFunc(tStr tStr tInt, tInt), 0);
  ADD_FUNCTION( "get", f_bdb_get, tFunc(tStr, tStr), 0);
  ADD_FUNCTION( "del", f_bdb_del, tFunc(tStr, tInt), 0);
  ADD_FUNCTION( "sync", f_bdb_sync, tFunc(tVoid, tInt), 0);

  set_exit_callback(free_bdb_struct);
  set_init_callback(init_bdb_struct);
  end_class("db", 0);
}

void f_bdb_create(INT32 args)
{
  int ret;
  if(BDB->db != NULL)
    BDBERR("Create already called!");
  ret = db_create(&(BDB->db), NULL, 0);
  if(ret) {
    BDB->db = NULL;
    BDBERR(db_strerror(ret));
  }
  BDB->state = 1; /* opened */
  pop_n_elems(args);
}

void f_bdb_open(INT32 args) {
  char *file, *database;
  u_int32_t flags;
  int mode, ret;
  DBTYPE type;
  CHECKBDB();
  get_all_args("BerkeleyDB.DB->open", args,
	       "%s%s%d%d%d", &file, &database, &type, &flags, &mode);
  if(!strlen(file)) {
    /* Use memory database */
    file = database = NULL;
  } else if(!strlen(database)) {
    /* Single database storage */
    database = NULL;
  }
  if(!type) {
    type = DB_UNKNOWN;
  }
  flags |= DB_THREAD; /* Always allow threaded access */
  ret = db->open(db, file, database, type, flags, mode);
  if(ret) {
    BDBERR(db_strerror(ret));
  }
  /* Save the database type for future need. We use get_type to get the
   * type even if DB_UNKNOWN was specified
   */
  BDB->type = BDB->db->get_type(BDB->db);
  pop_n_elems(args);
  push_int(1);
}

void f_bdb_put(INT32 args)
{
  struct pike_string *key, *data;
  DBT db_key, db_data;
  u_int32_t flags;
  int ret = 0;
  CHECKBDB();
  get_all_args("BerkeleyDB.DB->put", args,
	       "%S%S%d", &key, &data, &flags);

  if(key->size_shift)
    Pike_error("Invalid argument 1, expected 8-bit string.\n");
  if(data->size_shift)
    Pike_error("Invalid argument 2, expected 8-bit string.\n");
  MEMSET(&db_key, 0, sizeof(db_key));
  db_key.data = key->str;
  db_key.size = key->len;
  
  MEMSET(&db_data, 0, sizeof(db_data));
  db_data.data = data->str;
  db_data.size = data->len;

  THREADS_ALLOW();
  ret = db->put(db, txnid, &db_key, &db_data, flags);
  THREADS_DISALLOW();
  switch(ret)
  {
  case 0: /* All went fine */
    ret = 1;
    break;

  case DB_KEYEXIST: /* Key exists and overwrite mode is disabled */
    ret = 0;
    break;

  default: /* Fatal error */
    BDBERR(db_strerror(ret));
    break;
  }
  pop_n_elems(args);
  push_int(ret);
}

void f_bdb_get(INT32 args) {
  struct pike_string *key, *data;
  DBT db_key, db_data;
  int ret=0;
  CHECKBDB();
  get_all_args("BerkeleyDB.DB->get", args, "%S", &key);
  if(key->size_shift)
    Pike_error("Invalid argument 1, expected 8-bit string.\n");
  MEMSET(&db_key, 0, sizeof(db_key));
  db_key.data = key->str;
  db_key.size = key->len;
  
  MEMSET(&db_data, 0, sizeof(db_data));
  db_data.flags = DB_DBT_REALLOC;

  THREADS_ALLOW();
  ret = db->get(db, txnid, &db_key, &db_data, 0);
  THREADS_DISALLOW();

  switch(ret)
  {
  case 0: /* All went fine */
    data = make_shared_binary_string(db_data.data, db_data.size);
    free(db_data.data);
    pop_n_elems(args);
    push_string(data);
    break;
  case DB_NOTFOUND: /* no key or empty key */
  case DB_KEYEMPTY:
    pop_n_elems(args);
    push_int(0); 
    break;
  default: /* fatal error */ 
    BDBERR(db_strerror(ret));
    break;
  }
}

void f_bdb_del(INT32 args) {
  struct pike_string *key;
  DBT db_key;
  int ret=0;

  CHECKBDB();
  get_all_args("BerkeleyDB.DB->del", args, "%S", &key);
  if(key->size_shift)
    Pike_error("Invalid argument 1, expected 8-bit string.\n");
  MEMSET(&db_key, 0, sizeof(db_key));
  db_key.data = key->str;
  db_key.size = key->len;
  
  THREADS_ALLOW();
  ret = db->del(db, txnid, &db_key, 0);
  THREADS_DISALLOW();

  switch(ret)
  {
  case 0: /* All went fine */
    ret = 1;
    break;
  case DB_NOTFOUND: /* key not found */
    ret = 0;
    break;
  default: /* fatal error */ 
    BDBERR(db_strerror(ret));
    break;
  }
  pop_n_elems(args);
  push_int(ret);
}

void f_bdb_sync(INT32 args) {
  int ret=0;
  CHECKBDB();

  THREADS_ALLOW();
  ret = db->sync(db, 0);
  THREADS_DISALLOW();

  switch(ret)
  {
  case 0: /* All went fine */
    ret = 1;
    break;
  case DB_INCOMPLETE: /* key not found */
    ret = 0; 
    break;
  default: /* fatal error */ 
    BDBERR(db_strerror(ret));
    break;
  }
  pop_n_elems(args);
  push_int(ret);
}

#endif /* HAVE_BDB */
