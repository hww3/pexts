/*
 * Copyright (C) 1996-2000 Michael R. Elkins <me@cs.hmc.edu>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 */ 

char *mutt_choose_charset (const char *fromcode, const char *charsets,
		      char *u, size_t ulen, char **d, size_t *dlen);

void _rfc2047_encode_string (char **, int, int);
void rfc2047_encode_adrlist (ADDRESS *, const char *);

#define rfc2047_encode_string(a) _rfc2047_encode_string (a, 0, 32);

void rfc2047_decode (char **);
void rfc2047_decode_adrlist (ADDRESS *);
