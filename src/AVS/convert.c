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
**! method void init()
**! method void init(string psdict)
**!
**!     Create an AVS convert result object.
*/
/* void init(string|void psdict) */
static void f_init(INT32 args)
{
  int n;
  struct avs_convert_params params;

  if (args > 1)
    Pike_error("Too many arguments to Convert->init()\n");

  memset(&params, 0 , sizeof(struct avs_convert_params));
  if (args)
    get_all_args("Convert->init()", args, "%s", &params.cvtpath);

  n = avs_convert_init(&params);
  if (n != AVS_OK)
    Pike_error("Convert->init(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method void file2html(string from, string to)
**!
**!	Converts a document to an HTML file.
*/
/* void file2html(string from, string to) */
static void f_file2html(INT32 args)
{
  int n, err;
  char *from, *to;

  get_all_args("Convert->file2html()", args, "%s%s", &from, &to);

  n = avs_convert_file2html(from, to, &err);
  if (n != AVS_OK)
  {
    if (err)
      Pike_error("Search->file2html(): %s: %s\n", avs_errmsg(n), avs_cvterrmsg(err));
    else
      Pike_error("Search->file2html(): %s\n", avs_errmsg(n));
  }

  pop_n_elems(args);
}

/*
**! method void file2text(string from, string to)
**!
**!     Converts a document to a text file.
*/
/* void file2text(string from, string to) */
static void f_file2text(INT32 args)
{
  int n, err;
  char *from, *to;

  get_all_args("Convert->file2text()", args, "%s%s", &from, &to);

  n = avs_convert_file2text(from, to, &err);
  if (n != AVS_OK)
  {
    if (err)
      Pike_error("Search->file2text(): %s: %s\n", avs_errmsg(n), avs_cvterrmsg(err));    else
      Pike_error("Search->file2text(): %s\n", avs_errmsg(n));
  }

  pop_n_elems(args);
}

void init_avs_convert_program(void)
{
  // start building the AVS.Convert program
  start_new_program();

  // add the program methods
  add_function_constant("init", f_init, 
	"function(string|void:void)", ID_PUBLIC);
  add_function_constant("file2html", f_file2html, 
	"function(string,string:void)", ID_PUBLIC);
  add_function_constant("file2text", f_file2text, 
	"function(string,string:void)", ID_PUBLIC);

  // finish and add the AVS.Search program
  end_class("Convert", 0);
}
#endif
