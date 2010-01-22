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

#include <time.h>
#include <sys/timeb.h>

#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h>
#endif

#include "Common.h"
#include "Timer.h"
#include "StringUtil.h"

namespace Common
{

#ifdef _WIN32
u32 Timer::GetTimeMs() {
	return timeGetTime();
}
#else
u32 Timer::GetTimeMs()
{
	struct timeb t;
	ftime(&t);
	return ((u32)(t.time * 1000 + t.millitm));
}
#endif


// --------------------------------------------
// Initiate, Start, Stop, and Update the time
// --------------------------------------------

// Set initial values for the class
Timer::Timer()
	: m_LastTime(0), m_StartTime(0), m_Running(false)
{
	Update();

#ifdef _WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
#endif
}

// Write the starting time
void Timer::Start()
{
	m_StartTime = GetTimeMs();
	m_Running = true;
}

// Stop the timer
void Timer::Stop()
{
	// Write the final time
	m_LastTime = GetTimeMs();
	m_Running = false;
}

// Update the last time variable
void Timer::Update()
{
	m_LastTime = GetTimeMs();
	//TODO(ector) - QPF
}



// -------------------------------------
// Get time difference and elapsed time
// -------------------------------------

// Get the number of milliseconds since the last Update()
u64 Timer::GetTimeDifference()
{
	return GetTimeMs() - m_LastTime;
}

/* Add the time difference since the last Update() to the starting time. This is used to compensate
   for a paused game. */
void Timer::AddTimeDifference()
{
	m_StartTime += GetTimeDifference();
}
// Wind back the starting time to a custom time
void Timer::WindBackStartingTime(u64 WindBack)
{
	m_StartTime += WindBack;
}

// Get the time elapsed since the Start()
u64 Timer::GetTimeElapsed()
{
	/* If we have not started yet return 1 (because then I don't have to change the FPS
	   calculation in CoreRerecording.cpp */
	if (m_StartTime == 0) return 1;

	// Rrturn the final timer time if the timer is stopped
	if (!m_Running) return (m_LastTime - m_StartTime);

	return (GetTimeMs() - m_StartTime);
}

// Get the formattet time elapsed since the Start()
std::string Timer::GetTimeElapsedFormatted() const
{
	// If we have not started yet, return zero
	if (m_StartTime == 0)
		return "00:00:00:000";

	// The number of milliseconds since the start, use a different value if the timer is stopped
	u64 Milliseconds;
	if (m_Running)
		Milliseconds = GetTimeMs() - m_StartTime;
	else
		Milliseconds = m_LastTime - m_StartTime;
	// Seconds
	u32 Seconds = (u32)(Milliseconds / 1000);
	// Minutes
	u32 Minutes = Seconds / 60;
	// Hours
	u32 Hours = Minutes / 60;

	std::string TmpStr = StringFromFormat("%02i:%02i:%02i:%03i", Hours, Minutes % 60, Seconds % 60, Milliseconds % 1000);
	return TmpStr;
}

// Get current time
void Timer::IncreaseResolution()
{
#ifdef _WIN32
	timeBeginPeriod(1);
#endif
}


void Timer::RestoreResolution()
{
#ifdef _WIN32
	timeEndPeriod(1);
#endif
}


#ifdef __GNUC__
void _time64(u64* t)
{
	*t = 0; //TODO
}
#endif


// Get the number of seconds since January 1 1970
u64 Timer::GetTimeSinceJan1970()
{
	time_t ltime;
	time(&ltime);
	return((u64)ltime);
}

u64 Timer::GetLocalTimeSinceJan1970()
{
	time_t sysTime, tzDiff;
	struct tm * gmTime;

	time(&sysTime);
	// Lazy way to get local time in sec
	gmTime	= gmtime(&sysTime);
	tzDiff = sysTime - mktime(gmTime);

	return (u64)(sysTime + tzDiff);
}

// Return the current time formatted as Minutes:Seconds:Milliseconds in the form 00:00:000
std::string Timer::GetTimeFormatted()
{
	time_t sysTime;
	struct tm * gmTime;
	char formattedTime[13];
	char tmp[13];

	time(&sysTime);
	gmTime = localtime(&sysTime);

	strftime(tmp, 6, "%M:%S", gmTime);

	// Now tack on the milliseconds
	struct timeb tp;
	(void)::ftime(&tp);
	sprintf(formattedTime, "%s:%03i", tmp, tp.millitm);

	return std::string(formattedTime);
}


// Returns a timestamp with decimals for precise time comparisons
// ----------------
double Timer::GetDoubleTime()
{
	struct timeb tp;
	(void)::ftime(&tp);
	u64 TmpSeconds = Common::Timer::GetTimeSinceJan1970(); // Get continous timestamp

	/* Remove a few years. We only really want enough seconds to make sure that we are
	   detecting actual actions, perhaps 60 seconds is enough really, but I leave a
	   year of seconds anyway, in case the user's clock is incorrect or something like that */
	TmpSeconds = TmpSeconds - (38 * 365 * 24 * 60 * 60);

	//if (TmpSeconds < 0) return 0; // Check the the user's clock is working somewhat

	u32 Seconds = (u32)TmpSeconds; // Make a smaller integer that fits in the double
	double ms = tp.millitm / 1000.0;
	double TmpTime = Seconds + ms;
	return TmpTime;
}


} // Namespace Common


