#include "or.h"
#include "dlg_util.h"

HWND hDlgSystem=NULL;
HANDLE hEdit1;
extern or_options_t *tmpOptions;
extern HWND hMainDialog;
int hkreg=0;
//int frame3[]={11400,11401,11402,11010,11011,11300,11100,11301,11403,11404,11405,11101,11012,11013,11102,11406,11103,11050,11104,-1};
lang_dlg_info lang_dlg_system[]={
	{11010,LANG_DLG_ADVERTISED_OS},
	{11011,LANG_DLG_ADVERTISED_TORVER},
	{11013,LANG_DLG_LANGUAGE},
	{11400,LANG_DLG_START_AUTOMATICALLY},
	{11401,LANG_DLG_MINIMIZE_AT_STARTUP},
	{11402,LANG_DLG_START_WITH_WINDOWS},
	{11403,LANG_DLG_USE_HW_ACCEL},
	{11404,LANG_DLG_FLASH_MEM},
	{11405,LANG_DLG_CONTROL_PORT},
	{11012,LANG_DLG_CONTROL_ADDRESS},
	{11406,LANG_DLG_CONTROL_PASSWORD},
	{11050,LANG_DLG_SYSTEM_HOTKEYS},
	{11110,LANG_DLG_SYSTEM_HOTKEY_RESTORE},
	{11104,LANG_DLG_SYSTEM_HOTKEY_NEW_IDENTITY},
	{11105,LANG_DLG_SYSTEM_HOTKEY_INTERCEPT},
	{11106,LANG_DLG_SYSTEM_HOTKEY_RELEASE},
	{0,0}
};
char runkey[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
char valuename[]="AdvTor";
char voidmsg[]="\0";

void addTorVer(const char *newVer)
{	if(!hDlgSystem)	return;
	if(newVer)	SendDlgItemMessage(hDlgSystem,11100,CB_ADDSTRING,0,(LPARAM)newVer);
	else
	{	while(1)
		{	if(SendDlgItemMessage(hDlgSystem,11100,CB_DELETESTRING,0,0)==CB_ERR) break;
		}
	}
}

void dlgSystem_UnregisterHotKeys(void)
{	if((hkreg & 1) != 0)
	{	if(UnregisterHotKey(hMainDialog,11700))
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_NEW_IDENTITY_UNREGISTER));
			hkreg &= 1 ^ 0xff;
		}
		else	log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_NEW_IDENTITY_UNREGISTER_FAIL));
	}
	if((hkreg & 2) != 0)
	{	if(UnregisterHotKey(hMainDialog,11701))
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_INTERCEPT_UNREGISTER));
			hkreg &= 2 ^ 0xff;
		}
		else	log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_INTERCEPT_UNREGISTER_FAIL));
	}
	if((hkreg & 4) != 0)
	{	if(UnregisterHotKey(hMainDialog,11702))
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RELEASE_UNREGISTER));
			hkreg &= 4 ^ 0xff;
		}
		else	log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RELEASE_UNREGISTER_FAIL));
	}
	if((hkreg & 8) != 0)
	{	if(UnregisterHotKey(hMainDialog,11703))
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RESTORE_UNREGISTER));
			hkreg &= 8 ^ 0xff;
		}
		else	log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RESTORE_UNREGISTER_FAIL));
	}
}

void dlgSystem_RegisterHotKeys(void)
{	dlgSystem_UnregisterHotKeys();
	if((tmpOptions->HotkeyNewIdentity & 0x1000) != 0)
	{	if(!RegisterHotKey(hMainDialog,11700,((tmpOptions->HotkeyNewIdentity >> 8) & 0x0f),(tmpOptions->HotkeyNewIdentity & 0xff)))
			log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_NEW_IDENTITY_FAIL));
		else
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_NEW_IDENTITY_SUCCESS));
			hkreg |= 1;
		}
	}
	if((tmpOptions->HotkeyIntercept & 0x1000) != 0)
	{	if(!RegisterHotKey(hMainDialog,11701,((tmpOptions->HotkeyIntercept >> 8) & 0x0f),(tmpOptions->HotkeyIntercept & 0xff)))
			log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_INTERCEPT_FAIL));
		else
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_INTERCEPT_SUCCESS));
			hkreg |= 2;
		}
	}
	if((tmpOptions->HotkeyRelease & 0x1000) != 0)
	{	if(!RegisterHotKey(hMainDialog,11702,((tmpOptions->HotkeyRelease >> 8) & 0x0f),(tmpOptions->HotkeyRelease & 0xff)))
			log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RELEASE_FAIL));
		else
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RELEASE_SUCCESS));
			hkreg |= 4;
		}
	}
	if((tmpOptions->HotkeyRestore & 0x1000) != 0)
	{	if(!RegisterHotKey(hMainDialog,11703,((tmpOptions->HotkeyRestore >> 8) & 0x0f),(tmpOptions->HotkeyRestore & 0xff)))
			log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RESTORE_FAIL));
		else
		{	log(LOG_INFO,LD_APP,get_lang_str(LANG_LOG_DLG_HOTKEY_RESTORE_SUCCESS));
			hkreg |= 8;
		}
	}
}

int __stdcall dlgSystem(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgSystem=hDlg;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_system);
		}
		enumLanguages(GetDlgItem(hDlg,11301),tmpOptions->Language);
		if(tmpOptions->AutoStart&1)	CheckDlgButton(hDlg,11400,BST_CHECKED);
		if(tmpOptions->AutoStart&2)	CheckDlgButton(hDlg,11401,BST_CHECKED);
		if(tmpOptions->AutoStart&4)	CheckDlgButton(hDlg,11402,BST_CHECKED);
		if(tmpOptions->HardwareAccel)	CheckDlgButton(hDlg,11403,BST_CHECKED);
		if(tmpOptions->AvoidDiskWrites)	CheckDlgButton(hDlg,11404,BST_CHECKED);
		hEdit1=FindWindowEx(GetDlgItem(hDlg,11300),NULL,NULL,NULL);
		int i;
		SendDlgItemMessage(hDlg,11300,CB_ADDSTRING,0,(LPARAM)"<< Random >>");
		for(i=0;i<41;i++) SendDlgItemMessage(hDlg,11300,CB_ADDSTRING,0,(LPARAM)versions[i]);
		SetDlgItemText(hDlg,11300,tmpOptions->winver);
		SendDlgItemMessage(hDlg,11100,CB_ADDSTRING,0,(LPARAM)"<< Auto >>");
		SetDlgItemText(hDlg,11100,tmpOptions->SelectedTorVer);
		if(tmpOptions->ControlPort){ CheckDlgButton(hDlg,11405,BST_CHECKED);SetDlgItemInt(hDlg,11101,tmpOptions->ControlPort,0);}
		else{	EnableWindow(GetDlgItem(hDlg,11101),0);EnableWindow(GetDlgItem(hDlg,11012),0);EnableWindow(GetDlgItem(hDlg,11102),0);EnableWindow(GetDlgItem(hDlg,11406),0);EnableWindow(GetDlgItem(hDlg,11103),0);}
		if(tmpOptions->ControlListenAddress)	SetDlgItemText(hDlg,11102,tmpOptions->ControlListenAddress->value);
		else SetDlgItemText(hDlg,11102,"127.0.0.1");
		if(tmpOptions->CookieAuthentication){ CheckDlgButton(hDlg,11406,BST_CHECKED); if(tmpOptions->HashedControlPassword)  SetDlgItemText(hDlg,11103,tmpOptions->HashedControlPassword->value);}
		else	EnableWindow(GetDlgItem(hDlg,11103),0);
		if((tmpOptions->HotkeyNewIdentity & 0x800)!=0)	CheckDlgButton(hDlg,11107,BST_CHECKED);
		if((tmpOptions->HotkeyIntercept & 0x800)!=0)	CheckDlgButton(hDlg,11108,BST_CHECKED);
		if((tmpOptions->HotkeyRelease & 0x800)!=0)	CheckDlgButton(hDlg,11109,BST_CHECKED);
		if((tmpOptions->HotkeyRestore & 0x800)!=0)	CheckDlgButton(hDlg,11111,BST_CHECKED);
		SendDlgItemMessage(hDlg,11200,HKM_SETHOTKEY,(tmpOptions->HotkeyNewIdentity & 0xfff),0);
		SendDlgItemMessage(hDlg,11201,HKM_SETHOTKEY,(tmpOptions->HotkeyIntercept & 0xfff),0);
		SendDlgItemMessage(hDlg,11202,HKM_SETHOTKEY,(tmpOptions->HotkeyRelease & 0xfff),0);
		SendDlgItemMessage(hDlg,11203,HKM_SETHOTKEY,(tmpOptions->HotkeyRestore & 0xfff),0);
		if((tmpOptions->HotkeyNewIdentity & 0x1000)==0)
		{	EnableWindow(GetDlgItem(hDlg,11107),0);
			EnableWindow(GetDlgItem(hDlg,11200),0);
		}
		else	CheckDlgButton(hDlg,11104,BST_CHECKED);
		if((tmpOptions->HotkeyIntercept & 0x1000)==0)
		{	EnableWindow(GetDlgItem(hDlg,11108),0);
			EnableWindow(GetDlgItem(hDlg,11201),0);
		}
		else	CheckDlgButton(hDlg,11105,BST_CHECKED);
		if((tmpOptions->HotkeyRelease& 0x1000)==0)
		{	EnableWindow(GetDlgItem(hDlg,11109),0);
			EnableWindow(GetDlgItem(hDlg,11202),0);
		}
		else	CheckDlgButton(hDlg,11106,BST_CHECKED);
		if((tmpOptions->HotkeyRestore& 0x1000)==0)
		{	EnableWindow(GetDlgItem(hDlg,11111),0);
			EnableWindow(GetDlgItem(hDlg,11203),0);
		}
		else	CheckDlgButton(hDlg,11110,BST_CHECKED);
	}
	else if(uMsg==WM_COMMAND)
	{
		if(LOWORD(wParam)==11300)
		{	if((HIWORD(wParam)==CBN_EDITCHANGE)||(HIWORD(wParam)==CBN_EDITUPDATE))
			{	int tmpsize=SendMessage(hEdit1,WM_GETTEXTLENGTH,0,0);
				char *tmp1=tor_malloc(tmpsize+2);
				SendMessage(hEdit1,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
				char *tmp2=tmpOptions->winver;
				tmpOptions->winver=tmp1;tor_free(tmp2);
			}
			else if(HIWORD(wParam)==CBN_SELCHANGE)
			{	char *tmp1=tor_malloc(512);
				SendDlgItemMessage(hDlg,11300,CB_GETLBTEXT,SendDlgItemMessage(hDlg,11300,CB_GETCURSEL,0,0),(LPARAM)tmp1);
				char *tmp2=tmpOptions->winver;
				tmpOptions->winver=tmp1;tor_free(tmp2);
			}
		}
		else if((LOWORD(wParam)==11301)&&(HIWORD(wParam)==CBN_SELCHANGE))
		{	if(tmpOptions->Language) tor_free(tmpOptions->Language);
			tmpOptions->Language=tor_malloc(100);
			tmpOptions->Language[0]=0;
			SendDlgItemMessage(hDlg,11301,CB_GETLBTEXT,SendDlgItemMessage(hDlg,11301,CB_GETCURSEL,0,0),(LPARAM)tmpOptions->Language);
			if(load_lng(tmpOptions->Language))
			{	setNewLanguage();
				if(get_lang_str(LANG_STR_ABOUT_LNG))	log(LOG_WARN,LD_APP,get_lang_str(LANG_STR_ABOUT_LNG));
			}
		}
		else if(LOWORD(wParam)==11400)
		{	if(IsDlgButtonChecked(hDlg,11400)==BST_CHECKED)	tmpOptions->AutoStart|=1;
			else	tmpOptions->AutoStart&=0xfe;
		}
		else if(LOWORD(wParam)==11401)
		{	if(IsDlgButtonChecked(hDlg,11401)==BST_CHECKED)	tmpOptions->AutoStart|=2;
			else	tmpOptions->AutoStart&=0xff^2;
		}
		else if(LOWORD(wParam)==11402)
		{	HKEY hKey;
			char *tmp1,*tmp2=tor_malloc(4);
			if(IsDlgButtonChecked(hDlg,11402)==BST_CHECKED)
			{	tmpOptions->AutoStart|=4;
				CheckDlgButton(hDlg,11402,BST_CHECKED);
				tmp1=tor_malloc(MAX_PATH+1);GetModuleFileName(0,tmp1,MAX_PATH);
				int tmpsize=strlen(tmp1)+1;
				if(RegCreateKeyEx(HKEY_CURRENT_USER,runkey,0,(LPTSTR)&voidmsg,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,(PHKEY)&hKey,(LPDWORD)tmp2)==ERROR_SUCCESS)
				{	RegSetValueEx(hKey,valuename,0,REG_SZ,tmp1,tmpsize);
					RegCloseKey(hKey);
				}
				tor_free(tmp1);
			}
			else
			{	tmpOptions->AutoStart&=0xff^4;
				if(RegCreateKeyEx(HKEY_CURRENT_USER,runkey,0,(LPTSTR)&voidmsg,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,(PHKEY)&hKey,(LPDWORD)tmp2)==ERROR_SUCCESS)
				{	RegDeleteValue(hKey,valuename);
					RegCloseKey(hKey);
				}
			}
			tor_free(tmp2);
		}
		else if(LOWORD(wParam)==11403)
		{	if(IsDlgButtonChecked(hDlg,11403)==BST_CHECKED)	tmpOptions->HardwareAccel=1;
			else	tmpOptions->HardwareAccel=0;
		}
		else if(LOWORD(wParam)==11404)
		{	if(IsDlgButtonChecked(hDlg,11404)==BST_CHECKED)	tmpOptions->AvoidDiskWrites=1;
			else	tmpOptions->AvoidDiskWrites=0;
		}
		else if(LOWORD(wParam)==11405)
		{	if(IsDlgButtonChecked(hDlg,11405)==BST_CHECKED)
			{	tmpOptions->ControlPort=GetDlgItemInt(hDlg,11101,0,0);
				EnableWindow(GetDlgItem(hDlg,11101),1);EnableWindow(GetDlgItem(hDlg,11012),1);EnableWindow(GetDlgItem(hDlg,11102),1);EnableWindow(GetDlgItem(hDlg,11406),1);
				if(tmpOptions->CookieAuthentication)	EnableWindow(GetDlgItem(hDlg,11103),1);
				retry_all_listeners(0,0);
			}
			else
			{	tmpOptions->ControlPort=0; EnableWindow(GetDlgItem(hDlg,11101),0);EnableWindow(GetDlgItem(hDlg,11012),0);EnableWindow(GetDlgItem(hDlg,11102),0);EnableWindow(GetDlgItem(hDlg,11406),0);EnableWindow(GetDlgItem(hDlg,11103),0);
				retry_all_listeners(0,0);
			}
		}
		else if(LOWORD(wParam)==11406)
		{	if(IsDlgButtonChecked(hDlg,11406)==BST_CHECKED)
			{	tmpOptions->CookieAuthentication=1;EnableWindow(GetDlgItem(hDlg,11103),1);}
			else
			{	tmpOptions->CookieAuthentication=0;EnableWindow(GetDlgItem(hDlg,11103),0);}
		}
		else if((LOWORD(wParam)==11101)&&(HIWORD(wParam)==EN_CHANGE))
		{	tmpOptions->ControlPort=GetDlgItemInt(hDlg,11101,0,0);
			retry_all_listeners(0,0);
		}
		else if((LOWORD(wParam)==11102)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData(hDlg,11102,&tmpOptions->ControlListenAddress,"ControlListenAddress");
			retry_all_listeners(0,0);
		}
		else if((LOWORD(wParam)==11103)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData(hDlg,11103,&tmpOptions->HashedControlPassword,"HashedControlPassword");
		}
		else if((LOWORD(wParam)==11100)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,11100,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,11100,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->SelectedTorVer;
			tmpOptions->SelectedTorVer=tmp1;
			if(tmpOptions->SelectedTorVer[0]!='<')
			{	tmp1=tmpOptions->torver;
				tmpOptions->torver=tor_strdup(tmpOptions->SelectedTorVer);
				if(tmp1)	tor_free(tmp1);
			}
			tor_free(tmp2);
		}
		else if(LOWORD(wParam)==11104)
		{	if(IsDlgButtonChecked(hDlg,11104)==BST_CHECKED)
			{	tmpOptions->HotkeyNewIdentity|=0x1000;
				EnableWindow(GetDlgItem(hDlg,11107),1);EnableWindow(GetDlgItem(hDlg,11200),1);
			}
			else
			{	tmpOptions->HotkeyNewIdentity&=0xffff^0x1000;
				EnableWindow(GetDlgItem(hDlg,11107),0);EnableWindow(GetDlgItem(hDlg,11200),0);
			}
			tmpOptions->HotkeyNewIdentity &= 0xf800;
			tmpOptions->HotkeyNewIdentity |= SendDlgItemMessage(hDlg,11200,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11105)
		{	if(IsDlgButtonChecked(hDlg,11105)==BST_CHECKED)
			{	tmpOptions->HotkeyIntercept|=0x1000;
				EnableWindow(GetDlgItem(hDlg,11108),1);EnableWindow(GetDlgItem(hDlg,11201),1);
			}
			else
			{	tmpOptions->HotkeyIntercept&=0xffff^0x1000;
				EnableWindow(GetDlgItem(hDlg,11108),0);EnableWindow(GetDlgItem(hDlg,11201),0);
			}
			tmpOptions->HotkeyIntercept &= 0xf800;
			tmpOptions->HotkeyIntercept |= SendDlgItemMessage(hDlg,11201,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11106)
		{	if(IsDlgButtonChecked(hDlg,11106)==BST_CHECKED)
			{	tmpOptions->HotkeyRelease|=0x1000;
				EnableWindow(GetDlgItem(hDlg,11109),1);EnableWindow(GetDlgItem(hDlg,11202),1);
			}
			else
			{	tmpOptions->HotkeyRelease&=0xffff^0x1000;
				EnableWindow(GetDlgItem(hDlg,11109),0);EnableWindow(GetDlgItem(hDlg,11202),0);
			}
			tmpOptions->HotkeyRelease &= 0xf800;
			tmpOptions->HotkeyRelease |= SendDlgItemMessage(hDlg,11202,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11110)
		{	if(IsDlgButtonChecked(hDlg,11110)==BST_CHECKED)
			{	tmpOptions->HotkeyRestore|=0x1000;
				EnableWindow(GetDlgItem(hDlg,11111),1);EnableWindow(GetDlgItem(hDlg,11203),1);
			}
			else
			{	tmpOptions->HotkeyRestore&=0xffff^0x1000;
				EnableWindow(GetDlgItem(hDlg,11111),0);EnableWindow(GetDlgItem(hDlg,11203),0);
			}
			tmpOptions->HotkeyRestore &= 0xf800;
			tmpOptions->HotkeyRestore |= SendDlgItemMessage(hDlg,11203,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11107)
		{	if(IsDlgButtonChecked(hDlg,11107)==BST_CHECKED)	tmpOptions->HotkeyNewIdentity |= 0x800;
			else	tmpOptions->HotkeyNewIdentity &= 0xffff ^ 0x800;
			tmpOptions->HotkeyNewIdentity &= 0xf800;
			tmpOptions->HotkeyNewIdentity |= SendDlgItemMessage(hDlg,11200,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11108)
		{	if(IsDlgButtonChecked(hDlg,11108)==BST_CHECKED)	tmpOptions->HotkeyIntercept |= 0x800;
			else	tmpOptions->HotkeyIntercept &= 0xffff ^ 0x800;
			tmpOptions->HotkeyIntercept &= 0xf800;
			tmpOptions->HotkeyIntercept |= SendDlgItemMessage(hDlg,11201,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11109)
		{	if(IsDlgButtonChecked(hDlg,11109)==BST_CHECKED)	tmpOptions->HotkeyRelease |= 0x800;
			else	tmpOptions->HotkeyRelease &= 0xffff ^ 0x800;
			tmpOptions->HotkeyRelease &= 0xf800;
			tmpOptions->HotkeyRelease |= SendDlgItemMessage(hDlg,11202,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11111)
		{	if(IsDlgButtonChecked(hDlg,11111)==BST_CHECKED)	tmpOptions->HotkeyRestore |= 0x800;
			else	tmpOptions->HotkeyRestore &= 0xffff ^ 0x800;
			tmpOptions->HotkeyRestore &= 0xf800;
			tmpOptions->HotkeyRestore |= SendDlgItemMessage(hDlg,11203,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11200)
		{	tmpOptions->HotkeyNewIdentity &= 0xf800;
			tmpOptions->HotkeyNewIdentity |= SendDlgItemMessage(hDlg,11200,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11201)
		{	tmpOptions->HotkeyIntercept &= 0xf800;
			tmpOptions->HotkeyIntercept |= SendDlgItemMessage(hDlg,11201,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11202)
		{	tmpOptions->HotkeyRelease &= 0xf800;
			tmpOptions->HotkeyRelease |= SendDlgItemMessage(hDlg,11202,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
		else if(LOWORD(wParam)==11203)
		{	tmpOptions->HotkeyRestore &= 0xf800;
			tmpOptions->HotkeyRestore |= SendDlgItemMessage(hDlg,11203,HKM_GETHOTKEY,0,0) & 0x7ff;
			dlgSystem_RegisterHotKeys();
		}
	}
	return 0;
}
