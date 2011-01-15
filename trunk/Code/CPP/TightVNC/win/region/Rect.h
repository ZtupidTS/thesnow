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

#ifndef __RECT_H__
#define __RECT_H__

#include "windows-lib/winhdr.h"

class Rect
{
public:
  Rect(void) : left(0), top(0), right(0), bottom(0) {}
  Rect(const Rect *rect) { setRect(rect); }
  Rect(int lt, int tp, int rt, int bm) { setRect(lt, tp, rt, bm); }
  Rect(int width, int height) { setRect(0, 0, width, height); }

  ~Rect(void) {}
  
  int left;
  int top;
  int right;
  int bottom;

  inline void setRect(int lt, int tp, int rt, int bm)
  {
    left = lt;
    top = tp;
    right = rt;
    bottom = bm;
  }

  inline void setRect(const Rect *rect)
  {
    left    = rect->left;
    top     = rect->top;
    right   = rect->right;
    bottom  = rect->bottom;
  }

  // Convert to windows RECT
  inline RECT toWindowsRect() { RECT winRect;
                                winRect.left    = left;
                                winRect.top     = top;
                                winRect.right   = right;
                                winRect.bottom  = bottom;
                                return winRect; }

  // Load from windows RECT
  inline void fromWindowsRect(RECT *winRect)
  {
    left    = winRect->left;
    top     = winRect->top;
    right   = winRect->right;
    bottom  = winRect->bottom;
  }

  inline void move(int offsetX, int offsetY)
  {
    left    += offsetX;
    right   += offsetX;
    top     += offsetY;
    bottom  += offsetY;
  }

  inline void setLocation(int destX, int destY)
  {
    int offsetX = destX - left;
    int offsetY = destY - top;
    left    = destX;
    right   += offsetX;
    top     = destY;
    bottom  += offsetY;
  }

  inline bool cmpRect(const Rect *rect) { return  rect->left == left &&
                                                  rect->top == top &&
                                                  rect->right == right &&
                                                  rect->bottom == bottom; }

  inline void setWidth(int value)   { right = left + value; }
  inline void setHeight(int value)  { bottom = top + value; }

  inline int getWidth()  const { return right - left; }
  inline int getHeight() const { return bottom - top; }

  inline bool isEmpty() const { return getWidth() <= 0 || getHeight() <= 0; }
  inline int area() const { return isEmpty() ? 0 : getWidth() * getHeight(); }

  inline void clear() { left = top = right = bottom = 0; }

  Rect intersection(const Rect *other) const {
    Rect result;
    result.setRect((left > other->left) ? left : other->left,
                   (top > other->top) ? top : other->top,
                   (right < other->right) ? right : other->right,
                   (bottom < other->bottom) ? bottom : other->bottom);
    return result;
  }
};

#endif // __RECT_H__
