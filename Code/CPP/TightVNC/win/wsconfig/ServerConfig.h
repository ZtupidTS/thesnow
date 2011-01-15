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

#ifndef _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include "common/tstring.h"
#include "Rect.h"

//
// FIXME: truncate members and methods names
//

class ServerConfig
{
public:

  //
  // Enum defines server action when last client disconnects
  // from VNC server.
  //

  enum DisconnectAction {
    DA_DO_NOTHING = 0,
    DA_LOCK_WORKSTATION = 1,
    DA_LOGOUT_WORKSTATION = 2
  };

  //
  // Client connection priority enum
  //

  enum ClientConnectionPriority {
    CCP_DISCONNECT_EXISTING_CONNECTIONS = 0,
    CCP_AUTOMATIC_SHARED_SESSIONS = 1,
    CCP_REFUSE_CONCURENT_CONNECTIONS = 2
  };

public:
  ServerConfig(void);
  ~ServerConfig(void);

  //
  // Display (or port numbers) group
  //

  void setDisplayNumber(int displayNum) {
    m_vncPort = 5900 + displayNum;
    m_httpPort = 5800 + displayNum;
  }

  void setVncPort(int port) { m_vncPort = port; }
  int getVncPort() { return m_vncPort; }

  void setHttpPort(int port) { m_httpPort = port; }
  int getHttpPort() { return m_httpPort; }

  bool isBlackScreenOnClientConnectionEnabled() {
    return m_blankScreenOnClientConnections;
  }

  //
  // Other server options access methods
  //

  void enableFileTransfers(bool enabled) { m_fileTransfersEnabled = enabled; }
  bool isFileTransfersEnabled() { return m_fileTransfersEnabled; }

  //
  // FIXME: What is this???
  //

  void enableRemovingDesktopWallpaper(bool enabled) { m_allowRemovingDesktopWallpaper = enabled; }
  bool isRemovingDesktopWallpaperEnabled() { return m_allowRemovingDesktopWallpaper; }

  void setClientDisconnectAction(DisconnectAction action) {
    m_disconnectAction = action;
  }

  DisconnectAction getClientDisconnectAction() {
    return m_disconnectAction;
  }

  //
  // Incoming connections options group
  //

  bool isSocketConnectionsAllowed() { return m_allowSocketConnections; }
  void enableSocketConnections(bool enabled) { m_allowSocketConnections = enabled; }

  //
  // FIXME: Keep it just as string in memory without encription?
  //

  tstring getPassword() { return m_password; }
  void setPassord(tstring value) { m_password = value; }

  //
  // Settings from Administration tab
  //

  bool isEmptyPasswordAllowed(bool enabled) {
    return !m_disableEmptyPasswords;
  }

  void allowEmptyPassword(bool enabled) {
    m_disableEmptyPasswords = !enabled;
  }

  bool isOnlyLoopbackConnectionsAllowed() {
    return m_allowOnlyLoopbackConnections;
  }

  void enableOnlyLoopbackConnections(bool enabled) {
    m_allowOnlyLoopbackConnections = enabled;
  }

  bool isHttpServerEnabled() {
    return m_httpServerEnabled;
  }
  void enableHttpServer(bool enabled) {
    m_httpServerEnabled = enabled;
  }

  bool isAppletParamInUrlEnabled() {
    return m_appletParamInUrlEnabled;
  }
  void enableAppletParamInUrl(bool enabled) {
    m_appletParamInUrlEnabled = enabled;
  }

  ClientConnectionPriority getClientConnectionPriority() {
    return m_clientConnectionPriority;
  }
  void setClientConnectionPriority(ClientConnectionPriority priority) {
    m_clientConnectionPriority = priority;
  }

  int getLogLevel() { return m_logLevel; }
  void setLogLevel(int logLevel) { m_logLevel = logLevel; }

protected:

  //
  // Server port numbers
  //

  int m_vncPort;
  int m_httpPort;

  bool m_blankScreenOnClientConnections;

  //
  // Other server options members group
  //

  bool m_fileTransfersEnabled;
  bool m_allowRemovingDesktopWallpaper;

  //
  // Server action when last client disconnects from server
  //

  DisconnectAction m_disconnectAction;

  //
  // Incoming connections options group
  //

  bool m_allowSocketConnections;
  tstring m_password;

  //
  // Settings from Administration tab
  //

  bool m_disableEmptyPasswords;
  bool m_allowOnlyLoopbackConnections;
  bool m_httpServerEnabled;
  bool m_appletParamInUrlEnabled;
  ClientConnectionPriority m_clientConnectionPriority;
  int m_logLevel;
};

#endif
