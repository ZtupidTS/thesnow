#include "or.h"
#include "dlg_util.h"

HWND hDlgProxy=NULL;
extern HWND hMainDialog;
extern or_options_t *tmpOptions;
//int frame5[]={13400,13100,13010,13101,13401,13102,13402,13103,13403,13404,13405,13104,13406,13105,13050,-1};
lang_dlg_info lang_dlg_proxy[]={
	{13400,LANG_DLG_LOCAL_PROXY_PORT},
	{13010,LANG_DLG_LOCAL_PROXY_ADDR},
	{13401,LANG_DLG_IP_RESTRICTIONS},
	{13402,LANG_DLG_HANDSHAKE_TIMEOUT},
	{13403,LANG_DLG_DISALLOW_DNS_RESOLVE},
	{13404,LANG_DLG_ALLOW_NON_RFC953},
	{13050,LANG_DLG_DISALLOWED_CONNECTIONS},
	{13405,LANG_DLG_BANNED_PORTS},
	{13406,LANG_DLG_BANNED_HOSTS},
	{0,0}
};

BOOL is_banned(const char *_addr)
{	if(tmpOptions->BannedHosts)
	{	config_line_t *cfg;
		for(cfg=tmpOptions->BannedHosts;cfg;cfg=cfg->next)
		{	if(!stricmp(cfg->value,_addr)) return 1;
		}
	}
	return 0;
}


void dlgProxy_banSocksAddress(char *socksAddress)
{	if(!socksAddress[0]) return;
	int tmpsize=SendDlgItemMessage(hDlgProxy,13105,WM_GETTEXTLENGTH,0,0);
	int i;
	char *tmp2=tor_malloc(tmpsize+256+5),*tmp3;
	tmp3=tmp2;
	GetDlgItemText(hDlgProxy,13105,tmp2,tmpsize+1);tmp2+=tmpsize;
	if(tmpsize && (tmp3[tmpsize-1]!=13) && (tmp3[tmpsize-1]!=10)){	*tmp2++=13;*tmp2++=10;}
	for(i=0;(socksAddress[i]!=0)&&(socksAddress[i]!=':');i++)	*tmp2++=socksAddress[i];
	*tmp2++=13;*tmp2++=10;*tmp2++=0;
	SetDlgItemText(hDlgProxy,13105,tmp3);
	getEditData(hDlgProxy,13105,&tmpOptions->BannedHosts,"BannedHosts");
	tor_snprintf(tmp3,256,get_lang_str(LANG_MB_BAN_ADDED),&socksAddress[0]);
	LangMessageBox(hMainDialog,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	tor_free(tmp3);
}

void dlgProxy_banDebugAddress(char *strban)
{	if(strban[0])
	{	int i,tmpsize=SendDlgItemMessage(hDlgProxy,13105,WM_GETTEXTLENGTH,0,0);
		char *tmp2,*tmp3;
		tmp2=tor_malloc(tmpsize+256+5);tmp3=tmp2;
		GetDlgItemText(hDlgProxy,13105,tmp2,tmpsize+1);tmp2+=tmpsize;
		for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
		for(;i;i--) if(strban[i]<33) break;
		if(i&&strban[i]<33) i++;
		for(;(strban[i]!=0)&&(strban[i]!=':');i++)	*tmp2++=strban[i];
		*tmp2++=13;*tmp2++=10;*tmp2++=0;
		SetDlgItemText(hDlgProxy,13105,tmp3);
		getEditData(hDlgProxy,13105,&tmpOptions->BannedHosts,"BannedHosts");
		tor_free(tmp3);
	}
}

int __stdcall dlgProxy(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgProxy=hDlg;
		char *tmp1,*tmp2;
		int i,j;
		config_line_t *cfg;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_proxy);
		}
		if(tmpOptions->SocksPort){ CheckDlgButton(hDlg,13400,BST_CHECKED);SetDlgItemInt(hDlg,13100,tmpOptions->SocksPort,0);}
		else{	EnableWindow(GetDlgItem(hDlg,13100),0);EnableWindow(GetDlgItem(hDlg,13010),0);EnableWindow(GetDlgItem(hDlg,13101),0);}
		if(tmpOptions->SocksListenAddress)	SetDlgItemText(hDlg,13101,tmpOptions->SocksListenAddress->value);
		else SetDlgItemText(hDlg,13101,"127.0.0.1");
		if(tmpOptions->SocksPolicy){	setEditData(hDlg,13102,tmpOptions->SocksPolicy);CheckDlgButton(hDlg,13401,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,13102),0);
		SendDlgItemMessage(hDlg,13102,EM_LIMITTEXT,65536,0);
		if(tmpOptions->SocksTimeout>=32767) EnableWindow(GetDlgItem(hDlg,13103),0);
		else{	CheckDlgButton(hDlg,13402,BST_CHECKED);SetDlgItemInt(hDlg,13103,tmpOptions->SocksTimeout,0);}
		if(tmpOptions->RejectPlaintextPorts)
		{	tmp1=smartlist_join_strings(tmpOptions->RejectPlaintextPorts, ",", 0, NULL);
			SetDlgItemText(hDlg,13104,tmp1);
			tor_free(tmp1);}
		if(tmpOptions->SafeSocks)		CheckDlgButton(hDlg,13403,BST_CHECKED);
		if(tmpOptions->AllowNonRFC953Hostnames) CheckDlgButton(hDlg,13404,BST_CHECKED);
		SendDlgItemMessage(hDlg,13105,EM_LIMITTEXT,65536,0);
		if(tmpOptions->BannedHosts!=NULL)
		{	tmp1=tor_malloc(65536);i=0;tmp2=tmp1;
			for(cfg=tmpOptions->BannedHosts;cfg;cfg=cfg->next)
			{	for(j=0;i<65530;i++,j++)
				{	if(!cfg->value[j]) break;
					*tmp1++=cfg->value[j];
				}
				*tmp1++=13;*tmp1++=10;i+=2;
				if(i>65530) break;
			}
			*tmp1=0;
			SetDlgItemText(hDlg,13105,tmp2);
			tor_free(tmp2);
		}
	}
	else if(uMsg==WM_COMMAND)
	{	if(LOWORD(wParam)==13400)
		{	if(IsDlgButtonChecked(hDlg,13400)==BST_CHECKED)
			{	tmpOptions->SocksPort=GetDlgItemInt(hDlg,13100,0,0);
				EnableWindow(GetDlgItem(hDlg,13100),1);EnableWindow(GetDlgItem(hDlg,13010),1);EnableWindow(GetDlgItem(hDlg,13101),1);
				retry_all_listeners(0,0);
			}
			else
			{	tmpOptions->SocksPort=0; EnableWindow(GetDlgItem(hDlg,13100),0);EnableWindow(GetDlgItem(hDlg,13010),0);EnableWindow(GetDlgItem(hDlg,13101),0);
				retry_all_listeners(0,0);
			}
		}
		else if((LOWORD(wParam)==13100)&&(HIWORD(wParam)==EN_CHANGE))
		{	tmpOptions->SocksPort=GetDlgItemInt(hDlg,13100,0,0);
			retry_all_listeners(0,0);
		}
		else if((LOWORD(wParam)==13101)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData(hDlg,13101,&tmpOptions->SocksListenAddress,"SocksListenAddress");
			retry_all_listeners(0,0);
		}
		else if(LOWORD(wParam)==13401)
		{	if(IsDlgButtonChecked(hDlg,13401)==BST_CHECKED)
			{	EnableWindow(GetDlgItem(hDlg,13102),1);getEditData1(hDlg,13102,&tmpOptions->SocksPolicy,"SocksPolicy");
			}
			else
			{	EnableWindow(GetDlgItem(hDlg,13102),0);
				config_line_t *cfg;
				for(;;)
				{	cfg=tmpOptions->SocksPolicy;
					if(cfg==NULL) break;
					tor_free(cfg->key);tor_free(cfg->value);
					tmpOptions->SocksPolicy=cfg->next;
					tor_free(cfg);
				}
			}
			policies_parse_from_options(tmpOptions);
		}
		else if((LOWORD(wParam)==13102)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData1(hDlg,13102,&tmpOptions->SocksPolicy,"SocksPolicy");
			policies_parse_from_options(tmpOptions);
		}
		else if(LOWORD(wParam)==13402)
		{	if(IsDlgButtonChecked(hDlg,13402)==BST_CHECKED)
			{	tmpOptions->SocksTimeout=GetDlgItemInt(hDlg,13103,0,0);EnableWindow(GetDlgItem(hDlg,13103),1);
			}
			else
			{	tmpOptions->SocksTimeout=32767;EnableWindow(GetDlgItem(hDlg,13103),0);
			}
		}
		else if((LOWORD(wParam)==13103)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->SocksTimeout=GetDlgItemInt(hDlg,13103,0,0);
		else if(LOWORD(wParam)==13403)
		{	if(IsDlgButtonChecked(hDlg,13403)==BST_CHECKED)	tmpOptions->SafeSocks=1;
			else	tmpOptions->SafeSocks=0;
		}
		else if(LOWORD(wParam)==13404)
		{	if(IsDlgButtonChecked(hDlg,13404)==BST_CHECKED)	tmpOptions->AllowNonRFC953Hostnames=1;
			else	tmpOptions->AllowNonRFC953Hostnames=0;
		}
		else if((LOWORD(wParam)==13104)&&(HIWORD(wParam)==EN_CHANGE))
		{	char *tmp1=tor_malloc(32768);
			GetDlgItemText(hDlg,13104,tmp1,32767);
			if(tmpOptions->RejectPlaintextPorts)
			{	SMARTLIST_FOREACH(tmpOptions->RejectPlaintextPorts, char *, cp, tor_free(cp));
				smartlist_clear(tmpOptions->RejectPlaintextPorts);
			}
			else	tmpOptions->RejectPlaintextPorts=smartlist_create();
			smartlist_split_string(tmpOptions->RejectPlaintextPorts, tmp1, ",",SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
			tor_free(tmp1);
		}
		else if((LOWORD(wParam)==13105)&&(HIWORD(wParam)==EN_CHANGE))
			getEditData(hDlg,13105,&tmpOptions->BannedHosts,"BannedHosts");
	}
	return	0;
}
