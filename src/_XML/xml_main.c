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

#ifdef fp
#undef fp
#endif

#include "caudium_util.h"
#include "xml_config.h"

#ifdef HAVE_XML2
#include <libxml/entities.h>
#include "xml_sax.h"

/*! @decl int substituteEntitiesDefault(int def)
 */
static void f_substituteEntitiesDefault(INT32 args)
{
  int   i;

  get_all_args("substituteEntitiesDefault", args, "%d", &i);
  i = i ? 1 : 0;

  pop_n_elems(args);
  push_int(xmlSubstituteEntitiesDefault(i));
}

/* @decl int keepBlanksDefault(int def)
 */
static void f_keepBlanksDefault(INT32 args)
{
  int   i;

  get_all_args("keepBlanksDefault", args, "%d", &i);
  i = i ? 1 : 0;

  pop_n_elems(args);
  push_int(xmlKeepBlanksDefault(i));
}

static void f_utf8ToHTML(INT32 args)
{
  char *html = NULL;
  struct pike_string *str = NULL;
  int outlen, inlen;

  str = Pike_sp[-args].u.string;

  outlen = str->len << 1;
  html = (char*)malloc(outlen + 1);
  if (!html)
    Pike_error("Out of memory");
  
  inlen = str->len;
  if ( UTF8ToHtml(html, &outlen, str->str, &inlen) < 0 ) {
    free(html);
    Pike_error("Cannot convert to html!");
  }
  html[outlen] = '\0';
  pop_n_elems(args);
  push_text(html);
  free(html);
}

static void f_utf8ToISO(INT32 args)
{
  char *html = NULL;
  struct pike_string *str = NULL;
  int outlen, inlen;

  str = Pike_sp[-args].u.string;

  outlen = str->len << 1;
  html = (char*)malloc(outlen + 1);
  if (!html)
    Pike_error("Out of memory");
  
  inlen = str->len;
  if ( UTF8Toisolat1(html, &outlen, str->str, &inlen) < 0 ) {
    free(html);
    Pike_error("Cannot convert to isolat1!");
  }
  html[outlen] = '\0';
  pop_n_elems(args);
  push_text(html);
  free(html);
}

static void f_utf8Check(INT32 args)
{
  struct pike_string *str = NULL;
  int result;

  str = Pike_sp[-args].u.string;
  result = xmlCheckUTF8(str->str);
  pop_n_elems(args);
  push_int(result);
}

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif

  /* initialize the library */
  xmlInitParser();
  xmlLineNumbersDefault(1);
  xmlSubstituteEntitiesDefault(1);

  /* initialize the classes */
  if (!_init_xml_sax())
    Pike_error("Could not initialize the SAX class");

  /* global functions */
  ADD_FUNCTION("substituteEntitiesDefault", f_substituteEntitiesDefault,
               tFunc(tInt, tInt), 0);
  ADD_FUNCTION("keepBlanksDefault", f_keepBlanksDefault,
               tFunc(tInt, tInt), 0);
  ADD_FUNCTION("utf8_to_html", f_utf8ToHTML, tFunc(tString, tString), 0);
  ADD_FUNCTION("utf8_to_isolat1", f_utf8ToISO, tFunc(tString, tString), 0);
  ADD_FUNCTION("utf8_check", f_utf8Check, tFunc(tString, tInt), 0);
  
  /* some contstants */
  add_integer_constant("XML_INTERNAL_GENERAL_ENTITY", XML_INTERNAL_GENERAL_ENTITY, 0);
  add_integer_constant("XML_EXTERNAL_GENERAL_PARSED_ENTITY", XML_EXTERNAL_GENERAL_PARSED_ENTITY, 0);
  add_integer_constant("XML_EXTERNAL_GENERAL_UNPARSED_ENTITY", XML_EXTERNAL_GENERAL_UNPARSED_ENTITY, 0);
  add_integer_constant("XML_INTERNAL_PARAMETER_ENTITY", XML_INTERNAL_PARAMETER_ENTITY, 0);
  add_integer_constant("XML_EXTERNAL_PARAMETER_ENTITY", XML_EXTERNAL_PARAMETER_ENTITY, 0);
  add_integer_constant("XML_INTERNAL_PREDEFINED_ENTITY", XML_INTERNAL_PREDEFINED_ENTITY, 0);

  add_integer_constant("XML_ATTRIBUTE_CDATA", XML_ATTRIBUTE_CDATA, 0);
  add_integer_constant("XML_ATTRIBUTE_ID", XML_ATTRIBUTE_ID, 0);
  add_integer_constant("XML_ATTRIBUTE_IDREF", XML_ATTRIBUTE_IDREF, 0);
  add_integer_constant("XML_ATTRIBUTE_IDREFS", XML_ATTRIBUTE_IDREFS, 0);
  add_integer_constant("XML_ATTRIBUTE_ENTITY", XML_ATTRIBUTE_ENTITY, 0);
  add_integer_constant("XML_ATTRIBUTE_ENTITIES", XML_ATTRIBUTE_ENTITIES, 0);
  add_integer_constant("XML_ATTRIBUTE_NMTOKEN", XML_ATTRIBUTE_NMTOKEN, 0);
  add_integer_constant("XML_ATTRIBUTE_NMTOKENS", XML_ATTRIBUTE_NMTOKENS, 0);
  add_integer_constant("XML_ATTRIBUTE_ENUMERATION", XML_ATTRIBUTE_ENUMERATION, 0);
  add_integer_constant("XML_ATTRIBUTE_NOTATION", XML_ATTRIBUTE_NOTATION, 0);

  add_integer_constant("XML_ATTRIBUTE_NONE", XML_ATTRIBUTE_NONE, 0);
  add_integer_constant("XML_ATTRIBUTE_REQUIRED", XML_ATTRIBUTE_REQUIRED, 0);
  add_integer_constant("XML_ATTRIBUTE_IMPLIED", XML_ATTRIBUTE_IMPLIED, 0);
  add_integer_constant("XML_ATTRIBUTE_FIXED", XML_ATTRIBUTE_FIXED, 0);

  add_integer_constant("XML_ELEMENT_CONTENT_PCDATA", XML_ELEMENT_CONTENT_PCDATA, 0);
  add_integer_constant("XML_ELEMENT_CONTENT_ELEMENT", XML_ELEMENT_CONTENT_ELEMENT, 0);
  add_integer_constant("XML_ELEMENT_CONTENT_SEQ", XML_ELEMENT_CONTENT_SEQ, 0);
  add_integer_constant("XML_ELEMENT_CONTENT_OR", XML_ELEMENT_CONTENT_OR, 0);

  add_integer_constant("XML_ELEMENT_CONTENT_ONCE", XML_ELEMENT_CONTENT_ONCE, 0);
  add_integer_constant("XML_ELEMENT_CONTENT_OPT", XML_ELEMENT_CONTENT_OPT, 0);
  add_integer_constant("XML_ELEMENT_CONTENT_MULT", XML_ELEMENT_CONTENT_MULT, 0);
  add_integer_constant("XML_ELEMENT_CONTENT_PLUS", XML_ELEMENT_CONTENT_PLUS, 0);

  add_integer_constant("XML_ELEMENT_TYPE_UNDEFINED", XML_ELEMENT_TYPE_UNDEFINED, 0);
  add_integer_constant("XML_ELEMENT_TYPE_EMPTY", XML_ELEMENT_TYPE_EMPTY, 0);
  add_integer_constant("XML_ELEMENT_TYPE_ANY", XML_ELEMENT_TYPE_ANY, 0);
  add_integer_constant("XML_ELEMENT_TYPE_MIXED", XML_ELEMENT_TYPE_MIXED, 0);
  add_integer_constant("XML_ELEMENT_TYPE_ELEMENT", XML_ELEMENT_TYPE_ELEMENT, 0);

  /* error codes */
  add_integer_constant("XML_ERR_OK", XML_ERR_OK, 0);
  add_integer_constant("XML_ERR_INTERNAL_ERROR", XML_ERR_INTERNAL_ERROR, 0);
  add_integer_constant("XML_ERR_NO_MEMORY", XML_ERR_NO_MEMORY, 0);
  add_integer_constant("XML_ERR_DOCUMENT_START", XML_ERR_DOCUMENT_START, 0);
  add_integer_constant("XML_ERR_DOCUMENT_EMPTY", XML_ERR_DOCUMENT_EMPTY, 0);
  add_integer_constant("XML_ERR_DOCUMENT_END", XML_ERR_DOCUMENT_END, 0);
  add_integer_constant("XML_ERR_INVALID_HEX_CHARREF", XML_ERR_INVALID_HEX_CHARREF, 0);
  add_integer_constant("XML_ERR_INVALID_DEC_CHARREF", XML_ERR_INVALID_DEC_CHARREF, 0);
  add_integer_constant("XML_ERR_INVALID_CHARREF", XML_ERR_INVALID_CHARREF, 0);
  add_integer_constant("XML_ERR_INVALID_CHAR", XML_ERR_INVALID_CHAR, 0);
  add_integer_constant("XML_ERR_CHARREF_AT_EOF", XML_ERR_CHARREF_AT_EOF, 0);
  add_integer_constant("XML_ERR_CHARREF_IN_PROLOG", XML_ERR_CHARREF_IN_PROLOG, 0);
  add_integer_constant("XML_ERR_CHARREF_IN_EPILOG", XML_ERR_CHARREF_IN_EPILOG, 0);
  add_integer_constant("XML_ERR_CHARREF_IN_DTD", XML_ERR_CHARREF_IN_DTD, 0);
  add_integer_constant("XML_ERR_ENTITYREF_AT_EOF", XML_ERR_ENTITYREF_AT_EOF, 0);
  add_integer_constant("XML_ERR_ENTITYREF_IN_PROLOG", XML_ERR_ENTITYREF_IN_PROLOG, 0);
  add_integer_constant("XML_ERR_ENTITYREF_IN_EPILOG", XML_ERR_ENTITYREF_IN_EPILOG, 0);
  add_integer_constant("XML_ERR_ENTITYREF_IN_DTD", XML_ERR_ENTITYREF_IN_DTD, 0);
  add_integer_constant("XML_ERR_PEREF_AT_EOF", XML_ERR_PEREF_AT_EOF, 0);
  add_integer_constant("XML_ERR_PEREF_IN_PROLOG", XML_ERR_PEREF_IN_PROLOG, 0);
  add_integer_constant("XML_ERR_PEREF_IN_EPILOG", XML_ERR_PEREF_IN_EPILOG, 0);
  add_integer_constant("XML_ERR_PEREF_IN_INT_SUBSET", XML_ERR_PEREF_IN_INT_SUBSET, 0);
  add_integer_constant("XML_ERR_ENTITYREF_NO_NAME", XML_ERR_ENTITYREF_NO_NAME, 0);
  add_integer_constant("XML_ERR_ENTITYREF_SEMICOL_MISSING", XML_ERR_ENTITYREF_SEMICOL_MISSING, 0);
  add_integer_constant("XML_ERR_PEREF_NO_NAME", XML_ERR_PEREF_NO_NAME, 0);
  add_integer_constant("XML_ERR_PEREF_SEMICOL_MISSING", XML_ERR_PEREF_SEMICOL_MISSING, 0);
  add_integer_constant("XML_ERR_UNDECLARED_ENTITY", XML_ERR_UNDECLARED_ENTITY, 0);
  add_integer_constant("XML_WAR_UNDECLARED_ENTITY", XML_WAR_UNDECLARED_ENTITY, 0);
  add_integer_constant("XML_ERR_UNPARSED_ENTITY", XML_ERR_UNPARSED_ENTITY, 0);
  add_integer_constant("XML_ERR_ENTITY_IS_EXTERNAL", XML_ERR_ENTITY_IS_EXTERNAL, 0);
  add_integer_constant("XML_ERR_ENTITY_IS_PARAMETER", XML_ERR_ENTITY_IS_PARAMETER, 0);
  add_integer_constant("XML_ERR_UNKNOWN_ENCODING", XML_ERR_UNKNOWN_ENCODING, 0);
  add_integer_constant("XML_ERR_UNSUPPORTED_ENCODING", XML_ERR_UNSUPPORTED_ENCODING, 0);
  add_integer_constant("XML_ERR_STRING_NOT_STARTED", XML_ERR_STRING_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_STRING_NOT_CLOSED", XML_ERR_STRING_NOT_CLOSED, 0);
  add_integer_constant("XML_ERR_NS_DECL_ERROR", XML_ERR_NS_DECL_ERROR, 0);
  add_integer_constant("XML_ERR_ENTITY_NOT_STARTED", XML_ERR_ENTITY_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_ENTITY_NOT_FINISHED", XML_ERR_ENTITY_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_LT_IN_ATTRIBUTE", XML_ERR_LT_IN_ATTRIBUTE, 0);
  add_integer_constant("XML_ERR_ATTRIBUTE_NOT_STARTED", XML_ERR_ATTRIBUTE_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_ATTRIBUTE_NOT_FINISHED", XML_ERR_ATTRIBUTE_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_ATTRIBUTE_WITHOUT_VALUE", XML_ERR_ATTRIBUTE_WITHOUT_VALUE, 0);
  add_integer_constant("XML_ERR_ATTRIBUTE_REDEFINED", XML_ERR_ATTRIBUTE_REDEFINED, 0);
  add_integer_constant("XML_ERR_LITERAL_NOT_STARTED", XML_ERR_LITERAL_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_LITERAL_NOT_FINISHED", XML_ERR_LITERAL_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_COMMENT_NOT_FINISHED", XML_ERR_COMMENT_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_PI_NOT_STARTED", XML_ERR_PI_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_PI_NOT_FINISHED", XML_ERR_PI_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_NOTATION_NOT_STARTED", XML_ERR_NOTATION_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_NOTATION_NOT_FINISHED", XML_ERR_NOTATION_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_ATTLIST_NOT_STARTED", XML_ERR_ATTLIST_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_ATTLIST_NOT_FINISHED", XML_ERR_ATTLIST_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_MIXED_NOT_STARTED", XML_ERR_MIXED_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_MIXED_NOT_FINISHED", XML_ERR_MIXED_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_ELEMCONTENT_NOT_STARTED", XML_ERR_ELEMCONTENT_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_ELEMCONTENT_NOT_FINISHED", XML_ERR_ELEMCONTENT_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_XMLDECL_NOT_STARTED", XML_ERR_XMLDECL_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_XMLDECL_NOT_FINISHED", XML_ERR_XMLDECL_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_CONDSEC_NOT_STARTED", XML_ERR_CONDSEC_NOT_STARTED, 0);
  add_integer_constant("XML_ERR_CONDSEC_NOT_FINISHED", XML_ERR_CONDSEC_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_EXT_SUBSET_NOT_FINISHED", XML_ERR_EXT_SUBSET_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_DOCTYPE_NOT_FINISHED", XML_ERR_DOCTYPE_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_MISPLACED_CDATA_END", XML_ERR_MISPLACED_CDATA_END, 0);
  add_integer_constant("XML_ERR_CDATA_NOT_FINISHED", XML_ERR_CDATA_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_RESERVED_XML_NAME", XML_ERR_RESERVED_XML_NAME, 0);
  add_integer_constant("XML_ERR_SPACE_REQUIRED", XML_ERR_SPACE_REQUIRED, 0);
  add_integer_constant("XML_ERR_SEPARATOR_REQUIRED", XML_ERR_SEPARATOR_REQUIRED, 0);
  add_integer_constant("XML_ERR_NMTOKEN_REQUIRED", XML_ERR_NMTOKEN_REQUIRED, 0);
  add_integer_constant("XML_ERR_NAME_REQUIRED", XML_ERR_NAME_REQUIRED, 0);
  add_integer_constant("XML_ERR_PCDATA_REQUIRED", XML_ERR_PCDATA_REQUIRED, 0);
  add_integer_constant("XML_ERR_URI_REQUIRED", XML_ERR_URI_REQUIRED, 0);
  add_integer_constant("XML_ERR_PUBID_REQUIRED", XML_ERR_PUBID_REQUIRED, 0);
  add_integer_constant("XML_ERR_LT_REQUIRED", XML_ERR_LT_REQUIRED, 0);
  add_integer_constant("XML_ERR_GT_REQUIRED", XML_ERR_GT_REQUIRED, 0);
  add_integer_constant("XML_ERR_LTSLASH_REQUIRED", XML_ERR_LTSLASH_REQUIRED, 0);
  add_integer_constant("XML_ERR_EQUAL_REQUIRED", XML_ERR_EQUAL_REQUIRED, 0);
  add_integer_constant("XML_ERR_TAG_NAME_MISMATCH", XML_ERR_TAG_NAME_MISMATCH, 0);
  add_integer_constant("XML_ERR_TAG_NOT_FINISHED", XML_ERR_TAG_NOT_FINISHED, 0);
  add_integer_constant("XML_ERR_STANDALONE_VALUE", XML_ERR_STANDALONE_VALUE, 0);
  add_integer_constant("XML_ERR_ENCODING_NAME", XML_ERR_ENCODING_NAME, 0);
  add_integer_constant("XML_ERR_HYPHEN_IN_COMMENT", XML_ERR_HYPHEN_IN_COMMENT, 0);
  add_integer_constant("XML_ERR_INVALID_ENCODING", XML_ERR_INVALID_ENCODING, 0);
  add_integer_constant("XML_ERR_EXT_ENTITY_STANDALONE", XML_ERR_EXT_ENTITY_STANDALONE, 0);
  add_integer_constant("XML_ERR_CONDSEC_INVALID", XML_ERR_CONDSEC_INVALID, 0);
  add_integer_constant("XML_ERR_VALUE_REQUIRED", XML_ERR_VALUE_REQUIRED, 0);
  add_integer_constant("XML_ERR_NOT_WELL_BALANCED", XML_ERR_NOT_WELL_BALANCED, 0);
  add_integer_constant("XML_ERR_EXTRA_CONTENT", XML_ERR_EXTRA_CONTENT, 0);
  add_integer_constant("XML_ERR_ENTITY_CHAR_ERROR", XML_ERR_ENTITY_CHAR_ERROR, 0);
  add_integer_constant("XML_ERR_ENTITY_PE_INTERNAL", XML_ERR_ENTITY_PE_INTERNAL, 0);
  add_integer_constant("XML_ERR_ENTITY_LOOP", XML_ERR_ENTITY_LOOP, 0);
  add_integer_constant("XML_ERR_ENTITY_BOUNDARY", XML_ERR_ENTITY_BOUNDARY, 0);
  add_integer_constant("XML_ERR_INVALID_URI", XML_ERR_INVALID_URI, 0);
  add_integer_constant("XML_ERR_URI_FRAGMENT", XML_ERR_URI_FRAGMENT, 0);
  add_integer_constant("XML_WAR_CATALOG_PI", XML_WAR_CATALOG_PI, 0);
  add_integer_constant("XML_ERR_NO_DTD", XML_ERR_NO_DTD, 0);
  add_integer_constant("XML_ERR_CONDSEC_INVALID_KEYWORD", XML_ERR_CONDSEC_INVALID_KEYWORD, 0);
  add_integer_constant("XML_ERR_VERSION_MISSING", XML_ERR_VERSION_MISSING, 0);
  add_integer_constant("XML_WAR_UNKNOWN_VERSION", XML_WAR_UNKNOWN_VERSION, 0);
  add_integer_constant("XML_WAR_LANG_VALUE", XML_WAR_LANG_VALUE, 0);
  add_integer_constant("XML_WAR_NS_URI", XML_WAR_NS_URI, 0);
  add_integer_constant("XML_WAR_NS_URI_RELATIVE", XML_WAR_NS_URI_RELATIVE, 0);
  add_integer_constant("XML_ERR_MISSING_ENCODING", XML_ERR_MISSING_ENCODING, 0);
  add_integer_constant("XML_NS_ERR_XML_NAMESPACE", XML_NS_ERR_XML_NAMESPACE, 0);
  add_integer_constant("XML_NS_ERR_UNDEFINED_NAMESPACE", XML_NS_ERR_UNDEFINED_NAMESPACE, 0);
  add_integer_constant("XML_NS_ERR_QNAME", XML_NS_ERR_QNAME, 0);
  add_integer_constant("XML_NS_ERR_ATTRIBUTE_REDEFINED", XML_NS_ERR_ATTRIBUTE_REDEFINED, 0);
  add_integer_constant("XML_DTD_ATTRIBUTE_DEFAULT", XML_DTD_ATTRIBUTE_DEFAULT, 0);
  add_integer_constant("XML_DTD_ATTRIBUTE_REDEFINED", XML_DTD_ATTRIBUTE_REDEFINED, 0);
  add_integer_constant("XML_DTD_ATTRIBUTE_VALUE", XML_DTD_ATTRIBUTE_VALUE, 0);
  add_integer_constant("XML_DTD_CONTENT_ERROR", XML_DTD_CONTENT_ERROR, 0);
  add_integer_constant("XML_DTD_CONTENT_MODEL", XML_DTD_CONTENT_MODEL, 0);
  add_integer_constant("XML_DTD_CONTENT_NOT_DETERMINIST", XML_DTD_CONTENT_NOT_DETERMINIST, 0);
  add_integer_constant("XML_DTD_DIFFERENT_PREFIX", XML_DTD_DIFFERENT_PREFIX, 0);
  add_integer_constant("XML_DTD_ELEM_DEFAULT_NAMESPACE", XML_DTD_ELEM_DEFAULT_NAMESPACE, 0);
  add_integer_constant("XML_DTD_ELEM_NAMESPACE", XML_DTD_ELEM_NAMESPACE, 0);
  add_integer_constant("XML_DTD_ELEM_REDEFINED", XML_DTD_ELEM_REDEFINED, 0);
  add_integer_constant("XML_DTD_EMPTY_NOTATION", XML_DTD_EMPTY_NOTATION, 0);
  add_integer_constant("XML_DTD_ENTITY_TYPE", XML_DTD_ENTITY_TYPE, 0);
  add_integer_constant("XML_DTD_ID_FIXED", XML_DTD_ID_FIXED, 0);
  add_integer_constant("XML_DTD_ID_REDEFINED", XML_DTD_ID_REDEFINED, 0);
  add_integer_constant("XML_DTD_ID_SUBSET", XML_DTD_ID_SUBSET, 0);
  add_integer_constant("XML_DTD_INVALID_CHILD", XML_DTD_INVALID_CHILD, 0);
  add_integer_constant("XML_DTD_INVALID_DEFAULT", XML_DTD_INVALID_DEFAULT, 0);
  add_integer_constant("XML_DTD_LOAD_ERROR", XML_DTD_LOAD_ERROR, 0);
  add_integer_constant("XML_DTD_MISSING_ATTRIBUTE", XML_DTD_MISSING_ATTRIBUTE, 0);
  add_integer_constant("XML_DTD_MIXED_CORRUPT", XML_DTD_MIXED_CORRUPT, 0);
  add_integer_constant("XML_DTD_MULTIPLE_ID", XML_DTD_MULTIPLE_ID, 0);
  add_integer_constant("XML_DTD_NO_DOC", XML_DTD_NO_DOC, 0);
  add_integer_constant("XML_DTD_NO_DTD", XML_DTD_NO_DTD, 0);
  add_integer_constant("XML_DTD_NO_ELEM_NAME", XML_DTD_NO_ELEM_NAME, 0);
  add_integer_constant("XML_DTD_NO_PREFIX", XML_DTD_NO_PREFIX, 0);
  add_integer_constant("XML_DTD_NO_ROOT", XML_DTD_NO_ROOT, 0);
  add_integer_constant("XML_DTD_NOTATION_REDEFINED", XML_DTD_NOTATION_REDEFINED, 0);
  add_integer_constant("XML_DTD_NOTATION_VALUE", XML_DTD_NOTATION_VALUE, 0);
  add_integer_constant("XML_DTD_NOT_EMPTY", XML_DTD_NOT_EMPTY, 0);
  add_integer_constant("XML_DTD_NOT_PCDATA", XML_DTD_NOT_PCDATA, 0);
  add_integer_constant("XML_DTD_NOT_STANDALONE", XML_DTD_NOT_STANDALONE, 0);
  add_integer_constant("XML_DTD_ROOT_NAME", XML_DTD_ROOT_NAME, 0);
  add_integer_constant("XML_DTD_STANDALONE_WHITE_SPACE", XML_DTD_STANDALONE_WHITE_SPACE, 0);
  add_integer_constant("XML_DTD_UNKNOWN_ATTRIBUTE", XML_DTD_UNKNOWN_ATTRIBUTE, 0);
  add_integer_constant("XML_DTD_UNKNOWN_ELEM", XML_DTD_UNKNOWN_ELEM, 0);
  add_integer_constant("XML_DTD_UNKNOWN_ENTITY", XML_DTD_UNKNOWN_ENTITY, 0);
  add_integer_constant("XML_DTD_UNKNOWN_ID", XML_DTD_UNKNOWN_ID, 0);
  add_integer_constant("XML_DTD_UNKNOWN_NOTATION", XML_DTD_UNKNOWN_NOTATION, 0);
  add_integer_constant("XML_DTD_STANDALONE_DEFAULTED", XML_DTD_STANDALONE_DEFAULTED, 0);
  add_integer_constant("XML_DTD_XMLID_VALUE", XML_DTD_XMLID_VALUE, 0);
  add_integer_constant("XML_DTD_XMLID_TYPE", XML_DTD_XMLID_TYPE, 0);
  add_integer_constant("XML_HTML_STRUCURE_ERROR", XML_HTML_STRUCURE_ERROR, 0);
  add_integer_constant("XML_HTML_UNKNOWN_TAG", XML_HTML_UNKNOWN_TAG, 0);
  add_integer_constant("XML_RNGP_ANYNAME_ATTR_ANCESTOR", XML_RNGP_ANYNAME_ATTR_ANCESTOR, 0);
  add_integer_constant("XML_RNGP_ATTR_CONFLICT", XML_RNGP_ATTR_CONFLICT, 0);
  add_integer_constant("XML_RNGP_ATTRIBUTE_CHILDREN", XML_RNGP_ATTRIBUTE_CHILDREN, 0);
  add_integer_constant("XML_RNGP_ATTRIBUTE_CONTENT", XML_RNGP_ATTRIBUTE_CONTENT, 0);
  add_integer_constant("XML_RNGP_ATTRIBUTE_EMPTY", XML_RNGP_ATTRIBUTE_EMPTY, 0);
  add_integer_constant("XML_RNGP_ATTRIBUTE_NOOP", XML_RNGP_ATTRIBUTE_NOOP, 0);
  add_integer_constant("XML_RNGP_CHOICE_CONTENT", XML_RNGP_CHOICE_CONTENT, 0);
  add_integer_constant("XML_RNGP_CHOICE_EMPTY", XML_RNGP_CHOICE_EMPTY, 0);
  add_integer_constant("XML_RNGP_CREATE_FAILURE", XML_RNGP_CREATE_FAILURE, 0);
  add_integer_constant("XML_RNGP_DATA_CONTENT", XML_RNGP_DATA_CONTENT, 0);
  add_integer_constant("XML_RNGP_DEF_CHOICE_AND_INTERLEAVE", XML_RNGP_DEF_CHOICE_AND_INTERLEAVE, 0);
  add_integer_constant("XML_RNGP_DEFINE_CREATE_FAILED", XML_RNGP_DEFINE_CREATE_FAILED, 0);
  add_integer_constant("XML_RNGP_DEFINE_EMPTY", XML_RNGP_DEFINE_EMPTY, 0);
  add_integer_constant("XML_RNGP_DEFINE_MISSING", XML_RNGP_DEFINE_MISSING, 0);
  add_integer_constant("XML_RNGP_DEFINE_NAME_MISSING", XML_RNGP_DEFINE_NAME_MISSING, 0);
  add_integer_constant("XML_RNGP_ELEM_CONTENT_EMPTY", XML_RNGP_ELEM_CONTENT_EMPTY, 0);
  add_integer_constant("XML_RNGP_ELEM_CONTENT_ERROR", XML_RNGP_ELEM_CONTENT_ERROR, 0);
  add_integer_constant("XML_RNGP_ELEMENT_EMPTY", XML_RNGP_ELEMENT_EMPTY, 0);
  add_integer_constant("XML_RNGP_ELEMENT_CONTENT", XML_RNGP_ELEMENT_CONTENT, 0);
  add_integer_constant("XML_RNGP_ELEMENT_NAME", XML_RNGP_ELEMENT_NAME, 0);
  add_integer_constant("XML_RNGP_ELEMENT_NO_CONTENT", XML_RNGP_ELEMENT_NO_CONTENT, 0);
  add_integer_constant("XML_RNGP_ELEM_TEXT_CONFLICT", XML_RNGP_ELEM_TEXT_CONFLICT, 0);
  add_integer_constant("XML_RNGP_EMPTY", XML_RNGP_EMPTY, 0);
  add_integer_constant("XML_RNGP_EMPTY_CONSTRUCT", XML_RNGP_EMPTY_CONSTRUCT, 0);
  add_integer_constant("XML_RNGP_EMPTY_CONTENT", XML_RNGP_EMPTY_CONTENT, 0);
  add_integer_constant("XML_RNGP_EMPTY_NOT_EMPTY", XML_RNGP_EMPTY_NOT_EMPTY, 0);
  add_integer_constant("XML_RNGP_ERROR_TYPE_LIB", XML_RNGP_ERROR_TYPE_LIB, 0);
  add_integer_constant("XML_RNGP_EXCEPT_EMPTY", XML_RNGP_EXCEPT_EMPTY, 0);
  add_integer_constant("XML_RNGP_EXCEPT_MISSING", XML_RNGP_EXCEPT_MISSING, 0);
  add_integer_constant("XML_RNGP_EXCEPT_MULTIPLE", XML_RNGP_EXCEPT_MULTIPLE, 0);
  add_integer_constant("XML_RNGP_EXCEPT_NO_CONTENT", XML_RNGP_EXCEPT_NO_CONTENT, 0);
  add_integer_constant("XML_RNGP_EXTERNALREF_EMTPY", XML_RNGP_EXTERNALREF_EMTPY, 0);
  add_integer_constant("XML_RNGP_EXTERNAL_REF_FAILURE", XML_RNGP_EXTERNAL_REF_FAILURE, 0);
  add_integer_constant("XML_RNGP_EXTERNALREF_RECURSE", XML_RNGP_EXTERNALREF_RECURSE, 0);
  add_integer_constant("XML_RNGP_FORBIDDEN_ATTRIBUTE", XML_RNGP_FORBIDDEN_ATTRIBUTE, 0);
  add_integer_constant("XML_RNGP_FOREIGN_ELEMENT", XML_RNGP_FOREIGN_ELEMENT, 0);
  add_integer_constant("XML_RNGP_GRAMMAR_CONTENT", XML_RNGP_GRAMMAR_CONTENT, 0);
  add_integer_constant("XML_RNGP_GRAMMAR_EMPTY", XML_RNGP_GRAMMAR_EMPTY, 0);
  add_integer_constant("XML_RNGP_GRAMMAR_MISSING", XML_RNGP_GRAMMAR_MISSING, 0);
  add_integer_constant("XML_RNGP_GRAMMAR_NO_START", XML_RNGP_GRAMMAR_NO_START, 0);
  add_integer_constant("XML_RNGP_GROUP_ATTR_CONFLICT", XML_RNGP_GROUP_ATTR_CONFLICT, 0);
  add_integer_constant("XML_RNGP_HREF_ERROR", XML_RNGP_HREF_ERROR, 0);
  add_integer_constant("XML_RNGP_INCLUDE_EMPTY", XML_RNGP_INCLUDE_EMPTY, 0);
  add_integer_constant("XML_RNGP_INCLUDE_FAILURE", XML_RNGP_INCLUDE_FAILURE, 0);
  add_integer_constant("XML_RNGP_INCLUDE_RECURSE", XML_RNGP_INCLUDE_RECURSE, 0);
  add_integer_constant("XML_RNGP_INTERLEAVE_ADD", XML_RNGP_INTERLEAVE_ADD, 0);
  add_integer_constant("XML_RNGP_INTERLEAVE_CREATE_FAILED", XML_RNGP_INTERLEAVE_CREATE_FAILED, 0);
  add_integer_constant("XML_RNGP_INTERLEAVE_EMPTY", XML_RNGP_INTERLEAVE_EMPTY, 0);
  add_integer_constant("XML_RNGP_INTERLEAVE_NO_CONTENT", XML_RNGP_INTERLEAVE_NO_CONTENT, 0);
  add_integer_constant("XML_RNGP_INVALID_DEFINE_NAME", XML_RNGP_INVALID_DEFINE_NAME, 0);
  add_integer_constant("XML_RNGP_INVALID_URI", XML_RNGP_INVALID_URI, 0);
  add_integer_constant("XML_RNGP_INVALID_VALUE", XML_RNGP_INVALID_VALUE, 0);
  add_integer_constant("XML_RNGP_MISSING_HREF", XML_RNGP_MISSING_HREF, 0);
  add_integer_constant("XML_RNGP_NAME_MISSING", XML_RNGP_NAME_MISSING, 0);
  add_integer_constant("XML_RNGP_NEED_COMBINE", XML_RNGP_NEED_COMBINE, 0);
  add_integer_constant("XML_RNGP_NOTALLOWED_NOT_EMPTY", XML_RNGP_NOTALLOWED_NOT_EMPTY, 0);
  add_integer_constant("XML_RNGP_NSNAME_ATTR_ANCESTOR", XML_RNGP_NSNAME_ATTR_ANCESTOR, 0);
  add_integer_constant("XML_RNGP_NSNAME_NO_NS", XML_RNGP_NSNAME_NO_NS, 0);
  add_integer_constant("XML_RNGP_PARAM_FORBIDDEN", XML_RNGP_PARAM_FORBIDDEN, 0);
  add_integer_constant("XML_RNGP_PARAM_NAME_MISSING", XML_RNGP_PARAM_NAME_MISSING, 0);
  add_integer_constant("XML_RNGP_PARENTREF_CREATE_FAILED", XML_RNGP_PARENTREF_CREATE_FAILED, 0);
  add_integer_constant("XML_RNGP_PARENTREF_NAME_INVALID", XML_RNGP_PARENTREF_NAME_INVALID, 0);
  add_integer_constant("XML_RNGP_PARENTREF_NO_NAME", XML_RNGP_PARENTREF_NO_NAME, 0);
  add_integer_constant("XML_RNGP_PARENTREF_NO_PARENT", XML_RNGP_PARENTREF_NO_PARENT, 0);
  add_integer_constant("XML_RNGP_PARENTREF_NOT_EMPTY", XML_RNGP_PARENTREF_NOT_EMPTY, 0);
  add_integer_constant("XML_RNGP_PARSE_ERROR", XML_RNGP_PARSE_ERROR, 0);
  add_integer_constant("XML_RNGP_PAT_ANYNAME_EXCEPT_ANYNAME", XML_RNGP_PAT_ANYNAME_EXCEPT_ANYNAME, 0);
  add_integer_constant("XML_RNGP_PAT_ATTR_ATTR", XML_RNGP_PAT_ATTR_ATTR, 0);
  add_integer_constant("XML_RNGP_PAT_ATTR_ELEM", XML_RNGP_PAT_ATTR_ELEM, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_ATTR", XML_RNGP_PAT_DATA_EXCEPT_ATTR, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_ELEM", XML_RNGP_PAT_DATA_EXCEPT_ELEM, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_EMPTY", XML_RNGP_PAT_DATA_EXCEPT_EMPTY, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_GROUP", XML_RNGP_PAT_DATA_EXCEPT_GROUP, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_INTERLEAVE", XML_RNGP_PAT_DATA_EXCEPT_INTERLEAVE, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_LIST", XML_RNGP_PAT_DATA_EXCEPT_LIST, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_ONEMORE", XML_RNGP_PAT_DATA_EXCEPT_ONEMORE, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_REF", XML_RNGP_PAT_DATA_EXCEPT_REF, 0);
  add_integer_constant("XML_RNGP_PAT_DATA_EXCEPT_TEXT", XML_RNGP_PAT_DATA_EXCEPT_TEXT, 0);
  add_integer_constant("XML_RNGP_PAT_LIST_ATTR", XML_RNGP_PAT_LIST_ATTR, 0);
  add_integer_constant("XML_RNGP_PAT_LIST_ELEM", XML_RNGP_PAT_LIST_ELEM, 0);
  add_integer_constant("XML_RNGP_PAT_LIST_INTERLEAVE", XML_RNGP_PAT_LIST_INTERLEAVE, 0);
  add_integer_constant("XML_RNGP_PAT_LIST_LIST", XML_RNGP_PAT_LIST_LIST, 0);
  add_integer_constant("XML_RNGP_PAT_LIST_REF", XML_RNGP_PAT_LIST_REF, 0);
  add_integer_constant("XML_RNGP_PAT_LIST_TEXT", XML_RNGP_PAT_LIST_TEXT, 0);
  add_integer_constant("XML_RNGP_PAT_NSNAME_EXCEPT_ANYNAME", XML_RNGP_PAT_NSNAME_EXCEPT_ANYNAME, 0);
  add_integer_constant("XML_RNGP_PAT_NSNAME_EXCEPT_NSNAME", XML_RNGP_PAT_NSNAME_EXCEPT_NSNAME, 0);
  add_integer_constant("XML_RNGP_PAT_ONEMORE_GROUP_ATTR", XML_RNGP_PAT_ONEMORE_GROUP_ATTR, 0);
  add_integer_constant("XML_RNGP_PAT_ONEMORE_INTERLEAVE_ATTR", XML_RNGP_PAT_ONEMORE_INTERLEAVE_ATTR, 0);
  add_integer_constant("XML_RNGP_PAT_START_ATTR", XML_RNGP_PAT_START_ATTR, 0);
  add_integer_constant("XML_RNGP_PAT_START_DATA", XML_RNGP_PAT_START_DATA, 0);
  add_integer_constant("XML_RNGP_PAT_START_EMPTY", XML_RNGP_PAT_START_EMPTY, 0);
  add_integer_constant("XML_RNGP_PAT_START_GROUP", XML_RNGP_PAT_START_GROUP, 0);
  add_integer_constant("XML_RNGP_PAT_START_INTERLEAVE", XML_RNGP_PAT_START_INTERLEAVE, 0);
  add_integer_constant("XML_RNGP_PAT_START_LIST", XML_RNGP_PAT_START_LIST, 0);
  add_integer_constant("XML_RNGP_PAT_START_ONEMORE", XML_RNGP_PAT_START_ONEMORE, 0);
  add_integer_constant("XML_RNGP_PAT_START_TEXT", XML_RNGP_PAT_START_TEXT, 0);
  add_integer_constant("XML_RNGP_PAT_START_VALUE", XML_RNGP_PAT_START_VALUE, 0);
  add_integer_constant("XML_RNGP_PREFIX_UNDEFINED", XML_RNGP_PREFIX_UNDEFINED, 0);
  add_integer_constant("XML_RNGP_REF_CREATE_FAILED", XML_RNGP_REF_CREATE_FAILED, 0);
  add_integer_constant("XML_RNGP_REF_CYCLE", XML_RNGP_REF_CYCLE, 0);
  add_integer_constant("XML_RNGP_REF_NAME_INVALID", XML_RNGP_REF_NAME_INVALID, 0);
  add_integer_constant("XML_RNGP_REF_NO_DEF", XML_RNGP_REF_NO_DEF, 0);
  add_integer_constant("XML_RNGP_REF_NO_NAME", XML_RNGP_REF_NO_NAME, 0);
  add_integer_constant("XML_RNGP_REF_NOT_EMPTY", XML_RNGP_REF_NOT_EMPTY, 0);
  add_integer_constant("XML_RNGP_START_CHOICE_AND_INTERLEAVE", XML_RNGP_START_CHOICE_AND_INTERLEAVE, 0);
  add_integer_constant("XML_RNGP_START_CONTENT", XML_RNGP_START_CONTENT, 0);
  add_integer_constant("XML_RNGP_START_EMPTY", XML_RNGP_START_EMPTY, 0);
  add_integer_constant("XML_RNGP_START_MISSING", XML_RNGP_START_MISSING, 0);
  add_integer_constant("XML_RNGP_TEXT_EXPECTED", XML_RNGP_TEXT_EXPECTED, 0);
  add_integer_constant("XML_RNGP_TEXT_HAS_CHILD", XML_RNGP_TEXT_HAS_CHILD, 0);
  add_integer_constant("XML_RNGP_TYPE_MISSING", XML_RNGP_TYPE_MISSING, 0);
  add_integer_constant("XML_RNGP_TYPE_NOT_FOUND", XML_RNGP_TYPE_NOT_FOUND, 0);
  add_integer_constant("XML_RNGP_TYPE_VALUE", XML_RNGP_TYPE_VALUE, 0);
  add_integer_constant("XML_RNGP_UNKNOWN_ATTRIBUTE", XML_RNGP_UNKNOWN_ATTRIBUTE, 0);
  add_integer_constant("XML_RNGP_UNKNOWN_COMBINE", XML_RNGP_UNKNOWN_COMBINE, 0);
  add_integer_constant("XML_RNGP_UNKNOWN_CONSTRUCT", XML_RNGP_UNKNOWN_CONSTRUCT, 0);
  add_integer_constant("XML_RNGP_UNKNOWN_TYPE_LIB", XML_RNGP_UNKNOWN_TYPE_LIB, 0);
  add_integer_constant("XML_RNGP_URI_FRAGMENT", XML_RNGP_URI_FRAGMENT, 0);
  add_integer_constant("XML_RNGP_URI_NOT_ABSOLUTE", XML_RNGP_URI_NOT_ABSOLUTE, 0);
  add_integer_constant("XML_RNGP_VALUE_EMPTY", XML_RNGP_VALUE_EMPTY, 0);
  add_integer_constant("XML_RNGP_VALUE_NO_CONTENT", XML_RNGP_VALUE_NO_CONTENT, 0);
  add_integer_constant("XML_RNGP_XMLNS_NAME", XML_RNGP_XMLNS_NAME, 0);
  add_integer_constant("XML_RNGP_XML_NS", XML_RNGP_XML_NS, 0);
  add_integer_constant("XML_XPATH_EXPRESSION_OK", XML_XPATH_EXPRESSION_OK, 0);
  add_integer_constant("XML_XPATH_NUMBER_ERROR", XML_XPATH_NUMBER_ERROR, 0);
  add_integer_constant("XML_XPATH_UNFINISHED_LITERAL_ERROR", XML_XPATH_UNFINISHED_LITERAL_ERROR, 0);
  add_integer_constant("XML_XPATH_START_LITERAL_ERROR", XML_XPATH_START_LITERAL_ERROR, 0);
  add_integer_constant("XML_XPATH_VARIABLE_REF_ERROR", XML_XPATH_VARIABLE_REF_ERROR, 0);
  add_integer_constant("XML_XPATH_UNDEF_VARIABLE_ERROR", XML_XPATH_UNDEF_VARIABLE_ERROR, 0);
  add_integer_constant("XML_XPATH_INVALID_PREDICATE_ERROR", XML_XPATH_INVALID_PREDICATE_ERROR, 0);
  add_integer_constant("XML_XPATH_EXPR_ERROR", XML_XPATH_EXPR_ERROR, 0);
  add_integer_constant("XML_XPATH_UNCLOSED_ERROR", XML_XPATH_UNCLOSED_ERROR, 0);
  add_integer_constant("XML_XPATH_UNKNOWN_FUNC_ERROR", XML_XPATH_UNKNOWN_FUNC_ERROR, 0);
  add_integer_constant("XML_XPATH_INVALID_OPERAND", XML_XPATH_INVALID_OPERAND, 0);
  add_integer_constant("XML_XPATH_INVALID_TYPE", XML_XPATH_INVALID_TYPE, 0);
  add_integer_constant("XML_XPATH_INVALID_ARITY", XML_XPATH_INVALID_ARITY, 0);
  add_integer_constant("XML_XPATH_INVALID_CTXT_SIZE", XML_XPATH_INVALID_CTXT_SIZE, 0);
  add_integer_constant("XML_XPATH_INVALID_CTXT_POSITION", XML_XPATH_INVALID_CTXT_POSITION, 0);
  add_integer_constant("XML_XPATH_MEMORY_ERROR", XML_XPATH_MEMORY_ERROR, 0);
  add_integer_constant("XML_XPTR_SYNTAX_ERROR", XML_XPTR_SYNTAX_ERROR, 0);
  add_integer_constant("XML_XPTR_RESOURCE_ERROR", XML_XPTR_RESOURCE_ERROR, 0);
  add_integer_constant("XML_XPTR_SUB_RESOURCE_ERROR", XML_XPTR_SUB_RESOURCE_ERROR, 0);
  add_integer_constant("XML_XPATH_UNDEF_PREFIX_ERROR", XML_XPATH_UNDEF_PREFIX_ERROR, 0);
  add_integer_constant("XML_XPATH_ENCODING_ERROR", XML_XPATH_ENCODING_ERROR, 0);
  add_integer_constant("XML_XPATH_INVALID_CHAR_ERROR", XML_XPATH_INVALID_CHAR_ERROR, 0);
  add_integer_constant("XML_TREE_INVALID_HEX", XML_TREE_INVALID_HEX, 0);
  add_integer_constant("XML_TREE_INVALID_DEC", XML_TREE_INVALID_DEC, 0);
  add_integer_constant("XML_TREE_UNTERMINATED_ENTITY", XML_TREE_UNTERMINATED_ENTITY, 0);
  add_integer_constant("XML_SAVE_NOT_UTF8", XML_SAVE_NOT_UTF8, 0);
  add_integer_constant("XML_SAVE_CHAR_INVALID", XML_SAVE_CHAR_INVALID, 0);
  add_integer_constant("XML_SAVE_NO_DOCTYPE", XML_SAVE_NO_DOCTYPE, 0);
  add_integer_constant("XML_SAVE_UNKNOWN_ENCODING", XML_SAVE_UNKNOWN_ENCODING, 0);
  add_integer_constant("XML_REGEXP_COMPILE_ERROR", XML_REGEXP_COMPILE_ERROR, 0);
  add_integer_constant("XML_IO_UNKNOWN", XML_IO_UNKNOWN, 0);
  add_integer_constant("XML_IO_EACCES", XML_IO_EACCES, 0);
  add_integer_constant("XML_IO_EAGAIN", XML_IO_EAGAIN, 0);
  add_integer_constant("XML_IO_EBADF", XML_IO_EBADF, 0);
  add_integer_constant("XML_IO_EBADMSG", XML_IO_EBADMSG, 0);
  add_integer_constant("XML_IO_EBUSY", XML_IO_EBUSY, 0);
  add_integer_constant("XML_IO_ECANCELED", XML_IO_ECANCELED, 0);
  add_integer_constant("XML_IO_ECHILD", XML_IO_ECHILD, 0);
  add_integer_constant("XML_IO_EDEADLK", XML_IO_EDEADLK, 0);
  add_integer_constant("XML_IO_EDOM", XML_IO_EDOM, 0);
  add_integer_constant("XML_IO_EEXIST", XML_IO_EEXIST, 0);
  add_integer_constant("XML_IO_EFAULT", XML_IO_EFAULT, 0);
  add_integer_constant("XML_IO_EFBIG", XML_IO_EFBIG, 0);
  add_integer_constant("XML_IO_EINPROGRESS", XML_IO_EINPROGRESS, 0);
  add_integer_constant("XML_IO_EINTR", XML_IO_EINTR, 0);
  add_integer_constant("XML_IO_EINVAL", XML_IO_EINVAL, 0);
  add_integer_constant("XML_IO_EIO", XML_IO_EIO, 0);
  add_integer_constant("XML_IO_EISDIR", XML_IO_EISDIR, 0);
  add_integer_constant("XML_IO_EMFILE", XML_IO_EMFILE, 0);
  add_integer_constant("XML_IO_EMLINK", XML_IO_EMLINK, 0);
  add_integer_constant("XML_IO_EMSGSIZE", XML_IO_EMSGSIZE, 0);
  add_integer_constant("XML_IO_ENAMETOOLONG", XML_IO_ENAMETOOLONG, 0);
  add_integer_constant("XML_IO_ENFILE", XML_IO_ENFILE, 0);
  add_integer_constant("XML_IO_ENODEV", XML_IO_ENODEV, 0);
  add_integer_constant("XML_IO_ENOENT", XML_IO_ENOENT, 0);
  add_integer_constant("XML_IO_ENOEXEC", XML_IO_ENOEXEC, 0);
  add_integer_constant("XML_IO_ENOLCK", XML_IO_ENOLCK, 0);
  add_integer_constant("XML_IO_ENOMEM", XML_IO_ENOMEM, 0);
  add_integer_constant("XML_IO_ENOSPC", XML_IO_ENOSPC, 0);
  add_integer_constant("XML_IO_ENOSYS", XML_IO_ENOSYS, 0);
  add_integer_constant("XML_IO_ENOTDIR", XML_IO_ENOTDIR, 0);
  add_integer_constant("XML_IO_ENOTEMPTY", XML_IO_ENOTEMPTY, 0);
  add_integer_constant("XML_IO_ENOTSUP", XML_IO_ENOTSUP, 0);
  add_integer_constant("XML_IO_ENOTTY", XML_IO_ENOTTY, 0);
  add_integer_constant("XML_IO_ENXIO", XML_IO_ENXIO, 0);
  add_integer_constant("XML_IO_EPERM", XML_IO_EPERM, 0);
  add_integer_constant("XML_IO_EPIPE", XML_IO_EPIPE, 0);
  add_integer_constant("XML_IO_ERANGE", XML_IO_ERANGE, 0);
  add_integer_constant("XML_IO_EROFS", XML_IO_EROFS, 0);
  add_integer_constant("XML_IO_ESPIPE", XML_IO_ESPIPE, 0);
  add_integer_constant("XML_IO_ESRCH", XML_IO_ESRCH, 0);
  add_integer_constant("XML_IO_ETIMEDOUT", XML_IO_ETIMEDOUT, 0);
  add_integer_constant("XML_IO_EXDEV", XML_IO_EXDEV, 0);
  add_integer_constant("XML_IO_NETWORK_ATTEMPT", XML_IO_NETWORK_ATTEMPT, 0);
  add_integer_constant("XML_IO_ENCODER", XML_IO_ENCODER, 0);
  add_integer_constant("XML_IO_FLUSH", XML_IO_FLUSH, 0);
  add_integer_constant("XML_IO_WRITE", XML_IO_WRITE, 0);
  add_integer_constant("XML_IO_NO_INPUT", XML_IO_NO_INPUT, 0);
  add_integer_constant("XML_IO_BUFFER_FULL", XML_IO_BUFFER_FULL, 0);
  add_integer_constant("XML_IO_LOAD_ERROR", XML_IO_LOAD_ERROR, 0);
  add_integer_constant("XML_IO_ENOTSOCK", XML_IO_ENOTSOCK, 0);
  add_integer_constant("XML_IO_EISCONN", XML_IO_EISCONN, 0);
  add_integer_constant("XML_IO_ECONNREFUSED", XML_IO_ECONNREFUSED, 0);
  add_integer_constant("XML_IO_ENETUNREACH", XML_IO_ENETUNREACH, 0);
  add_integer_constant("XML_IO_EADDRINUSE", XML_IO_EADDRINUSE, 0);
  add_integer_constant("XML_IO_EALREADY", XML_IO_EALREADY, 0);
  add_integer_constant("XML_IO_EAFNOSUPPORT", XML_IO_EAFNOSUPPORT, 0);
  add_integer_constant("XML_XINCLUDE_RECURSION", XML_XINCLUDE_RECURSION, 0);
  add_integer_constant("XML_XINCLUDE_PARSE_VALUE", XML_XINCLUDE_PARSE_VALUE, 0);
  add_integer_constant("XML_XINCLUDE_ENTITY_DEF_MISMATCH", XML_XINCLUDE_ENTITY_DEF_MISMATCH, 0);
  add_integer_constant("XML_XINCLUDE_NO_HREF", XML_XINCLUDE_NO_HREF, 0);
  add_integer_constant("XML_XINCLUDE_NO_FALLBACK", XML_XINCLUDE_NO_FALLBACK, 0);
  add_integer_constant("XML_XINCLUDE_HREF_URI", XML_XINCLUDE_HREF_URI, 0);
  add_integer_constant("XML_XINCLUDE_TEXT_FRAGMENT", XML_XINCLUDE_TEXT_FRAGMENT, 0);
  add_integer_constant("XML_XINCLUDE_TEXT_DOCUMENT", XML_XINCLUDE_TEXT_DOCUMENT, 0);
  add_integer_constant("XML_XINCLUDE_INVALID_CHAR", XML_XINCLUDE_INVALID_CHAR, 0);
  add_integer_constant("XML_XINCLUDE_BUILD_FAILED", XML_XINCLUDE_BUILD_FAILED, 0);
  add_integer_constant("XML_XINCLUDE_UNKNOWN_ENCODING", XML_XINCLUDE_UNKNOWN_ENCODING, 0);
  add_integer_constant("XML_XINCLUDE_MULTIPLE_ROOT", XML_XINCLUDE_MULTIPLE_ROOT, 0);
  add_integer_constant("XML_XINCLUDE_XPTR_FAILED", XML_XINCLUDE_XPTR_FAILED, 0);
  add_integer_constant("XML_XINCLUDE_XPTR_RESULT", XML_XINCLUDE_XPTR_RESULT, 0);
  add_integer_constant("XML_XINCLUDE_INCLUDE_IN_INCLUDE", XML_XINCLUDE_INCLUDE_IN_INCLUDE, 0);
  add_integer_constant("XML_XINCLUDE_FALLBACKS_IN_INCLUDE", XML_XINCLUDE_FALLBACKS_IN_INCLUDE, 0);
  add_integer_constant("XML_XINCLUDE_FALLBACK_NOT_IN_INCLUDE", XML_XINCLUDE_FALLBACK_NOT_IN_INCLUDE, 0);
  add_integer_constant("XML_XINCLUDE_DEPRECATED_NS", XML_XINCLUDE_DEPRECATED_NS, 0);
  add_integer_constant("XML_XINCLUDE_FRAGMENT_ID", XML_XINCLUDE_FRAGMENT_ID, 0);
  add_integer_constant("XML_CATALOG_MISSING_ATTR", XML_CATALOG_MISSING_ATTR, 0);
  add_integer_constant("XML_CATALOG_ENTRY_BROKEN", XML_CATALOG_ENTRY_BROKEN, 0);
  add_integer_constant("XML_CATALOG_PREFER_VALUE", XML_CATALOG_PREFER_VALUE, 0);
  add_integer_constant("XML_CATALOG_NOT_CATALOG", XML_CATALOG_NOT_CATALOG, 0);
  add_integer_constant("XML_CATALOG_RECURSION", XML_CATALOG_RECURSION, 0);
  add_integer_constant("XML_SCHEMAP_PREFIX_UNDEFINED", XML_SCHEMAP_PREFIX_UNDEFINED, 0);
  add_integer_constant("XML_SCHEMAP_ATTRFORMDEFAULT_VALUE", XML_SCHEMAP_ATTRFORMDEFAULT_VALUE, 0);
  add_integer_constant("XML_SCHEMAP_ATTRGRP_NONAME_NOREF", XML_SCHEMAP_ATTRGRP_NONAME_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_ATTR_NONAME_NOREF", XML_SCHEMAP_ATTR_NONAME_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_COMPLEXTYPE_NONAME_NOREF", XML_SCHEMAP_COMPLEXTYPE_NONAME_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_ELEMFORMDEFAULT_VALUE", XML_SCHEMAP_ELEMFORMDEFAULT_VALUE, 0);
  add_integer_constant("XML_SCHEMAP_ELEM_NONAME_NOREF", XML_SCHEMAP_ELEM_NONAME_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_EXTENSION_NO_BASE", XML_SCHEMAP_EXTENSION_NO_BASE, 0);
  add_integer_constant("XML_SCHEMAP_FACET_NO_VALUE", XML_SCHEMAP_FACET_NO_VALUE, 0);
  add_integer_constant("XML_SCHEMAP_FAILED_BUILD_IMPORT", XML_SCHEMAP_FAILED_BUILD_IMPORT, 0);
  add_integer_constant("XML_SCHEMAP_GROUP_NONAME_NOREF", XML_SCHEMAP_GROUP_NONAME_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_IMPORT_NAMESPACE_NOT_URI", XML_SCHEMAP_IMPORT_NAMESPACE_NOT_URI, 0);
  add_integer_constant("XML_SCHEMAP_IMPORT_REDEFINE_NSNAME", XML_SCHEMAP_IMPORT_REDEFINE_NSNAME, 0);
  add_integer_constant("XML_SCHEMAP_IMPORT_SCHEMA_NOT_URI", XML_SCHEMAP_IMPORT_SCHEMA_NOT_URI, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_BOOLEAN", XML_SCHEMAP_INVALID_BOOLEAN, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_ENUM", XML_SCHEMAP_INVALID_ENUM, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_FACET", XML_SCHEMAP_INVALID_FACET, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_FACET_VALUE", XML_SCHEMAP_INVALID_FACET_VALUE, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_MAXOCCURS", XML_SCHEMAP_INVALID_MAXOCCURS, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_MINOCCURS", XML_SCHEMAP_INVALID_MINOCCURS, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_REF_AND_SUBTYPE", XML_SCHEMAP_INVALID_REF_AND_SUBTYPE, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_WHITE_SPACE", XML_SCHEMAP_INVALID_WHITE_SPACE, 0);
  add_integer_constant("XML_SCHEMAP_NOATTR_NOREF", XML_SCHEMAP_NOATTR_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_NOTATION_NO_NAME", XML_SCHEMAP_NOTATION_NO_NAME, 0);
  add_integer_constant("XML_SCHEMAP_NOTYPE_NOREF", XML_SCHEMAP_NOTYPE_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_REF_AND_SUBTYPE", XML_SCHEMAP_REF_AND_SUBTYPE, 0);
  add_integer_constant("XML_SCHEMAP_RESTRICTION_NONAME_NOREF", XML_SCHEMAP_RESTRICTION_NONAME_NOREF, 0);
  add_integer_constant("XML_SCHEMAP_SIMPLETYPE_NONAME", XML_SCHEMAP_SIMPLETYPE_NONAME, 0);
  add_integer_constant("XML_SCHEMAP_TYPE_AND_SUBTYPE", XML_SCHEMAP_TYPE_AND_SUBTYPE, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_ALL_CHILD", XML_SCHEMAP_UNKNOWN_ALL_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_ANYATTRIBUTE_CHILD", XML_SCHEMAP_UNKNOWN_ANYATTRIBUTE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_ATTR_CHILD", XML_SCHEMAP_UNKNOWN_ATTR_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_ATTRGRP_CHILD", XML_SCHEMAP_UNKNOWN_ATTRGRP_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_ATTRIBUTE_GROUP", XML_SCHEMAP_UNKNOWN_ATTRIBUTE_GROUP, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_BASE_TYPE", XML_SCHEMAP_UNKNOWN_BASE_TYPE, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_CHOICE_CHILD", XML_SCHEMAP_UNKNOWN_CHOICE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_COMPLEXCONTENT_CHILD", XML_SCHEMAP_UNKNOWN_COMPLEXCONTENT_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_COMPLEXTYPE_CHILD", XML_SCHEMAP_UNKNOWN_COMPLEXTYPE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_ELEM_CHILD", XML_SCHEMAP_UNKNOWN_ELEM_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_EXTENSION_CHILD", XML_SCHEMAP_UNKNOWN_EXTENSION_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_FACET_CHILD", XML_SCHEMAP_UNKNOWN_FACET_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_FACET_TYPE", XML_SCHEMAP_UNKNOWN_FACET_TYPE, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_GROUP_CHILD", XML_SCHEMAP_UNKNOWN_GROUP_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_IMPORT_CHILD", XML_SCHEMAP_UNKNOWN_IMPORT_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_LIST_CHILD", XML_SCHEMAP_UNKNOWN_LIST_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_NOTATION_CHILD", XML_SCHEMAP_UNKNOWN_NOTATION_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_PROCESSCONTENT_CHILD", XML_SCHEMAP_UNKNOWN_PROCESSCONTENT_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_REF", XML_SCHEMAP_UNKNOWN_REF, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_RESTRICTION_CHILD", XML_SCHEMAP_UNKNOWN_RESTRICTION_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_SCHEMAS_CHILD", XML_SCHEMAP_UNKNOWN_SCHEMAS_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_SEQUENCE_CHILD", XML_SCHEMAP_UNKNOWN_SEQUENCE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_SIMPLECONTENT_CHILD", XML_SCHEMAP_UNKNOWN_SIMPLECONTENT_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_SIMPLETYPE_CHILD", XML_SCHEMAP_UNKNOWN_SIMPLETYPE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_TYPE", XML_SCHEMAP_UNKNOWN_TYPE, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_UNION_CHILD", XML_SCHEMAP_UNKNOWN_UNION_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_ELEM_DEFAULT_FIXED", XML_SCHEMAP_ELEM_DEFAULT_FIXED, 0);
  add_integer_constant("XML_SCHEMAP_REGEXP_INVALID", XML_SCHEMAP_REGEXP_INVALID, 0);
  add_integer_constant("XML_SCHEMAP_FAILED_LOAD", XML_SCHEMAP_FAILED_LOAD, 0);
  add_integer_constant("XML_SCHEMAP_NOTHING_TO_PARSE", XML_SCHEMAP_NOTHING_TO_PARSE, 0);
  add_integer_constant("XML_SCHEMAP_NOROOT", XML_SCHEMAP_NOROOT, 0);
  add_integer_constant("XML_SCHEMAP_REDEFINED_GROUP", XML_SCHEMAP_REDEFINED_GROUP, 0);
  add_integer_constant("XML_SCHEMAP_REDEFINED_TYPE", XML_SCHEMAP_REDEFINED_TYPE, 0);
  add_integer_constant("XML_SCHEMAP_REDEFINED_ELEMENT", XML_SCHEMAP_REDEFINED_ELEMENT, 0);
  add_integer_constant("XML_SCHEMAP_REDEFINED_ATTRGROUP", XML_SCHEMAP_REDEFINED_ATTRGROUP, 0);
  add_integer_constant("XML_SCHEMAP_REDEFINED_ATTR", XML_SCHEMAP_REDEFINED_ATTR, 0);
  add_integer_constant("XML_SCHEMAP_REDEFINED_NOTATION", XML_SCHEMAP_REDEFINED_NOTATION, 0);
  add_integer_constant("XML_SCHEMAP_FAILED_PARSE", XML_SCHEMAP_FAILED_PARSE, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_PREFIX", XML_SCHEMAP_UNKNOWN_PREFIX, 0);
  add_integer_constant("XML_SCHEMAP_DEF_AND_PREFIX", XML_SCHEMAP_DEF_AND_PREFIX, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_INCLUDE_CHILD", XML_SCHEMAP_UNKNOWN_INCLUDE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_INCLUDE_SCHEMA_NOT_URI", XML_SCHEMAP_INCLUDE_SCHEMA_NOT_URI, 0);
  add_integer_constant("XML_SCHEMAP_INCLUDE_SCHEMA_NO_URI", XML_SCHEMAP_INCLUDE_SCHEMA_NO_URI, 0);
  add_integer_constant("XML_SCHEMAP_NOT_SCHEMA", XML_SCHEMAP_NOT_SCHEMA, 0);
  add_integer_constant("XML_SCHEMAP_UNKNOWN_MEMBER_TYPE", XML_SCHEMAP_UNKNOWN_MEMBER_TYPE, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_ATTR_USE", XML_SCHEMAP_INVALID_ATTR_USE, 0);
  add_integer_constant("XML_SCHEMAP_RECURSIVE", XML_SCHEMAP_RECURSIVE, 0);
  add_integer_constant("XML_SCHEMAP_SUPERNUMEROUS_LIST_ITEM_TYPE", XML_SCHEMAP_SUPERNUMEROUS_LIST_ITEM_TYPE, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_ATTR_COMBINATION", XML_SCHEMAP_INVALID_ATTR_COMBINATION, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_ATTR_INLINE_COMBINATION", XML_SCHEMAP_INVALID_ATTR_INLINE_COMBINATION, 0);
  add_integer_constant("XML_SCHEMAP_MISSING_SIMPLETYPE_CHILD", XML_SCHEMAP_MISSING_SIMPLETYPE_CHILD, 0);
  add_integer_constant("XML_SCHEMAP_INVALID_ATTR_NAME", XML_SCHEMAP_INVALID_ATTR_NAME, 0);
  add_integer_constant("XML_SCHEMAP_REF_AND_CONTENT", XML_SCHEMAP_REF_AND_CONTENT, 0);
  add_integer_constant("XML_SCHEMAV_NOROOT", XML_SCHEMAV_NOROOT, 0);
  add_integer_constant("XML_SCHEMAV_UNDECLAREDELEM", XML_SCHEMAV_UNDECLAREDELEM, 0);
  add_integer_constant("XML_SCHEMAV_NOTTOPLEVEL", XML_SCHEMAV_NOTTOPLEVEL, 0);
  add_integer_constant("XML_SCHEMAV_MISSING", XML_SCHEMAV_MISSING, 0);
  add_integer_constant("XML_SCHEMAV_WRONGELEM", XML_SCHEMAV_WRONGELEM, 0);
  add_integer_constant("XML_SCHEMAV_NOTYPE", XML_SCHEMAV_NOTYPE, 0);
  add_integer_constant("XML_SCHEMAV_NOROLLBACK", XML_SCHEMAV_NOROLLBACK, 0);
  add_integer_constant("XML_SCHEMAV_ISABSTRACT", XML_SCHEMAV_ISABSTRACT, 0);
  add_integer_constant("XML_SCHEMAV_NOTEMPTY", XML_SCHEMAV_NOTEMPTY, 0);
  add_integer_constant("XML_SCHEMAV_ELEMCONT", XML_SCHEMAV_ELEMCONT, 0);
  add_integer_constant("XML_SCHEMAV_HAVEDEFAULT", XML_SCHEMAV_HAVEDEFAULT, 0);
  add_integer_constant("XML_SCHEMAV_NOTNILLABLE", XML_SCHEMAV_NOTNILLABLE, 0);
  add_integer_constant("XML_SCHEMAV_EXTRACONTENT", XML_SCHEMAV_EXTRACONTENT, 0);
  add_integer_constant("XML_SCHEMAV_INVALIDATTR", XML_SCHEMAV_INVALIDATTR, 0);
  add_integer_constant("XML_SCHEMAV_INVALIDELEM", XML_SCHEMAV_INVALIDELEM, 0);
  add_integer_constant("XML_SCHEMAV_NOTDETERMINIST", XML_SCHEMAV_NOTDETERMINIST, 0);
  add_integer_constant("XML_SCHEMAV_CONSTRUCT", XML_SCHEMAV_CONSTRUCT, 0);
  add_integer_constant("XML_SCHEMAV_INTERNAL", XML_SCHEMAV_INTERNAL, 0);
  add_integer_constant("XML_SCHEMAV_NOTSIMPLE", XML_SCHEMAV_NOTSIMPLE, 0);
  add_integer_constant("XML_SCHEMAV_ATTRUNKNOWN", XML_SCHEMAV_ATTRUNKNOWN, 0);
  add_integer_constant("XML_SCHEMAV_ATTRINVALID", XML_SCHEMAV_ATTRINVALID, 0);
  add_integer_constant("XML_SCHEMAV_VALUE", XML_SCHEMAV_VALUE, 0);
  add_integer_constant("XML_SCHEMAV_FACET", XML_SCHEMAV_FACET, 0);
  add_integer_constant("XML_XPTR_UNKNOWN_SCHEME", XML_XPTR_UNKNOWN_SCHEME, 0);
  add_integer_constant("XML_XPTR_CHILDSEQ_START", XML_XPTR_CHILDSEQ_START, 0);
  add_integer_constant("XML_XPTR_EVAL_FAILED", XML_XPTR_EVAL_FAILED, 0);
  add_integer_constant("XML_XPTR_EXTRA_OBJECTS", XML_XPTR_EXTRA_OBJECTS, 0);
  add_integer_constant("XML_C14N_CREATE_CTXT", XML_C14N_CREATE_CTXT, 0);
  add_integer_constant("XML_C14N_REQUIRES_UTF8", XML_C14N_REQUIRES_UTF8, 0);
  add_integer_constant("XML_C14N_CREATE_STACK", XML_C14N_CREATE_STACK, 0);
  add_integer_constant("XML_C14N_INVALID_NODE", XML_C14N_INVALID_NODE, 0);
  add_integer_constant("XML_FTP_PASV_ANSWER", XML_FTP_PASV_ANSWER, 0);
  add_integer_constant("XML_FTP_EPSV_ANSWER", XML_FTP_EPSV_ANSWER, 0);
  add_integer_constant("XML_FTP_ACCNT", XML_FTP_ACCNT, 0);
  add_integer_constant("XML_HTTP_URL_SYNTAX", XML_HTTP_URL_SYNTAX, 0);
  add_integer_constant("XML_HTTP_USE_IP", XML_HTTP_USE_IP, 0);
  add_integer_constant("XML_HTTP_UNKNOWN_HOST", XML_HTTP_UNKNOWN_HOST, 0);

  /* error levels */
  add_integer_constant("XML_ERR_NONE", XML_ERR_NONE, 0);
  add_integer_constant("XML_ERR_WARNING", XML_ERR_WARNING, 0);
  add_integer_constant("XML_ERR_ERROR", XML_ERR_ERROR, 0);
  add_integer_constant("XML_ERR_FATAL", XML_ERR_FATAL, 0);

  /* error domains */
  add_integer_constant("XML_FROM_NONE", XML_FROM_NONE, 0);
  add_integer_constant("XML_FROM_PARSER", XML_FROM_PARSER, 0);
  add_integer_constant("XML_FROM_TREE", XML_FROM_TREE, 0);
  add_integer_constant("XML_FROM_NAMESPACE", XML_FROM_NAMESPACE, 0);
  add_integer_constant("XML_FROM_DTD", XML_FROM_DTD, 0);
  add_integer_constant("XML_FROM_HTML", XML_FROM_HTML, 0);
  add_integer_constant("XML_FROM_MEMORY", XML_FROM_MEMORY, 0);
  add_integer_constant("XML_FROM_OUTPUT", XML_FROM_OUTPUT, 0);
  add_integer_constant("XML_FROM_IO", XML_FROM_IO, 0);
  add_integer_constant("XML_FROM_FTP", XML_FROM_FTP, 0);
  add_integer_constant("XML_FROM_HTTP", XML_FROM_HTTP, 0);
  add_integer_constant("XML_FROM_XINCLUDE", XML_FROM_XINCLUDE, 0);
  add_integer_constant("XML_FROM_XPATH", XML_FROM_XPATH, 0);
  add_integer_constant("XML_FROM_XPOINTER", XML_FROM_XPOINTER, 0);
  add_integer_constant("XML_FROM_REGEXP", XML_FROM_REGEXP, 0);
  add_integer_constant("XML_FROM_DATATYPE", XML_FROM_DATATYPE, 0);
  add_integer_constant("XML_FROM_SCHEMASP", XML_FROM_SCHEMASP, 0);
  add_integer_constant("XML_FROM_SCHEMASV", XML_FROM_SCHEMASV, 0);
  add_integer_constant("XML_FROM_RELAXNGP", XML_FROM_RELAXNGP, 0);
  add_integer_constant("XML_FROM_RELAXNGV", XML_FROM_RELAXNGV, 0);
  add_integer_constant("XML_FROM_CATALOG", XML_FROM_CATALOG, 0);
  add_integer_constant("XML_FROM_C14N", XML_FROM_C14N, 0);
  add_integer_constant("XML_FROM_XSLT", XML_FROM_XSLT, 0);
  add_integer_constant("XML_FROM_VALID", XML_FROM_VALID, 0);
}

void pike_module_exit(void)
{
  _shutdown_xml_sax();
  xmlCleanupParser();
}
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
