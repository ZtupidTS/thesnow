
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: PollCycleControl.cpp 1972 2004-12-03 20:04:45Z jeffo3 $

#include "PollCycleControl.h"

// constants

static const int MAX_POLLING_USAGE = 30 ;
static const int MIN_POLLING_USAGE = 10 ;

static const int MAX_POLLING_CYCLE = 400 ;
static const int MIN_POLLING_CYCLE = 100 ;

static const POLLING_CYCLE_INTERVAL = 25 ;

// static instance initialization
PollCycleControl* PollCycleControl::m_instance = NULL ;

// static member initialization
CPUUsageWin32HRPC* PollCycleControl::m_cpu_hrpc = new CPUUsageWin32HRPC() ;
CPUUsage* PollCycleControl::m_cpu = CPUUsage::GetInstance() ;

UINT PollCycleControl::m_timer_id = 0 ;
int PollCycleControl::m_last_cycle = 0 ;

//
// class implementation
//

PollCycleControl*
PollCycleControl::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new PollCycleControl() ;
	return m_instance ;
}

PollCycleControl::PollCycleControl()
	: m_ms_per_cycle( 0 )
{
}

PollCycleControl::~PollCycleControl()
{
}

bool 
PollCycleControl::Start( void )
{
	if ( m_timer_id != 0 )
		return true ;

	// set the timer
	m_timer_id = SetTimer(
		NULL,
		NULL,
		1000,
		( TIMERPROC )( TimerProc )
	) ;

	if ( m_timer_id == 0 )
		return false ;

	return true ;
}

bool
PollCycleControl::Stop( void )
{
	if ( m_timer_id == 0 )
		return true ;

	return ( KillTimer( NULL, m_timer_id ) == TRUE )
		? true 
		: false
	;
}

VOID CALLBACK
PollCycleControl::TimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime ) 
{
	// this should always be true
	assert( idEvent == m_timer_id ) ;

	// get the polling usage
	int polling_usage = m_cpu_hrpc->GetCPUUsage() ;
	int polling_average = m_cpu_hrpc->GetCPUAverage() ;

	// get the server object
	vncServer* server = vncServerSingleton::GetInstance() ;

	// get the polling cycle
	int polling_cycle = server->GetPollingCycle() ;
	
	//
	// determine polling cycle delta
	//
		
	if ( polling_average > MAX_POLLING_USAGE )
	{
		// slow the cycle down
		polling_cycle += POLLING_CYCLE_INTERVAL ;
	}
	else if ( polling_average < MIN_POLLING_USAGE )
	{
		// let's try to go faster
		polling_cycle -= POLLING_CYCLE_INTERVAL ;	
	}
	else
	{
		// we're pretty happy here, but let's put 
		// a little downward pressure on the cycle
		polling_cycle = polling_cycle * 0.999 ;
	}
	
	//
	// normalize polling cycle
	//
	
	if ( polling_cycle > MAX_POLLING_CYCLE )
	{
		polling_cycle = MAX_POLLING_CYCLE ;
	}
	else if ( polling_cycle < MIN_POLLING_CYCLE )
	{
		polling_cycle = MIN_POLLING_CYCLE ;
	}

	//
	// when the cycle changes +/- 25 percent, 
	// log the state of the object
	//
#ifdef CPU_DEBUG
	// cpu usage
	vnclog.Print( 
		LL_ALL, 
		"system => %d, process => %d, polling => %d, cycle => %d\n", 
		m_cpu->GetSystemCPU(),
		m_cpu->GetProcessCPU(),
		m_cpu_hrpc->GetCPUAverage(),
		polling_cycle
	) ;
#endif // CPU_DEBUG

	if ( 
		polling_cycle >= static_cast< int >( m_last_cycle * 1.25 )
		|| polling_cycle <= static_cast< int >( m_last_cycle * 0.75 )
	)
	{	
		// new screen-refresh rate
		CalculateScreenRefresh( polling_cycle ) ;

		// remember this as new baseline
		m_last_cycle = polling_cycle ;
	}

	// update the polling cycle
	server->SetPollingCycle( polling_cycle ) ;

	return ;
}

void
PollCycleControl::CalculateScreenRefresh( UINT cycle )
{
	float max = -1.0 ;
	float min = -1.0 ;

	float adjusted_cycle = static_cast< float >( cycle ) ; // 16.0 ;
	
	if ( adjusted_cycle > 0.0 )
	{
		max = 1000.0 / adjusted_cycle ;
		min = 1000.0 / ( 32.0 * adjusted_cycle ) ;
	}

	vnclog.Print( 
		LL_INTINFO, 
		VNCLOG( "screen refresh rate, cycle => %d, max => %f, min => %f, adjusted_cycle => %f\n" ),
		cycle, max, min, adjusted_cycle 
	) ;

	return ;
}

//
// CPUUsageWin32HRPC adapter methods
//

bool
PollCycleControl::SetPollCycle( unsigned int ms_per_cycle )
{
	// store current cycle 
	m_ms_per_cycle = ms_per_cycle ;
	
	// calibrate performance counter
	return m_cpu_hrpc->SetPollCycle( ms_per_cycle ) ;
}

bool
PollCycleControl::MarkPollStart( void )
{
	return m_cpu_hrpc->MarkPollStart() ;
}

bool
PollCycleControl::MarkPollStop( void )
{
	return m_cpu_hrpc->MarkPollStop() ;
}


