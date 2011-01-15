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

#ifndef _SETTINGS_MANAGER_
#define _SETTINGS_MANAGER_

class SettingsManager
{
public:
  SettingsManager(void);
  ~SettingsManager(void);
public:  
  virtual bool isOk() { return true; };

  virtual bool keyExist(LPCTSTR name) { return false; };
  virtual bool deleteKey(LPCTSTR name) { return false; };

  virtual bool getString(LPCTSTR name, LPTSTR value) { return false; };
  virtual bool setString(LPCTSTR name, LPTSTR value) { return false; };

  virtual bool getLong(LPCTSTR name, long *value) { return false; };
  virtual bool setLong(LPCTSTR name, long value) { return false; };

  static void setInstance(SettingsManager *manager) { m_instance = manager; }
  static SettingsManager *getInstance() { return m_instance; }
protected:
  static SettingsManager *m_instance;
};

#endif
