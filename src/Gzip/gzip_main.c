/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000, 2001, 2002 The Caudium Group
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

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#if defined(HAVE_ALLOCA)
#define LOCAL_BUF(_s_) alloca(_s_)
#define LOCAL_FREE(_s_)
#else
#define LOCAL_BUF(_s_) malloc(_s_)
#define LOCAL_FREE(_n_) if (_n_) free(_n_)
#endif

static struct program   *gzip_program;

#define GMODE_NORMAL      0x00
#define GMODE_IN_CHUNK    0x01

typedef struct
{
    gzFile      in;
    int         out;
    char       *from;
    char       *to;
    int         gmode;
} GZSTRUCT;

#define THIS ((GZSTRUCT*)fp->current_storage)

static INLINE gzFile
open_gz(char *from)
{
    gzFile ret;
    
    if (from && (!strcmp(from, "stdin") || !strcmp(from, "-")))
        from = NULL;
      
    if (from)
        ret = gzopen(from, "rb");
    else
        ret = gzdopen(0, "rb");
    
    if (!ret)
        Pike_error("Error opening input gzip file '%s'\n",
                   from ? from : "stdin");

    return ret;
}

static INLINE int
read_gz_chunk(gzFile in, char *buf, int buflen, int close_on_err)
{
    int len;
    
    len = gzread(in, buf, buflen);
    if (len < 0) {
        if (close_on_err)
            gzclose(in);
    }

    return len;
}

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
    gzFile  in;
    int     out;

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

    in = open_gz(from);
    
    if (to && (!strcmp(to, "stdout") || !strcmp(to, "-")))
        to = NULL;

    if (to) {
        out = open(to, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if (out < 0)
            Pike_error("Error opening output file '%s'. %s\n",
                       to ? to : "stdout",
                       strerror(errno));
    } else
        out = 1;
    
    while(1){
        len = read_gz_chunk(in, buf, sizeof(buf), from != NULL);
        if (len < 0) {
            if (to)
                close(out);
            Pike_error("Error while decompressing data from file '%s'. %s\n",
                       from ? from : "stdin",
                       gzerror(in, &err));
        }
        
        if (!len)
            break;

        wlen = write(out, (const void*)buf, len);
        if (wlen < 0 || wlen != len) {
            if (from)
                gzclose(in);
            if (to)
                close(out);
            Pike_error("Error while writing the decompressed data to file '%s'. %s\n",
                       to ? to : "stdout",
                       strerror(errno));
        }
    }

    if (from && gzclose(in) != Z_OK)
        Pike_error("Error closing the input file '%s'\n",
                   from ? from : "stdin");
    if (to && close(out) < 0)
        Pike_error("Error closing the output file '%s'\n",
                   to ? to : "stdout");

    pop_n_elems(args);
}

/*
 * Read data from an input file (or stdin if from is 'stdin' or '-') and
 * return it as a Pike string - in _one chunk_. That might be BIG!
 */
static void
f_gzip_getdata(INT32 args)
{
    char                buf[BUFLEN]; /* todo: make it configurable */
    char               *tmpbuf;
    int                 len, wlen;
    int                 err;
    char               *from;
    gzFile              in;
    struct pike_string *ret;
    size_t              tmplen;
    
    switch (args) {            
        case 1:
            get_all_args("Gzip.gzip->getdata()", args, "%s",
                         &from);
            break;

        case 0:
            from = THIS->from;
            break;

        default:
            Pike_error("Wrong number of parameters\n");
    }

    in = open_gz(from);

    tmpbuf = NULL;
    tmplen = 0;
    
    while(1) {
        char  *tmp;
        
        len = read_gz_chunk(in, buf, sizeof(buf), from != NULL);
        if (len < 0)
            Pike_error("Error while decompressing data from file '%s'. %s\n",
                       from ? from : "stdin",
                       gzerror(in, &err));
        
        if (!len)
            break;

        /*
         * todo: need a smarter algo here
         */
        if (!tmpbuf) {
            tmpbuf = (char*)malloc(len * sizeof(char));
            tmplen = len;
            tmp = tmpbuf;
        } else {
            tmpbuf = (char*)realloc(tmpbuf, (tmplen + len) * sizeof(char));
            tmp = tmpbuf + tmplen;
            tmplen += len;
        }

        memcpy(tmp, buf, len);
    }

    if (from && gzclose(in) != Z_OK)
        Pike_error("Error closing the input file '%s'\n",
                   from ? from : "stdin");
    
    pop_n_elems(args);
    
    if (!tmpbuf)
        push_int(0);
    else {
        push_string(make_shared_binary_string(tmpbuf, tmplen));
        free(tmpbuf);
    }
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
    THIS->gmode = GMODE_NORMAL;
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
    ADD_FUNCTION("getdata", f_gzip_getdata,
                 tFunc(tOr(tString, tVoid), tString), 0);
    
    gzip_program = end_program();
    add_program_constant("gzip", gzip_program, 0);
}

void pike_module_exit(void)
{
  free_program(gzip_program);
}
#endif /* HAVE_LIBZ */
