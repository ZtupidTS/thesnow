//
// 設定保存クラス
//
#include "VirtuaNESres.h"
#include "DebugOut.h"

#include "Config.h"
#include "Registry.h"

#include "DirectInput.h"
#include "extsoundfile.h"

// Global instance
CConfig	Config;

// Sampling rate table
INT	CConfig::SamplingRateTable[] = {
	11025,  8, 11025, 16, 22050,  8, 22050, 16,
	32000,  8, 32000, 16,
	44100,  8, 44100, 16, 48000,  8, 48000, 16,
	96000,  8, 96000, 16, 192000, 8, 192000, 16,
	0, 0,
};

// Shortcut key IDs table
INT	CConfig::ShortcutKeyID[] = {
	// Main controls
	ID_OPEN,		IDS_CUT_OPEN,			0,
	ID_CLOSE,		IDS_CUT_CLOSE,			1,
	ID_LAUNCHER,		IDS_CUT_LAUNCHER,		2,

	ID_NETPLAY_CONNECT,	IDS_CUT_NETPLAY_CONNECT,	3,
	ID_NETPLAY_DISCONNECT,	IDS_CUT_NETPLAY_DISCONNECT,	4,
	ID_NETPLAY_CHAT,	IDS_CUT_NETPLAY_CHAT,		5,

	ID_ROMINFO,		IDS_CUT_ROMINFO,		8,
	ID_WAVERECORD,		IDS_CUT_WAVERECORD,		9,
	ID_EXIT,		IDS_CUT_EXIT,			15,

	// Emulation controls
	ID_HWRESET,		IDS_CUT_HWRESET,		16,
	ID_SWRESET,		IDS_CUT_SWRESET,		17,
	ID_PAUSE,		IDS_CUT_PAUSE,			18,
	ID_ONEFRAME,		IDS_CUT_ONEFRAME,		23,
	ID_THROTTLE,		IDS_CUT_THROTTLE,		19,
	ID_KEYTHROTTLE,		IDS_CUT_KEYTHROTTLE,		24,

	ID_FRAMESKIP_AUTO,	IDS_CUT_FRAMESKIP_AUTO,		20,
	ID_FRAMESKIP_UP,	IDS_CUT_FRAMESKIP_UP,		21,
	ID_FRAMESKIP_DOWN,	IDS_CUT_FRAMESKIP_DOWN,		22,

	// State controls
	ID_STATE_LOAD,		IDS_CUT_STATE_LOAD,		32,
	ID_STATE_SAVE,		IDS_CUT_STATE_SAVE,		33,
	ID_STATE_UP,		IDS_CUT_STATE_UP,		34,
	ID_STATE_DOWN,		IDS_CUT_STATE_DOWN,		35,
	ID_STATE_SLOT0,		IDS_CUT_STATE_SLOT0,		36,
	ID_STATE_SLOT1,		IDS_CUT_STATE_SLOT1,		37,
	ID_STATE_SLOT2,		IDS_CUT_STATE_SLOT2,		38,
	ID_STATE_SLOT3,		IDS_CUT_STATE_SLOT3,		39,
	ID_STATE_SLOT4,		IDS_CUT_STATE_SLOT4,		40,
	ID_STATE_SLOT5,		IDS_CUT_STATE_SLOT5,		41,
	ID_STATE_SLOT6,		IDS_CUT_STATE_SLOT6,		42,
	ID_STATE_SLOT7,		IDS_CUT_STATE_SLOT7,		43,
	ID_STATE_SLOT8,		IDS_CUT_STATE_SLOT8,		44,
	ID_STATE_SLOT9,		IDS_CUT_STATE_SLOT9,		45,

	// QuickLoad
	ID_QUICKLOAD_SLOT0,	IDS_CUT_QUICKLOAD_SLOT0,	256,
	ID_QUICKLOAD_SLOT1,	IDS_CUT_QUICKLOAD_SLOT1,	257,
	ID_QUICKLOAD_SLOT2,	IDS_CUT_QUICKLOAD_SLOT2,	258,
	ID_QUICKLOAD_SLOT3,	IDS_CUT_QUICKLOAD_SLOT3,	259,
	ID_QUICKLOAD_SLOT4,	IDS_CUT_QUICKLOAD_SLOT4,	260,
	ID_QUICKLOAD_SLOT5,	IDS_CUT_QUICKLOAD_SLOT5,	261,
	ID_QUICKLOAD_SLOT6,	IDS_CUT_QUICKLOAD_SLOT6,	262,
	ID_QUICKLOAD_SLOT7,	IDS_CUT_QUICKLOAD_SLOT7,	263,
	ID_QUICKLOAD_SLOT8,	IDS_CUT_QUICKLOAD_SLOT8,	264,
	ID_QUICKLOAD_SLOT9,	IDS_CUT_QUICKLOAD_SLOT9,	265,
	// QuickSave
	ID_QUICKSAVE_SLOT0,	IDS_CUT_QUICKSAVE_SLOT0,	266,
	ID_QUICKSAVE_SLOT1,	IDS_CUT_QUICKSAVE_SLOT1,	267,
	ID_QUICKSAVE_SLOT2,	IDS_CUT_QUICKSAVE_SLOT2,	268,
	ID_QUICKSAVE_SLOT3,	IDS_CUT_QUICKSAVE_SLOT3,	269,
	ID_QUICKSAVE_SLOT4,	IDS_CUT_QUICKSAVE_SLOT4,	270,
	ID_QUICKSAVE_SLOT5,	IDS_CUT_QUICKSAVE_SLOT5,	271,
	ID_QUICKSAVE_SLOT6,	IDS_CUT_QUICKSAVE_SLOT6,	272,
	ID_QUICKSAVE_SLOT7,	IDS_CUT_QUICKSAVE_SLOT7,	273,
	ID_QUICKSAVE_SLOT8,	IDS_CUT_QUICKSAVE_SLOT8,	274,
	ID_QUICKSAVE_SLOT9,	IDS_CUT_QUICKSAVE_SLOT9,	275,

	// Disk controls
	ID_DISK_EJECT,		IDS_CUT_DISK_EJECT,		48,
	ID_DISK_0A,		IDS_CUT_DISK_0A,		49,
	ID_DISK_0B,		IDS_CUT_DISK_0B,		50,
	ID_DISK_1A,		IDS_CUT_DISK_1A,		51,
	ID_DISK_1B,		IDS_CUT_DISK_1B,		52,
	ID_DISK_2A,		IDS_CUT_DISK_2A,		120,
	ID_DISK_2B,		IDS_CUT_DISK_2B,		121,
	ID_DISK_3A,		IDS_CUT_DISK_3A,		122,
	ID_DISK_3B,		IDS_CUT_DISK_3B,		123,

	// Movie controls
	ID_MOVIE_PLAY,		IDS_CUT_MOVIE_PLAY,		56,
	ID_MOVIE_REC,		IDS_CUT_MOVIE_REC,		57,
	ID_MOVIE_REC_APPEND,	IDS_CUT_MOVIE_REC_APPEND,	58,
	ID_MOVIE_STOP,		IDS_CUT_MOVIE_STOP,		59,
	ID_MOVIE_INFO,		IDS_CUT_MOVIE_INFO,		60,
	ID_MOVIE_CONVERT,	IDS_CUT_MOVIE_CONVERT,		61,

	// Screen controls
	ID_ZOOMx1,		IDS_CUT_ZOOMx1,			64,
	ID_ZOOMx2,		IDS_CUT_ZOOMx2,			65,
	ID_ZOOMx3,		IDS_CUT_ZOOMx3,			66,
	ID_ZOOMx4,		IDS_CUT_ZOOMx4,			67,
	ID_FULLSCREEN,		IDS_CUT_FULLSCREEN,		68,

	// Sound controls
	ID_MUTE_0,		IDS_CUT_MUTE_MASTER,		72,
	ID_MUTE_1,		IDS_CUT_MUTE_RECTANGLE1,	73,
	ID_MUTE_2,		IDS_CUT_MUTE_RECTANGLE2,	74,
	ID_MUTE_3,		IDS_CUT_MUTE_TRIANGLE,		75,
	ID_MUTE_4,		IDS_CUT_MUTE_NOISE,		76,
	ID_MUTE_5,		IDS_CUT_MUTE_DPCM,		77,
	ID_MUTE_6,		IDS_CUT_MUTE_EXTERNAL1,		78,
	ID_MUTE_7,		IDS_CUT_MUTE_EXTERNAL2,		79,
	ID_MUTE_8,		IDS_CUT_MUTE_EXTERNAL3,		80,
	ID_MUTE_9,		IDS_CUT_MUTE_EXTERNAL4,		81,
	ID_MUTE_A,		IDS_CUT_MUTE_EXTERNAL5,		82,
	ID_MUTE_B,		IDS_CUT_MUTE_EXTERNAL6,		83,
	ID_MUTE_C,		IDS_CUT_MUTE_EXTERNAL7,		84,
	ID_MUTE_D,		IDS_CUT_MUTE_EXTERNAL8,		85,

	// Tape controls
	ID_TAPE_PLAY,		IDS_CUT_TAPE_PLAY,		90,
	ID_TAPE_REC,		IDS_CUT_TAPE_REC,		91,
	ID_TAPE_STOP,		IDS_CUT_TAPE_STOP,		92,

	// Other controls
	ID_SNAPSHOT,		IDS_CUT_SNAPSHOT,		96,
	ID_FPSDISP,		IDS_CUT_FPSDISP,		97,
	ID_TVASPECT,		IDS_CUT_TVASPECT,		98,
	ID_TVFRAME,		IDS_CUT_TVFRAME,		99,
	ID_SCANLINE,		IDS_CUT_SCANLINE,		100,
	ID_ALLLINE,		IDS_CUT_ALLLINE,		101,
	ID_ALLSPRITE,		IDS_CUT_ALLSPRITE,		102,
	ID_LEFTCLIP,		IDS_CUT_LEFTCLIP,		105,
	ID_SYNCDRAW,		IDS_CUT_SYNCDRAW,		103,
	ID_FITSCREEN,		IDS_CUT_FITSCREEN,		104,

	// Tool controls
	ID_SEARCH,		IDS_CUT_SEARCH,			110,
	ID_CHEAT,		IDS_CUT_CHEAT,			111,
	ID_CHEAT_ENABLE,	IDS_CUT_CHEAT_ENABLE,		112,
	ID_CHEAT_DISABLE,	IDS_CUT_CHEAT_DISABLE,		113,
	ID_GENIE,		IDS_CUT_GENIE,			114,

	ID_VIEW_PATTERN,	IDS_CUT_VIEW_PATTERN,		116,
	ID_VIEW_NAMETABLE,	IDS_CUT_VIEW_NAMETABLE,		117,
	ID_VIEW_PALETTE,	IDS_CUT_VIEW_PALETTE,		118,

	ID_VIEW_MEMORY,		IDS_CUT_VIEW_MEMORY,		119,

	0, 0, 0
};

void	CConfig::Load()
{
INT	i, j;
string	ret;
wstring	section;
TCHAR	keys[64];
TCHAR	szTemp[MAX_PATH];
WORD	szKeyTemp[64];

// General
	section = _T("General");

	general.bDoubleExecute = (BOOL)CRegistry::GetProfileInt( section.c_str(), _T("DoubleExecute"), general.bDoubleExecute );
	general.bStartupLauncher = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"StartupLauncher", general.bStartupLauncher );

	general.bWindowZoom = CRegistry::GetProfileInt( section.c_str(), _T("WindowZoom"),   general.bWindowZoom );

	RECT	rc;
	if( CRegistry::GetProfileBinary( section.c_str(), L"WindowPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcWindowPos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"SearchDialogPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcSearchDlgPos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"PatternViewPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcPatternViewPos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"NameTableViewPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcNameTableViewPos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"PaletteViewPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcPaletteViewPos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"MemoryViewPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcMemoryViewPos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"BarcodePos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcBarcodePos = rc;
	}
	if( CRegistry::GetProfileBinary( section.c_str(), L"PaletteEditPos", (LPBYTE)&rc, sizeof(RECT) ) ) {
		general.rcPaletteEditPos = rc;
	}

	general.nScreenZoom = CRegistry::GetProfileInt( section.c_str(), L"ScreenZoom", general.nScreenZoom );

	general.bNoJoystickID = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"NoJoystickID", general.bNoJoystickID );

	general.nJoyAxisDisable = CRegistry::GetProfileInt( section.c_str(), L"JoyAxisDisable", general.nJoyAxisDisable );

	if( general.nJoyAxisDisable ) {
		// 以前の設定を引き継ぐ為
		WORD	bits = 0;
		switch( general.nJoyAxisDisable ) {
			case	1:
				bits = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5);
				break;
			case	2:
				bits = (1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5);
				break;
			case	3:
				bits = (1<<2)|(1<<3)|(1<<4)|(1<<5);
				break;
			case	4:
				bits = (1<<3)|(1<<4)|(1<<5);
				break;
			case	5:
				bits = (1<<4)|(1<<5);
				break;
			case	6:
				bits = (1<<5);
				break;
		}
		for( i = 0; i < 16; i++ ) {
			general.JoyAxisSetting[i] = bits;
		}

		general.nJoyAxisDisable = 0;
	} else {
		if( CRegistry::GetProfileBinary( section.c_str(), _T("JoyAxisSetting"), szKeyTemp, 16*sizeof(WORD) ) ) {
			::memcpy( general.JoyAxisSetting, szKeyTemp, 16*sizeof(WORD) );
		}
	}
// Paths
	section = L"Path";

	path.bRomPath      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"RomPathUse",      path.bRomPath );
	path.bSavePath     = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SavePathUse",     path.bSavePath );
	path.bStatePath    = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"StatePathUse",    path.bStatePath );
	path.bSnapshotPath = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SnapshotPathUse", path.bSnapshotPath );
	path.bMoviePath    = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"MoviePathUse",    path.bMoviePath );
	path.bWavePath     = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"WavePathUse",     path.bWavePath );
	path.bCheatPath    = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"CheatPathUse",    path.bCheatPath );
	path.bIpsPath      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"IpsPathUse",      path.bIpsPath );

	if( CRegistry::GetProfileString( section.c_str(), L"RomPath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szRomPath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"SavePath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szSavePath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"StatePath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szStatePath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"SnapshotPath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szSnapshotPath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"MoviePath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szMoviePath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"WavePath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szWavePath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"CheatPath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szCheatPath, szTemp );
	if( CRegistry::GetProfileString( section.c_str(), L"IpsPath", szTemp, sizeof(szTemp) ) )
		::wcscpy( path.szIpsPath, szTemp );

// Emulator
	section = L"Emulation";

	emulator.bIllegalOp     = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"IllegalOp",     emulator.bIllegalOp );
	emulator.bAutoFrameSkip = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"AutoFrameSkip", emulator.bAutoFrameSkip );
	emulator.bThrottle      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"Throttle",      emulator.bThrottle );
	emulator.nThrottleFPS   =       CRegistry::GetProfileInt( section.c_str(), L"ThrottleFPS",   emulator.nThrottleFPS );
	emulator.bBackground    = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"Background",    emulator.bBackground );
	emulator.nPriority      =       CRegistry::GetProfileInt( section.c_str(), L"Priority",      emulator.nPriority );
	emulator.bFourPlayer    = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"FourPlayer",    emulator.bFourPlayer );
	emulator.bCrcCheck      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"CrcCheck",      emulator.bCrcCheck );
	emulator.bDiskThrottle  = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"DiskThrottle",  emulator.bDiskThrottle );
	emulator.bLoadFullscreen= (BOOL)CRegistry::GetProfileInt( section.c_str(), L"LoadFullscreen",emulator.bLoadFullscreen );
	emulator.bPNGsnapshot   = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"PNGsnapshot",   emulator.bPNGsnapshot );
	emulator.bAutoIPS       = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"AutoIPS",       emulator.bAutoIPS );

// Graphic
	section = L"Graphics";

	graphics.bAspect        = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"Aspect",        graphics.bAspect );
	graphics.bAllSprite     = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SpriteMax",     graphics.bAllSprite );
	graphics.bAllLine       = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"AllLine",       graphics.bAllLine );
	graphics.bFPSDisp       = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"FPSDisp",       graphics.bFPSDisp );
	graphics.bTVFrame       = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"TVFrameMode",   graphics.bTVFrame );
	graphics.bScanline      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"ScanlineMode",  graphics.bScanline );
	graphics.nScanlineColor =       CRegistry::GetProfileInt( section.c_str(), L"ScanlineColor", graphics.nScanlineColor );
	graphics.bSyncDraw      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SyncDraw",      graphics.bSyncDraw );
	graphics.bFitZoom       = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"MaxZoom",       graphics.bFitZoom );

	graphics.bLeftClip      = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"LeftClip",      graphics.bLeftClip );

	graphics.bWindowVSync   = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"WindowVSync",   graphics.bWindowVSync );
	graphics.bSyncNoSleep   = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SyncNoSleep",   graphics.bSyncNoSleep );

	graphics.bDiskAccessLamp= (BOOL)CRegistry::GetProfileInt( section.c_str(), L"DiskAccessLamp",graphics.bDiskAccessLamp );

	graphics.bDoubleSize    = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"DoubleSize",    graphics.bDoubleSize );
	graphics.bSystemMemory  = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SystemMemory",  graphics.bSystemMemory );
	graphics.bUseHEL        = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"UseHEL",        graphics.bUseHEL );

	graphics.bNoSquareList  = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"NoSquareList",  graphics.bNoSquareList );

	graphics.nGraphicsFilter=       CRegistry::GetProfileInt( section.c_str(), L"GraphicsFilter",graphics.nGraphicsFilter );

	graphics.dwDisplayWidth  = (DWORD)CRegistry::GetProfileInt( section.c_str(), L"DisplayWidth",  graphics.dwDisplayWidth );
	graphics.dwDisplayHeight = (DWORD)CRegistry::GetProfileInt( section.c_str(), L"DisplayHeight", graphics.dwDisplayHeight );
	graphics.dwDisplayDepth  = (DWORD)CRegistry::GetProfileInt( section.c_str(), L"DisplayDepth",  graphics.dwDisplayDepth );
	graphics.dwDisplayRate   = (DWORD)CRegistry::GetProfileInt( section.c_str(), L"DisplayRate",   graphics.dwDisplayRate );

	graphics.bPaletteFile = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"PaletteUse", graphics.bPaletteFile );

	if( CRegistry::GetProfileString( section.c_str(), L"PaletteFile", szTemp, sizeof(szTemp) ) )
		::_tcscpy( graphics.szPaletteFile, szTemp );

// Sound
	section = L"Sound";

	sound.bEnable     = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"Enable",       sound.bEnable );
	sound.nRate       =       CRegistry::GetProfileInt( section.c_str(), L"SamplingRate", sound.nRate );
	sound.nBits       =       CRegistry::GetProfileInt( section.c_str(), L"SamplingBits", sound.nBits );
	sound.nBufferSize =       CRegistry::GetProfileInt( section.c_str(), L"BufferSize",   sound.nBufferSize );
	sound.nFilterType =       CRegistry::GetProfileInt( section.c_str(), L"FilterType",   sound.nFilterType );

	sound.bChangeTone = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"ChangeTone",   sound.bChangeTone );

	sound.bDisableVolumeEffect = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"DisableVolumeEffect",   sound.bDisableVolumeEffect );
	sound.bExtraSoundEnable = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"ExtraSoundEnable",   sound.bExtraSoundEnable );

	if( CRegistry::GetProfileBinary( section.c_str(), L"Volume", szTemp, sizeof(sound.nVolume) ) )
		::memcpy( sound.nVolume, szTemp, sizeof(sound.nVolume) );

// ShortCuts
	section = L"ShortCut";
	for( i = 0; i < sizeof(shortcut.nShortCut)/(16*sizeof(WORD)); i++ ) {
		::wsprintf( keys, L"Tbl%02d", i );
		if( CRegistry::GetProfileBinary( section.c_str(), keys, szTemp, 16*sizeof(WORD) ) )
			::memcpy( &shortcut.nShortCut[i*16], szTemp, 16*sizeof(WORD) );
	}

// Controllers
	for( i = 0; i < 4; i++ ) {
		::wsprintf( keys, L"Controller %d", i+1 );
		if( CRegistry::GetProfileBinary( keys, L"Keys", szKeyTemp, 64*sizeof(WORD) ) ) {
			::memcpy( controller.nButton[i], szKeyTemp, 64*sizeof(WORD) );
		} else if( CRegistry::GetProfileBinary( keys, L"Keys", szKeyTemp, 32*sizeof(WORD) ) ) {
			// 古い設定を引き継ぐ為の措置
			::memcpy( &controller.nButton[i][ 0], &szKeyTemp[ 0], 16*sizeof(WORD) );
			::memcpy( &controller.nButton[i][32], &szKeyTemp[16], 16*sizeof(WORD) );
			::memcpy( controller.nButton[i], szKeyTemp, 32*sizeof(WORD) );
		} else if( CRegistry::GetProfileBinary( keys, L"Keys", szKeyTemp, 20*sizeof(WORD) ) ) {
			// 古い設定を引き継ぐ為の措置
			::memcpy( &controller.nButton[i][ 0], &szKeyTemp[ 0], 10*sizeof(WORD) );
			::memcpy( &controller.nButton[i][32], &szKeyTemp[10], 10*sizeof(WORD) );
			// Micの変更
			if( i == 1 ) {
				controller.nButton[i][10] = szKeyTemp[ 8];
				controller.nButton[i][ 8] = 0;
				controller.nButton[i][42] = szKeyTemp[18];
				controller.nButton[i][40] = 0;
			}
		} else if( CRegistry::GetProfileBinary( keys, L"Keys", szKeyTemp, 10*sizeof(WORD) ) ) {
			// 古い設定を引き継ぐ為の措置
			::memcpy( controller.nButton[i], szKeyTemp, 10*sizeof(WORD) );
			// Micの変更
			if( i == 1 ) {
				controller.nButton[i][10] = szKeyTemp[ 8];
				controller.nButton[i][ 8] = 0;
			}
		}
		if( CRegistry::GetProfileBinary( keys, L"Rapid", szTemp, 2*sizeof(WORD) ) )
			::memcpy( controller.nRapid[i], szTemp, 2*sizeof(WORD) );
	}

// ExControllers
	for( i = 0; i < 4; i++ ) {
		if( i == 0 ) section = L"Crazy Climber";
		if( i == 1 ) section = L"Family Trainer";
		if( i == 2 ) section = L"Exciting Boxing";
		if( i == 3 ) section = L"Mahjang";

		if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 64*sizeof(WORD) ) ) {
			::memcpy( controller.nExButton[i], szKeyTemp, 64*sizeof(WORD) );
		} else if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 32*sizeof(WORD) ) ) {
			::memcpy( &controller.nExButton[i][ 0], &szKeyTemp[ 0], 16*sizeof(WORD) );
			::memcpy( &controller.nExButton[i][32], &szKeyTemp[16], 16*sizeof(WORD) );
		} else if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 20*sizeof(WORD) ) ) {
			// 古い設定を引き継ぐ為の措置
			::memcpy( &controller.nExButton[i][ 0], &szKeyTemp[ 0], 10*sizeof(WORD) );
			::memcpy( &controller.nExButton[i][32], &szKeyTemp[10], 10*sizeof(WORD) );
		} else if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 10*sizeof(WORD) ) ) {
			// 古い設定を引き継ぐ為の措置
			::memcpy( controller.nExButton[i], szKeyTemp, 10*sizeof(WORD) );
		}
	}

// NSF Contoller
	section = L"NSF controller";
	if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 64*sizeof(WORD) ) ) {
		::memcpy( controller.nNsfButton, szKeyTemp, 64*sizeof(WORD) );
	} else if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 32*sizeof(WORD) ) ) {
		::memcpy( &controller.nNsfButton[ 0], &szKeyTemp[ 0], 16*sizeof(WORD) );
		::memcpy( &controller.nNsfButton[32], &szKeyTemp[16], 16*sizeof(WORD) );
	} else if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 20*sizeof(WORD) ) ) {
		// 古い設定を引き継ぐ為の措置
		::memcpy( &controller.nNsfButton[ 0], &szKeyTemp[ 0], 10*sizeof(WORD) );
		::memcpy( &controller.nNsfButton[32], &szKeyTemp[10], 10*sizeof(WORD) );
	}

// VS-Unisystem
	section = L"VS-Unisystem";
	if( CRegistry::GetProfileBinary( section.c_str(), L"Keys", szKeyTemp, 64*sizeof(WORD) ) ) {
		::memcpy( controller.nVSUnisystem, szKeyTemp, 64*sizeof(WORD) );
	}

// Movie
	section = L"Movie";
	if( CRegistry::GetProfileBinary( section.c_str(), L"UsePlayer", szTemp, sizeof(movie.bUsePlayer) ) )
		::memcpy( movie.bUsePlayer, szTemp, sizeof(movie.bUsePlayer) );
	movie.bResetRec = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"ResetRec", movie.bResetRec );
	movie.bRerecord = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"Rerecord", movie.bRerecord );
	movie.bLoopPlay = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"LoopPlay", movie.bLoopPlay );
	movie.bPadDisplay = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"PadDisplay", movie.bPadDisplay );
	movie.bTimeDisplay = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"TimeDisplay", movie.bTimeDisplay );

// Launcher
	section = L"Launcher";

	if( CRegistry::GetProfileBinary( section.c_str(), L"WindowPos", (LPBYTE)&rc, sizeof(RECT) ) )
		launcher.rcWindowPos = rc;
	if( CRegistry::GetProfileBinary( section.c_str(), L"ColumnView", szTemp, sizeof(launcher.bHeaderView) ) )
		::memcpy( launcher.bHeaderView, szTemp, sizeof(launcher.bHeaderView) );
	if( CRegistry::GetProfileBinary( section.c_str(), L"ColumnOrder", szTemp, sizeof(launcher.nHeaderOrder) ) )
		::memcpy( launcher.nHeaderOrder, szTemp, sizeof(launcher.nHeaderOrder) );
	if( CRegistry::GetProfileBinary( section.c_str(), L"ColumnWidth", szTemp, sizeof(launcher.nHeaderWidth) ) )
		::memcpy( launcher.nHeaderWidth, szTemp, sizeof(launcher.nHeaderWidth) );

	launcher.nListSelect = CRegistry::GetProfileInt( section.c_str(), L"ListSelect",  launcher.nListSelect );

	launcher.bSortDir  = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"SortDir",  launcher.bSortDir );
	launcher.nSortType =       CRegistry::GetProfileInt( section.c_str(), L"SortType", launcher.nSortType );

	if( CRegistry::GetProfileBinary( section.c_str(), L"ColumnSort", szTemp, sizeof(launcher.nHeaderWidth) ) )
		::memcpy( launcher.nHeaderWidth, szTemp, sizeof(launcher.nHeaderWidth) );

	if( CRegistry::GetProfileBinary( section.c_str(), L"FolderUse", szTemp, sizeof(launcher.bFolderUse) ) )
		::memcpy( launcher.bFolderUse, szTemp, sizeof(launcher.bFolderUse) );

	for( i = 0; i < 16; i++ ) {
		::wsprintf( keys, L"Folder%02d", i );
		if( CRegistry::GetProfileString( section.c_str(), keys, szTemp, sizeof(szTemp) ) )
			::_tcscpy( launcher.szFolder[i], szTemp );
	}

	if( CRegistry::GetProfileString( section.c_str(), L"LastSelect", szTemp, sizeof(szTemp) ) )
		::_tcscpy( launcher.szLastSelect, szTemp );

	launcher.bActivePause = (BOOL)CRegistry::GetProfileInt( section.c_str(), L"ActivePause",  launcher.bActivePause );

// ExtraSound
	section = L"ExtraSound";

	for( i = ESF_MOEPRO_STRIKE; i <= ESF_MOEPRO_WA; i++ ) {
		::wsprintf( keys, L"Moepro%02d", i );
		if( CRegistry::GetProfileString( section.c_str(), keys, szTemp, sizeof(szTemp) ) )
			::_tcscpy( extsound.szExtSoundFile[i], szTemp );
	}

	for( i = ESF_MOETENNIS_00, j = 0; i <= ESF_MOETENNIS_12; i++, j++ ) {
		::wsprintf( keys, L"Moetennis%02d", j );
		if( CRegistry::GetProfileString( section.c_str(), keys, szTemp, sizeof(szTemp) ) )
			::_tcscpy( extsound.szExtSoundFile[i], szTemp );
	}

	for( i = ESF_DISKSYSTEM_BOOT, j = 0; i <= ESF_DISKSYSTEM_SEEKEND; i++, j++ ) {
		::wsprintf( keys, L"DiskSound%02d", j );
		if( CRegistry::GetProfileString( section.c_str(), keys, szTemp, sizeof(szTemp) ) )
			::_tcscpy( extsound.szExtSoundFile[i], szTemp );
	}

// NetPlay
	section = L"Netplay";

	if( CRegistry::GetProfileBinary( section.c_str(), L"ChatPos", (LPBYTE)&rc, sizeof(RECT) ) )
		netplay.rcChatPos = rc;

	if( CRegistry::GetProfileString( section.c_str(), L"NickName", szTemp, sizeof(szTemp) ) )
		::_tcscpy( netplay.szNick, szTemp );

	netplay.nRecentPort = CRegistry::GetProfileInt( section.c_str(), L"RecnetPortNum", netplay.nRecentPort );
	for( i = 0; i < netplay.nRecentPort; i++ ) {
		::wsprintf( keys, L"RecentPort%02d", i );
		if( CRegistry::GetProfileString( section.c_str(), keys, szTemp, sizeof(szTemp) ) )
			::_tcscpy(netplay.szRecentPort[i], szTemp );
	}

	netplay.nRecentHost = CRegistry::GetProfileInt( section.c_str(), L"RecnetHostNum", netplay.nRecentHost );
	for( i = 0; i < netplay.nRecentHost; i++ ) {
		::wsprintf( keys, L"RecentHost%02d", i );
		if( CRegistry::GetProfileString( section.c_str(), keys, szTemp, sizeof(szTemp) ) )
			::_tcscpy( netplay.szRecentHost[i], szTemp );
	}
}

void	CConfig::Save()
{
INT	i;
wstring	section;
TCHAR	keys[64];

// General
	section = L"General";

	CRegistry::WriteProfileInt   ( section.c_str(), L"DoubleExecute",   general.bDoubleExecute );
	CRegistry::WriteProfileInt   ( section.c_str(), L"StartupLauncher", general.bStartupLauncher );

//	CRegistry::WriteProfileInt( section.c_str(), "WindowPosSave",	general.bWindowSave );
	CRegistry::WriteProfileInt   ( section.c_str(), L"WindowZoom",	general.bWindowZoom );
	CRegistry::WriteProfileBinary( section.c_str(), L"WindowPos",	(LPBYTE)&general.rcWindowPos, sizeof(RECT) );
	CRegistry::WriteProfileInt   ( section.c_str(), L"ScreenZoom",	general.nScreenZoom );

	CRegistry::WriteProfileBinary( section.c_str(), L"SearchDialogPos", (LPBYTE)&general.rcSearchDlgPos, sizeof(RECT) );

	CRegistry::WriteProfileBinary( section.c_str(), L"PatternViewPos", (LPBYTE)&general.rcPatternViewPos, sizeof(RECT) );
	CRegistry::WriteProfileBinary( section.c_str(), L"NameTableViewPos", (LPBYTE)&general.rcNameTableViewPos, sizeof(RECT) );
	CRegistry::WriteProfileBinary( section.c_str(), L"PaletteViewPos", (LPBYTE)&general.rcPaletteViewPos, sizeof(RECT) );
	CRegistry::WriteProfileBinary( section.c_str(), L"MemoryViewPos", (LPBYTE)&general.rcMemoryViewPos, sizeof(RECT) );

	CRegistry::WriteProfileBinary( section.c_str(), L"BarcodePos", (LPBYTE)&general.rcBarcodePos, sizeof(RECT) );

	CRegistry::WriteProfileBinary( section.c_str(), L"PaletteEditPos", (LPBYTE)&general.rcPaletteEditPos, sizeof(RECT) );

	CRegistry::WriteProfileInt   ( section.c_str(), L"JoyAxisDisable", general.nJoyAxisDisable );
	CRegistry::WriteProfileBinary( section.c_str(), L"JoyAxisSetting", (LPBYTE)general.JoyAxisSetting, 16*sizeof(WORD) );

	CRegistry::WriteProfileInt   ( section.c_str(), L"NoJoystickID", general.bNoJoystickID );

// Paths
	section = L"Path";

	CRegistry::WriteProfileInt( section.c_str(), L"RomPathUse",      path.bRomPath );
	CRegistry::WriteProfileInt( section.c_str(), L"SavePathUse",     path.bSavePath );
	CRegistry::WriteProfileInt( section.c_str(), L"StatePathUse",    path.bStatePath );
	CRegistry::WriteProfileInt( section.c_str(), L"SnapshotPathUse", path.bSnapshotPath );
	CRegistry::WriteProfileInt( section.c_str(), L"MoviePathUse",    path.bMoviePath );
	CRegistry::WriteProfileInt( section.c_str(), L"WavePathUse",     path.bWavePath );
	CRegistry::WriteProfileInt( section.c_str(), L"CheatPathUse",    path.bCheatPath );
	CRegistry::WriteProfileInt( section.c_str(), L"IpsPathUse",      path.bIpsPath );

	CRegistry::WriteProfileString( section.c_str(), L"RomPath",      (LPCWSTR)path.szRomPath );
	CRegistry::WriteProfileString( section.c_str(), L"SavePath",     (LPCWSTR)path.szSavePath );
	CRegistry::WriteProfileString( section.c_str(), L"StatePath",    (LPCWSTR)path.szStatePath );
	CRegistry::WriteProfileString( section.c_str(), L"SnapshotPath", (LPCWSTR)path.szSnapshotPath );
	CRegistry::WriteProfileString( section.c_str(), L"MoviePath",    (LPCWSTR)path.szMoviePath );
	CRegistry::WriteProfileString( section.c_str(), L"WavePath",     (LPCWSTR)path.szWavePath );
	CRegistry::WriteProfileString( section.c_str(), L"CheatPath",    (LPCWSTR)path.szCheatPath );
	CRegistry::WriteProfileString( section.c_str(), L"IpsPath",      (LPCWSTR)path.szIpsPath );

// Emulation
	section = L"Emulation";

	CRegistry::WriteProfileInt( section.c_str(), L"IllegalOp",     emulator.bIllegalOp );
	CRegistry::WriteProfileInt( section.c_str(), L"AutoFrameSkip", emulator.bAutoFrameSkip );
	CRegistry::WriteProfileInt( section.c_str(), L"Throttle",      emulator.bThrottle );
	CRegistry::WriteProfileInt( section.c_str(), L"ThrottleFPS",   emulator.nThrottleFPS );
	CRegistry::WriteProfileInt( section.c_str(), L"Background",    emulator.bBackground );
	CRegistry::WriteProfileInt( section.c_str(), L"Priority",      emulator.nPriority );
	CRegistry::WriteProfileInt( section.c_str(), L"FourPlayer",    emulator.bFourPlayer );
	CRegistry::WriteProfileInt( section.c_str(), L"CrcCheck",      emulator.bCrcCheck );
	CRegistry::WriteProfileInt( section.c_str(), L"DiskThrottle",  emulator.bDiskThrottle );
	CRegistry::WriteProfileInt( section.c_str(), L"LoadFullscreen",emulator.bLoadFullscreen );
	CRegistry::WriteProfileInt( section.c_str(), L"PNGsnapshot",   emulator.bPNGsnapshot );
	CRegistry::WriteProfileInt( section.c_str(), L"AutoIPS",       emulator.bAutoIPS );

// Graphic
	section = L"Graphics";

	CRegistry::WriteProfileInt( section.c_str(), L"Aspect",        graphics.bAspect );
	CRegistry::WriteProfileInt( section.c_str(), L"SpriteMax",     graphics.bAllSprite );
	CRegistry::WriteProfileInt( section.c_str(), L"AllLine",       graphics.bAllLine );
	CRegistry::WriteProfileInt( section.c_str(), L"FPSDisp",       graphics.bFPSDisp );
	CRegistry::WriteProfileInt( section.c_str(), L"TVFrameMode",   graphics.bTVFrame );
	CRegistry::WriteProfileInt( section.c_str(), L"ScanlineMode",  graphics.bScanline );
	CRegistry::WriteProfileInt( section.c_str(), L"ScanlineColor", graphics.nScanlineColor );
	CRegistry::WriteProfileInt( section.c_str(), L"SyncDraw",      graphics.bSyncDraw );
	CRegistry::WriteProfileInt( section.c_str(), L"MaxZoom",       graphics.bFitZoom );

	CRegistry::WriteProfileInt( section.c_str(), L"LeftClip",      graphics.bLeftClip );

	CRegistry::WriteProfileInt( section.c_str(), L"WindowVSync",   graphics.bWindowVSync );
	CRegistry::WriteProfileInt( section.c_str(), L"SyncNoSleep",   graphics.bSyncNoSleep );

	CRegistry::WriteProfileInt( section.c_str(), L"DiskAccessLamp",graphics.bDiskAccessLamp );

	CRegistry::WriteProfileInt( section.c_str(), L"DoubleSize",    graphics.bDoubleSize );
	CRegistry::WriteProfileInt( section.c_str(), L"SystemMemory",  graphics.bSystemMemory );
	CRegistry::WriteProfileInt( section.c_str(), L"UseHEL",        graphics.bUseHEL );

	CRegistry::WriteProfileInt( section.c_str(), L"NoSquareList",  graphics.bNoSquareList );

	CRegistry::WriteProfileInt( section.c_str(), L"GraphicsFilter",graphics.nGraphicsFilter );

	CRegistry::WriteProfileInt( section.c_str(), L"DisplayWidth",  graphics.dwDisplayWidth );
	CRegistry::WriteProfileInt( section.c_str(), L"DisplayHeight", graphics.dwDisplayHeight );
	CRegistry::WriteProfileInt( section.c_str(), L"DisplayDepth",  graphics.dwDisplayDepth );
	CRegistry::WriteProfileInt( section.c_str(), L"DisplayRate",   graphics.dwDisplayRate );

	CRegistry::WriteProfileInt( section.c_str(), L"PaletteUse",   graphics.bPaletteFile );

// Sound
	section = L"Sound";

	CRegistry::WriteProfileInt( section.c_str(), L"Enable",       sound.bEnable );

	CRegistry::WriteProfileInt( section.c_str(), L"DisableVolumeEffect", sound.bDisableVolumeEffect );
	CRegistry::WriteProfileInt( section.c_str(), L"ExtraSoundEnable",    sound.bExtraSoundEnable );

	CRegistry::WriteProfileInt( section.c_str(), L"SamplingRate", sound.nRate );
	CRegistry::WriteProfileInt( section.c_str(), L"SamplingBits", sound.nBits );
	CRegistry::WriteProfileInt( section.c_str(), L"BufferSize",   sound.nBufferSize );
	CRegistry::WriteProfileInt( section.c_str(), L"FilterType",   sound.nFilterType );

	CRegistry::WriteProfileBinary( section.c_str(), L"Volume", (LPBYTE)sound.nVolume, sizeof(sound.nVolume) );

// ShortCut
	section = L"ShortCut";
	for( i = 0; i < sizeof(shortcut.nShortCut)/(16*sizeof(WORD)); i++ ) {
		::wsprintf( keys, L"TBL%02d", i );
		CRegistry::WriteProfileBinary( section.c_str(), keys, (LPBYTE)&shortcut.nShortCut[i*16], 16*sizeof(WORD) );
	}

// Controllers
	for( i = 0; i < 4; i++ ) {
		::wsprintf( keys, L"Controller %01d", i+1 );
		CRegistry::WriteProfileBinary( keys, L"Keys", (LPBYTE)controller.nButton[i], 64*sizeof(WORD) );
		CRegistry::WriteProfileBinary( keys, L"Rapid", (LPBYTE)controller.nRapid[i], 2*sizeof(WORD) );
	}

// ExControllers
	section = L"Crazy Climber";
	CRegistry::WriteProfileBinary( section.c_str(), L"Keys", (LPBYTE)controller.nExButton[0], 64*sizeof(WORD) );
	section = L"Family Trainer";
	CRegistry::WriteProfileBinary( section.c_str(), L"Keys", (LPBYTE)controller.nExButton[1], 64*sizeof(WORD) );
	section = L"Exciting Boxing";
	CRegistry::WriteProfileBinary( section.c_str(), L"Keys", (LPBYTE)controller.nExButton[2], 64*sizeof(WORD) );
	section = L"Mahjang";
	CRegistry::WriteProfileBinary( section.c_str(), L"Keys", (LPBYTE)controller.nExButton[3], 64*sizeof(WORD) );

// NSF Contoller
	section = L"NSF controller";
	CRegistry::WriteProfileBinary( section.c_str(), L"Keys", (LPBYTE)controller.nNsfButton, 64*sizeof(WORD) );

// VS-Unisystem
	section = L"VS-Unisystem";
	CRegistry::WriteProfileBinary( section.c_str(), L"Keys", (LPBYTE)controller.nVSUnisystem, 64*sizeof(WORD) );

// Movie
	section = L"Movie";
	CRegistry::WriteProfileBinary( section.c_str(), L"UsePlayer", (LPBYTE)movie.bUsePlayer, sizeof(movie.bUsePlayer) );
	CRegistry::WriteProfileInt( section.c_str(), L"ResetRec", movie.bResetRec );
	CRegistry::WriteProfileInt( section.c_str(), L"Rerecord", movie.bRerecord );
	CRegistry::WriteProfileInt( section.c_str(), L"LoopPlay", movie.bLoopPlay );
	CRegistry::WriteProfileInt( section.c_str(), L"PadDisplay", movie.bPadDisplay );
	CRegistry::WriteProfileInt( section.c_str(), L"TimeDisplay", movie.bTimeDisplay );

// Launcher
	section = L"Launcher";

	CRegistry::WriteProfileBinary( section.c_str(), L"WindowPos", (LPBYTE)&launcher.rcWindowPos, sizeof(RECT) );

	CRegistry::WriteProfileBinary( section.c_str(), L"ColumnView",  (LPBYTE)launcher.bHeaderView,  sizeof(launcher.bHeaderView) );
	CRegistry::WriteProfileBinary( section.c_str(), L"ColumnOrder", (LPBYTE)launcher.nHeaderOrder, sizeof(launcher.nHeaderOrder) );
	CRegistry::WriteProfileBinary( section.c_str(), L"ColumnWidth", (LPBYTE)launcher.nHeaderWidth, sizeof(launcher.nHeaderWidth) );

	CRegistry::WriteProfileInt( section.c_str(), L"ListSelect", launcher.nListSelect );

	CRegistry::WriteProfileInt( section.c_str(), L"SortDir",  launcher.bSortDir );
	CRegistry::WriteProfileInt( section.c_str(), L"SortType", launcher.nSortType );

	CRegistry::WriteProfileBinary( section.c_str(), L"FolderUse", (LPBYTE)launcher.bFolderUse, sizeof(launcher.bFolderUse) );

	for( i = 0; i < 16; i++ ) {
		::wsprintf( keys, L"Folder%02d", i );
		CRegistry::WriteProfileString( section.c_str(), keys, (LPCWSTR)launcher.szFolder[i] );
	}

	CRegistry::WriteProfileString( section.c_str(), L"LastSelect", (LPCWSTR)launcher.szLastSelect );

	CRegistry::WriteProfileInt( section.c_str(), L"ActivePause",  launcher.bActivePause );

// NetPlay
	section = L"Netplay";

	CRegistry::WriteProfileBinary( section.c_str(), L"ChatPos", (LPBYTE)&netplay.rcChatPos, sizeof(RECT) );

	CRegistry::WriteProfileString( section.c_str(), L"NickName", (LPCWSTR)netplay.szNick );

	CRegistry::WriteProfileInt( section.c_str(), L"RecnetPortNum", netplay.nRecentPort );
	for( i = 0; i < 16; i++ ) {
		::wsprintf( keys, L"RecentPort%02d", i );
		CRegistry::WriteProfileString( section.c_str(), keys, (LPCWSTR)netplay.szRecentPort[i] );
	}
	CRegistry::WriteProfileInt( section.c_str(), L"RecnetHostNum", netplay.nRecentHost );
	for( i = 0; i < 16; i++ ) {
		::wsprintf( keys, L"RecentHost%02d", i );
		CRegistry::WriteProfileString( section.c_str(), keys, (LPCWSTR)netplay.szRecentHost[i] );
	}
}

BOOL	CConfig::ButtonCheck( INT nNo, INT nID )
{
	if( m_bKeyboardDisable ) {
		if( (Config.controller.nButton[nNo][nID+ 0] >= 256) && DirectInput.m_Sw[Config.controller.nButton[nNo][nID+ 0]]
		 || (Config.controller.nButton[nNo][nID+32] >= 256) && DirectInput.m_Sw[Config.controller.nButton[nNo][nID+32]] )
			return	TRUE;
	} else {
		if( Config.controller.nButton[nNo][nID+ 0] && DirectInput.m_Sw[Config.controller.nButton[nNo][nID+ 0]]
		 || Config.controller.nButton[nNo][nID+32] && DirectInput.m_Sw[Config.controller.nButton[nNo][nID+32]] )
			return	TRUE;
	}
	return	FALSE;
}

BOOL	CConfig::ExButtonCheck( INT nNo, INT nID )
{
	if( Config.controller.nExButton[nNo][nID+ 0] && DirectInput.m_Sw[Config.controller.nExButton[nNo][nID+ 0]]
	 || Config.controller.nExButton[nNo][nID+32] && DirectInput.m_Sw[Config.controller.nExButton[nNo][nID+32]] )
		return	TRUE;
	return	FALSE;
}

BOOL	CConfig::NsfButtonCheck( INT nID )
{
	if( Config.controller.nNsfButton[nID+ 0] && DirectInput.m_Sw[Config.controller.nNsfButton[nID+ 0]]
	 || Config.controller.nNsfButton[nID+32] && DirectInput.m_Sw[Config.controller.nNsfButton[nID+32]] )
		return	TRUE;
	return	FALSE;
}

BOOL	CConfig::ButtonCheck( INT nID, WORD* pKey )
{
	if( pKey[nID+ 0] && DirectInput.m_Sw[pKey[nID+ 0]]
	 || pKey[nID+32] && DirectInput.m_Sw[pKey[nID+32]] )
		return	TRUE;
	return	FALSE;
}

wstring	CConfig::ShortcutToKeyName( INT nShortcutKey )
{
	wstring	str;
	if( nShortcutKey == 0 ) {
		str = L"----";
	} else {
		if( nShortcutKey & CCfgShortCut::K_ALT )
			str = str + L"Alt+";
		if( nShortcutKey & CCfgShortCut::K_CTRL )
			str = str + L"Ctrl+";
		if( nShortcutKey & CCfgShortCut::K_SHIFT )
			str = str + L"Shift+";

		str = str + DirectInput.SearchKeyName( nShortcutKey & 0x0FFF );
	}
	return	str;
}
/*
string	CConfig::ShortcutToKeyName( INT nShortcutKey )
{
	string	str;
	if( nShortcutKey == 0 ) {
		str = "----";
	} else {
		if( nShortcutKey & CCfgShortCut::K_ALT )
			str = str + "Alt+";
		if( nShortcutKey & CCfgShortCut::K_CTRL )
			str = str + "Ctrl+";
		if( nShortcutKey & CCfgShortCut::K_SHIFT )
			str = str + "Shift+";

		str = str + DirectInput.SearchKeyName( nShortcutKey & 0x0FFF );
	}
	return	str;
}
*/

/////////////////////////////////
CGameOption	GameOption;

void	CGameOption::Load( DWORD crc )
{
	CRegistry::SetRegistryKey( L"GameOption.ini" );

	TCHAR	szSection[256];
	::wsprintf( szSection, L"%08X", crc );
	nRenderMethod =       CRegistry::GetProfileInt( szSection, L"RenderMethod",    defRenderMethod );
	nIRQtype      =       CRegistry::GetProfileInt( szSection, L"IRQtype",         defIRQtype );
	bFrameIRQ     = (BOOL)CRegistry::GetProfileInt( szSection, L"FrameIRQ",        defFrameIRQ );
	bVideoMode    = (BOOL)CRegistry::GetProfileInt( szSection, L"VideoMode",       defVideoMode );
}

void	CGameOption::Save( LPCWSTR name, DWORD crc )
{
	CRegistry::SetRegistryKey( L"GameOption.ini" );

	TCHAR	szSection[256];
	::wsprintf( szSection, L"%08X", crc );
	CRegistry::WriteProfileString( szSection, L"Title", name );
	CRegistry::WriteProfileInt( szSection, L"RenderMethod",    nRenderMethod );
	CRegistry::WriteProfileInt( szSection, L"IRQtype",         nIRQtype );
	CRegistry::WriteProfileInt( szSection, L"FrameIRQ",        (INT)bFrameIRQ );
	CRegistry::WriteProfileInt( szSection, L"VideoMode",       (INT)bVideoMode );
}

void	CGameOption::Load( DWORD gid, DWORD mid )
{
	CRegistry::SetRegistryKey( L"GameOption.ini" );

	TCHAR	szSection[256];
	::wsprintf( szSection, L"%08X%08X", gid, mid );
	nRenderMethod =       CRegistry::GetProfileInt( szSection, L"RenderMethod",    defRenderMethod );
	nIRQtype      =       CRegistry::GetProfileInt( szSection, L"IRQtype",         defIRQtype );
	bFrameIRQ     = (BOOL)CRegistry::GetProfileInt( szSection, L"FrameIRQ",        defFrameIRQ );
}

void	CGameOption::Save( LPCWSTR name, DWORD gid, DWORD mid )
{
	CRegistry::SetRegistryKey( L"GameOption.ini" );

	TCHAR	szSection[256];
	::wsprintf( szSection, L"%08X%08X", gid, mid );
	CRegistry::WriteProfileString( szSection, L"Title", name );
	CRegistry::WriteProfileInt( szSection, L"RenderMethod",    nRenderMethod );
	CRegistry::WriteProfileInt( szSection, L"IRQtype",         nIRQtype );
	CRegistry::WriteProfileInt( szSection, L"FrameIRQ",        (INT)bFrameIRQ );
}

