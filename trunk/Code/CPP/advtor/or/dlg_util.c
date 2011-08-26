#include "or.h"
#include "dlg_util.h"
#include "language.h"

extern HWND hMainDialog;
HANDLE currentDialog=NULL;
extern HINSTANCE hInstance;

HWND createChildDialog(HANDLE hParent,int resourceId,DLGPROC dialogFunc)
{	HWND result;
	if(!hInstance) hInstance=GetModuleHandle(NULL);
	if(!hMainDialog) hMainDialog=hParent;
	result = CreateDialogParamW(hInstance,(LPWSTR)MAKEINTRESOURCE(resourceId),hParent,dialogFunc,0);
	ShowWindow(result,SW_HIDE);
	return result;
}

void changeDialogStrings(HWND hDlg,lang_dlg_info *dlgInfo)
{	int cnt;
	for(cnt=0;dlgInfo[cnt].ctrlId!=0;cnt++)
		LangSetDlgItemText(hDlg,dlgInfo[cnt].ctrlId,dlgInfo[cnt].langId);
}

void getEditData(HWND hDlg,int editBox,config_line_t **option,const char *value)
{	int i,j;
	int tmpsize=SendDlgItemMessage(hDlg,editBox,WM_GETTEXTLENGTH,0,0)+1;
	config_line_t *cfg,**cfg1;
	char *tmp1=tor_malloc(tmpsize+2);
	SendDlgItemMessage(hDlg,editBox,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
	while(*option)
	{	cfg=*option;
		tor_free(cfg->key);tor_free(cfg->value);
		*option=cfg->next;
		tor_free(cfg);
	}
	j=0;cfg1=option;*cfg1=NULL;
	for(i=0;i<tmpsize;i++)
	{	if((tmp1[i]==13)||(tmp1[i]==10)||(tmp1[i]==32)||(tmp1[i]==0))
		{	if(j!=i)
			{	tmp1[i]=0;
				*cfg1=tor_malloc_zero(sizeof(config_line_t));
				(*cfg1)->key = tor_strdup(value);
				(*cfg1)->value=tor_strdup(&tmp1[j]);
				cfg1=&((*cfg1)->next);
			}
			j=i+1;
		}
	}
	tor_free(tmp1);
}

void getEditData1(HWND hDlg,int editBox,config_line_t **option,const char *value)
{	int i,j;
	int tmpsize=SendDlgItemMessage(hDlg,editBox,WM_GETTEXTLENGTH,0,0)+1;
	config_line_t *cfg,**cfg1;
	char *tmp1=tor_malloc(tmpsize+2);
	SendDlgItemMessage(hDlg,editBox,WM_GETTEXT,tmpsize+1,(LPARAM)tmp1);
	RemoveComments(tmp1);tmpsize=strlen(tmp1);
	for(;;)
	{	cfg=*option;
		if(cfg==NULL) break;
		tor_free(cfg->key);tor_free(cfg->value);
		*option=cfg->next;
		tor_free(cfg);
	}
	j=0;cfg1=option;*cfg1=NULL;
	for(i=0;i<tmpsize;i++)
	{	if((tmp1[i]==13)||(tmp1[i]==10)||(tmp1[i]==0))
		{	if(j!=i)
			{	tmp1[i]=0;
				*cfg1=tor_malloc_zero(sizeof(config_line_t));
				(*cfg1)->key = tor_strdup(value);
				(*cfg1)->value=tor_strdup(&tmp1[j]);
				cfg1=&((*cfg1)->next);
			}
			j=i+1;
		}
	}
	tor_free(tmp1);
}

void setEditData(HWND hDlg,int editBox,config_line_t *option)
{	int i,j;
	if(option!=NULL)
	{	char *tmp1=tor_malloc(65536),*tmp2;
		i=0;tmp2=tmp1;
		config_line_t *cfg;
		for(cfg=option;cfg;cfg=cfg->next)
		{	for(j=0;i<65530;i++,j++)
			{	if(!cfg->value[j]) break;
				*tmp1++=cfg->value[j];
			}
			*tmp1++=13;*tmp1++=10;i+=2;
			if(i>65530) break;
		}
		*tmp1=0;
		SetDlgItemText(hDlg,editBox,tmp2);
		tor_free(tmp2);
	}
}

