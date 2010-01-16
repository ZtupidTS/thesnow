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
#include "RegistrySettingsManager.h"
#include "common/tstring.h"

RegistrySettingsManager::RegistrySettingsManager(void)
{
  m_systemRootKey = HKEY_CURRENT_USER;
  m_appRootKey = NULL;
  m_appFolderName = _tcsdup(_T("defaultFolder"));
}

void RegistrySettingsManager::setRootHKEY(HKEY key)
{
  m_systemRootKey = key;
}

HKEY RegistrySettingsManager::getRootHKEY()
{
  return m_systemRootKey;
}


void RegistrySettingsManager::setRootFolderName(LPCTSTR folderName)
{
  free(m_appFolderName);
  m_appFolderName = _tcsdup(folderName);
}

RegistrySettingsManager::~RegistrySettingsManager(void)
{
  if (m_appRootKey != NULL) {
    RegCloseKey(m_appRootKey);
  }
}

bool RegistrySettingsManager::isOk()
{
  if (m_appRootKey != NULL) return true;

  if (ERROR_SUCCESS != RegOpenKey(m_systemRootKey, m_appFolderName, &m_appRootKey)) {
    if (ERROR_SUCCESS != RegCreateKey(m_systemRootKey, m_appFolderName, &m_appRootKey)) {
//      debug(_T("Cannot create main entry for RegistrySettingsManager"));
      return false;
    }
  }
  return true;
}

// FIXME: CHECK ON LEMORY LEAK
LPTSTR RegistrySettingsManager::getFolderName(LPCTSTR key)
{
  tstring str = key;
  int len = _tcslen(key);
  int n = str.find_last_of('\\',len);

  TCHAR *res = new TCHAR[len];
  if (n != -1) {
    _tcscpy(res, str.substr(0,n).c_str());
  }
  else {
    _tcscpy(res, _T(""));
  }

  return res;
}

// FIXME: CHECK ON LEMORY LEAK
LPTSTR RegistrySettingsManager::getKeyName(LPCTSTR key)
{
  tstring str = key;
  int len = _tcslen(key);
  int n = str.find_last_of('\\',len);

  TCHAR *res = new TCHAR[len + 1];
  _tcscpy(res, str.substr(0,n == -1 ? len : n).c_str());

  if (n == -1) return res;

  _tcscpy(res, str.substr(n+1,len).c_str());

  return res;
}


bool RegistrySettingsManager::keyExist(LPCTSTR name)
{  
  if (!isOk()) return false;
  
  HKEY keyResult;
  if (ERROR_SUCCESS != RegOpenKey(m_appRootKey, name, &keyResult)) {
    return false;
  }
  RegCloseKey(keyResult);

  return true;
}

bool RegistrySettingsManager::deleteKey(LPCTSTR name)
{
  if (!isOk()) return false;
  return true;
}

bool RegistrySettingsManager::getString(LPCTSTR name, LPTSTR value)
{
  if (!isOk()) return false;
  
  bool bError = false;

  DWORD lpcbData = 1023*2;
  DWORD dataType = REG_SZ;

  HKEY hkey = NULL;

  TCHAR *keyName;
  TCHAR *folderName;

  keyName = getKeyName(name);
  folderName = getFolderName(name);

  if (_tcscmp(folderName, _T("")) == 0) {
    hkey = m_appRootKey;
  }
  else {
    if (ERROR_SUCCESS != RegOpenKey(m_appRootKey, folderName, &hkey)) {
      bError = true;
    }
  }

  if (!bError) {
    if (ERROR_SUCCESS != RegQueryValueEx(hkey, keyName, NULL, &dataType, (LPBYTE)value, &lpcbData)) {
      bError = true;
    }
  }
  
  delete []keyName;
  delete []folderName;

  if (hkey != m_appRootKey) {
    RegCloseKey(hkey);
  }

  return !bError;
}

bool RegistrySettingsManager::setString(LPCTSTR name, LPTSTR value)
{
  if (!isOk()) return false;

  bool bError = false;

  DWORD lpcbData = (_tcslen(value)+1)*sizeof(TCHAR);
  DWORD dataType = REG_SZ;

  HKEY hkey = NULL;

  TCHAR *keyName;
  TCHAR *folderName;

  keyName = getKeyName(name);
  folderName = getFolderName(name);

  if (_tcscmp(folderName, _T("")) == 0) {
    hkey = m_appRootKey;
  }
  else {
    if (ERROR_SUCCESS != RegOpenKey(m_appRootKey, folderName, &hkey)) {
      if (ERROR_SUCCESS != RegCreateKey(m_appRootKey, folderName, &hkey)) {
        bError = true;
      }
    }
  }

  if (ERROR_SUCCESS != RegSetValueEx(hkey, keyName, 0,  dataType, (BYTE*)&value[0], lpcbData)) {
    bError = true;
  }
  
  delete []keyName;
  delete []folderName;

  if (hkey != m_appRootKey) {
    RegCloseKey(hkey);
  }

  return true;
}

bool RegistrySettingsManager::getLong(LPCTSTR name, long *value)
{
  if (!isOk()) return false;
  
  bool bError = false;

  DWORD lpcbData = sizeof(DWORD);
  DWORD dataType = REG_DWORD;

  TCHAR *keyName;
  TCHAR *folderName;

  HKEY hkey = NULL;

  keyName = getKeyName(name);
  folderName = getFolderName(name);

  if (ERROR_SUCCESS != RegOpenKey(m_appRootKey, folderName, &hkey)) {
    bError = true;
  }

  if (ERROR_SUCCESS != RegQueryValueEx(hkey, keyName, NULL, &dataType, (LPBYTE)value, &lpcbData)) {
    bError = true;
  }
  
  delete []keyName;
  delete []folderName;

  RegCloseKey(hkey);

  return !bError;

}

bool RegistrySettingsManager::setLong(LPCTSTR name, long value)
{
  if (!isOk()) return false;
  
  bool bError = false;

  DWORD lpcbData = sizeof(DWORD);
  DWORD dataType = REG_DWORD;

  TCHAR *keyName;
  TCHAR *folderName;

  HKEY hkey = NULL;

  keyName = getKeyName(name);
  folderName = getFolderName(name);

  if (ERROR_SUCCESS != RegOpenKey(m_appRootKey, folderName, &hkey)) {
    if (ERROR_SUCCESS != RegCreateKey(m_appRootKey, folderName, &hkey)) {
      bError = true;
    }
  }

  if (ERROR_SUCCESS != RegSetValueEx(hkey, keyName, NULL, dataType, (LPBYTE)&value, lpcbData)) {
    bError = true;
  }
  
  delete []keyName;
  delete []folderName;

  RegCloseKey(hkey);

  return !bError;
}
