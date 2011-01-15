//  Copyright (C) 2003 Dennis Syrovatsky. All Rights Reserved.
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

#include "FileTransferItemInfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"

const char FileTransferItemInfo::folderText[] = "<Folder>";

int 
CompareFTItemInfo(const void *F, const void *S)
{
	if (strcmp(((FTITEMINFO *)F)->Size, ((FTITEMINFO *)S)->Size) == 0) {
		return stricmp(((FTITEMINFO *)F)->Name, ((FTITEMINFO *)S)->Name);
	} else {
		if (strcmp(((FTITEMINFO *)F)->Size, FileTransferItemInfo::folderText) == 0) return -1;
		if (strcmp(((FTITEMINFO *)S)->Size, FileTransferItemInfo::folderText) == 0) {
			return 1;
		} else {
		return stricmp(((FTITEMINFO *)F)->Name, ((FTITEMINFO *)S)->Name);
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileTransferItemInfo::FileTransferItemInfo()
{
	m_NumEntries = 0;
	m_pEntries = NULL;
}

FileTransferItemInfo::~FileTransferItemInfo()
{
	Free();
}

void FileTransferItemInfo::Add(char *Name, char *Size, unsigned int Data)
{
	FTITEMINFO *pTemporary = new FTITEMINFO[m_NumEntries + 1];
	if (m_NumEntries != 0) 
		memcpy(pTemporary, m_pEntries, m_NumEntries * sizeof(FTITEMINFO));
	strcpy(pTemporary[m_NumEntries].Name, Name);
	strcpy(pTemporary[m_NumEntries].Size, Size);
	pTemporary[m_NumEntries].Data = Data;
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_NumEntries++;
}

void FileTransferItemInfo::Add(char *Name, int Size, int Data)
{
	int len = strlen(Name);
	if (len > MAX_PATH) return;
	
	FTITEMINFO *pTemporary = new FTITEMINFO[m_NumEntries + 1];
	if (m_NumEntries != 0) 
		memcpy(pTemporary, m_pEntries, m_NumEntries * sizeof(FTITEMINFO));
	strcpy(pTemporary[m_NumEntries].Name, Name);
	if (Size < 0) {
		strcpy(pTemporary[m_NumEntries].Size, folderText);
	} else {
		sprintf(pTemporary[m_NumEntries].Size, "%d", Size);
	}
	pTemporary[m_NumEntries].Data = Data;
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_NumEntries++;
}

void FileTransferItemInfo::Free()
{
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_NumEntries = 0;
}

void FileTransferItemInfo::Sort()
{
	qsort(m_pEntries, m_NumEntries, sizeof(FTITEMINFO), CompareFTItemInfo);
}

char * FileTransferItemInfo::GetNameAt(int Number)
{
	if ((Number >= 0) && (Number <= m_NumEntries))
		return m_pEntries[Number].Name;
	return NULL;
}

char * FileTransferItemInfo::GetSizeAt(int Number)
{
	if ((Number >= 0) && (Number <= m_NumEntries)) 
		return m_pEntries[Number].Size; 
	return NULL;
}

unsigned int FileTransferItemInfo::GetDataAt(int Number)
{
	if ((Number >= 0) && (Number <= m_NumEntries)) 
		return m_pEntries[Number].Data;
	return 0;
}

int FileTransferItemInfo::GetNumEntries()
{
	return m_NumEntries;
}

int FileTransferItemInfo::GetIntSizeAt(int Number)
{
	return ConvertCharToInt(GetSizeAt(Number));
}

int FileTransferItemInfo::GetSummaryNamesLength()
{
	int sumLen = 0;
	for (int i = 0; i < m_NumEntries; i++)
		sumLen += strlen(m_pEntries[i].Name);
	return sumLen;
}

void FileTransferItemInfo::DeleteAt(int Number)
{
  if ((Number >= m_NumEntries) || (Number < 0)) return;

  FTITEMINFO *pTemporary = new FTITEMINFO[m_NumEntries - 1];

  if (Number == 0) {
    memcpy(pTemporary, &m_pEntries[1], (m_NumEntries - 1) * sizeof(FTITEMINFO));
  } else {
    memcpy(pTemporary, m_pEntries, Number * sizeof(FTITEMINFO));
    if (Number != (m_NumEntries - 1)) memcpy(&pTemporary[Number], &m_pEntries[Number + 1], (m_NumEntries - Number - 1) * sizeof(FTITEMINFO));
  }
  
  if (m_pEntries != NULL) {
    delete [] m_pEntries;
    m_pEntries = NULL;
  }
  m_pEntries = pTemporary;
  pTemporary = NULL;
  m_NumEntries = m_NumEntries - 1;
}

int FileTransferItemInfo::ConvertCharToInt(char *pStr)
{
	int strLen = strlen(pStr);
	int res = 0, tenX = 1;
	if (strcmp(pStr, folderText) == 0) return -1;
	for (int i = (strLen - 1); i >= 0; i--) {
		switch (pStr[i])
		{
		case '1': res = res + 1 * tenX; break;
		case '2': res = res + 2 * tenX;	break;
		case '3': res = res + 3 * tenX;	break;
		case '4': res = res + 4 * tenX;	break;
		case '5': res = res + 5 * tenX;	break;
		case '6': res = res + 6 * tenX; break;
		case '7': res = res + 7 * tenX;	break;
		case '8': res = res + 8 * tenX; break;
		case '9': res = res + 9 * tenX;	break;
		}
		tenX = tenX * 10;
	}
	if (pStr[0] == '-') res = res * (-1);
	return res;
}
