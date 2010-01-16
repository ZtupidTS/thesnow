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

#include "WindowsScreenJpegEncoder.h"

WindowsScreenJpegEncoder::WindowsScreenJpegEncoder()
: m_headerLength(0)
{
}

WindowsScreenJpegEncoder::~WindowsScreenJpegEncoder()
{
}

void WindowsScreenJpegEncoder::setQuality(int level)
{
  if (level < 0) {
    level = 0;
  } else if (level > 9) {
    level = 9;
  }
  m_compressor.setQuality(level * 10 + 5);
}

UINT WindowsScreenJpegEncoder::getNumCodedRects(const RECT &rect) const
{
  return 1;
}

void WindowsScreenJpegEncoder::encodeRectangle(const FrameBuffer *fb, const RECT &rect)
{
  Rect r(rect.left, rect.top, rect.right, rect.bottom);

  PixelFormat fmt = fb->getPixelFormat();
  int bytesPerPixel = fmt.bitsPerPixel / 8;
  const char *ptr = (const char *)fb->getBuffer();
  ptr += (r.top * fb->getDimension().width + r.left) * bytesPerPixel;
  int stride = fb->getDimension().width * bytesPerPixel;

  m_compressor.compress(ptr, &fmt, r.getWidth(), r.getHeight(), stride);

  encodeLength(getDataLength());
}

void WindowsScreenJpegEncoder::encodeLength(int compressedLen)
{
  m_headerLength = 0;

  m_header[m_headerLength++] = (char)0x90;
  m_header[m_headerLength++] = compressedLen & 0x7F;
  if (compressedLen > 0x7F) {
    m_header[m_headerLength-1] |= 0x80;
    m_header[m_headerLength++] = compressedLen >> 7 & 0x7F;
    if (compressedLen > 0x3FFF) {
      m_header[m_headerLength-1] |= 0x80;
      m_header[m_headerLength++] = compressedLen >> 14 & 0xFF;
    }
  }
}
