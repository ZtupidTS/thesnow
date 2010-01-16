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

#include "DesktopSelector.h"

unsigned int DesktopSelector::m_platform_id = 0;
unsigned int DesktopSelector::m_version_major = 0;
unsigned int DesktopSelector::m_version_minor = 0;

HDESK DesktopSelector::getCurrentDesktop()
{
  HDESK desktop = 0;
  // Are we running on NT?
  if (isWinNT()) {
    // No, so open the input desktop
    desktop = OpenInputDesktop(0, TRUE,
                               DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
                               DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
                               DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
                               DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
  }

  return desktop;
}

BOOL DesktopSelector::selectDesktop()
{
  HDESK desktop = getCurrentDesktop();

  if (isWinNT()) {
    // Switch to the new desktop
    if (!selectHDESK(desktop)) {
      // Failed to enter the new desktop, so free it!
      if (!CloseDesktop(desktop)) {}
      return FALSE;
    }
  }

  return TRUE;
}

BOOL DesktopSelector::selectHDESK(HDESK new_desktop)
{
  // Are we running on NT?
  if (isWinNT())
  {
    HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());

    DWORD dummy;
    char new_name[256];

    if (!GetUserObjectInformation(new_desktop, UOI_NAME, &new_name, 256, &dummy)) {
      return FALSE;
    }

    // Switch the desktop
    if(!SetThreadDesktop(new_desktop)) {
      return FALSE;
    }

    // Switched successfully - destroy the old desktop
    if (!CloseDesktop(old_desktop))
      return TRUE;
  }

  return TRUE;
}

BOOL DesktopSelector::getDesktopChanging()
{
  // Are we running on NT?
  if (isWinNT())
  {
    // Get the input and thread desktops
    HDESK threaddesktop = GetThreadDesktop(GetCurrentThreadId());
    HDESK inputdesktop = OpenInputDesktop(0, TRUE,
                                          DESKTOP_CREATEMENU |
                                          DESKTOP_CREATEWINDOW |
                                          DESKTOP_ENUMERATE |
                                          DESKTOP_HOOKCONTROL |
                                          DESKTOP_WRITEOBJECTS |
                                          DESKTOP_READOBJECTS |
                                          DESKTOP_SWITCHDESKTOP |
                                          GENERIC_WRITE);

    if (inputdesktop == NULL) {
      // Returning TRUE on ERROR_BUSY fixes the bug #1109102.
      // FIXME: Probably this is not the most correct way to do it.
      return (GetLastError() == ERROR_BUSY) ? FALSE : TRUE;
    }

    DWORD dummy;
    char threadname[256];
    char inputname[256];

    if (!GetUserObjectInformation(threaddesktop, UOI_NAME, &threadname, 256, &dummy)) {
      if (!CloseDesktop(inputdesktop)) {}
      return FALSE;
    }

    if (!GetUserObjectInformation(inputdesktop, UOI_NAME, &inputname, 256, &dummy)) {
      if (!CloseDesktop(inputdesktop)) {}
      return FALSE;
    }

    if (!CloseDesktop(inputdesktop)) {}

    if (strcmp(threadname, inputname) != 0) {
      return TRUE;
    }
  }

  return FALSE;
}
