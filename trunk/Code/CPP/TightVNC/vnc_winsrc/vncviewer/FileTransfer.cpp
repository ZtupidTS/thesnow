//  Copyright (C) 2003-2005 Dennis Syrovatsky. All Rights Reserved.
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

#include "ClientConnection.h"
#include "FileTransfer.h"
#include "DirManager.h"
#include "FileInfo.h"
#include "FileInfoEx.h"
#include "FileTransferDialog.h"

FileTransfer::FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp)
{
	m_pCC = pCC;
	m_pApp = pApp;
	m_bFTDlgStatus = false;
	m_bGettingTotalSize = false;
	m_bResizeNeeded = false;
	m_bSendWMCloseOnYes = false;
	setFTBoolean(false);
	m_pFileTransferDlg = new FileTransferDialog(m_pApp->m_instance, this);
	m_dwNumDelError = 0;
	m_dwNumRenError = 0;
	m_dwNumFTError = 0;
	m_dwDirSizeRqstNum = 0;
}

FileTransfer::~FileTransfer()
{
	if (m_pFileTransferDlg != NULL) {
		m_pFileTransferDlg->closeFileTransferDialog();
		delete m_pFileTransferDlg;
		m_pFileTransferDlg = NULL;
	}
	m_fileTransferInfoEx.free();
	m_fileListRequestQueue.free();
	m_fileDelInfoEx.free();
	m_fileRenInfoEx.free();
}

void 
FileTransfer::createFileTransfer()
{
	m_bFTDlgStatus = m_pFileTransferDlg->createFileTransferDialog();
}

void 
FileTransfer::setFTDlgStatus(bool status)
{
	m_bFTDlgStatus = status;
	m_pCC->m_fileTransferDialogShown = false;
}

void
FileTransfer::addTransferQueue(char *pLocalPath, char *pRemotePath, FileInfo *pFI, unsigned int attr)
{
	if ((m_bFTDlgStatus) && (!isTransferEnable())) m_pFileTransferDlg->setStatusText("Starting Copy Operation");

	m_dwDirSizeRqstNum = 0;
	m_dw64TotalSize = 0;
	m_bGettingTotalSize = true;
	m_bOverwriteAll = false;
	m_bOverwrite0 = false;

	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pToolBar->setAllButtonsState(-1, 0, 0, 0, 1);
	m_fileTransferInfoEx.add(pLocalPath, pRemotePath, pFI, attr);

	if (isTransferEnable()) {
		m_bResizeNeeded = true;
	} else {
		m_bFileTransfer = true;
		resizeTotalSize64();
	}
}

void
FileTransfer::resizeTotalSize64()
{
	if (m_bFTCancel) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->setStatusText("Copy Operation Cancelled");
		closeUndoneTransfer();
		return;
	}

	for (unsigned int i = m_dwDirSizeRqstNum; i < m_fileTransferInfoEx.getNumEntries(); i++) {
		unsigned int flags = m_fileTransferInfoEx.getFlagsAt(i);
		if (flags & FT_ATTR_FILE) {
			m_dw64TotalSize += m_fileTransferInfoEx.getSizeAt(i);
		} else {
			if (flags & FT_ATTR_FOLDER) {
				if (flags & FT_ATTR_COPY_DOWNLOAD) {
					char *pPath = m_fileTransferInfoEx.getFullRemPathAt(i);
					m_dwDirSizeRqstNum = i;
					sendFileDirSizeRqstMsg(strlen(pPath), pPath, FT_FDSR_DEST_TRANSFER);
					return;
				} else {
					if (flags & FT_ATTR_COPY_UPLOAD) {
						DirManager dm;
						DWORD64 dw64Size;
						dm.getDirSize(m_fileTransferInfoEx.getFullLocPathAt(i), &dw64Size);
						m_dw64TotalSize += dw64Size;
					}
				}
			}
		}
	}
	m_dwDirSizeRqstNum = m_fileTransferInfoEx.getNumEntries();
	m_bGettingTotalSize = false;
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->initLeftControl(m_dw64TotalSize, 0);
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->initRightControl(0, 0);
	PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
}

void
FileTransfer::uploadFile()
{
	char *pPath = m_fileTransferInfoEx.getFullRemPathAt(0);

	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->makeStatusText("Upload Started:", 
																		 m_fileTransferInfoEx.getLocPathAt(0),
																		 m_fileTransferInfoEx.getRemPathAt(0),
																		 m_fileTransferInfoEx.getLocNameAt(0),
																		 m_fileTransferInfoEx.getRemNameAt(0));

	if (!m_fileReader.open(m_fileTransferInfoEx.getFullLocPathAt(0))) {
		if (m_bFTDlgStatus) setErrorString("Upload Failed", m_fileReader.getLastError());
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->increase(m_fileTransferInfoEx.getSizeAt(0));
		m_fileTransferInfoEx.deleteAt(0);
		PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
		return;
	}
	m_bUpload = true;
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->initRightControl(m_fileTransferInfoEx.getSizeAt(0), 0);
	sendFileUploadRqstMsg(strlen(pPath), pPath, 0);
	PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_UPLOADFILEPORTION, (WPARAM) 0, (LPARAM) 0);
}

void
FileTransfer::uploadFilePortion()
{
	if (!m_bUpload) return;

	DWORD dwNumBytesRead = 0;
	DWORD dwNumBytesToRead = 8192;
	char pBuf[8192];
	if (m_bFTDlgStatus) m_pFileTransferDlg->processDlgMessage(NULL);
	if (!m_bFTCancel) {
		if (m_fileReader.readBlock(dwNumBytesToRead, pBuf, &dwNumBytesRead)) {
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->increase(dwNumBytesRead);
			if (dwNumBytesRead == 0) {
				unsigned int mTime;
				m_fileReader.getTime(&mTime);
				sendFileUploadDataMsg(mTime);
				m_fileReader.close();
				m_bUpload = false;
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Upload Completed");
				if ((m_bFTDlgStatus) && (strcmp(m_fileTransferInfoEx.getRemPathAt(0), m_pFileTransferDlg->getRemotePath()) == 0)) {
					m_pFileTransferDlg->reloadRemoteFileList();
				}
				m_fileTransferInfoEx.deleteAt(0);
				PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
				return;
			} else {
				sendFileUploadDataMsg((unsigned short)dwNumBytesRead, pBuf);
				PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_UPLOADFILEPORTION, (WPARAM) 0, (LPARAM) 0);
				return;
			}
		} else {
			if (m_bFTDlgStatus) setErrorString("Upload Failed", m_fileReader.getLastError());
			unsigned int *pSize = 0;
			m_fileReader.getSize(pSize);
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->increase(*pSize);
			m_fileReader.close();
			m_bUpload = false;
			m_fileTransferInfoEx.deleteAt(0);
			char reason[] = "Upload failed";
			sendFileUploadFailedMsg(strlen(reason), reason);
			PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
			return;
		}
	} else {
		m_bFTCancel = false;
		char reason[] = "File Transfer Operation Cancelled";
		sendFileUploadFailedMsg(strlen(reason), reason);
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText(reason);
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pToolBar->setAllButtonsState(-1, -1, -1, -1, 0);
		closeUndoneTransfer();
	}
}

void
FileTransfer::downloadFile()
{
	if (!m_bOverwriteAll) {
		FILEINFO fiStruct1;
		DirManager dm;
		FileInfo fi;
		dm.getFilesInfo(&fi, m_fileTransferInfoEx.getLocPathAt(0), 0);
		int num = isExistName(&fi, m_fileTransferInfoEx.getLocNameAt(0));
		if (num >= 0) {
			strcpy(fiStruct1.name, m_fileTransferInfoEx.getRemNameAt(0));
			fiStruct1.info.size = m_fileTransferInfoEx.getSizeAt(0);
			fiStruct1.info.data = m_fileTransferInfoEx.getDataAt(0);
			fiStruct1.info.flags = m_fileTransferInfoEx.getFlagsAt(0);
			switch (m_pFileTransferDlg->createConfirmDlg(fi.getFullDataAt(num), &fiStruct1))
			{
			case IDOK:
				break;
			case IDCANCEL:
				m_fileTransferInfoEx.deleteAt(0);
				PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
				return;
			case IDC_CONFIRM_YESTOALL:
				m_bOverwriteAll = true;
				break;
/*
			case IDC_CONFIRM_RENAME:
				MessageBox(NULL, "IDRENAME", NULL, MB_OK);
				break;
*/
			}
		}
	}
	if (!m_fileWriter.open(m_fileTransferInfoEx.getFullLocPathAt(0))) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->increase(m_fileTransferInfoEx.getSizeAt(0));
		if (m_bFTDlgStatus) setErrorString("Download Failed", m_fileWriter.getLastError());
		m_fileTransferInfoEx.deleteAt(0);
		PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
		return;
	}

	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->initRightControl(m_fileTransferInfoEx.getSizeAt(0), 0);
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->makeStatusText("Download Started:", 
																		 m_fileTransferInfoEx.getRemPathAt(0),
																		 m_fileTransferInfoEx.getLocPathAt(0),
																		 m_fileTransferInfoEx.getRemNameAt(0),
																		 m_fileTransferInfoEx.getLocNameAt(0));
	char *pPath = m_fileTransferInfoEx.getFullRemPathAt(0);
	sendFileDownloadRqstMsg(strlen(pPath), pPath, 0);
}

void
FileTransfer::onEndTransfer()
{
	m_dw64TotalSize = 0;
	setFTBoolean(false);
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pToolBar->setAllButtonsState(-1, -1, -1, -1, 0);
	m_pFileTransferDlg->m_bEndFTDlgOnYes = false;
	m_pFileTransferDlg->endCancelingDlg(FALSE);
}

void
FileTransfer::checkTransferQueue()
{
	if ((m_fileTransferInfoEx.getNumEntries() == 0) && (!m_bResizeNeeded)) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("File Transfer Operation Completed Successfully");
		if (m_bFTDlgStatus) m_pFileTransferDlg->reloadLocalFileList();
		if (m_bFTDlgStatus) m_pFileTransferDlg->reloadRemoteFileList();
		onEndTransfer();
		return;
	}
	
	m_bFileTransfer = true;
	if (m_bResizeNeeded) {
		m_bResizeNeeded = false;
		resizeTotalSize64();
		return;
	}

	bool bOverwrite = false;
	unsigned int flags = m_fileTransferInfoEx.getFlagsAt(0);

	if (flags & FT_ATTR_COPY_UPLOAD) {
		if (flags & FT_ATTR_FILE) {
			if (m_bOverwriteAll) {
				uploadFile();
				return;
			} else {
				char *pPath = m_fileTransferInfoEx.getRemPathAt(0);
				m_fileTransferInfoEx.setFlagsAt(0, (m_fileTransferInfoEx.getFlagsAt(0) | FT_ATTR_FLR_UPLOAD_CHECK));
				sendFileListRqstMsg(strlen(pPath), pPath, FT_FLR_DEST_UPLOAD, 0);
				return;
			}
		}
		if (flags & FT_ATTR_FOLDER) {
			char *pFullPath = m_fileTransferInfoEx.getFullRemPathAt(0);
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Creating Remote Folder. %s", pFullPath);
			sendFileCreateDirRqstMsg(strlen(pFullPath), pFullPath);
			char *pPath = m_fileTransferInfoEx.getRemPathAt(0);
			m_fileTransferInfoEx.setFlagsAt(0, (m_fileTransferInfoEx.getFlagsAt(0) | FT_ATTR_FLR_UPLOAD_CHECK));
			sendFileListRqstMsg(strlen(pPath), pPath, FT_FLR_DEST_UPLOAD, 0);
			return;
		}
	} else {
		if (flags & FT_ATTR_COPY_DOWNLOAD) {
			if (flags & FT_ATTR_FILE) {
				downloadFile();
				return;
			}
			if (flags & FT_ATTR_FOLDER) {
				DirManager dm;
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Creating Local Folder. %s", m_fileTransferInfoEx.getFullLocPathAt(0));
				if (!dm.createDirectory(m_fileTransferInfoEx.getFullLocPathAt(0))) {
					if (dm.getLastError() == ERROR_ALREADY_EXISTS) {
						if (!m_bOverwriteAll) {
							FILEINFO fiStruct1, fiStruct2;
							dm.getInfo(m_fileTransferInfoEx.getFullLocPathAt(0), &fiStruct1);
							strcpy(fiStruct2.name, m_fileTransferInfoEx.getRemNameAt(0));
							fiStruct2.info.size = m_fileTransferInfoEx.getSizeAt(0);
							fiStruct2.info.data = m_fileTransferInfoEx.getDataAt(0);
							fiStruct2.info.flags = m_fileTransferInfoEx.getFlagsAt(0);
							switch (m_pFileTransferDlg->createConfirmDlg(&fiStruct1, &fiStruct2))
							{
							case IDOK:
								bOverwrite = true;
								break;
							case IDCANCEL:
								m_fileTransferInfoEx.deleteAt(0);
								PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
								return;
							case IDC_CONFIRM_YESTOALL:
								m_bOverwriteAll = true;
								bOverwrite = true;
								break;
	/*
							case IDC_CONFIRM_RENAME:
								MessageBox(NULL, "IDRENAME", NULL, MB_OK);
								return;
	*/
							}
						} else {
							bOverwrite = true;
						}
					} else {
						if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->increase(m_fileTransferInfoEx.getSizeAt(0));
						setErrorString("Download Failed", dm.getLastError());
						m_fileTransferInfoEx.deleteAt(0);
						PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
						return;
					}
				}
				if ((m_bFTDlgStatus) && (strcmp(m_fileTransferInfoEx.getLocPathAt(0), m_pFileTransferDlg->getLocalPath()) == 0)) {
					m_pFileTransferDlg->reloadLocalFileList();
				}
				char *pPath = m_fileTransferInfoEx.getFullRemPathAt(0);
				m_fileTransferInfoEx.setFlagsAt(0, (m_fileTransferInfoEx.getFlagsAt(0) | FT_ATTR_FLR_DOWNLOAD_ADD));
				sendFileListRqstMsg(strlen(pPath), pPath, FT_FLR_DEST_DOWNLOAD, 0);
				return;
			}
		}
	}
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("File Transfer Operation Failed. Unknown data in the transfer queue");
	closeUndoneTransfer();
}

void
FileTransfer::closeUndoneTransfer()
{
	m_bFileTransfer = false;
	m_bUpload = false;
	m_bDownload = false;
	m_bFTCancel = false;

	m_fileWriter.close();
	m_fileReader.close();
	m_fileTransferInfoEx.free();
	m_fileDelInfoEx.free();
	m_fileRenInfoEx.free();
	m_fileLastRqstFailedMsgs.free();
	m_fileListRequestQueue.free();

	if (m_bSendWMCloseOnYes) PostMessage(m_pCC->m_hwnd1, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
}

void 
FileTransfer::deleteLocal(char *pPathPrefix, FileInfo *pFI)
{
	for (unsigned int i = 0; i < pFI->getNumEntries(); i++) {
		m_fileDelInfoEx.add(pPathPrefix, "", pFI->getNameAt(i), "", pFI->getSizeAt(i), 
							pFI->getDataAt(i), pFI->getFlagsAt(i) | FT_ATTR_DELETE_LOCAL);
	}
	checkDeleteQueue();
}

void 
FileTransfer::deleteRemote(char *pPathPrefix, FileInfo *pFI)
{
	for (unsigned int i = 0; i < pFI->getNumEntries(); i++) {
		m_fileDelInfoEx.add("", pPathPrefix, "", pFI->getNameAt(i), pFI->getSizeAt(i), 
							pFI->getDataAt(i), pFI->getFlagsAt(i) | FT_ATTR_DELETE_REMOTE);
	}
	checkDeleteQueue();
}

void
FileTransfer::checkDeleteQueue()
{
	if (m_fileDelInfoEx.getNumEntries() > 0) {
		if (m_fileDelInfoEx.getFlagsAt(0) & FT_ATTR_DELETE_LOCAL) {
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->makeStatusText("Delete Operation Executing:", 
																				 m_fileDelInfoEx.getLocPathAt(0), 
																				 m_fileDelInfoEx.getLocNameAt(0));
			DirManager dm;
			FILEINFO fiStruct;
			strcpy(fiStruct.name, m_fileDelInfoEx.getLocNameAt(0));
			fiStruct.info.data = m_fileDelInfoEx.getDataAt(0);
			fiStruct.info.size = m_fileDelInfoEx.getSizeAt(0);
			fiStruct.info.flags = m_fileDelInfoEx.getFlagsAt(0);
			if (!dm.deleteIt(m_fileDelInfoEx.getLocPathAt(0), &fiStruct)) {
				m_dwNumDelError++;
				setErrorString("Delete Operation Failed", dm.getLastError());
			} else {
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Delete Operation Completed");
			}
			m_fileDelInfoEx.deleteAt(0);
			if (m_bFTDlgStatus) m_pFileTransferDlg->reloadLocalFileList();
			PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKDELETEQUEUE, (WPARAM) 0, (LPARAM) 0);
		} else {
			if (m_fileDelInfoEx.getFlagsAt(0) & FT_ATTR_DELETE_REMOTE) {
				char *pName = m_fileDelInfoEx.getFullRemPathAt(0);
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->makeStatusText("Delete Operation Executing:",
																					 m_fileDelInfoEx.getRemPathAt(0),
																					 m_fileDelInfoEx.getRemNameAt(0));
				sendFileDeleteRqstMsg(strlen(pName), pName);
				sendFileListRqstMsg(strlen(m_fileDelInfoEx.getRemPathAt(0)), m_fileDelInfoEx.getRemPathAt(0), FT_FLR_DEST_DELETE, 0);
			}
		}
	} else {
		if (m_dwNumDelError != 0) {
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Delete Operation Completed. Number of error(s): %d", m_dwNumDelError);
			m_dwNumDelError = 0;
		} else {
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Delete Operation Completed Successfully");
		}
		if (!m_bFileTransfer) onEndTransfer();
	}
}

void
FileTransfer::renameLocal(char *pPath, char *pNewName, FILEINFO *pFIStruct)
{
	m_fileRenInfoEx.add(pPath, "", pFIStruct->name, pNewName, pFIStruct->info.size, 
						pFIStruct->info.data, pFIStruct->info.flags | FT_ATTR_RENAME_LOCAL);
	checkRenameQueue();
}

void
FileTransfer::renameRemote(char *pPath, char *pNewName, FILEINFO *pFIStruct)
{
	m_fileRenInfoEx.add("", pPath, pFIStruct->name, pNewName, pFIStruct->info.size, 
						pFIStruct->info.data, pFIStruct->info.flags | FT_ATTR_RENAME_REMOTE);
	checkRenameQueue();
}

void
FileTransfer::checkRenameQueue()
{
	if (m_fileRenInfoEx.getNumEntries() > 0) {
		if (m_fileRenInfoEx.getFlagsAt(0) & FT_ATTR_RENAME_LOCAL) {
			DirManager dm;
			bool bRename = false;
			if (dm.renameIt(m_fileRenInfoEx.getLocPathAt(0), 
							m_fileRenInfoEx.getLocNameAt(0), 
							m_fileRenInfoEx.getRemNameAt(0))) {
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Rename Operation Completed");
			} else {
				m_dwNumRenError++;
				setErrorString("Rename Operation Failed", dm.getLastError());
				if (dm.getLastError() == ERROR_ALREADY_EXISTS) {
					char text1[MAX_PATH] = "Rename: Name Already Exist";
					char text2[MAX_PATH] = "Enter New Name";
					if (m_pFileTransferDlg->createRenameDirDlg(m_fileRenInfoEx.getRemNameAt(0), text1, text2)) {
						bRename = true;
						m_dwNumRenError--;
					}
				}
			}
			if (!bRename) {
				m_fileRenInfoEx.deleteAt(0);
				if (m_bFTDlgStatus) m_pFileTransferDlg->reloadLocalFileList();
			}
			PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKRENAMEQUEUE, (WPARAM) 0, (LPARAM) 0);
		} else {
			if (m_fileRenInfoEx.getFlagsAt(0) & FT_ATTR_RENAME_REMOTE) {
				char oldName[MAX_PATH];
				char newName[MAX_PATH];
				sprintf(oldName, "%s\\%s", m_fileRenInfoEx.getRemPathAt(0), m_fileRenInfoEx.getLocNameAt(0));
				sprintf(newName, "%s\\%s", m_fileRenInfoEx.getRemPathAt(0), m_fileRenInfoEx.getRemNameAt(0));
				sendFileRenameRqstMsg(strlen(oldName), strlen(newName), oldName, newName);
				sendFileListRqstMsg(strlen(m_fileRenInfoEx.getRemPathAt(0)), m_fileRenInfoEx.getRemPathAt(0), FT_FLR_DEST_RENAME, 0);
			}
		}
	} else {
		if (m_dwNumRenError != 0) {
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Rename Operation Completed. Number of error(s): %d", m_dwNumRenError);
			m_dwNumRenError = 0;
		} else {
			if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Rename Operation Completed Successfully");
		}
		if (!m_bFileTransfer) onEndTransfer();
	}
}

bool 
FileTransfer::sendFileListRqstMsg(unsigned short dirNameSize, char *pDirName, 
								  int dest, unsigned char flags)
{
	m_fileListRequestQueue.add(pDirName, 0, 0, dest);

	char _pDirName[MAX_PATH];
	strcpy(_pDirName, pDirName);
	CARD16 _dirNameSize = convertToUnixPath(_pDirName);

	int msgLen = sz_rfbFileListRequestMsg + _dirNameSize;
	char *pAllFLRMsg = new char[msgLen];

	if (pAllFLRMsg == NULL) return false;

	rfbFileListRequestMsg *pFLR = (rfbFileListRequestMsg *) pAllFLRMsg;
	char *pFollow = &pAllFLRMsg[sz_rfbFileListRequestMsg];
	pFLR->type = rfbFileListRequest;
	pFLR->flags = flags;
	pFLR->dirNameSize = Swap16IfLE(_dirNameSize);
	memcpy(pFollow, _pDirName, _dirNameSize);
	m_pCC->WriteExact(pAllFLRMsg, msgLen);
	delete [] pAllFLRMsg;
	
	return true;
}

bool 
FileTransfer::sendFileSpecDirRqstMsg(unsigned short specFlags)
{
	rfbFileSpecDirRequestMsg FSDR;

	FSDR.type = rfbFileSpecDirRequest;
	FSDR.flags = 0;
	FSDR.specFlags = Swap16IfLE(specFlags);

	m_pCC->WriteExact((char *)&FSDR, sz_rfbFileSpecDirRequestMsg);
	
	return true;
}

bool 
FileTransfer::sendFileDownloadRqstMsg(unsigned short filenameLen, char *pFilename, 
									  unsigned int position)
{
	char _pFilename[MAX_PATH];
	strcpy(_pFilename, pFilename);
	CARD16 _filenameLen = convertToUnixPath(_pFilename);

	int msgLen = sz_rfbFileDownloadRequestMsg + _filenameLen;
	char *pAllFDRMsg = new char[msgLen];

	if (pAllFDRMsg == NULL) return false;

	rfbFileDownloadRequestMsg *pFDR = (rfbFileDownloadRequestMsg *) pAllFDRMsg;
	char *pFollow = &pAllFDRMsg[sz_rfbFileDownloadRequestMsg];
	pFDR->type = rfbFileDownloadRequest;
	pFDR->compressedLevel = 0;
	pFDR->fNameSize = Swap16IfLE(_filenameLen);
	pFDR->position = Swap32IfLE(position);
	memcpy(pFollow, _pFilename, _filenameLen);
	m_pCC->WriteExact(pAllFDRMsg, msgLen);
	delete [] pAllFDRMsg;
	
	return true;
}

bool 
FileTransfer::sendFileUploadRqstMsg(unsigned short filenameLen, char *pFilename, 
									unsigned int position)
{
	char _pFilename[MAX_PATH];
	strcpy(_pFilename, pFilename);
	CARD16 _filenameLen = convertToUnixPath(_pFilename);

	int msgLen = sz_rfbFileUploadRequestMsg + _filenameLen;
	char *pAllFURMsg = new char[msgLen];

	if (pAllFURMsg == NULL) return false;

	rfbFileUploadRequestMsg *pFUR = (rfbFileUploadRequestMsg *) pAllFURMsg;
	char *pFollow = &pAllFURMsg[sz_rfbFileUploadRequestMsg];
	pFUR->type = rfbFileUploadRequest;
	pFUR->compressedLevel = 0;
	pFUR->fNameSize = Swap16IfLE(_filenameLen);
	pFUR->position = Swap32IfLE(position);
	memcpy(pFollow, _pFilename, _filenameLen);
	m_pCC->WriteExact(pAllFURMsg, msgLen);
	delete [] pAllFURMsg;

	return true;
}

bool 
FileTransfer::sendFileUploadDataMsg(unsigned short dataSize, char *pData)
{
	int msgLen = sz_rfbFileUploadDataMsg + dataSize;
	char *pAllFUDMsg = new char[msgLen];

	if (pAllFUDMsg == NULL) return false;

	rfbFileUploadDataMsg *pFUD = (rfbFileUploadDataMsg *) pAllFUDMsg;
	char *pFollow = &pAllFUDMsg[sz_rfbFileUploadDataMsg];
	pFUD->type = rfbFileUploadData;
	pFUD->compressedLevel = 0;
	pFUD->compressedSize = Swap16IfLE(dataSize);
	pFUD->realSize = Swap16IfLE(dataSize);
	memcpy(pFollow, pData, dataSize);
	m_pCC->WriteExact(pAllFUDMsg, msgLen);
	delete [] pAllFUDMsg;

	return true;
}

bool 
FileTransfer::sendFileUploadDataMsg(unsigned int modTime)
{
	int msgLen = sz_rfbFileUploadDataMsg + sizeof(modTime);
	char *pAllFUDMsg = new char[msgLen];

	if (pAllFUDMsg == NULL) return false;

	modTime = Swap32IfLE(modTime);

	rfbFileUploadDataMsg *pFUD = (rfbFileUploadDataMsg *) pAllFUDMsg;
	char *pFollow = &pAllFUDMsg[sz_rfbFileUploadDataMsg];
	pFUD->type = rfbFileUploadData;
	pFUD->compressedLevel = 0;
	pFUD->compressedSize = Swap16IfLE(0);
	pFUD->realSize = Swap16IfLE(0);
	memcpy(pFollow, &modTime, sizeof(modTime));
	m_pCC->WriteExact(pAllFUDMsg, msgLen);
	delete [] pAllFUDMsg;

	return true;
}

bool 
FileTransfer::sendFileDownloadCancelMsg(unsigned short reasonLen, char *pReason)
{
	int msgLen = sz_rfbFileDownloadCancelMsg + reasonLen;
	char *pAllFDCMsg = new char[msgLen];

	if (pAllFDCMsg == NULL) return false;

	rfbFileDownloadCancelMsg *pFDC = (rfbFileDownloadCancelMsg *) pAllFDCMsg;
	char *pFollow = &pAllFDCMsg[sz_rfbFileDownloadCancelMsg];
	pFDC->type = rfbFileDownloadCancel;
	pFDC->reasonLen = Swap16IfLE(reasonLen);
	memcpy(pFollow, pReason, reasonLen);
	m_pCC->WriteExact(pAllFDCMsg, msgLen);
	delete [] pAllFDCMsg;

	return true;
}

bool 
FileTransfer::sendFileUploadFailedMsg(unsigned short reasonLen, char *pReason)
{
	int msgLen = sz_rfbFileUploadFailedMsg + reasonLen;
	char *pAllFUFMsg = new char[msgLen];

	if (pAllFUFMsg == NULL) return false;

	rfbFileUploadFailedMsg *pFUF = (rfbFileUploadFailedMsg *) pAllFUFMsg;
	char *pFollow = &pAllFUFMsg[sz_rfbFileUploadFailedMsg];
	pFUF->type = rfbFileUploadFailed;
	pFUF->reasonLen = Swap16IfLE(reasonLen);
	memcpy(pFollow, pReason, reasonLen);
	m_pCC->WriteExact(pAllFUFMsg, msgLen);
	delete [] pAllFUFMsg;

	return true;
}

bool 
FileTransfer::sendFileCreateDirRqstMsg(unsigned short dirNameLen, char *pDirName)
{
	char _pDirName[MAX_PATH];
	strcpy(_pDirName, pDirName);
	CARD16 _dirNameLen = convertToUnixPath(_pDirName);

	int msgLen = sz_rfbFileCreateDirRequestMsg + _dirNameLen;
	char *pAllFCDRMsg = new char[msgLen];

	if (pAllFCDRMsg == NULL) return false;

	rfbFileCreateDirRequestMsg *pFCDR = (rfbFileCreateDirRequestMsg *) pAllFCDRMsg;
	char *pFollow = &pAllFCDRMsg[sz_rfbFileCreateDirRequestMsg];
	pFCDR->type = rfbFileCreateDirRequest;
	pFCDR->dNameLen = Swap16IfLE(_dirNameLen);
	memcpy(pFollow, _pDirName, _dirNameLen);
	m_pCC->WriteExact(pAllFCDRMsg, msgLen);
	delete [] pAllFCDRMsg;

	return true;
}

bool 
FileTransfer::sendFileDirSizeRqstMsg(unsigned short dirNameLen, char *pDirName, int dest)
{
	char _pDirName[MAX_PATH];
	strcpy(_pDirName, pDirName);
	CARD16 _dirNameLen = convertToUnixPath(_pDirName);

	int msgLen = sz_rfbFileDirSizeRequestMsg + _dirNameLen;
	char *pAllFDSRMsg = new char[msgLen];

	if (pAllFDSRMsg == NULL) return false;

	rfbFileDirSizeRequestMsg *pFDSR = (rfbFileDirSizeRequestMsg *) pAllFDSRMsg;
	char *pFollow = &pAllFDSRMsg[sz_rfbFileDirSizeRequestMsg];
	pFDSR->type = rfbFileDirSizeRequest;
	pFDSR->dNameLen = Swap16IfLE(_dirNameLen);
	memcpy(pFollow, _pDirName, _dirNameLen);
	m_pCC->WriteExact(pAllFDSRMsg, msgLen);
	delete [] pAllFDSRMsg;

	return true;
}

bool 
FileTransfer::sendFileRenameRqstMsg(unsigned short oldNameLen, unsigned short newNameLen, 
									char *pOldName, char *pNewName)
{
	char _pOldName[MAX_PATH]; strcpy(_pOldName, pOldName);
	char _pNewName[MAX_PATH]; strcpy(_pNewName, pNewName);
	CARD16 _oldNameLen = convertToUnixPath(_pOldName);
	CARD16 _newNameLen = convertToUnixPath(_pNewName);

	int msgLen = sz_rfbFileRenameRequestMsg + _oldNameLen + _newNameLen;
	char *pAllFRRMessage = new char[msgLen];
	rfbFileRenameRequestMsg *pFRR = (rfbFileRenameRequestMsg *) pAllFRRMessage;
	char *pFollow1 = &pAllFRRMessage[sz_rfbFileRenameRequestMsg];
	char *pFollow2 = &pAllFRRMessage[sz_rfbFileRenameRequestMsg + _oldNameLen];
	pFRR->type = rfbFileRenameRequest;
	pFRR->oldNameLen = Swap16IfLE(_oldNameLen);
	pFRR->newNameLen = Swap16IfLE(_newNameLen);
	memcpy(pFollow1, _pOldName, _oldNameLen);
	memcpy(pFollow2, _pNewName, _newNameLen);
	m_pCC->WriteExact(pAllFRRMessage, msgLen);
	delete [] pAllFRRMessage;

	return true;
}

bool 
FileTransfer::sendFileDeleteRqstMsg(unsigned short nameLen, char *pName)
{
	char _pName[MAX_PATH];
	strcpy(_pName, pName);
	CARD16 _nameLen = convertToUnixPath(_pName);

	int msgLen = sz_rfbFileDeleteRequestMsg + _nameLen;
	char *pAllFDRMsg = new char[msgLen];

	if (pAllFDRMsg == NULL) return false;

	rfbFileDeleteRequestMsg *pFDR = (rfbFileDeleteRequestMsg *) pAllFDRMsg;
	char *pFollow = &pAllFDRMsg[sz_rfbFileDeleteRequestMsg];
	pFDR->type = rfbFileDeleteRequest;
	pFDR->nameLen = Swap16IfLE(_nameLen);
	memcpy(pFollow, _pName, _nameLen);
	m_pCC->WriteExact(pAllFDRMsg, msgLen);
	delete [] pAllFDRMsg;

	return true;
}

bool 
FileTransfer::createFileInfo(unsigned int numFiles, FileInfo *fi, 
							 SIZEDATAINFO *pSDInfo, char *pFilenames)
{
	int pos = 0;
	int size = 0;
	for (unsigned int i = 0; i < numFiles; i++) {
		size = Swap32IfLE(pSDInfo[i].size);
		if (size == -1) {
			fi->add((pFilenames + pos), size, Swap32IfLE(pSDInfo[i].data), FT_ATTR_FOLDER);
		} else {
			fi->add((pFilenames + pos), size, Swap32IfLE(pSDInfo[i].data), FT_ATTR_FILE);
		}
		pos += strlen(pFilenames + pos) + 1;
	}
	return true;
}

bool 
FileTransfer::createFileInfo(unsigned int numFiles, FileInfo *fi, 
							 SIZEDATAFLAGSINFO *pSDFInfo, char *pFilenames)
{
	int pos = 0;
	for (unsigned int i = 0; i < numFiles; i++) {
		fi->add((pFilenames + pos), Swap32IfLE(pSDFInfo[i].size), Swap32IfLE(pSDFInfo[i].data), Swap32IfLE(pSDFInfo[i].flags));
		pos += strlen(pFilenames + pos) + 1;
	}
	return true;
}

bool 
FileTransfer::procFileListDataMsg()
{
	rfbFileListDataMsg fld;
	m_pCC->ReadExact((char *) &fld, sz_rfbFileListDataMsg);
	
	fld.numFiles = Swap16IfLE(fld.numFiles);
	fld.dataSize = Swap16IfLE(fld.dataSize);
	fld.compressedSize = Swap16IfLE(fld.compressedSize);

	bool bReqFldUnavailable = false;
	FileInfo fi;

	if (fld.flags & 0x80) {
		bReqFldUnavailable = true;
	} else {
		if (fld.numFiles > 0) {
			char *pFilenames = new char[fld.compressedSize];
			if (fld.flags & 0x20) {
				SIZEDATAFLAGSINFO *pSDFI = new SIZEDATAFLAGSINFO[fld.numFiles];
				m_pCC->ReadExact((char *)pSDFI, (fld.numFiles * sizeof(SIZEDATAFLAGSINFO)));
				m_pCC->ReadExact(pFilenames, fld.compressedSize);
				createFileInfo(fld.numFiles, &fi, pSDFI, pFilenames);
				delete [] pSDFI;
			} else {
				SIZEDATAINFO *pSDI = new SIZEDATAINFO[fld.numFiles];
				m_pCC->ReadExact((char *)pSDI, (fld.numFiles * sizeof(SIZEDATAINFO)));
				m_pCC->ReadExact(pFilenames, fld.compressedSize);
				createFileInfo(fld.numFiles, &fi, pSDI, pFilenames);
				delete [] pSDI;
			}
			delete [] pFilenames;
		}
	}

	bool bResult;
	switch (m_fileListRequestQueue.getFlagsAt(0))
	{
	case FT_FLR_DEST_MAIN:
		if (!m_bFTDlgStatus) break;

		if (bReqFldUnavailable) {
			m_pFileTransferDlg->reqFolderUnavailable();
			bResult = true;
		} else {
			bResult = procFLRMain(fld.numFiles, &fi);
		}
		break;
	case FT_FLR_DEST_BROWSE:
		bResult = procFLRBrowse(fld.numFiles, &fi);
		break;
	case FT_FLR_DEST_UPLOAD:
		bResult = procFLRUpload(fld.numFiles, &fi);
		break;
	case FT_FLR_DEST_DOWNLOAD:
		bResult = procFLRDownload(fld.numFiles, &fi);
		break;
	case FT_FLR_DEST_DELETE:
		bResult = procFLRDelete(fld.numFiles, &fi);
		break;
	case FT_FLR_DEST_RENAME:
		bResult = procFLRRename(fld.numFiles, &fi);
		break;
	}
	m_fileListRequestQueue.deleteAt(0);
	return bResult;
}

bool
FileTransfer::procFLRMain(unsigned short numFiles, FileInfo *pFI)
{
	if (numFiles == 0) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->addRemoteLVItems(NULL);
		return true;
	} 
	if (numFiles > 0) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->addRemoteLVItems(pFI);
		return true;
	}
	return false;
}

bool
FileTransfer::procFLRBrowse(unsigned short numFiles, FileInfo *pFI)
{
	return false;
}

bool
FileTransfer::procFLRUpload(unsigned short numFiles, FileInfo *pFI)
{
	bool bOverwrite = false;

	if (m_bFTCancel) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->setStatusText("File Transfer Operation Cancelled");
		closeUndoneTransfer();
		return false;
	}

	unsigned int flags = m_fileTransferInfoEx.getFlagsAt(0);
	if (flags & FT_ATTR_FLR_UPLOAD_CHECK) {
		int num = isExistName(pFI, m_fileTransferInfoEx.getRemNameAt(0));
		if (num >= 0) { 
			if ((m_bFTDlgStatus) && (strcmp(m_fileTransferInfoEx.getRemPathAt(0), m_pFileTransferDlg->getRemotePath()) == 0)) {
				m_pFileTransferDlg->reloadRemoteFileList();
			}
			if ((flags & FT_ATTR_FILE) || 
				((flags & FT_ATTR_FOLDER) && (flags & FT_ATTR_FOLDER_EXISTS)) &&
				//(!(flags & FT_ATTR_COPY_OVERWRITE)) && 
				(!m_bOverwriteAll)) {
				FILEINFO fiStruct;
				fiStruct.info.data = m_fileTransferInfoEx.getDataAt(0);
				fiStruct.info.size = m_fileTransferInfoEx.getSizeAt(0);
				fiStruct.info.flags = m_fileTransferInfoEx.getFlagsAt(0);
				strcpy(fiStruct.name, m_fileTransferInfoEx.getLocNameAt(0));
				switch (m_pFileTransferDlg->createConfirmDlg(&fiStruct, pFI->getFullDataAt(num)))
				{
				case IDOK:
					bOverwrite = true;
					break;
				case IDCANCEL:
					m_fileTransferInfoEx.deleteAt(0);
					break;
				case IDC_CONFIRM_YESTOALL:
					m_bOverwriteAll = true;
					bOverwrite = true;
					break;
/*
				case IDC_CONFIRM_RENAME:
					MessageBox(NULL, "IDRENAME", NULL, MB_OK);
					break;
*/
				}
			} else {
				bOverwrite = true;
			}
		} else {
			if (flags & FT_ATTR_FOLDER) {
				m_fileTransferInfoEx.deleteAt(0);
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Create Remote Folder Failed.");
				bOverwrite = false;
			} else {
				bOverwrite = true;
			}
		}
	}
	if (bOverwrite) {
		DirManager dm;
		FileInfo fi;
		flags = m_fileTransferInfoEx.getFlagsAt(0);
		if (flags & FT_ATTR_FILE) {
			uploadFile();
			return true;
		} else {
			if (dm.getFilesInfo(&fi, m_fileTransferInfoEx.getFullLocPathAt(0), 0)) {
				m_fileTransferInfoEx.add(m_fileTransferInfoEx.getFullLocPathAt(0),
					m_fileTransferInfoEx.getFullRemPathAt(0),
					&fi, FT_ATTR_COPY_UPLOAD);// | FT_ATTR_COPY_OVERWRITE);
			}
		}
		m_fileTransferInfoEx.deleteAt(0);
	}
	PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
	return true;
}

bool
FileTransfer::procFLRDownload(unsigned short numFiles, FileInfo *pFI)
{
	if (m_bFTCancel) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->setStatusText("Copy Operation Cancelled");
		closeUndoneTransfer();
		return false;
	}
	
	unsigned int flags = m_fileTransferInfoEx.getFlagsAt(0);

	if (flags & FT_ATTR_FLR_DOWNLOAD_CHECK) {
	}

	if ((flags & FT_ATTR_FOLDER) && (flags & FT_ATTR_FLR_DOWNLOAD_ADD)) {
		for (unsigned int i = 0; i < pFI->getNumEntries(); i++) {
			char locPath[MAX_PATH]; strcpy(locPath, m_fileTransferInfoEx.getFullLocPathAt(0));
			char remPath[MAX_PATH]; strcpy(remPath, m_fileTransferInfoEx.getFullRemPathAt(0));
			m_fileTransferInfoEx.add(locPath, remPath, pFI->getNameAt(i), pFI->getNameAt(i),
									 pFI->getSizeAt(i), pFI->getDataAt(i), (pFI->getFlagsAt(i) | FT_ATTR_COPY_DOWNLOAD));
		}
		m_fileTransferInfoEx.deleteAt(0);
		PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
		return true;
	} else {
		closeUndoneTransfer();
		if (m_bFTDlgStatus) m_pFileTransferDlg->setStatusText("File Transfer Operation Failed: Unknown data from server.");
	}
	return false;
}

bool
FileTransfer::procFLRDelete(unsigned short numFiles, FileInfo *pFI)
{
	if (m_bFTCancel) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->setStatusText("Delete Operation Cancelled");
		closeUndoneTransfer();
		return false;
	}

	if (isExistName(pFI, m_fileDelInfoEx.getRemNameAt(0)) >= 0) {
		m_dwNumDelError++;
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Delete Operation Failed: %s", 
								m_fileLastRqstFailedMsgs.getNameAt(m_fileLastRqstFailedMsgs.getNumEntries() - 1));
		m_fileLastRqstFailedMsgs.deleteAt(m_fileLastRqstFailedMsgs.getNumEntries() - 1);
	} else {
		if (m_bFTDlgStatus) {
			m_pFileTransferDlg->m_pStatusBox->setStatusText("Delete Operation Completed");
			m_pFileTransferDlg->reloadRemoteFileList();
		}
	}
	m_fileDelInfoEx.deleteAt(0);
	checkDeleteQueue();
	return true;
}

bool
FileTransfer::procFLRRename(unsigned short numFiles, FileInfo *pFI)
{
	if (m_bFTCancel) {
		if (m_bFTDlgStatus) m_pFileTransferDlg->setStatusText("Rename Operation Cancelled");
		closeUndoneTransfer();
		return false;
	}

	bool bRename = false;
	if (m_fileRenInfoEx.getNumEntries() > 0) {
		if ((isExistName(pFI, m_fileRenInfoEx.getLocNameAt(0)) < 0) &&
			(isExistName(pFI, m_fileRenInfoEx.getRemNameAt(0)) >= 0) &&
			(m_bFTDlgStatus)) {
				m_pFileTransferDlg->m_pStatusBox->setStatusText("Rename Operation Completed");
				m_pFileTransferDlg->addRemoteLVItems(pFI);
		} else {
			if ((isExistName(pFI, m_fileRenInfoEx.getLocNameAt(0)) >= 0) &&
				(isExistName(pFI, m_fileRenInfoEx.getRemNameAt(0)) < 0)) {
				m_dwNumRenError++;
				if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Rename Operation Failed: %s", 
										m_fileLastRqstFailedMsgs.getNameAt(m_fileLastRqstFailedMsgs.getNumEntries() - 1));
			} else {
				if ((isExistName(pFI, m_fileRenInfoEx.getLocNameAt(0)) >= 0) &&
					(isExistName(pFI, m_fileRenInfoEx.getRemNameAt(0)) >= 0)) {
					char text1[MAX_PATH] = "Rename: Name Already Exist";
					char text2[MAX_PATH] = "Enter New Name";
					if (m_pFileTransferDlg->createRenameDirDlg(m_fileRenInfoEx.getRemNameAt(0), text1, text2)) {
						bRename = true;
					} else {
						m_dwNumRenError++;
					}
				}
			}
		}
		if (!bRename) {
			m_fileRenInfoEx.deleteAt(0);
			if (m_bFTDlgStatus) m_pFileTransferDlg->reloadRemoteFileList();
		}
	}
	checkRenameQueue();
	return true;
}

int
FileTransfer::isExistName(FileInfo *pFI, char *pName)
{
	for (unsigned int i = 0; i < pFI->getNumEntries(); i++) {
		if (strcmp(pFI->getNameAt(i), pName) == 0) {
			return i;
		}
	}
	return -1;
}

bool 
FileTransfer::procFileSpecDirDataMsg()
{
	rfbFileSpecDirDataMsg fsdd;
	m_pCC->ReadExact((char *) &fsdd, sz_rfbFileSpecDirDataMsg);

	fsdd.specFlags = Swap16IfLE(fsdd.specFlags);
	fsdd.dirNameSize = Swap16IfLE(fsdd.dirNameSize);

	char path[MAX_PATH];

	if ((fsdd.dirNameSize < 0) || (fsdd.dirNameSize >= MAX_PATH)) {
		return false;
	} else {
		m_pCC->ReadExact(path, fsdd.dirNameSize);
		path[fsdd.dirNameSize] = '\0';
		convertFromUnixPath(path);
		if (m_bFTDlgStatus) m_pFileTransferDlg->showRemoteFiles(path);
		return true;
	}
}

bool 
FileTransfer::procFileDownloadDataMsg()
{
	rfbFileDownloadDataMsg fdd;
	m_pCC->ReadExact((char *)&fdd, sz_rfbFileDownloadDataMsg);

	fdd.compressedSize = Swap16IfLE(fdd.compressedSize);
	fdd.realSize = Swap16IfLE(fdd.realSize);

	if ((fdd.compressedSize == 0) && (fdd.realSize == 0)) {
		unsigned int mTime = 0;
		m_pCC->ReadExact((char *) &mTime, sizeof(mTime));
//		mTime = Swap32IfLE(mTime);
		m_fileWriter.setTime(mTime);
		m_fileWriter.close();
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Download Completed");
		if ((m_bFTDlgStatus) && (strcmp(m_fileTransferInfoEx.getLocPathAt(0), m_pFileTransferDlg->getLocalPath()) == 0)) {
			m_pFileTransferDlg->reloadLocalFileList();
		}
		m_fileTransferInfoEx.deleteAt(0);
		PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
		return true;
	}
	
	char *pBuf = new char [fdd.compressedSize];
	m_pCC->ReadExact(pBuf, fdd.compressedSize);

	if (!m_bFileTransfer) return false;

	if (m_bFTDlgStatus) m_pFileTransferDlg->processDlgMessage(NULL);
	if (m_bFTCancel) {
		m_bFTCancel = false;
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pToolBar->setAllButtonsState(-1, -1, -1, -1, 0);
		char reason[] = "File Transfer Operation Cancelled";
		sendFileDownloadCancelMsg(strlen(reason), reason);
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("File Transfer Operation Cancelled");
		delete [] pBuf;
		closeUndoneTransfer();
		return true;
	}
	DWORD dwBytesWritten = 0;
	m_fileWriter.writeBlock(fdd.compressedSize, pBuf, &dwBytesWritten);

	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pProgress->increase(fdd.compressedSize);

	delete [] pBuf;
	return true;
}

bool 
FileTransfer::procFileUploadCancelMsg()
{
	rfbFileUploadCancelMsg fuc;
	m_pCC->ReadExact((char *)&fuc, sz_rfbFileUploadCancelMsg);

	fuc.reasonLen = Swap16IfLE(fuc.reasonLen);

	char *pReason = new char [fuc.reasonLen + 1];
	m_pCC->ReadExact(pReason, fuc.reasonLen);
	pReason[fuc.reasonLen] = '\0';

	m_bUpload = false;
	m_fileReader.close();
	m_fileTransferInfoEx.deleteAt(0);
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Upload Failed.");
	PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);

	return true;
}

bool 
FileTransfer::procFileDownloadFailedMsg()
{
	rfbFileDownloadFailedMsg fdf;
	m_pCC->ReadExact((char *)&fdf, sz_rfbFileDownloadFailedMsg);

	fdf.reasonLen = Swap16IfLE(fdf.reasonLen);
	
	char *pReason = new char [fdf.reasonLen + 1];
	m_pCC->ReadExact(pReason, fdf.reasonLen);
	pReason[fdf.reasonLen] = '\0';

	m_fileWriter.close();
	m_fileTransferInfoEx.deleteAt(0);
	m_bDownload = false;
	if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("Download Failed.");
	PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);

	return true;
}

bool 
FileTransfer::procFileDirSizeDataMsg()
{
	rfbFileDirSizeDataMsg fdsd;
	m_pCC->ReadExact((char *)&fdsd, sz_rfbFileDirSizeDataMsg);

	CARD16 size16 = Swap16IfLE(fdsd.dSizeHigh16);
	CARD32 size32 = Swap32IfLE(fdsd.dSizeLow32);

	DWORD64 size64 = 0;
	size64 = size16;
	size64 = (size64 << 32) + size32;

	m_dw64TotalSize += size64;
	if (size16 == 0) m_fileTransferInfoEx.setSizeAt(m_dwDirSizeRqstNum, size32);
	m_dwDirSizeRqstNum++;
	resizeTotalSize64();

	return true;
}

bool 
FileTransfer::procFileLastRqstFailedMsg()
{
	rfbFileLastRequestFailedMsg flrf;
	m_pCC->ReadExact((char *)&flrf, sz_rfbFileLastRequestFailedMsg);

	flrf.reasonLen = Swap16IfLE(flrf.reasonLen);
	flrf.sysError = Swap32IfLE(flrf.sysError);

	char *pReason = new char [flrf.reasonLen + 1];
	m_pCC->ReadExact(pReason, flrf.reasonLen);
	pReason[flrf.reasonLen] = '\0';

	switch (flrf.typeOfRequest) 
	{
	case rfbFileSpecDirRequest:
		if (m_bFTDlgStatus) m_pFileTransferDlg->reloadLocalFileList();
		break;
	case rfbFileDownloadRequest:
		{
			m_fileWriter.close();
			DirManager dm;
			dm.deleteIt(m_fileTransferInfoEx.getFullLocPathAt(0));
			if (m_bFTDlgStatus) {
				m_pFileTransferDlg->m_pProgress->increase(m_fileTransferInfoEx.getSizeAt(0));
				m_pFileTransferDlg->m_pStatusBox->setStatusText("Download Failed: %s", pReason);
			}
			m_fileTransferInfoEx.deleteAt(0);
			PostMessage(m_pCC->m_hwnd, (UINT) WM_FT_CHECKTRANSFERQUEUE, (WPARAM) 0, (LPARAM) 0);
			break;
		}
	case rfbFileUploadRequest:
	case rfbFileRenameRequest:
	case rfbFileDeleteRequest:
		if (flrf.reasonLen >= (MAX_PATH - 1)) pReason[MAX_PATH - 1] = '\0';
		m_fileLastRqstFailedMsgs.add(pReason, flrf.sysError, 0, flrf.typeOfRequest);
		break;
	case rfbFileCreateDirRequest:
		{
			if (flrf.sysError == ERROR_ALREADY_EXISTS) {
				unsigned int flags = (m_fileTransferInfoEx.getFlagsAt(0) | FT_ATTR_FOLDER_EXISTS);
				m_fileTransferInfoEx.setFlagsAt(0, flags);
			}
		}
		break;
	case rfbFileDirSizeRequest:
		{
			m_fileTransferInfoEx.deleteAt(m_dwDirSizeRqstNum);
			resizeTotalSize64();
		}
		break;
	}

	delete [] pReason;
	return true;
}

int
FileTransfer::convertToUnixPath(char *path)
{
	int len = strlen(path);
	if (len >= MAX_PATH) return -1;
	if (len == 0) {strcpy(path, "/"); return 1;}
	for (int i = (len - 1); i >= 0; i--) {
		if (path[i] == '\\') path[i] = '/';
		path[i+1] = path[i];
	}
	path[len + 1] = '\0';
	path[0] = '/';
	return strlen(path);
}

int 
FileTransfer::convertFromUnixPath(char *path)
{
	int len = strlen(path);
	if (len >= MAX_PATH) return -1;
	if ((path[0] == '/') && (len == 1)) {
		path[0] = '\0'; 
		return 0;
	}
	for(int i = 0; i < (len - 1); i++) {
		if(path[i+1] == '/') path[i+1] = '\\';
		path[i] = path[i+1];
	}
	path[len-1] = '\0';
	return strlen(path);
}

char *
FileTransfer::getHostName() { 
	return m_pCC->m_desktopName; 
}

void
FileTransfer::setFTBoolean(bool status)
{
	m_bFileTransfer = status;
	m_bUpload = status;
	m_bDownload = status;
	m_bFTCancel = status;
	m_bOverwriteAll = status;
	m_bOverwrite0 = status;
}

bool 
FileTransfer::setErrorString(char *pPrefix, DWORD error)
{
	LPVOID pErrStr = NULL;//formatErrorString(error);

	if (pErrStr != NULL) {
		char *pStr = strdup((LPTSTR) pErrStr);
		LocalFree(pErrStr);

		int len = strlen(pStr);
		if (len > 2) { pStr[len - 2] = '\0'; } else { pStr[0] = '\0'; }
		
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("%s: %s", pPrefix, pStr);
		return true;
	} else {
		if (m_bFTDlgStatus) m_pFileTransferDlg->m_pStatusBox->setStatusText("%s.", pPrefix);
		return false;
	}
}

LPVOID
FileTransfer::formatErrorString(DWORD error)
{
	LPVOID pStr;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
					   FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
					   MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
					   (LPTSTR) &pStr, 0, NULL)) {
		if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
						   FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
						   MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
						   (LPTSTR) &pStr, 0, NULL)) return NULL;
	}
	return pStr;
}