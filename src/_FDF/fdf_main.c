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

/* pdfFileSpec stuff */
#define pfsFS      1
#define pfsF       2
#define pfsMac     3
#define pfsDOS     4
#define pfsUnix    5

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
static struct pike_string  *s_optnelem;
static struct pike_string  *s_optbuf1;
static struct pike_string  *s_optbuf2;

/*
 * Descriptions and names of all FDF constants
 */
#define ERR(NAME, DESC) {NAME, #NAME, DESC, 1}
#define CON(NAME) {NAME, #NAME, 0, 0}
#define COND(NAME, DESC) {NAME, #NAME, DESC, 0}
static constant_desc    constants[] =
{
    /* errors */
    ERR(FDFErcOK, "The function returned successfully"),
    ERR(FDFErcInternalError, "An internal FDF Library error occurred"),
    ERR(FDFErcBadParameter, "One or more of the parameters passed to the function are invalid"),
    ERR(FDFErcFileSysErr, "A file system error occurred, including 'file not found'"),
    ERR(FDFErcBadFDF, "The FDF file being opened or parsed is invalid"),
    ERR(FDFErcFieldNotFound, "The field whose name was passed in the parameter fieldName does not exist in the FDF file"),
    ERR(FDFErcNoValue,"The field whose value was requested has no value"),
    ERR(FDFErcEnumStopped, "Enumeration was stopped by FDFEnumValues by returning FALSE"),
    ERR(FDFErcCantInsertField, "The field whose name was passed in the parameter fieldName cannot be inserted into the FDF file. This might happen if you try to insert  a.b  into an FDF file that already has a field, such as  a.b.c . Conversely, you might try to insert  a.b.c  into a FDF file already containing  a.b "),
    ERR(FDFErcNoOption, "The requested element in a field s /Opt key does not exist, or the field has no /Opt key"),
    ERR(FDFErcNoFlags, "The field has no /F or /Ff keys"),
    ERR(FDFErcBadPDF, "The PDF file passed as the parameter to FDFSetAP iis invalid, or does not contain pageNum"),
    ERR(FDFErcBufTooShort, "The buffer passed as a parameter is too short for the length of the data that the function wants to return"),
    ERR(FDFErcNoAP, "The field has no /AP key"),
    ERR(FDFErcIncompatibleFDF, "An attempt to mix classic and template-based FDF files was made"),
    ERR(FDFErcNoAppendSaves, "The FDF does not include a /Difference key"),
    ERR(FDFErcValueIsArray, "The value of this field is an array Use FDFGetNthValue"),
    ERR(FDFErcEmbeddedFDFs, "The FDF you passed as a parameter is a container for one or more FDFs embedded within it. Use FDFOpenFromEmbedded to gain access to each embedded FDF"),
    ERR(FDFErcNoMoreFDFs, "Returned by FDFOpenFromEmbedded when parameter iWhich >= the number of embedded FDFs (including the case when the passed FDF does not contain any embedded FDFs)"),
    ERR(FDFErcInvalidPassword, "Returned by FDFOpenFromEmbedded when the embedded FDF is encrypted, and you did not provide the correct password"),

    /* FDFItem */
    CON(FDFValue), CON(FDFStatus), CON(FDFFile), CON(FDFID), CON(FDFOpt),
    CON(FDFFf), CON(FDFSetFf), CON(FDFClearFf), CON(FDFFlags), CON(FDFSetF),
    CON(FDFClrF), CON(FDFAP), CON(FDFAS), CON(FDFAction), CON(FDFAA),
    CON(FDFAPRef), CON(FDFIF), CON(FDFTargetFrame), CON(FDFEncoding),
    CON(FDFAppendSaves), CON(FDFJavaScript),

    /* FDFAppFace */
    CON(FDFNormalAP), CON(FDFRolloverAP), CON(FDFDownAP),

    /* FDFScaleWhen */
    CON(FDFAlways), CON(FDFTooSmall), CON(FDFTooBig), CON(FDFNever),

    /* FDFActionTrigger */
    CON(FDFEnter), CON(FDFExit), CON(FDFDown), CON(FDFUp), CON(FDFFormat),
    CON(FDFValidate), CON(FDFKeystroke), CON(FDFCalculate), CON(FDFOnFocus),
    CON(FDFOnBlur),
};
#define NCONSTANTS (sizeof(constants)/sizeof(constant_desc))

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

static INLINE int can_continue(const char *fname, INT32 args)
{
    if (!THIS->theFDF) {
        FDEBUG("FDF->%s called but file not opened.\n", fname);
        pop_n_elems(args);
        push_int(FDFErcBadParameter);
        return 0;
    }

    return 1;
}

static INLINE void trigger_error(const char *fname, int which)
{
    Pike_error("FDF->%s: wrong action trigger value %0x08X\n", fname, which);
}

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
    
    if (!can_continue("Close", args))
        return;

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
    struct program    *fdf_file_prog;
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

    if (!can_continue("EmbedAndClose", args))
        return;
    
    /*
     * Make sure it's an FDF object... or that it inherits from it
     */
    push_constant_text("_FDF.File");
    APPLY_MASTER("resolv", 1);
    fdf_file_prog = program_from_svalue(Pike_sp - 1);
    if (low_get_storage(embed->prog, fdf_file_prog) !=
        low_get_storage(fdf_file_prog, fdf_file_prog))
        Pike_error("FDF->EmbedAndClose: trying to embed an object that's not FDF.File!\n");
    pop_stack();

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

    if (!can_continue("GetFDFVersion", args))
        return;
    
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

    if (!can_continue("RemoveItem", args))
        return;

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

    if (!can_continue("Save", args))
        return;

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

    if (!can_continue("SetOnImportJavaScript", args))
        return;

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

    if (!can_continue("EnumValues", args))
        return;

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

    if (!can_continue("ExtractAppendSaves", args))
        return;

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
    
    if (!can_continue("ExtractAttachment", args))
        return;

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

    if (!can_continue("GetAP", args))
        return;

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

    if (!can_continue("GetEncoding", args))
        return;

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

/*! @decl int|string GetFile();
 *! Gets the value of the FDF file s /F key which points to the PDF form
 *! that this FDF data came from, or is meant for. This must be a PDF
 *! string. Acrobat produces this key when exporting the FDF file.
 *!
 *! @note
 *! An /F key generated by FDFSetFileEx cannot be read back with
 *! FDFGetFile.
 */
static void f_FDFGetFile(INT32 args)
{
    char      *buf;
    ASInt32    bufSize;
    ASInt32    nBytes;
    FDFErc     ret;
    
    if (args)
        WRONG_NUM_OF_ARGS("FDF->GetFile", args, 0);

    if (!can_continue("GetFile", args))
        return;

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
    
    if (!can_continue("GetFlags", args))
        return;

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
    
    if (!can_continue("GetID", args))
        return;

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
    
    if (!can_continue("GetNthValue", args))
        return;

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
{
    FDFErc           ret;
    char            *buf1, *buf2, *field;
    ASInt32          nElemNum, bufSize, nRet;
    struct mapping  *m;
    struct svalue    key, val;
    
    get_all_args("FDF->GetOpt", args, "%s%i", &field, &nElemNum);
    
    if (!can_continue("GetOpt", args))
        return;

    ret = FDFGetOpt(THIS->theFDF, field, -1, NULL, NULL, 0, &nRet);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetOpt failed in a strange way...\n");
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    m = allocate_mapping(2);
    if (!m)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetOpt", sizeof(*m));

    key.type = T_STRING;
    key.u.string = s_optnelem;
    val.type = T_INT;
    val.u.integer = nElemNum;
    mapping_insert(m, &key, &val);
    
    if (nElemNum == -1) {
        pop_n_elems(args);
        push_mapping(m);
        return;
    }
    
    /* Get the buffers size */
    ret = FDFGetOpt(THIS->theFDF, field, 0, NULL, NULL, 0, &nRet);
    if (ret != FDFErcOK) {
        free_mapping(m);
        pop_n_elems(args);
        push_int(FDFErcInternalError);
        return;
    }

    bufSize = nRet + 2;
#ifdef HAVE_ALLOCA
    buf1 = (char*)alloca(bufSize * sizeof(char));
    buf2 = (char*)alloca(bufSize * sizeof(char));
#else
    buf1 = (char*)malloc(bufSize * sizeof(char));
    buf2 = (char*)malloc(bufSize * sizeof(char));
#endif

    if (!buf1 || !buf2) {
        unsigned  mem = 0;
        
        if (buf1) {
#ifndef HAVE_ALLOCA
            free(buf1);
#endif
            mem = bufSize * sizeof(char);
        }
        
        if (buf2) {
#ifndef HAVE_ALLOCA
            free(buf2);
#endif
            mem += bufSize * sizeof(char);
        }
        
        free_mapping(m);
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetOpt", mem);
    }

    /* finally get what we want */
    ret = FDFGetOpt(THIS->theFDF, field, nElemNum,
                    buf1, buf2, bufSize, &nRet);
    if (ret != FDFErcOK) {
#ifndef HAVE_ALLOCA
        free(buf1);
        free(buf2);
#endif
        free_mapping(m);
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    key.u.string = s_optbuf1;
    val.type = T_STRING;
    val.u.string = make_shared_string(buf1);
    mapping_insert(m, &key, &val);
    
    key.u.string = s_optbuf2;
    val.u.string = make_shared_string(buf2);
    mapping_insert(m, &key, &val);

    pop_n_elems(args);
    push_mapping(m);

#ifndef HAVE_ALLOCA
    free(buf1);
    free(buf2);
#endif
}

static void f_FDFGetStatus(INT32 args)
{
    char      *buf;
    ASInt32    bufSize;
    ASInt32    nBytes;
    FDFErc     ret;
    
    if (args)
        WRONG_NUM_OF_ARGS("FDF->GetStatus", args, 0);

    if (!can_continue("GetStatus", args))
        return;

    ret = FDFGetStatus(THIS->theFDF, NULL, 0, &nBytes);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetStatus failed in a strange way...\n");
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
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetOpt", bufSize);

    ret = FDFGetStatus(THIS->theFDF, buf, bufSize, NULL);
    pop_n_elems(args);
    
    if (ret != FDFErcOK) {
#ifndef HAVE_ALLOCA
        free(buf);
#endif
        push_int(ret);
    }

    push_text(buf);

#ifndef HAVE_ALLOCA
    free(buf);
#endif
}

static void f_FDFGetValue(INT32 args)
{
    FDFErc           ret;
    char            *buf, *field;
    ASInt32          nBytes, bufSize;
    
    get_all_args("FDF->GetValue", args, "%s", &field);
    
    if (!can_continue("GetValue", args))
        return;

    ret = FDFGetValue(THIS->theFDF, field, NULL, 0, &nBytes);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->GetValue failed in a strange way...\n");
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
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->GetValue", bufSize);

    ret = FDFGetValue(THIS->theFDF, field, buf, bufSize, NULL);
    pop_n_elems(args);
    
    if (ret != FDFErcOK) {
#ifndef HAVE_ALLOCA
        free(buf);
#endif
        push_int(ret);
    }

    push_text(buf);

#ifndef HAVE_ALLOCA
    free(buf);
#endif
}

static void f_FDFNextFieldName(INT32 args)
{
    FDFErc           ret;
    char            *field, *nextField;
    ASInt32          nBytes, bufSize;
    
    get_all_args("FDF->NextFieldName", args, "%s", &field);
    
    if (!can_continue("NextFieldName", args))
        return;

    ret = FDFNextFieldName(THIS->theFDF, field, NULL, 0, &nBytes);
    if (ret != FDFErcOK) {
        FDEBUG("FDF->NextFieldName failed in a strange way...\n");
        pop_n_elems(args);
        push_int(ret);
        return;
    }

    bufSize = nBytes + 1;
#ifdef HAVE_ALLOCA
    nextField = (char*)alloca(bufSize * sizeof(char));
#else
    nextField = (char*)malloc(bufSize * sizeof(char));
#endif

    if (!nextField)
        SIMPLE_OUT_OF_MEMORY_ERROR("FDF->NextFieldName", bufSize);

    ret = FDFNextFieldName(THIS->theFDF, field, nextField, bufSize, NULL);
    pop_n_elems(args);
    
    if (ret != FDFErcOK) {
#ifndef HAVE_ALLOCA
        free(nextField);
#endif
        push_int(ret);
    }

    push_text(nextField);

#ifndef HAVE_ALLOCA
    free(buf);
#endif
}


/* Generating FDF functions */

static void f_FDFAddDocJavaScript(INT32 args)
{
    FDFErc           ret;
    char            *script, *scriptName;
    
    get_all_args("FDF->AddDocJavaScript", args, "%s%s", &script, &scriptName);
    
    if (!can_continue("AddDocJavaScript", args))
        return;

    ret = FDFAddDocJavaScript(THIS->theFDF, scriptName, script);
    pop_n_elems(args);    
    push_int(ret);
}

/* TO BE DONE */
static void f_FDFAddTemplate(INT32 args)
{}

/* TO BE DONE */
static void f_FDFSetAP(INT32 args)
{}

/* TO BE DONE */
static void f_FDFSetAPRef(INT32 args)
{}

static void f_FDFSetEncoding(INT32 args)
{
    FDFErc           ret;
    char            *enc;
    
    get_all_args("FDF->SetEncoding", args, "%s", &enc);
    
    if (!can_continue("SetEncoding", args))
        return;

    ret = FDFSetEncoding(THIS->theFDF, enc);
    pop_n_elems(args);    
    push_int(ret);
}

static void f_FDFSetFDFVersion(INT32 args)
{
    FDFErc           ret;
    char            *ver;
    
    get_all_args("FDF->SetFDFVersion", args, "%s", &ver);
    
    if (!can_continue("SetFDFVersion", args))
        return;

    ret = FDFSetFDFVersion(THIS->theFDF, ver);
    pop_n_elems(args);    
    push_int(ret);
}

static void f_FDFSetFile(INT32 args)
{
    FDFErc           ret;
    char            *file;
    
    get_all_args("FDF->SetFile", args, "%s", &file);
    
    if (!can_continue("SetFile", args))
        return;

    ret = FDFSetFile(THIS->theFDF, file);
    pop_n_elems(args);    
    push_int(ret);
}

static void f_FDFSetFileEx(INT32 args)
{
    FDFErc           ret;
    char            *file, **tmp;
    int              which;
    pdfFileSpecRec   fileSpec;
    
    get_all_args("FDF->SetFileEx", args, "%s%i", &file, &which);
    
    if (!can_continue("SetFileEx", args))
        return;

    memset(&fileSpec, 0, sizeof(fileSpec));
    
    switch(which) {
        case pfsFS:
            tmp = &fileSpec.FS;
            break;

        case pfsF:
            tmp = &fileSpec.F;
            break;

        case pfsMac:
            tmp = &fileSpec.Mac;
            break;

        case pfsDOS:
            tmp = &fileSpec.DOS;
            break;

        case pfsUnix:
            tmp = &fileSpec.Unix;
            break;

        default:
            pop_n_elems(args);
            Pike_error("FDF->SetFileEx: unknown namespace\n");
    }

    *tmp = file;
    ret = FDFSetFileEx(THIS->theFDF, &fileSpec);
    pop_n_elems(args);    
    push_int(ret);
}

static void f_FDFSetFlags(INT32 args)
{
    FDFErc       ret;
    char        *field;
    ASUns32      flags;
    FDFItem      which;
    
    get_all_args("FDF->SetFlags", args, "%s%i%i", &field, &which, &flags);
    
    if (!can_continue("SetFlags", args))
        return;

    switch(which) {
        case FDFFf:
        case FDFSetFf:
        case FDFClearFf:
        case FDFFlags:
        case FDFSetF:
        case FDFClrF:
            break;

        default:
            Pike_error("FDF->SetFlags: wrong flags value %0x08X\n", which);
    }

    ret = FDFSetFlags(THIS->theFDF, field, which, flags);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetGoToAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *dest;
    FDFActionTrigger  which;
    ASBool            notUsed = 0;

    switch(args) {
        case 3:
            get_all_args("FDF->SetGoToAction", args, "%s%i%s", &field, &which, &dest);
            break;

        case 4:
            get_all_args("FDF->SetGoToAction", args, "%s%i%s%i", &field, &which, &dest, &notUsed);
            break;
            
        default:
            WRONG_NUM_OF_ARGS("FDF->SetGoToAction", args, 4);
    }
    
    if (!can_continue("SetGoToAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetGoToAction", which);
    }
    
    ret = FDFSetGoToAction(THIS->theFDF, field, which, dest, notUsed);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetGoToRAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *dest, *theFile;
    FDFActionTrigger  which;
    ASBool            addNewWindow = 0, newWindow = 1, notUsed = 0;

    switch(args) {
        case 3:
            get_all_args("FDF->SetGoToRAction", args, "%s%i%s", &field, &which, &dest);
            break;

        case 4:
            get_all_args("FDF->SetGoToRAction", args, "%s%i%s%i", &field, &which, &dest,
                         &notUsed);
            break;

        case 6:
            get_all_args("FDF->SetGoToRAction", args, "%s%i%s%i%s%i", &field, &which, &dest,
                         &notUsed, &theFile, &addNewWindow);
            break;
            
        case 7:
            get_all_args("FDF->SetGoToRAction", args, "%s%i%s%i%s%i%i", &field, &which, &dest,
                         &notUsed, &theFile, &addNewWindow, &newWindow);
            break;
            
        default:
            WRONG_NUM_OF_ARGS("FDF->SetGoToRAction", args, 7);
    }
    
    if (!can_continue("SetGoToRAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetGoToRAction", which);
    }
    
    ret = FDFSetGoToRAction(THIS->theFDF, field, which, dest, notUsed,
                            theFile, addNewWindow, newWindow);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetHideAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *dest;
    FDFActionTrigger  which;
    ASBool            isHide = 1;

    switch(args) {
        case 3:
            get_all_args("FDF->SetHideAction", args, "%s%i%s", &field, &which, &dest);
            break;

        case 4:
            get_all_args("FDF->SetHideAction", args, "%s%i%s%i", &field, &which, &dest, &isHide);
            break;
            
        default:
            WRONG_NUM_OF_ARGS("FDF->SetHideAction", args, 4);
    }
    
    if (!can_continue("SetHideAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetHideAction", which);
    }
    
    ret = FDFSetHideAction(THIS->theFDF, field, which, dest, isHide);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetIF(INT32 args)
{
    FDFErc            ret;
    char             *field;
    FDFScaleWhen      scaleWhen;
    ASBool            proportional = 0;
    float             x, y;

    get_all_args("FDF->SetIF", args, "%s%i%i%f%f", &field, &scaleWhen,
                 &proportional, &x, &y);
    
    if (!can_continue("SetIF", args))
        return;

    switch(scaleWhen) {
        case FDFAlways:
        case FDFTooSmall:
        case FDFTooBig:
        case FDFNever:
            break;

        default:
            Pike_error("FDF->SetIF: wrong value for scaleWhen 0x%08X\n", scaleWhen);
    }
    
    ret = FDFSetIF(THIS->theFDF, field, scaleWhen, proportional, x, y);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetImportDataAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *dest;
    FDFActionTrigger  which;

    get_all_args("FDF->SetImportDataAction", args, "%s%i%s", &field, &which, &dest);
    
    if (!can_continue("SetImportDataAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetImportDataAction", which);
    }
    
    ret = FDFSetImportDataAction(THIS->theFDF, field, which, dest);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetJavaScriptAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *script;
    FDFActionTrigger  which;

    get_all_args("FDF->SetJavaScriptAction", args, "%s%i%s", &field, &which, &script);
    
    if (!can_continue("SetJavaScriptAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFFormat:
        case FDFValidate:
        case FDFKeystroke:
        case FDFCalculate:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetJavaScriptAction", which);
    }
    
    ret = FDFSetJavaScriptAction(THIS->theFDF, field, which, script);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetNamedAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *named;
    FDFActionTrigger  which;

    get_all_args("FDF->SetNamedAction", args, "%s%i%s", &field, &which, &named);
    
    if (!can_continue("SetNamedAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetNamedAction", which);
    }
    
    ret = FDFSetNamedAction(THIS->theFDF, field, which, named);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetOpt(INT32 args)
{
    FDFErc            ret;
    char             *field, *str1, *str2 = NULL;
    ASInt32           nElemNum;
    
    switch(args) {
        case 3:
            get_all_args("FDF->SetOpt", args, "%s%i%s", &field, &nElemNum, &str1);
            break;

        case 4:
            get_all_args("FDF->SetOpt", args, "%s%i%s%s", &field, &nElemNum, &str1, &str2);
            break;
            
        default:
            WRONG_NUM_OF_ARGS("FDF->SetOpt", args, 4);
    }
    
    if (!can_continue("SetOpt", args))
        return;

    ret = FDFSetOpt(THIS->theFDF, field, nElemNum, str1, str2);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetResetByNameAction(INT32 args)
{
    FDFErc            ret;
    char             *field, **fields = 0;
    struct array     *arr;
    ASUns32           flags;
    ASInt32           nfields = 0;
    FDFActionTrigger  which;    

    get_all_args("FDF->SetResetByNameAction", args, "%s%i%i%a", &field, &which, &flags, &arr);

    if (!can_continue("SetResetByNameAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetResetByNameAction", which);
    }
    
    if (arr->size) {
        int   i;

#ifdef HAVE_ALLOCA
        fields = (char**)alloca(arr->size * sizeof(char*));
#else
        fields = (char**)malloc(arr->size * sizeof(char*));
#endif
        if (!fields)
            SIMPLE_OUT_OF_MEMORY_ERROR("FDF->SetResetByNameAction", arr->size * sizeof(char*));
        nfields = 0;
        
        for(i = 0; i < arr->size; i++)
            if (arr->item[i].type == T_STRING && arr->item[i].u.string->size_shift == 0) {
                fields[nfields++] = arr->item[i].u.string->str;
            }
    }

    flags = flags ? 1 : 0;
    ret = FDFSetResetByNameAction(THIS->theFDF, field, which, flags,
                                  nfields, (const char**)fields);
    pop_n_elems(args);
    push_int(ret);

#ifndef HAVE_ALLOCA
    if (fields)
        free(fields);
#endif
}

static void f_FDFSetResetFormAction(INT32 args)
{
    FDFErc            ret;
    char             *field;
    FDFActionTrigger  which;    

    get_all_args("FDF->SetResetFormAction", args, "%s%i", &field, &which);

    if (!can_continue("SetResetFormAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetResetFormAction", which);
    }

    ret = FDFSetResetFormAction(THIS->theFDF, field, which);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetStatus(INT32 args)
{
    FDFErc            ret;
    char             *status;

    get_all_args("FDF->SetStatus", args, "%s", &status);

    if (!can_continue("SetStatus", args))
        return;

    ret = FDFSetStatus(THIS->theFDF, status);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetSubmitByNameAction(INT32 args)
{
    FDFErc            ret;
    char             *field, **fields = 0, *url;
    struct array     *arr;
    ASUns32           flags;
    ASInt32           nfields = 0;
    FDFActionTrigger  which;    

    get_all_args("FDF->SetSubmitByNameAction", args, "%s%i%s%i%a", &field, &which,
                 &url, &flags, &arr);

    if (!can_continue("SetSubmitByNameAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetSubmitByNameAction", which);
    }
    
    if (arr->size) {
        int   i;

#ifdef HAVE_ALLOCA
        fields = (char**)alloca(arr->size * sizeof(char*));
#else
        fields = (char**)malloc(arr->size * sizeof(char*));
#endif
        if (!fields)
            SIMPLE_OUT_OF_MEMORY_ERROR("FDF->SetSubmitByNameAction", arr->size * sizeof(char*));
        nfields = 0;
        
        for(i = 0; i < arr->size; i++)
            if (arr->item[i].type == T_STRING && arr->item[i].u.string->size_shift == 0) {
                fields[nfields++] = arr->item[i].u.string->str;
            }
    }

    flags = flags ? 1 : 0;
    ret = FDFSetSubmitByNameAction(THIS->theFDF, field, which, url, flags,
                                   nfields, (const char**)fields);
    pop_n_elems(args);
    push_int(ret);

#ifndef HAVE_ALLOCA
    if (fields)
        free(fields);
#endif
}

static void f_FDFSetSubmitFormAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *url;
    ASUns32           flags;
    FDFActionTrigger  which;    

    get_all_args("FDF->SetSubmitFormAction", args, "%s%i%s%i", &field, &which,
                 &url, &flags);

    if (!can_continue("SetSubmitFormAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetSubmitFormAction", which);
    }

    ret = FDFSetSubmitFormAction(THIS->theFDF, field, which, url, flags);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetTargetFrame(INT32 args)
{
    FDFErc            ret;
    char             *target;

    get_all_args("FDF->SetTargetFrame", args, "%s", &target);

    if (!can_continue("SetTargetFrame", args))
        return;

    ret = FDFSetTargetFrame(THIS->theFDF, target);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetURIAction(INT32 args)
{
    FDFErc            ret;
    char             *field, *uri;
    FDFActionTrigger  which;    
    ASBool            isMap;
    
    get_all_args("FDF->SetURIAction", args, "%s%i%s%i", &field, &which, &uri, &isMap);

    if (!can_continue("SetURIAction", args))
        return;

    switch(which) {
        case FDFEnter:
        case FDFExit:
        case FDFDown:
        case FDFUp:
        case FDFOnFocus:
        case FDFOnBlur:
            break;

        default:
            trigger_error("SetURIAction", which);
    }

    ret = FDFSetURIAction(THIS->theFDF, field, which, uri, isMap);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetValue(INT32 args)
{
    FDFErc            ret;
    char             *field, *val;
    ASBool            notUsed;
    
    get_all_args("FDF->SetValue", args, "%s%s%i", &field, &val, &notUsed);

    if (!can_continue("SetValue", args))
        return;

    ret = FDFSetValue(THIS->theFDF, field, val, notUsed);
    pop_n_elems(args);
    push_int(ret);
}

static void f_FDFSetValues(INT32 args)
{
    FDFErc            ret;
    char             *field, **values = 0;
    struct array     *arr;
    ASInt32           nValues = 0;
    
    get_all_args("FDF->SetValues", args, "%s%a", &field, &arr);

    if (!can_continue("SetValues", args))
        return;

    if (arr->size) {
        int   i;

#ifdef HAVE_ALLOCA
        values = (char**)alloca(arr->size * sizeof(char*));
#else
        values = (char**)malloc(arr->size * sizeof(char*));
#endif
        if (!values)
            SIMPLE_OUT_OF_MEMORY_ERROR("FDF->SetValues", arr->size * sizeof(char*));
        nValues = 0;
        
        for(i = 0; i < arr->size; i++)
            if (arr->item[i].type == T_STRING && arr->item[i].u.string->size_shift == 0) {
                values[nValues++] = arr->item[i].u.string->str;
            }
    }
    
    ret = FDFSetValues(THIS->theFDF, field, nValues, (const char**)values);
    pop_n_elems(args);
    push_int(ret);

#ifndef HAVE_ALLOCA
    if (values)
        free(values);
#endif
}

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

static void f_strerror(INT32 args)
{
    int       errcode;
    unsigned  i;
    
    get_all_args("FDF->strerror", args, "%i", &errcode);
    pop_n_elems(args);

    for(i = 0; i < NCONSTANTS; i++)
        if (constants[i].isError && constants[i].val == errcode) {
            if (constants[i].desc)
                push_string(make_shared_string(constants[i].desc));
            else
                push_text("Error with no description");
            return;
        }
    
    push_text("Unknown error code");
}

static void f_errname(INT32 args)
{
    int       errcode;
    unsigned  i;
    
    get_all_args("FDF->errname", args, "%i", &errcode);
    pop_n_elems(args);

    for(i = 0; i < NCONSTANTS; i++)
        if (constants[i].isError && constants[i].val == errcode) {
            push_string(make_shared_string(constants[i].name));
            return;
        }
    
    push_int(0);
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
    unsigned i;
    
#ifdef PEXTS_VERSION
    pexts_init();
#endif

    /*
     * This must be called only once per app and only on Unix. Since I
     * don't care about Windows, I will call it unconditionally :P
     */
    FDFInitialize();

    MAKE_CONSTANT_SHARED_STRING(s_filename, "FileName");
    MAKE_CONSTANT_SHARED_STRING(s_mimetype, "MimeType");
    MAKE_CONSTANT_SHARED_STRING(s_fieldname, "FieldName");

    /* FDF flags strings */
    MAKE_CONSTANT_SHARED_STRING(s_ReadOnly, "ReadOnly");
    MAKE_CONSTANT_SHARED_STRING(s_Required, "Required");
    MAKE_CONSTANT_SHARED_STRING(s_Password, "Password");
    MAKE_CONSTANT_SHARED_STRING(s_FileSelect, "FileSelect");
    MAKE_CONSTANT_SHARED_STRING(s_DoNotSpellCheck, "DoNotSpellCheck");
    MAKE_CONSTANT_SHARED_STRING(s_DoNotScroll, "DoNotScroll");
    MAKE_CONSTANT_SHARED_STRING(s_Editable, "Editable");
    MAKE_CONSTANT_SHARED_STRING(s_OptionsSorted, "OptionsSorted");
    MAKE_CONSTANT_SHARED_STRING(s_MultipleSelection, "MultipleSelection");
    MAKE_CONSTANT_SHARED_STRING(s_Hidden, "Hidden");
    MAKE_CONSTANT_SHARED_STRING(s_Print, "Print");
    MAKE_CONSTANT_SHARED_STRING(s_NoView, "NoView");

    /* Field options mapping */
    MAKE_CONSTANT_SHARED_STRING(s_optnelem, "size");
    MAKE_CONSTANT_SHARED_STRING(s_optbuf1, "value1");
    MAKE_CONSTANT_SHARED_STRING(s_optbuf2, "value2");
    
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
    ADD_FUNCTION("GetOpt", f_FDFGetOpt,
                 tFunc(tString tInt, tOr(tMapping, tInt)), 0);
    ADD_FUNCTION("GetStatus", f_FDFGetStatus,
                 tFunc(tVoid, tOr(tInt, tString)), 0);
    ADD_FUNCTION("GetValue", f_FDFGetValue,
                 tFunc(tString, tOr(tInt, tString)), 0);
    ADD_FUNCTION("NextFieldName", f_FDFNextFieldName,
                 tFunc(tString, tOr(tInt, tString)), 0);

    /* Generating FDF functions */
    ADD_FUNCTION("AddDocJavaScript", f_FDFAddDocJavaScript,
                 tFunc(tString tString, tInt), 0);
    ADD_FUNCTION("SetEncoding", f_FDFSetEncoding,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("SetFDFVersion", f_FDFSetFDFVersion,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("SetFile", f_FDFSetFile,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("SetFileEx", f_FDFSetFile,
                 tFunc(tString tInt, tInt), 0);
    ADD_FUNCTION("SetFlags", f_FDFSetFlags,
                 tFunc(tString tInt tInt, tInt), 0);
    ADD_FUNCTION("SetGoToAction", f_FDFSetGoToAction,
                 tFunc(tString tInt tString tOr(tInt, tVoid), tInt), 0);
    ADD_FUNCTION("SetGoToRAction", f_FDFSetGoToAction,
                 tFunc(tString tInt tString tOr(tInt, tVoid)
                       tOr(tString, tVoid)
                       tOr(tInt, tVoid)
                       tOr(tInt, tVoid), tInt), 0);
    ADD_FUNCTION("SetHideAction", f_FDFSetHideAction,
                 tFunc(tString tInt tString tOr(tInt, tVoid), tInt), 0);
    ADD_FUNCTION("SetIF", f_FDFSetIF,
                 tFunc(tString tInt tInt tFloat tFloat, tInt), 0);
    ADD_FUNCTION("SetImportDataAction", f_FDFSetImportDataAction,
                 tFunc(tString tInt tString, tInt), 0);
    ADD_FUNCTION("SetJavaScriptAction", f_FDFSetJavaScriptAction,
                 tFunc(tString tInt tString, tInt), 0);
    ADD_FUNCTION("SetNamedAction", f_FDFSetNamedAction,
                 tFunc(tString tInt tString, tInt), 0);
    ADD_FUNCTION("SetOpt", f_FDFSetOpt,
                 tFunc(tString tInt tString tOr(tString, tVoid), tInt), 0);
    ADD_FUNCTION("SetResetByNameAction", f_FDFSetResetByNameAction,
                 tFunc(tString tInt tInt tArray, tInt), 0);
    ADD_FUNCTION("SetResetFormAction", f_FDFSetResetFormAction,
                 tFunc(tString tInt, tInt), 0);
    ADD_FUNCTION("SetStatus", f_FDFSetStatus,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("SetSubmitByNameAction", f_FDFSetSubmitByNameAction,
                 tFunc(tString tInt tString tInt tArray, tInt), 0);
    ADD_FUNCTION("SetSubmitFormAction", f_FDFSetResetFormAction,
                 tFunc(tString tInt tString tInt, tInt), 0);
    ADD_FUNCTION("SetTargetFrame", f_FDFSetTargetFrame,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("SetURIAction", f_FDFSetURIAction,
                 tFunc(tString tInt tString tInt, tInt), 0);
    ADD_FUNCTION("SetValue", f_FDFSetValue,
                 tFunc(tString tString tInt, tInt), 0);
    ADD_FUNCTION("SetValues", f_FDFSetValues,
                 tFunc(tString tArray, tInt), 0);
    
    fdf_program = end_program();
    add_program_constant("File", fdf_program, 0);
    
    ADD_FUNCTION("GetVersion", f_FDFGetVersion,
                 tFunc(tVoid, tString), 0);
    ADD_FUNCTION("strerror", f_strerror,
                 tFunc(tInt, tString), 0);
    ADD_FUNCTION("errname", f_errname,
                 tFunc(tInt, tString), 0);
    
    /* Constants */
    for (i = 0; i < NCONSTANTS; i++)
        add_integer_constant(constants[i].name, constants[i].val, 0);
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
    free_string(s_optnelem);
    free_string(s_optbuf1);
    free_string(s_optbuf2);
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
