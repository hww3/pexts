/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001, 2002 The Caudium Group
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

struct program * index_program = NULL;

/*
**! method void close()
**!
**!     Close an AVS index.
*/
/* void close(void) */
static void f_close(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->close()\n");

  if (index->init)
  {
    AVS_LOCK();
    n = avs_close(index->handle);
    AVS_UNLOCK();
    if (n != AVS_OK)
      Pike_error("Index->close(): %s\n", avs_errmsg(n));
    index->init = 0;
  }
  if (index->license != NULL) 
  {
    free(index->license);
    index->license = NULL;
  }
}

/*
**! method void open(string path)
**! method void open(string path, string mode)
**! method void open(string path, string mode, mapping params)
**!
**!     Open an AVS index.
**!
**!     To build or query the AVS index, you must first open it.
**!
**!     If you give no mode it will be opened in read-only mode.
*/
/* void open(string path, string|void mode, mapping|void params) */
static void f_open(INT32 args)
{
  avs_parameters_t params = AVS_PARAMETERS_INIT;
  char *path, *mode;
  struct mapping * m;
  struct svalue * s;
  int n;
  GET_PIKE_INDEX();
  mode = "r";
  m = NULL;
  params.license = LICENSEKEY_LIMITED;

  if (!args)
    Pike_error("Too few arguments to Index->open()\n");
  else if (args == 1)
    get_all_args("Index->open()", args, "%s", &path);
  else if (args == 2)
    get_all_args("Index->open()", args, "%s%s", &path, &mode);
  else
    get_all_args("Index->open()", args, "%s%s%m", &path, &mode, &m);

  if (m != NULL)
  {
    s = simple_mapping_string_lookup(m, "license");
    if (s != NULL)
    {
      if (s->type != T_STRING)
        Pike_error("Bad argument 3 to Index->open()\n");
      // we need to keep a local copy of the license string
      if (index->license != NULL) 
        free(index->license);
      if ((index->license = malloc(s->u.string->len + 1)) == NULL)
        Pike_error("Index->open(): can't allocate memory\n");
      memcpy(index->license, s->u.string->str, s->u.string->len);
      index->license[s->u.string->len] = '\0';
      params.license = index->license;
    }

    s = simple_mapping_string_lookup(m, "ignored_thresh");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.ignored_thresh = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "chars_before_wildcard");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.chars_before_wildcard = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "unlimited_wild_words");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.unlimited_wild_words = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "indexformat");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.indexformat = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "cache_threshold");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.cache_threshold = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "options");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.options = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "charset");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.charset = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "ntiers");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.ntiers = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "nbuckets");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 3 to Index->open()\n");
      params.nbuckets = s->u.integer;
    }
  }

  if (index->init)
    f_close(0);
  
  AVS_LOCK();
  n = avs_open(&params, path, mode, &index->handle);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->open(): %s\n", avs_errmsg(n));

  index->init = 1;
  pop_n_elems(args);
}

/*
**! method void create()
**! method void create(string path)
**! method void create(string path, string mode)
**! method void create(string path, string mode, mapping params)
**!
**!     Open an AVS index.
**!
**!     To build or query the AVS index, you must first open it.
**!
**!     If you give no mode it will be opened in read-only mode.
*/
/* void create(string|void path, string|void mode| mapping|void params) */
static void f_create(INT32 args)
{
  PIKE_INDEX->license = NULL;
  if (args)
    f_open(args);
}

/*
**! method void destroy()
**!
**!	Close an AVS index.
*/
/* void destroy(void) */
static void f_destroy(INT32 args)
{
  if (PIKE_INDEX->init)
    f_close(0);

  pop_n_elems(args);
}

/*
**! method void start_doc(string docid)
**! method void start_doc(string docid, int flags)
**!
**!	Prepares to create a new document in the index.
*/
/* void start_doc(string docid, int|void flags) */
static void f_start_doc(INT32 args)
{
  char *docid;
  INT32 n, flags;
  GET_PIKE_INDEX();

  flags = 0;

  if (!args)
    Pike_error("Too few arguments to Index->start_doc()\n");
  else if (args == 1)
    get_all_args("Index->start_doc()", args, "%s", &docid);
  else
    get_all_args("Index->start_doc()", args, "%s%i", &docid, &flags);

  AVS_LOCK();
  n = avs_startdoc(index->handle, docid, flags, &index->location);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->start_doc(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method void end_doc()
**!
**!	Terminates the sequence of calls for adding a document to the index
**!	begun by the startdoc method.
*/
/* void end_doc(void) */
static void f_end_doc(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->end_doc()\n");

  AVS_LOCK();
  n = avs_enddoc(index->handle);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->end_doc: %s\n", avs_errmsg(n));
}

/*
**! method int delete_docid(string docid)
**!
**!     Marks the specified document for deletion.
*/
/* int delete_docid(string docid) */
static void f_delete_docid(INT32 args)
{
  int count, n;
  char *docid;
  GET_PIKE_INDEX();

  get_all_args("Index->delete_docid()", args, "%s", &docid);

  AVS_LOCK();
  n = avs_deletedocid(index->handle, docid, &count);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->delete_docid: %s\n", avs_errmsg(n));

  pop_n_elems(args);
  push_int(count);
}


/*
**! method void add_words(string words)
**!
**!	Adds words to the document index.
*/
/* void add_words(string words) */
static void f_add_words(INT32 args)
{
  char *words;
  int n;
  long numwords;
  GET_PIKE_INDEX();

  get_all_args("Index->add_words()", args, "%s", &words);

  AVS_LOCK();
  n = avs_addword(index->handle, words, index->location, &numwords);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->add_words(): %s\n", avs_errmsg(n));
  index->location += numwords;

  pop_n_elems(args);
}

/*
**! method void set_parse_flags(int flags)
**!
**!     Sets avs_addword parsing options.
*/
/* void set_parse_flags(int flags) */
static void f_set_parse_flags(INT32 args)
{
  INT32 flags;
  GET_PIKE_INDEX();

  get_all_args("Index->set_parse_flags()", args, "%i", &flags);

  AVS_LOCK();
  avs_setparseflags(index->handle, flags);
  AVS_UNLOCK();
  pop_n_elems(args);
}

/*
**! method void add_literal(string word)
**!
**!     Adds a single word exactly as entered to a document index.
*/
/* void add_literal(string word) */
static void f_add_literal(INT32 args)
{
  char *word;
  int n;
  GET_PIKE_INDEX();

  get_all_args("Index->add_literal()", args, "%s", &word);

  AVS_LOCK();
  n = avs_addliteral(index->handle, word, index->location);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->add_literal(): %s\n", avs_errmsg(n));
  index->location += 1;

  pop_n_elems(args);
}

/*
**! method void add_date(int year, int month, int day)
**!
**!     Indexes the supplied date in standard format.
*/
/* void add_date(int year, int month, int day) */
static void f_add_date(INT32 args)
{
  INT32 n, year, month, day;
  GET_PIKE_INDEX();

  get_all_args("Index->add_date()", args, "%i%i%i", &year, &month, &day);

  AVS_LOCK();
  n = avs_adddate(index->handle, year, month, day, index->location);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->add_date(): %s\n", avs_errmsg(n));
  index->location += 1;

  pop_n_elems(args);
}

/*
**! method void add_field(string name, int start, int end)
**!
**!     Marks a set of locations as belonging to a field.
*/
/* void add_field(string name, int start, int end) */
static void f_add_field(INT32 args)
{
  char *name;
  int n;
  long start, end;
  GET_PIKE_INDEX();

  get_all_args("Index->add_field()", args, "%s%i%i", &name, &start, &end);

  AVS_LOCK();
  n = avs_addfield(index->handle, name, start, end);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->add_field(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method void add_field_words(string name, string words)
**!
**!     Marks a set of locations as belonging to a field.
*/
/* void add_field_words(string name, string words) */
static void f_add_field_words(INT32 args)
{
  char *name, *words;
  int n;
  long start, end, numwords;
  GET_PIKE_INDEX();

  get_all_args("Index->add_field_words()", args, "%s%s", &name, &words);

  start = index->location;
  AVS_LOCK();
  n = avs_addword(index->handle, words, index->location, &numwords);
  if (n == AVS_OK) {
    index->location += numwords;
    end = index->location;
    n = avs_addfield(index->handle, name, start, end);
  }
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->add_field_words(): %s\n", avs_errmsg(n));
  
  pop_n_elems(args);
}


/*
**! method void set_doc_date(int year, int month, int day)
**! method void set_doc_date(int year, int month, int day, int hour, int min, int sec)
**!
**!     Sets the date and time on a document being added.
*/
/* void set_doc_date(int, int, int, int|void, int|void, int|void) */
static void f_set_doc_date(INT32 args)
{
  INT32 n, year, month, day, hour, min, sec;
  GET_PIKE_INDEX();

  if (args < 3)
    Pike_error("Too few arguments to Index->set_doc_date()\n");
  else if (args == 3)
  {
    get_all_args("Index->set_doc_date()", args, "%i%i%i", &year, &month, &day);
    AVS_LOCK();
    n = avs_setdocdate(index->handle,year,month,day);
    AVS_UNLOCK();
  }
  else
  {
    get_all_args("Index->set_doc_date()", args, "%i%i%i%i%i%i", 
		 &year, &month, &day, &hour, &min, &sec);
    AVS_LOCK();
    n = avs_setdocdatetime(index->handle,year,month,day,hour,min,sec);
    AVS_UNLOCK();
  }

  if (n != AVS_OK)
    Pike_error("Index->set_doc_date(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}

/*
**! method void set_doc_data(string data)
**!
**!     Sets the document data for a document being added.
*/
/* void set_doc_data(string data) */
static void f_set_doc_data(INT32 args)
{
  struct pike_string *data;
  int n;
  GET_PIKE_INDEX();

  get_all_args("Index->set_doc_date()", args, "%S", &data);

  AVS_LOCK();
  n = avs_setdocdata(index->handle, data->str, data->len);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->set_doc_data(): %s\n", avs_errmsg(n));
}

/*
**! method int get_location()
**!
**!     Gets the next available location in the index.
*/
/* int get_location() */
static void f_get_location(INT32 args)
{
  if (args)
    Pike_error("Too many arguments to Index->get_location()\n");

  push_int(PIKE_INDEX->location);
}

/*
**! method int get_max_location()
**!
**!     Returns the current maximum location value in the index.
*/
/* int get_max_location() */
static void f_get_max_location(INT32 args)
{
  int n;
  long maxloc;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->get_max_location()\n");

  AVS_LOCK();
  n = avs_getmaxloc(index->handle, &maxloc);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->get_max_location(): %s\n", avs_errmsg(n));
  push_int(maxloc);
}

/*
**! method int get_total_docs()
**!
**!     Returns the total number of documents in the index.
*/
/* int get_total_docs() */
static void f_get_total_docs(INT32 args)
{
  int n;
  long totaldocs;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->get_total_docs()\n");

  AVS_LOCK();
  n = avs_total_docs(index->handle, &totaldocs);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->get_total_docs(): %s\n", avs_errmsg(n));
#ifdef INT64
  push_int64(totaldocs);
#else
  push_int(totaldocs);
#endif
}


/*
**! method void buildmode()
**! method void buildmode(int ntiers)
**!
**!     Optimizes your index for building or adding documents,
**!     and also sets the number of tiers.
*/
/* void buildmode() */
static void f_buildmode(INT32 args)
{
  int n, ntiers;
  GET_PIKE_INDEX();

  if (!args)
    n = avs_buildmode(index->handle);
  else
  {
    get_all_args("Index->buildmode()", args, "%i", &ntiers);
    AVS_LOCK();
    n = avs_buildmode_ex(index->handle, ntiers);
    AVS_UNLOCK();
  }

  if (n != AVS_OK)
    Pike_error("Index->buildmode(): %s\n", avs_errmsg(n));

  pop_n_elems(args);
}


/*
**! method void querymode()
**!
**!     Optimizes an index for optimum query performance.
*/
/* void querymode() */
static void f_querymode(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->querymode()\n");

  AVS_LOCK();
  n = avs_querymode(index->handle);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->querymode(): %s\n", avs_errmsg(n));
}

/*
**! method int compact()
**! method int compact(int minor)
**!
**!     Causes one level of compaction on the index. Returns 1 if more
**!	compaction is needed.
*/
/* int compact(int|void) */
static void f_compact(INT32 args)
{
  int n, more, minor;
  GET_PIKE_INDEX();

  minor = 0;
  if (args)
    get_all_args("Index->compact()", args, "%i", &minor);
  AVS_LOCK();
  if (minor)
    n = avs_compact_minor(index->handle, &more);
  else
    n = avs_compact(index->handle, &more);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->compact(): %s\n", avs_errmsg(n));
  pop_n_elems(args);
  push_int(more);
}

/*
**! method int compaction_needed()
**!
**!     Returns a non-zero value if the index needs compaction.
*/
/* int compaction_needed() */
static void f_compaction_needed(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->compaction_needed()\n");

  AVS_LOCK();
  n = avs_compactionneeded(index->handle);
  AVS_UNLOCK();
  push_int(n);
}

/*
**! method int get_index_mode()
**!
**!     Returns whether the current index is in build or query mode.
*/
/* int get_index_mode() */
static void f_get_index_mode(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->get_index_mode()\n");

  AVS_LOCK();
  n = avs_getindexmode(index->handle);
  AVS_UNLOCK();
  push_int(n);
}

/*
**! method int get_index_version()
**!
**!     Returns the current stable version number of the index.
*/
/* int get_index_version() */
static void f_get_index_version(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->get_index_version()\n");

  AVS_LOCK();
  n = avs_getindexversion(index->handle);
  AVS_UNLOCK();
  push_int(n);
}

/*
**! method void make_stable()
**!
**!     Commits any pending index updates to disk.
*/
/* int make_stable() */
static void f_make_stable(INT32 args)
{
  int n;
  GET_PIKE_INDEX();

  if (args)
    Pike_error("Too many arguments to Index->make_stable()\n");
  
  AVS_LOCK();
  n = avs_makestable(index->handle);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->make_stable(): %s\n", avs_errmsg(n));
}

/*
**! method void timer(int current)
**!
**!     Sets a timeout value for query processing.
*/
/* void timer(int) */
static void f_timer(INT32 args)
{
  unsigned long current;
  GET_PIKE_INDEX();

  get_all_args("Index->timer()", args, "%i", &current);

  AVS_LOCK();
  avs_timer(current);
  AVS_UNLOCK();
  pop_n_elems(args);
}

/*
**! method array(string) version(string|void license)
**!
**!     Returns an array to library version strings.
*/
/* array(string) version() */
/* array(string) version(string license) */
static void f_avs_version(INT32 args)
{
  int i;
  const char **version;
  char *license;
  GET_PIKE_INDEX();
  license = LICENSEKEY_LIMITED;

  if (args > 1)
    Pike_error("Too many arguments to Index->version()\n");
  else if (args == 1)
    get_all_args("Index->version()", args, "%s", &license);

  AVS_LOCK();
  version = avs_version(license);
  AVS_UNLOCK();

  i = 0;
  while(version[i])
    push_text(version[i++]);

  f_aggregate(i);
}

/*
**! method array(int) licenseinfo(string license)
**!
**!     Returns the license expiration date and document limit as an array
**!	of integers.
*/
/* array(int) licenseinfo(string license) */
static void f_licenseinfo(INT32 args)
{
  int n;
  char *license;
  time_t exp;
  long doclimit;
  GET_PIKE_INDEX();

  get_all_args("Index->licenseinfo()", args, "%s", &license);

  AVS_LOCK();
  n = avs_licenseinfo(license, &exp, &doclimit);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->licenseinfo(): %s\n", avs_errmsg(n));

  pop_n_elems(args);

  push_int((long)exp);
  push_int(doclimit);

  f_aggregate(2);
}


/*
**! method object(AVS.Search) search(int type, string query, string|void extra,
**!			mapping|void options, string|void since)
**!
**!     Searches for documents that match a query expression and the given 
**!	search parameters.
*/
/* object(AVS.Search) search(int, string, string, mapping|void, string|void) */
/* namespace conflict with the built in f_search */
static void f_avs_search(INT32 args)
{
  int n = AVS_OK, type=-1;
  char *query, *extra, *since;
  struct mapping * m;
  struct svalue * s;
  avs_options_t options;
  GET_PIKE_INDEX();

  m = NULL;
  extra = since = NULL;

  if (args < 2)
    Pike_error("Too few arguments to Index->search()\n");
  else if (args == 2)
    get_all_args("Index->search()", args, "%i%s", &type, &query);
  else if (args == 3)
    get_all_args("Index->search()", args, "%i%s%s", &type, &query, &extra);
  else if (args == 4)
    get_all_args("Index->search()", args, "%i%s%s%m", &type, &query,&extra,&m);
  else
    get_all_args("Index->search()", args, "%i%s%s%ms", &type, &query,&extra,
			&m, &since);

  avs_default_options(&options);
  if (m != NULL)
  {
    s = simple_mapping_string_lookup(m, "limit");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 4 to Index->search()\n");
      options.limit = (long)s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "timeout");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 4 to Index->search()\n");
      options.timeout = s->u.integer;
    }

    s = simple_mapping_string_lookup(m, "flags");
    if (s != NULL)
    {
      if (s->type != T_INT)
        Pike_error("Bad argument 4 to Index->search()\n");
      options.flags = s->u.integer;
    }
  }
  // simple query
#ifdef _REENTRANT
  THREADS_ALLOW();
  mt_lock(mtx);
  THREADS_DISALLOW();
#endif
  
  THREADS_ALLOW();
  if (type == SEARCH_SIMPLE)
  {
    options.flags |= AVS_OPT_FLAGS_RANK_TO_BOOL;
    n = avs_search_ex(index->handle, query, NULL, &options, since,
		      &index->tmp_search.docsfound,
		      &index->tmp_search.docsreturned,
		      &index->tmp_search.termcount,
		      &index->tmp_search.handle);
  }
  // boolean query. ranking by keywords
  else if (type == SEARCH_BOOLEAN)
  {
    options.flags &= ~AVS_OPT_FLAGS_RANK_TO_BOOL;
    n = avs_search_ex(index->handle, query, extra, &options, since,
		      &index->tmp_search.docsfound, 
		      &index->tmp_search.docsreturned,
		      &index->tmp_search.termcount,
		      &index->tmp_search.handle);
  }
  // boolean query. ranking by values (e.g. date)
  else if (type == SEARCH_RANK)
  {
    n = avs_search_genrank(index->handle, query, extra, 
			   (avs_ranksetup_t *)NULL, &options, since,
			   &index->tmp_search.docsfound,
			   &index->tmp_search.docsreturned,
			   &index->tmp_search.handle);
    index->tmp_search.termcount = 0;
  } else {
    type = -1;
  }
  THREADS_DISALLOW();

  if(type == -1) {
#ifdef _REENTRANT
    mt_unlock(mtx);
#endif
    Pike_error("Bad argument 1 to Index->search()\n");
  }
  
  if (n != AVS_OK)
    Pike_error("Index->search(): %s\n", avs_errmsg(n));

  ref_push_object(Pike_fp->current_object);
  push_object(clone_object(search_program, 1));

#ifdef _REENTRANT
  mt_unlock(mtx);
#endif
}

/*
**! method object(AVS.Count) count(string word_prefix)
**!
**!     Enumerates all the words beginning with a specified prefix in the
**!	index and, for each, how many times it occurs.
*/
/* object(AVS.Count) count(string word_prefix) */
static void f_count(INT32 args)
{
  int n;
  char *word_prefix;
  GET_PIKE_INDEX();
  
  get_all_args("Index->count()", args, "%s", &word_prefix);

  AVS_LOCK();
  n = avs_count(index->handle, word_prefix, &index->tmp_count.handle);
  AVS_UNLOCK();
  if (n != AVS_OK)
    Pike_error("Index->count(): %s\n", avs_errmsg(n));

  ref_push_object(Pike_fp->current_object);
  push_object(clone_object(count_program, 1));
}



static void init_index_priv_data(struct object *o)
{
  memset(PIKE_INDEX, 0 , sizeof(struct private_index_data));
  UPDATE_INIT();
}

static void exit_index_priv_data(struct object *o)
{
  UPDATE_EXIT();
  if (PIKE_INDEX->license != NULL) 
  {
    free(PIKE_INDEX->license);
    PIKE_INDEX->license = NULL;
  }
}

/* called by Pike to initialize the module */
void pike_module_init(void)
{
  // start building the AVS.Index program
  start_new_program();

  // request space for the per-object private data
  ADD_STORAGE(struct private_index_data);

  // add the program methods
  add_function("create", f_create, 
	"function(string|void, string|void, mapping|void:void)", ID_PUBLIC);
  add_function("open", f_open,
	"function(string, string|void, mapping|void:void)", ID_PUBLIC);
  add_function("destroy", f_destroy,
	"function(void:void)", ID_PUBLIC);
  add_function("close", f_close,
	"function(void:void)", ID_PUBLIC);
  add_function("start_doc", f_start_doc,
	"function(string, int|void:void)", ID_PUBLIC);
  add_function("end_doc", f_end_doc,
	"function(void:void)", ID_PUBLIC);
  add_function("delete_docid", f_delete_docid,
	"function(string:int)", ID_PUBLIC);
  add_function("add_words", f_add_words,
	"function(string:void)", ID_PUBLIC);
  add_function("set_parse_flags", f_set_parse_flags,
	"function(int:void)", ID_PUBLIC);
  add_function("set_doc_data", f_set_doc_data,
	"function(string:void)", ID_PUBLIC);
  add_function("add_literal", f_add_literal,
	"function(string:void)", ID_PUBLIC);
  add_function("add_date", f_add_date,
	"function(int, int, int:void)", ID_PUBLIC);
  add_function("add_field", f_add_field,
	"function(string, int, int:void)", ID_PUBLIC);
  add_function("add_field_words", f_add_field_words,
	"function(string, string:void)", ID_PUBLIC);
  add_function("set_doc_date", f_set_doc_date,
	"function(int,int,int,int|void,int|void,int|void:void)", ID_PUBLIC);
  add_function("get_location", f_get_location,
	"function(void:int)", ID_PUBLIC);
  add_function("get_max_location", f_get_max_location,
	"function(void:int)", ID_PUBLIC);
  add_function("get_total_docs", f_get_total_docs,
        "function(void:int)", ID_PUBLIC);
  add_function("buildmode", f_buildmode,
	"function(int|void:void)", ID_PUBLIC);
  add_function("querymode", f_querymode,
	"function(void:void)", ID_PUBLIC);
  add_function("compact", f_compact,
	"function(int|void:int)", ID_PUBLIC);
  add_function("compaction_needed", f_compaction_needed,
	"function(void:int)", ID_PUBLIC);
  add_function("get_index_mode", f_get_index_mode,
	"function(void:int)", ID_PUBLIC);
  add_function("get_index_version", f_get_index_version,
	"function(void:int)", ID_PUBLIC);
  add_function("make_stable", f_make_stable,
	"function(void:void)", ID_PUBLIC);
  add_function("timer", f_timer,
	"function(int:void)", ID_PUBLIC);
  add_function("version", f_avs_version,
	"function(void|string:array(string))", ID_PUBLIC);
  add_function("licenseinfo", f_licenseinfo,
	"function(string:array(int))", ID_PUBLIC);
  add_function("search", f_avs_search,
	"function(int,string,string,mapping(string:int)|void,string|void:object)", 
	ID_PUBLIC);
  add_function("count", f_count,
	"function(string:object)", ID_PUBLIC);

  // add the program constants

  // search types
  add_integer_constant("SEARCH_SIMPLE", SEARCH_SIMPLE, 0);
  add_integer_constant("SEARCH_BOOLEAN", SEARCH_BOOLEAN, 0);
  add_integer_constant("SEARCH_RANK", SEARCH_RANK, 0);

  // flags to search
  add_integer_constant("SEARCH_RANK_TO_BOOL", AVS_OPT_FLAGS_RANK_TO_BOOL, 0);
  add_integer_constant("SEARCH_NO_POS_BOOST", AVS_OPT_FLAGS_NO_POS_BOOST, 0);
  add_integer_constant("SEARCH_NO_LOGGING", AVS_OPT_FLAGS_NO_LOGGING, 0);
  add_integer_constant("SEARCH_RANK_LATEST", AVS_OPT_FLAGS_RANK_LATEST, 0);

  // index options
  add_integer_constant("OPTION_SEARCHSINCE", AVS_OPTION_SEARCHSINCE, 0);
  add_integer_constant("OPTION_RANKBYDATE", AVS_OPTION_RANKBYDATE, 0);
  add_integer_constant("OPTION_SEARCHBYDATE", AVS_OPTION_SEARCHBYDATE, 0);

  // parsing flags - setparseflags
  add_integer_constant("PARSE_NORMAL", 0, 0);
  add_integer_constant("PARSE_SGML", AVS_PARSE_SGML, 0);

  // character sets
  add_integer_constant("CHARSET_LATIN1", AVS_CHARSET_LATIN1, 0);
  add_integer_constant("CHARSET_UTF8", AVS_CHARSET_UTF8, 0);
  add_integer_constant("CHARSET_ASCII8", AVS_CHARSET_ASCII8, 0);

  // statdoc modes
  add_integer_constant("STARTDOC_NORMAL", 0, 0);
  add_integer_constant("STARTDOC_NEW", 1, 0);
  add_integer_constant("STARTDOC_REPLACE", 2, 0);
  add_integer_constant("STARTDOC_DUP", 4, 0);

  set_init_callback(init_index_priv_data);
  set_exit_callback(exit_index_priv_data);

  // finish and add the AVS.Index program
  index_program = end_program();
  add_program_constant("Index", index_program, 0);

  init_avs_search_program();
  init_avs_count_program();
#ifdef DOC_CONVERTERS
  init_avs_convert_program();
#endif
  start_new_program();
  init_avs_phrase_program();
  init_avs_spell_program();
  init_avs_stem_program();
  init_avs_thesaurus_program();
  end_class("Linguistics", 0);
}

/* called by Pike to deinitialize the module */
void pike_module_exit(void)
{
  free_program(search_program);
  free_program(count_program);
  free_program(index_program);
}
#endif
