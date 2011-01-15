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

#ifndef __UPDATEFILTER_H__
#define __UPDATEFILTER_H__

#include "libscreen/WindowsScreenGrabber.h"
#include "libscreen/FrameBuffer.h"
#include "thread/CriticalSection.h"
#include "UpdateContainer.h"

class UpdateFilter
{
public:
  UpdateFilter(ScreenGrabber *screenGrabber,
               FrameBuffer *frameBuffer,
               CriticalSection *frameBufferCriticalSection);
  ~UpdateFilter(void);

  void filter(UpdateContainer *updateContainer);

private:
  void getChangedRegion(rfb::Region &rgn, const Rect &rect);
  void updateChangedRect(rfb::Region &rgn, const Rect &rect);
  void updateChangedSubRect(rfb::Region &rgn, const Rect &rect);

  ScreenGrabber *m_screenGrabber;
  FrameBuffer *m_frameBuffer;
  CriticalSection *m_frameBufferCriticalSection;
};

#endif // __UPDATEFILTER_H__
