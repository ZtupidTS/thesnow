#include "gssapi.h"
#include "krb5.h"

typedef OM_uint32 (KRB5_CALLCONV *t_gss_release_buffer)
			(OM_uint32 FAR *,		/* minor_status */
			gss_buffer_t			/* buffer */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_display_status)
			(OM_uint32 FAR *,		/* minor_status */
			OM_uint32,				/* status_value */
			int,					/* status_type */
			gss_OID,				/* mech_type (used to be const) */
			OM_uint32 FAR *,		/* message_context */
			gss_buffer_t			/* status_string */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_import_name)

			(OM_uint32 FAR *,		/* minor_status */
			gss_buffer_t,			/* input_name_buffer */
			gss_OID,				/* input_name_type(used to be const) */
			gss_name_t FAR *		/* output_name */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_release_name)
			(OM_uint32 FAR *,		/* minor_status */
			gss_name_t FAR *		/* input_name */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_wrap)
			(OM_uint32 FAR *,		/* minor_status */
			gss_ctx_id_t,			/* context_handle */
			int,					/* conf_req_flag */
			gss_qop_t,				/* qop_req */
			gss_buffer_t,			/* input_message_buffer */
			int FAR *,				/* conf_state */
			gss_buffer_t			/* output_message_buffer */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_unwrap)
			(OM_uint32 FAR *,		/* minor_status */
			gss_ctx_id_t,			/* context_handle */
			gss_buffer_t,			/* input_message_buffer */
			gss_buffer_t,			/* output_message_buffer */
			int FAR *,				/* conf_state */
			gss_qop_t FAR *			/* qop_state */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_init_sec_context)
			(OM_uint32 FAR *,		/* minor_status */
			gss_cred_id_t,			/* claimant_cred_handle */
			gss_ctx_id_t FAR *,		/* context_handle */
			gss_name_t,				/* target_name */
			gss_OID,				/* mech_type (used to be const) */
			OM_uint32,				/* req_flags */
			OM_uint32,				/* time_req */
			gss_channel_bindings_t,	/* input_chan_bindings */
			gss_buffer_t,			/* input_token */
			gss_OID FAR *,			/* actual_mech_type */
			gss_buffer_t,			/* output_token */
			OM_uint32 FAR *,		/* ret_flags */
			OM_uint32 FAR *			/* time_rec */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_wrap_size_limit)
			(OM_uint32 FAR *,		/* minor_status */
			gss_ctx_id_t,			/* context_handle */
			int,					/* conf_req_flag */
			gss_qop_t,				/* qop_req */
			OM_uint32,				/* req_output_size */
			OM_uint32 *				/* max_input_size */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_accept_sec_context)
			(OM_uint32 FAR *,		/* minor_status */
			gss_ctx_id_t FAR *,		/* context_handle */
			gss_cred_id_t,			/* acceptor_cred_handle */
			gss_buffer_t,			/* input_token_buffer */
			gss_channel_bindings_t,	/* input_chan_bindings */
			gss_name_t FAR *,		/* src_name */
			gss_OID FAR *,			/* mech_type */
			gss_buffer_t,			/* output_token */
			OM_uint32 FAR *,		/* ret_flags */
			OM_uint32 FAR *,		/* time_rec */
			gss_cred_id_t FAR *		/* delegated_cred_handle */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_acquire_cred)
			(OM_uint32 FAR *,		/* minor_status */
			gss_name_t,				/* desired_name */
			OM_uint32,				/* time_req */
			gss_OID_set,			/* desired_mechs */
			gss_cred_usage_t,		/* cred_usage */
			gss_cred_id_t FAR *,	/* output_cred_handle */
			gss_OID_set FAR *,		/* actual_mechs */
			OM_uint32 FAR *			/* time_rec */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_release_cred)
			(OM_uint32 FAR *,		/* minor_status */
			gss_cred_id_t FAR *		/* cred_handle */
			); 

typedef OM_uint32 (KRB5_CALLCONV *t_gss_krb5_copy_ccache)
			(OM_uint32 *minor_status,
			gss_cred_id_t cred_handle,
			krb5_ccache out_ccache
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_display_name)
			(OM_uint32 FAR *,		/* minor_status */
			gss_name_t,				/* input_name */
			gss_buffer_t,			/* output_name_buffer */
			gss_OID FAR *			/* output_name_type */
			);

typedef OM_uint32 (KRB5_CALLCONV *t_gss_unseal)
			(OM_uint32 FAR *,		/* minor_status */
			gss_ctx_id_t,			/* context_handle */
			gss_buffer_t,			/* input_message_buffer */
			gss_buffer_t,			/* output_message_buffer */
			int FAR *,				/* conf_state */
			int FAR *				/* qop_state */
			);

typedef gss_OID *t_gss_nt_service_name;

typedef krb5_error_code (KRB5_CALLCONV *t_krb5_cc_resolve)				(krb5_context, const char FAR *,	krb5_ccache FAR *);
typedef krb5_error_code (KRB5_CALLCONV *t_krb5_init_context)			(krb5_context FAR *);
typedef void			(KRB5_CALLCONV *t_krb5_free_context)			(krb5_context);
typedef krb5_error_code (KRB5_CALLCONV *t_krb5_parse_name)				(krb5_context, krb5_const char FAR *, krb5_principal FAR * );
typedef krb5_error_code (KRB5_CALLCONV_C *t_krb5_build_principal_ext)	(krb5_context, krb5_principal FAR *, int, krb5_const char FAR *, ...);
typedef void			(KRB5_CALLCONV *t_krb5_free_principal)			(krb5_context, krb5_principal );
typedef krb5_boolean	(KRB5_CALLCONV *t_krb5_kuserok)					(krb5_context, krb5_principal, const char *);
typedef krb5_error_code (KRB5_CALLCONV *t_krb5_timeofday)				(krb5_context, krb5_int32 FAR *);
typedef krb5_error_code (KRB5_CALLCONV *t_krb5_get_in_tkt_with_password)(krb5_context, krb5_const krb5_flags, krb5_address FAR * krb5_const FAR *,
																		 krb5_enctype FAR *, krb5_preauthtype FAR *, krb5_const char FAR *,
																		 krb5_ccache, krb5_creds FAR *,	krb5_kdc_rep FAR * FAR * );
typedef krb5_error_code (KRB5_CALLCONV *t_krb5_cc_default)				(krb5_context, krb5_ccache FAR *);
typedef krb5_error_code (KRB5_CALLCONV *t_krb5_unparse_name)			(krb5_context, krb5_const_principal, char FAR * FAR *);
typedef void			(KRB5_CALLCONV *t_krb5_free_unparsed_name)		(krb5_context, char FAR *);


