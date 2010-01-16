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

#include "TabDialogContainer.h"

TabDialogContainer::TabDialogContainer()
{
}

void TabDialogContainer::clear()
{
  m_windowList.clear();
}

void TabDialogContainer::addDialog(HWND hDialog, const char *tabLabel)
{
  m_windowList.push_back(hDialog);
  m_labelList.push_back(tabLabel);
}

HWND TabDialogContainer::getWindow(int tabId) const
{
  if (tabId < 0 || tabId >= (int)m_windowList.size()) {
    return 0;
  }
  return m_windowList[tabId];
}

const char * TabDialogContainer::getLabel(int tabId) const
{
  if (tabId < 0 || tabId >= (int)m_labelList.size()) {
    return 0;
  }
  return m_labelList[tabId];
}
