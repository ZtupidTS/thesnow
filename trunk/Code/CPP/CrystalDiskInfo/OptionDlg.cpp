/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2008-2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskInfo.h"
#include "GraphDlg.h"
#include "OptionDlg.h"

IMPLEMENT_DYNCREATE(COptionDlg, CDHtmlDialog)

COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialogEx(COptionDlg::IDD, COptionDlg::IDH, pParent)
{
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskInfoApp*)AfxGetApp())->m_Ini.GetString());
	
	m_CurrentLangPath = ((CDHtmlMainDialog*)pParent)->m_CurrentLangPath;
	m_DefaultLangPath = ((CDHtmlMainDialog*)pParent)->m_DefaultLangPath;
	m_ZoomType = ((CDHtmlMainDialog*)pParent)->GetZoomType();

	for(int i = 0; i <= CAtaSmart::MAX_DISK; i++)
	{
		m_CurrentLineColor[i] = ((CGraphDlg*)pParent)->GetLineColor(i);
		m_DefaultLineColor[i] = ((CGraphDlg*)pParent)->m_DefaultLineColor[i];
	}
}

COptionDlg::~COptionDlg()
{
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialogEx::DoDataExchange(pDX);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelThreshold"), m_LabelThreshold);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelLineColor"), m_LabelLineColor);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelBgImage"), m_LabelBgImage);
	DDX_DHtml_ElementInnerText(pDX, _T("BgImage"), m_BgImage);
	DDX_DHtml_ElementInnerText(pDX, _T("Reset"), m_Reset);

	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode0"), m_ColorCode[0]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode1"), m_ColorCode[1]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode2"), m_ColorCode[2]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode3"), m_ColorCode[3]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode4"), m_ColorCode[4]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode5"), m_ColorCode[5]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode6"), m_ColorCode[6]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode7"), m_ColorCode[7]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode8"), m_ColorCode[8]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode9"), m_ColorCode[9]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode10"), m_ColorCode[10]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode11"), m_ColorCode[11]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode12"), m_ColorCode[12]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode13"), m_ColorCode[13]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode14"), m_ColorCode[14]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode15"), m_ColorCode[15]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode16"), m_ColorCode[16]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode17"), m_ColorCode[17]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode18"), m_ColorCode[18]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode19"), m_ColorCode[19]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode20"), m_ColorCode[20]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode21"), m_ColorCode[21]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode22"), m_ColorCode[22]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode23"), m_ColorCode[23]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode24"), m_ColorCode[24]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode25"), m_ColorCode[25]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode26"), m_ColorCode[26]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode27"), m_ColorCode[27]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode28"), m_ColorCode[28]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode29"), m_ColorCode[29]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode30"), m_ColorCode[30]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode31"), m_ColorCode[31]);
	DDX_DHtml_ElementInnerText(pDX, _T("ColorCode32"), m_ColorCode[32]);
}

BOOL COptionDlg::OnInitDialog()
{
	CDHtmlDialogEx::OnInitDialog();

	SetWindowText(i18n(_T("WindowTitle"), _T("CUSTOMIZE")));

	EnableDpiAware();
	InitDHtmlDialog(SIZE_X, SIZE_Y, ((CDiskInfoApp*)AfxGetApp())->m_OptionDlgPath);

	return TRUE;
}

BEGIN_MESSAGE_MAP(COptionDlg, CDHtmlDialogEx)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(COptionDlg)
	DHTML_EVENT_ONCLICK(_T("Select0"), OnSelect0)
	DHTML_EVENT_ONCLICK(_T("Select1"), OnSelect1)
	DHTML_EVENT_ONCLICK(_T("Select2"), OnSelect2)
	DHTML_EVENT_ONCLICK(_T("Select3"), OnSelect3)
	DHTML_EVENT_ONCLICK(_T("Select4"), OnSelect4)
	DHTML_EVENT_ONCLICK(_T("Select5"), OnSelect5)
	DHTML_EVENT_ONCLICK(_T("Select6"), OnSelect6)
	DHTML_EVENT_ONCLICK(_T("Select7"), OnSelect7)
	DHTML_EVENT_ONCLICK(_T("Select8"), OnSelect8)
	DHTML_EVENT_ONCLICK(_T("Select9"), OnSelect9)
	DHTML_EVENT_ONCLICK(_T("Select10"), OnSelect10)
	DHTML_EVENT_ONCLICK(_T("Select11"), OnSelect11)
	DHTML_EVENT_ONCLICK(_T("Select12"), OnSelect12)
	DHTML_EVENT_ONCLICK(_T("Select13"), OnSelect13)
	DHTML_EVENT_ONCLICK(_T("Select14"), OnSelect14)
	DHTML_EVENT_ONCLICK(_T("Select15"), OnSelect15)
	DHTML_EVENT_ONCLICK(_T("Select16"), OnSelect16)
	DHTML_EVENT_ONCLICK(_T("Select17"), OnSelect17)
	DHTML_EVENT_ONCLICK(_T("Select18"), OnSelect18)
	DHTML_EVENT_ONCLICK(_T("Select19"), OnSelect19)
	DHTML_EVENT_ONCLICK(_T("Select20"), OnSelect20)
	DHTML_EVENT_ONCLICK(_T("Select21"), OnSelect21)
	DHTML_EVENT_ONCLICK(_T("Select22"), OnSelect22)
	DHTML_EVENT_ONCLICK(_T("Select23"), OnSelect23)
	DHTML_EVENT_ONCLICK(_T("Select24"), OnSelect24)
	DHTML_EVENT_ONCLICK(_T("Select25"), OnSelect25)
	DHTML_EVENT_ONCLICK(_T("Select26"), OnSelect26)
	DHTML_EVENT_ONCLICK(_T("Select27"), OnSelect27)
	DHTML_EVENT_ONCLICK(_T("Select28"), OnSelect28)
	DHTML_EVENT_ONCLICK(_T("Select29"), OnSelect29)
	DHTML_EVENT_ONCLICK(_T("Select30"), OnSelect30)
	DHTML_EVENT_ONCLICK(_T("Select31"), OnSelect31)
	DHTML_EVENT_ONCLICK(_T("Select32"), OnSelect32)
	DHTML_EVENT_ONCLICK(_T("SelectBgImage"), OnSelectBgImage)
	DHTML_EVENT_ONCLICK(_T("NoBgImage"), OnNoBgImage)
	DHTML_EVENT_ONCLICK(_T("Reset"), OnReset)
END_DHTML_EVENT_MAP()


void COptionDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		m_FlagShowWindow = TRUE;

		m_LabelThreshold = i18n(_T("Dialog"), _T("LIST_THRESHOLD"));
		m_LabelLineColor = i18n(_T("Customize"), _T("LINE_COLOR"));
		m_LabelBgImage   = i18n(_T("Customize"), _T("BACKGROUND_IMAGE"));
		m_Reset          = i18n(_T("Graph"), _T("RESET"));

		TCHAR str[256];
		GetPrivateProfileString(_T("Customize"), _T("GraphBgImage"), _T(""), str, 256, m_Ini);
		m_BgImage = str;

		for(int i = 0; i <= CAtaSmart::MAX_DISK; i++)
		{
			cstr.Format(_T("%d"), i);
			m_ColorCode[i].Format(_T("#%02x%02x%02x"),
				GetRValue(m_CurrentLineColor[i]),
				GetGValue(m_CurrentLineColor[i]),
				GetBValue(m_CurrentLineColor[i]));
			CallScript(_T("changeBackgroundColor"), cstr + _T(", ") + m_ColorCode[i]);
		}

		UpdateData(FALSE);
		ChangeZoomType(m_ZoomType);
		SetClientRect((DWORD)(SIZE_X * m_ZoomRatio), (DWORD)(SIZE_Y * m_ZoomRatio), 0);

		ShowWindow(SW_SHOW);
	}
}

HRESULT COptionDlg::OnSelect0(IHTMLElement* /*pElement*/){SelectColor(0);return S_FALSE;}
HRESULT COptionDlg::OnSelect1(IHTMLElement* /*pElement*/){SelectColor(1);return S_FALSE;}
HRESULT COptionDlg::OnSelect2(IHTMLElement* /*pElement*/){SelectColor(2);return S_FALSE;}
HRESULT COptionDlg::OnSelect3(IHTMLElement* /*pElement*/){SelectColor(3);return S_FALSE;}
HRESULT COptionDlg::OnSelect4(IHTMLElement* /*pElement*/){SelectColor(4);return S_FALSE;}
HRESULT COptionDlg::OnSelect5(IHTMLElement* /*pElement*/){SelectColor(5);return S_FALSE;}
HRESULT COptionDlg::OnSelect6(IHTMLElement* /*pElement*/){SelectColor(6);return S_FALSE;}
HRESULT COptionDlg::OnSelect7(IHTMLElement* /*pElement*/){SelectColor(7);return S_FALSE;}
HRESULT COptionDlg::OnSelect8(IHTMLElement* /*pElement*/){SelectColor(8);return S_FALSE;}
HRESULT COptionDlg::OnSelect9(IHTMLElement* /*pElement*/){SelectColor(9);return S_FALSE;}
HRESULT COptionDlg::OnSelect10(IHTMLElement* /*pElement*/){SelectColor(10);return S_FALSE;}
HRESULT COptionDlg::OnSelect11(IHTMLElement* /*pElement*/){SelectColor(11);return S_FALSE;}
HRESULT COptionDlg::OnSelect12(IHTMLElement* /*pElement*/){SelectColor(12);return S_FALSE;}
HRESULT COptionDlg::OnSelect13(IHTMLElement* /*pElement*/){SelectColor(13);return S_FALSE;}
HRESULT COptionDlg::OnSelect14(IHTMLElement* /*pElement*/){SelectColor(14);return S_FALSE;}
HRESULT COptionDlg::OnSelect15(IHTMLElement* /*pElement*/){SelectColor(15);return S_FALSE;}
HRESULT COptionDlg::OnSelect16(IHTMLElement* /*pElement*/){SelectColor(16);return S_FALSE;}
HRESULT COptionDlg::OnSelect17(IHTMLElement* /*pElement*/){SelectColor(17);return S_FALSE;}
HRESULT COptionDlg::OnSelect18(IHTMLElement* /*pElement*/){SelectColor(18);return S_FALSE;}
HRESULT COptionDlg::OnSelect19(IHTMLElement* /*pElement*/){SelectColor(19);return S_FALSE;}
HRESULT COptionDlg::OnSelect20(IHTMLElement* /*pElement*/){SelectColor(20);return S_FALSE;}
HRESULT COptionDlg::OnSelect21(IHTMLElement* /*pElement*/){SelectColor(21);return S_FALSE;}
HRESULT COptionDlg::OnSelect22(IHTMLElement* /*pElement*/){SelectColor(22);return S_FALSE;}
HRESULT COptionDlg::OnSelect23(IHTMLElement* /*pElement*/){SelectColor(23);return S_FALSE;}
HRESULT COptionDlg::OnSelect24(IHTMLElement* /*pElement*/){SelectColor(24);return S_FALSE;}
HRESULT COptionDlg::OnSelect25(IHTMLElement* /*pElement*/){SelectColor(25);return S_FALSE;}
HRESULT COptionDlg::OnSelect26(IHTMLElement* /*pElement*/){SelectColor(26);return S_FALSE;}
HRESULT COptionDlg::OnSelect27(IHTMLElement* /*pElement*/){SelectColor(27);return S_FALSE;}
HRESULT COptionDlg::OnSelect28(IHTMLElement* /*pElement*/){SelectColor(28);return S_FALSE;}
HRESULT COptionDlg::OnSelect29(IHTMLElement* /*pElement*/){SelectColor(29);return S_FALSE;}
HRESULT COptionDlg::OnSelect30(IHTMLElement* /*pElement*/){SelectColor(30);return S_FALSE;}
HRESULT COptionDlg::OnSelect31(IHTMLElement* /*pElement*/){SelectColor(31);return S_FALSE;}
HRESULT COptionDlg::OnSelect32(IHTMLElement* /*pElement*/){SelectColor(32);return S_FALSE;}

void COptionDlg::SelectColor(DWORD i)
{
	CColorDialog dlg;
	if(dlg.DoModal() == IDOK)
	{
		COLORREF color = dlg.GetColor();
		CString cstr1, cstr2;
		cstr1.Format(_T("%d"), i);
		cstr2.Format(_T("#%02x%02x%02x"), GetRValue(color), GetGValue(color), GetBValue(color));
		WritePrivateProfileString(_T("LineColor"), cstr1, cstr2, m_Ini);
		m_ColorCode[i] = cstr2;

		CallScript(_T("changeBackgroundColor"), cstr1 + _T(", ") + cstr2);
		UpdateData(FALSE);
		::PostMessage(m_ParentWnd->GetSafeHwnd(), MY_UPDATE_LINE_COLOR, NULL, NULL);
	}
}

HRESULT COptionDlg::OnSelectBgImage(IHTMLElement* /*pElement*/)
{
	CFileDialog dlg(TRUE, _T(""), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		i18n(_T("Customize"), _T("SUPPORTED_IMAGE_FORMAT"))
		+ _T("|*.bmp;*.png;*.jpg;*.jpeg;*.gif|BMP (*.bmp)|*.bmp|PNG (*.png)|*.png|JPEG (*.jpg *.jpeg)|*.jpg;*.jpeg|GIF (*.gif)|*.gif;|")
		+ i18n(_T("Customize"), _T("ALL_FILES"))
		+ _T(" (*.*)|*.*||"));

	if(dlg.DoModal() == IDOK)
	{
		m_BgImage = dlg.GetPathName();
		WritePrivateProfileString(_T("Customize"), _T("GraphBgImage"), m_BgImage, m_Ini);
		UpdateData(FALSE);
		::PostMessage(m_ParentWnd->GetSafeHwnd(), MY_UPDATE_BG_IMAGE, NULL, NULL);
	}
	return S_FALSE;
}

HRESULT COptionDlg::OnNoBgImage(IHTMLElement* /*pElement*/)
{
	m_BgImage = _T("");
	WritePrivateProfileString(_T("Customize"), _T("GraphBgImage"), m_BgImage, m_Ini);
	UpdateData(FALSE);
	::PostMessage(m_ParentWnd->GetSafeHwnd(), MY_UPDATE_BG_IMAGE, NULL, NULL);
	return S_FALSE;
}

HRESULT COptionDlg::OnReset(IHTMLElement* /*pElement*/)
{
	for(int i = 0; i <= CAtaSmart::MAX_DISK; i++)
	{
		COLORREF color = m_DefaultLineColor[i];
		CString cstr1, cstr2;
		cstr1.Format(_T("%d"), i);
		cstr2.Format(_T("#%02x%02x%02x"), GetRValue(color), GetGValue(color), GetBValue(color));
		WritePrivateProfileString(_T("LineColor"), cstr1, cstr2, m_Ini);
		m_ColorCode[i] = cstr2;

		CallScript(_T("changeBackgroundColor"), cstr1 + _T(", ") + cstr2);
	}
	::PostMessage(m_ParentWnd->GetSafeHwnd(), MY_UPDATE_LINE_COLOR, NULL, NULL);
	UpdateData(FALSE);

	return S_FALSE;
}