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
 * Definitions missing from the libc mntent.h header. Based on mntent.h
 * from the Quota 2.00 package.
 *
 * $Id$
 */
#ifndef _AT_MNTENT_H
#define _AT_MNTENT_H

#include <features.h>

#define MNTMAXSTR	512

#define MNTTYPE_COHERENT	"coherent"	/* Coherent file system */
#define MNTTYPE_EXT2		"ext2"		/* Second Extended file system */
#define MNTTYPE_EXT3		"ext3"		/* Second Extended file system w/ journaling */
#define MNTTYPE_HPFS		"hpfs"		/* OS/2's high performance file system */
#define MNTTYPE_ISO9660		"iso9660"	/* ISO CDROM file system */
#define MNTTYPE_MINIX		"minix"		/* MINIX file system */
#define MNTTYPE_MSDOS		"msdos"		/* MS-DOS file system */
#define MNTTYPE_SYSV		"sysv"		/* System V file system */
#define MNTTYPE_UMSDOS		"umsdos"	/* U MS-DOS file system */
#define MNTTYPE_XENIX		"xenix"		/* Xenix file system */
#define MNTTYPE_NFS		"nfs"		/* Network file system */
#define MNTTYPE_PROC		"proc"		/* Linux process file system */
#define MNTTYPE_IGNORE		"ignore"	/* Ignore this entry */
#define MNTTYPE_SWAP		"swap"		/* Swap device */
#define MNTTYPE_REISER		"reiser"	/* Reiser file system */
#define MNTTYPE_UFS		"ufs"		/* UNIX file system */
#define MNTTYPE_UDF		"udf"		/* OSTA UDF file system */

/* generic mount options */
#define MNTOPT_DEFAULTS		"defaults"	/* use all default opts */
#define MNTOPT_RO		"ro"		/* read only */
#define MNTOPT_RW		"rw"		/* read/write */
#define MNTOPT_SUID		"suid"		/* set uid allowed */
#define MNTOPT_NOSUID		"nosuid"	/* no set uid allowed */
#define MNTOPT_NOAUTO		"noauto"	/* don't auto mount */

/* ext2 and msdos options */
#define	MNTOPT_CHECK		"check"		/* filesystem check level */

/* ext2 specific options */
#define	MNTOPT_BSDDF		"bsddf"		/* disable MINIX compatibility disk free counting */
#define	MNTOPT_BSDGROUPS	"bsdgroups"	/* set BSD group usage */
#define	MNTOPT_ERRORS		"errors"	/* set behaviour on error */
#define	MNTOPT_GRPID		"grpid"		/* set BSD group usage */
#define	MNTOPT_MINIXDF		"minixdf"	/* enable MINIX compatibility disk free counting */
#define	MNTOPT_NOCHECK		"nocheck"	/* reset filesystem checks */
#define	MNTOPT_NOGRPID		"nogrpid"	/* set SYSV group usage */
#define	MNTOPT_RESGID		"resgid"	/* group to consider like root for reserved blocks */
#define	MNTOPT_RESUID		"resuid"	/* user to consider like root for reserved blocks */
#define	MNTOPT_SB		"sb"		/* set used super block */
#define	MNTOPT_SYSVGROUPS	"sysvgroups"	/* set SYSV group usage */

/* options common to hpfs, isofs, and msdos */
#define	MNTOPT_CONV		"conv"		/* convert specified types of data */
#define	MNTOPT_GID		"gid"		/* use given gid */
#define	MNTOPT_UID		"uid"		/* use given uid */
#define	MNTOPT_UMASK		"umask"		/* use given umask, not isofs */

/* hpfs specific options */
#define	MNTOPT_CASE		"case"		/* case conversation */

/* isofs specific options */
#define	MNTOPT_BLOCK		"block"		/* use given block size */
#define	MNTOPT_CRUFT		"cruft"		/* ??? */
#define	MNTOPT_MAP		"map"		/* ??? */
#define	MNTOPT_NOROCK		"norock"	/* not rockwell format ??? */

/* msdos specific options */
#define	MNTOPT_FAT		"fat"		/* set FAT size */
#define	MNTOPT_QUIET		"quiet"		/* ??? */

/* swap specific options */

/* options common to ext, ext2, minix, xiafs, sysv, xenix, coherent */
#define MNTOPT_NOQUOTA		"noquota"	/* don't use any quota on this partition */
#define MNTOPT_QUOTA		"quota"		/* use userquota on this partition */
#define MNTOPT_USRQUOTA		"usrquota"	/* use userquota on this partition */
#define MNTOPT_GRPQUOTA		"grpquota"	/* use groupquota on this partition */
#if defined(Q_RSQUASH)
#define MNTOPT_RSQUASH		"rsquash"	/* threat root as an ordinary user */
#endif

#endif /* _MNTENT_H */
