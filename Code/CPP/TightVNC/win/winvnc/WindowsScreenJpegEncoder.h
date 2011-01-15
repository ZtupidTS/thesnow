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

#ifndef __WINDOWSSCREENJPEGENCODER_H__
#define __WINDOWSSCREENJPEGENCODER_H__

#include "stdhdrs.h"
#include "rfb.h"
#include "libscreen/FrameBuffer.h"
#include "JpegCompressor.h"

// FIXME: Tight-encoded rectangles should not be wider than 2048 pixels.
// FIXME: Fall back to other encoders if bits-per-pixel is 8.
// FIXME: Don't use this encoder if the client does not support Tight+JPEG.

class WindowsScreenJpegEncoder
{
public:
  WindowsScreenJpegEncoder();
  virtual ~WindowsScreenJpegEncoder();

  void setQuality(int level);
  UINT getNumCodedRects(const RECT &rect) const;
  void encodeRectangle(const FrameBuffer *fb, const RECT &rect);

  // Access results of the compression.
  size_t getHeaderLength() { return m_headerLength; }
  const char *getHeaderPtr() { return m_header; }
  size_t getDataLength() { return m_compressor.getDataLength(); }
  const char *getDataPtr() { return m_compressor.getDataPtr(); }

private:
  void encodeLength(int compressedLen);

  StandardJpegCompressor m_compressor;
  char m_header[4];
  size_t m_headerLength;
};

#endif // __WINDOWSSCREENJPEGENCODER_H__
