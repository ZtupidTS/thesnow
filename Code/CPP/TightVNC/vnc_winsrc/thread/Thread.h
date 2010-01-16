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

#ifndef __THREAD_H__
#define __THREAD_H__

#include "windows-lib/winhdr.h"
#include "CriticalSection.h"

enum THREAD_PRIORITY
{
  PRIORITY_IDLE,
  PRIORITY_LOWEST,
  PRIORITY_BELOW_NORMAL,
  PRIORITY_NORMAL,
  PRIORITY_ABOVE_NORMAL,
  PRIORITY_HIGHEST,
  PRIORITY_TIME_CRITICAL
};

class Thread
{
public:
  Thread(void);
  virtual ~Thread(void);

  bool wait();
  bool suspend();
  bool resume();
  inline void terminate() { m_terminated = true;
                            onTerminate(); }
  bool isActive() const { return m_active; }
  bool setPriority(THREAD_PRIORITY value);

protected:
  virtual void execute() = 0;
  virtual void onTerminate() {}
  virtual void waitTerminated(unsigned int sleepTime, int quant = 100);

  static DWORD WINAPI threadProc(LPVOID pThread);

  HANDLE m_hThread;
  DWORD m_threadID;
  THREAD_PRIORITY m_priority;

  bool m_active;
  volatile bool m_terminated;
};

#endif // __THREAD_H__
