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

#include "avs_config.h"
#ifdef HAVE_AVS_SDK
/* Pike headers */
#include "avs-pike.h"

/*
**! method void create(string,string)
**!
**!     Create an AVS.Linguistics.Stem object.
*/
/* void create(string,string)) */
static void f_create(INT32 args)
{
  char *path, *tags;
  int n;

  get_all_args("Stem->create()", args, "%s%s", &path, &tags);

  n = avsl_stem_init(path, tags, &PIKE_STEM->handle);
  if (n != AVS_OK)
    Pike_error("Stem->create():  %s: %s\n", avs_errmsg(n), 
		avsl_stem_getlasterr());

  pop_n_elems(args);
}

/*
**! method void destroy()
**!
**!	Close an AVS.Linguistics.Stem object .
*/
/* void destroy(void) */
static void f_destroy(INT32 args)
{
  int n;

  n = avsl_stem_close(PIKE_STEM->handle);
  if (n != AVS_OK)
    Pike_error("Stem->destroy():  %s: %s\n", avs_errmsg(n), 
                avsl_stem_getlasterr());

  pop_n_elems(args);
}

/*
**! method void get(string, string|void)
**!
**!	Gets words expanded from stem.
*/
/* void get(string) */
/* void get(string, string) */
static void f_get(INT32 args)
{
  char *word, *language, *buff=NULL;
  int found, returned, bufsiz, n, i, c;

  if (!args)
    Pike_error("Too few arguments to Stem->get()\n");

  language = NULL;
  if (args > 1)
    get_all_args("Stem->get()", args, "%s%s", &word, &language);
  else
    get_all_args("Stem->get()", args, "%s", &word);

  bufsiz = 128;
  do
  {
    buff = realloc(buff, bufsiz);

    if(buff == NULL) {
      Pike_error("Stem->get(): Can't allocate memory.\n");
    }

    n = avsl_stem_get(PIKE_STEM->handle, word, language, buff, 
			bufsiz, &found, &returned);
    if (n != AVS_OK)
    {
      free(buff);
      Pike_error("Stem->get():  %s: %s\n", avs_errmsg(n),
                avsl_stem_getlasterr());
    }
    bufsiz <<= 1;
  } while(found != returned &&  /* loop until done or */
	  bufsiz < 262144);    /* until buffer is 256 KB */

  pop_n_elems(args);

  for (i = 0, c = 0; i < returned; i++)
  {
    n = strlen(&(buff[c]));
    push_string(make_shared_binary_string(&(buff[c]), n++));
    c += n;
  }

  free(buff);

  f_aggregate(returned);
}

void init_avs_stem_program(void)
{
  // start building the AVS.Linguistics.Stem program
  start_new_program();

  // request space for the per-object private data
  ADD_STORAGE(struct private_stem_data);

  // add the program methods
  add_function("create", f_create, 
	"function(string,string:void)", ID_PUBLIC);
  add_function("destroy", f_destroy, 
	"function(void:void)", ID_PUBLIC);
  add_function("get", f_get, 
	"function(string,string|void:array(string))", ID_PUBLIC);

  // finish and add the AVS.Linguistics.Stem program
  end_class("Stem", 0);
}
#endif
