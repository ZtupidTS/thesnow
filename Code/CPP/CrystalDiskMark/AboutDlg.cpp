/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskMark.h"
#include "AboutDlg.h"

#define URL_JAPANESE	_T("http://crystalmark.info/")
#define URL_ENGLISH		_T("http://crystalmark.info/?lang=en")

#define CLIENT_SIZE_X		240
#define CLIENT_SIZE_Y		160


IMPLEMENT_DYNCREATE(CAboutDlg, CDHtmlDialog)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CAboutDlg::IDD, CAboutDlg::IDH, pParent)
{

}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	m_hAccelerator = ::LoadAccelerators(AfxGetInstanceHandle(),
		                                MAKEINTRESOURCE(IDR_ACCELERATOR));

	m_FlagShowWindow = FALSE;

//////////////////////////
// Enabled Visual Style //
//////////////////////////
	DOCHOSTUIINFO info;
	info.cbSize = sizeof(info);
	GetHostInfo(&info);
	SetHostFlags(info.dwFlags | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_THEME);

///////////////////
// ReSize Dialog //
///////////////////
	MoveWindow(0, 0,
		CLIENT_SIZE_X + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		CLIENT_SIZE_Y + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYMENU)
		);
	CenterWindow();

	return TRUE;
}

BOOL CAboutDlg::PreTranslateMessage(MSG* pMsg) 
{
	if( 0 != ::TranslateAccelerator(m_hWnd, m_hAccelerator, pMsg) )
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDHtmlDialog)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CAboutDlg)
	DHTML_EVENT_ONCLICK(_T("CrystalDewWorld"), OnCrystalDewWorld)
END_DHTML_EVENT_MAP()

HRESULT CAboutDlg::OnCrystalDewWorld(IHTMLElement* /*pElement*/)
{
	if(GetUserDefaultLCID() == 0x0411){// Japanese
		ShellExecute(NULL, NULL, URL_JAPANESE, NULL, NULL, SW_SHOWNORMAL);
	}else{// Other Language
		ShellExecute(NULL, NULL, URL_ENGLISH, NULL, NULL, SW_SHOWNORMAL);
	}
	return S_FALSE;
}

void CAboutDlg::OnWindowPosChanging(WINDOWPOS * lpwndpos)
{
	if(! m_FlagShowWindow){
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}
    CDialog::OnWindowPosChanging(lpwndpos);
}

void CAboutDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CComBSTR bstr;
	bstr = CRYSTAL_DISK_MARK_EDITION;
	SetElementHtml(_T("Edition"), bstr);
	m_FlagShowWindow = TRUE;
	ShowWindow(SW_SHOW);
}

void CAboutDlg::OnOK()
{

}