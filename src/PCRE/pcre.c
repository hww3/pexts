/* $Id$ */
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



#if 0
static int preg_get_backref(const char *walk, int *backref)
{
	if (*walk && *walk >= '0' && *walk <= '9')
		*backref = *walk - '0';
	else
		return 0;
	
	if (walk[1] && walk[1] >= '0' && walk[1] <= '9')
		*backref = *backref * 10 + walk[1] - '0';
	
	return 1;	
}


char *php_pcre_replace(char *regex,   int regex_len,
					   char *subject, int subject_len,
					   char *replace, int replace_len,
					   int  *result_len, int limit)
{
	pcre			*re = NULL;			/* Compiled regular expression */
	pcre_extra		*extra = NULL;		/* Holds results of studying */
	int			 	 exoptions = 0;		/* Execution options */
	int			 	 preg_options = 0;	/* Custom preg options */
	int			 	 count = 0;			/* Count of matched subpatterns */
	int			 	*offsets;			/* Array of subpattern offsets */
	int			 	 size_offsets;		/* Size of the offsets array */
	int				 new_len;			/* Length of needed storage */
	int				 alloc_len;			/* Actual allocated length */
	int				 eval_result_len=0;	/* Length of the eval'ed string */
	int				 match_len;			/* Length of the current match */
	int				 backref;			/* Backreference number */
	int				 eval;				/* If the replacement string should be eval'ed */
	int				 start_offset;		/* Where the new search starts */
	int				 g_notempty = 0;	/* If the match should not be empty */
	char			*result,			/* Result of replacement */
					*new_buf,			/* Temporary buffer for re-allocation */
					*walkbuf,			/* Location of current replacement in the result */
					*walk,				/* Used to walk the replacement string */
					*match,				/* The current match */
					*piece,				/* The current piece of subject */
					*replace_end,		/* End of replacement string */
					*eval_result;		/* Result of eval */

	/* Compile regex or get it from cache. */
	if ((re = pcre_get_compiled_regex(regex, extra, &preg_options)) == NULL) {
		return NULL;
	}
	
	/* Calculate the size of the offsets array, and allocate memory for it. */
	size_offsets = (pcre_info(re, NULL, NULL) + 1) * 3;
	offsets = (int *)emalloc(size_offsets * sizeof(int));
	
	alloc_len = 2 * subject_len + 1;
	result = emalloc(alloc_len * sizeof(char));
	if (!result) {
		zend_error(E_WARNING, "Unable to allocate memory in pcre_replace");
		efree(re);
		efree(offsets);
		return NULL;
	}

	/* Initialize */
	match = NULL;
	*result_len = 0;
	start_offset = 0;
	replace_end = replace + replace_len;
	eval = preg_options & PREG_REPLACE_EVAL;
	
	while (1) {
		/* Execute the regular expression. */
		count = pcre_exec(re, extra, subject, subject_len, start_offset,
						  exoptions|g_notempty, offsets, size_offsets);
		
		/* Check for too many substrings condition. */
		if (count == 0) {
			zend_error(E_NOTICE, "Matched, but too many substrings\n");
			count = size_offsets/3;
		}

		piece = subject + start_offset;

		if (count > 0 && (limit == -1 || limit > 0)) {
			/* Set the match location in subject */
			match = subject + offsets[0];

			new_len = *result_len + offsets[0] - start_offset; /* part before the match */
			
			/* If evaluating, do it and add the return string's length */
			if (eval) {
				eval_result_len = preg_do_eval(replace, replace_len, subject,
											   offsets, count, &eval_result);
				new_len += eval_result_len;
			} else { /* do regular substitution */
				walk = replace;
				while (walk < replace_end)
					if ('\\' == *walk && preg_get_backref(walk+1, &backref) && backref < count) {
						new_len += offsets[(backref<<1)+1] - offsets[backref<<1];
						walk += (backref > 9) ? 3 : 2;
					} else {
						new_len++;
						walk++;
					}
			}

			if (new_len + 1 > alloc_len) {
				alloc_len = 1 + alloc_len + 2 * new_len;
				new_buf = emalloc(alloc_len);
				memcpy(new_buf, result, *result_len);
				efree(result);
				result = new_buf;
			}
			/* copy the part of the string before the match */
			memcpy(&result[*result_len], piece, match-piece);
			*result_len += match-piece;

			/* copy replacement and backrefs */
			walkbuf = result + *result_len;
			
			/* If evaluating, copy result to the buffer and clean up */
			if (eval) {
				memcpy(walkbuf, eval_result, eval_result_len);
				*result_len += eval_result_len;
				efree(eval_result);
			} else { /* do regular backreference copying */
				walk = replace;
				while (walk < replace_end)
					if ('\\' == *walk && preg_get_backref(walk+1, &backref) && backref < count) {
						match_len = offsets[(backref<<1)+1] - offsets[backref<<1];
						memcpy(walkbuf, subject + offsets[backref<<1], match_len);
						walkbuf += match_len;
						walk += (backref > 9) ? 3 : 2;
					} else {
						*walkbuf++ = *walk++;
					}
				*walkbuf = '\0';
				/* increment the result length by how much we've added to the string */
				*result_len += walkbuf - (result + *result_len);

				if (limit != -1)
					limit--;
			}
		} else { /* Failed to match */
			/* If we previously set PCRE_NOTEMPTY after a null match,
			   this is not necessarily the end. We need to advance
			   the start offset, and continue. Fudge the offset values
			   to achieve this, unless we're already at the end of the string. */
			if (g_notempty != 0 && start_offset < subject_len) {
				offsets[0] = start_offset;
				offsets[1] = start_offset + 1;
				memcpy(&result[*result_len], piece, 1);
				(*result_len)++;
			} else {
				new_len = *result_len + subject_len - start_offset;
				if (new_len + 1 > alloc_len) {
					alloc_len = new_len + 1; /* now we know exactly how long it is */
					new_buf = emalloc(alloc_len * sizeof(char));
					memcpy(new_buf, result, *result_len);
					efree(result);
					result = new_buf;
				}
				/* stick that last bit of string on our output */
				memcpy(&result[*result_len], piece, subject_len - start_offset);
				*result_len += subject_len - start_offset;
				result[*result_len] = '\0';
				break;
			}
		}
			
		/* If we have matched an empty string, mimic what Perl's /g options does.
		   This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
		   the match again at the same point. If this fails (picked up above) we
		   advance to the next character. */
		g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;
		
		/* Advance to the next piece. */
		start_offset = offsets[1];
	}
	
	efree(offsets);

	return result;
}


static char *php_replace_in_subject(zval *regex, zval *replace, zval **subject, int *result_len, int limit)
{
	zval		**regex_entry,
				**replace_entry = NULL;
	char		*replace_value = NULL,
				*subject_value,
				*result;
	int			 subject_len,
				 replace_len = 0;

	/* Make sure we're dealing with strings. */	
	convert_to_string_ex(subject);
	
	/* If regex is an array */
	if (regex->type == IS_ARRAY) {
		/* Duplicate subject string for repeated replacement */
		subject_value = estrndup((*subject)->value.str.val, (*subject)->value.str.len);
		subject_len = (*subject)->value.str.len;
		
		zend_hash_internal_pointer_reset(regex->value.ht);

		if (replace->type == IS_ARRAY) {
			zend_hash_internal_pointer_reset(replace->value.ht);
		}
		else {
			/* Set replacement value to the passed one */
			replace_value = replace->value.str.val;
			replace_len = replace->value.str.len;
		}

		/* For each entry in the regex array, get the entry */
		while (zend_hash_get_current_data(regex->value.ht,
										  (void **)&regex_entry) == SUCCESS) {
			/* Make sure we're dealing with strings. */	
			convert_to_string_ex(regex_entry);
		
			/* If replace is an array */
			if (replace->type == IS_ARRAY) {
				/* Get current entry */
				if (zend_hash_get_current_data(replace->value.ht, (void **)&replace_entry) == SUCCESS) {
					/* Make sure we're dealing with strings. */	
					convert_to_string_ex(replace_entry);
					
					/* Set replacement value to the one we got from array */
					replace_value = (*replace_entry)->value.str.val;
					replace_len = (*replace_entry)->value.str.len;

					zend_hash_move_forward(replace->value.ht);
				} else {
					/* We've run out of replacement strings, so use an empty one */
					replace_value = empty_string;
					replace_len = 0;
				}
			}
			
			/* Do the actual replacement and put the result back into subject_value
			   for further replacements. */
			if ((result = php_pcre_replace((*regex_entry)->value.str.val,
										   (*regex_entry)->value.str.len,
										   subject_value,
										   subject_len,
										   replace_value,
										   replace_len,
										   result_len,
										   limit)) != NULL) {
				efree(subject_value);
				subject_value = result;
				subject_len = *result_len;
			}
			
			zend_hash_move_forward(regex->value.ht);
		}

		return subject_value;
	} else {
		result = php_pcre_replace(regex->value.str.val,
								  regex->value.str.len,
							      (*subject)->value.str.val,
								  (*subject)->value.str.len,
							      replace->value.str.val,
								  replace->value.str.len,
								  result_len,
								  limit);
		return result;
	}
}


/* {{{ proto string preg_replace(string|array regex, string|array replace, string|array subject [, int limit])
   Perform Perl-style regular expression replacement. */
PHP_FUNCTION(preg_replace)
{
	zval		   **regex,
				   **replace,
				   **subject,
				   **limit,
				   **subject_entry;
	char			*result;
	int				 result_len;
	int				 limit_val = -1;
	char			*string_key;
	ulong			 num_key;
	
	/* Get function parameters and do error-checking. */
	if (ZEND_NUM_ARGS() < 3 || ZEND_NUM_ARGS() > 4 ||
		zend_get_parameters_ex(ZEND_NUM_ARGS(), &regex, &replace, &subject, &limit) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	SEPARATE_ZVAL(regex);
	SEPARATE_ZVAL(replace);
	SEPARATE_ZVAL(subject);

	if (ZEND_NUM_ARGS() > 3) {
		convert_to_long_ex(limit);
		limit_val = Z_LVAL_PP(limit);
	}
		
	/* Make sure we're dealing with strings and do the replacement */
	if ((*regex)->type != IS_ARRAY) {
		convert_to_string_ex(regex);
		convert_to_string_ex(replace);
	} else if ((*replace)->type != IS_ARRAY)
		convert_to_string_ex(replace);
	
	/* if subject is an array */
	if ((*subject)->type == IS_ARRAY) {
		array_init(return_value);
		zend_hash_internal_pointer_reset((*subject)->value.ht);

		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		while (zend_hash_get_current_data((*subject)->value.ht,
										  (void **)&subject_entry) == SUCCESS) {
			if ((result = php_replace_in_subject(*regex, *replace, subject_entry, &result_len, limit_val)) != NULL) {
				/* Add to return array */
				switch(zend_hash_get_current_key((*subject)->value.ht, &string_key, &num_key))
				{
					case HASH_KEY_IS_STRING:
						add_assoc_stringl(return_value, string_key, result, result_len, 0);
						efree(string_key);
						break;

					case HASH_KEY_IS_LONG:
						add_index_stringl(return_value, num_key, result, result_len, 0);
						break;
				}
			}
		
			zend_hash_move_forward((*subject)->value.ht);
		}
	}
	else {	/* if subject is not an array */
		if ((result = php_replace_in_subject(*regex, *replace, subject, &result_len, limit_val)) != NULL) {
			RETVAL_STRINGL(result, result_len, 0);
		}
	}	
}
/* }}} */


/* {{{ proto array preg_split(string pattern, string subject [, int limit [, int flags]]) 
   Split string into an array using a perl-style regular expression as a delimiter */
PHP_FUNCTION(preg_split)
{
	zval		   **regex,				/* Regular expression to split by */
				   **subject,			/* Subject string to split */
				   **limit,				/* Number of pieces to return */
				   **flags;
	pcre			*re = NULL;			/* Compiled regular expression */
	pcre_extra		*extra = NULL;		/* Holds results of studying */
	int			 	*offsets;			/* Array of subpattern offsets */
	int			 	 size_offsets;		/* Size of the offsets array */
	int				 exoptions = 0;		/* Execution options */
	int			 	 preg_options = 0;	/* Custom preg options */
	int				 argc;				/* Argument count */
	int				 limit_val;			/* Integer value of limit */
	int				 no_empty = 0;		/* If NO_EMPTY flag is set */
	int				 count = 0;			/* Count of matched subpatterns */
	int				 start_offset;		/* Where the new search starts */
	int				 g_notempty = 0;	/* If the match should not be empty */
	char			*match,				/* The current match */
					*last_match;		/* Location of last match */

	/* Get function parameters and do error checking */	
	argc = ZEND_NUM_ARGS();
	if (argc < 1 || argc > 4 || zend_get_parameters_ex(argc, &regex, &subject, &limit, &flags) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	if (argc == 3) {
		convert_to_long_ex(limit);
		limit_val = (*limit)->value.lval;
	}
	else
		limit_val = -1;
	
	if (argc == 4) {
		convert_to_long_ex(flags);
		no_empty = (*flags)->value.lval & PREG_SPLIT_NO_EMPTY;
	}
	
	/* Make sure we're dealing with strings */
	convert_to_string_ex(regex);
	convert_to_string_ex(subject);
	
	/* Compile regex or get it from cache. */
	if ((re = pcre_get_compiled_regex((*regex)->value.str.val, extra, &preg_options)) == NULL) {
		RETURN_FALSE;
	}
	
	/* Initialize return value */
	array_init(return_value);

	/* Calculate the size of the offsets array, and allocate memory for it. */
	size_offsets = (pcre_info(re, NULL, NULL) + 1) * 3;
	offsets = (int *)emalloc(size_offsets * sizeof(int));
	
	/* Start at the beginning of the string */
	start_offset = 0;
	last_match = (*subject)->value.str.val;
	match = NULL;
	
	/* Get next piece if no limit or limit not yet reached and something matched*/
	while ((limit_val == -1 || limit_val > 1)) {
		count = pcre_exec(re, extra, (*subject)->value.str.val,
						  (*subject)->value.str.len, start_offset,
						  exoptions|g_notempty, offsets, size_offsets);

		/* Check for too many substrings condition. */
		if (count == 0) {
			zend_error(E_NOTICE, "Matched, but too many substrings\n");
			count = size_offsets/3;
		}
				
		/* If something matched */
		if (count > 0) {
			match = (*subject)->value.str.val + offsets[0];

			if (!no_empty || &(*subject)->value.str.val[offsets[0]] != last_match)
				/* Add the piece to the return value */
				add_next_index_stringl(return_value, last_match,
									   &(*subject)->value.str.val[offsets[0]]-last_match, 1);
			
			last_match = &(*subject)->value.str.val[offsets[1]];
			
			/* One less left to do */
			if (limit_val != -1)
				limit_val--;
		} else { /* Failed to match */
			/* If we previously set PCRE_NOTEMPTY after a null match,
			   this is not necessarily the end. We need to advance
			   the start offset, and continue. Fudge the offset values
			   to achieve this, unless we're already at the end of the string. */
			if (g_notempty != 0 && start_offset < (*subject)->value.str.len) {
				offsets[0] = start_offset;
				offsets[1] = start_offset + 1;
			} else
				break;
		}

		/* If we have matched an empty string, mimic what Perl's /g options does.
		   This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
		   the match again at the same point. If this fails (picked up above) we
		   advance to the next character. */
		g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;
		
		/* Advance to the position right after the last full match */
		start_offset = offsets[1];
	}
	
	if (!no_empty || start_offset != (*subject)->value.str.len)
		/* Add the last piece to the return value */
		add_next_index_string(return_value,
							  &(*subject)->value.str.val[start_offset], 1);
	
	/* Clean up */
	efree(offsets);
}
/* }}} */


/* {{{ proto string preg_quote(string str, string delim_char)
   Quote regular expression characters plus an optional character */
PHP_FUNCTION(preg_quote)
{
	zval    **in_str_arg;	/* Input string argument */
	zval	**delim;		/* Additional delimiter argument */
	char 	*in_str,		/* Input string */
	        *in_str_end,    /* End of the input string */
			*out_str,		/* Output string with quoted characters */
		 	*p,				/* Iterator for input string */
			*q,				/* Iterator for output string */
			 delim_char,	/* Delimiter character to be quoted */
		 	 c;				/* Current character */
	zend_bool quote_delim = 0; /* Whether to quote additional delim char */
	
	/* Get the arguments and check for errors */
	if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 2 ||
		zend_get_parameters_ex(ZEND_NUM_ARGS(), &in_str_arg, &delim) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	/* Make sure we're working with strings */
	convert_to_string_ex(in_str_arg);
	in_str = (*in_str_arg)->value.str.val;
	in_str_end = (*in_str_arg)->value.str.val + (*in_str_arg)->value.str.len;

	/* Nothing to do if we got an empty string */
	if (in_str == in_str_end) {
		RETVAL_STRINGL(empty_string, 0, 0);
	}

	if (ZEND_NUM_ARGS() == 2) {
		convert_to_string_ex(delim);
		if (Z_STRLEN_PP(delim) > 0) {
			delim_char = Z_STRVAL_PP(delim)[0];
			quote_delim = 1;
		}
	}
	
	/* Allocate enough memory so that even if each character
	   is quoted, we won't run out of room */
	out_str = emalloc(2 * (*in_str_arg)->value.str.len + 1);
	
	/* Go through the string and quote necessary characters */
	for(p = in_str, q = out_str; p != in_str_end; p++) {
		c = *p;
		switch(c) {
			case '.':
			case '\\':
			case '+':
			case '*':
			case '?':
			case '[':
			case '^':
			case ']':
			case '$':
			case '(':
			case ')':
			case '{':
			case '}':
			case '=':
			case '!':
			case '>':
			case '<':
			case '|':
			case ':':
				*q++ = '\\';
				*q++ = c;
				break;

			default:
				if (quote_delim && c == delim_char)
					*q++ = '\\';
				*q++ = c;
				break;
		}
	}
	*q = '\0';
	
	/* Reallocate string and return it */
	RETVAL_STRINGL(erealloc(out_str, q - out_str + 1), q - out_str, 0);
}
/* }}} */


/* {{{ proto array preg_grep(string regex, array input)
   Searches array and returns entries which match regex */
PHP_FUNCTION(preg_grep)
{
  zval		   **regex,				/* Regular expression */
    **input,				/* Input array */
    **entry;				/* An entry in the input array */
  pcre			*re = NULL;			/* Compiled regular expression */
  pcre_extra		*extra = NULL;		/* Holds results of studying */
  int			 	 preg_options = 0;	/* Custom preg options */
  int			 	*offsets;			/* Array of subpattern offsets */
  int			 	 size_offsets;		/* Size of the offsets array */
  int			 	 count = 0;			/* Count of matched subpatterns */
  char			*string_key;
  ulong			 num_key;
	
  /* Get arguments and do error checking */
	
  if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(ZEND_NUM_ARGS(), &regex, &input) == FAILURE) {
    WRONG_PARAM_COUNT;
  }
	
  if ((*input)->type != IS_ARRAY) {
    zend_error(E_WARNING, "Secong argument to preg_grep() should be an array");
    return;
  }

  SEPARATE_ZVAL(input);
	
  /* Make sure regex is a string */
  convert_to_string_ex(regex);
	
  /* Compile regex or get it from cache. */
  if ((re = pcre_get_compiled_regex((*regex)->value.str.val, extra, &preg_options)) == NULL) {
    RETURN_FALSE;
  }

  /* Calculate the size of the offsets array, and allocate memory for it. */
  size_offsets = (pcre_info(re, NULL, NULL) + 1) * 3;
  offsets = (int *)emalloc(size_offsets * sizeof(int));
	
  /* Initialize return array */
  array_init(return_value);

  /* Go through the input array */
  zend_hash_internal_pointer_reset((*input)->value.ht);
  while(zend_hash_get_current_data((*input)->value.ht, (void **)&entry) == SUCCESS) {

    convert_to_string_ex(entry);

    /* Perform the match */
    count = pcre_exec(re, extra, (*entry)->value.str.val,
		      (*entry)->value.str.len, 0,
		      0, offsets, size_offsets);

    /* Check for too many substrings condition. */
    if (count == 0) {
      zend_error(E_NOTICE, "Matched, but too many substrings\n");
      count = size_offsets/3;
    }

    /* If something matched */
    if (count > 0) {
      (*entry)->refcount++;

      /* Add to return array */
      switch(zend_hash_get_current_key((*input)->value.ht, &string_key, &num_key))
      {
      case HASH_KEY_IS_STRING:
	zend_hash_update(return_value->value.ht, string_key,
			 strlen(string_key)+1, entry, sizeof(zval *), NULL);
	efree(string_key);
	break;

      case HASH_KEY_IS_LONG:
	zend_hash_next_index_insert(return_value->value.ht, entry,
				    sizeof(zval *), NULL);
	break;
      }
    }
		
    zend_hash_move_forward((*input)->value.ht);
  }
	
  /* Clean up */
  efree(offsets);
}
#endif

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
