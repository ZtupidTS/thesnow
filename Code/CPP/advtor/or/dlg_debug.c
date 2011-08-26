#include "or.h"
#include "dlg_util.h"

HWND hDlgDebug=NULL;
WNDPROC oldEditProc;
extern HWND hMainDialog;
extern or_options_t *tmpOptions;
char strban[256+3];
extern char exename[MAX_PATH+1];
extern HINSTANCE hInstance;

//int frame2[]={400,401,10,300,100,-1};
lang_dlg_info lang_dlg_debug[]={
	{400,LANG_DLG_SAVE_TO_LOG},
	{401,LANG_DLG_AUTO_REFRESH},
	{11,LANG_DLG_RESERVED_05},
	{10,LANG_DLG_CLEAR_WINDOW},
	{0,0}
};

lang_dlg_info lang_dlg_filters[]={
	{11050,LANG_DLG_DEBUG_FILTER},
	{0,0}
};

BOOL is_dns_letter(char c)
{	if((c<='Z')&&(c>='A'))	return 1;
	if((c<='z')&&(c>='a'))	return 1;
	if((c<='9')&&(c>='0'))	return 1;
	if(c=='-')	return 1;
	return 0;
}

BOOL check_ip(char *addr)
{	char *addr_c;
	addr_c=addr;
	while(*addr_c)
	{	if(*addr_c==':') return 1;
		if(*addr_c>'9') return 0;
		addr_c++;
	}
	return 1;
}

void dlgDebug_logFilterAdd(char *strban)
{	if(strban[0])
	{	int i,j,k=0;
		char *tmp1=getLogFilter();for(j=0;tmp1&&*(tmp1+j);j++);
		char *tmp2=tor_malloc(j+256+5),*tmp3;tmp3=tmp2;
		if(tmp1)
		{	for(i=0;*tmp1;i++){	k=*tmp1;*tmp2++=*tmp1++;}
			if((k!=13)&&(k!=10)){	*tmp2++=13;*tmp2++=10;}
		}
		for(i=0;strban[i];i++)	*tmp2++=strban[i];
		*tmp2++=13;*tmp2++=10;*tmp2++=0;
		setLogFilter(tmp3);set_log_filter();strban[0]=0;
	}
}

void dlgDebug_setLogFilter(or_options_t *options)
{	if(options->NotifyFilter!=NULL)
	{	char *tmp1=tor_malloc(65536),*tmp2;
		int i=0,j;tmp2=tmp1;
		config_line_t *cfg;
		for(cfg=options->NotifyFilter;cfg;cfg=cfg->next)
		{	for(j=0;i<65530;i++,j++)
			{	if(!cfg->value[j]) break;
				*tmp1++=cfg->value[j];
			}
			*tmp1++=13;*tmp1++=10;i+=2;
			if(i>65530) break;
		}
		*tmp1=0;
		setLogFilter(tmp2);
	}
}

void set_log_filter(void)
{	int i,j,k;
	if(!(tmpOptions)) return;
	char *tmp1=getLogFilter();
	if(!tmp1) return;
	config_line_t *cfg,**cfg1;
	while(tmpOptions->NotifyFilter)
	{	cfg=tmpOptions->NotifyFilter;
		tor_free(cfg->key);tor_free(cfg->value);
		tmpOptions->NotifyFilter=cfg->next;
		tor_free(cfg);
	}
	j=0;cfg1=&tmpOptions->NotifyFilter;
	int tmpsize=strlen(tmp1)+1;
	for(i=0;i<=tmpsize;i++)
	{	if((tmp1[i]==13)||(tmp1[i]==10)||(tmp1[i]==0))
		{	if(j!=i)
			{	k=tmp1[i];tmp1[i]=0;
				*cfg1=tor_malloc_zero(sizeof(config_line_t));
				(*cfg1)->key = tor_strdup("NotifyFilter");
				(*cfg1)->value=tor_strdup(&tmp1[j]);
				cfg1=&((*cfg1)->next);
				tmp1[i]=k;
			}
			while((tmp1[i]==13)||(tmp1[i]==10)) i++;
			j=i;
		}
	}
	tmp1=NULL;
}

int __stdcall dlgFilters(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	if(LangGetLanguage())
		{	SetWindowTextL(hDlg,LANG_FILTERS_TITLE);
			changeDialogStrings(hDlg,lang_dlg_filters);
		}
		SendDlgItemMessage(hDlg,11104,EM_LIMITTEXT,65536,0);
		SetDlgItemTextL(hDlg,11104,getLogFilter()?getLogFilter():"");
	}
	else if(uMsg==WM_COMMAND)
	{	if(LOWORD(wParam)==2)	EndDialog(hDlg,0);
		else if(LOWORD(wParam)==1)
		{	int tmpsize=SendDlgItemMessage(hDlg,11104,WM_GETTEXTLENGTH,0,0)*2;
			char *tmp1=tor_malloc(tmpsize+2);
			LangGetDlgItemText(hDlg,11104,tmp1,tmpsize+1);
			setLogFilter(tmp1);tmp1=NULL;set_log_filter();
			EndDialog(hDlg,0);
		}
	}
	return 0;
}

int __stdcall newEditProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	int lpStart=0,lpEnd,i,j;
	char *tmpsel,*tmpsel1,*tmpsel2,*tmpsel3;
	HMENU hMenu;POINT cPoint;
	if(uMsg==WM_RBUTTONUP)
	{	tmpsel=tor_malloc_zero(2048);
		lpEnd=LangGetSelText(hDlg,tmpsel+1024);
		if(lpEnd)
		{	tmpsel1=tmpsel;tmpsel2=tmpsel+1024;tmpsel3=&strban[0];i=0;
			tor_snprintf(tmpsel1,1024,get_lang_str(LANG_MNU_FILTER));
			while(*tmpsel1) tmpsel1++;
			while((*tmpsel2==13)||(*tmpsel2==10)||(*tmpsel2==32)||(*tmpsel2==9))	tmpsel2++;
			while((*tmpsel2)&&(*tmpsel2!=13)&&(*tmpsel2!=10)&&(i<(lpEnd-lpStart)))
			{	if(*tmpsel2=='&')	*tmpsel1++='&';
				*tmpsel3++=*tmpsel2;*tmpsel1++=*tmpsel2++;i++;
			}
			*tmpsel1++=34;*tmpsel1=0;*tmpsel3=0;
		}
		hMenu=CreatePopupMenu();
		LangAppendMenu(hMenu,MF_STRING|MF_UNCHECKED|MF_ENABLED,20100,LANG_MENU_COPY);
		if(*tmpsel)
		{	AppendMenu(hMenu,MF_SEPARATOR,0,0);
			AppendMenu(hMenu,MF_SEPARATOR,0,0);
			LangAppendMenuStr(hMenu,MF_STRING|MF_UNCHECKED|MF_ENABLED,20101,tmpsel);
			for(i=0,lpStart=0;strban[i];i++)
			{	if(strban[i]=='.')
				{
					if(is_dns_letter(strban[i+1]))	lpStart++;
					else	break;
				}
				else if(strban[i]==':')	break;
				else if(is_dns_letter(strban[i]))	;
			}
			if(lpStart&&((strban[i]==':')||(strban[i]==0)||((strban[i]==0x20)&&(strban[i+1]=='.'))))
			{	char *tmp1=tmpsel;
				tor_snprintf(tmp1,1024,get_lang_str(LANG_MNU_TRACK_EXIT));
				while(*tmp1) tmp1++;
				for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
				for(;i;i--){	if((strban[i]=='.')||(strban[i]<33)) break;}
				if(i&&(strban[i]=='.')) i--;
				for(;i;i--){	if((strban[i]=='.')||(strban[i]<33)) break;}
				if(strban[i]=='.'||(strban[i]<33)) i++;
				*tmp1++='.';
				for(;(strban[i]!=0)&&(strban[i]!=':');i++)
				{	if(strban[i]=='&')	*tmp1++='&';
					*tmp1++=strban[i];}
				*tmp1=0;
				if(!check_ip(strban))	LangInsertMenuStr(hMenu,2,MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_BYPOSITION,20104,tmpsel);
				tmp1=tmpsel;
				tor_snprintf(tmpsel,1024,get_lang_str(LANG_MNU_TRACK_EXIT_1));
				while(*tmp1) tmp1++;
				for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
				for(;i;i--) if(strban[i]<33) break;
				if(i&&strban[i]<33) i++;
				j=i;
				for(;(strban[i]!=0)&&(strban[i]!=':');i++)
				{	if(strban[i]=='&')	*tmp1++='&';
					*tmp1++=strban[i];}
				*tmp1=0;
				LangInsertMenuStr(hMenu,3,MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_BYPOSITION,20103,tmpsel);
				tmp1=tmpsel;
				tor_snprintf(tmp1,1024,get_lang_str(LANG_MNU_REMEMBER_EXIT));
				while(*tmp1) tmp1++;
				for(i=j;(strban[i]!=0)&&(strban[i]!=':');i++)
				{	if(strban[i]=='&')	*tmp1++='&';
					*tmp1++=strban[i];}
				*tmp1=0;
				LangInsertMenuStr(hMenu,4,MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_BYPOSITION,20105,tmpsel);
				tmp1=tmpsel;
				tor_snprintf(tmp1,1024,get_lang_str(LANG_MNU_FORGET_EXIT));
				while(*tmp1) tmp1++;
				for(i=j;(strban[i]!=0)&&(strban[i]!=':');i++)
				{	if(strban[i]=='&')	*tmp1++='&';
					*tmp1++=strban[i];}
				*tmp1=0;
				LangInsertMenuStr(hMenu,5,MF_STRING|MF_UNCHECKED|MF_ENABLED|MF_BYPOSITION,20106,tmpsel);
				tmp1=tmpsel;
				tor_snprintf(tmp1,1024,get_lang_str(LANG_MNU_BAN_HOST));
				while(*tmp1) tmp1++;
				for(i=j;(strban[i]!=0)&&(strban[i]!=':');i++)
				{	if(strban[i]=='&')	*tmp1++='&';
					*tmp1++=strban[i];}
				*tmp1=0;
				LangAppendMenuStr(hMenu,MF_STRING|MF_UNCHECKED|MF_ENABLED,20102,tmpsel);
			}
		}
		LangAppendMenu(hMenu,MF_STRING|MF_UNCHECKED|MF_ENABLED,20107,LANG_MNU_DUMP_STATS);
		GetCursorPos(&cPoint);
		TrackPopupMenu(hMenu,TPM_LEFTALIGN,cPoint.x,cPoint.y,0,hMainDialog,0);
		DestroyMenu(hMenu);
		tor_free(tmpsel);
		return 0;
	}
	return CallWindowProc(oldEditProc,hDlg,uMsg,wParam,lParam);
}

void dlgDebug_langUpdate(void)
{	if(!hDlgDebug || !LangGetLanguage()) return;
	int i,j=0;
	char *langTmp,*langTmp1;
	SendDlgItemMessage(hDlgDebug,300,CB_RESETCONTENT,0,0);
	SendDlgItemMessage(hDlgDebug,300,CB_SETITEMDATA,LangCbAddString(hDlgDebug,300,LANG_CB_DEBUG),(LPARAM)LOG_DEBUG);
	SendDlgItemMessage(hDlgDebug,300,CB_SETITEMDATA,LangCbAddString(hDlgDebug,300,LANG_CB_INFO),(LPARAM)LOG_INFO);
	SendDlgItemMessage(hDlgDebug,300,CB_SETITEMDATA,LangCbAddString(hDlgDebug,300,LANG_CB_PROXY),(LPARAM)LOG_ADDR);
	SendDlgItemMessage(hDlgDebug,300,CB_SETITEMDATA,LangCbAddString(hDlgDebug,300,LANG_CB_NOTICE),(LPARAM)LOG_NOTICE);
	SendDlgItemMessage(hDlgDebug,300,CB_SETITEMDATA,LangCbAddString(hDlgDebug,300,LANG_CB_WARNING),(LPARAM)LOG_WARN);
	SendDlgItemMessage(hDlgDebug,300,CB_SETITEMDATA,LangCbAddString(hDlgDebug,300,LANG_CB_ERROR),(LPARAM)LOG_ERR);
	for(i=0;i<10;i++)
	{	j=SendDlgItemMessage(hDlgDebug,300,CB_GETITEMDATA,i,0);
		if(j==CB_ERR) break;
		if((tmpOptions->logging&0xff)==j)
		{	SendDlgItemMessage(hDlgDebug,300,CB_SETCURSEL,i,0);
			break;
		}
	}
	langTmp=tor_malloc(MAX_PATH+1);tor_snprintf(langTmp,MAX_PATH,"%s.&log",exename);
	i=strlen(langTmp)+50;
	langTmp1=tor_malloc(i);
	tor_snprintf(langTmp1,i,get_lang_str(LANG_DLG_SAVE_TO_OTHER_LOG),langTmp);
	SetDlgItemTextL(hDlgDebug,400,langTmp1);
	tor_free(langTmp);tor_free(langTmp1);
	changeDialogStrings(hDlgDebug,lang_dlg_debug);
}

int __stdcall dlgDebug(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgDebug=hDlg;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_debug);
		}
		SendDlgItemMessage(hDlg,100,EM_LIMITTEXT,65536,0);
		char *fname,*fname1;
		if(tmpOptions->logging&0x8000)
		{	CheckDlgButton(hDlg,400,BST_CHECKED);
			fname=get_datadir_fname2_suffix(NULL,NULL,"log");
			setLog(tmpOptions->logging&0xff,fname);
			tor_free(fname);
		}
		fname=tor_malloc(MAX_PATH+1);tor_snprintf(fname,MAX_PATH,"%s.&log",exename);
		int i=strlen(fname)+50;
		fname1=tor_malloc(i);
		tor_snprintf(fname1,i,get_lang_str(LANG_DLG_SAVE_TO_OTHER_LOG),fname);
		SetDlgItemTextL(hDlg,400,fname1);
		tor_free(fname);tor_free(fname1);
		strban[0]=0;
		SendDlgItemMessage(hDlg,100,EM_LIMITTEXT,65535,0);
		SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,LangCbAddString(hDlg,300,LANG_CB_DEBUG),(LPARAM)LOG_DEBUG);
		SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,LangCbAddString(hDlg,300,LANG_CB_INFO),(LPARAM)LOG_INFO);
		SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,LangCbAddString(hDlg,300,LANG_CB_PROXY),(LPARAM)LOG_ADDR);
		SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,LangCbAddString(hDlg,300,LANG_CB_NOTICE),(LPARAM)LOG_NOTICE);
		SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,LangCbAddString(hDlg,300,LANG_CB_WARNING),(LPARAM)LOG_WARN);
		SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,LangCbAddString(hDlg,300,LANG_CB_ERROR),(LPARAM)LOG_ERR);
		if(tmpOptions->logging&0x4000){	CheckDlgButton(hDlg,401,BST_CHECKED);setDialog(hDlg);}
		else	setDialog(NULL);
		int j=0;
		for(i=0;i<10;i++)
		{	j=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,i,0);
			if(j==CB_ERR) break;
			if((tmpOptions->logging&0xff)==j)
			{	SendDlgItemMessage(hDlg,300,CB_SETCURSEL,i,0);
				break;
			}
		}
		oldEditProc=(WNDPROC)GetWindowLong(GetDlgItem(hDlg,100),GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hDlg,100),GWL_WNDPROC,(LONG)&newEditProc);
	}
	else if(uMsg==WM_COMMAND)
	{
		if((LOWORD(wParam)==300)&&(HIWORD(wParam)==CBN_SELCHANGE))
		{	int i=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
			if(i!=CB_ERR)
			{	tmpOptions->logging&=0xff00;tmpOptions->logging|=i&0xff;
				setLogging(i);
				if((tmpOptions->logging&0xff)<LOG_ADDR)	tmpOptions->SafeLogging=1;
				else	tmpOptions->SafeLogging=0;
			}
		}
		else if(LOWORD(wParam)==400)
		{	if(IsDlgButtonChecked(hDlg,400)==BST_CHECKED)
			{	tmpOptions->logging|=0x8000;
				char *fname=fname=get_datadir_fname2_suffix(NULL,NULL,"log");
				setLog(tmpOptions->logging&0xff,fname);
				tor_free(fname);
			}
			else
			{	tmpOptions->logging&=0x7fff;
				setLog(tmpOptions->logging&0xff,NULL);
			}
		}
		else if(LOWORD(wParam)==401)
		{	if(IsDlgButtonChecked(hDlg,401)==BST_CHECKED)	setDialog(hDlg);
			else	setDialog(NULL);
		}
		else if(LOWORD(wParam)==10)	LangClearText(hDlg);
		else if(LOWORD(wParam)==11)	DialogBoxParamW(hInstance,(LPWSTR)MAKEINTRESOURCE(1121),hDlg,&dlgFilters,0);
	}
	else if(uMsg==WM_TIMER)
	{	if(wParam==101)	LangDebugScroll(hDlg);
	}
	return 0;
}
