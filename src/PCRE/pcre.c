#include "global.h"
RCSID("$Id$");

#include "stralloc.h"
#include "pike_macros.h"
#include "module_support.h"
#include "program.h"
#include "error.h"
#include "threads.h"
#include "array.h"
#include "pcre_config.h"

#ifdef HAVE_PCRE 
#include <stdio.h>
#include <fcntl.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif


/* Create a new PCRE regular expression and compile
 * the passed pattern.
 */

void f_pcre_create(INT32 args)
{
  struct pike_string *regexp; /* Regexp pattern */
  struct svalue *opt_sval;    /* Option string svalue */
  pcre_extra *extra = NULL;   /* result from study, if enabled */
  pcre *re = NULL;            /* compiled regexp */
  int opts = 0;           /* Regexp compile options */
  const char *errmsg;          /* Error message pointer */
  int erroffset;              /* Error offset */
  int do_study = 0;           /* Study the regexp when it's compiled */
  char *pp;                   /* Temporary char pointer */
  unsigned const char *table = NULL; /* Translation table */
#if HAVE_SETLOCALE
  char *locale = setlocale(LC_CTYPE, NULL); /* Get current locale for
					     * translation table. */
#endif
  get_all_args("PCRE.Regexp->create", args, "%S%*", &regexp, &opt_sval);
  switch(opt_sval->type) {
  case T_STRING:
    pp = opt_sval->u.string->str;
    while(*pp) {
      switch (*pp++) {
	/* Perl compatible options */
      case 'i':	opts |= PCRE_CASELESS;  break;
      case 'm':	opts |= PCRE_MULTILINE; break;
      case 's':	opts |= PCRE_DOTALL;	break;
      case 'x':	opts |= PCRE_EXTENDED;	break;
	
	/* PCRE specific options */
      case 'A':	opts |= PCRE_ANCHORED;	     break;
      case 'D':	opts |= PCRE_DOLLAR_ENDONLY; break;
      case 'S':	do_study  = 1;		     break;
      case 'U':	opts |= PCRE_UNGREEDY;	     break;
      case 'X':	opts |= PCRE_EXTRA;	     break;
	
      case ' ': case '\n':
	break;
	
      default:
	error("PCRE.Regexp->create: Unknown option modifier '%c'.\n", pp[-1]);
      }
    }
    break;
  case T_INT:
    if(opt_sval->u.integer == 0) {
      break;
    }
    /* Fallthrough */
  default:
    error("Bad argument 2 to PCRE.Regexp->create() - expected string.\n");
    break;
  }
  
#if HAVE_SETLOCALE
  if (strcmp(locale, "C"))
    table = pcre_maketables();
#endif

  /* Compile the pattern and handle errors */
  THREADS_ALLOW();
  re = pcre_compile(regexp->str, opts, &errmsg, &erroffset, table);
  THREADS_DISALLOW();
  if(re == NULL) {
    error("Failed to compile regexp: %s at offset %d\n", errmsg, erroffset);
  }

  /* If study option was specified, study the pattern and
     store the result in extra for passing to pcre_exec. */
  if (do_study) {
    THREADS_ALLOW();
    extra = pcre_study(re, 0, &errmsg);
    THREADS_DISALLOW();
    if (errmsg != NULL) {
      error("Error while studying pattern: %s", errmsg);
    }
  }
  THIS->regexp = re;
  THIS->extra = extra;
  THIS->pattern = regexp;
  add_ref(regexp);
  pop_n_elems(args);
}

/* Do a regular expression match */
void f_pcre_match(INT32 args) 
{
  struct pike_string *data; /* Data to match */
  struct svalue *opt_sval;  /* Options string svalue */
  pcre_extra *extra = NULL;   /* result from study, if enabled */
  pcre *re = NULL;            /* compiled regexp */
  char *pp;                 /* Pointer... */
  int opts = 0;             /* Match options */
  int is_match;             /* Did it match? */
  switch(args)
  {
  case 2:
    opt_sval = &sp[-2];
    switch(opt_sval->type) {
    case T_STRING:
      pp = opt_sval->u.string->str;
      while(*pp) {
	switch (*pp++) {
	case 'A':	opts |= PCRE_ANCHORED; break;
	case 'B':	opts |= PCRE_NOTBOL;   break;
	case 'L':	opts |= PCRE_NOTEOL;   break;
	case 'E':	opts |= PCRE_NOTEMPTY; break;
	case ' ': case '\n':
	  break;
	default:
	  error("PCRE.Regexp->match(): Unknown option modifier '%c'.\n", pp[-1]);
	}
      }
      break;
    case T_INT:
      if(opt_sval->u.integer == 0) {
	break;
      }
      /* Fallthrough */
    default:
      error("Bad argument 2 to PCRE.Regexp->match() - expected string.\n");
      break;
    }
    /* Fall through */
  case 1:
    if(sp[-1].type != T_STRING || sp[-1].u.string->size_shift > 0) {
      error("PCRE.Regexp->match(): Invalid argument 1. Expected 8-bit string.\n");
    }
    data = sp[-1].u.string;
    break;
  default:
    error("PCRE.Regexp->match(): Invalid number of arguments. Expected 1 or 2.\n");
  }
  re = THIS->regexp;
  extra = THIS->extra;

  //  THREADS_ALLOW();
  
  /* Do the pattern matching */
  is_match = pcre_exec(re, extra, data->str, data->len, 0,
		       opts, NULL, 0);
  //  THREADS_DISALLOW();
  pop_n_elems(args);
  switch(is_match) {
  case PCRE_ERROR_NOMATCH:   push_int(0);  break;
  case PCRE_ERROR_NULL:      error("Invalid argumens passed to pcre_exec.\n");
  case PCRE_ERROR_BADOPTION: error("Invalid options sent to pcre_exec.\n");
  case PCRE_ERROR_BADMAGIC:  error("Invalid magic number.\n");
  case PCRE_ERROR_UNKNOWN_NODE: error("Unknown node encountered. PCRE bug or memory error.\n");
  case PCRE_ERROR_NOMEMORY:  error("Out of memory during execution.\n");
  default:
    push_int(1); /* A match! */
    break; 
  }
}

void f_pcre_split(INT32 args) 
{
  struct array *arr;	    /* Result array */ 
  struct pike_string *data; /* Data to split */
  struct svalue *opt_sval;  /* Options string svalue */
  pcre_extra *extra = NULL; /* result from study, if enabled */
  pcre *re = NULL;          /* compiled regexp */
  char *pp;                 /* Pointer... */
  int opts = 0;             /* Match options */
  int *ovector, ovecsize;   /* Subpattern storage */
  int ret;                  /* Result codes */
  int i, e;                 /* Counter variable */
  get_all_args("PCRE.Regexp->split", args, "%S", &data);
  switch(args) {
  case 2:
    opt_sval = &sp[-2];
    switch(opt_sval->type) {
    case T_STRING:
      pp = opt_sval->u.string->str;
      while(*pp) {
	switch (*pp++) {
	case 'A':	opts |= PCRE_ANCHORED; break;
	case 'B':	opts |= PCRE_NOTBOL;   break;
	case 'L':	opts |= PCRE_NOTEOL;   break;
	case 'E':	opts |= PCRE_NOTEMPTY; break;
	case ' ': case '\n':
	  break;
	default:
	  error("PCRE.Regexp->split(): Unknown option modifier '%c'.\n", pp[-1]);
	}
      }
      break;
    case T_INT:
      if(opt_sval->u.integer == 0) {
	break;
      }
      /* Fallthrough */
    default:
      error("Bad argument 2 to PCRE.Regexp->split() - expected string.\n");
      break;
    }
    /* Fallthrough */
  case 1:
    if(sp[-1].type != T_STRING || sp[-1].u.string->size_shift > 0) {
      error("PCRE.Regexp->match(): Invalid argument 1. Expected 8-bit string.\n");
    }
    data = sp[-1].u.string;
    break;
  default:
    error("PCRE.Regexp->match(): Invalid number of arguments. Expected 1 or 2.\n");    
  }
  re = THIS->regexp;
  extra = THIS->extra;

  /* Calculate the size of the offsets array, and allocate memory for it. */
  pcre_fullinfo(re, extra, PCRE_INFO_CAPTURECOUNT, &ovecsize);
  ovecsize = (ovecsize + 1) * 3;
  ovector = (int *)alloca(ovecsize * sizeof(int));
  if(ovector == NULL)
    error("PCRE.Regexp->split(): Out of memory.\n");

  /* Do the pattern matching */
  //  THREADS_ALLOW();
  ret = pcre_exec(re, extra, data->str, data->len, 0,
		       opts, ovector, ovecsize);
  //  THREADS_DISALLOW();
  switch(ret) {
  case PCRE_ERROR_NOMATCH:   pop_n_elems(args); push_int(0);  break;
  case PCRE_ERROR_NULL:      error("Invalid argumens passed to pcre_exec.\n");
  case PCRE_ERROR_BADOPTION: error("Invalid options sent to pcre_exec.\n");
  case PCRE_ERROR_BADMAGIC:  error("Invalid magic number.\n");
  case PCRE_ERROR_UNKNOWN_NODE: error("Unknown node encountered. PCRE bug or memory error.\n");
  case PCRE_ERROR_NOMEMORY:  error("Out of memory during execution.\n");
  default:
    e = ret * 2;
    for (i = 0; i < e ; i += 2) {
      push_string(make_shared_binary_string(data->str + ovector[i],
					    (int)(ovector[i+1] - ovector[i])));
      
    }
    arr = aggregate_array(ret);
    pop_n_elems(args);
    push_array(arr);
    break;
  }
  //  free(ovector);
}

static struct program *pcre_regexp_program;
static void free_regexp(struct object *o)
{
  if(THIS->pattern) {
    free_string(THIS->pattern);
  }
  if(THIS->regexp) {
    pcre_free(THIS->regexp);
  }
  if(THIS->extra) {
    pcre_free(THIS->extra);    
  }
  MEMSET(THIS, 0, sizeof(PCRE_Regexp));
}

static void init_regexp(struct object *o)
{
  MEMSET(THIS, 0, sizeof(PCRE_Regexp));
}

/* Init the module */
void pike_module_init(void)
{
  start_new_program();
  ADD_STORAGE( PCRE_Regexp  );
  add_function( "create", f_pcre_create,
		"function(string,string|void:void)", 0 ); 
  add_function("match", f_pcre_match,
	       "function(string,string|void:int)", 0 ); 
  add_function("split", f_pcre_split,
	       "function(string,string|void:array(string))", 0 ); 
  set_init_callback(init_regexp);
  set_exit_callback(free_regexp);
  pcre_regexp_program = end_program();
  add_program_constant("Regexp", pcre_regexp_program, 0);
}


/* Restore and exit module */
void pike_module_exit( void )
{
  free_program(pcre_regexp_program);
}


#endif /* HAVE_PCRE */

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
