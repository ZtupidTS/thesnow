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

#include "windows-lib/winhdr.h"
#include "Poller.h"
#include "region/Region.h"
#include "libscreen/DesktopSelector.h"

Poller::Poller(UpdateKeeper *updateKeeper,
               ScreenGrabber *screenGrabber,
               FrameBuffer *backupFrameBuffer,
               CriticalSection *frameBufferCriticalSection)
: UpdateDetector(updateKeeper),
m_screenGrabber(screenGrabber),
m_backupFrameBuffer(backupFrameBuffer),
m_frameBufferCriticalSection(frameBufferCriticalSection),
m_sleepTime(DEFAULT_SLEEP_TIME)
{
  m_pollingRect.setRect(0, 0, 16, 16);
}

Poller::~Poller(void)
{
}

void Poller::execute()
{
  DesktopSelector::selectDesktop();

  FrameBuffer *screenFrameBuffer;

  m_frameBufferCriticalSection->enter();
  screenFrameBuffer = m_screenGrabber->getScreenBuffer();

  rfb::Region fullScreenRegion;
  Rect fullScreenRect(screenFrameBuffer->getDimension().getRect());

  fullScreenRegion.addRect(&fullScreenRect);

  m_updateKeeper->addChangedRegion(&fullScreenRegion);
  m_frameBufferCriticalSection->leave();

  while (!m_terminated) {
    rfb::Region region;

    // Enter to the critical section
    m_frameBufferCriticalSection->enter();

    screenFrameBuffer = m_screenGrabber->getScreenBuffer();
    if (!screenFrameBuffer->cmp(m_backupFrameBuffer)) {
      m_updateKeeper->setScreenSizeChanged();
    } else {
      m_screenGrabber->grab();

      // Polling
      int pollingWidth = m_pollingRect.getWidth();
      int pollingHeight = m_pollingRect.getHeight();
      int screenWidth = screenFrameBuffer->getDimension().width;
      int screenHeight = screenFrameBuffer->getDimension().height;

      Rect scanRect;
      for (int iRow = 0; iRow < screenHeight; iRow += pollingHeight) {
        for (int iCol = 0; iCol < screenWidth; iCol += pollingWidth) {
          scanRect.setRect(iCol, iRow, min(iCol + pollingWidth, screenWidth),
                           min(iRow + pollingHeight, screenHeight));
          if (!screenFrameBuffer->cmpFrom(&scanRect, m_backupFrameBuffer,
                                          scanRect.left, scanRect.top)) {
            region.addRect(scanRect);
          }
        }
      }

      m_updateKeeper->addChangedRegion(&region);
    }

    // Leave from the critical section
    m_frameBufferCriticalSection->leave();

    // Send event
    if (!region.is_empty()) {
      doUpdate();
    }

    Sleep(m_sleepTime);
  }
}
