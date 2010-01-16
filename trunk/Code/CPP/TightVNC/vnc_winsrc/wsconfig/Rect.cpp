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

#include "StdAfx.h"
#include "Rect.h"
#include "common/tstring.h"
#include "common/StringParser.h"

Rect::Rect()
: x(0), y(0), width(0), height(0)
{
}

Rect::~Rect()
{
}

tstring Rect::toString()
{
  TCHAR str[10];
  tstring out = _T("");
  _ltot(width, &str[0], 10);
  out += str;
  out += _T("x");
  _ltot(height, &str[0], 10);
  out += str;
  out += _T("+");
  _ltot(x, &str[0], 10);
  out += str;
  out += _T("+");
  _ltot(y, &str[0], 10);
  out += str;
  return out;
}

bool Rect::parse(LPTSTR string, Rect *out)
{
  tstring src = string;

  // Variables for parsing
  tstring strWidth = _T("");
  tstring strHeight = _T("");
  tstring strX = _T("");
  tstring strY = _T("");

  // Not enabled characters in string
  if (src.find_first_not_of(_T("-0123456789x+")) != tstring::npos)
    return false;
  // Search "x" delimitter
  size_t xPos = src.find(_T("x"));
  // No "x" delimitter
  if (xPos == tstring::npos)
    return false;
  // Found width
  strWidth = src.substr(0, xPos);
  // Search first "+" symbol
  size_t pPos1 = src.find(_T("+"), xPos);
  if (pPos1 == tstring::npos)
    return false;
  // Found height
  strHeight = src.substr(xPos + 1, pPos1 - xPos - 1);
  // Search second "+" symbol
  size_t pPos2 = src.find(_T("+"), pPos1 + 1);
  if (pPos2 == tstring::npos)
    return false;
  // Found X and Y
  strX = src.substr(pPos1 + 1, pPos2 - pPos1 - 1);
  strY = src.substr(pPos2 + 1, src.length() - pPos2 - 1);
  // Try parse this values to int
  if ((!StringParser::tryParseInt(strWidth)) || (!StringParser::tryParseInt(strHeight)) ||
      (!StringParser::tryParseInt(strY))     || (!StringParser::tryParseInt(strX))) {
      return false;
  }
  else {
    // Fill out structure
    if (out != NULL) {
      StringParser::parseInt(strX, &out->x);
      StringParser::parseInt(strY, &out->y);
      StringParser::parseInt(strWidth, &out->width);
      StringParser::parseInt(strHeight, &out->height);
    }
  }
  return true;
}

bool Rect::tryParse(LPTSTR string)
{
  return parse(string, NULL);
}
