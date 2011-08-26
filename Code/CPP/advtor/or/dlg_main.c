#include "or.h"
#include "dlg_util.h"
#include "plugins.h"
#include "dlg_resize.h"
#include <shellapi.h>

#define STARTUP_OPTION_START_TOR 1
#define STARTUP_OPTION_MINIMIZE_AT_STARTUP 2
#define STARTUP_OPTION_RESIZE_SETTINGS_APPLIED 4

HWND hMainDialog=NULL;
HANDLE hIcon,hIcon1,hMenu;
NOTIFYICONDATA nid;
extern HWND hDlgProxy;
extern HWND hDlgAuthorities;
extern HWND hDlgRouterRestrictions;
extern HWND hDlgCircuitBuild;
extern HWND hDlgConnections;
extern HWND hDlgBridges;
extern HWND hDlgHiddenServices;
extern HWND hDlgPlugins;
extern HWND hDlgSystem;
extern HWND hDlgForceTor;
extern HWND hDlgServer;
extern HWND hDlgNetInfo;
extern HWND hDlgDebug;
extern HWND hDlgAbout;
extern HINSTANCE hInstance;
extern HANDLE hThread;
extern DWORD thread_id;
extern int selectedVer;
extern int lastSort;
extern char strban[256+3];
extern or_options_t *tmpOptions;
extern char exename[MAX_PATH+1];
extern BOOL started;
extern resize_info_t resize_main[];
extern resize_info_t resize_proxy[];
extern resize_info_t resize_authorities[];
extern resize_info_t resize_router_restrictions[];
extern resize_info_t resize_circuit_build[];
extern resize_info_t resize_connections[];
extern resize_info_t resize_bridges[];
extern resize_info_t resize_hs[];
extern resize_info_t resize_plugins[];
extern resize_info_t resize_system[];
extern resize_info_t resize_force_tor[];
extern resize_info_t resize_server[];
extern resize_info_t resize_network_information[];
extern resize_info_t resize_debug[];
extern resize_info_t resize_about[];
extern resize_info_t resize_plugin[];
char newname[200];
int frame=1100;
HWND selWnd=NULL;
resize_info_t *resize_sel=NULL;
int debugIdx=0;
int startupOption=0;
POINT point;

HANDLE hLibrary;
LPFN1 ShowProcesses=NULL,TORUnhook=NULL,SetGetLangStrCallback=NULL,GetProcessChainKey=NULL,RegisterPluginKey=NULL,UnregisterPluginKey=NULL;
LPFN2 SetProc=NULL;
LPFN3 TORHook=NULL,CreateNewProcess=NULL;
LPFN4 UnloadDLL=NULL,GetAdvTorVer=NULL;
LPFN5 GetConnInfo=NULL;
LPFN6 ShowProcessTree=NULL;
LPFN7 HookProcessTree=NULL;
LPFN8 PidFromAddr=NULL;
LPFN9 ProcessNameFromPid=NULL;
LPFN11 ShowOpenPorts=NULL;
LPFN10 GetChainKeyName=NULL;

lang_dlg_info lang_dlg_main[]={
	{1,LANG_DLG_START_TOR},
	{6,LANG_DLG_NEW_IDENTITY},
	{3,LANG_DLG_SAVE_SETTINGS},
	{5,LANG_DLG_ALWAYS_ON_TOP},
	{4,LANG_DLG_MINIMIZE_TO_TRAY},
	{2,LANG_DLG_EXIT},
	{0,0}
};

int __stdcall dlgProxy(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgInterceptProcesses(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgDebug(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgForceTor(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgCircuitBuild(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgConnections(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgBridges(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgHiddenServices(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgPlugins(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgAuthorities(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgRouterRestrictions(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgExitSelect(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgSystem(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgServer(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgNetInfo(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int __stdcall dlgAbout(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
void dlgProxy_banDebugAddress(char *strban);
void dlgDebug_logFilterAdd(char *strban);
void dlgCircuitBuild_trackedHostExitAdd(HWND hDlg,char *newAddr);
void dlgCircuitBuild_trackedDomainExitAdd(HWND hDlg,char *newAddr);
void dlgCircuitBuild_addressMapAdd(HWND hDlg,char *newAddr);
void dlgCircuitBuild_addressMapRemove(HWND hDlg,char *newAddr);
void dlgForceTor_quickStart();
void dlgForceTor_unhookAll(void);
void dlgForceTor_menuAppendHookedProcesses(HMENU hMenu3);
void dlgForceTor_menuRelease(int item);
void dlgForceTor_quickStartClearAll(void);
void dlgForceTor_quickStartFromMenu(int item);
void dlgForceTor_interceptNewProcess(void);
void dlgForceTor_interceptFocusedProcess(void);
void dlgForceTor_releaseFocusedProcess(void);
void dlgSystem_RegisterHotKeys(void);
void dlgSystem_UnregisterHotKeys(void);
void releaseGraph(void);
void recalcGraph(void);
void signewnym_impl(time_t now);
void dumpstats(int severity);
HMENU getProcessesMenu(int tray);

void __stdcall _proxy_log(int log_level,const char *msg)
{	log(log_level,LD_APP,msg);
}

void setStartupOption(int commandId)
{	if(commandId==CMD_START)	startupOption |= STARTUP_OPTION_START_TOR;
	else if(commandId==CMD_MINIMIZE)	startupOption |= STARTUP_OPTION_MINIMIZE_AT_STARTUP;
}

void progressLog(int percent,const char *message)
{	SendDlgItemMessage(hMainDialog,1002,PBM_SETPOS,percent,0);
	SetDlgItemTextL(hMainDialog,1003,message);
}

void showLastExit(char *rname,uint32_t addr)
{	uint32_t raddr=geoip_reverse(addr);
	if(rname)
	{	tor_snprintf(newname,200,"%s [exit(%s): %d.%d.%d.%d (%s)]",exename,geoip_get_country_name(geoip_get_country_by_ip(raddr)),raddr&0xff,(raddr>>8)&0xff,(raddr>>16)&0xff,(raddr>>24)&0xff,rname);
		SetWindowText(hMainDialog,newname);
	}
	else
	{	raddr=get_router_sel();
		if(raddr)
		{	raddr=geoip_reverse(raddr);
			tor_snprintf(newname,200,"%s [exit(%s): %d.%d.%d.%d]%s",exename,geoip_get_country_name(geoip_get_country_by_ip(raddr)),raddr&0xff,(raddr>>8)&0xff,(raddr>>16)&0xff,(raddr>>24)&0xff,(addr==-1)?"":" - no exit");
		}
		else if(get_country_sel()!=0x200)	tor_snprintf(newname,200,"%s [exit(%s)]%s",exename,geoip_get_country_name(get_country_sel()),(addr==-1)?"":" - no exit");
		else	tor_snprintf(newname,200,"%s - %s exit",exename,(addr==-1)?"new":"no");
		SetWindowText(hMainDialog,newname);
	}
	if(nid.cbSize)
	{	tor_snprintf(nid.szTip,63,newname);
		nid.uFlags=NIF_TIP;
		Shell_NotifyIcon(NIM_MODIFY,&nid);
	}
}

char *getLanguageFileName(char *source)
{	char *lngname=tor_malloc(MAX_PATH+1);
	tor_snprintf(lngname,MAX_PATH,"%s-%s.lng",exename,source);
	return lngname;
}

int getProcessName(char *buffer,int bufsize,DWORD pid)
{	if(ProcessNameFromPid) return ProcessNameFromPid(buffer,bufsize,pid);
	*buffer=0;
	return 0;
}

DWORD getPID(uint32_t addr,int port)
{	if(PidFromAddr)	return PidFromAddr(addr,port);
	return 0;
}

DWORD getChainKey(DWORD pid)
{	if(pid && GetProcessChainKey) return GetProcessChainKey(pid);
	return 0;
}

void getExclKeyName(char *s1,DWORD exclKey)
{	if(!GetChainKeyName)	tor_snprintf(s1,10,"GENERAL");
	else	GetChainKeyName(s1,exclKey);
}

RECT lastWindowRect,desktopRect;
unsigned int isZoomed=0;
void saveWindowPos(void)
{	isZoomed=IsZoomed(hMainDialog);
	if(!isZoomed)
	{	GetWindowRect(GetDesktopWindow(),&desktopRect);
		GetWindowRect(hMainDialog,&lastWindowRect);
	}
	char *oldvar=tmpOptions->WindowPos;
	tmpOptions->WindowPos=tor_malloc(100);
	tor_snprintf(tmpOptions->WindowPos,100,"%u,%u,%u,%u,%u,%u,%u,%u",isZoomed,(unsigned int)desktopRect.right,(unsigned int)desktopRect.bottom,(unsigned int)lastWindowRect.left,(unsigned int)lastWindowRect.top,(unsigned int)lastWindowRect.right,(unsigned int)lastWindowRect.bottom,(lastSort<0)?((-lastSort) | 0x80):lastSort);
	if(oldvar)	tor_free(oldvar);
}

void setLastSort(int newSort)
{	lastSort = newSort;
	saveWindowPos();
}

void dlgRouterRestrictions_langUpdate(void);
void dlgHiddenServices_langUpdate(void);
void dlgPlugins_langUpdate(void);
void tree_lang_update(void);
void dlgDebug_langUpdate(void);
void setNewLanguage()
{	extern lang_dlg_info lang_dlg_proxy[];
	extern lang_dlg_info lang_dlg_authorities[];
	extern lang_dlg_info lang_dlg_circuitbuild[];
	extern lang_dlg_info lang_dlg_connections[];
	extern lang_dlg_info lang_dlg_bridges[];
	extern lang_dlg_info lang_dlg_system[];
	extern lang_dlg_info lang_dlg_force_tor[];
	extern lang_dlg_info lang_dlg_server[];
	if(hMainDialog==NULL) return;
	changeDialogStrings(hMainDialog,lang_dlg_main);
	if(hDlgProxy)	changeDialogStrings(hDlgProxy,lang_dlg_proxy);
	if(hDlgAuthorities) changeDialogStrings(hDlgAuthorities,lang_dlg_authorities);
	dlgRouterRestrictions_langUpdate();
	if(hDlgCircuitBuild)	changeDialogStrings(hDlgCircuitBuild,lang_dlg_circuitbuild);
	if(hDlgConnections)	changeDialogStrings(hDlgConnections,lang_dlg_connections);
	if(hDlgBridges)	changeDialogStrings(hDlgBridges,lang_dlg_bridges);
	dlgHiddenServices_langUpdate();
	dlgPlugins_langUpdate();
	if(hDlgSystem)	changeDialogStrings(hDlgSystem,lang_dlg_system);
	if(hDlgForceTor)	changeDialogStrings(hDlgForceTor,lang_dlg_force_tor);
	if(hDlgServer)	changeDialogStrings(hDlgServer,lang_dlg_server);
	tree_lang_update();
	dlgDebug_langUpdate();
	SendDlgItemMessage(hMainDialog,200,LB_RESETCONTENT,0,0);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_PROXY),1100);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_AUTHORITIES),1101);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_ROUTER_RESTRICTIONS),1102);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_CIRCUIT_BUILD),1103);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_CONNECTIONS),1104);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_BRIDGES),1105);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_HIDDEN_SERVICES),1106);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_PLUGINS),1107);
	int i=LangLbAddString(hMainDialog,200,LANG_LB_SYSTEM);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,i,1108);
	SendDlgItemMessage(hMainDialog,200,LB_SETCURSEL,i,0);
	if(hLibrary)	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_FORCE_TOR),1109);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_SERVER),1110);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_NETSTAT),1111);
	debugIdx=LangLbAddString(hMainDialog,200,LANG_LB_DEBUG);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,debugIdx,1112);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,LangLbAddString(hMainDialog,200,LANG_LB_ABOUT),1113);
	SendDlgItemMessage(hMainDialog,200,LB_SETITEMDATA,SendDlgItemMessage(hMainDialog,200,LB_ADDSTRING,0,(LPARAM)"--------------------------------"),0);
	dlg_add_all_plugins();
}


int __stdcall dlgfunc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	int i;
	if(uMsg==WM_INITDIALOG)
	{	hMainDialog=hDlg;
		SetClassLong(hDlg,GCL_STYLE,GetClassLong(hDlg,GCL_STYLE)|CS_HREDRAW|CS_VREDRAW|CS_PARENTDC);
		if(LangGetLanguage()) changeDialogStrings(hDlg,lang_dlg_main);
		get_winver();
		tor_snprintf(newname,200,"%s  %s by Albu Cristian, 2009-2011",exename,advtor_ver);
		SetWindowText(hDlg,newname);
		hIcon1=LoadIcon(hInstance,MAKEINTRESOURCE(9));
		SendDlgItemMessage(hDlg,9,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcon1);
		hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(1));nid.cbSize=0;
		SendMessage(hDlg,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)hIcon);
		SendDlgItemMessage(hDlg,1002,0,PBM_SETRANGE,MAKELPARAM(0,100));
		selWnd=hDlgProxy=createChildDialog(hDlg,1100,&dlgProxy);
		resize_sel = resize_proxy;
		ShowWindow(hDlgProxy,SW_SHOW);
		hDlgForceTor=createChildDialog(hDlg,1109,&dlgInterceptProcesses);
		hDlgDebug=createChildDialog(hDlg,1112,&dlgDebug);
		EnableWindow(GetDlgItem(hDlg,6),0);EnableWindow(GetDlgItem(hDlg,8),0);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_PROXY),1100);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_AUTHORITIES),1101);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_ROUTER_RESTRICTIONS),1102);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_CIRCUIT_BUILD),1103);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_CONNECTIONS),1104);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_BRIDGES),1105);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_HIDDEN_SERVICES),1106);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_PLUGINS),1107);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_SYSTEM),1108);
		char *tmp1=get_datadir_fname2_suffix(NULL,NULL,"dll");
		hLibrary=LoadLibrary(tmp1);
		if(hLibrary)
		{	GetAdvTorVer=(LPFN4)GetProcAddress(hLibrary,"GetAdvTorVer");
			if(GetAdvTorVer && GetAdvTorVer()>=0x00020002)
			{	SetProc=(LPFN2)GetProcAddress(hLibrary,"SetProc");ShowProcesses=(LPFN1)GetProcAddress(hLibrary,"ShowProcesses");
				TORHook=(LPFN3)GetProcAddress(hLibrary,"TORHook");TORUnhook=(LPFN1)GetProcAddress(hLibrary,"TORUnhook");
				SetGetLangStrCallback=(LPFN1)GetProcAddress(hLibrary,"SetGetLangStrCallback");
				if(SetGetLangStrCallback) SetGetLangStrCallback((DWORD)&get_lang_str);
				if(SetProc((DWORD)&_proxy_log,hDlgForceTor,pipeName)==0)
				{	FreeLibrary(hLibrary);SetProc=NULL;ShowProcesses=NULL;TORHook=NULL;TORUnhook=NULL;
				}
				else
				{	SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_FORCE_TOR),1109);
				}
				UnloadDLL=(LPFN4)GetProcAddress(hLibrary,"UnloadDLL");
				GetConnInfo=(LPFN5)GetProcAddress(hLibrary,"GetConnInfo");
				ShowProcessTree=(LPFN6)GetProcAddress(hLibrary,"ShowProcessTree");
				HookProcessTree=(LPFN7)GetProcAddress(hLibrary,"HookProcessTree");
				PidFromAddr=(LPFN8)GetProcAddress(hLibrary,"PidFromAddr");
				GetProcessChainKey=(LPFN1)GetProcAddress(hLibrary,"GetProcessChainKey");
				GetChainKeyName=(LPFN10)GetProcAddress(hLibrary,"GetChainKeyName");
				ProcessNameFromPid=(LPFN9)GetProcAddress(hLibrary,"ProcessNameFromPid");
				ShowOpenPorts=(LPFN11)GetProcAddress(hLibrary,"ShowOpenPorts");
				RegisterPluginKey=(LPFN1)GetProcAddress(hLibrary,"RegisterPluginKey");
				UnregisterPluginKey=(LPFN1)GetProcAddress(hLibrary,"UnregisterPluginKey");
				CreateNewProcess=(LPFN3)GetProcAddress(hLibrary,"CreateNewProcess");
			}
			else
			{	log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_VERSION_TOO_OLD));
				FreeLibrary(hLibrary);hLibrary=NULL;
			}
		}
		else log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_ERROR_LOADING_DLL),tmp1);
		tor_free(tmp1);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_SERVER),1110);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_NETSTAT),1111);
		debugIdx=LangLbAddString(hDlg,200,LANG_LB_DEBUG);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,debugIdx,1112);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,LangLbAddString(hDlg,200,LANG_LB_ABOUT),1113);
		SendDlgItemMessage(hDlg,200,LB_SETITEMDATA,SendDlgItemMessage(hDlg,200,LB_ADDSTRING,0,(LPARAM)"--------------------------------"),0);
		SendDlgItemMessage(hDlg,200,LB_SETCURSEL,0,0);
		if((tmpOptions->AutoStart&1) || (startupOption&STARTUP_OPTION_START_TOR)){	CheckDlgButton(hDlg,1,BST_CHECKED);PostMessage(hDlg,WM_COMMAND,1,(LPARAM)GetDlgItem(hDlg,1));}
		if((tmpOptions->AutoStart&2) || (startupOption&STARTUP_OPTION_MINIMIZE_AT_STARTUP)){	ShowWindow(hDlg,SW_MINIMIZE);PostMessage(hDlg,WM_SYSCOMMAND,SC_MINIMIZE,0);}
		log(LOG_NOTICE,LD_GENERAL,get_lang_str(LANG_LOG_DLG_WELCOME),advtor_ver,tmpOptions->SocksPort,tmpOptions->SocksPort);
		dlgForceTor_quickStart();
		dlgStatsRWInit();
		load_plugins();
		dlgSystem_RegisterHotKeys();
		PostMessage(hDlg,WM_SIZE,0,0);
	}
	else if(uMsg==WM_COMMAND)
	{
		if(LOWORD(wParam)==2)
		{	if(!unload_plugins(hDlg)) return 0;
			remove_plugins();
			if(nid.cbSize) Shell_NotifyIcon(NIM_DELETE,&nid);nid.cbSize=0;
			dlgForceTor_unhookAll();
			dlgSystem_UnregisterHotKeys();
			releaseGraph();
			EndDialog(hDlg,0);
		}
		else if(LOWORD(wParam)==3)
		{	if(options_save_current()<0) LangMessageBox(hDlg,get_lang_str(LANG_MB_WRITE_ERROR),LANG_MB_ERROR,MB_OK);
			else LangMessageBox(hDlg,get_lang_str(LANG_MB_OPTIONS_SAVED),LANG_MB_SAVE_SETTINGS,MB_OK);
		}
		else if(LOWORD(wParam)==1)
		{	if(IsDlgButtonChecked(hDlg,1)==BST_UNCHECKED)
			{	started=2;EnableWindow(GetDlgItem(hDlg,6),0);EnableWindow(GetDlgItem(hDlg,8),0);
				plugins_start(0);
				circuit_expire_all_circs(0);
				hibernate_go_dormant(get_time(NULL));
				LangSetDlgItemText(hDlg,1,LANG_DLG_START_TOR);
			}
			else if(started==2)
			{	started=1;plugins_start(1);
				EnableWindow(GetDlgItem(hDlg,6),1);EnableWindow(GetDlgItem(hDlg,8),1);
				hibernate_end_time_elapsed(get_time(NULL));
				LangSetDlgItemText(hDlg,1,LANG_MNU_STOP_TOR);
			}
			else
			{	started=1;plugins_start(1);	//SetDlgItemText(hDlg,1,"&Stop");
				LangSetDlgItemText(hDlg,1,LANG_MNU_STOP_TOR);
				if(!tmpOptions->MaxUnusedOpenCircuits)	control_event_bootstrap(BOOTSTRAP_STATUS_STARTED,0);
				hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)tor_thread,0,0,(LPDWORD)&thread_id);
				EnableWindow(GetDlgItem(hDlg,6),1);EnableWindow(GetDlgItem(hDlg,8),1);
				if(frame!=1111)
				{	SendDlgItemMessage(hDlg,200,LB_SETCURSEL,debugIdx,0);
					PostMessage(hDlg,WM_COMMAND,LBN_SELCHANGE<<16|200,(LPARAM)GetDlgItem(hDlg,200));
				}
			}
		}
		else if(LOWORD(wParam)==6 && started==1)
		{	if((tmpOptions->BestTimeDelta)&&(tmpOptions->DirFlags&32)) delta_t=tmpOptions->BestTimeDelta;
			else delta_t=crypto_rand_int(tmpOptions->MaxTimeDelta*2)-tmpOptions->MaxTimeDelta;
			update_best_delta_t(delta_t);
			if(selectedVer==-1) selectedVer=crypto_rand_int(41);
			set_router_sel(0,1);signewnym_impl(0);
			if(tmpOptions->IdentityFlags&IDENTITY_FLAG_DESTROY_CIRCUITS)	circuit_expire_all_circs(0);
			showLastExit(NULL,-1);
			LangMessageBox(NULL,get_lang_str(LANG_MB_IDENTITY_DIFFERENT_IP),LANG_MB_NEW_IDENTITY,MB_OK|MB_TASKMODAL|MB_SETFOREGROUND);
		}
		else if(LOWORD(wParam)==5)
		{	if(IsDlgButtonChecked(hDlg,5)==BST_CHECKED)	SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			else	SetWindowPos(hDlg,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		else if(LOWORD(wParam)==4)
		{	nid.cbSize=sizeof(nid);nid.hWnd=hDlg;nid.uID=100;nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;nid.uCallbackMessage=WM_USER+10;nid.hIcon=hIcon;
			tor_snprintf(nid.szTip,63,newname);
			Shell_NotifyIcon(NIM_ADD,&nid);
			ShowWindow(hDlg,SW_HIDE);
		}
		else if(LOWORD(wParam)==7)
		{	Shell_NotifyIcon(NIM_DELETE,&nid);nid.cbSize=0;
			ShowWindow(hDlg,SW_SHOWNORMAL);BringWindowToTop(hDlg);SetForegroundWindow(hDlg);
			SendDlgItemMessage(hDlg,100,EM_SCROLLCARET,0,0);
		}
		else if(LOWORD(wParam)==8)
		{	DialogBoxParamW(hInstance,(LPWSTR)MAKEINTRESOURCE(1001),hDlg,&dlgExitSelect,SELECT_SET_EXIT);
		}
		else if(LOWORD(wParam)==9)
		{	if(IsWindowVisible(hDlg)) wParam=1;
			else wParam=0;
			if(wParam) ShowWindow(hDlg,SW_HIDE);
			DialogBoxParamW(hInstance,(LPWSTR)MAKEINTRESOURCE(1002),hDlg,&dlgForceTor,0);
			if(wParam) ShowWindow(hDlg,SW_SHOW);
		}
		else if(LOWORD(wParam)==11)
		{	CheckDlgButton(hDlg,1,BST_CHECKED);PostMessage(hDlg,WM_COMMAND,1,(LPARAM)GetDlgItem(hDlg,1));
		}
		else if(LOWORD(wParam)==12)
		{	CheckDlgButton(hDlg,1,BST_UNCHECKED);PostMessage(hDlg,WM_COMMAND,1,(LPARAM)GetDlgItem(hDlg,1));
		}
		else if((LOWORD(wParam)==200)&&(HIWORD(wParam)==LBN_SELCHANGE))
		{	int tmpframe=SendDlgItemMessage(hDlg,200,LB_GETITEMDATA,SendDlgItemMessage(hDlg,200,LB_GETCURSEL,0,0),0);
			if(frame==tmpframe) return 0;
			switch(frame)
			{	case 1100:
					ShowWindow(hDlgProxy,SW_HIDE);
					break;
				case 1101:
					ShowWindow(hDlgAuthorities,SW_HIDE);
					break;
				case 1102:
					ShowWindow(hDlgRouterRestrictions,SW_HIDE);
					break;
				case 1103:
					ShowWindow(hDlgCircuitBuild,SW_HIDE);
					break;
				case 1104:
					ShowWindow(hDlgConnections,SW_HIDE);
					break;
				case 1105:
					ShowWindow(hDlgBridges,SW_HIDE);
					break;
				case 1106:
					ShowWindow(hDlgHiddenServices,SW_HIDE);
					break;
				case 1107:
					ShowWindow(hDlgPlugins,SW_HIDE);
					break;
				case 1108:
					ShowWindow(hDlgSystem,SW_HIDE);
					break;
				case 1109:
					KillTimer(hDlgForceTor,100);
					ShowWindow(hDlgForceTor,SW_HIDE);
					break;
				case 1110:
					ShowWindow(hDlgServer,SW_HIDE);
					break;
				case 1111:
					KillTimer(hDlgNetInfo,102);
					ShowWindow(hDlgNetInfo,SW_HIDE);
					break;
				case 1112:
					ShowWindow(hDlgDebug,SW_HIDE);
					break;
				case 1113:
					ShowWindow(hDlgAbout,SW_HIDE);
					break;
				default:
					if(frame>=4096)
					{	selWnd=get_plugin_window(frame);
						if(selWnd)	ShowWindow(selWnd,SW_HIDE);
					}
					break;
			}
			frame=tmpframe;
			switch(frame)
			{	case 1100:
					if(!hDlgProxy)	hDlgProxy=createChildDialog(hDlg,1100,&dlgProxy);
					selWnd = hDlgProxy;
					resize_sel = resize_proxy;
					break;
				case 1101:
					if(!hDlgAuthorities)	hDlgAuthorities=createChildDialog(hDlg,1101,&dlgAuthorities);
					selWnd = hDlgAuthorities;
					resize_sel = resize_authorities;
					break;
				case 1102:
					if(!hDlgRouterRestrictions)	hDlgRouterRestrictions=createChildDialog(hDlg,1102,&dlgRouterRestrictions);
					selWnd = hDlgRouterRestrictions;
					resize_sel = resize_router_restrictions;
					break;
				case 1103:
					if(!hDlgCircuitBuild)	hDlgCircuitBuild=createChildDialog(hDlg,1103,&dlgCircuitBuild);
					selWnd = hDlgCircuitBuild;
					resize_sel = resize_circuit_build;
					break;
				case 1104:
					if(!hDlgConnections)	hDlgConnections=createChildDialog(hDlg,1104,&dlgConnections);
					selWnd = hDlgConnections;
					resize_sel = resize_connections;
					break;
				case 1105:
					if(!hDlgBridges)	hDlgBridges=createChildDialog(hDlg,1105,&dlgBridges);
					selWnd = hDlgBridges;
					resize_sel = resize_bridges;
					break;
				case 1106:
					if(!hDlgHiddenServices)	hDlgHiddenServices=createChildDialog(hDlg,1106,&dlgHiddenServices);
					selWnd = hDlgHiddenServices;
					resize_sel = resize_hs;
					break;
				case 1107:
					if(!hDlgPlugins)	hDlgPlugins=createChildDialog(hDlg,1107,&dlgPlugins);
					selWnd = hDlgPlugins;
					resize_sel = resize_plugins;
					break;
				case 1108:
					if(!hDlgSystem)	hDlgSystem=createChildDialog(hDlg,1108,&dlgSystem);
					selWnd = hDlgSystem;
					resize_sel = resize_system;
					break;
				case 1109:
					if(!hDlgForceTor)	hDlgForceTor=createChildDialog(hDlg,1109,&dlgInterceptProcesses);
					selWnd = hDlgForceTor;
					resize_sel = resize_force_tor;
					SetTimer(hDlgForceTor,100,1000,0);
					break;
				case 1110:
					if(!hDlgServer)	hDlgServer=createChildDialog(hDlg,1110,&dlgServer);
					selWnd = hDlgServer;
					resize_sel = resize_server;
					break;
				case 1111:
					if(!hDlgNetInfo)	hDlgNetInfo=createChildDialog(hDlg,1111,&dlgNetInfo);
					selWnd = hDlgNetInfo;
					resize_sel = resize_network_information;
					SetTimer(hDlgNetInfo,102,1000,0);
					break;
				case 1112:
					if(!hDlgDebug)	hDlgDebug=createChildDialog(hDlg,1112,&dlgDebug);
					selWnd = hDlgDebug;
					resize_sel = resize_debug;
					break;
				case 1113:
					if(!hDlgAbout)	hDlgAbout=createChildDialog(hDlg,1113,&dlgAbout);
					selWnd = hDlgAbout;
					resize_sel = resize_about;
					break;
				default:
					if(frame>=4096)
					{	selWnd = get_plugin_window(frame);
						resize_plugin[0].refWidthControl=frame;
						resize_sel = resize_plugin;
					}
					else
					{	selWnd = 0;
						resize_sel = 0;
					}
					break;
			}
			if(selWnd)
			{	resizeChildDialog(hDlg,selWnd,resize_sel);
				if(resize_sel==resize_network_information) recalcGraph();
				ShowWindow(selWnd,SW_SHOW);
				if(resize_sel==resize_debug)	LangDebugScroll(hDlgDebug);
				RedrawWindow(hDlg,NULL,NULL,RDW_INVALIDATE|RDW_INTERNALPAINT|RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_NOERASE);
			}
		}
		else if(LOWORD(wParam)==12402)
			dlgForceTor_interceptNewProcess();
		else if(LOWORD(wParam)==20100 && hDlgDebug)
			SendDlgItemMessage(hDlgDebug,100,WM_COPY,0,0);
		else if(LOWORD(wParam)==20101)
			dlgDebug_logFilterAdd(strban);
		else if(LOWORD(wParam)==20102)
			dlgProxy_banDebugAddress(strban);
		else if(LOWORD(wParam)==20103)
		{	if(strban[0])
			{	for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
				for(;i;i--) if(strban[i]<33) break;
				if(i&&strban[i]<33) i++;
				dlgCircuitBuild_trackedHostExitAdd(hDlg,&strban[i]);
			}
		}
		else if(LOWORD(wParam)==20104)
		{	if(strban[0])
			{	for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
				for(;i;i--) if(strban[i]<33) break;
				if(i&&strban[i]<33) i++;
				dlgCircuitBuild_trackedDomainExitAdd(hDlg,&strban[i]);
			}
		}
		else if(LOWORD(wParam)==20105)
		{	if(strban[0])
			{	for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
				for(;i;i--) if(strban[i]<33) break;
				if(i&&strban[i]<33) i++;
				dlgCircuitBuild_addressMapAdd(hDlg,&strban[i]);
			}
		}
		else if(LOWORD(wParam)==20106)
		{	if(strban[0])
			{	for(i=0;(strban[i]!=0)&&(strban[i]!=':');i++)	;
				for(;i;i--) if(strban[i]<33) break;
				if(i&&strban[i]<33) i++;
				dlgCircuitBuild_addressMapRemove(hDlg,&strban[i]);
			}
		}
		else if(LOWORD(wParam)==20107)	dumpstats(LOG_NOTICE);
		else if(LOWORD(wParam)==20199)
		{	set_router_sel((uint32_t)0x0100007f,1);
			if((tmpOptions->BestTimeDelta)&&(tmpOptions->DirFlags&32)) delta_t=tmpOptions->BestTimeDelta;
			else delta_t=crypto_rand_int(tmpOptions->MaxTimeDelta*2)-tmpOptions->MaxTimeDelta;
			update_best_delta_t(delta_t);
			if(selectedVer==-1) selectedVer=crypto_rand_int(41);
			showLastExit(NULL,0);signewnym_impl(0);
			if(tmpOptions->IdentityFlags&IDENTITY_FLAG_DESTROY_CIRCUITS)	circuit_expire_all_circs(0);
			showLastExit(NULL,-1);
			LangMessageBox(hDlg,get_lang_str(LANG_MB_IDENTITY_NO_EXIT),LANG_MB_NEW_IDENTITY,MB_OK);
		}
		else if((LOWORD(wParam)>=20200)&&(LOWORD(wParam)<21000))
		{	uint32_t newSel=get_menu_selection(LOWORD(wParam)-20200);
			if(newSel)
			{	set_router_sel(newSel,1);
				if((tmpOptions->BestTimeDelta)&&(tmpOptions->DirFlags&32)) delta_t=tmpOptions->BestTimeDelta;
				else delta_t=crypto_rand_int(tmpOptions->MaxTimeDelta*2)-tmpOptions->MaxTimeDelta;
				update_best_delta_t(delta_t);
				if(selectedVer==-1) selectedVer=crypto_rand_int(41);
				signewnym_impl(0);
				if(tmpOptions->IdentityFlags&IDENTITY_FLAG_DESTROY_CIRCUITS)	circuit_expire_all_circs(newSel!=0);
				showLastExit(NULL,-1);
				LangMessageBox(hDlg,get_lang_str(LANG_MB_IDENTITY_DIFFERENT_IP),LANG_MB_NEW_IDENTITY,MB_OK);
			}
		}
		else if((LOWORD(wParam)>=22000)&&(LOWORD(wParam)<(22000+MAX_PID_LIST)))
			dlgForceTor_menuRelease(LOWORD(wParam)-22000);
		else if(LOWORD(wParam)==22999)
			dlgForceTor_quickStartClearAll();
		else if((LOWORD(wParam)>=23000)&&(LOWORD(wParam)<24000))
			dlgForceTor_quickStartFromMenu(LOWORD(wParam)-23000);
	}
	else if((uMsg==(WM_USER+10))&&(LOWORD(wParam)==100))
	{	if((lParam==WM_LBUTTONUP)||(lParam==WM_LBUTTONDBLCLK))
		{	if(nid.cbSize){	Shell_NotifyIcon(NIM_DELETE,&nid);nid.cbSize=0;}
			ShowWindow(hDlg,SW_SHOWNORMAL);BringWindowToTop(hDlg);SetForegroundWindow(hDlg);
			if(resize_sel==resize_debug)	LangDebugScroll(hDlgDebug);
		}
		else if(lParam==WM_RBUTTONUP)
		{	HMENU hMenu2=NULL,hMenu3=NULL,hMenu4=getProcessesMenu(1);
			hMenu=CreatePopupMenu();
			LangAppendMenu(hMenu,MF_STRING,7,LANG_MENU_SHOW_WINDOW);
			LangAppendMenu(hMenu,MF_STRING,9,LANG_MENU_FORCE_TOR);
			LangAppendMenu(hMenu,MF_POPUP,(UINT)hMenu4,LANG_MENU_QUICK_START);
			hMenu3=CreatePopupMenu();
			LangAppendMenu(hMenu,MF_POPUP,(UINT)hMenu3,LANG_MENU_RELEASE);
			dlgForceTor_menuAppendHookedProcesses(hMenu3);
			if(started&1)
			{	AppendMenu(hMenu,MF_SEPARATOR,0,0);
				LangAppendMenu(hMenu,MF_STRING,6,LANG_MENU_NEW_IDENTITY);
				hMenu2=CreatePopupMenu();
				add_routers_to_menu(hMenu2);
				AppendMenu(hMenu2,MF_STRING,20199,"-- &No exit");
				AppendMenu(hMenu2,MF_SEPARATOR,0,0);
				AppendMenu(hMenu2,MF_STRING,8,"&Advanced ...");
				LangAppendMenu(hMenu,MF_POPUP,(UINT)hMenu2,LANG_MENU_SELECT_IP);
				AppendMenu(hMenu,MF_SEPARATOR,0,0);
			}
			else LangAppendMenu(hMenu,MF_STRING,11,LANG_MENU_START_TOR);
			LangAppendMenu(hMenu,MF_STRING,3,LANG_MENU_SAVE_SETTINGS);
			if(started&1)	LangAppendMenu(hMenu,MF_STRING,12,LANG_MNU_STOP_TOR);
			LangAppendMenu(hMenu,MF_STRING,2,LANG_MENU_EXIT);
			GetCursorPos(&point);
			SetForegroundWindow(hDlg);
			TrackPopupMenu(hMenu,TPM_RIGHTALIGN,point.x,point.y,0,hDlg,NULL);
			DestroyMenu(hMenu);
			if(hMenu2) DestroyMenu(hMenu2);
			DestroyMenu(hMenu3);DestroyMenu(hMenu4);
		}
	}
	else if(uMsg==WM_USER+11)
	{	if(IsWindowVisible(hDlg)) wParam=1;
		else wParam=0;
		if(wParam) ShowWindow(hDlg,SW_HIDE);
		DialogBoxParamW(hInstance,(LPWSTR)MAKEINTRESOURCE(1002),hDlg,&dlgForceTor,1);
		if(wParam) ShowWindow(hDlg,SW_SHOW);
	}
	else if(uMsg==WM_SIZE)
	{	if(!IsIconic(hDlg))
		{	if(!(startupOption&STARTUP_OPTION_RESIZE_SETTINGS_APPLIED))
			{	if(tmpOptions->WindowPos)
				{	int i,j;
					tor_sscanf(tmpOptions->WindowPos,"%u,%u,%u,%u,%u,%u,%u,%u",&isZoomed,(unsigned int *)&desktopRect.right,(unsigned int *)&desktopRect.bottom,(unsigned int *)&lastWindowRect.left,(unsigned int *)&lastWindowRect.top,(unsigned int *)&lastWindowRect.right,(unsigned int *)&lastWindowRect.bottom,&lastSort);
					if(((lastSort&0x80)!=0) && lastSort>0)	lastSort = -(lastSort & 0x7f);
					if(isZoomed)	ShowWindow(hDlg,SW_MAXIMIZE);
					else
					{	i=desktopRect.right;j=desktopRect.bottom;
						GetWindowRect(GetDesktopWindow(),&desktopRect);
						if(i==desktopRect.right && j==desktopRect.bottom)
						{	if(lastWindowRect.left>desktopRect.left && lastWindowRect.right>lastWindowRect.left && lastWindowRect.right<desktopRect.right && lastWindowRect.top>desktopRect.top && lastWindowRect.bottom>lastWindowRect.top && lastWindowRect.bottom<desktopRect.bottom)
								MoveWindow(hDlg,lastWindowRect.left,lastWindowRect.top,lastWindowRect.right-lastWindowRect.left,lastWindowRect.bottom-lastWindowRect.top,1);
						}
					}
				}
				startupOption |= STARTUP_OPTION_RESIZE_SETTINGS_APPLIED;
			}
			resizeDialogControls(hDlg,resize_main);
			if(selWnd)	resizeChildDialog(hDlg,selWnd,resize_sel);
			if(resize_sel==resize_network_information) recalcGraph();
			else if(resize_sel==resize_debug)	LangDebugScroll(hDlgDebug);
			RedrawWindow(hDlg,NULL,NULL,RDW_INVALIDATE|RDW_INTERNALPAINT|RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_NOERASE);
			if(startupOption&STARTUP_OPTION_RESIZE_SETTINGS_APPLIED)	saveWindowPos();
		}
	}
	else if(uMsg==WM_MOVE)
	{	if(!IsIconic(hDlg) && (startupOption&STARTUP_OPTION_RESIZE_SETTINGS_APPLIED))	saveWindowPos();
	}
	else if(uMsg==WM_HOTKEY)
	{	switch(LOWORD(wParam))
		{	case 11700:
				PostMessage(hDlg,WM_COMMAND,6,(LPARAM)GetDlgItem(hDlg,6));
				break;
			case 11701:
				dlgForceTor_interceptFocusedProcess();
				break;
			case 11702:
				dlgForceTor_releaseFocusedProcess();
				break;
			case 11703:
				if(IsWindowVisible(hDlg))	PostMessage(hDlg,WM_COMMAND,4,(LPARAM)GetDlgItem(hDlg,4));
				else				PostMessage(hDlg,WM_COMMAND,7,(LPARAM)GetDlgItem(hDlg,7));
				break;
			default:
				break;
		}
	}
	else if(uMsg==WM_SYSCOMMAND)
	{	if((wParam&0xfff0)==SC_MINIMIZE)
		{	PostMessage(hDlg,WM_COMMAND,4,(LPARAM)GetDlgItem(hDlg,4));
		}
		else if(((wParam&0xfff0)==SC_MINIMIZE)&&(nid.cbSize))
		{	Shell_NotifyIcon(NIM_DELETE,&nid);nid.cbSize=0;
		}
		else if(((wParam&0xfff0)==SC_CLOSE))
		{	if(!unload_plugins(hDlg)) return 1;
			remove_plugins();
			if(nid.cbSize){ Shell_NotifyIcon(NIM_DELETE,&nid);nid.cbSize=0;}
			dlgForceTor_unhookAll();
			dlgSystem_UnregisterHotKeys();
			releaseGraph();
		}
	}
	return 0;
}
