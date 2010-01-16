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

#include <math.h>
#include <exdispid.h>

#define MAX_METER_LENGTH	150
#define WM_THEME_ID			(WM_USER+0x100)
#define WM_LANGUAGE_ID		(WM_USER+0x200)
#define MAIN_CSS_FILE_NAME	_T("Main.css")

#define CLIENT_SIZE_X		400
#define CLIENT_SIZE_Y		300

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_DHTML_EVENT_MAP(CDiskMarkDlg)
	DHTML_EVENT_ONCLICK(_T("All"), OnAll)
	DHTML_EVENT_ONCLICK(_T("TestDrive"), OnSelectDrive)
	DHTML_EVENT_ONCLICK(_T("Sequential"), OnSequential)
	DHTML_EVENT_ONCLICK(_T("Random512KB"), OnRandom512KB)
	DHTML_EVENT_ONCLICK(_T("Random4KB"), OnRandom4KB)
END_DHTML_EVENT_MAP()

CDiskMarkDlg::CDiskMarkDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CDiskMarkDlg::IDD, CDiskMarkDlg::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// Init m_ini 
	TCHAR* ptrEnd;
	::GetModuleFileName(NULL, m_ini, MAX_PATH);
	if((ptrEnd = _tcsrchr(m_ini, '.')) != NULL)
	{
		*ptrEnd = '\0';
		_tcscat_s(m_ini, MAX_PATH, _T(".ini"));
	}
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

BEGIN_MESSAGE_MAP(CDiskMarkDlg, CDHtmlDialog)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_EXIT, OnExit)
	ON_COMMAND(ID_HELP_ABOUT, OnAbout)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_MESSAGE(WM_USER_UPDATE_SCORE, OnUpdateScore)
	ON_MESSAGE(WM_USER_UPDATE_MESSAGE, OnUpdateMessage)
	ON_MESSAGE(WM_USER_EXIT_BENCHMARK, OnExitBenchmark)
	ON_WM_WINDOWPOSCHANGING()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDiskMarkDlg::OnWindowPosChanging(WINDOWPOS * lpwndpos)
{
	if(! m_FlagShowWindow)
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}
    CDialog::OnWindowPosChanging(lpwndpos);
}

BOOL CDiskMarkDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	m_hAccelerator = ::LoadAccelerators(AfxGetInstanceHandle(),
		                                MAKEINTRESOURCE(IDR_ACCELERATOR));

	CString title;
	title.Format(_T("%s %s"),
		CRYSTAL_DISK_MARK_PRODUCT, CRYSTAL_DISK_MARK_VERSION);
	SetWindowText(title);

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_IndexTestDrive = 0;	// default value may be "C:\".
	m_IndexTestNumber = 4;	// default value is 5.
	m_IndexTestSize = 1;// default value is 100MB;

	m_WinThread = NULL;
	m_DiskBenchStatus = FALSE;
	m_FlagShowWindow = FALSE;

	TCHAR str[256];
	TCHAR tmp[MAX_PATH];
	TCHAR *ptrEnd;
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	CString langPath;
	int i = 0;
	WORD PrimaryLangID;
	CString PrimaryLang;

//////////////////////////
// Enabled Visual Style //
//////////////////////////
	DOCHOSTUIINFO info;
	info.cbSize = sizeof(info);
	GetHostInfo(&info);
	SetHostFlags(info.dwFlags | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_THEME);

///////////////
// Set Theme //
///////////////
	GetPrivateProfileString(_T("Setting"), _T("Theme"), _T("default"), str, 256, m_ini);
	m_CurrentTheme = str;

//////////////////
// Set Language //
//////////////////
	GetPrivateProfileString(_T("Setting"), _T("Language"), _T(""), str, 256, m_ini);

	langPath.Format(_T("%s\\%s.lang"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir, str);
	if(_tcscmp(str, _T("")) != 0 && IsFileExist((const TCHAR*)langPath)){
		m_CurrentLang = str;
	}else{
		m_CurrentLocalID.Format(_T("0x%04X"), GetUserDefaultLCID());
		PrimaryLangID = PRIMARYLANGID(GetUserDefaultLCID());

		langPath.Format(_T("%s\\*.lang"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir);

		hFind = ::FindFirstFile(langPath, &findData);
		if(hFind != INVALID_HANDLE_VALUE)
		{
			do{
				if(findData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
				{
					i++;
					CString cstr;
					cstr.Format(_T("%s\\%s"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir, findData.cFileName);
					GetPrivateProfileString(_T("Language"), _T("LOCALE_ID"), _T(""), str, 256, cstr);
					if((ptrEnd = _tcsrchr(findData.cFileName, '.')) != NULL){*ptrEnd = '\0';}

					if(_tcsstr(str, m_CurrentLocalID) != NULL){
						m_CurrentLang = findData.cFileName;
					}
					if(PrimaryLangID == PRIMARYLANGID(_tcstol(str, NULL, 16))){
						PrimaryLang = findData.cFileName;
					}
				}
			}while(::FindNextFile(hFind, &findData) && i <= 0xFF);
		}
		FindClose(hFind);

		if(m_CurrentLang.IsEmpty()){
			if(PrimaryLang.IsEmpty()){
				m_CurrentLang = _T("English");
			}else{
				m_CurrentLang = PrimaryLang;
			}	
		}
	}
	InitMenu();

//////////////////////////
// Navigate Main Dialog //
//////////////////////////

	GetModuleFileName(NULL, tmp, MAX_PATH);
	if((ptrEnd = _tcsrchr(tmp, '\\')) != NULL){*ptrEnd = '\0';}
	Navigate(_T("file://") + ((CDiskMarkApp*)AfxGetApp())->m_MainDialogPath, navNoHistory);

///////////////////
// ReSize Dialog //
///////////////////
	SetClientRect(CLIENT_SIZE_X, CLIENT_SIZE_Y);

	return TRUE;
}


void CDiskMarkDlg::SetClientRect(DWORD sizeX, DWORD sizeY)
{
	RECT rc;
	RECT clientRc;
	rc.left = 0;
	rc.top = 0;
	rc.right = sizeX;
	rc.bottom = sizeY;

	GetClientRect(&clientRc);
	if(clientRc.bottom - clientRc.top == sizeY && clientRc.right - clientRc.left == sizeX)
	{
		return;
	}

	AdjustWindowRect(&rc, WS_DLGFRAME, TRUE);
	SetWindowPos(&CWnd::wndTop, -1, -1, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE);

	GetClientRect(&clientRc);
	if(clientRc.bottom - clientRc.top != sizeY)
	{
		SetWindowPos(&CWnd::wndTop , -1, -1, 
			rc.right - rc.left,
			rc.bottom - rc.top + sizeY - (clientRc.bottom - clientRc.top), SWP_NOMOVE);	
	}
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
	return 0;
}

void CDiskMarkDlg::OnPaint()
{
	CDHtmlDialog::OnPaint();
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
	CAboutDlg dlg;
	dlg.DoModal();
}

void CDiskMarkDlg::OnCancel()
{
	if(m_WinThread != NULL){
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
}

HRESULT CDiskMarkDlg::OnSequential(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL){
		UpdateData(TRUE);
		m_SequentialReadScore = 0.0;
		m_SequentialWriteScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchSequential, (void*)this);
		if(m_WinThread == NULL){
			m_DiskBenchStatus = FALSE;
		}else{
			ChangeButtonStatus(FALSE);
		}
	}else{
		Stop();
	}
	return S_FALSE;
}

HRESULT CDiskMarkDlg::OnRandom512KB(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL){
		UpdateData(TRUE);
		m_RandomRead512KBScore = 0.0;
		m_RandomWrite512KBScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchRandom512KB, (void*)this);
		if(m_WinThread == NULL){
			m_DiskBenchStatus = FALSE;
		}else{
			ChangeButtonStatus(FALSE);
		}
	}else{
		Stop();
	}
	return S_FALSE;
}

HRESULT CDiskMarkDlg::OnRandom4KB(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL){
		UpdateData(TRUE);
		m_RandomRead4KBScore = 0.0;
		m_RandomWrite4KBScore = 0.0;
		UpdateScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchRandom4KB, (void*)this);
		if(m_WinThread == NULL){
			m_DiskBenchStatus = FALSE;
		}else{
			ChangeButtonStatus(FALSE);
		}
	}else{
		Stop();
	}
	return S_FALSE;
}

HRESULT CDiskMarkDlg::OnAll(IHTMLElement* /*pElement*/)
{
	if(m_WinThread == NULL){
		UpdateData(TRUE);
		InitScore();
		m_DiskBenchStatus = TRUE;
		m_WinThread = AfxBeginThread(ExecDiskBenchAll, (void*)this);
		if(m_WinThread == NULL){
			m_DiskBenchStatus = FALSE;
		}else{
			ChangeButtonStatus(FALSE);
		}
	}else{
		Stop();
	}
	return S_FALSE;
}

void CDiskMarkDlg::Stop()
{
	if(m_DiskBenchStatus){
		m_DiskBenchStatus = FALSE;
		UpdateMessage(_T("Message"), _T("Stopping..."));
	}
}

void CDiskMarkDlg::ChangeButtonStatus(BOOL status)
{
	if(status){
		ChangeButton(_T("All"),			_T("All.png"),			_T("All"));
		ChangeButton(_T("Sequential"),	_T("Sequential.png"),	_T("Sequential"));
		ChangeButton(_T("Random512KB"),	_T("Random512KB.png"),	_T("Random512KB"));
		ChangeButton(_T("Random4KB"),	_T("Random4KB.png"),	_T("Random4KB"));
		ChangeSelectStatus(_T("TestDrive"),	VARIANT_FALSE);
		ChangeSelectStatus(_T("TestNumber"),VARIANT_FALSE);
		ChangeSelectStatus(_T("TestSize"),	VARIANT_FALSE);
	}else{
		ChangeButton(_T("All"),			_T("Stop.png"),			_T("Stop"));
		ChangeButton(_T("Sequential"),	_T("Stop.png"),			_T("Stop"));
		ChangeButton(_T("Random512KB"),	_T("Stop.png"),			_T("Stop"));
		ChangeButton(_T("Random4KB"),	_T("Stop.png"),			_T("Stop"));
		ChangeSelectStatus(_T("TestDrive"),	VARIANT_TRUE);
		ChangeSelectStatus(_T("TestNumber"),VARIANT_TRUE);
		ChangeSelectStatus(_T("TestSize"),	VARIANT_TRUE);
	}
}

void CDiskMarkDlg::ChangeButton(CString ElementName, CString imgName, CString title)
{
	CComPtr<IHTMLStyle> pHtmlStyle;
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;

	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if(FAILED(hr)) return ;

	bstr = title;
	hr = pHtmlElement->put_title(bstr);
	if(FAILED(hr)) return ;

	hr = pHtmlElement->get_style(&pHtmlStyle);
	if(FAILED(hr)) return ;

	cstr.Format(_T("url(%s\\%s)"), m_CurrentTheme, imgName);
	bstr = cstr;
	pHtmlStyle->put_backgroundImage(bstr);
	if(FAILED(hr)) return ;
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
	CString cstr;
	cstr = *((CString*)wParam);
	UpdateMessage(_T("Message"), cstr);

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

	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if(FAILED(hr)) return ;

	hr = pHtmlElement->get_style(&pHtmlStyle);
	if(FAILED(hr)) return ;

	int meterLength;
	meterLength = (int)(MAX_METER_LENGTH / 3 * log10(Score));
	if(meterLength > MAX_METER_LENGTH){
		meterLength = MAX_METER_LENGTH;
	}else if(meterLength < 1){
		meterLength = 0;
	}

	cstr.Format(_T("%dpx"), -1 * (MAX_METER_LENGTH - meterLength));
	bstr = cstr;
	va.vt = VT_BSTR;
	va.bstrVal = bstr;
	pHtmlStyle->put_backgroundPositionX(va);

	if(Score > 100000.0){
		cstr.Format(_T("99999"));
	}else if(Score >= 10000.0){
		cstr.Format(_T("%5d"), int(Score));
	}else if(Score >= 1000.0){
		cstr.Format(_T("&nbsp;%4d"), int(Score));
	}else if(Score < 10.0){
		cstr.Format(_T("&nbsp;%.3f"), Score);
	}else if(Score < 100.0){
		cstr.Format(_T("&nbsp;%.2f"), Score);
	}else{
		cstr.Format(_T("&nbsp;%.1f"), Score);
	}
	bstr = cstr;
	pHtmlElement->put_innerHTML(bstr);

	UpdateData(FALSE);
}

void CDiskMarkDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if(cstr.Find(_T("dlg")) != -1){
		ChangeTheme(m_CurrentTheme);
		UpdateMessage(_T("Message"), _T(""));
		InitDrive(_T("TestDrive"));
		InitScore();
		ChangeLang(m_CurrentLang);
		UpdateData(TRUE);
		m_FlagShowWindow = TRUE;
		SetClientRect(CLIENT_SIZE_X, CLIENT_SIZE_Y);
		ShowWindow(SW_SHOW);
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
	TCHAR szDrives[128];
	TCHAR *pDrive;
	TCHAR rootPath[4];
	TCHAR fileSystem[32];
	GetLogicalDriveStrings(sizeof(szDrives), szDrives);
	pDrive = szDrives;

	select = _T("<select name=\"TestDrive\" id=\"TestDrive\" title=\"Test Drive\" onChange=\"this.click()\">\n");
	while( *pDrive )
	{
		_tcscpy_s(fileSystem, sizeof(fileSystem), _T(""));
		_tcsupr_s(pDrive, sizeof(TCHAR) * 4);
		int result = GetDriveType(pDrive);
		if((result == DRIVE_REMOVABLE) && (pDrive[0] == 'A' || pDrive[0] == 'B'))
		{
			pDrive += _tcslen(pDrive) + 1;
			continue;
		}
		int forward = (int)_tcslen( pDrive );
		pDrive[1] = _T('\0');
		cstr = _T("<option value=\"");
		cstr += pDrive;
		cstr += _T("\">");
		cstr += pDrive;
		cstr += _T(":");
		if(result == DRIVE_FIXED){
			rootPath[0] = pDrive[0];rootPath[1] = ':';rootPath[2] = '\\';rootPath[3] = '\0';
			GetVolumeInformation(rootPath, NULL, NULL, NULL, NULL, NULL, fileSystem, sizeof(fileSystem));
			cstr += _T("Hard Disk");
			select += cstr;
			if(fileSystem[0] != '\0'){
				cstr.Format(_T(" [%s]"), fileSystem);
				select += cstr;
			}
		}else if(result == DRIVE_REMOTE){
			rootPath[0] = pDrive[0];rootPath[1] = ':';rootPath[2] = '\\';rootPath[3] = '\0';
			GetVolumeInformation(rootPath, NULL, NULL, NULL, NULL, NULL, fileSystem, sizeof(fileSystem));
			cstr += _T("Remote");
			select += cstr;
			if(fileSystem[0] != '\0'){
				cstr.Format(_T(" [%s]"), fileSystem);
				select += cstr;
			}
		}else if(result == DRIVE_REMOVABLE){
			rootPath[0] = pDrive[0];rootPath[1] = ':';rootPath[2] = '\\';rootPath[3] = '\0';
			GetVolumeInformation(rootPath, NULL, NULL, NULL, NULL, NULL, fileSystem, sizeof(fileSystem));
			cstr += _T("Removable");
			select += cstr;
			if(fileSystem[0] != '\0'){
				cstr.Format(_T(" [%s]"), fileSystem);
				select += cstr;
			}
		}else if(result == DRIVE_RAMDISK){
			rootPath[0] = pDrive[0];rootPath[1] = ':';rootPath[2] = '\\';rootPath[3] = '\0';
			GetVolumeInformation(rootPath, NULL, NULL, NULL, NULL, NULL, fileSystem, sizeof(fileSystem));
			cstr += _T("RAM Disk");
			select += cstr;
			if(fileSystem[0] != '\0'){
				cstr.Format(_T(" [%s]"), fileSystem);
				select += cstr;
			}
		}else{
			cstr += _T("Unknown");
		}

		cstr += _T("</option>\n");
		pDrive += forward + 1;
	}

	select += _T("</select>");

	bstr = select;
	pHtmlElement->put_outerHTML(bstr);
	UpdateData(FALSE);
}

void CDiskMarkDlg::ChangeTheme(CString ThemeName)
{
	CComPtr<IHTMLDocument2> pHtmlDoc;
	CComPtr<IHTMLStyleSheet> pHtmlStyleSheet;
	CComPtr<IHTMLStyleSheetsCollection> pStyleSheetsCollection;

	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	LONG length;

	hr = GetDHtmlDocument(&pHtmlDoc);
	if(FAILED(hr)) return ;

	hr = pHtmlDoc->get_styleSheets(&pStyleSheetsCollection);
	if(FAILED(hr)) return ;

	hr = pStyleSheetsCollection->get_length(&length);
	if(FAILED(hr)) return ;

	VARIANT index;
// by ordinal
	index.vt = VT_I4;
	index.intVal = 0;
// by name
//	index.vt = VT_BSTR;
//	index.bstrVal = L"StyleSheet";
	VARIANT dispatch;
	dispatch.vt = VT_DISPATCH;

	hr = pStyleSheetsCollection->item(&index, &dispatch);
	if(FAILED(hr)) return ;

	dispatch.pdispVal->QueryInterface(IID_IHTMLStyleSheet, (void **) &pHtmlStyleSheet);
	if(FAILED(hr)) return ;

	cstr.Format(_T("%s\\%s"), ThemeName, MAIN_CSS_FILE_NAME);
	bstr = cstr;
	hr = pHtmlStyleSheet->put_href(bstr);
	if(FAILED(hr)) return ;

	WritePrivateProfileString(_T("Setting"), _T("Theme"), ThemeName, m_ini);

	ChangeButtonStatus(! m_DiskBenchStatus);
}

void CDiskMarkDlg::ChangeLang(CString LangName)
{
	CString langPath;
	TCHAR str[256];
	langPath.Format(_T("%s\\%s.lang"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir, LangName);
	CMenu *menu = GetMenu();

	GetPrivateProfileString(_T("Menu"), _T("FILE"), _T("&File"), str, 256, langPath);
	menu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, str);
	GetPrivateProfileString(_T("Menu"), _T("EDIT"), _T("&Edit"), str, 256, langPath);
	menu->ModifyMenu(1, MF_BYPOSITION | MF_STRING, 1, str);
	GetPrivateProfileString(_T("Menu"), _T("THEME"), _T("&Theme"), str, 256, langPath);
	menu->ModifyMenu(2, MF_BYPOSITION | MF_STRING, 2, str);
	GetPrivateProfileString(_T("Menu"), _T("HELP"), _T("&Help"), str, 256, langPath);
	menu->ModifyMenu(3, MF_BYPOSITION | MF_STRING, 3, str);

	GetPrivateProfileString(_T("Menu"), _T("FILE_EXIT"), _T("&Exit"), str, 256, langPath);
	menu->ModifyMenu(ID_FILE_EXIT, MF_STRING, ID_FILE_EXIT, str);
	GetPrivateProfileString(_T("Menu"), _T("EDIT_COPY"), _T("&Copy"), str, 256, langPath);
	menu->ModifyMenu(ID_EDIT_COPY, MF_STRING, ID_EDIT_COPY, str);
	GetPrivateProfileString(_T("Menu"), _T("HELP_ABOUT"), _T("&About CrystalDiskMark"), str, 256, langPath);
	menu->ModifyMenu(ID_HELP_ABOUT, MF_STRING, ID_HELP_ABOUT, str);

	SetMenu(menu);

	GetPrivateProfileString(_T("Message"), _T("STOP_BENCHMARK"), _T("Please Stop Benchmark."), str, 256, langPath);
	m_MesStopBenchmark = str;
	GetPrivateProfileString(_T("Message"), _T("DISK_CAPACITY_ERROR"), _T("Disk capacity is insufficient."), str, 256, langPath);
	m_MesDiskCapacityError = str;
	GetPrivateProfileString(_T("Message"), _T("DISK_CREATE_FILE_ERROR"), _T("Failed Create File."), str, 256, langPath);
	m_MesDiskCreateFileError = str;
	GetPrivateProfileString(_T("Message"), _T("DISK_WRITE_ERROR"), _T("Write Error."), str, 256, langPath);
	m_MesDiskWriteError = str;
	GetPrivateProfileString(_T("Message"), _T("DISK_READ_ERROR"), _T("Read Error."), str, 256, langPath);
	m_MesDiskReadError = str;

	GetPrivateProfileString(_T("Title"), _T("TEST_DRIVE"), _T("Test Drive"), str, 256, langPath);
	m_TitleTestDrive = str;
	GetPrivateProfileString(_T("Title"), _T("TEST_SIZE"), _T("Test Size"), str, 256, langPath);
	m_TitleTestSize = str;
	GetPrivateProfileString(_T("Title"), _T("TEST_NUMBER"), _T("Test Number"), str, 256, langPath);
	m_TitleTestNumber = str;

	ChangeSelectTitle(_T("TestDrive"),	m_TitleTestDrive);
	ChangeSelectTitle(_T("TestNumber"),	m_TitleTestNumber);
	ChangeSelectTitle(_T("TestSize"),	m_TitleTestSize);

	WritePrivateProfileString(_T("Setting"), _T("Language"), LangName, m_ini);

	SetClientRect(CLIENT_SIZE_X, CLIENT_SIZE_Y);
}

void CDiskMarkDlg::InitMenu()
{
	CMenu menu;
	CMenu subMenu;
	BOOL FlagHitTheme = FALSE;
	BOOL FlagHitLang = FALSE;
	UINT newItemID = 0;
	UINT currentItemID = 0;
	UINT defaultStyleItemID = 0;
	UINT defaultLanguageItemID = 0;
	WIN32_FIND_DATA findData;
	WIN32_FIND_DATA findCssData;
	HANDLE hFind;
	HANDLE hCssFind;
	CString themePath;
	CString themeCssPath;
	CString langPath;
	int i = 0;
	TCHAR *ptrEnd;
	TCHAR str[256];
	
	menu.Attach(GetMenu()->GetSafeHmenu());
	subMenu.Attach(menu.GetSubMenu(2)->GetSafeHmenu()); // 3rd is "Theme".
	subMenu.RemoveMenu(0, MF_BYPOSITION);

	themePath.Format(_T("%s\\*.*"), ((CDiskMarkApp*)AfxGetApp())->m_ThemeDir);

	hFind = ::FindFirstFile(themePath, &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		while(::FindNextFile(hFind, &findData) && i <= 0xFF)
		{
			if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				themeCssPath.Format(_T("%s\\%s\\%s"), ((CDiskMarkApp*)AfxGetApp())->m_ThemeDir,
									findData.cFileName, MAIN_CSS_FILE_NAME);
				hCssFind = ::FindFirstFile(themeCssPath, &findCssData);
				if(hCssFind != INVALID_HANDLE_VALUE)
				{
					// Add Theme
					newItemID = WM_THEME_ID + i;
					i++;
					subMenu.AppendMenu(MF_STRING, (UINT_PTR)newItemID, findData.cFileName);
					m_MenuArrayTheme.Add(findData.cFileName);
					if(m_CurrentTheme.Compare(findData.cFileName) == 0)
					{
						currentItemID = newItemID;
						FlagHitTheme = TRUE;
					}
					// default style 1.0.4
					if(_tcsstr(findData.cFileName, _T("default")) != NULL)
					{
						defaultStyleItemID = newItemID;
					}
				}
			}
		}
	}
	FindClose(hFind);

	if(! FlagHitTheme)
	{
		currentItemID = defaultStyleItemID;
		m_CurrentTheme = _T("default");
	}

	subMenu.CheckMenuRadioItem(WM_THEME_ID, WM_THEME_ID + (UINT)m_MenuArrayTheme.GetSize(),
								currentItemID, MF_BYCOMMAND);

	subMenu.Detach();

#ifdef _UNICODE
	subMenu.Attach(menu.GetSubMenu(4)->GetSafeHmenu()); // 5th is "Language".

	CMenu subMenuAN;
	CMenu subMenuOZ;

	subMenuAN.Attach(subMenu.GetSubMenu(0)->GetSafeHmenu()); // 1st is "A~N"
	subMenuAN.RemoveMenu(0, MF_BYPOSITION);
	subMenuOZ.Attach(subMenu.GetSubMenu(1)->GetSafeHmenu()); // 2nd is "O~Z"
	subMenuOZ.RemoveMenu(0, MF_BYPOSITION);

	langPath.Format(_T("%s\\*.lang"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir);
	i = 0;
	hFind = ::FindFirstFile(langPath, &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do{
			if(findData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
				newItemID = WM_LANGUAGE_ID + i;
				i++;

				// Add Language
				CString cstr;
				cstr.Format(_T("%s\\%s"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir, findData.cFileName);
				GetPrivateProfileString(_T("Language"), _T("LANGUAGE"), _T(""), str, 256, cstr);
				if((ptrEnd = _tcsrchr(findData.cFileName, '.')) != NULL){*ptrEnd = '\0';}

				cstr.Format(_T("%s, [%s]"), str, findData.cFileName);
				if('A' <= findData.cFileName[0] && findData.cFileName[0] <= 'N'){
					subMenuAN.AppendMenu(MF_STRING, (UINT_PTR)newItemID, cstr);
				}else{
					subMenuOZ.AppendMenu(MF_STRING, (UINT_PTR)newItemID, cstr);
				}
				m_MenuArrayLang.Add(findData.cFileName);

				if(m_CurrentLang.Compare(findData.cFileName) == 0){
					currentItemID = newItemID;
					FlagHitLang = TRUE;
				}
			}
		}while(::FindNextFile(hFind, &findData) && i <= 0xFF);
	}
	FindClose(hFind);

	subMenuAN.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
								currentItemID, MF_BYCOMMAND);
	subMenuOZ.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
								currentItemID, MF_BYCOMMAND);

	subMenuOZ.Detach();
	subMenuAN.Detach();
	subMenu.Detach();
	menu.Detach();
#else
	subMenu.Attach(menu.GetSubMenu(4)->GetSafeHmenu()); // 5th is "Language".
	subMenu.RemoveMenu(0, MF_BYPOSITION);//A~N
	subMenu.RemoveMenu(0, MF_BYPOSITION);//O~Z
	langPath.Format(_T("%s\\*.lang"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir);
	i = 0;
	hFind = ::FindFirstFile(langPath, &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do{
			if(findData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
			{
				newItemID = WM_LANGUAGE_ID + i;
				i++;

				// Add Language
				CString cstr;
				cstr.Format(_T("%s\\%s"), ((CDiskMarkApp*)AfxGetApp())->m_LangDir, findData.cFileName);
				GetPrivateProfileString(_T("Language"), _T("LANGUAGE"), _T(""), str, 256, cstr);
				if((ptrEnd = _tcsrchr(findData.cFileName, '.')) != NULL){*ptrEnd = '\0';}

				cstr.Format(_T("%s, [%s]"), str, findData.cFileName);
				subMenu.AppendMenu(MF_STRING, (UINT_PTR)newItemID, cstr);
				m_MenuArrayLang.Add(findData.cFileName);

				if(m_CurrentLang.Compare(findData.cFileName) == 0)
				{
					currentItemID = newItemID;
					FlagHitLang = TRUE;
				}
			}
		}while(::FindNextFile(hFind, &findData) && i <= 0xFF);

	}
	FindClose(hFind);

	subMenu.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
								currentItemID, MF_BYCOMMAND);

	subMenu.Detach();
	menu.Detach();
#endif
	if(! FlagHitLang)
	{
		AfxMessageBox(_T("Fatal Error. Missing Language Files!!"));
	}
}

BOOL CDiskMarkDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(wParam == WM_USER_EXIT_BENCHMARK){
		Stop();
	}else if(WM_THEME_ID <= wParam && wParam < WM_THEME_ID + (UINT)m_MenuArrayTheme.GetSize()){	// Select Theme
		CMenu menu;
		CMenu subMenu;
		menu.Attach(GetMenu()->GetSafeHmenu());
		subMenu.Attach(menu.GetSubMenu(2)->GetSafeHmenu()); // 3rd is "Theme".

		m_CurrentTheme = m_MenuArrayTheme.GetAt(wParam - WM_THEME_ID);
		ChangeTheme(m_MenuArrayTheme.GetAt(wParam - WM_THEME_ID));
		subMenu.CheckMenuRadioItem(WM_THEME_ID, WM_THEME_ID + (UINT)m_MenuArrayTheme.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);
		subMenu.Detach();
		menu.Detach();
	}else if(WM_LANGUAGE_ID <= wParam && wParam < WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize()){ // Select Language
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

	return CDialog::OnCommand(wParam, lParam);
}

void CDiskMarkDlg::OnEditCopy()
{
	CString cstr, clip;

	UpdateData(TRUE);

	clip = _T("\
--------------------------------------------------\r\n\
%PRODUCT% %VERSION% (C) %COPY_YEAR% hiyohiyo\r\n\
      Crystal Dew World : http://crystalmark.info/\r\n\
--------------------------------------------------\r\n\
\r\n\
   Sequential Read : %SequentialRead%\r\n\
  Sequential Write : %SequentialWrite%\r\n\
 Random Read 512KB : %RandomRead512KB%\r\n\
Random Write 512KB : %RandomWrite512KB%\r\n\
   Random Read 4KB : %RandomRead4KB%\r\n\
  Random Write 4KB : %RandomWrite4KB%\r\n\
\r\n\
         Test Size : %TestSize%\r\n\
              Date : %Date%\r\n\
%Comment%\
");
	clip.Replace(_T("%PRODUCT%"), CRYSTAL_DISK_MARK_PRODUCT);
	clip.Replace(_T("%VERSION%"), CRYSTAL_DISK_MARK_VERSION);
	clip.Replace(_T("%COPY_YEAR%"), CRYSTAL_DISK_MARK_YEAR);

	cstr.Format(_T("%8.3f MB/s"), m_SequentialReadScore);
	clip.Replace(_T("%SequentialRead%"), cstr);
	cstr.Format(_T("%8.3f MB/s"), m_SequentialWriteScore);
	clip.Replace(_T("%SequentialWrite%"), cstr);
	cstr.Format(_T("%8.3f MB/s"), m_RandomRead512KBScore);
	clip.Replace(_T("%RandomRead512KB%"), cstr);
	cstr.Format(_T("%8.3f MB/s"), m_RandomWrite512KBScore);
	clip.Replace(_T("%RandomWrite512KB%"), cstr);
	cstr.Format(_T("%8.3f MB/s"), m_RandomRead4KBScore);
	clip.Replace(_T("%RandomRead4KB%"), cstr);
	cstr.Format(_T("%8.3f MB/s"), m_RandomWrite4KBScore);
	clip.Replace(_T("%RandomWrite4KB%"), cstr);

	// Added 1.0.6
	cstr.Format(_T("%d MB"), _tstoi(m_ValueTestSize));
	clip.Replace(_T("%TestSize%"), cstr);

	if(m_Comment.IsEmpty())
	{
		clip.Replace(_T("%Comment%"), _T(""));
	}else{
		clip.Replace(_T("%Comment%"), _T("::Comment::\r\n") + m_Comment + _T("\r\n"));
	}

/*
               OS : %OS%\r\n\
	CString OSName,OSType,OSCSD,OSVersion,OSBuild,OSInfo;
	GetOSInfo(OSName,OSType,OSCSD,OSVersion,OSBuild,OSInfo);
	cstr = OSName + " " + OSType + " " + OSCSD;
	clip.Replace("%OS%", m_OS);
*/

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
