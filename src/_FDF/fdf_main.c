/*
 * Pike Extension Modules - A collection of modules for the Pike Language
 * Copyright © 2000-2002 The Caudium Group
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
 * Glue for the Adobe FDF SDK
 *
 * $Id$
 */

/*! @module _FDF
 *!   This module implements the glue for the Adobe FDF Toolkit.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "global.h"
RCSID("$Id$");

#include "caudium_util.h"
#include "fdf_config.h"
#include "fdf_global.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_LIBFDFTK
#include <fdftk.h>

/* Glue */
typedef struct
{
    FDFDoc         theFDF;
#ifdef PIKE_THREADS
    PIKE_MUTEX_T   mutex;
#endif
} FDF_STORAGE;

#define THIS ((FDF_STORAGE*)get_storage(fp->current_object, fdf_program))
#define THAT(X) ((FDF_STORAGE*)get_storage((X), (X)->prog))

static struct program   *fdf_program;

static struct pike_string  *s_filename;
static struct pike_string  *s_mimetype;
static struct pike_string  *s_fieldname;
static struct pike_string  *s_ReadOnly;
static struct pike_string  *s_Required;
static struct pike_string  *s_Password;
static struct pike_string  *s_FileSelect;
static struct pike_string  *s_DoNotSpellCheck;
static struct pike_string  *s_DoNotScroll;
static struct pike_string  *s_Editable;
static struct pike_string  *s_OptionsSorted;
static struct pike_string  *s_MultipleSelection;
static struct pike_string  *s_Hidden;
static struct pike_string  *s_Print;
static struct pike_string  *s_NoView;

#ifdef FDF_DEBUG
static void FDEBUG(char *format, ...)
{
    char     buf[4096];
    va_list  args;

    va_start(args, format);
    
#ifdef HAVE_VSNPRINTF
    vsnprintf(buf, sizeof(buf), format, args);
#else
    VSPRINTF(buf, format, args);
#endif

    va_end(args);

    write(1, buf, strlen(buf));
}
#else /* !FDF_DEBUG */
#ifdef __GNUC__
#define FDEBUG(format, args...)
#else
static void FDEBUG(char *format, ...)
{}
#endif
#endif

/*! @class File
 *!  Implements the FDF file abstraction. Contains all functions that
 *!  accept the FDF file argument.
 */

/* General functions */

/*! @decl void Close();
 *!  Closes the previously opened file and frees all resources associated
 *!  with it.
 */
static void f_FDFClose(INT32 args)
{
    if (args != 0)
        WRONG_NUM_OF_ARGS("FDF->Close", args, 0);
    
    if (!THIS->theFDF) {
        FDEBUG("FDF.Close called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcOK);
        return;
    }

    FDFClose(THIS->theFDF);
    THIS->theFDF = NULL;
    
    pop_n_elems(args);
}

/*! @decl int EmbedAndClose(object embed, string|void pass);
 *!  Embed the passed FDF file in the object being accessed. The passed
 *!  file is closed after the operation succeeds. If the @tt{pass@}
 *!  argument is present the embedded document will be protected with
 *!  password. 
 *!
 *! @param embed
 *!   The file to be embedded. It must be of the _FDF.File type!
 *! @param pass
 *!   Optional password for the embedded file (8-bit string)
 *!
 *! @returns
 *!   Returns an FDF error code. Possible values are:
 *!
 *!   @ol
 *!     @item ErcOK
 *!     @item ErcBadParameter
 *!     @item ErcIncompatibleFDF
 *!     @item ErcEmbeddedFDFs
 *!     @item ErcInternalError
 *!   @endol
 */
static void f_FDFEmbedAndClose(INT32 args)
{
    struct object     *embed = NULL;
    char              *pass = NULL;
    FDF_STORAGE       *that;
    int                i;
    struct identifier *id;
    struct svalue      to;
    FDFDoc             emFDF;
    FDFErc             ret;
    
    switch(args) {
        case 2:
            get_all_args("FDF->EmbedAndClose", args, "%o%s", &embed, &pass);
            break;

        case 1:
            get_all_args("FDF->EmbedAndClose", args, "%o", &embed);
            break;

        default:
            WRONG_NUM_OF_ARGS("FDF->EmbedAndClose", args, 2);
    }

    /*
     * Make sure it's an FDF object...
     */
    i = find_identifier("__FDF_MAGIC", embed->prog);
    if (i == -1)
        Pike_error("FDF->EmbedAndClose: trying to embed an object that's not FDF.File!\n");
    id = ID_FROM_INT(embed->prog, i);
    if (!IDENTIFIER_IS_CONSTANT(id->identifier_flags))
        Pike_error("FDF->EmbedAndClose: trying to embed an object that's not FDF.File!\n");
    
    low_object_index_no_free(&to, embed, embed->prog->identifier_index[i-1]);
    
    if (to.type != T_INT || to.u.integer != FDF_MAGIC)
        Pike_error("FDF->EmbedAndClose: trying to embed an object that's not FDF.File!\n");
    

    that = THAT(embed);
    if (!that->theFDF)
        Pike_error("FDF->EmbedAndClose: passed object doesn't have any file open.\n");
    
    pop_n_elems(args);
    
    emFDF = that->theFDF;
    that->theFDF = NULL;

    ret = FDFEmbedAndClose(THIS->theFDF, emFDF, pass);
    if (ret != FDFErcOK)
        that->theFDF = emFDF;

    push_int(ret);
}

/*! @decl string|int GetFDFVersion();
 *!   Returns a string with the version of the FDF. Possible
 *!   return values are @tt{1.2@}, @tt{1.3@} or @tt{1.4@}, which correspond
 *!   to the version of the PDF Reference where the various FDF features
 *!   were first introduced. The return of this function signifies
 *!   the version that describes all the features actually used in this
 *!   file.
 *!
 *! @returns
 *!   @tt{ErcBadParameter@} on error, version string on success.
 */
static void f_FDFGetFDFVersion(INT32 args)
{
    const char   *ver;

    if (args)
        WRONG_NUM_OF_ARGS("FDF->GetFDFVersion", args, 0);

    if (!THIS->theFDF) {
        FDEBUG("FDF->GetFDFVersion called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }
    
    ver = FDFGetFDFVersion(THIS->theFDF);
    pop_n_elems(args);

    push_string(make_shared_string(ver));
}

/*! @decl string GetVersion();
 *!   Gets the current version of the FDF library.
 */
static void f_FDFGetVersion(INT32 args)
{
    const char   *ver;

    if (args)
        WRONG_NUM_OF_ARGS("FDF->GetFDFVersion", args, 0);

    ver = FDFGetVersion();
    pop_n_elems(args);

    push_string(make_shared_string(ver));
}

/*TODO*/
static void f_FDFOpenFromEmbedded(INT32 args)
{}

/*! @decl int RemoveItem(string field, int which);
 *!   Removes a key-pair value from the FDF data.
 *!
 *! @param field
 *!  String representing the fully qualified name of the field (for
 *!  example, employee.name.last) from which the key-value pair is to be
 *!  removed. If the item to be removed is not specific to one particular
 *!  field (FDFStatus, FDFFile, FDFID, FDFTargetFrame, FDFEncoding, FDFJavaScript,
 *!  FDFAppendSaves), pass an empty string.
 *!
 *! @param which
 *!   A value identifying the item to be removed. If the item to be removed
 *!   is not present, no error occurs. The function returns @tt{ErcOK@}.
 *!
 *! @returns
 *!   @ol
 *!     @item ErcOK
 *!     @item ErcBadParameter
 *!     @item ErcBadFDF
 *!     @item ErcIncompatibleFDF
 *!     @item ErcFieldNotFound
 *!     @item ErcInternalError
 *!   @endol
 */
static void f_FDFRemoveItem(INT32 args)
{
    char    *field;
    int      which;
    FDFErc   ret;
    
    get_all_args("FDF->RemoveItem", args, "%s%i", &field, &which);

    if (!THIS->theFDF) {
        FDEBUG("FDF->RemoveItem called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFRemoveItem(THIS->theFDF, field, which);
    pop_n_elems(args);

    push_int(ret);
}

/*! @decl int Save(string path);
 *!   Save the FDF to the file given in @tt{path@}.
 *!
 *! @param path
 *!   Full path to the file data should be saved in.
 *!
 *! @returns
 *!   @ol
 *!     @item ErcOK
 *!     @item ErcBadParameter
 *!     @item ErcFileSysErr
 *!     @item ErcInternalError
 *!   @endol
 */
static void f_FDFSave(INT32 args)
{
    FDFErc   ret;
    char    *path;
    
    get_all_args("FDF->Save", args, "%s", &path);

    if (!THIS->theFDF) {
        FDEBUG("FDF->Save called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFSave(THIS->theFDF, path);
    pop_n_elems(args);

    push_int(ret);
}

/*! @decl int SetOnImportJavaScript(string script, int before);
 *!  Adds a script to the FDF, which Acrobat (or another PDF reader) then
 *!  executes when it opens the FDF. You can set two such scripts: one
 *!  executes just before the data in the FDF is imported and one executes
 *!  afterwards. (Hint: you need to call @tt{FDFSetOnImportJavaScript@}
 *!  twice to set both scripts.)
 *!
 *! @param script
 *!  The script to be assigned to the action.
 *!
 *! @param before
 *!  Set to 0 to have the script execute before the data in the FDF is
 *!  imported, != 0 to indicate execution after import.
 *!
 *! @returns
 *!   @ol
 *!     @item ErcOK
 *!     @item ErcIncompatibleFDF
 *!     @item ErcEmbeddedFDFs
 *!     @item ErcInternalError
 *!   @endol
 */
static void f_FDFSetOnImportJavaScript(INT32 args)
{
    FDFErc   ret;
    char    *script = NULL;
    ASBool   before = 0;
    
    get_all_args("FDF->SetOnImportJavaScript", args, "%s%i", &script, &before);

    if (!THIS->theFDF) {
        FDEBUG("FDF->SetOnImportJavaScript called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFSetOnImportJavaScript(THIS->theFDF, script, before);
    pop_n_elems(args);

    push_int(ret);
}


/* Parsing FDF Functions */

#define BUF_SIZE      1024
#define MAX_BUF_SIZE  8192
static ASBool enumValuesProc(char *fieldName, char *fieldVal, void *data)
{
    struct mapping  *m = (struct mapping*)data;
    struct svalue   key;
    struct svalue   val;
    
    key.type = T_STRING;
    key.u.string = make_shared_string(fieldName);

    val.type = T_STRING;
    val.u.string = make_shared_string(fieldVal);

    mapping_insert(m, &key, &val);

    return true;
}

/*! @decl int|mapping EnumValues(int|void skipEmpty);
 *!   Enumerates the field values in the FDF file. If @tt{skipEmpty@} is
 *!   present and set to a value != 0, then empty fields will be skipped
 *!   from the output.
 *!
 *! @param skipEmpty
 *!   Skip empty fields if set to != 0
 *!
 *! @returns
 *!   A mapping with the field:value pairs or one of the following error
 *!   codes should an error occur:
 *!
 *!   @ol
 *!     @item ErcOK
 *!     @item ErcBadParameter
 *!     @item ErcEnumStopped
 *!     @item ErcBadFDF
 *!     @item ErcIncompatibleFDF
 *!     @item ErcEmbeddedFDFs
 *!     @item ErcInternalError
 *!   @endol
 */
static void f_FDFEnumValues(INT32 args)
{
    FDFErc          ret;
    ASBool          skipEmpty = 0;
    char           *nameBuf;
    char           *valueBuf;
    unsigned        memamt = 0;
    size_t          bufsize = BUF_SIZE * sizeof(char);
    struct mapping *m;
    
    if (args)
        get_all_args("FDF->EnumValues", args, "%i", &skipEmpty);

    if (!THIS->theFDF) {
        FDEBUG("FDF->EnumValues called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

#ifdef HAVE_ALLOCA
    nameBuf = (char*)alloca(bufsize);
    valueBuf = (char*)alloca(bufsize);
#else
    nameBuf = (char*)malloc(bufsize);
    valueBuf = (char*)malloc(bufsize); 
#endif

    if (!nameBuf)
        memamt = bufsize;
    if (!valueBuf)
        memamt += bufsize;

    if (memamt)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->EnumValues", memamt);

    m = allocate_mapping(2);
    if (!m) {
#ifndef HAVE_ALLOCA
        free(nameBuf);
        free(valueBuf);
#endif
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->EnumValues", sizeof(*m));
    }

    while(1) {
        ret = FDFEnumValues(THIS->theFDF,
                            enumValuesProc,
                            nameBuf, bufsize,
                            valueBuf, bufsize,
                            m, skipEmpty);

        if (ret == FDFErcBufTooShort) {
            if (bufsize >= MAX_BUF_SIZE)
                break;
            bufsize <<= 1;
            
#ifdef HAVE_ALLOCA
            nameBuf = (char*)alloca(bufsize);
            valueBuf = (char*)alloca(bufsize);
#else
            nameBuf = (char*)realloc(nameBuf, bufsize);
            valueBuf = (char*)realloc(valueBuf, bufsize);

            memamt = 0;
            if (!nameBuf)
                memamt = bufsize;
            if (!valueBuf)
                memamt += bufsize;

            if (memamt)
                SIMPLE_OUT_OF_MEMORY_ERROR("FDF->EnumValues", memamt);
#endif
        } else
            break;
    }
    
    if (ret != FDFErcOK) {
        free_mapping(m);
    }

#ifndef HAVE_ALLOCA
    free(nameBuf);
    free(valueBuf);
#endif

    pop_n_elems(args);

    if (ret != FDFErcOK)
        push_int(ret);
    else
        push_mapping(m);
}
#undef BUF_SIZE
#undef MAX_BUF_SIZE

/*! @decl int ExtractAppendSaves(string path);
 *!  Extracts the incremental changes to the PDF that were submitted
 *!  inside the FDF, and creates a file out of them at the requested
 *!  location.
 *!
 *! @note
 *!  @i{Extract@} does not imply that the attachment gets removed
 *!  from the FDF.
 *!
 *! @param path
 *!  Path to the file where the extracted data should be saved.
 *!
 *! @returns
 *!   @ol
 *!     @item ErcOK
 *!     @item ErcBadParameter
 *!     @item ErcFileSysErr
 *!     @item ErcBadFDF
 *!     @item ErcIncompatibleFDF
 *!     @item ErcEmbeddedFDFs
 *!     @item ErcNoAppendSaves
 *!     @item ErcInternalError
 *!   @endol
 */
static void f_FDFExtractAppendSaves(INT32 args)
{
    FDFErc     ret;
    char      *path;
    
    get_all_args("FDF->ExtractAppendSaves", args, "%s", &path);

    if (!THIS->theFDF) {
        FDEBUG("FDF->ExtractAppendSaves called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    if (!strlen(path)) {
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFExtractAppendSaves(THIS->theFDF, path);

    pop_n_elems(args);

    push_int(ret);
}

/*! @decl int|mapping ExtractAttachment(string fieldname, string|void path, string|void isFilePath);
 *!  Extracts the file uploaded by means of a "file selection" field, and
 *!  creates a file out of it at the requested location.
 *!
 *! @note
 *!  @i{Extract@} does not imply that the attachment gets removed from the
 *!  FDF.
 *!
 *! @param fieldname
 *!  String representing the fully qualified name of the field.
 *!
 *! @param path
 *!  Host-encoded string for the pathname of the file to create with the
 *!  extracted attachment. If @tt{path@} is absent or an empty string, Acrobat
 *!  saves the file in the current directory, using the filename included with
 *!  the attachment. If @tt{isFilePath@} is !=0 and @tt{path@} is not empty,
 *!  Acrobat appends the filename included with the attachment to @tt{path@}
 *!  and saves the attachment there. For example, if you pass /tmp/  and the
 *!  filename found in the attachment is photo.jpg, Acrobat will save the
 *!  file to /tmp/photo.jpg.
 *!
 *! @param isFilePath
 *!  If !=0  and @tt{path@} is not empty, Acrobat appends the filename
 *!  included with the attachment to @tt{path@} and saves the attachment
 *!  there.
 *!
 *! @returns
 *!  A mapping with information about the attachment or an error number
 *!  should the call fail. The mapping has the following fields:
 *!
 *! @dl
 *!   @item fieldName
 *!     Name of the field which the attachment was copied from.
 *!   @item fileName
 *!     Full path to the file where the attachment was saved.
 *!   @item mimeType
 *!     The MIME type of the attachment.
 *! @enddl
 *!
 *! If the call fails, it returns one of the following errors:
 *!
 *! @ol
 *!   @item ErcOK
 *!   @item ErcBadParameter
 *!   @item ErcFileSysErr
 *!   @item ErcBadFDF
 *!   @item ErcIncompatibleFDF
 *!   @item ErcEmbeddedFDFs
 *!   @item ErcNoValue
 *!   @item ErcInternalError
 *! @endol 
 */
static void f_FDFExtractAttachment(INT32 args)
{
#define BUF_SIZE 1024
    
    FDFErc          ret;
    ASBool          isFilePath = 0;
    char           *path = NULL, *fieldName = NULL, *mimeType = NULL;
    ASInt32         nFileName, nMimeTypeSize;
    struct mapping *m;
    struct svalue   key, val;
    
    switch(args) {
        case 3:
            get_all_args("FDF->ExtractAttachment", args, "%s%s%i", &fieldName, &path,
                         &isFilePath);
            break;
            
        case 2:
            get_all_args("FDF->ExtractAttachment", args, "%s%s", &fieldName, &path);
            break;

        case 1:
            get_all_args("FDF->ExtractAttachment", args, "%s", &fieldName);
            break;

        default:
            WRONG_NUM_OF_ARGS("FDF->ExtractAttachment", args, 3);
    }
    
    if (!THIS->theFDF) {
        FDEBUG("FDF->ExtractAttachment called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    if (path && !strlen(path) && isFilePath)
        Pike_error("FDF->ExtractAttachment: cannot accept empty path.\n");

    if (!path) {
#ifdef HAVE_ALLOCA
        path = (char*)alloca(BUF_SIZE * sizeof(char));
#else
        path = (char*)malloc(BUF_SIZE * sizeof(char));
#endif
        nFileName = BUF_SIZE;
        isFilePath = 0;
    } else
        nFileName = 0;
    
#ifdef HAVE_ALLOCA
    mimeType = (char*)alloca(BUF_SIZE * sizeof(char));
#else
    mimeType = (char*)malloc(BUF_SIZE * sizeof(char));
#endif
    nMimeTypeSize = BUF_SIZE;

    ret = FDFExtractAttachment(THIS->theFDF, fieldName, path,
                               nFileName, isFilePath, mimeType, nMimeTypeSize);

    if (ret != FDFErcOK) {
#ifndef HAVE_ALLOCA
        if (nFileName && path)
            free(path);
        free(mimeType);
#endif
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    m = allocate_mapping(6);
    if (!m) {
#ifndef HAVE_ALLOCA
        if (nFileName && path)
            free(path);
        free(mimeType);
#endif
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->ExtractAttachment", sizeof(*m));
    }

    key.type = T_STRING;
    key.u.string = s_fieldname;
    val.type = T_STRING;
    val.u.string = make_shared_string(fieldName);
    mapping_insert(m, &key, &val);

    key.u.string = s_filename;
    val.u.string = make_shared_string(path);
    mapping_insert(m, &key, &val);

    key.u.string = s_mimetype;
    val.u.string = make_shared_string(mimeType);
    mapping_insert(m, &key, &val);

#ifndef HAVE_ALLOCA
    if (nFileName && path)
        free(path);
    free(mimeType);
#endif

    pop_n_elems(args);

    push_mapping(m);
#undef BUF_SIZE
}


/*! @decl int GetAP(string field, int whichFace, string path);
 *!  Gets the appearance of a field (the value of one of the
 *!  faces of the /AP key) and creates a PDF document out of it.
 *!
 *! @param field
 *!  String representing the fully qualified name of the field.
 *!
 *! @param whichFace
 *!  An value indicating which face of the /AP key is
 *!  requested. Can be one of the following values:
 *!
 *!   @ul
 *!     @item _FDF.NormalAP
 *!     @item _FDF.RolloverAP
 *!     @item _FDF.DownAP
 *!   @endul
 *!
 *! @param path
 *!  Host-encoded string for the pathname of the PDF file to create.
 *!
 *! @returns
 *!   @ul
 *!     @item ErcOK
 *!     @item ErcBadParameter
 *!     @item ErcBadFDF
 *!     @item ErcIncompatibleFDF
 *!     @item ErcFieldNotFound
 *!     @item ErcFileSysErr
 *!     @item ErcNoAP
 *!     @item ErcInternalError
 *!   @endul
 */
static void f_FDFGetAP(INT32 args)
{
    FDFErc       ret;
    char        *path, *field;
    FDFAppFace   whichFace;
    
    get_all_args("FDF->GetAP", args, "%s%i%s", &field, &whichFace, &path);

    if (!THIS->theFDF) {
        FDEBUG("FDF->GetAP called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFGetAP(THIS->theFDF, field, whichFace, path);

    pop_n_elems(args);
    
    push_int(ret);
}


/*! @decl int|string GetEncoding();
 *!  Gets the value of the FDF file's @b{/Encoding@} key as a string. If
 *!  this key exists, then all Values (see @[EnumValues], @[GetValue],
 *!  @[GetNthValue]), as well as Options (see @[GetOpt]) within that FDF
 *!  are encoded using the flavor of Host encoding indicated by that
 *!  key. Possible encodings are: @tt{Shift_JIS@}, @tt{UHC@}, @tt{GBK@},
 *!  @tt{BigFive@}. If the FDF contains no Encoding key, then Values and
 *!  Options are all in either PDFDocEncoding or in Unicode if they cannot
 *!  be represented in PDFDocEncoding (i.e. they contain double-byte
 *!  characters).
 *!
 *! @returns
 *!   Either the encoding string or one of the following errors:
 *!
 *! @ul
 *!   @item ErcOK
 *!   @item ErcBadParameter
 *!   @item ErcIncompatibleFDF
 *!   @item ErcBufTooShort
 *!   @item ErcInternalError
 *! @endul
 */
static void f_FDFGetEncoding(INT32 args)
{
    char      *buf;
    ASInt32    bufSize;
    ASInt32    nBytes;
    FDFErc     ret;
    
    if (args)
        WRONG_NUM_OF_ARGS("FDF->GetEncoding", args, 0);

    if (!THIS->theFDF) {
        FDEBUG("FDF->GetEncoding called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFGetEncoding(THIS->theFDF, NULL, 0, &nBytes);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetEncoding failed in a strange way...\n");
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    bufSize = nBytes + 1;
#ifdef HAVE_ALLOCA
    buf = (char*)alloca(bufSize * sizeof(char));
#else
    buf = (char*)malloc(bufSize * sizeof(char));
#endif
    if (!buf)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetEncoding", bufSize);

    pop_n_elems(args);
    
    ret = FDFGetEncoding(THIS->theFDF, buf, bufSize, NULL);
    
    if (ret == FDFErcOK)
        push_string(make_shared_string(buf));
    else
        push_int(ret);

#ifndef HAVE_ALLOCA
    free(buf);
#endif
}

static void f_FDFGetFile(INT32 args)
{
    char      *buf;
    ASInt32    bufSize;
    ASInt32    nBytes;
    FDFErc     ret;
    
    if (args)
        WRONG_NUM_OF_ARGS("FDF->GetFile", args, 0);

    if (!THIS->theFDF) {
        FDEBUG("FDF->GetFile called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFGetFile(THIS->theFDF, NULL, 0, &nBytes);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetFile failed in a strange way...\n");
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    bufSize = nBytes + 1;
#ifdef HAVE_ALLOCA
    buf = (char*)alloca(bufSize * sizeof(char));
#else
    buf = (char*)malloc(bufSize * sizeof(char));
#endif
    if (!buf)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetFile", bufSize);

    pop_n_elems(args);
    
    ret = FDFGetFile(THIS->theFDF, buf, bufSize, NULL);
    
    if (ret == FDFErcOK)
        push_string(make_shared_string(buf));
    else
        push_int(ret);

#ifndef HAVE_ALLOCA
    free(buf);
#endif
}

static void f_FDFGetFlags(INT32 args)
{
    FDFErc           ret;
    ASUns32          flags;
    FDFItem          whichFlags;
    char            *field;
    int              count;
    
    get_all_args("FDF->GetFlags", args, "%s%i", &field, &whichFlags);
    
    if (!THIS->theFDF) {
        FDEBUG("FDF->GetFlags called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    if (whichFlags != FDFFf && whichFlags != FDFFlags) {
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }
        
    ret = FDFGetFlags(THIS->theFDF, field, whichFlags, &flags);
    if (ret != FDFErcOK) {
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    pop_n_elems(args);

    count = 0;
    if (whichFlags == FDFFf) {
        if (flags & 0x00000001) {
            push_string(s_ReadOnly);
            count++;
        }
        
        if (flags & 0x00000002) {
            push_string(s_Required);
            count++;
        }
        
        if (flags & 0x00002000) {
            push_string(s_Password);
            count++;
        }

        if (flags & 0x00100000) {
            push_string(s_FileSelect);
            count++;
        }

        if (flags & 0x00400000) {
            push_string(s_DoNotSpellCheck);
            count++;
        }
        
        if (flags & 0x00800000) {
            push_string(s_DoNotScroll);
            count++;
        }
        
        if (flags & 0x00040000) {
            push_string(s_Editable);
            count++;
        }
        
        if (flags & 0x00080000) {
            push_string(s_OptionsSorted);
            count++;
        }
        
        if (flags & 0x00200000) {
            push_string(s_MultipleSelection);
            count++;
        }
    } else if (whichFlags == FDFFlags) {
        if (flags & 0x02) {
            push_string(s_Hidden);
            count++;
        }
        
        if (flags & 0x04) {
            push_string(s_Print);
            count++;
        }
        
        if (flags & 0x20) {
            push_string(s_NoView);
            count++;
        }
    }

    if (!count)
        push_int(FDFErcNoFlags);
    else
        f_aggregate_multiset(count);
}

static void f_FDFGetID(INT32 args)
{
    FDFErc    ret;
    ASInt32   nElemNum, bufSize;
    ASUns8   *buf;

    get_all_args("FDF->GetID", args, "%i", &nElemNum);
    
    if (!THIS->theFDF) {
        FDEBUG("FDF->GetID called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    if (nElemNum < 0 || nElemNum > 1) {
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFGetID(THIS->theFDF, nElemNum, NULL, 0, &bufSize);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetID failed in a strange way...\n");
        pop_n_elems(args);
        push_int(ret);
        return;
    }

#ifdef HAVE_ALLOCA
    buf = (char*)alloca(bufSize * sizeof(ASUns8));
#else
    buf = (char*)malloc(bufSize * sizeof(ASUns8));
#endif
    if (!buf)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetID", bufSize);

    pop_n_elems(args);
    
    ret = FDFGetID(THIS->theFDF, nElemNum, buf, bufSize, NULL);
    
    if (ret == FDFErcOK)
        push_string(make_shared_binary_string(buf, bufSize));
    else
        push_int(ret);

#ifndef HAVE_ALLOCA
    free(buf);
#endif    
}

static void f_FDFGetNthValue(INT32 args)
{
    FDFErc    ret;
    ASInt32   idx, nBytes, bufSize;
    char     *field, *buf;

    get_all_args("FDF->GetNthValue", args, "%s%i", &field, &idx);
    
    if (!THIS->theFDF) {
        FDEBUG("FDF->GetNthValue called but file not opened.\n");
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return;
    }

    ret = FDFGetNthValue(THIS->theFDF, field, idx,
                         NULL, 0, &nBytes);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetNthValue failed in a strange way...\n");
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    bufSize = nBytes + 2;
#ifdef HAVE_ALLOCA
    buf = (char*)alloca(bufSize * sizeof(char));
#else
    buf = (char*)malloc(bufSize * sizeof(char));
#endif
    if (!buf)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetID", bufSize);

    pop_n_elems(args);

    ret = FDFGetNthValue(THIS->theFDF, field, idx,
                         buf, bufSize, NULL);

    if (ret == FDFErcOK)
        push_string(make_shared_string(buf));
    else
        push_int(ret);
    
#ifndef HAVE_ALLOCA
    free(buf);
#endif
}

static void f_FDFGetOpt(INT32 args)
{}

/*! @decl void create(string|void path, int|void howMany);
 *!  Constructs a new FDF file object. The method of construction depends
 *!  upon the @tt{path@} parameter presence. If the argument is missing, new
 *!  file will be constructed by creating an empty one in memory. If @tt{path@}
 *!  is present an attempt is made to open the file at the specified
 *!  location. If @tt{path@} is present but is an empty string, the file is
 *!  assumed to be stdin. In that case you must also specify how many
 *!  characters to read from the input.
 *!
 *!  @param path
 *!    Path to a file on disk or an empty string to specify @tt{stdin@} as
 *!    the input.
 *!
 *!  @param howMany
 *!    Number of characters to read from stdin. Ignored if @tt{path@}
 *!    doesn't specify stdin as the input file.
 *!
 *!  @note
 *!    On error the constructor throws an exception with one of the errors
 *!    enumerated below.
 *!
 *!    @ol
 *!      @item ErcOK
 *!      @item ErcFileSysErr
 *!      @item ErcBadFDF
 *!      @item ErcInternalError
 *!    @endol
 */
static void f_create(INT32 args)
{
    char   *path = NULL;
    int     howMany = 0;
    int     fromRealFile = 1;
    FDFErc  ret;
    
    switch(args) {
        case 2:
            get_all_args("FDF->create", args, "%s%i", &path, &howMany);
            if (!strlen(path))
                path = "-";
            break;

        case 1:
            get_all_args("FDF->create", args, "%s", &path);
            if (!strlen(path))
                Pike_error("FDF->create: you must specify the length "
                           "of input when input is 'stdin'\n");
            break;

        case 0:
            fromRealFile = 0;
            break;

        default:
            WRONG_NUM_OF_ARGS("FDF->create", args, 2);
    }

    if (!fromRealFile)
        ret = FDFCreate(&THIS->theFDF);
    else
        ret = FDFOpen(path, howMany, &THIS->theFDF);

    if (ret != FDFErcOK) {
        THIS->theFDF = NULL;
        Pike_error("FDF->create: error creating/opening an FDF file (%d)\n",
                   ret);
    }
    
    pop_n_elems(args);
}
/*! @endclass
 */

/* Pike interface */
static void init_fdf(struct object *o)
{
    THIS->theFDF = NULL;
#ifdef PIKE_THREADS
    mt_init(&THIS->mutex);
#endif
}

static void exit_fdf(struct object *o)
{
#ifdef PIKE_THREADS
    mt_destroy(&THIS->mutex);
#endif
}
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
    pexts_init();
#endif

    /*
     * This must be called only once per app and only on Unix. Since I
     * don't care about Windows, I will call it unconditionally :P
     */
    FDFInitialize();

    s_filename = make_shared_string("FileName");
    s_mimetype = make_shared_string("MimeType");
    s_fieldname = make_shared_string("FieldName");

    /* FDF flags strings */
    s_ReadOnly = make_shared_string("ReadOnly");
    s_Required = make_shared_string("Required");
    s_Password = make_shared_string("Password");
    s_FileSelect = make_shared_string("FileSelect");
    s_DoNotSpellCheck = make_shared_string("DoNotSpellCheck");
    s_DoNotScroll = make_shared_string("DoNotScroll");
    s_Editable = make_shared_string("Editable");
    s_OptionsSorted = make_shared_string("OptionsSorted");
    s_MultipleSelection = make_shared_string("MultipleSelection");
    s_Hidden = make_shared_string("Hidden");
    s_Print = make_shared_string("Print");
    s_NoView = make_shared_string("NoView");
    
    start_new_program();
    ADD_STORAGE(FDF_STORAGE);

    set_init_callback(init_fdf);
    set_exit_callback(exit_fdf);

    /* General functions */
    ADD_FUNCTION("create", f_create,
                 tFunc(tOr(tString, tVoid) tOr(tInt, tVoid), tVoid), 0);
    ADD_FUNCTION("Close", f_FDFClose,
                 tFunc(tVoid, tVoid), 0);
    ADD_FUNCTION("EmbedAndClose", f_FDFEmbedAndClose,
                 tFunc(tObj tOr(tString, tVoid), tInt), 0);
    ADD_FUNCTION("GetFDFVersion", f_FDFGetFDFVersion,
                 tFunc(tVoid, tOr(tInt, tString)), 0);
    ADD_FUNCTION("RemoveItem", f_FDFRemoveItem,
                 tFunc(tString tInt, tInt), 0);
    ADD_FUNCTION("Save", f_FDFSave,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("SetOnImportJavaScript", f_FDFSetOnImportJavaScript,
                 tFunc(tString tInt, tInt), 0);

    /* Parse FDF functions */
    ADD_FUNCTION("EnumValues", f_FDFEnumValues,
                 tFunc(tOr(tInt, tVoid), tOr(tInt, tMapping)), 0);
    ADD_FUNCTION("ExtractAppendSaves", f_FDFExtractAppendSaves,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("ExtractAttachment", f_FDFExtractAttachment,
                 tFunc(tString tOr(tString, tVoid) tOr(tInt, tVoid), tOr(tInt, tMapping)), 0);
    ADD_FUNCTION("GetAP", f_FDFGetAP,
                 tFunc(tString tInt tString, tInt), 0);
    ADD_FUNCTION("GetEncoding", f_FDFGetEncoding,
                 tFunc(tVoid, tOr(tString, tInt)), 0);
    ADD_FUNCTION("GetFile", f_FDFGetFile,
                 tFunc(tVoid, tOr(tString, tInt)), 0);
    ADD_FUNCTION("GetFlags", f_FDFGetFlags,
                 tFunc(tString tInt, tOr(tInt, tMultiset)), 0);
    ADD_FUNCTION("GetID", f_FDFGetFlags,
                 tFunc(tInt, tOr(tString, tInt)), 0);
    ADD_FUNCTION("GetNthValue", f_FDFGetNthValue,
                 tFunc(tString tInt, tOr(tString, tInt)), 0);
    
    add_integer_constant("__FDF_MAGIC", FDF_MAGIC, 0);
    fdf_program = end_program();
    add_program_constant("File", fdf_program, 0);
    
    ADD_FUNCTION("GetVersion", f_FDFGetVersion,
                 tFunc(tVoid, tString), 0);
}

void pike_module_exit(void)
{
    /* Read comment in pike_module_init :P */
    FDFFinalize();    
    free_program(fdf_program);
    free_string(s_filename);
    free_string(s_mimetype);
    free_string(s_fieldname);
    free_string(s_ReadOnly);
    free_string(s_Required);
    free_string(s_Password);
    free_string(s_FileSelect);
    free_string(s_DoNotSpellCheck);
    free_string(s_DoNotScroll);
    free_string(s_Editable);
    free_string(s_OptionsSorted);
    free_string(s_MultipleSelection);
    free_string(s_Hidden);
    free_string(s_Print);
    free_string(s_NoView);
}
#else /* !HAVE_LIBDFDFTK */
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif

/*! @endmodule
 */
