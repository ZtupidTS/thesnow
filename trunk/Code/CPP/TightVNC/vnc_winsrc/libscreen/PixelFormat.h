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

#ifndef __PIXELFORMAT_H__
#define __PIXELFORMAT_H__

#include "rfb/inttypes.h"

struct PixelFormat
{
  PixelFormat()
  {
    union {
      char test;
      int i;
    } testBigEndian;
    testBigEndian.i = 1;
    bigEndian = (testBigEndian.test == 0);
  }

  bool cmp(const PixelFormat *pf) {
    return bitsPerPixel == pf->bitsPerPixel &&
           colorDepth   == pf->colorDepth &&
           redMax       == pf->redMax &&
           greenMax     == pf->greenMax &&
           blueMax      == pf->blueMax &&
           redShift     == pf->redShift &&
           greenShift   == pf->greenShift &&
           blueShift    == pf->blueShift &&
           bigEndian    == pf->bigEndian;
  }

  unsigned short bitsPerPixel;
  unsigned short colorDepth;

  unsigned short redMax;
  unsigned short greenMax;
  unsigned short blueMax;

  unsigned short redShift;
  unsigned short greenShift;
  unsigned short blueShift;

  bool bigEndian;
};

#endif // __PIXELFORMAT_H__
