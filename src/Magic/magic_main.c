/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2004 The Caudium Group
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
 * $Id$
 */

/*
 * File licensing and authorship information block.
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *
 * Marek Habersack <grendel@caudium.net>
 *
 * Portions created by the Initial Developer are Copyright (C) Marek Habersack
 * & The Caudium Group. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Significant Contributors to this file are:
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "magic_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_LIBMAGIC
#ifdef HAVE_MAGIC_H
#include <magic.h>
#endif

typedef struct 
{
  magic_t    cookie;
} MAGIC_STORAGE;

#define THIS ((MAGIC_STORAGE*)get_storage(Pike_fp->current_object, magic_program))

static struct program   *magic_program;

/* Glue */
/*
 * void create(void|int flags, void|int forceclose);
 * void open(void|int flags, void|int forceclose);
 *
 * Create/open the connection to the library/database. 'flags' can be an
 * OR-ed mix of the following values:
 *
 *  MAGIC_NONE      No special handling.
 *
 *  MAGIC_DEBUG     Print debugging messages to stderr.
 *
 *  MAGIC_SYMLINK   If the file queried is a symlink, follow it.
 *
 *  MAGIC_COMPRESS  If the file is compressed, unpack it and look at the
 *                  contents.
 *
 *  MAGIC_DEVICES   If the file is a block or character special device,
 *                  then open the device and try to look in its contents.
 *
 *  MAGIC_MIME      Return a mime string, instead of a textual
 *                  description.
 *
 *  MAGIC_CONTINUE  Return all matches, not just the first.
 *
 *  MAGIC_CHECK     Check the magic database for consistency and print
 *                  warnings to stderr.
 *
 *  MAGIC_PRESERVE_ATIME
 *                  On systems that support utime(2) or utimes(2), attempt
 *                  to preserve the access time of files analyzed.
 *
 *  MAGIC_RAW       Don't translate unprintable characters to a \ooo octal
 *                  representation.
 *
 *  MAGIC_ERROR     Treat operating system errors while trying to open
 *                  files and follow symlinks as real errors, instead of
 *                  printing them in the magic buffer.
 *
 * 'forceclose' forces the method to close the existing connection to the
 * database before attempting to open a new one.
 *
 * Note: after calling this method you MUST call the 'load' method in order
 * to point the code to the right file. See the 'load' documentation for
 * details.
 */
static void f_magic_open(INT32 args)
{
  int   flags = 0, forceclose = 1;

  switch (args) {
      case 2:
        get_all_args("open", args, "%i%i", &flags, &forceclose);
        break;

      case 1:
        get_all_args("open", args, "%i", &flags);
        break;

      case 0:
        break;
        
      default:
        Pike_error("Wrong number of arguments");
  }

  if (THIS->cookie && !forceclose)
    Pike_error("The database is open. Close it first.");
  else if (THIS->cookie && forceclose)
    magic_close(THIS->cookie);
  
  THIS->cookie = magic_open(flags);
  if (!THIS->cookie)
    Pike_error("Error allocating the magic cookie");

  magic_compile(THIS->cookie, NULL);
  pop_n_elems(args);
}

/*
 * void close();
 *
 * This one will catch you by surprise. It closes the open connection to
 * the database.
 */
static void f_magic_close(INT32 args)
{
  if (THIS->cookie) {
    magic_close(THIS->cookie);
    THIS->cookie = NULL;
  }
  pop_n_elems(args);
}

/*
 * string error();
 *
 * Returns the string representation of the last magic error (if anything)
 */
static void f_magic_error(INT32 args)
{
  char   *err;

  pop_n_elems(args);
  if (!THIS->cookie)
    return;

  err = (char*)magic_error(THIS->cookie);
  if (err)
    push_text(err);
  else
    push_int(0);
}

/*
 * int errno();
 *
 * Similar to 'error' above, but returns the errno returned from the last
 * system call issued by the library.
 */
static void f_magic_errno(INT32 args)
{
  pop_n_elems(args);
  if (!THIS->cookie)
    push_int(-1);
  else
    push_int(magic_errno(THIS->cookie));
}

/*
 * string|int file(void|string path);
 *
 * This is the reason for which the glue exists. Checks the file type and
 * returns either a mime string corresponding to the type (if MAGIC_MIME
 * flag was set in call to 'open', 'create' or 'setflags') or a
 * human-readable description of the file type.
 *
 * If the 'path' parameter is empty, this method will read the data from
 * stdin.
 *
 * Returns either the file type as described above, or an integer if an
 * error happens.
 */
static void f_magic_file(INT32 args)
{
  char    *filename = NULL;
  char    *ret;

  if (args)
    get_all_args("file", args, "%s", &filename);
  
  if (!THIS->cookie) {
    pop_n_elems(args);
    push_int(-1);
    return;
  }
  
  ret = (char*)magic_file(THIS->cookie, filename);
  pop_n_elems(args);
  if (!ret)
    push_int(-1);
  else
    push_text(ret);
}

/*
 * string|int buffer(string buf);
 *
 * The same function as 'file' but the data is passed in a buffer which is
 * required in this case.
 *
 * Returns the same stuff what 'file'
 */
static void f_magic_buffer(INT32 args)
{
  struct pike_string   *buffer;
  char                 *ret;
  
  get_all_args("buffer", args, "%S", &buffer);
  if (!THIS->cookie) {
    pop_n_elems(args);
    push_int(-1);
    return;
  }

  ret = (char*)magic_buffer(THIS->cookie, buffer->str, buffer->len);
  pop_n_elems(args);
  if (!ret)
    push_int(-1);
  else
    push_text(ret);
}

/*
 * int setflags(int flags);
 *
 * Set the flags described in the 'open' documentation.
 *
 * Returns the old flags.
 */
static void f_magic_setflags(INT32 args)
{
  int    flags;
  
  get_all_args("setflags", args, "%i", &flags);
  if (!THIS->cookie) {
    pop_n_elems(args);
    push_int(-1);
    return;
  }

  flags = magic_setflags(THIS->cookie, flags);
  pop_n_elems(args);
  push_int(flags);
}

/*
 * int check(void|string files);
 *
 * Can be used to check the validity of entries in the colon separated
 * database files passed in as 'files', or void for the default
 * database. It returns 0 on success and -1 on failure.
 */
static void f_magic_check(INT32 args)
{
  int    ret;
  char  *filename = NULL;

  if (args)
    get_all_args("check", args, "%s", &filename);
  
  if (!THIS->cookie) {
    pop_n_elems(args);
    push_int(-1);
    return;
  }

  ret = magic_check(THIS->cookie, filename);
  pop_n_elems(args);
  push_int(ret);
}

/*
 * int compile(void|string files);
 *
 * Can be used to compile the the colon separated list of database files
 * passed in as 'files', or void for the default database. It returns 0 on
 * success and -1 on failure. The compiled files created are named from the
 * basename of each file argument with ".mgc" appended to it.
 */
static void f_magic_compile(INT32 args)
{
  int    ret;
  char  *filename = NULL;

  if (args)
    get_all_args("compile", args, "%s", &filename);
  if (!THIS->cookie) {
    pop_n_elems(args);
    push_int(-1);
    return;
  }

  ret = magic_compile(THIS->cookie, filename);
  pop_n_elems(args);
  push_int(ret);
}

/*
 * int load(void|string files);
 *
 * must be used to load the the colon separated list of database files
 * passed in as 'files', or void for the default database file before any
 * magic queries can performed.
 * The default database file is named by the MAGIC environment variable.
 * If that variable is not set, the default database file name is
 * /usr/share/misc/file/magic.
 * load() adds ".mime" and/or ".mgc" to the database filename as
 * appropriate, so do not pass the extension to the function.
 */
static void f_magic_load(INT32 args)
{
  int    ret;
  char  *filename = NULL;

  if (args)
    get_all_args("load", args, "%s", &filename);
  
  if (!THIS->cookie) {
    pop_n_elems(args);
    push_int(-1);
    return;
  }

  ret = magic_load(THIS->cookie, filename);
  pop_n_elems(args);
  push_int(ret);
}

/* Pike interface */
static void init_magic(struct object *o)
{
  THIS->cookie = NULL;
}

static void exit_magic(struct object *o)
{
  if (THIS->cookie) {
    magic_close(THIS->cookie);
    THIS->cookie = NULL;
  }
}

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif

  start_new_program();
  ADD_STORAGE(MAGIC_STORAGE);

  set_init_callback(init_magic);
  set_exit_callback(exit_magic);

  ADD_FUNCTION("create", f_magic_open,
               tFunc(tOr(tInt, tVoid) tOr(tInt, tVoid), tVoid), 0);
  ADD_FUNCTION("open", f_magic_open,
               tFunc(tOr(tInt, tVoid) tOr(tInt, tVoid), tVoid), 0);
  ADD_FUNCTION("close", f_magic_close,
               tFunc(tVoid, tVoid), 0);
  ADD_FUNCTION("error", f_magic_error,
               tFunc(tVoid, tOr(tString, tInt)), 0);
  ADD_FUNCTION("errno", f_magic_errno,
               tFunc(tVoid, tInt), 0);
  ADD_FUNCTION("file", f_magic_file,
               tFunc(tOr(tString, tVoid), tOr(tString, tInt)), 0);
  ADD_FUNCTION("buffer", f_magic_buffer,
               tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("setflags", f_magic_setflags,
               tFunc(tInt, tInt), 0);
  ADD_FUNCTION("check", f_magic_check,
               tFunc(tString, tInt), 0);
  ADD_FUNCTION("compile", f_magic_compile,
               tFunc(tOr(tVoid, tString), tInt), 0);
  ADD_FUNCTION("load", f_magic_load,
               tFunc(tString, tInt), 0);
  
  magic_program = end_program();
  add_program_constant("Magic", magic_program, 0);
    
  /* Flags */
  add_integer_constant("MAGIC_NONE", MAGIC_NONE, 0);
  add_integer_constant("MAGIC_DEBUG", MAGIC_DEBUG, 0);
  add_integer_constant("MAGIC_SYMLINK", MAGIC_SYMLINK, 0);
  add_integer_constant("MAGIC_COMPRESS", MAGIC_COMPRESS, 0);
  add_integer_constant("MAGIC_DEVICES", MAGIC_DEVICES, 0);
  add_integer_constant("MAGIC_MIME", MAGIC_MIME, 0);
  add_integer_constant("MAGIC_CONTINUE", MAGIC_CONTINUE, 0);
  add_integer_constant("MAGIC_CHECK", MAGIC_CHECK, 0);
  add_integer_constant("MAGIC_PRESERVE_ATIME", MAGIC_PRESERVE_ATIME, 0);
  add_integer_constant("MAGIC_RAW", MAGIC_RAW, 0);
  add_integer_constant("MAGIC_ERROR", MAGIC_ERROR, 0);
}

void pike_module_exit(void)
{
  free_program(magic_program);
}
#else /* !HAVE_LIBMAGIC */
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif
