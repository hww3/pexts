#ifndef __XSLT_H
#define __XSLT_H

#include <malloc.h>

/* This allows execution of c-code that requires the Pike interpreter to 
 * be locked from the Sablotron callback functions.
 */
#if defined(PIKE_THREADS) && defined(_REENTRANT)
#define THREAD_SAFE_RUN(COMMAND)  do {\
  struct thread_state *state;\
 if((state = thread_state_for_id(th_self()))!=NULL) {\
    if(!state->swapped) {\
      COMMAND;\
    } else {\
      mt_lock(&interpreter_lock);\
      SWAP_IN_THREAD(state);\
      COMMAND;\
      SWAP_OUT_THREAD(state);\
      mt_unlock(&interpreter_lock);\
    }\
  }\
} while(0)
#else
#define THREAD_SAFE_RUN(COMMAND) COMMAND
#endif


static ptrdiff_t Stylesheet_storage_offset;

#define THIS ((xslt_storage *)Pike_interpreter.frame_pointer->current_storage)
#define THAT ((xslt_storage *)Pike_interpreter.frame_pointer->current_storage)
/*#define THAT ((stylesheet_storage *)(Pike_interpreter.frame_pointer->current_storage))
 */

typedef struct
{
    struct pike_string      *xml;
    struct pike_string *base_uri;
    struct pike_string *encoding;
    struct pike_string  *err_str;
    struct pike_string      *xsl;

    struct svalue* match_include;
    struct svalue*  open_include;
    struct svalue*  read_include;
    struct svalue* close_include;
    int iPosition; // position inside the current file
    struct object* file; // the object returned from pike
    
    xsltStylesheetPtr stylesheet;
    
    struct mapping *variables;  
    struct mapping *err;
    struct pike_string *language;
    char *content_type, *charset;
} xslt_storage;

typedef struct
{
    xsltStylesheetPtr stylesheet;
    struct pike_string *err_str;
    struct svalue* match_include;
    struct svalue*  open_include;
    struct svalue*  read_include;
    struct svalue* close_include;
    int iPosition; // position inside the current file
    struct object* file; // the object returned from pike
    struct pike_string *language;
} stylesheet_storage;


#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#define MY_MAPPING_LOOP(md, COUNT, KEY) \
  for(COUNT=0;COUNT < md->hashsize; COUNT++ ) \
	for(KEY=md->hash[COUNT];KEY;KEY=KEY->next)
#else
/* Pike 7.x and newer */
#define MY_MAPPING_LOOP(md, COUNT, KEY) \
  for(COUNT=0;COUNT < md->data->hashsize; COUNT++ ) \
	for(KEY=md->data->hash[COUNT];KEY;KEY=KEY->next)
#endif

static void f_set_xml_data(INT32 args); 
static void f_set_xml_file(INT32 args); 
static void f_set_variables(INT32 args); 
static void f_set_base_uri(INT32 args); 
static void f_parse( INT32 args );
static void f_create( INT32 args );
static void f_create_stylesheets( INT32 args );
static void f_content_type( INT32 args );
static void f_charset( INT32 args );
static void f_set_include_callbacks( INT32 args );
static void f_get_version(INT32 args);
static void f_create_stylesheet(INT32 args);
static void f_set_language(INT32 args);
static int f_include_read(void* context, char* buffer, int len);

static int _include_match(const char* filename);
static void* _include_open(const char* filename);
static int _include_read(void* context, char* buffer, int len);
static int _include_close(void *context);

static void xml_error(void* ctx, const char* msg, ...);
static void xsl_error(void* ctx, const char* msg, ...);
#endif /* __XSLT_H */
