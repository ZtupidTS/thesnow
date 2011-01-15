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

#include "InputBlocker.h"

HHOOK InputBlocker::m_hKeyboardHook = 0;
HHOOK InputBlocker::m_hMouseHook = 0;
bool InputBlocker::m_isSecondaryInstance = false;

InputBlocker::InputBlocker(void)
: m_isKeyboardBlocking(false),
  m_isMouseBlocking(false)
{
  // Is this a secondary instance?
  if (m_isSecondaryInstance) {
    m_isValid = false;
    m_terminated = true;
  } else {
    m_isSecondaryInstance = true;
    m_isValid = true;
  }

  resume();
}

InputBlocker::~InputBlocker(void)
{
  if (m_isValid) {
    m_isSecondaryInstance = false;

    terminate();
    wait();
  }
}

bool InputBlocker::setKeyboardBlocking(bool block)
{
  if (m_isValid) {
    m_isKeyboardBlocking = block;
    return PostThreadMessage(m_threadID, 0, 0, 0) != 0;
  }
  return false;
}

bool InputBlocker::setMouseBlocking(bool block)
{
  if (m_isValid) {
    m_isMouseBlocking = block;
    return PostThreadMessage(m_threadID, 0, 0, 0) != 0;
  }
  return false;
}

bool InputBlocker::setKeyboardFilterHook(bool block)
{
  if (block) {
    if (m_hKeyboardHook == 0) {
      HINSTANCE hinst = GetModuleHandle(0);
      m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)lowLevelKeyboardFilterProc, hinst, 0L);
    }
    return m_hKeyboardHook != 0;

  } else {
    if (m_hKeyboardHook != 0) {
      if (UnhookWindowsHookEx(m_hKeyboardHook) == FALSE) {
        return false;
      }
      m_hKeyboardHook = 0;
    }
  }
  return true;
}

bool InputBlocker::setMouseFilterHook(bool block)
{
  if (block) {
    if (m_hMouseHook == 0) {
      HINSTANCE hinst = GetModuleHandle(0);
      m_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)lowLevelMouseFilterProc, hinst, 0L);
    }
    return m_hMouseHook != 0;

  } else {
    if (m_hMouseHook != 0) {
      if (UnhookWindowsHookEx(m_hMouseHook) == FALSE) {
        return false;
      }
      m_hMouseHook = 0;
    }
  }
  return true;
}

LRESULT CALLBACK InputBlocker::lowLevelKeyboardFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT *hookStruct = (KBDLLHOOKSTRUCT *)lParam;
    // If this a hardware event then block it.
    if (!(hookStruct->flags & LLKHF_INJECTED)) {
      return TRUE;
    }
  }
  return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK InputBlocker::lowLevelMouseFilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode == HC_ACTION) {
    MSLLHOOKSTRUCT *hookStruct = (MSLLHOOKSTRUCT *)lParam;
    // If this a hardware event then block it.
    if (!(hookStruct->flags & LLMHF_INJECTED)) {
      return TRUE;
    }
  }
  return CallNextHookEx(m_hMouseHook, nCode, wParam, lParam);
}

void InputBlocker::onTerminate()
{
  PostThreadMessage(m_threadID, WM_QUIT, 0, 0);
}

void InputBlocker::execute()
{
  MSG msg;
  while (!m_terminated) {
    if (m_isKeyboardBlocking && m_hKeyboardHook == 0) {
      // FIXME: write error handler
      setKeyboardFilterHook(true);
    }
    if (!m_isKeyboardBlocking && m_hKeyboardHook != 0) {
      // FIXME: write error handler
      setKeyboardFilterHook(false);
    }

    if (m_isMouseBlocking && m_hMouseHook == 0) {
      // FIXME: write error handler
      setMouseFilterHook(true);
    }
    if (!m_isMouseBlocking && m_hMouseHook != 0) {
      // FIXME: write error handler
      setMouseFilterHook(false);
    }

    if (!PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
      if (!WaitMessage()) {
        break;
      }

    } else if (msg.message == WM_QUIT) {
      break;

    } else {
      DispatchMessage(&msg);
    }
  }

  // Free system resources
  if (m_isValid) {
    setKeyboardFilterHook(false);
    setMouseFilterHook(false);
  }
}
