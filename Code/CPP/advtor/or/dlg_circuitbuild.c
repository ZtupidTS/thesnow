#include "or.h"
#include "dlg_util.h"

HWND hDlgCircuitBuild=NULL;
extern HWND hMainDialog;
extern int frame;
//int frame6[]={14400,14100,14401,14101,14402,14102,14403,14103,14404,14104,14407,14105,14050,14051,14106,14107,14408,14108,14409,14109,14410,14110,14300,14001,-1};
lang_dlg_info lang_dlg_circuitbuild[]={
	{14400,LANG_DLG_CIRCUIT_BUILD_TIMEOUT},
	{14401,LANG_DLG_CIRCUIT_IDLE_TIMEOUT},
	{14402,LANG_DLG_MAXIMUM_PREDICTED_CIRCS},
	{14403,LANG_DLG_CIRCUIT_BUILD_PERIOD},
	{14404,LANG_DLG_CIRCUIT_EXPIRATION},
	{14407,LANG_DLG_CIRCUIT_TIMEOUT_EXIT},
	{14410,LANG_DLG_ENTRY_GUARDS_USE},
	{14001,LANG_DLG_ENTRY_GUARDS_REINIT},
	{14050,LANG_DLG_FORCED_EXIT_HOSTS},
	{14051,LANG_DLG_NODE_FAMILIES},
	{14408,LANG_DLG_TRACKED_HOSTS},
	{14409,LANG_DLG_TRACKED_HOSTS_EXPIRE},
	{0,0}
};

extern or_options_t *tmpOptions;
extern smartlist_t *entry_guards;

int __stdcall dlgCircuitBuild(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

void addentryguards(void)
{	if(!hDlgCircuitBuild) return;
	SendDlgItemMessage(hDlgCircuitBuild,14300,CB_RESETCONTENT,0,0);
	if(entry_guards)
		SMARTLIST_FOREACH(entry_guards, entry_guard_t *, entry,
		{	SendDlgItemMessage(hDlgCircuitBuild,14300,CB_ADDSTRING,0,(LPARAM)(entry->nickname));
		});
}

void dlgCircuitBuild_trackedHostExitAdd(HWND hDlg,char *newAddr)
{	if(!hDlgCircuitBuild)
	{	hDlgCircuitBuild=createChildDialog(hMainDialog,1103,&dlgCircuitBuild);
		if(frame!=1103) ShowWindow(hDlgCircuitBuild,SW_HIDE);
	}
	int tmpsize=SendDlgItemMessage(hDlgCircuitBuild,14108,WM_GETTEXTLENGTH,0,0);
	char *tmp2=tor_malloc(tmpsize+256+5),*tmp3;tmp3=tmp2;
	GetDlgItemText(hDlgCircuitBuild,14108,tmp2,tmpsize+1);tmp2+=tmpsize;
	if((tmpsize<2)||(tmp3[tmpsize-1]>32)){ *tmp2++=13;*tmp2++=10;}
	if(newAddr[0])
	{	int i;
		for(i=0;(newAddr[i]!=0)&&(newAddr[i]!=':');i++)	*tmp2++=newAddr[i];
		*tmp2++=13;*tmp2++=10;*tmp2++=0;
		SetDlgItemText(hDlgCircuitBuild,14108,tmp3);
		if(tmpOptions->TrackHostExits)
		{	SMARTLIST_FOREACH(tmpOptions->TrackHostExits, char *, cp, tor_free(cp));
			smartlist_clear(tmpOptions->TrackHostExits);
		}
		else	tmpOptions->TrackHostExits=smartlist_create();
		smartlist_split_string(tmpOptions->TrackHostExits, tmp3, "\r\n",SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
		tor_snprintf(tmp3,256,get_lang_str(LANG_LOG_CIRCUITUSE_REGISTERING_NEW_TRACKED_EXIT),newAddr);
		log(LOG_NOTICE,LD_APP,tmp3);
		LangMessageBox(hDlg,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	}
	tor_free(tmp3);
	addressmap_clear_transient();
}

void dlgCircuitBuild_trackedDomainExitAdd(HWND hDlg,char *newAddr)
{	if(!hDlgCircuitBuild)
	{	hDlgCircuitBuild=createChildDialog(hMainDialog,1103,&dlgCircuitBuild);
		if(frame!=1103) ShowWindow(hDlgCircuitBuild,SW_HIDE);
	}
	int tmpsize=SendDlgItemMessage(hDlgCircuitBuild,14108,WM_GETTEXTLENGTH,0,0);
	char *tmp2=tor_malloc(tmpsize+256+5),*tmp3;tmp3=tmp2;
	GetDlgItemText(hDlgCircuitBuild,14108,tmp2,tmpsize+1);tmp2+=tmpsize;
	if((tmpsize<2)||(tmp3[tmpsize-1]>32)){ *tmp2++=13;*tmp2++=10;}
	if(newAddr[0])
	{	int i;
		char *s1=newAddr,*s2;
		s2=s1;
		while(*s2)	s2++;
		while(s2>s1)
		{	s2--;
			if(*s2=='.') break;
		}
		while(s2>s1)
		{	s2--;
			if(*s2=='.') break;
		}
		for(i=0;(s2[i]!=0)&&(s2[i]!=':');i++)	*tmp2++=s2[i];
		*tmp2++=13;*tmp2++=10;*tmp2++=0;
		SetDlgItemText(hDlgCircuitBuild,14108,tmp3);
		if(tmpOptions->TrackHostExits)
		{	SMARTLIST_FOREACH(tmpOptions->TrackHostExits, char *, cp, tor_free(cp));
			smartlist_clear(tmpOptions->TrackHostExits);
		}
		else	tmpOptions->TrackHostExits=smartlist_create();
		smartlist_split_string(tmpOptions->TrackHostExits, tmp3, "\r\n",SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
		tor_snprintf(tmp3,256,get_lang_str(LANG_LOG_CIRCUITUSE_REGISTERING_NEW_TRACKED_EXIT),s2);
		log(LOG_NOTICE,LD_APP,tmp3);
		LangMessageBox(hDlg,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	}
	tor_free(tmp3);
	addressmap_clear_transient();
}

void dlgCircuitBuild_addressMapAdd(HWND hDlg,char *newAddr)
{	char *tmp1a,*tmp1b;
	if(!hDlgCircuitBuild)
	{	hDlgCircuitBuild=createChildDialog(hMainDialog,1103,&dlgCircuitBuild);
		if(frame!=1103) ShowWindow(hDlgCircuitBuild,SW_HIDE);
	}
	int tmpsize=SendDlgItemMessage(hDlgCircuitBuild,14106,WM_GETTEXTLENGTH,0,0);
	char *tmp2=tor_malloc(tmpsize+256+5),*tmp1,*tmp3;tmp3=tmp2;
	int i;
	GetDlgItemText(hDlgCircuitBuild,14106,tmp2,tmpsize+1);tmp2+=tmpsize;
	tmp1=tmp2;
	for(i=0;(newAddr[i]!=0)&&(newAddr[i]!=':');i++)	*tmp2++=newAddr[i];
	*tmp2++=0;tmp1a=tmp2;
	for(;*tmp1!=0;tmp1++,tmp2++) *tmp2=*tmp1;
	*tmp2=0;*tmp1++=0x20;
	tmp1b=circuit_find_most_recent_exit(tmp1);
	if(tmp1b)
	{	*tmp2++='.';
		for(i=0;tmp1b[i];i++)	*tmp2++=tmp1b[i];
		tor_free(tmp1b);
		*tmp2++='.';*tmp2++='e';*tmp2++='x';*tmp2++='i';*tmp2++='t';*tmp2=0;
		log(LOG_ADDR,LD_APP,get_lang_str(LANG_LOG_DLG_ADDRESSMAP_ADDED),tmp1a);
		*tmp2++=13;*tmp2++=10;*tmp2++=0;
		SetDlgItemText(hDlgCircuitBuild,14106,tmp3);
		getEditData1(hDlgCircuitBuild,14106,&tmpOptions->AddressMap,"AddressMap");
		config_register_addressmaps(tmpOptions);
		parse_virtual_addr_network(tmpOptions->VirtualAddrNetwork, 0, 0);
		tor_snprintf(tmp3,256,get_lang_str(LANG_LOG_DLG_ADDRESSMAP_ADDED),newAddr);
		LangMessageBox(hDlg,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	}
	else
	{	log(LOG_WARN,LD_APP,get_lang_str(LANG_LOG_DLG_NO_EXIT_FOUND),tmp1,tmp1);
		tor_snprintf(tmp3,256,get_lang_str(LANG_LOG_DLG_NO_EXIT_FOUND),tmp1,tmp1);
		LangMessageBox(hDlg,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	}
	tor_free(tmp3);
	addressmap_clear_transient();
}

void dlgCircuitBuild_addressMapRemove(HWND hDlg,char *newAddr)
{	char *tmp1a;
	if(!hDlgCircuitBuild)
	{	hDlgCircuitBuild=createChildDialog(hMainDialog,1103,&dlgCircuitBuild);
		if(frame!=1103) ShowWindow(hDlgCircuitBuild,SW_HIDE);
	}
	int tmpsize=SendDlgItemMessage(hDlgCircuitBuild,14106,WM_GETTEXTLENGTH,0,0);
	char *tmp2=tor_malloc(tmpsize+256+5),*tmp1,*tmp3;tmp3=tmp2;
	int i;
	GetDlgItemText(hDlgCircuitBuild,14106,tmp2,tmpsize+1);tmp2+=tmpsize+1;
	tmp1=tmp2;
	for(i=0;(newAddr[i]!=0)&&(newAddr[i]!=':');i++)	*tmp2++=newAddr[i]|0x20;
	*tmp2++=0;tmp1a=tmp3;tmp2=tmp1;
	while(*tmp1a)
	{	for(i=0;tmp2[i];i++)
		{	if(tmp2[i] != (tmp1a[i]|0x20)) break;
		}
		if((tmp2[i]==0)&&(tmp1a[i]<33))	break;
		while(*tmp1a>32) tmp1a++;
		while(*tmp1a<33 && *tmp1a) tmp1a++;
	}
	if(*tmp1a)
	{	for(i=0;tmp1a[i]&&tmp1a[i]!=0x0d&&tmp1a[i]!=0x0a;i++)	;
		if(tmp1a[i]==0x0d) i++;
		if(tmp1a[i]==0x0a) i++;
		while(tmp1a[i]){	*tmp1a=tmp1a[i];tmp1a++;}
		*tmp1a=0;
		log(LOG_ADDR,LD_APP,get_lang_str(LANG_LOG_DLG_ADDRESSMAP_REMOVED),tmp1);
		SetDlgItemText(hDlgCircuitBuild,14106,tmp3);
		getEditData1(hDlgCircuitBuild,14106,&tmpOptions->AddressMap,"AddressMap");
		config_register_addressmaps(tmpOptions);
		parse_virtual_addr_network(tmpOptions->VirtualAddrNetwork, 0, 0);
		tor_snprintf(tmp3,256,get_lang_str(LANG_LOG_DLG_ADDRESSMAP_REMOVED),newAddr);
		LangMessageBox(hDlg,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	}
	else
	{	log(LOG_ADDR,LD_APP,get_lang_str(LANG_LOG_DLG_ADDRESSMAP_ENTRY_NOT_FOUND),tmp1);
		tor_snprintf(tmp3,256,get_lang_str(LANG_LOG_DLG_ADDRESSMAP_ENTRY_NOT_FOUND),newAddr);
		LangMessageBox(hDlg,tmp3,LANG_LB_CONNECTIONS,MB_OK);
	}
	tor_free(tmp3);
	addressmap_clear_transient();
}

int __stdcall dlgCircuitBuild(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgCircuitBuild=hDlg;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_circuitbuild);
		}
		SetDlgItemInt(hDlg,14100,tmpOptions->CircuitBuildTimeout,0);
		SetDlgItemInt(hDlg,14101,tmpOptions->CircuitIdleTimeout,0);
		SetDlgItemInt(hDlg,14102,tmpOptions->MaxUnusedOpenCircuits,0);
		SetDlgItemInt(hDlg,14103,tmpOptions->NewCircuitPeriod,0);
		SetDlgItemInt(hDlg,14104,tmpOptions->MaxCircuitDirtiness,0);
		SetDlgItemInt(hDlg,14105,tmpOptions->ShutdownWaitLength,0);
		SetDlgItemInt(hDlg,14110,tmpOptions->NumEntryGuards,0);
		SetDlgItemInt(hDlg,14109,tmpOptions->TrackHostExitsExpire,0);
		if(tmpOptions->CircuitBuildTimeout){ CheckDlgButton(hDlg,14400,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,14100),0);
		if(tmpOptions->CircuitIdleTimeout){ CheckDlgButton(hDlg,14401,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,14101),0);
		if(tmpOptions->MaxUnusedOpenCircuits){ CheckDlgButton(hDlg,14402,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,14102),0);
		if(tmpOptions->NewCircuitPeriod){ CheckDlgButton(hDlg,14403,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,14103),0);
		if(tmpOptions->MaxCircuitDirtiness){ CheckDlgButton(hDlg,14404,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,14104),0);
		if(tmpOptions->ShutdownWaitLength){ CheckDlgButton(hDlg,14407,BST_CHECKED);}
		else	EnableWindow(GetDlgItem(hDlg,14105),0);
		if(tmpOptions->UseEntryGuards){ CheckDlgButton(hDlg,14410,BST_CHECKED);}
		else{	EnableWindow(GetDlgItem(hDlg,14110),0);EnableWindow(GetDlgItem(hDlg,14300),0);EnableWindow(GetDlgItem(hDlg,14001),0);}
		setEditData(hDlg,14106,tmpOptions->AddressMap);
		setEditData(hDlg,14107,tmpOptions->NodeFamilies);
		if(tmpOptions->TrackHostExits)
		{	char *tmp1=smartlist_join_strings(tmpOptions->TrackHostExits, "\r\n", 0, NULL);
			SetDlgItemText(hDlg,14108,tmp1);
			tor_free(tmp1);
			CheckDlgButton(hDlg,14408,BST_CHECKED);
		}
		else
		{	EnableWindow(GetDlgItem(hDlg,14108),0);
			EnableWindow(GetDlgItem(hDlg,14109),0);
			EnableWindow(GetDlgItem(hDlg,14409),0);
		}
		addentryguards();
	}
	else if(uMsg==WM_COMMAND)
	{
		if(LOWORD(wParam)==14400)
		{	if(IsDlgButtonChecked(hDlg,14400)==BST_CHECKED)
			{	tmpOptions->CircuitBuildTimeout=GetDlgItemInt(hDlg,14100,0,0);EnableWindow(GetDlgItem(hDlg,14100),1);
			}
			else
			{	tmpOptions->CircuitBuildTimeout=0;EnableWindow(GetDlgItem(hDlg,14100),0);
			}
		}
		else if(LOWORD(wParam)==14401)
		{	if(IsDlgButtonChecked(hDlg,14401)==BST_CHECKED)
			{	tmpOptions->CircuitIdleTimeout=GetDlgItemInt(hDlg,14101,0,0);EnableWindow(GetDlgItem(hDlg,14101),1);
			}
			else
			{	tmpOptions->CircuitIdleTimeout=0;EnableWindow(GetDlgItem(hDlg,14101),0);
			}
		}
		else if(LOWORD(wParam)==14402)
		{	if(IsDlgButtonChecked(hDlg,14402)==BST_CHECKED)
			{	tmpOptions->MaxUnusedOpenCircuits=GetDlgItemInt(hDlg,14102,0,0);EnableWindow(GetDlgItem(hDlg,14102),1);
			}
			else
			{	tmpOptions->MaxUnusedOpenCircuits=0;EnableWindow(GetDlgItem(hDlg,14102),0);
			}
		}
		else if(LOWORD(wParam)==14403)
		{	if(IsDlgButtonChecked(hDlg,14403)==BST_CHECKED)
			{	tmpOptions->NewCircuitPeriod=GetDlgItemInt(hDlg,14103,0,0);EnableWindow(GetDlgItem(hDlg,14103),1);
			}
			else
			{	tmpOptions->NewCircuitPeriod=0;EnableWindow(GetDlgItem(hDlg,14103),0);
			}
		}
		else if(LOWORD(wParam)==14404)
		{	if(IsDlgButtonChecked(hDlg,14404)==BST_CHECKED)
			{	tmpOptions->MaxCircuitDirtiness=GetDlgItemInt(hDlg,14104,0,0);EnableWindow(GetDlgItem(hDlg,14104),1);
			}
			else
			{	tmpOptions->MaxCircuitDirtiness=0;EnableWindow(GetDlgItem(hDlg,14104),0);
			}
		}
		else if(LOWORD(wParam)==14407)
		{	if(IsDlgButtonChecked(hDlg,14407)==BST_CHECKED)
			{	tmpOptions->ShutdownWaitLength=GetDlgItemInt(hDlg,14105,0,0);EnableWindow(GetDlgItem(hDlg,14105),1);
			}
			else
			{	tmpOptions->ShutdownWaitLength=0;EnableWindow(GetDlgItem(hDlg,14105),0);
			}
		}
		else if(LOWORD(wParam)==14410)
		{	if(IsDlgButtonChecked(hDlg,14410)==BST_CHECKED)
			{	tmpOptions->UseEntryGuards=GetDlgItemInt(hDlg,14110,0,0);EnableWindow(GetDlgItem(hDlg,14110),1);EnableWindow(GetDlgItem(hDlg,14300),1);EnableWindow(GetDlgItem(hDlg,14001),1);
			}
			else
			{	tmpOptions->UseEntryGuards=0;EnableWindow(GetDlgItem(hDlg,14110),0);EnableWindow(GetDlgItem(hDlg,14300),0);EnableWindow(GetDlgItem(hDlg,14001),0);
			}
		}
		else if((LOWORD(wParam)==14100)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->CircuitBuildTimeout=GetDlgItemInt(hDlg,14100,0,0);
		else if((LOWORD(wParam)==14110)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->NumEntryGuards=GetDlgItemInt(hDlg,14110,0,0);
		else if((LOWORD(wParam)==14109)&&(HIWORD(wParam)==EN_CHANGE))
		{	tmpOptions->TrackHostExitsExpire=GetDlgItemInt(hDlg,14109,0,0);
		}
		else if((LOWORD(wParam)==14101)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->CircuitIdleTimeout=GetDlgItemInt(hDlg,14101,0,0);
		else if((LOWORD(wParam)==14102)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->MaxUnusedOpenCircuits=GetDlgItemInt(hDlg,14102,0,0);
		else if((LOWORD(wParam)==14103)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->NewCircuitPeriod=GetDlgItemInt(hDlg,14103,0,0);
		else if((LOWORD(wParam)==14104)&&(HIWORD(wParam)==EN_CHANGE))
		{	tmpOptions->MaxCircuitDirtiness=GetDlgItemInt(hDlg,14104,0,0);
			if(!tmpOptions->MaxCircuitDirtiness) tmpOptions->MaxCircuitDirtiness++;
		}
		else if((LOWORD(wParam)==14105)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->ShutdownWaitLength=GetDlgItemInt(hDlg,14105,0,0);
		else if((LOWORD(wParam)==14106)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData1(hDlg,14106,&tmpOptions->AddressMap,"AddressMap");
			addressmap_clear_transient();
			config_register_addressmaps(tmpOptions);
			parse_virtual_addr_network(tmpOptions->VirtualAddrNetwork, 0, 0);
		}
		else if((LOWORD(wParam)==14107)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData1(hDlg,14107,&tmpOptions->NodeFamilies,"NodeFamilies");
			config_register_addressmaps(tmpOptions);
			parse_virtual_addr_network(tmpOptions->VirtualAddrNetwork, 0, 0);
		}
		else if(LOWORD(wParam)==14408)
		{	if(IsDlgButtonChecked(hDlg,14408)==BST_CHECKED)
			{	char *tmp1=tor_malloc(32768);
				GetDlgItemText(hDlg,14108,tmp1,32767);
				if(tmpOptions->TrackHostExits)
				{	SMARTLIST_FOREACH(tmpOptions->TrackHostExits, char *, cp, tor_free(cp));
					smartlist_clear(tmpOptions->TrackHostExits);
				}
				else	tmpOptions->TrackHostExits=smartlist_create();
				smartlist_split_string(tmpOptions->TrackHostExits, tmp1, "\r\n",SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
				tor_free(tmp1);
				EnableWindow(GetDlgItem(hDlg,14108),1);
				EnableWindow(GetDlgItem(hDlg,14409),1);
				EnableWindow(GetDlgItem(hDlg,14109),1);
			}
			else
			{	if(tmpOptions->TrackHostExits)
				{	SMARTLIST_FOREACH(tmpOptions->TrackHostExits, char *, cp, tor_free(cp));
					smartlist_clear(tmpOptions->TrackHostExits);
				}
				tmpOptions->TrackHostExits=NULL;
				EnableWindow(GetDlgItem(hDlg,14108),0);
				EnableWindow(GetDlgItem(hDlg,14109),0);
				EnableWindow(GetDlgItem(hDlg,14409),0);
			}
		}
		else if((LOWORD(wParam)==14108)&&(HIWORD(wParam)==EN_CHANGE))
		{	char *tmp1=tor_malloc(32768);
			GetDlgItemText(hDlg,14108,tmp1,32767);
			if(tmpOptions->TrackHostExits)
			{	SMARTLIST_FOREACH(tmpOptions->TrackHostExits, char *, cp, tor_free(cp));
				smartlist_clear(tmpOptions->TrackHostExits);
			}
			else	tmpOptions->TrackHostExits=smartlist_create();
			smartlist_split_string(tmpOptions->TrackHostExits, tmp1, "\r\n",SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
			addressmap_clear_transient();
			tor_free(tmp1);
		}
		else if(LOWORD(wParam)==14001)
		{	entry_guards_free_all();addentryguards();}
	}
	return 0;
}
