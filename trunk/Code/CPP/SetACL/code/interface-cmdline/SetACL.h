/////////////////////////////////////////////////////////////////////////////
//
//
//	SetACL.h
//
//
//	Description:	Main include file for command line interface
//
//	Author:			Helge Klein
//
//	Created with:	MS Visual C++ 8.0
//
// Required:		Headers and libs from the platform SDK
//
//	Tabs set to:	3
//
//
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Includes
//
//////////////////////////////////////////////////////////////////////


#include "resource.h"
#include "..\baseclasses\CSetACL.h"


//////////////////////////////////////////////////////////////////////
//
// Function definitions
//
//////////////////////////////////////////////////////////////////////


void		PrintMsg (CString sMessage);		// Print a line to console and/or log file
void     PrintHelp ();							// Print help text
DWORD		ProcessCmdLine (int argc, TCHAR* argv[], void (* funcNotify) (CString) = NULL, CSetACL* oSetACL = NULL);		// Process the command line
