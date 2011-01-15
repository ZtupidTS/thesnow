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

#ifndef __WINDOWSMOUSEGRABBER_H__
#define __WINDOWSMOUSEGRABBER_H__

#include "MouseGrabber.h"
#include "windows-lib/winhdr.h"

class WindowsMouseGrabber : public MouseGrabber
{
public:
  WindowsMouseGrabber(void);
  virtual ~WindowsMouseGrabber(void);

  virtual bool grab(PixelFormat *pixelFormat);

  virtual bool isCursorShapeChanged();

protected:
  bool grabPixels(PixelFormat *pixelFormat);

  HCURSOR getHCursor();

  static void inverse(char *bits, int count);
  static void fixCursorShape(FrameBuffer *pixels, char *maskAND, char *maskXOR);
  static bool testBit(char byte, int index);

  HCURSOR m_lastHCursor;
};

#endif // __WINDOWSMOUSEGRABBER_H__
