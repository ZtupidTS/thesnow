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

#ifndef _LIST_BOX_H_
#define _LIST_BOX_H_

#include "Control.h"

class ListBox : public Control
{
public:
  ListBox();
  ~ListBox();
public:
  void addString(const TCHAR *str);
  tstring getItemText(int index);
  void setItemText(int index, TCHAR *str);
  void insertString(int index, const TCHAR *str);
  void insertString(int index, const TCHAR *str, LPARAM data);
  void addString(const TCHAR *str, LPARAM data);
  void setItemData(int index, LPARAM data);
  void removeString(int index);
  int getSelectedIndex();
  int getTopIndex();
  void setTopIndex(int index);
  void setSelectedIndex(int index);
  LPARAM getItemData(int index);
  int getCount();
  void clear();
};

#endif
