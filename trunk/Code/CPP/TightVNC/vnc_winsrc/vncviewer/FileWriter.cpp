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

#include "FileWriter.h"
#include "FileTransferTypes.h"

FileWriter::FileWriter()
{
	m_dwLastError = ERROR_SUCCESS;
	m_hFile = NULL;
}

FileWriter::~FileWriter()
{
	close();
}

bool 
FileWriter::open(char *pFilename)
{
	m_hFile = CreateFile(pFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
						 NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	m_dwLastError = GetLastError();
	if (m_hFile == INVALID_HANDLE_VALUE) return false;

	m_dwLastError = ERROR_SUCCESS;
	return true;
}

bool 
FileWriter::close()
{
	if (m_hFile == NULL) return true;
	if (CloseHandle(m_hFile) == 0) {
		m_dwLastError = GetLastError();
		return false;
	}
	m_hFile = NULL;
	m_dwLastError = ERROR_SUCCESS;
	return true;
}

bool 
FileWriter::writeBlock(DWORD nBytesToWrite, char *pData, LPDWORD nBytesWritten)
{
	if (WriteFile(m_hFile, pData, nBytesToWrite, nBytesWritten, NULL) != 0) {
		m_dwLastError = ERROR_SUCCESS;
		return true;
	} else {
		m_dwLastError = GetLastError();
		return false;
	}
}

bool 
FileWriter::setPointer(LONG position)
{
	return false;
}

bool
FileWriter::setTime(unsigned int mTime)
{
	FILETIME ft;
	Time70ToFiletime(mTime, &ft);
	if (SetFileTime(m_hFile, &ft, &ft, &ft) == 0) {
		m_dwLastError = GetLastError();
		return false;
	}
	m_dwLastError = ERROR_SUCCESS;
	return true;
}
