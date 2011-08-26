#include "or.h"
#include "dlg_util.h"

HWND hDlgForceTor=NULL;
HWND hCapturedWindow;
HANDLE hCursor=NULL,hOldCursor=NULL;
DWORD pidlist[MAX_PID_LIST];
LV_ITEM lvit;
NMLISTVIEW *pnmlv;
char *cmdLineTmp=NULL,*progNameTmp;
extern HANDLE hLibrary;
extern LPFN1 ShowProcesses,TORUnhook,SetGetLangStrCallback,GetProcessChainKey,RegisterPluginKey,UnregisterPluginKey;
extern LPFN2 SetProc;
extern LPFN3 TORHook,CreateNewProcess;
extern LPFN4 UnloadDLL,GetAdvTorVer;
extern LPFN5 GetConnInfo;
extern LPFN6 ShowProcessTree;
extern LPFN7 HookProcessTree;
extern LPFN8 PidFromAddr;
extern LPFN9 ProcessNameFromPid;
extern LPFN11 ShowOpenPorts;
extern LPFN10 GetChainKeyName;
extern or_options_t *tmpOptions;
extern HANDLE hIcon1;
extern HWND hMainDialog;
extern HINSTANCE hInstance;
extern int debugIdx;
//int frame4[]={12400,12010,12011,12100,12401,12402,12403,12404,12405,12406,12407,-1};
lang_dlg_info lang_dlg_force_tor[]={
	{12010,LANG_STR_FRAME4},
	{12401,LANG_DLG_FAKE_LOCAL_TIME},
	{12404,LANG_DLG_RESOLVE_TO_FAKE_IPS},
	{12405,LANG_DLG_DISALLOW_NON_TCP},
	{12406,LANG_DLG_CHANGE_PROCESS_ICON},
	{12407,LANG_DLG_FORCE_TOR_RESERVED1},
	{12011,LANG_DLG_FAKE_LOCAL_ADDRESS},
	{12402,LANG_DLG_RUN_INTERCEPTED},
	{0,0}
};

lang_dlg_info lang_dlg_pf[]={
	{10,LANG_PF_DLG_HELP},
	{1,LANG_PF_DLG_FORCE},
	{2,LANG_PF_DLG_CANCEL},
	{3,LANG_PF_DLG_CANCEL},
	{0,0}
};

lang_dlg_info lang_dlg_ofn[]={
	{10010,LANG_OFN_DLG_CMDLINE},
	{10400,LANG_OFN_DLG_REMEMBER},
	{10011,LANG_OFN_DLG_PROGNAME},
	{0,0}
};

UINT CALLBACK ofnHookProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
void plugins_interceptprocess(DWORD,BOOL);
void dlgForceTor_menuRelease(int item);

void XorWindow(HWND hWnd)
{	RECT r1;HDC hDC;HPEN hPen,hOldPen;HBRUSH hOldBrush;
	GetWindowRect(hWnd,&r1);
	hDC=GetWindowDC(hWnd);
	SetROP2(hDC,R2_XORPEN);
	hPen=CreatePen(PS_INSIDEFRAME,4,0x870f0f);
	hOldPen=SelectObject(hDC,hPen);
	hOldBrush=SelectObject(hDC,GetStockObject(NULL_BRUSH));
	Rectangle(hDC,0,0,r1.right-r1.left,r1.bottom-r1.top);
	r1.right-=r1.left;r1.left=0;
	r1.bottom-=r1.top;r1.top=0;
	InvertRect(hDC,&r1);
	SelectObject(hDC,hOldPen);
	SelectObject(hDC,hOldBrush);
	ReleaseDC(hWnd,hDC);
	DeleteObject(hPen);
}

int __stdcall dlgForceTor(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{	case WM_INITDIALOG:
			if(LangGetLanguage())
			{	SetWindowTextL(hDlg,LANG_PF_DLG_TITLE);
				changeDialogStrings(hDlg,lang_dlg_pf);
			}
			ShowWindow(GetDlgItem(hDlg,500),SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,12),SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,1),SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,2),SW_HIDE);
			SendDlgItemMessage(hDlg,11,STM_SETICON,(WPARAM)hIcon1,(LPARAM)0);
			SendDlgItemMessage(hDlg,500,TVM_DELETEITEM,0,(LPARAM)TVI_ROOT);
			hCapturedWindow=NULL;
			if(lParam)
			{	Sleep(10);SetTimer(hDlg,100,10,0);
			}
			break;
		case WM_COMMAND:
			if((LOWORD(wParam)==2)||(LOWORD(wParam)==3))
			{	if(GetCapture()==hDlg) ReleaseCapture();
				EndDialog(hDlg,0);
			}
			else if(LOWORD(wParam)==1)
			{	if(GetCapture()==hDlg) ReleaseCapture();
				ShowWindow(hDlg,SW_HIDE);
				if(HookProcessTree)
				{	int i=4|tmpOptions->ForceFlags;
					char *tmp2=tor_strdup(tmpOptions->LocalHost);if(!tmp2)	tmp2=tor_strdup("localhost");
					HookProcessTree(GetDlgItem(hDlg,500),(DWORD)tmpOptions->SocksPort,best_delta_t,(DWORD)crypto_rand_int(0x7fffffff),i,tmp2);
					tor_free(tmp2);
				}
				EndDialog(hDlg,1);
			}
			break;
		case WM_MOUSEMOVE:
			if(GetCapture())
			{	POINT p1;HWND testWnd;
				GetCursorPos(&p1);
				testWnd=WindowFromPoint(p1);
				while(GetWindowLong(testWnd,GWL_STYLE)&WS_CHILD) testWnd=GetParent(testWnd);
				if((testWnd!=hCapturedWindow)&&(testWnd!=hDlg)&&(testWnd!=hMainDialog))
				{	if(hCapturedWindow) XorWindow(hCapturedWindow);
					hCapturedWindow=testWnd;
					XorWindow(hCapturedWindow);
					char *tmp1=tor_malloc(200);
					GetWindowText(hCapturedWindow,tmp1,200);
					SetDlgItemText(hDlg,12,tmp1);
					tor_free(tmp1);
					if(ShowProcessTree) ShowProcessTree(hCapturedWindow,GetDlgItem(hDlg,500));
				}
			}
			break;
		case WM_LBUTTONUP:
			if(hCapturedWindow)
			{	XorWindow(hCapturedWindow);
				InvalidateRect(0,0,1);
				if(hOldCursor) SetCursor(hOldCursor);
				ReleaseCapture();
			}
			break;
		case WM_TIMER:
			KillTimer(hDlg,100);
		case WM_LBUTTONDOWN:
			ShowWindow(GetDlgItem(hDlg,500),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg,12),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg,1),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg,2),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg,3),SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,10),SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,11),SW_HIDE);
			SetCapture(hDlg);
			if(hCursor==NULL) hCursor=LoadCursor(hInstance,(LPCTSTR)10);
			if(hOldCursor==NULL) hOldCursor=LoadCursor(NULL,IDC_ARROW);
			SetCursor(hCursor);
			break;
	}
	return	0;
}

void dlgForceTor_interceptFocusedProcess(void)
{	HWND testWnd=GetForegroundWindow();
	DWORD pid=0;
	while(GetWindowLong(testWnd,GWL_STYLE)&WS_CHILD) testWnd=GetParent(testWnd);
	GetWindowThreadProcessId(testWnd,&pid);
	if(pid && pid != GetCurrentProcessId())
	{	if(ShowProcesses)	ShowProcesses((DWORD)hDlgForceTor);
		lvit.iItem=0;lvit.iSubItem=0;lvit.mask=LVIF_PARAM;
		int i=0;
		char *tmp1 = tor_malloc(500);
		char *processname = tor_malloc(200);
		processname[0] = 0;
		getProcessName(processname,200,pid);
		while(i<MAX_PID_LIST)
		{	lvit.lParam=0;
			if(SendDlgItemMessage(hDlgForceTor,12400,LVM_GETITEM,0,(LPARAM)&lvit)==0) break;
			if(lvit.lParam==pid)
			{	if(SendDlgItemMessage(hDlgForceTor,12400,LVM_GETITEMSTATE,lvit.iItem,LVIS_STATEIMAGEMASK)&8192)
				{	i = -1;	break;
				}
			}
			lvit.iItem++;
		}
		if(i==-1)	tor_snprintf(tmp1,500,get_lang_str(LANG_MB_ALREADY_INTERCEPTED),processname);
		else
		{	i=4|tmpOptions->ForceFlags;
			char *tmp2=tor_malloc(256);GetDlgItemText(hDlgForceTor,12100,tmp2,255);
			if(TORHook(pid,(HANDLE)tmpOptions->SocksPort,i,best_delta_t,tmp2,(DWORD)crypto_rand_int(0x7fffffff)))
			{	lvit.mask=LVIF_STATE;
				lvit.state=4096;
				lvit.stateMask=LVIS_STATEIMAGEMASK;
				SendDlgItemMessage(hDlgForceTor,12400,LVM_SETITEMSTATE,lvit.iItem,(LPARAM)&lvit);
				tor_snprintf(tmp1,500,get_lang_str(LANG_MB_INTERCEPT_SUCCESS),processname);
			}
			else	tor_snprintf(tmp1,500,get_lang_str(LANG_MB_INTERCEPT_FAIL),processname);
			tor_free(tmp2);
		}
		LangMessageBox(NULL,tmp1,LANG_DLG_SYSTEM_HOTKEY_INTERCEPT,MB_OK|MB_TASKMODAL|MB_SETFOREGROUND);
		tor_free(tmp1);
		tor_free(processname);
	}
}

void dlgForceTor_releaseFocusedProcess(void)
{	HWND testWnd=GetForegroundWindow();
	DWORD pid=0;
	while(GetWindowLong(testWnd,GWL_STYLE)&WS_CHILD) testWnd=GetParent(testWnd);
	GetWindowThreadProcessId(testWnd,&pid);
	if(pid && pid != GetCurrentProcessId())
	{	pidlist[0] = pid;
		dlgForceTor_menuRelease(0);
	}
}

UINT CALLBACK ofnHookProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if(uMsg==WM_INITDIALOG)
	{	if(LangGetLanguage())	changeDialogStrings(hDlg,lang_dlg_ofn);
		if(get_options()->ForceFlags&4) CheckDlgButton(hDlg,10400,BST_CHECKED);
		else
		{	EnableWindow(GetDlgItem(hDlg,10011),0);
			EnableWindow(GetDlgItem(hDlg,10101),0);
		}
	}
	else if(uMsg==WM_COMMAND)
	{	if(LOWORD(wParam)==10400)
		{	if(IsDlgButtonChecked(hDlg,10400)==BST_CHECKED)
			{	EnableWindow(GetDlgItem(hDlg,10011),1);
				EnableWindow(GetDlgItem(hDlg,10101),1);
				get_options()->ForceFlags|=4;
			}
			else
			{	EnableWindow(GetDlgItem(hDlg,10011),0);
				EnableWindow(GetDlgItem(hDlg,10101),0);
				get_options()->ForceFlags&=4^0xffff;
			}
		}
	}
	else if(uMsg==WM_NOTIFY)
	{	NMHDR *hdr=(NMHDR*)lParam;
		if(hdr->code==CDN_FILEOK)
		{	cmdLineTmp=tor_malloc(256);progNameTmp=tor_malloc(256);
			GetDlgItemText(hDlg,10100,cmdLineTmp,255);
			GetDlgItemText(hDlg,10101,progNameTmp,255);
		}
	}
	return 0;
}

void dlgForceTor_menuRelease(int item)
{	char *tmp1=tor_malloc(500);
	if(ShowProcesses)	ShowProcesses((DWORD)hDlgForceTor);
	lvit.lParam=pidlist[item];
	if((lvit.lParam)&&(TORUnhook))
	{	if(TORUnhook(lvit.lParam))
		{	tor_snprintf(tmp1,500,get_lang_str(LANG_MB_PID_RELEASED),(int)lvit.lParam);
			LangMessageBox(NULL,tmp1,LANG_MB_RELEASE,MB_OK|MB_TASKMODAL|MB_SETFOREGROUND);
		}
		else
		{	tor_snprintf(tmp1,500,get_lang_str(LANG_MB_RELEASE_ERROR),(int)lvit.lParam);
			LangMessageBox(NULL,tmp1,LANG_MB_RELEASE,MB_OK|MB_TASKMODAL|MB_SETFOREGROUND);
		}
	}
	tor_free(tmp1);
}

void dlgForceTor_unhookAll()
{	HANDLE hLib=hLibrary;hLibrary=NULL;
	GetConnInfo=NULL;ShowProcessTree=NULL;HookProcessTree=NULL;GetAdvTorVer=NULL;ShowOpenPorts=NULL;PidFromAddr=NULL;ProcessNameFromPid=NULL;GetProcessChainKey=NULL;GetChainKeyName=NULL;RegisterPluginKey=NULL;UnregisterPluginKey=NULL;
	if(hLib)
	{	if(ShowProcesses)	ShowProcesses((DWORD)hDlgForceTor);
		ShowProcesses=NULL;
		lvit.iItem=0;lvit.iSubItem=0;lvit.mask=LVIF_PARAM;
		while(1)
		{	lvit.lParam=0;
			if(SendDlgItemMessage(hDlgForceTor,12400,LVM_GETITEM,0,(LPARAM)&lvit)==0) break;
			if((lvit.lParam)&&(TORUnhook))
			{	if(SendDlgItemMessage(hDlgForceTor,12400,LVM_GETITEMSTATE,lvit.iItem,LVIS_STATEIMAGEMASK)&8192)
				{	TORUnhook(lvit.lParam);
				}
			}
			lvit.iItem++;
		}
		if(UnloadDLL) UnloadDLL();
		UnloadDLL=NULL;
		TORUnhook=NULL;TORHook=NULL;
		CreateNewProcess=NULL;
		FreeLibrary(hLib);
	}
}

HMENU getProcessesMenu(int tray)
{	HMENU hMenu=CreatePopupMenu();int i,currentOption=0;
	char *tmp2=tor_malloc(256);
	config_line_t *cfg;
	for(cfg=tmpOptions->QuickStart;cfg;cfg=cfg->next)
	{	char *tmp1=cfg->value;tmp2[0]=0;
		if(tmp1)
		{	for(i=0;(*tmp1!='=')&&*tmp1;tmp1++)	tmp2[i++]=*tmp1;
			tmp2[i]=0;
		}
		AppendMenu(hMenu,MF_STRING,23000+currentOption,tmp2);
		currentOption++;
	}
	if(currentOption||tray)
	{	if(currentOption) AppendMenu(hMenu,MF_SEPARATOR,0,0);
		if(tray) LangAppendMenu(hMenu,MF_STRING,12402,LANG_MENU_INTERCEPT);
		LangAppendMenu(hMenu,MF_STRING,22999,LANG_MENU_CLEAR_LIST);
	}
	tor_free(tmp2);
	return hMenu;
}

void dlgForceTor_interceptNewProcess(void)
{	OPENFILENAME ofn;
	char *exeName;
	char fnFilter[]="Executable files (*.exe,*.pif,*.cmd,*.lnk)\0*.exe;*.pif;*.cmd;*.lnk\0All files\0*.*\0\0";
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hMainDialog;
	ofn.hInstance=hInstance;
	ofn.lpstrFilter=fnFilter;
	exeName=tor_malloc(8192);exeName[0]=0;
	ofn.lpstrFile=exeName;
	ofn.nMaxFile=8192;
	ofn.Flags=OFN_EXPLORER|OFN_ENABLETEMPLATE|OFN_ENABLEHOOK|OFN_HIDEREADONLY|OFN_NOCHANGEDIR|OFN_NODEREFERENCELINKS;
	ofn.lpTemplateName=MAKEINTRESOURCE(1010);
	ofn.lpfnHook=ofnHookProc;tmpOptions->ForceFlags&=4^0xffff;
	cmdLineTmp=NULL;progNameTmp=NULL;
	if(GetOpenFileName(&ofn))
	{	char *tmp1=tor_malloc(sizeof(exeName)+1024),*tmp2,*tmp3;tmp2=exeName;
		int i;
		config_line_t *cfg;
		if(GetFileAttributes(exeName)&FILE_ATTRIBUTE_DIRECTORY)
		{	for(;*tmp2;tmp2++)	;
			for(;(tmp2>exeName)&&(*tmp2!='\\')&&(*tmp2!='/');tmp2--)	;
			if((*tmp2=='\\')||(*tmp2=='/')) tmp2++;
		}
		for(tmp3=progNameTmp;*tmp3;tmp3++)
		{	if(*tmp3==34) *tmp3=39;
			else if(*tmp3=='=') *tmp3=':';
		}
		i=4|tmpOptions->ForceFlags;
		tor_snprintf(tmp1,sizeof(exeName)+1024,"%s=%d,\"%s\" %s",progNameTmp,i,tmp2,cmdLineTmp);
		if(tmpOptions->ForceFlags&4)
		{	if(tmpOptions->QuickStart)
			{	for(cfg=tmpOptions->QuickStart;cfg->next;cfg=cfg->next);
				cfg->next=tor_malloc_zero(sizeof(config_line_t));
				cfg=cfg->next;
			}
			else
			{	cfg=tor_malloc_zero(sizeof(config_line_t));
				tmpOptions->QuickStart=cfg;
			}
			cfg->key = tor_strdup("QuickStart");
			cfg->value=tor_strdup(tmp1);
			cfg=NULL;
		}
		tmp2=tor_malloc(256);GetDlgItemText(hDlgForceTor,12100,tmp2,255);
		if(CreateNewProcess) CreateNewProcess((DWORD)tmp1,(HANDLE)tmpOptions->SocksPort,i,best_delta_t,tmp2,(DWORD)crypto_rand_int(0x7fffffff));
		tor_free(tmp1);tor_free(tmp2);
	}
	tor_free(exeName);
	if(cmdLineTmp){ tor_free(cmdLineTmp);cmdLineTmp=NULL;}
	if(progNameTmp){ tor_free(progNameTmp);progNameTmp=NULL;}
}

void dlgForceTor_menuAppendHookedProcesses(HMENU hMenu3)
{	if(ShowProcesses)	ShowProcesses((DWORD)hDlgForceTor);
	char *tmpMenu=tor_malloc(100),*tmpMenu1=tor_malloc(200);
	lvit.iItem=0;lvit.iSubItem=0;lvit.mask=LVIF_PARAM|LVIF_TEXT;lvit.cchTextMax=99;lvit.pszText=tmpMenu;
	int i=0;
	while(i<MAX_PID_LIST)
	{	lvit.lParam=0;
		if(SendDlgItemMessage(hDlgForceTor,12400,LVM_GETITEM,0,(LPARAM)&lvit)==0) break;
		if(lvit.lParam)
		{	if(SendDlgItemMessage(hDlgForceTor,12400,LVM_GETITEMSTATE,lvit.iItem,LVIS_STATEIMAGEMASK)&8192)
			{	tor_snprintf(tmpMenu1,200,"%s (PID: %d)",tmpMenu,(int)lvit.lParam);
				AppendMenu(hMenu3,MF_STRING,22000+i,tmpMenu1);pidlist[i]=lvit.lParam;i++;
			}
		}
		lvit.iItem++;
	}
	tor_free(tmpMenu);tor_free(tmpMenu1);
}

void dlgForceTor_quickStart()
{	if(tmpOptions->QuickStart && CreateNewProcess)
	{	config_line_t *cfg,*cfg2;
		char *tmp2=NULL;
		for(cfg=tmpOptions->QuickStart;cfg;cfg=cfg->next)
		{	if(cfg->value)
			{	if(!strchr(cfg->value, '='))
				{	if(!tmp2)
					{	tmp2=tor_strdup(tmpOptions->LocalHost);if(!tmp2)	tmp2=tor_strdup("localhost");}
					for(cfg2=tmpOptions->QuickStart;cfg2;cfg2=cfg2->next)
					{	if(!strcasecmpstart(cfg2->value,cfg->value) && strchr(cfg2->value, '='))
						{	CreateNewProcess((DWORD)cfg2->value,(HANDLE)tmpOptions->SocksPort,4|tmpOptions->ForceFlags,best_delta_t,tmp2,(DWORD)crypto_rand_int(0x7fffffff));
							break;
						}
					}
				}
			}
		}
		if(tmp2)
		{	tor_free(tmp2);
			if(IsWindowVisible(hMainDialog))
			{	SendDlgItemMessage(hMainDialog,200,LB_SETCURSEL,debugIdx,0);
				PostMessage(hMainDialog,WM_COMMAND,LBN_SELCHANGE<<16|200,(LPARAM)GetDlgItem(hMainDialog,200));
			}
			cfg=tmpOptions->QuickStart;
			while(cfg)
			{	if(!strchr(cfg->value, '='))
				{	tor_free(cfg->key);tor_free(cfg->value);
					tmpOptions->QuickStart=cfg->next;
					tor_free(cfg);cfg=tmpOptions->QuickStart;
				}
				else break;
			}
			for(cfg=tmpOptions->QuickStart;cfg&&cfg->next;)
			{	cfg2=cfg->next;
				if(!strchr(cfg2->value, '='))
				{	tor_free(cfg2->key);tor_free(cfg2->value);
					cfg->next=cfg2->next;
					tor_free(cfg2);
				}
				else cfg=cfg->next;
			}
		}
	}
}

void dlgForceTor_quickStartFromMenu(int item)
{	config_line_t *cfg;
	for(cfg=tmpOptions->QuickStart;item&&cfg;cfg=cfg->next)	item--;
	item=4|tmpOptions->ForceFlags;
	char *tmp2=tor_strdup(tmpOptions->LocalHost);if(!tmp2)	tmp2=tor_strdup("localhost");
	if(cfg&&cfg->value)
	{	if(CreateNewProcess) CreateNewProcess((DWORD)cfg->value,(HANDLE)tmpOptions->SocksPort,item,best_delta_t,tmp2,(DWORD)crypto_rand_int(0x7fffffff));
		if(IsWindowVisible(hMainDialog))
		{	SendDlgItemMessage(hMainDialog,200,LB_SETCURSEL,debugIdx,0);
			PostMessage(hMainDialog,WM_COMMAND,LBN_SELCHANGE<<16|200,(LPARAM)GetDlgItem(hMainDialog,200));
		}
	}
	tor_free(tmp2);
}

void dlgForceTor_quickStartClearAll(void)
{	config_line_t *cfg;
	for(;;)
	{	cfg=tmpOptions->QuickStart;
		if(cfg==NULL) break;
		tor_free(cfg->key);tor_free(cfg->value);
		tmpOptions->QuickStart=cfg->next;
		tor_free(cfg);
	}
}

int __stdcall dlgInterceptProcesses(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgForceTor=hDlg;
		if(LangGetLanguage())	changeDialogStrings(hDlg,lang_dlg_force_tor);
		LangSetDlgItemText(hDlg,12010,LANG_STR_FRAME4);
		if(tmpOptions->LocalHost==NULL)	tmpOptions->LocalHost="localhost";
		SetDlgItemText(hDlg,12100,tmpOptions->LocalHost);
		if(tmpOptions->ForceFlags&2) CheckDlgButton(hDlg,12401,BST_CHECKED);
		if(tmpOptions->ForceFlags&8) CheckDlgButton(hDlg,12404,BST_CHECKED);
		if(tmpOptions->ForceFlags&16) CheckDlgButton(hDlg,12405,BST_CHECKED);
		if(tmpOptions->ForceFlags&32) CheckDlgButton(hDlg,12406,BST_CHECKED);
		if(tmpOptions->ForceFlags&64) CheckDlgButton(hDlg,12407,BST_CHECKED);
	}
	else if(uMsg==WM_COMMAND)
	{
		if(LOWORD(wParam)==12401)
		{	if(IsDlgButtonChecked(hDlg,12401)==BST_CHECKED)	tmpOptions->ForceFlags|=2;
			else	tmpOptions->ForceFlags&=0xffff^2;
		}
		else if(LOWORD(wParam)==12404)
		{	if(IsDlgButtonChecked(hDlg,12404)==BST_CHECKED)	tmpOptions->ForceFlags|=8;
			else	tmpOptions->ForceFlags&=0xffff^8;
		}
		else if(LOWORD(wParam)==12405)
		{	if(IsDlgButtonChecked(hDlg,12405)==BST_CHECKED)	tmpOptions->ForceFlags|=16;
			else	tmpOptions->ForceFlags&=0xffff^16;
		}
		else if(LOWORD(wParam)==12406)
		{	if(IsDlgButtonChecked(hDlg,12406)==BST_CHECKED)	tmpOptions->ForceFlags|=32;
			else	tmpOptions->ForceFlags&=0xffff^32;
		}
		else if(LOWORD(wParam)==12407)
		{	if(IsDlgButtonChecked(hDlg,12407)==BST_CHECKED)	tmpOptions->ForceFlags|=64;
			else	tmpOptions->ForceFlags&=0xffff^64;
		}
		else if(LOWORD(wParam)==12402)
			dlgForceTor_interceptNewProcess();
		else if(LOWORD(wParam)==12403)
		{	RECT wRect;GetWindowRect(GetDlgItem(hDlg,12402),&wRect);
			HMENU hMenu=getProcessesMenu(0);
			TrackPopupMenu(hMenu,TPM_LEFTALIGN,wRect.left,wRect.bottom,0,hMainDialog,0);
			DestroyMenu(hMenu);
		}
		else if((LOWORD(wParam)==12100)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,12100,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,12100,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->LocalHost;
			tmpOptions->LocalHost=tmp1;tor_free(tmp2);
		}
	}
	else if(uMsg==WM_TIMER)
	{	if((wParam==100)&&(ShowProcesses))	ShowProcesses((DWORD)hDlg);
	}
	else if(uMsg==WM_USER+10)
		PostMessage(hMainDialog,uMsg,wParam,lParam);
	else if(uMsg==WM_USER+11)
		plugins_interceptprocess(wParam,lParam);
	else if(uMsg==WM_NOTIFY)
	{	if(wParam==12400)
		{	pnmlv=(LPNMLISTVIEW)lParam;
			NMHDR *hdr=(LPNMHDR)lParam;
			if((hdr->code==LVN_ITEMCHANGED)&&(pnmlv->uChanged&LVIF_STATE)&&(pnmlv->iItem!=-1))
			{
				if((pnmlv->uOldState^pnmlv->uNewState)&8192)
				{	lvit.iItem=pnmlv->iItem;lvit.iSubItem=0;lvit.lParam=0;lvit.mask=LVIF_PARAM;
					SendDlgItemMessage(hDlg,12400,LVM_GETITEM,0,(LPARAM)&lvit);
					if(lvit.lParam)
					{	if((pnmlv->uNewState&8192)&&(TORHook!=0))
						{	int i=4|tmpOptions->ForceFlags;
							char *tmp2=tor_malloc(256);GetDlgItemText(hDlg,12100,tmp2,255);
							if(TORHook(lvit.lParam,(HANDLE)tmpOptions->SocksPort,i,best_delta_t,tmp2,(DWORD)crypto_rand_int(0x7fffffff))==0)
							{	lvit.mask=LVIF_STATE;
								lvit.state=4096;
								lvit.stateMask=LVIS_STATEIMAGEMASK;
								SendDlgItemMessage(hDlg,12400,LVM_SETITEMSTATE,lvit.iItem,(LPARAM)&lvit);
							}
							tor_free(tmp2);
						}
						else if(TORUnhook!=0)
						{	TORUnhook(lvit.lParam);
						}
					}
				}
			}
		}

	}
	return 0;
}
