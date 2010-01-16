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

#include "MouseDetector.h"
#include "libscreen/DesktopSelector.h"

#define MOUSE_SLEEP_TIME 10

MouseDetector::MouseDetector(UpdateKeeper *updateKeeper)
: UpdateDetector(updateKeeper)
{
  m_sleepTime = MOUSE_SLEEP_TIME;
}

MouseDetector::~MouseDetector(void)
{
}

void MouseDetector::execute()
{
  DesktopSelector::selectDesktop();

  POINT curPoint;

  while (!m_terminated) {
    GetCursorPos(&curPoint);
    if (m_lastCursorPos.x != curPoint.x || m_lastCursorPos.y != curPoint.y) {
      m_lastCursorPos.x = curPoint.x;
      m_lastCursorPos.y = curPoint.y;

      m_updateKeeper->setCursorPosChanged();

      doUpdate();
    }

    Sleep(m_sleepTime);
  }
}
