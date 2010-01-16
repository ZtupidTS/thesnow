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

#ifndef _PORT_MAPPING_DIALOG_H_
#define _PORT_MAPPING_DIALOG_H_

#include "ui/BaseDialog.h"
#include "ui/TextBox.h"
#include "PortMapping.h"

typedef enum {
  Add   = 0x0,
  Edit  = 0x1
} PortMappingDialogType;

class PortMappingDialog : public BaseDialog
{
public:
  PortMappingDialog();
  ~PortMappingDialog();
public:
  PortMapping getMapping() { return m_mapping; }
  void setMapping(PortMapping mapping) { m_mapping = mapping; }
  PortMappingDialogType getDialogType() { return m_dialogType; }
  void setDialogType(PortMappingDialogType dialogType) { m_dialogType = dialogType; }
protected:
  void initControls();
  bool isUserDataValid();
  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT cID, UINT nID);
  void onOkButtonClick();
  void onCancelButtonClick();
protected:
  TextBox m_geometryTextBox;
  TextBox m_portTextBox;
  PortMappingDialogType m_dialogType;
  PortMapping m_mapping;
};

#endif
