#include "avs_config.h"
#ifdef HAVE_AVS_SDK
/* Pike headers */
#include "avs-pike.h"

/*
**! method void create(string)
**!
**!     Create an AVS.Linguistics.Thesaurus object.
*/
/* void create(string)) */
static void f_create(INT32 args)
{
  char *configfile;
  int n;
  PIKE_THESAURUS->handle = NULL;
  get_all_args("Thesaurus->create()", args, "%s", &configfile);

  n = avsl_thesaurus_init(configfile, &PIKE_THESAURUS->handle);
  if (n != AVS_OK)
    Pike_error("Thesaurus->create():  %s: %s\n", avs_errmsg(n), 
		avsl_thesaurus_getlasterr());

  pop_n_elems(args);
}

/*
**! method void destroy()
**!
**!	Close an AVS.Linguistics.Thesaurus object .
*/
/* void destroy(void) */
static void f_destroy(INT32 args)
{
  int n;
  if(PIKE_THESAURUS->handle != NULL) {
    n = avsl_thesaurus_close(PIKE_THESAURUS->handle);
    if (n != AVS_OK)
      Pike_error("Thesaurus->destroy():  %s: %s\n", avs_errmsg(n), 
		 avsl_thesaurus_getlasterr());
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
  char *word, *language, *buff = NULL;
  int found, returned, bufsiz, n, i, c;

  if (!args)
    Pike_error("Too few arguments to Thesaurus->get()\n");

  language = NULL;
  if (args > 1)
    get_all_args("Thesaurus->get()", args, "%s%s", &word, &language);
  else
    get_all_args("Thesaurus->get()", args, "%s", &word);

  bufsiz = 128;
  i = 1;
  do
  {
    buff = realloc(buff, bufsiz);
    if (buff == NULL)
      Pike_error("Thesaurus->get(): Can't allocate memory.\n");

    n = avsl_thesaurus_get(PIKE_THESAURUS->handle, word, language, buff, 
			bufsiz, &found, &returned);
    if (n != AVS_OK)
    {
      free(buff);
      Pike_error("Thesaurus->destroy():  %s: %s\n", avs_errmsg(n),
                avsl_thesaurus_getlasterr());
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

void init_avs_thesaurus_program(void)
{
  // start building the AVS.Linguistics.Thesaurus program
  start_new_program();

  // request space for the per-object private data
  ADD_STORAGE(struct private_thesaurus_data);

  // add the program methods
  add_function("create", f_create, 
	"function(string:void)", ID_PUBLIC);
  add_function("destroy", f_destroy, 
	"function(void:void)", ID_PUBLIC);
  add_function("get", f_get, 
	"function(string,string|void:array(string))", ID_PUBLIC);

  // finish and add the AVS.Linguistics.Thesaurus program
  end_class("Thesaurus", 0);
}
#endif
