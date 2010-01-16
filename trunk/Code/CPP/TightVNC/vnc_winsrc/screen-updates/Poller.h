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

#ifndef __POLLER_H__
#define __POLLER_H__

#include "UpdateDetector.h"
#include "libscreen/WindowsScreenGrabber.h"
#include "libscreen/FrameBuffer.h"
#include "region/Rect.h"

#define DEFAULT_SLEEP_TIME 1000

class Poller : public UpdateDetector
{
public:
  Poller(UpdateKeeper *updateKeeper,
         ScreenGrabber *screenGrabber,
         FrameBuffer *backupFrameBuffer,
         CriticalSection *frameBufferCriticalSection);

  virtual ~Poller(void);

  void setSleepTime(const int sleepTime) { m_sleepTime = sleepTime; }
  int getsleepTime() const { return m_sleepTime; }

protected:
  virtual void execute();

private:
  ScreenGrabber *m_screenGrabber;
  FrameBuffer *m_backupFrameBuffer;
  CriticalSection *m_frameBufferCriticalSection;
  Rect m_pollingRect;
  int m_sleepTime;
};

#endif // __POLLER_H__
