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

struct program * search_program = NULL;
extern struct program *index_program;

/*
**! method void create(object(AVS.Index))
**!
**!     Create an AVS search result object.
*/
/* void create(object(AVS.Index)) */
static void f_create(INT32 args)
{
  struct private_index_data *index;
  if (!args)
    Pike_error("Too few arguments to Search->create()\n");

  if ((sp[-args].type != T_OBJECT) || 
      (!(index = (struct private_index_data *)
                 get_storage(sp[-args].u.object, index_program))))
    Pike_error("Bad argument 1 to Search->create()\n");

  memcpy(PIKE_SEARCH, &index->tmp_search, sizeof(struct private_search_data));
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
  GET_PIKE_SEARCH();
  THREADS_ALLOW();
  n = avs_search_close(search->handle);
  THREADS_DISALLOW();
  if (n != AVS_OK)
    Pike_error("Search->destroy(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method void docsfound()
**!
**!     Returns how many documents where found.
*/
/* int docsfound(void) */
static void f_docsfound(INT32 args)
{
  if (args)
    Pike_error("Too many arguments to Search->docsfound()\n");
  push_int(PIKE_SEARCH->docsfound);
}

/*
**! method void docsreturned()
**!
**!     Returns how many documents where returned.
*/
/* int docsreturned(void) */
static void f_docsreturned(INT32 args)
{
  if (args)
    Pike_error("Too many arguments to Search->docsreturned()\n");
  push_int(PIKE_SEARCH->docsreturned);
}

/*
**! method void termcount()
**!
**!     Returns the number of terms in the rank string.
*/
/* int termcount(void) */
static void f_termcount(INT32 args)
{
  if (args)
    Pike_error("Too many arguments to Search->termcount()\n");
  push_int(PIKE_SEARCH->termcount);
}

/*
**! method void get_result(int result)
**!
**!     Retrieves results of a search.
*/
/* void get_result(int result) */
static void f_get_result(INT32 args)
{
  int n, result;
  GET_PIKE_SEARCH();
  get_all_args("Search->get_result()", args, "%i", &result);

  THREADS_ALLOW();
  n = avs_getsearchresults(search->handle, result);
  THREADS_DISALLOW();
  if (n != AVS_OK)
    Pike_error("Search->get_result(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method array get_term(int term_num)
**!
**!     Retrieves one ranking term and statistics for a search.
*/
/* array get_term(int term_num) */
static void f_get_term(INT32 args)
{
  int n, term_num;
  long count;
  char *term;
  GET_PIKE_SEARCH();
  get_all_args("Search->get_term()", args, "%i", &term_num);

  THREADS_ALLOW();
  n = avs_getsearchterms(search->handle, term_num, &term, &count);
  THREADS_DISALLOW();
  if (n != AVS_OK)
    Pike_error("Search->get_term(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
  push_text(term);
  push_int(count);

  f_aggregate(2);
}

/*
**! method string get_version()
**!
**!     Retrieves a version string which defines the version of the 
**!	index used for this search.
*/
/* string get_version() */
static void f_get_version(INT32 args)
{
  int n;
  char version[AVS_SEARCHVERSION_MAXLEN]; 
  GET_PIKE_SEARCH();
  if (args)
    Pike_error("Too many arguments to Search->get_version()\n");

  THREADS_ALLOW();
  n = avs_getsearchversion(search->handle, version);
  THREADS_DISALLOW();
  if (n != AVS_OK)
    Pike_error("Search->get_version(): %s\n", avs_errmsg(n));

  push_text(version);
}

/*
**! method array get_date()
**!
**!     Returns the date associated with a search result.
*/
/* array get_date() */
static void f_get_date(INT32 args)
{
  int year, month, day;
  GET_PIKE_SEARCH();
  if (args)
    Pike_error("Too many arguments to Search->get_date()\n");

  THREADS_ALLOW();
  avs_search_getdate(search->handle, &year, &month, &day);
  THREADS_DISALLOW();

  push_int(year);
  push_int(month);
  push_int(day);

  f_aggregate(3);
}

/*
**! method string get_data()
**!
**!     Returns the data associated with a search result.
*/
/* string get_data() */
static void f_get_data(INT32 args)
{
  char *data;
  int len;
  GET_PIKE_SEARCH();
  if (args)
    Pike_error("Too many arguments to Search->get_data()\n");
  THREADS_ALLOW();
  len  = avs_search_getdatalen(search->handle);
  data = avs_search_getdata(search->handle);
  THREADS_DISALLOW();

  push_string(make_shared_binary_string(data, len));
}

/*
**! method string get_docid()
**!
**!     Returns the unique identifier associated with a search result.
*/
/* string get_docid() */
static void f_get_docid(INT32 args)
{
  char *docid;
  GET_PIKE_SEARCH();
  if (args)
    Pike_error("Too many arguments to Search->get_docid()\n");

  THREADS_ALLOW();
  docid = avs_search_getdocid(search->handle);
  THREADS_DISALLOW();

  push_text(docid);
}

/*
**! method float get_relevance()
**!
**!     Returns the relevance value associated with a search result.
*/
/* float get_relevance() */
static void f_get_relevance(INT32 args)
{
  float relevance;
  GET_PIKE_SEARCH();
  if (args)
    Pike_error("Too many arguments to Search->get_relevance()\n");

  THREADS_ALLOW();
  relevance = avs_search_getrelevance(search->handle);
  THREADS_DISALLOW();

  push_float(relevance);
}
void init_avs_search_program(void)
{
  // start building the AVS.Search program
  start_new_program();

  // request space for the per-object private data
  ADD_STORAGE(struct private_search_data);


  // add the program methods
  add_function("create", f_create, 
	"function(object:void)", ID_PUBLIC);
  add_function("destroy", f_destroy, 
	"function(void:void)", ID_PUBLIC);
  add_function("docsfound", f_docsfound, 
	"function(void:int)", ID_PUBLIC);
  add_function("docsreturned", f_docsreturned, 
	"function(void:int)", ID_PUBLIC);
  add_function("termcount", f_termcount, 
	"function(void:int)", ID_PUBLIC);
  add_function("get_result", f_get_result, 
	"function(int:void)", ID_PUBLIC);
  add_function("get_term", f_get_term,
	"function(int:array)", ID_PUBLIC);
  add_function("get_version", f_get_version,
	"function(void:string)", ID_PUBLIC);
  add_function("get_date", f_get_date,
	"function(void:array)", ID_PUBLIC);
  add_function("get_data", f_get_data,
	"function(void:string)", ID_PUBLIC);
  add_function("get_docid", f_get_docid,
	"function(void:string)", ID_PUBLIC);
  add_function("get_relevance", f_get_relevance,
	"function(void:float)", ID_PUBLIC);

  // finish and add the AVS.Search program
  search_program = end_program();
  add_program_constant("Search", search_program, 0);
}
#endif
