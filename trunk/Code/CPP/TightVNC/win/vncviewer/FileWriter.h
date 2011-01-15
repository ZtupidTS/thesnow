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

#ifndef FILEWRITER_H__
#define FILEWRITER_H__

class FileWriter
{
public:
	FileWriter();
	~FileWriter();

	bool open(char *pFilename);
	bool close();

	bool writeBlock(DWORD nBytesToWrite, char *pData, LPDWORD nBytesWritten);

	bool setPointer(LONG position);

	DWORD getLastError() { return m_dwLastError; };

	bool setTime(unsigned int mTime);

private:
	HANDLE m_hFile;
	DWORD m_dwLastError;
};

#endif // FILEWRITER_H__
