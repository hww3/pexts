/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright � 2000-2003 The Caudium Group
 */

/* Copyright (C) 2002-2003 The Caudium Group
 * Copyright (C) 2002-2003 Marek Habersack
 * 
 * This file is part of the Pike Extensions package.
 *
 * The GeoIP Pexts Module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GeoIP Pexts Module is distributed in the hope that it will be useful,
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
 *
 * Glue for the Maxmind GeoIP library.
 *
 
 */
#define _GNU_SOURCE

#include "global.h"
RCSID("$Id$");

#include "geoip_config.h"

#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#ifdef HAVE_GEOIP_H
#include <GeoIP.h>
#endif

#define MODULE_MAJOR 0
#define MODULE_MINOR 0
#define MODULE_BUILD 1

#include "caudium_util.h"

void pike_module_init(void);
void pike_module_exit(void);

#ifdef HAVE_LIBGEOIP

#define THIS ((geoip_storage*)Pike_fp->current_storage)

typedef enum {
  GIL_CODE_BY_ADDR,
  GIL_CODE_BY_NAME,
  GIL_CODE3_BY_ADDR,
  GIL_CODE3_BY_NAME,
  GIL_NAME_BY_ADDR,
  GIL_NAME_BY_NAME,
  GIL_REGION_BY_NAME,
  GIL_REGION_BY_ADDR
} GEOIP_LOOKUP_TYPE;

typedef struct 
{
  GeoIP          *gi;
  GeoIPRegion    *gireg;
} geoip_storage;

static struct program    *geoip_program;

static void init_geoip(struct object *obj)
{
  THIS->gi = NULL;
  THIS->gireg = NULL;
}

static void exit_geoip(struct object *obj)
{
  if (THIS->gi)
    GeoIP_delete(THIS->gi);
  THIS->gi = NULL;
  THIS->gireg = NULL;
}

static void f_geoip_create(INT32 args)
{
  INT32      flags = GEOIP_STANDARD;

  if (THIS->gi)
    Pike_error("GeoIP database already initialized.\n");

  if (args == 1)
    get_all_args("create", args, "%d", &flags);
  else if (args)
    Pike_error("Too many parameters.\n");

  THIS->gi = GeoIP_new(flags);
  if (!THIS->gi)
    Pike_error("Cannot initialize the GeoIP database.\n");

  pop_n_elems(args);
}

static const char* do_lookup(GEOIP_LOOKUP_TYPE which, const char *data)
{
  switch(which) {
      case GIL_CODE_BY_ADDR:
        return GeoIP_country_code_by_addr(THIS->gi, data);
        
      case GIL_CODE_BY_NAME:
        return GeoIP_country_code_by_name(THIS->gi, data);
        
      case GIL_CODE3_BY_ADDR:
        return GeoIP_country_code3_by_addr(THIS->gi, data);
        
      case GIL_CODE3_BY_NAME:
        return GeoIP_country_code3_by_name(THIS->gi, data);
        
      case GIL_NAME_BY_ADDR:
        return GeoIP_country_name_by_addr(THIS->gi, data);
        
      case GIL_NAME_BY_NAME:
        return GeoIP_country_name_by_name(THIS->gi, data);

      default:
        return NULL;
  }
  
  return NULL; /* unreachable */
}

static void f_geoip_codebyaddr(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("CodeByAddr", args, "%s", &data);
  ret = do_lookup(GIL_CODE_BY_ADDR, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(0);
  else
    push_text(ret);
}

static void f_geoip_codebyname(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("CodeByName", args, "%s", &data);
  ret = do_lookup(GIL_CODE_BY_NAME, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(0);
  else
    push_text(ret);
}

static void f_geoip_code3byaddr(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("Code3ByAddr", args, "%s", &data);
  ret = do_lookup(GIL_CODE3_BY_ADDR, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(0);
  else
    push_text(ret);
}

static void f_geoip_code3byname(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("Code3ByName", args, "%s", &data);
  ret = do_lookup(GIL_CODE3_BY_NAME, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(0);
  else
    push_text(ret);
}

static void f_geoip_namebyaddr(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("NameByAddr", args, "%s", &data);
  ret = do_lookup(GIL_NAME_BY_ADDR, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(0);
  else
    push_text(ret);
}

static void f_geoip_namebyname(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("NameByName", args, "%s", &data);
  ret = do_lookup(GIL_NAME_BY_NAME, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(0);
  else
    push_text(ret);
}

static void f_geoip_idbyaddr(INT32 args)
{
  char       *data;
  int         ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("IdByAddr", args, "%s", &data);
  ret = GeoIP_country_id_by_addr(THIS->gi, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(-1);
  else
    push_int(ret);
}

static void f_geoip_idbyname(INT32 args)
{
  char       *data;
  int         ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("IdByName", args, "%s", &data);
  ret = GeoIP_country_id_by_name(THIS->gi, data);

  pop_n_elems(args);
  
  if (!ret)
    push_int(-1);
  else
    push_int(ret);
}

static void do_region(GEOIP_LOOKUP_TYPE which, const char *data)
{
  switch(which) {
      case GIL_REGION_BY_NAME:
        THIS->gireg = GeoIP_region_by_addr(THIS->gi, data);
        break;

      case GIL_REGION_BY_ADDR:
        THIS->gireg = GeoIP_region_by_addr(THIS->gi, data);
        break;

      default:
        push_int(0);
        return;
  }

  if (!THIS->gireg)
    push_int(0);
  else {
    push_text("country_code");
    push_text(THIS->gireg->country_code);
    push_text("region");
    push_text(THIS->gireg->region);
    GeoIPRegion_delete(THIS->gireg);
    THIS->gireg = NULL;

    f_aggregate_mapping(4);
  }
}

static void f_geoip_regionbyaddr(INT32 args)
{
  char       *data;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("RegionByAddr", args, "%s", &data);
  
  pop_n_elems(args);
  
  do_region(GIL_REGION_BY_ADDR, data);
}

static void f_geoip_regionbyname(INT32 args)
{
  char       *data;
  const char *ret;

  if (!THIS->gi)
    Pike_error("GeoIP not initialized properly.\n");
  
  get_all_args("RegionByName", args, "%s", &data);
  
  pop_n_elems(args);

  do_region(GIL_REGION_BY_ADDR, data);
}

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif

  add_integer_constant("GEOIP_STANDARD", GEOIP_STANDARD, 0);
  add_integer_constant("GEOIP_MEMORY_CACHE", GEOIP_MEMORY_CACHE, 0);
  add_integer_constant("GEOIP_CHECK_CACHE", GEOIP_CHECK_CACHE, 0);

  start_new_program();
  ADD_STORAGE(geoip_storage);
  set_init_callback(init_geoip);
  set_exit_callback(exit_geoip);

  ADD_FUNCTION("create", f_geoip_create, tFunc(tOr(tVoid, tInt), tVoid), 0);
  ADD_FUNCTION("CodeByAddr", f_geoip_codebyaddr, tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("CodeByName", f_geoip_codebyname, tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("Code3ByAddr", f_geoip_code3byaddr, tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("Code3ByName", f_geoip_code3byname, tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("NameByAddr", f_geoip_namebyaddr, tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("NameByName", f_geoip_namebyname, tFunc(tString, tOr(tString, tInt)), 0);
  ADD_FUNCTION("IdByAddr", f_geoip_idbyaddr, tFunc(tString, tInt), 0);
  ADD_FUNCTION("IdByName", f_geoip_idbyname, tFunc(tString, tInt), 0);
  ADD_FUNCTION("RegionByAddr", f_geoip_regionbyaddr, tFunc(tString, tOr(tMapping, tInt)), 0);
  ADD_FUNCTION("RegionByName", f_geoip_regionbyname, tFunc(tString, tOr(tMapping, tInt)), 0);
  
  geoip_program = end_program();
  add_program_constant("Lookup", geoip_program, 0);
}

void pike_module_exit(void)
{}
#else
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif
