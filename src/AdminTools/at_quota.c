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

#include <stdio.h>
#include <string.h>
#include <mntent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/quota.h>

#include "at_common.h"
#include "at_mntent.h"

#ifndef _PATH_MOUNTED
#define _PATH_MOUNTED "/etc/mtab"
#endif

struct quota_struct
{
    struct dqblk     dq;
    struct dqstats   stats;
    char             *fs; /* which FS do we operate on? */
};

static char *_object_name = "Quota";

static struct program *quota_program;

#define THIS_LOW ((ATSTORAGE*)get_storage(fp->current_object, quota_program))
#define THIS ((struct quota_struct*)THIS_LOW->object_data)

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
	strncpy(qfullname, pathname, min((option - pathname), (int)sizeof(qfullname)));
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

static struct mntent *is_mounted(char *dev, char *fn)
{
    struct mntent   *ret;
    FILE            *fmnt;
    
    if (!dev)
	return NULL;
	
    fmnt = setmntent(_PATH_MOUNTED, "r");
    if (!fmnt)
	FERROR(fn, "Cannot open the mounted filesystems file");
	
    ret = getmntent(fmnt);
    while(ret) {
	if (!strncmp(ret->mnt_fsname, dev, strlen(ret->mnt_fsname))) {
	    endmntent(fmnt);
	    return ret;
	}
	    
	ret = getmntent(fmnt);
    }
    
    endmntent(fmnt);
    return ret;
}

static void
f_quotaon(INT32 args)
{
    char           *fname;
    struct mntent  *mnt;
    
    if (args < 1)
	FERROR("on", "one STRING argument required");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("on", "Wrong argument type for argument 1. Expected 8-bit string");
	
    mnt = is_mounted(ARG(1).u.string->str, "on");
    if (!mnt)
	FERROR("on", "Requested file system is not mounted: %s", 
	       ARG(1).u.string->str);
	       
    pop_n_elems(args);
}

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
	
    pop_n_elems(args);
}

static void
init_quota(struct object *o)
{
    THIS_LOW->object_name = _object_name;
    THIS_LOW->object_data = malloc(sizeof(struct quota_struct));
    if (!THIS_LOW->object_data)
	error("Out of memory in AdminTools.Shadow init!\n");

    memset(&THIS->dq, 0, sizeof(THIS->dq));
    memset(&THIS->stats, 0, sizeof(THIS->stats));
    THIS->fs = NULL;
}

static void
exit_quota(struct object *o)
{
    if (THIS_LOW->object_data)
	free(THIS_LOW->object_data);
}

struct program*
_at_quota_init(void)
{
    start_new_program();
    ADD_STORAGE(ATSTORAGE);

    set_init_callback(init_quota);
    set_exit_callback(exit_quota);

    ADD_FUNCTION("create", f_create, tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("on", f_quotaon, tFunc(tString, tVoid), 0);
    ADD_FUNCTION("off", f_quotaoff, tFunc(tVoid, tVoid), 0);
    
    add_integer_constant("USRQUOTA", USRQUOTA, 0);
    add_integer_constant("GRPQUOTA", GRPQUOTA, 0);
    
    quota_program = end_program();
    add_program_constant("Quota", quota_program, 0);
    
    return quota_program;
}
#else
struct program*
_at_quota_init(void)
{
    return NULL;
}
#endif
