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

#include "Thread.h"

Thread::Thread(void)
: m_terminated(false), m_active(false)
{
  m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) threadProc,
                           (LPVOID) this, CREATE_SUSPENDED, (LPDWORD) &m_threadID);
}

Thread::~Thread(void)
{
}

DWORD WINAPI Thread::threadProc(LPVOID pThread)
{
  ((Thread *)pThread)->execute();
  ((Thread *)pThread)->m_active = false;
  return 0;
}

bool Thread::wait()
{
  return (WaitForSingleObject(m_hThread, INFINITE) != WAIT_FAILED);
}

void Thread::waitTerminated(unsigned int sleepTime, int quant)
{
  int times = sleepTime/quant + sleepTime%quant;

  while (!m_terminated && times-- > 0) {
    Sleep(quant);
  }
}

bool Thread::suspend()
{
  m_active = !(SuspendThread(m_hThread) != -1);
  return !m_active;
}

bool Thread::resume()
{
  m_active = ResumeThread(m_hThread) != -1;
  return m_active;
}

bool Thread::setPriority(THREAD_PRIORITY value)
{
  int priority;

  switch(value)
  {
  case PRIORITY_IDLE:
    priority = THREAD_PRIORITY_IDLE;
    break;
  case PRIORITY_LOWEST:
    priority = THREAD_PRIORITY_LOWEST;
    break;
  case PRIORITY_BELOW_NORMAL:
    priority = THREAD_PRIORITY_BELOW_NORMAL;
    break;
  case PRIORITY_NORMAL:
    priority = THREAD_PRIORITY_NORMAL;
    break;
  case PRIORITY_ABOVE_NORMAL:
    priority = THREAD_PRIORITY_ABOVE_NORMAL;
    break;
  case PRIORITY_HIGHEST:
    priority = THREAD_PRIORITY_HIGHEST;
    break;
  case PRIORITY_TIME_CRITICAL:
    priority = THREAD_PRIORITY_TIME_CRITICAL;
    break;
  default:
    priority = THREAD_PRIORITY_NORMAL;
  }

  return SetThreadPriority(m_hThread, priority) != 0;
}
