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

#ifndef __UPDATEKEEPER_H__
#define __UPDATEKEEPER_H__

#include "UpdateListener.h"
#include "region/Region.h"
#include "region/Point.h"
#include "UpdateFilter.h"
#include "UpdateContainer.h"
#include "CopyRectDetector.h"
#include "thread/Thread.h"
#include "system/WinTimeMillis.h"
#include "thread/CriticalSection.h"

class UpdateKeeper : public Thread
{
public:
  UpdateKeeper(UpdateFilter *updateFilter, const FrameBuffer *frameBuffer,
               UpdateListener *updateListener);
  ~UpdateKeeper(void);

  void lock()
  {
    m_updContCritSec.enter();
  }

  void unLock()
  {
    m_updContCritSec.leave();
  }

  void addChangedRegion(const rfb::Region *changedRegion);
  void addChangedRect(const Rect *changedRect);

  void addCopyRect(const Rect *copyRect, const Point *src);

  void setScreenSizeChanged();
  void setCursorPosChanged();

  void setExcludedRegion(const rfb::Region *excludedRegion);

  const UpdateContainer *getUpdateContainer() const { return &m_updateContainer; }

  void extract(UpdateContainer *updateContainer);

private:
  void doUpdate()
  {
    if (m_updateListener) {
      m_updateListener->onUpdate();
    }
  }

  virtual void execute();
  void hold(UpdateContainer *updateContainer);

  UpdateFilter *m_updateFilter;
  CopyRectDetector m_copyRectDetector;

  // For getDimension() only
  const FrameBuffer *m_frameBuffer;
  Rect m_borderRect;

  rfb::Region m_excludedRegion;
  CriticalSection m_exclRegCritSec;

  UpdateContainer m_updateContainer;
  CriticalSection m_updContCritSec;

  rfb::Region m_heldChangedRegion;
  WinTimeMillis m_heldTime;
  CriticalSection m_heldRegCritSec;

  UpdateListener *m_updateListener;
};

#endif // __UPDATEKEEPER_H__
