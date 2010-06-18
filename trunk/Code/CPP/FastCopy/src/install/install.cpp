static char *install_id = 
	"@(#)Copyright (C) 2005-2010 H.Shirouzu		install.cpp	Ver2.00";
/* ========================================================================
	Project  Name			: Installer for FastCopy
	Module Name				: Installer Application Class
	Create					: 2005-02-02(Wed)
	Update					: 2010-05-09(Sun)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#include "../tlib/tlib.h"
#include "instrc.h"
#include "install.h"

char *current_shell = TIsWow64() ? FCSHELLEX64_DLL : FCSHELLEXT1_DLL;
char *SetupFiles [] = {
	FASTCOPY_EXE, INSTALL_EXE, FCSHELLEXT1_DLL, FCSHELLEX64_DLL, README_TXT, HELP_CHM, NULL
};

/*
	WinMain
*/
int WINAPI WinMain(HINSTANCE hI, HINSTANCE, LPSTR cmdLine, int nCmdShow)
{
	return	TInstApp(hI, cmdLine, nCmdShow).Run();
}

/*
	インストールアプリケーションクラス
*/
TInstApp::TInstApp(HINSTANCE _hI, LPSTR _cmdLine, int _nCmdShow) : TApp(_hI, _cmdLine, _nCmdShow)
{
}

TInstApp::~TInstApp()
{
}

void TInstApp::InitWindow(void)
{
/*	if (IsWinNT() && TIsWow64()) {
		DWORD	val = 0;
		TWow64DisableWow64FsRedirection(&val);
	}
*/
	TDlg *maindlg = new TInstDlg(cmdLine);
	mainWnd = maindlg;
	maindlg->Create();
}


/*
	メインダイアログクラス
*/
TInstDlg::TInstDlg(char *cmdLine) : TDlg(INSTALL_DIALOG), staticText(this)
{
	cfg.mode = strcmp(cmdLine, "/r") ? SETUP_MODE : UNINSTALL_MODE;
	cfg.programLink	= TRUE;
	cfg.desktopLink	= TRUE;
}

TInstDlg::~TInstDlg()
{
}

/*
	親ディレクトリ取得（必ずフルパスであること。UNC対応）
*/
BOOL GetParentDir(const char *srcfile, char *dir)
{
	char	path[MAX_PATH], *fname=NULL;

	if (GetFullPathName(srcfile, MAX_PATH, path, &fname) == 0 || fname == NULL)
		return	strcpy(dir, srcfile), FALSE;

	if (fname - path > 3 || path[1] != ':') fname--;
	*fname = 0;

	strcpy(dir, path);
	return	TRUE;
}


/*
	メインダイアログ用 WM_INITDIALOG 処理ルーチン
*/
BOOL TInstDlg::EvCreate(LPARAM lParam)
{
	GetWindowRect(&rect);
	int		cx = ::GetSystemMetrics(SM_CXFULLSCREEN), cy = ::GetSystemMetrics(SM_CYFULLSCREEN);
	int		xsize = rect.right - rect.left, ysize = rect.bottom - rect.top;

	::SetClassLong(hWnd, GCL_HICON, (LONG)::LoadIcon(TApp::GetInstance(), (LPCSTR)SETUP_ICON));
	MoveWindow((cx - xsize)/2, (cy - ysize)/2, xsize, ysize, TRUE);
	Show();

// プロパティシートの生成
	staticText.CreateByWnd(GetDlgItem(INSTALL_STATIC));
	propertySheet = new TInstSheet(&staticText, &cfg);

// 現在ディレクトリ設定
	char		buf[MAX_PATH], setupDir[MAX_PATH];
	TRegistry	reg(HKEY_LOCAL_MACHINE);

// Program Filesのパス取り出し
	if (reg.OpenKey(REGSTR_PATH_SETUP)) {
		if (reg.GetStr(REGSTR_PROGRAMFILES, buf, sizeof(buf)))
			MakePath(setupDir, buf, FASTCOPY);
		reg.CloseKey();
	}

// 既にセットアップされている場合は、セットアップディレクトリを読み出す
	if (reg.OpenKey(REGSTR_PATH_UNINSTALL)) {
		if (reg.OpenKey(FASTCOPY)) {
			if (reg.GetStr(REGSTR_VAL_UNINSTALLER_COMMANDLINE, setupDir, sizeof(setupDir))) {
				GetParentDir(setupDir, setupDir);
			}
			reg.CloseKey();
		}
		reg.CloseKey();
	}
	SetDlgItemText(FILE_EDIT, setupDir);

	CheckDlgButton(cfg.mode == SETUP_MODE ? SETUP_RADIO : UNINSTALL_RADIO, 1);
	ChangeMode();

	return	TRUE;
}

/*
	メインダイアログ用 WM_COMMAND 処理ルーチン
*/
BOOL TInstDlg::EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl)
{
	switch (wID) {
	case IDOK:
		propertySheet->GetData();
		if (cfg.mode == UNINSTALL_MODE)
			UnInstall();
		else
			Install();
		return	TRUE;

	case IDCANCEL:
		::PostQuitMessage(0);
		return	TRUE;

	case FILE_BUTTON:
		BrowseDirDlg(this, FILE_EDIT, "Select Install Directory");
		return	TRUE;

	case SETUP_RADIO:
	case UNINSTALL_RADIO:
		if (wNotifyCode == BN_CLICKED)
			ChangeMode();
		return	TRUE;
	}
	return	FALSE;
}

void TInstDlg::ChangeMode(void)
{
	cfg.mode = IsDlgButtonChecked(SETUP_RADIO) ? SETUP_MODE : UNINSTALL_MODE;
	::EnableWindow(GetDlgItem(FILE_EDIT), cfg.mode == SETUP_MODE);
	propertySheet->Paste();
}

BOOL IsSameFile(char *src, char *dst)
{
	WIN32_FIND_DATA	src_dat, dst_dat;
	HANDLE	hFind;

	if ((hFind = ::FindFirstFile(src, &src_dat)) == INVALID_HANDLE_VALUE)
		return	FALSE;
	::FindClose(hFind);

	if ((hFind = ::FindFirstFile(dst, &dst_dat)) == INVALID_HANDLE_VALUE)
		return	FALSE;
	::FindClose(hFind);

	if (src_dat.nFileSizeLow != dst_dat.nFileSizeLow
	||  src_dat.nFileSizeHigh != dst_dat.nFileSizeHigh)
		return	FALSE;

	return
		(*(_int64 *)&dst_dat.ftLastWriteTime == *(_int64 *)&src_dat.ftLastWriteTime)
	||  ( (*(_int64 *)&src_dat.ftLastWriteTime % 10000000) == 0 ||
		  (*(_int64 *)&dst_dat.ftLastWriteTime % 10000000) == 0 )
	 &&	*(_int64 *)&dst_dat.ftLastWriteTime + 20000000 >= *(_int64 *)&src_dat.ftLastWriteTime
	 &&	*(_int64 *)&dst_dat.ftLastWriteTime - 20000000 <= *(_int64 *)&src_dat.ftLastWriteTime;
}

BOOL MiniCopy(char *src, char *dst)
{
	HANDLE		hSrc, hDst, hMap;
	BOOL		ret = FALSE;
	DWORD		srcSize, dstSize;
	void		*view;
	FILETIME	ct, la, ft;

	if ((hSrc = ::CreateFile(src, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0))
				== INVALID_HANDLE_VALUE)
		return	FALSE;
	srcSize = ::GetFileSize(hSrc, 0);

	if ((hDst = ::CreateFile(dst, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0))
				!= INVALID_HANDLE_VALUE) {
		if ((hMap = ::CreateFileMapping(hSrc, 0, PAGE_READONLY, 0, 0, 0)) != NULL) {
			if ((view = ::MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) != NULL) {
				if (::WriteFile(hDst, view, srcSize, &dstSize, 0) && srcSize == dstSize) {
					if (::GetFileTime(hSrc, &ct, &la, &ft))
						::SetFileTime(hDst, &ct, &la, &ft);
					ret = TRUE;
				}
				::UnmapViewOfFile(view);
			}
			::CloseHandle(hMap);
		}
		::CloseHandle(hDst);
	}
	::CloseHandle(hSrc);

	return	ret;
}

BOOL DelayCopy(char *src, char *dst)
{
	char	tmp_file[MAX_PATH];
	BOOL	ret = FALSE;

	wsprintf(tmp_file, "%s.new", dst);

	if (MiniCopy(src, tmp_file) == FALSE)
		return	FALSE;

	if (IsWinNT()) {
		ret = ::MoveFileEx(tmp_file, dst, MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING);
	}
	else {
		char	win_ini[MAX_PATH], short_tmp[MAX_PATH], short_dst[MAX_PATH];

		::GetShortPathName(tmp_file, short_tmp, sizeof(short_tmp));
		::GetShortPathName(dst, short_dst, sizeof(short_dst));
		::GetWindowsDirectory(win_ini, sizeof(win_ini));
		strcat(win_ini, "\\WININIT.INI");
		// WritePrivateProfileString("Rename", "NUL", short_dst, win_ini); 必要なさそ
		ret = WritePrivateProfileString("Rename", short_dst, short_tmp, win_ini);
	}
	return	ret;
}

#define FASTCOPY_INI_W			L"fastcopy.ini"

BOOL ConvertVirtualStoreConf(void *execDirV, void *userDirV, void *userOldDirV)
{
	BOOL	ret = FALSE;
	WCHAR	buf[MAX_PATH];
	WCHAR	org_ini[MAX_PATH], usr_ini[MAX_PATH], vs_ini[MAX_PATH];

	MakePathV(usr_ini, userDirV,    FASTCOPY_INI_W);
	MakePathV(vs_ini,  userOldDirV, FASTCOPY_INI_W);
	MakePathV(org_ini, execDirV,    FASTCOPY_INI_W);

	if (GetFileAttributesV(vs_ini) != 0xffffffff) {
		ret = ::CopyFileW(vs_ini, usr_ini, TRUE);
		if (ret) {
			MakePathV(buf, userDirV, L"to_OldDir(VirtualStore).lnk");
			SymLinkV(userOldDirV, buf);
			goto END;
		}
	}

	if (GetFileAttributesV(org_ini) != 0xffffffff) {
		ret = ::CopyFileW(org_ini, usr_ini, TRUE);
		goto END;
	}

END:
	if (ret) {
		sprintfV(buf, L"%s.obsolete", vs_ini);
		MoveFileW(vs_ini, buf);
		sprintfV(buf, L"%s.obsolete", org_ini);
		MoveFileW(org_ini, buf);
	}
	return	ret;
}

BOOL TInstDlg::Install(void)
{
	char	buf[MAX_PATH], setupDir[MAX_PATH], setupPath[MAX_PATH], curDir[MAX_PATH];
	BOOL	is_delay_copy = FALSE;

// インストールパス設定
	GetDlgItemText(FILE_EDIT, setupDir, sizeof(setupDir));
	CreateDirectory(setupDir, NULL);
	DWORD	attr = GetFileAttributes(setupDir);

	if (attr == 0xffffffff || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return	MessageBox(GetLoadStr(IDS_NOTCREATEDIR)), FALSE;
	MakePath(setupPath, setupDir, FASTCOPY_EXE);

	if (MessageBox(GetLoadStr(IDS_START), INSTALL_STR, MB_OKCANCEL|MB_ICONINFORMATION) != IDOK)
		return	FALSE;

// ファイルコピー
	if (cfg.mode == SETUP_MODE) {
		char	installPath[MAX_PATH], orgDir[MAX_PATH];

		::GetModuleFileName(NULL, orgDir, sizeof(orgDir));
		GetParentDir(orgDir, orgDir);

		for (int cnt=0; SetupFiles[cnt] != NULL; cnt++) {
			MakePath(buf, orgDir, SetupFiles[cnt]);
			MakePath(installPath, setupDir, SetupFiles[cnt]);

			if (MiniCopy(buf, installPath) || IsSameFile(buf, installPath))
				continue;

			if ((strcmp(SetupFiles[cnt], FCSHELLEXT1_DLL) == 0 ||
				 strcmp(SetupFiles[cnt], FCSHELLEX64_DLL) == 0) && DelayCopy(buf, installPath)) {
				is_delay_copy = TRUE;
				continue;
			}
			return	MessageBox(installPath, GetLoadStr(IDS_NOTCREATEFILE)), FALSE;
		}
	}

// スタートメニュー＆デスクトップに登録
	TRegistry	reg(HKEY_CURRENT_USER);
	if (reg.OpenKey(REGSTR_SHELLFOLDERS)) {
		char	*regStr[]	= { REGSTR_PROGRAMS, REGSTR_DESKTOP, NULL };
		BOOL	execFlg[]	= { cfg.programLink, cfg.desktopLink, NULL };

		for (int cnt=0; regStr[cnt]; cnt++) {
			if (reg.GetStr(regStr[cnt], buf, sizeof(buf))) {
				if (cnt != 0 || RemoveSameLink(buf, buf) == FALSE) {
					::wsprintf(buf + strlen(buf), "\\%s", FASTCOPY_SHORTCUT);
				}
				if (execFlg[cnt]) {
					if (IS_WINNT_V) {
						Wstr	w_setup(setupPath, BY_MBCS);
						Wstr	w_buf(buf, BY_MBCS);
						SymLinkV(w_setup.Buf(), w_buf.Buf());
					}
					else {
						SymLinkV(setupPath, buf);
					}
				}
				else {
					if (IS_WINNT_V) {
						Wstr	w_buf(buf, BY_MBCS);
						DeleteLinkV(w_buf.Buf());
					}
					else {
						DeleteLinkV(buf);
					}
				}
			}
		}
		reg.CloseKey();
	}

// シェル拡張情報の update
	::GetModuleFileName(NULL, curDir, sizeof(curDir));
	GetParentDir(curDir, curDir);

	MakePath(buf, setupDir, FCSHELLEXT1_DLL);
	HMODULE		hShellExtDll = TLoadLibrary(buf);

	if (hShellExtDll) {
		BOOL (WINAPI *IsRegistServer)(void) = (BOOL (WINAPI *)(void))
			GetProcAddress(hShellExtDll, "IsRegistServer");
		BOOL (WINAPI *UpdateDll)(void) = (BOOL (WINAPI *)(void))
			GetProcAddress(hShellExtDll, "UpdateDll");

		if (IsRegistServer && UpdateDll) {
			if (IsRegistServer()) {
				UpdateDll();
			}
		}
		::FreeLibrary(hShellExtDll);
	}
	// Wow64環境では Update しない（rundll32 は戻り値が正しく取れないため）

#if 0
// レジストリにアンインストール情報を登録
	if (reg.OpenKey(REGSTR_PATH_UNINSTALL)) {
		if (reg.CreateKey(FASTCOPY)) {
			MakePath(buf, setupDir, INSTALL_EXE);
			strcat(buf, " /r");
			reg.SetStr(REGSTR_VAL_UNINSTALLER_DISPLAYNAME, FASTCOPY);
			reg.SetStr(REGSTR_VAL_UNINSTALLER_COMMANDLINE, buf);
			reg.CloseKey();
		}
		reg.CloseKey();
	}
#endif

	Wstr	w_setup(setupDir);
	WCHAR	wbuf[MAX_PATH] = L"", wpath[MAX_PATH] = L"", upath[MAX_PATH] = L"";

	if (IsWinVista() && TIsEnableUAC() && TIsVirtualizedDirV(w_setup.Buf())) {
		if (TSHGetSpecialFolderPathV(NULL, wbuf, CSIDL_APPDATA, FALSE)) {
			MakePathV(upath, wbuf, L"FastCopy");

			if (GetFileAttributesV(upath) == 0xffffffff) {
				CreateDirectoryV(upath, NULL);
			}

			if (TMakeVirtualStorePathV(w_setup.Buf(), wpath)) {
				MakePathV(wbuf, upath, FASTCOPY_INI_W);
				if (GetFileAttributesV(wbuf) == 0xffffffff) {
					ConvertVirtualStoreConf(w_setup.Buf(), upath, wpath);
				}
			}
		}
	}

// コピーしたアプリケーションを起動
	if (MessageBox(GetLoadStr(is_delay_copy ? IDS_DELAYSETUPCOMPLETE : IDS_SETUPCOMPLETE),
			INSTALL_STR, MB_OKCANCEL|MB_ICONINFORMATION) == IDOK) {
		::SetCurrentDirectory(setupDir);
		::WinExec(setupPath, SW_SHOW);
	}

	::PostQuitMessage(0);
	return	TRUE;
}

BOOL TInstDlg::UnInstall(void)
{
	char	buf[MAX_PATH];

	if (MessageBox(GetLoadStr(IDS_START), UNINSTALL_STR, MB_OKCANCEL|MB_ICONINFORMATION) != IDOK)
		return	FALSE;

// スタートメニュー＆デスクトップから削除
	TRegistry	reg(HKEY_CURRENT_USER);
	if (reg.OpenKey(REGSTR_SHELLFOLDERS)) {
		char	*regStr[]	= { REGSTR_PROGRAMS, REGSTR_DESKTOP, NULL };

		for (int cnt=0; regStr[cnt] != NULL; cnt++) {
			if (reg.GetStr(regStr[cnt], buf, sizeof(buf))) {
				if (cnt == 0)
					RemoveSameLink(buf);
				::wsprintf(buf + strlen(buf), "\\%s", FASTCOPY_SHORTCUT);
				if (IS_WINNT_V) {
					Wstr	w_buf(buf, BY_MBCS);
					DeleteLinkV(w_buf.Buf());
				}
				else {
					DeleteLinkV(buf);
				}
			}
		}
		reg.CloseKey();
	}

// レジストリからアプリケーション情報を削除
	char	setupDir[MAX_PATH] = "";

	::GetModuleFileName(NULL, setupDir, sizeof(setupDir));
	GetParentDir(setupDir, setupDir);
	BOOL	is_shext = FALSE;

	MakePath(buf, setupDir, FCSHELLEXT1_DLL);
	HMODULE		hShellExtDll = TLoadLibrary(buf);
	if (hShellExtDll) {
		BOOL (WINAPI *IsRegisterDll)(void) = (BOOL (WINAPI *)(void))
			GetProcAddress(hShellExtDll, "IsRegistServer");
		HRESULT (WINAPI *UnRegisterDll)(void) = (HRESULT (WINAPI *)(void))
			GetProcAddress(hShellExtDll, "DllUnregisterServer");

		if (IsRegisterDll && UnRegisterDll && (is_shext = IsRegisterDll())) {
			UnRegisterDll();
		}
		::FreeLibrary(hShellExtDll);
	}

	if (IS_WINNT_V && TIsWow64()) {
		ShellExecute(NULL, NULL, "rundll32.exe",
			"DllUnregisterServer," FCSHELLEX64_DLL, NULL, SW_SHOW);
	}

// レジストリからアンインストール情報を削除
	if (reg.OpenKey(REGSTR_PATH_UNINSTALL)) {
		if (reg.OpenKey(FASTCOPY)) {
			if (reg.GetStr(REGSTR_VAL_UNINSTALLER_COMMANDLINE, setupDir, sizeof(setupDir)))
				GetParentDir(setupDir, setupDir);
			reg.CloseKey();
		}
		reg.DeleteKey(FASTCOPY);
		reg.CloseKey();
	}

// 終了メッセージ
	MessageBox(is_shext ? GetLoadStr(IDS_UNINSTSHEXTFIN) : GetLoadStr(IDS_UNINSTFIN));

// インストールディレクトリを開く
	if (*setupDir)
		::ShellExecute(NULL, NULL, setupDir, 0, 0, SW_SHOW);

	::PostQuitMessage(0);
	return	TRUE;
}


BOOL ReadLink(char *src, char *dest, char *arg=NULL)
{
	IShellLink		*shellLink;		// 実際は IShellLinkA or IShellLinkW
	IPersistFile	*persistFile;
	WCHAR			wbuf[MAX_PATH];
	BOOL			ret = FALSE;

	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
			(void **)&shellLink))) {
		if (SUCCEEDED(shellLink->QueryInterface(IID_IPersistFile, (void **)&persistFile))) {
			::MultiByteToWideChar(CP_ACP, 0, (char *)src, -1, wbuf, MAX_PATH);
			if (SUCCEEDED(persistFile->Load(wbuf, STGM_READ))) {
				if (SUCCEEDED(shellLink->GetPath(dest, MAX_PATH, NULL, 0))) {
					if (arg) {
						shellLink->GetArguments(arg, MAX_PATH);
					}
					ret = TRUE;
				}
			}
			persistFile->Release();
		}
		shellLink->Release();
	}
	return	ret;
}

/*
	同じ内容を持つショートカットを削除（スタートメニューへの重複登録よけ）
*/
BOOL TInstDlg::RemoveSameLink(const char *dir, char *remove_path)
{
	char			path[MAX_PATH], dest[MAX_PATH], arg[MAX_PATH];
	HANDLE			fh;
	WIN32_FIND_DATA	data;
	BOOL			ret = FALSE;

	::wsprintf(path, "%s\\*.*", dir);
	if ((fh = ::FindFirstFile(path, &data)) == INVALID_HANDLE_VALUE)
		return	FALSE;

	do {
		::wsprintf(path, "%s\\%s", dir, data.cFileName);
		if (ReadLink(path, dest, arg) && *arg == 0) {
			int		dest_len = strlen(dest), fastcopy_len = strlen(FASTCOPY_EXE);
			if (dest_len > fastcopy_len
			&& strncmpi(dest + dest_len - fastcopy_len, FASTCOPY_EXE, fastcopy_len) == 0) {
				ret = ::DeleteFile(path);
				if (remove_path != NULL)
					strcpy(remove_path, path);
			}
		}

	} while (::FindNextFile(fh, &data));

	::FindClose(fh);
	return	ret;
}

TInstSheet::TInstSheet(TWin *_parent, InstallCfg *_cfg) : TDlg(INSTALL_SHEET, _parent)
{
	cfg = _cfg;
}

void TInstSheet::GetData(void)
{
	if (resId == UNINSTALL_SHEET) {
	}
	else {
		cfg->programLink	= IsDlgButtonChecked(PROGRAM_CHECK);
		cfg->desktopLink	= IsDlgButtonChecked(DESKTOP_CHECK);
	}
}

void TInstSheet::PutData(void)
{
	if (resId == UNINSTALL_SHEET) {
	}
	else {
		CheckDlgButton(PROGRAM_CHECK, cfg->programLink);
		CheckDlgButton(DESKTOP_CHECK, cfg->desktopLink);
	}
}

void TInstSheet::Paste(void)
{
	if (hWnd) {
		if ((resId == UNINSTALL_SHEET) == (cfg->mode == UNINSTALL_MODE))
			return;
		GetData();
		Destroy();
	}
	resId = cfg->mode == UNINSTALL_MODE ? UNINSTALL_SHEET : INSTALL_SHEET;

	Create();
	PutData();
}

BOOL TInstSheet::EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl)
{
	switch (wID)
	{
	case IDOK:	case IDCANCEL:
		{
			TWin	*top = parent;
			while (top->GetParent())
				top = top->GetParent();
			top->EvCommand(wNotifyCode, wID, hwndCtl);
		}
		return	TRUE;
	}
	return	FALSE;
}

BOOL TInstSheet::EvCreate(LPARAM lParam)
{
	Show();
	return	TRUE;
}

/*
	ディレクトリダイアログ用汎用ルーチン
*/
void BrowseDirDlg(TWin *parentWin, UINT editCtl, char *title)
{ 
	IMalloc			*iMalloc = NULL;
	BROWSEINFO		brInfo;
	LPITEMIDLIST	pidlBrowse;
	char			fileBuf[MAX_PATH];

	parentWin->GetDlgItemText(editCtl, fileBuf, sizeof(fileBuf));
	if (!SUCCEEDED(SHGetMalloc(&iMalloc)))
		return;

	TBrowseDirDlg	dirDlg(fileBuf);
	brInfo.hwndOwner = parentWin->hWnd;
	brInfo.pidlRoot = 0;
	brInfo.pszDisplayName = fileBuf;
	brInfo.lpszTitle = title;
	brInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	brInfo.lpfn = BrowseDirDlg_Proc;
	brInfo.lParam = (LPARAM)&dirDlg;
	brInfo.iImage = 0;

	do {
		if ((pidlBrowse = ::SHBrowseForFolder(&brInfo)) != NULL) {
			if (::SHGetPathFromIDList(pidlBrowse, fileBuf))
				::SetDlgItemText(parentWin->hWnd, editCtl, fileBuf);
			iMalloc->Free(pidlBrowse);
			break;
		}
	} while (dirDlg.IsDirty());

	iMalloc->Release();
}

/*
	BrowseDirDlg用コールバック
*/
int CALLBACK BrowseDirDlg_Proc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM data)
{
	switch (uMsg) {
	case BFFM_INITIALIZED:
		((TBrowseDirDlg *)data)->CreateByWnd(hWnd);
		break;

	case BFFM_SELCHANGED:
		if (((TBrowseDirDlg *)data)->hWnd)
			((TBrowseDirDlg *)data)->SetFileBuf(lParam);
		break;
	}
	return 0;
}

/*
	BrowseDlg用サブクラス生成
*/
BOOL TBrowseDirDlg::CreateByWnd(HWND _hWnd)
{
	BOOL	ret = TSubClass::CreateByWnd(_hWnd);
	dirtyFlg = FALSE;

// ディレクトリ設定
	DWORD	attr = GetFileAttributes(fileBuf);
	if (attr == 0xffffffff || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
		GetParentDir(fileBuf, fileBuf);
	SendMessage(BFFM_SETSELECTION, TRUE, (LPARAM)fileBuf);
	SetWindowText(FASTCOPY);

// ボタン作成
	RECT	tmp_rect;
	::GetWindowRect(GetDlgItem(IDOK), &tmp_rect);
	POINT	pt = { tmp_rect.left, tmp_rect.top };
	::ScreenToClient(hWnd, &pt);
	int		cx = (pt.x - 30) / 2, cy = tmp_rect.bottom - tmp_rect.top;

	::CreateWindow(BUTTON_CLASS, GetLoadStr(IDS_MKDIR), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 10,
		pt.y, cx, cy, hWnd, (HMENU)MKDIR_BUTTON, TApp::GetInstance(), NULL);
	::CreateWindow(BUTTON_CLASS, GetLoadStr(IDS_RMDIR), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
		18 + cx, pt.y, cx, cy, hWnd, (HMENU)RMDIR_BUTTON, TApp::GetInstance(), NULL);

	HFONT	hDlgFont = (HFONT)SendDlgItemMessage(IDOK, WM_GETFONT, 0, 0L);
	if (hDlgFont) {
		SendDlgItemMessage(MKDIR_BUTTON, WM_SETFONT, (UINT)hDlgFont, 0L);
		SendDlgItemMessage(RMDIR_BUTTON, WM_SETFONT, (UINT)hDlgFont, 0L);
	}

	return	ret;
}

/*
	BrowseDlg用 WM_COMMAND 処理
*/
BOOL TBrowseDirDlg::EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl)
{
	switch (wID) {
	case MKDIR_BUTTON:
		{
			char		dirBuf[MAX_PATH], path[MAX_PATH];
			TInputDlg	dlg(dirBuf, this);
			if (dlg.Exec() != IDOK)
				return	TRUE;

			MakePath(path, fileBuf, dirBuf);
			if (::CreateDirectory(path, NULL)) {
				strcpy(fileBuf, path);
				dirtyFlg = TRUE;
				PostMessage(WM_CLOSE, 0, 0);
			}
		}
		return	TRUE;

	case RMDIR_BUTTON:
		if (::RemoveDirectory(fileBuf)) {
			GetParentDir(fileBuf, fileBuf);
			dirtyFlg = TRUE;
			PostMessage(WM_CLOSE, 0, 0);
		}
		return	TRUE;
	}
	return	FALSE;
}

BOOL TBrowseDirDlg::SetFileBuf(LPARAM list)
{
	return	::SHGetPathFromIDList((LPITEMIDLIST)list, fileBuf);
}

/*
	一行入力
*/
BOOL TInputDlg::EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl)
{
	switch (wID) {
	case IDOK:
		GetDlgItemText(INPUT_EDIT, dirBuf, MAX_PATH);
		EndDialog(wID);
		return	TRUE;

	case IDCANCEL:
		EndDialog(wID);
		return	TRUE;
	}
	return	FALSE;
}

/*
	ファイルの保存されているドライブ識別
*/
UINT GetDriveTypeEx(const char *file)
{
	if (file == NULL)
		return	GetDriveType(NULL);

	if (IsUncFile(file))
		return	DRIVE_REMOTE;

	char	buf[MAX_PATH];
	int		len = strlen(file), len2;

	strcpy(buf, file);
	do {
		len2 = len;
		GetParentDir(buf, buf);
		len = strlen(buf);
	} while (len != len2);

	return	GetDriveType(buf);
}

