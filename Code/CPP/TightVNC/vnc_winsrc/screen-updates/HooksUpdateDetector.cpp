//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
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

#include "windows-lib/winhdr.h"
#include "HooksUpdateDetector.h"
#include "region/Rect.h"
#include "libscreen/DesktopSelector.h"

typedef int (*SetHookFunction)(HWND hWnd, UINT UpdateMsg, UINT CopyMsg, UINT MouseMsg);
typedef int (*UnsetHookFunction)(HWND hWnd);

// Constants
const UINT RFB_SCREEN_UPDATE = RegisterWindowMessage(_T("TightVNC.Server.Update.DrawRect"));
const UINT RFB_COPYRECT_UPDATE = RegisterWindowMessage(_T("TightVNC.Server.Update.CopyRect"));
const UINT RFB_MOUSE_UPDATE = RegisterWindowMessage(_T("TightVNC.Server.Update.Mouse"));

HooksUpdateDetector::HooksUpdateDetector(UpdateKeeper *updateKeeper)
: UpdateDetector(updateKeeper),
m_hooksTargetWindow(0),
m_hHooks(0),
m_pSetHook(0),
m_pUnSetHook(0)
{
}

HooksUpdateDetector::~HooksUpdateDetector(void)
{
}

void HooksUpdateDetector::onTerminate()
{
  if (m_hooksTargetWindow != 0) {
    PostMessage(m_hooksTargetWindow->getHWND(), WM_QUIT, 0, 0);
  }
}

bool HooksUpdateDetector::initHook()
{
  HINSTANCE hinst = GetModuleHandle(0);

  m_hooksTargetWindow = new HooksTargetWindow(hinst);
  if (!m_hooksTargetWindow->createWindow()) {
    return false;
  }

  // Dll initializing
  if ((m_hHooks = LoadLibrary(_T(LIBRARY_NAME))) == 0) {
    return false;
  }

  m_pSetHook = GetProcAddress(m_hHooks, SET_HOOK_FUNCTION_NAME);
  m_pUnSetHook = GetProcAddress(m_hHooks, UNSET_HOOK_FUNCTION_NAME);
  if (!m_pSetHook || !m_pUnSetHook) {
    return false;
  }

  // Hooks initializing
  SetHookFunction setHookFunction = (SetHookFunction)m_pSetHook;
  if (setHookFunction(m_hooksTargetWindow->getHWND(),
                      RFB_SCREEN_UPDATE,
                      RFB_COPYRECT_UPDATE,
                      RFB_MOUSE_UPDATE) == FALSE) {
    return false;
  }

  return true;
}

bool HooksUpdateDetector::unInitHook()
{
  bool result = true;

  if (m_pUnSetHook) {
    if (m_hooksTargetWindow) {
      if (m_hooksTargetWindow->getHWND() != 0) {
        UnsetHookFunction unsetHookFunction = (UnsetHookFunction)m_pUnSetHook;
        result &= (unsetHookFunction(m_hooksTargetWindow->getHWND()) != FALSE);
      }
    }
    m_pUnSetHook = 0;
    m_pSetHook = 0;
  }

  if (m_hHooks != 0) {
    if ((result &= (::FreeLibrary(m_hHooks) != FALSE))) {
      m_hHooks = 0;
    }
  }

  if (m_hooksTargetWindow) {
    delete m_hooksTargetWindow;
    m_hooksTargetWindow = 0;
  }

  return result;
}

void HooksUpdateDetector::execute()
{
  DesktopSelector::selectDesktop();

  while (!m_terminated) {
    if (!initHook()) {
      waitTerminated(5000);
      unInitHook();
      waitTerminated(5000);
    } else {
      break;
    }
  }

  MSG msg;
  Rect screenRect;
  while (!m_terminated) {
    if (!PeekMessage(&msg, m_hooksTargetWindow->getHWND(), NULL, NULL, PM_REMOVE)) {
      if (!WaitMessage()) {
        break;
      }
    } else if (msg.message == RFB_SCREEN_UPDATE) {
      Rect rect;
      rect.left = (SHORT)LOWORD(msg.wParam);
      rect.top = (SHORT)HIWORD(msg.wParam);
      rect.right = (SHORT)LOWORD(msg.lParam);
      rect.bottom = (SHORT)HIWORD(msg.lParam);

      // Adjust
      int destopX = GetSystemMetrics(SM_XVIRTUALSCREEN);
      int destopY = GetSystemMetrics(SM_YVIRTUALSCREEN);

      rect.move(-destopX, -destopY);
      m_updateKeeper->addChangedRect(&rect);

      doUpdate();
    } else if (msg.message == WM_QUIT) {
      break;
    } else {
      DispatchMessage(&msg);
    }
  }

  unInitHook();
}
