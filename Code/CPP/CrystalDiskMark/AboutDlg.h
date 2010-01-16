/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

class CAboutDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CAboutDlg)

public:
	CAboutDlg(CWnd* pParent = NULL);
	virtual ~CAboutDlg();
	HRESULT OnCrystalDewWorld(IHTMLElement *pElement);

	enum { IDD = IDD_ABOUT_DIALOG, IDH = IDR_HTML_ABOUTDLG };

protected:
	HACCEL m_hAccelerator;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL m_FlagShowWindow;

	afx_msg void OnWindowPosChanging(WINDOWPOS * lpwndpos); // for ShowWindow(SW_HIDE);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
