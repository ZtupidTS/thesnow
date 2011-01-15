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

#include "UpdateKeeper.h"
#include "thread/AutoLock.h"

#define SLEEP_TIME 10
#define HOLD_TIME 300

UpdateKeeper::UpdateKeeper(UpdateFilter *updateFilter,
                           const FrameBuffer *frameBuffer,
                           UpdateListener *updateListener)
: m_updateFilter(updateFilter),
  m_frameBuffer(frameBuffer),
  m_updateListener(updateListener)
{
  m_borderRect.setRect(&m_frameBuffer->getDimension().getRect());
  resume();
}

UpdateKeeper::~UpdateKeeper(void)
{
  terminate();
  wait();
}

void UpdateKeeper::addChangedRegion(const rfb::Region *changedRegion)
{
  AutoLock al(&m_updContCritSec);

  // FIXME: Calling assign_subtract() function is correct if use
  // copy region instead of copy rectangle.
  //m_updateContainer.copiedRegion.assign_subtract(*changedRegion);
  m_updateContainer.changedRegion.assign_union(*changedRegion);

  rfb::Region borderRegion(m_borderRect);
  m_updateContainer.changedRegion.assign_intersect(borderRegion);
}

void UpdateKeeper::addChangedRect(const Rect *changedRect)
{
  rfb::Region region(changedRect);
  addChangedRegion(&region);
}

void UpdateKeeper::addCopyRect(const Rect *copyRect, const Point *src)
{
  AutoLock al(&m_updContCritSec);

  if (copyRect->isEmpty()) {
    return;
  }

  rfb::Region *changedRegion = &m_updateContainer.changedRegion;
  rfb::Region *copiedRegion = &m_updateContainer.copiedRegion;
  Point *copySrc = &m_updateContainer.copySrc;
  Rect dstCopyRect(copyRect);

  // Create copy of copyRect in the source coordinates.
  Rect srcCopyRect(copyRect);
  srcCopyRect.setLocation(src->x, src->y);

  // Clipping dstCopyRect
  dstCopyRect = dstCopyRect.intersection(&m_borderRect);
  // Correcting source coordinates
  srcCopyRect.left    += dstCopyRect.left - copyRect->left;
  srcCopyRect.top     += dstCopyRect.top - copyRect->top;
  srcCopyRect.right   += dstCopyRect.right - copyRect->right;
  srcCopyRect.bottom  += dstCopyRect.bottom - copyRect->bottom;
  // Clipping srcCopyRect
  Rect dummySrcCopyRect(&srcCopyRect);
  srcCopyRect = srcCopyRect.intersection(&m_borderRect);
  // Correcting destination coordinates
  dstCopyRect.left    += srcCopyRect.left - dummySrcCopyRect.left;
  dstCopyRect.top     += srcCopyRect.top - dummySrcCopyRect.top;
  dstCopyRect.right   += srcCopyRect.right - dummySrcCopyRect.right;
  dstCopyRect.bottom  += srcCopyRect.bottom - dummySrcCopyRect.bottom;

  if (dstCopyRect.isEmpty()) {
    return;
  }

  copySrc->x = srcCopyRect.left;
  copySrc->y = srcCopyRect.top;

  // Old copiedRegion must be added to changedRegion - (?)
  if (!copiedRegion->is_empty()) {
    changedRegion->assign_union(*copiedRegion);
    copiedRegion->clear();
    addChangedRect(copyRect);
    return;
  }

  copiedRegion->clear();
  copiedRegion->addRect(dstCopyRect);

  // copiedRegion must be substracted from changedRegion
  changedRegion->assign_subtract(*copiedRegion);

  // Create region that is intersection of changedRegion and srcCopyRect.
  rfb::Region addonChangedRegion;
  addonChangedRegion.addRect(srcCopyRect);
  addonChangedRegion.assign_intersect(*changedRegion);

  // Move addonChangedRegion and add it to changedRegion.
  addonChangedRegion.move(dstCopyRect.left - copySrc->x,
                          dstCopyRect.top - copySrc->y);
  changedRegion->assign_union(addonChangedRegion);

  // Clipping regions
  rfb::Region borderRegion(m_borderRect);
  m_updateContainer.changedRegion.assign_intersect(borderRegion);
  m_updateContainer.copiedRegion.assign_intersect(borderRegion);
}

void UpdateKeeper::setScreenSizeChanged()
{
  AutoLock al(&m_updContCritSec);
  m_updateContainer.screenSizeChanged = true;
}

void UpdateKeeper::setCursorPosChanged()
{
  AutoLock al(&m_updContCritSec);
  m_updateContainer.cursorPosChanged = true;
}

void UpdateKeeper::extract(UpdateContainer *updateContainer)
{
  {
    AutoLock al(&m_updContCritSec);

    // Reinitializing m_borderRect
    m_borderRect.setRect(&m_frameBuffer->getDimension().getRect());
    // Clipping regions
    rfb::Region borderRegion(m_borderRect);
    m_updateContainer.changedRegion.assign_intersect(borderRegion);
    m_updateContainer.copiedRegion.assign_intersect(borderRegion);
  }
  {
    Rect copyRect;
    Point copySrc;
    m_copyRectDetector.detectWindowMovements(&copyRect, &copySrc);
    addCopyRect(&copyRect, &copySrc);

    AutoLock al(&m_updContCritSec);

    *updateContainer = m_updateContainer;
    m_updateContainer.clear();
  }
  {
    AutoLock al(&m_exclRegCritSec);
    updateContainer->changedRegion.assign_subtract(m_excludedRegion);
    updateContainer->copiedRegion.assign_subtract(m_excludedRegion);
  }

  hold(updateContainer);

  m_updateFilter->filter(updateContainer);
}

void UpdateKeeper::setExcludedRegion(const rfb::Region *excludedRegion)
{
  AutoLock al(&m_exclRegCritSec);

  if(excludedRegion == 0) {
    m_excludedRegion.clear();
  } else {
    m_excludedRegion = *excludedRegion;
  }
}

void UpdateKeeper::hold(UpdateContainer *updateContainer)
{
  AutoLock alHeldReg(&m_heldRegCritSec);

  // Check for "CopyRect" presence.
  if (!updateContainer->copiedRegion.is_empty()) {
    // Add copiedRegion with destination coordinate
    m_heldChangedRegion.assign_union(updateContainer->copiedRegion);

    // Add copiedRegion with source coordinate
    rfb::Region srcCopiedRegion(updateContainer->copiedRegion);
    Point *src = &updateContainer->copySrc;
    Point dst(srcCopiedRegion.get_bounding_rect().left, srcCopiedRegion.get_bounding_rect().top);

    srcCopiedRegion.move(src->x - dst.x, src->y - dst.y);
    m_heldChangedRegion.assign_union(srcCopiedRegion);

    m_heldTime.update();
  }

  if (!m_heldChangedRegion.is_empty()) {
    // Hold changedRegion
    m_heldChangedRegion.assign_union(updateContainer->changedRegion);
    updateContainer->changedRegion.clear();
  }
}

void UpdateKeeper::execute()
{
  WinTimeMillis currentTime;

  while(!m_terminated) {
    m_heldRegCritSec.enter();

    if (!m_heldChangedRegion.is_empty()) {
      currentTime.update();
      if (currentTime.diffFrom(&m_heldTime) > HOLD_TIME) {
        addChangedRegion(&m_heldChangedRegion);
        m_heldChangedRegion.clear();

        m_heldRegCritSec.leave();
        doUpdate();
        continue;
      }
    }

    m_heldRegCritSec.leave();

    Sleep(SLEEP_TIME);
  }
}
