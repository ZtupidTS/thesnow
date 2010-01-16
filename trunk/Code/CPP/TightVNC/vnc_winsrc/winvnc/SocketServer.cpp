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

#include <string.h>
#include "stdhdrs.h"
#include "SocketServer.h"

SocketServer::SocketServer()
{
}

SocketServer::~SocketServer()
{
  reset();
}

void SocketServer::reset()
{
  std::list<vncSockConnect *>::iterator i;
  for (i = m_sockConnList.begin(); i != m_sockConnList.end(); i++) {
    delete *i;
  }
  m_sockConnList.clear();
}

void SocketServer::setListenPorts(vncServer *server, const char *spec)
{
  reset();

  // Parse spec, call addListenPort() for each port:geometry pair.
  const char *ptr = spec;
  while (ptr != 0) {
    const char *colonPtr = strchr(ptr, ',');
    int len = (colonPtr != 0) ? (colonPtr - ptr) : strlen(ptr);
    char *part = new char[len + 1];
    if (part) {
      memcpy(part, ptr, len);
      part[len] = '\0';
      (void)addListenPort(server, part);
      delete[] part;
    }
    ptr = colonPtr;
    if (ptr != 0) {
      ptr++;  // skip colon character
    }
  }
}

bool SocketServer::addListenPort(vncServer *server, const char *spec)
{
  unsigned int port, w, h;
  int x, y;
  int numFields = sscanf(spec, "%u:%ux%u+%d+%d", &port, &w, &h, &x, &y);
  if (numFields != 5 || port == 0 || w == 0 || h == 0) {
    return false;
  }

  vncSockConnect *listener = new vncSockConnect;
  RECT viewport;
  SetRect(&viewport, x, y, x + w, y + h);
  if (!listener->Init(server, port, &viewport)) {
    delete listener;
    return false;
  }

  m_sockConnList.push_back(listener);
  return true;
}
