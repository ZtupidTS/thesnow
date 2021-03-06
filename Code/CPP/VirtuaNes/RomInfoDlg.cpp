//
// ROM情報ダイアログクラス
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
using namespace std;

#include "VirtuaNESres.h"
#include "DebugOut.h"
#include "App.h"
#include "Pathlib.h"

#include "Wnd.h"
#include "RomInfoDlg.h"

DLG_MESSAGE_BEGIN(CRomInfoDlg)
DLG_ON_MESSAGE( WM_INITDIALOG,	OnInitDialog )

DLG_COMMAND_BEGIN()
DLG_ON_COMMAND( IDOK, OnOK )
DLG_ON_COMMAND( IDCANCEL, OnCancel )
DLG_COMMAND_END()
DLG_MESSAGE_END()

INT	CRomInfoDlg::DoModal( HWND hWndParent )
{
	return	::DialogBoxParam( CApp::GetPlugin(), MAKEINTRESOURCE(IDD_ROMINFO),
				hWndParent, g_DlgProc, (LPARAM)this );
}

DLGMSG	CRomInfoDlg::OnInitDialog( DLGMSGPARAM )
{
//	DEBUGOUT( "CRomInfoDlg::OnInitDialog\n" );

	::SetDlgItemText( m_hWnd, IDC_ROM_NAME, m_szName );

	TCHAR	szStr[64];
	if( m_nMapper < 256 ) ::wsprintf( szStr, L"%d", m_nMapper );
	else		      ::wsprintf( szStr, L"NSF" );

	::SetDlgItemText( m_hWnd, IDC_ROM_MAPPER, szStr );

	::wsprintf( szStr, L"%dKB", m_nPRG*16 );
	::SetDlgItemText( m_hWnd, IDC_ROM_PRG, szStr );
	::wsprintf( szStr, L"%dKB", m_nCHR*8 );
	::SetDlgItemText( m_hWnd, IDC_ROM_CHR, szStr );

	::SetDlgItemText( m_hWnd, IDC_ROM_MIRROR,  m_bMirror?L"V":L"H" );
	::SetDlgItemText( m_hWnd, IDC_ROM_SRAM,    m_bSram?L"Yes":L"No" );
	::SetDlgItemText( m_hWnd, IDC_ROM_4SCREEN, m_b4Screen?L"Yes":L"No" );
	::SetDlgItemText( m_hWnd, IDC_ROM_TRAINER, m_bTrainer?L"Yes":L"No" );
	::SetDlgItemText( m_hWnd, IDC_ROM_VSUNISYSTEM, m_bVSUnisystem?L"Yes":L"No" );

	::wsprintf( szStr, L"%08X", m_dwCRC );
	::SetDlgItemText( m_hWnd, IDC_ROM_CRC, szStr );
	::wsprintf( szStr, L"%08X", m_dwCRCALL );
	::SetDlgItemText( m_hWnd, IDC_ROM_CRCALL, szStr );
	::wsprintf( szStr, L"%08X", m_dwCRCCHR );
	::SetDlgItemText( m_hWnd, IDC_ROM_CRCCHR, szStr );

	return	TRUE;
}

DLGCMD	CRomInfoDlg::OnOK( DLGCMDPARAM )
{
//	DEBUGOUT( "CRomInfoDlg::OnOK\n" );

	::EndDialog( m_hWnd, IDOK );
}

DLGCMD	CRomInfoDlg::OnCancel( DLGCMDPARAM )
{
//	DEBUGOUT( "CRomInfoDlg::OnCancel\n" );

	::EndDialog( m_hWnd, IDCANCEL );
}

