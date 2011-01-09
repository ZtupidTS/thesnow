// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#ifndef _WII_IPC_HLE_DEVICE_FILEIO_H_
#define _WII_IPC_HLE_DEVICE_FILEIO_H_

#include "WII_IPC_HLE_Device.h"

std::string HLE_IPC_BuildFilename(const char* _pFilename, int _size);

class CWII_IPC_HLE_Device_FileIO : public IWII_IPC_HLE_Device
{
public:
	CWII_IPC_HLE_Device_FileIO(u32 _DeviceID, const std::string& _rDeviceName);

	virtual ~CWII_IPC_HLE_Device_FileIO();

	bool Close(u32 _CommandAddress, bool _bForce);
    bool Open(u32 _CommandAddress, u32 _Mode);
	bool Seek(u32 _CommandAddress);
	bool Read(u32 _CommandAddress);
	bool Write(u32 _CommandAddress);
    bool IOCtl(u32 _CommandAddress);
	void DoState(PointerWrap &p);

private:
	enum
	{
		ISFS_OPEN_READ				= 1,
		ISFS_OPEN_WRITE,
		ISFS_OPEN_RW				= (ISFS_OPEN_READ | ISFS_OPEN_WRITE)
	};

    enum
    {
        ISFS_FUNCNULL				= 0,
        ISFS_FUNCGETSTAT,
        ISFS_FUNCREADDIR,
        ISFS_FUNCGETATTR,
        ISFS_FUNCGETUSAGE
    };

    enum
    {
        ISFS_IOCTL_FORMAT			= 1,
        ISFS_IOCTL_GETSTATS,
        ISFS_IOCTL_CREATEDIR,
        ISFS_IOCTL_READDIR,
        ISFS_IOCTL_SETATTR,
        ISFS_IOCTL_GETATTR,
        ISFS_IOCTL_DELETE,
        ISFS_IOCTL_RENAME,
        ISFS_IOCTL_CREATEFILE,
        ISFS_IOCTL_SETFILEVERCTRL,
        ISFS_IOCTL_GETFILESTATS,
        ISFS_IOCTL_GETUSAGE,
        ISFS_IOCTL_SHUTDOWN
    };

    FILE* m_pFileHandle;
    u32 m_FileLength;
	u32 m_Mode;
	s32 m_Seek;

	std::string m_Filename;
};

#endif

