/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2002 The Caudium Group
 */

/* Copyright (C) 2002 The Caudium Group
 * Copyright (C) 2002 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The FDF Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The FDF Pexts Module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Pike Extensions; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 *
 * $Id$
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
