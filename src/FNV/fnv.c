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

#define _GNU_SOURCE

/*
 * Glue for the FNV hash library.
 * See http://www.isthe.com/chongo/tech/comp/fnv/ for more information.
 * 
 * (c)2003 oliv3
 */

/*
 * USAGE:
 * > .FNV.fnv1_32()->hash(string)      -- compute 32bit hash
 * > .FNV.fnv1_64()->hash(string)      -- compute 64bit hash
 *
 * TODO: find out how to make sthg like:
 * > Crypto.FNV.hash32(string)
 * > Crypto.FNV.hash64(string)
 */

#include "global.h"
#include "svalue.h"
#include "interpret.h"
#include "fnv.h"
#include "bignum.h"

static void f_fnv1_32_create (INT32 args) { }

static void f_fnv1_32_do (INT32 args) {
  struct pike_string *input;
  Fnv32_t hash_val;

  if (args == 1) {
    if (Pike_sp[-1].type != T_STRING)
      Pike_error ("argument must be a string\n");
  }
  else
    Pike_error ("wrong number of arguments\n");

  input = Pike_sp[-args].u.string;

  hash_val = fnv_32_buf ((void *)input->str, input->len, FNV1_32_INIT);
   
  pop_n_elems (args);
  push_int64 (hash_val);
}

static void f_fnv1_64_create (INT32 args) { }

static void f_fnv1_64_do (INT32 args) {
  struct pike_string *input;
  Fnv64_t hash_val;

  if (args == 1) {
    if (Pike_sp[-1].type != T_STRING)
      Pike_error ("argument must be a string\n");
  }
  else
    Pike_error ("wrong number of arguments\n");

  input = Pike_sp[-args].u.string;

  hash_val = fnv_64_buf ((void *)input->str, input->len, FNV1_64_INIT);
   
  pop_n_elems (args);
  push_int64 (hash_val);
}

void pike_module_init (void) {
  /* FNV1_32 program */
  start_new_program ();
  ADD_FUNCTION("create", f_fnv1_32_create, tFunc(tVoid, tVoid), 0);
  ADD_FUNCTION("hash", f_fnv1_32_do, tFunc(tString, tInt), 0);
  end_class ("fnv1_32", 0);

  /* FNV1_64 program */
  start_new_program ();
  ADD_FUNCTION("create", f_fnv1_64_create, tFunc(tVoid, tVoid), 0);
  ADD_FUNCTION("hash", f_fnv1_64_do, tFunc(tString, tInt), 0);
  end_class ("fnv1_64", 0);
}

void pike_module_exit (void) { }
