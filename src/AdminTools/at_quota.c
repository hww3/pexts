/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2005 The Caudium Group
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
 * $Id$
 */

/*
 * File licensing and authorship information block.
 *
 * Version: MPL 1.1/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *
 * Marek Habersack <grendel@caudium.net>
 *
 * Portions created by the Initial Developer are Copyright (C) Marek Habersack
 * & The Caudium Group. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of the LGPL, and not to allow others to use your version
 * of this file under the terms of the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL or the LGPL.
 *
 * Significant Contributors to this file are:
 *
 */

/*
 *
 * Simple glue for more advanced Unix functions.
 * quotactl support
 */
#define _GNU_SOURCE
#define _POSIX_PTHREAD_SEMANTICS

#include "global.h"
RCSID("$Id$");

/*
 **| file: at_quota.c
 **|  Implementation of the Quota class.
 **
 **| cvs_version: $Id$
 */
#include "caudium_util.h"

#include "at_config.h"

#ifdef HAVE_QUOTA_SUPPORT

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <mntent.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/quota.h>
#include <errno.h>

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

#define THIS_LOW ((ATSTORAGE*)get_storage(Pike_fp->current_object, quota_program))
#define THIS ((struct quota_struct*)THIS_LOW->object_data)

#define F_USRQUOTA 0x01
#define F_GRPQUOTA 0x02

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
    struct mntent   *ret = NULL;
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
quota_onoff(INT32 args, char *fn, int cmd)
{
    char           *fname;
    struct mntent  *mnt;
    int            qret = 0;
    char           *fs;
    int            which = F_USRQUOTA;
    
    if (args < 1)
        FERROR(fn, "single STRING argument required");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR(fn, "Wrong argument type for argument 1. Expected 8-bit string");

    if (args > 1) {
        if (ARG(2).type != T_INT)
            FERROR(fn, "Wrong argument type for argument 2. Expected INT");
        which = 0;
        if (ARG(2).u.integer & F_USRQUOTA)
            which |= F_USRQUOTA;
        if (ARG(2).u.integer & F_GRPQUOTA)
            which |= F_GRPQUOTA;
	    
        if (!which)
            FERROR(fn, "Wrong argument value for argument 2. Expected a combination of USRQUOTA and GRPQUOTA");
    }
    	    
    fs = ARG(1).u.string->str;
    	
    mnt = is_mounted(fs, "on");
    if (!mnt)
        FERROR(fn, "File system is not mounted: %s", fs);

    if (which & F_USRQUOTA) {
        if (!hasquota(mnt, USRQUOTA, &fname))	
            qret |= F_USRQUOTA;
        else {
            if (quotactl(QCMD(cmd, USRQUOTA), fs, 0, fname) < 0)
                FERROR(fn, "System error while turning user quota on:\n\t%s",
                       strerror(errno));
        }
    }
    
    if (which & F_GRPQUOTA) {
        if (!hasquota(mnt, GRPQUOTA, &fname))
            qret |= F_GRPQUOTA;
        else {
	    
            if (quotactl(QCMD(cmd, GRPQUOTA), fs, 0, fname) < 0)
                FERROR(fn, "System error while turning group quota on:\n\t%s",
                       strerror(errno));
        }
    }
    
    if ((which & F_USRQUOTA) && (qret & F_USRQUOTA))
        FERROR(fn, "User quota requested but not supported on '%s'", fs);
	
    if ((which & F_GRPQUOTA) && (qret & F_GRPQUOTA))
        FERROR(fn, "Group quota requested but not supported on '%s'", fs);
}

static void
f_quotaon(INT32 args)
{
    quota_onoff(args, "on", Q_QUOTAON);
    pop_n_elems(args);
}

static void
f_quotaoff(INT32 args)
{
    quota_onoff(args, "off", Q_QUOTAOFF);
    pop_n_elems(args);
}

/*
 * Push dquota block and create a suitable array
 */
static struct mapping*
make_time_mapping(time_t t, char *fn)
{
    struct mapping *m;
    struct svalue  sv, sk;
    struct tm      *tm;
    
    m = allocate_mapping(9);
    if (!m)
        FERROR(fn, "Error while allocating time mapping");
	
    tm = localtime(&t);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_sec;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("sec");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_min;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("min");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_hour;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("hour");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_mday;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("mday");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_mon;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("mon");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_year;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("year");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_wday;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("wday");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_yday;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("yday");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_INT;
    sv.u.integer = tm->tm_isdst;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("isdst");
    mapping_insert(m, &sk, &sv);
    
    return m;
}

static void
push_dqblk(struct dqblk *dqb, char *fn)
{
    struct mapping *m;
    struct svalue  sv, sk;
    
    m = allocate_mapping(7);
    if (!m)
        FERROR(fn, "Error while allocating dqblk mapping");
	
    /* blocks hard limit */    
    sv.type = T_INT;
    sv.u.integer = dqb->dqb_bhardlimit;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("bhardlimit");
    mapping_insert(m, &sk, &sv);
    
    /* blocks soft limit */
    sv.type = T_INT;
    sv.u.integer = dqb->dqb_bsoftlimit;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("bsoftlimit");
    mapping_insert(m, &sk, &sv);
    
    /* currently allocated blocks */
    sv.type = T_INT;
    sv.u.integer = dqb->dqb_curblocks;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("curblocks");
    mapping_insert(m, &sk, &sv);
    
    /* inodes hard limit */
    sv.type = T_INT;
    sv.u.integer = dqb->dqb_ihardlimit;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("ihardlimit");
    mapping_insert(m, &sk, &sv);
    
    /* inodes soft limit */
    sv.type = T_INT;
    sv.u.integer = dqb->dqb_isoftlimit;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("isoftlimit");
    mapping_insert(m, &sk, &sv);
    
    /* Currently allocated inodes */
    sv.type = T_INT;
    sv.u.integer = dqb->dqb_curinodes;
    sk.type = T_STRING;
    sk.u.string = make_shared_string("curinodes");
    mapping_insert(m, &sk, &sv);
    
    /* times... */
    sv.type = T_MAPPING;
    sv.u.mapping = make_time_mapping(dqb->dqb_btime, fn);
    sk.type = T_STRING;
    sk.u.string = make_shared_string("btime");
    mapping_insert(m, &sk, &sv);
    
    sv.type = T_MAPPING;
    sv.u.mapping = make_time_mapping(dqb->dqb_itime, fn);
    sk.type = T_STRING;
    sk.u.string = make_shared_string("itime");
    mapping_insert(m, &sk, &sv);

    push_mapping(m);
}
            
/*
 **| method: int|array(mapping) get(string dev);
 **|    alt: int|array(mapping) get(string dev, string|int user);
 **|    alt: int|array(mapping) get(string dev, string|int user, string|int group);
 **|    alt: int|array(mapping) get(string dev, string|int user, string|int group, int qtype);
 **|  Get the user/group quota on the given device (filesystem). The
 **|  first form asks for the user quota of the user with UID 0.
 **|  The second form lets the caller specify which user to query quota
 **|  for. You can give either the user's UID or name.
 **|  The third form lets you specify the GID or name of the group for
 **|  which you want to query the quota. Note that because of the
 **|  default query type which is USRQUOTA, the third form doesn't make
 **|  much sense.
 **|  The fourth form lets you chose which quota type the call should
 **|  query for. You can use two symbolic names that can be ORed
 **|  together to specify the query type:<br /><br />
 **|    - F_USRQUOTA - query for user quota (default)
 **|    - F_GRPQUOTA - query for group quota
 **
 **| name: get - get the quota statistics for the given filesystem
 */
static void
f_getquota(INT32 args)
{
    char            *fs;
    struct mntent   *mnt;
    struct dqblk    dqb;
    struct array    *arr;
    int             which = F_USRQUOTA;
    uid_t           uid = 0;
    gid_t           gid = 0;
    
    if (args < 1)
        FERROR("get", "Single STRING argument required");
	
    if (ARG(1).type != T_STRING || ARG(1).u.string->size_shift > 0)
        FERROR("get", "Wrong argument type for argument 1. Expected 8-bit string");

    if (args > 1) {
        switch (ARG(2).type) {
            case T_INT:
            {
                struct passwd* pass;
                
                uid = ARG(2).u.integer;
                pass = getpwuid(uid);
                if (!pass)
                    FERROR("get", "User ID %d doesn't exist.", ARG(2).u.integer);
                break;
            }
            
            case T_STRING:
            {
                struct passwd* pass;
                
                if (ARG(2).u.string->size_shift > 0)
                    FERROR("get", "Wrong argument size for argument 2. Expected 8 bit string.");

                pass = getpwnam(ARG(2).u.string->str);
                if (!pass)
                    FERROR("get", "User %s doesn't exist.", ARG(2).u.string->str);
                uid = pass->pw_uid;
                break;
            }

            default:
                FERROR("get", "Wrong argument type for argument 2. Expected either integer or an 8-bit string.");
                break;
        }
    }

    if (args > 2) {
        switch (ARG(3).type) {
            case T_INT:
            {
                struct group* grp;
                
                gid = ARG(3).u.integer;
                grp = getgrgid(gid);
                if (!grp)
                    FERROR("get", "Group ID %d doesn't exist.", ARG(3).u.integer);
                break;
            }
            
            case T_STRING:
            {
                struct group* grp;
                
                if (ARG(3).u.string->size_shift > 0)
                    FERROR("get", "Wrong argument size for argument 3. Expected 8 bit string.");

                grp = getgrnam(ARG(3).u.string->str);
                if (!grp)
                    FERROR("get", "Group %s doesn't exist.", ARG(3).u.string->str);
                gid = grp->gr_gid;
                break;
            }

            default:
                FERROR("get", "Wrong argument type for argument 3. Expected either integer or an 8-bit string.");
                break;
        }
    }
    
    if (args > 3) {
        if (ARG(4).type != T_INT)
            FERROR("get", "Wrong argument type for argument 4. Expected integer.");
        which = 0;
        if (ARG(4).u.integer & F_USRQUOTA)
            which |= F_USRQUOTA;
        if (ARG(4).u.integer & F_GRPQUOTA)
            which |= F_GRPQUOTA;
	    
        if (!which)
            FERROR("get", "Wrong argument value for argument 4. Expected a combination of USRQUOTA and GRPQUOTA");
    }
    
    fs = ARG(1).u.string->str;
    
    mnt = is_mounted(fs, "get");
    
    if (!mnt)
        FERROR("get", "File system is not mounted: %s", fs);

    if (which & F_USRQUOTA) {
        if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), fs, uid, (void*)&dqb) < 0) {
            if (errno == ESRCH || errno == ENOSYS)
                push_int(0);
            else
                FERROR("get", "System error ocurred while getting user quota on '%s':\n\t%s",
                       fs, strerror(errno));
        } else
            push_dqblk(&dqb, "get");
    } else
        push_int(0);
		   
    if (which & F_GRPQUOTA) {
        if (quotactl(QCMD(Q_GETQUOTA, GRPQUOTA), fs, gid, (void*)&dqb) < 0) {
            if (errno == ESRCH || errno == ENOSYS)
                push_int(0);
            else
                FERROR("get", "System error ocurred while getting group quota on '%s':\n\t%s",
                       fs, strerror(errno));
        } else
            push_dqblk(&dqb, "get");
    } else
        push_int(0);

    arr = aggregate_array(2);
    pop_n_elems(args);

    push_array(arr);
}

static void qm_missing_key(char *key, int idx)
{
    if (idx >= 0)
        Pike_error("Quota mapping at index %d is missing the '%s' key.",
                   idx, "dev");
    else
        Pike_error("Quota mapping is missing the '%s' key.", "dev");
}

static void qm_incorrect_key_type(char *key, char *req, int idx)
{}

static int set_quota(struct mapping *m, int idx)
{
    struct svalue     *sv;
    char              *dev;
    struct dqblk       qb;
    
    if (!m)
        return 1;

    sv = simple_mapping_string_lookup(m, "dev");
    if (!sv)
        qm_missing_key("dev", idx);

    if (sv->type != T_STRING || sv->u.string->size_shift > 1)
        qm_incorrect_key_type("dev", "string", idx);
    
}

/*
 **| method: int set(mapping|array(mapping) quota);
 **| Set all the possible quota values for the given device(s). The 'quota'
 **| mapping, described below, contains all the values.
 *
 **| arg: mapping quota
 **|  The values to be set. Either inner mapping is optional. All of the
 **|  values are optional as well and default to 0. Mapping contents:
 **|   <pre>
 **|     dev  - device for which to set the quota
 **|     user - user quota settings (mapping)
 **|       bsoft    - soft block quota (int)
 **|       bhard    - hard block quota (int)
 **|       btime    - block grace time (int, seconds)
 **|       isoft    - soft inode quota (int)
 **|       ihard    - hard inode quota (int)
 **|       itime    - inode grace time (int)
 **|       uid      - uid (int|string)
 **|     group - group quota settings (mapping)
 **|       bsoft    - soft block quota (int)
 **|       bhard    - hard block quota (int)
 **|       btime    - block grace time (int, seconds)
 **|       isoft    - soft inode quota (int)
 **|       ihard    - hard inode quota (int)
 **|       itime    - inode grace time (int)
 **|       gid      - gid (int|string)
 **|   </pre>
 */
static void
f_setquota(INT32 args)
{
    struct mapping      *m = NULL;
    struct array        *a = NULL;
    int                  ret;
    
    if (args != 1)
        Pike_error("Incorrect number of arguments. Expected 1.");

    switch(ARG(1).type) {
        case T_MAPPING:
            m = ARG(1).u.mapping;
            break;

        case T_ARRAY:
            a = ARG(1).u.array;
            break;

        default:
            Pike_error("Incorrect argument type. Expected array(mapping) or mapping.");
    }

    ret = 0;
    if (a) {
        int                i;
        struct svalue      sv;
        
        for(i = 0; i < a->size; i++) {
            array_index_no_free(&sv, a, i);
            if (sv.type != T_MAPPING)
                Pike_error("Index '%d' of the passed array is not a mapping.", i);
           ret += set_quota(sv.u.mapping, i);
        }
    } else if (m)
        ret = set_quota(m, -1);
    else
        ret = 1;

    pop_n_elems(args);
    push_int(ret != 0);
}

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
        Pike_error("Out of memory in AdminTools.Shadow init!\n");

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

    ADD_FUNCTION("create", f_create, 
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("on", f_quotaon, 
                 tFunc(tString tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("off", f_quotaoff, 
                 tFunc(tString tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("get", f_getquota, 
                 tFunc(tString tOr(tOr(tInt, tString), tVoid) tOr(tInt, tVoid), tOr(tArray, tInt)), 0);
    
    add_integer_constant("USRQUOTA", F_USRQUOTA, 0);
    add_integer_constant("GRPQUOTA", F_GRPQUOTA, 0);
    
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
