//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
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

#include "CopyRectDetector.h"
#include "windows-lib/winhdr.h"

ATOM VNC_WINDOWPOS_ATOM = NULL;
const TCHAR *VNC_WINDOWPOS_ATOMNAME = _T("VNCHooks.CopyRect.WindowPos");

CopyRectDetector::CopyRectDetector()
: m_copyRect(0)
{
  VNC_WINDOWPOS_ATOM = GlobalAddAtom(VNC_WINDOWPOS_ATOMNAME);
}

CopyRectDetector::~CopyRectDetector()
{
  // Free the WindowPos atom!
  if (VNC_WINDOWPOS_ATOM != NULL) {
    GlobalDeleteAtom(VNC_WINDOWPOS_ATOM);
  }
}

void CopyRectDetector::detectWindowMovements(Rect *copyRect, Point *source)
{
  if (VNC_WINDOWPOS_ATOM != 0) {
    m_copyRect = copyRect;
    m_source = source;
    EnumWindows((WNDENUMPROC)enumWindowsFnCopyRect, (LPARAM)this);
    m_copyRect = 0;
    source = 0;
  }
}

// Callback routine used internally to catch window movement...
BOOL CALLBACK CopyRectDetector::enumWindowsFnCopyRect(HWND hwnd, LPARAM arg)
{
  CopyRectDetector *_this = (CopyRectDetector *)arg;
  return _this->checkWindowMovements(hwnd);
}

BOOL CopyRectDetector::checkWindowMovements(HWND hwnd)
{
  //For excluding the popup windows
  if ((GetWindowLong( hwnd, GWL_STYLE) & WS_POPUP) ==0)
  {
    HANDLE prop = GetProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));

    if (prop != NULL) {
      if (IsWindowVisible(hwnd)) {
        RECT dest;
        Point source;

        // Get the window rectangle
        if (GetWindowRect(hwnd, &dest)) {
          // Old position
          source.x = (SHORT) LOWORD(prop);
          source.y = (SHORT) HIWORD(prop);

          // Got the destination position.  Now send to clients!
          if ((source.x != dest.left) || (source.y != dest.top)) {
            // Update the property entry
            SHORT x = (SHORT) dest.left;
            SHORT y = (SHORT) dest.top;
            SetProp(hwnd,
                    (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0),
                    (HANDLE) MAKELONG(x, y));

            // Store of the copyrect
            m_copyRect->setRect(&Rect(dest.left, dest.top, dest.right, dest.bottom));
            *m_source = source;

            // Adjust
            int destopX = GetSystemMetrics(SM_XVIRTUALSCREEN);
            int destopY = GetSystemMetrics(SM_YVIRTUALSCREEN);
            m_copyRect->move(-destopX, -destopY);
            m_source->move(-destopX, -destopY);

            // Stop EnumWindows() function.
            return FALSE;
          }
        } else {
          RemoveProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
        }
      } else {
        RemoveProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
      }
    } else {
      // If the window has become visible then save its position!
      if (IsWindowVisible(hwnd)) {
        RECT dest;

        if (GetWindowRect(hwnd, &dest)) {
          SHORT x = (SHORT) dest.left;
          SHORT y = (SHORT) dest.top;
          SetProp(hwnd,
            (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0),
            (HANDLE) MAKELONG(x, y));
        }
      }
    }
  }
  return TRUE;
}
