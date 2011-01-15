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

#include "StdAfx.h"
#include "BaseDialog.h"
#include <commctrl.h>

BaseDialog::BaseDialog()
: m_ctrlParent(NULL)
{
  m_resourceName = _T("");
}

BaseDialog::BaseDialog(tstring resourceName)
: m_ctrlParent(NULL)
{
  m_resourceName = resourceName;
}

BaseDialog::BaseDialog(LPTSTR resourceName)
: m_ctrlParent(NULL)
{
  m_resourceName = resourceName;
}

BaseDialog::~BaseDialog()
{
}

void BaseDialog::setResourceName(LPTSTR resourceName)
{
  m_resourceName = resourceName;
}

void BaseDialog::setParent(Control *ctrlParent)
{
  m_ctrlParent = ctrlParent;
}

int BaseDialog::show()
{
  return 0;
}

void BaseDialog::kill(int code)
{
  EndDialog(m_ctrlThis.getWindow(), code);
}

int BaseDialog::showModal()
{
  TCHAR *dialogName = (TCHAR *)m_resourceName.c_str();
  int dialogResult = -1;
  HWND parentHwnd = NULL;

  if (m_ctrlParent != NULL) {
    parentHwnd = m_ctrlParent->getWindow();
  }
  dialogResult = DialogBoxParam(GetModuleHandle(NULL), dialogName, parentHwnd,
                                modalDialogProc, (LPARAM)this);
  if (dialogResult == -1) {
    // Error notification
    //
  }
  return dialogResult;
}

BOOL BaseDialog::onInitDialog()
{
  return TRUE;
}

BOOL BaseDialog::onNotify(UINT controlID, LPARAM data)
{
  return TRUE;
}

BOOL BaseDialog::onCommand(UINT controlID, UINT notificationID)
{
  return TRUE;
}

BOOL BaseDialog::onDestroy()
{
  return TRUE;
}

bool BaseDialog::InitCommonControlsEx()
{
  INITCOMMONCONTROLSEX iccsex = {0};
  iccsex.dwICC = ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS |
                 ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS |
                 ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES |
                 ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES;
  iccsex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  return (::InitCommonControlsEx(&iccsex) == TRUE);
}

BOOL CALLBACK BaseDialog::modalDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{  
  BaseDialog *_this;
  if (uMsg == WM_INITDIALOG) {
    _this = (BaseDialog *)lParam;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)_this);
    _this->m_ctrlThis.setWindow(hwnd);
  } else {
    _this = (BaseDialog *)GetWindowLong(hwnd, GWL_USERDATA);
    if (_this == 0) {
      return FALSE;
    }
  }

  switch (uMsg) {
  case WM_INITDIALOG:
    return _this->onInitDialog();
  case WM_NOTIFY:
    return _this->onNotify(LOWORD(wParam), lParam);
  case WM_COMMAND:
    return _this->onCommand(LOWORD(wParam), HIWORD(wParam));
  case WM_DESTROY:
    return _this->onDestroy();
  }

  return FALSE;
}
