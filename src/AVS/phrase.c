/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001 The Caudium Group
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
**! method void create(string)
**!
**!     Create an AVS.Linguistics.Phrase object.
*/
/* void create(string)) */
static void f_create(INT32 args)
{
  char *configfile;
  int n;
  PIKE_PHRASE->handle = NULL;
  if (!args)
    Pike_error("Too few arguments to Phrase->create()\n");

  get_all_args("Phrase->create()", args, "%s", &configfile);

  n = avsl_phrase_init(configfile, &PIKE_PHRASE->handle);
  if (n != AVS_OK)
    Pike_error("Phrase->create():  %s: %s\n", avs_errmsg(n), 
		avsl_phrase_getlasterr());

  pop_n_elems(args);
}

/*
**! method void destroy()
**!
**!	Close an AVS.Linguistics.Phrase object .
*/
/* void destroy(void) */
static void f_destroy(INT32 args)
{
  int n;
  if(PIKE_PHRASE->handle != NULL) {
    n = avsl_phrase_close(PIKE_PHRASE->handle);
    if (n != AVS_OK)
      Pike_error("Phrase->destroy():  %s: %s\n", avs_errmsg(n), 
		 avsl_phrase_getlasterr());
  }
  pop_n_elems(args);
}

/*
**! method void get(string, string|void)
**!
**!     Gets phrases from the query string.
*/
/* void get(string) */
/* void get(string, string) */
static void f_get(INT32 args)
{
  char *query, *language, *buff = NULL;
  int found, returned, bufsiz, n, i, c;

  if (!args)
    Pike_error("Too few arguments to Phrase->get()\n");

  language = NULL;
  if (args > 1)
    get_all_args("Phrase->get()", args, "%s%s", &query, &language);
  else
    get_all_args("Phrase->get()", args, "%s", &query);

  bufsiz = 256;
  do
  {
    buff = realloc(buff, bufsiz);
    if (buff == NULL)
      Pike_error("Phrase->get(): Can't allocate memory.\n");

    n = avsl_phrase_get(PIKE_PHRASE->handle, query, language, buff, 
			bufsiz, &found, &returned);
    if (n != AVS_OK)
    {
      free(buff);
      Pike_error("Phrase->get():  %s: %s\n", avs_errmsg(n),
                avsl_phrase_getlasterr());
    }

    bufsiz <<= 1;
  } while(found != returned);

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

void init_avs_phrase_program(void)
{
  // start building the AVS.Linguistics.Phrase program
  start_new_program();

  // request space for the per-object private data
  ADD_STORAGE(struct private_phrase_data);

  // add the program methods
  add_function("create", f_create, 
	"function(string:void)", ID_PUBLIC);
  add_function("destroy", f_destroy, 
	"function(void:void)", ID_PUBLIC);
  add_function("get", f_get, 
	"function(string,string|void:array(string))", ID_PUBLIC);

  // finish and add the AVS.Linguistics.Phrase program
  end_class("Phrase", 0);
}
#endif
