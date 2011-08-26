
#ifndef LVM_SETEXTENDEDLISTVIEWSTYLE
	#define LVM_SETEXTENDEDLISTVIEWSTYLE 0x1036
#endif
#ifndef LVS_EX_ONECLICKACTIVATE
	#define LVS_EX_ONECLICKACTIVATE 0x40
#endif
#ifndef LVS_EX_FULLROWSELECT
	#define LVS_EX_FULLROWSELECT 0x20
#endif


typedef struct lang_dlg_info
{	int ctrlId;
	int langId;
} lang_dlg_info;

HWND createChildDialog(HANDLE hParent,int resourceId,DLGPROC dialogFunc);
void getEditData(HWND hDlg,int editBox,config_line_t **option,const char *value);
void setEditData(HWND hDlg,int editBox,config_line_t *option);
void changeDialogStrings(HWND hDlg,lang_dlg_info *dlgInfo);
void getEditData1(HWND hDlg,int editBox,config_line_t **option,const char *value);
