/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2003 The Caudium Group
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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define SAX_DEBUG 1

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "xml_config.h"
#include "xml_sax.h"

#ifdef HAVE_XML2
#include <libxml/tree.h>
#include <libxml/SAX.h>
#include <libxml/entities.h>
#include <libxml/parserInternals.h>

#define THIS ((sax_storage*)Pike_fp->current_storage)
#define CB_ABSENT(_idx_) (THIS->callbackOffsets[(_idx_)] == -1)
#define CB_API(_name_) {#_name_, 1}

#define internalSubsetSAX 0x00
#define isStandaloneSAX 0x01
#define hasInternalSubsetSAX 0x02
#define hasExternalSubsetSAX 0x03
#define resolveEntitySAX 0x04
#define getEntitySAX 0x05
#define entityDeclSAX 0x06
#define notationDeclSAX 0x07
#define attributeDeclSAX 0x08
#define elementDeclSAX 0x09
#define unparsedEntityDeclSAX 0x0A
#define setDocumentLocatorSAX 0x0B
#define startDocumentSAX 0x0C
#define endDocumentSAX 0x0D
#define startElementSAX 0x0E
#define endElementSAX 0x0F
#define referenceSAX 0x10
#define charactersSAX 0x11
#define ignorableWhitespaceSAX   0x12
#define processingInstructionSAX 0x13
#define commentSAX 0x14
#define warningSAX 0x15
#define errorSAX 0x16
#define fatalErrorSAX 0x17
#define getParameterEntitySAX 0x18
#define cdataBlockSAX 0x19
#define externalSubsetSAX 0x1A
#define CB_API_SIZE 0x1B

typedef enum {
  PARSE_PUSH_PARSER = 0x01,
  PARSE_MEMORY_PARSER = 0x02,
  PARSE_FILE_PARSER = 0x03
} xmlParsingMethod;

typedef struct
{
  char   *name;
  int     req;
} pikeCallbackAPI;

typedef struct 
{
  xmlParserCtxtPtr     ctxt;
  xmlSAXHandlerPtr     sax;
  xmlParsingMethod     parsing_method;
  int                  callbackOffsets[CB_API_SIZE];
  char                *filename;
  struct object       *callbacks;
  struct object       *file_obj;
  struct pike_string  *input_data;
} sax_storage;

static void pextsInternalSubset(void *ctx, const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID);
static int pextsIsStandalone(void *ctx);
static int pextsHasInternalSubset(void *ctx);
static int pextsHasExternalSubset(void *ctx);
static xmlParserInputPtr pextsResolveEntity(void *ctx, const xmlChar *publicId, const xmlChar *systemId);
static xmlEntityPtr pextsGetEntity(void *ctx, const xmlChar *name);
static void pextsEntityDecl(void *ctx, const xmlChar *name, int type, const xmlChar *publicId,
                            const xmlChar *systemId, xmlChar *content);
static void pextsNotationDecl(void *ctx, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId);
static void pextsAttributeDecl(void *ctx, const xmlChar *elem, const xmlChar *fullname, int type, int def,
                               const xmlChar *defaultValue, xmlEnumerationPtr tree);
static void pextsElementDecl(void *ctx, const xmlChar *name, int type, xmlElementContentPtr content);
static void pextsUnparsedEntityDecl(void *ctx, const xmlChar *name, const xmlChar *publicId,
                                    const xmlChar *systemId, const xmlChar *notationName);
static void pextsSetDocumentLocator(void *ctx, xmlSAXLocatorPtr loc);
static void pextsStartDocument(void *ctx);
static void pextsEndDocument(void *ctx);
static void pextsStartElement(void *ctx, const xmlChar *name, const xmlChar **atts);
static void pextsEndElement(void *ctx, const xmlChar *name);
static void pextsReference(void *ctx, const xmlChar *name);
static void pextsCharacters(void *ctx, const xmlChar *ch, int len);
static void pextsIgnorableWhitespace(void *ctx, const xmlChar *ch, int len);
static void pextsProcessingInstruction(void *ctx, const xmlChar *target, const xmlChar *data);
static void pextsComment(void *ctx, const xmlChar *value);
static void pextsWarning(void *ctx, const char *msg, ...);
static void pextsError(void *ctx, const char *msg, ...);
static void pextsFatalError(void *ctx, const char *msg, ...);
static xmlEntityPtr pextsGetParameterEntity(void *ctx, const xmlChar *name);
static void pextsCdataBlock(void *ctx, const xmlChar *value, int len);
static void pextsExternalSubset(void *ctx, const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID);
  
static struct program  *sax_program;

static pikeCallbackAPI  callback_api[] =
{
  CB_API(internalSubsetSAX),
  CB_API(isStandaloneSAX),
  CB_API(hasInternalSubsetSAX),
  CB_API(hasExternalSubsetSAX),
  CB_API(resolveEntitySAX),
  CB_API(getEntitySAX),
  CB_API(entityDeclSAX),
  CB_API(notationDeclSAX),
  CB_API(attributeDeclSAX),
  CB_API(elementDeclSAX),
  CB_API(unparsedEntityDeclSAX),
  CB_API(setDocumentLocatorSAX),
  CB_API(startDocumentSAX),
  CB_API(endDocumentSAX),
  CB_API(startElementSAX),
  CB_API(endElementSAX),
  CB_API(referenceSAX),
  CB_API(charactersSAX),
  CB_API(ignorableWhitespaceSAX),
  CB_API(processingInstructionSAX),
  CB_API(commentSAX),
  CB_API(warningSAX),
  CB_API(errorSAX),
  CB_API(fatalErrorSAX),
  CB_API(getParameterEntitySAX),
  CB_API(cdataBlockSAX),
  CB_API(externalSubsetSAX),
};

static xmlSAXHandler   pextsSAX = {
  .internalSubset = pextsInternalSubset,
  .isStandalone = pextsIsStandalone,
  .hasInternalSubset = pextsHasInternalSubset,
  .hasExternalSubset = pextsHasExternalSubset,
  .resolveEntity = pextsResolveEntity,
  .getEntity = pextsGetEntity,
  .entityDecl = pextsEntityDecl,
  .notationDecl = pextsNotationDecl,
  .attributeDecl = pextsAttributeDecl,
  .elementDecl = pextsElementDecl,
  .unparsedEntityDecl = pextsUnparsedEntityDecl,
  .setDocumentLocator = pextsSetDocumentLocator,
  .startDocument = pextsStartDocument,
  .endDocument = pextsEndDocument,
  .startElement = pextsStartElement,
  .endElement = pextsEndElement,
  .reference = pextsReference,
  .characters = pextsCharacters,
  .ignorableWhitespace = pextsIgnorableWhitespace,
  .processingInstruction = pextsProcessingInstruction,
  .comment = pextsComment,
  .warning = pextsWarning,
  .error = pextsError,
  .fatalError = pextsFatalError,
  .getParameterEntity = pextsGetParameterEntity,
  .cdataBlock = pextsCdataBlock,
  .externalSubset = pextsExternalSubset
};

/* Parser callbacks */

static void pextsInternalSubset(void *ctx, const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID)
{
  if (CB_ABSENT(internalSubsetSAX))
    return;
}

static int pextsIsStandalone(void *ctx)
{
  if (CB_ABSENT(isStandaloneSAX))
    return 1;
  return 0;
}

static int pextsHasInternalSubset(void *ctx)
{
  if (CB_ABSENT(hasInternalSubsetSAX))
    return 0;
  return 1;
}

static int pextsHasExternalSubset(void *ctx)
{
  if (CB_ABSENT(hasExternalSubsetSAX))
    return 0;
  return 1;
}

static xmlParserInputPtr pextsResolveEntity(void *ctx, const xmlChar *publicId, const xmlChar *systemId)
{
  if (CB_ABSENT(resolveEntitySAX))
    return NULL;
  return NULL;
}

static xmlEntityPtr pextsGetEntity(void *ctx, const xmlChar *name)
{
  if (CB_ABSENT(getEntitySAX))
    return NULL;
  return NULL;
}

static void pextsEntityDecl(void *ctx, const xmlChar *name, int type, const xmlChar *publicId,
                            const xmlChar *systemId, xmlChar *content)
{
  if (CB_ABSENT(entityDeclSAX))
    return;
}

static void pextsNotationDecl(void *ctx, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId)
{
  if (CB_ABSENT(notationDeclSAX))
    return;
}

static void pextsAttributeDecl(void *ctx, const xmlChar *elem, const xmlChar *fullname, int type, int def,
                               const xmlChar *defaultValue, xmlEnumerationPtr tree)
{
  if (CB_ABSENT(attributeDeclSAX))
    return;
}

static void pextsElementDecl(void *ctx, const xmlChar *name, int type, xmlElementContentPtr content)
{
  if (CB_ABSENT(elementDeclSAX))
    return;
}

static void pextsUnparsedEntityDecl(void *ctx, const xmlChar *name, const xmlChar *publicId,
                                    const xmlChar *systemId, const xmlChar *notationName)
{
  if (CB_ABSENT(unparsedEntityDeclSAX))
    return;
}

static void pextsSetDocumentLocator(void *ctx, xmlSAXLocatorPtr loc)
{
  if (CB_ABSENT(setDocumentLocatorSAX))
    return;
}

static void pextsStartDocument(void *ctx)
{
  if (CB_ABSENT(startDocumentSAX))
    return;
}

static void pextsEndDocument(void *ctx)
{
  if (CB_ABSENT(endDocumentSAX))
    return;
}

static void pextsStartElement(void *ctx, const xmlChar *name, const xmlChar **atts)
{
  if (CB_ABSENT(startElementSAX))
    return;
}

static void pextsEndElement(void *ctx, const xmlChar *name)
{
  if (CB_ABSENT(endElementSAX))
    return;
}

static void pextsReference(void *ctx, const xmlChar *name)
{
  if (CB_ABSENT(referenceSAX))
    return;
}

static void pextsCharacters(void *ctx, const xmlChar *ch, int len)
{
  if (CB_ABSENT(charactersSAX))
    return;
}

static void pextsIgnorableWhitespace(void *ctx, const xmlChar *ch, int len)
{
  if (CB_ABSENT(ignorableWhitespaceSAX))
    return;
}

static void pextsProcessingInstruction(void *ctx, const xmlChar *target, const xmlChar *data)
{
  if (CB_ABSENT(processingInstructionSAX))
    return;
}

static void pextsComment(void *ctx, const xmlChar *value)
{
  if (CB_ABSENT(commentSAX))
    return;
}

static void pextsWarning(void *ctx, const char *msg, ...)
{
  if (CB_ABSENT(warningSAX))
    return;
}

static void pextsError(void *ctx, const char *msg, ...)
{
  if (CB_ABSENT(errorSAX))
    return;
}

static void pextsFatalError(void *ctx, const char *msg, ...)
{
  if (CB_ABSENT(fatalErrorSAX))
    return;
}

static xmlEntityPtr pextsGetParameterEntity(void *ctx, const xmlChar *name)
{
  if (CB_ABSENT(getParameterEntitySAX))
    return NULL;
  return NULL;
}

static void pextsCdataBlock(void *ctx, const xmlChar *value, int len)
{
  if (CB_ABSENT(cdataBlockSAX))
    return;
}

static void pextsExternalSubset(void *ctx, const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID)
{
  if (CB_ABSENT(externalSubsetSAX))
    return;
}

/*
 * We require the callbacks object to have all the methods found in the
 * xmlSAXHandler structure to be present in the Pike object passed as the
 * callbacks:
 *
 *   internalSubsetSAX
 *   isStandaloneSAX
 *   hasInternalSubsetSAX
 *   hasExternalSubsetSAX
 *   resolveEntitySAX
 *   getEntitySAX
 *   entityDeclSAX
 *   notationDeclSAX
 *   attributeDeclSAX
 *   elementDeclSAX
 *   unparsedEntityDeclSAX
 *   setDocumentLocatorSAX
 *   startDocumentSAX
 *   endDocumentSAX
 *   startElementSAX
 *   endElementSAX
 *   referenceSAX
 *   charactersSAX
 *   ignorableWhitespaceSAX
 *   processingInstructionSAX
 *   commentSAX
 *   warningSAX
 *   errorSAX
 *   fatalErrorSAX
 *   getParameterEntitySAX
 *   cdataBlockSAX
 *   externalSubsetSAX
 */
static int is_callback_ok(struct object *callbacks)
{
  int                 ioff, i;
  struct identifier  *ident;
  
  if (!callbacks)
    return 0;

  i = 0;
  while (i < CB_API_SIZE) {
    ioff = find_identifier(callback_api[i].name, callbacks->prog);
    if (ioff < 0 && callback_api[i].req)
      return 0;
    else if (ioff < 0) {
      THIS->callbackOffsets[i] = -1;
      i++;
      continue;
    }
    
    ident = ID_FROM_INT(callbacks->prog, ioff);
    if (!IDENTIFIER_IS_FUNCTION(ident->identifier_flags))
      return 0;
    THIS->callbackOffsets[i] = ioff;
    i++;
  }
  
  return 1;
}

/*! @decl void create(string|object input, object callbacks, mapping|void entities, int|void input_is_data)
 */
static void f_create(INT32 args)
{
  struct object      *file_obj = NULL, *callbacks = NULL;
  char               *file_name = NULL;
  struct mapping     *entities = NULL;
  int                 input_is_data = 0;
  struct pike_string *input_data = NULL;
  
  switch(args) {
      case 4:
        if (ARG(4).type != T_INT)
          Pike_error("Incorrect type for argument 4: expected an integer");
        input_is_data = ARG(4).u.integer != 0;
        /* fall through */
        
      case 3:
        if (ARG(3).type != T_MAPPING)
          Pike_error("Incorrect type for argument 3: expected a mapping");
        entities = ARG(3).u.mapping;
        /* fall through */

      case 2:
        if (ARG(2).type != T_OBJECT)
          Pike_error("Incorrect type for argument 2: expected an object");
        callbacks = ARG(2).u.object;
        /* fall through */

      case 1:
        if (ARG(1).type != T_OBJECT && ARG(1).type != T_STRING)
          Pike_error("Incorrect type for argument 1: expected a string or an object");
        if (ARG(1).type == T_OBJECT)
          file_obj = ARG(1).u.object;
        else
          input_data = ARG(1).u.string;
        break;

      default:
        Pike_error("Incorrect number of arguments: expected between 2 and 4");
  }

  /* check whether file_obj is Stdio.File or derived */
  if (file_obj && find_identifier("read", file_obj->prog) < 0)
    Pike_error("Passed file object is not Stdio.File or derived from it");
  
  /* check whether the callbacks object contains all the required methods
   * */
  if (!is_callback_ok(callbacks))
    Pike_error("Passed callbacks object is not valid.");
  
  /* choose the parsing method */
  if (file_obj)
    THIS->parsing_method = PARSE_PUSH_PARSER;
  else if (input_data && input_is_data)
    THIS->parsing_method = PARSE_MEMORY_PARSER;
  else if (input_data)
    THIS->parsing_method = PARSE_FILE_PARSER;
  else
    Pike_error("Cannot determine the parser type to use");
  
  /* initialize the parser and state */
  THIS->sax = &pextsSAX;
  switch (THIS->parsing_method) {
      case PARSE_PUSH_PARSER:
        THIS->file_obj = file_obj;
        /* the context creation is delayed in this case */
        break;

      case PARSE_MEMORY_PARSER:
        THIS->input_data = input_data;
        break;

      case PARSE_FILE_PARSER:
        THIS->input_data = input_data;
        break;
  }
}

static void init_sax(struct object *o)
{
  if (!THIS)
    return;

  THIS->ctxt = NULL;
  THIS->sax = NULL;
  THIS->filename = NULL;
  THIS->parsing_method = 0;
  THIS->callbacks = NULL;
  THIS->file_obj = NULL;
  THIS->input_data = NULL;
}

static void exit_sax(struct object *o)
{
  if (!THIS)
    return;

  if (THIS->filename)
    free(THIS->filename);
}

int _init_xml_sax(void)
{
  start_new_program();
  ADD_STORAGE(sax_storage);

  set_init_callback(init_sax);
  set_exit_callback(exit_sax);

  ADD_FUNCTION("create", f_create,
               tFunc(tOr(tString, tObj) tObj tOr(tMapping, tVoid) tOr(tInt, tVoid), tVoid), 0);
  
  sax_program = end_program();
  add_program_constant("SAX", sax_program, 0);

  return 1;
}

int _shutdown_xml_sax(void)
{
  return 1;
}
#endif
