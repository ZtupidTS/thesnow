#include "or.h"
#include "dlg_util.h"
#include "plugins.h"


//int frame11[]={19010,19100,19011,19101,19001,19012,19102,19013,19103,19400,19003,19004,19005,19014,19104,19015,19105,19016,19106,19017,19107,19018,19108,19019,19109,19020,-1};
lang_dlg_info lang_dlg_hidden_services[]={
	{19010,LANG_DLG_HS_REAL_PORT},
	{19011,LANG_DLG_HS_REAL_ADDR},
	{19001,LANG_DLG_HS_SELECT_PORT},
	{19012,LANG_DLG_HS_VIRTUAL_PORT},
	{19013,LANG_DLG_HS_ONION_ADDR},
	{19003,LANG_DLG_HS_ADD},
	{19004,LANG_DLG_HS_DELETE},
	{19005,LANG_DLG_HS_PUBLISH_NOW},
	{19014,LANG_DLG_HS_INTRO},
	{19015,LANG_DLG_HS_BUILD_PERIOD},
	{19016,LANG_DLG_HS_MAX_INTRO},
	{19017,LANG_DLG_HS_MAX_REND_FAIL},
	{19018,LANG_DLG_HS_REND_TIMEOUT},
	{19019,LANG_DLG_HS_PUBLISH_PERIOD},
	{0,0}
};

lang_dlg_info lang_dlg_get_hs_key[]={
	{10,LANG_HS_ADDRESS },
	{50,LANG_HS_PRIVATE_KEY},
	{1,LANG_HS_OK},
	{2,LANG_HS_CANCEL},
	{0,0}
};

HWND hDlgHiddenServices=NULL;
extern LPFN11 ShowOpenPorts;
LV_ITEM lvit;
extern or_options_t *tmpOptions;
extern HINSTANCE hInstance;
extern HWND hMainDialog;
extern HANDLE hLibrary;
void *get_plugins_hs(void);

rend_service_t *service;
int __stdcall dlgHSKey(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	if(LangGetLanguage())
		{	SetWindowTextL(hDlg,LANG_HS_USE_EXISTING);
			changeDialogStrings(hDlg,lang_dlg_get_hs_key);
		}
		service=(rend_service_t *)lParam;
		if(service)
		{	char *address=tor_malloc(100);
			tor_snprintf(address,100,"%s.onion",service->service_id);
			SetDlgItemText(hDlg,100,address);
			tor_free(address);
		}
	}
	else if(uMsg==WM_COMMAND)
	{	if(LOWORD(wParam)==1)
		{	char *keystr=tor_malloc(8192);
			*keystr=0;
			GetDlgItemText(hDlg,101,keystr,8192);
			if(service) set_hs_key(keystr,service);
			tor_free(keystr);
			EndDialog(hDlg,1);
		}
		else if(LOWORD(wParam)==2)
		{	EndDialog(hDlg,0);
		}
	}
	return 0;
}

void getHSKey(rend_service_t *service)
{	DialogBoxParamW(hInstance,(LPWSTR)MAKEINTRESOURCE(1004),hMainDialog,&dlgHSKey,(LPARAM)service);
}

void insertService(rend_service_t *ptr)
{	char *servPorts=tor_malloc(1024);
	char portStr[20];
	int idx,idx1,is_dll=ptr->plugin[0];
	lvit.iItem=SendDlgItemMessage(hDlgHiddenServices,19400,LVM_GETITEMCOUNT,0,0);
	lvit.iSubItem=0;lvit.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE;lvit.state=0;lvit.stateMask=0;lvit.iImage=0;
	lvit.stateMask=LVIS_STATEIMAGEMASK;lvit.state=LVIS_SELECTED;
	lvit.lParam=(unsigned long)(ptr->descriptor_version<<16)|(atoi(ptr->directory));
	lvit.pszText=servPorts;lvit.cchTextMax=1024;
	if(is_dll)	tor_snprintf(servPorts,20,"[Internal]");
	else
	{	idx=0;
		SMARTLIST_FOREACH(ptr->ports, rend_service_port_config_t *, cPort,
		{	tor_snprintf(portStr,20,"%i",cPort->real_port);
			idx1=0;
			while(idx<1023 && portStr[idx1]) servPorts[idx++]=portStr[idx1++];
			servPorts[idx++]=',';
		});
		if(idx) idx--;servPorts[idx]=0;
	}
	lvit.iItem=SendDlgItemMessage(hDlgHiddenServices,19400,LVM_INSERTITEM,0,(LPARAM)&lvit);

	lvit.iSubItem=1;lvit.mask=LVIF_TEXT;
	if(is_dll)	lvit.pszText=&ptr->plugin[0];
	else
	{	SMARTLIST_FOREACH(ptr->ports, rend_service_port_config_t *, cPort,
		{	tor_addr_to_str(servPorts,&cPort->real_addr,1024,0);
		});
	}
	lvit.cchTextMax=1024;
	SendDlgItemMessage(hDlgHiddenServices,19400,LVM_SETITEM,0,(LPARAM)&lvit);

	lvit.iSubItem=2;lvit.mask=LVIF_TEXT;
	lvit.pszText=servPorts;
	lvit.cchTextMax=1024;idx=0;
	SMARTLIST_FOREACH(ptr->ports, rend_service_port_config_t *, cPort,
	{	tor_snprintf(portStr,20,"%i",cPort->virtual_port);
		idx1=0;
		while(idx<1023 && portStr[idx1]) servPorts[idx++]=portStr[idx1++];
		servPorts[idx++]=',';
	});
	if(idx) idx--;servPorts[idx]=0;
	SendDlgItemMessage(hDlgHiddenServices,19400,LVM_SETITEM,0,(LPARAM)&lvit);

	lvit.iSubItem=3;lvit.mask=LVIF_TEXT;
	lvit.pszText=servPorts;
	lvit.cchTextMax=1024;idx=0;
	tor_snprintf(servPorts,1024,"%s.onion",ptr->service_id);
	SendDlgItemMessage(hDlgHiddenServices,19400,LVM_SETITEM,0,(LPARAM)&lvit);
	lvit.stateMask=LVIS_STATEIMAGEMASK;lvit.state=LVIS_SELECTED;
	lvit.iSubItem=0;lvit.mask=LVIF_STATE;
	SendDlgItemMessage(hDlgHiddenServices,19400,LVM_SETITEMSTATE,lvit.iItem,(LPARAM)&lvit);
	tor_free(servPorts);
}

void showService(unsigned long serviceKey)
{	rend_service_t *ptr=find_service_by_key(serviceKey);
	if(ptr==NULL) return;
	char *servPorts=tor_malloc(1024);
	char portStr[20];
	int idx,idx1;
	int is_dll=ptr->plugin[0];
	idx=0;
	if(is_dll)	tor_snprintf(servPorts,20,"[Internal]");
	else
	{	SMARTLIST_FOREACH(ptr->ports, rend_service_port_config_t *, cPort,
		{	tor_snprintf(portStr,20,"%i",cPort->real_port);
			idx1=0;
			while(idx<1023 && portStr[idx1]) servPorts[idx++]=portStr[idx1++];
			servPorts[idx++]=',';
		});
		if(idx) idx--;servPorts[idx]=0;
	}
	SetDlgItemText(hDlgHiddenServices,19100,servPorts);

	if(is_dll)	SetDlgItemText(hDlgHiddenServices,19101,ptr->plugin);
	else
	{	SMARTLIST_FOREACH(ptr->ports, rend_service_port_config_t *, cPort,
		{	tor_addr_to_str(servPorts,&cPort->real_addr,1024,0);
		});
		SetDlgItemText(hDlgHiddenServices,19101,servPorts);
	}

	idx=0;
	SMARTLIST_FOREACH(ptr->ports, rend_service_port_config_t *, cPort,
	{	tor_snprintf(portStr,20,"%i",cPort->virtual_port);
		idx1=0;
		while(idx<1023 && portStr[idx1]) servPorts[idx++]=portStr[idx1++];
		servPorts[idx++]=',';
	});
	if(idx) idx--;servPorts[idx]=0;
	SetDlgItemText(hDlgHiddenServices,19102,servPorts);

	tor_snprintf(servPorts,1024,"%s.onion",ptr->service_id);
	SetDlgItemText(hDlgHiddenServices,19103,servPorts);
	if(ptr->descriptor_version==0) LangSetDlgItemText(hDlgHiddenServices,19020,LANG_DLG_HS_VERSION_0);
	else if(ptr->descriptor_version==2) LangSetDlgItemText(hDlgHiddenServices,19020,LANG_DLG_HS_VERSION_2);
	else LangSetDlgItemText(hDlgHiddenServices,19020,LANG_DLG_HS_VERSION_ALL);
	tor_free(servPorts);
}

void clearServiceList(void)
{	SendDlgItemMessage(hDlgHiddenServices,19400,LVM_DELETEALLITEMS,0,0);
}

void dlgHiddenServices_langUpdate(void)
{	if(hDlgHiddenServices && LangGetLanguage())
	{	changeDialogStrings(hDlgHiddenServices,lang_dlg_hidden_services);
		LangSetColumn(hDlgHiddenServices,19400,81,LANG_COLUMN_HS_1,0);
		LangSetColumn(hDlgHiddenServices,19400,81,LANG_COLUMN_HS_2,1);
		LangSetColumn(hDlgHiddenServices,19400,81,LANG_COLUMN_HS_3,2);
		LangSetColumn(hDlgHiddenServices,19400,81,LANG_COLUMN_HS_4,3);
	}
}

int __stdcall dlgHiddenServices(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgHiddenServices=hDlg;
		LangInsertColumn(hDlg,19400,81,LANG_COLUMN_HS_1,0,LVCFMT_LEFT);
		LangInsertColumn(hDlg,19400,81,LANG_COLUMN_HS_2,1,LVCFMT_LEFT);
		LangInsertColumn(hDlg,19400,81,LANG_COLUMN_HS_3,2,LVCFMT_LEFT);
		LangInsertColumn(hDlg,19400,81,LANG_COLUMN_HS_4,3,LVCFMT_LEFT);
		dlgHiddenServices_langUpdate();
		SetDlgItemInt(hDlg,19104,tmpOptions->NumIntroPoints,0);
		SetDlgItemInt(hDlg,19105,tmpOptions->IntroCircRetryPeriod,0);
		SetDlgItemInt(hDlg,19106,tmpOptions->MaxCircsPerPeriod,0);
		SetDlgItemInt(hDlg,19107,tmpOptions->MaxRendFailures,0);
		SetDlgItemInt(hDlg,19108,tmpOptions->MaxRendTimeout,0);
		SetDlgItemInt(hDlg,19109,tmpOptions->RendPostPeriod,0);
		if(tmpOptions->PublishHidServDescriptors){	CheckDlgButton(hDlg,19019,BST_CHECKED);}
		else EnableWindow(GetDlgItem(hDlg,19109),0);
		if(!ShowOpenPorts) EnableWindow(GetDlgItem(hDlg,19001),0);
		SendDlgItemMessage(hDlg,19400,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_ONECLICKACTIVATE|LVS_EX_FULLROWSELECT,LVS_EX_ONECLICKACTIVATE|LVS_EX_FULLROWSELECT);
		lvit.iItem=0;
		insert_services();
	}
	else if(uMsg==WM_COMMAND)
	{
		if(LOWORD(wParam)==19001)
		{	if(!ShowOpenPorts && hLibrary) ShowOpenPorts=(LPFN11)GetProcAddress(hLibrary,"ShowOpenPorts");
			if(ShowOpenPorts)	ShowOpenPorts(hMainDialog,hDlg,get_plugins_hs());
		}
		else if(LOWORD(wParam)==19003)
		{	char *tmp1=tor_malloc(1024);GetDlgItemText(hDlg,19100,tmp1,1024);
			char *tmp2=tor_malloc(1024);GetDlgItemText(hDlg,19101,tmp2,1024);if(strlen(tmp2)<2){ tor_snprintf(tmp2,100,"127.0.0.1");SetDlgItemText(hDlg,19101,tmp2);}
			char *tmp3=tor_malloc(1024);GetDlgItemText(hDlg,19102,tmp3,1024);if(strlen(tmp3)<1){ tor_snprintf(tmp3,1024,tmp1);SetDlgItemText(hDlg,19102,tmp3);}
			char *tmp_onion=tor_malloc(1024);GetDlgItemText(hDlg,19103,tmp_onion,1024);
			int i=rend_add_new_service(tmp1,tmp3,tmp2,tmp_onion);
			if(i)
			{	unsigned long servKey=0xffff0000;servKey|=(unsigned int)i;
				rend_service_t *serv=find_service_by_key(servKey);
				if(serv)
				{	tor_free(tmp1);tmp1=tor_malloc(100);
					tor_snprintf(tmp1,100,"%s.onion",serv->service_id);
					SetDlgItemText(hDlg,19103,tmp1);
				}
			}
			tor_free(tmp1);tor_free(tmp2);tor_free(tmp3);tor_free(tmp_onion);
		}
		else if(LOWORD(wParam)==19004)
		{	lvit.iItem=SendDlgItemMessage(hDlg,19400,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			if(lvit.iItem!=-1)
			{	lvit.lParam=-1;lvit.mask=LVIF_PARAM;lvit.iSubItem=0;
				SendDlgItemMessage(hDlg,19400,LVM_GETITEM,0,(LPARAM)&lvit);
				if(lvit.lParam!=-1)	delete_service(lvit.lParam);
			}
		}
		else if(LOWORD(wParam)==19005)
		{	lvit.iItem=SendDlgItemMessage(hDlg,19400,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			if(lvit.iItem!=-1)
			{	lvit.lParam=-1;lvit.mask=LVIF_PARAM;lvit.iSubItem=0;
				SendDlgItemMessage(hDlg,19400,LVM_GETITEM,0,(LPARAM)&lvit);
				if(lvit.lParam!=-1)
				{	set_publish_time(lvit.lParam);
				}
			}
		}
		else if(((LOWORD(wParam)==19104)&&(HIWORD(wParam)==EN_CHANGE)))
			tmpOptions->NumIntroPoints=GetDlgItemInt(hDlg,19104,NULL,0);
		else if(((LOWORD(wParam)==19105)&&(HIWORD(wParam)==EN_CHANGE)))
			tmpOptions->IntroCircRetryPeriod=GetDlgItemInt(hDlg,19105,NULL,0);
		else if(((LOWORD(wParam)==19106)&&(HIWORD(wParam)==EN_CHANGE)))
			tmpOptions->MaxCircsPerPeriod=GetDlgItemInt(hDlg,19106,NULL,0);
		else if(((LOWORD(wParam)==19107)&&(HIWORD(wParam)==EN_CHANGE)))
			tmpOptions->MaxRendFailures=GetDlgItemInt(hDlg,19107,NULL,0);
		else if(((LOWORD(wParam)==19108)&&(HIWORD(wParam)==EN_CHANGE)))
			tmpOptions->MaxRendTimeout=GetDlgItemInt(hDlg,19108,NULL,0);
		else if(((LOWORD(wParam)==19109)&&(HIWORD(wParam)==EN_CHANGE)))
			tmpOptions->RendPostPeriod=GetDlgItemInt(hDlg,19109,NULL,0);
		else if(LOWORD(wParam)==19019)
		{	if(IsDlgButtonChecked(hDlg,19019)==BST_CHECKED){	tmpOptions->PublishHidServDescriptors=1;EnableWindow(GetDlgItem(hDlg,19109),1);}
			else{	tmpOptions->PublishHidServDescriptors=0;EnableWindow(GetDlgItem(hDlg,19109),0);}
		}
	}
	else if(uMsg==WM_NOTIFY)
	{	if(wParam==19400)
		{	NMLISTVIEW *pnmlv=(LPNMLISTVIEW)lParam;
			NMHDR *hdr=(LPNMHDR)lParam;
			if((hdr->code==LVN_ITEMCHANGED)&&(pnmlv->uChanged&LVIF_STATE)&&(pnmlv->iItem!=-1))
			{
				if(((pnmlv->uOldState^pnmlv->uNewState)&LVIS_SELECTED)&&(pnmlv->uNewState&LVIS_SELECTED))
				{	lvit.iItem=pnmlv->iItem;lvit.iSubItem=0;lvit.lParam=0;lvit.mask=LVIF_PARAM;
					SendDlgItemMessage(hDlg,19400,LVM_GETITEM,0,(LPARAM)&lvit);
					if(lvit.lParam)
					{	showService(lvit.lParam);
					}
				}
			}
		}
	}
	return 0;
}
