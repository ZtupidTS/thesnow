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

#define oneSecond 1000
#define oneMinute 60 * oneSecond
#define oneHour 60 * oneMinute
#define oneDay 24 * oneHour

class time
{
public:
  // Time in msec
  static unsigned int getCurrentTime()
  {
    int currentTime;
    SYSTEMTIME winCurrentTime;

    GetLocalTime(&winCurrentTime);
    currentTime = winCurrentTime.wDay * oneDay
                  + winCurrentTime.wHour * oneHour
                  + winCurrentTime.wMinute * oneMinute
                  + winCurrentTime.wSecond * oneSecond
                  + winCurrentTime.wMilliseconds;

    return currentTime;
  }
};
