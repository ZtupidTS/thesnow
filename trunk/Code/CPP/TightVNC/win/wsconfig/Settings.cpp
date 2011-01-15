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
#include "Settings.h"

Settings::Settings()
{
}

bool Settings::saveToStorage(SettingsManager *sm)
{
  bool saveResult = true;
  // Save Port Mapping Vector
  tstring strPortMapping = _T("");
  for (size_t i = 0; i < m_vPortMapping.size(); i++) {
    strPortMapping += m_vPortMapping.at(i).toString();
    if (i != m_vPortMapping.size() - 1)
      strPortMapping += _T(",");
  }
  if (!sm->setString(_T("ExtraPorts"), (TCHAR *)strPortMapping.c_str())) {
    saveResult = false;
  }
  return saveResult;
}

bool Settings::loadFromStorage(SettingsManager *sm)
{
  bool loadResult = true;
  TCHAR strVal[1024*4];
  if (!sm->getString(_T("ExtraPorts"), &strVal[0])) {
    loadResult = false;
  }
  else {
    tstring strMappings = strVal;
    size_t index = 0;
    size_t lastIndex = 0;
    while (true) {
      index = strMappings.find(_T(","), lastIndex);
      if (index == tstring::npos) {
        break;
      }
      tstring strMapping = strMappings.substr(lastIndex, index - lastIndex);
      PortMapping newPortMapping;
      if (PortMapping::parse(strMapping, &newPortMapping)) {
        m_vPortMapping.push_back(newPortMapping);
      }
      lastIndex = index + 1;
    }
    tstring strMapping = strMappings.substr(lastIndex, strMappings.size() - lastIndex);
    PortMapping newPortMapping;
    if (PortMapping::parse(strMapping, &newPortMapping)) {
      m_vPortMapping.push_back(newPortMapping);
    }
  }
  return loadResult;
}
