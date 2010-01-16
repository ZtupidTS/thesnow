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

#ifndef _BASE_DIALOG_H_
#define _BASE_DIALOG_H_

#include "common/tstring.h"
#include "Control.h"

class BaseDialog
{
public:
  BaseDialog();
  BaseDialog(tstring resourceName);
  BaseDialog(LPTSTR resourceName);
  ~BaseDialog();
public:
  int show();
  int showModal();
  void setResourceName(LPTSTR resourceName);
  void setParent(Control *ctrlParent);
  virtual void kill(int code);

  static bool InitCommonControlsEx();
protected:
  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy();
  static BOOL CALLBACK modalDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
  tstring m_resourceName; // Name of dialog resource
  Control m_ctrlThis;     // This dialog control
  Control *m_ctrlParent;
};

#endif
