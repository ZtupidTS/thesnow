//	**************************************************************************************
//	File:			main.cpp
//	By:				David Euresti
//	Created:		07/02/99
//	Copyright		@1999 Massachusetts Institute of Technology - All rights reserved.
//	Description:	C file for anybody who wants to use the external API calls and
//					contains variables and functions for the SoFTP application
//
//	History:
//
//	MM/DD/YY	Inits	Description of Change
//	07/02/99	DE		Original
//	08/02/99	ADL	 Allot of modifing to fit the new SoFTP application
//	**************************************************************************************

#include <winsock2.h>
#include <Ws2tcpip.h>

#define DEFINITIONS
#include "ftp_var.h"
#include "externvars.h"

#include "gssapi.h"

#include <stdio.h>
#include "krb5.h"

static int ccid = 0;

#define DLLit extern "C" __declspec(dllexport)

typedef int (CALLBACK *t_callbackProc)(void *pData, int nParam1, int nParam2, int nParam3);

#include "CriticalSectionWrapper.h"
static CCriticalSectionWrapper critSection;

typedef struct
{
	int kerror;
	int auth_type;
	int authorized;
	gss_ctx_id_t gcontext;
	int	level;		/* protection level data channel*/
	int ctrlevel;   /* protection level control channel*/
	int limitpbsz;	   /* file size that we can send */
	int realpbsz;   /* file size that we need to allocate for in mem */
	int want_creds;
	int have_creds;
	gss_buffer_desc client_name;
	int askpasswd;
	int promptpasswd;
	krb5_ccache ccache;
	int nClientMode;
	int nClientState;

	struct t_ClientAuthData {
		char *hostname;
		const struct sockaddr *myaddr;
		const struct sockaddr *hisaddr;
		char protLevel;
		int gssBufferSize;
		OM_uint32 maj_stat, min_stat;
		gss_buffer_desc send_tok;
		gss_name_t target_name;
		gss_buffer_desc recv_tok, *token_ptr;
		struct gss_channel_bindings_struct chan;
		char out_buf[FTP_BUFSIZ];
		bool chanBindings;
	} clientAuthData;

	t_callbackProc pCallbackProc;
	void *pCallbackData;

	HINSTANCE hDLL;
	t_gss_release_buffer		gss_release_buffer;
	t_gss_display_status		gss_display_status;
	t_gss_import_name			gss_import_name;
	t_gss_release_name			gss_release_name;
	t_gss_wrap					gss_wrap;
	t_gss_unwrap				gss_unwrap;
	t_gss_init_sec_context		gss_init_sec_context;
	t_gss_wrap_size_limit		gss_wrap_size_limit;
	t_gss_accept_sec_context	gss_accept_sec_context;
	t_gss_acquire_cred			gss_acquire_cred;
	t_gss_release_cred			gss_release_cred;
	t_gss_krb5_copy_ccache		gss_krb5_copy_ccache;
	t_gss_display_name			gss_display_name;
	t_gss_unseal				gss_unseal;
	t_gss_nt_service_name		gss_nt_service_name;

	HINSTANCE hKRB5DLL;
	t_krb5_cc_resolve			krb5_cc_resolve;
	t_krb5_init_context			krb5_init_context;
	t_krb5_free_context			krb5_free_context;
	t_krb5_parse_name			krb5_parse_name;
	t_krb5_build_principal_ext	krb5_build_principal_ext;
	t_krb5_free_principal		krb5_free_principal;
	t_krb5_kuserok				krb5_kuserok;
	t_krb5_timeofday			krb5_timeofday;
	t_krb5_get_in_tkt_with_password	krb5_get_in_tkt_with_password;
	t_krb5_cc_default			krb5_cc_default;
	t_krb5_free_unparsed_name	krb5_free_unparsed_name;
	t_krb5_unparse_name			krb5_unparse_name;

	krb5_context kcontext;

} t_GSS_API_data;

const char* gss_services[] = { "ftp", "host", 0 };

#define PROT_C							  'C'
#define PROT_S							  'S'
#define PROT_P							  'P'


extern "C" int radix_encode(void *in, void *out,
					int *len,  int decode);
extern "C" char *radix_error(int e);

//Command handlers
int DoAdat(t_GSS_API_data* pGssApiData, char* args, char* sendme);
int DoProt(t_GSS_API_data* pGssApiData, char* args, char* sendme);
int DoPbsz(t_GSS_API_data* pGssApiData, char* args, char* sendme);
int DoUser(t_GSS_API_data *pGssApiData, char* user, char* sendme);
int DoPass(t_GSS_API_data *pGssApiData, char* name, char* passwd, char* sendme);

void reply_gss_error(t_GSS_API_data *pGssApiData, OM_uint32 maj_stat, OM_uint32 min_stat, char* s);
void ftpd_gss_convert_creds(t_GSS_API_data* pGssApiData, char* name, gss_cred_id_t creds);
int ftpd_gss_userok(t_GSS_API_data *pGssApiData, char * name);
//****

/* ftpd_gss_convert_creds -- write out forwarded creds */
/* (code lifted from login.krb5) */
void ftpd_gss_convert_creds(t_GSS_API_data* pGssApiData, char* name, gss_cred_id_t creds)
{
	OM_uint32 major_status, minor_status;
	krb5_principal me;
	char tmpPath[MAX_PATH+10];
	char ccname[MAX_PATH];

	memset(tmpPath, 0, sizeof(tmpPath));

	CLock lock(&critSection);
	
	/* Set up ccache */
	if (pGssApiData->krb5_parse_name(pGssApiData->kcontext, name, &me))
		return;
	
	GetTempPath(MAX_PATH, tmpPath);
	if (strcmp(tmpPath, "") == 0)
		strcpy(tmpPath, "c:\\tmp");

	sprintf(ccname, "FILE:%skrb5cc_ftpd%d", tmpPath, ccid++);
	if (pGssApiData->krb5_cc_resolve(pGssApiData->kcontext, ccname, &(pGssApiData->ccache)))
		return;

	if (krb5_cc_initialize(pGssApiData->kcontext, pGssApiData->ccache, me))
		return;

	/* Copy GSS creds into ccache */
	major_status = pGssApiData->gss_krb5_copy_ccache(&minor_status, creds, pGssApiData->ccache);
	if (major_status == GSS_S_COMPLETE)
	{
		pGssApiData->have_creds = 1;
		return;
	}

	krb5_cc_destroy(pGssApiData->kcontext, pGssApiData->ccache);
}

void reply_gss_error(t_GSS_API_data* pGssApiData, OM_uint32 maj_stat, OM_uint32 min_stat, char* s)
{
	/* a lot of work just to report the error */
	OM_uint32 gmaj_stat, gmin_stat;
	gss_buffer_desc msg;
	unsigned int msg_ctx;
	msg_ctx = 0;

	CLock lock(&critSection);
	do
	{
		gmaj_stat = pGssApiData->gss_display_status(&gmin_stat, maj_stat,
													GSS_C_GSS_CODE,
													GSS_C_NULL_OID,
													&msg_ctx, &msg);
		if ((gmaj_stat == GSS_S_COMPLETE)||
			(gmaj_stat == GSS_S_CONTINUE_NEEDED))
		{
			if (strlen(s))
				strcat(s, " ");
			strcat(s, "GSSAPI error major: ");
			strcat(s,  (char*)msg.value);
			pGssApiData->gss_release_buffer(&gmin_stat, &msg);
		}
		if (gmaj_stat != GSS_S_CONTINUE_NEEDED)
			break;
	} while (msg_ctx);
	msg_ctx = 0;
	do
	{
		gmaj_stat = pGssApiData->gss_display_status(&gmin_stat, min_stat,
													GSS_C_MECH_CODE,
													GSS_C_NULL_OID,
													&msg_ctx, &msg);
		if ((gmaj_stat == GSS_S_COMPLETE)||
			(gmaj_stat == GSS_S_CONTINUE_NEEDED)) {
			if (strlen(s))
				strcat(s, " ");
			strcat(s, "GSSAPI error minor: ");
			strcat(s, (char*)msg.value);
			pGssApiData->gss_release_buffer(&gmin_stat, &msg);
		}
		if (gmaj_stat != GSS_S_CONTINUE_NEEDED)
			break;
	} while (msg_ctx);
}

BOOL WINAPI DllMain( HANDLE hModule, DWORD fdwreason,  LPVOID lpReserved )
{
	return TRUE;
}

DLLit BOOL InitGSS(VOID *pData, t_callbackProc pCallbackProc, void *pCallbackData, int promptPassword)
{
	t_GSS_API_data **ppGssApiData;
	t_GSS_API_data *pGssApiData;
	ppGssApiData=(t_GSS_API_data **)pData;
	*ppGssApiData = new t_GSS_API_data;
	pGssApiData=*ppGssApiData;

	memset(pGssApiData, 0, sizeof(t_GSS_API_data));
	pGssApiData->auth_type=0;
	pGssApiData->want_creds = 0;
	pGssApiData->have_creds = 0;
	pGssApiData->limitpbsz = 0;	  /* file size that we can send */
	pGssApiData->realpbsz = 0;
	pGssApiData->level = 0;
	pGssApiData->ctrlevel = PROT_P;
	pGssApiData->authorized = 0;
	pGssApiData->promptpasswd = promptPassword;
	pGssApiData->askpasswd = 0;
	pGssApiData->pCallbackProc = pCallbackProc;
	pGssApiData->pCallbackData = pCallbackData;

	if (!pCallbackProc)
		return FALSE;

	critSection.Lock();
	pGssApiData->hDLL = LoadLibrary("gssapi32.dll");
	critSection.Unlock();
	if (!pGssApiData->hDLL)
		return FALSE;

	pGssApiData->gss_release_buffer = (t_gss_release_buffer)GetProcAddress(pGssApiData->hDLL, "gss_release_buffer");
	pGssApiData->gss_display_status = (t_gss_display_status)GetProcAddress(pGssApiData->hDLL, "gss_display_status");
	pGssApiData->gss_import_name = (t_gss_import_name)GetProcAddress(pGssApiData->hDLL, "gss_import_name");
	pGssApiData->gss_release_name = (t_gss_release_name)GetProcAddress(pGssApiData->hDLL, "gss_release_name");
	pGssApiData->gss_wrap = (t_gss_wrap)GetProcAddress(pGssApiData->hDLL, "gss_wrap");
	pGssApiData->gss_unwrap = (t_gss_unwrap)GetProcAddress(pGssApiData->hDLL, "gss_unwrap");
	pGssApiData->gss_init_sec_context = (t_gss_init_sec_context)GetProcAddress(pGssApiData->hDLL, "gss_init_sec_context");
	pGssApiData->gss_wrap_size_limit = (t_gss_wrap_size_limit)GetProcAddress(pGssApiData->hDLL, "gss_wrap_size_limit");
	pGssApiData->gss_nt_service_name = (t_gss_nt_service_name)GetProcAddress(pGssApiData->hDLL, "gss_nt_service_name");
	pGssApiData->gss_accept_sec_context = (t_gss_accept_sec_context)GetProcAddress(pGssApiData->hDLL, "gss_accept_sec_context");
	pGssApiData->gss_acquire_cred = (t_gss_acquire_cred)GetProcAddress(pGssApiData->hDLL, "gss_acquire_cred");
	pGssApiData->gss_release_cred = (t_gss_release_cred)GetProcAddress(pGssApiData->hDLL, "gss_release_cred");
	pGssApiData->gss_krb5_copy_ccache = (t_gss_krb5_copy_ccache)GetProcAddress(pGssApiData->hDLL, "gss_krb5_copy_ccache");
	pGssApiData->gss_display_name = (t_gss_display_name)GetProcAddress(pGssApiData->hDLL, "gss_display_name");

	if (!pGssApiData->gss_release_buffer ||
		!pGssApiData->gss_display_status ||
		!pGssApiData->gss_import_name ||
		!pGssApiData->gss_release_name ||
		!pGssApiData->gss_wrap ||
		!pGssApiData->gss_unwrap ||
		!pGssApiData->gss_init_sec_context ||
		!pGssApiData->gss_wrap_size_limit ||
		!pGssApiData->gss_nt_service_name ||
		!pGssApiData->gss_accept_sec_context ||
		!pGssApiData->gss_acquire_cred ||
		!pGssApiData->gss_release_cred ||
		!pGssApiData->gss_krb5_copy_ccache ||
		!pGssApiData->gss_display_name)
	{
		FreeLibrary(pGssApiData->hDLL);
		pGssApiData->hDLL = NULL;
		return FALSE;
	}

	if (!pGssApiData->hKRB5DLL)
	{
		critSection.Lock();
		pGssApiData->hKRB5DLL = LoadLibrary("krb5_32.dll");
		critSection.Unlock();
		if (!pGssApiData->hKRB5DLL)
			return FALSE;

		pGssApiData->krb5_cc_resolve			= (t_krb5_cc_resolve)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_cc_resolve");
		pGssApiData->krb5_init_context			= (t_krb5_init_context)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_init_context");
		pGssApiData->krb5_free_context			= (t_krb5_free_context)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_free_context");
		pGssApiData->krb5_parse_name			= (t_krb5_parse_name)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_parse_name");
		pGssApiData->krb5_build_principal_ext	= (t_krb5_build_principal_ext)	GetProcAddress(pGssApiData->hKRB5DLL, "krb5_build_principal_ext");
		pGssApiData->krb5_free_principal		= (t_krb5_free_principal)		GetProcAddress(pGssApiData->hKRB5DLL, "krb5_free_principal");
		pGssApiData->krb5_kuserok				= (t_krb5_kuserok)				GetProcAddress(pGssApiData->hKRB5DLL, "krb5_kuserok");
		pGssApiData->krb5_timeofday				= (t_krb5_timeofday)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_timeofday");
		pGssApiData->krb5_get_in_tkt_with_password	= (t_krb5_get_in_tkt_with_password)	GetProcAddress(pGssApiData->hKRB5DLL, "krb5_get_in_tkt_with_password");
		pGssApiData->krb5_cc_default			= (t_krb5_cc_default)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_cc_default");
		pGssApiData->krb5_unparse_name			=(t_krb5_unparse_name)			GetProcAddress(pGssApiData->hKRB5DLL, "krb5_unparse_name");
		pGssApiData->krb5_free_unparsed_name	= (t_krb5_free_unparsed_name)	GetProcAddress(pGssApiData->hKRB5DLL, "krb5_free_unparsed_name");

		if (!pGssApiData->krb5_cc_resolve	||
			!pGssApiData->krb5_init_context	||
			!pGssApiData->krb5_free_context	||
			!pGssApiData->krb5_parse_name	||
			!pGssApiData->krb5_build_principal_ext	||
			!pGssApiData->krb5_free_principal	||
			!pGssApiData->krb5_kuserok	||
			!pGssApiData->krb5_timeofday	||
			!pGssApiData->krb5_get_in_tkt_with_password	||
			!pGssApiData->krb5_cc_default ||
			!pGssApiData->krb5_unparse_name
			)
		{
			FreeLibrary(pGssApiData->hKRB5DLL);
			pGssApiData->hKRB5DLL = NULL;
			return FALSE;
		}
	}

	critSection.Lock();
	pGssApiData->krb5_init_context(&pGssApiData->kcontext);
	critSection.Unlock();

	return TRUE;
}

// This method should only be called right before destruction.
DLLit BOOL KillGSS(void *pData)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	if (pGssApiData->have_creds)
	{
		critSection.Lock();
		krb5_cc_destroy(pGssApiData->kcontext, pGssApiData->ccache);
		critSection.Unlock();
		pGssApiData->have_creds = 0;
	}

	if (pGssApiData->hDLL)
		FreeLibrary(pGssApiData->hDLL);

	if (pGssApiData->hKRB5DLL)
	{
		critSection.Lock();
		pGssApiData->krb5_free_context(pGssApiData->kcontext);
		critSection.Unlock();
		FreeLibrary(pGssApiData->hKRB5DLL);
	}

	delete [] pGssApiData->clientAuthData.hostname;
	delete pGssApiData;

	return TRUE;
}

// Takes a *cmd and encrypts it Returns 0 if unsuccesful 1 if succesful
DLLit int EncryptMessage(void *pData, char *sendcommand, char* sendme)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	char temp[FTP_BUFSIZ], temp2[FTP_BUFSIZ];
	int length;
	memset(temp, 0, sizeof(temp));
	memset(temp2, 0, sizeof(temp));

	if (pGssApiData->auth_type)
	{
		gss_buffer_desc in_buf, out_buf;
		OM_uint32 maj_stat, min_stat;
		int conf_state;
		in_buf.value = sendcommand;
		in_buf.length = strlen(sendcommand) + 1;
		critSection.Lock();
		maj_stat = pGssApiData->gss_wrap(&min_stat, pGssApiData->gcontext,
			pGssApiData->ctrlevel == PROT_P, // confidential
			GSS_C_QOP_DEFAULT,
			&in_buf, &conf_state,
			&out_buf);
		critSection.Unlock();
		if (maj_stat != GSS_S_COMPLETE)
		{
			if (sendme)
			{
				strcpy(sendme, "535 unable to wrap data:");
				critSection.Lock();
				reply_gss_error(pGssApiData, maj_stat, min_stat, sendme);
				critSection.Unlock();
			}
			return 0;
		}
		else if ((pGssApiData->ctrlevel == PROT_P) && !conf_state)
		{
			if (sendme)
				strcpy(sendme, "535 only integrity service has been applied");
			return 0;
		}
		else
		{
			length = out_buf.length;
			memcpy(sendcommand, out_buf.value, length);
			sendcommand[length] = '\0';
			critSection.Lock();
			pGssApiData->gss_release_buffer(&min_stat, &out_buf);
			critSection.Unlock();
			if (pGssApiData->kerror = radix_encode(sendcommand, temp, &length, 0))
			{
				if (sendme)
					sprintf(sendme,"Couldn't base 64 encode command (%s)",
						radix_error(pGssApiData->kerror));
				return 0;
			}
			else
			{
				if (pGssApiData->nClientMode)
					sprintf(temp2, "%s %s\0", pGssApiData->ctrlevel==PROT_P ? "ENC" : "MIC", temp);
				else
					sprintf(temp2, "%s %s\0", pGssApiData->ctrlevel==PROT_P ? "632" : "631", temp);
				memcpy(sendcommand, temp2, length+5);
				sendcommand[length+5] = '\0';
				return 1;
			}
		}
	}

	return 0;
}

DLLit int DecryptMessage(void *pData,
						 char *ibuf, char* sendme)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	if (pGssApiData->auth_type)
	{
		int safe = 0;
		int kerror;
		char temp3;
		char temp[FTP_BUFSIZ], temp2[FTP_BUFSIZ];
		int len;


		gss_buffer_desc xmit_buf, msg_buf;
		OM_uint32 maj_stat, min_stat;
		int conf_state;
		memset(temp, 0, sizeof(temp));
		memset(temp2, 0, sizeof(temp2));
		temp3 = ibuf[3];
		ibuf[3] = 0;
		safe = atoi (ibuf);
		ibuf[3] = temp3;


		// Kill off reply Code
		strcpy(temp, &ibuf[4]);
		if (kerror = radix_encode(temp, temp2, &len, 1))
		{
			if (pGssApiData->pCallbackProc)
			{
				char debugbuffer[FTP_BUFSIZ];
				sprintf(debugbuffer, "Couln't decode data (%s)", radix_error(kerror));
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)debugbuffer, 0);
			}
			if (sendme)
				sprintf(sendme, "501 Couldn't decode data (%s)",
					radix_error(kerror));
			return 0;
		}

		xmit_buf.value = temp2;
		xmit_buf.length = len;

		// decrypt/verify the message
		conf_state = safe;
		critSection.Lock();
		maj_stat = pGssApiData->gss_unwrap(&min_stat, pGssApiData->gcontext,
			&xmit_buf, &msg_buf,
			&conf_state, NULL);
		critSection.Unlock();
		if (maj_stat != GSS_S_COMPLETE)
		{
			if (sendme)
			{
				strcpy(sendme, "535 unable to unwrap data:");
				reply_gss_error(pGssApiData, maj_stat, min_stat, sendme);
			}
			return 0;
		}
		else
		{
			memcpy(ibuf, msg_buf.value, msg_buf.length);
			ibuf[msg_buf.length] = '\0';
			critSection.Lock();
			pGssApiData->gss_release_buffer(&min_stat,&msg_buf);
			critSection.Unlock();
			return 1;
		}
	}

	return 0;
}

DLLit u_long EncryptData(void *pData,
	char* chunk,
	int length,
	char* sendme
	)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	if (pGssApiData->auth_type)
	{
		gss_buffer_desc in_buf, out_buf;
		OM_uint32 maj_stat, min_stat;
		int conf_state;

		in_buf.value = chunk;
		in_buf.length = length;
		critSection.Lock();
		maj_stat = pGssApiData->gss_wrap(&min_stat, pGssApiData->gcontext,
			pGssApiData->level=PROT_P, // confidential
			GSS_C_QOP_DEFAULT,
			&in_buf, &conf_state,
			&out_buf);
		critSection.Unlock();
		if (maj_stat != GSS_S_COMPLETE)
		{
			strcpy(sendme, "535 unable to wrap data:");
			reply_gss_error(pGssApiData, maj_stat, min_stat, sendme);
			return 0;
		}
		else if ((PROT_P == pGssApiData->level) && !conf_state)
		{
			strcpy(sendme, "535 only integrity service has been applied");
			return 0;
		}
		else
		{
			char *p;
			char xch;
			unsigned int temp;

			temp = out_buf.length;
			length = out_buf.length;
			p = (char *)&temp;
			xch = p[0]; p[0] = p[3]; p[3] = xch;
			xch = p[1]; p[1] = p[2]; p[2] = xch;
			memcpy(chunk, &temp, 4);
			memcpy(&chunk[4], out_buf.value, out_buf.length);
			critSection.Lock();
			pGssApiData->gss_release_buffer(&min_stat, &out_buf);
			critSection.Unlock();
			return length + 4;
		}
	}

	return 0;
}

DLLit u_long DecryptData(void *pData, char* chunk, int length, char* sendme)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	if (pGssApiData->auth_type)
	{
		gss_buffer_desc xmit_buf, msg_buf;
		OM_uint32 maj_stat, min_stat;
		int conf_state;
		xmit_buf.value = chunk;
		xmit_buf.length = length;

		// decrypt/verify the message
		conf_state = 1;
		critSection.Lock();
		maj_stat = pGssApiData->gss_unwrap(&min_stat, pGssApiData->gcontext,
			&xmit_buf, &msg_buf,
			&conf_state, NULL);
		critSection.Unlock();

		if (GSS_ERROR(maj_stat))
		{
			strcpy(sendme, "535 unable to unwrap data:");
			reply_gss_error(pGssApiData, maj_stat, min_stat, sendme);
			return 0;
		}
		else
		{
			memcpy(chunk, msg_buf.value, msg_buf.length);
			length = msg_buf.length;
			critSection.Lock();
			pGssApiData->gss_release_buffer(&min_stat,&msg_buf);
			critSection.Unlock();
			return length;
		}
	}

	return 0;
}

DLLit int ProcessCommand(void* pData, const char* command, char* args, char* sendme)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;
	if (strcmp(command, "ADAT") == 0)
		return DoAdat(pGssApiData, args, sendme);
	if (strcmp(command, "PROT") == 0)
		return DoProt(pGssApiData, args, sendme);
	if (strcmp(command, "PBSZ") == 0)
		return DoPbsz(pGssApiData, args, sendme);
	if (strcmp(command, "USER") == 0)
		return DoUser(pGssApiData, args, sendme);
	if (strcmp(command, "PASS") == 0)
		return DoPass(pGssApiData, args, args + strlen(args) + 1, sendme);
	return 0;
}

int ftpd_gss_userok(t_GSS_API_data *pGssApiData, char * name)
{
	int retval = -1;
	krb5_principal p;

	critSection.Lock();
	retval = pGssApiData->krb5_parse_name(pGssApiData->kcontext, (char *)pGssApiData->client_name.value, &p);
	critSection.Unlock();
	if (retval)
		return -1;
	
	critSection.Lock();
	retval = pGssApiData->krb5_kuserok(pGssApiData->kcontext, p, name);
	critSection.Lock();
	if (retval)
		retval = 0;
	else
		retval = 1;
	critSection.Lock();
	pGssApiData->krb5_free_principal(pGssApiData->kcontext, p);
	critSection.Unlock();
	return retval;
}

int DoUser(t_GSS_API_data *pGssApiData, char* user, char* sendme)
{
	int result;
	char buf[FTP_BUFSIZ];
	if (!pGssApiData->auth_type) {
		strcpy(sendme, "530 Must perform authentication before identifying USER.");
		return -1;
	}

	pGssApiData->authorized = (ftpd_gss_userok(pGssApiData, user) == 0);
	sprintf(buf, "GSSAPI user %s is%s authorized as %s",
			(pGssApiData->client_name).value, pGssApiData->authorized ? "" : " not",
				user);

	if (!pGssApiData->authorized && !pGssApiData->promptpasswd) {
		strcat(buf, "; Access denied.");
		result = 530;
	}
	else if (!pGssApiData->authorized || (pGssApiData->want_creds && !pGssApiData->have_creds))
	{
		strcat(buf, "; Password required.");
		pGssApiData->askpasswd = 1;
		result = 331;
	}
	else
		result = 232;

	sprintf(sendme, "%d %s", result, buf);
	return (pGssApiData->authorized ? 0 : -1);

}

int DoPass(t_GSS_API_data *pGssApiData, char* name, char* passwd, char* sendme)
{
	char tmpPath[MAX_PATH];

	krb5_principal server, me;
	krb5_creds my_creds;
	krb5_timestamp now;
	char ccname[MAX_PATH];

	if (pGssApiData->authorized && !pGssApiData->want_creds) {
		strcpy(sendme, "202 PASS command superfluous.");
		return -1;
	}

	if ((pGssApiData->askpasswd) == 0) {
	  	strcpy(sendme, "503 Login with USER first.");
		return -1;
	}

	strcpy(sendme, "530 Login incorrect.");
	memset((char *)&my_creds, 0, sizeof(my_creds));

	memset(tmpPath, 0, sizeof(tmpPath));
	
	CLock lock(&critSection);
	if (pGssApiData->krb5_parse_name(pGssApiData->kcontext, name, &me))
		return -1;
	my_creds.client = me;

	GetTempPath(MAX_PATH, tmpPath);
	if (strcmp(tmpPath, "") == 0)
		strcpy(tmpPath, "c:\\tmp");

	sprintf(ccname, "FILE:%skrb5cc_ftpd%d", tmpPath, ccid++);

	if (pGssApiData->krb5_cc_resolve(pGssApiData->kcontext, ccname, &(pGssApiData->ccache)))
		return(-1);
	
	if (krb5_cc_initialize(pGssApiData->kcontext, pGssApiData->ccache, me))
		return(-1);
	
	if (pGssApiData->krb5_build_principal_ext(pGssApiData->kcontext, &server,
					 krb5_princ_realm(pGssApiData->kcontext, me)->length,
					 krb5_princ_realm(pGssApiData->kcontext, me)->data,
					 KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME,
					 krb5_princ_realm(pGssApiData->kcontext, me)->length,
					 krb5_princ_realm(pGssApiData->kcontext, me)->data,
					 0))
		goto nuke_ccache;
	
	my_creds.server = server;
	if (pGssApiData->krb5_timeofday(pGssApiData->kcontext, &now))
		goto nuke_ccache;
	my_creds.times.starttime = 0; /* start timer when
					 request gets to KDC */
	my_creds.times.endtime = now + 60 * 60 * 10;
	my_creds.times.renew_till = 0;

	if (pGssApiData->krb5_get_in_tkt_with_password(pGssApiData->kcontext, 0,
					  0, NULL, 0 /*preauth*/,
					  passwd,
					  pGssApiData->ccache,
					  &my_creds, 0))
		goto nuke_ccache;

	if (pGssApiData->askpasswd)
		pGssApiData->askpasswd = 0;


	sprintf(sendme, "230 User %s logged in.", name);
	if (!pGssApiData->want_creds) {
		krb5_cc_destroy(pGssApiData->kcontext, pGssApiData->ccache);
		return 0;
	}
	
	pGssApiData->have_creds = 1;
	return 0;

nuke_ccache:

	krb5_cc_destroy(pGssApiData->kcontext, pGssApiData->ccache);
	return -1;
}

int DoAdat(t_GSS_API_data* pGssApiData, char* args, char* sendme)
{
	int kerror, length;
	int replied = 0;
	int found = 0;
	gss_cred_id_t server_creds = {0};
	gss_cred_id_t deleg_creds = {0};
	gss_name_t client = {0};
	unsigned int ret_flags = 0;
	gss_OID mechid = {0};
	gss_buffer_desc name_buf = {0};
	gss_name_t server_name = {0};
	OM_uint32 acquire_maj = 0;
	OM_uint32 acquire_min = 0;
	OM_uint32 accept_maj = 0;
	OM_uint32 accept_min = 0;
	OM_uint32 stat_maj = 0;
	OM_uint32 stat_min = 0;
	gss_buffer_desc tok = {0};
	gss_buffer_desc out_tok = {0};
	char gbuf[FTP_BUFSIZ];
	u_char gout_buf[FTP_BUFSIZ];
	char *localname = 0;
	char service_name[MAXHOSTNAMELEN+10];
	char **service = 0;
	struct gss_channel_bindings_struct chan = {0};
	int addrlen = 0;
	char xxx[40000];
	memset(xxx, 0, 40000);

	if (pGssApiData->auth_type) {
		strcpy(sendme, "503 Authentication already established");
		return -1;
	}

	localname = args+8;

	if (kerror = radix_encode(args + 8 + strlen(localname) + 1, gout_buf, &length, 1))
	{
		sprintf(sendme, "501 Couldn't decode ADAT (%s)",
				  radix_error(kerror));
			return -1;
	}

	tok.value = gout_buf;
	tok.length = length;

	chan.initiator_addrtype = GSS_C_AF_INET;
	chan.initiator_address.length = 4;
	chan.initiator_address.value = args;
	chan.acceptor_addrtype = GSS_C_AF_INET;
	chan.acceptor_address.length = 4;
	chan.acceptor_address.value = args+4;
	chan.application_data.length = 0;
	chan.application_data.value = 0;

	for (service = (char**)gss_services; *service; service++)
	{
		sprintf(service_name, "%s@%s", *service, localname);
		name_buf.value = service_name;
		name_buf.length = strlen((char *)name_buf.value) + 1;

		critSection.Lock();
		stat_maj = pGssApiData->gss_import_name(&stat_min, &name_buf,
												*pGssApiData->gss_nt_service_name,
												&server_name);
		critSection.Unlock();

		if (stat_maj != GSS_S_COMPLETE)
		{
			strcpy(sendme, "501 gss importing name failed:");
			reply_gss_error(pGssApiData, stat_maj, stat_min, sendme);
			return -1;
		}

		critSection.Lock();
		acquire_maj = pGssApiData->gss_acquire_cred(&acquire_min, server_name, 0,
							   GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
							   &server_creds, NULL, NULL);
		pGssApiData->gss_release_name(&stat_min, &server_name);
		critSection.Unlock();

		if (acquire_maj != GSS_S_COMPLETE)
			continue;

		found++;

		pGssApiData->gcontext = GSS_C_NO_CONTEXT;

		critSection.Lock();
		accept_maj = pGssApiData->gss_accept_sec_context(&accept_min,
					&pGssApiData->gcontext, /* context_handle */
					server_creds, /* verifier_cred_handle */
					&tok, /* input_token */
					&chan, /* channel bindings */
					&client, /* src_name */
					&mechid, /* mech_type */
					&out_tok, /* output_token */
					&ret_flags,
					NULL, 	/* ignore time_rec */
					&deleg_creds  /* forwarded credentials */
								);
		critSection.Unlock();
		
		if (accept_maj==GSS_S_COMPLETE||accept_maj==GSS_S_CONTINUE_NEEDED)
			break;
	}

	if (found)
	{
		if (accept_maj!=GSS_S_COMPLETE && accept_maj!=GSS_S_CONTINUE_NEEDED)
		{
			strcpy(sendme, "535 accepting context:");
			reply_gss_error(pGssApiData, accept_maj, accept_min, sendme);
			critSection.Lock();
			pGssApiData->gss_release_cred(&stat_min, &server_creds);
			if (ret_flags & GSS_C_DELEG_FLAG)
				pGssApiData->gss_release_cred(&stat_min,
							&deleg_creds);
			critSection.Unlock();
			return -1;
		}
	}
	else
	{
		/* Kludge to make sure the right error gets reported, so we don't *
		 * get those nasty "error: no error" messages.					  */
		strcpy(sendme, "501 acquiring credentials:");
		if(stat_maj != GSS_S_COMPLETE)
			reply_gss_error(pGssApiData, stat_maj, stat_min, sendme);
		else
			reply_gss_error(pGssApiData, acquire_maj, acquire_min,
							sendme);
		return -1;
	}

	if (out_tok.length)
	{
		if (out_tok.length >= ((FTP_BUFSIZ - sizeof("ADAT="))
					   / 4 * 3))
		{
			sprintf(sendme, "ADAT: reply too long");
			critSection.Lock();
			pGssApiData->gss_release_cred(&stat_min, &server_creds);
			if (ret_flags & GSS_C_DELEG_FLAG)
				pGssApiData->gss_release_cred(&stat_min,
							&deleg_creds);
			critSection.Unlock();
			return -1;
		}
		if (kerror = radix_encode(out_tok.value, gbuf, (int *)&out_tok.length, 0))
		{
			sprintf(sendme, "Couldn't encode ADAT reply (%s)",
					 radix_error(kerror));
			critSection.Lock();
			pGssApiData->gss_release_cred(&stat_min, &server_creds);
			if (ret_flags & GSS_C_DELEG_FLAG)
				pGssApiData->gss_release_cred(&stat_min,
							&deleg_creds);
			critSection.Unlock();
			return -1;
		}
		if (stat_maj == GSS_S_COMPLETE)
		{
			sprintf(sendme, "235 ADAT=%s", gbuf);
			replied = 1;
		}
		else
		{
			/* If the server accepts the security data, and
			   requires additional data, it should respond
			   with reply code 335. */
			sprintf(sendme, "335 ADAT=%s", gbuf);
		}
		critSection.Lock();
		pGssApiData->gss_release_buffer(&stat_min, &out_tok);
		critSection.Unlock();
	}
	if (stat_maj == GSS_S_COMPLETE) {
		/* GSSAPI authentication succeeded */
		critSection.Lock();
		stat_maj = pGssApiData->gss_display_name(&stat_min, client,
												&(pGssApiData->client_name), &mechid);
		critSection.Unlock();
		if (stat_maj != GSS_S_COMPLETE) {
			/* "If the server rejects the security data (if
			   a checksum fails, for instance), it should
			   respond with reply code 535." */
			strcpy(sendme, "535 extracting GSSAPI identity name:");
			reply_gss_error(pGssApiData, stat_maj, stat_min, sendme);
			
			critSection.Lock();
			pGssApiData->gss_release_cred(&stat_min, &server_creds);
			if (ret_flags & GSS_C_DELEG_FLAG)
				pGssApiData->gss_release_cred(&stat_min,
							&deleg_creds);
			critSection.Unlock();
			return -1;
		}
		pGssApiData->auth_type = 1;
		pGssApiData->realpbsz = FTP_BUFSIZ;
		pGssApiData->ctrlevel = PROT_P;
		
		pGssApiData->gss_release_cred(&stat_min, &server_creds);
		if (ret_flags & GSS_C_DELEG_FLAG)
		{
			critSection.Lock();
			if (pGssApiData->want_creds)
				ftpd_gss_convert_creds(pGssApiData, (char *)pGssApiData->client_name.value,
									   deleg_creds);
			pGssApiData->gss_release_cred(&stat_min, &deleg_creds);
			critSection.Unlock();
		}

		/* If the server accepts the security data, but does
		   not require any additional data (i.e., the security
		   data exchange has completed successfully), it must
		   respond with reply code 235. */
		if (!replied)
		  {
			if (ret_flags & GSS_C_DELEG_FLAG && !(pGssApiData->have_creds))
			  strcpy(sendme, "235 GSSAPI Authentication succeeded, but could not accept forwarded credentials");
			else
			  strcpy(sendme, "235 GSSAPI Authentication succeeded");
		  }

		return 0;
	}
	else if (stat_maj == GSS_S_CONTINUE_NEEDED)
	{
		/* If the server accepts the security data, and
		   requires additional data, it should respond with
		   reply code 335. */
		strcpy(sendme, "335 more data needed");
		critSection.Lock();
		pGssApiData->gss_release_cred(&stat_min, &server_creds);
		if (ret_flags & GSS_C_DELEG_FLAG)
			pGssApiData->gss_release_cred(&stat_min, &deleg_creds);
		critSection.Unlock();
		return 0;
	}
	else
	{
		/* "If the server rejects the security data (if
		   a checksum fails, for instance), it should
		   respond with reply code 535." */
		strcpy(sendme, "535 GSSAPI failed processing ADAT:");
		reply_gss_error(pGssApiData, stat_maj, stat_min,
						sendme);
		critSection.Lock();
		pGssApiData->gss_release_cred(&stat_min, &server_creds);
		if (ret_flags & GSS_C_DELEG_FLAG)
			pGssApiData->gss_release_cred(&stat_min, &deleg_creds);
		critSection.Unlock();
		return -1;
	}

	return 0;
}

int DoProt(t_GSS_API_data* pGssApiData, char* args, char* sendme)
{
	if (pGssApiData->limitpbsz)
	{
		char prot_level = args[0];
		switch (prot_level) {
		case PROT_S:
		case PROT_P:
			{
				sprintf(sendme, "200 Data channel protection level set to %s.",
					(pGssApiData->level = prot_level) == PROT_S ?
						"safe" : pGssApiData->level == PROT_P ?
						"private" : "clear");
				return 0;
			}
		default:
			sprintf(sendme, "536 protection level not supported.", prot_level);
		}
	}
	else
	{
		strcpy(sendme, "503 Must first set PBSZ");
		return -1;
	}
	return 0;
}

int DoPbsz(t_GSS_API_data* pGssApiData, char* args, char* sendme)
{
	if (!pGssApiData->auth_type)
	{
		strcpy(sendme, "503 Must first perform authentication");
		return -1;
	}
	else if (strlen(args) > 10 ||
			 strlen(args) == 10 && strcmp(args,"4294967296") >= 0)
	{
		sprintf(sendme, "501 Bad value for PBSZ: %s", args);
		return -1;
	}
	else
	{
		char* ucbuf = NULL;
		unsigned int actualbuf = (unsigned int) atol(args);

		pGssApiData->realpbsz = actualbuf;
		/* I attempt what is asked for first, and if that
		fails, I try dividing by 4 */
		while ((ucbuf = (char *)malloc(actualbuf)) == NULL)
		{
			if (actualbuf)
				sprintf(sendme, "200 Trying %u", actualbuf >>= 2);
			else
			{
				strcpy(sendme, "421 Local resource failure: malloc");
				return -1;
			}
		}

		if (ucbuf)
			free(ucbuf);

		pGssApiData->limitpbsz = actualbuf;
		sprintf(sendme, "200 PBSZ=%u", actualbuf);
	}

	return 0;
}

unsigned long SizeGssBuffer(void *pData, unsigned long pGssBufferSize)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	unsigned int temp;
	OM_uint32 maj_stat, min_stat;

	critSection.Lock();
	maj_stat = pGssApiData->gss_wrap_size_limit(&min_stat, pGssApiData->gcontext,
								   PROT_P,GSS_C_QOP_DEFAULT,
								   pGssBufferSize, &temp);
	critSection.Unlock();
	
	return temp;
}

DLLit int DoClientAuth(void *pData,
					   char *hostname,
					   const struct sockaddr *myaddr,
					   const struct sockaddr *hisaddr,
					   char protLevel,
					   int gssBufferSize)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	char debugbuffer[FTP_BUFSIZ];
	sprintf(debugbuffer, "GFtpDoAuth(\"%s\", %d, %d, %d, %d[%d])", hostname, myaddr, hisaddr, protLevel, gssBufferSize, gssBufferSize);
	pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)debugbuffer, 0);

	if (pGssApiData->auth_type)
	{
		pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"Authorization Already Succeeded", 0);
		return pGssApiData->auth_type;
	}

	pGssApiData->nClientMode = 1;

	if (!protLevel)
		protLevel = 'P';

	if (!gssBufferSize)
		gssBufferSize = FTP_BUFSIZ;

	pGssApiData->clientAuthData.hostname = new char[strlen(hostname) + 1];
	strcpy(pGssApiData->clientAuthData.hostname, hostname);
	pGssApiData->clientAuthData.myaddr = myaddr;
	pGssApiData->clientAuthData.hisaddr = hisaddr;
	pGssApiData->clientAuthData.protLevel = protLevel;
	pGssApiData->clientAuthData.gssBufferSize = gssBufferSize;

	pGssApiData->nClientState = 0;


	pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 1, (int)"AUTH GSSAPI", 0);

	return -1;
}

DLLit int ProcessReply(void *pData, char *reply)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;

	int nReply = 5;

	TCHAR error[10000];
	error[0] = 0;

	if (strlen(reply) > 4)
	{
		nReply = reply[0]-'0';
	}

	switch (pGssApiData->nClientState)
	{
	case 0:
		if (nReply != CONTINUE)
		{
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"GSSAPI authentication failed", 0);
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"Server maybe is not Kerberos aware", 0);
			return SERVER_NOT_KERBEROS_AWARE;
		}
		else
		{
			char stbuf[FTP_BUFSIZ];
			char tempbuf[FTP_BUFSIZ];
			const char **service_name;
			int len;
			switch (pGssApiData->clientAuthData.myaddr->sa_family)
			{
			case AF_INET:
				{
					const struct sockaddr_in *sin = (const struct sockaddr_in *)pGssApiData->clientAuthData.myaddr;
					pGssApiData->clientAuthData.chan.initiator_addrtype = GSS_C_AF_INET;
					pGssApiData->clientAuthData.chan.initiator_address.length = 4;
					pGssApiData->clientAuthData.chan.initiator_address.value = const_cast<in_addr *>(&sin->sin_addr);
					pGssApiData->clientAuthData.chanBindings = true;
				}
				break;
			case AF_INET6:
				{
					pGssApiData->clientAuthData.chanBindings = false;
					break;
				}
				break;
			default:
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"Unknown initiator protocol type", 0);
				pGssApiData->clientAuthData.chanBindings = false;
			}
			switch (pGssApiData->clientAuthData.hisaddr->sa_family)
			{
			case AF_INET:
				{
					const struct sockaddr_in *sin = (const struct sockaddr_in *)pGssApiData->clientAuthData.hisaddr;
					pGssApiData->clientAuthData.chan.acceptor_addrtype = GSS_C_AF_INET;
					pGssApiData->clientAuthData.chan.acceptor_address.length = 4;
					pGssApiData->clientAuthData.chan.acceptor_address.value = const_cast<in_addr *>(&sin->sin_addr);
				}
				break;
			case AF_INET6:
				{
					pGssApiData->clientAuthData.chanBindings = false;
					break;
				}
			default:
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"Unknown acceptor protocol type", 0);
				pGssApiData->clientAuthData.chanBindings = false;
			}
			pGssApiData->clientAuthData.chan.application_data.length = 0;
			pGssApiData->clientAuthData.chan.application_data.value = 0;

			CLock lock(&critSection);

			// blob from gss-client
			for (service_name = gss_services; *service_name; service_name++)
			{
				// ftp@hostname first, the host@hostname
				// the V5 GSSAPI binding canonicalizes this for us...

				sprintf(stbuf, "%s@%s", *service_name, pGssApiData->clientAuthData.hostname);

				// Let the user know what server it's trying to authenticate
				sprintf(tempbuf, "Trying to authenticate to <%s>", stbuf);
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)tempbuf, 0);

				pGssApiData->clientAuthData.send_tok.value = stbuf;
				pGssApiData->clientAuthData.send_tok.length = strlen(stbuf);

				pGssApiData->clientAuthData.maj_stat = pGssApiData->gss_import_name(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.send_tok,
					*pGssApiData->gss_nt_service_name, &pGssApiData->clientAuthData.target_name);
				
				if (pGssApiData->clientAuthData.maj_stat != GSS_S_COMPLETE)
				{
					strcpy(error, "gss_import_name failed:");
					reply_gss_error(pGssApiData, pGssApiData->clientAuthData.maj_stat, pGssApiData->clientAuthData.min_stat, error);
					pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)error, 0);
					continue;
				}

				pGssApiData->clientAuthData.token_ptr = GSS_C_NO_BUFFER;
				pGssApiData->gcontext = GSS_C_NO_CONTEXT;

				pGssApiData->clientAuthData.maj_stat =
					pGssApiData->gss_init_sec_context(&pGssApiData->clientAuthData.min_stat,
													  GSS_C_NO_CREDENTIAL,
													  &pGssApiData->gcontext,
													  pGssApiData->clientAuthData.target_name,
													  GSS_C_NULL_OID,
													  GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG |
													  GSS_C_DELEG_FLAG,
													  0,
													  pGssApiData->clientAuthData.chanBindings ? &pGssApiData->clientAuthData.chan : 0,
													  pGssApiData->clientAuthData.token_ptr,
													  NULL,
													  &pGssApiData->clientAuthData.send_tok,
													  NULL,
													  NULL);
				
				if (pGssApiData->clientAuthData.maj_stat!=GSS_S_COMPLETE &&
					pGssApiData->clientAuthData.maj_stat!=GSS_S_CONTINUE_NEEDED)
				{
					// could just be that we missed on the service name
					sprintf(error, "Failed to authenticate to <%s@%s>:", *service_name, pGssApiData->clientAuthData.hostname);
					reply_gss_error(pGssApiData, pGssApiData->clientAuthData.maj_stat, pGssApiData->clientAuthData.min_stat, error);

					pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)error, 0);
					pGssApiData->gss_release_name(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.target_name);

					continue;
				}
			}
			if (pGssApiData->clientAuthData.maj_stat != GSS_S_COMPLETE && pGssApiData->clientAuthData.maj_stat != GSS_S_CONTINUE_NEEDED)
			{
				strcpy(error, "GSSAPI authentication failed:");
				reply_gss_error(pGssApiData, pGssApiData->clientAuthData.maj_stat, pGssApiData->clientAuthData.min_stat, error);
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)error, 0);
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"NOTE: Kerberos 5 tickets are REQUIRED for authentication.", 0);

				return KERBEROS_5_TICKETS_REQUIRED;
			}

			if (pGssApiData->clientAuthData.send_tok.length == 0)
			{
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"GSSAPI authentication failed, send_tok.length is zero", 0);
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"NOTE: Kerberos 5 tickets are REQUIRED for authentication.", 0);

				return KERBEROS_5_TICKETS_REQUIRED;
			}

			len = pGssApiData->clientAuthData.send_tok.length;

			// for command() later
			pGssApiData->kerror = radix_encode(pGssApiData->clientAuthData.send_tok.value, pGssApiData->clientAuthData.out_buf, &len, 0);
			if (strlen(pGssApiData->clientAuthData.out_buf)>=FTP_BUFSIZ)
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"Warning: strlen(pGssApiData->clientAuthData.out_buf)>=FTP_BUFSIZE", 0);
			if (pGssApiData->kerror)
			{
				sprintf(tempbuf, "Base 64 encoding failed: %s\0",
					(char*)radix_error(pGssApiData->kerror));
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)tempbuf, 0);

				return UNKNOWN_ERROR;
			}
			else
			{
				char sendme[FTP_BUFSIZ];
				sprintf(sendme, "ADAT %s", pGssApiData->clientAuthData.out_buf);

				pGssApiData->nClientState = 1;
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 1, (int)sendme, 0);
				return -1;
			}
		}
		break;
	case 1:
	  {
		CLock lock(&critSection);
		if (nReply != COMPLETE || strlen(reply)<9)
		{
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"GSSAPI ADAT failed", 0);
			pGssApiData->gss_release_buffer(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.send_tok);
			pGssApiData->gss_release_name(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.target_name);
			return UNKNOWN_ERROR;
		}

		reply += 9;

		if (!strlen(reply))
		{
			if (pGssApiData->clientAuthData.maj_stat == GSS_S_COMPLETE)
			{
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"No authentication data received from server, but no more was needed", 0);
				pGssApiData->gss_release_buffer(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.send_tok);
				pGssApiData->gss_release_name(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.target_name);
				break;
			}
			else
			{
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"ADAT failed: No authentication data received from server", 0);
				pGssApiData->gss_release_buffer(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.send_tok);
				pGssApiData->gss_release_name(&pGssApiData->clientAuthData.min_stat, &pGssApiData->clientAuthData.target_name);
				return UNKNOWN_ERROR;
			}
		}

		{
			int i;
			if (pGssApiData->kerror = radix_encode(reply,
				pGssApiData->clientAuthData.out_buf, &i, 1))
			{
				char tempbuf[FTP_BUFSIZ];
				sprintf(tempbuf, "Base 64 decoding failed:\
						%s\0",
						(char*) radix_error(pGssApiData->kerror));
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)tempbuf, 0);
				return UNKNOWN_ERROR;
			}

			// everything worked
			pGssApiData->clientAuthData.token_ptr = &pGssApiData->clientAuthData.recv_tok;
			pGssApiData->clientAuthData.recv_tok.value = pGssApiData->clientAuthData.out_buf;
			pGssApiData->clientAuthData.recv_tok.length = i;

			pGssApiData->clientAuthData.maj_stat =
						pGssApiData->gss_init_sec_context(&pGssApiData->clientAuthData.min_stat,
							GSS_C_NO_CREDENTIAL,
							&pGssApiData->gcontext,
							pGssApiData->clientAuthData.target_name,
							GSS_C_NULL_OID,
							GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG |
							GSS_C_DELEG_FLAG,
							0,
							pGssApiData->clientAuthData.chanBindings ? &pGssApiData->clientAuthData.chan : 0,
							pGssApiData->clientAuthData.token_ptr,
							NULL,
							&pGssApiData->clientAuthData.send_tok,
							NULL,
							NULL);
		}

		if (pGssApiData->clientAuthData.maj_stat == GSS_S_COMPLETE)
		{
			// Success!!
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"GSSAPI authentication succeeded", 0);
			pGssApiData->auth_type = 1;

			if (pGssApiData->clientAuthData.protLevel)
			{
				char buffer[FTP_BUFSIZ];
				char buffer2[FTP_BUFSIZ];
				unsigned long temp = 0;
				memset(buffer, 0 , FTP_BUFSIZ);
				sprintf(buffer, "PBSZ %u", pGssApiData->clientAuthData.gssBufferSize);
				strcpy(buffer2, buffer);
				EncryptMessage (pData, buffer, NULL);

				pGssApiData->nClientState = 2;
				pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 1, (int)buffer, (int)buffer2);

				return -1;
			}
			else
				pGssApiData->level = PROT_C;

			pGssApiData->ctrlevel = PROT_P;
			return GSSAPI_AUTHENTICATION_SUCCEEDED;
		}
		else
		{
			strcpy(error, "GSSAPI authentication failed:");
			reply_gss_error(pGssApiData, pGssApiData->clientAuthData.maj_stat, pGssApiData->clientAuthData.min_stat, error);
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)error, 0);
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"NOTE: Kerberos 5 tickets are REQUIRED for authentication.", 0);

			return KERBEROS_5_TICKETS_REQUIRED;
		}
		break;
	  }
	case 2:
		if (nReply != COMPLETE)
		{
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"GSSAPI PBSZ failed", 0);
			return UNKNOWN_ERROR;
		}

		if (strlen(reply) <= 9 || reply[8] != '=')
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"PBSZ reply did not contain a buffer size, using original value", 0);
		else
		{
			reply +=9;
			pGssApiData->clientAuthData.gssBufferSize = atoi(reply);
		}

		char buffer[FTP_BUFSIZ];
		char buffer2[FTP_BUFSIZ];

		memset(buffer, 0, FTP_BUFSIZ);
		sprintf(buffer, "PROT %c", pGssApiData->clientAuthData.protLevel);
		strcpy(buffer2, buffer);
		EncryptMessage (pData, buffer, NULL);

		pGssApiData->nClientState = 3;
		pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 1, (int)buffer, (int)buffer2);
		return -1;

		break;
	case 3:
		if (nReply != COMPLETE)
		{
			pGssApiData->pCallbackProc(pGssApiData->pCallbackData, 0, (int)"GSSAPI PROT failed", 0);
			return UNKNOWN_ERROR;
		}

		int temp;
		switch (pGssApiData->clientAuthData.protLevel)
		{
			case 'C':
				pGssApiData->level = PROT_C;
				pGssApiData->limitpbsz = 0;
				break;
			case 'S':
				pGssApiData->level = PROT_S;
				temp = SizeGssBuffer(pData, pGssApiData->clientAuthData.gssBufferSize);
				pGssApiData->realpbsz = pGssApiData->clientAuthData.gssBufferSize;
				pGssApiData->limitpbsz = temp - 8;
				pGssApiData->clientAuthData.gssBufferSize = temp - 8;
				break;
			case 'P'	:
				pGssApiData->level = PROT_P;

				temp = SizeGssBuffer(pData, pGssApiData->clientAuthData.gssBufferSize);
				pGssApiData->realpbsz = pGssApiData->clientAuthData.gssBufferSize;
				pGssApiData->limitpbsz = temp - 8;
				pGssApiData->clientAuthData.gssBufferSize = temp - 8;
				break;
		}
		return GSSAPI_AUTHENTICATION_SUCCEEDED;
		break;
	}
	return UNKNOWN_ERROR;
}

DLLit BOOL GetUserFromKrbTicket(void *pData, char *user)
{
	t_GSS_API_data *pGssApiData=*(t_GSS_API_data **)pData;
	
    char *PrincipalName = NULL;
    LPSTR puserName = NULL;
    
	krb5_ccache	KRBv5Cache = NULL;
	krb5_principal KRBv5Principal = NULL;
	
	int nError = 0;

	if (!pGssApiData->kcontext)
		return FALSE;
    
	critSection.Lock();
	nError = pGssApiData->krb5_cc_default(pGssApiData->kcontext, &KRBv5Cache);
	critSection.Unlock();
	if (KRBv5Cache)
	{
		critSection.Lock();
		krb5_cc_get_principal(pGssApiData->kcontext, KRBv5Cache, &KRBv5Principal);
		critSection.Unlock();
		if (KRBv5Principal)
		{
			critSection.Lock();
			pGssApiData->krb5_unparse_name(pGssApiData->kcontext, KRBv5Principal, (char **)&PrincipalName);
			critSection.Unlock();
			if (PrincipalName)
			{
				puserName = strchr(PrincipalName, '@');
				*puserName = 0;
				strncpy(user, PrincipalName, 255);
				user[255] = 0;
				critSection.Lock();
				pGssApiData->krb5_free_unparsed_name(pGssApiData->kcontext, PrincipalName);
				critSection.Unlock();
			}
			critSection.Lock();
			pGssApiData->krb5_free_principal(pGssApiData->kcontext, KRBv5Principal);
			critSection.Unlock();
		}
		// krb5_cc_destroy(pGssApiData->kcontext, KRBv5Cache);
		// Don't destroy default credentials cache
	}
	    
	if(!strcmp(user, ""))
    {
        if( NULL != getenv( "USERNAME" ) )
		{
			strncpy(user, getenv("USERNAME"), 255);
			user[255] = 0;
		}
    }
    
	return strcmp(user, "");
}