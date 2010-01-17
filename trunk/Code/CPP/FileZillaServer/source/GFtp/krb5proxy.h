// **************************************************************************************
// File:             krb5proxy.h
// By:               Ian M Garcia
// Created:          10/17/00
// Copyright:        @2000 Massachusetts Institute of Technology - All rights reserved.
// Description:      H file for krb5proxy.cpp. Proxy function calls for krb5
//                   library
//
// History:
//
// MM/DD/YY   Inits   Description of Change
// **************************************************************************************

#ifndef _KRB5PROXY_H_
#define _KRB5PROXY_H_

#include "stdafx.h"

#ifdef krb5_cc_get_principal
#else /* !krb5_cc_principal */

/* extracted from krb5-1-2 krb5.h */

typedef struct _krb5_ccache {
    krb5_magic magic;
    struct _krb5_cc_ops FAR *ops;
    krb5_pointer data;
} FAR *krb5_ccache;

#ifndef KRB5_NPROTOTYPE
#define KRB5_NPROTOTYPE(x) x
#endif

typedef struct _krb5_cc_ops {
    krb5_magic magic;
    char FAR *prefix;
    char FAR * (KRB5_CALLCONV *get_name) KRB5_NPROTOTYPE((krb5_context, krb5_ccache));
    krb5_error_code (KRB5_CALLCONV *resolve) KRB5_NPROTOTYPE((krb5_context, krb5_ccache FAR *,
					    const char FAR *));
    krb5_error_code (KRB5_CALLCONV *gen_new) KRB5_NPROTOTYPE((krb5_context, krb5_ccache FAR *));
    krb5_error_code (KRB5_CALLCONV *init) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_principal));
    krb5_error_code (KRB5_CALLCONV *destroy) KRB5_NPROTOTYPE((krb5_context, krb5_ccache));
    krb5_error_code (KRB5_CALLCONV *close) KRB5_NPROTOTYPE((krb5_context, krb5_ccache));
    krb5_error_code (KRB5_CALLCONV *store) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_creds FAR *));
    krb5_error_code (KRB5_CALLCONV *retrieve) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_flags, krb5_creds FAR *,
					    krb5_creds FAR *));
    krb5_error_code (KRB5_CALLCONV *get_princ) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_principal FAR *));
    krb5_error_code (KRB5_CALLCONV *get_first) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_cc_cursor FAR *));
    krb5_error_code (KRB5_CALLCONV *get_next) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_cc_cursor FAR *, krb5_creds FAR *));
    krb5_error_code (KRB5_CALLCONV *end_get) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_cc_cursor FAR *));
    krb5_error_code (KRB5_CALLCONV *remove_cred) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_flags, krb5_creds FAR *));
    krb5_error_code (KRB5_CALLCONV *set_flags) KRB5_NPROTOTYPE((krb5_context, krb5_ccache,
					    krb5_flags));
} krb5_cc_ops;

#endif /* !krb5_cc_principal */

#define KRB5_INIT_CONTEXT(context) pkrb5_init_context(context)
#define KRB5_FREE_CONTEXT(context) pkrb5_free_context(context)
#define KRB5_UNPARSE_NAME(context, principal, name) \
           pkrb5_unparse_name(context, principal, name)
#define KRB5_CC_DEFAULT(context,cache) pkrb5_cc_default(context, cache)
krb5_error_code KRB5_CC_GET_PRINCIPAL
                (krb5_context context, krb5_ccache cache,
		krb5_principal FAR *principal);


#endif _KRB5PROXY_H_
