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

#ifndef FILETRANSFERTYPES_H__
#define FILETRANSFERTYPES_H__

#define WM_FT_CHECKTRANSFERQUEUE WM_USER+130
#define WM_FT_CHECKDELETEQUEUE   WM_USER+131
#define WM_FT_CHECKRENAMEQUEUE   WM_USER+132
#define WM_FT_UPLOADFILEPORTION  WM_USER+133

#define FT_ATTR_UNKNOWN			0x00000000
#define FT_ATTR_FILE			0x00000001
#define FT_ATTR_FOLDER			0x00000002

#define FT_ATTR_FOLDER_EXISTS	0x00100000
#define FT_ATTR_COPY_OVERWRITE	0x00200000

#define FT_ATTR_FLR_UPLOAD_CHECK	0x01400000
#define FT_ATTR_FLR_UPLOAD_ADD		0x01800000
#define FT_ATTR_COPY_UPLOAD			0x01000000

#define FT_ATTR_FLR_DOWNLOAD_CHECK  0x0A000000
#define FT_ATTR_FLR_DOWNLOAD_ADD	0x0C000000
#define FT_ATTR_COPY_DOWNLOAD		0x08000000

#define FT_ATTR_DELETE_LOCAL	0x10000000
#define FT_ATTR_DELETE_REMOTE	0x20000000

#define FT_ATTR_RENAME_LOCAL	0x40000000
#define FT_ATTR_RENAME_REMOTE	0x80000000

#define FT_FLR_DEST_MAIN     101
#define FT_FLR_DEST_BROWSE   102
#define FT_FLR_DEST_DOWNLOAD 103
#define FT_FLR_DEST_UPLOAD   104
#define FT_FLR_DEST_DELETE   105
#define FT_FLR_DEST_RENAME   106

#define FT_FDSR_DEST_MAIN     201
#define FT_FDSR_DEST_TRANSFER 202

#define FT_ID_MYCOMPUTER  0
#define FT_ID_MYDOCUMENTS 1
#define FT_ID_MYPICTURES  2
#define FT_ID_MYMUSIC     3
#define FT_ID_MYDESKTOP   4
#define FT_ID_MYNETHOOD   5


#define FT_MAX_STATUS_STRINGS		 255
#define FT_MAX_LENGTH_STATUS_STRINGS 130

typedef struct tagSIZEDATAINFO
{
	unsigned int size;
	unsigned int data;
} SIZEDATAINFO;

typedef struct tagSIZEDATAFLAGSINFO
{
	unsigned int size;
	unsigned int data;
	unsigned int flags;
} SIZEDATAFLAGSINFO;

typedef struct tagFILEINFO
{
	char name[MAX_PATH];
	SIZEDATAFLAGSINFO info;
} FILEINFO;

typedef struct tagFILEINFOEX
{
	char locPath[MAX_PATH];
	char locName[MAX_PATH];
	char remPath[MAX_PATH];
	char remName[MAX_PATH];
	SIZEDATAFLAGSINFO info;
} FILEINFOEX;

unsigned int FiletimeToTime70(FILETIME ftime);
void Time70ToFiletime(unsigned int time70, FILETIME *pftime);

#endif // FILETRANSFERTYPES_H__
