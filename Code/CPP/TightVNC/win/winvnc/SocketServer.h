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

#ifndef __SOCKETSERVER_H__
#define __SOCKETSERVER_H__

class SocketServer;

#include <list>
#include "vncServer.h"
#include "vncSockConnect.h"

class SocketServer
{
public:
  SocketServer();
  virtual ~SocketServer();

  // Clear the port list, delete all vncSockConnect objects.
  void reset();

  // Replace the list of vncSockConnect objects. Old contents of the list will
  // be lost, and the corresponding vncSockConnect objects will be destroyed.
  // New vncSockConnect objects will be created in accordance with the spec
  // argument.
  // The spec string looks like this: "5901:640x480+0+0,5902:320x240+100+200".
  // It may be null in which case the function will just clear the lists.
  // Port:geometry pairs are separated with commas, and geometry is separated
  // from port with a colon.
  void setListenPorts(vncServer *server, const char *spec);

protected:
  std::list<vncSockConnect *> m_sockConnList;

  // Create new vncSockConnect instance and add its pointer to m_sockConnList.
  // Returns true on success, false on an error.
  bool addListenPort(vncServer *server, const char *spec);
};

#endif // __SOCKETSERVER_H__
