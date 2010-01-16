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

#ifndef FILETRANSFER_H__
#define FILETRANSFER_H__

#include "ClientConnection.h"
#include "FileInfo.h"
#include "FileInfoEx.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "FileTransferDialog.h"
#include "FileTransferTypes.h"

class ClientConnection;

class FileTransfer  
{
public:
	FileTransfer(ClientConnection * pCC, VNCviewerApp * pApp);
	~FileTransfer();

	FileTransferDialog *m_pFileTransferDlg;

	void createFileTransfer();

	bool sendFileListRqstMsg(unsigned short dirNameSize, char *pDirName,
							 int dest, unsigned char flags);
	bool sendFileDownloadRqstMsg(unsigned short filenameLen, char *pFilename, 
								 unsigned int position);
	bool sendFileUploadRqstMsg(unsigned short filenameLen, char *pFilename, 
							   unsigned int position);
	bool sendFileUploadDataMsg(unsigned short dataSize, char *pData);
	bool sendFileUploadDataMsg(unsigned int modTime);
	bool sendFileDownloadCancelMsg(unsigned short reasonLen, char *pReason);
	bool sendFileUploadFailedMsg(unsigned short reasonLen, char *pReason);
	bool sendFileCreateDirRqstMsg(unsigned short dirNameLen, char *pDirName);
	bool sendFileDirSizeRqstMsg(unsigned short dirNameLen, char *pDirName, int dest);
	bool sendFileRenameRqstMsg(unsigned short oldNameLen, unsigned short newNameLen, 
							   char *pOldName, char *pNewName);
	bool sendFileDeleteRqstMsg(unsigned short nameLen, char *pName);
	bool sendFileSpecDirRqstMsg(unsigned short specFlags);

	bool procFileListDataMsg();
	bool procFileSpecDirDataMsg();
	bool procFileDownloadDataMsg();
	bool procFileUploadCancelMsg();
	bool procFileDownloadFailedMsg();
	bool procFileDirSizeDataMsg();
	bool procFileLastRqstFailedMsg();

	bool isTransferEnable() { return m_bFileTransfer; };

	void setFTDlgStatus(bool status);

	void deleteLocal(char *pPathPrefix, FileInfo *pFI);
	void deleteRemote(char *pPathPrefix, FileInfo *pFI);
	void checkDeleteQueue();

	void renameLocal(char *pPath, char *pNewName, FILEINFO *pFIStruct);
	void renameRemote(char *pPath, char *pNewName, FILEINFO *pFIStruct);
	void checkRenameQueue();

	char *getHostName();

	void addTransferQueue(char *pLocalPath, char *pRemotePath, FileInfo *pFI, unsigned int attr);
	void checkTransferQueue();

	void uploadFile();
	void uploadFilePortion();

	void downloadFile();

	void closeUndoneTransfer();

	bool m_bFTCancel;
	bool m_bSendWMCloseOnYes;

private:
	int convertToUnixPath(char *path);
	int convertFromUnixPath(char *path);

	bool createFileInfo(unsigned int numFiles, FileInfo *fi, SIZEDATAINFO *pSDInfo, 
						char *pFilenames);
	bool createFileInfo(unsigned int numFiles, FileInfo *fi, SIZEDATAFLAGSINFO *pSDFInfo, 
						char *pFilenames);

	bool procFLRMain(unsigned short numFiles, FileInfo *pFI);
	bool procFLRBrowse(unsigned short numFiles, FileInfo *pFI);
	bool procFLRUpload(unsigned short numFiles, FileInfo *pFI);
	bool procFLRDownload(unsigned short numFiles, FileInfo *pFI);
	bool procFLRDelete(unsigned short numFiles, FileInfo *pFI);
	bool procFLRRename(unsigned short numFiles, FileInfo *pFI);

	bool setErrorString(char *pPrefix, DWORD error);
	LPVOID formatErrorString(DWORD error);

	int isExistName(FileInfo *pFI, char *pName);
	
	void resizeTotalSize64();
	void setFTBoolean(bool status);
	void onEndTransfer();

	ClientConnection *m_pCC;
	VNCviewerApp *m_pApp;

	FileInfo m_fileListRequestQueue;
	FileInfo m_fileLastRqstFailedMsgs;

	FileInfoEx m_fileTransferInfoEx;
	FileInfoEx m_fileDelInfoEx;
	FileInfoEx m_fileRenInfoEx;

	FileReader m_fileReader;
	FileWriter m_fileWriter;

	DWORD m_dwNumFTError;
	DWORD m_dwNumRenError;
	DWORD m_dwNumDelError;

	DWORD m_dwDirSizeRqstNum;

	DWORD64 m_dw64TotalSize;

	bool m_bFTDlgStatus;

	bool m_bUpload;
	bool m_bDownload;
	bool m_bFileTransfer;
	bool m_bGettingTotalSize;
	bool m_bResizeNeeded;
	bool m_bOverwriteAll;
	bool m_bOverwrite0;
};

#endif // FILETRANSFER_H__
