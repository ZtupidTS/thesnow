//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC homepage on the Web: http://www.tightvnc.com/

#ifndef __DESKTOPSELECTOR_H__
#define __DESKTOPSELECTOR_H__

#include "windows-lib/winhdr.h"

class DesktopSelector
{
public:
  static BOOL selectDesktop();
  static BOOL selectHDESK(HDESK new_desktop);

  static HDESK getCurrentDesktop();
  static BOOL getDesktopChanging();

  inline static BOOL isWinNT()
  {
    return (m_platform_id == VER_PLATFORM_WIN32_NT);
  }

  // Init() function must be called first
  inline static void init()
  {
    OSVERSIONINFO osversioninfo;
    osversioninfo.dwOSVersionInfoSize = sizeof(osversioninfo);

    // Get the current OS version
    if (!GetVersionEx(&osversioninfo))
    {
      m_platform_id = 0;
    }

    m_platform_id = osversioninfo.dwPlatformId;
    m_version_major = osversioninfo.dwMajorVersion;
    m_version_minor = osversioninfo.dwMinorVersion;
  }

private:
  static unsigned int m_platform_id;
  static unsigned int m_version_major;
  static unsigned int m_version_minor;
};

#endif //__DESKTOPSELECTOR_H__
