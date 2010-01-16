//  Copyright (C) 2003-2004 Dennis Syrovatsky. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

#ifndef _VNC_FILETRANSFERDIALOG_H__
#define _VNC_FILETRANSFERDIALOG_H__

#include "FileInfo.h"
#include "FTListView.h"
#include "FTEditBox.h"
#include "FileTransfer.h"
#include "FTToolBar.h"
#include "FTStatusBox.h"
#include "ProgressControls.h"

class FileTransfer;

class FileTransferDialog  
{
public:
	FileTransferDialog(HINSTANCE hInst, FileTransfer *pFT);
	~FileTransferDialog();

	bool createFileTransferDialog();
	void closeFileTransferDialog();
	void endFileTransferDialog();
	void processDlgMessage(HWND hwnd);
	
	static BOOL CALLBACK fileTransferDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK FTCreateDirDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK FTRenameDirDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK confirmDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK cancelingDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void addLocalLVItems(FileInfo *pFI);
	void addRemoteLVItems(FileInfo *pFI);

	void reqFolderUnavailable();
	void showRemoteFiles(char *pPath);
	void setStatusText(char *pText);

	void reloadLocalFileList();
	void reloadRemoteFileList();

	FTToolBar *m_pToolBar;
	FTStatusBox *m_pStatusBox;
	ProgressControls *m_pProgress;

	bool createRenameDirDlg(char *pName, char *pText1, char *pText2);
	INT_PTR createConfirmDlg(FILEINFO *pFIStruct1, FILEINFO *pFIStruct2);
	bool createCancelingDlg();
	void endCancelingDlg(BOOL result);
	bool m_bEndFTDlgOnYes;

	char *getLocalPath() { return m_szLocalPath; };
	char *getRemotePath() { return m_szRemotePath; };

private:
	FileTransfer *m_pFileTransfer;

	FTListView *m_pLocalLV;
	FTListView *m_pRemoteLV;

	FTEditBox *m_pLocalEB;
	FTEditBox *m_pRemoteEB;

	HWND m_hwndFileTransfer;
	HWND m_hwndFTCanceling;
	HINSTANCE m_hInstance;

	bool addAllLVColumns();

	bool createRenameDirDlg(FileInfo *pFI);

	void onFTCopy();
	void onFTCancel();
	void onFTRename();
	void onFTDelete();
	void onFTCreateFolder();
	void onLocalPathItemChange();
	void onRemotePathItemChange();

	void onLocalItemActivate(LPNMITEMACTIVATE lpnmia);
	void onRemoteItemActivate(LPNMITEMACTIVATE lpnmia);
	void showComputerNames();
	bool showLocalFiles(char *pPath);
	void initFTIcons();
	void initPathControls();
	void setIcon(int dest, int idIcon);
	void checkFTDialog();

	void localOneUpFolder(char *pPath);
	void remoteOneUpFolder(char *pPath);
	int makeOneUpFolder(char *pPath);

	DWORD m_dwCheckedLV;

	char m_szLocalPath[MAX_PATH];
	char m_szRemotePath[MAX_PATH];
	char m_szLocalPathTmp[MAX_PATH];
	char m_szRemotePathTmp[MAX_PATH];

	char m_szCreateDirName[MAX_PATH];

	char m_szRenameDlgText1[MAX_PATH];
	char m_szRenameDlgText2[MAX_PATH];
	char m_szRenameDlgText3[MAX_PATH];

	char *m_pConfirmDlgCaption;
	char *m_pConfirmDlgText;

	bool m_bFirstStart;
	bool m_bDlgShown;

	int m_nextLocalPathCtrlID;
	int m_nextRemotePathCtrlID;

	static const char myComputerText[];
	static const char myDocumentsText[];
	static const char myPicturesText[];
	static const char myMusicText[];
	static const char myDesktopText[];
	static const char myNetHood[];
};

#endif // FILETRANSFERDIALOG_H__
