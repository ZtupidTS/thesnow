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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "common/SettingsManager.h"
#include "PortMappingVector.h"
#include "IpAccessControlContainer.h"
#include "ServerConfig.h"
#include "QueryConfig.h"
#include "InputHandlingConfig.h"

class Settings
{
public:
  Settings();
public:
  bool saveToStorage(SettingsManager *sm);
  bool loadFromStorage(SettingsManager *sm);

  //
  // Protected members read methods
  //

  PortMappingVector &getPortMappingContainer() { return m_vPortMapping; }

  IpAccessControlContainer getAccessControlContainer() {
    return m_accessControlContainer;
  }

  ServerConfig &getServerConfig() { return m_serverConfig; }

  QueryConfig &getQueryConfig() { return m_queryConfig; }

  InputHandlingConfig &getInputConfig() { return m_inputConfig; }
private:

  //
  // Serialize and deserialize methods
  //

  bool savePortMappingContainerToStorage(SettingsManager *sm);
  bool loadPortMappingContainerFromStorage(SettingsManager *sm, PortMappingVector *portMapping);

  bool saveIpAccessControlContainerToStorage(SettingsManager *sm);
  bool loadIpAccessControlContainerFromStorage(SettingsManager *sm,
                                               IpAccessControlContainer *ipContainer);

  bool saveServerConfigToStorage(SettingsManager *sm);
  bool loadServerConfigFromStorage(SettingsManager *sm, ServerConfig *config);

  bool saveQueryConfigToStorage(SettingsManager *sm);
  bool loadQueryConfigFromStorage(SettingsManager *sm, QueryConfig *config);

  bool saveInputHandlingConfigToStorage(SettingsManager *sm);
  bool loadInputHandlingConfigFromStorage(SettingsManager *sm,
                                          InputHandlingConfig *config);

protected:
  PortMappingVector m_vPortMapping;
  IpAccessControlContainer m_accessControlContainer;
  ServerConfig m_serverConfig;
  QueryConfig m_queryConfig;

  //
  // FIXME: Maybe move this member to ServerConfig class?
  //

  InputHandlingConfig m_inputConfig;
};

#endif
