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

#include "caudium_util.h"
#include "avs.h"
#include "avscvt.h"
#include "avsl.h"

extern struct program *search_program;
extern struct program *count_program;

void pike_module_init(void);
void pike_module_exit(void);

extern void init_avs_search_program(void);
extern void init_avs_count_program(void);
#ifdef DOC_CONVERTERS
extern void init_avs_convert_program(void);
#endif
extern void init_avs_phrase_program(void);
extern void init_avs_spell_program(void);
extern void init_avs_stem_program(void);
extern void init_avs_thesaurus_program(void);

struct private_search_data
{
  avs_searchHdl_t handle;
  long            docsfound;
  long            docsreturned;
  long            termcount;
};

struct private_count_data
{
  avs_countsHdl_t handle;
};

struct private_index_data
{
  int		init;
  avs_idxHdl_t  handle;
  long		location;
  struct private_search_data tmp_search;
  struct private_count_data tmp_count;
  char *	license;
#ifdef _REENTRANT
  PIKE_MUTEX_T smtx;
  PIKE_MUTEX_T mtx;
#endif
};

struct private_phrase_data
{
  avsl_phraseHdl_t handle;
};

struct private_spell_data
{
  avsl_spellHdl_t handle;
};

struct private_stem_data
{
  avsl_stemHdl_t handle;
};

struct private_thesaurus_data
{
  avsl_thsrsHdl_t handle;
};


#define PIKE_INDEX     ((struct private_index_data     *)(fp->current_storage))
#define PIKE_SEARCH    ((struct private_search_data    *)(fp->current_storage))
#define PIKE_COUNT     ((struct private_count_data     *)(fp->current_storage))
#define PIKE_PHRASE    ((struct private_phrase_data    *)(fp->current_storage))
#define PIKE_SPELL     ((struct private_spell_data     *)(fp->current_storage))
#define PIKE_STEM      ((struct private_stem_data      *)(fp->current_storage))
#define PIKE_THESAURUS ((struct private_thesaurus_data *)(fp->current_storage))

#ifdef _REENTRANT
# define GET_MUTEX(X) PIKE_MUTEX_T *mtx = &(X)->mtx
#else
# define GET_MUTEX()
#endif
#define GET_PIKE_INDEX() struct private_index_data *index = PIKE_INDEX; GET_MUTEX(index)
#define GET_PIKE_SEARCH() struct private_search_data *search = PIKE_SEARCH; 

#ifdef _REENTRANT
#ifdef AVS_GLUE_DEBUG
#define DEBUG_LOCK()   fprintf(stderr, "%s:%d: Locking.\n", __FUNCTION__, __LINE__)
#define DEBUG_UNLOCK() fprintf(stderr, "%s:%d: Unlocking.\n", __FUNCTION__, __LINE__)
#else
#define DEBUG_LOCK()
#define DEBUG_UNLOCK()
#endif
# define AVS_LOCK() THREADS_ALLOW(); mt_lock(mtx); DEBUG_LOCK()
# define AVS_UNLOCK() DEBUG_UNLOCK(); mt_unlock(mtx); THREADS_DISALLOW()
# define UPDATE_INIT() mt_init(&PIKE_INDEX->mtx); 
# define UPDATE_EXIT() mt_destroy(&PIKE_INDEX->mtx); 
#else
# define AVS_LOCK() 
# define AVS_UNLOCK()
# define UPDATE_INIT() 
# define UPDATE_EXIT() 
#endif

#define SEARCH_SIMPLE	0
#define SEARCH_BOOLEAN	1
#define SEARCH_RANK	2
