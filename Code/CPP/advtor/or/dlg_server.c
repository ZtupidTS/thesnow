#include "or.h"
#include "dlg_util.h"

HWND hDlgServer=NULL;
//int frame9[]={17400,17100,17010,17101,17011,17102,17012,17103,17013,17104,17401,17402,17403,17404,17105,17406,17106,17015,17107,17016,17050,17108,17051,17109,17017,17407,17408,17409,17410,17411,17001,-1};
lang_dlg_info lang_dlg_server[]={
	{17400,LANG_DLG_SERVER_PORT},
	{17010,LANG_DLG_SERVER_ADDR},
	{17011,LANG_DLG_SERVER_NICK},
	{17012,LANG_DLG_SERVER_MAIL},
	{17013,LANG_DLG_SERVER_ADV_ADDR},
	{17401,LANG_DLG_SERVER_SELF_TEST},
	{17402,LANG_DLG_SERVER_ACT_AS_BRIDGE},
	{17403,LANG_DLG_SERVER_GEOIP_STATS},
	{17406,LANG_DLG_SERVER_MAX_ONIONSKINS},
	{17410,LANG_DLG_RESERVED_03},
	{17411,LANG_DLG_RESERVED_04},
	{17015,LANG_DLG_SERVER_CPUS},
	{17016,LANG_DLG_SERVER_MSG_EXIT},
	{17050,LANG_DLG_SERVER_ACCEPT_POLICY},
	{17051,LANG_DLG_SERVER_REJECT_POLICY},
	{17017,LANG_DLG_SERVER_PUBLISH},
	{17001,LANG_DLG_SERVER_PUBLISH_NOW},
	{0,0}
};

extern int compute_publishserverdescriptor(or_options_t *options);
extern or_options_t *tmpOptions;

void getEditData2(config_line_t **option)
{	int i,j,k,l,s;
	k=0;
	int tmpsize=SendDlgItemMessage(hDlgServer,17108,WM_GETTEXTLENGTH,0,0) + SendDlgItemMessage(hDlgServer,17109,WM_GETTEXTLENGTH,0,0)+1+1+2;
	char *tmp1=tor_malloc(tmpsize+2);
	config_line_t *cfg,**cfg1;
	for(;;)
	{	cfg=*option;
		if(cfg==NULL) break;
		tor_free(cfg->key);tor_free(cfg->value);
		*option=cfg->next;
		tor_free(cfg);
	}
	j=0;cfg1=option;*cfg1=NULL;
	SendDlgItemMessage(hDlgServer,17108,WM_GETTEXT,tmpsize,(LPARAM)tmp1);
	s=strlen(tmp1);
	for(i=0;i<s+1;i++)
	{	if((tmp1[i]==13)||(tmp1[i]==10)||(tmp1[i]==0))
		{	if(j!=i)
			{	if((tmp1[j]=='*')&&(tmp1[j+1]==':')&&(tmp1[j+2]=='*'))
					k=1;
				else
				{
					tmp1[i]=0;
					*cfg1=tor_malloc_zero(sizeof(config_line_t));
					(*cfg1)->key = tor_strdup("ExitPolicy");
					(*cfg1)->value=tor_malloc(strlen(&tmp1[j])+7+2);
					(*cfg1)->value[0]='a';(*cfg1)->value[1]='c';(*cfg1)->value[2]='c';(*cfg1)->value[3]='e';(*cfg1)->value[4]='p';(*cfg1)->value[5]='t';(*cfg1)->value[6]=' ';
					l=7;
					while(tmp1[j]!=0)
					{	(*cfg1)->value[l]=tmp1[j];
						j++;l++;
					}
					(*cfg1)->value[l]=0;
					cfg1=&((*cfg1)->next);
				}
			}
			j=i+1;
		}
	}
	SendDlgItemMessage(hDlgServer,17109,WM_GETTEXT,tmpsize,(LPARAM)tmp1);
	s=strlen(tmp1);
	for(i=0,j=0;i<s+1;i++)
	{	if((tmp1[i]==13)||(tmp1[i]==10)||(tmp1[i]==0))
		{	if(j!=i)
			{	tmp1[i]=0;
				*cfg1=tor_malloc_zero(sizeof(config_line_t));
				(*cfg1)->key = tor_strdup("ExitPolicy");
				(*cfg1)->value=tor_malloc(strlen(&tmp1[j])+7+2);
				(*cfg1)->value[0]='r';(*cfg1)->value[1]='e';(*cfg1)->value[2]='j';(*cfg1)->value[3]='e';(*cfg1)->value[4]='c';(*cfg1)->value[5]='t';(*cfg1)->value[6]=' ';
				l=7;
				while(tmp1[j]!=0)
				{	(*cfg1)->value[l]=tmp1[j];
					j++;l++;
				}
				(*cfg1)->value[l]=0;
				cfg1=&((*cfg1)->next);
			}
			j=i+1;
		}
	}
	if(k)
	{
		*cfg1=tor_malloc_zero(sizeof(config_line_t));
		(*cfg1)->key = tor_strdup("ExitPolicy");
		(*cfg1)->value=tor_strdup("accept *:*");
	}
	tor_free(tmp1);
}


void setEditData1(int editBox,config_line_t **option,BOOL isBanned)
{	int j;
	if(*option==NULL)
	{
		*option=tor_malloc_zero(sizeof(config_line_t));
		(*option)->key = tor_strdup("ExitPolicy");
		(*option)->value=tor_strdup("accept *:*");
	}
	if(*option!=NULL)
	{	char *tmp1=tor_malloc(65536),*tmp2;
		int i=0;tmp2=tmp1;
		config_line_t *cfg;
		for(cfg=*option;cfg;cfg=cfg->next)
		{	if(isBanned)
			{	j=0;
				while(1)
				{
					if(((cfg->value[j]|0x20)=='r')&&((cfg->value[j+1]|0x20)=='e')&&((cfg->value[j+2]|0x20)=='j')&&((cfg->value[j+3]|0x20)=='e')&&((cfg->value[j+4]|0x20)=='c')&&((cfg->value[j+5]|0x20)=='t'))
					{	j+=6;
						for(;cfg->value[j]==32;j++)	;
						for(;i<65530;i++,j++)
						{	if((!cfg->value[j])||(cfg->value[j]<32)||(cfg->value[j]==',')||(cfg->value[j]==';')) break;
							*tmp1++=cfg->value[j];
						}
						*tmp1++=13;*tmp1++=10;i+=2;
						if(i>65530) break;
					}
					else
					{
						for(;;j++)
						{	if((!cfg->value[j])||(cfg->value[j]<32)||(cfg->value[j]==',')||(cfg->value[j]==';')) break;
						}
					}
					while((cfg->value[j]==',')||(cfg->value[j]==';')||(cfg->value[j]<33))
					{
						if(!cfg->value[j]) break;
						j++;
					}
					if(!cfg->value[j]) break;
				}
			}
			else
			{
				j=0;
				while(1)
				{
					if(((cfg->value[j]|0x20)=='r')&&((cfg->value[j+1]|0x20)=='e')&&((cfg->value[j+2]|0x20)=='j')&&((cfg->value[j+3]|0x20)=='e')&&((cfg->value[j+4]|0x20)=='c')&&((cfg->value[j+5]|0x20)=='t'))
					{
						for(;;j++)
						{	if((!cfg->value[j])||(cfg->value[j]<32)||(cfg->value[j]==',')||(cfg->value[j]==';')) break;
						}
					}
					else
					{
						if(((cfg->value[j]|0x20)=='a')&&((cfg->value[j+1]|0x20)=='c')&&((cfg->value[j+2]|0x20)=='c')&&((cfg->value[j+3]|0x20)=='e')&&((cfg->value[j+4]|0x20)=='p')&&((cfg->value[j+5]|0x20)=='t'))
						{
							j+=6;
							for(;cfg->value[j]==32;j++)	;
						}
						for(;i<65530;i++,j++)
						{	if((!cfg->value[j])||(cfg->value[j]<32)||(cfg->value[j]==',')||(cfg->value[j]==';')) break;
							*tmp1++=cfg->value[j];
						}
						*tmp1++=13;*tmp1++=10;i+=2;
						if(i>65530) break;
					}
					while((cfg->value[j]==',')||(cfg->value[j]==';')||(cfg->value[j]<33))
					{
						if(!cfg->value[j]) break;
						j++;
					}
					if(!cfg->value[j]) break;
				}
			}
		}
		*tmp1=0;
		SetDlgItemText(hDlgServer,editBox,tmp2);
		tor_free(tmp2);
		cfg=NULL;
	}
}

void addpublishoptions(void)
{
	if(tmpOptions->PublishServerDescriptor)
	{	SMARTLIST_FOREACH(tmpOptions->PublishServerDescriptor, char *, s,tor_free(s));
		smartlist_clear(tmpOptions->PublishServerDescriptor);
	}
	if(!tmpOptions->PublishServerDescriptor)	tmpOptions->PublishServerDescriptor=smartlist_create();
	if(IsDlgButtonChecked(hDlgServer,17407))	smartlist_add(tmpOptions->PublishServerDescriptor,"v1");
	if(IsDlgButtonChecked(hDlgServer,17408))	smartlist_add(tmpOptions->PublishServerDescriptor,"v2");
	if(IsDlgButtonChecked(hDlgServer,17409))	smartlist_add(tmpOptions->PublishServerDescriptor,"v3");
	if(IsDlgButtonChecked(hDlgServer,17410))	smartlist_add(tmpOptions->PublishServerDescriptor,"bridge");
	if(IsDlgButtonChecked(hDlgServer,17411))	smartlist_add(tmpOptions->PublishServerDescriptor,"hidserv");
	compute_publishserverdescriptor(tmpOptions);
}

BOOL dlgServerInit=0;
int dlgServerChanged=0;

void dlgServerUpdate(void)
{	if(!dlgServerChanged) return;
	int i;
	switch(dlgServerChanged)
	{	case 1:
			init_keys();retry_all_listeners(0,0);
			break;
		case 2:
			policies_parse_from_options(tmpOptions);
			break;
		case 3:
			i=tmpOptions->ORPort;if(!tmpOptions->ORPort)	tmpOptions->ORPort=9501;
			init_keys();router_rebuild_descriptor(0);router_upload_dir_desc_to_dirservers(1);
			tmpOptions->ORPort=i;
			break;
		default:
			break;
	}
	dlgServerChanged = 0;
}

int __stdcall dlgServer(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{	if(uMsg==WM_INITDIALOG)
	{	hDlgServer=hDlg;
		dlgServerInit=1;
		if(LangGetLanguage())
		{	changeDialogStrings(hDlg,lang_dlg_server);
		}
		if(tmpOptions->Nickname) SetDlgItemText(hDlg,17102,tmpOptions->Nickname);
		if(tmpOptions->ContactInfo) SetDlgItemText(hDlg,17103,tmpOptions->ContactInfo);
		if(tmpOptions->Address) SetDlgItemText(hDlg,17104,tmpOptions->Address);
		if(tmpOptions->ORPort){ CheckDlgButton(hDlg,17400,BST_CHECKED);SetDlgItemInt(hDlg,17100,tmpOptions->ORPort,0);}
		else{	EnableWindow(GetDlgItem(hDlg,17100),0);EnableWindow(GetDlgItem(hDlg,17010),0);EnableWindow(GetDlgItem(hDlg,17101),0);}
		if(tmpOptions->ORListenAddress)	SetDlgItemText(hDlg,17101,tmpOptions->ORListenAddress->value);
		else SetDlgItemText(hDlg,17101,"127.0.0.1");
		if(tmpOptions->MaxOnionsPending){	SetDlgItemInt(hDlg,17106,tmpOptions->MaxOnionsPending,0);CheckDlgButton(hDlg,17406,BST_CHECKED);}
		else{	EnableWindow(GetDlgItem(hDlg,17106),0);EnableWindow(GetDlgItem(hDlg,17015),0);EnableWindow(GetDlgItem(hDlg,17107),0);}
		SetDlgItemInt(hDlg,17107,tmpOptions->NumCpus,0);
		SendDlgItemMessage(hDlg,17108,EM_LIMITTEXT,65536,0);
		setEditData1(17108,&tmpOptions->ExitPolicy,0);
		SendDlgItemMessage(hDlg,17109,EM_LIMITTEXT,65536,0);
		setEditData1(17109,&tmpOptions->ExitPolicy,1);
		if(tmpOptions->AssumeReachable)	CheckDlgButton(hDlg,17401,BST_CHECKED);
		if(tmpOptions->BridgeRelay)	CheckDlgButton(hDlg,17402,BST_CHECKED);
		else	EnableWindow(GetDlgItem(hDlg,17403),0);
		if(tmpOptions->BridgeRecordUsageByCountry)	CheckDlgButton(hDlg,17403,BST_CHECKED);
		if(tmpOptions->PublishServerDescriptor)
		{	SMARTLIST_FOREACH(tmpOptions->PublishServerDescriptor,const char *,string,{
				if(!strcasecmp(string, "v1"))	CheckDlgButton(hDlg,17407,BST_CHECKED);
				else if(!strcmp(string, "1"))
				{	if(tmpOptions->BridgeRelay) CheckDlgButton(hDlg,17410,BST_CHECKED);
					else{	CheckDlgButton(hDlg,17408,BST_CHECKED);CheckDlgButton(hDlg,17409,BST_CHECKED);}
				}
				else if(!strcasecmp(string, "v2"))	CheckDlgButton(hDlg,17408,BST_CHECKED);
				else if(!strcasecmp(string, "v3"))	CheckDlgButton(hDlg,17409,BST_CHECKED);
				else if(!strcasecmp(string, "bridge"))	CheckDlgButton(hDlg,17410,BST_CHECKED);
				else if(!strcasecmp(string, "hidserv"))	CheckDlgButton(hDlg,17411,BST_CHECKED);
			});
		}
		dlgServerInit=0;
	}
	else if(uMsg==WM_COMMAND && !dlgServerInit)
	{
		if(LOWORD(wParam)==17406)
		{	if(IsDlgButtonChecked(hDlg,17406)==BST_CHECKED)
			{	tmpOptions->MaxOnionsPending=GetDlgItemInt(hDlg,17106,0,0);EnableWindow(GetDlgItem(hDlg,17106),1);EnableWindow(GetDlgItem(hDlg,17015),1);EnableWindow(GetDlgItem(hDlg,17107),1);
			}
			else
			{	tmpOptions->MaxOnionsPending=0;EnableWindow(GetDlgItem(hDlg,17106),0);EnableWindow(GetDlgItem(hDlg,17015),0);EnableWindow(GetDlgItem(hDlg,17107),0);
			}
		}
		else if((LOWORD(wParam)==17106)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->MaxOnionsPending=GetDlgItemInt(hDlg,17106,0,0);
		else if((LOWORD(wParam)==17107)&&(HIWORD(wParam)==EN_CHANGE))
			tmpOptions->NumCpus=GetDlgItemInt(hDlg,17107,0,0);
		else if((LOWORD(wParam)==17102)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,17102,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,17102,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->Nickname;
			tmpOptions->Nickname=tmp1;tor_free(tmp2);
		}
		else if((LOWORD(wParam)==17103)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,17103,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,17103,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->ContactInfo;
			tmpOptions->ContactInfo=tmp1;tor_free(tmp2);
		}
		else if((LOWORD(wParam)==17104)&&(HIWORD(wParam)==EN_CHANGE))
		{	int tmpsize=SendDlgItemMessage(hDlg,17104,WM_GETTEXTLENGTH,0,0);
			char *tmp1=tor_malloc(tmpsize+2);
			SendDlgItemMessage(hDlg,17104,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
			char *tmp2=tmpOptions->Address;
			if(*tmp1) tmpOptions->Address=tmp1;
			else tmpOptions->Address=NULL;
			tor_free(tmp2);
		}
		else if(LOWORD(wParam)==17400)
		{	if(IsDlgButtonChecked(hDlg,17400)==BST_CHECKED)
			{	tmpOptions->ORPort=GetDlgItemInt(hDlg,17100,0,0);
				EnableWindow(GetDlgItem(hDlg,17100),1);EnableWindow(GetDlgItem(hDlg,17010),1);EnableWindow(GetDlgItem(hDlg,17101),1);
			}
			else
			{	tmpOptions->ORPort=0; EnableWindow(GetDlgItem(hDlg,17100),0);EnableWindow(GetDlgItem(hDlg,17010),0);EnableWindow(GetDlgItem(hDlg,17101),0);
			}
			dlgServerChanged = 1;
		}
		else if((LOWORD(wParam)==17100)&&(HIWORD(wParam)==EN_CHANGE))
		{	tmpOptions->ORPort=GetDlgItemInt(hDlg,17100,0,0);
			dlgServerChanged = 1;
		}
		else if((LOWORD(wParam)==17101)&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData(hDlg,17101,&tmpOptions->ORListenAddress,"ORListenAddress");
			dlgServerChanged = 1;
		}
		else if(((LOWORD(wParam)==17108)||(LOWORD(wParam)==17109))&&(HIWORD(wParam)==EN_CHANGE))
		{	getEditData2(&tmpOptions->ExitPolicy);
			dlgServerChanged = 2;
		}
		else if(LOWORD(wParam)==17401)
		{	if(IsDlgButtonChecked(hDlg,17401)==BST_CHECKED)	tmpOptions->AssumeReachable=1;
			else	tmpOptions->AssumeReachable=0;
		}
		else if(LOWORD(wParam)==17402)
		{	if(IsDlgButtonChecked(hDlg,17402)==BST_CHECKED){	tmpOptions->BridgeRelay=1;EnableWindow(GetDlgItem(hDlg,17403),1);}
			else{	tmpOptions->BridgeRelay=0;EnableWindow(GetDlgItem(hDlg,17403),0);}
		}
		else if(LOWORD(wParam)==17403)
		{	if(IsDlgButtonChecked(hDlg,17403)==BST_CHECKED)	tmpOptions->BridgeRecordUsageByCountry=1;
			else	tmpOptions->BridgeRecordUsageByCountry=0;
		}
		else if(LOWORD(wParam)==17001)
		{	dlgServerChanged = 3;
		}
		else if((LOWORD(wParam)>=17407)&&(LOWORD(wParam)<=17411))
		{	addpublishoptions();}
	}
	return 0;
}

