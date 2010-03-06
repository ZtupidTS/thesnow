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
#include "App.h"
#include "MainFrame.h"
#include "IniInterface.h"
#include "Plugins.h"

#include <wx/stdpaths.h>
#include "DebugTools/Debug.h"

//////////////////////////////////////////////////////////////////////////////////////////
// PathDefs Namespace -- contains default values for various pcsx2 path names and locations.
//
// Note: The members of this namespace are intended for default value initialization only.
// Most of the time you should use the path folder assignments in Conf() instead, since those
// are user-configurable.
//
namespace PathDefs
{
	namespace Base
	{
		const wxDirName& Snapshots()
		{
			static const wxDirName retval( L"snaps" );
			return retval;
		}

		const wxDirName& Savestates()
		{
			static const wxDirName retval( L"sstates" );
			return retval;
		}

		const wxDirName& MemoryCards()
		{
			static const wxDirName retval( L"memcards" );
			return retval;
		}

		const wxDirName& Settings()
		{
			static const wxDirName retval( L"inis" );
			return retval;
		}

		const wxDirName& Plugins()
		{
			static const wxDirName retval( L"plugins" );
			return retval;
		}

		const wxDirName& Logs()
		{
			static const wxDirName retval( L"logs" );
			return retval;
		}

		const wxDirName& Dumps()
		{
			static const wxDirName retval( L"dumps" );
			return retval;
		}

		const wxDirName& Themes()
		{
			static const wxDirName retval( L"themes" );
			return retval;
		}
	};

	// Specifies the root folder for the application install.
	// (currently it's the CWD, but in the future I intend to move all binaries to a "bin"
	// sub folder, in which case the approot will become "..")
	const wxDirName& AppRoot()
	{
		static const wxDirName retval( L"." );
		return retval;
	}

	// Fetches the path location for user-consumable documents -- stuff users are likely to want to
	// share with other programs: screenshots, memory cards, and savestates.
	wxDirName GetDocuments()
	{
		switch( DocsFolderMode )
		{
			case DocsFolder_User:	return (wxDirName)Path::Combine( wxStandardPaths::Get().GetDocumentsDir(), wxGetApp().GetAppName() );
			case DocsFolder_CWD:	return (wxDirName)wxGetCwd();
			case DocsFolder_Custom: return CustomDocumentsFolder;
			
			jNO_DEFAULT
		}
		
		return wxDirName();
	}

	wxDirName GetSnapshots()
	{
		return GetDocuments() + Base::Snapshots();
	}

	wxDirName GetBios()
	{
		return GetDocuments() + wxDirName( L"bios" );
	}

	wxDirName GetSavestates()
	{
		return GetDocuments() + Base::Savestates();
	}

	wxDirName GetMemoryCards()
	{
		return GetDocuments() + Base::MemoryCards();
	}

	wxDirName GetPlugins()
	{
		return AppRoot() + Base::Plugins();
	}

	wxDirName GetSettings()
	{
		return GetDocuments() + Base::Settings();
	}

	wxDirName GetThemes()
	{
		return AppRoot() + Base::Themes();
	}

	wxDirName GetLogs()
	{
		return GetDocuments() + Base::Logs();
	}

	wxDirName Get( FoldersEnum_t folderidx )
	{
		switch( folderidx )
		{
			case FolderId_Plugins:		return GetPlugins();
			case FolderId_Settings:		return GetSettings();
			case FolderId_Bios:			return GetBios();
			case FolderId_Snapshots:	return GetSnapshots();
			case FolderId_Savestates:	return GetSavestates();
			case FolderId_MemoryCards:	return GetMemoryCards();
			case FolderId_Logs:			return GetLogs();
			
			case FolderId_Documents:	return CustomDocumentsFolder;

			jNO_DEFAULT
		}
		return wxDirName();
	}
};

wxDirName& AppConfig::FolderOptions::operator[]( FoldersEnum_t folderidx )
{
	switch( folderidx )
	{
		case FolderId_Plugins:		return Plugins;
		case FolderId_Settings:		return SettingsFolder;
		case FolderId_Bios:			return Bios;
		case FolderId_Snapshots:	return Snapshots;
		case FolderId_Savestates:	return Savestates;
		case FolderId_MemoryCards:	return MemoryCards;
		case FolderId_Logs:			return Logs;
		
		case FolderId_Documents:	return CustomDocumentsFolder;

		jNO_DEFAULT
	}
	return Plugins;		// unreachable, but suppresses warnings.
}

const wxDirName& AppConfig::FolderOptions::operator[]( FoldersEnum_t folderidx ) const
{
	return const_cast<FolderOptions*>( this )->operator[]( folderidx );
}

bool AppConfig::FolderOptions::IsDefault( FoldersEnum_t folderidx ) const
{
	switch( folderidx )
	{
		case FolderId_Plugins:		return UseDefaultPlugins;
		case FolderId_Settings:		return UseDefaultSettingsFolder;
		case FolderId_Bios:			return UseDefaultBios;
		case FolderId_Snapshots:	return UseDefaultSnapshots;
		case FolderId_Savestates:	return UseDefaultSavestates;
		case FolderId_MemoryCards:	return UseDefaultMemoryCards;
		case FolderId_Logs:			return UseDefaultLogs;

		case FolderId_Documents:	return false;

		jNO_DEFAULT
	}
	return false;
}

void AppConfig::FolderOptions::Set( FoldersEnum_t folderidx, const wxString& src, bool useDefault )
{
	switch( folderidx )
	{
		case FolderId_Plugins:
			Plugins = src;
			UseDefaultPlugins = useDefault;
		break;

		case FolderId_Settings:
			SettingsFolder = src;
			UseDefaultSettingsFolder = useDefault;
		break;

		case FolderId_Bios:
			Bios = src;
			UseDefaultBios = useDefault;
		break;

		case FolderId_Snapshots:
			Snapshots = src;
			UseDefaultSnapshots = useDefault;
		break;

		case FolderId_Savestates:
			Savestates = src;
			UseDefaultSavestates = useDefault;
		break;

		case FolderId_MemoryCards:
			MemoryCards = src;
			UseDefaultMemoryCards = useDefault;
		break;

		case FolderId_Logs:
			Logs = src;
			UseDefaultLogs = useDefault;
		break;
		
		case FolderId_Documents:
			CustomDocumentsFolder = src;
		break;

		jNO_DEFAULT
	}
}

// --------------------------------------------------------------------------------------
//  Default Filenames
// --------------------------------------------------------------------------------------
namespace FilenameDefs
{
	wxFileName GetConfig()
	{
		// TODO : ini extension on Win32 is normal.  Linux ini filename default might differ
		// from this?  like pcsx2_conf or something ... ?

		return wxGetApp().GetAppName() + L".ini";
	}

	wxFileName GetUsermodeConfig()
	{
		return wxFileName( L"usermode.ini" );
	}

	const wxFileName& Memcard( uint port, uint slot )
	{
		static const wxFileName retval[2][4] =
		{
			{
				wxFileName( L"Mcd001.ps2" ),
				wxFileName( L"Mcd003.ps2" ),
				wxFileName( L"Mcd005.ps2" ),
				wxFileName( L"Mcd007.ps2" ),
			},
			{
				wxFileName( L"Mcd002.ps2" ),
				wxFileName( L"Mcd004.ps2" ),
				wxFileName( L"Mcd006.ps2" ),
				wxFileName( L"Mcd008.ps2" ),
			}
		};

		IndexBoundsCheckDev( L"FilenameDefs::Memcard", port, 2 );
		IndexBoundsCheckDev( L"FilenameDefs::Memcard", slot, 4 );

		return retval[port][slot];
	}
};

wxString AppConfig::FullpathTo( PluginsEnum_t pluginidx ) const
{
	return Path::Combine( Folders.Plugins, BaseFilenames[pluginidx] );
}

// returns true if the filenames are quite absolutely the equivalent.  Works for all
// types of filenames, relative and absolute.  Very important that you use this function
// rather than any other type of more direct string comparison!
bool AppConfig::FullpathMatchTest( PluginsEnum_t pluginId, const wxString& cmpto ) const
{
	wxFileName right( cmpto );
	wxFileName left( FullpathTo(pluginId) );

	left.MakeAbsolute();
	right.MakeAbsolute();

	return left == right;
}

wxDirName GetSettingsFolder()
{
	return UseDefaultSettingsFolder ? PathDefs::GetSettings() : SettingsFolder;
}

wxString GetSettingsFilename()
{
	return GetSettingsFolder().Combine( FilenameDefs::GetConfig() ).GetFullPath();
}


wxString AppConfig::FullpathToBios() const				{ return Path::Combine( Folders.Bios, BaseFilenames.Bios ); }
wxString AppConfig::FullpathToMcd( uint port, uint slot ) const
{
	return Path::Combine( Folders.MemoryCards, Mcd[port][slot].Filename );
}

AppConfig::AppConfig()
	: MainGuiPosition( wxDefaultPosition )
	, SysSettingsTabName( L"Cpu" )
	, AppSettingsTabName( L"GS Window" )
	, DeskTheme( L"default" )
{
	LanguageId			= wxLANGUAGE_DEFAULT;
	RecentIsoCount		= 12;
	Listbook_ImageSize	= 32;
	Toolbar_ImageSize	= 24;
	Toolbar_ShowLabels	= true;

	McdEnableNTFS		= true;
	EnableSpeedHacks	= false;
	EnableGameFixes		= false;

	CdvdSource			= CDVDsrc_Iso;

	for( uint port=0; port<2; ++port )
	{
		for( uint slot=0; slot<4; ++slot )
		{
			Mcd[port][slot].Enabled		= (slot==0);	// enables main 2 slots
			Mcd[port][slot].Filename	= FilenameDefs::Memcard( port, slot );
		}
	}
}

// ------------------------------------------------------------------------
void AppConfig::LoadSaveUserMode( IniInterface& ini, const wxString& cwdhash )
{
	IniScopedGroup path( ini, cwdhash );

	// timestamping would be useful if we want to auto-purge unused entries after
	// a period of time.  Dunno if it's needed.

	/*wxString timestamp_now( wxsFormat( L"%s %s",
		wxDateTime::Now().FormatISODate().c_str(), wxDateTime::Now().FormatISOTime().c_str() )
	);

	ini.GetConfig().Write( L"Timestamp", timestamp_now );*/

	static const wxChar* DocsFolderModeNames[] =
	{
		L"User",
		L"CWD",
		L"Custom",
	};

	ini.EnumEntry( L"DocumentsFolderMode",	DocsFolderMode,	DocsFolderModeNames, DocsFolder_User );

	ini.Entry( L"UseDefaultSettingsFolder", UseDefaultSettingsFolder,	true );
	ini.Entry( L"CustomDocumentsFolder",	CustomDocumentsFolder,		wxDirName() );
	ini.Entry( L"SettingsFolder",			SettingsFolder,				PathDefs::GetSettings() );

	ini.Flush();
}

// ------------------------------------------------------------------------
void AppConfig::LoadSaveMemcards( IniInterface& ini )
{
	AppConfig defaults;
	IniScopedGroup path( ini, L"MemoryCards" );

	for( uint port=0; port<2; ++port )
	{
		for( int slot=0; slot<4; ++slot )
		{
			ini.Entry( wxsFormat( L"Port%d_Slot%d_Enable", port, slot ),
				Mcd[port][slot].Enabled, defaults.Mcd[port][slot].Enabled );
			ini.Entry( wxsFormat( L"Port%d_Slot%d_Filename", port, slot ),
				Mcd[port][slot].Filename, defaults.Mcd[port][slot].Filename );
		}
	}
}

void AppConfig::LoadSaveRootItems( IniInterface& ini )
{
	AppConfig defaults;

	IniEntry( MainGuiPosition );
	IniEntry( SysSettingsTabName );
	IniEntry( AppSettingsTabName );
	ini.EnumEntry( L"LanguageId", LanguageId, NULL, defaults.LanguageId );
	IniEntry( RecentIsoCount );
	IniEntry( DeskTheme );
	IniEntry( Listbook_ImageSize );
	IniEntry( Toolbar_ImageSize );
	IniEntry( Toolbar_ShowLabels );

	IniEntry( CurrentIso );
	IniEntry( CurrentELF );

	IniEntry( EnableSpeedHacks );
	IniEntry( EnableGameFixes );

	ini.EnumEntry( L"CdvdSource", CdvdSource, CDVD_SourceLabels, defaults.CdvdSource );
}

// ------------------------------------------------------------------------
void AppConfig::LoadSave( IniInterface& ini )
{
	LoadSaveRootItems( ini );
	LoadSaveMemcards( ini );

	// Process various sub-components:
	ProgLogBox		.LoadSave( ini, L"ProgramLog" );

	Folders			.LoadSave( ini );
	BaseFilenames	.LoadSave( ini );
	GSWindow		.LoadSave( ini );
	Framerate		.LoadSave( ini );
	
	// Load Emulation options and apply some defaults overtop saved items, which are regulated
	// by the PCSX2 UI.

	EmuOptions.LoadSave( ini );
	if( ini.IsLoading() )
		EmuOptions.GS.LimitScalar = Framerate.NominalScalar;

	ini.Flush();
}

// ------------------------------------------------------------------------
AppConfig::ConsoleLogOptions::ConsoleLogOptions()
	: DisplayPosition( wxDefaultPosition )
	, DisplaySize( wxSize( 680, 560 ) )
{
	Visible		= false;
	AutoDock	= true;
	FontSize	= 8;
}

void AppConfig::ConsoleLogOptions::LoadSave( IniInterface& ini, const wxChar* logger )
{
	ConsoleLogOptions defaults;
	IniScopedGroup path( ini, logger );

	IniEntry( Visible );
	IniEntry( AutoDock );
	IniEntry( DisplayPosition );
	IniEntry( DisplaySize );
	IniEntry( FontSize );
}

void AppConfig::FolderOptions::ApplyDefaults()
{
	if( UseDefaultPlugins )		Plugins		= PathDefs::GetPlugins();
	if( UseDefaultBios )		Bios		= PathDefs::GetBios();
	if( UseDefaultSnapshots )	Snapshots	= PathDefs::GetSnapshots();
	if( UseDefaultSavestates )	Savestates	= PathDefs::GetSavestates();
	if( UseDefaultMemoryCards )	MemoryCards	= PathDefs::GetMemoryCards();
	if( UseDefaultLogs )		Logs		= PathDefs::GetLogs();
}

// ------------------------------------------------------------------------
AppConfig::FolderOptions::FolderOptions()
	: Plugins		( PathDefs::GetPlugins() )
	, Bios			( PathDefs::GetBios() )
	, Snapshots		( PathDefs::GetSnapshots() )
	, Savestates	( PathDefs::GetSavestates() )
	, MemoryCards	( PathDefs::GetMemoryCards() )
	, Logs			( PathDefs::GetLogs() )

	, RunIso( PathDefs::GetDocuments() )			// raw default is always the Documents folder.
	, RunELF( PathDefs::GetDocuments() )			// raw default is always the Documents folder.
{
	bitset = 0xffffffff;
}

void AppConfig::FolderOptions::LoadSave( IniInterface& ini )
{
	FolderOptions defaults;
	IniScopedGroup path( ini, L"Folders" );

	if( ini.IsSaving() )
	{
		ApplyDefaults();
	}

	IniBitBool( UseDefaultPlugins );
	IniBitBool( UseDefaultSettings );
	IniBitBool( UseDefaultBios );
	IniBitBool( UseDefaultSnapshots );
	IniBitBool( UseDefaultSavestates );
	IniBitBool( UseDefaultMemoryCards );
	IniBitBool( UseDefaultLogs );

	IniEntry( Plugins );
	IniEntry( Bios );
	IniEntry( Snapshots );
	IniEntry( Savestates );
	IniEntry( MemoryCards );
	IniEntry( Logs );

	IniEntry( RunIso );
	IniEntry( RunELF );

	if( ini.IsLoading() )
	{
		ApplyDefaults();

		if( DocsFolderMode != DocsFolder_CWD )
		{
			for( int i=0; i<FolderId_COUNT; ++i )
				operator[]( (FoldersEnum_t)i ).Normalize();
		}
	}
}

// ------------------------------------------------------------------------
const wxFileName& AppConfig::FilenameOptions::operator[]( PluginsEnum_t pluginidx ) const
{
	IndexBoundsCheckDev( L"Filename[Plugin]", pluginidx, PluginId_Count );
	return Plugins[pluginidx];
}

void AppConfig::FilenameOptions::LoadSave( IniInterface& ini )
{
	IniScopedGroup path( ini, L"Filenames" );

	static const wxFileName pc( L"Please Configure" );

	for( int i=0; i<PluginId_Count; ++i )
		ini.Entry( tbl_PluginInfo[i].GetShortname(), Plugins[i], pc );

	ini.Entry( L"BIOS", Bios, pc );
}

// ------------------------------------------------------------------------
AppConfig::GSWindowOptions::GSWindowOptions()
{
	CloseOnEsc				= true;
	DefaultToFullscreen		= false;
	AlwaysHideMouse			= false;
	DisableResizeBorders	= false;
	DisableScreenSaver		= true;

	AspectRatio				= AspectRatio_4_3;
	
	WindowSize				= wxSize( 640, 480 );
	WindowPos				= wxDefaultPosition;
	IsMaximized				= false;
}

void AppConfig::GSWindowOptions::SanityCheck()
{
	// Ensure Conformation of various options...

	WindowSize.x = std::max( WindowSize.x, 8 );
	WindowSize.x = std::min( WindowSize.x, wxGetDisplayArea().GetWidth()-16 );

	WindowSize.y = std::max( WindowSize.y, 8 );
	WindowSize.y = std::min( WindowSize.y, wxGetDisplayArea().GetHeight()-48 );

	// Make sure the upper left corner of the window is visible enought o grab and
	// move into view:
	if( !wxGetDisplayArea().Contains( wxRect( WindowPos, wxSize( 48,48 ) ) ) )
		WindowPos = wxDefaultPosition;

	if( (uint)AspectRatio >= (uint)AspectRatio_MaxCount )
		AspectRatio = AspectRatio_4_3;
}

void AppConfig::GSWindowOptions::LoadSave( IniInterface& ini )
{
	IniScopedGroup path( ini, L"GSWindow" );
	GSWindowOptions defaults;

	IniEntry( CloseOnEsc );
	IniEntry( DefaultToFullscreen );
	IniEntry( AlwaysHideMouse );
	IniEntry( DisableResizeBorders );
	IniEntry( DisableScreenSaver );
	
	IniEntry( WindowSize );
	IniEntry( WindowPos );
	
	static const wxChar* AspectRatioNames[] =
	{
		L"Stretch",
		L"4:3",
		L"16:9",
	};

	ini.EnumEntry( L"AspectRatio", AspectRatio, AspectRatioNames, defaults.AspectRatio );

	if( ini.IsLoading() ) SanityCheck();
}

// ----------------------------------------------------------------------------
AppConfig::FramerateOptions::FramerateOptions()
{
	NominalScalar			= 1.0;
	TurboScalar				= 3.0;
	SlomoScalar				= 0.33;
	
	SkipOnLimit				= false;
	SkipOnTurbo				= false;
}

void AppConfig::FramerateOptions::SanityCheck()
{
	// Ensure Conformation of various options...

	NominalScalar	.ConfineTo( 0.05, 10.0 );
	TurboScalar		.ConfineTo( 0.05, 10.0 );
	SlomoScalar		.ConfineTo( 0.05, 10.0 );
}


void AppConfig::FramerateOptions::LoadSave( IniInterface& ini )
{
	IniScopedGroup path( ini, L"Framerate" );
	FramerateOptions defaults;

	IniEntry( NominalScalar );
	IniEntry( TurboScalar );
	IniEntry( SlomoScalar );
	
	IniEntry( SkipOnLimit );
	IniEntry( SkipOnTurbo );
}


wxFileConfig* OpenFileConfig( const wxString& filename )
{
	return new wxFileConfig( wxEmptyString, wxEmptyString, filename, wxEmptyString, wxCONFIG_USE_RELATIVE_PATH );
}

void RelocateLogfile()
{
	g_Conf->Folders.Logs.Mkdir();

	wxString newlogname( Path::Combine( g_Conf->Folders.Logs.ToString(), L"emuLog.txt" ) );

	if( (emuLog != NULL) && (emuLogName != newlogname) )
	{
		Console.WriteLn( L"\nRelocating Logfile...\n\tFrom: %s\n\tTo  : %s\n", emuLogName.c_str(), newlogname.c_str() );
		wxGetApp().DisableDiskLogging();

		fclose( emuLog );
		emuLog = NULL;
	}

	if( emuLog == NULL )
	{
		emuLogName = newlogname;
		emuLog = fopen( emuLogName.ToUTF8(), "wb" );
	}

	wxGetApp().EnableAllLogging();
}

// Parameters:
//   overwrite - this option forces the current settings to overwrite any existing settings that might
//      be saved to the configured ini/settings folder.
//
void AppConfig_OnChangedSettingsFolder( bool overwrite )
{
	if( DocsFolderMode != DocsFolder_CWD )
		PathDefs::GetDocuments().Mkdir();

	GetSettingsFolder().Mkdir();

	const wxString iniFilename( GetSettingsFilename() );

	if( overwrite )
	{
		if( wxFileExists( iniFilename ) && !wxRemoveFile( iniFilename ) )
			throw Exception::AccessDenied( "Failed to overwrite settings; permission to file was denied." );
	}

	// Bind into wxConfigBase to allow wx to use our config internally, and delete whatever
	// comes out (cleans up prev config, if one).
	delete wxConfigBase::Set( OpenFileConfig( iniFilename ) );
	GetAppConfig()->SetRecordDefaults();

	if( !overwrite )
		AppLoadSettings();

	AppApplySettings();
}

// Returns the current application configuration file.  This is preferred over using
// wxConfigBase::GetAppConfig(), since it defaults to *not* creating a config file
// automatically (which is typically highly undesired behavior in our system)
wxConfigBase* GetAppConfig()
{
	return wxConfigBase::Get( false );
}
