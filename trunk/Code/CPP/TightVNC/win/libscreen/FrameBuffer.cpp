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

#include "FrameBuffer.h"
#include <string.h>

FrameBuffer::FrameBuffer(void)
: m_buffer(0)
{
  memset(&m_pixelFormat, 0, sizeof(m_pixelFormat));
}

FrameBuffer::~FrameBuffer(void)
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
}

bool FrameBuffer::assignProperties(const FrameBuffer *srcFrameBuffer, const bool resizeBuff)
{
  setPixelFormat(&srcFrameBuffer->getPixelFormat(), false);
  setDimension(&srcFrameBuffer->getDimension(), false);

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

bool FrameBuffer::clone(const FrameBuffer *srcFrameBuffer)
{
  if (!assignProperties(srcFrameBuffer)) {
    return false;
  }

  Rect fbRect = &m_dimension.getRect();
  copyFrom(&fbRect, srcFrameBuffer, fbRect.left, fbRect.top);

  return true;
}

bool FrameBuffer::cmp(const FrameBuffer *frameBuffer)
{
  return m_dimension.cmpDim(&frameBuffer->getDimension()) &&
         m_pixelFormat.cmp(&frameBuffer->getPixelFormat());
}

void FrameBuffer::clipRect(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           const int srcX, const int srcY,
                           Rect *dstClippedRect, Rect *srcClippedRect)
{
  Rect dstBufferRect = m_dimension.getRect();
  Rect srcBufferRect = srcFrameBuffer->getDimension().getRect();

  // Building srcRect
  Rect srcRect(srcX, srcY, srcX + dstRect->getWidth(), srcY + dstRect->getHeight());

  // Finding common area between the dstRect, srcRect and the FrameBuffers
  Rect dstRectFB = dstBufferRect.intersection(dstRect);
  Rect srcRectFB = srcBufferRect.intersection(&srcRect);

  // Finding common area between the dstRectFB and the srcRectFB
  Rect dstCommonArea(&dstRectFB);
  Rect srcCommonArea(&srcRectFB);
  // Move to common place (left = 0, top = 0)
  dstCommonArea.move(-dstRect->left, -dstRect->top);
  srcCommonArea.move(-srcRect.left, -srcRect.top);

  Rect commonRect(&dstCommonArea.intersection(&srcCommonArea));

  // Moving commonRect to destination coordinates and source
  dstClippedRect->setRect(&commonRect);
  dstClippedRect->move(dstRect->left, dstRect->top);

  srcClippedRect->setRect(&commonRect);
  srcClippedRect->move(srcRect.left, srcRect.top);
}

bool FrameBuffer::copyFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                           const int srcX, const int srcY)
{
  if (!m_pixelFormat.cmp(&srcFrameBuffer->getPixelFormat())) {
    return false;
  }

  Rect srcClippedRect, dstClippedRect;

  clipRect(dstRect, srcFrameBuffer, srcX, srcY, &dstClippedRect, &srcClippedRect);

  // Shortcuts
  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  int dstStrideBytes = m_dimension.width * pixelSize;
  int srcStrideBytes = srcFrameBuffer->getDimension().width * pixelSize;

  int resultHeight = dstClippedRect.getHeight();
  int resultWidthBytes = dstClippedRect.getWidth() * pixelSize;

  UINT8 *pdst = (UINT8 *)m_buffer
                + dstClippedRect.top * dstStrideBytes
                + pixelSize * dstClippedRect.left;

  UINT8 *psrc = (UINT8 *)srcFrameBuffer->getBuffer()
                + srcClippedRect.top * srcStrideBytes
                + pixelSize * srcClippedRect.left;

  for (int i = 0; i < resultHeight; i++, pdst += dstStrideBytes, psrc += srcStrideBytes) {
    memcpy(pdst, psrc, resultWidthBytes);
  }

  return true;
}

bool FrameBuffer::cmpFrom(const Rect *dstRect, const FrameBuffer *srcFrameBuffer,
                          const int srcX, const int srcY)
{
  if (!m_pixelFormat.cmp(&srcFrameBuffer->getPixelFormat())) {
    return false;
  }

  Rect srcClippedRect, dstClippedRect;

  clipRect(dstRect, srcFrameBuffer, srcX, srcY, &dstClippedRect, &srcClippedRect);

  // Shortcuts
  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  int dstStrideBytes = m_dimension.width * pixelSize;
  int srcStrideBytes = srcFrameBuffer->getDimension().width * pixelSize;

  int resultHeight = dstClippedRect.getHeight();
  int resultWidthBytes = dstClippedRect.getWidth() * pixelSize;

  UINT8 *pdst = (UINT8 *)m_buffer
                + dstClippedRect.top * dstStrideBytes
                + pixelSize * dstClippedRect.left;

  UINT8 *psrc = (UINT8 *)srcFrameBuffer->getBuffer()
                + srcClippedRect.top * srcStrideBytes
                + pixelSize * srcClippedRect.left;

  for (int i = 0; i < resultHeight; i++, pdst += dstStrideBytes, psrc += srcStrideBytes) {
    if (memcmp(pdst, psrc, resultWidthBytes) != 0) {
      return false;
    }
  }

  return true;
}

void FrameBuffer::move(const Rect *dstRect, const int srcX, const int srcY)
{
  Rect srcClippedRect, dstClippedRect;

  clipRect(dstRect, this, srcX, srcY, &dstClippedRect, &srcClippedRect);

  // Data copy
  int pixelSize = m_pixelFormat.bitsPerPixel / 8;
  int strideBytes = m_dimension.width * pixelSize;

  int resultHeight = dstClippedRect.getHeight();
  int resultWidthBytes = dstClippedRect.getWidth() * pixelSize;

  UINT8 *pdst, *psrc;

  if (srcY > dstRect->top) {
    // Pointers set to first string of the rectanles
    pdst = (UINT8 *)m_buffer + dstClippedRect.top * strideBytes
           + pixelSize * dstClippedRect.left;
    psrc = (UINT8 *)m_buffer + srcClippedRect.top * strideBytes
           + pixelSize * srcClippedRect.left;

    for (int i = 0; i < resultHeight; i++, pdst += strideBytes, psrc += strideBytes) {
      memcpy(pdst, psrc, resultWidthBytes);
    }

  } else {
    // Pointers set to last string of the rectanles
    pdst = (UINT8 *)m_buffer + (dstClippedRect.bottom - 1) * strideBytes
           + pixelSize * dstClippedRect.left;
    psrc = (UINT8 *)m_buffer + (srcClippedRect.bottom - 1) * strideBytes
           + pixelSize * srcClippedRect.left;

    for (int i = resultHeight - 1; i >= 0; i--, pdst -= strideBytes, psrc -= strideBytes) {
      memmove(pdst, psrc, resultWidthBytes);
    }
  }
}

bool FrameBuffer::setPixelFormat(const PixelFormat *pixelFormat, bool resizeBuff)
{
  m_pixelFormat = *pixelFormat;

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

bool FrameBuffer::setDimension(const Dimension *newDim, bool resizeBuff)
{
  m_dimension = *newDim;

  if (resizeBuff)
  {
    return resizeBuffer();
  }
  return true;
}

int FrameBuffer::getBufferSize() const
{ 
  return (m_dimension.area() * m_pixelFormat.bitsPerPixel) / 8;
}

bool FrameBuffer::resizeBuffer()
{
  if (m_buffer != 0) {
    delete []m_buffer;
  }
  if ((m_buffer = new UINT8[getBufferSize()]) == 0) {
    return false;
  }
  return true;
}
