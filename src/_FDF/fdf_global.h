/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000-2003 The Caudium Group
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

#ifndef __FDF_GLOBAL_H
#define __FDF_GLOBAL_H

#if PIKE_MAJOR_VERSION == 7 && PIKE_MINOR_VERSION > 0
#define WRONG_NUM_OF_ARGS(name, nargs, nexpec) wrong_number_of_args_error(name, nargs, nexpec)
#else
#define WRONG_NUM_OF_ARGS(name, nargs, nexpec) \
    Pike_error("%s: wrong number of arguments (%d found, %d expected)\n", \
               name, nargs, nexpec)
#endif

typedef struct 
{
    int      val;
    char    *name;
    char    *desc;
    int      isError;
} constant_desc;

#endif
