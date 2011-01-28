// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#ifndef _THREAD_H_
#define _THREAD_H_

#include "StdThread.h"

// Don't include common.h here as it will break LogManager
#include "CommonTypes.h"
#include <stdio.h>
#include <string.h>

// This may not be defined outside _WIN32
#ifndef _WIN32
#ifndef INFINITE
#define INFINITE 0xffffffff
#endif

#include <xmmintrin.h>

//for gettimeofday and struct time(spec|val)
#include <time.h>
#include <sys/time.h>
#endif


namespace Common
{

int CurrentThreadId();

void SetThreadAffinity(std::thread::native_handle_type thread, u32 mask);
void SetCurrentThreadAffinity(u32 mask);
	
	class CriticalSection
	{
#ifdef _WIN32
		CRITICAL_SECTION section;
#else
#ifdef _POSIX_THREADS
		pthread_mutex_t mutex;
#endif
#endif
	public:
		
		CriticalSection(int spincount = 1000);
		~CriticalSection();
		void Enter();
		bool TryEnter();
		void Leave();
	};
	
#ifdef _WIN32
	// Event(WaitForSingleObject) is too expensive
	// as it always enters Ring0 regardless of the state of lock
	// This EventEx will try to stay in Ring3 as much as possible
	// If the lock can be obtained in the first time, Ring0 won't be entered at all
	class EventEx
	{
	public:
		EventEx();
		void Init();
		void Shutdown();
		void Set();
		// Infinite wait
		void Spin();
		// Infinite wait with sleep
		void Wait();
		// Wait with message processing and sleep
		bool MsgWait();
	private:
		volatile long m_Lock;
	};
#else
	// TODO: implement for Linux
#define EventEx	Event
#endif
	
	class Event
	{
	public:
		Event();
		void Init();
		void Shutdown();
		
		void Set();
		//returns whether the wait timed out
		bool Wait(const u32 timeout = INFINITE);
#ifdef _WIN32
		void MsgWait();
#else
		void MsgWait() {Wait();}
#endif
		
		
	private:
#ifdef _WIN32
		
		HANDLE m_hEvent;
		/* If we have waited more than five seconds we can be pretty sure that the thread is deadlocked.
		 So then we can just as well continue and hope for the best. I could try several times that
		 this works after a five second timeout (with works meaning that the game stopped and I could
		 start another game without any noticable problems). But several times it failed to, and ended
		 with a crash. But it's better than an infinite deadlock. */
		static const int THREAD_WAIT_TIMEOUT = 5000; // INFINITE or 5000 for example
		
#else
		
		bool is_set_;
#ifdef _POSIX_THREADS
		pthread_cond_t event_;
		pthread_mutex_t mutex_;
#endif
		
#endif
	};
	
	void SleepCurrentThread(int ms);
	void SwitchCurrentThread();	// On Linux, this is equal to sleep 1ms

	// YieldCPU: This function is only effective on HyperThreading CPU
	// Use this function during a spin-wait to make the current thread
	// relax while another thread is working. This may be more efficient
	// than using events because event functions use kernel calls.
	inline void YieldCPU()
	{
#ifdef _WIN32
		YieldProcessor();
#elif defined(_M_IX86) || defined(_M_X64)
		_mm_pause();
#endif
	}
	
	void SetCurrentThreadName(const char *name);
	
} // namespace Common

#endif // _THREAD_H_
