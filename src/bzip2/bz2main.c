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
 * Glue for the bzip2 compression library v1.0.x
 *
 * $Id$
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

/*
 * Pike includes
 */
#include "stralloc.h"
#include "pike_macros.h"
#include "module_support.h"
#include "program.h"
#include "error.h"
#include "threads.h"
#include "array.h"
#include "pike_types.h"
#include "interpret.h"

#include "bz2_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_BZLIB_H
#include <bzlib.h>
#endif

#include <errno.h>
#include <string.h>

#define ARG(_n_) sp[-(args - _n_)]

#ifdef HAVE_BZLIB_H
static struct program   *inflate_program;
static struct program   *deflate_program;

typedef struct
{
    bz_stream   *stream;
    int         blkSize;
} BZSTRUCT;

#define THIS ((BZSTRUCT*)fp->current_storage)

/* Inflate class */
static void
f_inflate_create(INT32 args)
{
    if (args == 1) {
	if (ARG(0).type != T_INT)
	    error("bzip2.inflate->create(): argument must be of type INT\n");
	    
	THIS->blkSize = ARG(0).u.integer != 0;
    } else if (args > 1) {
	error("bzip2.inflate->create(): expected 1 argument of type INT.\n");
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
	if (ARG(0).type != T_STRING || ARG(0).u.string->size_shift > 0)
	    error("bzip2.inflate->inflate(): argument 1 must be an 8-bit string\n");
	if (!ARG(0).u.string->str || !strlen(ARG(0).u.string->str))
	    error("bzip2.inflate->inflate(): cannot decompress an empty string!\n");

	src = ARG(0).u.string;
    } else if (args != 1) {
	error("bzip2.inflate->inflate(): expected exactly one argument of type STRING.\n");
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
	error("bzip2.inflate->inflate(): out of memory (needed %u bytes)\n",
	      dlen);
	      
     retval = BZ2_bzBuffToBuffDecompress(dest,
                                         &dlen,
					 src->str,
					 src->len,
					 THIS->blkSize,
					 0);
					 
    switch(retval) {
	case BZ_CONFIG_ERROR:
	    error("bzip2.inflate->inflate(): your copy of libbz2 is seriously damaged!\n");
	    break; /* never reached */
	    
	case BZ_MEM_ERROR:
	    error("bzip2.inflate->inflate(): out of memory decompressing block.\n");
	    break; /* never reached */
	    
	case BZ_OUTBUFF_FULL:
	    if (dest)
		free(dest);
	    dlen <<= 1; /* double it */
	    goto destalloc;
	    break; /* never reached */	
	    
	case BZ_DATA_ERROR:
	    error("bzip2.inflate->inflate(): data integrity error in compressed data\n");
	    break;
	    
	case BZ_DATA_ERROR_MAGIC:
	    error("bzip2.inflate->inflate(): wrong compressed data magic number\n");
	    break;
	    
	case BZ_UNEXPECTED_EOF:
	    error("bzip2.inflate->inflate(): data ends unexpectedly\n");
	    break;
	    
	case BZ_OK:
	    break;
	    
	default:
	    error("bzip2.inflate->inflate(): unknown error code %d\n", retval);
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
	error("Cannot allocate memory for compression structures\n");
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
	if (ARG(0).type != T_INT)
	    error("bzip2.deflate->create(): argument must be of type INT\n");
	    
	if ((ARG(0).u.integer < 0) && (ARG(0).u.integer > 9))
	    error("bzip2.deflate->create(): argument 1 must be between 0 and 9\n");
	    
	THIS->blkSize = ARG(0).u.integer;
    } else if (args > 1) {
	error("bzip2.deflate->create(): expected 1 argument of type INT.\n");
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
      if(ARG(1).type != T_INT) {
	  error("bzip2.deflate->deflate(): argument 2 not an integer.\n");
      }
      verbosity = ARG(1).u.integer;
      if( verbosity > 4 || verbosity < 0 ) {
	error("bzip2.deflate->deflate(): verbosity should be between 0 and 4.\n");
      }
      /* FALLTHROUGH */

     case 1:
      if (ARG(0).type != T_STRING)
	  error("bzip2.deflate->deflate(): argument 1 must be a string.\n");
      if (!ARG(0).u.string->str || !ARG(0).u.string->len)
	  error("bzip2.deflate->deflate(): cannot compress an empty string!\n");
      src = ARG(0).u.string;
      break;
     default:
      error("bzip2.deflate->deflate(): expected  1 to 2 arguments.\n");
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
	error("bzip2.deflate->deflate(): out of memory (needed %u bytes)\n",
	      dlen);
	
    retval = BZ2_bzBuffToBuffCompress(dest,
                                      &dlen,
				      src->str,
				      src->len << src->size_shift,
				      THIS->blkSize,
				      verbosity, 0);
    switch(retval) {
     case BZ_CONFIG_ERROR:
      error("bzip2.deflate->deflate(): your copy of libbz2 is seriously damaged!\n");
      break; /* never reached */
	    
     case BZ_MEM_ERROR:
      error("bzip2.deflate->deflate(): out of memory compressing block.\n");
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
      error("bzip2.deflate->deflate(): Invalid parameters.\n");
      break;
      
     default:
      error("bzip2.deflate->deflate(): unknown error code %d\n", retval);
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
	error("Cannot allocate memory for compression structures\n");
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
{}

void pike_module_exit(void)
{}
#endif
