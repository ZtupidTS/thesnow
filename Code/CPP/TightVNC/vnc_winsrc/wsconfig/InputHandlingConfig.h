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

#ifndef _INPUT_HANDLING_CONFIG_H_
#define _INPUT_HANDLING_CONFIG_H_

//
// FIXME: truncate members and methods names
//

class InputHandlingConfig
{
public:
  InputHandlingConfig();
  ~InputHandlingConfig();

  void blockRemoteInputEvents(bool blockEnabled) {
    m_blockRemoteInputEvents = blockEnabled;
  }

  bool isBlockingRemoteInputEvents() { return m_blockRemoteInputEvents; }

  void blockRemoteInputOnLocalActivity(bool blockEnabled) {
    m_blockRemoteInputOnLocalActivity = blockEnabled;
  }

  bool isBlockingRemoteInputOnLocalActivity() {
    return m_blockRemoteInputOnLocalActivity;
  }

  unsigned int getInactivityTimeout() { return m_inactivityTimeout; }
  void setInactivityTimeout(unsigned int value) { m_inactivityTimeout = value; }

  void enableLocalInputDuringClientSession(bool enabled) {
    m_noLocalInputDuringClientSessions = !enabled;
  }

  bool isLocalInputDuringClientSessionEnabled() {
    return !m_noLocalInputDuringClientSessions;
  }

protected:
  bool m_blockRemoteInputEvents;
  bool m_blockRemoteInputOnLocalActivity;
  bool m_noLocalInputDuringClientSessions;

  unsigned int m_inactivityTimeout;
};

#endif
