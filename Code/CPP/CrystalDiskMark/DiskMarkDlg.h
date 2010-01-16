/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

class CDiskMarkDlg : public CDHtmlDialog
{
public:
	CDiskMarkDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_DISKMARK_DIALOG, IDH = IDR_HTML_DISKMARK_DIALOG };

	volatile CWinThread* m_WinThread;
	volatile BOOL m_DiskBenchStatus;

	void InitScore();
	void UpdateScore();

	double m_SequentialReadScore;
	double m_SequentialWriteScore;
	double m_RandomRead512KBScore;
	double m_RandomWrite512KBScore;
	double m_RandomRead4KBScore;
	double m_RandomWrite4KBScore;

	void SetMeter(CString ElementName, double Score);
	void UpdateMessage(CString ElementName, CString message);
	void ChangeTheme(CString ThemeName);
	void ChangeLang(CString LangName);
	void ChangeButtonStatus(BOOL status);
	void ChangeButton(CString ElementName, CString imgName, CString title);
	void ChangeSelectStatus(CString ElementName, VARIANT_BOOL status);
	void ChangeSelectTitle(CString ElementName, CString title);

	CString m_CurrentLocalID;
	CString m_ValueTestDrive;
	CString m_ValueTestNumber;
	CString m_ValueTestSize;
	long m_IndexTestDrive;
	long m_IndexTestNumber;
	long m_IndexTestSize;

	TCHAR m_ini[MAX_PATH];

	// Message //
	CString m_MesDiskCapacityError;
	CString m_MesDiskWriteError;
	CString m_MesDiskReadError;
	CString m_MesStopBenchmark;
	CString m_MesDiskCreateFileError;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnAll(IHTMLElement *pElement);
	HRESULT OnSelectDrive(IHTMLElement *pElement);
	HRESULT OnSequential(IHTMLElement *pElement);
	HRESULT OnRandom512KB(IHTMLElement *pElement);
	HRESULT OnRandom4KB(IHTMLElement *pElement);

	void Stop();

	CStringArray m_MenuArrayTheme;
	CStringArray m_MenuArrayLang;
	CString m_CurrentTheme;
	CString m_CurrentLang;

	CString m_TitleTestDrive;
	CString m_TitleTestNumber;
	CString m_TitleTestSize;

	CString m_Comment;

protected:
	HICON m_hIcon;
	HACCEL m_hAccelerator;

	BOOL m_FlagShowWindow;

	void InitDrive(CString ElementName);
	void InitMenu();

	void SetClientRect(DWORD sizeX, DWORD sizeY);

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnUpdateScore(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExitBenchmark(WPARAM wParam, LPARAM lParam);
	
	afx_msg void OnWindowPosChanging(WINDOWPOS * lpwndpos); // for ShowWindow(SW_HIDE);

	afx_msg void OnExit();
	afx_msg void OnAbout();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()

public:
	afx_msg void OnEditCopy();
};
