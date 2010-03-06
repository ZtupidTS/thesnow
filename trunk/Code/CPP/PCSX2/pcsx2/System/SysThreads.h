/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Utilities/PersistentThread.h"
#include "x86emitter/tools.h"

using namespace Threading;

// --------------------------------------------------------------------------------------
//  ISysThread
// --------------------------------------------------------------------------------------
class ISysThread : public virtual IThread
{
public:
	ISysThread() {}
	virtual ~ISysThread() throw() {}

	virtual bool Suspend( bool isBlocking = true ) { return false; }
	virtual bool Pause() { return false; }
	virtual void Resume() {}
};

// --------------------------------------------------------------------------------------
//  SysThreadBase
// --------------------------------------------------------------------------------------

class SysThreadBase : public PersistentThread, public virtual ISysThread
{
	typedef PersistentThread _parent;

public:
	// Important: The order of these enumerations matters.  All "not-open" statuses must
	// be listed before ExecMode_Closed, since there are "optimized" tests that rely on the
	// assumption that "ExecMode <= ExecMode_Closed" equates to a closed thread status.
	enum ExecutionMode
	{
		// Thread has not been created yet.  Typically this is the same as IsRunning()
		// returning FALSE.
		ExecMode_NoThreadYet,

		// Close signal has been sent to the thread, but the thread's response is still
		// pending (thread is busy/running).
		ExecMode_Closing,

		// Thread is safely paused, with plugins in a "closed" state, and waiting for a
		// resume/open signal.
		ExecMode_Closed,

		// Thread is active and running, with pluigns in an "open" state.
		ExecMode_Opened,

		// Pause signal has been sent to the thread, but the thread's response is still
		// pending (thread is busy/running).
		ExecMode_Pausing,

		// Thread is safely paused, with plugins in an "open" state, and waiting for a
		// resume/open signal.
		ExecMode_Paused,
	};

protected:
	volatile ExecutionMode	m_ExecMode;

	// This lock is used to avoid simultaneous requests to Suspend/Resume/Pause from
	// contending threads.
	MutexLockRecursive		m_ExecModeMutex;

	// Used to wake up the thread from sleeping when it's in a suspended state.
	Semaphore				m_ResumeEvent;

	// Locked whenever the thread is not in a suspended state (either closed or paused).
	// Issue a Wait against this mutex for performing actions that require the thread
	// to be suspended.
	Mutex					m_RunningLock;
	
	// Protects the thread from re-entrant resume requests while dependent resources are
	// being constructed.
	NonblockingMutex		m_ResumeProtection;

public:
	explicit SysThreadBase();
	virtual ~SysThreadBase() throw();

	// Thread safety for IsOpen / IsClosed: The execution mode can change at any time on
	// any thread, so the actual status may have already changed by the time this function
	// returns its result.  Typically this isn't of major concern.  However if you need
	// more assured execution mode status, issue a lock against the ExecutionModeMutex()
	// first.
	bool IsOpen() const
	{
		return m_ExecMode > ExecMode_Closed;
	}

	bool HasPendingStateChangeRequest() const
	{
		ExecutionMode mode = m_ExecMode;
		return (mode == ExecMode_Closing) || (mode == ExecMode_Pausing);
	}

	bool IsClosed() const { return !IsOpen(); }

	ExecutionMode GetExecutionMode() const { return m_ExecMode; }
	Mutex& ExecutionModeMutex() { return m_ExecModeMutex; }

	virtual bool Suspend( bool isBlocking = true );
	virtual void Resume();
	virtual bool Pause();
	
	virtual bool AcquireResumeLock() { return m_ResumeProtection.TryAcquire(); }
	virtual void ReleaseResumeLock() { m_ResumeProtection.Release(); }

	virtual wxTimeSpan GetDeadlockTimeout() const;
	virtual void ThrowDeadlockException();

protected:
	virtual void DoThreadDeadlocked();
	virtual void OnStart();

	// This function is called by Resume immediately prior to releasing the suspension of
	// the core emulation thread.  You should overload this rather than Resume(), since
	// Resume() has a lot of checks and balances to prevent re-entrance and race conditions.
	virtual void OnResumeReady() {}

	virtual void StateCheckInThread();
	virtual void OnCleanupInThread();
	virtual void OnStartInThread();

	// Used internally from Resume(), so let's make it private here.
	virtual void Start();

	// Extending classes should implement this, but should not call it.  The parent class
	// handles invocation by the following guidelines: Called *in thread* from StateCheckInThread()
	// prior to suspending the thread (ie, when Suspend() has been called on a separate
	// thread, requesting this thread suspend itself temporarily).  After this is called,
	// the thread enters a waiting state on the m_ResumeEvent semaphore.
	virtual void OnSuspendInThread()=0;

	// Extending classes should implement this, but should not call it.  The parent class
	// handles invocation by the following guidelines: Called *in thread* from StateCheckInThread()
	// prior to pausing the thread (ie, when Pause() has been called on a separate thread,
	// requesting this thread pause itself temporarily).  After this is called, the thread
	// enters a waiting state on the m_ResumeEvent semaphore.
	virtual void OnPauseInThread()=0;

	// Extending classes should implement this, but should not call it.  The parent class
	// handles invocation by the following guidelines: Called from StateCheckInThread() after the
	// thread has been suspended and then subsequently resumed.
	// Parameter:
	//   isSuspended - set to TRUE if the thread is returning from a suspended state, or
	//     FALSE if it's returning from a paused state.
	virtual void OnResumeInThread( bool isSuspended )=0;
};

// --------------------------------------------------------------------------------------
//  SysCoreThread class
// --------------------------------------------------------------------------------------
class SysCoreThread : public SysThreadBase
{
	typedef SysThreadBase _parent;

protected:
	bool			m_resetRecompilers;
	bool			m_resetProfilers;
	bool			m_resetVsyncTimers;
	bool			m_resetVirtualMachine;
	bool			m_hasValidState;

	// Used by SETJMP only, but ifdef'ing it out clutters up the code.
	bool			m_CoreCancelDamnit;

	wxString		m_elf_override;
	
	SSE_MXCSR		m_mxcsr_saved;

public:
	static SysCoreThread& Get();

public:
	explicit SysCoreThread();
	virtual ~SysCoreThread() throw();

	virtual void ApplySettings( const Pcsx2Config& src );
	virtual void OnResumeReady();
	virtual void Reset();
	virtual void RecoverState();
	virtual void Cancel( bool isBlocking=true );
	virtual bool Cancel( const wxTimeSpan& timeout );
	
	bool HasValidState()
	{
		return m_hasValidState;
	}

	bool HasPendingStateChangeRequest() const;
	virtual void StateCheckInThread();
	virtual void VsyncInThread();
	virtual void PostVsyncToUI()=0;
	
	virtual const wxString& GetElfOverride() const { return m_elf_override; }
	virtual void SetElfOverride( const wxString& elf );
	virtual void ChangeCdvdSource( CDVD_SourceType type );

protected:
	void _reset_stuff_as_needed();

	virtual void CpuInitializeMess();
	virtual void Start();
	virtual void OnSuspendInThread();
	virtual void OnPauseInThread() {}
	virtual void OnResumeInThread( bool IsSuspended );
	virtual void OnCleanupInThread();
	virtual void ExecuteTaskInThread();
	virtual void DoCpuReset();
	
	void _StateCheckThrows();
};

// --------------------------------------------------------------------------------------
//  ScopedCoreThreadSuspend
// --------------------------------------------------------------------------------------
// This class behaves a bit differently from other scoped classes due to the "standard"
// assumption that we actually do *not* want to resume CoreThread operations when an
// exception occurs.  Because of this, the destructor of this class does *not* unroll the
// suspend operation.  Instead you must manually instruct the class to resume using a call
// to the provisioned Resume() method.
//
// If the class leaves scope without having been resumed, a log is written to the console.
// This can be useful for troubleshooting, and also allows the log a second line of info
// indicating the status of CoreThread execution at the time of the exception.
//
struct ScopedCoreThreadSuspend
{
	bool m_ResumeWhenDone;

	ScopedCoreThreadSuspend();
	virtual ~ScopedCoreThreadSuspend() throw();
	virtual void Resume();
};

struct ScopedCoreThreadPause
{
	bool m_ResumeWhenDone;

	ScopedCoreThreadPause();
	virtual ~ScopedCoreThreadPause() throw();
	virtual void Resume();
};

// GetCoreThread() is a required external implementation. This function is *NOT*
// provided by the PCSX2 core library.  It provides an interface for the linking User
// Interface apps or DLLs to reference their own instance of SysCoreThread (also allowing
// them to extend the class and override virtual methods).
//
extern SysCoreThread& GetCoreThread();

extern int sys_resume_lock;
