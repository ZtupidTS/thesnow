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

#include <vector>

#include "Thread.h"
#include "PowerPC/PowerPC.h"
#include "CoreTiming.h"
#include "Core.h"
#include "StringUtil.h"
#include "PluginManager.h"

#define MAX_SLICE_LENGTH 20000

namespace CoreTiming
{

struct EventType
{
	TimedCallback callback;
	const char *name;
};

std::vector<EventType> event_types;

struct BaseEvent
{
	s64 time;
	u64 userdata;
	int type;
	bool fifoWait;
//	Event *next;
};

typedef LinkedListItem<BaseEvent> Event;

// STATE_TO_SAVE (how?)
Event *first;
Event *tsFirst;
Event *tsLast;

// event pools
Event *eventPool = 0;
Event *eventTsPool = 0;
int allocatedTsEvents = 0;

int downcount, slicelength;
int maxSliceLength = MAX_SLICE_LENGTH;

s64 globalTimer;
s64 idledCycles;

Common::CriticalSection externalEventSection;

void (*advanceCallback)(int cyclesExecuted) = NULL;

Event* GetNewEvent()
{
    if(!eventPool)
        return new Event;

    Event* ev = eventPool;
    eventPool = ev->next;
    return ev;
}

Event* GetNewTsEvent()
{
    allocatedTsEvents++;

    if(!eventTsPool)
        return new Event;

    Event* ev = eventTsPool;
    eventTsPool = ev->next;
    return ev;
}

void FreeEvent(Event* ev)
{
    ev->next = eventPool;
    eventPool = ev;
}

void FreeTsEvent(Event* ev)
{
    ev->next = eventTsPool;
    eventTsPool = ev;
	allocatedTsEvents--;
}

int RegisterEvent(const char *name, TimedCallback callback)
{
	EventType type;
	type.name = name;
	type.callback = callback;
	event_types.push_back(type);
	return (int)event_types.size() - 1;
}

void UnregisterAllEvents()
{
	if (first)
		PanicAlert("Cannot unregister events with events pending");
	event_types.clear();
}

void Init()
{
	downcount = maxSliceLength;
	slicelength = maxSliceLength;
	globalTimer = 0;
	idledCycles = 0;
}

void Shutdown()
{
	ClearPendingEvents();
	UnregisterAllEvents();

    while(eventPool)
    {
        Event *ev = eventPool;
        eventPool = ev->next;
        delete ev;
    }

    externalEventSection.Enter();
    while(eventTsPool)
    {
        Event *ev = eventTsPool;
        eventTsPool = ev->next;
        delete ev;
    }
    externalEventSection.Leave();
}

void DoState(PointerWrap &p)
{
	externalEventSection.Enter();
	p.Do(downcount);
	p.Do(slicelength);
	p.Do(globalTimer);
	p.Do(idledCycles);
	// OK, here we're gonna need to specialize depending on the mode.
	// Should do something generic to serialize linked lists.
	switch (p.GetMode()) {
	case PointerWrap::MODE_READ:
		{
		ClearPendingEvents();
		if (first)
			PanicAlert("Clear failed.");
		int more_events = 0;
		Event *prev = 0;
		while (true) {
			p.Do(more_events);
			if (!more_events)
				break;
			Event *ev = GetNewEvent();
			if (!prev)
				first = ev;
			else
				prev->next = ev;
			p.Do(ev->time);
			p.Do(ev->type);
			p.Do(ev->userdata);
			p.Do(ev->fifoWait);
			ev->next = 0;
			prev = ev;
			ev = ev->next;
		}
		}
		break;
	case PointerWrap::MODE_MEASURE:
	case PointerWrap::MODE_VERIFY:
	case PointerWrap::MODE_WRITE:
		{
		Event *ev = first;
		int more_events = 1;
		while (ev) {
			p.Do(more_events);
			p.Do(ev->time);
			p.Do(ev->type);
			p.Do(ev->userdata);
			p.Do(ev->fifoWait);
			ev = ev->next;
		}
		more_events = 0;
		p.Do(more_events);
		break;
		}
	}
	externalEventSection.Leave();
}

u64 GetTicks()
{
    return (u64)globalTimer; 
}

u64 GetIdleTicks()
{
	return (u64)idledCycles;
}

// This is to be called when outside threads, such as the graphics thread, wants to
// schedule things to be executed on the main thread.
void ScheduleEvent_Threadsafe(int cyclesIntoFuture, int event_type, u64 userdata, bool fifoWait)
{
	externalEventSection.Enter();
	Event *ne = GetNewTsEvent();
	ne->time = globalTimer + cyclesIntoFuture;
	ne->type = event_type;
	ne->next = 0;
	ne->userdata = userdata;
	ne->fifoWait = fifoWait;
	if(!tsFirst)
		tsFirst = ne;
	if(tsLast)
		tsLast->next = ne;
	tsLast = ne;
	externalEventSection.Leave();
}

// Same as ScheduleEvent_Threadsafe(0, ...) EXCEPT if we are already on the main thread
// in which case the event will get handled immediately, before returning.
void ScheduleEvent_Threadsafe_Immediate(int event_type, u64 userdata)
{
	if(Core::IsRunningInCurrentThread())
	{
		externalEventSection.Enter();
		event_types[event_type].callback(userdata, 0);
		externalEventSection.Leave();
	}
	else
		ScheduleEvent_Threadsafe(0, event_type, userdata, false);
}

void ClearPendingEvents()
{
	while (first)
	{
		Event *e = first->next;
		FreeEvent(first);
		first = e;
	}
}

void AddEventToQueue(Event* ne)
{
	Event* prev = NULL;
	Event** pNext = &first;
	for(;;)
	{
		Event*& next = *pNext;
		if(!next || ne->time < next->time)
		{
			ne->next = next;
			next = ne;
			break;
		}
		prev = next;
		pNext = &prev->next;
	}
}

// This must be run ONLY from within the cpu thread
// cyclesIntoFuture may be VERY inaccurate if called from anything else
// than Advance 
void ScheduleEvent(int cyclesIntoFuture, int event_type, u64 userdata)
{
	Event *ne = GetNewEvent();
	ne->userdata = userdata;
	ne->type = event_type;
	ne->time = globalTimer + cyclesIntoFuture;
	ne->fifoWait = false;
	AddEventToQueue(ne);
}

void RegisterAdvanceCallback(void (*callback)(int cyclesExecuted))
{
	advanceCallback = callback;
}

bool IsScheduled(int event_type) 
{
	if (!first)
		return false;
	Event *e = first;
	while (e) {
		if (e->type == event_type)
			return true;
		e = e->next;
	}
	return false;
}

void RemoveEvent(int event_type)
{

	if (!first)
		return;
	if (first->type == event_type)
	{
		Event *next = first->next;
		FreeEvent(first);
		first = next;
	}
	if (!first)
		return;
	Event *prev = first;
	Event *ptr = prev->next;
	while (ptr)
	{
		if (ptr->type == event_type)
		{
			prev->next = ptr->next;
			FreeEvent(ptr);
			ptr = prev->next;
		}
		else
		{
			prev = ptr;
			ptr = ptr->next;
		}
	}
}

void RemoveThreadsafeEvent(int event_type)
{
	externalEventSection.Enter();
	if (!tsFirst)
	{
		externalEventSection.Leave();
		return;
	}
	if (tsFirst->type == event_type)
	{
		Event *next = tsFirst->next;
		FreeTsEvent(tsFirst);
		tsFirst = next;
	}
	if (!tsFirst)
	{
		externalEventSection.Leave();
		return;
	}
	Event *prev = tsFirst;
	Event *ptr = prev->next;
	while (ptr)
	{
		if (ptr->type == event_type)
		{	
			prev->next = ptr->next;
			FreeTsEvent(ptr);
			ptr = prev->next;
		}
		else
		{
			prev = ptr;
			ptr = ptr->next;
		}
	}
	externalEventSection.Leave();
}

void RemoveAllEvents(int event_type)
{	
	RemoveThreadsafeEvent(event_type);
	RemoveEvent(event_type);
}

void SetMaximumSlice(int maximumSliceLength)
{
	maxSliceLength = maximumSliceLength;
}

void ResetSliceLength()
{
	maxSliceLength = MAX_SLICE_LENGTH;
}


//This raise only the events required while the fifo is processing data
void ProcessFifoWaitEvents()
{
	MoveEvents();

	while (first)
	{
		if ((first->time <= globalTimer) && first->fifoWait)
		{
			
			Event* evt = first;
			first = first->next;
			event_types[evt->type].callback(evt->userdata, (int)(globalTimer - evt->time));
			FreeEvent(evt);
		}
		else
		{
			break;
		}
	}

}

void MoveEvents()
{

	externalEventSection.Enter();
    // Move events from async queue into main queue
	while (tsFirst)
	{
		Event *next = tsFirst->next;
		AddEventToQueue(tsFirst);
		tsFirst = next;
	}
	tsLast = NULL;

    // Move free events to threadsafe pool
    while(allocatedTsEvents > 0 && eventPool)
    {        
        Event *ev = eventPool;
        eventPool = ev->next;
        ev->next = eventTsPool;
        eventTsPool = ev;
        allocatedTsEvents--;
    }
	externalEventSection.Leave();

}

void Advance()
{	

	MoveEvents();		

	int cyclesExecuted = slicelength - downcount;
	globalTimer += cyclesExecuted;
	downcount = slicelength;

	while (first)
	{
		if (first->time <= globalTimer)
		{
//			LOG(GEKKO, "[Scheduler] %s     (%lld, %lld) ", 
//				first->name ? first->name : "?", (u64)globalTimer, (u64)first->time);
			Event* evt = first;
			first = first->next;
			event_types[evt->type].callback(evt->userdata, (int)(globalTimer - evt->time));
			FreeEvent(evt);
		}
		else
		{
			break;
		}
	}
	if (!first) 
	{
		WARN_LOG(POWERPC, "WARNING - no events in queue. Setting downcount to 10000");
		downcount += 10000;
	}
	else
	{
		slicelength = (int)(first->time - globalTimer);
		if (slicelength > maxSliceLength)
			slicelength = maxSliceLength;
		downcount = slicelength;
	}
	if (advanceCallback)
		advanceCallback(cyclesExecuted);
}

void LogPendingEvents()
{
	Event *ptr = first;
	while (ptr)
	{
		INFO_LOG(POWERPC, "PENDING: Now: %lld Pending: %lld Type: %d", globalTimer, ptr->time, ptr->type);
		ptr = ptr->next;
	}
}

void Idle()
{
	//DEBUG_LOG(POWERPC, "Idle");
	
	//When the FIFO is processing data we must not advance because in this way
	//the VI will be desynchronized. So, We are waiting until the FIFO finish and 
	//while we process only the events required by the FIFO.
	while (CPluginManager::GetInstance().GetVideo()->Video_IsFifoBusy())
	{
		ProcessFifoWaitEvents();		
		Common::YieldCPU();
	}

	idledCycles += downcount;
	downcount = 0;
	
	Advance();
}

std::string GetScheduledEventsSummary()
{
	Event *ptr = first;
	std::string text = "Scheduled events\n";
	text.reserve(1000);
	while (ptr)
	{
		unsigned int t = ptr->type;
		if (t >= event_types.size())
			PanicAlert("Invalid event type %i", t);
		const char *name = event_types[ptr->type].name;
		if (!name)
			name = "[unknown]";
		text += StringFromFormat("%s : %i %08x%08x\n", event_types[ptr->type].name, ptr->time, ptr->userdata >> 32, ptr->userdata);
		ptr = ptr->next;
	}
	return text;
}

}  // namespace
