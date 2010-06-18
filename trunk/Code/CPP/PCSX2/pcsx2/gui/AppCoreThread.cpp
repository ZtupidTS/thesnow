/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
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
#include "App.h"
#include "AppSaveStates.h"

#include "Utilities/TlsVariable.inl"

#include "ps2/BiosTools.h"
#include "GS.h"

__aligned16 SysMtgsThread mtgsThread;
__aligned16 AppCoreThread CoreThread;

typedef void (AppCoreThread::*FnPtr_CoreThreadMethod)();

// --------------------------------------------------------------------------------------
//  SysExecEvent_InvokeCoreThreadMethod
// --------------------------------------------------------------------------------------
class SysExecEvent_InvokeCoreThreadMethod : public SysExecEvent
{
protected:
	FnPtr_CoreThreadMethod	m_method;

public:
	wxString GetEventName() const { return L"CoreThreadMethod"; }
	virtual ~SysExecEvent_InvokeCoreThreadMethod() throw() {}
	SysExecEvent_InvokeCoreThreadMethod* Clone() const { return new SysExecEvent_InvokeCoreThreadMethod(*this); }
	
	SysExecEvent_InvokeCoreThreadMethod( FnPtr_CoreThreadMethod method )
	{
		m_method = method;
	}
	
protected:
	void InvokeEvent()
	{
		if( m_method ) (CoreThread.*m_method)();
	}
};

bool ProcessingMethodViaThread( FnPtr_CoreThreadMethod method )
{
	if( GetSysExecutorThread().IsSelf() ) return false;
	SysExecEvent_InvokeCoreThreadMethod evt( method );
	GetSysExecutorThread().ProcessEvent( evt );
	return false;
}

static void PostCoreStatus( CoreThreadStatus pevt )
{
	sApp.PostAction( CoreThreadStatusEvent( pevt ) );
}

// --------------------------------------------------------------------------------------
//  AppCoreThread Implementations
// --------------------------------------------------------------------------------------
AppCoreThread::AppCoreThread() : SysCoreThread()
{
	m_resetCdvd = false;
}

AppCoreThread::~AppCoreThread() throw()
{
	_parent::Cancel();		// use parent's, skips thread affinity check.
}

void AppCoreThread::Cancel( bool isBlocking )
{
	AffinityAssert_AllowFrom_SysExecutor();
	_parent::Cancel( wxTimeSpan(0, 0, 2, 0) );
}

void AppCoreThread::Shutdown()
{
	AffinityAssert_AllowFrom_SysExecutor();
	_parent::Reset();
	CorePlugins.Shutdown();
}

ExecutorThread& GetSysExecutorThread()
{
	return wxGetApp().SysExecutorThread;
}

static void _Suspend()
{
	GetCoreThread().Suspend(true);
}

void AppCoreThread::Suspend( bool isBlocking )
{
	if( !GetSysExecutorThread().ProcessMethodSelf( _Suspend ) )
		_parent::Suspend(true);
}

void AppCoreThread::Resume()
{
	//if( !AffinityAssert_AllowFrom_SysExecutor() ) return;
	if( !GetSysExecutorThread().IsSelf() )
	{
		GetSysExecutorThread().PostEvent( SysExecEvent_InvokeCoreThreadMethod(&AppCoreThread::Resume) );
		return;
	}

	//if( m_ExecMode == ExecMode_Opened ) return;
	//if( !pxAssert( CorePlugins.AreLoaded() ) ) return;

	_parent::Resume();

}

void AppCoreThread::ChangeCdvdSource()
{
	if( !GetSysExecutorThread().IsSelf() )
	{
		GetSysExecutorThread().PostEvent( new SysExecEvent_InvokeCoreThreadMethod(&AppCoreThread::ChangeCdvdSource) );
		return;
	}

	CDVD_SourceType cdvdsrc( g_Conf->CdvdSource );
	if( cdvdsrc == CDVDsys_GetSourceType() ) return;

	// Fast change of the CDVD source only -- a Pause will suffice.

	ScopedCoreThreadPause paused_core;
	GetCorePlugins().Close( PluginId_CDVD );
	CDVDsys_ChangeSource( cdvdsrc );
	paused_core.AllowResume();

	// TODO: Add a listener for CDVDsource changes?  Or should we bother?
}

void AppCoreThread::OnResumeReady()
{
	ApplySettings( g_Conf->EmuOptions );

	CDVD_SourceType cdvdsrc( g_Conf->CdvdSource );
	if( cdvdsrc != CDVDsys_GetSourceType() || (cdvdsrc==CDVDsrc_Iso && (CDVDsys_GetFile(cdvdsrc) != g_Conf->CurrentIso)) )
	{
		m_resetCdvd = true;
	}

	CDVDsys_SetFile( CDVDsrc_Iso, g_Conf->CurrentIso );

	AppSaveSettings();

	_parent::OnResumeReady();
}

void AppCoreThread::ApplySettings( const Pcsx2Config& src )
{
	Pcsx2Config fixup( src );
	if( !g_Conf->EnableSpeedHacks )
		fixup.Speedhacks = Pcsx2Config::SpeedhackOptions();
	if( !g_Conf->EnableGameFixes )
		fixup.Gamefixes = Pcsx2Config::GamefixOptions();

	// Re-entry guard protects against cases where code wants to manually set core settings
	// which are not part of g_Conf.  The subsequent call to apply g_Conf settings (which is
	// usually the desired behavior) will be ignored.

	static int localc = 0;
	RecursionGuard guard( localc );
	if( guard.IsReentrant() ) return;
	if( fixup == EmuConfig ) return;

	if( m_ExecMode >= ExecMode_Opened )
	{
		ScopedCoreThreadPause paused_core;
		_parent::ApplySettings( fixup );
		paused_core.AllowResume();
	}
	else
	{
		_parent::ApplySettings( fixup );
	}
}

// --------------------------------------------------------------------------------------
//  AppCoreThread *Worker* Implementations
//    (Called from the context of this thread only)
// --------------------------------------------------------------------------------------

void AppCoreThread::DoCpuReset()
{
	PostCoreStatus( CoreThread_Reset );
	_parent::DoCpuReset();
}

void AppCoreThread::OnResumeInThread( bool isSuspended )
{
	if( m_resetCdvd )
	{
		GetCorePlugins().Close( PluginId_CDVD );
		CDVDsys_ChangeSource( g_Conf->CdvdSource );
		m_resetCdvd = false;	
	}

	_parent::OnResumeInThread( isSuspended );
	PostCoreStatus( CoreThread_Resumed );
}

void AppCoreThread::OnSuspendInThread()
{
	_parent::OnSuspendInThread();
	PostCoreStatus( CoreThread_Suspended );
}

// Called whenever the thread has terminated, for either regular or irregular reasons.
// Typically the thread handles all its own errors, so there's no need to have error
// handling here.  However it's a good idea to update the status of the GUI to reflect
// the new (lack of) thread status, so this posts a message to the App to do so.
void AppCoreThread::OnCleanupInThread()
{
	PostCoreStatus( CoreThread_Stopped );
	_parent::OnCleanupInThread();
}

void AppCoreThread::PostVsyncToUI()
{
	wxGetApp().LogicalVsync();
}

void AppCoreThread::StateCheckInThread()
{
	_parent::StateCheckInThread();
}

void AppCoreThread::UploadStateCopy( const VmStateBuffer& copy )
{
	ScopedCoreThreadPause paused_core;
	_parent::UploadStateCopy( copy );
	paused_core.AllowResume();
}

void AppCoreThread::ExecuteTaskInThread()
{
	PostCoreStatus( CoreThread_Started );
	_parent::ExecuteTaskInThread();
}

// --------------------------------------------------------------------------------------
//  BaseSysExecEvent_ScopedCore / SysExecEvent_CoreThreadClose / SysExecEvent_CoreThreadPause
// --------------------------------------------------------------------------------------
void BaseSysExecEvent_ScopedCore::_post_and_wait( IScopedCoreThread& core )
{
	DoScopedTask();

	ScopedLock lock( m_mtx_resume );
	PostResult();

	if( m_resume )
	{
		// If the sender of the message requests a non-blocking resume, then we need
		// to deallocate the m_sync object, since the sender will likely leave scope and
		// invalidate it.
		switch( m_resume->WaitForResult() )
		{
			case ScopedCore_BlockingResume:
				if( m_sync ) m_sync->ClearResult();
				core.AllowResume();
			break;

			case ScopedCore_NonblockingResume:
				m_sync = NULL;
				core.AllowResume();
			break;

			case ScopedCore_SkipResume:
				m_sync = NULL;
			break;
		}
	}
}


void SysExecEvent_CoreThreadClose::InvokeEvent()
{
	ScopedCoreThreadClose closed_core;
	_post_and_wait(closed_core);
	closed_core.AllowResume();
}	


void SysExecEvent_CoreThreadPause::InvokeEvent()
{
	ScopedCoreThreadPause paused_core;
	_post_and_wait(paused_core);
	paused_core.AllowResume();
}	


// --------------------------------------------------------------------------------------
//  ScopedCoreThreadClose / ScopedCoreThreadPause
// --------------------------------------------------------------------------------------

static DeclareTls(bool) ScopedCore_IsPaused			= false;
static DeclareTls(bool) ScopedCore_IsFullyClosed	= false;

BaseScopedCoreThread::BaseScopedCoreThread()
{
	//AffinityAssert_AllowFrom_MainUI();

	m_allowResume		= false;
	m_alreadyStopped	= false;
	m_alreadyScoped		= false;
}

BaseScopedCoreThread::~BaseScopedCoreThread() throw()
{
}

// Allows the object to resume execution upon object destruction.  Typically called as the last thing
// in the object's scope.  Any code prior to this call that causes exceptions will not resume the emulator,
// which is *typically* the intended behavior when errors occur.
void BaseScopedCoreThread::AllowResume()
{
	m_allowResume = true;
}

void BaseScopedCoreThread::DisallowResume()
{
	m_allowResume = false;
}

void BaseScopedCoreThread::DoResume()
{
	if( m_alreadyStopped ) return;
	if( !GetSysExecutorThread().IsSelf() )
	{
		//DbgCon.WriteLn("(ScopedCoreThreadPause) Threaded Scope Created!");
		m_sync_resume.PostResult( m_allowResume ? ScopedCore_NonblockingResume : ScopedCore_SkipResume );
		m_mtx_resume.Wait();
	}
	else
		CoreThread.Resume();
}

// Returns TRUE if the event is posted to the SysExecutor.
// Returns FALSE if the thread *is* the SysExecutor (no message is posted, calling code should
//  handle the code directly).
bool BaseScopedCoreThread::PostToSysExec( BaseSysExecEvent_ScopedCore* msg )
{
	ScopedPtr<BaseSysExecEvent_ScopedCore> smsg( msg );
	if( !smsg || GetSysExecutorThread().IsSelf()) return false;

	msg->SetSyncState(m_sync);
	msg->SetResumeStates(m_sync_resume, m_mtx_resume);

	GetSysExecutorThread().PostEvent( smsg.DetachPtr() );
	m_sync.WaitForResult();
	m_sync.RethrowException();

	return true;
}

ScopedCoreThreadClose::ScopedCoreThreadClose()
{
	if( ScopedCore_IsFullyClosed )
	{
		// tracks if we're already in scope or not.
		m_alreadyScoped = true;
		return;
	}
	
	if( !PostToSysExec(new SysExecEvent_CoreThreadClose()) )
	{
		if( !(m_alreadyStopped = CoreThread.IsClosed()) )
			CoreThread.Suspend();
	}

	ScopedCore_IsFullyClosed = true;
}

ScopedCoreThreadClose::~ScopedCoreThreadClose() throw()
{
	if( m_alreadyScoped ) return;
	_parent::DoResume();
	ScopedCore_IsFullyClosed = false;
}

ScopedCoreThreadPause::ScopedCoreThreadPause( BaseSysExecEvent_ScopedCore* abuse_me )
{
	if( ScopedCore_IsFullyClosed || ScopedCore_IsPaused )
	{
		// tracks if we're already in scope or not.
		m_alreadyScoped = true;
		return;
	}

	if( !abuse_me ) abuse_me = new SysExecEvent_CoreThreadPause();
	if( !PostToSysExec( abuse_me ) )
	{
		if( !(m_alreadyStopped = CoreThread.IsPaused()) )
			CoreThread.Pause();
	}

	ScopedCore_IsPaused = true;
}

ScopedCoreThreadPause::~ScopedCoreThreadPause() throw()
{
	if( m_alreadyScoped ) return;
	_parent::DoResume();
	ScopedCore_IsPaused = false;
}

ScopedCoreThreadPopup::ScopedCoreThreadPopup()
{
	// The old style GUI (without GSopen2) must use a full close of the CoreThread, in order to
	// ensure that the GS window isn't blocking the popup, and to avoid crashes if the GS window
	// is maximized or fullscreen.

	if( !GSopen2 )
		m_scoped_core = new ScopedCoreThreadClose();
	else
		m_scoped_core = new ScopedCoreThreadPause();
};

void ScopedCoreThreadPopup::AllowResume()
{
	if( m_scoped_core ) m_scoped_core->AllowResume();
}

void ScopedCoreThreadPopup::DisallowResume()
{
	if( m_scoped_core ) m_scoped_core->DisallowResume();
}
