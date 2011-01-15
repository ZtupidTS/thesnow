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

#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__

#include "windows-lib/winhdr.h"
#include "screen-updates/Window.h"
#include "screen-updates/UpdateListener.h"
#include "thread/Thread.h"

class Clipboard : protected Window, Thread
{
public:
  Clipboard(void);
  virtual ~Clipboard(void);

  TCHAR *extract();

  // This function replaces clipboard content by the text
  virtual bool writeToClipBoard(const TCHAR *text);

  // This function read text from the clipboard. The function return
  // ponter to a TCHAR string. After use the returned string,
  // pointer must be freed by the delete[] operator. If the function
  // fails, the return value is zero.
  virtual TCHAR *readFromClipBoard() const;

  virtual void setUpdateListener(UpdateListener *updateListener)
  {
    m_updateListener = updateListener;
  }

protected:
  void doUpdate()
  {
    if (m_updateListener) {
      m_updateListener->onUpdate();
    }
  }
  virtual bool wndProc(UINT message, WPARAM wParam, LPARAM lParam);

  virtual void execute();
  virtual void onTerminate();

  HWND m_hwndNextViewer;

  UpdateListener *m_updateListener;
  TCHAR *m_clipboardText;
  bool m_isClipboardChanged;
};

#endif // __CLIPBOARD_H__
