/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
 */

/* Copyright (C) 2000-2003 The Caudium Group
 * Copyright (C) 2000-2003 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The OpenLDAP Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The OpenLDAP Pexts Module is distributed in the hope that it will be useful,
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
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "ol_config.h"
#include "ol_common.h"

#ifdef HAVE_LIBLDAP

struct array*
make_pike_array(char **carr)
{
    char          **tmp;
    int             nparts;
    
    tmp = carr;
    while(tmp && *tmp) {
        nparts++;
        push_string(make_shared_string(*tmp));
        tmp++;
    }

    if (!nparts)
        return NULL;
    
    return aggregate_array(nparts);
}
#endif
