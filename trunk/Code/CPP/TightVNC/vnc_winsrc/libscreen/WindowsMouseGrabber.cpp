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

#include "WindowsMouseGrabber.h"
#include "WindowsScreenGrabber.h"

WindowsMouseGrabber::WindowsMouseGrabber(void)
: m_lastHCursor(0)
{
  m_cursorShape.mask = 0;
}

WindowsMouseGrabber::~WindowsMouseGrabber(void)
{
  if (m_cursorShape.mask != 0) {
    delete []m_cursorShape.mask;
  }
}

bool WindowsMouseGrabber::isCursorShapeChanged()
{
  HCURSOR hCursor = getHCursor();
  if (hCursor == m_lastHCursor) {
    return false;
  }
  m_lastHCursor = hCursor;

  return true;
}

bool WindowsMouseGrabber::grab(PixelFormat *pixelFormat)
{
  return grabPixels(pixelFormat);
}

bool WindowsMouseGrabber::grabPixels(PixelFormat *pixelFormat)
{
  HCURSOR hCursor = getHCursor();
  if (hCursor == 0) {
    return false;
  }
  m_lastHCursor = hCursor;

  // Get bitmap mask
  ICONINFO iconInfo;
  if (!GetIconInfo(hCursor, &iconInfo)) {
    return false;
  }

  if (iconInfo.hbmMask == NULL) {
    return false;
  }

  bool isColorShape = (iconInfo.hbmColor != NULL);

  BITMAP bmMask;
  if (!GetObject(iconInfo.hbmMask, sizeof(BITMAP), (LPVOID)&bmMask)) {
    DeleteObject(iconInfo.hbmMask);
    return false;
  }

  if (bmMask.bmPlanes != 1 || bmMask.bmBitsPixel != 1) {
    DeleteObject(iconInfo.hbmMask);
    return false;
  }

  m_cursorShape.hotSpot.x = iconInfo.xHotspot;
  m_cursorShape.hotSpot.y = iconInfo.yHotspot;

  int width = bmMask.bmWidth;
  int height = isColorShape ? bmMask.bmHeight : bmMask.bmHeight/2;
  int widthBytes = bmMask.bmWidthBytes;

  FrameBuffer *pixels= &m_cursorShape.pixels;

  pixels->setDimension(&Dimension(width, height), false);
  pixels->setPixelFormat(pixelFormat);

  if (m_cursorShape.mask != 0) {
    delete[] m_cursorShape.mask;
  }
  m_cursorShape.mask = new char[widthBytes * bmMask.bmHeight];
  char *mask = m_cursorShape.mask;

  bool result = GetBitmapBits(iconInfo.hbmMask,
                              widthBytes * bmMask.bmHeight,
                              mask) != 0;

  DeleteObject(iconInfo.hbmMask);
  if (!result) {
    return false;
  }

  // Get cursor pixels
  HDC screenDC = GetDC(0);
  if (screenDC == NULL) {
    return false;
  }

  WindowsScreenGrabber::BMI bmi;
  if (!WindowsScreenGrabber::getBMI(&bmi, screenDC)) {
    return false;
  }

  bmi.bmiHeader.biBitCount = pixelFormat->bitsPerPixel;
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biCompression = BI_BITFIELDS;
  bmi.red   = pixelFormat->redMax   << pixelFormat->redShift;
  bmi.green = pixelFormat->greenMax << pixelFormat->greenShift;
  bmi.blue  = pixelFormat->blueMax  << pixelFormat->blueShift;

  HDC destDC = CreateCompatibleDC(NULL);
  if (destDC == NULL) {
    DeleteDC(screenDC);
    return FALSE;
  }

  void *buffer;
  HBITMAP hbmDIB = CreateDIBSection(destDC, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, &buffer, NULL, NULL);
  if (hbmDIB == 0) {
    DeleteDC(destDC);
    DeleteDC(screenDC);
    return FALSE;
  }

  HBITMAP hbmOld = (HBITMAP)SelectObject(destDC, hbmDIB);

  result = DrawIconEx(destDC, 0, 0, hCursor, 0, 0, 0, NULL, DI_IMAGE) != 0;

  memcpy(pixels->getBuffer(), buffer, pixels->getBufferSize());

  if (!isColorShape) {
    fixCursorShape(pixels, mask, mask + widthBytes * height);
  } else {
    inverse(mask, widthBytes * height);
  }

  SelectObject(destDC, hbmOld);
  DeleteObject(hbmDIB);
  DeleteDC(destDC);
  DeleteDC(screenDC);

  return result;
}

HCURSOR WindowsMouseGrabber::getHCursor()
{
  CURSORINFO cursorInfo;
  cursorInfo.cbSize = sizeof(CURSORINFO);

  if (GetCursorInfo(&cursorInfo) == 0) {
    return false;
  }

  return cursorInfo.hCursor;
}

void WindowsMouseGrabber::inverse(char *bits, int count)
{
  for (int i = 0; i < count; i++, bits++) {
    *bits = ~*bits;
  }
}

void WindowsMouseGrabber::fixCursorShape(FrameBuffer *pixels, char *maskAND, char *maskXOR)
{
  char *pixelsBuffer = (char *)pixels->getBuffer();
  char *pixel;
  int pixelSize = pixels->getPixelFormat().bitsPerPixel / 8;
  int pixelCount = pixels->getBufferSize() / pixelSize;

  char *maskANDByte = maskAND - 1;
  char *maskXORByte = maskXOR - 1;

  bool maskANDBit, maskXORBit;

  for (int iPixel = 0; iPixel < pixelCount; iPixel++) {
    pixel = pixelsBuffer + iPixel * pixelSize;

    if ((iPixel % 8) == 0) {
      // Next byte of maskAND and maskXOR
      maskANDByte++;
      maskXORByte++;
    }

    maskANDBit = testBit(*maskANDByte, iPixel % 8);
    maskXORBit = testBit(*maskXORByte, iPixel % 8);

    if (!maskANDBit && !maskXORBit) { // Black dot
      memset(pixel, 0, pixelSize);
    } else if (!maskANDBit && maskXORBit) { // White dot
      memset(pixel, 0xff, pixelSize);
    } else if (maskANDBit && maskXORBit) { // Inverted
      memset(pixel, 0, pixelSize);
    }
  }

  inverse(maskAND, pixelCount / 8);
  for (int i = 0; i < pixelCount / 8; i++) {
    *(maskAND +i) |= *(maskXOR + i);
  }
}

bool WindowsMouseGrabber::testBit(char byte, int index)
{
  char dummy = 128 >> index;
  return (dummy & byte) != 0;
}
