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

#include "stdhdrs.h"
#include "vncviewer.h"

#include "FileTransfer.h"
#include "DirManager.h"
#include "FTEditBox.h"

const char FileTransferDialog::myComputerText[] = "My Computer";
const char FileTransferDialog::myDocumentsText[] = "My Documents";
const char FileTransferDialog::myPicturesText[] = "My Pictures";
const char FileTransferDialog::myMusicText[] = "My Music";
const char FileTransferDialog::myDesktopText[] = "Desktop";
const char FileTransferDialog::myNetHood[] = "My Network Places";

FileTransferDialog::FileTransferDialog(HINSTANCE hInst, FileTransfer *pFT)
{
	m_pFileTransfer = pFT;
	m_hInstance = hInst;
	m_pLocalLV = NULL;
	m_pRemoteLV = NULL;
	m_pLocalEB = NULL;
	m_pRemoteEB = NULL;
	m_pToolBar = NULL;
	m_pStatusBox = NULL;
	m_pProgress = NULL;
	m_bFirstStart = true;
	m_bDlgShown = false;
	m_bEndFTDlgOnYes = false;
	m_szLocalPath[0] = '\0';
	m_szRemotePath[0] = '\0';
	m_szLocalPathTmp[0] = '\0';
	m_szRemotePathTmp[0] = '\0';
	m_szCreateDirName[0] = '\0';
	m_szRenameDlgText1[0] = '\0';
	m_szRenameDlgText2[0] = '\0';
	m_szRenameDlgText3[0] = '\0';
	m_dwCheckedLV = 0;
	m_nextLocalPathCtrlID = -1;
	m_nextRemotePathCtrlID = -1;
	m_hwndFTCanceling = NULL;
}

FileTransferDialog::~FileTransferDialog()
{
	closeFileTransferDialog();
}

bool
FileTransferDialog::createFileTransferDialog()
{
	m_hwndFileTransfer = CreateDialogParam(m_hInstance, 
										   MAKEINTRESOURCE(IDD_FILETRANSFER_DLG),
										   NULL, 
										   (DLGPROC) fileTransferDlgProc,
										   (LONG) this);
	
	if (m_hwndFileTransfer == NULL) return false;

	m_pLocalLV = new FTListView(GetDlgItem(m_hwndFileTransfer, IDC_FTCLIENTLIST));
	m_pRemoteLV = new FTListView(GetDlgItem(m_hwndFileTransfer, IDC_FTSERVERLIST));

	m_pLocalEB = new FTEditBox(GetDlgItem(m_hwndFileTransfer, IDC_CLIENTPATH));
	m_pRemoteEB = new FTEditBox(GetDlgItem(m_hwndFileTransfer, IDC_SERVERPATH));

	m_pToolBar = new FTToolBar(m_hInstance, m_hwndFileTransfer);
	m_pStatusBox = new FTStatusBox(GetDlgItem(m_hwndFileTransfer, IDC_FTSTATUS));
	m_pProgress = new ProgressControls(m_hwndFileTransfer);

	if ((m_pLocalLV == NULL) || (m_pRemoteLV == NULL) ||
		(m_pLocalEB == NULL) || (m_pRemoteEB == NULL) || 
		(m_pToolBar == NULL) || (m_pStatusBox == NULL)) {
		closeFileTransferDialog();
		return false;
	}

	addAllLVColumns();

	m_pLocalLV->setExtendedLVStyle(LVS_EX_FULLROWSELECT);
	m_pRemoteLV->setExtendedLVStyle(LVS_EX_FULLROWSELECT);

	showComputerNames();

	initFTIcons();
	initPathControls();

	m_pToolBar->createToolBar();
	m_pToolBar->initImageList();
	m_pToolBar->addButtons();

	ShowWindow(m_hwndFileTransfer, SW_SHOW);
	UpdateWindow(m_hwndFileTransfer);
	m_bDlgShown = true;

	if (m_bFirstStart) {
		DirManager dm;
		char path[MAX_PATH];
		if (dm.getSpecFolderPath(path, rfbSpecDirMyDocuments)) {
			strcpy(m_szLocalPathTmp, path);
		}
		m_nextLocalPathCtrlID = FT_ID_MYDOCUMENTS;
		m_nextRemotePathCtrlID = FT_ID_MYDOCUMENTS;
		showLocalFiles(m_szLocalPathTmp);
		m_pFileTransfer->sendFileSpecDirRqstMsg(rfbSpecDirMyDocuments);
	} else {
		showLocalFiles(m_szLocalPath);
		m_pFileTransfer->sendFileListRqstMsg(strlen(m_szRemotePath), m_szRemotePath, FT_FLR_DEST_MAIN, 0);
	}

	m_bFirstStart = false;
	return true;
}

void 
FileTransferDialog::processDlgMessage(HWND hwnd)
{
	if (hwnd == NULL) hwnd = m_hwndFileTransfer;

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool 
FileTransferDialog::addAllLVColumns()
{
	RECT Rect;
	GetClientRect(m_pLocalLV->getWndHandle(), &Rect);
	Rect.right -= GetSystemMetrics(SM_CXHSCROLL);
	int xwidth0 = (int) (0.35 * Rect.right);
	int xwidth1 = (int) (0.22 * Rect.right);
	int xwidth2 = (int) (0.43 * Rect.right);

	m_pLocalLV->addColumn("Name", 0, xwidth0, LVCFMT_LEFT);
	m_pLocalLV->addColumn("Size", 1, xwidth1, LVCFMT_RIGHT);
	m_pLocalLV->addColumn("Data", 2, xwidth2, LVCFMT_LEFT);

	GetClientRect(m_pRemoteLV->getWndHandle(), &Rect);
	Rect.right -= GetSystemMetrics(SM_CXHSCROLL);
	xwidth0 = (int) (0.35 * Rect.right);
	xwidth1 = (int) (0.22 * Rect.right);
	xwidth2 = (int) (0.43 * Rect.right);
	
	m_pRemoteLV->addColumn("Name", 0, xwidth0, LVCFMT_LEFT);
	m_pRemoteLV->addColumn("Size", 1, xwidth1, LVCFMT_RIGHT);
	m_pRemoteLV->addColumn("Data", 2, xwidth2, LVCFMT_LEFT);

	return true;
}

void
FileTransferDialog::closeFileTransferDialog()
{
	if (m_pFileTransfer->isTransferEnable()) {
		m_bEndFTDlgOnYes = true;
		if (m_hwndFTCanceling == NULL) createCancelingDlg();
		return;
	}
	endFileTransferDialog();
}

void
FileTransferDialog::endFileTransferDialog()
{
	m_pFileTransfer->setFTDlgStatus(false);

	if (m_pLocalLV != NULL) {
		m_pLocalLV->deleteAllItems();
		delete m_pLocalLV;
		m_pLocalLV = NULL;
	}
	if (m_pRemoteLV != NULL) {
		m_pRemoteLV->deleteAllItems();
		delete m_pRemoteLV;
		m_pRemoteLV = NULL;
	}
	if (m_pLocalEB != NULL) {
		delete m_pLocalEB;
		m_pLocalEB = NULL;
	}
	if (m_pRemoteEB != NULL) {
		delete m_pRemoteEB;
		m_pRemoteEB = NULL;
	}
	if (m_pToolBar != NULL) {
		delete m_pToolBar;
		m_pToolBar = NULL;
	}
	if (m_pStatusBox != NULL) {
		delete m_pStatusBox;
		m_pStatusBox = NULL;
	}
	if (m_pProgress != NULL) {
		delete m_pProgress;
		m_pProgress = NULL;
	}
	
	EndDialog(m_hwndFileTransfer, TRUE);
	m_bDlgShown = false;
	m_bEndFTDlgOnYes = false;
}

BOOL CALLBACK 
FileTransferDialog::fileTransferDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransferDialog *_this = (FileTransferDialog *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, (LONG) lParam);
		SetForegroundWindow(hwnd);
		CentreWindow(hwnd);
		return TRUE;
	case WM_HELP:	
		help.Popup(lParam);
		return FALSE;
	case WM_COMMAND:
		{
		switch (LOWORD(wParam))
		{
			case IDC_CLIENTPATH:
				switch (HIWORD (wParam))
				{
					case CBN_SETFOCUS:
						_this->checkFTDialog();
						return FALSE;
					case CBN_SELENDOK:
						_this->checkFTDialog();
						_this->onLocalPathItemChange();
						return FALSE;
				}
			break;
			case IDC_SERVERPATH:
				switch (HIWORD (wParam))
				{
					case CBN_SETFOCUS:
						_this->checkFTDialog();
						return FALSE;
					case CBN_SELCHANGE:
						_this->checkFTDialog();
						_this->onRemotePathItemChange();
						return FALSE;
				}
			break;
			case IDC_EXIT:
				_this->closeFileTransferDialog();
				return FALSE;
			case IDC_CLIENTUP:
				_this->checkFTDialog();
				_this->m_nextLocalPathCtrlID = -1;
				_this->localOneUpFolder(_this->m_szLocalPathTmp);
				return FALSE;
			case IDC_SERVERUP:
				_this->checkFTDialog();
				_this->m_nextRemotePathCtrlID = -1;
				_this->remoteOneUpFolder(_this->m_szRemotePathTmp);
				return FALSE;
			case IDC_CLIENTRELOAD:
				_this->checkFTDialog();
				_this->reloadLocalFileList();
				return FALSE;
			case IDC_SERVERRELOAD:
				_this->checkFTDialog();
				_this->reloadRemoteFileList();
				return FALSE;
			case IDC_FTCOPY:
				_this->onFTCopy();
				return FALSE;
			case IDC_FTCANCEL:
				_this->onFTCancel();
				return FALSE;
			case IDC_CLIENTBROWSE_BUT:
				return FALSE;
			case IDC_SERVERBROWSE_BUT:
				return FALSE;
			case IDC_CREATEFLD:
				_this->onFTCreateFolder();
				return FALSE;
			case IDC_FTDELETE:
				_this->onFTDelete();
				return FALSE;
			case IDC_FTRENAME:
				_this->onFTRename();
				return FALSE;
		}
		}
	break;

	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_FTCLIENTLIST:
			switch (((LPNMHDR) lParam)->code)
			{
				case NM_CLICK:
				case NM_RCLICK:
					_this->checkFTDialog();
					return FALSE;
				case NM_SETFOCUS:
					_this->checkFTDialog();
					return FALSE;
				case LVN_ITEMCHANGED:
					_this->checkFTDialog();
					return FALSE;
				case LVN_GETDISPINFO:
					_this->m_pLocalLV->onGetDispInfo((NMLVDISPINFO *) lParam);
					return FALSE;
				case LVN_ITEMACTIVATE:
					_this->m_nextLocalPathCtrlID = -1;
					_this->onLocalItemActivate((LPNMITEMACTIVATE) lParam);
					return FALSE;
			}
		break;
		case IDC_FTSERVERLIST:
			switch (((LPNMHDR) lParam)->code)
			{
				case NM_CLICK:
				case NM_RCLICK:
					_this->checkFTDialog();
					return FALSE;
				case NM_SETFOCUS:
					_this->checkFTDialog();
					return FALSE;
				case LVN_ITEMCHANGED:
					_this->checkFTDialog();
					return FALSE;
				case LVN_GETDISPINFO:
					_this->m_pRemoteLV->onGetDispInfo((NMLVDISPINFO *) lParam);
					return FALSE;
				case LVN_ITEMACTIVATE:
					_this->m_nextRemotePathCtrlID = -1;
					_this->onRemoteItemActivate((LPNMITEMACTIVATE) lParam);
					return FALSE;
			}
		break;
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		_this->closeFileTransferDialog();
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK 
FileTransferDialog::FTCreateDirDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransferDialog *_this = (FileTransferDialog *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		SetFocus(GetDlgItem(hwnd, IDC_EDITDIRNAME));
		return FALSE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				GetWindowText(GetDlgItem(hwnd, IDC_EDITDIRNAME), _this->m_szCreateDirName, MAX_PATH);
				EndDialog(hwnd, TRUE);
				return FALSE;
			case IDCANCEL:
				_this->m_szCreateDirName[0] = '\0';
				EndDialog(hwnd, FALSE);
				return FALSE;
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK 
FileTransferDialog::FTRenameDirDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransferDialog *_this = (FileTransferDialog *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		SetWindowText(hwnd, (LPCTSTR)((FileTransferDialog *)lParam)->m_szRenameDlgText1);
		SetDlgItemText(hwnd, IDC_MAINTEXT, (LPCTSTR)((FileTransferDialog *)lParam)->m_szRenameDlgText2);
		SetDlgItemText(hwnd, IDC_EDITDIRNAME, (LPCTSTR)((FileTransferDialog *)lParam)->m_szRenameDlgText3);
		SendMessage(GetDlgItem(hwnd, IDC_EDITDIRNAME), EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
		SetFocus(GetDlgItem(hwnd, IDC_EDITDIRNAME));
		CentreWindow(hwnd);
		((FileTransferDialog *)lParam)->m_szRenameDlgText1[0] = '\0';
		((FileTransferDialog *)lParam)->m_szRenameDlgText2[0] = '\0';
		((FileTransferDialog *)lParam)->m_szRenameDlgText3[0] = '\0';
		return TRUE;
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				GetWindowText(GetDlgItem(hwnd, IDC_EDITDIRNAME), _this->m_szRenameDlgText3, MAX_PATH);
				EndDialog(hwnd, TRUE);
				return TRUE;
			case IDCANCEL:
				EndDialog(hwnd, FALSE);
				return TRUE;
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		return TRUE;
	}
	return FALSE;
}

void 
FileTransferDialog::onFTCopy()
{
	FileInfo fi;
	int len = 0;
	int numSel = -1;
	char prefix[MAX_PATH];
	prefix[0] = '\0';
	unsigned int flags = 0;
	if (m_pFileTransfer->isTransferEnable()) {
		strcpy(prefix, "File Transfer is active.\nDo you want to add selected file(s)/folder(s) to transfer queue?\n\n");
	}
	if (m_dwCheckedLV == 1) {
		numSel = m_pLocalLV->getSelectedItems(&fi);
		if (numSel > 0) {
			len = strlen(prefix);
			char *pBuf = new char [(numSel + 4) * (MAX_PATH + 3) + len + 1];
			sprintf(pBuf, "%sFrom: Local Computer %s\\\n\n", prefix, m_szLocalPath);
			for (unsigned int i = 0; i < fi.getNumEntries(); i++) {
				sprintf(pBuf, "%s%s, ", pBuf, fi.getNameAt(i));
			}
			sprintf(pBuf, "%s\n\nTo: Remote Computer %s\\", pBuf, m_szRemotePath);
			if (strlen(pBuf) > 2048) {
				sprintf(pBuf, "%sFrom: Local Computer %s\\\n\nTo: Remote Computer %s\\\n\nTotal %d file(s)/folder(s)",
						prefix, m_szLocalPath, m_szRemotePath, numSel);
			}
			if (MessageBox(m_hwndFileTransfer, pBuf, "Copy Selected Files and Folders", MB_OKCANCEL) == IDOK) {
				m_pFileTransfer->addTransferQueue(m_szLocalPath, m_szRemotePath, &fi, FT_ATTR_COPY_UPLOAD);
			}
			delete [] pBuf;
			return;
		}
	} else {
		if (m_dwCheckedLV == 2) {
			int numSel = m_pRemoteLV->getSelectedItems(&fi);
			if (numSel > 0) {
				len = strlen(prefix);
				char *pBuf = new char [(numSel + 4) * (MAX_PATH + 3) + len + 1];
				sprintf(pBuf, "%sFrom: Remote Computer %s\\\n\n", prefix, m_szRemotePath);
				for (unsigned int i = 0; i < fi.getNumEntries(); i++) {
					sprintf(pBuf, "%s%s, ", pBuf, fi.getNameAt(i));
				}
				sprintf(pBuf, "%s\n\nTo: Local Computer %s\\", pBuf, m_szLocalPath);
				if (strlen(pBuf) > 2048) {
					sprintf(pBuf, "%sFrom: Remote Computer %s\\\n\nTo: Local Computer %s\\\n\nTotal %d file(s)/folder(s)",
							prefix, m_szRemotePath, m_szLocalPath, numSel);
				}
				if (MessageBox(m_hwndFileTransfer, pBuf, "Copy Selected Files and Folders", MB_OKCANCEL) == IDOK) {
					m_pFileTransfer->addTransferQueue(m_szLocalPath, m_szRemotePath, &fi, FT_ATTR_COPY_DOWNLOAD);
				}
				delete [] pBuf;
				return;
			}
		}
	}
	m_pStatusBox->setText("File Transfer Impossible. No file(s) selected for transfer");
	m_pToolBar->setAllButtonsState(-1, 0, -1, -1, -1);
}

void 
FileTransferDialog::onFTCancel()
{
	m_bEndFTDlgOnYes = false;
	createCancelingDlg();
}

void 
FileTransferDialog::onFTRename()
{
	FileInfo fi;
	int numSel = 0;
	if (m_dwCheckedLV == 1) {
		numSel = m_pLocalLV->getSelectedItems(&fi);
		if (numSel == 1) {
			if (createRenameDirDlg(&fi)) {
				m_pFileTransfer->renameLocal(m_szLocalPath, m_szRenameDlgText3, fi.getFullDataAt(0));
			} else {
				setStatusText("Rename Operation Cancelled");
			}
		}
	} else {
		if (m_dwCheckedLV == 2) {
			numSel = m_pRemoteLV->getSelectedItems(&fi);
			if (numSel == 1) {
				if (createRenameDirDlg(&fi)) {
					m_pFileTransfer->renameRemote(m_szRemotePath, m_szRenameDlgText3, fi.getFullDataAt(0));
				} else {
					setStatusText("Rename Operation Cancelled");
				}
			}
		} else {
			return;
		}
	}
}

INT_PTR
FileTransferDialog::createConfirmDlg(FILEINFO *pFIStruct1, FILEINFO *pFIStruct2)
{
	char fileOrFolder[16];
	if (pFIStruct1->info.flags & FT_ATTR_FILE) {
		strcpy(fileOrFolder, "file");
	} else {
		if (pFIStruct1->info.flags & FT_ATTR_FOLDER) {
			strcpy(fileOrFolder, "folder");
		} else {
			return IDCANCEL;
		}
	}
	FILETIME ft;
	SYSTEMTIME st;
	Time70ToFiletime(pFIStruct1->info.data, &ft);
	FileTimeToSystemTime(&ft, &st);
	char fileData1[MAX_PATH + 36];
		if (st.wHour > 12) {
			sprintf(fileData1, "%s, modified on %d/%d/%d %d:%d PM", pFIStruct1->name, 
					st.wDay, st.wMonth, st.wYear, st.wHour - 12, st.wMinute);
		} else {
			sprintf(fileData1, "%s, modified on %d/%d/%d %d:%d AM", pFIStruct1->name, 
					st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
		}
	Time70ToFiletime(pFIStruct2->info.data, &ft);
	FileTimeToSystemTime(&ft, &st);
	char fileData2[MAX_PATH + 36];
	if (st.wHour > 12) {
		sprintf(fileData2, "%s, modified on %d/%d/%d %d:%d PM", pFIStruct2->name, 
				st.wDay, st.wMonth, st.wYear, st.wHour - 12, st.wMinute);
	} else {
		sprintf(fileData2, "%s, modified on %d/%d/%d %d:%d AM", pFIStruct2->name, 
				st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
	}
	char buf[4 * MAX_PATH];
	if (pFIStruct1->info.flags & FT_ATTR_FOLDER) {
		sprintf(buf, "This folder already exists.\n\nWould you like to replace the existing file\n\n%s,\n\nWith this one?\n\n%s\n",
				fileData1, fileData2);
		m_pConfirmDlgCaption = strdup("Confirm Folder Copy");
	} else {
		sprintf(buf, "This file already exists.\n\nWould you like to replace the existing file\n\n%s,\n\nWith this one?\n\n%s\n",
				fileData1, fileData2);
		m_pConfirmDlgCaption = strdup("Confirm File Copy");
	}

	m_pConfirmDlgText = strdup(buf);
	INT_PTR result = DialogBoxParam(m_hInstance, 
									MAKEINTRESOURCE(IDD_FTCONFIRM), 
									m_hwndFileTransfer, 
									(DLGPROC) confirmDlgProc, 
									(LONG) this);
	free(m_pConfirmDlgCaption);
	free(m_pConfirmDlgText);
	return result;
}

BOOL CALLBACK 
FileTransferDialog::confirmDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransferDialog *_this = (FileTransferDialog *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		SetWindowText(hwnd, (LPCTSTR)((FileTransferDialog *)lParam)->m_pConfirmDlgCaption);
		SetDlgItemText(hwnd, IDC_CONFIRM_TEXT, (LPCTSTR)((FileTransferDialog *)lParam)->m_pConfirmDlgText);
		CentreWindow(hwnd);
		return FALSE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_CONFIRM_YESTOALL:
				EndDialog(hwnd, IDC_CONFIRM_YESTOALL);
				UpdateWindow(_this->m_hwndFileTransfer);
				return TRUE;
/*
			case IDC_CONFIRM_RENAME:
				EndDialog(hwnd, IDC_CONFIRM_RENAME);
				return TRUE;
*/
			case IDOK:
				EndDialog(hwnd, IDOK);
				UpdateWindow(_this->m_hwndFileTransfer);
				return TRUE;
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				UpdateWindow(_this->m_hwndFileTransfer);
				return TRUE;
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, IDCANCEL);
		UpdateWindow(_this->m_hwndFileTransfer);
		return TRUE;
	}
	return FALSE;
}

bool 
FileTransferDialog::createCancelingDlg()
{
	m_hwndFTCanceling = CreateDialogParam(m_hInstance, 
										  MAKEINTRESOURCE(IDD_FTCANCELING),
										  NULL, 
										  (DLGPROC) cancelingDlgProc,
										  (LONG) this);
	ShowWindow(m_hwndFTCanceling, SW_SHOW);
	DrawIcon(GetDC(m_hwndFTCanceling), 15, 22, LoadIcon(NULL, IDI_QUESTION));
	UpdateWindow(m_hwndFTCanceling);
	return true;
}

BOOL CALLBACK 
FileTransferDialog::cancelingDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FileTransferDialog *_this = (FileTransferDialog *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) lParam);
			SetForegroundWindow(hwnd);
			CentreWindow(hwnd);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
				_this->endCancelingDlg(TRUE);
				return TRUE;
			case IDCANCEL:
				_this->endCancelingDlg(FALSE);
				return TRUE;
		}
	break;
	case WM_CLOSE:
	case WM_DESTROY:
		_this->endCancelingDlg(FALSE);
		return TRUE;
	}
	return FALSE;
}

void
FileTransferDialog::endCancelingDlg(BOOL result)
{
	if (m_hwndFTCanceling != NULL) {
		EndDialog(m_hwndFTCanceling, result);
		m_hwndFTCanceling = NULL;
		if (result) {
			m_pFileTransfer->m_bFTCancel = true;
			if (m_bEndFTDlgOnYes) endFileTransferDialog();
		}
	}
}

bool
FileTransferDialog::createRenameDirDlg(FileInfo *pFI)
{
	char text1[MAX_PATH];
	char text2[MAX_PATH];
	char name[MAX_PATH];
	strcpy(name, pFI->getNameAt(0));
	if (pFI->getFlagsAt(0) & FT_ATTR_FOLDER) {
		strcpy(text1, "Rename Folder");
		sprintf(text2, "Rename Folder '%s'", name);
	} else {
		strcpy(text1, "Rename File");
		sprintf(text2, "Rename File '%s'", name);
	}
	return createRenameDirDlg(name, text1, text2);
}

bool
FileTransferDialog::createRenameDirDlg(char *pName, char *pText1, char *pText2)
{
	strcpy(m_szRenameDlgText1, pText1);
	strcpy(m_szRenameDlgText2, pText2);
	strcpy(m_szRenameDlgText3, pName);
	if (DialogBoxParam(m_hInstance, 
					   MAKEINTRESOURCE(IDD_FTDIRNAME), 
		               m_hwndFileTransfer, 
					   (DLGPROC) FTRenameDirDlgProc, 
					   (LONG) this)) {
		strcpy(pName, m_szRenameDlgText3);
		return true;
	} else {
		return false;
	}
}

void 
FileTransferDialog::onFTDelete()
{
	FileInfo fi;
	int numSel = 0;
	if (m_dwCheckedLV == 1) {
		numSel = m_pLocalLV->getSelectedItems(&fi);
	} else {
		if (m_dwCheckedLV == 2) {
			numSel = m_pRemoteLV->getSelectedItems(&fi);
		} else {
			return;
		}
	}
	bool bDel = false;
	if (numSel == 1) {
		if (fi.getFlagsAt(0) & FT_ATTR_FOLDER) {
			char buf[MAX_PATH + 38];
			sprintf(buf, "Are you sure you want to delete '%s' and all its contents?", fi.getNameAt(0));
			if (MessageBox(m_hwndFileTransfer, 
				_T(buf),
				_T("Confirm Folder Delete"),
				MB_YESNO | MB_ICONQUESTION) == IDYES) {
				bDel = true;
			}
		} else {
			char buf[MAX_PATH + 38];
			sprintf(buf, "Are you sure you want to delete file '%s'?", fi.getNameAt(0));
			if (MessageBox(m_hwndFileTransfer, 
				_T(buf),
				_T("Confirm File Delete"),
				MB_YESNO | MB_ICONQUESTION) == IDYES) {
				bDel = true;
			}
		}
	} else {
		if (MessageBox(m_hwndFileTransfer, 
			_T("Are you sure you want to delete all selected files and folders?"),
			_T("Confirm Multiple Delete"),
			MB_YESNO | MB_ICONQUESTION) == IDYES) {
			bDel = true;
		}
	}
	if (bDel) {
		m_pToolBar->setAllButtonsState(-1, -1, -1, -1, 1);
		if (m_dwCheckedLV == 1) {
			m_pFileTransfer->deleteLocal(m_szLocalPath, &fi);
			reloadLocalFileList();
		} else {
			if (m_dwCheckedLV == 2) {
				m_pFileTransfer->deleteRemote(m_szRemotePath, &fi);
			} else {
				return;
			}
		}
	}
}

void 
FileTransferDialog::onFTCreateFolder()
{
	char dirName[MAX_PATH];
	if (!DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_FTDIRNAME), m_hwndFileTransfer, (DLGPROC) FTCreateDirDlgProc, (LONG) this)) {
		return;
	}
	if (m_dwCheckedLV == 1) {
		sprintf(dirName, "%s\\%s", m_szLocalPath, m_szCreateDirName);
		m_pStatusBox->setStatusText("Creating Local Folder. %s", dirName);
		CreateDirectory(dirName, NULL);
		showLocalFiles(m_szLocalPath);
	} else {
		if (m_dwCheckedLV == 2) {
			sprintf(dirName, "%s\\%s", m_szRemotePath, m_szCreateDirName);
			m_pStatusBox->setStatusText("Creating Remote Folder. %s", dirName);
			m_pFileTransfer->sendFileCreateDirRqstMsg(strlen(dirName), dirName);
			m_pFileTransfer->sendFileListRqstMsg(strlen(m_szRemotePath),m_szRemotePath,
												 FT_FLR_DEST_MAIN, 0);
		}
	}
}

void 
FileTransferDialog::onLocalPathItemChange()
{
	int curSel = SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
	DirManager dm;
	char path[MAX_PATH];
	path[0] = '\0';
	switch (curSel) 
	{ 
	case FT_ID_MYCOMPUTER:
		m_nextLocalPathCtrlID = FT_ID_MYCOMPUTER;
		strcpy(m_szLocalPathTmp, path);
		showLocalFiles(m_szLocalPathTmp);
		return;
	case FT_ID_MYDOCUMENTS:
		if (dm.getSpecFolderPath(path, rfbSpecDirMyDocuments)) {
			m_nextLocalPathCtrlID = FT_ID_MYDOCUMENTS;
			if (showLocalFiles(path)) {
				strcpy(m_szLocalPath, path);
				return;
			}
		}
		break;
	case FT_ID_MYPICTURES:
		if (dm.getSpecFolderPath(path, rfbSpecDirMyPictures)) {
			m_nextLocalPathCtrlID = FT_ID_MYPICTURES;
			if (showLocalFiles(path)) {
				strcpy(m_szLocalPath, path);
				return;
			}
		}
		break;
	case FT_ID_MYMUSIC:
		if (dm.getSpecFolderPath(path, rfbSpecDirMyMusic)) {
			m_nextLocalPathCtrlID = FT_ID_MYMUSIC;
			if (showLocalFiles(path)) {
				strcpy(m_szLocalPath, path);
				return;
			}
		}
		break;
	case FT_ID_MYDESKTOP:
		if (dm.getSpecFolderPath(path, rfbSpecDirDesktop)) {
			m_nextLocalPathCtrlID = FT_ID_MYDESKTOP;
			if (showLocalFiles(path)) {
				strcpy(m_szLocalPath, path);
				return;
			}
		}
		break;
	case FT_ID_MYNETHOOD:
		if (dm.getSpecFolderPath(path, rfbSpecDirMyNetHood)) {
			m_nextLocalPathCtrlID = FT_ID_MYNETHOOD;
			if (showLocalFiles(path)) {
				strcpy(m_szLocalPath, path);
				return;
			}
		}
		break;
	}
}

void 
FileTransferDialog::onRemotePathItemChange()
{
	int curSel = SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
	switch (curSel) 
	{ 
	case FT_ID_MYCOMPUTER:
		m_nextRemotePathCtrlID = FT_ID_MYCOMPUTER;
		showRemoteFiles("");
		break;
	case FT_ID_MYDOCUMENTS:
		m_nextRemotePathCtrlID = FT_ID_MYDOCUMENTS;
		m_pFileTransfer->sendFileSpecDirRqstMsg(rfbSpecDirMyDocuments);
		break;
	case FT_ID_MYPICTURES:
		m_nextRemotePathCtrlID = FT_ID_MYPICTURES;
		m_pFileTransfer->sendFileSpecDirRqstMsg(rfbSpecDirMyDocuments);
		break;
	case FT_ID_MYMUSIC:
		m_nextRemotePathCtrlID = FT_ID_MYMUSIC;
		m_pFileTransfer->sendFileSpecDirRqstMsg(rfbSpecDirMyMusic);
		break;
	case FT_ID_MYDESKTOP:
		m_nextRemotePathCtrlID = FT_ID_MYDESKTOP;
		m_pFileTransfer->sendFileSpecDirRqstMsg(rfbSpecDirDesktop);
		break;
	case FT_ID_MYNETHOOD:
		m_nextRemotePathCtrlID = FT_ID_MYNETHOOD;
		m_pFileTransfer->sendFileSpecDirRqstMsg(rfbSpecDirMyNetHood);
		break;
	}
}

void
FileTransferDialog::checkFTDialog()
{
	if (!m_bDlgShown) return;

	int locPathLen = strlen(m_szLocalPath);
	int remPathLen = strlen(m_szRemotePath);

	if (GetFocus() == m_pLocalLV->getWndHandle()) {
		m_dwCheckedLV = 1;
		if (strlen(m_szLocalPath) != 0) {
			int nCount = ListView_GetSelectedCount(m_pLocalLV->getWndHandle());
			if (nCount <= 0) {
				m_pToolBar->setAllButtonsState(1, 0, 0, 0, -1);
			} else {
				if (remPathLen == 0) {
					m_pToolBar->setAllButtonsState(1, 0, 0, 1, -1);
				} else {
					m_pToolBar->setAllButtonsState(1, 1, 0, 1, -1);
				}
				if (nCount == 1) m_pToolBar->setAllButtonsState(-1, -1, 1, -1, -1);
			}
		} else {
			m_pToolBar->setAllButtonsState(0, 0, 0, 0, -1);
		}
	} else {
		if (GetFocus() == m_pRemoteLV->getWndHandle()) {
			m_dwCheckedLV = 2;
			if (strlen(m_szRemotePath) != 0) {
				int nCount = ListView_GetSelectedCount(m_pRemoteLV->getWndHandle());
				if (nCount <= 0) {
					m_pToolBar->setAllButtonsState(2, 0, 0, 0, -1);
				} else {
					if (locPathLen == 0) {
						m_pToolBar->setAllButtonsState(2, 0, 0, 1, -1);
					} else {
						m_pToolBar->setAllButtonsState(2, 2, 0, 1, -1);
					}
					if (nCount == 1) m_pToolBar->setAllButtonsState(-1, -1, 1, -1, -1);
				}
			} else {
				m_pToolBar->setAllButtonsState(0, 0, 0, 0, -1);
			}
		} else {
			m_pToolBar->setAllButtonsState(0, 0, 0, 0, -1);
		}
	}
	if (m_pFileTransfer->isTransferEnable()) {
		m_pToolBar->setAllButtonsState(-1, -1, -1, -1, 1);
	} else {
		m_pToolBar->setAllButtonsState(-1, -1, -1, -1, 0);
	}
}

void
FileTransferDialog::reloadLocalFileList()
{
	strcpy(m_szLocalPathTmp, m_szLocalPath);
	showLocalFiles(m_szLocalPath);
}

void
FileTransferDialog::reloadRemoteFileList()
{
	strcpy(m_szRemotePathTmp, m_szRemotePath);
	m_pFileTransfer->sendFileListRqstMsg(strlen(m_szRemotePath), m_szRemotePath, FT_FLR_DEST_MAIN, 0);
}

void 
FileTransferDialog::onLocalItemActivate(LPNMITEMACTIVATE lpnmia)
{
	if (strlen(m_szLocalPath) == 0) {
		strcpy(m_szLocalPathTmp, m_pLocalLV->getActivateItemName(lpnmia));
	} else {
		sprintf(m_szLocalPathTmp, "%s\\%s", m_szLocalPath, m_pLocalLV->getActivateItemName(lpnmia));
	}
	showLocalFiles(m_szLocalPathTmp);
}

void 
FileTransferDialog::onRemoteItemActivate(LPNMITEMACTIVATE lpnmia)
{
	if (strlen(m_szRemotePath) == 0) {
		strcpy(m_szRemotePathTmp, m_pRemoteLV->getActivateItemName(lpnmia));
	} else {
		sprintf(m_szRemotePathTmp, "%s\\%s", m_szRemotePath, m_pRemoteLV->getActivateItemName(lpnmia));
	}
	m_pFileTransfer->sendFileListRqstMsg(strlen(m_szRemotePathTmp), m_szRemotePathTmp, FT_FLR_DEST_MAIN, 0);
}

void
FileTransferDialog::addLocalLVItems(FileInfo *pFI)
{
	strcpy(m_szLocalPath, m_szLocalPathTmp);
	m_pLocalLV->deleteAllItems();
	pFI->sort();
	m_pLocalLV->addItems(pFI);
	m_pToolBar->setAllButtonsState(-1, 0, 0, 0, -1);
}

void 
FileTransferDialog::addRemoteLVItems(FileInfo *pFI)
{
	strcpy(m_szRemotePath, m_szRemotePathTmp);
	m_pRemoteLV->deleteAllItems();
	if (m_nextRemotePathCtrlID >= 0) {
		SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_SETCURSEL, (WPARAM) m_nextRemotePathCtrlID, (LPARAM) 0);
	} else {
		m_pRemoteEB->setText(m_szRemotePathTmp);
	}
	if (pFI != NULL) {
		pFI->sort();
		m_pRemoteLV->addItems(pFI);
	}
	m_pToolBar->setAllButtonsState(-1, 0, 0, 0, -1);
}

void 
FileTransferDialog::reqFolderUnavailable()
{
	strcpy(m_szRemotePathTmp, m_szRemotePath);
	m_pRemoteEB->setText(m_szRemotePath);
}

void 
FileTransferDialog::localOneUpFolder(char *pPath)
{
	makeOneUpFolder(pPath);
	showLocalFiles(pPath);
}

void 
FileTransferDialog::remoteOneUpFolder(char *pPath)
{
	int len = makeOneUpFolder(pPath);
	m_pFileTransfer->sendFileListRqstMsg(len, pPath, FT_FLR_DEST_MAIN, 0);
}

int
FileTransferDialog::makeOneUpFolder(char *pPath)
{
	if (strcmp(pPath, "") == 0) return strlen(pPath);
	for (int i=(strlen(pPath)-2); i>=0; i--) {
		if (pPath[i] == '\\') {
			pPath[i] = '\0';
			break;
		}
		if (i == 0) pPath[0] = '\0';
	}
	return strlen(pPath);
}

bool
FileTransferDialog::showLocalFiles(char *pPath)
{
	DirManager dm;
	FileInfo fi;
	if (dm.getFilesInfo(&fi, pPath, 0)) {
		if (m_nextLocalPathCtrlID >= 0) {
			SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_SETCURSEL, (WPARAM) m_nextLocalPathCtrlID, (LPARAM) 0);
		} else {
			m_pLocalEB->setText(m_szLocalPathTmp);
		}
		addLocalLVItems(&fi);
		return true;
	} else {
		strcpy(m_szLocalPathTmp, m_szLocalPath);
		m_pLocalEB->setText(m_szLocalPath);
		return false;
	}
}

void
FileTransferDialog::showRemoteFiles(char *pPath)
{
	strcpy(m_szRemotePathTmp, pPath);
	m_pFileTransfer->sendFileListRqstMsg(strlen(m_szRemotePathTmp), m_szRemotePathTmp, 
										 FT_FLR_DEST_MAIN, 0);
}

void 
FileTransferDialog::setStatusText(char *pText)
{
	m_pStatusBox->setText(pText);
}
void
FileTransferDialog::showComputerNames()
{
	char buf[MAX_HOST_NAME_LEN + 32];
	char localHostName[MAX_HOST_NAME_LEN + 1];
	if (gethostname(localHostName, MAX_HOST_NAME_LEN) != SOCKET_ERROR) {
		sprintf(buf, "Local Computer: %s", localHostName);
	} else {
		sprintf(buf, "Local Computer: - Unknown -");
	}
	SetDlgItemText(m_hwndFileTransfer, IDC_LOCAL_COMP_LABEL, buf);

	sprintf(buf, "Remote Computer: %s", m_pFileTransfer->getHostName());
	SetDlgItemText(m_hwndFileTransfer, IDC_TVNC_SERV_LABEL, buf);
}

void
FileTransferDialog::setIcon(int dest, int idIcon)
{
	HANDLE hIcon = LoadImage(m_hInstance, MAKEINTRESOURCE(idIcon), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(GetDlgItem(m_hwndFileTransfer, dest), BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);
	DestroyIcon((HICON) hIcon);
}

void
FileTransferDialog::initFTIcons()
{
	setIcon(IDC_CLIENTUP, IDI_FILEUP);
	setIcon(IDC_SERVERUP, IDI_FILEUP);
	setIcon(IDC_CLIENTRELOAD, IDI_FILERELOAD);
	setIcon(IDC_SERVERRELOAD, IDI_FILERELOAD);

	m_pLocalLV->initImageList(m_hInstance);
	m_pRemoteLV->initImageList(m_hInstance);
}

void
FileTransferDialog::initPathControls()
{
	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYCOMPUTER, (LPARAM) myComputerText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYDOCUMENTS, (LPARAM) myDocumentsText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYPICTURES, (LPARAM) myPicturesText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYMUSIC, (LPARAM) myMusicText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYDESKTOP, (LPARAM) myDesktopText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYNETHOOD, (LPARAM) myNetHood);

	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYCOMPUTER, (LPARAM) myComputerText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYDOCUMENTS, (LPARAM) myDocumentsText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYPICTURES, (LPARAM) myPicturesText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYMUSIC, (LPARAM) myMusicText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYDESKTOP, (LPARAM) myDesktopText);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_INSERTSTRING, (WPARAM) FT_ID_MYNETHOOD, (LPARAM) myNetHood);

	SendDlgItemMessage(m_hwndFileTransfer, IDC_CLIENTPATH, CB_LIMITTEXT, (WPARAM) MAX_PATH, (LPARAM) 0);
	SendDlgItemMessage(m_hwndFileTransfer, IDC_SERVERPATH, CB_LIMITTEXT, (WPARAM) MAX_PATH, (LPARAM) 0);

}