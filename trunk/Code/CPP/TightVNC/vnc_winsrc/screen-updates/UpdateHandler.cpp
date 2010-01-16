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

#include "UpdateHandler.h"
#include "Poller.h"
#include "HooksUpdateDetector.h"
#include "MouseDetector.h"

UpdateHandler::UpdateHandler(UpdateListener *registerUpdateListener)
: m_registerUpdateListener(registerUpdateListener)
{
  m_updateFilter = new UpdateFilter(&m_screenGrabber, &m_backupFrameBuffer,
                                    &m_criticalSection);
  m_updateKeeper = new UpdateKeeper(m_updateFilter,
                                    m_screenGrabber.getScreenBuffer(),
                                    this);
  m_poller = new Poller(m_updateKeeper, &m_screenGrabber,
                        &m_backupFrameBuffer, &m_criticalSection);
  m_poller->setOutUpdateListener(this);
  m_hooks = new HooksUpdateDetector(m_updateKeeper);
  m_hooks->setOutUpdateListener(this);
  m_mouseDetector = new MouseDetector(m_updateKeeper);
  m_mouseDetector->setOutUpdateListener(this);

  executeDetectors();
}

UpdateHandler::~UpdateHandler(void)
{
  terminateDetectors();
  delete m_mouseDetector;
  delete m_poller;
  delete m_hooks;
  delete m_updateKeeper;
  delete m_updateFilter;
}

void UpdateHandler::extract(UpdateContainer *updateContainer)
{
  m_criticalSection.enter();

  m_updateKeeper->extract(updateContainer);

  // Checking for screen properties changing or frame buffers differ
  if (m_screenGrabber.getPropertiesChanged() ||
      !m_backupFrameBuffer.cmp(m_screenGrabber.getScreenBuffer())) {
    if (m_screenGrabber.getScreenSizeChanged()) {
      updateContainer->screenSizeChanged = true;
    }
    m_screenGrabber.applyNewProperties();
    m_backupFrameBuffer.clone(m_screenGrabber.getScreenBuffer());
    updateContainer->changedRegion.clear();
    updateContainer->copiedRegion.clear();
  }

  if (m_mouseGrabber.isCursorShapeChanged()) {
    updateContainer->cursorShapeChanged = true;
    m_mouseGrabber.grab(&m_backupFrameBuffer.getPixelFormat());
  }

  m_criticalSection.leave();
}

void UpdateHandler::setFullUpdateRequested(const rfb::Region *region)
{
  m_updateKeeper->addChangedRegion(region);
}

void UpdateHandler::executeDetectors()
{
  m_backupFrameBuffer.assignProperties(m_screenGrabber.getScreenBuffer());
  m_poller->resume();
  m_hooks->resume();
  m_mouseDetector->resume();
}

void UpdateHandler::terminateDetectors()
{
  m_poller->terminate();
  m_hooks->terminate();
  m_mouseDetector->terminate();
  m_poller->wait();
  m_hooks->wait();
  m_mouseDetector->wait();
}

void UpdateHandler::onUpdate()
{
  m_criticalSection.enter();

  if (!m_updateKeeper->getUpdateContainer()->isEmpty()) {
    m_criticalSection.leave();
    doUpdate();
    return;
  }

  m_criticalSection.leave();
}

bool UpdateHandler::checkForUpdates(rfb::Region *region)
{
  m_updateKeeper->lock();
  UpdateContainer updateContainer = *m_updateKeeper->getUpdateContainer();
  m_updateKeeper->unLock();

  rfb::Region resultRegion = updateContainer.changedRegion;
  resultRegion.assign_union(updateContainer.copiedRegion);

  resultRegion.assign_intersect(*region);

  bool result = updateContainer.cursorPosChanged ||
                updateContainer.cursorShapeChanged ||
                updateContainer.screenSizeChanged ||
                !resultRegion.is_empty();

  return result;
}

void UpdateHandler::setExcludedRegion(const rfb::Region *excludedRegion)
{
  m_updateKeeper->setExcludedRegion(excludedRegion);
}
