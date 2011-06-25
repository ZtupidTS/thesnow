#include <filezilla.h>
#ifdef _MSC_VER
#pragma hdrstop
#endif
#include "filezillaapp.h"
#include "Mainfrm.h"
#include "Options.h"
#include "wrapengine.h"
#include "buildinfo.h"
#ifdef __WXMSW__
#include <wx/msw/registry.h> // Needed by CheckForWin2003FirewallBug
#endif
#include <wx/tokenzr.h>
#include "cmdline.h"
#include "welcome_dialog.h"

#include <wx/xrc/xh_bmpbt.h>
#include <wx/xrc/xh_bttn.h>
#include <wx/xrc/xh_chckb.h>
#include <wx/xrc/xh_chckl.h>
#include <wx/xrc/xh_choic.h>
#include <wx/xrc/xh_dlg.h>
#include <wx/xrc/xh_gauge.h>
#include <wx/xrc/xh_listb.h>
#include <wx/xrc/xh_listc.h>
#include <wx/xrc/xh_menu.h>
#include <wx/xrc/xh_notbk.h>
#include <wx/xrc/xh_panel.h>
#include <wx/xrc/xh_radbt.h>
#include <wx/xrc/xh_scwin.h>
#include <wx/xrc/xh_sizer.h>
#include <wx/xrc/xh_spin.h>
#include <wx/xrc/xh_stbmp.h>
#include <wx/xrc/xh_stbox.h>
#include <wx/xrc/xh_stlin.h>
#include <wx/xrc/xh_sttxt.h>
#include <wx/xrc/xh_text.h>
#include <wx/xrc/xh_tree.h>
#include <wx/xrc/xh_hyperlink.h>
#include "xh_toolb_ex.h"
#include "xh_menu_ex.h"
#ifdef __WXMSW__
#include <wx/socket.h>
#include <wx/dynlib.h>
#endif
#ifdef WITH_LIBDBUS
#include <../dbus/session_manager.h>
#endif

#if defined(__WXMAC__) || defined(__UNIX__)
#include <wx/stdpaths.h>
#endif

#ifdef __WXGTK__
#include "locale_initializer.h"
#endif

#ifdef ENABLE_BINRELOC
	#define BR_PTHREADS 0
	#include "prefix.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef __WXGTK__
IMPLEMENT_APP(CFileZillaApp);
#else
IMPLEMENT_APP_NO_MAIN(CFileZillaApp);
#endif //__WXGTK__

CFileZillaApp::CFileZillaApp()
{
	m_pWrapEngine = 0;
	m_pLocale = 0;
	m_pCommandLine = 0;
}

CFileZillaApp::~CFileZillaApp()
{
	delete m_pLocale;
	delete m_pWrapEngine;
	delete m_pCommandLine;
	COptions::Destroy();
}

#ifdef __WXMSW__
bool IsServiceRunning(const wxString& serviceName)
{
	SC_HANDLE hScm = OpenSCManager(0, 0, GENERIC_READ);
	if (!hScm)
	{
		//wxMessageBox(_T("OpenSCManager failed"));
		return false;
	}

	SC_HANDLE hService = OpenService(hScm, serviceName, GENERIC_READ);
	if (!hService)
	{
		CloseServiceHandle(hScm);
		//wxMessageBox(_T("OpenService failed"));
		return false;
	}

	SERVICE_STATUS status;
	if (!ControlService(hService, SERVICE_CONTROL_INTERROGATE, &status))
	{
		CloseServiceHandle(hService);
		CloseServiceHandle(hScm);
		//wxMessageBox(_T("ControlService failed"));
		return false;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hScm);

	if (status.dwCurrentState == 0x07 || status.dwCurrentState == 0x01)
		return false;

	return true;
}

bool CheckForWin2003FirewallBug()
{
	const wxString os = ::wxGetOsDescription();
	if (os.Find(_T("Windows Server 2003")) == -1)
		return false;

	if (!IsServiceRunning(_T("SharedAccess")))
		return false;

	if (!IsServiceRunning(_T("ALG")))
		return false;

	wxRegKey key(_T("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile"));
	if (!key.Exists() || !key.Open(wxRegKey::Read))
		return false;

	long value;
	if (!key.HasValue(_T("EnableFirewall")) || !key.QueryValue(_T("EnableFirewall"), &value))
		return false;

	if (!value)
		return false;

	return true;
}

extern "C" 
{
	typedef HRESULT (WINAPI *t_SetCurrentProcessExplicitAppUserModelID)(PCWSTR AppID);
}

static void SetAppId()
{
	wxDynamicLibrary dll;
	if (!dll.Load(_T("shell32.dll")))
		return;

	if (!dll.HasSymbol(_T("SetCurrentProcessExplicitAppUserModelID")))
		return;

	t_SetCurrentProcessExplicitAppUserModelID pSetCurrentProcessExplicitAppUserModelID =
		(t_SetCurrentProcessExplicitAppUserModelID)dll.GetSymbol(_T("SetCurrentProcessExplicitAppUserModelID"));

	if (!pSetCurrentProcessExplicitAppUserModelID)
		return;

	pSetCurrentProcessExplicitAppUserModelID(_T("FileZilla.Client.AppID"));
}

#endif //__WXMSW__

bool CFileZillaApp::OnInit()
{
	srand( (unsigned)time( NULL ) );

#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION
	//wxHandleFatalExceptions();
#endif

#ifdef __WXMSW__
	// Need to call WSAStartup. Let wx do that for us
	wxSocketBase::Initialize();

	SetAppId();
#endif

	//wxSystemOptions is slow, if a value is not set, it keeps querying the environment
	//each and every time...
	wxSystemOptions::SetOption(_T("filesys.no-mimetypesmanager"), 0);
	wxSystemOptions::SetOption(_T("window-default-variant"), _T(""));
#ifdef __WXMSW__
	wxSystemOptions::SetOption(_T("no-maskblt"), 0);
	wxSystemOptions::SetOption(_T("msw.window.no-clip-children"), 0);
	wxSystemOptions::SetOption(_T("msw.font.no-proof-quality"), 0);
#endif

#ifdef __WXMSW__
	wxSystemOptions::SetOption(_T("msw.remap"), 0);
#endif
#ifdef __WXMAC__
	wxSystemOptions::SetOption(_T("mac.listctrl.always_use_generic"), 1);
#endif

	int cmdline_result = ProcessCommandLine();
	if (!cmdline_result)
		return false;

	LoadLocales();

	if (cmdline_result < 0)
	{
		m_pCommandLine->DisplayUsage();
		return false;
	}

	InitDefaultsDir();

	COptions::Init();

	wxString language = COptions::Get()->GetOption(OPTION_LANGUAGE);
	const wxLanguageInfo* pInfo = wxLocale::FindLanguageInfo(language);
	if (language != _T(""))
	{
#ifdef __WXGTK__
		if (CInitializer::error)
		{
			wxString error;

			wxLocale *loc = wxGetLocale();
			const wxLanguageInfo* currentInfo = loc ? loc->GetLanguageInfo(loc->GetLanguage()) : 0;
			if (!loc || !currentInfo)
			{
				if (!pInfo)
					error.Printf(_("Failed to set language to %s, using default system language."),
						language.c_str());
				else
					error.Printf(_("Failed to set language to %s (%s), using default system language."),
						pInfo->Description.c_str(), language.c_str());
			}
			else
			{
				wxString currentName = currentInfo->CanonicalName;

				if (!pInfo)
					error.Printf(_("Failed to set language to %s, using default system language (%s, %s)."),
						language.c_str(), loc->GetLocale(),
						currentName.c_str());
				else
					error.Printf(_("Failed to set language to %s (%s), using default system language (%s, %s)."),
						pInfo->Description.c_str(), language.c_str(), loc->GetLocale(),
						currentName.c_str());
			}

			error += _T("\n");
			error += _("Please make sure the requested locale is installed on your system.");
			wxMessageBox(error, _("Failed to change language"), wxICON_EXCLAMATION);

			COptions::Get()->SetOption(OPTION_LANGUAGE, _T(""));
		}
#else
		if (!pInfo || !SetLocale(pInfo->Language))
		{
			if (pInfo && pInfo->Description)
				wxMessageBox(wxString::Format(_("Failed to set language to %s (%s), using default system language"), pInfo->Description.c_str(), language.c_str()), _("Failed to change language"), wxICON_EXCLAMATION);
			else
				wxMessageBox(wxString::Format(_("Failed to set language to %s, using default system language"), language.c_str()), _("Failed to change language"), wxICON_EXCLAMATION);
		}
#endif
	}

#ifndef _DEBUG
	const wxString& buildType = CBuildInfo::GetBuildType();
	if (buildType == _T("nightly"))
        wxMessageBox(_T("You are using a nightly development version of FileZilla 3, do not expect anything to work.\r\nPlease use the official releases instead.\r\n\r\n\
Unless explicitly instructed otherwise,\r\n\
DO NOT post bugreports,\r\n\
DO NOT use it in production environments,\r\n\
DO NOT distribute the binaries,\r\n\
DO NOT complain about it\r\n\
USE AT OWN RISK"), _T("Important Information"));
#endif

	if (!LoadResourceFiles())
	{
		COptions::Destroy();
		return false;
	}

	CheckExistsFzsftp();

#ifdef __WXMSW__
	if (CheckForWin2003FirewallBug())
	{
		const wxString& error = _("Warning!\n\nA bug in Windows causes problems with FileZilla\n\n\
The bug occurs if you have\n\
- Windows Server 2003 or XP 64\n\
- Windows Firewall enabled\n\
- Application Layer Gateway service enabled\n\
See http://support.microsoft.com/kb/931130 for background information.\n\n\
Unless you either disable Windows Firewall or the Application Layer Gateway service,\n\
FileZilla will timeout on big transfers.\
");
		wxMessageBox(error, _("Operating system problem detected"), wxICON_EXCLAMATION);
	}
#endif

	// Turn off idle events, we don't need them
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);

#ifdef WITH_LIBDBUS
	CSessionManager::Init();
#endif

	// Load the text wrapping engine
	m_pWrapEngine = new CWrapEngine();
	m_pWrapEngine->LoadCache();

	CMainFrame *frame = new CMainFrame();
	frame->Show(true);
	SetTopWindow(frame);

	CWelcomeDialog *welcome_dialog = new CWelcomeDialog;
	welcome_dialog->Run(frame, false, true);

	frame->ProcessCommandLine();
	frame->PostInitialize();

	return true;
}

int CFileZillaApp::OnExit()
{
	COptions::Get()->SaveIfNeeded();

#ifdef WITH_LIBDBUS
	CSessionManager::Uninit();
#endif
#ifdef __WXMSW__
	wxSocketBase::Shutdown();
#endif
	return wxApp::OnExit();
}

bool CFileZillaApp::FileExists(const wxString& file) const
{
	int pos = file.Find('*');
	if (pos == -1)
		return wxFileExists(file);

	wxASSERT(pos > 0);
	wxASSERT(file[pos - 1] == '/');
	wxASSERT(file[pos + 1] == '/');

	wxLogNull nullLog;
	wxDir dir(file.Left(pos));
	if (!dir.IsOpened())
		return false;

	wxString subDir;
	bool found = dir.GetFirst(&subDir, _T(""), wxDIR_DIRS);
	while (found)
	{
		if (FileExists(file.Left(pos) + subDir + file.Mid(pos + 1)))
			return true;

		found = dir.GetNext(&subDir);
	}

	return false;
}

wxString CFileZillaApp::GetDataDir(wxString fileToFind) const
{
	/*
	 * Finding the resources in all cases is a difficult task,
	 * due to the huge variety of diffent systems and their filesystem
	 * structure.
	 * Basically we just check a couple of paths for presence of the resources,
	 * and hope we find them. If not, the user can still specify on the cmdline
	 * and using environment variables where the resources are.
	 *
	 * At least on OS X it's simple: All inside application bundle.
	 */

#ifdef __WXMAC__
	wxString path = wxStandardPaths::Get().GetDataDir();
	if (FileExists(path + fileToFind))
		return path;

	return _T("");
#else

	wxPathList pathList;
	// FIXME: --datadir cmdline

	// First try the user specified data dir.
	pathList.AddEnvList(_T("FZ_DATADIR"));

	// Next try the current path and the current executable path.
	// Without this, running development versions would be difficult.
	pathList.Add(wxGetCwd());

#ifdef ENABLE_BINRELOC
	const char* path = SELFPATH;
	if (path && *path)
	{
		wxString datadir(SELFPATH , *wxConvCurrent);
		wxFileName fn(datadir);
		datadir = fn.GetPath();
		if (datadir != _T(""))
			pathList.Add(datadir);

	}
	path = DATADIR;
	if (path && *path)
	{
		wxString datadir(DATADIR, *wxConvCurrent);
		if (datadir != _T(""))
			pathList.Add(datadir);
	}
#elif defined __WXMSW__
	wxChar path[1024];
	int res = GetModuleFileName(0, path, 1000);
	if (res > 0 && res < 1000)
	{
		wxFileName fn(path);
		pathList.Add(fn.GetPath());
	}
#endif //ENABLE_BINRELOC and __WXMSW__ blocks

	// Now scan through the path
	pathList.AddEnvList(_T("PATH"));

#ifndef __WXMSW__
	// Try some common paths
	pathList.Add(_T("/usr/share/filezilla"));
	pathList.Add(_T("/usr/local/share/filezilla"));
#endif

	// For each path, check for the resources
	wxPathList::const_iterator node;
	for (node = pathList.begin(); node != pathList.end(); node++)
	{
		wxString cur = *node;
		if (FileExists(cur + fileToFind))
			return cur;
		if (FileExists(cur + _T("/share/filezilla") + fileToFind))
			return cur + _T("/share/filezilla");
		if (FileExists(cur + _T("/filezilla") + fileToFind))
			return cur + _T("/filezilla");
	}

	for (node = pathList.begin(); node != pathList.end(); node++)
	{
		wxString cur = *node;
		if (FileExists(cur + _T("/..") + fileToFind))
			return cur + _T("/..");
		if (FileExists(cur + _T("/../share/filezilla") + fileToFind))
			return cur + _T("/../share/filezilla");
	}

	for (node = pathList.begin(); node != pathList.end(); node++)
	{
		wxString cur = *node;
		if (FileExists(cur + _T("/../../") + fileToFind))
			return cur + _T("/../..");
	}

	return _T("");
#endif //__WXMAC__
}

bool CFileZillaApp::LoadResourceFiles()
{
	m_resourceDir = GetDataDir(_T("/resources/menus.xrc"));

	wxImage::AddHandler(new wxPNGHandler());

	if (m_resourceDir == _T(""))
	{
		wxString msg = _("Could not find the resource files for FileZilla, closing FileZilla.\nYou can set the data directory of FileZilla using the '--datadir <custompath>' commandline option or by setting the FZ_DATADIR environment variable.");
		wxMessageBox(msg, _("FileZilla Error"), wxOK | wxICON_ERROR);
		return false;
	}

	if (m_resourceDir[m_resourceDir.Length() - 1] != wxFileName::GetPathSeparator())
		m_resourceDir += wxFileName::GetPathSeparator();

	m_resourceDir += _T("resources/");

	wxXmlResource *pResource = wxXmlResource::Get();

#ifndef __WXDEBUG__
	pResource->SetFlags(pResource->GetFlags() | wxXRC_NO_RELOADING);
#endif

	pResource->AddHandler(new wxMenuXmlHandler);
	pResource->AddHandler(new wxMenuBarXmlHandlerEx);
	pResource->AddHandler(new wxDialogXmlHandler);
	pResource->AddHandler(new wxPanelXmlHandler);
	pResource->AddHandler(new wxSizerXmlHandler);
	pResource->AddHandler(new wxButtonXmlHandler);
	pResource->AddHandler(new wxBitmapButtonXmlHandler);
	pResource->AddHandler(new wxStaticTextXmlHandler);
	pResource->AddHandler(new wxStaticBoxXmlHandler);
	pResource->AddHandler(new wxStaticBitmapXmlHandler);
	pResource->AddHandler(new wxTreeCtrlXmlHandler);
	pResource->AddHandler(new wxListCtrlXmlHandler);
	pResource->AddHandler(new wxCheckListBoxXmlHandler);
	pResource->AddHandler(new wxChoiceXmlHandler);
	pResource->AddHandler(new wxGaugeXmlHandler);
	pResource->AddHandler(new wxCheckBoxXmlHandler);
	pResource->AddHandler(new wxSpinCtrlXmlHandler);
	pResource->AddHandler(new wxRadioButtonXmlHandler);
	pResource->AddHandler(new wxNotebookXmlHandler);
	pResource->AddHandler(new wxTextCtrlXmlHandler);
	pResource->AddHandler(new wxListBoxXmlHandler);
	pResource->AddHandler(new wxToolBarXmlHandlerEx);
	pResource->AddHandler(new wxStaticLineXmlHandler);
	pResource->AddHandler(new wxScrolledWindowXmlHandler);
	pResource->AddHandler(new wxHyperlinkCtrlXmlHandler);

	wxString resourceDir = m_resourceDir;
#if wxUSE_FILESYSTEM
	resourceDir.Replace(_T("%"), _T("%25"));
	resourceDir.Replace(_T(":"), _T("%3A"));
	resourceDir.Replace(_T("#"), _T("%23"));
#endif
	pResource->Load(resourceDir + _T("*.xrc"));

	return true;
}

bool CFileZillaApp::InitDefaultsDir()
{
#ifdef __WXGTK__
	wxFileName fn = wxFileName(wxGetHomeDir(), _T("fzdefaults.xml"));
	fn.AppendDir(_T(".filezilla"));
	if (fn.FileExists())
		m_defaultsDir = fn.GetPath();
	else if (wxFileName::FileExists(_T("/etc/filezilla/fzdefaults.xml")))
		m_defaultsDir = _T("/etc/filezilla");
	else
#endif
	m_defaultsDir = GetDataDir(_T("/fzdefaults.xml"));

	return m_defaultsDir != _T("");
}

bool CFileZillaApp::LoadLocales()
{
#ifndef __WXMAC__
	m_localesDir = GetDataDir(_T("/../locale/*/filezilla.mo"));
	if (m_localesDir == _T(""))
		m_localesDir = GetDataDir(_T("/../locale/*/LC_MESSAGES/filezilla.mo"));
	if (m_localesDir != _T(""))
	{
		if (m_localesDir[m_localesDir.Length() - 1] != wxFileName::GetPathSeparator())
			m_localesDir += wxFileName::GetPathSeparator();

		m_localesDir += _T("../locale");
	}
	else
	{
		m_localesDir = GetDataDir(_T("/locales/*/filezilla.mo"));
		if (m_localesDir != _T(""))
		{
			if (m_localesDir[m_localesDir.Length() - 1] != wxFileName::GetPathSeparator())
				m_localesDir += wxFileName::GetPathSeparator();

			m_localesDir += _T("locales");
		}
	}
#else
	m_localesDir = wxStandardPaths::Get().GetDataDir() + _T("/locales");
#endif

	if (m_localesDir != _T(""))
	{
		wxFileName fn(m_localesDir, _T(""));
		fn.Normalize();
		m_localesDir = fn.GetFullPath();

		wxLocale::AddCatalogLookupPathPrefix(m_localesDir);
	}

	SetLocale(wxLANGUAGE_DEFAULT);

	return true;
}

bool CFileZillaApp::SetLocale(int language)
{
	// First check if we can load the new locale
	wxLocale* pLocale = new wxLocale();
	wxLogNull log;
	pLocale->Init(language);
	if (!pLocale->IsOk() || !pLocale->AddCatalog(_T("filezilla")))
	{
		delete pLocale;
		return false;
	}

	// Now unload old locale
	// We unload new locale as well, else the internal locale chain in wxWidgets get's broken.
	delete pLocale;
	delete m_pLocale;
	m_pLocale = 0;

	// Finally load new one
	pLocale = new wxLocale();
	pLocale->Init(language);
	if (!pLocale->IsOk() || !pLocale->AddCatalog(_T("filezilla")))
	{
		delete pLocale;
		return false;
	}
	m_pLocale = pLocale;

	return true;
}

int CFileZillaApp::GetCurrentLanguage() const
{
	if (!m_pLocale)
		return wxLANGUAGE_ENGLISH;

	return m_pLocale->GetLanguage();
}

wxString CFileZillaApp::GetCurrentLanguageCode() const
{
	if (!m_pLocale)
		return _T("");

	return m_pLocale->GetCanonicalName();
}

#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION

void CFileZillaApp::OnFatalException()
{
	//GenerateReport(wxDebugReport::Context_Exception);
}

void CFileZillaApp::GenerateReport(wxDebugReport::Context ctx)
{
	/*
	wxDebugReport report;

	// add all standard files: currently this means just a minidump and an
	// XML file with system info and stack trace
	report.AddAll(ctx);

	// calling Show() is not mandatory, but is more polite
	if ( wxDebugReportPreviewStd().Show(report) )
	{
		if ( report.Process() )
		{
			// report successfully uploaded
		}
	}*/
	//else: user cancelled the report
}

#endif

void CFileZillaApp::DisplayEncodingWarning()
{
	static bool displayedEncodingWarning = false;
	if (displayedEncodingWarning)
		return;

	displayedEncodingWarning = true;

	wxMessageBox(_("A local filename could not be decoded.\nPlease make sure the LC_CTYPE (or LC_ALL) environment variable is set correctly.\nUnless you fix this problem, files might be missing in the file listings.\nNo further warning will be displayed this session."), _("Character encoding issue"), wxICON_EXCLAMATION);
}

CWrapEngine* CFileZillaApp::GetWrapEngine()
{
	return m_pWrapEngine;
}

void CFileZillaApp::CheckExistsFzsftp()
{
	// Get the correct path to the fzsftp executable

#ifdef __WXMAC__
	wxString executable = wxStandardPaths::Get().GetExecutablePath();
	int pos = executable.Find('/', true);
	if (pos != -1)
		executable = executable.Left(pos);
	executable += _T("/fzsftp");
	if (!wxFileName::FileExists(executable))
	{
		wxMessageBox(wxString::Format(_("%s could not be found. Without this component of FileZilla, SFTP will not work.\n\nPlease download FileZilla again. If this problem persists, please submit a bug report."), executable.c_str()),
			_("File not found"), wxICON_ERROR);
		executable.clear();
	}

#else

	wxString program = _T("fzsftp");
#ifdef __WXMSW__
	program += _T(".exe");
#endif

	bool found = false;

	// First check the FZ_FZSFTP environment variable
	wxString executable;
	if (wxGetEnv(_T("FZ_FZSFTP"), &executable))
	{
		if (wxFileName::FileExists(executable))
			found = true;
	}

	if (!found)
	{
		wxPathList pathList;

		// Add current working directory
		const wxString &cwd = wxGetCwd();
		pathList.Add(cwd);
#ifdef __WXMSW__

		// Add executable path
		wxChar modulePath[1000];
		DWORD len = GetModuleFileName(0, modulePath, 999);
		if (len)
		{
			modulePath[len] = 0;
			wxString path(modulePath);
			int pos = path.Find('\\', true);
			if (pos != -1)
			{
				path = path.Left(pos);
				pathList.Add(path);
			}
		}
#endif

		// Add a few paths relative to the current working directory
		pathList.Add(cwd + _T("/bin"));
		pathList.Add(cwd + _T("/src/putty"));
		pathList.Add(cwd + _T("/putty"));

		executable = pathList.FindAbsoluteValidPath(program);
		if (executable != _T(""))
			found = true;
	}

	if (!found)
	{
#ifdef __UNIX__
		const wxString prefix = ((const wxStandardPaths&)wxStandardPaths::Get()).GetInstallPrefix();
		if (prefix != _T("/usr/local"))
		{
			// /usr/local is the fallback value. /usr/local/bin is most likely in the PATH
			// environment variable already so we don't have to check it. Furthermore, other
			// directories might be listed before it (For example a developer's own
			// application prefix)
			wxFileName fn(prefix + _T("/bin/"), program);
			fn.Normalize();
			if (fn.FileExists())
			{
				executable = fn.GetFullPath();
				found = true;
			}
		}
#endif
	}

	if (!found)
	{
		// Check PATH
		wxPathList pathList;
		pathList.AddEnvList(_T("PATH"));
		executable = pathList.FindAbsoluteValidPath(program);
		if (executable != _T(""))
			found = true;
	}

	// Quote path if it contains spaces
	if (executable.Find(_T(" ")) != -1 && executable[0] != '"' && executable[0] != '\'')
		executable = _T("\"") + executable + _T("\"");

	if (!found)
	{
		wxMessageBox(wxString::Format(_("%s could not be found. Without this component of FileZilla, SFTP will not work.\n\nPossible solutions:\n- Make sure %s is in a directory listed in your PATH environment variable.\n- Set the full path to %s in the FZ_FZSFTP environment variable."), program.c_str(), program.c_str(), program.c_str()),
			_("File not found"), wxICON_ERROR);
		executable.clear();
	}
#endif

	// Quote path if it contains spaces
	if (executable.Find(_T(" ")) != -1 && executable[0] != '"' && executable[0] != '\'')
		executable = _T("\"") + executable + _T("\"");

	COptions::Get()->SetOption(OPTION_FZSFTP_EXECUTABLE, executable);
}

#ifdef __WXMSW__
extern "C" BOOL CALLBACK EnumWindowCallback(HWND hwnd, LPARAM lParam)
{
	HWND child = FindWindowEx(hwnd, 0, 0, _T("FileZilla process identificator 3919DB0A-082D-4560-8E2F-381A35969FB4"));
	if (child)
	{
		::PostMessage(hwnd, WM_ENDSESSION, (WPARAM)TRUE, (LPARAM)ENDSESSION_LOGOFF);
	}

	return TRUE;
}
#endif

int CFileZillaApp::ProcessCommandLine()
{
	m_pCommandLine = new CCommandLine(argc, argv);
	int res = m_pCommandLine->Parse() ? 1 : -1;

	if (res > 0)
	{
		if (m_pCommandLine->HasSwitch(CCommandLine::close))
		{
#ifdef __WXMSW__
			EnumWindows((WNDENUMPROC)EnumWindowCallback, 0);
#endif
			return 0;
		}

		if (m_pCommandLine->HasSwitch(CCommandLine::version))
		{
			wxString out = wxString::Format(_T("FileZilla %s"), CBuildInfo::GetVersion().c_str());
			if (!CBuildInfo::GetBuildType().empty())
				out += _T(" ") + CBuildInfo::GetBuildType() + _T(" build");
			out += _T(", compiled on ") + CBuildInfo::GetBuildDateString();

			printf("%s\n", (const char*)out.mb_str());
			return 0;
		}
	}

	return res;
}
