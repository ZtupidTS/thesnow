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

#ifndef __INPUTBLOCKER_H__
#define __INPUTBLOCKER_H__

#include "windows-lib/winhdr.h"
#include "thread/Thread.h"

// Only one instance of this class may be created.

class InputBlocker : protected Thread
{
public:
  InputBlocker(void);
  virtual ~InputBlocker(void);

  bool setKeyboardBlocking(bool block);
  bool setMouseBlocking(bool block);

protected:
  virtual void execute();
  virtual void onTerminate();

  bool setKeyboardFilterHook(bool block);
  bool setMouseFilterHook(bool block);

  static LRESULT CALLBACK lowLevelKeyboardFilterProc(int nCode,
                                                     WPARAM wParam,
                                                     LPARAM lParam);
  static LRESULT CALLBACK lowLevelMouseFilterProc(int nCode,
                                                  WPARAM wParam,
                                                  LPARAM lParam);

  static HHOOK m_hKeyboardHook;
  static HHOOK m_hMouseHook;

  static bool m_isSecondaryInstance;

  bool m_isValid;

  bool m_isKeyboardBlocking;
  bool m_isMouseBlocking;
};

#endif // __INPUTBLOCKER_H__
