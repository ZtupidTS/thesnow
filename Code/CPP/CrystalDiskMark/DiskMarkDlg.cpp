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
#include "DiskMarkDlg.h"
#include "DiskBench.h"
#include "AboutDlg.h"
#include "GetFileVersion.h"
#include "GetOsInfo.h"

#include <math.h>
#include <exdispid.h>

#define MAX_METER_LENGTH	150
//#define WM_THEME_ID			(WM_USER+0x100)
//#define WM_LANGUAGE_ID		(WM_USER+0x200)
#define MAIN_CSS_FILE_NAME	_T("Main.css")

extern int DISK_TEST_TIME;

#define SIZE_X		400
#define SIZE_Y		320

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_DHTML_EVENT_MAP(CDiskMarkDlg)
	DHTML_EVENT_ONCLICK(_T("All"), OnAll)
	DHTML_EVENT_ONCLICK(_T("TestDrive"), OnSelectDrive)
	DHTML_EVENT_ONCLICK(_T("Sequential"), OnSequential)
	DHTML_EVENT_ONCLICK(_T("Random512KB"), OnRandom512KB)
	DHTML_EVENT_ONCLICK(_T("Random4KB"), OnRandom4KB)
	DHTML_EVENT_ONCLICK(_T("Random4KB32QD"), OnRandom4KB32QD)
END_DHTML_EVENT_MAP()

CDiskMarkDlg::CDiskMarkDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlMainDialog(CDiskMarkDlg::IDD, CDiskMarkDlg::IDH,
	((CDiskMarkApp*)AfxGetApp())->m_ThemeDir,
	((CDiskMarkApp*)AfxGetApp())->m_ThemeIndex,
	((CDiskMarkApp*)AfxGetApp())->m_LangDir,
	((CDiskMarkApp*)AfxGetApp())->m_LangIndex,
	pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIconMini = AfxGetApp()->LoadIcon(IDI_TRAY_ICON);

	m_AboutDlg = NULL;

	// Init m_ini 
	m_ExeDir = ((CDiskMarkApp*)AfxGetApp())->m_ExeDir;
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskMarkApp*)AfxGetApp())->m_Ini);
}

void CDiskMarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_DHtml_SelectValue(pDX, _T("TestDrive"), m_ValueTestDrive);
	DDX_DHtml_SelectIndex(pDX, _T("TestDrive"), m_IndexTestDrive);
	DDX_DHtml_SelectValue(pDX, _T("TestNumber"), m_ValueTestNumber);
	DDX_DHtml_SelectIndex(pDX, _T("TestNumber"), m_IndexTestNumber);
	DDX_DHtml_SelectValue(pDX, _T("TestSize"), m_ValueTestSize);
	DDX_DHtml_SelectIndex(pDX, _T("TestSize"), m_IndexTestSize);

	DDX_DHtml_ElementText(pDX, _T("Comment"), DISPID_IHTMLINPUTELEMENT_VALUE, m_Comment);
}

BEGIN_MESSAGE_MAP(CDiskMarkDlg, CDHtmlMainDialog)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_EXIT, OnExit)
	ON_COMMAND(ID_HELP_ABOUT, OnAbout)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_MESSAGE(WM_USER_UPDATE_SCORE, OnUpdateScore)
	ON_MESSAGE(WM_USER_UPDATE_MESSAGE, OnUpdateMessage)
	ON_MESSAGE(WM_USER_EXIT_BENCHMARK, OnExitBenchmark)
	ON_COMMAND(ID_ZOOM_100, &CDiskMarkDlg::OnZoom100)
	ON_COMMAND(ID_ZOOM_125, &CDiskMarkDlg::OnZoom125)
	ON_COMMAND(ID_ZOOM_150, &CDiskMarkDlg::OnZoom150)
	ON_COMMAND(ID_ZOOM_200, &CDiskMarkDlg::OnZoom200)
	ON_COMMAND(ID_ZOOM_300, &CDiskMarkDlg::OnZoom300)
	ON_COMMAND(ID_ZOOM_400, &CDiskMarkDlg::OnZoom400)
	ON_COMMAND(ID_ZOOM_AUTO, &CDiskMarkDlg::OnZoomAuto)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, &CDiskMarkDlg::OnHelp)
	ON_COMMAND(ID_HELP_CRYSTALDEWWORLD, &CDiskMarkDlg::OnCrystalDewWorld)
	ON_COMMAND(ID_MODE_DEFAULT, &CDiskMarkDlg::OnModeDefault)
	ON_COMMAND(ID_MODE_ALL0X00, &CDiskMarkDlg::OnModeAll0x00)
	ON_COMMAND(ID_MODE_ALL0XFF, &CDiskMarkDlg::OnModeAll0xFF)
END_MESSAGE_MAP()

BOOL CDiskMarkDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	m_hAccelerator = ::LoadAccelerators(AfxGetInstanceHandle(),
		                                MAKEINTRESOURCE(IDR_ACCELERATOR));

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIconMini, FALSE);

	m_IndexTestDrive = 0;	// default value may be "C:\".
	m_IndexTestNumber = 4;	// default value is 5.
	m_IndexTestSize = 3;	// default value is 1000MB;

	m_WinThread = NULL;
	m_DiskBenchStatus = FALSE;

	InitThemeLang();
	InitMenu();

	switch(GetPrivateProfileInt(_T("Setting"), _T("ZoomType"), 0, m_Ini))
	{
	case 100:  CheckRadioZoomType(ID_ZOOM_100, 100); break;
	case 125:  CheckRadioZoomType(ID_ZOOM_125, 125); break;
	case 150:  CheckRadioZoomType(ID_ZOOM_150, 150); break;
	case 200:  CheckRadioZoomType(ID_ZOOM_200, 200); break;
	case 300:  CheckRadioZoomType(ID_ZOOM_300, 300); break;
	case 400:  CheckRadioZoomType(ID_ZOOM_400, 400); break;
	default:   CheckRadioZoomType(ID_ZOOM_AUTO, 0); break;
	}

	m_TestData = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("TestData"), TEST_DATA_DEFAULT, m_Ini);

/*
	DISK_TEST_TIME = GetPrivateProfileInt(_T("Setting"), _T("TestTime"), 0, m_Ini) * 1000;
	if(DISK_TEST_TIME <= 0)
	{
		DISK_TEST_TIME = 6 * 1000;
	}
	CString cstr;
	cstr.Format(_T("%d"), DISK_TEST_TIME / 1000);
	WritePrivateProfileString(_T("Setting"), _T("TestTime"), cstr, m_Ini);
*/
	m_SizeX = SIZE_X;
	m_SizeY = SIZE_Y;

	if(m_TestData == TEST_DATA_ALL0XFF)
	{
		SetWindowTitle(_T(""), ALL_0XFF_1FILL);
	}
	else if(m_TestData == TEST_DATA_ALL0X00)
	{
		SetWindowTitle(_T(""), ALL_0X00_0FILL);
	}
	else
	{
		SetWindowTitle(_T(""), _T(""));
	}
	
	EnableDpiAware();
	InitDHtmlDialog(m_SizeX, m_SizeY, ((CDiskMarkApp*)AfxGetApp())->m_MainDlgPath);

	return TRUE;
}

BOOL CDiskMarkDlg::PreTranslateMessage(MSG* pMsg) 
{
	if( 0 != ::TranslateAccelerator(m_hWnd, m_hAccelerator, pMsg) )
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

HRESULT CDiskMarkDlg::OnSelectDrive(IHTMLElement *pElement)
{
	UpdateData(TRUE);
	return TRUE;
}


LRESULT CDiskMarkDlg::OnUpdateScore(WPARAM wParam, LPARAM lParam)
{
	UpdateScore();
	return 0;
}

LRESULT CDiskMarkDlg::OnExitBenchmark(WPARAM wParam, LPARAM lParam)
{
	ChangeButtonStatus(TRUE);
	EnableMenus();

	return 0;
}

void CDiskMarkDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlMainDialog::OnPaint();
	}
}

HCURSOR CDiskMarkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDiskMarkDlg::OnOK()
{

}

void CDiskMarkDlg::OnExit()
{
	OnCancel();
}

void CDiskMarkDlg::OnAbout()
{
	m_AboutDlg = new CAboutDlg(this);
	m_AboutDlg->Create(CAboutDlg::IDD, m_AboutDlg, ID_HELP_ABOUT, this);
}

void CDiskMarkDlg::OnCancel()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	//	Stop();
	}
	CDialog::OnCancel();
}

void CDiskMarkDlg::InitScore()
{
	m_SequentialReadScore = 0.0;
	m_SequentialWriteScore = 0.0;
	m_RandomRead512KBScore = 0.0;
	m_RandomWrite512KBScore = 0.0;
	m_RandomRead4KBScore = 0.0;
	m_RandomWrite4KBScore = 0.0;
	m_RandomRead4KB32QDScore = 0.0;
	m_RandomWrite4KB32QDScore = 0.0;

	UpdateScore();
}

void CDiskMarkDlg::UpdateScore()
{
	UpdateData(TRUE); // Hold Comment
	SetMeter(_T("SequentialRead"), m_SequentialReadScore);
	SetMeter(_T("SequentialWrite"), m_SequentialWriteScore);
	SetMeter(_T("RandomRead512KB"), m_RandomRead512KBScore);
	SetMeter(_T("RandomWrite512KB"), m_RandomWrite512KBScore);
	SetMeter(_T("RandomRead4KB"), m_RandomRead4KBScore);
	SetMeter(_T("RandomWrite4KB"), m_RandomWrite4KBScore);
	SetMeter(_T("RandomRead4KB32QD"), m_RandomRead4KB32QDScore);
	SetMeter(_T("RandomWrite4KB32QD"), m_RandomWrite4KB32QDScore);

	// Set IOPS value as title
	CString cstr;
	cstr.Format(_T("%8.1f IOPS"), m_RandomRead4KBScore * 1000 * 1000 / 4096);
	SetElementPropertyEx(_T("RandomRead4KB"), DISPID_IHTMLELEMENT_TITLE, cstr);
	cstr.Format(_T("%8.1f IOPS"), m_RandomWrite4KBScore * 1000 * 1000 / 4096);
	SetElementPropertyEx(_T("RandomWrite4KB"), DISPID_IHTMLELEMENT_TITLE, cstr);
	cstr.Format(_T("%8.1f IOPS"), m_RandomRead4KB32QDScore * 1000 * 1000 / 4096);
	SetElementPropertyEx(_T("RandomRead4KB32QD"), DISPID_IHTMLELEMENT_TITLE, cstr);
	cstr.Format(_T("%8.1f IOPS"), m_RandomWrite4KB32QDScore * 1000 * 1000 / 4096);
	SetElementPropertyEx(_T("RandomWrite4KB32QD"), DISPID_IHTMLELEMENT_TITLE, cstr);
}

HRESULT CDiskMarkDlg::OnSequential(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL)
	{
		UpdateData(TRUE);
		m_SequentialReadScore = 0.0;
		m_SequentialWriteScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchSequential, (void*)this);
		if(m_WinThread == NULL)
		{
			m_DiskBenchStatus = FALSE;
		}
		else
		{
			ChangeButtonStatus(FALSE);
		}
		DisableMenus();
	}
	else
	{
		Stop();
	}
	return S_FALSE;
}

HRESULT CDiskMarkDlg::OnRandom512KB(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL)
	{
		UpdateData(TRUE);
		m_RandomRead512KBScore = 0.0;
		m_RandomWrite512KBScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchRandom512KB, (void*)this);
		if(m_WinThread == NULL)
		{
			m_DiskBenchStatus = FALSE;
		}
		else
		{
			ChangeButtonStatus(FALSE);
		}
		DisableMenus();
	}
	else
	{
		Stop();
	}
	return S_FALSE;
}

HRESULT CDiskMarkDlg::OnRandom4KB(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL)
	{
		UpdateData(TRUE);
		m_RandomRead4KBScore = 0.0;
		m_RandomWrite4KBScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchRandom4KB, (void*)this);
		if(m_WinThread == NULL)
		{
			m_DiskBenchStatus = FALSE;
		}
		else
		{
			ChangeButtonStatus(FALSE);
		}
		DisableMenus();
	}
	else
	{
		Stop();
	}
	return S_FALSE;
}

HRESULT CDiskMarkDlg::OnRandom4KB32QD(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL)
	{
		UpdateData(TRUE);
		m_RandomRead4KB32QDScore = 0.0;
		m_RandomWrite4KB32QDScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchRandom4KB32QD, (void*)this);
		if(m_WinThread == NULL)
		{
			m_DiskBenchStatus = FALSE;
		}
		else
		{
			ChangeButtonStatus(FALSE);
		}
		DisableMenus();
	}
	else
	{
		Stop();
	}
	return S_FALSE;
}


HRESULT CDiskMarkDlg::OnAll(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL)
	{
		UpdateData(TRUE);
		InitScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchAll, (void*)this);
		if(m_WinThread == NULL)
		{
			m_DiskBenchStatus = FALSE;
		}
		else
		{
			ChangeButtonStatus(FALSE);
		}
		DisableMenus();
	}
	else
	{
		Stop();
	}
	return S_FALSE;
}

void CDiskMarkDlg::Stop()
{
	if(m_DiskBenchStatus)
	{
		m_DiskBenchStatus = FALSE;
		UpdateMessage(_T("Message"), _T("Stopping..."));
	}
	EnableMenus();
}

void CDiskMarkDlg::EnableMenus()
{
	CMenu *menu = GetMenu();
	menu->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
	menu->EnableMenuItem(1, MF_BYPOSITION | MF_ENABLED);
	menu->EnableMenuItem(2, MF_BYPOSITION | MF_ENABLED);
	menu->EnableMenuItem(3, MF_BYPOSITION | MF_ENABLED);
	menu->EnableMenuItem(4, MF_BYPOSITION | MF_ENABLED);
	SetMenu(menu);
}

void CDiskMarkDlg::DisableMenus()
{
	CMenu *menu = GetMenu();
	menu->EnableMenuItem(0, MF_BYPOSITION | MF_GRAYED);
	menu->EnableMenuItem(1, MF_BYPOSITION | MF_GRAYED);
	menu->EnableMenuItem(2, MF_BYPOSITION | MF_GRAYED);
	menu->EnableMenuItem(3, MF_BYPOSITION | MF_GRAYED);
	menu->EnableMenuItem(4, MF_BYPOSITION | MF_GRAYED);
	SetMenu(menu);
}

void CDiskMarkDlg::ChangeButtonStatus(BOOL status)
{
	if(status)
	{
		ChangeButton(_T("All"),				_T("button1"),	_T("All"),						_T("All"));
		ChangeButton(_T("Sequential"),		_T("button1"),	_T("Sequential"),				_T("Seq"));
		ChangeButton(_T("Random512KB"),		_T("button1"),	_T("Random 512KB"),				_T("512K"));
		ChangeButton(_T("Random4KB"),		_T("button1"),	_T("Random 4KB, Queue Depth=1"),_T("4K"));
		ChangeButton(_T("Random4KB32QD"),	_T("button2"),	_T("Random 4KB, Queue Depth=32"),_T("4K<br>QD32"));
		ChangeSelectStatus(_T("TestDrive"),	VARIANT_FALSE);
		ChangeSelectStatus(_T("TestNumber"),VARIANT_FALSE);
		ChangeSelectStatus(_T("TestSize"),	VARIANT_FALSE);
	}
	else
	{
		ChangeButton(_T("All"),				_T("button1"),	_T("Stop"),	_T("Stop"));
		ChangeButton(_T("Sequential"),		_T("button1"),	_T("Stop"),	_T("Stop"));
		ChangeButton(_T("Random512KB"),		_T("button1"),	_T("Stop"),	_T("Stop"));
		ChangeButton(_T("Random4KB"),		_T("button1"),	_T("Stop"),	_T("Stop"));
		ChangeButton(_T("Random4KB32QD"),	_T("button1"),	_T("Stop"),	_T("Stop"));
		ChangeSelectStatus(_T("TestDrive"),	VARIANT_TRUE);
		ChangeSelectStatus(_T("TestNumber"),VARIANT_TRUE);
		ChangeSelectStatus(_T("TestSize"),	VARIANT_TRUE);
	}
}

void CDiskMarkDlg::ChangeButton(CString elementName, CString className, CString title, CString innerHtml)
{
	SetElementPropertyEx(elementName, DISPID_IHTMLELEMENT_CLASSNAME, className);
	SetElementPropertyEx(elementName, DISPID_IHTMLELEMENT_TITLE, title);
	SetElementInnerHtmlEx(elementName, innerHtml);
}

void CDiskMarkDlg::ChangeSelectStatus(CString ElementName, VARIANT_BOOL status)
{
	CComPtr<IHTMLSelectElement> pHtmlSelectElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	
	hr = GetElementInterface(ElementName, IID_IHTMLSelectElement, (void **) &pHtmlSelectElement);
	if(FAILED(hr)) return ;

	hr = pHtmlSelectElement->put_disabled(status);
	if(FAILED(hr)) return ;
}

void CDiskMarkDlg::ChangeSelectTitle(CString ElementName, CString title)
{
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	
	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if(FAILED(hr)) return ;
	bstr = title;

	hr = pHtmlElement->put_title(bstr);
	if(FAILED(hr)) return ;
}

LRESULT CDiskMarkDlg::OnUpdateMessage(WPARAM wParam, LPARAM lParam)
{
	CString wstr = _T("");
	CString lstr = _T("");

	if(wParam != NULL)
	{
		wstr = *((CString*)wParam);
	}

	if(lParam != NULL)
	{
		lstr = *((CString*)lParam);
	}

	SetWindowTitle(wstr, lstr);
	return 0;
}

void CDiskMarkDlg::UpdateMessage(CString ElementName, CString message)
{
	CComBSTR bstr;
	bstr = _T("&nbsp;") + message;
	SetElementHtml(ElementName, bstr);
}

void CDiskMarkDlg::SetMeter(CString ElementName, double Score)
{
	CComPtr<IHTMLStyle> pHtmlStyle;
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	VARIANT va;
	VariantInit(&va);

	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if(FAILED(hr)) return ;

	hr = pHtmlElement->get_style(&pHtmlStyle);
	if(FAILED(hr)) return ;

	int meterLength;
	meterLength = (int)(MAX_METER_LENGTH / 5 * log10(Score * 10));
	if(meterLength > MAX_METER_LENGTH)
	{
		meterLength = MAX_METER_LENGTH;
	}
	else if(meterLength < 1)
	{
		meterLength = 0;
	}

	cstr.Format(_T("%dpx"), -1 * (MAX_METER_LENGTH - meterLength));
	bstr = cstr;
	va.vt = VT_BSTR;
	va.bstrVal = bstr;
	pHtmlStyle->put_backgroundPositionX(va);

	if(Score > 100000.0)
	{
		cstr.Format(_T("99999&nbsp;"));
	}
	else if(Score >= 10000.0)
	{
		cstr.Format(_T("%5d&nbsp;"), int(Score));
	}
	else if(Score >= 1000.0)
	{
		cstr.Format(_T("&nbsp;%4d&nbsp;"), int(Score));
	}
	else if(Score < 10.0)
	{
		cstr.Format(_T("&nbsp;%.3f&nbsp;"), Score);
	}
	else if(Score < 100.0)
	{
		cstr.Format(_T("&nbsp;%.2f&nbsp;"), Score);
	}
	else
	{
		cstr.Format(_T("&nbsp;%.1f&nbsp;"), Score);
	}
	bstr = cstr;
	pHtmlElement->put_innerHTML(bstr);

	UpdateData(FALSE);

	VariantClear(&va);

	if(Score > 0.0)
	{
		SetElementPropertyEx(ElementName, DISPID_IHTMLELEMENT_CLASSNAME, _T("meter1"));
	}
	else
	{
		SetElementPropertyEx(ElementName, DISPID_IHTMLELEMENT_CLASSNAME, _T("meter0"));
	}
}

void CDiskMarkDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	static BOOL once = FALSE;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		if(! once)
		{
			ChangeTheme(m_CurrentTheme);
			UpdateMessage(_T("Message"), _T(""));
			InitDrive(_T("TestDrive"));
			InitScore();
			ChangeLang(m_CurrentLang);
			ChangeZoomType(m_ZoomType);
			UpdateData(TRUE);
			m_FlagShowWindow = TRUE;
			SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio));
			CenterWindow();
			ShowWindow(SW_SHOW);
			once = TRUE;
		}
	}
}

void CDiskMarkDlg::InitDrive(CString ElementName)
{
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	CString select;

	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if(FAILED(hr)) return ;

	// list up drive
	TCHAR szDrives[256] = {0};
	LPTSTR pDrive = szDrives;
	TCHAR rootPath[4] = {0};
	TCHAR fileSystem[32] = {0};
	GetLogicalDriveStrings(255, szDrives);

	select = _T("<select name=\"TestDrive\" id=\"TestDrive\" title=\"Test Drive\" onChange=\"this.click()\">\n");
	while( pDrive[0] != _T('\0') )
	{
		ULARGE_INTEGER freeBytesAvailableToCaller = {0};
		ULARGE_INTEGER totalNumberOfBytes = {0};
		ULARGE_INTEGER totalNumberOfFreeBytes = {0};

	//	_tcsupr_s(pDrive, sizeof(TCHAR) * 4);
		int result = GetDriveType(pDrive);
		if((result == DRIVE_REMOVABLE) && (pDrive[0] == 'A' || pDrive[0] == 'B'))
		{
			pDrive += _tcslen(pDrive) + 1;
			continue;
		}
		int forward = (int)_tcslen( pDrive );

		if(result == DRIVE_FIXED || result == DRIVE_REMOTE || result == DRIVE_REMOVABLE || result == DRIVE_RAMDISK)
		{
			pDrive[1] = _T('\0');
			cstr.Format(_T("%C: "), pDrive[0]);
			if(GetDiskFreeSpaceEx(cstr, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes) != 0)
			{
				cstr = _T("<option value=\"");
				cstr += pDrive;
				cstr += _T("\">");
				select += cstr;

				if(totalNumberOfBytes.QuadPart < ((ULONGLONG)8 * 1024 * 1024 * 1024)) // < 8 GB
				{
					cstr.Format(_T("%s: %.0f%% (%.0f/%.0fMB)\r\n"), pDrive,
							(double)(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / (double)totalNumberOfBytes.QuadPart * 100,
							(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / 1024 / 1024.0,
							totalNumberOfBytes.QuadPart  / 1024 / 1024.0);
				}
				else
				{
					cstr.Format(_T("%s: %.0f%% (%.0f/%.0fGB)\r\n"), pDrive,
							(double)(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / (double)totalNumberOfBytes.QuadPart * 100,
							(totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart) / 1024 / 1024 / 1024.0,
							totalNumberOfBytes.QuadPart  / 1024 / 1024 / 1024.0);
				}
				select += cstr;
				select += _T("</option>\n");
			}
		}
		pDrive += forward + 1;
	}

	select += _T("</select>");

	bstr = select;
	pHtmlElement->put_outerHTML(bstr);
	UpdateData(FALSE);
}

void CDiskMarkDlg::ChangeLang(CString LangName)
{
	m_CurrentLangPath.Format(_T("%s\\%s.lang"), m_LangDir, LangName);

	CString cstr;
	CMenu *menu = GetMenu();
	CMenu subMenu;

	cstr = i18n(_T("Menu"), _T("FILE"));
	menu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, cstr);
	cstr = i18n(_T("Menu"), _T("EDIT"));
	menu->ModifyMenu(1, MF_BYPOSITION | MF_STRING, 1, cstr);
	cstr = i18n(_T("Menu"), _T("THEME"));
	menu->ModifyMenu(2, MF_BYPOSITION | MF_STRING, 2, cstr);
	cstr = i18n(_T("Menu"), _T("HELP"));
	menu->ModifyMenu(3, MF_BYPOSITION | MF_STRING, 3, cstr);
	cstr = i18n(_T("Menu"), _T("LANGUAGE"));
	if(cstr.Find(_T("Language")) >= 0)
	{
		cstr = _T("&Language");
		menu->ModifyMenu(4, MF_BYPOSITION | MF_STRING, 4, cstr);
	}
	else
	{
		menu->ModifyMenu(4, MF_BYPOSITION | MF_STRING, 4, cstr + _T("(&Language)"));
	}

	cstr = i18n(_T("Menu"), _T("FILE_EXIT"));
	menu->ModifyMenu(ID_FILE_EXIT, MF_STRING, ID_FILE_EXIT, cstr);
	cstr = i18n(_T("Menu"), _T("EDIT_COPY"));
	menu->ModifyMenu(ID_EDIT_COPY, MF_STRING, ID_EDIT_COPY, cstr);
	cstr = i18n(_T("Menu"), _T("HELP"));
	menu->ModifyMenu(ID_HELP, MF_STRING, ID_HELP, cstr);
	cstr = i18n(_T("Menu"), _T("HELP_ABOUT"));
	menu->ModifyMenu(ID_HELP_ABOUT, MF_STRING, ID_HELP_ABOUT, cstr);

	subMenu.Attach(menu->GetSubMenu(0)->GetSafeHmenu());
	cstr = i18n(_T("Menu"), _T("TEST_DATA"));
	subMenu.ModifyMenu(0, MF_BYPOSITION, 0, cstr);
	subMenu.Detach();

	cstr = i18n(_T("Menu"), _T("DEFAULT_RANDOM"));
	menu->ModifyMenu(ID_MODE_DEFAULT, MF_STRING, ID_MODE_DEFAULT, cstr);

	// Theme
	subMenu.Attach(menu->GetSubMenu(2)->GetSafeHmenu());
	cstr = i18n(_T("Menu"), _T("ZOOM"));
	if(GetIeVersion() < 800)
	{
		cstr += _T(" [IE8-]");
		subMenu.ModifyMenu(0, MF_BYPOSITION, 0, cstr);
		subMenu.EnableMenuItem(0, MF_BYPOSITION|MF_GRAYED);   
	}
	else
	{
		subMenu.ModifyMenu(0, MF_BYPOSITION, 0, cstr);
	}
	subMenu.Detach();

	cstr = i18n(_T("Menu"), _T("AUTO"));
	menu->ModifyMenu(ID_ZOOM_AUTO, MF_STRING, ID_ZOOM_AUTO, cstr);

	CheckRadioZoomType();

	if(m_TestData == TEST_DATA_ALL0XFF)
	{
		OnModeAll0xFF();
	}
	else if(m_TestData == TEST_DATA_ALL0X00)
	{
		OnModeAll0x00();
	}
	else
	{
		OnModeDefault();
	}

	SetMenu(menu);

	m_MesStopBenchmark = i18n(_T("Message"), _T("STOP_BENCHMARK"));
	m_MesDiskCapacityError = i18n(_T("Message"), _T("DISK_CAPACITY_ERROR"));
	m_MesDiskCreateFileError = i18n(_T("Message"), _T("DISK_CREATE_FILE_ERROR"));
	m_MesDiskWriteError = i18n(_T("Message"), _T("DISK_WRITE_ERROR"));
	m_MesDiskReadError = i18n(_T("Message"), _T("DISK_READ_ERROR"));

	m_TitleTestDrive = i18n(_T("Title"), _T("TEST_DRIVE"));
	m_TitleTestSize = i18n(_T("Title"), _T("TEST_SIZE"));
	m_TitleTestNumber = i18n(_T("Title"), _T("TEST_NUMBER"));

	ChangeSelectTitle(_T("TestDrive"),	m_TitleTestDrive);
	ChangeSelectTitle(_T("TestNumber"),	m_TitleTestNumber);
	ChangeSelectTitle(_T("TestSize"),	m_TitleTestSize);

	WritePrivateProfileString(_T("Setting"), _T("Language"), LangName, m_ini);

	SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio), 1);
}

BOOL CDiskMarkDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// Select Language
	if(WM_LANGUAGE_ID <= wParam && wParam < WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize())
	{
#ifdef _UNICODE
		CMenu menu;
		CMenu subMenu;
		CMenu subMenuAN;
		CMenu subMenuOZ;
		menu.Attach(GetMenu()->GetSafeHmenu());
		subMenu.Attach(menu.GetSubMenu(4)->GetSafeHmenu()); // 5th is "Language".
		subMenuAN.Attach(subMenu.GetSubMenu(0)->GetSafeHmenu());
		subMenuOZ.Attach(subMenu.GetSubMenu(1)->GetSafeHmenu());

		m_CurrentLang = m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID);
		ChangeLang(m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID));
		subMenuAN.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);
		subMenuOZ.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);

		subMenuOZ.Detach();
		subMenuAN.Detach();
		subMenu.Detach();
		menu.Detach();
#else
		CMenu menu;
		CMenu subMenu;
		menu.Attach(GetMenu()->GetSafeHmenu());
		subMenu.Attach(menu.GetSubMenu(4)->GetSafeHmenu()); // 5th is "Language".

		m_CurrentLang = m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID);
		ChangeLang(m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID));
		subMenu.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);
		subMenu.Detach();
		menu.Detach();
#endif
	}

	return CDHtmlMainDialog::OnCommand(wParam, lParam);
}

void CDiskMarkDlg::OnEditCopy()
{
	CString cstr, clip;

	UpdateData(TRUE);

	clip = _T("\
-----------------------------------------------------------------------\r\n\
%PRODUCT% %VERSION% (C) %COPY_YEAR% hiyohiyo\r\n\
                           Crystal Dew World : http://crystalmark.info/\r\n\
-----------------------------------------------------------------------\r\n\
* MB/s = 1,000,000 byte/s [SATA/300 = 300,000,000 byte/s]\r\n\
\r\n\
           Sequential Read : %SequentialRead%\r\n\
          Sequential Write : %SequentialWrite%\r\n\
         Random Read 512KB : %RandomRead512KB%\r\n\
        Random Write 512KB : %RandomWrite512KB%\r\n\
    Random Read 4KB (QD=1) : %RandomRead4KB%\r\n\
   Random Write 4KB (QD=1) : %RandomWrite4KB%\r\n\
   Random Read 4KB (QD=32) : %RandomRead4KB32QD%\r\n\
  Random Write 4KB (QD=32) : %RandomWrite4KB32QD%\r\n\
\r\n\
  Test : %TestSize% (x%TestNumber%)%TestMode%\r\n\
  Date : %Date%\r\n\
    OS : %OS%\r\n\
  %Comment%\
");

	clip.Replace(_T("%PRODUCT%"), PRODUCT_NAME);
	clip.Replace(_T("%VERSION%"), PRODUCT_VERSION);
	clip.Replace(_T("%COPY_YEAR%"), PRODUCT_COPY_YEAR);

	cstr.Format(_T("%9.3f MB/s"), m_SequentialReadScore);
	clip.Replace(_T("%SequentialRead%"), cstr);
	cstr.Format(_T("%9.3f MB/s"), m_SequentialWriteScore);
	clip.Replace(_T("%SequentialWrite%"), cstr);
	cstr.Format(_T("%9.3f MB/s"), m_RandomRead512KBScore);
	clip.Replace(_T("%RandomRead512KB%"), cstr);
	cstr.Format(_T("%9.3f MB/s"), m_RandomWrite512KBScore);
	clip.Replace(_T("%RandomWrite512KB%"), cstr);

	cstr.Format(_T("%9.3f MB/s [%8.1f IOPS]"), m_RandomRead4KBScore, m_RandomRead4KBScore * 1000 * 1000 / 4096);
	clip.Replace(_T("%RandomRead4KB%"), cstr);
	cstr.Format(_T("%9.3f MB/s [%8.1f IOPS]"), m_RandomWrite4KBScore, m_RandomWrite4KBScore * 1000 * 1000 / 4096);
	clip.Replace(_T("%RandomWrite4KB%"), cstr);

	cstr.Format(_T("%9.3f MB/s [%8.1f IOPS]"), m_RandomRead4KB32QDScore, m_RandomRead4KB32QDScore * 1000 * 1000 / 4096);
	clip.Replace(_T("%RandomRead4KB32QD%"), cstr);
	cstr.Format(_T("%9.3f MB/s [%8.1f IOPS]"), m_RandomWrite4KB32QDScore, m_RandomWrite4KB32QDScore * 1000 * 1000 / 4096);
	clip.Replace(_T("%RandomWrite4KB32QD%"), cstr);

	cstr.Format(_T("%d MB [%s]"), _tstoi(m_ValueTestSize), m_TestDriveInfo);
	clip.Replace(_T("%TestSize%"), cstr);
	cstr.Format(_T("%d"), _tstoi(m_ValueTestNumber));
	clip.Replace(_T("%TestNumber%"), cstr);

	if(m_Comment.IsEmpty())
	{
		clip.Replace(_T("%Comment%"), _T(""));
	}else
	{
		clip.Replace(_T("%Comment%"), _T("  ") + m_Comment + _T("\r\n"));
	}

	if(m_TestData == TEST_DATA_ALL0XFF)
	{
		clip.Replace(_T("%TestMode%"), _T(" ") ALL_0XFF_1FILL);
	}
	else if(m_TestData == TEST_DATA_ALL0X00)
	{
		clip.Replace(_T("%TestMode%"), _T(" ") ALL_0X00_0FILL);
	}
	else
	{
		clip.Replace(_T("%TestMode%"), _T(""));
	}

	GetOsName(cstr);
	clip.Replace(_T("%OS%"), cstr);

	SYSTEMTIME st;
	GetLocalTime(&st);
	cstr.Format(_T("%04d/%02d/%02d %d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	clip.Replace(_T("%Date%"), cstr);

	if(OpenClipboard())
	{
		HGLOBAL clipbuffer;
		TCHAR* buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, sizeof(TCHAR) * (clip.GetLength() + 1));
		buffer = (TCHAR*)GlobalLock(clipbuffer);
		_tcscpy_s(buffer, clip.GetLength() + 1, LPCTSTR(clip));
		GlobalUnlock(clipbuffer);
#ifdef _UNICODE
		SetClipboardData(CF_UNICODETEXT, clipbuffer);
#else
		SetClipboardData(CF_OEMTEXT, clipbuffer);
#endif
		CloseClipboard();
	}
}


void CDiskMarkDlg::OnZoom100()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_100, 100))
	{
	//	ChangeZoom();
		ReExecute();
	}
}

void CDiskMarkDlg::OnZoom125()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_125, 125))
	{
	//	ChangeZoom();
		ReExecute();
	}
}

void CDiskMarkDlg::OnZoom150()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_150, 150))
	{
	//	ChangeZoom();
		ReExecute();
	}
}

void CDiskMarkDlg::OnZoom200()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_200, 200))
	{
	//	ChangeZoom();
		ReExecute();
	}
}

void CDiskMarkDlg::OnZoom300()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_300, 300))
	{
	//	ChangeZoom();
		ReExecute();
	}
}

void CDiskMarkDlg::OnZoom400()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_400, 400))
	{
	//	ChangeZoom();
		ReExecute();
	}
}
void CDiskMarkDlg::OnZoomAuto()
{
	if(m_WinThread != NULL)
	{
		AfxMessageBox(m_MesStopBenchmark);
		return ;
	}

	if(CheckRadioZoomType(ID_ZOOM_AUTO, 0))
	{
	//	ChangeZoom();
		ReExecute();
	}
}

void CDiskMarkDlg::ChangeZoom()
{
	ChangeZoomType(m_ZoomType);
	SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio));
}

void CDiskMarkDlg::ReExecute()
{
	ShowWindow(SW_HIDE);
	EndDialog(RE_EXEC);
}

BOOL CDiskMarkDlg::CheckRadioZoomType(int id, int value)
{
	if(m_ZoomType == value)
	{
		return FALSE;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_ZOOM_100, ID_ZOOM_AUTO, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_ZoomType = value;

	CString cstr;
	cstr.Format(_T("%d"), value);
	WritePrivateProfileString(_T("Setting"), _T("ZoomType"), cstr, m_Ini);

	return TRUE;
}

void CDiskMarkDlg::CheckRadioZoomType()
{
	int id = ID_ZOOM_AUTO;

	switch(m_ZoomType)
	{
	case 100: id = ID_ZOOM_100;	break;
	case 125: id = ID_ZOOM_125;	break;
	case 150: id = ID_ZOOM_150;	break;
	case 200: id = ID_ZOOM_200;	break;
	case 300: id = ID_ZOOM_300;	break;
	case 400: id = ID_ZOOM_400;	break;
	default:  id = ID_ZOOM_AUTO;	break;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_ZOOM_100, ID_ZOOM_AUTO, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskMarkDlg::OnHelp()
{
	CString cstr;
	if(GetUserDefaultLCID() == 0x0411) // Japanese
	{
		cstr.Format(_T("%s%s"), m_ExeDir, HTML_HELP_JA);
		ShellExecute(NULL, NULL, cstr, NULL, NULL, SW_SHOWNORMAL);
	}
	else // Other Language
	{
		cstr.Format(_T("%s%s"), m_ExeDir, HTML_HELP_EN);
		ShellExecute(NULL, NULL, cstr, NULL, NULL, SW_SHOWNORMAL);
	}	
}

void CDiskMarkDlg::OnCrystalDewWorld()
{
	if(GetUserDefaultLCID() == 0x0411) // Japanese
	{
		ShellExecute(NULL, NULL, URL_CRYSTAL_DEW_WORLD_JA, NULL, NULL, SW_SHOWNORMAL);
	}
	else // Other Language
	{
		ShellExecute(NULL, NULL, URL_CRYSTAL_DEW_WORLD_EN, NULL, NULL, SW_SHOWNORMAL);
	}
}

void CDiskMarkDlg::OnModeDefault()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_MODE_DEFAULT, ID_MODE_ALL0XFF, ID_MODE_DEFAULT, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_TestData = TEST_DATA_DEFAULT;
	WritePrivateProfileString(_T("Setting"), _T("TestData"), _T("0"), m_Ini);
	SetWindowTitle(_T(""), _T(""));
}

void CDiskMarkDlg::OnModeAll0x00()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_MODE_DEFAULT, ID_MODE_ALL0XFF, ID_MODE_ALL0X00, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_TestData = TEST_DATA_ALL0X00;
	WritePrivateProfileString(_T("Setting"), _T("TestData"), _T("1"), m_Ini);
	SetWindowTitle(_T(""), ALL_0X00_0FILL);
}


void CDiskMarkDlg::OnModeAll0xFF()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_MODE_DEFAULT, ID_MODE_ALL0XFF, ID_MODE_ALL0XFF, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_TestData = TEST_DATA_ALL0XFF;
	WritePrivateProfileString(_T("Setting"), _T("TestData"), _T("2"), m_Ini);
	SetWindowTitle(_T(""), ALL_0XFF_1FILL);
}


