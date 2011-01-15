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

#ifndef __UPDATEDETECTOR_H__
#define __UPDATEDETECTOR_H__

#include "UpdateKeeper.h"
#include "thread/Thread.h"
#include "UpdateListener.h"

class UpdateDetector : public Thread
{
public:
  UpdateDetector(UpdateKeeper *updateKeeper);
  virtual ~UpdateDetector(void);

  void setUpdateKeeper(UpdateKeeper *updateKeeper) { m_updateKeeper = updateKeeper; }
  UpdateKeeper *getUpdateKeeper() const { return m_updateKeeper; }

  void setOutUpdateListener(UpdateListener *updateListener)
  { 
    m_updateListener = updateListener;
  }

protected:
  void doUpdate()
  {
    if (m_updateListener) {
      m_updateListener->onUpdate();
    }
  }

  UpdateKeeper *m_updateKeeper;

  UpdateListener *m_updateListener;
};

#endif // __UPDATEDETECTOR_H__
