/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000 The Caudium Group
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

#ifndef NEWT_CONFIG_H
#define NEWT_CONFIG_H

@TOP@

@BOTTOM@

#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)
#define HAVE_NEWT
#include <newt.h>
#endif

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * fill-column: 75
 * End:
 */