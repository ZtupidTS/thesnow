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

#if !defined(FILETRANSFERITEMINFO_H)
#define FILETRANSFERITEMINFO_H

#include "windows.h"

typedef struct tagFTITEMINFO
{
    char Name[MAX_PATH];
    char Size[16];
	unsigned int Data;
} FTITEMINFO;

typedef struct tagFTSIZEDATA
{
	unsigned int size;
	unsigned int data;
} FTSIZEDATA;

class FileTransferItemInfo  
{
public:
	int GetIntSizeAt(int Number);
	static const char folderText[];
	int GetNumEntries();
	int GetSummaryNamesLength();
	char * GetSizeAt(int Number);
	char * GetNameAt(int Number);
	unsigned int GetDataAt(int Number);
	void Sort();
	void Free();
	void Add(char *Name, char *Size, unsigned int Data);
	void Add(char *Name, int Size, int Data);
	void DeleteAt(int Number);
	FileTransferItemInfo();
	virtual ~FileTransferItemInfo();

private:
	int ConvertCharToInt(char *pStr);
	FTITEMINFO * m_pEntries;
	int m_NumEntries;
};

#endif // !defined(FILETRANSFERITEMINFO_H)
