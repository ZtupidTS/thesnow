#include "util.h"
#include "log.h"
#include "language.h"
#include "lang_def.h"

#define LANUGAGE_ANSI 0
#define LANGUAGE_UNICODE 1
#define LANGUAGE_UTF8 2

int languageType=0;
int languageIndex=0;		// 0 = default
int hastimer=0;
DWORD lastUpdate=0;
char **langDefault=NULL;
char *loadedLng=NULL;
int langErrors=0;
int lngFlag=0;
HLOCAL hEditMem=NULL;;
CRITICAL_SECTION hCriticalSection;

char *getLanguageFileName(char *source);
void setNewLanguage(void);

char *get_lang_str(int x)
{	if(lang_tbl[x].index!=x) asm(".intel_syntax noprefix\nint 3\n.att_syntax prefix");
	if(x>=LANG_MAX) asm(".intel_syntax noprefix\nint 3\n.att_syntax prefix");
//	log(LOG_DEBUG,LD_APP,"Language string: %d",x);
	return lang_tbl[x].langStr;
}

void LangInitCriticalSection(void)
{	InitializeCriticalSection(&hCriticalSection);
}

void LangDeleteCriticalSection(void)
{	DeleteCriticalSection(&hCriticalSection);
}

void LangEnterCriticalSection(void)
{	EnterCriticalSection(&hCriticalSection);
}

void LangLeaveCriticalSection(void)
{	LeaveCriticalSection(&hCriticalSection);
}

int LangSetDlgItemText(HWND hDlg,int item,int langId)
{	if(languageType==LANUGAGE_ANSI)	return SetDlgItemText(hDlg,item,get_lang_str(langId));
	else
	{	char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		langStrLen=SendMessageW(GetDlgItem(hDlg,item),WM_SETTEXT,0,(LPARAM)txt);
		tor_free(txt);
		return langStrLen;
	}
}

int SetDlgItemTextL(HWND hDlg,int item,LPCTSTR lpString)
{	if(languageType==LANUGAGE_ANSI)	return SetDlgItemText(hDlg,item,lpString);
	else
	{	int langStrLen=strlen(lpString);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)lpString,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		langStrLen=SetDlgItemTextW(hDlg,item,txt);
		tor_free(txt);
		return langStrLen;
	}
}

int SetWindowTextL(HWND hDlg,int langId)
{	if(languageType==LANUGAGE_ANSI)	return SetWindowText(hDlg,get_lang_str(langId));
	else
	{	char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		langStrLen=SetWindowTextW(hDlg,txt);
		tor_free(txt);
		return langStrLen;
	}
}

void LangInsertColumn(HWND hDlg,int item,int width,int langId,int subItem,int format)
{	LV_COLUMN lvcol;LV_COLUMNW lvcol1;
	if(languageType==LANUGAGE_ANSI)
	{	lvcol.pszText=get_lang_str(langId);
		lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH;
		lvcol.fmt=format;
		lvcol.cx=width;
		lvcol.iSubItem=0;
		lvcol.cchTextMax=sizeof(lvcol.pszText);
		SendDlgItemMessage(hDlg,item,LVM_INSERTCOLUMN,subItem,(LPARAM)&lvcol);
	}
	else
	{	char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		lvcol1.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH;
		lvcol1.fmt=format;
		lvcol1.cx=width;
		lvcol1.iSubItem=0;
		lvcol1.pszText=txt;
		lvcol1.cchTextMax=langStrLen;
		SendDlgItemMessageW(hDlg,item,LVM_INSERTCOLUMNW,subItem,(LPARAM)&lvcol1);
		tor_free(txt);
	}
}

void LangSetColumn(HWND hDlg,int item,int width,int langId,int subItem)
{	LV_COLUMN lvcol;LV_COLUMNW lvcol1;
	if(languageType==LANUGAGE_ANSI)
	{	lvcol.pszText=get_lang_str(langId);
		lvcol.mask=LVCF_TEXT;
		lvcol.fmt=LVCFMT_LEFT;
		lvcol.cx=width;
		lvcol.iSubItem=0;
		lvcol.cchTextMax=sizeof(lvcol.pszText);
		SendDlgItemMessage(hDlg,item,LVM_SETCOLUMN,subItem,(LPARAM)&lvcol);
	}
	else
	{	lvcol1.mask=LVCF_TEXT;
		lvcol1.fmt=LVCFMT_LEFT;
		lvcol1.cx=width;
		lvcol1.iSubItem=0;
		char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		lvcol1.pszText=txt;
		lvcol1.cchTextMax=langStrLen;
		SendDlgItemMessageW(hDlg,item,LVM_SETCOLUMNW,subItem,(LPARAM)&lvcol1);
		tor_free(txt);
	}
}

void LangSetLVItem(HWND hDlg,int listcontrol,int item,int subitem,char *langstr)
{	LV_ITEM lvit;LV_ITEMW lvit1;
	if(languageType==LANUGAGE_ANSI)
	{	lvit.pszText=langstr;
		lvit.mask=LVIF_TEXT;
		lvit.iItem=item;
		lvit.iSubItem=subitem;
		lvit.cchTextMax=sizeof(lvit.pszText);
		SendDlgItemMessage(hDlg,listcontrol,LVM_SETITEM,0,(LPARAM)&lvit);
	}
	else
	{	lvit1.mask=LVCF_TEXT;
		lvit1.iItem=item;
		lvit1.iSubItem=subitem;
		int langStrLen=strlen(langstr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langstr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		lvit1.pszText=txt;
		lvit1.cchTextMax=langStrLen;
		SendDlgItemMessageW(hDlg,listcontrol,LVM_SETITEMW,0,(LPARAM)&lvit1);
		tor_free(txt);
	}
}

void LangAppendMenu(HMENU hMenu,UINT uFlags,UINT item,int langId)
{
	if(languageType==LANUGAGE_ANSI)	AppendMenu(hMenu,uFlags,item,get_lang_str(langId));
	else
	{	char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		AppendMenuW(hMenu,uFlags,item,txt);
		tor_free(txt);
	}
}

void LangAppendMenuStr(HMENU hMenu,UINT uFlags,UINT item,LPCTSTR lpLangStr)
{
	if(languageType==LANUGAGE_ANSI)	AppendMenu(hMenu,uFlags,item,lpLangStr);
	else
	{	int langStrLen=strlen(lpLangStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)lpLangStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		AppendMenuW(hMenu,uFlags,item,txt);
		tor_free(txt);
	}
}

void LangInsertMenuStr(HMENU hMenu,UINT uPosition,UINT uFlags,UINT item,LPCTSTR lpLangStr)
{
	if(languageType==LANUGAGE_ANSI)	InsertMenu(hMenu,uPosition,uFlags,item,lpLangStr);
	else
	{	int langStrLen=strlen(lpLangStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)lpLangStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		InsertMenuW(hMenu,uPosition,uFlags,item,txt);
		tor_free(txt);
	}
}

int LangCbAddString(HWND hDlg,UINT combo,int langId)
{
	if(languageType==LANUGAGE_ANSI)	return SendDlgItemMessage(hDlg,combo,CB_ADDSTRING,0,(LPARAM)get_lang_str(langId));
	else
	{	char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		langStrLen=SendDlgItemMessageW(hDlg,combo,CB_ADDSTRING,0,(LPARAM)txt);
		tor_free(txt);
		return langStrLen;
	}
}

int LangMessageBox(HWND hDlg,char *message,int titleId,UINT uType)
{	if(languageType==LANUGAGE_ANSI)	return MessageBox(hDlg,message,get_lang_str(titleId),uType);
	else
	{	char *langStr=get_lang_str(titleId);
		int langStrLen=strlen(message),titleStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		LPWSTR title=malloc(titleStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)message,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		titleStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,titleStrLen,title,titleStrLen*2);
		title[titleStrLen]=0;
		langStrLen=MessageBoxW(hDlg,txt,title,uType);
		tor_free(txt);
		return langStrLen;
	}
}

int LangLbAddString(HWND hDlg,UINT listbox,int langId)
{
	if(languageType==LANUGAGE_ANSI)	return SendDlgItemMessage(hDlg,listbox,LB_ADDSTRING,0,(LPARAM)get_lang_str(langId));
	else
	{	char *langStr=get_lang_str(langId);
		int langStrLen=strlen(langStr);
		LPWSTR txt=tor_malloc(langStrLen*2+2);
		langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)langStr,langStrLen,txt,langStrLen*2);
		txt[langStrLen]=0;
		langStrLen=SendDlgItemMessageW(hDlg,listbox,LB_ADDSTRING,0,(LPARAM)txt);
		tor_free(txt);
		return langStrLen;
	}
}

char noText[]="\0\0";

void LangClearText(HWND hDlg)
{	if(0)	//(languageType==LANUGAGE_ANSI)
	{	SendDlgItemMessage(hDlg,100,EM_SETSEL,0,-1);
		SendDlgItemMessage(hDlg,100,EM_REPLACESEL,0,(LPARAM)&noText);
	}
	else
	{	EnterCriticalSection(&hCriticalSection);
		if(hEditMem)
		{	LPWSTR editText=LocalLock(hEditMem);
			*editText=0;
			LocalUnlock(hEditMem);
			LeaveCriticalSection(&hCriticalSection);
		}
		else
		{	HLOCAL hMemTxt=(HLOCAL)SendDlgItemMessage(hDlg,100,EM_GETHANDLE,0,0);
			if(!hMemTxt || LocalFlags(hMemTxt)==LMEM_INVALID_HANDLE)
			{	LeaveCriticalSection(&hCriticalSection);
				SendDlgItemMessage(hDlg,100,EM_SETSEL,0,-1);
				SendMessageW(GetDlgItem(hDlg,100),EM_REPLACESEL,0,(LPARAM)&noText);
			}
			else
			{	LPWSTR editText=LocalLock(hMemTxt);
				*editText=0;
				LocalUnlock(hMemTxt);
				LeaveCriticalSection(&hCriticalSection);
				SendDlgItemMessage(hDlg,100,EM_SETHANDLE,(WPARAM)hMemTxt,0);
			}
		}
	}
	LangDebugScroll(hDlg);
}

void LangReplaceSel(char *replacement,HWND hDlg)
{
	if(0)	//(languageType==LANUGAGE_ANSI)
	{	int txtsize=SendDlgItemMessage(hDlg,100,WM_GETTEXTLENGTH,0,(LPARAM)0);
		if(txtsize>40000)
		{	SendDlgItemMessage(hDlg,100,EM_SETSEL,0,16384);
			SendDlgItemMessage(hDlg,100,EM_REPLACESEL,0,(LPARAM)"\0");
			txtsize=SendDlgItemMessage(hDlg,100,WM_GETTEXTLENGTH,0,0);
		}
		SendDlgItemMessage(hDlg,100,EM_SETSEL,txtsize,txtsize);
		SendDlgItemMessage(hDlg,100,EM_REPLACESEL,0,(LPARAM)replacement);
	}
	else
	{	EnterCriticalSection(&hCriticalSection);
		if(hEditMem)
		{	LPWSTR editText=LocalLock(hEditMem);
			if(!editText) return;
			LPWSTR hTmpTxt=editText;
			int langStrLen=strlen(replacement),txtLen=0;
			while(*editText)
			{	editText++;
				txtLen++;
			}
			if(txtLen>40000)
			{	editText=hTmpTxt;
				hTmpTxt+=16384;
				while(*hTmpTxt) *editText++=*hTmpTxt++;
			}
			langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)replacement,langStrLen,editText,langStrLen);
			editText[langStrLen]=0;
			LocalUnlock(hEditMem);
			LeaveCriticalSection(&hCriticalSection);
		}
		else
		{	HLOCAL hMemTxt=(HLOCAL)SendDlgItemMessage(hDlg,100,EM_GETHANDLE,0,0);
			int langStrLen=strlen(replacement),txtLen=0;
			if(!hMemTxt || LocalFlags(hMemTxt)==LMEM_INVALID_HANDLE)
			{	LPWSTR txt=tor_malloc(langStrLen*2+2);
				LeaveCriticalSection(&hCriticalSection);
				langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)replacement,langStrLen,txt,langStrLen*2);
				txt[langStrLen]=0;
				txtLen=SendDlgItemMessage(hDlg,100,WM_GETTEXTLENGTH,0,(LPARAM)0);
				if(txtLen>40000)
				{	SendDlgItemMessage(hDlg,100,EM_SETSEL,0,16384);
					SendDlgItemMessage(hDlg,100,EM_REPLACESEL,0,(LPARAM)"\0");
					txtLen=SendDlgItemMessage(hDlg,100,WM_GETTEXTLENGTH,0,0);
				}
				SendDlgItemMessage(hDlg,100,EM_SETSEL,txtLen,txtLen);
				langStrLen=SendMessageW(GetDlgItem(hDlg,100),EM_REPLACESEL,0,(LPARAM)txt);
				tor_free(txt);
			}
			else
			{	if(LocalSize(hMemTxt) <= (SendDlgItemMessage(hDlg,100,WM_GETTEXTLENGTH,0,(LPARAM)0)+langStrLen+1)*2)
				{	HLOCAL hTxt=LocalAlloc(LMEM_MOVEABLE,65536*2);
					LPWSTR hOldTxt=LocalLock(hMemTxt);
					LPWSTR hNewTxt=LocalLock(hTxt),hTmpTxt;
					hTmpTxt=hNewTxt;
					while(*hOldTxt)
					{	*hNewTxt++=*hOldTxt++;
						txtLen++;
					}
					if(txtLen>40000)
					{	hOldTxt=hTmpTxt+16384;
						hNewTxt=hTmpTxt;
						while(*hOldTxt)	*hNewTxt++=*hOldTxt++;
					}
					langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)replacement,langStrLen,hNewTxt,langStrLen);
					hNewTxt[langStrLen]=0;
					LocalUnlock(hTxt);
					hEditMem=hTxt;
					LocalUnlock(hMemTxt);
				}
				else
				{	LPWSTR editText=LocalLock(hMemTxt);
					LPWSTR hTmpTxt=editText;
					while(*editText)
					{	editText++;
						txtLen++;
					}
					if(txtLen>40000)
					{	editText=hTmpTxt;
						hTmpTxt+=16384;
						while(*hTmpTxt) *editText++=*hTmpTxt++;
					}
					langStrLen=MultiByteToWideChar(CP_UTF8,0,(LPSTR)replacement,langStrLen,editText,langStrLen);
					editText[langStrLen]=0;
					LocalUnlock(hMemTxt);
				}
				LeaveCriticalSection(&hCriticalSection);
			}
		}
	}
	if((hastimer&1) && (GetTickCount()-lastUpdate > 500))
	{	LangDebugScroll(hDlg);
	}
	else if(!(hastimer&1))
	{	SetTimer(hDlg,101,250,0);
		hastimer|=1;
	}
}

void LangDebugScroll(HWND hDlg)
{
	if(TryEnterCriticalSection(&hCriticalSection))
	{	if(hEditMem)
		{	HLOCAL hMemTxt=(HLOCAL)SendDlgItemMessage(hDlg,100,EM_GETHANDLE,0,0);
			if(hMemTxt && LocalFlags(hMemTxt)!=LMEM_INVALID_HANDLE)	LocalFree(hMemTxt);
			SendDlgItemMessage(hDlg,100,EM_SETHANDLE,(WPARAM)hEditMem,0);
			hEditMem=NULL;
		}
		int txtsize=SendDlgItemMessage(hDlg,100,WM_GETTEXTLENGTH,0,(LPARAM)0);
		SendDlgItemMessage(hDlg,100,EM_SETSEL,txtsize,txtsize);
		SendDlgItemMessage(hDlg,100,EM_SCROLLCARET,0,0);
		lastUpdate=GetTickCount();
		if(hastimer&1)
		{	KillTimer(hDlg,101);
			hastimer=0;
		}
		LeaveCriticalSection(&hCriticalSection);
	}
}

int LangGetSelText(HWND hDlg,char *replText)
{	int lpStart,lpEnd,i,j;
	BOOL* usedDefault=NULL;
	HLOCAL hMemTxt=(HLOCAL)SendMessage(hDlg,EM_GETHANDLE,0,0);
	SendMessage(hDlg,EM_GETSEL,(WPARAM)&lpStart,(LPARAM)&lpEnd);
	if(lpEnd<lpStart){	i=lpEnd;lpEnd=lpStart;lpStart=i;}
	else if(lpEnd==lpStart || lpStart==-1 || lpEnd==-1){	*replText=0;return 0;}
	if(!hMemTxt || LocalFlags(hMemTxt)==LMEM_INVALID_HANDLE)
	{	LPWSTR txt=tor_malloc(1024);
		i=SendMessage(hDlg,EM_LINEFROMCHAR,lpStart,0);
		*txt=768;
		j=SendMessageW(hDlg,EM_GETLINE,i,(LPARAM)(txt));
		*(txt+j)=0;
		i=SendMessage(hDlg,EM_LINEINDEX,i,0);
		if(lpStart>i) i=lpStart-i;
		else	i=0;
		i=WideCharToMultiByte(CP_UTF8,0,txt+i,j-i,replText,1024,NULL,usedDefault);
		tor_free(txt);
		return i;
	}
	else
	{	if(lpEnd-lpStart > 768) lpEnd=lpStart+768;
		LPWSTR hOldTxt=LocalLock(hMemTxt);
		i=WideCharToMultiByte(CP_UTF8,0,hOldTxt+lpStart,lpEnd-lpStart,replText,1024,NULL,usedDefault);
		LocalUnlock(hMemTxt);
		replText[i]=0;
		return i;
	}
}

void LangGetDlgItemText(HWND hDlg,int item,char *utfStr,int nChars)
{	int i;
	BOOL* usedDefault=NULL;
	HLOCAL hMemTxt=(HLOCAL)SendDlgItemMessage(hDlg,item,EM_GETHANDLE,0,0);
	if(!hMemTxt || LocalFlags(hMemTxt)==LMEM_INVALID_HANDLE)
	{	SendDlgItemMessage(hDlg,item,WM_GETTEXT,nChars,(LPARAM)utfStr);
	}
	else
	{	LPWSTR hOldTxt=LocalLock(hMemTxt);
		i=0;while(*(hOldTxt+i)) i++;
		i=WideCharToMultiByte(CP_UTF8,0,hOldTxt,i,utfStr,nChars,NULL,usedDefault);
		if(i>=0 && i<=nChars)	utfStr[i]=0;
		LocalUnlock(hMemTxt);
	}
}

int LangGetLanguage(void)
{	return languageIndex;
}

int LangGetLanguageType(void)
{	return languageType;
}

void checkLangDefault(void)
{	int langStrCnt;
	if(langDefault==NULL)
	{	langDefault=tor_malloc(LANG_MAX*sizeof(char*));
		for(langStrCnt=0;langStrCnt<LANG_MAX;langStrCnt++)
		{	langDefault[langStrCnt]=lang_tbl[langStrCnt].langStr;
			if(lang_tbl[langStrCnt].index!=langStrCnt)
			{	char *langErrorStr=tor_malloc(500);
				tor_snprintf(langErrorStr,500,"There is an error in default language file definitions, %d %s\r\n\tExpected index: %d",lang_tbl[langStrCnt].index,lang_tbl[langStrCnt].langStr,langStrCnt);
				MessageBox(0,langErrorStr,"Verify lng",0);
				tor_free(langErrorStr);
				langErrors++;
			}
		}
	}
	else
	{	for(langStrCnt=0;langStrCnt<LANG_MAX;langStrCnt++)
		{	lang_tbl[langStrCnt].langStr=langDefault[langStrCnt];
		}
	}
}

int isUtf(char *str,int len)
{	int i,utfTmp=0,tmpUtf,isAscii=1,isUtf=1;
	for(i=0;i<len-1;i++)
	{	if(str[i]&0x80) isAscii=0;
		else if(str[i]<':')
		{	if(str[i+1]==0)	return LANGUAGE_UNICODE;
		}
		else if(isUtf)
		{	if(utfTmp==0)
			{	if(str[i]&0x80)
				{	tmpUtf=str[i];
					while(tmpUtf&0x80)
					{	tmpUtf<<=1;
						utfTmp++;
					}
					utfTmp--;
					if(!utfTmp) isUtf=0;
				}
			}
			else
			{	if((str[i]&0xC0)!=0x80) isUtf=0;
				utfTmp--;
			}
		}
	}
	if(isAscii || !isUtf) return LANUGAGE_ANSI;
	return LANGUAGE_UTF8;
}

int is_digit(char c)
{	return (c>='0') && (c<='9');
}

void unload_languages(void)
{	if(langDefault)
	{	tor_free(langDefault);
		langDefault=NULL;
	}
	if(loadedLng)
	{	tor_free(loadedLng);
		loadedLng=NULL;
	}
}

int load_lng(char *file)
{	FILE *lng;
	BOOL* usedDefault=NULL;
	unsigned char *loadedFile=NULL,*tmpFile=NULL,*toFree=NULL,*lastLine=NULL;
	int fsize,fsize1=0;
	int newLine=1,isComment=0,langIdx=0;
	int numDefs=0;
	int lType=LANUGAGE_ANSI;
	if(lngFlag) return 0;
	checkLangDefault();
	if(file[0]=='<')
	{	checkLangDefault();
		return LANG_MAX-1;
	}
	lng=fopen(file,"rb");
	if(!lng)
	{	file=getLanguageFileName(file);
		if(file)
		{	lng=fopen(file,"rb");
			tor_free(file);
		}
	}
	if(lng)
	{	fseek(lng,0,SEEK_END);
		fsize=ftell(lng);
		fseek(lng,0,SEEK_SET);
		if(fsize>5)
		{	loadedFile=tor_malloc(fsize+3);
			fread(loadedFile,1,fsize,lng);
			loadedFile[fsize]=0;loadedFile[fsize+1]=0;
			lType=isUtf(loadedFile,fsize);
			if(lType==LANGUAGE_UNICODE)
			{	tmpFile=tor_malloc(10);
				fsize1=WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)loadedFile,fsize/2,tmpFile,0,NULL,usedDefault);
				tor_free(tmpFile);
				if(fsize1!=0)
				{	tmpFile=tor_malloc(fsize1+1);
					fsize1=WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)loadedFile,fsize/2,tmpFile,fsize1,NULL,usedDefault);
					if(fsize1)
					{	tmpFile[fsize1]=0;
						tor_free(loadedFile);
						loadedFile=tmpFile;
						fsize=fsize1;
					}
					else tor_free(tmpFile);
				}
			}
			if(loadedLng) toFree=loadedLng;
			loadedLng=loadedFile;
			if((loadedFile[0]==0xEF)&&(loadedFile[1]==0xBB)&&(loadedFile[1]==0xBF))
			{	loadedFile+=3;
				if(fsize>3) fsize-=3;
			}
			else if((loadedFile[0]==0xFF)&&((loadedFile[1]==0xFF)||(loadedFile[1]==0xFE)))
			{	loadedFile+=2;
				if(fsize>3) fsize-=2;
			}
			tmpFile=NULL;
			while(fsize)
			{	if((*loadedFile==13)||(*loadedFile==10))
				{	newLine=1;
					if(!isComment)
					{	if(*loadedFile==13)	lastLine=loadedFile;
						else if((*loadedFile==10)&&((UINT)loadedFile-1!=(UINT)lastLine))	lastLine=loadedFile;
					}
				}
				else if(newLine)
				{	if(*loadedFile==';')
						isComment=1;
					else if((fsize>5) && is_digit(loadedFile[0]) && is_digit(loadedFile[1]) && is_digit(loadedFile[2]) && is_digit(loadedFile[3]) && loadedFile[4]<33)
					{	if(tmpFile)
						{	lang_tbl[langIdx].langStr=tmpFile;
							if(lastLine) *lastLine=0;
							tmpFile=NULL;
						}
						if((loadedFile[5]==13)||(loadedFile[5]==10))
						{	isComment=1;
						}
						else
						{	loadedFile[4]=0;
							langIdx=atoi(loadedFile);
							loadedFile+=5;
							fsize-=5;
							tmpFile=loadedFile;
							numDefs++;
							isComment=0;
						}
					}
					newLine=0;
				}
				else newLine=0;
				loadedFile++;
				fsize--;
			}
			if(tmpFile)
			{	lang_tbl[langIdx].langStr=tmpFile;
				if(lastLine) *lastLine=0;
			}
			languageIndex=1;
			languageType=lType;
			fclose(lng);
			if(toFree) tor_free(toFree);
			setNewLanguage();
			return numDefs;
		}
		fclose(lng);
	}
	return 0;
}

void checkFormat(char *src,char *dest,int max)
{	int j=0;
	while(*src && j<max)
	{	if((src[0]=='%')&&(src[1]!='%'))
		{	dest[j++]='%';
			if(src[1]=='l')
			{	dest[j++]='l';dest[j++]=src[2];}
			else if((src[1]=='.')||((src[1]<='9')&&(src[1]>='0')))
			{	src++;
				while(*src && j<max && ((src[0]=='.')||((src[0]<='9')&&(src[0]>='0'))))
				{	dest[j++]=src[0];
					src++;
				}
				if(src[0]=='l')
				{	dest[j++]='l';dest[j++]=src[1];}
				else if(src[1]) dest[j++]=src[1];
				continue;
			}
			else if((src[1]=='I')&&(src[2]=='6')&&(src[3]=='4')&&(src[4]=='u'))
			{	dest[j++]='I';dest[j++]='6';dest[j++]='4';dest[j++]='u';}
			else if(src[1]) dest[j++]=src[1];
			dest[j++]=1;
		}
		src++;
	}
	dest[j]=0;
}

void matchExpected(char *str1,char *str2)
{	int i;
	if(*str1!='%') return;
	while(*str2)
	{	if(*str2=='%')
		{	for(i=0;str1[i]>1 && str2[i]>1;i++)
				if(str1[i]!=str2[i]) break;
			if((str1[i]==1)&&(str2[i]==1))
			{	str1[i]=2;str2[i]=2;return;
			}
		}
		str2++;
	}
}

void verify_lng(char *file)
{	int numDefs=load_lng(file);
	int i,lastError;
	char *tmpStr;
	if(numDefs && !langErrors)
	{	char *msgErr=tor_malloc(1024);
		char *expected,*found;
		expected=tor_malloc(1024);found=tor_malloc(1024);
		if(langDefault)
		{	for(i=0;i<LANG_MAX;i++)
			{	tmpStr=langDefault[i];
				if(tmpStr)
				{	checkFormat(tmpStr,expected,1000);
					tmpStr=lang_tbl[i].langStr;
					checkFormat(tmpStr,found,1000);
					tmpStr=expected;
					lastError=0;
					while(*tmpStr)
					{	if(*tmpStr=='%') matchExpected(tmpStr,found);
						else if(*tmpStr==1)
						{	lastError=1;*tmpStr=32;}
						else if(*tmpStr==2) *tmpStr=32;
						tmpStr++;
					}
					tmpStr=found;
					while(*tmpStr)
					{	if(*tmpStr==1)
						{	lastError=1;*tmpStr=32;}
						else if(*tmpStr==2) *tmpStr=32;
						tmpStr++;
					}
					if(lastError)
					{	tor_snprintf(msgErr,1023,"Format error in definition %04d:\r\n\r\nExpected: %s\r\nFound: %s",i,expected,found);
						MessageBox(0,msgErr,"Load language file",0);
					}
				}
			}
		}
		tor_snprintf(msgErr,200,"Loaded language definitions %d",numDefs);
		MessageBox(0,msgErr,"Load language file",0);
		tor_free(msgErr);tor_free(expected);tor_free(found);
		langErrors++;
	}
	else
	{	char *msgErr=tor_malloc(200);
		tor_snprintf(msgErr,200,"Error loading language file %s",file);
		MessageBox(0,msgErr,0,0);
		tor_free(msgErr);
		langErrors++;
	}
	unload_languages();
}

void enumLanguages(HWND hCombo,char *selected)
{	WIN32_FIND_DATA wfdata;
	int offs,i;
	if(SendMessage(hCombo,CB_GETCOUNT,0,0)) return;
	lngFlag=1;
	char *findStr=getLanguageFileName("");
	offs=strlen(findStr)-4;
	tor_free(findStr);
	findStr=getLanguageFileName("*");
	HANDLE hFind=FindFirstFile(findStr,&wfdata);
	tor_free(findStr);
	SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)"< Default >");
	if(hFind!=INVALID_HANDLE_VALUE)
	{	while(1)
		{	if(strlen(wfdata.cFileName)>offs)
			{	findStr=&wfdata.cFileName[offs];
				i=strlen(findStr);
				if((i>4)&&(!stricmp(findStr+i-4,".lng")))	findStr[i-4]=0;
				SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)findStr);
			}
			if(!FindNextFile(hFind,&wfdata)) break;
		}
		FindClose(hFind);
	}
	offs=CB_ERR;
	if(selected)
	{	offs=SendMessage(hCombo,CB_FINDSTRINGEXACT,-1,(LPARAM)selected);
		if(offs==CB_ERR)	offs=SendMessage(hCombo,CB_FINDSTRING,-1,(LPARAM)selected);
	}
	if(offs==CB_ERR)	SendMessage(hCombo,CB_SETCURSEL,0,0);
	else	SendMessage(hCombo,CB_SETCURSEL,offs,0);
	lngFlag=0;
}
