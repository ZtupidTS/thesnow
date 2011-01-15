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

#include "JpegCompressor.h"

const int StandardJpegCompressor::ALLOC_CHUNK_SIZE = 65536;
const int StandardJpegCompressor::DEFAULT_JPEG_QUALITY = 75;

//
// Extend jpeg_destination_mgr struct with a pointer to our object.
//

typedef struct {
  struct jpeg_destination_mgr pub;
  StandardJpegCompressor *_this;
} my_destination_mgr;

//
// C-compatible interface to our destination manager. It just obtains
// a pointer to the right object and calls a corresponding C++ member
// function on that object.
//

static void
init_destination(j_compress_ptr cinfo)
{
  my_destination_mgr *dest_ptr = (my_destination_mgr *)cinfo->dest;
  dest_ptr->_this->initDestination();
}

static boolean
empty_output_buffer (j_compress_ptr cinfo)
{
  my_destination_mgr *dest_ptr = (my_destination_mgr *)cinfo->dest;
  return (boolean)dest_ptr->_this->emptyOutputBuffer();
}

static void
term_destination (j_compress_ptr cinfo)
{
  my_destination_mgr *dest_ptr = (my_destination_mgr *)cinfo->dest;
  dest_ptr->_this->termDestination();
}

//
// Constructor and destructor.
//

StandardJpegCompressor::StandardJpegCompressor()
  : m_quality(-1), // make sure (m_quality != n_new_quality)
    m_new_quality(DEFAULT_JPEG_QUALITY),
    m_cdata(0),
    m_cdata_allocated(0),
    m_cdata_ready(0)
{
  // Initialize JPEG compression structure.
  m_cinfo.err = jpeg_std_error(&m_jerr);
  jpeg_create_compress(&m_cinfo);

  // Set up a destination manager.
  my_destination_mgr *dest = new my_destination_mgr;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->_this = this;
  m_cinfo.dest = (jpeg_destination_mgr *)dest;

  // Set up compression parameters. Do not set quality level here,
  // it will be set right before the compression.
  m_cinfo.input_components = 3;
  m_cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&m_cinfo);

  // We prefer speed over quality.
  m_cinfo.dct_method = JDCT_FASTEST;
}

StandardJpegCompressor::~StandardJpegCompressor()
{
  // Free compressed data buffer.
  if (m_cdata)
    free(m_cdata);

  // Clean up the destination manager.
  delete m_cinfo.dest;
  m_cinfo.dest = NULL;

  // Release the JPEG compression structure.
  jpeg_destroy_compress(&m_cinfo);
}

//
// Our implementation of destination manager.
//

void
StandardJpegCompressor::initDestination()
{
  if (!m_cdata) {
    size_t new_size = ALLOC_CHUNK_SIZE;
    m_cdata = (unsigned char *)malloc(new_size);
    m_cdata_allocated = new_size;
  }

  m_cdata_ready = 0;
  m_cinfo.dest->next_output_byte = m_cdata;
  m_cinfo.dest->free_in_buffer =  m_cdata_allocated;
}

bool
StandardJpegCompressor::emptyOutputBuffer()
{
  size_t old_size = m_cdata_allocated;
  size_t new_size = old_size + ALLOC_CHUNK_SIZE;

  m_cdata = (unsigned char *)realloc(m_cdata, new_size);
  m_cdata_allocated = new_size;

  m_cinfo.dest->next_output_byte = &m_cdata[old_size];
  m_cinfo.dest->free_in_buffer = new_size - old_size;

  return true;
}

void
StandardJpegCompressor::termDestination()
{
  m_cdata_ready = m_cdata_allocated - m_cinfo.dest->free_in_buffer;
}

//
// Set JPEG quality level (0..100)
//

void
StandardJpegCompressor::setQuality(int level)
{
  if (level < 0) {
    level = 0;
  } else if (level > 100) {
    level = 100;
  }
  m_new_quality = level;
}

// FIXME: Add proper error handling.
void
StandardJpegCompressor::compress(const void *buf,
                                 const PixelFormat *fmt,
                                 int w, int h, int stride)
{
  bool useQuickConversion =
    (fmt->bitsPerPixel == 32 && fmt->colorDepth == 24 &&
     fmt->redMax == 255 && fmt->greenMax == 255 && fmt->blueMax == 255);

  m_cinfo.image_width = w;
  m_cinfo.image_height = h;

  if (m_new_quality != m_quality) {
    jpeg_set_quality(&m_cinfo, m_new_quality, true);
    m_quality = m_new_quality;
  }

  jpeg_start_compress(&m_cinfo, TRUE);

  const char *src = (const char *)buf;

  // We'll pass up to 8 rows to jpeg_write_scanlines().
  JSAMPLE *rgb = new JSAMPLE[w * 3 * 8];
  JSAMPROW row_pointer[8];
  for (int i = 0; i < 8; i++)
    row_pointer[i] = &rgb[w * 3 * i];

  // Feed the pixels to the JPEG library.
  while (m_cinfo.next_scanline < m_cinfo.image_height) {
    int max_rows = m_cinfo.image_height - m_cinfo.next_scanline;
    if (max_rows > 8) {
      max_rows = 8;
    }
    for (int dy = 0; dy < max_rows; dy++) {
      if (useQuickConversion) {
        convertRow24(row_pointer[dy], src, fmt, w);
      } else {
        convertRow(row_pointer[dy], src, fmt, w);
      }
      src += stride;
    }
    jpeg_write_scanlines(&m_cinfo, row_pointer, max_rows);
  }

  delete[] rgb;

  jpeg_finish_compress(&m_cinfo);
}

void
StandardJpegCompressor::convertRow24(JSAMPLE *dst, const void *src,
                                     const PixelFormat *fmt, int numPixels)
{
  const CARD32 *srcPixels = (const CARD32 *)src;
  while (numPixels--) {
    CARD32 pixel = *srcPixels++;
    *dst++ = (JSAMPLE)(pixel >> fmt->redShift);
    *dst++ = (JSAMPLE)(pixel >> fmt->greenShift);
    *dst++ = (JSAMPLE)(pixel >> fmt->blueShift);
  }
}

// FIXME: Currently, this is a copy of convertRow24, it makes the same
//        assumptions about the pixel format.
void
StandardJpegCompressor::convertRow(JSAMPLE *dst, const void *src,
                                   const PixelFormat *fmt, int numPixels)
{
  if (fmt->bitsPerPixel == 32) {
    const CARD32 *srcPixels = (const CARD32 *)src;
    for (int x = 0; x < numPixels; x++) {
      CARD32 pixel = *srcPixels++;
      *dst++ = (JSAMPLE)((pixel >> fmt->redShift & fmt->redMax) * 255 / fmt->redMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->greenShift & fmt->greenMax) * 255 / fmt->greenMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->blueShift & fmt->blueMax) * 255 / fmt->blueMax);
    }
  } else { // assuming (fmt->bitsPerPixel == 16)
    const CARD16 *srcPixels = (const CARD16 *)src;
    for (int x = 0; x < numPixels; x++) {
      CARD16 pixel = *srcPixels++;
      *dst++ = (JSAMPLE)((pixel >> fmt->redShift & fmt->redMax) * 255 / fmt->redMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->greenShift & fmt->greenMax) * 255 / fmt->greenMax);
      *dst++ = (JSAMPLE)((pixel >> fmt->blueShift & fmt->blueMax) * 255 / fmt->blueMax);
    }
  }
}
