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

#include "avs_config.h"
#ifdef HAVE_AVS_SDK
/* Pike headers */
#include "avs-pike.h"

struct program * count_program = NULL;
extern struct program *index_program;

/*
**! method void create(object(AVS.Index))
**!
**!     Create an AVS count object.
*/
/* void create(object(AVS.Index)) */
static void f_create(INT32 args)
{
  struct private_index_data *index;

  if (!args)
    Pike_error("Too few arguments to Count->create()\n");

  if ((Pike_sp[-args].type != T_OBJECT) || 
      (!(index = (struct private_index_data *)
                 get_storage(Pike_sp[-args].u.object, index_program))))
    Pike_error("Bad argument 1 to Search->create()\n");

  memcpy(PIKE_COUNT, &index->tmp_count, sizeof(struct private_count_data));

  pop_n_elems(args);
}

/*
**! method void destroy()
**!
**!	Close an AVS search.
*/
/* void destroy(void) */
static void f_destroy(INT32 args)
{
  int n;

  n = avs_count_close(PIKE_COUNT->handle);
  if (n != AVS_OK)
    Pike_error("Count->destroy(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method int get_count()
**!
**!     Retrieves the number of word occurrences corresponding to the most
**!	recent call to Count->next().
*/
/* int get_count(void) */
static void f_get_count(INT32 args)
{
  if (args)
    Pike_error("Too many arguments to Count->get_count()\n");
  push_int(avs_count_getcount(PIKE_COUNT->handle));
}

/*
**! method void next()
**!
**!     Retrieves the first or next index entry matching the word or 
**!	prefix specified to Index->count()
*/
/* void next(void) */
static void f_next(INT32 args)
{
  int n;

  if (args)
    Pike_error("Too many arguments to Count->next()\n");

  n = avs_countnext(PIKE_COUNT->handle);
  if (n != AVS_OK)
    Pike_error("Count->next(): %s\n", avs_errmsg(n));
}

/*
**! method string get_word()
**!
**!     Retrieves the word corresponding to the most recent call to next()
*/
/* string get_word(void) */
static void f_get_word(INT32 args)
{
  char *word;

  if (args)
    Pike_error("Too many arguments to Count->get_word()\n");
  word = avs_count_getword(PIKE_COUNT->handle);
  push_text(word);
}

void init_avs_count_program(void)
{
  // start building the AVS.Search program
  start_new_program();

  // request space for the per-object private data
  ADD_STORAGE(struct private_count_data);

  // add the program methods
  add_function("create", f_create, 
	"function(object:void)", ID_PUBLIC);
  add_function("destroy", f_destroy, 
	"function(void:void)", ID_PUBLIC);
  add_function("get_count", f_get_count, 
	"function(void:int)", ID_PUBLIC);
  add_function("next", f_next, 
	"function(void:void)", ID_PUBLIC);
  add_function("get_word", f_get_word, 
	"function(void:string)", ID_PUBLIC);

  // finish and add the AVS.Search program
  count_program = end_program();
  add_program_constant("Count", count_program, 0);
}
#endif
