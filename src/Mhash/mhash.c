/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000 The Caudium Group
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

/* Glue for the MHash library, for various hashing routines. See
 * http://mhash.sourceforge.net/ for more information about mhash.
 */

#include "global.h"
RCSID("$Id$");

#include "stralloc.h"
#include "pike_macros.h"
#include "module_support.h"
#include "program.h"
#include "error.h"
#include "threads.h"
#include "mhash_config.h"

#ifdef HAVE_MHASH

/* Free allocated data in a hash object */

static void free_hash(void)
{
  if(THIS->hash != NULL) {
    void *tmp = mhash_end(THIS->hash);
    if(tmp != NULL) free(tmp);
    THIS->hash = NULL;
  }
  if(THIS->res != NULL) {
    free(THIS->res);
    THIS->res = NULL;
  }
}


/* Create a new hash object.  */

void f_hash_create(INT32 args)
{
  if(THIS->type != -1 || THIS->hash || THIS->res) {
    error("Recursive call to create. Use Mhash.Hash()->reset() or \n"
	  "Mhash.Hash()->set_type() to change the hash type or reset\n"
	  "the object.\n");
  }
  switch(args) {
  default:
    error("Invalid number of arguments to Mhash.Hash(), expected 0 or 1.\n");
    break;
  case 1:
    if(sp[-args].type != T_INT) {
      error("Invalid argument 1. Expected integer.\n");
    }
    THIS->type = sp[-args].u.integer;
    THIS->hash = mhash_init(THIS->type);
    if(THIS->hash == MHASH_FAILED) {
      THIS->hash = NULL;
      error("Failed to initialize hash.\n");
    }
    break;
  case 0:
    break;
  }
  
  pop_n_elems(args);
}

/* Add feed to a the hash */
void f_hash_feed(INT32 args) 
{
  if(THIS->hash == NULL) {
    if(THIS->type != -1)
      error("Hash is ended. Use Mhash.Hash()->reset() to reset the hash.\n");
    else
      error("Hash is uninitialized. Use Mhash.Hash()->set_type() to select hash type.\n");
  }
  if(args == 1) {
    if(sp[-args].type != T_STRING) {
      error("Invalid argument 1. Expected string.\n");
    }
    mhash(THIS->hash, sp[-args].u.string->str,
	  sp[-args].u.string->len << sp[-args].u.string->size_shift);
  } else {
    error("Invalid number of arguments to Mhash.Hash->feed(), expected 1.\n");
  }
  pop_n_elems(args);
}

static int get_digest(void)
{
  if(THIS->res == NULL && THIS->hash != NULL) {
    THIS->res = mhash_end(THIS->hash);
    THIS->hash = NULL;
  }
  if(THIS->res == NULL) {
    error("No hash result available!\n");
  }
  return mhash_get_block_size(THIS->type);
}

void f_hash_digest(INT32 args)
{
  int len, i;
  struct pike_string *res;
  len = get_digest();
  res = begin_shared_string(len);
  for(i = 0; i < len; i++) {
    STR0(res)[i] = THIS->res[i];
  }
  res = end_shared_string(res);
  pop_n_elems(args);
  push_string(res);
}

void f_hash_hexdigest(INT32 args)
{
  int len, i, e;
  char hex[3];
  struct pike_string *res;
  len = get_digest();
  res = begin_shared_string(len*2);
  for(e = 0, i = 0; i < len; i++, e+=2) {
    snprintf(hex, 3, "%.2x", THIS->res[i]);
    STR0(res)[e] = hex[0];
    STR0(res)[e+1] = hex[1];
  }
  res = end_shared_string(res);
  pop_n_elems(args);
  push_string(res);
}

void f_hash_name(INT32 args)
{
  char *name;
  pop_n_elems(args);
  if(THIS->type != -1) {
    name = mhash_get_hash_name(THIS->type);
    if(name == NULL) {
      push_int(0);
    } else {
      push_text(name);
      free(name);
    }
  } else {
    error("Hash object not initialized!\n");
  }
}

/* Reset the hash */
void f_hash_reset(INT32 args)
{
  free_hash();
  if(THIS->type != -1) {
    THIS->hash = mhash_init(THIS->type);
    if(THIS->hash == MHASH_FAILED) {
      THIS->hash = NULL;
      error("Failed to initialize hash.\n");
    }
  }
  pop_n_elems(args);
}

/* Change hash type */
void f_hash_set_type(INT32 args)
{
  if(args == 1) {
    if(sp[-args].type != T_INT) {
      error("Invalid argument 1. Expected integer.\n");
    } 
    THIS->type = sp[-args].u.integer;
  } else {
    error("Invalid number of arguments to Mhash.Hash()->set_type, expected 1.\n");
  }
  free_hash();
  if(THIS->type != -1) {
    THIS->hash = mhash_init(THIS->type);
    if(THIS->hash == MHASH_FAILED) {
      THIS->hash = NULL;
      error("Failed to initialize hash.\n");
    }
  }
  pop_n_elems(args);
}

/* Hash id -> name */
void f_query_name(INT32 args)
{
  char *name;
  if(args == 1) {
    if(sp[-args].type != T_INT) {
      error("Invalid argument 1. Expected integer.\n");
    } 
    name = mhash_get_hash_name(sp[-args].u.integer);
    pop_n_elems(args);
    if(name == NULL) {
      push_int(0);
    } else {
      push_text(name);
      free(name);
    }
  } else {
    error("Invalid number of arguments to Mhash.Hash()->set_type, expected 1.\n");
  }
}


static struct program *hash_program;
static void free_hash_storage(struct object *o)
{
  free_hash();
}

static void init_hash_storage(struct object *o)
{
  MEMSET(THIS, 0, sizeof(mhash_storage));
  THIS->type = -1;
}

/* Init the module */
void pike_module_init(void)
{
  start_new_program();
  ADD_STORAGE( mhash_storage  );
  add_function("create", f_hash_create, "function(int|void:void)", 0 ); 
  add_function("update", f_hash_feed,   "function(string:void)", 0 ); 
  add_function("feed", f_hash_feed,     "function(string:void)", 0 ); 
  add_function("digest", f_hash_digest, "function(void:string)", 0 ); 
  add_function("hexdigest", f_hash_hexdigest, "function(void:string)", 0 ); 
  add_function("name", f_hash_name,     "function(void:string)", 0 ); 
  add_function("reset", f_hash_reset,   "function(void:void)", 0 ); 
  add_function("set_type", f_hash_set_type, "function(void:void)", 0 ); 
  set_init_callback(init_hash_storage);
  set_exit_callback(free_hash_storage);
  hash_program = end_program();
  add_program_constant("Hash", hash_program, 0);

  add_function("query_name", f_query_name,
	       "function(int:string)", 0 ); 
  add_integer_constant("CRC32", MHASH_CRC32, 0);
  add_integer_constant("MD5", MHASH_MD5, 0);
  add_integer_constant("SHA1", MHASH_SHA1, 0);
  add_integer_constant("HAVAL256", MHASH_HAVAL256, 0);
  add_integer_constant("RIPEMD160", MHASH_RIPEMD160, 0);
  add_integer_constant("TIGER", MHASH_TIGER, 0);
  add_integer_constant("GOST", MHASH_GOST, 0);
  add_integer_constant("CRC32B", MHASH_CRC32B, 0);
  add_integer_constant("HAVAL192", MHASH_HAVAL192, 0);
  add_integer_constant("HAVAL160", MHASH_HAVAL160, 0);
  add_integer_constant("HAVAL128", MHASH_HAVAL128, 0);
  add_integer_constant("HAVAL224", MHASH_HAVAL224, 0);
#if 0
  /* Not existing yet...  */
  add_integer_constant("SNEFRU", MHASH_SNEFRU, 0);
  add_integer_constant("MD2", MHASH_MD2, 0);
#endif
}


/* Restore and exit module */
void pike_module_exit( void )
{
  free_program(hash_program);
}

#else /* HAVE_MHASH */
void pike_module_exit( void ) { }
void pike_module_init( void ) { }
#endif /* HAVE_MHASH */

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
