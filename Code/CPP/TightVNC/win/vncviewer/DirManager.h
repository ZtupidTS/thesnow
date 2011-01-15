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

#ifndef DIRMANAGER_H__
#define DIRMANAGER_H__

#include "shlobj.h"

#include "FileInfo.h"
#include "FileTransferTypes.h"

class DirManager  
{
public:
	DirManager();
	~DirManager();

	bool getDriveInfo(FileInfo *pFI);
	bool getFilesInfo(FileInfo *pFI, char *pPath, unsigned short dirOnly);

	bool createDirectory(char *pPath, char *pName);
	bool createDirectory(char *pFullPath);


	bool deleteIt(char *pFullPath);
	bool deleteIt(char *pPath, FILEINFO *pFIStruct);

	bool renameIt(char *pOldFullName, char *pNewFullName);
	bool renameIt(char *pPath, char *pOldName, char *pNewName);

	char * getPath(char *pFullPath);
	char * getName(char *pFullPath);

	bool getInfo(char *pFullPath, FILEINFO *pFIStruct);
	bool getDirSize(char *pFullPath, DWORD64 *dirSize);

	bool getSpecFolderPath(char *pPath, unsigned int rfbSpecDirID);
	bool getSpecFolderInfo(FileInfo *pFI, unsigned int rfbSpecDirID, unsigned short dirOnly);

	DWORD getLastError() { return m_dwLastError; };

private:
	DWORD m_dwLastError;
	char m_szTempPath[MAX_PATH];
	char m_szTempName[MAX_PATH];
};

#endif // DIRMANAGER_H__
