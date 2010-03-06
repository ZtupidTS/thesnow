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

#include "PrecompiledHeader.h"
#include "Common.h"

#include "Counters.h"
#include "GS.h"
#include "Elfheader.h"
#include "Patch.h"
#include "PageFaultSource.h"
#include "SysThreads.h"

#include "Utilities/TlsVariable.inl"

#ifdef __WXMSW__
#	include <wx/msw/wrapwin.h>
#endif

#include <xmmintrin.h>

static DeclareTls(SysCoreThread*) tls_coreThread( NULL );

// --------------------------------------------------------------------------------------
//  SysCoreThread *External Thread* Implementations
//    (Called from outside the context of this thread)
// --------------------------------------------------------------------------------------

SysCoreThread::SysCoreThread()
{
	m_name					= L"EE Core";
	m_resetRecompilers		= true;
	m_resetProfilers		= true;
	m_resetVsyncTimers		= true;
	m_resetVirtualMachine	= true;
	m_hasValidState			= false;
}

SysCoreThread::~SysCoreThread() throw()
{
	SysCoreThread::Cancel();
}

void SysCoreThread::Cancel( bool isBlocking )
{
	m_CoreCancelDamnit = true;
	_parent::Cancel();
	ReleaseResumeLock();
}

bool SysCoreThread::Cancel( const wxTimeSpan& span )
{
	m_CoreCancelDamnit = true;
	if( _parent::Cancel( span ) )
	{
		ReleaseResumeLock();
		return true;
	}
	return false;
}

void SysCoreThread::Start()
{
	if( g_plugins == NULL ) return;
	g_plugins->Init();
	m_CoreCancelDamnit = false;		// belongs in OnStart actually, but I'm tired :P
	_parent::Start();
}

// Resumes the core execution state, or does nothing is the core is already running.  If
// settings were changed, resets will be performed as needed and emulation state resumed from
// memory savestates.
//
// Exceptions (can occur on first call only):
//   PluginInitError     - thrown if a plugin fails init (init is performed on the current thread
//                         on the first time the thread is resumed from it's initial idle state)
//   ThreadCreationError - Insufficient system resources to create thread.
//
void SysCoreThread::OnResumeReady()
{
	if( m_resetVirtualMachine )
		m_hasValidState = false;
	
	if( !m_hasValidState )
		m_resetRecompilers = true;
}

// Tells the thread to recover from the in-memory state copy when it resumes.  (thread must be
// resumed manually).
void SysCoreThread::RecoverState()
{
	Pause();
	m_resetVirtualMachine	= true;
	m_hasValidState			= false;
}

void SysCoreThread::Reset()
{
	Suspend();
	m_resetVirtualMachine	= true;
	m_hasValidState			= false;
}

// This function *will* reset the emulator in order to allow the specified elf file to
// take effect.  This is because it really doesn't make sense to change the elf file outside
// the context of a reset/restart.
void SysCoreThread::SetElfOverride( const wxString& elf )
{
	pxAssertDev( !m_hasValidState, "Thread synchronization error while assigning ELF override." );
	m_elf_override = elf;
}

ScopedCoreThreadSuspend::ScopedCoreThreadSuspend()
{
	m_ResumeWhenDone = GetCoreThread().Suspend();
}

ScopedCoreThreadSuspend::~ScopedCoreThreadSuspend() throw()
{
	if( m_ResumeWhenDone )
	{
		Console.WriteLn( Color_Gray, "Scoped CoreThread suspend was not allowed to resume." );
	}
}

// Resumes CoreThread execution, but *only* if it was in a running state when this object
// was instanized.  Subsequent calls to Resume() will be ignored.
void ScopedCoreThreadSuspend::Resume()
{
	if( m_ResumeWhenDone )
		GetCoreThread().Resume();
	m_ResumeWhenDone = false;
}

ScopedCoreThreadPause::ScopedCoreThreadPause()
{
	m_ResumeWhenDone = GetCoreThread().Pause();
}

ScopedCoreThreadPause::~ScopedCoreThreadPause() throw()
{
	if( m_ResumeWhenDone )
	{
		Console.WriteLn( Color_Gray, "Scoped CoreThread pause was not allowed to resume." );
	}
}

// Resumes CoreThread execution, but *only* if it was in a running state when this object
// was instanized.  Subsequent calls to Resume() will be ignored.
void ScopedCoreThreadPause::Resume()
{
	if( m_ResumeWhenDone )
		GetCoreThread().Resume();
	m_ResumeWhenDone = false;
}


// Applies a full suite of new settings, which will automatically facilitate the necessary
// resets of the core and components (including plugins, if needed).  The scope of resetting
// is determined by comparing the current settings against the new settings, so that only
// real differences are applied.
void SysCoreThread::ApplySettings( const Pcsx2Config& src )
{
	if( src == EmuConfig ) return;

	ScopedCoreThreadPause sys_paused;

	m_resetRecompilers		= ( src.Cpu != EmuConfig.Cpu ) || ( src.Recompiler != EmuConfig.Recompiler ) ||
							  ( src.Gamefixes != EmuConfig.Gamefixes ) || ( src.Speedhacks != EmuConfig.Speedhacks );
	m_resetProfilers		= ( src.Profiler != EmuConfig.Profiler );
	m_resetVsyncTimers		= ( src.GS != EmuConfig.GS );
	
	const_cast<Pcsx2Config&>(EmuConfig) = src;
	sys_paused.Resume();
}

void SysCoreThread::ChangeCdvdSource( CDVD_SourceType type )
{
	if( type == CDVDsys_GetSourceType() ) return;

	// Fast change of the CDVD source only -- a Pause will suffice.

	bool resumeWhenDone = Pause();
	GetPluginManager().Close( PluginId_CDVD );
	CDVDsys_ChangeSource( type );
	if( resumeWhenDone ) Resume();
}

// --------------------------------------------------------------------------------------
//  SysCoreThread *Worker* Implementations
//    (Called from the context of this thread only)
// --------------------------------------------------------------------------------------
SysCoreThread& SysCoreThread::Get()
{
	pxAssertMsg( tls_coreThread != NULL, L"This function must be called from the context of a running SysCoreThread." );
	return *tls_coreThread;
}

bool SysCoreThread::HasPendingStateChangeRequest() const
{
	return m_CoreCancelDamnit || GetMTGS().HasPendingException() || _parent::HasPendingStateChangeRequest();
}

struct ScopedBool_ClearOnError
{
	bool&	m_target;
	bool	m_success;
	
	ScopedBool_ClearOnError( bool& target ) :
		m_target( target ), m_success( false )
	{
		m_target = true;
	}

	virtual ~ScopedBool_ClearOnError()
	{
		m_target = m_success;
	}
	
	void Success() { m_success = true; }
};

void SysCoreThread::_reset_stuff_as_needed()
{
	if( m_resetVirtualMachine || m_resetRecompilers || m_resetProfilers )
	{
		SysClearExecutionCache();
		memBindConditionalHandlers();
		m_resetRecompilers		= false;
		m_resetProfilers		= false;
	}

	if( m_resetVirtualMachine )
	{
		DoCpuReset();
		m_resetVirtualMachine	= false;
		m_resetRecompilers		= true;
	}

	if( m_resetVsyncTimers )
	{
		UpdateVSyncRate();
		frameLimitReset();
		m_resetVsyncTimers = false;
	}
	
	SetCPUState( EmuConfig.Cpu.sseMXCSR, EmuConfig.Cpu.sseVUMXCSR );
}

void SysCoreThread::DoCpuReset()
{
	AffinityAssert_AllowFromSelf( pxDiagSpot );
	cpuReset();
}

void SysCoreThread::CpuInitializeMess()
{
	if( m_hasValidState ) return;

	_reset_stuff_as_needed();

	ScopedBool_ClearOnError sbcoe( m_hasValidState );

	wxString elf_file( m_elf_override );
	if( elf_file.IsEmpty() && EmuConfig.SkipBiosSplash && (CDVDsys_GetSourceType() != CDVDsrc_NoDisc))
	{
		// Fetch the ELF filename and CD type from the CDVD provider.
		wxString ename;
		int result = GetPS2ElfName( ename );
		switch( result )
		{
			case 0:
				throw Exception::RuntimeError( wxLt("Fast Boot failed: CDVD image is not a PS1 or PS2 game.") );

			case 1:
				if (!ENABLE_LOADING_PS1_GAMES)
                    throw Exception::RuntimeError( wxLt("Fast Boot failed: PCSX2 does not support emulation of PS1 games.") );

			case 2:
				// PS2 game.  Valid!
				elf_file = ename;
			break;

			jNO_DEFAULT
		}
	}

	if( !elf_file.IsEmpty() )
	{
		// Skip Bios Hack *or* Manual ELF override:
		//   Runs the PS2 BIOS stub, and then manually loads the ELF executable data, and
		//   injects the cpuRegs.pc with the address of the execution start point.
		//
		// This hack is necessary for non-CD ELF files, and is optional for game CDs as a
		// fast boot up option. (though not recommended for games because of rare ill side
		// effects).

		SetCPUState( EmuConfig.Cpu.sseMXCSR, EmuConfig.Cpu.sseVUMXCSR );

		Console.WriteLn( Color_StrongGreen, "(PCSX2 Core) Executing Bios Stub..." );

		do {
			// Even the BiosStub invokes vsyncs, so we need to be weary of state
			// changes and premature loop exits, and re-enter the stub executer until
			// the critera is met.

			StateCheckInThread();
			Cpu->ExecuteBiosStub();
		} while( cpuRegs.pc != 0x00200008 && cpuRegs.pc != 0x00100008 );

		Console.WriteLn( Color_StrongGreen, "(PCSX2 Core) Execute Bios Stub Complete");
		loadElfFile( elf_file );
	}
	
	sbcoe.Success();
}

void SysCoreThread::PostVsyncToUI()
{
}

// This is called from the PS2 VM at the start of every vsync (either 59.94 or 50 hz by PS2
// clock scale, which does not correlate to the actual host machine vsync).
//
// Default tasks: Updates PADs and applies vsync patches.  Derived classes can override this
// to change either PAD and/or Patching behaviors.
//
// [TODO]: Should probably also handle profiling and debugging updates, once those are
// re-implemented.
//
void SysCoreThread::VsyncInThread()
{
	PostVsyncToUI();
	if (EmuConfig.EnablePatches) ApplyPatch();
}

void SysCoreThread::StateCheckInThread()
{
	GetMTGS().RethrowException();
	_parent::StateCheckInThread();

	if( !m_hasValidState )
		throw Exception::RuntimeError( "Invalid emulation state detected; Virtual machine threads have been cancelled." );

	_reset_stuff_as_needed();		// kinda redundant but could catch unexpected threaded state changes...
}

void SysCoreThread::ExecuteTaskInThread()
{
	Threading::EnableHiresScheduler();
	tls_coreThread = this;
	m_sem_event.WaitWithoutYield();
	
	m_mxcsr_saved.bitmask = _mm_getcsr();
	
	PCSX2_PAGEFAULT_PROTECT {
		do {
			StateCheckInThread();
			Cpu->Execute();
		} while( true );
	} PCSX2_PAGEFAULT_EXCEPT;
}

void SysCoreThread::OnSuspendInThread()
{
	if( g_plugins != NULL )
		g_plugins->Close();
}

void SysCoreThread::OnResumeInThread( bool isSuspended )
{
	if( g_plugins != NULL )
		g_plugins->Open();

	CpuInitializeMess();
}


// Invoked by the pthread_exit or pthread_cancel.
void SysCoreThread::OnCleanupInThread()
{
	m_hasValidState = false;

	_mm_setcsr( m_mxcsr_saved.bitmask );

	Threading::DisableHiresScheduler();

	if( g_plugins != NULL )
		g_plugins->Close();

	tls_coreThread = NULL;
	_parent::OnCleanupInThread();
}

