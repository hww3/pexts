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
 * Glue for the bzip2 compression library v1.0.x
 *
 * $Id$
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "gpgme_config.h"

#ifdef HAVE_LIBGPGME
#ifdef HAVE_GPGME_H
#include <gpgme.h>
#endif

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#else /* !HAVE_LIBGPGME */
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif
#endif
