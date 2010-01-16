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

#ifndef __WINDOWSSCREENGRABBER_H__
#define __WINDOWSSCREENGRABBER_H__

#include "windows-lib/winhdr.h"
#ifndef CAPTUREBLT
#define CAPTUREBLT          (DWORD)0x40000000 /* Include layered windows */
#endif

#include "ScreenGrabber.h"
#include "region/Rect.h"
#include "rfb/inttypes.h"

//
// An abstract interface for screen grabbing.
//

/*
  //
  // Usage example:
  //

  ScreenGrabber *frameBuffer;

  // Initialisation
  frameBuffer = new WindowsScreenGrabber;

  Rect grabRect, workRect;
  workRect.setRect(100, 100, 500, 500);
  grabRect.setRect(20, 20, 120, 120); // Relative to the workRect
  frameBuffer->setWorkRect(&workRect);

  // One-time grabbing
  while (!frameBuffer->grab(&grabRect)) {
    if (frameBuffer->getPropertiesChanged()) { // Check desktop properties
      if (!frameBuffer->applyNewProperties()) {
        MessageBox(NULL, _T("Cannot apply new screen properties"), _T("Error"), MB_ICONHAND);
        return 1;
      }
    } else {
      MessageBox(NULL, _T("Cannot grab screen"), _T("Error"), MB_ICONHAND);
      return 1;
    }
  }
*/

class WindowsScreenGrabber :
  public ScreenGrabber
{
public:
  WindowsScreenGrabber(void);
  virtual ~WindowsScreenGrabber(void);

  virtual bool grab(const Rect *rect = 0);

  inline virtual bool getPropertiesChanged();
  inline virtual bool getPixelFormatChanged();
  inline virtual bool getScreenSizeChanged();

  virtual bool applyNewFullScreenRect();
  virtual bool applyNewPixelFormat();
  virtual bool applyNewProperties();

  struct BMI
  {
    BITMAPINFOHEADER bmiHeader;
    UINT32 red;
    UINT32 green;
    UINT32 blue;
  };

  static bool getBMI(BMI *bmi, HDC dc = 0);

protected:
  virtual bool openDIBSection();
  virtual bool closeDIBSection();
  virtual bool grabByDIBSection(const Rect *rect);
  static bool fillPixelFormat(PixelFormat *pixelFormat, const BMI *bmi);

  // Find position of first true bit
  static inline int findFirstBit(const UINT32 bits);

  // Windows specific variebles
  HDC m_destDC, m_screenDC;
  HBITMAP m_hbmOld, m_hbmDIB;

private:
  Dimension m_dibSectionDim;
};

#endif // __WINDOWSSCREENGRABBER_H__
