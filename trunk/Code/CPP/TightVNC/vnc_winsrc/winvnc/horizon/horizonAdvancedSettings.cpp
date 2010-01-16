
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonAdvancedSettings.cpp 1892 2004-10-06 14:20:15Z const_k $

#include "horizonAdvancedSettings.h"

//
// class implemtentation
//

bool 
horizonAdvancedSettings::Show( void )
{
	if ( m_dlgvisible == true )
		return true ;
	else
		m_dlgvisible = true ;

	int result = DialogBoxParam(
		hAppInstance,
		MAKEINTRESOURCE( IDD_ADVANCED_CONTROLS ), 
		NULL,
		( DLGPROC )( DialogProc ),
		( LONG )( this )
	);
	
	// increment times through
	++m_times_through ;
	
	m_dlgvisible = false ;

	return ( result == -1 ) ? false : true ;
}

BOOL CALLBACK 
horizonAdvancedSettings::DialogProc(
	HWND hwnd, 
	UINT uMsg,
	WPARAM wParam, 
	LPARAM lParam
)
{
	// get the dialog's object
	horizonAdvancedSettings* _this =  reinterpret_cast< horizonAdvancedSettings* >( 
		GetWindowLong( hwnd, GWL_USERDATA ) ) ;
	
	//
	// handle the message
	//
	
	switch ( uMsg )
	{
	
	// init the dialog
	case WM_INITDIALOG:
		{
			//
			// retrieve the dialog box parameter and use it as a pointer
			// to the calling horizonProperties object
			//
			SetWindowLong( hwnd, GWL_USERDATA, lParam ) ;
			horizonAdvancedSettings* _this = reinterpret_cast< horizonAdvancedSettings* >( lParam ) ;
			
			// move the dialog to the foreground
			SetForegroundWindow( hwnd ) ;

			_this->m_pollcontrols = new horizonPollControls( hwnd ) ;

			return FALSE ;
		}
	
	case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
			case IDOK:
				{
					_this->m_pollcontrols->Apply() ;
					EndDialog( hwnd, IDOK ) ;
					return TRUE ;
				}

			case IDC_APPLY:
				{
					_this->m_pollcontrols->Apply() ;
					return TRUE ;
				}

			case IDCANCEL:
				{
					EndDialog( hwnd, IDCANCEL ) ;
					return TRUE ;
				}
	
			default:
				{
					// give m_pollcontrols a shot at handling the event
					return _this->m_pollcontrols->HandleEvent( uMsg, wParam, lParam ) ;
				}
		
			} // switch ( LOWORD( wParam ) )

		} // WM_COMMAND

	case WM_DESTROY:
		{
			if ( _this->m_pollcontrols != NULL )
			{
				delete _this->m_pollcontrols ;
				_this->m_pollcontrols = NULL ;
			}
			return FALSE ;
		}
	}
	
	return FALSE ;

}
