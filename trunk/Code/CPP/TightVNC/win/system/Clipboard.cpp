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

#include "Clipboard.h"

static const TCHAR s_clipbWinClassName[] = _T("ClipboardWindow");
static const HINSTANCE s_hinst = GetModuleHandle(0);

Clipboard::Clipboard()
: Window(s_hinst, s_clipbWinClassName),
  m_hwndNextViewer(0),
  m_updateListener(0),
  m_clipboardText(0),
  m_isClipboardChanged(false)
{
  resume();
}

Clipboard::~Clipboard(void)
{
  // Auto terminating
  terminate();
  wait();

  if (m_clipboardText) {
    delete[] m_clipboardText;
  }
}

TCHAR *Clipboard::extract()
{
  if (m_isClipboardChanged) {
    // Free old text.
    if (m_clipboardText) {
      delete[] m_clipboardText;
    }

    m_clipboardText = readFromClipBoard();

    m_isClipboardChanged = false;
    return m_clipboardText;
  } else {
    return 0;
  }
}

bool Clipboard::writeToClipBoard(const TCHAR *text)
{
  if (OpenClipboard(m_hwnd)) {
    EmptyClipboard();

    unsigned int textLen = _tcslen(text);

    HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, (textLen + 1) * sizeof(TCHAR));

    if (hglb) {
      TCHAR *buff = (TCHAR *)GlobalLock(hglb);
      _tcscpy_s(buff, (textLen + 1) * sizeof(TCHAR), text);
      GlobalUnlock(hglb);

      SetClipboardData(CF_TEXT, hglb);
    }

    CloseClipboard();
  }

  return false;
}

TCHAR *Clipboard::readFromClipBoard() const
{
  TCHAR *clipText = 0;

  if (!IsClipboardFormatAvailable(CF_TEXT)) {
    TCHAR empty[] = L"";
    clipText = new TCHAR[sizeof(empty)];
    _tcscpy_s(clipText, sizeof(empty) * sizeof(TCHAR), empty);
    return clipText;
  }

  if (!OpenClipboard(m_hwnd)) {
    return NULL;
  }

  HANDLE hglb = GetClipboardData(CF_TEXT);
  if (hglb != NULL) {
    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
    if (lptstr != NULL) {
      unsigned int textLen = _tcslen(lptstr);
      clipText = new TCHAR[textLen + 1];
      _tcscpy_s(clipText, (textLen + 1)* sizeof(TCHAR), lptstr);
      GlobalUnlock(hglb);
    }
  }
  CloseClipboard();

  return clipText;
}

bool Clipboard::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  int fake = 3;
  switch (message)
  {
  case WM_CREATE:
    m_hwndNextViewer = SetClipboardViewer((HWND)wParam);
    break;

  case WM_CHANGECBCHAIN:
    if ((HWND) wParam == m_hwndNextViewer) {
      m_hwndNextViewer = (HWND) lParam;
    }
    else if (m_hwndNextViewer != NULL) {
      SendMessage(m_hwndNextViewer, message, wParam, lParam);
    }

    break;

  case WM_DESTROY:
    ChangeClipboardChain(m_hwnd, m_hwndNextViewer);
    break;

  case WM_DRAWCLIPBOARD:  // clipboard contents changed.
    m_isClipboardChanged = true;

    doUpdate();

    SendMessage(m_hwndNextViewer, message, wParam, lParam);

    break;

  default:
    return false; // Message not processing
  }

  return true;
}

void Clipboard::onTerminate()
{
  if (m_hwnd != 0) {
    PostMessage(m_hwnd, WM_QUIT, 0, 0);
  }
}

void Clipboard::execute()
{
  if (!createWindow()) {
    return;
  }

  MSG msg;
  while (!m_terminated) {
    if (GetMessage(&msg, m_hwnd, 0, 0)) {
      DispatchMessage(&msg);
    } else {
      break;
    }
  }

  destroyWindow();
}
