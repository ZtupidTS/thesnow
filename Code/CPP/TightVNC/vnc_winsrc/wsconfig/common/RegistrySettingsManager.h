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

#ifndef _REGISTRY_SETTINGS_MANAGER_
#define _REGISTRY_SETTINGS_MANAGER_

#include "SettingsManager.h"

class RegistrySettingsManager : public SettingsManager
{
public:
  RegistrySettingsManager(void);
  ~RegistrySettingsManager(void);

public:  
  virtual bool isOk();

  virtual bool keyExist(LPCTSTR name);
  virtual bool deleteKey(LPCTSTR name);

  virtual bool getString(LPCTSTR name, LPTSTR value);
  virtual bool setString(LPCTSTR name, LPTSTR value);

  virtual bool getLong(LPCTSTR name, long *value);
  virtual bool setLong(LPCTSTR name, long value);

  void setRootHKEY(HKEY key);
  HKEY getRootHKEY();

  void setRootFolderName(LPCTSTR folderName);
  LPTSTR getRootFolderName() { return m_appFolderName; }

protected:
  HKEY m_systemRootKey;
  HKEY m_appRootKey;
  LPTSTR m_appFolderName;

  LPTSTR getFolderName(LPCTSTR key);
  LPTSTR getKeyName(LPCTSTR key);

};

#endif
