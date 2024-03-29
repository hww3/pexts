/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000-2005 The Caudium Group
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
 * Version: MPL 1.1/LGPL 2.1
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
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of the LGPL, and not to allow others to use your version
 * of this file under the terms of the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL or the LGPL.
 *
 * Significant Contributors to this file are:
 *
 */

/*
 *
 * Glue for the bzip2 compression library v1.0.x
 *
 * $Id$
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "bz2_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_BZLIB_H
#include <bzlib.h>
#endif

#include <errno.h>
#include <string.h>

#ifdef HAVE_BZLIB_H
static struct program   *inflate_program;
static struct program   *deflate_program;

typedef struct
{
    bz_stream   *stream;
    int         blkSize;
} BZSTRUCT;

#define THIS ((BZSTRUCT*)Pike_fp->current_storage)

/* Inflate class */
static void
f_inflate_create(INT32 args)
{
    if (args == 1) {
	if (ARG(1).type != T_INT)
	    Pike_error("bzip2.inflate->create(): argument must be of type INT\n");
	    
	THIS->blkSize = ARG(1).u.integer != 0;
    } else if (args > 1) {
	Pike_error("bzip2.inflate->create(): expected 1 argument of type INT.\n");
    } else
	THIS->blkSize = 0;
	
    pop_n_elems(args);
}

static void
f_inflate_inflate(INT32 args)
{
    struct pike_string  *src;
    char                *dest;
    unsigned            dlen;
    struct pike_string  *retstr;
    int                 retval;
    
    if (args == 1) {
	if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
	    Pike_error("bzip2.inflate->inflate(): argument 1 must be an 8-bit string\n");
	if (!ARG(1).u.string->str || !strlen(ARG(1).u.string->str))
	    Pike_error("bzip2.inflate->inflate(): cannot decompress an empty string!\n");

	src = ARG(1).u.string;
    } else if (args != 1) {
	Pike_error("bzip2.inflate->inflate(): expected exactly one argument of type STRING.\n");
    }

    /*
     * Let's assume the compression ratio was 50%
     * and create the destination buffer that many
     * bytes long
     */
     dlen = src->len + 1;
     dlen <<= 1;
     
destalloc:
     dest = (char*)calloc(dlen, sizeof(char));
     if (!dest)
	Pike_error("bzip2.inflate->inflate(): out of memory (needed %u bytes)\n",
	      dlen);
	      
#ifdef HAVE_OLD_LIBBZ2
     retval = bzBuffToBuffDecompress(dest, &dlen, src->str, src->len,
				     THIS->blkSize, 0);
#else
     retval = BZ2_bzBuffToBuffDecompress(dest, &dlen, src->str, src->len,
					 THIS->blkSize, 0);
#endif
					 
    switch(retval) {
#ifdef BZ_CONFIG_ERROR
	case BZ_CONFIG_ERROR:
	    Pike_error("bzip2.inflate->inflate(): your copy of libbz2 is seriously damaged!\n");
	    break; /* never reached */
#endif
	case BZ_MEM_ERROR:
	    Pike_error("bzip2.inflate->inflate(): out of memory decompressing block.\n");
	    break; /* never reached */
	    
	case BZ_OUTBUFF_FULL:
	    if (dest)
		free(dest);
	    dlen <<= 1; /* double it */
	    goto destalloc;
	    break; /* never reached */	
	    
	case BZ_DATA_ERROR:
	    Pike_error("bzip2.inflate->inflate(): data integrity error in compressed data\n");
	    break;
	    
	case BZ_DATA_ERROR_MAGIC:
	    Pike_error("bzip2.inflate->inflate(): wrong compressed data magic number\n");
	    break;
	    
	case BZ_UNEXPECTED_EOF:
	    Pike_error("bzip2.inflate->inflate(): data ends unexpectedly\n");
	    break;
	    
	case BZ_OK:
	    break;
	    
	default:
	    Pike_error("bzip2.inflate->inflate(): unknown error code %d\n", retval);
	    break; /* never reached */
    }
    
    pop_n_elems(args);
    if (dest) {
	retstr = make_shared_binary_string(dest, dlen);
	free(dest);
	push_string(retstr);
    } else
	push_int(0);    
}

static void
init_inflate(struct object *o)
{
    THIS->stream = (bz_stream*)malloc(sizeof(bz_stream));
    if (!THIS->stream)
	Pike_error("Cannot allocate memory for compression structures\n");
    memset(THIS->stream, 0, sizeof(bz_stream));
    THIS->blkSize = 0;
}

static void
exit_inflate(struct object *o)
{
    if (THIS->stream)
	free(THIS->stream);
}

/* Deflate class */
static void
f_deflate_create(INT32 args)
{
    if (args == 1) {
	if (ARG(1).type != T_INT)
	    Pike_error("bzip2.deflate->create(): argument must be of type INT\n");
	    
	if ((ARG(1).u.integer < 0) && (ARG(1).u.integer > 9))
	    Pike_error("bzip2.deflate->create(): argument 1 must be between 0 and 9\n");
	    
	THIS->blkSize = ARG(1).u.integer;
    } else if (args > 1) {
	Pike_error("bzip2.deflate->create(): expected 1 argument of type INT.\n");
    } else
	THIS->blkSize = 9;
    pop_n_elems(args);
}

/*
 * This is the Gz module-compatible STRING compressor
 * Takes a string to compress and returns a compressed string.
 * Allows an optional INT argument for Gz.deflate compat, but
 * ignores it.
 *
 * It uses a very high-level libbz2 call for simplicity.
 */
static void
f_deflate_deflate(INT32 args)
{
    char               *dest;
    unsigned           dlen;
    struct pike_string *src;
    int                retval;
    struct pike_string *retstr;
    int                verbosity = 0;
    switch(args) {
     case 2:
      if(ARG(2).type != T_INT) {
	  Pike_error("bzip2.deflate->deflate(): argument 2 not an integer.\n");
      }
      verbosity = ARG(2).u.integer;
      if( verbosity > 4 || verbosity < 0 ) {
	Pike_error("bzip2.deflate->deflate(): verbosity should be between 0 and 4.\n");
      }
      /* FALLTHROUGH */

     case 1:
      if (ARG(1).type != T_STRING)
	  Pike_error("bzip2.deflate->deflate(): argument 1 must be a string.\n");
      if (!ARG(1).u.string->str || !ARG(1).u.string->len)
	  Pike_error("bzip2.deflate->deflate(): cannot compress an empty string!\n");
      src = ARG(1).u.string;
      break;
     default:
      Pike_error("bzip2.deflate->deflate(): expected  1 to 2 arguments.\n");
    }
    
    /*
     * We assume the worst case when the destination string doesn't compress
     * and will instead grow. The assumption is that it can grow by 1/3 of the
     * source string. We also add an extra 40 bytes since that is what the
     * minimum size seems to be.
     */
    dlen = (src->len << src->size_shift) + 1;
    dlen += dlen / 3 + 40;
    
destalloc: /* Yep, I know. goto's are ugly. But efficient. :P */
    dest = (char*)calloc(dlen, sizeof(char));
    if (!dest)
	Pike_error("bzip2.deflate->deflate(): out of memory (needed %u bytes)\n",
	      dlen);
	
#ifdef HAVE_OLD_LIBBZ2
    retval = bzBuffToBuffCompress(dest, &dlen, src->str,
				  src->len << src->size_shift,
				  THIS->blkSize, verbosity, 0);
#else
    retval = BZ2_bzBuffToBuffCompress(dest, &dlen, src->str,
				      src->len << src->size_shift,
				      THIS->blkSize, verbosity, 0);
#endif
    switch(retval) {
#ifdef BZ_CONFIG_ERROR
     case BZ_CONFIG_ERROR:
      Pike_error("bzip2.deflate->deflate(): your copy of libbz2 is seriously damaged!\n");
      break; /* never reached */
#endif	    
     case BZ_MEM_ERROR:
      Pike_error("bzip2.deflate->deflate(): out of memory compressing block.\n");
      break; /* never reached */
	    
     case BZ_OUTBUFF_FULL:
      if (dest)
	free(dest);
      dlen <<= 1;
      goto destalloc;
      break; /* never reached */
	    
     case BZ_OK:
      break;

     case BZ_PARAM_ERROR:
      Pike_error("bzip2.deflate->deflate(): Invalid parameters.\n");
      break;
      
     default:
      Pike_error("bzip2.deflate->deflate(): unknown error code %d\n", retval);
      break; /* never reached */
    }
    
    pop_n_elems(args);
    if (dest) {
	retstr = make_shared_binary_string(dest, dlen);
	free(dest);
	push_string(retstr);
    } else
	push_int(0);
}

/*
 * This function implements file compression.
 * Takes input and output (optional) file paths.
 * If no output file is given, the name is created
 * by appending .bz2 to the input path.
 */
static void
f_deflate_file(INT32 args)
{
  pop_n_elems(args);
}

static void
init_deflate(struct object *o)
{
    THIS->stream = (bz_stream*)malloc(sizeof(bz_stream));
    if (!THIS->stream)
	Pike_error("Cannot allocate memory for compression structures\n");
    memset(THIS->stream, 0, sizeof(bz_stream));
    THIS->blkSize = 0;
}

static void
exit_deflate(struct object *o)
{
    if (THIS->stream)
	free(THIS->stream);
}

void pike_module_init(void)
{
  #ifdef PEXTS_VERSION
  pexts_init();
#endif

    /* Compression program */
    start_new_program();
    ADD_STORAGE(bz_stream);
    
    set_init_callback(init_deflate);
    set_exit_callback(exit_deflate);

    ADD_FUNCTION("create", f_deflate_create, 
                 tFunc( tOr(tVoid, tInt), tVoid), 0);
    ADD_FUNCTION("deflate", f_deflate_deflate,
		 tFunc(tString tOr(tInt, tVoid), tString), 0);
    ADD_FUNCTION("compress_file", f_deflate_file,
		 tFunc(tString tOr(tString, tVoid), tVoid), 0);

    deflate_program = end_program();
    add_program_constant("deflate", deflate_program, 0);
    
    /* Decompression program */
    start_new_program();
    ADD_STORAGE(bz_stream);

    ADD_FUNCTION("create", f_inflate_create, 
                 tFunc(tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("inflate", f_inflate_inflate,
		 tFunc(tString, tString), 0);
		 
    set_init_callback(init_inflate);
    set_exit_callback(exit_inflate);
    
    inflate_program = end_program();
    add_program_constant("inflate", inflate_program, 0);
}

void pike_module_exit(void)
{
  free_program(inflate_program);
  free_program(deflate_program);
}
#else /* !HAVE_BZLIB_H */
void pike_module_init(void)
{
  #ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif
