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

#ifndef _CONFIG_DIALOG_H_
#define _CONFIG_DIALOG_H_

#include "common/SettingsManager.h"
#include "common/RegistrySettingsManager.h"
#include "ui/BaseDialog.h"
#include "ui/Control.h"
#include "ui/ListBox.h"
#include "Settings.h"

class ConfigDialog : public BaseDialog
{
public:
  ConfigDialog(void);
  ~ConfigDialog(void);
protected:
  void initControls();
  void loadSettings();
  // Init dialog handler
  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  // Button handlers
  void onAddButtonClick();
  void onEditButtonClick();
  void onRemoveButtonClick();
  void onCancelButtonClick();
  void onOKButtonClick();
  void onApplyButtonClick();
  void onMappingListBoxSelChange();
  void onMappingListBoxDoubleClick();
protected:
  // Controls
  ListBox m_ctrlMappingListBox;
  Control m_ctrlEditButton;
  Control m_ctrlRemoveButton;
  Control m_ctrlApplyButton;
  // Settings
  Settings m_config;
  SettingsManager *m_settingsManager;
};

#endif
