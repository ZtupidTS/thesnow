/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#include "AboutDlg.h"

class CDiskMarkDlg : public CDHtmlMainDialog
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
	double m_RandomRead4KB32QDScore;
	double m_RandomWrite4KB32QDScore;

	void SetMeter(CString ElementName, double score);
	void UpdateMessage(CString elementName, CString message);
	void ChangeLang(CString LangName);
	void ChangeButtonStatus(BOOL status);
	void ChangeButton(CString elementName, CString className, CString title, CString html);
	void ChangeSelectStatus(CString elementName, VARIANT_BOOL status);
	void ChangeSelectTitle(CString elementName, CString title);

	CString m_CurrentLocalID;
	CString m_ValueTestDrive;
	CString m_ValueTestNumber;
	CString m_ValueTestSize;
	CString m_TestDriveInfo;
	long m_IndexTestDrive;
	long m_IndexTestNumber;
	long m_IndexTestSize;

	TCHAR m_ini[MAX_PATH];
	
	DWORD m_TestData;

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
	HRESULT OnRandom4KB32QD(IHTMLElement *pElement);

	void Stop();

	CString m_TitleTestDrive;
	CString m_TitleTestNumber;
	CString m_TitleTestSize;
	CString m_TitleTestQSize;

	CString m_ExeDir;


	CString m_Comment;

protected:
	HICON m_hIcon;
	HICON m_hIconMini;
	HACCEL m_hAccelerator;

	int m_SizeX;
	int m_SizeY;

	CAboutDlg*		m_AboutDlg;

	void InitDrive(CString ElementName);

	BOOL CheckRadioZoomType(int id, int value);
	void CheckRadioZoomType();
	void ChangeZoom();
	void ReExecute();

	void EnableMenus();
	void DisableMenus();

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
	afx_msg void OnZoom100();
	afx_msg void OnZoom125();
	afx_msg void OnZoom150();
	afx_msg void OnZoom200();
	afx_msg void OnZoom300();
	afx_msg void OnZoom400();
	afx_msg void OnZoomAuto();
	
	afx_msg void OnExit();
	afx_msg void OnAbout();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()

public:
	afx_msg void OnEditCopy();
	afx_msg void OnHelp();
	afx_msg void OnCrystalDewWorld();
	afx_msg void OnModeDefault();
	afx_msg void OnModeAll0x00();
	afx_msg void OnModeAll0xFF();
};
