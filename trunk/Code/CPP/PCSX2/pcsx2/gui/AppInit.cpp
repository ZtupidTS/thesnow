/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "IniInterface.h"
#include "MainFrame.h"
#include "ConsoleLogger.h"
#include "MSWstuff.h"

#include "DebugTools/Debug.h"
#include "Dialogs/ModalPopups.h"

#include <wx/cmdline.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>

static bool m_ForceWizard = false;

static void CpuCheckSSE2()
{
	if( x86caps.hasStreamingSIMD2Extensions ) return;

	// Only check once per process session:
	static bool checked = false;
	if( checked ) return;
	checked = true;

	wxDialogWithHelpers exconf( NULL, _("PCSX2 - �Ƽ� SSE2"), wxVERTICAL );

	exconf += exconf.Heading( pxE( ".Error:Startup:NoSSE2",
		L"����: ���ĵ��Բ�֧�� SSE2 ָ�, which is required by many PCSX2 recompilers and plugins. "
		L"Your options will be limited and emulation will be *very* slow." )
	);

	pxIssueConfirmation( exconf, MsgButtons().OK(), L"Error:Startup:NoSSE2" );

	// Auto-disable anything that needs SSE2:

	g_Conf->EmuOptions.Cpu.Recompiler.EnableEE	= false;
	g_Conf->EmuOptions.Cpu.Recompiler.EnableVU0	= false;
	g_Conf->EmuOptions.Cpu.Recompiler.EnableVU1	= false;
}


void Pcsx2App::OpenWizardConsole()
{
	if( !IsDebugBuild ) return;
	g_Conf->ProgLogBox.Visible = true;
	m_id_ProgramLogBox = (new ConsoleLogFrame( NULL, L"PCSX2 Program Log", g_Conf->ProgLogBox ))->GetId();
	EnableAllLogging();
}

void Pcsx2App::WipeUserModeSettings()
{
	wxDirName usrlocaldir( wxStandardPaths::Get().GetUserLocalDataDir() );
	if( !usrlocaldir.Exists() ) return;

	wxString cwd( Path::Normalize( wxGetCwd() ) );
	u32 hashres = HashTools::Hash( (char*)cwd.c_str(), cwd.Length() );

	wxFileName usermodefile( FilenameDefs::GetUsermodeConfig() );
	usermodefile.SetPath( usrlocaldir.ToString() );
	ScopedPtr<wxFileConfig> conf_usermode( OpenFileConfig( usermodefile.GetFullPath() ) );

	wxString groupname( wxsFormat( L"CWD.%08x", hashres ) );
	Console.WriteLn( "(UserModeSettings) Removing entry:" );
	Console.Indent().WriteLn( L"Path: %s\nHash:%s", cwd.c_str(), groupname.c_str() );
	conf_usermode->DeleteGroup( groupname );
}

// User mode settings can't be stored in the CWD for two reasons:
//   (a) the user may not have permission to do so (most obvious)
//   (b) it would result in sloppy usermode.ini found all over a hard drive if people runs the
//       exe from many locations (ugh).
//
// So better to use the registry on Win32 and a "default ini location" config file under Linux,
// and store the usermode settings for the CWD based on the CWD's hash.
//
void Pcsx2App::ReadUserModeSettings()
{
	wxDirName usrlocaldir( wxStandardPaths::Get().GetUserLocalDataDir() );
	if( !usrlocaldir.Exists() )
	{
		Console.WriteLn( L"Creating UserLocalData folder: " + usrlocaldir.ToString() );
		usrlocaldir.Mkdir();
	}

	wxString cwd( Path::Normalize( wxGetCwd() ) );
	u32 hashres = HashTools::Hash( (char*)cwd.c_str(), cwd.Length() );

	wxFileName usermodefile( FilenameDefs::GetUsermodeConfig() );
	usermodefile.SetPath( usrlocaldir.ToString() );
	ScopedPtr<wxFileConfig> conf_usermode( OpenFileConfig( usermodefile.GetFullPath() ) );

	wxString groupname( wxsFormat( L"CWD.%08x", hashres ) );

	if (IOP_ENABLE_SIF_HACK == 1)
	{
		wxDialogWithHelpers hackedVersion( NULL, _("It will devour your young! - PCSX2 Shub-Niggurath edition"), wxVERTICAL );

		hackedVersion.SetSizer( new wxBoxSizer( wxVERTICAL ) );
		hackedVersion += new pxStaticText( &hackedVersion,
			L"NOTICE!! This is a version of Pcsx2 with hacks enabled meant for developers only. "
			L"It will likely crash on all games, devour your young, and make you an object of shame and disgrace among your family and friends. "
			L"Do not report any bugs with this version if you received this popup. \n\nYou have been warned. ", wxALIGN_CENTER
		);
		
		hackedVersion += new wxButton( &hackedVersion, wxID_OK ) | pxSizerFlags::StdCenter();
		hackedVersion.ShowModal();
	}

	bool hasGroup = conf_usermode->HasGroup( groupname );
	bool forceWiz = m_ForceWizard || !hasGroup;
	
	if( !forceWiz )
	{
		conf_usermode->SetPath( groupname );
		forceWiz = !conf_usermode->HasEntry( L"DocumentsFolderMode" );
		conf_usermode->SetPath( L".." );
	}

	if( forceWiz )
	{
		// Pre-Alpha Warning!  Why didn't I think to add this sooner?!

		if( !hasGroup )
		{
			wxDialogWithHelpers preAlpha( NULL, _("It might devour your kittens! - PCSX2 0.9.7 Pre-Alpha"), wxVERTICAL );

			preAlpha.SetSizer( new wxBoxSizer( wxVERTICAL ) );
			preAlpha += new pxStaticText( &preAlpha,
				L"NOTICE!!  This is a *PRE-ALPHA* developer build of PCSX2 0.9.7.  We are in the middle of major rewrites of the " 
				L"user interface, and many parts of the program have *NOT* been implemented yet.  Options will be missing.  "
				L"Some things may crash or hang without warning.  Other things will seem plainly stupid and the product of incompetent "
				L"programmers.  This is normal.  We're working on it.\n\nYou have been warned!", wxALIGN_CENTER
			);
			
			preAlpha += new wxButton( &preAlpha, wxID_OK ) | pxSizerFlags::StdCenter();
			preAlpha.ShowModal();
		}
	
		// first time startup, so give the user the choice of user mode:
		OpenWizardConsole();
		FirstTimeWizard wiz( NULL );
		if( !wiz.RunWizard( wiz.GetUsermodePage() ) )
			throw Exception::StartupAborted( L"Startup aborted: User canceled FirstTime Wizard." );

		// Save user's new settings
		IniSaver saver( *conf_usermode );
		g_Conf->LoadSaveUserMode( saver, groupname );
		AppConfig_OnChangedSettingsFolder( true );
		AppSaveSettings();
	}
	else
	{
		// usermode.ini exists and is populated with valid data -- assume User Documents mode,
		// unless the ini explicitly specifies otherwise.
//		UseAdminMode = true;	//moded
		DocsFolderMode = DocsFolder_User;

		IniLoader loader( *conf_usermode );
		g_Conf->LoadSaveUserMode( loader, groupname );

		if( !wxFile::Exists( GetSettingsFilename() ) )
		{
			// user wiped their pcsx2.ini -- needs a reconfiguration via wizard!
			// (we skip the first page since it's a usermode.ini thing)

			OpenWizardConsole();
			FirstTimeWizard wiz( NULL );
			if( !wiz.RunWizard( wiz.GetPostUsermodePage() ) )
				throw Exception::StartupAborted( L"Startup aborted: User canceled Configuration Wizard." );

			// Save user's new settings
			IniSaver saver( *conf_usermode );
			g_Conf->LoadSaveUserMode( saver, groupname );
			AppConfig_OnChangedSettingsFolder( true );
			AppSaveSettings();
		}
	}
	
	// force a reset here to unload plugins loaded by the wizard.  If we don't do this
	// the recompilers might fail to allocate the memory they need to function.
	SysReset();
	sys_resume_lock = 0;
}

void Pcsx2App::DetectCpuAndUserMode()
{
	cpudetectInit();

	if( !x86caps.hasMultimediaExtensions )
	{
		// Note: due to memcpy_fast, we need minimum MMX even for interpreters.  This will
		// hopefully change later once we have a dynamically recompiled memcpy.
		throw Exception::HardwareDeficiency(L"MMX Extensions not available.", _("PCSX2 requires cpu with MMX instruction to run."));
	}

//	ReadUserModeSettings();			//mod for admin mode by thesnoW
	AppConfig_OnChangedSettingsFolder();

	PostMethod( &Pcsx2App::OpenMainFrame );
	PostMethod( &Pcsx2App::OpenConsoleLog );
	PostMethod( &Pcsx2App::AllocateCoreStuffs );
}

void Pcsx2App::OpenMainFrame()
{
	if( GetMainFramePtr() != NULL ) return;

	MainEmuFrame* mainFrame = new MainEmuFrame( NULL, L"PCSX2" );
	m_id_MainFrame = mainFrame->GetId();
	mainFrame->PushEventHandler( &GetRecentIsoManager() );

	if( wxWindow* deleteme = GetProgramLog() )
	{
		deleteme->Destroy();
		g_Conf->ProgLogBox.Visible = true;
		m_id_ProgramLogBox = wxID_ANY;
		PostIdleMethod( &Pcsx2App::OpenConsoleLog );
	}

	SetTopWindow( mainFrame );		// not really needed...
	SetExitOnFrameDelete( true );	// but being explicit doesn't hurt...
	mainFrame->Show();
}

void Pcsx2App::OpenConsoleLog()
{
	if( GetProgramLog() != NULL ) return;
	m_id_ProgramLogBox	= (new ConsoleLogFrame( GetMainFramePtr(), L"PCSX2 Program Log", g_Conf->ProgLogBox ))->GetId();
	EnableAllLogging();
}

void Pcsx2App::AllocateCoreStuffs()
{
	CpuCheckSSE2();
	SysLogMachineCaps();
	AppApplySettings();

	if( !m_CoreAllocs )
	{
		m_CoreAllocs = new SysCoreAllocations();

		if( m_CoreAllocs->HadSomeFailures( g_Conf->EmuOptions.Cpu.Recompiler ) )
		{
			// HadSomeFailures only returns 'true' if an *enabled* cpu type fails to init.  If
			// the user already has all interps configured, for example, then no point in
			// popping up this dialog.
			
			wxDialogWithHelpers exconf( NULL, _("PCSX2 Recompiler Error(s)"), wxVERTICAL );

			exconf += 12;
			exconf += exconf.Heading( pxE( ".Error:RecompilerInit",
				L"Warning: Some of the configured PS2 recompilers failed to initialize and will not be available for this session:\n" )
			);

			wxTextCtrl* scrollableTextArea = new wxTextCtrl(
				&exconf, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
				wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP
			);

			exconf += scrollableTextArea	| pxSizerFlags::StdExpand();
			
			if( !m_CoreAllocs->IsRecAvailable_EE() )
			{
				scrollableTextArea->AppendText( L"* R5900 (EE)\n\n" );

				g_Conf->EmuOptions.Recompiler.EnableEE = false;
			}

			if( !m_CoreAllocs->IsRecAvailable_IOP() )
			{
				scrollableTextArea->AppendText( L"* R3000A (IOP)\n\n" );
				g_Conf->EmuOptions.Recompiler.EnableIOP = false;
			}

			if( !m_CoreAllocs->IsRecAvailable_MicroVU0() )
			{
				scrollableTextArea->AppendText( L"* microVU0\n\n" );
				g_Conf->EmuOptions.Recompiler.UseMicroVU0	= false;
				g_Conf->EmuOptions.Recompiler.EnableVU0		= g_Conf->EmuOptions.Recompiler.EnableVU0 && m_CoreAllocs->IsRecAvailable_SuperVU0();
			}

			if( !m_CoreAllocs->IsRecAvailable_MicroVU1() )
			{
				scrollableTextArea->AppendText( L"* microVU1\n\n" );
				g_Conf->EmuOptions.Recompiler.UseMicroVU1	= false;
				g_Conf->EmuOptions.Recompiler.EnableVU1		= g_Conf->EmuOptions.Recompiler.EnableVU1 && m_CoreAllocs->IsRecAvailable_SuperVU1();
			}

			if( !m_CoreAllocs->IsRecAvailable_SuperVU0() )
			{
				scrollableTextArea->AppendText( L"* SuperVU0\n\n" );
				g_Conf->EmuOptions.Recompiler.UseMicroVU0	= m_CoreAllocs->IsRecAvailable_MicroVU0();
				g_Conf->EmuOptions.Recompiler.EnableVU0		= g_Conf->EmuOptions.Recompiler.EnableVU0 && g_Conf->EmuOptions.Recompiler.UseMicroVU0;
			}

			if( !m_CoreAllocs->IsRecAvailable_SuperVU1() )
			{
				scrollableTextArea->AppendText( L"* SuperVU1\n\n" );
				g_Conf->EmuOptions.Recompiler.UseMicroVU1	= m_CoreAllocs->IsRecAvailable_MicroVU1();
				g_Conf->EmuOptions.Recompiler.EnableVU1		= g_Conf->EmuOptions.Recompiler.EnableVU1 && g_Conf->EmuOptions.Recompiler.UseMicroVU1;
			}

			exconf += new ModalButtonPanel( &exconf, MsgButtons().OK() ) | pxSizerFlags::StdCenter();

			exconf.ShowModal();

			// Failures can be SSE-related OR memory related.  Should do per-cpu error reports instead...

			/*message += pxE( ".Popup Error:EmuCore:MemoryForRecs",
				L"These errors are the result of memory allocation failures (see the program log for details). "
				L"Closing out some memory hogging background tasks may resolve this error.\n\n"
				L"These recompilers have been disabled and interpreters will be used in their place.  "
				L"Interpreters can be very slow, so don't get too excited.  Press OK to continue or CANCEL to close PCSX2."
			);*/

			//if( !Msgbox::OkCancel( message, _("PCSX2 Initialization Error"), wxICON_ERROR ) )
			//	return false;
		}
	}

	LoadPluginsPassive( NULL );
}


void Pcsx2App::OnInitCmdLine( wxCmdLineParser& parser )
{
	parser.SetLogo( (wxString)L" >>  PCSX2  --  PC �ϵ� Playstation2 ģ����  <<\n\n" +
		_("����ѡ��ֻ�ڱ��λỰ��Ч,���ᱣ������.\n")
	);

	parser.AddParam( _("IsoFile"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );

	parser.AddSwitch( L"h",			L"help",	_("��ʾ���������ѡ���б�"), wxCMD_LINE_OPTION_HELP );

	parser.AddSwitch( wxEmptyString,L"nogui",	_("��������Ϸʱ�ر�GUI����") );
	parser.AddSwitch( wxEmptyString,L"skipbios",_("������׼ BIOS ������������") );
	parser.AddOption( wxEmptyString,L"elf",		_("ִ�� ELF ����"), wxCMD_LINE_VAL_STRING );
	parser.AddSwitch( wxEmptyString,L"nodisc",	_("������DVD����; ���ڽ��� PS2 ϵͳ�˵�") );
	parser.AddSwitch( wxEmptyString,L"usecd",	_("�������õ� CDVD ������� (���� IsoFile ����)") );

	parser.AddOption( wxEmptyString,L"cfgpath",	_("�޸������ļ�·��"), wxCMD_LINE_VAL_STRING );
	parser.AddOption( wxEmptyString,L"cfg",		_("ָ��Ҫʹ�õ� PCSX2 �����ļ� [δʵ��]"), wxCMD_LINE_VAL_STRING );

	parser.AddSwitch( wxEmptyString,L"forcewiz",_("ǿ�� PCSX2 ������һ��������") );

	const PluginInfo* pi = tbl_PluginInfo; do {
		parser.AddOption( wxEmptyString, pi->GetShortname().Lower(),
			wxsFormat( _("specify the file to use as the %s plugin"), pi->GetShortname().c_str() )
		);
	} while( ++pi, pi->shortname != NULL );

	parser.SetSwitchChars( L"-" );
}

bool Pcsx2App::OnCmdLineError( wxCmdLineParser& parser )
{
	wxApp::OnCmdLineError( parser );
	return false;
}

bool Pcsx2App::OnCmdLineParsed( wxCmdLineParser& parser )
{
	if( parser.GetParamCount() >= 1 )
	{
		// [TODO] : Unnamed parameter is taken as an "autorun" option for a cdvd/iso.
		parser.GetParam( 0 );
	}

	// Suppress wxWidgets automatic options parsing since none of them pertain to PCSX2 needs.
	//wxApp::OnCmdLineParsed( parser );

	//bool yay = parser.Found(L"nogui");
	m_ForceWizard = parser.Found( L"forcewiz" );

	const PluginInfo* pi = tbl_PluginInfo; do
	{
		wxString dest;
		if( !parser.Found( pi->GetShortname().Lower(), &dest ) ) continue;

		OverrideOptions.Filenames.Plugins[pi->id] = dest;

		if( wxFileExists( dest ) )
			Console.Warning( pi->GetShortname() + L" override: " + dest );
		else
		{
			bool result = Msgbox::OkCancel(
				wxsFormat( _("�������ʧ��!  ָ���� %s ���������:\n\n"), pi->GetShortname().c_str() ) +
				dest +
				_("���[ȷ��]ʹ��Ĭ�����õĲ��,���ߵ�[ȡ��]�ر�."),
				_("������Ǵ��� - PCSX2"), wxICON_ERROR
			);

			if( !result ) return false;
		}
	} while( ++pi, pi->shortname != NULL );

	parser.Found( L"cfgpath", &OverrideOptions.SettingsFolder );

	return true;
}

typedef void (wxEvtHandler::*pxInvokeMethodEventFunction)(pxInvokeAppMethodEvent&);
typedef void (wxEvtHandler::*pxStuckThreadEventHandler)(pxMessageBoxEvent&);

bool Pcsx2App::OnInit()
{
#define pxMethodEventHandler(func) \
	(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(pxInvokeMethodEventFunction, &func )

	Connect( pxEvt_OpenModalDialog,			wxCommandEventHandler( Pcsx2App::OnOpenModalDialog ) );

	pxDoAssert = AppDoAssert;

	g_Conf = new AppConfig();
	EnableAllLogging();

    wxInitAllImageHandlers();
	if( !_parent::OnInit() ) return false;

	m_StdoutRedirHandle = NewPipeRedir(stdout);
	m_StderrRedirHandle = NewPipeRedir(stderr);
	wxLocale::AddCatalogLookupPathPrefix( wxGetCwd() );

	Connect( pxEvt_FreezeThreadFinished,	wxCommandEventHandler	(Pcsx2App::OnFreezeThreadFinished) );
	Connect( pxEvt_CoreThreadStatus,		wxCommandEventHandler	(Pcsx2App::OnCoreThreadStatus) );
	Connect( pxEvt_LoadPluginsComplete,		wxCommandEventHandler	(Pcsx2App::OnLoadPluginsComplete) );
	Connect( pxEvt_PluginStatus,			wxCommandEventHandler	(Pcsx2App::OnPluginStatus) );
	Connect( pxEvt_SysExecute,				wxCommandEventHandler	(Pcsx2App::OnSysExecute) );
	Connect( pxEvt_InvokeMethod,			pxMethodEventHandler	(Pcsx2App::OnInvokeMethod) );

	Connect( pxID_PadHandler_Keydown, wxEVT_KEY_DOWN, wxKeyEventHandler( Pcsx2App::OnEmuKeyDown ) );

	// User/Admin Mode Dual Setup:
	//   PCSX2 now supports two fundamental modes of operation.  The default is Classic mode,
	//   which uses the Current Working Directory (CWD) for all user data files, and requires
	//   Admin access on Vista (and some Linux as well).  The second mode is the Vista-
	//   compatible \documents folder usage.  The mode is determined by the presence and
	//   contents of a usermode.ini file in the CWD.  If the ini file is missing, we assume
	//   the user is setting up a classic install.  If the ini is present, we read the value of
	//   the UserMode and SettingsPath vars.
	//
	//   Conveniently this dual mode setup applies equally well to most modern Linux distros.

	try
	{
		InitDefaultGlobalAccelerators();
		delete wxLog::SetActiveTarget( new pxLogConsole() );

		m_RecentIsoList = new RecentIsoList();

#ifdef __WXMSW__
		pxDwm_Load();
#endif

		DetectCpuAndUserMode();
	}
	// ----------------------------------------------------------------------------
	catch( Exception::StartupAborted& ex )		// user-aborted, no popups needed.
	{
		Console.Warning( ex.FormatDiagnosticMessage() );
		CleanupOnExit();
		return false;
	}
	catch( Exception::HardwareDeficiency& ex )
	{
		Msgbox::Alert( ex.FormatDisplayMessage() + _("\n\nPress OK to close PCSX2."), _("PCSX2 Error: Hardware Deficiency") );
		CleanupOnExit();
		return false;
	}
	// ----------------------------------------------------------------------------
	// Failures on the core initialization procedure (typically OutOfMemory errors) are bad,
	// since it means the emulator is completely non-functional.  Let's pop up an error and
	// exit gracefully-ish.
	//
	catch( Exception::RuntimeError& ex )
	{
		Console.Error( ex.FormatDiagnosticMessage() );
		Msgbox::Alert( ex.FormatDisplayMessage() + _("\n\n��� [ȷ��] �ر� PCSX2."),
			_("PCSX2 Critical Error"), wxICON_ERROR );
		CleanupOnExit();
		return false;
	}
    return true;
}

// This cleanup procedure can only be called when the App message pump is still active.
// OnExit() must use CleanupOnExit instead.
void Pcsx2App::CleanupRestartable()
{
	AffinityAssert_AllowFromMain();

	// app is shutting down, so don't let the system resume for anything.  (sometimes
	// there are pending Resume messages in the queue from previous user actions, and
	// this will block them from executing).
	sys_resume_lock += 10;

	PingDispatcher( "Cleanup" );
	DeletionDispatcher();

	CoreThread.Cancel();

	if( m_CorePlugins )
		m_CorePlugins->Shutdown();

	if( g_Conf )
		AppSaveSettings();

	sMainFrame.RemoveEventHandler( &GetRecentIsoManager() );
}

// This cleanup handler can be called from OnExit (it doesn't need a running message pump),
// but should not be called from the App destructor.  It's needed because wxWidgets doesn't
// always call OnExit(), so I had to make CleanupRestartable, and then encapsulate it here
// to be friendly to the OnExit scenario (no message pump).
void Pcsx2App::CleanupOnExit()
{
	AffinityAssert_AllowFromMain();

	try
	{
		CleanupRestartable();
		CleanupResources();
	}
	catch( Exception::ThreadDeadlock& )		{ throw; }
	catch( Exception::CancelEvent& )		{ throw; }
	catch( Exception::RuntimeError& ex )
	{
		// Handle runtime errors gracefully during shutdown.  Mostly these are things
		// that we just don't care about by now, and just want to "get 'er done!" so
		// we can exit the app. ;)

		Console.Error( L"Runtime exception handled during CleanupOnExit:\n" );
		Console.Indent().Error( ex.FormatDiagnosticMessage() );
	}

#ifdef __WXMSW__
	pxDwm_Unload();
#endif
	
	// Notice: deleting the plugin manager (unloading plugins) here causes Lilypad to crash,
	// likely due to some pending message in the queue that references lilypad procs.
	// We don't need to unload plugins anyway tho -- shutdown is plenty safe enough for
	// closing out all the windows.  So just leave it be and let the plugins get unloaded
	// during the wxApp destructor. -- air
	
	// FIXME: performing a wxYield() here may fix that problem. -- air

	pxDoAssert = pxAssertImpl_LogIt;
	Console_SetActiveHandler( ConsoleWriter_Stdout );
}

void Pcsx2App::CleanupResources()
{
	delete wxConfigBase::Set( NULL );

	while( wxGetLocale() != NULL )
		delete wxGetLocale();

	m_Resources = NULL;
}

int Pcsx2App::OnExit()
{
	CleanupOnExit();

	return wxApp::OnExit();
}


Pcsx2App::Pcsx2App() 
{
	m_id_MainFrame		= wxID_ANY;
	m_id_GsFrame		= wxID_ANY;
	m_id_ProgramLogBox	= wxID_ANY;

	SetAppName( L"pcsx2" );
	BuildCommandHash();
}

Pcsx2App::~Pcsx2App()
{
	pxDoAssert = pxAssertImpl_LogIt;
}

void Pcsx2App::CleanUp()
{
	CleanupResources();
	m_Resources		= NULL;
	m_RecentIsoList	= NULL;

	DisableDiskLogging();

	if( emuLog != NULL )
	{
		fclose( emuLog );
		emuLog = NULL;
	}

	_parent::CleanUp();
}


// ------------------------------------------------------------------------------------------
//  Using the MSVCRT to track memory leaks:
// ------------------------------------------------------------------------------------------
// When exiting PCSX2 normally, the CRT will make a list of all memory that's leaked.  The
// number inside {} can be pasted into the line below to cause MSVC to breakpoint on that
// allocation at the time it's made.  And then using a stacktrace you can figure out what
// leaked! :D
//
// Limitations: Unfortunately, wxWidgets gui uses a lot of heap allocations while handling
// messages, and so any mouse movements will pretty much screw up the leak value.  So to use
// this feature you need to execute pcsx in no-gui mode, and then not move the mouse or use
// the keyboard until you get to the leak. >_<
//
// (but this tool is still better than nothing!)

#ifdef PCSX2_DEBUG
struct CrtDebugBreak
{
	CrtDebugBreak( int spot )
	{
#ifdef __WXMSW__
		_CrtSetBreakAlloc( spot );
#endif
	}
};

//CrtDebugBreak breakAt( 1175 );

#endif
