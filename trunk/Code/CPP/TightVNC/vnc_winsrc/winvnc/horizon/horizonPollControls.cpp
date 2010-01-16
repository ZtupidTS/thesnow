
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonPollControls.cpp 1892 2004-10-06 14:20:15Z const_k $

#include "horizonPollControls.h"
#include "horizonProperties.h"

horizonPollControls::horizonPollControls( HWND hwnd )
	: m_hwnd( hwnd ), m_server( vncServerSingleton::GetInstance() )
{	
	// set checkboxes based on server settings
	SetChecked( IDC_POLL_FULLSCREEN, m_server->PollFullScreen() ) ;
	SetChecked( IDC_POLL_FOREGROUND, m_server->PollForeground() ) ;
	SetChecked( IDC_POLL_UNDER_CURSOR, m_server->PollUnderCursor() ) ;
	SetChecked( IDC_POLL_CONSOLE_ONLY, m_server->PollConsoleOnly() ) ;
	SetChecked( IDC_USE_HOOKS, ! m_server->DontSetHooks() ) ;
	SetChecked( IDC_DONT_USE_DRIVER, m_server->DontUseDriver() ) ;

	SetChecked( IDC_POLL_ON_INTERVAL, ! m_server->PollOnEventOnly() ) ;
	SetChecked( IDC_POLL_ONEVENT_ONLY, m_server->PollOnEventOnly() ) ;
	
	// set text fields based on server settings
	SetDlgItemInt( m_hwnd, IDC_POLL_CYCLE, m_server->GetPollingCycle(), FALSE ) ;

	// polling algorith
	if ( horizonPollingType::isTypeScanLinesAdjacent() )
	{
		SetChecked( IDC_POLL_ALGRM_SCANLINES_ADJ, TRUE ) ;
		SetChecked( IDC_POLL_ALGRM_SCANLINES, FALSE ) ;
	}
	else
	{
		SetChecked( IDC_POLL_ALGRM_SCANLINES_ADJ, FALSE ) ;
		SetChecked( IDC_POLL_ALGRM_SCANLINES, TRUE ) ;
	}
	

	// validate checkbox settings
	Validate() ;
}

BOOL
horizonPollControls::HandleEvent( UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
	switch ( uMsg )
	{
	case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
				//
				// frequency
				//
				case IDC_POLL_ON_INTERVAL:
					{
						SetChecked( IDC_POLL_ONEVENT_ONLY, ! IsChecked( IDC_POLL_ON_INTERVAL ) ) ;
						Enable( IDC_POLL_CYCLE, IsChecked( IDC_POLL_ON_INTERVAL ) ) ;
						return TRUE ;
					}
					
				case IDC_POLL_ONEVENT_ONLY:
					{
						SetChecked( IDC_POLL_ON_INTERVAL, ! IsChecked( IDC_POLL_ONEVENT_ONLY ) ) ;
						Enable( IDC_POLL_CYCLE, ! IsChecked( IDC_POLL_ONEVENT_ONLY ) ) ;
						return TRUE ;
					}

				//
				// algorithm
				//
				case IDC_POLL_ALGRM_SCANLINES:
					{
						SetChecked( IDC_POLL_ALGRM_SCANLINES_ADJ, ! IsChecked( IDC_POLL_ALGRM_SCANLINES ) ) ;
						return TRUE ;
					}
					
				case IDC_POLL_ALGRM_SCANLINES_ADJ:
					{
						SetChecked( IDC_POLL_ALGRM_SCANLINES, ! IsChecked( IDC_POLL_ALGRM_SCANLINES_ADJ ) ) ;
						return TRUE ;
					}

/*
				case IDC_POLL_ALGRM_QUADRANTS:
					{
						SetChecked( IDC_POLL_ALGRM_SCANLINES, ! IsChecked( IDC_POLL_ALGRM_QUADRANTS ) ) ;
						return TRUE ;
					}
*/

				case IDC_POLL_FOREGROUND:
				case IDC_POLL_UNDER_CURSOR:
				case IDC_POLL_FULLSCREEN:
					{
						Validate() ;
						return TRUE ;
					}	
				
/*
				case IDC_POLL_CYCLE:
					{
						if ( this != NULL )
						{
							UINT pollingCycle = GetDlgItemInt( m_hwnd, IDC_POLL_CYCLE, NULL, TRUE ) ;
							CalculateScreenRefresh( pollingCycle ) ;
						}
					}
*/
			}
		}
	}
	
	return FALSE ;
}

void 
horizonPollControls::Validate()
{
	// are we polling the whole screen?
	BOOL full_polling = IsChecked( IDC_POLL_FULLSCREEN ) ;

	// are we polling a window?
	BOOL window_polling = IsChecked( IDC_POLL_FOREGROUND ) 
		|| IsChecked( IDC_POLL_UNDER_CURSOR ) ;

	Enable( IDC_POLL_FULLSCREEN,   TRUE ) ;
	Enable( IDC_USE_HOOKS,    full_polling ) ;

	Enable( IDC_POLL_FOREGROUND,   ! full_polling ) ;
	Enable( IDC_POLL_UNDER_CURSOR, ! full_polling ) ;
	Enable( IDC_POLL_CONSOLE_ONLY, ! full_polling && window_polling ) ;

	return ;
}

void horizonPollControls::Apply()
{
	m_server->PollFullScreen( IsChecked( IDC_POLL_FULLSCREEN ) ) ;
	m_server->PollForeground( IsChecked( IDC_POLL_FOREGROUND ) ) ;
	m_server->PollUnderCursor( IsChecked( IDC_POLL_UNDER_CURSOR ) ) ;
	m_server->PollConsoleOnly( IsChecked( IDC_POLL_CONSOLE_ONLY ) ) ;
	m_server->PollOnEventOnly( IsChecked( IDC_POLL_ONEVENT_ONLY ) ) ;
	m_server->DontSetHooks( ! IsChecked( IDC_USE_HOOKS ) ) ;
	m_server->DontUseDriver( IsChecked( IDC_DONT_USE_DRIVER ) ) ;

	BOOL success ;
	UINT pollingCycle = GetDlgItemInt(m_hwnd, IDC_POLL_CYCLE, &success, TRUE);

	if ( success )
		m_server->SetPollingCycle(pollingCycle);

	if ( IsChecked( IDC_POLL_ALGRM_SCANLINES_ADJ ) )
		horizonPollingType::SetPollingType( horizonPollingType::SCANLINES_ADJACENT ) ;
	else // if ( IsChecked( IDC_POLL_ALGRM_SCANLINES ) )
		horizonPollingType::SetPollingType( horizonPollingType::SCANLINES ) ;

	return ;
}

