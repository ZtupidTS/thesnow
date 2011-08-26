#include "or.h"
#include "dlg_util.h"

#define MAX_ROUTERSELECT_RETRIES 100

NMLISTVIEW *nmLV;
LV_ITEM lvit;
HWND hListView=NULL;

extern or_options_t *tmpOptions;
extern int selectedVer;
char *bannedBW="--------";
char *nostr="\0";
char *maxstr="\xff\xff\xff\xff\xff\xff\xff\xff";

int lastSort=0,lastSel=0;
uint32_t lastRouter=0;

lang_dlg_info lang_dlg_exit[]={
	{10,LANG_EXIT_DLG_COUNTRY},
	{401,LANG_EXIT_DLG_CLOSE_CONN},
	{402,LANG_EXIT_DLG_EXPIRE_HOSTS},
	{403,LANG_EXIT_DLG_CONSECUTIVE_EXITS},
	{1,LANG_EXIT_DLG_SELECT},
	{3,LANG_EXIT_DLG_ADD_FAV},
	{4,LANG_EXIT_DLG_SET_BAN},
	{2,LANG_EXIT_DLG_CANCEL},
	{0,0}
};

lang_dlg_info lang_dlg_routers[]={
	{10,LANG_EXIT_DLG_COUNTRY},
	{1,LANG_EXIT_DLG_SELECT},
	{3,LANG_EXIT_DLG_ADD_FAV},
	{4,LANG_EXIT_DLG_SET_BAN},
	{2,LANG_EXIT_DLG_CANCEL},
	{0,0}
};

void showLastExit(char *,uint32_t);
void signewnym_impl(time_t now);
void add_router_to_favorites(HWND hDlg,char *router,char favtype);
void add_router_to_banlist(HWND hDlg,char *router,char bantype);
void setLastSort(int newSort);


int CALLBACK CompareFunc1(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort)
{	if(lParam1==0)	return -1;
	else if(lParam2==0) return 1;
	else if(lParam1<0)
	{	if(lParam2>=0) return 1;
		lParam1 = 0-lParam1;lParam2 = 0-lParam2;
	}
	else if(lParam2<0)	return -1;
	routerinfo_t *r1,*r2;
	int result=0;
	r1=get_router(lParam1);
	r2=get_router(lParam2);
	if(!r1) r1=r2;
	if(!r2) r2=r1;
	if(!r1) return 0;
	if((lastSort==2)||(lastSort==-2)||(lastSort==4)||(lastSort==-4))
	{	if(lastSort==2)
		{	if((r1->addr>>16) == (r2->addr>>16)) result = (r1->addr&0xffff) - (r2->addr&0xffff);
			else result = (r1->addr>>16&0xffff) - (r2->addr>>16&0xffff);
		}
		else if(lastSort==-2)
		{	if((r1->addr>>16) == (r2->addr>>16)) result = (r2->addr&0xffff) - (r1->addr&0xffff);
			else result = (r2->addr>>16&0xffff) - (r1->addr>>16&0xffff);
		}
		else if(lastSort==4) result = r1->bandwidthrate - r2->bandwidthrate;
		else result = r2->bandwidthrate - r1->bandwidthrate;
	}
	else if((lastSort==1) || (lastSort==-1))
	{	if(lastSort==1) result = strcmp(geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(r1->addr))),geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(r2->addr))));
		else result = strcmp(geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(r2->addr))),geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(r1->addr))));
	}
	else if((lastSort==3) || (lastSort==-3))
	{	if(lastSort==3) result = stricmp(r1->nickname,r2->nickname);
		else result = stricmp(r2->nickname,r1->nickname);
	}
	else if((lastSort==5) || (lastSort==-5))
	{	if(lastSort==5) result = (r1->is_exit?2:0)+(r1->is_possible_guard?1:0)-(r2->is_exit?2:0)-(r2->is_possible_guard?1:0);
		else result = (r2->is_exit?2:0)+(r2->is_possible_guard?1:0)-(r1->is_exit?2:0)-(r1->is_possible_guard?1:0);
	}
	if(!result)
	{	if(lastSort>0)	result = r1->router_id - r2->router_id;
		else result = r2->router_id - r1->router_id;
	}
	return result;
}

void sort_all_items(void)
{	if(lastSort)	SendMessage(hListView,LVM_SORTITEMS,lastSort,(LPARAM)(PFNLVCOMPARE)CompareFunc1);
	lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
	if(lvit.iItem!=-1)
	{	RECT rcItem;
		rcItem.left=0;
		SendMessage(hListView,LVM_GETITEMRECT,lvit.iItem,(LPARAM)&rcItem);
		lvit.lParam=SendMessage(hListView,LVM_GETCOUNTPERPAGE,0,0)/2;
		if(lvit.iItem<lvit.lParam) lvit.lParam=lvit.iItem;
		SendMessage(hListView,LVM_SCROLL,0,(lvit.iItem-lvit.lParam-SendMessage(hListView,LVM_GETTOPINDEX,0,0))*(rcItem.bottom-rcItem.top));
		SendMessage(hListView,LVM_ENSUREVISIBLE,lvit.iItem,0);
	}
}

int last_country_sel;
int __stdcall dlgExitSelect(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if(uMsg==WM_INITDIALOG)
	{	int	cIndex,cbErr;
		last_country_sel=get_country_sel();
		if(LangGetLanguage())
		{	SetWindowTextL(hDlg,LANG_EXIT_DLG_TITLE);
			changeDialogStrings(hDlg,lang_dlg_exit);
		}
		cbErr=LangCbAddString(hDlg,300,LANG_CB_RANDOM_COUNTRY);
		if(cbErr!=CB_ERR) SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,cbErr,0x200);
		cbErr=LangCbAddString(hDlg,300,LANG_CB_NO_EXIT);
		if(cbErr!=CB_ERR) SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,cbErr,0x1ff);
		if(last_country_sel==0x200) SendDlgItemMessage(hDlg,300,CB_SETCURSEL,0,0);
		else if(last_country_sel==0x1ff) SendDlgItemMessage(hDlg,300,CB_SETCURSEL,1,0);
		for(cIndex=0;cIndex<geoip_get_n_countries();cIndex++)
		{	cbErr=SendDlgItemMessage(hDlg,300,CB_ADDSTRING,0,(LPARAM)GeoIP_getfullname(cIndex));
			if(cbErr!=CB_ERR) SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,cbErr,(LPARAM)cIndex);
			if(cIndex==last_country_sel) SendDlgItemMessage(hDlg,300,CB_SETCURSEL,cbErr,0);
		}
		SendDlgItemMessage(hDlg,400,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_ONECLICKACTIVATE|LVS_EX_FULLROWSELECT,LVS_EX_ONECLICKACTIVATE|LVS_EX_FULLROWSELECT);
		LangInsertColumn(hDlg,400,50,LANG_COLUMN_EXIT_1,0,LVCFMT_LEFT);
		LangInsertColumn(hDlg,400,150,LANG_COLUMN_EXIT_2,1,LVCFMT_LEFT);
		LangInsertColumn(hDlg,400,165,LANG_COLUMN_EXIT_3,2,LVCFMT_LEFT);
		LangInsertColumn(hDlg,400,70,LANG_COLUMN_EXIT_4,3,LVCFMT_RIGHT);
		hListView=GetDlgItem(hDlg,400);
		routerlist_reindex();
		add_all_routers_to_list(hDlg,SELECT_EXIT,last_country_sel);
		if(tmpOptions->IdentityFlags&IDENTITY_FLAG_DESTROY_CIRCUITS)	CheckDlgButton(hDlg,401,BST_CHECKED);
		if(tmpOptions->IdentityFlags&IDENTITY_FLAG_EXPIRE_TRACKED_HOSTS)	CheckDlgButton(hDlg,402,BST_CHECKED);
		if(tmpOptions->IdentityFlags&IDENTITY_FLAG_LIST_SELECTION)	CheckDlgButton(hDlg,403,BST_CHECKED);
		sort_all_items();
		SetFocus(hListView);
	}
	else if(uMsg==WM_COMMAND)
	{	if(LOWORD(wParam)==2)
		{	EndDialog(hDlg,0);
		}
		else if(LOWORD(wParam)==1)
		{	lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=0;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam==0)
			{	set_country_sel(last_country_sel,1);
				if(tmpOptions->IdentityFlags&IDENTITY_FLAG_LIST_SELECTION)
				{	routerinfo_t *router = get_router_by_index(get_random_router_index(SELECT_EXIT,last_country_sel));
					if(router)
					{	set_router_id_sel(router->router_id,1);
						lastRouter=router->router_id;
					}
				}
			}
			else if(lvit.lParam<0)
			{	set_country_sel(last_country_sel,0);
				return -1;
			}
			else if(lvit.lParam==1)
			{	set_router_sel(0x0100007f,1);
				set_country_sel(0x1ff,0);
			}
			else if(lvit.lParam>0)
			{	set_country_sel(last_country_sel,0);
				routerinfo_t *router=get_router_by_index(lvit.lParam);
				if(router)
				{	if(tmpOptions->IdentityFlags&IDENTITY_FLAG_LIST_SELECTION)	set_router_id_sel(router->router_id,1);
					else	set_router_sel(router->addr,1);
					lastRouter=router->router_id;
				}
				else return 0;
			}
			if((tmpOptions->BestTimeDelta)&&(tmpOptions->DirFlags&32)) best_delta_t=tmpOptions->BestTimeDelta;
			else best_delta_t=crypto_rand_int(tmpOptions->MaxTimeDelta*2)-tmpOptions->MaxTimeDelta;
			update_best_delta_t(delta_t);
			if(selectedVer==-1) selectedVer=crypto_rand_int(41);
			showLastExit(NULL,0);
			signewnym_impl(0);
			if(tmpOptions->IdentityFlags&IDENTITY_FLAG_DESTROY_CIRCUITS)	circuit_expire_all_circs(lvit.lParam!=0);
			showLastExit(NULL,-1);
			EndDialog(hDlg,1);
		}
		else if(LOWORD(wParam)==3)
		{	char *favtmp1=NULL;
			lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam>=0) favtmp1=find_router_by_index(lvit.lParam);
			if(lvit.lParam>=0 && favtmp1==NULL)
			{	lvit.lParam=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
				if(lvit.lParam!=0x200)
				{
					favtmp1=tor_malloc(10);
					tor_snprintf(favtmp1,10,"{%s}",geoip_get_country_name(lvit.lParam));
				}
			}
			if(favtmp1)
			{	add_router_to_favorites(hDlg,favtmp1,'X');
				tor_free(favtmp1);
			}
		}
		else if(LOWORD(wParam)==4)
		{	char *bantmp1=NULL,cBan=0;
			lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam>=0) bantmp1=find_router_by_index(lvit.lParam);
			if(lvit.lParam>=0 && bantmp1==NULL)
			{	lvit.lParam=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
				if(lvit.lParam!=0x200)
				{	cBan++;
					bantmp1=tor_malloc(10);
					tor_snprintf(bantmp1,10,"{%s}",geoip_get_country_name(lvit.lParam));
				}
			}
			if(bantmp1)
			{	add_router_to_banlist(hDlg,bantmp1,'X');
				if(cBan)
				{	SendMessage(hListView,LVM_DELETEALLITEMS,0,0);
					add_all_routers_to_list(hDlg,SELECT_EXIT,last_country_sel);
				}
				else
				{	lvit.pszText=bannedBW;
					lvit.cchTextMax=10;
					lvit.mask=LVIF_TEXT;
					lvit.iSubItem=3;
					SendMessage(hListView,LVM_SETITEM,0,(LPARAM)&lvit);
					lvit.mask=LVIF_PARAM;
					lvit.lParam=0-lvit.lParam;
					lvit.iSubItem=0;
					SendMessage(hListView,LVM_SETITEM,0,(LPARAM)&lvit);
				}
				sort_all_items();
				tor_free(bantmp1);
				EnableWindow(GetDlgItem(hDlg,1),0);EnableWindow(GetDlgItem(hDlg,3),0);EnableWindow(GetDlgItem(hDlg,4),0);
			}
		}
		else if((LOWORD(wParam)==300)&&(HIWORD(wParam)==CBN_SELCHANGE))
		{	int cbErr;
			cbErr=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
			if(cbErr!=CB_ERR)
			{	last_country_sel=cbErr;
				SendMessage(hListView,LVM_DELETEALLITEMS,0,0);
				add_all_routers_to_list(hDlg,SELECT_EXIT,last_country_sel);
				sort_all_items();
			}
		}
		else if(LOWORD(wParam)==401)
		{	if(IsDlgButtonChecked(hDlg,401)==BST_CHECKED)	tmpOptions->IdentityFlags |= IDENTITY_FLAG_DESTROY_CIRCUITS;
			else	tmpOptions->IdentityFlags &= IDENTITY_FLAGS_ALL^IDENTITY_FLAG_DESTROY_CIRCUITS;
		}
		else if(LOWORD(wParam)==402)
		{	if(IsDlgButtonChecked(hDlg,402)==BST_CHECKED)	tmpOptions->IdentityFlags |= IDENTITY_FLAG_EXPIRE_TRACKED_HOSTS;
			else	tmpOptions->IdentityFlags &= IDENTITY_FLAGS_ALL^IDENTITY_FLAG_EXPIRE_TRACKED_HOSTS;
		}
		else if(LOWORD(wParam)==403)
		{	if(IsDlgButtonChecked(hDlg,403)==BST_CHECKED)	tmpOptions->IdentityFlags |= IDENTITY_FLAG_LIST_SELECTION;
			else	tmpOptions->IdentityFlags &= IDENTITY_FLAGS_ALL^IDENTITY_FLAG_LIST_SELECTION;
		}
	}
	else if(uMsg==WM_NOTIFY)
	{	nmLV=(LPNMLISTVIEW)lParam;
		if(nmLV->hdr.code==LVN_COLUMNCLICK)
		{	if(((nmLV->iSubItem+1)==lastSort)||((nmLV->iSubItem+1)==-lastSort))	setLastSort(-lastSort);
			else	setLastSort(nmLV->iSubItem+1);
			sort_all_items();SetFocus(hListView);
		}
		else if((nmLV->hdr.code==LVN_ITEMCHANGED) && ((nmLV->uChanged&LVIF_STATE)!=0) && (nmLV->iItem!=-1) && (((nmLV->uNewState ^ nmLV->uOldState)&LVIS_SELECTED)!=0) && ((nmLV->uNewState & LVIS_SELECTED) != 0))
		{	lvit.iItem=nmLV->iItem;
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam<0)
			{	EnableWindow(GetDlgItem(hDlg,1),0);
				EnableWindow(GetDlgItem(hDlg,3),0);
				EnableWindow(GetDlgItem(hDlg,4),0);
			}
			else
			{	EnableWindow(GetDlgItem(hDlg,1),1);
				EnableWindow(GetDlgItem(hDlg,3),1);
				EnableWindow(GetDlgItem(hDlg,4),1);
			}
		}
	}
	return	0;
}

int __stdcall dlgRouterSelect(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if(uMsg==WM_INITDIALOG)
	{	int	cIndex,cbErr,cbSel=get_country_sel();
		if(LangGetLanguage())
		{	SetWindowTextL(hDlg,LANG_EXIT_DLG_TITLE);
			changeDialogStrings(hDlg,lang_dlg_routers);
		}
		lastSel=lParam;
		last_country_sel=(lastSel==SELECT_EXIT)?cbSel:0x200;
		cbErr=LangCbAddString(hDlg,300,LANG_CB_RANDOM_COUNTRY);
		if(cbErr!=CB_ERR) SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,cbErr,0x200);
		if(cbSel==0x200 || lastSel!=SELECT_EXIT)	SendDlgItemMessage(hDlg,300,CB_SETCURSEL,0,0);
		cbErr=LangCbAddString(hDlg,300,LANG_CB_NO_EXIT);
		if(cbErr!=CB_ERR) SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,cbErr,0x1ff);
		for(cIndex=0;cIndex<geoip_get_n_countries();cIndex++)
		{	cbErr=SendDlgItemMessage(hDlg,300,CB_ADDSTRING,0,(LPARAM)GeoIP_getfullname(cIndex));
			if(cbErr!=CB_ERR) SendDlgItemMessage(hDlg,300,CB_SETITEMDATA,cbErr,(LPARAM)cIndex);
			if(cIndex==cbSel && lastSel==SELECT_EXIT) SendDlgItemMessage(hDlg,300,CB_SETCURSEL,cbErr,0);
		}
		SendDlgItemMessage(hDlg,400,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_ONECLICKACTIVATE|LVS_EX_FULLROWSELECT,LVS_EX_ONECLICKACTIVATE|LVS_EX_FULLROWSELECT);
		if(lParam==SELECT_ANY)
		{	LangInsertColumn(hDlg,400,50,LANG_COLUMN_EXIT_1,0,LVCFMT_LEFT);
			LangInsertColumn(hDlg,400,130,LANG_COLUMN_EXIT_2,1,LVCFMT_LEFT);
			LangInsertColumn(hDlg,400,155,LANG_COLUMN_EXIT_3,2,LVCFMT_LEFT);
			LangInsertColumn(hDlg,400,70,LANG_COLUMN_EXIT_4,3,LVCFMT_RIGHT);
			LangInsertColumn(hDlg,400,30,LANG_COLUMN_EXIT_5,4,LVCFMT_LEFT);
		}
		else
		{	LangInsertColumn(hDlg,400,50,LANG_COLUMN_EXIT_1,0,LVCFMT_LEFT);
			LangInsertColumn(hDlg,400,150,LANG_COLUMN_EXIT_2,1,LVCFMT_LEFT);
			LangInsertColumn(hDlg,400,165,LANG_COLUMN_EXIT_3,2,LVCFMT_LEFT);
			LangInsertColumn(hDlg,400,70,LANG_COLUMN_EXIT_4,3,LVCFMT_RIGHT);
		}
		hListView=GetDlgItem(hDlg,400);
		routerlist_reindex();
		add_all_routers_to_list(hDlg,lastSel,last_country_sel);
		sort_all_items();SetFocus(hListView);
	}
	else if(uMsg==WM_COMMAND)
	{	if(LOWORD(wParam)==2)
			EndDialog(hDlg,-1);
		else if(LOWORD(wParam)==1)
		{	lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam==0)
			{	lvit.lParam=get_random_router_index(lastSel,last_country_sel);
			}
			if(lvit.lParam>=0)	EndDialog(hDlg,lvit.lParam);
		}
		else if(LOWORD(wParam)==3)
		{	char *favtmp1=NULL;
			lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam>=0) favtmp1=find_router_by_index(lvit.lParam);
			if(lvit.lParam>=0 && favtmp1==NULL)
			{	lvit.lParam=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
				if(lvit.lParam!=0x200)
				{
					favtmp1=tor_malloc(10);
					tor_snprintf(favtmp1,10,"{%s}",geoip_get_country_name(lvit.lParam));
				}
			}
			if(favtmp1)
			{	add_router_to_favorites(hDlg,favtmp1,(lastSel==SELECT_EXIT)?'X':((lastSel==SELECT_ENTRY)?'E':'X'));
				tor_free(favtmp1);
			}
		}
		else if(LOWORD(wParam)==4)
		{	char *bantmp1=NULL,cBan=0;
			lvit.iItem=SendMessage(hListView,LVM_GETNEXTITEM,-1,LVNI_SELECTED);
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam>=0) bantmp1=find_router_by_index(lvit.lParam);
			if(lvit.lParam>=0 && bantmp1==NULL)
			{	lvit.lParam=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
				if(lvit.lParam!=0x200)
				{	cBan++;
					bantmp1=tor_malloc(10);
					tor_snprintf(bantmp1,10,"{%s}",geoip_get_country_name(lvit.lParam));
				}
			}
			if(bantmp1)
			{	add_router_to_banlist(hDlg,bantmp1,(lastSel==SELECT_EXIT)?'X':0);
				if(cBan)
				{	SendMessage(hListView,LVM_DELETEALLITEMS,0,0);
					add_all_routers_to_list(hDlg,lastSel,last_country_sel);
				}
				else
				{	lvit.pszText=bannedBW;
					lvit.cchTextMax=10;
					lvit.mask=LVIF_TEXT;
					lvit.iSubItem=3;
					SendMessage(hListView,LVM_SETITEM,0,(LPARAM)&lvit);
					lvit.mask=LVIF_PARAM;
					lvit.lParam=0-lvit.lParam;
					lvit.iSubItem=0;
					SendMessage(hListView,LVM_SETITEM,0,(LPARAM)&lvit);
				}
				sort_all_items();
				tor_free(bantmp1);
				EnableWindow(GetDlgItem(hDlg,1),0);EnableWindow(GetDlgItem(hDlg,3),0);EnableWindow(GetDlgItem(hDlg,4),0);
			}
		}
		else if((LOWORD(wParam)==300)&&(HIWORD(wParam)==CBN_SELCHANGE))
		{	int cbErr;
			cbErr=SendDlgItemMessage(hDlg,300,CB_GETITEMDATA,SendDlgItemMessage(hDlg,300,CB_GETCURSEL,0,0),0);
			if(cbErr!=CB_ERR) last_country_sel=cbErr;
			SendMessage(hListView,LVM_DELETEALLITEMS,0,0);
			add_all_routers_to_list(hDlg,lastSel,last_country_sel);
			sort_all_items();
		}
	}
	else if(uMsg==WM_NOTIFY)
	{	nmLV=(LPNMLISTVIEW)lParam;
		if(nmLV->hdr.code==LVN_COLUMNCLICK)
		{	if(((nmLV->iSubItem+1)==lastSort)||((nmLV->iSubItem+1)==-lastSort))	setLastSort(-lastSort);
			else	setLastSort(nmLV->iSubItem+1);
			sort_all_items();SetFocus(hListView);
		}
		else if((nmLV->hdr.code==LVN_ITEMCHANGED) && ((nmLV->uChanged&LVIF_STATE)!=0) && (nmLV->iItem!=-1) && (((nmLV->uNewState ^ nmLV->uOldState)&LVIS_SELECTED)!=0) && ((nmLV->uNewState & LVIS_SELECTED) != 0))
		{	lvit.iItem=nmLV->iItem;
			lvit.mask=LVIF_PARAM;
			lvit.iSubItem=0;
			lvit.lParam=-1;
			SendMessage(hListView,LVM_GETITEM,0,(LPARAM)&lvit);
			if(lvit.lParam<0)
			{	EnableWindow(GetDlgItem(hDlg,1),0);
				EnableWindow(GetDlgItem(hDlg,3),0);
				EnableWindow(GetDlgItem(hDlg,4),0);
			}
			else
			{	EnableWindow(GetDlgItem(hDlg,1),1);
				EnableWindow(GetDlgItem(hDlg,3),1);
				EnableWindow(GetDlgItem(hDlg,4),1);
			}
		}
	}
	return	0;
}

void next_router_from_sorted_exits(void)
{	routerlist_t *rl;
	int retries = 0;
	routerinfo_t *prev,*prev2;
	smartlist_t *sl;
	int i,min,min2,r;
	unsigned long l,minl;
	char *rstr,*minstr;
	lastRouter = get_router_id_sel();
	prev = get_router_by_index(lastRouter);
	if(!prev)
	{	if(get_router_sel())	prev = get_router_by_ip(get_router_sel());
		else	prev = get_router_by_index(get_random_router_index(SELECT_EXIT,get_country_sel()));
	}
	if(prev) lastRouter = prev->router_id;
nrfse_restart:
	rl = router_get_routerlist();
	if(!rl) return;
	sl = rl->routers;
	int csel=get_country_sel();
	prev2 = prev = get_router_by_index(lastRouter);
	i=0;min=0;min2=0;
	if(prev) i = prev->router_id;
	min = i;
	switch(lastSort)
	{	case 0:
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	if(((router->router_id <=min) || (i==min)) && router->router_id > i)
					{	prev = router;
						min = router->router_id;
					}
				}
			});
			break;
		case 1:		//country,asc
			if(prev) rstr = (char*)geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(prev->addr)));
			else rstr = (char*)geoip_get_country_name(0);
			minstr = rstr;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	r = strcmp(rstr,geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(router->addr))));
					if(r==0)
					{	if(((router->router_id <=min) || (i==min)) && router->router_id > i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(r<0)
					{	if((strcmp(geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(router->addr))),minstr) <= 0) || !strcmp(minstr,rstr))
						{	prev2 = router;
							minstr=(char*)geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(router->addr)));
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case -1:	//country,desc
			if(prev) rstr = (char*)geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(prev->addr)));
			else rstr = maxstr;
			minstr = rstr;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	r = strcmp(rstr,geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(router->addr))));
					if(r==0)
					{	if(((router->router_id >=min) || (i==min)) && router->router_id < i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(r>0)
					{	if((strcmp(geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(router->addr))),minstr) >= 0) || !strcmp(minstr,rstr))
						{	prev2 = router;
							minstr=(char*)geoip_get_country_name(geoip_get_country_by_ip(geoip_reverse(router->addr)));
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case 2:		//IP,asc
			if(prev) l = prev->addr;
			else l = 0;
			minl = l;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	if(l==router->addr)
					{	if(((router->router_id <=min) || (i==min)) && router->router_id > i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(l < (unsigned long)router->addr)
					{	if(((unsigned long)router->addr <= minl) || (l==minl))
						{	prev2 = router;
							minl = router->addr;
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case -2:	//IP,desc
			if(prev) l = prev->addr;
			else l = 0xffffffff;
			minl = l;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	if(l==router->addr)
					{	if(((router->router_id >=min) || (i==min)) && router->router_id < i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(l > (unsigned long)router->addr)
					{	if(((unsigned long)router->addr >= minl) || (l==minl))
						{	prev2 = router;
							minl = router->addr;
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case 3:		//nickname,asc
			if(prev) rstr = prev->nickname;
			else rstr = nostr;
			minstr = rstr;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	r = strcasecmp(rstr,router->nickname);
					if(r==0)
					{	if(((router->router_id <=min) || (i==min)) && router->router_id > i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(r<0)
					{	if((strcasecmp(router->nickname,minstr) <= 0) || !strcasecmp(rstr,minstr))
						{	prev2 = router;
							minstr=router->nickname;
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case -3:	//nickname,desc
			if(prev) rstr = prev->nickname;
			else rstr = maxstr;
			minstr = rstr;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	r = strcasecmp(rstr,router->nickname);
					if(r==0)
					{	if(((router->router_id >=min) || (i==min)) && router->router_id < i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(r>0)
					{	if((strcasecmp(router->nickname,minstr) >= 0) || !strcasecmp(rstr,minstr))
						{	prev2 = router;
							minstr=router->nickname;
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case 4:		//bandwidth,asc
			if(prev) l = prev->bandwidthrate;
			else l = 0;
			minl = l;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	if(l==router->bandwidthrate)
					{	if(((router->router_id <=min) || (i==min)) && router->router_id > i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(l<router->bandwidthrate)
					{	if((router->bandwidthrate <= minl) || (l==minl))
						{	prev2 = router;
							minl = router->bandwidthrate;
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		case -4:	//bandwidth,desc
			if(prev) l = prev->bandwidthrate;
			else l = 0xffffffff;
			minl = l;
			SMARTLIST_FOREACH(sl,routerinfo_t *, router,
			{	if(router->is_exit &&(csel==0x200 || geoip_get_country_by_ip(geoip_reverse(router->addr))==csel))
				{	if(l==router->bandwidthrate)
					{	if(((router->router_id >=min) || (i==min)) && router->router_id < i)
						{	prev = router;
							min = router->router_id;
						}
					}
					else if(l>router->bandwidthrate)
					{	if((router->bandwidthrate >= minl)||(l==minl))
						{	prev2 = router;
							minl = router->bandwidthrate;
							min2 = router->router_id;
						}
					}
				}
			});
			break;
		default:
			break;
	}
	if(min==i)
	{	min=min2;
		prev=prev2;
	}
	lastRouter = min;
	if(prev && tmpOptions->_ExcludeExitNodesUnion && (routerset_contains_router(tmpOptions->_ExcludeExitNodesUnion,prev)))
	{
		if(++retries < MAX_ROUTERSELECT_RETRIES)
			goto nrfse_restart;
		lastRouter = min = 0;
	}
	set_router_id_sel(min,0);
}
