#include "or.h"
#include "dlg_util.h"

HWND hDlgBridges=NULL;
//int frame12[]={24010,24400,24401,24002,24050,24100,24402,24403,24404,24405,24101,24406,24102,24407,24103,24408,24104,-1};
extern or_options_t *tmpOptions;
lang_dlg_info lang_dlg_bridges[]={
	{24010,LANG_BRIDGES_MSG_ENABLE},
	{24400,LANG_DLG_USE_BRIDGES},
	{24401,LANG_DLG_UPDATE_BRIDGES},
	{24002,LANG_BRIDGES_GET_NEW},
	{24050,LANG_BRIDGES_USE_BRIDGES},
	{24402,LANG_BRIDGES_TUNNEL_DIR_CONNS},
	{24403,LANG_BRIDGES_AVOID_DIRS},
	{24404,LANG_DLG_DIR_PRIVATE},
	{24405,LANG_BRIDGES_HTTP_PROXY},
	{24406,LANG_BRIDGES_PROXY_ACCOUNT},
	{24407,LANG_BRIDGES_HTTPS_PROXY},
	{24408,LANG_BRIDGES_PROXY_ACCOUNT},
	{0,0}
};


#define INTERNET_FLAG_NO_CACHE_WRITE 0x04000000
#define INTERNET_FLAG_NO_COOKIES 0x80000
typedef PVOID HINTERNET;
typedef HINTERNET (WINAPI *LPInternetOpenA)(LPCSTR,DWORD,LPCSTR,LPCSTR,DWORD);
typedef HINTERNET (WINAPI *LPInternetOpenUrlA)(HINTERNET,LPCSTR,LPCSTR,DWORD,DWORD,DWORD);
typedef BOOL (WINAPI *LPInternetReadFile)(HINTERNET,PVOID,DWORD,PDWORD);
typedef BOOL (WINAPI *LPInternetCloseHandle)(HINTERNET);

LPInternetOpenA _InternetOpen=NULL;
LPInternetOpenUrlA _InternetOpenUrl=NULL;
LPInternetReadFile _InternetReadFile=NULL;
LPInternetCloseHandle _InternetCloseHandle=NULL;
HINSTANCE hWininet=NULL;

int getNewBridges(HWND hDlg)
{	char *bridges_tmp=tor_malloc(16384);
	char *tmp1,*tmp2,*tmp3;
	HINTERNET hInternet,hUrl;
	DWORD bytesRead=0;
	if(_InternetOpen==NULL)
	{	GetSystemDirectory(bridges_tmp,8192);
		tor_snprintf(bridges_tmp+8192,8192,"%s\\wininet.dll",bridges_tmp);
		hWininet=LoadLibrary(bridges_tmp+8192);
		if(hWininet==NULL)
		{	LangMessageBox(hDlg,get_lang_str(LANG_BRIDGES_WININET),LANG_MB_ERROR,MB_OK);
			tor_free(bridges_tmp);return 0;
		}
		_InternetOpen=(LPInternetOpenA)GetProcAddress(hWininet,"InternetOpenA");
		_InternetOpenUrl=(LPInternetOpenUrlA)GetProcAddress(hWininet,"InternetOpenUrlA");
		_InternetReadFile=(LPInternetReadFile)GetProcAddress(hWininet,"InternetReadFile");
		_InternetCloseHandle=(LPInternetCloseHandle)GetProcAddress(hWininet,"InternetCloseHandle");
		if(!(_InternetOpen && _InternetOpenUrl && _InternetReadFile && _InternetCloseHandle))
		{	LangMessageBox(hDlg,get_lang_str(LANG_BRIDGES_WININET),LANG_MB_ERROR,MB_OK);
			tor_free(bridges_tmp);return 0;
		}
	}
	hInternet=_InternetOpen("",0,NULL,NULL,0);
	hUrl=_InternetOpenUrl(hInternet,"https://bridges.torproject.org",NULL,0,INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_COOKIES,0);
	_InternetReadFile(hUrl,bridges_tmp,16384,&bytesRead);
	_InternetCloseHandle(hUrl);
	_InternetCloseHandle(hInternet);
	if(bytesRead)
	{	bridges_tmp[bytesRead]=0;
		tmp1=tmp2=bridges_tmp;
		while(*tmp2)
		{
			if(*tmp2=='<')
			{	if(((tmp2[1]|0x20)=='p') && ((tmp2[2]<33) || (tmp2[2]=='>'))){	*tmp1++=13;*tmp1++=10;}
				else if(((tmp2[1]|0x20)=='b') && ((tmp2[2]|0x20)=='r') && ((tmp2[3]<33) || (tmp2[3]=='>'))){	*tmp1++=13;*tmp1++=10;}
				while((*tmp2!='>')&&(*tmp2!=0))	tmp2++;
				if(*tmp2=='>')	tmp2++;
			}
			else if(*tmp2=='&')
			{	if(!strcmpstart(tmp2,"amp;")){	*tmp1++='&';tmp2+=5;}
				else if(!strcmpstart(tmp2,"lt;")){	*tmp1++='<';tmp2+=4;}
				else if(!strcmpstart(tmp2,"gt;")){	*tmp1++='>';tmp2+=4;}
				else if(!strcmpstart(tmp2,"quot;")){	*tmp1++=34;tmp2+=6;}
				else if(!strcmpstart(tmp2,"apos;")){	*tmp1++=39;tmp2+=6;}
				else	*tmp1++=*tmp2++;
			}
			else	*tmp1++=*tmp2++;
		}
		*tmp1=0;
		tmp1=tmp2=bridges_tmp;
		while(*tmp1)
		{	if((*tmp1>='0')&&(*tmp1<='9'))
			{	if(is_ip(tmp1))
				{	while(((*tmp1>='0')&&(*tmp1<='9'))||(*tmp1=='.')||(*tmp1==':'))	*tmp2++=*tmp1++;
					*tmp2++=13;*tmp2++=10;
				}
				else	while((*tmp1>='0')&&(*tmp1<='9')) tmp1++;
			}
			else	tmp1++;
		}
		if(bridges_tmp==tmp2){	tor_free(bridges_tmp);return 0;}
		*tmp2=0;
		if(tmp2-bridges_tmp<8192)
		{	tor_snprintf(bridges_tmp+8192,8192,get_lang_str(LANG_BRIDGES_DOWNLOAD_OK),bridges_tmp);
			LangMessageBox(hDlg,bridges_tmp+8192,LANG_BRIDGES_DOWNLOAD,MB_OK);
		}
		bytesRead=SendDlgItemMessage(hDlg,24100,WM_GETTEXTLENGTH,0,0);
		tmp2=tor_malloc(bytesRead+strlen(bridges_tmp)+5);tmp3=tmp2;
		if(bytesRead>32000) bytesRead=32000;
		GetDlgItemText(hDlg,24100,tmp2,bytesRead+1);tmp2+=bytesRead;
		if(bytesRead && *(tmp2-1)>31){	*tmp2++=13;*tmp2++=10;}
		tmp1=bridges_tmp;
		for(;*tmp1!=0;)	*tmp2++=*tmp1++;
		*tmp2=0;
		tmp2=tor_malloc(65535);
		tmp2=SortIPList(tmp3,tmp2);
		SetDlgItemText(hDlg,24100,tmp2);
		tor_free(tmp2);
		tor_free(tmp3);
		tor_free(bridges_tmp);
		return 1;
	}
	else	LangMessageBox(hDlg,get_lang_str(LANG_BRIDGES_NO_BRIDGES),LANG_BRIDGES_DOWNLOAD,MB_OK);
	tor_free(bridges_tmp);
	return 0;
}

void setEditBridges(config_line_t **option)
{	int i,j;
	if(*option!=NULL)
	{	char *tmp1=tor_malloc(65536),*tmp2;i=0;tmp2=tmp1;
		config_line_t *cfg;
		for(cfg=*option;cfg;cfg=cfg->next)
		{	for(j=0;i<32000;i++,j++)
			{	if(!cfg->value[j]) break;
				*tmp1++=cfg->value[j];
			}
			*tmp1++=13;*tmp1++=10;i+=2;
			if(i>32000) break;
		}
		*tmp1=0;
		tmp1=tor_malloc(65535);
		tmp1=SortIPList(tmp2,tmp1);
		SetDlgItemText(hDlgBridges,24100,tmp1);
		tor_free(tmp1);
		tor_free(tmp2);
	}
}

int __stdcall dlgBridges(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgBridges=hDlg;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_bridges);
		}
		if(tmpOptions->UseBridges){ CheckDlgButton(hDlg,24400,BST_CHECKED);}
		else
		{	EnableWindow(GetDlgItem(hDlg,24401),0);
			EnableWindow(GetDlgItem(hDlg,24002),0);
			EnableWindow(GetDlgItem(hDlg,24050),0);
			EnableWindow(GetDlgItem(hDlg,24100),0);
		}
		if(tmpOptions->UpdateBridgesFromAuthority){ CheckDlgButton(hDlg,24401,BST_CHECKED);}
		if(tmpOptions->TunnelDirConns)	CheckDlgButton(hDlg,24402,BST_CHECKED);
		else	EnableWindow(GetDlgItem(hDlg,24403),0);
		if(tmpOptions->PreferTunneledDirConns)	CheckDlgButton(hDlg,24403,BST_CHECKED);
		if(tmpOptions->AllDirActionsPrivate) CheckDlgButton(hDlg,24404,BST_CHECKED);
		SendDlgItemMessage(hDlg,24102,EM_LIMITTEXT,48,0);
		SendDlgItemMessage(hDlg,24104,EM_LIMITTEXT,48,0);
		if(tmpOptions->HttpProxy!=NULL)
		{	CheckDlgButton(hDlg,24405,BST_CHECKED);
			SetDlgItemText(hDlg,24101,tmpOptions->HttpProxy);
		}
		else
		{	EnableWindow(GetDlgItem(hDlg,24101),0);
			EnableWindow(GetDlgItem(hDlg,24406),0);
			EnableWindow(GetDlgItem(hDlg,24102),0);
		}
		if(tmpOptions->HttpProxyAuthenticator!=NULL){ CheckDlgButton(hDlg,24406,BST_CHECKED);SetDlgItemText(hDlg,24102,tmpOptions->HttpProxyAuthenticator);}
		else	EnableWindow(GetDlgItem(hDlg,24102),0);
		if(tmpOptions->HttpsProxy!=NULL)
		{	CheckDlgButton(hDlg,24407,BST_CHECKED);
			SetDlgItemText(hDlg,24103,tmpOptions->HttpsProxy);
		}
		else
		{	EnableWindow(GetDlgItem(hDlg,24103),0);
			EnableWindow(GetDlgItem(hDlg,24408),0);
			EnableWindow(GetDlgItem(hDlg,24104),0);
		}
		if(tmpOptions->HttpsProxyAuthenticator!=NULL){ CheckDlgButton(hDlg,24408,BST_CHECKED);SetDlgItemText(hDlg,24104,tmpOptions->HttpsProxyAuthenticator);}
		else	EnableWindow(GetDlgItem(hDlg,24104),0);
		setEditBridges(&tmpOptions->Bridges);
	}
	else if(uMsg==WM_COMMAND)
	{
		if(LOWORD(wParam)==24400)
		{	if(IsDlgButtonChecked(hDlg,24400)==BST_CHECKED)
			{	tmpOptions->UseBridges=1;
				EnableWindow(GetDlgItem(hDlg,24401),1);
				EnableWindow(GetDlgItem(hDlg,24002),1);
				EnableWindow(GetDlgItem(hDlg,24050),1);
				EnableWindow(GetDlgItem(hDlg,24100),1);
			}
			else
			{	tmpOptions->UseBridges=0;
				EnableWindow(GetDlgItem(hDlg,24401),0);
				EnableWindow(GetDlgItem(hDlg,24002),0);
				EnableWindow(GetDlgItem(hDlg,24050),0);
				EnableWindow(GetDlgItem(hDlg,24100),0);
			}
		}
		else if(LOWORD(wParam)==24401)
		{	if(IsDlgButtonChecked(hDlg,24401)==BST_CHECKED)	tmpOptions->UpdateBridgesFromAuthority=1;
			else	tmpOptions->UpdateBridgesFromAuthority=0;
		}
		else if(LOWORD(wParam)==24002)
		{	if(getNewBridges(hDlg))
			{	getEditData1(hDlg,24100,&tmpOptions->Bridges,"Bridges");
				clear_bridge_list();
				config_line_t *cfg;
				for(cfg=tmpOptions->Bridges;cfg;cfg=cfg->next)	parse_bridge_line(cfg->value,0);
			}
		}
		else if((LOWORD(wParam)==24100)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData1(hDlg,24100,&tmpOptions->Bridges,"Bridges");
			clear_bridge_list();
			config_line_t *cfg;
			for(cfg=tmpOptions->Bridges;cfg;cfg=cfg->next)	parse_bridge_line(cfg->value,0);
		}
		else if(LOWORD(wParam)==24402)
		{	if(IsDlgButtonChecked(hDlg,24402)==BST_CHECKED)
			{	tmpOptions->TunnelDirConns=1;
				EnableWindow(GetDlgItem(hDlg,24403),1);
				if(IsDlgButtonChecked(hDlg,24403)==BST_CHECKED) tmpOptions->PreferTunneledDirConns=1;
			}
			else
			{	tmpOptions->TunnelDirConns=0;
				tmpOptions->PreferTunneledDirConns=0;
				EnableWindow(GetDlgItem(hDlg,24403),0);
			}
		}
		else if(LOWORD(wParam)==24403)
		{	if(IsDlgButtonChecked(hDlg,24403)==BST_CHECKED)	tmpOptions->PreferTunneledDirConns=1;
			else	tmpOptions->PreferTunneledDirConns=0;
		}
		else if((LOWORD(wParam)==24101)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,24101,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,24101,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->HttpProxy;
			tmpOptions->HttpProxy=tmp1;tor_free(tmp2);
			parse_addr_port(LOG_WARN,tmpOptions->HttpProxy,NULL,&tmpOptions->HttpProxyAddr,&tmpOptions->HttpProxyPort);
			if(tmpOptions->HttpProxyPort==0)	tmpOptions->HttpProxyPort=80;
		}
		else if((LOWORD(wParam)==24102)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,24102,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,24102,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->HttpProxyAuthenticator;
			tmpOptions->HttpProxyAuthenticator=tmp1;tor_free(tmp2);
		}
		else if((LOWORD(wParam)==24103)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,24103,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,24103,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->HttpsProxy;
			tmpOptions->HttpsProxy=tmp1;tor_free(tmp2);
			parse_addr_port(LOG_WARN,tmpOptions->HttpsProxy,NULL,&tmpOptions->HttpsProxyAddr,&tmpOptions->HttpsProxyPort);
			if(tmpOptions->HttpsProxyPort==0)	tmpOptions->HttpsProxyPort=443;
		}
		else if((LOWORD(wParam)==24104)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,24104,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,24104,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->HttpsProxyAuthenticator;
			tmpOptions->HttpsProxyAuthenticator=tmp1;tor_free(tmp2);
		}
		else if(LOWORD(wParam)==24404)
		{	if(IsDlgButtonChecked(hDlg,24404)==BST_CHECKED){	tmpOptions->AllDirActionsPrivate|=1;}
			else{	tmpOptions->AllDirActionsPrivate=0;}
		}
		else if(LOWORD(wParam)==24405)
		{	if(IsDlgButtonChecked(hDlg,24405)==BST_CHECKED)
			{	int tmpsize=SendDlgItemMessage(hDlg,24101,WM_GETTEXTLENGTH,0,0);
				char *tmp1=tor_malloc(tmpsize+2);
				SendDlgItemMessage(hDlg,24101,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
				char *tmp2=tmpOptions->HttpProxy;
				tmpOptions->HttpProxy=tmp1;tor_free(tmp2);
				EnableWindow(GetDlgItem(hDlg,24101),1);
				EnableWindow(GetDlgItem(hDlg,24406),1);
				if(tmpOptions->HttpProxyAuthenticator)	EnableWindow(GetDlgItem(hDlg,24102),1);
			}
			else
			{	char *tmp1=tmpOptions->HttpProxy;
				tmpOptions->HttpProxy=NULL;tor_free(tmp1);
				EnableWindow(GetDlgItem(hDlg,24101),0);
				EnableWindow(GetDlgItem(hDlg,24406),0);
				EnableWindow(GetDlgItem(hDlg,24102),0);
			}
		}
		else if(LOWORD(wParam)==24406)
		{	if(IsDlgButtonChecked(hDlg,24406)==BST_CHECKED)
			{	int tmpsize=SendDlgItemMessage(hDlg,24102,WM_GETTEXTLENGTH,0,0);
				char *tmp1=tor_malloc(tmpsize+2);
				SendDlgItemMessage(hDlg,24102,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
				char *tmp2=tmpOptions->HttpProxyAuthenticator;
				tmpOptions->HttpProxyAuthenticator=tmp1;tor_free(tmp2);
				EnableWindow(GetDlgItem(hDlg,24102),1);
			}
			else
			{	char *tmp1=tmpOptions->HttpProxyAuthenticator;
				tmpOptions->HttpProxyAuthenticator=NULL;tor_free(tmp1);
				EnableWindow(GetDlgItem(hDlg,24102),0);
			}
		}
		else if(LOWORD(wParam)==24407)
		{	if(IsDlgButtonChecked(hDlg,24407)==BST_CHECKED)
			{	int tmpsize=SendDlgItemMessage(hDlg,24103,WM_GETTEXTLENGTH,0,0);
				char *tmp1=tor_malloc(tmpsize+2);
				SendDlgItemMessage(hDlg,24103,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
				char *tmp2=tmpOptions->HttpsProxy;
				tmpOptions->HttpsProxy=tmp1;tor_free(tmp2);
				EnableWindow(GetDlgItem(hDlg,24103),1);
				EnableWindow(GetDlgItem(hDlg,24408),1);
				if(tmpOptions->HttpsProxyAuthenticator)	EnableWindow(GetDlgItem(hDlg,24104),1);
			}
			else
			{	char *tmp1=tmpOptions->HttpsProxy;
				tmpOptions->HttpsProxy=NULL;tor_free(tmp1);
				EnableWindow(GetDlgItem(hDlg,24103),0);
				EnableWindow(GetDlgItem(hDlg,24408),0);
				EnableWindow(GetDlgItem(hDlg,24104),0);
			}
		}
		else if(LOWORD(wParam)==24408)
		{	if(IsDlgButtonChecked(hDlg,24408)==BST_CHECKED)
			{	int tmpsize=SendDlgItemMessage(hDlg,24104,WM_GETTEXTLENGTH,0,0);
				char *tmp1=tor_malloc(tmpsize+2);
				SendDlgItemMessage(hDlg,24104,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
				char *tmp2=tmpOptions->HttpsProxyAuthenticator;
				tmpOptions->HttpsProxyAuthenticator=tmp1;tor_free(tmp2);
				EnableWindow(GetDlgItem(hDlg,24104),1);
			}
			else
			{	char *tmp1=tmpOptions->HttpsProxyAuthenticator;
				tmpOptions->HttpsProxyAuthenticator=NULL;tor_free(tmp1);
				EnableWindow(GetDlgItem(hDlg,24104),0);
			}
		}
	}
	return 0;
}
