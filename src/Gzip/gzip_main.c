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
 * Code for reading the files packed with gzip(1)
 *
 * $Id$
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "gzip_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_LIBZ

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

static struct program   *gzip_program;

typedef struct
{
    gzFile      in;
    int         out;
    char       *from;
    char       *to;
} GZSTRUCT;

#define THIS ((GZSTRUCT*)fp->current_storage)

/*
 * Uncompress data from THIS->from to THIS->to.
 * If either is NULL then use stdin and stdout, respectively.
 */
#define BUFLEN 8192
static void
f_gzip_uncompress(INT32 args)
{
    char    buf[BUFLEN]; /* todo: make it configurable */
    int     len, wlen;
    int     err;
    char   *from;
    char   *to;

    switch (args) {
        case 2:
            get_all_args("Gzip.gzip->uncompress()", args, "%s%s",
                         &from, &to);
            break;
            
        case 1:
            get_all_args("Gzip.gzip->uncompress()", args, "%s",
                         &from);
            break;

        case 0:
            from = THIS->from;
            to = THIS->to;
            break;

        default:
            Pike_error("Wrong number of parameters\n");
    }

    if (from && (!strcmp(from, "stdin") || !strcmp(from, "-")))
        THIS->from = NULL;
    else
        THIS->from = from;
    
    if (to && (!strcmp(to, "stdout") || !strcmp(to, "-")))
        THIS->to = NULL;
    else
        THIS->to = to;
    
    if (THIS->from)
        THIS->in = gzopen(THIS->from, "rb");
    else
        THIS->in = gzdopen(0, "rb");
    
    if (!THIS->in)
        Pike_error("Error opening input gzip file '%s'\n",
                   THIS->from ? THIS->from : "stdin");

    if (THIS->to) {
        THIS->out = open(THIS->to, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if (THIS->out < 0)
            Pike_error("Error opening output file '%s'. %s\n",
                       THIS->to ? THIS->to : "stdout",
                       strerror(errno));
    } else
        THIS->out = 1;
    
    while(1){
        len = gzread(THIS->in, buf, sizeof(buf));
        if (len < 0) {
            if (THIS->from)
                gzclose(THIS->in);
            if (THIS->to)
                close(THIS->out);
            THIS->out = -1;
            THIS->in = NULL;
            Pike_error("Error while decompressing data from file '%s'. %s\n",
                       THIS->from ? THIS->from : "stdin",
                       gzerror(THIS->in, &err));
        }
        
        if (!len)
            break;

        wlen = write(THIS->out, (const void*)buf, len);
        if (wlen < 0 || wlen != len) {
            if (THIS->from)
                gzclose(THIS->in);
            if (THIS->to)
                close(THIS->out);
            THIS->out = -1;
            THIS->in = NULL;
            Pike_error("Error while writing the decompressed data to file '%s'. %s\n",
                       THIS->to ? THIS->to : "stdout",
                       strerror(errno));
        }
    }

    if (THIS->from && gzclose(THIS->in) != Z_OK)
        Pike_error("Error closing the input file '%s'\n",
                   THIS->from ? THIS->from : "stdin");
    if (THIS->to && close(THIS->out) < 0)
        Pike_error("Error closing the output file '%s'\n",
                   THIS->to ? THIS->to : "stdout");

    THIS->out = -1;
    THIS->in = NULL;
}

static void
f_gzip_create(INT32 args)
{
    char   *from;
    char   *to;

    switch (args) {
        case 2:
            get_all_args("Gzip.gzip->create()", args, "%s%s",
                         &from, &to);
            break;
            
        case 1:
            get_all_args("Gzip.gzip->create()", args, "%s",
                         &from);
            break;

        case 0:
            from = to = NULL;
            break;

        default:
            Pike_error("Wrong number of parameters\n");
    }

    if (from && (!strcmp(from, "stdin") || !strcmp(from, "-")))
        from = NULL;

    if (to && (!strcmp(to, "stdout") || !strcmp(to, "-")))
        to = NULL;

    THIS->from = from;
    THIS->to = to;

    pop_n_elems(args);
}

static void
init_gzip(struct object *o)
{
    THIS->from = NULL;
    THIS->to = NULL;
    THIS->in = NULL;
    THIS->out = -1;
}

static void
exit_gzip(struct object *o)
{}

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
    pexts_init();
#endif

    start_new_program();
    ADD_STORAGE(GZSTRUCT);
    
    set_init_callback(init_gzip);
    set_exit_callback(exit_gzip);

    ADD_FUNCTION("create", f_gzip_create,
                 tFunc(tOr(tString, tVoid) tOr(tString, tVoid), tVoid), 0);
    ADD_FUNCTION("uncompress", f_gzip_uncompress,
                 tFunc(tOr(tString, tVoid) tOr(tString, tVoid), tVoid), 0);
    
    gzip_program = end_program();
    add_program_constant("gzip", gzip_program, 0);
}

void pike_module_exit(void)
{
  free_program(gzip_program);
}
#endif /* HAVE_LIBZ */
