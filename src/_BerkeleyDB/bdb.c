;/* $Id$ */
#include "global.h"
RCSID("$Id$");
#include "stralloc.h"
#include "mapping.h"
#include "pike_macros.h"
#include "module_support.h"
#include "error.h"

#include "threads.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif


void free_buf_struct(struct object *o)
{
}

/* Initialize and start module */
static struct program *parsehttp_program;
void pike_module_init( void )
{
#if 0
  
  start_new_program();
  ADD_STORAGE( buffer  );
  add_function( "append", f_buf_append,
		"function(string:int)", OPT_SIDE_EFFECT );
  add_function( "create", f_buf_create, "function(mapping,mapping:void)", 0 );
  set_exit_callback(free_buf_struct);
  parsehttp_program = end_program();
  add_program_constant("ParseHTTP", parsehttp_program, 0);
#endif
}

/* Restore and exit module */
void pike_module_exit( void )
{
#if 0
  free_program(parsehttp_program);
#endif
}

