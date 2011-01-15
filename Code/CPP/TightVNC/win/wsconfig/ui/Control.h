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

#ifndef __CONTROL_H_
#define __CONTROL_H_

#include "common/tstring.h"

// Simple class to manipulate Window
//
class Control
{
public:
  Control();
  Control(HWND hwnd);
  virtual ~Control() { }

  HWND getWindow() { return m_hwnd; }
  void setWindow(HWND hwnd) { m_hwnd = hwnd; }

  // Window style
  void setStyle(DWORD style);
  DWORD getStyle();

  void setStyleFlag(DWORD value);
  void clearStyleFlag(DWORD value);
  bool isStyleFlagEnabled(DWORD value);

  // Enable/disabe methods
  void enable();
  void disable();
  bool isEnabled();

  void setFocus();
  void setForeground();
  void hide();
  void show();
  void invalidate();

  // FIXME: Think - in all components we have setText/getText
  // methods?
  void setText(LPTSTR text);
  tstring getText();

protected:
  HWND m_hwnd;

  // FIXME: Not using it yet. Maybe need to remove it?
  virtual void setDefaultFlags() { };
};

#endif
