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
 * Glue for the bzip2 compression library v1.0.x
 *
 * $Id$
 */
#define _GNU_SOURCE

#include <stdio.h>

#include "global.h"
RCSID("$Id$");

#include "gpgme_config.h"

#ifdef HAVE_LIBGPGME

#ifdef HAVE_GPGME_H
#include <gpgme.h>
#endif

#include "caudium_util.h"

#ifndef GPGME_MIN_VER
#define GPGME_MIN_VER "0.3.4"
#endif

static struct program      *gpgme_program;

typedef struct
{
    unsigned int       ev_ok:1; /* is engine version ok? */
    GpgmeError         lasterr;
    GpgmeCtx           context;
    const char        *gver;
} GPGME_STRUCT;

#define THIS ((GPGME_STRUCT*)get_storage(Pike_fp->current_object, gpgme_program))
#define CHECK_OK if (!THIS->ev_ok) {pop_n_elems(args); push_int(GPGME_Invalid_Engine); return; }


static void f_get_engine_info(INT32 args)
{
    const char  *ret;
    
    CHECK_OK;
    ret = gpgme_get_engine_info();

    pop_n_elems(args);
    push_string(make_shared_string(ret));
}

static void f_strerror(INT32 args)
{
    const char  *ret;
    GpgmeError  err = GPGME_No_Error;
    
    CHECK_OK;
    get_all_args("strerror", args, "%i", &err);
    
    pop_n_elems(args);
    ret = gpgme_strerror(err);

    if (!ret)
        push_string(make_shared_string("Unknown error code"));
    else
        push_string(make_shared_string(ret));
}

static void f_set_protocol(INT32 args)
{
    GpgmeError     err;
    int            proto = GPGME_PROTOCOL_OpenPGP;

    CHECK_OK;
    if (args)
        get_all_args("set_protocol", args, "%i", &proto);

    pop_n_elems(args);
    THIS->lasterr = gpgme_set_protocol(THIS->context, proto);

    err = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
    THIS->ev_ok = err != GPGME_No_Error ? 0 : 1;
    
    push_int(THIS->lasterr);
}

static void f_set_armor(INT32 args)
{
    int       yes = 1;

    CHECK_OK;
    if (args)
        get_all_args("set_armor", args, "%i", &yes);

    gpgme_set_armor(THIS->context, yes);
    pop_n_elems(args);
}

static void f_get_armor(INT32 args)
{
    int       yes;

    CHECK_OK;
    yes = gpgme_get_armor(THIS->context);
    pop_n_elems(args);

    push_int(yes);
}

static void f_set_textmode(INT32 args)
{
    int       yes = 1;

    CHECK_OK;
    if (args)
        get_all_args("set_textmode", args, "%i", &yes);

    gpgme_set_textmode(THIS->context, yes);
    pop_n_elems(args);
}

static void f_get_textmode(INT32 args)
{
    int      yes;

    CHECK_OK;
    yes = gpgme_get_textmode(THIS->context);
    pop_n_elems(args);

    push_int(yes);
}

static void f_set_include_certs(INT32 args)
{
    int   nrcerts;

    CHECK_OK;
    get_all_args("set_include_certs", args, "%i", &nrcerts);

    pop_n_elems(args);
    gpgme_set_include_certs(THIS->context, nrcerts);
}

static void f_get_include_certs(INT32 args)
{
    int   nrcerts;

    CHECK_OK;

    pop_n_elems(args);
    nrcerts = gpgme_get_include_certs(THIS->context);

    push_int(nrcerts);
}

static void f_set_keylist_mode(INT32 args)
{
    int   mode;

    CHECK_OK;
    get_all_args("set_keylist_mode", args, "%i", &mode);
    pop_n_elems(args);

    THIS->lasterr = gpgme_set_keylist_mode(THIS->context, mode);

    push_int(THIS->lasterr);
}

static void f_get_keylist_mode(INT32 args)
{
    int   mode;

    CHECK_OK;
    pop_n_elems(args);

    mode = gpgme_get_keylist_mode(THIS->context);

    push_int(mode);
}

static int push_userid(GpgmeKey key, int i)
{
    const char      *userid;
    const char      *name;
    const char      *email;
    const char      *comment;
    const char      *validity;
    unsigned long    validityu;
    unsigned long    revoked;
    unsigned long    invalid;
    int              nvals = 0;

    userid = gpgme_key_get_string_attr(key, GPGME_ATTR_USERID, NULL, i);
    if (!userid)
        return 0;
    
    name = gpgme_key_get_string_attr(key, GPGME_ATTR_NAME, NULL, i);
    email = gpgme_key_get_string_attr(key, GPGME_ATTR_EMAIL, NULL, i);
    comment = gpgme_key_get_string_attr(key, GPGME_ATTR_COMMENT, NULL, i);
    validity = gpgme_key_get_string_attr(key, GPGME_ATTR_VALIDITY, NULL, i);
    validityu = gpgme_key_get_ulong_attr(key, GPGME_ATTR_VALIDITY, NULL, i);
    revoked = gpgme_key_get_ulong_attr(key, GPGME_ATTR_UID_REVOKED, NULL, i);
    invalid = gpgme_key_get_ulong_attr(key, GPGME_ATTR_UID_INVALID, NULL, i);

    if (userid) {
        push_text("userid");
        push_text(userid);
        nvals++;
    }

    if (name) {
        push_text("name");
        push_text(name);
        nvals++;
    }

    if (email) {
        push_text("email");
        push_text(email);
        nvals++;
    }

    if (comment) {
        push_text("comment");
        push_text(comment);
        nvals++;
    }

    if (validity) {
        push_text("validity");
        push_text(validity);
        nvals++;
    }

    if (validityu) {
        push_text("validitynum");
        push_int(validityu);
        nvals++;
    }

    if (revoked) {
        push_text("revoked");
        push_int(revoked);
        nvals++;
    }

    if (invalid) {
        push_text("invalid");
        push_int(invalid);
        nvals++;
    }

    nvals <<= 1;
    f_aggregate_mapping(nvals);

    return 1;
}

static void push_gpgme_key(GpgmeKey key)
{
    const char     *sval;
    unsigned long   uval;
    int             nvals = 0, i, j;

    sval = gpgme_key_get_string_attr(key,
                                     GPGME_ATTR_KEYID,
                                     NULL, 0);
    if (sval) {
        push_text("keyid");
        push_text(sval);
        nvals++;
    }

    sval = gpgme_key_get_string_attr(key,
                                     GPGME_ATTR_FPR,
                                     NULL, 0);
    if (sval) {
        push_text("fingerprint");
        push_text(sval);
        nvals++;
    }

    sval = gpgme_key_get_string_attr(key,
                                     GPGME_ATTR_ALGO,
                                     NULL, 0);
    if (sval) {
        push_text("algo");
        push_text(sval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_ALGO,
                                    NULL, 0);
    if (uval) {
        push_text("algonum");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_LEN,
                                    NULL, 0);
    if (uval) {
        push_text("len");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_CREATED,
                                    NULL, 0);
    if (uval) {
        push_text("created");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_EXPIRE,
                                    NULL, 0);
    if (uval) {
        push_text("expire");
        push_int(uval);
        nvals++;
    }

    i = 0;
    push_text("userids");
    while(1) {
        if (!push_userid(key, i)) {
            f_aggregate(i);
            nvals++;
            break;
        }
        i++;
    }
    if (!i)
        pop_n_elems(1);

    sval = gpgme_key_get_string_attr(key,
                                     GPGME_ATTR_IS_SECRET,
                                     NULL, 0);
    if (sval) {
        push_text("issecret");
        push_text(sval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_IS_SECRET,
                                    NULL, 0);
    if (uval) {
        push_text("issecretnum");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_KEY_REVOKED,
                                    NULL, 0);
    if (uval) {
        push_text("revoked");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_KEY_INVALID,
                                    NULL, 0);
    if (uval) {
        push_text("invalid");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_KEY_EXPIRED,
                                    NULL, 0);
    if (uval) {
        push_text("expired");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_KEY_DISABLED,
                                    NULL, 0);
    if (uval) {
        push_text("disabled");
        push_int(uval);
        nvals++;
    }

    sval = gpgme_key_get_string_attr(key,
                                     GPGME_ATTR_KEY_CAPS,
                                     NULL, 0);
    if (sval) {
        push_text("caps");
        push_text(sval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_CAN_ENCRYPT,
                                    NULL, 0);
    if (uval) {
        push_text("can_encrypt");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_CAN_SIGN,
                                    NULL, 0);
    if (uval) {
        push_text("can_sign");
        push_int(uval);
        nvals++;
    }

    uval = gpgme_key_get_ulong_attr(key,
                                    GPGME_ATTR_CAN_CERTIFY,
                                    NULL, 0);
    if (uval) {
        push_text("can_certify");
        push_int(uval);
        nvals++;
    }

    nvals <<= 1;
    f_aggregate_mapping(nvals);
}

static void f_list_keys(INT32 args)
{
    char           *pattern = NULL;
    int             so = 0;
    int             asXML = 0;
    struct array   *ret;
    GpgmeKey        key;
    int             nkeys;
    
    switch(args) {
        case 0:
            break;
            
        case 1:
            get_all_args("list_keys", args, "%s", &pattern);
            break;

        case 2:
            get_all_args("list_keys", args, "%s%i", &pattern, &so);
            break;

        case 3:
            get_all_args("list_keys", args, "%s%i%i", &pattern, &so, &asXML);
            break;
            
        default:
            Pike_error("Incorrect number of arguments (%d)", args);
    }

    pop_n_elems(args);

    THIS->lasterr = gpgme_op_keylist_start(THIS->context, pattern, so);
    if (THIS->lasterr != GPGME_No_Error) {
        push_int(THIS->lasterr);
        return;
    }

    nkeys = 0;
    while (1) {
        THIS->lasterr = gpgme_op_keylist_next(THIS->context, &key);
        if (THIS->lasterr != GPGME_No_Error && THIS->lasterr != GPGME_EOF) {
            pop_n_elems(args);
            push_int(THIS->lasterr);
            return;
        }

        if (THIS->lasterr == GPGME_EOF)
            break;
        
        if (asXML) {
            char   *tmp = gpgme_key_get_as_xml(key);

            if (!tmp)
                continue;

            nkeys++;
            push_string(make_shared_string(tmp));
            free(tmp);
        } else {
            push_gpgme_key(key);
            nkeys++;
        }
    }

    THIS->lasterr = gpgme_op_keylist_end(THIS->context);
    
    if (nkeys)
        f_aggregate(nkeys);
    else
        push_int(0);
}

static void f_op_export(INT32 args)
{
    struct array      *reca;
    struct array      *ret;
    struct svalue      sv;
    GpgmeRecipients    recipients;
    GpgmeValidity      val;
    GpgmeData          keydata;
    char              *name, *buf;
    size_t             nread;
    INT32              i;
    
    CHECK_OK;
    get_all_args("export", args, "%a", &reca);

    if (!reca->size) {
        pop_n_elems(args);
        push_int(GPGME_Invalid_Recipients);
        return;
    };

    THIS->lasterr = gpgme_recipients_new(&recipients);
    if (THIS->lasterr != GPGME_No_Error) {
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    for (i = 0; i < reca->size; i++) {
        array_index_no_free(&sv, reca, i);
        switch(sv.type) {
            case T_STRING:
                THIS->lasterr = gpgme_recipients_add_name(recipients,
                                                          sv.u.string->str);
                break;

            case T_MAPPING:
            {
                struct mapping  *m = sv.u.mapping;
                struct svalue   *sn, *sv;

                sn = simple_mapping_string_lookup(m, "name");
                sv = simple_mapping_string_lookup(m, "validity");
                if (!sn || !sv)
                    Pike_error("Index %u of input array is an invalid recipient mapping",
                               i);
                
                if (sn->type != T_STRING || sn->u.string->size_shift > 0)
                    Pike_error("The recipient name in the mapping must be an 8-bit string");
                
                if (sv->type != T_INT)
                    Pike_error("The recipient validity must be an integer");

                THIS->lasterr = gpgme_recipients_add_name_with_validity(recipients,
                                                                        sn->u.string->str,
                                                                        sv->u.integer);
                break;
            }
        }
        
        if (THIS->lasterr == GPGME_Invalid_Value)
            Pike_error("Index %u in the recipients array is has an invalid value", i);
    };

    THIS->lasterr = gpgme_data_new(&keydata);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_recipients_release(recipients);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    THIS->lasterr = gpgme_op_export(THIS->context, recipients, keydata);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_recipients_release(recipients);
        gpgme_data_release(keydata);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    };

    THIS->lasterr = gpgme_data_read(keydata, NULL, 0, &nread);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_recipients_release(recipients);
        gpgme_data_release(keydata);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    buf = (char*)malloc((nread + 1) * sizeof(char));
    if (!buf) {
        gpgme_recipients_release(recipients);
        gpgme_data_release(keydata);
        Pike_error("Out of memory!");
    }

    THIS->lasterr = gpgme_data_read(keydata, buf, nread, &nread);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_recipients_release(recipients);
        gpgme_data_release(keydata);
        free(buf);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    gpgme_recipients_release(recipients);
    gpgme_data_release(keydata);
    
    pop_n_elems(args);
    push_string(make_shared_binary_string(buf, nread));
    free(buf);
}

static void f_op_import(INT32 args)
{
    struct pike_string   *s;
    GpgmeData             keydata;
    
    CHECK_OK;
    get_all_args("import", args, "%S", &s);

    if (!s->len) {
        pop_n_elems(args);
        push_int(GPGME_No_Data);
        return;
    }
    
    THIS->lasterr = gpgme_data_new_from_mem(&keydata, s->str,
                                            s->len, 1);
    if (THIS->lasterr != GPGME_No_Error) {
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    THIS->lasterr = gpgme_op_import(THIS->context, keydata);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_data_release(keydata);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    gpgme_data_release(keydata);
    pop_n_elems(args);
    push_int(GPGME_No_Error);
}

static void f_op_delete(INT32 args)
{
    struct mapping     *m;
    struct svalue      *keyid;
    int                 allow_secret = 0;
    GpgmeKey            key;
    
    CHECK_OK;
    switch(args) {
        case 1:
            get_all_args("delete", args, "%m", &m);
            break;

        case 2:
            get_all_args("delete", args, "%m%i", &allow_secret);
            break;

        default:
            Pike_error("Incorrect number of arguments");
            break;
    }

    if (!m || !m->data->num_keypairs) {
        pop_n_elems(args);
        push_int(GPGME_Invalid_Key);
        return;
    }

    keyid = simple_mapping_string_lookup(m, "keyid");
    if (!keyid || keyid->type != T_STRING) {
        pop_n_elems(args);
        push_int(GPGME_Invalid_Key);
        return;
    }

    THIS->lasterr = gpgme_op_keylist_start(THIS->context, keyid->u.string->str, 0);
    if (THIS->lasterr != GPGME_No_Error) {
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    THIS->lasterr = gpgme_op_keylist_next(THIS->context, &key);
    if (THIS->lasterr != GPGME_No_Error) {
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    // OK, _finally_ - we have the key the user asked for
    THIS->lasterr = gpgme_op_delete(THIS->context, key, allow_secret);
    pop_n_elems(args);
    push_int(THIS->lasterr);
}

static void f_decrypt(INT32 args)
{
    GpgmeData             cipher;
    GpgmeData             plain;
    struct pike_string   *cin;
    char                 *ret;
    size_t                nbytes;
    
    CHECK_OK;
    get_all_args("decrypt", args, "%S", &cin);

    if (!cin || !cin->len) {
        pop_n_elems(args);
        push_int(GPGME_No_Data);
        return;
    }

    THIS->lasterr = gpgme_data_new_from_mem(&cipher, cin->str,
                                            cin->len, 1);
    THIS->lasterr = gpgme_data_new(&plain);

    if (THIS->lasterr != GPGME_No_Error) {
        if (cipher)
            gpgme_data_release(cipher);
        if (plain)
            gpgme_data_release(plain);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    THIS->lasterr = gpgme_op_decrypt(THIS->context, cipher, plain);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_data_release(cipher);
        gpgme_data_release(plain);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    THIS->lasterr = gpgme_data_read(cipher, NULL, 0, &nbytes);
    if (THIS->lasterr != GPGME_No_Error) {
        gpgme_data_release(cipher);
        gpgme_data_release(plain);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    ret = (char*)malloc((nbytes + 1) * sizeof(char));
    if (!ret) {
        gpgme_data_release(cipher);
        gpgme_data_release(plain);
        Pike_error("Out of memory!");
    }

    THIS->lasterr = gpgme_data_read(cipher, ret, nbytes, &nbytes);
    if (THIS->lasterr != GPGME_No_Error) {
        free(ret);
        gpgme_data_release(cipher);
        gpgme_data_release(plain);
        pop_n_elems(args);
        push_int(THIS->lasterr);
        return;
    }

    pop_n_elems(args);
    push_string(make_shared_binary_string(ret, nbytes));
    free(ret);
    gpgme_data_release(cipher);
    gpgme_data_release(plain);
}

static void f_verify(INT32 args)
{
    struct pike_string    *sig;
    struct pike_string    *plain;
    
    CHECK_OK;
}

static void f_create(INT32 args)
{
    GpgmeError    err;
    int           proto = GPGME_PROTOCOL_OpenPGP;
    
    if (args)
        get_all_args("create", args, "%i", proto);
    
    err = gpgme_new(&THIS->context);
    if (err != GPGME_No_Error)
        Pike_error("Cannot create a GPGME context");

    err = gpgme_set_protocol(THIS->context, proto);
    if (err == GPGME_No_Error)
        switch(proto) {
            case GPGME_PROTOCOL_OpenPGP:
            case GPGME_PROTOCOL_CMS:
                THIS->lasterr = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
                THIS->ev_ok = THIS->lasterr != GPGME_No_Error ? 0 : 1;
                break;
        }
    
    pop_n_elems(args);
}

static void init_gpgme(struct object *o)
{
    THIS->gver = gpgme_check_version(GPGME_MIN_VER);

    if (!THIS->gver)
        Pike_error("The GPGME libary is older than v%s",
                   GPGME_MIN_VER);
    
    THIS->context = 0;
}

static void exit_gpgme(struct object *o)
{
    if (THIS->context)
        gpgme_release(THIS->context);
    
    free_program(gpgme_program);
}

void pike_module_init(void)
{
#ifdef PEXTS_VERSION
    pexts_init();
#endif

    start_new_program();
    ADD_STORAGE(GPGME_STRUCT);

    set_init_callback(init_gpgme);
    set_exit_callback(exit_gpgme);

    ADD_FUNCTION("create", f_create,
                 tFunc(tOr(tVoid, tInt), tVoid), 0);
    
    ADD_FUNCTION("get_engine_info", f_get_engine_info,
                 tFunc(tVoid, tOr(tString, tInt)), 0);
    ADD_FUNCTION("strerror", f_strerror,
                 tFunc(tInt, tString), 0);
    ADD_FUNCTION("set_protocol", f_set_protocol,
                 tFunc(tInt, tInt), 0);
    ADD_FUNCTION("set_armor", f_set_armor,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("get_armor", f_get_armor,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("set_textmode", f_set_textmode,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("get_textmode", f_get_textmode,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("set_include_certs", f_set_include_certs,
                 tFunc(tInt, tVoid), 0);
    ADD_FUNCTION("get_include_certs", f_get_include_certs,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("set_keylist_mode", f_set_keylist_mode,
                 tFunc(tInt, tInt), 0);
    ADD_FUNCTION("get_keylist_mode", f_get_keylist_mode,
                 tFunc(tVoid, tInt), 0);
    ADD_FUNCTION("list_keys", f_list_keys,
                 tFunc(tOr(tVoid, tString) tOr(tVoid, tInt) tOr(tVoid, tInt),
                       tOr(tArray,tInt)), 0);
    
    /*TBD: passphrase callback */
    /*TBD: progress meter callback */
    /*TBD: genkey - learn more about the params */

    ADD_FUNCTION("export", f_op_export,
                 tFunc(tArray, tOr(tInt, tString)), 0);
    ADD_FUNCTION("import", f_op_import,
                 tFunc(tString, tInt), 0);
    ADD_FUNCTION("delete", f_op_delete,
                 tFunc(tMapping tOr(tVoid, tInt), tInt), 0);

    /*TBD: trust item management */

    ADD_FUNCTION("decrypt", f_decrypt,
                 tFunc(tString, tOr(tString, tInt)), 0);
    ADD_FUNCTION("verify", f_verify,
                 tFunc(tString tOr(tString, tVoid), tMapping), 0);
    
    gpgme_program = end_program();
    add_program_constant("gpgme", gpgme_program, 0);

    /* Error codes */
    add_integer_constant("GPGME_EOF", GPGME_EOF, 0);
    add_integer_constant("GPGME_No_Error", GPGME_No_Error, 0);
    add_integer_constant("GPGME_General_Error", GPGME_General_Error, 0);
    add_integer_constant("GPGME_Out_Of_Core", GPGME_Out_Of_Core, 0);
    add_integer_constant("GPGME_Invalid_Value", GPGME_Invalid_Value, 0);
    add_integer_constant("GPGME_Busy", GPGME_Busy, 0);
    add_integer_constant("GPGME_No_Request", GPGME_No_Request, 0);
    add_integer_constant("GPGME_Exec_Error", GPGME_Exec_Error, 0);
    add_integer_constant("GPGME_Too_Many_Procs", GPGME_Too_Many_Procs, 0);
    add_integer_constant("GPGME_Pipe_Error", GPGME_Pipe_Error, 0);
    add_integer_constant("GPGME_No_Recipients", GPGME_No_Recipients, 0);
    add_integer_constant("GPGME_Invalid_Recipients", GPGME_Invalid_Recipients, 0);
    add_integer_constant("GPGME_No_Data", GPGME_No_Data, 0);
    add_integer_constant("GPGME_Conflict", GPGME_Conflict, 0);
    add_integer_constant("GPGME_Not_Implemented", GPGME_Not_Implemented, 0);
    add_integer_constant("GPGME_Read_Error", GPGME_Read_Error, 0);
    add_integer_constant("GPGME_Write_Error", GPGME_Write_Error, 0);
    add_integer_constant("GPGME_Invalid_Type", GPGME_Invalid_Type, 0);
    add_integer_constant("GPGME_Invalid_Mode", GPGME_Invalid_Mode, 0);
    add_integer_constant("GPGME_File_Error", GPGME_File_Error, 0);
    add_integer_constant("GPGME_Decryption_Failed", GPGME_Decryption_Failed, 0);
    add_integer_constant("GPGME_No_Passphrase", GPGME_No_Passphrase, 0);
    add_integer_constant("GPGME_Canceled", GPGME_Canceled, 0);
    add_integer_constant("GPGME_Invalid_Key", GPGME_Invalid_Key, 0);
    add_integer_constant("GPGME_Invalid_Engine", GPGME_Invalid_Engine, 0);

    /* Protocol constants */
    add_integer_constant("GPGME_PROTOCOL_OpenPGP", GPGME_PROTOCOL_OpenPGP, 0);
    add_integer_constant("GPGME_PROTOCOL_CMS", GPGME_PROTOCOL_CMS, 0);

    /* Keylist constants */
    add_integer_constant("GPGME_KEYLIST_MODE_LOCAL", GPGME_KEYLIST_MODE_LOCAL, 0);
    add_integer_constant("GPGME_KEYLIST_MODE_EXTERN", GPGME_KEYLIST_MODE_EXTERN, 0);

    /* Validity constants */
    add_integer_constant("GPGME_VALIDITY_UNKNOWN", GPGME_VALIDITY_UNKNOWN, 0);
    add_integer_constant("GPGME_VALIDITY_UNDEFINED", GPGME_VALIDITY_UNDEFINED, 0);
    add_integer_constant("GPGME_VALIDITY_NEVER", GPGME_VALIDITY_NEVER, 0);
    add_integer_constant("GPGME_VALIDITY_MARGINAL", GPGME_VALIDITY_MARGINAL, 0);
    add_integer_constant("GPGME_VALIDITY_FULL", GPGME_VALIDITY_FULL, 0);
    add_integer_constant("GPGME_VALIDITY_ULTIMATE", GPGME_VALIDITY_ULTIMATE, 0);
}

void pike_module_exit(void)
{}
#else /* !HAVE_LIBGPGME */
void pike_module_init(void)
{
#ifdef PEXTS_VERSION
  pexts_init();
#endif
}

void pike_module_exit(void)
{}
#endif /* HAVE_LIBGPGME */
