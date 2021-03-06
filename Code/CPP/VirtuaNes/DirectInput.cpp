//
// DirectInput class
//
#include "DebugOut.h"
#include "DirectInput.h"
#include "COM.h"
#include "Config.h"

CDirectInput	DirectInput;

#define	COMUSE	TRUE

//
// Table
//
CDirectInput::DIKEYTBL	CDirectInput::DIKeyTable[] = {
	DIK_ESCAPE,	L"ESC",		DIK_1,		L"1",
	DIK_2,		L"2",		DIK_3,		L"3",
	DIK_4,		L"4",		DIK_5,		L"5",
	DIK_6,		L"6",		DIK_7,		L"7",
	DIK_8,		L"8",		DIK_9,		L"9",
	DIK_0,		L"0",		DIK_MINUS,	L"-",
	DIK_EQUALS,	L"=",		DIK_BACK,	L"BackSpace",
	DIK_TAB,	L"TAB",		DIK_Q,		L"Q",
	DIK_W,		L"W",		DIK_E,		L"E",
	DIK_R,		L"R",		DIK_T,		L"T",
	DIK_Y,		L"Y",		DIK_U,		L"U",
	DIK_I,		L"I",		DIK_O,		L"O",
	DIK_P,		L"P",		DIK_LBRACKET,	L"[",
	DIK_RBRACKET,	L"]",		DIK_RETURN,	L"Enter",
	DIK_LCONTROL,	L"L Ctrl",	DIK_A,		L"A",
	DIK_S,		L"S",		DIK_D,		L"D",
	DIK_F,		L"F",		DIK_G,		L"G",
	DIK_H,		L"H",		DIK_J,		L"J",
	DIK_K,		L"K",		DIK_L,		L"L",
	DIK_SEMICOLON,	L";",		DIK_APOSTROPHE,	L"'",
	DIK_GRAVE,	L"`",		DIK_LSHIFT,	L"L Shift",
	DIK_BACKSLASH,	L"\\",		DIK_Z,		L"Z",
	DIK_X,		L"X",		DIK_C,		L"C",
	DIK_V,		L"V",		DIK_B,		L"B",
	DIK_N,		L"N",		DIK_M,		L"M",
	DIK_COMMA,	L",",		DIK_PERIOD,	L".",
	DIK_SLASH,	L"/",		DIK_RSHIFT,	L"R Shift",
	DIK_MULTIPLY,	L"*",		DIK_LMENU,	L"L Alt",
	DIK_SPACE,	L"Space",
	DIK_F1,		L"F1",		DIK_F2,		L"F2",
	DIK_F3,		L"F3",		DIK_F4,		L"F4",
	DIK_F5,		L"F5",		DIK_F6,		L"F6",
	DIK_F7,		L"F7",		DIK_F8,		L"F8",
	DIK_F9,		L"F9",		DIK_F10,	L"F10",

	DIK_NUMPAD7,	L"Num 7",	DIK_NUMPAD8,	L"Num 8",
	DIK_NUMPAD9,	L"Num 9",	DIK_SUBTRACT,	L"Num -",
	DIK_NUMPAD4,	L"Num 4",	DIK_NUMPAD5,	L"Num 5",
	DIK_NUMPAD6,	L"Num 6",	DIK_ADD,	L"Num +",
	DIK_NUMPAD1,	L"Num 1",	DIK_NUMPAD2,	L"Num 2",
	DIK_NUMPAD3,	L"Num 3",	DIK_NUMPAD0,	L"Num 0",
	DIK_DECIMAL,	L"Num .",	DIK_F11,	L"F11",
	DIK_F12,	L"F12",		DIK_F13,	L"F13",
	DIK_F14,	L"F14",		DIK_F15,	L"F15",
	DIK_CONVERT,	L"変換",
	DIK_NOCONVERT,	_T("無変換"),	DIK_YEN,	L"\\",
	DIK_NUMPADEQUALS,L"Num =",	DIK_CIRCUMFLEX,	L"^",
	DIK_AT,		L"@",		DIK_COLON,	L":",
	DIK_UNDERLINE,	L"_",
	DIK_STOP,	L"Stop",		DIK_NUMPADENTER,L"Num Enter",
	DIK_RCONTROL,	L"R Ctrl",	DIK_NUMPADCOMMA,L"Num ,",
	DIK_DIVIDE,	L"Num /",	DIK_SYSRQ,	L"SysRq",
	DIK_RMENU,	L"R Alt",	DIK_PAUSE,	L"Pause",
	DIK_HOME,	L"Home",		DIK_UP,		L"Up",
	DIK_PRIOR,	L"Page Up",	DIK_LEFT,	L"Left",
	DIK_RIGHT,	L"Right",	DIK_END,	L"End",
	DIK_DOWN,	L"Down",		DIK_NEXT,	L"Page Down",
	DIK_INSERT,	L"Insert",	DIK_DELETE,	L"Delete",
	DIK_LWIN,	L"L Windows",	DIK_LWIN,	L"R Windows",
	DIK_APPS,	L"AppMenu",

#if	0
// トグル系キーなので使えない
	DIK_CAPITAL,	"Caps Lock",
	DIK_NUMLOCK,	"NumLock",
	DIK_SCROLL,	"ScrollLock",
	DIK_KANA,	"カナ",	
	DIK_KANJI,	"漢字",
#endif
	0x00,		NULL
};

LPTSTR	CDirectInput::DIKeyDirTable[] = {
	L"X+", L"X-", L"Y+", L"Y-", L"Z+", L"Z-",
	L"RX+", L"RX-", L"RY+", L"RY-", L"RZ+", L"RZ-",
	L"S0+", L"S0-", L"S1+", L"S1-",
};

LPTSTR	CDirectInput::DIKeyDirTable2[] = {
	L"P0 Up", L"P0 Down", L"P0 Left", L"P0 Right",
	L"P1 Up", L"P1 Down", L"P1 Left", L"P1 Right",
	L"P2 Up", L"P2 Down", L"P2 Left", L"P2 Right",
	L"P3 Up", L"P3 Down", L"P3 Left", L"P3 Right",
};

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDirectInput::CDirectInput()
{
	m_lpDI          = NULL;
	m_lpKeyboard    = NULL;

	m_nJoystickNum  = 0;

	ZEROMEMORY( m_lpJoystick, sizeof(m_lpJoystick) );
	ZEROMEMORY( m_Sw, sizeof(m_Sw) );

	ZEROMEMORY( m_JoyAxisMode, sizeof(m_JoyAxisMode) );

#if	COMUSE
	COM::AddRef();
#endif
}

CDirectInput::~CDirectInput()
{
	ReleaseDInput();

#if	COMUSE
	COM::AddRef();
#endif
}

//////////////////////////////////////////////////////////////////////
// メンバ関数
//////////////////////////////////////////////////////////////////////
// デバイスオブジェクト列挙コールバック
BOOL CALLBACK CDirectInput::DIEnumDevicesCallback( LPDIDEVICEINSTANCE lpddi, LPVOID pvRef )
{
	CDirectInput* pCDi = (CDirectInput*)pvRef;

//	DEBUGOUT( "dwDevType=%08X  IName:%s  PName:%s\n", lpddi->dwDevType, lpddi->tszInstanceName, lpddi->tszProductName );

	if( pCDi->AddJoystickDevice( lpddi->guidInstance ) )
		return	DIENUM_CONTINUE;

	return	DIENUM_STOP;
}

// ジョイスティックデバイスオブジェクトの作成
BOOL	CDirectInput::AddJoystickDevice( GUID deviceguid )
{
	LPDIRECTINPUTDEVICE7	lpDIDev;

	if( m_lpDI->CreateDeviceEx( deviceguid, IID_IDirectInputDevice7,
		(LPVOID*)&lpDIDev, NULL ) != DI_OK ) {
		return	FALSE;
	}

	if( lpDIDev->SetDataFormat( &c_dfDIJoystick ) != DI_OK ) {
		DEBUGOUT( "CDirectInput:SetDataFormat failed.\n" );
		RELEASE( lpDIDev );
		return	FALSE;
	}

	INT	nID = m_nJoystickNum;

	if( !Config.general.bNoJoystickID ) {
		// DX7では隠し要素のジョイスティックIDの取得(DX8からはマニュアルに記載されている)
		DIPROPDWORD	diprp_dw;
		ZEROMEMORY( &diprp_dw, sizeof(diprp_dw) );
		diprp_dw.diph.dwSize       = sizeof(DIPROPDWORD);
		diprp_dw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diprp_dw.diph.dwHow        = DIPH_DEVICE;
		diprp_dw.diph.dwObj        = 0;

		if( lpDIDev->GetProperty( DIPROP_JOYSTICKID, &diprp_dw.diph ) != DI_OK ) {
			DEBUGOUT( "CDirectInput:GetProperty failed.\n" );
			RELEASE( lpDIDev );
			return	FALSE;
		}
DEBUGOUT( "ID:%d\n", diprp_dw.dwData );

		nID = diprp_dw.dwData;
	}

	if( nID < DIJOYSTICK_MAX ) {
		m_lpJoystick[ nID ] = lpDIDev;

		// 各軸のレンジを設定
		DIPROPRANGE	diprg; 
		diprg.diph.dwSize       = sizeof(DIPROPRANGE);
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diprg.diph.dwHow        = DIPH_BYOFFSET;
		diprg.diph.dwObj        = DIJOFS_X;
		diprg.lMin              = -10000;
		diprg.lMax              = +10000;
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );
		diprg.diph.dwObj        = DIJOFS_Y;
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );
		diprg.diph.dwObj        = DIJOFS_Z;
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );
		diprg.diph.dwObj        = DIJOFS_RX;
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );
		diprg.diph.dwObj        = DIJOFS_RY;
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );
		diprg.diph.dwObj        = DIJOFS_RZ;
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );

		diprg.diph.dwObj        = DIJOFS_SLIDER(0);
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );
		diprg.diph.dwObj        = DIJOFS_SLIDER(1);
		lpDIDev->SetProperty( DIPROP_RANGE, &diprg.diph );

		// 名称の取得
		DIDEVICEINSTANCE didins;
		ZEROMEMORY( &didins, sizeof(didins) );
		didins.dwSize = sizeof( didins );
		lpDIDev->GetDeviceInfo( &didins );

		m_JoyName[ nID ] = didins.tszInstanceName;
DEBUGOUT( "Instance Name:%s\n", didins.tszInstanceName );
//DEBUGOUT( "Product  Name:%s\n", didins.tszProductName );
	} else {
		m_lpJoystick[ nID ] = NULL;
		RELEASE( lpDIDev );
	}

	m_nJoystickNum++;

	return	TRUE;
}

// DirectInputオブジェクト／デバイスオブジェクトの構築
BOOL	CDirectInput::InitialDInput(HWND hWnd, HINSTANCE hInst)
{
	try {
		// CDirectInputオブジェクトの作成
#if	!COMUSE
		if( DirectInputCreateEx( hInst, DIRECTINPUT_VERSION, IID_IDirectInput7, (LPVOID*)&m_lpDI, NULL ) != DI_OK ) {
			m_lpDI = NULL;
			throw "CDirectInput:DirectInputCreateEx failed.";
		}
#else
		// COM的利用
//		COM::AddRef();
		if( FAILED(CoCreateInstance( CLSID_DirectInput, NULL, CLSCTX_INPROC_SERVER, IID_IDirectInput7, (VOID**)&m_lpDI )) ) {
			m_lpDI = NULL;
			throw	"CDirectInput:CoCreateInstance failed.";
		}
		if( m_lpDI->Initialize( hInst, DIRECTINPUT_VERSION ) != DI_OK )
			throw	"CDirectInput:IDirectInput7->Initialize failed.";
#endif

		if( m_lpDI->CreateDevice( GUID_SysKeyboard, &m_lpKeyboard, NULL ) != DI_OK )
			throw	"CDirectInput:CreateDevice failed.";

		if( m_lpKeyboard ) {
			if( m_lpKeyboard->SetDataFormat( &c_dfDIKeyboard ) != DI_OK )
				throw	"CDirectInput:SetDataFormat failed.";
			if( m_lpKeyboard->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND) != DI_OK )
				throw	"CDirectInput:SetCooperativeLevel failed.";
			if( m_lpKeyboard->Acquire() != DI_OK ) {
//				DEBUGOUT( "CDirectInput:Acquire failed.\n" );
			}
		}

		m_nJoystickNum = 0;
		if( m_lpDI->EnumDevices( DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACK)DIEnumDevicesCallback,
					(LPVOID)this, DIEDFL_ATTACHEDONLY ) != DI_OK ) {
			DEBUGOUT( "CDirectInput:EnumDevices failed.\n" );
		}

		if( !m_nJoystickNum ) {
			DEBUGOUT( "CDirectInput:No Joystick device available.\n" );
		} else {
			for( INT i = 0; i < DIJOYSTICK_MAX; i++ ) {
				if( m_lpJoystick[i] ) {
					if( m_lpJoystick[i]->SetCooperativeLevel( hWnd, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND) != DI_OK ) {
						DEBUGOUT( "CDirectInput:SetCooperativeLevel failed.\n" );
						throw 	"CDirectInput:SetCooperativeLevel failed.";
					}
				}
			}

			DEBUGOUT( "CDirectInput:Can use %d Joystick(s)\n", m_nJoystickNum );
		}
	} catch( char *str ) {
		ReleaseDInput();

		MessageBoxA( hWnd, str, "ERROR", MB_ICONERROR|MB_OK );

		return	FALSE;
	}

	return	TRUE;
}

void	CDirectInput::ReleaseDInput()
{
	for( INT i = 0; i < DIJOYSTICK_MAX; i++ ) {
		RELEASE( m_lpJoystick[i] );
	}

	if( m_lpKeyboard ) {
//		m_lpKeyboard->Unacquire();
		RELEASE( m_lpKeyboard );
	}

	if( m_lpDI ) {
		RELEASE( m_lpDI );
#if	COMUSE
//		COM::Release();
#endif
	}
}

// 入力フォーカスを取得
void	CDirectInput::Acquire()
{
	if( !m_lpDI )
		return;
	if( m_lpKeyboard )
		m_lpKeyboard->Acquire();
	for( INT i = 0; i < DIJOYSTICK_MAX; i++ ) {
		if( m_lpJoystick[i] ) {
			m_lpJoystick[i]->Acquire();
		}
	}
}

// 入力フォーカスを開放
void	CDirectInput::Unacquire()
{
	if( !m_lpDI )
		return;
	if( m_lpKeyboard )
		m_lpKeyboard->Unacquire();
	for( INT i = 0; i < DIJOYSTICK_MAX; i++ ) {
		if( m_lpJoystick[i] ) {
			m_lpJoystick[i]->Unacquire();
		}
	}
}

// データポーリング
void	CDirectInput::Poll()
{
DIJOYSTATE	js;

	ZEROMEMORY( m_Sw, sizeof(m_Sw) );

	if( !m_lpDI ) {
		return;
	}

	if( m_lpKeyboard ) {
		if( m_lpKeyboard->GetDeviceState( 256, &m_Sw ) == DIERR_INPUTLOST ) {
			m_lpKeyboard->Acquire();
			m_lpKeyboard->GetDeviceState( 256, &m_Sw );
		}

		m_Sw[DIK_LCONTROL] =
		m_Sw[DIK_RCONTROL] = (GetAsyncKeyState( VK_CONTROL ) < 0) ? 0x80 : 0;
	}

	INT	idx;
	for( INT i = 0; i < DIJOYSTICK_MAX; i++ ) {
		if( !m_lpJoystick[i] )
			continue;

		idx = 256+i*64;

		if( m_lpJoystick[i]->Poll() == DIERR_INPUTLOST ) {
			m_lpJoystick[i]->Acquire();
			m_lpJoystick[i]->Poll();
		}
		if( m_lpJoystick[i]->GetDeviceState( sizeof(DIJOYSTATE), &js ) != DI_OK ) {
			ZEROMEMORY( &js, sizeof(DIJOYSTATE) );
		}

		m_JoyAxis[i][0] = js.lX;
		m_JoyAxis[i][1] = js.lY;
		m_JoyAxis[i][2] = js.lZ;
		m_JoyAxis[i][3] = js.lRx;
		m_JoyAxis[i][4] = js.lRy;
		m_JoyAxis[i][5] = js.lRz;

		if( !(m_JoyAxisMode[i] & (1<<0)) ) {
			if( js.lX >  8000 ) m_Sw[idx + DI_XAXIS+0] = 0x80;
			if( js.lX < -8000 ) m_Sw[idx + DI_XAXIS+1] = 0x80;
		}
		if( !(m_JoyAxisMode[i] & (1<<1)) ) {
			if( js.lY >  8000 ) m_Sw[idx + DI_YAXIS+0] = 0x80;
			if( js.lY < -8000 ) m_Sw[idx + DI_YAXIS+1] = 0x80;
		}
		if( !(m_JoyAxisMode[i] & (1<<2)) ) {
			if( js.lZ >  8000 ) m_Sw[idx + DI_ZAXIS+0] = 0x80;
			if( js.lZ < -8000 ) m_Sw[idx + DI_ZAXIS+1] = 0x80;
		}
		if( !(m_JoyAxisMode[i] & (1<<3)) ) {
			if( js.lRx >  8000 ) m_Sw[idx + DI_RXAXIS+0] = 0x80;
			if( js.lRx < -8000 ) m_Sw[idx + DI_RXAXIS+1] = 0x80;
		}
		if( !(m_JoyAxisMode[i] & (1<<4)) ) {
			if( js.lRy >  8000 ) m_Sw[idx + DI_RYAXIS+0] = 0x80;
			if( js.lRy < -8000 ) m_Sw[idx + DI_RYAXIS+1] = 0x80;
		}
		if( !(m_JoyAxisMode[i] & (1<<5)) ) {
			if( js.lRz >  8000 ) m_Sw[idx + DI_RZAXIS+0] = 0x80;
			if( js.lRz < -8000 ) m_Sw[idx + DI_RZAXIS+1] = 0x80;
		}

#if	0
// 2003/11/3 とりあえず無効化
		if( js.rglSlider[0] >  8000 ) m_Sw[idx + DI_SLIDER0+0] = 0x80;
		if( js.rglSlider[0] < -8000 ) m_Sw[idx + DI_SLIDER0+1] = 0x80;
		if( js.rglSlider[1] >  8000 ) m_Sw[idx + DI_SLIDER1+0] = 0x80;
		if( js.rglSlider[1] < -8000 ) m_Sw[idx + DI_SLIDER1+1] = 0x80;
#endif

		for( INT j = 0; j < 32; j++ ) {
			m_Sw[idx + DI_BUTTON + j] = js.rgbButtons[j];
		}

		// POV
		for( INT pov = 0; pov < 4; pov++ ) {
			DWORD	dwPOV = js.rgdwPOV[pov];
			BOOL	bPOVcenter = (LOWORD(dwPOV) == 0xFFFF);

			BYTE	data = 0;
			if( !bPOVcenter ) {
				static const BYTE dirtbl[] = {
					(1<<0), (1<<0)|(1<<3), (1<<3), (1<<1)|(1<<3),
					(1<<1), (1<<1)|(1<<2), (1<<2), (1<<0)|(1<<2),
				};

				data = dirtbl[ ((dwPOV+(DWORD)(22.5*DI_DEGREES)) % (360*DI_DEGREES))/(45*DI_DEGREES) ];
			}

			// Up/Down
			if( data & (1<<0) ) m_Sw[idx + DI_POV0_UD+i*4+0] = 0x80;
			if( data & (1<<1) ) m_Sw[idx + DI_POV0_UD+i*4+1] = 0x80;
			// Left/Right
			if( data & (1<<2) ) m_Sw[idx + DI_POV0_LR+i*4+0] = 0x80;
			if( data & (1<<3) ) m_Sw[idx + DI_POV0_LR+i*4+1] = 0x80;
		}
	}
}

LPCTSTR	CDirectInput::SearchKeyName( INT key )
{
LPDIKEYTBL kt = DIKeyTable;
static	TCHAR	KeyStr[256];

	if( key == 0x00 )
		return	NULL;

	if( key < 0x100 ) {
		while( kt->name != NULL ) {
			if( kt->key == key )
				return	kt->name;
			kt++;
		}
	} else {
		INT	no  = (key-256)>>6;
		INT	idx = key & 0x3F;
		if( idx < DI_MAXAXIS ) {
			::wsprintf( KeyStr, L"J:%d %s", no, DIKeyDirTable[idx] );
			return	KeyStr;
		} else if( idx >= DI_BUTTON && idx < DI_BUTTON_END ) {
			::wsprintf( KeyStr, L"J:%d B:%02d", no, idx-DI_BUTTON );
			return	KeyStr;
		} else if( idx >= DI_EXT && idx < DI_EXT_END ) {
			::wsprintf( KeyStr, L"J:%d %s", no, DIKeyDirTable2[idx-DI_EXT] );
			return	KeyStr;
		}
	}

	return	NULL;
}

