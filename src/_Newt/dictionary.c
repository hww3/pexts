/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
 */

/* Copyright (C) 2000-2003 The Caudium Group
 * Copyright (C) 2000-2002 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The Newt Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The Newt Pexts Module is distributed in the hope that it will be useful,
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

/*
 * The object dictionary
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include <unistd.h>
#include <stdio.h>

#include "caudium_util.h"

#include "newt_config.h"

#if defined(HAVE_NEWT_H) && defined(HAVE_LIBNEWT)
#include "newt_global.h"

#define INIT_DICT_SIZE     8
#define INIT_DICTIONARIES  4

static DICT      **dictionaries;
static unsigned    dictcount;
static unsigned    dictmax;

static int
dict_insert(DICT *dict, struct object *obj, void *data)
{
    INFUN();
    
    if (!obj)
        return DICT_NULL_OBJECT;

    if (!data)
        return DICT_NULL_DATA;

    if (!dict)
        return DICT_NULL_DICT;
    

    if (dict->used && (dict->used >= dict->dict->data->num_keypairs)) {
        struct mapping  *nmap, *tmp;

        nmap = allocate_mapping(dict->dict->data->num_keypairs << 1);
        tmp = merge_mappings(dict->dict, nmap, PIKE_ARRAY_OP_AND);
        free_mapping(dict->dict);
        free_mapping(nmap);
        dict->dict = tmp;
    }

    {
        struct svalue   skey, sval, *tmp;
        char            addr[64];
        
        sval.type = T_OBJECT;
        sval.u.object = obj;

        sprintf(addr, "0x%X", (unsigned)data);
        skey.type = T_STRING;
        skey.u.string = make_shared_string(addr);

        tmp = low_mapping_lookup(dict->dict, &skey);
        if (tmp) {
            free_string(skey.u.string);
	    OUTFUN();
            return DICT_EXISTS;
        }

        mapping_insert(dict->dict, &skey, &sval);
        dict->used++;
    }

    OUTFUN();
    return DICT_OK;
}

static struct object*
dict_lookup(DICT *dict, void *data)
{
    struct svalue   *obj, skey;
    char             addr[64];
    
    INFUN();
    if (!dict || !data)
        return NULL;

    sprintf(addr, "0x%X", (unsigned)data);
    skey.type = T_STRING;
    skey.u.string = make_shared_string(addr);
    
    obj = low_mapping_lookup(dict->dict, &skey);

    free_string(skey.u.string);

    OUTFUN();
    return obj->u.object;
}

static void
dict_foreach(DICT *dict, dict_cb_fn cb)
{
    struct array      *arr;
    int               i;

    INFUN();
    
    if (!dict || !cb)
        return;

    if (!dict->dict->data->num_keypairs)
        return;

    arr = mapping_to_array(dict->dict);
    for (i = 0; i < arr->size; i++)
        cb(arr->item[i].u.object);

    free_array(arr);

    OUTFUN();
}

/*
 * Duplicate names are OK. It's just for reference.
 */
DICT*
dict_create(char *fn, char *name)
{
    DICT      *tmp;

    INFUN();
    
    if (!dictionaries) {
        dictionaries = (DICT**)calloc(sizeof(DICT*), INIT_DICTIONARIES);
        if (!dictionaries) {
            OUTFUN();
            FERROR(fn, "Failed to allocate memory for dictionaries (%d bytes)",
                   INIT_DICTIONARIES * sizeof(DICT*));
        }
        
        dictmax = INIT_DICTIONARIES;
    } else if(dictcount >= dictmax)  {
        dictionaries = (DICT**)realloc(dictionaries, (dictcount << 1) * sizeof(DICT*));
        if (!dictionaries) {
            OUTFUN();
            FERROR(fn, "Failed to enlarge memory for dictionaries (by %d bytes)",
                   INIT_DICTIONARIES * sizeof(DICT*));
        }
        
        dictmax = dictcount << 1;
    }
    
    tmp = dictionaries[dictcount] = (DICT*)calloc(sizeof(DICT), 1);
    if (!dictionaries[dictcount]) {
        OUTFUN();
        FERROR(fn, "Failed to allocate memory for dictionary (%d bytes)",
               sizeof(DICT));
    }

    dictcount++;
    tmp->dict = allocate_mapping(INIT_DICT_SIZE);
    tmp->used = 0;
    tmp->insert = dict_insert;
    tmp->lookup = dict_lookup;
    tmp->foreach = dict_foreach;
    
    if (name)
        tmp->name = strdup(name);
    else
        tmp->name = NULL;

    OUTFUN();
    
    return tmp;
}

void
init_dictionary(void)
{
    INFUN();
    
    dictionaries = NULL;
    dictcount = 0;

    OUTFUN();
}

#else
static void i_am_void() 
{}
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * fill-column: 75
 * End:
 */
 
