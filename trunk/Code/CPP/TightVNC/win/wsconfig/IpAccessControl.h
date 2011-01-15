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

#ifndef _IP_ACCESS_CONTROL_H_
#define _IP_ACCESS_CONTROL_H_

#include "common/tstring.h"

//
// Class contains information about
// host access (allow, deny hosts etc).
//

class IpAccessControl
{
public:

  //
  // Host access type
  //

  enum AccessType {
    ACCESS_TYPE_ALLOW = 0,
    ACCESS_TYPE_DENY  = 1,
    ACCESS_TYPE_QUERY = 2
  };

public:
  IpAccessControl();
  ~IpAccessControl();
public:

  //
  // Method to access protected members
  //

  tstring getPattern() { return m_pattern; }
  void setPattern(tstring value) { m_pattern = value; }
  void setPattern(LPTSTR value) { m_pattern = value; }

  AccessType getAction() { return m_action; }
  void setAction(AccessType value) { m_action = value; }
protected:
  tstring m_pattern;
  AccessType m_action;
};

#endif
