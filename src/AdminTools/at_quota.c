/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000 The Caudium Group
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
 * Simple glue for more advanced Unix functions.
 * quotactl support
 */
#define _GNU_SOURCE
#define _POSIX_PTHREAD_SEMANTICS

#include "global.h"
RCSID("$Id$");

/*
 * Pike includes
 */
#include "stralloc.h"
#include "pike_macros.h"
#include "module_support.h"
#include "program.h"
#include "error.h"
#include "threads.h"
#include "array.h"

#include "at_config.h"

#ifdef HAVE_QUOTA_SUPPORT

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <mntent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/quota.h>

#include "at_common.h"
#include "at_mntent.h"

struct quota_struct
{
    struct dqblk     dq;
    struct dqstats   stats;
    char             *fs; /* which FS do we operate on? */
};

static char *_object_name = "Quota";

static struct program *quota_program;

#define THIS ((struct quota_struct*)get_storage(fp->current_object, quota_program))

/**
 ** Support functions
 **/

/* 
 * Stolen from the quota 2.0 package 
 *
 * STOLEN START :-)
 */

#define min(x,y) ((x) < (y)) ? (x) : (y)

#define CORRECT_FSTYPE(type) \
(!strcmp(type, MNTTYPE_EXT2)) || \
(!strcmp(type, MNTTYPE_EXT3)) || \
(!strcmp(type, MNTTYPE_MINIX)) || \
(!strcmp(type, MNTTYPE_UFS)) || \
(!strcmp(type, MNTTYPE_UDF)) || \
(!strcmp(type, MNTTYPE_REISER))

static char *qfextension[] = INITQFNAMES;
static char *qfname = QUOTAFILENAME;


/*
 * Check to see if a particular quota is to be enabled.
 */
static int
hasquota(struct mntent *mnt, int type, char **qfnamep)
{
   char *option, *pathname, has_quota_file_definition;
   char qfullname[PATH_MAX];
   
   if (!CORRECT_FSTYPE(mnt->mnt_type))
      return (0);

   has_quota_file_definition = 0;

   if ((type == USRQUOTA) && (option = hasmntopt(mnt, MNTOPT_USRQUOTA)) != (char *)NULL) {
      if (*(pathname = option + strlen(MNTOPT_USRQUOTA)) == '=')
         has_quota_file_definition = 1;
   } else {
      if ((type == GRPQUOTA) && (option = hasmntopt(mnt, MNTOPT_GRPQUOTA)) != (char *)NULL) {
         if (*(pathname = option + strlen(MNTOPT_GRPQUOTA)) == '=')
            has_quota_file_definition = 1;
      } else {
         if ((type == USRQUOTA) && (option = hasmntopt(mnt, MNTOPT_QUOTA)) != (char *)NULL) {
            if (*(pathname = option + strlen(MNTOPT_QUOTA)) == '=')
               has_quota_file_definition = 1;
         } else {
            return(0);
         }
      }
   }

   if (has_quota_file_definition) {
      if ((option = strchr(++pathname, ',')) != (char *)NULL) 
         strncpy(qfullname, pathname, min((option - pathname), sizeof(qfullname)));
      else
         strncpy(qfullname, pathname, sizeof(qfullname));
   } else {
      (void) sprintf(qfullname, "%s%s%s.%s", mnt->mnt_dir,
                    (mnt->mnt_dir[strlen(mnt->mnt_dir) - 1] == '/') ? "" : "/",
                     qfname, qfextension[type]);
   }

   *qfnamep = strdup(qfullname);
   return (1);
}
/* STOLEN END */

static void
f_quotaon(INT32 args)
{}

static void
f_quotaoff(INT32 args)
{}

static void
f_getquota(INT32 args)
{}

static void
f_setquota(INT32 args)
{}

static void
f_setqlim(INT32 args)
{}

static void
f_setuse(INT32 args)
{}

static void
f_sync(INT32 args)
{}

static void
f_getstats(INT32 args)
{}

static void
f_create(INT32 args)
{
    if (THIS->fs)
	free(THIS->fs);
}

static void
init_quota(struct object *o)
{
    memset(&THIS->dq, 0, sizeof(THIS->dq));
    memset(&THIS->stats, 0, sizeof(THIS->stats));
    THIS->fs = NULL;
}

static void
exit_quota(struct object *o)
{}

struct program*
_at_quota_init(void)
{
    start_new_program();
    ADD_STORAGE(struct quota_struct);

    set_init_callback(init_quota);
    set_exit_callback(exit_quota);

    add_integer_constant("USRQUOTA", USRQUOTA, 0);
    add_integer_constant("GRPQUOTA", GRPQUOTA, 0);
    
    quota_program = end_program();
    add_program_constant("Directory", quota_program, 0);
    
    return quota_program;
}
#else
struct program*
_at_quota_init(void)
{
    return NULL;
}
#endif
