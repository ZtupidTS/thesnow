#include "or.h"
#include "dlg_util.h"

void expire_consensus(void);
void update_v2_networkstatus_cache_downloads(time_t);
void update_consensus_networkstatus_downloads(time_t);
void update_certificate_downloads(time_t);

//int frame10[]={18000,18100,18012,18020,18401,18402,18001,18403,18101,18404,18405,18010,18102,18011,18406,18407,18103,18408,18104,18014,-1};
lang_dlg_info lang_dlg_authorities[]={
	{18000,LANG_DLG_DIR_LIST},
	{18012,LANG_DLG_DIR_STATUS},
	{18401,LANG_DLG_DIR_RANDOM},
	{18402,LANG_DLG_DIR_CONSENSUS_EXP},
	{18001,LANG_DLG_DIR_REFRESH_NOW},
	{18403,LANG_DLG_DIR_RESOLUTION_TIMEOUT},
	{18404,LANG_DLG_DIR_WAIT_DESCRIPTORS},
	{18405,LANG_DLG_DIR_FAKE_TIME},
	{18010,LANG_DLG_DIR_RANDOMIZE_DELTA},
	{18011,LANG_DLG_DIR_RANDOMIZE_DELTA_UNITS},
	{18406,LANG_DLG_DIR_ROUTER_TIME},
	{18407,LANG_DLG_DIR_MAX_DL_FAILURES},
	{18408,LANG_DLG_DIR_REMOVE_CONSENSUS},
	{18014,LANG_DLG_DIR_REMOVE_CONSENSUS_UNITS},
	{0,0}
};
HWND hDlgAuthorities=NULL;
extern or_options_t *tmpOptions;
char *dirservers[] = {
	"moria1 orport=9101 no-v2 v3ident=D586D18309DED4CD6D57C18FDB97EFA96D330566 128.31.0.39:9131 9695 DFC3 5FFE B861 329B 9F1A B04C 4639 7020 CE31",
	"tor26 v1 orport=443 v3ident=14C131DFC5C6F93646BE72FA1401C02A8DF2E8B4 86.59.21.38:80 847B 1F85 0344 D787 6491 A548 92F9 0493 4E4E B85D",
	"dizum orport=443 v3ident=E8A9C45EDE6D711294FADF8E7951F4DE6CA56B58 194.109.206.212:80 7EA6 EAD6 FD83 083C 538F 4403 8BBF A077 587D D755",
	"Tonga orport=443 bridge no-v2 82.94.251.203:80 4A0C CD2D DC79 9508 3D73 F5D6 6710 0C8A 5831 F16D",
	"ides orport=9090 no-v2 v3ident=27B6B5996C426270A5C95488AA5BCEB6BCC86956 216.224.124.114:9030 F397 038A DC51 3361 35E7 B80B D99C A384 4360 292B",
	"gabelmoo orport=443 no-v2 v3ident=ED03BB616EB2F60BEC80151114BB25CEF515B226 212.112.245.170:80 F204 4413 DAC2 E02E 3D6B CF47 35A1 9BCA 1DE9 7281",
	"dannenberg orport=443 no-v2 v3ident=585769C78764D58426B8B52B6651A5A71137189A 193.23.244.244:80 7BE6 83E6 5D48 1413 21C5 ED92 F075 C553 64AC 7123",
	"urras orport=80 no-v2 v3ident=80550987E1D626E3EBA5E5E75A458DE0626D088C 208.83.223.34:443 0AD3 FA88 4D18 F89E EA2D 89C0 1937 9E0E 7FD9 4417",
	"maatuska orport=80 no-v2 v3ident=49015F787433103580E3B66A1707A00E60F2D15B 213.115.239.118:443 BD6A 8292 55CB 08E6 6FBE 7D37 4836 3586 E46B 3810",NULL };

void dlgAuthorities_initDirServers(config_line_t **option)
{	int i;
	config_line_t *cfg,**cfg1;
	if(*option==NULL)
	{
		cfg1=option;*cfg1=NULL;
		for(i=0;i<9;i++)
		{	*cfg1=tor_malloc_zero(sizeof(config_line_t));
			(*cfg1)->key = tor_strdup("DirServers");
			(*cfg1)->value=dirservers[i];
			cfg1=&((*cfg1)->next);
		}
		for(cfg=*option;cfg;cfg=cfg->next) parse_dir_server_line(cfg->value,NO_AUTHORITY,0);
	}
	last_dir_status[0]=0;
}

void setEditDataDir(config_line_t **option)
{	if(*option!=NULL)
	{	char *tmp1=tor_malloc(65536),*tmp2;
		config_line_t *cfg;
		int i=0,j;
		tmp2=tmp1;
		for(cfg=*option;cfg;cfg=cfg->next)
		{	for(j=0;i<65530;i++,j++)
			{	if(!cfg->value[j]) break;
				*tmp1++=cfg->value[j];
			}
			*tmp1++=13;*tmp1++=10;i+=2;
			if(i>65530) break;
		}
		*tmp1=0;
		SetDlgItemText(hDlgAuthorities,18100,tmp2);
		tor_free(tmp2);
	}
}

void updateDirStatus(void)
{	if(hDlgAuthorities)	SetDlgItemTextL(hDlgAuthorities,18020,last_dir_status);
}


int __stdcall dlgAuthorities(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgAuthorities=hDlg;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_authorities);
		}
		updateDirStatus();
		setEditDataDir(&tmpOptions->DirServers);
		if(tmpOptions->DirFlags&1) CheckDlgButton(hDlg,18401,BST_CHECKED);
		if(tmpOptions->DirFlags&2) CheckDlgButton(hDlg,18402,BST_CHECKED);
		if(tmpOptions->ResolveTimeout) CheckDlgButton(hDlg,18403,BST_CHECKED);
		SetDlgItemInt(hDlg,18101,tmpOptions->ResolveTimeout,0);
		if(tmpOptions->DirFlags&4) CheckDlgButton(hDlg,18404,BST_CHECKED);
		if(tmpOptions->MaxDlFailures) CheckDlgButton(hDlg,18407,BST_CHECKED);
		SetDlgItemInt(hDlg,18103,tmpOptions->MaxDlFailures,0);
		if(tmpOptions->MaxFileAge) CheckDlgButton(hDlg,18408,BST_CHECKED);
		SetDlgItemInt(hDlg,18104,tmpOptions->MaxFileAge,0);
		if(tmpOptions->DirFlags&16)
			CheckDlgButton(hDlg,18405,BST_CHECKED);
		else
		{	EnableWindow(GetDlgItem(hDlg,18010),0);
			EnableWindow(GetDlgItem(hDlg,18102),0);
			EnableWindow(GetDlgItem(hDlg,18011),0);
			EnableWindow(GetDlgItem(hDlg,18406),0);
		}
		SetDlgItemInt(hDlg,18102,tmpOptions->MaxTimeDelta,0);
		if(tmpOptions->DirFlags&32) CheckDlgButton(hDlg,18406,BST_CHECKED);
	}
	else if(uMsg==WM_COMMAND)
	{
		if((LOWORD(wParam)==18100)&&(HIWORD(wParam)==EN_CHANGE))
		{	config_line_t *cfg;
			getEditData1(hDlg,18100,&tmpOptions->DirServers,"DirServers");
			clear_trusted_dir_servers();
			for(cfg=tmpOptions->DirServers;cfg;cfg=cfg->next) parse_dir_server_line(cfg->value,NO_AUTHORITY,0);
		}
		else if(LOWORD(wParam)==18401)
		{	if(IsDlgButtonChecked(hDlg,18401)==BST_CHECKED){	tmpOptions->DirFlags|=1;}
			else{	tmpOptions->DirFlags&=0xfffe;}
		}
		else if(LOWORD(wParam)==18402)
		{	if(IsDlgButtonChecked(hDlg,18402)==BST_CHECKED){	tmpOptions->DirFlags|=2;}
			else{	tmpOptions->DirFlags&=0xffff^2;}
		}
		else if((LOWORD(wParam)==18403)||(((LOWORD(wParam)==18101)&&(HIWORD(wParam)==EN_CHANGE))))
		{	if(IsDlgButtonChecked(hDlg,18403)==BST_CHECKED){	tmpOptions->ResolveTimeout=GetDlgItemInt(hDlg,18101,NULL,0);}
			else{	tmpOptions->ResolveTimeout=0;}
		}
		else if(LOWORD(wParam)==18404)
		{	if(IsDlgButtonChecked(hDlg,18404)==BST_CHECKED){	tmpOptions->DirFlags|=4;}
			else{	tmpOptions->DirFlags&=0xffff^4;}
		}
		else if((LOWORD(wParam)==18407)||(((LOWORD(wParam)==18103)&&(HIWORD(wParam)==EN_CHANGE))))
		{	if(IsDlgButtonChecked(hDlg,18407)==BST_CHECKED){	tmpOptions->MaxDlFailures=GetDlgItemInt(hDlg,18103,NULL,0);}
			else{	tmpOptions->MaxDlFailures=0;}
		}
		else if((LOWORD(wParam)==18408)||(((LOWORD(wParam)==18104)&&(HIWORD(wParam)==EN_CHANGE))))
		{	if(IsDlgButtonChecked(hDlg,18408)==BST_CHECKED){	tmpOptions->MaxFileAge=GetDlgItemInt(hDlg,18104,NULL,0);}
			else{	tmpOptions->MaxFileAge=0;}
		}
		else if(LOWORD(wParam)==18405)
		{	if(IsDlgButtonChecked(hDlg,18405)==BST_CHECKED)
			{	EnableWindow(GetDlgItem(hDlg,18010),1);
				EnableWindow(GetDlgItem(hDlg,18102),1);
				EnableWindow(GetDlgItem(hDlg,18011),1);
				EnableWindow(GetDlgItem(hDlg,18406),1);
				tmpOptions->DirFlags|=16;
				if((tmpOptions->BestTimeDelta)&&(tmpOptions->DirFlags&32)) delta_t=tmpOptions->BestTimeDelta;
				else delta_t=crypto_rand_int(tmpOptions->MaxTimeDelta*2)-tmpOptions->MaxTimeDelta;
				update_best_delta_t(delta_t);
			}
			else
			{	EnableWindow(GetDlgItem(hDlg,18010),0);
				EnableWindow(GetDlgItem(hDlg,18102),0);
				EnableWindow(GetDlgItem(hDlg,18011),0);
				EnableWindow(GetDlgItem(hDlg,18406),0);
				tmpOptions->DirFlags&=0xffef;
				delta_t=0;
				update_best_delta_t(0);
			}
		}
		else if(((LOWORD(wParam)==18102)&&(HIWORD(wParam)==EN_CHANGE)))
		{	tmpOptions->MaxTimeDelta=GetDlgItemInt(hDlg,18102,NULL,0);
			if((tmpOptions->BestTimeDelta)&&(tmpOptions->DirFlags&32)) delta_t=tmpOptions->BestTimeDelta;
			else delta_t=crypto_rand_int(tmpOptions->MaxTimeDelta*2)-tmpOptions->MaxTimeDelta;
			update_best_delta_t(delta_t);
		}
		else if(LOWORD(wParam)==18406)
		{	if(IsDlgButtonChecked(hDlg,18406)==BST_CHECKED){	tmpOptions->DirFlags|=32;}
			else{	tmpOptions->DirFlags&=0xffff^0x20;}
		}
		else if(LOWORD(wParam)==18001)
		{	expire_consensus();
			update_v2_networkstatus_cache_downloads(get_time(NULL));
			update_consensus_networkstatus_downloads(get_time(NULL));
			update_certificate_downloads(get_time(NULL));
		}
	}
	return 0;
}
