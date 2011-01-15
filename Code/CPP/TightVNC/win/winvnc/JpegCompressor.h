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

#ifndef __JPEGCOMPRESSOR_H__
#define __JPEGCOMPRESSOR_H__

#include <stdio.h>
// #include <sys/types.h>
extern "C" {
#include "libjpeg/jpeglib.h"
}

#include "stdhdrs.h"
#include "rfb.h"
#include "libscreen/PixelFormat.h"

//
// An abstract interface for performing JPEG compression.
//

class JpegCompressor
{
public:
  virtual ~JpegCompressor() {}

  // Set JPEG quality level (0..100)
  virtual void setQuality(int level) = 0;

  // Actually compress an image.
  // Note that the stride value is measured in bytes not pixels.
  virtual void compress(const void *buf, const PixelFormat *fmt,
                        int w, int h, int stride) = 0;

  // Access results of the compression.
  virtual size_t getDataLength() = 0;
  virtual const char *getDataPtr() = 0;
};

//
// A C++ class for performing JPEG compression via the
// Independent JPEG Group's software (free JPEG library).
//

class StandardJpegCompressor : public JpegCompressor
{
public:
  StandardJpegCompressor();
  virtual ~StandardJpegCompressor();

  // Set JPEG quality level (0..100)
  virtual void setQuality(int level);

  // Actually compress the image.
  virtual void compress(const void *buf, const PixelFormat *fmt,
                        int w, int h, int stride);

  // Access results of the compression.
  virtual size_t getDataLength() { return m_cdata_ready; }
  virtual const char *getDataPtr() { return (const char *)m_cdata; }

public:
  // Our implementation of JPEG destination manager. These three
  // functions should never be called directly. They are made public
  // because they should be accessible from C-compatible functions
  // called by the JPEG library.
  void initDestination();
  bool emptyOutputBuffer();
  void termDestination();

protected:
  static const int ALLOC_CHUNK_SIZE;
  static const int DEFAULT_JPEG_QUALITY;

  int m_quality;
  int m_new_quality;

  struct jpeg_compress_struct m_cinfo;
  struct jpeg_error_mgr m_jerr;

  unsigned char *m_cdata;
  size_t m_cdata_allocated;
  size_t m_cdata_ready;

  // Convert one row (scanline) from the specified pixel format to the format
  // supported by the IJG JPEG library (one byte per one color component).
  void convertRow(JSAMPLE *dst, const void *src,
                  const PixelFormat *fmt, int numPixels);

  // Convert one row (scanline) from the specified pixel format to the format
  // supported by the IJG JPEG library (one byte per one color component).
  // This is a faster version assuming that source pixels are 32-bit values
  // with actual color depth of 24 (redMax, greenMax and blueMax are all 255).
  void convertRow24(JSAMPLE *dst, const void *src,
                    const PixelFormat *fmt, int numPixels);
};

#endif // __JPEGCOMPRESSOR_H__

