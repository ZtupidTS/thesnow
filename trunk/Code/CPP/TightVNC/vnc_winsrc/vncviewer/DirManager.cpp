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

#include "DirManager.h"

DirManager::DirManager()
{
	m_dwLastError = ERROR_SUCCESS;
}

DirManager::~DirManager()
{
}

bool 
DirManager::getDriveInfo(FileInfo *pFI)
{
	TCHAR szDrivesList[256];
	if (GetLogicalDriveStrings(255, szDrivesList) == 0)
		return false;
	int i = 0;
	while (szDrivesList[i] != '\0') {
		char *drive = strdup(&szDrivesList[i]);
		char *backslash = strrchr(drive, '\\');
		if (backslash != NULL)
			*backslash = '\0';
		pFI->add(drive, 0, 0, FT_ATTR_FOLDER);
		free(drive);
		i += strcspn(&szDrivesList[i], "\0") + 1;
	}
	m_dwLastError = ERROR_SUCCESS;
	return true;
}

bool
DirManager::getSpecFolderPath(char *pPath, unsigned int rfbSpecDirID)
{
	unsigned int idFolder = 0;

	switch (rfbSpecDirID)
	{
	case rfbSpecDirMyDocuments:
		idFolder = CSIDL_PERSONAL;
		break;
	case rfbSpecDirMyPictures:
//		idFolder = CSIDL_MYPICTURES;
		idFolder = 0x0027;
		break;
	case rfbSpecDirMyMusic:
//		idFolder = CSIDL_MYMUSIC;
		idFolder = 0x000d;
		break;
	case rfbSpecDirDesktop:
		idFolder = CSIDL_DESKTOP;
		break;
	case rfbSpecDirMyNetHood:
		idFolder = CSIDL_NETHOOD;
		break;
	default:
		return false;
	}
	BOOL bCreate = FALSE;
	if (SHGetSpecialFolderPath(NULL, pPath, idFolder, bCreate)) {
		m_dwLastError = ERROR_SUCCESS;
		return true;
	} else {
		m_dwLastError = GetLastError();
		return false;
	}
}

bool
DirManager::getSpecFolderInfo(FileInfo *pFI, unsigned int rfbSpecDirID, unsigned short dirOnly)
{
	char specPath[MAX_PATH];
	if (!getSpecFolderPath(specPath, rfbSpecDirID)) return false;

	return getFilesInfo(pFI, specPath, dirOnly);
}

bool 
DirManager::getFilesInfo(FileInfo *pFI, char *pPath, unsigned short dirOnly)
{
	if (strlen(pPath) == 0) return getDriveInfo(pFI);

	char _pPath[MAX_PATH];
	sprintf(_pPath, "%s\\*", pPath);

	WIN32_FIND_DATA FindFileData;
	SetErrorMode(SEM_FAILCRITICALERRORS);
	HANDLE handle = FindFirstFile(_pPath, &FindFileData);
	DWORD lastError = GetLastError();
	SetErrorMode(0);

	if (handle != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(FindFileData.cFileName, ".") != 0 &&
				strcmp(FindFileData.cFileName, "..") != 0) {
				LARGE_INTEGER li;
				li.LowPart = FindFileData.ftLastWriteTime.dwLowDateTime;
				li.HighPart = FindFileData.ftLastWriteTime.dwHighDateTime;							
				li.QuadPart = (li.QuadPart - 116444736000000000) / 10000000;
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {	
					pFI->add(FindFileData.cFileName, 0, li.LowPart, FT_ATTR_FOLDER);
				} else {
					if (!dirOnly)
						pFI->add(FindFileData.cFileName, FindFileData.nFileSizeLow, li.LowPart, FT_ATTR_FILE);
				}
			}
			
		} while (FindNextFile(handle, &FindFileData));
	} else {
		m_dwLastError = lastError;
		return false;
	}
	FindClose(handle);
	m_dwLastError = ERROR_SUCCESS;
	return true;
}

bool
DirManager::getDirSize(char *pFullPath, DWORD64 *dirSize)
{
	char fullPath[MAX_PATH];
	FileInfo fi;
	fi.add(pFullPath, 0, 0, FT_ATTR_FOLDER);
	DWORD64 dirFileSize64 = 0;
	do {
		sprintf(fullPath, "%s\\*", fi.getNameAt(0));
		WIN32_FIND_DATA FindFileData;
		SetErrorMode(SEM_FAILCRITICALERRORS);
		HANDLE hFile = FindFirstFile(fullPath, &FindFileData);
		m_dwLastError = GetLastError();
		SetErrorMode(0);
		
		if (hFile != INVALID_HANDLE_VALUE) {
			do {
				if (strcmp(FindFileData.cFileName, ".") != 0 &&
					strcmp(FindFileData.cFileName, "..") != 0) {
					char buff[MAX_PATH];
					if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {	
						sprintf(buff, "%s\\%s", fi.getNameAt(0), FindFileData.cFileName);
						fi.add(buff, 0, 0, FT_ATTR_FOLDER);
					} else {
						dirFileSize64 += FindFileData.nFileSizeLow;
					}
				}
			} while (FindNextFile(hFile, &FindFileData));
			FindClose(hFile);
		}
		fi.deleteAt(0);
	} while (fi.getNumEntries() > 0);
	*dirSize = dirFileSize64;
	return true;
}

bool
DirManager::getInfo(char *pFullPath, FILEINFO *pFIStruct)
{
	WIN32_FIND_DATA FindFileData;
	SetErrorMode(SEM_FAILCRITICALERRORS);
	HANDLE hFile = FindFirstFile(pFullPath, &FindFileData);
	DWORD lastError = GetLastError();
	SetErrorMode(0);
	if (hFile != INVALID_HANDLE_VALUE) {
		FindClose(hFile);
		strcpy(pFIStruct->name, FindFileData.cFileName);
		pFIStruct->info.data = FiletimeToTime70(FindFileData.ftLastWriteTime);
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {	
			pFIStruct->info.size = 0;
			pFIStruct->info.flags = FT_ATTR_FOLDER;
			return true;
		} else {
			pFIStruct->info.size = FindFileData.nFileSizeLow;
			pFIStruct->info.flags = FT_ATTR_FILE;
			return true;
		}
	}
	m_dwLastError = lastError;
	return false;
}

char *
DirManager::getPath(char *pFullPath)
{
	char _pFullPath[MAX_PATH];
	strcpy(_pFullPath, pFullPath);

	for (int i = strlen(_pFullPath) - 2; i > 0; i--) {
		if (_pFullPath[i] == '\\') {
			_pFullPath[i] = '\0';
			break;
		}
	}
	strcpy(m_szTempPath, _pFullPath);
	return m_szTempPath;
}

char *
DirManager::getName(char *pFullPath)
{
	char _pFullPath[MAX_PATH];
	strcpy(_pFullPath, pFullPath);

	for (int i = strlen(_pFullPath) - 2; i > 0; i--) {
		if (_pFullPath[i] == '\\') {
			strcpy(_pFullPath, (char *)&_pFullPath[i + 1]);
			break;
		}
	}
	strcpy(m_szTempName, _pFullPath);
	return m_szTempName;
}

bool
DirManager::deleteIt(char *pFullPath)
{
	FILEINFO fiStruct;
	if (getInfo(pFullPath, &fiStruct)) {
		return deleteIt(getPath(pFullPath), &fiStruct);
	} else {
		return false;
	}
}

bool 
DirManager::deleteIt(char *pPath, FILEINFO *pFIStruct)
{
	char fullPath[MAX_PATH];
	bool bResult = true;
	FileInfo fi;
	fi.add(pFIStruct);
	while (fi.getNumEntries() > 0) {
		int lastEntries = fi.getNumEntries() - 1;
		sprintf(fullPath, "%s\\%s", pPath, fi.getNameAt(lastEntries));
		if (fi.getFlagsAt(lastEntries) & FT_ATTR_FOLDER) {
			if (!RemoveDirectory(fullPath)) {
				DWORD lastError = GetLastError();
				if (lastError == ERROR_DIR_NOT_EMPTY) {
					strcat(fullPath, "\\*");
					WIN32_FIND_DATA FindFileData;
					SetErrorMode(SEM_FAILCRITICALERRORS);
					HANDLE hFile = FindFirstFile(fullPath, &FindFileData);
					lastError = GetLastError();
					SetErrorMode(0);
					if (hFile != INVALID_HANDLE_VALUE) {
						do {
							if (strcmp(FindFileData.cFileName, ".") != 0 &&
								strcmp(FindFileData.cFileName, "..") != 0) {
								char buff[MAX_PATH];
								if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {	
									sprintf(buff, "%s\\%s", fi.getNameAt(lastEntries), FindFileData.cFileName);
									fi.add(buff, 0, 0, FT_ATTR_FOLDER);
								} else {
									sprintf(buff, "%s\\%s\\%s", pPath, fi.getNameAt(lastEntries), FindFileData.cFileName);
									SetFileAttributes(buff, FILE_ATTRIBUTE_NORMAL);
									if (!DeleteFile(buff)) {
										m_dwLastError = GetLastError();
										bResult = false;
									}
								}
							}
						} while (FindNextFile(hFile, &FindFileData));
						FindClose(hFile);
					} else {
						m_dwLastError = lastError;
					}
				} else {
					bResult = false;
					m_dwLastError = lastError;
					fi.deleteAt(lastEntries);
				}
			} else {
				fi.deleteAt(lastEntries);
			}
		} else {
			SetFileAttributes(fullPath, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(fullPath)) {
				bResult = false;
				m_dwLastError = GetLastError();
			}
			fi.deleteAt(lastEntries);
		}
	}
	if (bResult) m_dwLastError = ERROR_SUCCESS;
	return bResult;
}

bool 
DirManager::renameIt(char *pPath, char *pOldName, char *pNewName)
{
	char oldName[MAX_PATH];
	char newName[MAX_PATH];
	sprintf(oldName, "%s\\%s", pPath, pOldName);
	sprintf(newName, "%s\\%s", pPath, pNewName);
	return renameIt(oldName, newName);
}

bool 
DirManager::renameIt(char *pOldFullName, char *pNewFullName)
{
	if (MoveFile(pOldFullName, pNewFullName)) {
		m_dwLastError = ERROR_SUCCESS;
		return true;
	} else {
		m_dwLastError = GetLastError();
		return false;
	}
}

bool
DirManager::createDirectory(char *pPath, char *pName)
{
	char fullPath[MAX_PATH];
	sprintf(fullPath, "%s\\%s", pPath, pName);
	return createDirectory(fullPath);
}

bool
DirManager::createDirectory(char *pFullPath)
{
	if (CreateDirectory(pFullPath, NULL) == 0) {
		m_dwLastError = GetLastError();
		return false;
	} else {
		m_dwLastError = ERROR_SUCCESS;
		return true;
	}
}