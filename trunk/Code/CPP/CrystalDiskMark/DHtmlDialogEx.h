/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2007-2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

class CDHtmlDialogEx : public CDHtmlDialog
{
public:
	CDHtmlDialogEx(UINT dlgResouce, UINT dlgHtml, CWnd* pParent = NULL);
	virtual ~CDHtmlDialogEx();

	void InitDHtmlDialog(DWORD sizeX, DWORD sizeY, CString dlgPath);
	void SetClientRect(DWORD sizeX, DWORD sizeY, DWORD menuLine = 0);
	void ShowWindowEx(int nCmdShow);

	virtual BOOL Create(UINT nIDTemplate, CWnd* dlgWnd, UINT menuId, CWnd* pParentWnd = NULL);

	CString m_CurrentLangPath;
	CString m_DefaultLangPath;

	enum ZOOM_TYPE
	{
		ZOOM_TYPE_AUTO = 0,
		ZOOM_TYPE_100 = 100,
		ZOOM_TYPE_125 = 125,
		ZOOM_TYPE_150 = 150,
		ZOOM_TYPE_200 = 200,
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	HACCEL m_hAccelerator;
	BOOL m_FlagShowWindow;
	BOOL m_FlagModelessDlg;
	CWnd* m_ParentWnd;
	CWnd* m_DlgWnd;
	UINT m_MenuId;
	TCHAR m_Ini[MAX_PATH];

	double m_ZoomRatio;
	DWORD m_ZoomType;

	void SetElementPropertyEx(LPCTSTR szElementId, DISPID dispid, CString className);
	void SetElementOuterHtmlEx(LPCTSTR szElementId, CString outerHtml);
	void SetElementInnerHtmlEx(LPCTSTR szElementId, CString innerHtml);
	void CallScript(CString funcition, CString argument);
	void EnableDpiAware();
	DWORD ChangeZoomType(DWORD zoomType);
	double GetZoomRatio();
	CString i18n(CString section, CString key);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual void PostNcDestroy();
	virtual BOOL OnAmbientProperty(COleControlSite* pSite, DISPID dispid, VARIANT* pvar);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
