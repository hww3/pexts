#ifndef PCRE_CONFIG_H
#define PCRE_CONFIG_H

@TOP@

@BOTTOM@

#if defined(HAVE_PCRE_H) && defined(HAVE_LIBPCRE)
# define HAVE_PCRE
#include <pcre.h>
#define THIS ((PCRE_Regexp *)fp->current_object->storage)

typedef struct
{
  pcre *regexp;
  pcre_extra *extra;
  struct pike_string *pattern;
} PCRE_Regexp;

#ifndef ADD_STORAGE
/* Pike 0.6 */
#define ADD_STORAGE(x) add_storage(sizeof(x))
#endif
#endif

void f_pcre_create(INT32 args);
void f_pcre_match(INT32 args);
void f_pcre_split(INT32 args);
void pike_module_init(void);
void pike_module_exit(void);
#endif
