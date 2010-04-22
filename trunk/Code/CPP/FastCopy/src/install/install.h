/* @(#)Copyright (C) H.Shirouzu 2005   install.h	Ver1.12 */
/* ========================================================================
	Project  Name			: Installer for IPMSG32
	Module Name				: Main Header
	Create					: 2005-02-02(Wed)
	Update					: 2005-05-10(Tue)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

enum InstMode { SETUP_MODE, UNINSTALL_MODE };

struct InstallCfg {
	InstMode	mode;
	BOOL		programLink;
	BOOL		desktopLink;
};

class TInstSheet : public TDlg
{
	InstallCfg	*cfg;

public:
	TInstSheet(TWin *_parent, InstallCfg *_cfg);

	virtual BOOL	EvCreate(LPARAM lParam);
	virtual BOOL	EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl);

	void	Paste(void);
	void	GetData(void);
	void	PutData(void);
};

class TInstDlg : public TDlg
{
protected:
	TSubClassCtl	staticText;
	TInstSheet		*propertySheet;
	InstallCfg		cfg;

public:
	TInstDlg(char *cmdLine);
	virtual ~TInstDlg();

	virtual BOOL	EvCreate(LPARAM lParam);
	virtual BOOL	EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl);
#if 0
	virtual BOOL	EvNcDestroy(void);
	virtual BOOL	EventUser(UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
	BOOL	Install(void);
	BOOL	UnInstall(void);
	void	ChangeMode(void);
	BOOL	RemoveSameLink(const char *dir, char *remove_path=NULL);
};

class TInstApp : public TApp
{
public:
	TInstApp(HINSTANCE _hI, LPSTR _cmdLine, int _nCmdShow);
	virtual ~TInstApp();

	void InitWindow(void);
};

class TBrowseDirDlg : public TSubClass
{
protected:
	char	*fileBuf;
	BOOL	dirtyFlg;

public:
	TBrowseDirDlg(char *_fileBuf) { fileBuf = _fileBuf; }
	virtual BOOL	CreateByWnd(HWND _hWnd);
	virtual BOOL	EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl);
	virtual BOOL	SetFileBuf(LPARAM list);
	BOOL	IsDirty(void) { return dirtyFlg; };
};

class TInputDlg : public TDlg
{
protected:
	char	*dirBuf;

public:
	TInputDlg(char *_dirBuf, TWin *_win) : TDlg(INPUT_DIALOG, _win) { dirBuf = _dirBuf; }
	virtual BOOL	EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl);
};

#define FASTCOPY			"FastCopy"

#define FASTCOPY_EXE		"FastCopy.exe"
#define INSTALL_EXE			"setup.exe"
#define README_TXT			"readme.txt"
#define HELP_CHM			"FastCopy.chm"
#define SHELLEXT1_DLL		"FastCopy_shext.dll"
#define SHELLEXT2_DLL		"FastCopy_shext2.dll"
#define SHELLEXT3_DLL		"FastCopy_shext3.dll"
#define SHELLEXT4_DLL		"FastCopy_shext4.dll"
#define FCSHELLEXT1_DLL		"FastExt1.dll"
#define FCSHELLEX64_DLL		"FastEx64.dll"
#define CURRENT_SHEXTDLL	FCSHELLEXT1_DLL
#define UNC_PREFIX			"\\\\"

#define UNINSTALL_CMDLINE	"/r"
#define FASTCOPY_SHORTCUT	"FastCopy.lnk"

#define REGSTR_SHELLFOLDERS	REGSTR_PATH_EXPLORER "\\Shell Folders"
#define REGSTR_STARTUP		"Startup"
#define REGSTR_DESKTOP		"Desktop"
#define REGSTR_PROGRAMS		"Programs"
#define REGSTR_PATH			"Path"
#define REGSTR_PROGRAMFILES	"ProgramFilesDir"

#define INSTALL_STR			"Install"
#define UNINSTALL_STR		"UnInstall"

// function prototype
int strncmpi(const char *str1, const char *str2, int num);
BOOL SymLink(LPCSTR src, LPSTR dest, LPCSTR arg="");
BOOL ReadLink(LPCSTR src, LPSTR dest, LPSTR arg);
BOOL DeleteLink(LPCSTR path);
void BrowseDirDlg(TWin *parentWin, UINT editCtl, char *title);
int CALLBACK BrowseDirDlg_Proc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM data);
BOOL GetParentDir(const char *srcfile, char *dir);
int MakePath(char *dest, const char *dir, const char *file);
UINT GetDriveTypeEx(const char *file);

// inline function
inline BOOL IsUncFile(const char *path) { return strnicmp(path, UNC_PREFIX, 2) == 0; }
