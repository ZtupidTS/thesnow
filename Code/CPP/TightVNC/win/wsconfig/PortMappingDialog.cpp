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
#include "Resource.h"
#include "common/StringParser.h"
#include "PortMappingDialog.h"

PortMappingDialog::PortMappingDialog()
: m_dialogType(Add)
{
  setResourceName(_T("WSConfig.EditPortMapping"));
}

PortMappingDialog::~PortMappingDialog()
{
}

void PortMappingDialog::onCancelButtonClick()
{
  kill(IDCANCEL);
}

void PortMappingDialog::onOkButtonClick()
{
  if (!isUserDataValid())
    return ;
  Rect::parse((TCHAR *)m_geometryTextBox.getText().c_str(), &m_mapping.rect);
  StringParser::parseInt(m_portTextBox.getText(), &m_mapping.port);
  kill(IDOK);
}

void PortMappingDialog::initControls()
{
  HWND dialogHwnd = m_ctrlThis.getWindow();
  m_geometryTextBox.setWindow(GetDlgItem(dialogHwnd, IDC_GEOMETRY_EDIT));
  m_portTextBox.setWindow(GetDlgItem(dialogHwnd, IDC_PORT_EDIT));
}

bool PortMappingDialog::isUserDataValid()
{
  if (!Rect::tryParse((TCHAR *)m_geometryTextBox.getText().c_str())) {
    MessageBox(m_ctrlThis.getWindow(), _T("Wrong geometry string format"), _T("Error"), MB_OK | MB_ICONWARNING);
    m_geometryTextBox.setFocus();
    return false;
  }
  int port;
  StringParser::parseInt((TCHAR *)m_portTextBox.getText().c_str(), &port);
  if ((port < 1) || (port > 65535)) {
    MessageBox(m_ctrlThis.getWindow(), _T("Port must be between 1 and 65535"), _T("Error"), MB_OK | MB_ICONWARNING);
    m_portTextBox.setFocus();
    return false;
  }
  return true;
}

BOOL PortMappingDialog::onInitDialog()
{
  initControls();
  if (m_dialogType == Add) {
    m_portTextBox.setText(_T("5901"));
    m_geometryTextBox.setText(_T("640x480+0+0"));
  }
  if (m_dialogType == Edit) {
    TCHAR str[10];
    _ltot(m_mapping.port, &str[0], 10);
    m_portTextBox.setText(str);
    m_geometryTextBox.setText((TCHAR *)m_mapping.rect.toString().c_str());
  }
  return TRUE;
}

BOOL PortMappingDialog::onCommand(UINT cID, UINT nID)
{
  switch (cID) {
  case IDOK:
    onOkButtonClick();
    break;
  case IDCANCEL:
    onCancelButtonClick();
    break;
  }
  return TRUE;
}
