/////////////////////////////////////////////////////////////////////////////
//
//
//	CSetACL.cpp
//
//
//	Description:	Main (worker) classes for SetACL
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


#include "CSetACL.h"


//////////////////////////////////////////////////////////////////////
//
// Class CSetACL
//
//////////////////////////////////////////////////////////////////////


//
// Constructor: initialize all member variables
//
CSetACL::CSetACL (void (* funcNotify) (CString))
{
	// Notification function
	m_funcNotify					=	funcNotify;

	m_nObjectType					=	SE_UNKNOWN_OBJECT_TYPE;
	m_nAction						=	0;
	m_nDACLProtected				=	INHPARNOCHANGE;
	m_fDACLResetChildObjects	=	false;
	m_nSACLProtected				=	INHPARNOCHANGE;
	m_fSACLResetChildObjects	=	false;
	m_nRecursionType				=	RECURSE_NO;
	m_nAPIError						=	0;
	m_paclExistingDACL			=	NULL;
	m_paclExistingSACL			=	NULL;
	m_psidExistingOwner			=	NULL;
	m_psidExistingGroup			=	NULL;
	m_pOwner							=	new CTrustee;
	m_pPrimaryGroup				=	new CTrustee;
	m_nDACLEntries					=	0;
	m_nSACLEntries					=	0;
	m_nListFormat					=	LIST_CSV;
	m_nListWhat						=	ACL_DACL;
	m_nListNameSID					=	LIST_NAME;
	m_fListInherited				=	false;
	m_fProcessSubObjectsOnly	=	false;
	m_fhBackupRestoreFile		=	NULL;
	m_fUseLowLevelWrites2SD		=	false;
	m_fIgnoreErrors				=	false;
	m_fhLog							=	NULL;
	m_fTrusteesProcessDACL		=	false;
	m_fTrusteesProcessSACL		=	false;
	m_fDomainsProcessDACL		=	false;
	m_fDomainsProcessSACL		=	false;
}


//
// Destructor: clean up
//
CSetACL::~CSetACL ()
{
	// Try to disable all privileges we possibly enabled
	SetPrivilege (SE_BACKUP_NAME, false);
	SetPrivilege (SE_RESTORE_NAME, false);
	SetPrivilege (SE_SECURITY_NAME, false);
	SetPrivilege (SE_TAKE_OWNERSHIP_NAME, false);

	// Free memory
	if (m_pOwner)			delete m_pOwner;
	if (m_pPrimaryGroup)	delete m_pPrimaryGroup;

	//
	// Delete all entries from m_lstACEs
	//
	POSITION	pos	=	m_lstACEs.GetHeadPosition ();

	while (pos)
	{
		delete m_lstACEs.GetNext (pos);
	}

	// Remove all list entries. Objects pointed to by the list members are NOT destroyed!
	m_lstACEs.RemoveAll ();

	//
	// Delete all entries from m_lstTrustees
	//
	pos	=	m_lstTrustees.GetHeadPosition ();

	while (pos)
	{
		CTrustee*	oTrustee	=	m_lstTrustees.GetNext (pos);

		if (oTrustee->m_oNewTrustee) delete oTrustee->m_oNewTrustee;
		delete oTrustee;
	}

	// Remove all list entries. Objects pointed to by the list members are NOT destroyed!
	m_lstTrustees.RemoveAll ();

	//
	// Delete all entries from m_lstDomains
	//
	pos	=	m_lstDomains.GetHeadPosition ();

	while (pos)
	{
		CDomain*	oDomain	=	m_lstDomains.GetNext (pos);

		if (oDomain->m_oNewDomain) delete oDomain->m_oNewDomain;
		delete oDomain;
	}

	// Remove all list entries. Objects pointed to by the list members are NOT destroyed!
	m_lstDomains.RemoveAll ();

	// Remove all entries from the filter list
	m_lstObjectFilter.RemoveAll ();

	// Close the output file, if necessary
	if (m_fhBackupRestoreFile)
	{
		fclose (m_fhBackupRestoreFile);
		m_fhBackupRestoreFile	=	NULL;
	}

	// Close the log file if it is open
	if (m_fhLog)
	{
		fclose (m_fhLog);

		m_fhLog				=	NULL;
	}
}


//
// SetObject: Sets the path and type of the object on which all actions are performed
//
DWORD CSetACL::SetObject (CString sObjectPath, SE_OBJECT_TYPE nObjectType)
{
	DWORD		nLocalPart;
	HANDLE	hNetEnum;

	if (sObjectPath.GetLength () >= 1 && (nObjectType == SE_FILE_OBJECT || nObjectType == SE_SERVICE || nObjectType == SE_PRINTER || nObjectType == SE_REGISTRY_KEY || nObjectType == SE_LMSHARE))
	{
		m_sObjectPath	=	sObjectPath;
		m_nObjectType	=	nObjectType;

		//
		// Try to determine the name of the computer the path points to
		//
		m_sTargetSystemName	=	TEXT ("");

		// If the path starts with a drive letter, try to find the server it is connected to
		if (sObjectPath.GetLength () >= 2 && nObjectType == SE_FILE_OBJECT && sObjectPath[1] == TCHAR (':'))
		{
			// Enumerate network connections
			if (WNetOpenEnum (RESOURCE_CONNECTED, RESOURCETYPE_DISK, 0, NULL, &hNetEnum) == NO_ERROR)
			{
				// Initialize a buffer for WNetEnumResource
				DWORD	nListEntries	=	1;
				DWORD	nBufferSize		=	16384;
				BYTE*	pBuffer			= new BYTE[16384];
				SecureZeroMemory (pBuffer, nBufferSize);

				while (WNetEnumResource (hNetEnum, &nListEntries, pBuffer, &nBufferSize) == NO_ERROR)
				{
					NETRESOURCE*	nrConnection	=	(NETRESOURCE*) pBuffer;
					CString			sLocalName		=	nrConnection->lpLocalName;

					if (sObjectPath.Left (2).CompareNoCase (sLocalName) == 0)
					{
						sObjectPath						=	nrConnection->lpRemoteName;
					}

					nListEntries	=	1;
					nBufferSize		=	16384;
					SecureZeroMemory (pBuffer, nBufferSize);
				}

				delete [] pBuffer;
				WNetCloseEnum (hNetEnum);
			}
		}

		if (sObjectPath.Left (2) == TEXT ("\\\\"))
		{
			// The path is in UNC format. Find the end of the computer name.
			nLocalPart = sObjectPath.Find (TEXT ("\\"), 2);

			if (nLocalPart != -1)
			{
				// Extract the computer name
				m_sTargetSystemName	=	sObjectPath.Mid (2, nLocalPart - 2);
			}
		}

		return RTN_OK;
	}
	else
	{
		m_sObjectPath	=	TEXT ("");
		m_nObjectType	=	SE_UNKNOWN_OBJECT_TYPE;

		return RTN_ERR_PARAMS;
	}
}


//
// SetBackupRestoreFile: Specify a (unicode) file to be used for backup/restore operations
//
DWORD CSetACL::SetBackupRestoreFile (CString sBackupRestoreFile)
{
	m_sBackupRestoreFile	=	sBackupRestoreFile;

	return RTN_OK;
}


//
// LogMessage: Log a message string
//
DWORD CSetACL::LogMessage (CString sMessage)
{
	int nBytesWritten	=	0;

	// Pass the message on to our caller
	if (m_funcNotify)
	{
		(*m_funcNotify) (sMessage);
	}

	// Write the message to the log file, if there is one
	if (m_fhLog)
	{
		// Output file was opened in binary mode (due to unicode). Correct newlines.
		sMessage.Replace (TEXT ("\n"), TEXT ("\r\n"));

		// Print to log file
		nBytesWritten	=	_ftprintf (m_fhLog, TEXT ("%s\r\n"), sMessage.GetString ());

		// Flush the buffer to disk directly
		fflush (m_fhLog);
	}

	if (nBytesWritten < 0)
	{
		return RTN_ERR_WRITE_LOGFILE;
	}
	else
	{
		return RTN_OK;
	}
}


//
// SetLogFile: Specify a (unicode) file to be used for logging
//
DWORD CSetACL::SetLogFile (CString sLogFile)
{
	m_sLogFile				=	sLogFile;

	// Close the log file if it is already open
	if (m_fhLog)
	{
		fclose (m_fhLog);

		m_fhLog				=	NULL;
	}

	// Check if the log file already exists
	errno_t	nErr			=	0;
	FILE*		fhTest		=	NULL;

	nErr	=	_tfopen_s (&fhTest, sLogFile, TEXT ("r"));
	BOOL	fExists			=	(fhTest != NULL);

	if (fhTest)	fclose (fhTest);

	// Open the log file for appending. If it does not exist, it is created
	nErr	=	_tfopen_s (&m_fhLog, sLogFile, TEXT ("ab"));

	if (m_fhLog == NULL || nErr != 0)
	{
		LogMessage (TEXT ("ERROR: Opening log file: <") + sLogFile + TEXT ("> failed!"));

		return RTN_ERR_OPEN_LOGFILE;
	}

	// Write the BOM if the file did not exist already
	if (! fExists)
	{
		WCHAR cBOM	=	0xFEFF;
		fwrite (&cBOM, sizeof (WCHAR), 1, m_fhLog);
	}

	return RTN_OK;
}


//
// SetAction: Set the action to be performed. All former values are erased.
//
DWORD CSetACL::SetAction (DWORD nAction)
{
	if (CheckAction (nAction))
	{
		m_nAction	=	nAction;

		return RTN_OK;
	}
	else
	{
		m_nAction	=	0;

		return RTN_ERR_PARAMS;
	}
}


//
// AddAction: Add an action to be performed. All former values are preserved.
//
DWORD CSetACL::AddAction (DWORD nAction)
{
	if (CheckAction (nAction))
	{
		m_nAction	|=	nAction;

		return RTN_OK;
	}
	else
	{
		return RTN_ERR_PARAMS;
	}
}


//
// AddObjectFilter: Add a keyword to be filtered out - objects containing this keyword are not processed
//
void CSetACL::AddObjectFilter (CString sKeyword)
{
	m_lstObjectFilter.AddTail (sKeyword);
}


//
// AddACE: Add an ACE to be processed.
//
DWORD CSetACL::AddACE (CString sTrustee, BOOL fTrusteeIsSID, CString sPermission, DWORD nInheritance, BOOL fInhSpecified, DWORD nAccessMode, DWORD nACLType)
{
	if (sTrustee.IsEmpty ())
	{
		LogMessage (TEXT ("ERROR: AddACE: No trustee specified."));

		return RTN_ERR_PARAMS;
	}

	if (fInhSpecified && (! CheckInheritance (nInheritance)))
	{
		LogMessage (TEXT ("ERROR: AddACE: Invalid inheritance specified."));

		return RTN_ERR_PARAMS;
	}

	if (! CheckACEAccessMode (nAccessMode, nACLType))
	{
		LogMessage (TEXT ("ERROR: AddACE: Invalid access mode for this ACL type specified."));

		return RTN_ERR_PARAMS;
	}

	CTrustee*	pTrustee	=	new CTrustee  (sTrustee, fTrusteeIsSID, ACTN_ADDACE, false, false);
	CACE*			pACE		=	new CACE (pTrustee, sPermission, nInheritance, fInhSpecified, (ACCESS_MODE) nAccessMode, nACLType);

	// Modify the count of DACL/SACL entries in the list
	if (nACLType == ACL_DACL)
	{
		m_nDACLEntries++;
	}
	else if (nACLType == ACL_SACL)
	{
		m_nSACLEntries++;
	}

	m_lstACEs.AddTail (pACE);

	return RTN_OK;
}


//
// AddTrustee: Add a trustee to be processed.
//
DWORD CSetACL::AddTrustee (CString sTrustee, CString sNewTrustee, BOOL fTrusteeIsSID, BOOL fNewTrusteeIsSID, DWORD nAction, BOOL fDACL, BOOL fSACL)
{
	if (! sTrustee.IsEmpty ())
	{
		CTrustee*	pTrustee		=	NULL;
		CTrustee*	pNewTrustee	=	NULL;

		try
		{
			pTrustee		=	new CTrustee  (sTrustee, fTrusteeIsSID, nAction, fDACL, fSACL);
			pNewTrustee	=	new CTrustee  (sNewTrustee, fNewTrusteeIsSID, nAction, false, false);

			pTrustee->m_oNewTrustee	=	pNewTrustee;

			m_lstTrustees.AddTail (pTrustee);

			// Remember whether DACL and/or SACL have to be processed
			if (fDACL)
			{
				m_fTrusteesProcessDACL	=	true;
			}
			if (fSACL)
			{
				m_fTrusteesProcessSACL	=	true;
			}

			return RTN_OK;

		}
		catch (...)
		{
			if (pTrustee != NULL)
			{
				delete pTrustee;
				pTrustee = NULL;
			}
			if (pNewTrustee != NULL)
			{
				delete pNewTrustee;
				pNewTrustee = NULL;
			}

			return RTN_ERR_OUT_OF_MEMORY;
		}
	}
	else
	{
		LogMessage (TEXT ("ERROR: AddTrustee: No trustee specified."));

		return RTN_ERR_PARAMS;
	}
}


//
// AddDomain: Add a domain to be processed.
//
DWORD CSetACL::AddDomain (CString sDomain, CString sNewDomain, DWORD nAction, BOOL fDACL, BOOL fSACL)
{
	DWORD	nError					=	RTN_OK;

	CDomain* pDomain		= NULL;
	CDomain*	pNewDomain	= NULL;

	try
	{
		if (! sDomain.IsEmpty ())
		{
			pDomain						=	new CDomain  ();

			nError						=	pDomain->SetDomain  (sDomain, nAction, fDACL, fSACL);
			if (nError != RTN_OK)
			{
				LogMessage (TEXT ("ERROR: AddDomain: Domain name <") + sDomain +  TEXT ("> is probably incorrect."));

				delete pDomain;

				return nError;
			}

			if (! sNewDomain.IsEmpty ())
			{
				pNewDomain				=	new CDomain  ();

				nError					=	pNewDomain->SetDomain  (sNewDomain, nAction, fDACL, fSACL);
				if (nError != RTN_OK)
				{
					LogMessage (TEXT ("ERROR: AddDomain: Domain name <") + sNewDomain +  TEXT ("> is probably incorrect."));

					delete pNewDomain;

					return nError;
				}

				pDomain->m_oNewDomain	=	pNewDomain;
			}

			m_lstDomains.AddTail (pDomain);

			// Remember whether DACL and/or SACL have to be processed
			if (fDACL)
			{
				m_fDomainsProcessDACL	=	true;
			}
			if (fSACL)
			{
				m_fDomainsProcessSACL	=	true;
			}

			return RTN_OK;
		}
		else
		{
			LogMessage (TEXT ("ERROR: AddDomain: No domain specified."));

			return RTN_ERR_PARAMS;
		}
	}
	catch (...)
	{
		if (pDomain != NULL)
		{
			delete pDomain;
			pDomain = NULL;
		}
		if (pNewDomain != NULL)
		{
			delete pNewDomain;
			pNewDomain = NULL;
		}

		return RTN_ERR_OUT_OF_MEMORY;
	}
}


//
// SetIgnoreErrors: Ignore errors, do NOT stop execution (unknown consequences!)
//
BOOL CSetACL::SetIgnoreErrors (BOOL fIgnoreErrors)
{
	m_fIgnoreErrors	=	fIgnoreErrors;

	return RTN_OK;
}


//
// SetOwner: Set an owner to be set by Run ()
//
DWORD CSetACL::SetOwner (CString sTrustee, BOOL fTrusteeIsSID)
{
	if (m_pOwner) delete m_pOwner;

	if (! sTrustee.IsEmpty ())
	{
		m_pOwner	=	new CTrustee (sTrustee, fTrusteeIsSID, ACTN_SETOWNER, false, false);

		return RTN_OK;
	}
	else
	{
		return RTN_ERR_PARAMS;
	}
}


//
// SetPrimaryGroup: Set the primary group to be set by Run ()
//
DWORD CSetACL::SetPrimaryGroup (CString sTrustee, BOOL fTrusteeIsSID)
{
	if (m_pPrimaryGroup) delete m_pPrimaryGroup;

	if (! sTrustee.IsEmpty ())
	{
		m_pPrimaryGroup	=	new CTrustee (sTrustee, fTrusteeIsSID, ACTN_SETGROUP, false, false);

		return RTN_OK;
	}
	else
	{
		return RTN_ERR_PARAMS;
	}
}


//
// SetRecursion: Set the recursion
//
DWORD CSetACL::SetRecursion (DWORD nRecursionType)
{
	if (! m_nObjectType)
	{
		m_nRecursionType	=	0;

		return RTN_ERR_OBJECT_NOT_SET;
	}

	if (m_nObjectType == SE_FILE_OBJECT)
	{
		if (nRecursionType == RECURSE_NO || nRecursionType == RECURSE_CONT || nRecursionType == RECURSE_OBJ || nRecursionType == RECURSE_CONT_OBJ)
		{
			m_nRecursionType	=	nRecursionType;

			return RTN_OK;
		}
	}
	else if (m_nObjectType == SE_REGISTRY_KEY)
	{
		if (nRecursionType == RECURSE_NO || nRecursionType == RECURSE_CONT)
		{
			m_nRecursionType	=	nRecursionType;

			return RTN_OK;
		}
	}
	else
	{
		m_nRecursionType	=	RECURSE_NO;

		return RTN_ERR_PARAMS;
	}

	return RTN_OK;
}


//
// SetObjectFlags: Set flags specific to the object
//
DWORD CSetACL::SetObjectFlags (DWORD nDACLProtected, DWORD nSACLProtected, BOOL fDACLResetChildObjects, BOOL fSACLResetChildObjects)
{
	if (CheckInhFromParent (nDACLProtected) && CheckInhFromParent (nSACLProtected))
	{
		m_nDACLProtected				=	nDACLProtected;
		m_fDACLResetChildObjects	=	fDACLResetChildObjects;

		m_nSACLProtected				=	nSACLProtected;
		m_fSACLResetChildObjects	=	fSACLResetChildObjects;

		return RTN_OK;
	}
	else
	{
		m_nDACLProtected				=	INHPARNOCHANGE;
		m_fDACLResetChildObjects	=	false;

		m_nSACLProtected				=	INHPARNOCHANGE;
		m_fSACLResetChildObjects	=	false;

		return RTN_ERR_PARAMS;
	}
}


//
// SetListOptions: Set the options for ACL listing
//
DWORD CSetACL::SetListOptions (DWORD nListFormat, DWORD nListWhat, BOOL fListInherited, DWORD nListNameSID)
{
	if (nListWhat > 0 && nListWhat <= (ACL_DACL + ACL_SACL + SD_OWNER + SD_GROUP) && (nListFormat == LIST_SDDL || nListFormat == LIST_CSV || nListFormat == LIST_TAB) && nListNameSID >= LIST_NAME && nListNameSID <= LIST_NAME_SID)
	{
		m_nListFormat		=	nListFormat;
		m_nListWhat			=	nListWhat;
		m_nListNameSID		=	nListNameSID;
		m_fListInherited	=	fListInherited;
		return RTN_OK;
	}

	m_nListFormat		=	LIST_CSV;
	m_nListWhat			=	ACL_DACL;
	m_nListNameSID		=	LIST_NAME;
	m_fListInherited	=	false;
	return RTN_ERR_LIST_OPTIONS;
}


//
// GetLastErrorMessage: Return the error string in the system's language of either the error code passed in or from m_nAPIError
//
CString CSetACL::GetLastErrorMessage (DWORD nError)
{
	CString	sMessage;
	HMODULE	hModule			= NULL;
	DWORD		nFormatFlags	= FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;

	if (! nError)
	{
		if (! m_nAPIError)
		{
			nError	=	GetLastError ();
		}
		else
		{
			nError	=	m_nAPIError;
		}
	}

	//
	// If nError is in the network range, load the message source
	//
	if (nError >= NERR_BASE && nError <= MAX_NERR)
	{
		hModule	=	LoadLibraryEx (TEXT ("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);

		if (hModule)
		{
			nFormatFlags	|=	FORMAT_MESSAGE_FROM_HMODULE;
		}
	}

	//
	// Call FormatMessage () to get the message text from the system or the supplied module handle
	//
	FormatMessage (nFormatFlags, hModule, nError, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), sMessage.GetBuffer (1024), sizeof (TCHAR) * 1024, NULL);
	sMessage.ReleaseBuffer ();

	// If we loaded a message source, unload it
	if (hModule)
	{
		FreeLibrary (hModule);
	}

	return sMessage;
}


//
// Run: Do it: apply all settings.
//
DWORD CSetACL::Run ()
{
	DWORD		nError				=	RTN_OK;
	CString	sInternalError;
	CString	sAPIError;
	CString	sErrorMessage;

	try
	{
		// Do things like looking up binary SIDs for every trustee
		nError	=	Prepare ();

		if (nError != RTN_OK)
		{
			throw nError;
		}

		//
		// Perform requested action(s)
		//
		// But first check if there IS an action
		if (m_nAction == 0)
		{
			throw (DWORD) RTN_ERR_NO_ACTN_SPECIFIED;
		}

		if (m_nAction & ACTN_RESTORE)
		{
			// Do the work
			nError	=	DoActionRestore ();

			if (nError != RTN_OK)
			{
				throw nError;
			}
		}

		if (m_nAction & ACTN_ADDACE || m_nAction & ACTN_SETOWNER || m_nAction & ACTN_SETGROUP || m_nAction & ACTN_SETINHFROMPAR || m_nAction & ACTN_CLEARDACL || m_nAction & ACTN_CLEARSACL || m_nAction & ACTN_TRUSTEE || m_nAction & ACTN_DOMAIN && nError == RTN_OK)
		{
			// Do the work
			nError	=	DoActionWrite ();

			if (nError != RTN_OK)
			{
				throw nError;
			}
		}

		if (m_nAction & ACTN_RESETCHILDPERMS && nError == RTN_OK)
		{
			// Set sub-object-only processing to on.
			m_fProcessSubObjectsOnly	=	true;

			//
			// Set some variables to the correct values for this operation, but first back them up so we can restore the original values later.
			//
			// 1. Backup
			DWORD	nAction					=	m_nAction;
			DWORD	nRecursionType			=	m_nRecursionType;
			DWORD nDACLProtected			=	m_nDACLProtected;
			DWORD nSACLProtected			=	m_nSACLProtected;
			DWORD	nDACLEntries			=	m_nDACLEntries;
			DWORD	nSACLEntries			=	m_nSACLEntries;

			// 2. Set new values: do nothing unless explicitly specified (by parameter "-rst")
			m_nAction						=	0;
			m_nRecursionType				=	RECURSE_CONT_OBJ;
			m_nDACLProtected				=	INHPARNOCHANGE;
			m_nSACLProtected				=	INHPARNOCHANGE;
			m_nDACLEntries					=	0;
			m_nSACLEntries					=	0;

			if (m_fDACLResetChildObjects)
			{
				m_nAction					|= ACTN_SETINHFROMPAR | ACTN_CLEARDACL;
				m_nDACLProtected			=	INHPARYES;
			}
			if (m_fSACLResetChildObjects)
			{
				m_nAction					|= ACTN_SETINHFROMPAR | ACTN_CLEARSACL;
				m_nSACLProtected			=	INHPARYES;
			}

			// 3. Do the work
			if (m_nAction)
			{
				nError						=	DoActionWrite ();

				if (nError != RTN_OK)
				{
					throw nError;
				}
			}
			else
			{
				LogMessage (TEXT ("WARNING: Action 'reset children' was used without specifying whether to reset the DACL, SACL, or both. Nothing was reset."));
			}

			// 4. Restore the variables we backed up.
			m_nAction						=	nAction;
			m_nRecursionType				=	nRecursionType;
			m_nDACLProtected				=	nDACLProtected;
			m_nSACLProtected				=	nSACLProtected;
			m_nDACLEntries					=	nDACLEntries;
			m_nSACLEntries					=	nSACLEntries;

			// Set sub-object-only processing back to default state.
			m_fProcessSubObjectsOnly	=	false;
		}

		if (m_nAction & ACTN_LIST && nError == RTN_OK)
		{
			// Do the work
			nError	=	DoActionList ();

			if (nError != RTN_OK)
			{
				throw nError;
			}
		}

		// No Errors occured
		sErrorMessage		=	TEXT ("\nSetACL finished successfully.");
	}
	catch (DWORD nRunError)
	{
		if (nRunError || m_nAPIError)
		{
			sErrorMessage	=	TEXT ("\nSetACL finished with error(s): ");

			if (nRunError)
			{
				// Get the internal error message from the resource table
				if (sInternalError.LoadString (nRunError))
				{
					sErrorMessage	+=	TEXT ("\nSetACL error message: ") + sInternalError;
				}
			}

			if (m_nAPIError)
			{
				// Get the API error from the OS
				sAPIError	=	GetLastErrorMessage ();
				sErrorMessage	+=	TEXT ("\nOperating system error message: ") + sAPIError;
			}
		}
	}

	// Notify caller of result
	LogMessage (sErrorMessage);

	return nError;
}


//
// Prepare: Prepare for execution.
//
DWORD CSetACL::Prepare ()
{
	DWORD	nError;

	// Was the object path set?
	if (m_sObjectPath.IsEmpty ())
	{
		return RTN_ERR_OBJECT_NOT_SET;
	}

	// Check the type and version of the OS we are running on
	OSVERSIONINFO osviVersion;

	ZeroMemory (&osviVersion, sizeof (OSVERSIONINFO));

	osviVersion.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

	if (GetVersionEx (&osviVersion))
	{
		BOOL fVersionOK = (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osviVersion.dwMajorVersion > 4);

		if (! fVersionOK)
		{
			LogMessage (TEXT ("ERROR: SetACL works only on NT based operating systems newer than NT4."));

			return RTN_ERR_OS_NOT_SUPPORTED;
		}
	}
	else
	{
		LogMessage (TEXT ("INFO: The version of your operating system could not be determined. SetACL may not work correctly."));
	}

	// Try to enable the backup privilege. This allows us to read file system objects that we do not have permission for.
	if (SetPrivilege (SE_BACKUP_NAME, true) != RTN_OK)
	{
		LogMessage (TEXT ("INFO: Privilege 'Back up files and directories' could not be enabled. This can probably be ignored."));
	}

	// Also enable the restore privilege which lets us set the owner to any (!) user/group, not only administrators
	if (SetPrivilege (SE_RESTORE_NAME, true) != RTN_OK)
	{
		LogMessage (TEXT ("INFO: Privilege 'Restore files and directories' could not be enabled. This can probably be ignored."));
	}

	//
	// Look up the SID of every trustee set
	//
	// First iterate through the entries in m_lstACEs
	POSITION	pos	=	m_lstACEs.GetHeadPosition ();

	while (pos)
	{
		CACE*	pACE	=	m_lstACEs.GetNext (pos);

		if (pACE && pACE->m_pTrustee->LookupSID () != RTN_OK)
		{
			return RTN_ERR_LOOKUP_SID;
		}
	}

	// Next, iterate through the entries in m_lstTrustees
	pos	=	m_lstTrustees.GetHeadPosition ();

	while (pos)
	{
		CTrustee*	pTrustee	=	m_lstTrustees.GetNext (pos);

		if (pTrustee && pTrustee->LookupSID () != RTN_OK)
		{
			return RTN_ERR_LOOKUP_SID;
		}
		if (pTrustee && pTrustee->m_oNewTrustee->LookupSID () != RTN_OK)
		{
			return RTN_ERR_LOOKUP_SID;
		}
	}

	// Finally process the owner and primary group
	if (m_pOwner && m_pOwner->LookupSID () != RTN_OK)
	{
		return RTN_ERR_LOOKUP_SID;
	}

	if (m_pPrimaryGroup && m_pPrimaryGroup->LookupSID () != RTN_OK)
	{
		return RTN_ERR_LOOKUP_SID;
	}

	//
	// Set the correct permission and inheritance values for each ACE entry
	//
	nError = DetermineACEAccessMasks ();
	if (nError != ERROR_SUCCESS)
	{
		return nError;
	}

	// If the registry is to be processed: correct the registry path (ie. "hklm\software" -> "machine\software")
	if (m_nObjectType == SE_REGISTRY_KEY)
	{
		HKEY hKey	=	NULL;

		nError		=	OpenRegKey (&m_sObjectPath, &hKey);

		if (hKey)
		{
			RegCloseKey (hKey);
		}

		if (nError != RTN_OK)
		{
			return nError;
		}
	}

	//
	// Correct file system paths. We use Unicode and have to use another syntax to overcome the MAX_PATH restriction (260 chars):
	//
	// c:						local file system root, does not persist when the system is restarted	->	\\?\c:
	// c:\data				"normal" paths																				->	\\?\c:\
	// \\server\share		UNC paths																					->	\\?\UNC\server\share
	//
	if (m_nObjectType	== SE_FILE_OBJECT)
	{
		// Build the "long" version of the path
		BuildLongUnicodePath (&m_sObjectPath);
	}

	return RTN_OK;
}


//
// CheckAction: Check if an action is valid
//
BOOL CSetACL::CheckAction (DWORD nAction)
{
	switch (nAction)
	{
	case ACTN_ADDACE:
	case ACTN_LIST:
	case ACTN_SETOWNER:
	case ACTN_SETGROUP:
	case ACTN_CLEARDACL:
	case ACTN_CLEARSACL:
	case ACTN_TRUSTEE:
	case ACTN_DOMAIN:
	case ACTN_SETINHFROMPAR:
	case ACTN_RESETCHILDPERMS:
	case ACTN_RESTORE:
		return true;
	default:
		return false;
	}
}


//
// CheckInhFromParent: Check if inheritance from parent flags are valid
//
BOOL CSetACL::CheckInhFromParent (DWORD nInheritance)
{
	switch (nInheritance)
	{
	case INHPARNOCHANGE:
	case INHPARYES:
	case INHPARCOPY:
	case INHPARNOCOPY:
		return true;
	default:
		return false;
	}
}


//
// CheckInheritance: Check if inheritance flags are valid
//
BOOL CSetACL::CheckInheritance (DWORD nInheritance)
{
	DWORD	nAllFlags	=	SUB_OBJECTS_ONLY_INHERIT | SUB_CONTAINERS_ONLY_INHERIT | INHERIT_NO_PROPAGATE | INHERIT_ONLY;

	if (nInheritance >= NO_INHERITANCE && nInheritance <= nAllFlags)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//
// CheckACEType: Check if an ACE access mode (deny, set...) is valid
//
BOOL CSetACL::CheckACEAccessMode (DWORD nAccessMode, DWORD nACLType)
{
	if (nACLType == ACL_DACL)
	{
		switch (nAccessMode)
		{
		case DENY_ACCESS:
		case REVOKE_ACCESS:
		case SET_ACCESS:
		case GRANT_ACCESS:
			return true;
		default:
			return false;
		}
	}
	else if (nACLType == ACL_SACL)
	{
		switch (nAccessMode)
		{
		case REVOKE_ACCESS:
		case SET_AUDIT_SUCCESS:
		case SET_AUDIT_FAILURE:
		case SET_AUDIT_FAILURE + SET_AUDIT_SUCCESS:
			return true;
		default:
			return false;
		}
	}

	return false;
}


//
// CheckFilterList: Check whether a certain path needs to be filtered out
//
BOOL CSetACL::CheckFilterList (CString sObjectPath)
{
	// Ignore case when searching for keywords
	sObjectPath.MakeLower ();

	POSITION pos	=	m_lstObjectFilter.GetHeadPosition ();

	while (pos)
	{
		CString	sKeyword	=	m_lstObjectFilter.GetNext (pos);

		sKeyword.MakeLower ();

		if (sObjectPath.Find (sKeyword) != -1)
		{
			// The path is on the filter list
			return true;
		}
	}

	return false;
}


//
// DetermineACEAccessMask: Set the access masks and inheritance values for all ACEs
//
DWORD CSetACL::DetermineACEAccessMasks ()
{
	CTypedPtrList<CPtrList, CACE*>	lstACEs2Add;

	//
	// Iterate through the entries in m_lstACEs
	//
	POSITION	pos	=	m_lstACEs.GetHeadPosition ();

	while (pos)
	{
		// Get the current list element
		CACE*	pACE	=	m_lstACEs.GetNext (pos);

		//
		// In ONE case a combination of access modes is allowed (audit success + audit fail). We have to copy
		// the ACE and set both modes separately because SetEntriesInAcl is rather picky about this.
		//
		if (pACE->m_nAccessMode == SET_AUDIT_FAILURE + SET_AUDIT_SUCCESS)
		{
			pACE->m_nAccessMode	=	SET_AUDIT_FAILURE;

			CACE*	pACE2				=	CopyACE (pACE);
			pACE2->m_nAccessMode	=	SET_AUDIT_SUCCESS;

			POSITION posNew		=	m_lstACEs.AddTail (pACE2);

			// Make sure the new element is processed even if this is the last element
			if (! pos)
			{
				pos					=	posNew;
			}
		}

		// Reset the access mask
		pACE->m_nAccessMask	=	0;

		//
		// Multiple permissions can be specified. Process them all
		//
		CStringArray	asPermissions;

		// Split the permission list
		if (! Split (TEXT (","), pACE->m_sPermission, &asPermissions))
		{
			return RTN_ERR_PARAMS;
		}

		//
		// Loop through the permission list
		//
		for (int k = 0; k < asPermissions.GetSize (); k++)
		{
			//
			// FILE / DIRECTORY
			//
			if (m_nObjectType == SE_FILE_OBJECT)
			{
				if (asPermissions[k].CompareNoCase (TEXT ("read")) == 0)
				{
					pACE->m_nAccessMask			|=	MY_DIR_READ_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write")) == 0)
				{
					pACE->m_nAccessMask			|= MY_DIR_WRITE_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("list_folder")) == 0)
				{
					pACE->m_nAccessMask			|= MY_DIR_LIST_FOLDER_ACCESS;

					if (! pACE->m_fInhSpecified)
					{
						pACE->m_fInhSpecified	=	true;
						pACE->m_nInheritance		=	CONTAINER_INHERIT_ACE;
					}
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("read_ex")) == 0)
				{
					pACE->m_nAccessMask			|= MY_DIR_READ_EXECUTE_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("change")) == 0)
				{
					pACE->m_nAccessMask			|= MY_DIR_CHANGE_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("profile")) == 0)
				{
					pACE->m_nAccessMask			|= MY_DIR_CHANGE_ACCESS | WRITE_DAC;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("full")) == 0)
				{
					pACE->m_nAccessMask			|= MY_DIR_FULL_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("traverse")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_TRAVERSE;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("list_dir")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_LIST_DIRECTORY;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("read_attr")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_READ_ATTRIBUTES;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("read_ea")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_READ_EA;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("add_file")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_ADD_FILE;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("add_subdir")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_ADD_SUBDIRECTORY;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write_attr")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_WRITE_ATTRIBUTES;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write_ea")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_WRITE_EA;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("del_child")) == 0)
				{
					pACE->m_nAccessMask			|= FILE_DELETE_CHILD;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("delete")) == 0)
				{
					pACE->m_nAccessMask			|= DELETE;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("read_dacl")) == 0)
				{
					pACE->m_nAccessMask			|= READ_CONTROL;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write_dacl")) == 0)
				{
					pACE->m_nAccessMask			|= WRITE_DAC;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write_owner")) == 0)
				{
					pACE->m_nAccessMask			|= WRITE_OWNER;
				}
				else
				{
					return RTN_ERR_INV_DIR_PERMS;
				}

				//
				// When setting permissions (excluding DENY), always set the SYNCHRONIZE flag
				//
				if (pACE->m_nAccessMode != DENY_ACCESS)
				{
					pACE->m_nAccessMask			|=	SYNCHRONIZE;
				}

				//
				// Set default inheritance flags if none were specified
				//
				if (! pACE->m_fInhSpecified)
				{
					pACE->m_nInheritance			=	SUB_CONTAINERS_AND_OBJECTS_INHERIT;
				}
			}

			//
			// PRINTER
			//
			else if (m_nObjectType == SE_PRINTER)
			{
				if (asPermissions[k].CompareNoCase (TEXT ("print")) == 0)
				{
					pACE->m_nAccessMask			|= MY_PRINTER_PRINT_ACCESS;

					if (! pACE->m_fInhSpecified)
					{
						pACE->m_nInheritance		= 0;
					}
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("man_printer")) == 0)
				{
					pACE->m_nAccessMask			|= MY_PRINTER_MAN_PRINTER_ACCESS;

					if (! pACE->m_fInhSpecified)
					{
						pACE->m_nInheritance		= 0;
					}
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("man_docs")) == 0)
				{
					// We have to set two different ACEs, in order to obtain standard manage documents permissions;
					// Only do it, if inheritance was NOT specified by the user. If it was, assume he knows what he is doing

					if (! pACE->m_fInhSpecified)
					{
						CACE*			pACE2			=	CopyACE (pACE);

						lstACEs2Add.AddTail (pACE2);

						pACE->m_nAccessMask		=	READ_CONTROL;
						pACE->m_nInheritance		=	CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE;

						pACE2->m_nAccessMask		=	MY_PRINTER_MAN_DOCS_ACCESS;
						pACE2->m_nInheritance	=	OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE;
					}
					else
					{
						pACE->m_nAccessMask		|=	MY_PRINTER_MAN_DOCS_ACCESS;
					}
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("full")) == 0)
				{
					// We have to set two different ACEs, in order to obtain standard manage documents permissions;
					// Only do it, if inheritance was NOT specified by the user. If it was, assume he knows what he is doing

					if (! pACE->m_fInhSpecified)
					{
						CACE*			pACE2			=	CopyACE (pACE);

						lstACEs2Add.AddTail (pACE2);

						pACE->m_nAccessMask		=	MY_PRINTER_MAN_PRINTER_ACCESS;
						pACE->m_nInheritance		=	0;

						pACE2->m_nAccessMask		=	MY_PRINTER_MAN_DOCS_ACCESS;
						pACE2->m_nInheritance	=	OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE;
					}
					else
					{
						pACE->m_nAccessMask		|=	MY_PRINTER_MAN_PRINTER_ACCESS;
					}
				}
				else
				{
					return RTN_ERR_INV_PRN_PERMS;
				}
			}

			//
			// REGISTRY
			//
			else if (m_nObjectType == SE_REGISTRY_KEY)
			{
				if (asPermissions[k].CompareNoCase (TEXT ("read")) == 0)
				{
					pACE->m_nAccessMask			|= MY_REG_READ_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("full")) == 0)
				{
					pACE->m_nAccessMask			|= MY_REG_FULL_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("query_val")) == 0)
				{
					pACE->m_nAccessMask			|= KEY_QUERY_VALUE;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("set_val")) == 0)
				{
					pACE->m_nAccessMask			|= KEY_SET_VALUE;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("create_subkey")) == 0)
				{
					pACE->m_nAccessMask			|= KEY_CREATE_SUB_KEY;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("enum_subkeys")) == 0)
				{
					pACE->m_nAccessMask			|= KEY_ENUMERATE_SUB_KEYS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("notify")) == 0)
				{
					pACE->m_nAccessMask			|= KEY_NOTIFY;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("create_link")) == 0)
				{
					pACE->m_nAccessMask			|= KEY_CREATE_LINK;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("delete")) == 0)
				{
					pACE->m_nAccessMask			|= DELETE;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write_dacl")) == 0)
				{
					pACE->m_nAccessMask			|= WRITE_DAC;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("write_owner")) == 0)
				{
					pACE->m_nAccessMask			|= WRITE_OWNER;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("read_access")) == 0)
				{
					pACE->m_nAccessMask			|= READ_CONTROL;
				}
				else
				{
					return RTN_ERR_INV_REG_PERMS;
				}

				//
				// Set default inheritance flags if none were specified
				//
				if (! pACE->m_fInhSpecified)
				{
					pACE->m_nInheritance			=	CONTAINER_INHERIT_ACE;
				}
			}

			//
			// SERVICE
			//
			else if (m_nObjectType == SE_SERVICE)
			{
				if (asPermissions[k].CompareNoCase (TEXT ("read")) == 0)
				{
					pACE->m_nAccessMask			|= MY_SVC_READ_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("start_stop")) == 0)
				{
					pACE->m_nAccessMask			|= MY_SVC_STARTSTOP_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("full")) == 0)
				{
					pACE->m_nAccessMask			|= MY_SVC_FULL_ACCESS;
				}
				else
				{
					return RTN_ERR_INV_SVC_PERMS;
				}

				//
				// Set default inheritance flags if none were specified
				//
				if (! pACE->m_fInhSpecified)
				{
					pACE->m_nInheritance			=	0;
				}
			}

			//
			// SHARE
			//
			else if (m_nObjectType == SE_LMSHARE)
			{
				if (asPermissions[k].CompareNoCase (TEXT ("read")) == 0)
				{
					pACE->m_nAccessMask			|= MY_SHARE_READ_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("change")) == 0)
				{
					pACE->m_nAccessMask			|= MY_SHARE_CHANGE_ACCESS;
				}
				else if (asPermissions[k].CompareNoCase (TEXT ("full")) == 0)
				{
					pACE->m_nAccessMask			|= MY_SHARE_FULL_ACCESS;
				}
				else
				{
					return RTN_ERR_INV_SHR_PERMS;
				}

				//
				// Set default inheritance flags if none were specified
				//
				if (! pACE->m_fInhSpecified)
				{
					pACE->m_nInheritance			=	0;
				}
			}

			//
			// Never set the SYNCHRONIZE flag on audit ACEs or ANY type of access will be audited, not only the one specified.
			//
			if (pACE->m_nAccessMode == SET_AUDIT_FAILURE || pACE->m_nAccessMode == SET_AUDIT_SUCCESS)
			{
				pACE->m_nAccessMask				&=	~SYNCHRONIZE;
			}
		}
	}

	// Maybe additional ACEs were generated. Append them to the main ACE list.
	m_lstACEs.AddTail (&lstACEs2Add);

	// Remove all temporary list entries. Objects pointed to by the list members are NOT destroyed!
	lstACEs2Add.RemoveAll ();

	return RTN_OK;
}


//
// CopyACE: Copy an existing ACE
//
CACE* CSetACL::CopyACE (CACE* pACE)
{
	if (! pACE)
	{
		return NULL;
	}

	CTrustee*	pTrustee		=	new CTrustee  (pACE->m_pTrustee->m_sTrustee, pACE->m_pTrustee->m_fTrusteeIsSID, pACE->m_pTrustee->m_nAction, pACE->m_pTrustee->m_fDACL, pACE->m_pTrustee->m_fSACL);

	if (! pTrustee) return NULL;

	pTrustee->m_psidTrustee	=	CopySID (pACE->m_pTrustee->m_psidTrustee);

	if (! pTrustee->m_psidTrustee) return NULL;

	CACE*			pACENew		=	new CACE (pTrustee, pACE->m_sPermission, pACE->m_nInheritance, pACE->m_fInhSpecified, pACE->m_nAccessMode, pACE->m_nACLType);

	if (! pACENew) return NULL;

	pACENew->m_nAccessMask	=	pACE->m_nAccessMask;

	// Adjust the internal ACE count
	if (pACE->m_nACLType == ACL_DACL)
	{
		m_nDACLEntries++;
	}
	else if (pACE->m_nACLType == ACL_SACL)
	{
		m_nSACLEntries++;
	}

	return pACENew;
}


//
// DoActionList: Create a permission listing
//
DWORD CSetACL::DoActionList ()
{
	DWORD	nError;

	// Close the output file should it be open
	if (m_fhBackupRestoreFile)
	{
		fclose (m_fhBackupRestoreFile);
		m_fhBackupRestoreFile	=	NULL;
	}

	// Was an output file specified?
	if (! m_sBackupRestoreFile.IsEmpty ())
	{
		// Open the output file
		errno_t	nErr	= 0;
		nErr	=	_tfopen_s (&m_fhBackupRestoreFile, m_sBackupRestoreFile, TEXT ("wb"));

		if (m_fhBackupRestoreFile != NULL && nErr == 0)
		{
			// Write the unicode BOM (byte order mark)
			WCHAR cBOM	=	0xFEFF;
			fwrite (&cBOM, sizeof (WCHAR), 1, m_fhBackupRestoreFile);
		}
		else
		{
			return RTN_ERR_OPEN_LOGFILE;
		}
	}

	if (m_nObjectType == SE_FILE_OBJECT)
	{
		nError =	RecurseDirs (m_sObjectPath, &CSetACL::ListSD);
	}
	else if (m_nObjectType == SE_REGISTRY_KEY)
	{
		nError =	RecurseRegistry (m_sObjectPath, &CSetACL::ListSD);
	}
	else
	{
		nError =	ListSD (m_sObjectPath);
	}

	// Close the output file
	if (m_fhBackupRestoreFile)
	{
		fclose (m_fhBackupRestoreFile);
		m_fhBackupRestoreFile	=	NULL;
	}

	return nError;
}


//
// DoActionRestore: Restore permissions from a file
//
DWORD CSetACL::DoActionRestore ()
{
	DWORD						nError			=	RTN_OK;
	PACL						paclDACL			=	NULL;
	PACL						paclSACL			=	NULL;
	PSID						psidOwner		=	NULL;
	PSID						psidGroup		=	NULL;
	PSECURITY_DESCRIPTOR	pSDSelfRel		=	NULL;
	PSECURITY_DESCRIPTOR	pSDAbsolute		=	NULL;
	UINT						nRead				=	0;
	CSD*						csdSD				=	NULL;
	CString					sLineIn;
	errno_t					nErr				= 0;


	// Was an input file specified?
	if (m_sBackupRestoreFile.IsEmpty ())
	{
		return RTN_ERR_NO_LOGFILE;
	}

	// Close input file should it be open
	if (m_fhBackupRestoreFile)
	{
		fclose (m_fhBackupRestoreFile);
		m_fhBackupRestoreFile	=	NULL;
	}

	// Open input file for reading in binary mode
	nErr	=	_tfopen_s (&m_fhBackupRestoreFile, m_sBackupRestoreFile, TEXT ("rb"));

	if (m_fhBackupRestoreFile == NULL || nErr != 0)
	{
		return RTN_ERR_OPEN_LOGFILE;
	}

	//
	// Check for the unicode BOM (byte order mark)
	//
	// Read two bytes...
	WCHAR cCheckBOM;
	nRead	=	(UINT) fread (&cCheckBOM, sizeof (WCHAR), 1, m_fhBackupRestoreFile);

	if (nRead != 1)
	{
		return RTN_ERR_READ_LOGFILE;
	}

	// ... and compare with the BOM
	if (cCheckBOM == 0xFEFF)
	{
		// This should be a unicode file
		LogMessage (TEXT ("INFO: Input file for restore operation: <" + m_sBackupRestoreFile + "> detected as Unicode."));
	}
	else
	{
		// This should be an ANSI (1 byte per char) file
		LogMessage (TEXT ("INFO: Input file for restore operation: <" + m_sBackupRestoreFile + "> detected as ANSI."));

		// Reopen again in text mode
		fclose (m_fhBackupRestoreFile);
		nErr	=	_tfopen_s (&m_fhBackupRestoreFile, m_sBackupRestoreFile, TEXT ("r"));

		if (m_fhBackupRestoreFile == NULL || nErr != 0)
		{
			return RTN_ERR_OPEN_LOGFILE;
		}
	}

	//
	// Read and process the input file line by line
	//
	while (_fgetts (sLineIn.GetBuffer (8192), 8190, m_fhBackupRestoreFile))
	{
		sLineIn.ReleaseBuffer ();

		// Free buffers that may have been allocated in the previous iteration
		if (pSDSelfRel)	{LocalFree (pSDSelfRel);	pSDSelfRel	=	NULL;}
		if (pSDAbsolute)	{LocalFree (pSDAbsolute);	pSDAbsolute	=	NULL;}
		if (paclDACL)		{LocalFree (paclDACL);		paclDACL		=	NULL;}
		if (paclSACL)		{LocalFree (paclSACL);		paclSACL		=	NULL;}
		if (psidOwner)		{LocalFree (psidOwner);		psidOwner	=	NULL;}
		if (psidGroup)		{LocalFree (psidGroup);		psidGroup	=	NULL;}

		DWORD						nBufSD			=	0;
		DWORD						nBufDACL			=	0;
		DWORD						nBufSACL			=	0;
		DWORD						nBufOwner		=	0;
		DWORD						nBufGroup		=	0;
		CString					sObjectPath;
		CString					sSDDL;
		SE_OBJECT_TYPE			nObjectType		=	SE_UNKNOWN_OBJECT_TYPE;
		SECURITY_INFORMATION	siSecInfo		=	0;

		// Remove unnecessary characters from the beginning and the end
		sLineIn.TrimRight (TEXT ("\r\n\""));
		sLineIn.TrimLeft (TEXT ("\""));

		// Split the line into object path, object type and SDDL string
		DWORD nFind1	=	sLineIn.Find (TEXT ("\","));
		sObjectPath		=	sLineIn.Left (nFind1);
		DWORD nFind2	=	sLineIn.Find (TEXT (",\""), nFind1 + 1);
		nObjectType		=	(SE_OBJECT_TYPE) _ttoi (sLineIn.Mid (nFind1 + 2, nFind2 - nFind1 - 1));
		sSDDL				=	sLineIn.Right (sLineIn.GetLength () - nFind2 - 2);
		sSDDL.MakeUpper ();

		// Check if the current path is on the filter list. If yes -> next line
		if (CheckFilterList (sObjectPath))
		{
			// Notify caller of omission
			LogMessage (TEXT ("INFO: Omitting ACL of: <" + sObjectPath + "> because a filter keyword matched."));

			continue;
		}

		// Notify caller of progress
		LogMessage (TEXT ("INFO: Restoring ACL of: <" + sObjectPath + ">"));

		//
		// Which parts of the SD do we set? Also determine DACL and SACL flags.
		//
		CString	sFlags;

		DWORD		nFind		=	sSDDL.Find (TEXT ("D:"));
		if (nFind != -1)
		{
			// Process the DACL
			siSecInfo		|=	DACL_SECURITY_INFORMATION;

			// Find the flags
			DWORD	nFlags	=	UNPROTECTED_DACL_SECURITY_INFORMATION;
			DWORD	nFind3	=	sSDDL.Find (TEXT ("("), nFind + 2);

			if (nFind3 != -1)
			{
				sFlags		=	sSDDL.Mid (nFind + 2, nFind3 - nFind - 2);
			}

			// Check for the protection flag
			if (sFlags.Find (TEXT ("P")) != -1)
			{
				nFlags		=	PROTECTED_DACL_SECURITY_INFORMATION;
			}

			// Set the appropriate flag(s)
			siSecInfo		|=	nFlags;
		}

		nFind					=	sSDDL.Find (TEXT ("S:"));
		if (nFind != -1)
		{
			// Process the SACL
			siSecInfo		|=	SACL_SECURITY_INFORMATION;

			// Find the flags
			DWORD	nFlags	=	UNPROTECTED_SACL_SECURITY_INFORMATION;
			DWORD	nFind3	=	sSDDL.Find (TEXT ("("), nFind + 2);

			if (nFind3 != -1)
			{
				sFlags		=	sSDDL.Mid (nFind + 2, nFind3 - nFind - 2);
			}

			// Check for the protection flag
			if (sFlags.Find (TEXT ("P")) != -1)
			{
				nFlags		=	PROTECTED_SACL_SECURITY_INFORMATION;
			}

			// Set the appropriate flag(s)
			siSecInfo		|=	nFlags;
		}
		if (sSDDL.Find (TEXT ("O:")) != -1)
		{
			// Process the owner
			siSecInfo		|=	OWNER_SECURITY_INFORMATION;
		}
		if (sSDDL.Find (TEXT ("G:")) != -1)
		{
			// Process the primary group
			siSecInfo		|=	GROUP_SECURITY_INFORMATION;
		}

		// Convert the SDDL string into an SD
		if (! ConvertStringSecurityDescriptorToSecurityDescriptor (sSDDL, SDDL_REVISION_1, &pSDSelfRel, NULL))
		{
			if (m_fIgnoreErrors)
			{
				LogMessage (TEXT ("ERROR: Restoring ACL of <" + sObjectPath + ">: " + GetLastErrorMessage (GetLastError ())));

				continue;
			}
			else
			{
				nError		=	RTN_ERR_CONVERT_SD;
				m_nAPIError	=	GetLastError ();

				goto CleanUp;
			}
		}

		// Convert the self-relative SD into an absolute SD:
		// 1. Determine the size of the buffers needed
		MakeAbsoluteSD (pSDSelfRel, pSDAbsolute, &nBufSD, paclDACL, &nBufDACL, paclSACL, &nBufSACL, psidOwner, &nBufOwner, psidGroup, &nBufGroup);
		if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
		{
			if (m_fIgnoreErrors)
			{
				LogMessage (TEXT ("ERROR: Restoring ACL of <" + sObjectPath + ">: " + GetLastErrorMessage (GetLastError ())));

				continue;
			}
			else
			{
				nError		=	RTN_ERR_CONVERT_SD;
				m_nAPIError	=	GetLastError ();

				goto CleanUp;
			}
		}

		// 2. Allocate the buffers
		pSDAbsolute		=	(PSECURITY_DESCRIPTOR)	LocalAlloc (LPTR, nBufSD);
		paclDACL			=	(PACL)						LocalAlloc (LPTR, nBufDACL);
		paclSACL			=	(PACL)						LocalAlloc (LPTR, nBufSACL);
		psidOwner		=	(PSID)						LocalAlloc (LPTR, nBufOwner);
		psidGroup		=	(PSID)						LocalAlloc (LPTR, nBufGroup);

		// 3. Do the conversion
		if (! MakeAbsoluteSD (pSDSelfRel, pSDAbsolute, &nBufSD, paclDACL, &nBufDACL, paclSACL, &nBufSACL, psidOwner, &nBufOwner, psidGroup, &nBufGroup))
		{
			if (m_fIgnoreErrors)
			{
				LogMessage (TEXT ("ERROR: Restoring ACL of <" + sObjectPath + ">: " + GetLastErrorMessage (GetLastError ())));

				continue;
			}
			else
			{
				nError		=	RTN_ERR_CONVERT_SD;
				m_nAPIError	=	GetLastError ();

				goto CleanUp;
			}
		}

		// Set the SD on the object
		csdSD				=	new CSD (this);
		nError			=	csdSD->SetSD (sObjectPath.GetBuffer (sObjectPath.GetLength () + 1), nObjectType, siSecInfo, paclDACL, paclSACL, psidOwner, psidGroup);
		m_nAPIError		=	csdSD->m_nAPIError;

		sObjectPath.ReleaseBuffer ();

		if (nError != RTN_OK)
		{
			LogMessage (TEXT ("ERROR: Writing SD to <" + sObjectPath + "> failed with: " + GetLastErrorMessage (m_nAPIError)));

			if (m_nAPIError == ERROR_ACCESS_DENIED || m_nAPIError == ERROR_FILE_NOT_FOUND || m_fIgnoreErrors)
			{
				// The error can be ignored -> suppress and next line
				m_nAPIError	=	ERROR_SUCCESS;
				nError		=	RTN_OK;

				continue;
			}

			goto CleanUp;
		}
	}

	sLineIn.ReleaseBuffer ();

CleanUp:

	// If errors are to be ignored (param "-ignoreerr" set), then do NOT return an error
	if (nError == RTN_ERR_IGNORED)
	{
		m_nAPIError	=	ERROR_SUCCESS;
		nError		=	RTN_OK;
	}

	// Close the input file
	if (m_fhBackupRestoreFile)
	{
		fclose (m_fhBackupRestoreFile);
		m_fhBackupRestoreFile	=	NULL;
	}

	// Free Memory
	if (pSDSelfRel)	{LocalFree (pSDSelfRel);	pSDSelfRel	=	NULL;}
	if (pSDAbsolute)	{LocalFree (pSDAbsolute);	pSDAbsolute	=	NULL;}
	if (paclDACL)		{LocalFree (paclDACL);		paclDACL		=	NULL;}
	if (paclSACL)		{LocalFree (paclSACL);		paclSACL		=	NULL;}
	if (psidOwner)		{LocalFree (psidOwner);		psidOwner	=	NULL;}
	if (psidGroup)		{LocalFree (psidGroup);		psidGroup	=	NULL;}
	if (csdSD)			{delete csdSD;}

	return nError;
}


//
// DoActionWrite: Process actions that write to the SD
//
DWORD CSetACL::DoActionWrite ()
{
	if (m_nObjectType == SE_FILE_OBJECT)
	{
		return RecurseDirs (m_sObjectPath, &CSetACL::Write2SD);
	}
	else if (m_nObjectType == SE_REGISTRY_KEY)
	{
		return RecurseRegistry (m_sObjectPath, &CSetACL::Write2SD);
	}
	else
	{
		return Write2SD (m_sObjectPath);
	}
}


//
// RecurseDirs: Recurse a directory structure and call the function for every file / dir
//
DWORD CSetACL::RecurseDirs (CString sObjectPath, DWORD (CSetACL::*funcProcess) (CString sObjectPath))
{
	WIN32_FIND_DATA		FindFileData;
	HANDLE					hFind;
	DWORD						nError			=	RTN_OK;
	BOOL						fRootDrive		=	false;

	// If no recursion is desired, process only the current path
	if (m_nRecursionType & RECURSE_NO)
	{
		nError	=	(this->*funcProcess) (sObjectPath);

		return nError;
	}

	// Also do that if a local file system root (ie. "c:") is to be processed
	if (sObjectPath.GetAt (sObjectPath.GetLength () - 1) == ':')
	{
		if (m_nRecursionType & RECURSE_CONT || m_nRecursionType & RECURSE_OBJ)
		{
			LogMessage (TEXT ("WARNING: Recursion is not possible for local file system roots. You may want to append a backslash ('c:\\')."));
		}
		
		nError	=	(this->*funcProcess) (sObjectPath);

		return nError;
	}

	// Remove a trailing backslash to get a consistent state
	sObjectPath.TrimRight (TEXT ("\\"));

	//
	// The path specified may be the root of a drive. FindFirstFile cannot handle that (because a drive root is no file/directory) -> we'll do it ourselves.
	//
	if (sObjectPath.GetAt (sObjectPath.GetLength () - 1) == ':')
	{
		// It is a root drive ('c:\')
		fRootDrive		=	1;
	}
	else if (sObjectPath.Left (7).CompareNoCase (TEXT ("\\\\?\\UNC")) == 0)
	{
		// This is a UNC path - get the number of backslashes to determine whether it is a share ('\\server\share') or a directory inside a share ('\\server\share\dir')
		DWORD	nPos		=	1;
		DWORD nFound	=	0;
		while ((nPos = sObjectPath.Find ('\\', nPos + 1)) != -1)
		{
			nFound++;
		}

		if (nFound == 3)
		{
			// It is a root drive ('\\?\UNC\server\share')
			fRootDrive	=	2;
		}
	}

	// Is the path a drive root? Process it and start to recurse, if necessary.
	if (fRootDrive)
	{
		// Process the root only if specified by recursion options!

		if (m_nRecursionType & RECURSE_CONT)
		{
			if (fRootDrive == 1)
			{
				// This is a local drive. Append a backslash, because c:\ is different from c:!!

				// Call the function provided
				nError	=	(this->*funcProcess) (sObjectPath + TEXT ("\\"));
			}
			else
			{
				// This is a UNC path.

				// Call the function provided
				nError	=	(this->*funcProcess) (sObjectPath);
			}

			if (nError != RTN_OK)
			{
				return nError;
			}
		}

		// Modify the path so FindFirstFile will accept it
		sObjectPath	+=	TEXT ("\\*.*");
	}

	//
	// Start recursively processing the path
	//
	hFind = FindFirstFile (sObjectPath, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		CString sDir = sObjectPath.Left (sObjectPath.ReverseFind ('\\') + 1);
		do
		{
			// The directories '.' and '..' are returned, too -> ignore
			CString sTmp1	=	FindFileData.cFileName;
			if (sTmp1 == TEXT (".") || sTmp1 == TEXT (".."))
			{
				continue;
			}

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (m_nRecursionType & RECURSE_CONT)
				{
					// Directories are processed. Start recursion for found dirs.
					// Process the directory...
					nError	=	(this->*funcProcess) (sDir + FindFileData.cFileName);

					if (nError != RTN_OK)
					{
						return nError;
					}
				}

				// ...and continue recursion
				nError	=	RecurseDirs (sDir + FindFileData.cFileName + TEXT ("\\*.*"), funcProcess);

				if (nError != RTN_OK)
				{
					return nError;
				}
			}

			if (! (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// Files are processed
				if (m_nRecursionType & RECURSE_OBJ)
				{
					// Process the file
					nError	=	(this->*funcProcess) (sDir + FindFileData.cFileName);

					if (nError != RTN_OK)
					{
						return nError;
					}
				}
			}
		}
		while (FindNextFile (hFind, &FindFileData));

		FindClose (hFind);
	}
	else
	{
		// FindFirstFile reported an error: probably an invalid path
		m_nAPIError	=	GetLastError ();
		return RTN_ERR_FINDFILE;
	}

	return nError;
}


//
// OpenRegKey: Open a registry key
//
DWORD CSetACL::OpenRegKey (CString* sObjectPath, PHKEY hSubKey)
{
	int								nLocalPart		=	0;
	int								nMainKey			=	0;
	HKEY								hMainKey			=	NULL;
	HKEY								hRemoteKey		=	NULL;
	HKEY								hOpenStd			=	NULL;
	HKEY								hOpenBckp		=	NULL;
	CString							sSubkeyPath;
	CString							sMainKey;
	CString							sMachinePath;
	CString							sLocalPath;

	// No backslash at the end of the path
	(*sObjectPath).TrimRight (TEXT ("\\"));

	// Talk to the registry on the local or on a remote computer?
	if ((*sObjectPath).Left (2) == TEXT ("\\\\"))
	{
		// Find end of computer name
		nLocalPart = (*sObjectPath).Find (TEXT ("\\"), 2);

		// Nothing found? -> exit with error
		if (nLocalPart == -1)
		{
			return RTN_ERR_REG_PATH;
		}

		// Split into computer name and rest of path
		sMachinePath	=	(*sObjectPath).Left (nLocalPart);
		sLocalPath		=	(*sObjectPath).Right ((*sObjectPath).GetLength () - nLocalPart - 1);
	}
	else
	{
		// sMachinePath stays empty -> talk to the local machine
		sLocalPath		=	(*sObjectPath);
	}

	// Find the registry root
	nMainKey				=	sLocalPath.Find (TEXT ("\\"));

	if (nMainKey == -1)
	{
		// One of the root keys was specified
		sMainKey			=	sLocalPath;
	}
	else
	{
		// A subkey was specified
		sMainKey			=	sLocalPath.Left (nMainKey);
		sSubkeyPath		=	sLocalPath.Right (sLocalPath.GetLength () - nMainKey - 1);
	}

	//
	// Make registry paths easier to use: accept formats: machine, hklm, hkey_local_machine
	//
	if (sMainKey.CompareNoCase (TEXT ("hklm")) == 0)
	{
		sMainKey			=	TEXT ("machine");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hkey_local_machine")) == 0)
	{
		sMainKey			=	TEXT ("machine");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hku")) == 0)
	{
		sMainKey			=	TEXT ("users");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hkey_users")) == 0)
	{
		sMainKey			=	TEXT ("users");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hkcr")) == 0)
	{
		sMainKey			=	TEXT ("classes_root");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hkey_classes_root")) == 0)
	{
		sMainKey			=	TEXT ("classes_root");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hkcu")) == 0)
	{
		sMainKey			=	TEXT ("current_user");
	}
	else if (sMainKey.CompareNoCase (TEXT ("hkey_current_user")) == 0)
	{
		sMainKey			=	TEXT ("current_user");
	}

	// Build a corrected object path
	if (sMachinePath > TEXT (""))
	{
		*sObjectPath	=	sMachinePath + TEXT ("\\");
	}
	else
	{
		*sObjectPath	=	TEXT ("");
	}

	*sObjectPath		+=	sMainKey;

	if (sSubkeyPath > TEXT (""))
	{
		*sObjectPath	+=	TEXT ("\\") + sSubkeyPath;
	}

	// HKEY_CLASSES_ROOT and HKEY_CURRENT_USER cannot be used with RegConnectRegistry
	if (sMachinePath > TEXT (""))
	{
		if (sMainKey.CompareNoCase (TEXT ("classes_root")) == 0 || sMainKey.CompareNoCase (TEXT ("current_user")) == 0)
		{
			return RTN_ERR_REG_PATH;
		}
	}

	// Which registry hive?
	if (sMainKey.CompareNoCase (TEXT ("machine")) == 0)
	{
		hMainKey			=	HKEY_LOCAL_MACHINE;
	}
	else if (sMainKey.CompareNoCase (TEXT ("users")) == 0)
	{
		hMainKey			=	HKEY_USERS;
	}
	else if (sMainKey.CompareNoCase (TEXT ("classes_root")) == 0)
	{
		hMainKey			=	HKEY_CLASSES_ROOT;
	}
	else if (sMainKey.CompareNoCase (TEXT ("current_user")) == 0)
	{
		hMainKey			=	HKEY_CURRENT_USER;
	}
	else
	{
		return RTN_ERR_REG_PATH;
	}

	// Now we can connect to the remote registry
	if (sMachinePath > TEXT (""))
	{
		m_nAPIError		=	RegConnectRegistry (sMachinePath, hMainKey, &hRemoteKey);

		if (m_nAPIError != ERROR_SUCCESS)
		{
			return RTN_ERR_REG_CONNECT;
		}

		hMainKey			=	hRemoteKey;
	}

	//
	// Open the key specified either on the remote or the local machine
	//

	// Open the key using regular methods
	m_nAPIError			=	RegOpenKeyEx (hRemoteKey ? hRemoteKey : hMainKey,  sSubkeyPath, 0, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE, &hOpenStd);

	// We now know the key exists. Let's try some black magic and open it like a backup program
	if (hOpenStd && (m_nAPIError == ERROR_SUCCESS || m_nAPIError == ERROR_ACCESS_DENIED))
	{
		DWORD	nNewCreated	=	0;
		DWORD nErrTmp	=	0;

		nErrTmp			=	RegCreateKeyEx (hRemoteKey ? hRemoteKey : hMainKey,  sSubkeyPath, 0, NULL, REG_OPTION_BACKUP_RESTORE, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE, NULL, &hOpenBckp, &nNewCreated);

		// Assert we did not unintentionally create a new key
		if (nNewCreated == REG_CREATED_NEW_KEY)
		{
			LogMessage (TEXT ("ERROR: Critical internal error. Unintentionally the following registry key was created: <" + *sObjectPath + ">."));

			throw RTN_ERR_GENERAL;
		}

		// Check which opened key to use (standard or with privileges)
		if (hOpenBckp && nErrTmp == ERROR_SUCCESS)
		{
			*hSubKey		=	hOpenBckp;

			// The standard key is not needed
			RegCloseKey (hOpenStd);
		}
		else
		{
			*hSubKey		=	hOpenStd;
		}
	}

	// The remote key is no longer needed - we have a handle to the subkey
	if (hRemoteKey) RegCloseKey (hRemoteKey);

	if (m_nAPIError != ERROR_SUCCESS)
	{
		return RTN_ERR_REG_OPEN;
	}

	return RTN_OK;
}


//
// RecurseRegistry: Recurse the registry and call the function for every key
//
DWORD CSetACL::RecurseRegistry (CString  sObjectPath, DWORD (CSetACL::*funcProcess) (CString sObjectPath))
{
	CString							sFindKey;
	CArray<CString, CString>	asSubKeys;
	DWORD								nError			=	RTN_OK;
	DWORD								nBuffer			=	512;
	HKEY								hSubKey			=	NULL;
	PFILETIME						pFileTime		=	NULL;

	// Open the key
	nError	=	OpenRegKey (&sObjectPath, &hSubKey);

	if (nError != RTN_OK)
	{
		return nError;
	}

	// We know now that the path passed is valid -> process the current key
	nError	=	(this->*funcProcess) (sObjectPath);

	// Stop here on error or if no recursion is desired
	if (nError != RTN_OK || m_nRecursionType & RECURSE_NO)
	{
		return nError;
	}

	// Check for recursion type desired
	if (m_nRecursionType != RECURSE_CONT)
	{
		RTN_ERR_PARAMS;
	}

	// We have to save the subkeys since they change when setting permissions!
	m_nAPIError			=	RegEnumKeyEx (hSubKey, 0, sFindKey.GetBuffer (512), &nBuffer, NULL, NULL, NULL, pFileTime);
	sFindKey.ReleaseBuffer ();
	for (int i = 1; m_nAPIError == ERROR_SUCCESS; i++)
	{
		asSubKeys.Add (sFindKey);

		nBuffer			=	512;
		m_nAPIError		=	RegEnumKeyEx (hSubKey, i, sFindKey.GetBuffer (512), &nBuffer, NULL, NULL, NULL, pFileTime);
		sFindKey.ReleaseBuffer ();
	}

	if (m_nAPIError != ERROR_NO_MORE_ITEMS)
	{
		if (hSubKey) RegCloseKey (hSubKey);

		return RTN_ERR_REG_ENUM;
	}
	else
	{
		m_nAPIError		=	ERROR_SUCCESS;
	}

	if (hSubKey) RegCloseKey (hSubKey);

	// Process each subkey (start recursion)
	for (int i = 0; i < asSubKeys.GetSize (); i++)
	{
		RecurseRegistry (sObjectPath + TEXT ("\\") + asSubKeys.GetAt (i), funcProcess);
	}

	return nError;
}


//
// Write2SD: Set/Add the ACEs, owner and primary group specified to the ACLs
//
DWORD CSetACL::Write2SD (CString sObjectPath)
{
	EXPLICIT_ACCESS					*eaDACL					=	NULL;
	EXPLICIT_ACCESS					*eaSACL					=	NULL;
	PACL									paclDACLNew				=	NULL;
	PACL									paclSACLNew				=	NULL;
	DWORD									nError					=	RTN_OK;
	POSITION								pos;
	DWORD									nDACLACEs				=	0;
	DWORD									nSACLACEs				=	0;
	SECURITY_INFORMATION				siSecInfo				=	0;
	BOOL									fDelInhACEsFromDACL	=	false;
	BOOL									fDelInhACEsFromSACL	=	false;
	CSD*									csdSD						=	NULL;


	//
	// If sub-object-only processing is enabled: check if this is a sub-object or not
	//
	if (m_fProcessSubObjectsOnly)
	{
		// Remove a trailing backslash to get a consistent state
		CString	sPath1	=	sObjectPath;
		CString	sPath2	=	m_sObjectPath;

		sPath1.TrimRight (TEXT ("\\"));
		sPath2.TrimRight (TEXT ("\\"));

		if (sPath1.CompareNoCase (sPath2) == 0)
		{
			// This is the object itself -> do nothing
			return RTN_OK;
		}
	}

	// Check if the current path is on the filter list. If yes -> do nothing
	if (CheckFilterList (sObjectPath))
	{
		// Notify caller of omission
		LogMessage (TEXT ("INFO: Omitting ACL of: <" + sObjectPath + "> because a filter keyword matched."));

		return RTN_OK;
	}

	// Notify caller of progress
	LogMessage (TEXT ("INFO: Processing ACL of: <" + sObjectPath + ">"));

	//
	// Check which elements of the SD we need to process, depending on the action(s) desired
	//
	if (m_nDACLEntries && (m_nAction & ACTN_ADDACE))
	{
		siSecInfo		|=	DACL_SECURITY_INFORMATION;
	}
	if (m_nSACLEntries && (m_nAction & ACTN_ADDACE))
	{
		siSecInfo		|=	SACL_SECURITY_INFORMATION;
	}
	if (m_pOwner->m_psidTrustee && (m_nAction & ACTN_SETOWNER))
	{
		siSecInfo		|=	OWNER_SECURITY_INFORMATION;
	}
	if (m_pPrimaryGroup->m_psidTrustee && (m_nAction & ACTN_SETGROUP))
	{
		siSecInfo		|=	GROUP_SECURITY_INFORMATION;
	}

	if (m_nAction & ACTN_SETINHFROMPAR)
	{
		if (m_nDACLProtected	== INHPARYES)
		{
			siSecInfo	|=	DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION;
		}
		else if (m_nDACLProtected	== INHPARCOPY)
		{
			siSecInfo	|=	DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION;
		}
		else if (m_nDACLProtected	== INHPARNOCOPY)
		{
			// Remove inherited ACEs
			fDelInhACEsFromDACL	=	true;

			siSecInfo	|=	DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION;
		}

		if (m_nSACLProtected	== INHPARYES)
		{
			siSecInfo	|=	SACL_SECURITY_INFORMATION | UNPROTECTED_SACL_SECURITY_INFORMATION;
		}
		else if (m_nSACLProtected	== INHPARCOPY)
		{
			siSecInfo	|=	SACL_SECURITY_INFORMATION | PROTECTED_SACL_SECURITY_INFORMATION;
		}
		else if (m_nSACLProtected	== INHPARNOCOPY)
		{
			// Remove inherited ACEs
			fDelInhACEsFromSACL	=	true;

			siSecInfo	|=	SACL_SECURITY_INFORMATION | PROTECTED_SACL_SECURITY_INFORMATION;
		}
	}

	if (m_nAction & ACTN_CLEARDACL)
	{
		siSecInfo	|=	DACL_SECURITY_INFORMATION;
	}
	if (m_nAction & ACTN_CLEARSACL)
	{
		siSecInfo	|=	SACL_SECURITY_INFORMATION;
	}

	if ((m_nAction & ACTN_TRUSTEE) && m_fTrusteesProcessDACL)
	{
		siSecInfo	|=	DACL_SECURITY_INFORMATION;
	}
	if ((m_nAction & ACTN_TRUSTEE) && m_fTrusteesProcessSACL)
	{
		siSecInfo	|=	SACL_SECURITY_INFORMATION;
	}

	if ((m_nAction & ACTN_DOMAIN) && m_fDomainsProcessDACL)
	{
		siSecInfo	|=	DACL_SECURITY_INFORMATION;
	}
	if ((m_nAction & ACTN_DOMAIN) && m_fDomainsProcessSACL)
	{
		siSecInfo	|=	SACL_SECURITY_INFORMATION;
	}


	//
	// Get the SD
	//
	csdSD					=	new CSD (this);

	nError				=	csdSD->GetSD (sObjectPath, m_nObjectType, siSecInfo);
	m_nAPIError			=	csdSD->m_nAPIError;

	if (nError != RTN_OK)
	{
		if (m_fIgnoreErrors)
		{
			LogMessage (TEXT ("ERROR (ignored): Reading the SD from <" + sObjectPath + "> failed with: " + GetLastErrorMessage (m_nAPIError)));
			nError		=	RTN_ERR_IGNORED;
		}
		else
		{
			LogMessage (TEXT ("ERROR: Reading the SD from <" + sObjectPath + "> failed with: " + GetLastErrorMessage (m_nAPIError)));
		}

		goto CleanUp;
	}

	//
	// Remove inherited ACEs if necessary
	//
	if (fDelInhACEsFromDACL)
	{
		nError	=	csdSD->DeleteACEsByHeaderFlags (ACL_DACL, INHERITED_ACE, true);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}
	if (fDelInhACEsFromSACL)
	{
		nError	=	csdSD->DeleteACEsByHeaderFlags (ACL_SACL, INHERITED_ACE, true);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}

	//
	// Remove noninherited ACEs if necessary
	//
	if (m_nAction & ACTN_CLEARDACL)
	{
		nError	=	csdSD->DeleteACEsByHeaderFlags (ACL_DACL, INHERITED_ACE, false);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}
	if (m_nAction & ACTN_CLEARSACL)
	{
		nError	=	csdSD->DeleteACEsByHeaderFlags (ACL_SACL, INHERITED_ACE, false);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}

	//
	// Process specified trustees if necessary
	//
	if ((m_nAction & ACTN_TRUSTEE) && m_fTrusteesProcessDACL)
	{
		nError	=	csdSD->ProcessACEsOfGivenTrustees (ACL_DACL);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}

	if ((m_nAction & ACTN_TRUSTEE) && m_fTrusteesProcessSACL)
	{
		nError	=	csdSD->ProcessACEsOfGivenTrustees (ACL_SACL);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}

	//
	// Process specified domains if necessary
	//
	if ((m_nAction & ACTN_DOMAIN) && m_fDomainsProcessDACL)
	{
		nError	=	csdSD->ProcessACEsOfGivenDomains (ACL_DACL);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}

	if ((m_nAction & ACTN_DOMAIN) && m_fDomainsProcessSACL)
	{
		nError	=	csdSD->ProcessACEsOfGivenDomains (ACL_SACL);

		if (nError != RTN_OK)
		{
			goto CleanUp;
		}
	}

	//
	//	Process action AddACE
	//
	if (m_nAction & ACTN_ADDACE)
	{
		// Allocate memory for two lists of EXPLICIT_ACCESS structures
		if (m_nDACLEntries)
		{
			eaDACL			=	new EXPLICIT_ACCESS[m_nDACLEntries];
			
			if (eaDACL == NULL)
			{
				nError	= RTN_ERR_OUT_OF_MEMORY;
				goto CleanUp;
			}
		}
		if (m_nSACLEntries)
		{
			eaSACL			=	new EXPLICIT_ACCESS[m_nSACLEntries];
			
			if (eaSACL == NULL)
			{
				nError	= RTN_ERR_OUT_OF_MEMORY;
				goto CleanUp;
			}
		}

		//
		// We have got pointers to the DACL and SACL. Now loop through the ACEs specified and fill EXPLCIT_ACCESS structures describing them.
		//
		pos	=	m_lstACEs.GetHeadPosition ();

		while (pos)
		{
			CACE*	pACE	=	m_lstACEs.GetNext (pos);

			// Fill an EXPLICIT_ACCESS structure describing this ACE
			if (pACE->m_nACLType == ACL_DACL && m_nDACLEntries)
			{
				eaDACL[nDACLACEs].grfAccessMode			=	pACE->m_nAccessMode;
				eaDACL[nDACLACEs].grfAccessPermissions	=	pACE->m_nAccessMask;
				eaDACL[nDACLACEs].grfInheritance			=	pACE->m_nInheritance;
				eaDACL[nDACLACEs].Trustee.TrusteeForm	=	TRUSTEE_IS_SID;
				eaDACL[nDACLACEs].Trustee.ptstrName		=	(TCHAR *) pACE->m_pTrustee->m_psidTrustee;

				nDACLACEs++;
			}
			if (pACE->m_nACLType == ACL_SACL && m_nSACLEntries)
			{
				eaSACL[nSACLACEs].grfAccessMode			=	pACE->m_nAccessMode;
				eaSACL[nSACLACEs].grfAccessPermissions	=	pACE->m_nAccessMask;
				eaSACL[nSACLACEs].grfInheritance			=	pACE->m_nInheritance;
				eaSACL[nSACLACEs].Trustee.TrusteeForm	=	TRUSTEE_IS_SID;
				eaSACL[nSACLACEs].Trustee.ptstrName		=	(TCHAR *) pACE->m_pTrustee->m_psidTrustee;

				nSACLACEs++;
			}
		}	// if (m_nAction == ACTN_ADDACE)

		// ASSERT count is correct
		if (nDACLACEs != m_nDACLEntries || nSACLACEs != m_nSACLEntries)
		{
			nError	=	RTN_ERR_INTERNAL;

			goto CleanUp;
		}
	}

	//
	// Merge the existing (maybe modified) and (maybe empty) new ACL
	//
	m_nAPIError		=	SetEntriesInAcl (m_nDACLEntries, eaDACL, csdSD->m_paclDACL, &paclDACLNew);

	if (m_nAPIError != ERROR_SUCCESS)
	{
		nError		=	RTN_ERR_SETENTRIESINACL;

		goto CleanUp;
	}

	m_nAPIError		=	SetEntriesInAcl (m_nSACLEntries, eaSACL, csdSD->m_paclSACL, &paclSACLNew);

	if (m_nAPIError != ERROR_SUCCESS)
	{
		nError		=	RTN_ERR_SETENTRIESINACL;

		goto CleanUp;
	}

	//
	// Set the new ACLs, owner and primary group on the object
	//
	// Is there anything to do?
	if (siSecInfo)
	{
		nError		=	csdSD->SetSD (sObjectPath, m_nObjectType, siSecInfo, paclDACLNew, paclSACLNew, m_pOwner->m_psidTrustee, m_pPrimaryGroup->m_psidTrustee);
		m_nAPIError	=	csdSD->m_nAPIError;
	}

	if (nError != RTN_OK)
	{
		if (m_nAPIError == ERROR_ACCESS_DENIED || m_fIgnoreErrors)
		{
			LogMessage (TEXT ("ERROR: Writing SD to <" + sObjectPath + "> failed with: " + GetLastErrorMessage (m_nAPIError)));

			nError	=	RTN_ERR_IGNORED;
		}

		goto CleanUp;
	}

CleanUp:

	// If errors are to be ignored (param "-ignoreerr" set), then do NOT return an error
	if (nError == RTN_ERR_IGNORED)
	{
		m_nAPIError	=	ERROR_SUCCESS;
		nError		=	RTN_OK;
	}

	if (csdSD)			delete (csdSD);
	if (eaDACL)			delete [] eaDACL;
	if (eaSACL)			delete [] eaSACL;
	if (paclDACLNew)	LocalFree (paclDACLNew);
	if (paclSACLNew)	LocalFree (paclSACLNew);

	return nError;
}


//
// ListSD: List the contents of a SD in text format
//
DWORD CSetACL::ListSD (CString sObjectPath)
{
	DWORD									nError			=	RTN_OK;
	SECURITY_INFORMATION				siSecInfo		=	0;
	LPTSTR								sSDDL				=	NULL;
	CString								sLineOut;
	CString								sObjectType;
	CString								sSD;
	CString								sDACLControl;
	CString								sSACLControl;
	CSD*									csdSD				=	NULL;

	// Convert the object type to a string for storage
	sObjectType.Format (TEXT ("%d"), m_nObjectType);

	// Were the options set?
	if (! m_nListWhat)
	{
		return RTN_ERR_LIST_OPTIONS;
	}

	// Check if the current path is on the filter list. If yes -> do nothing
	if (CheckFilterList (sObjectPath))
	{
		// Notify caller of the omission
		LogMessage (TEXT ("INFO: Omitting ACL of: <" + sObjectPath + "> because a filter keyword matched."));

		return RTN_OK;
	}

	// Check which elements of the SD we need to process
	if (m_nListWhat & ACL_DACL)
	{
		siSecInfo	|=	DACL_SECURITY_INFORMATION;
	}
	if (m_nListWhat & ACL_SACL)
	{
		siSecInfo	|=	SACL_SECURITY_INFORMATION;
	}
	if (m_nListWhat & SD_OWNER)
	{
		siSecInfo	|=	OWNER_SECURITY_INFORMATION;
	}
	if (m_nListWhat & SD_GROUP)
	{
		siSecInfo	|=	GROUP_SECURITY_INFORMATION;
	}


	//
	// Get the SD and the SD's control information
	//
	csdSD				=	new CSD (this);

	nError			=	csdSD->GetSD (sObjectPath, m_nObjectType, siSecInfo);
	m_nAPIError		=	csdSD->m_nAPIError;

	if (nError != RTN_OK)
	{
		if (m_fIgnoreErrors)
		{
			LogMessage (TEXT ("ERROR (ignored): Reading the SD from <" + sObjectPath + "> failed with: " + GetLastErrorMessage (m_nAPIError)));
			nError		=	RTN_ERR_IGNORED;
		}
		else
		{
			LogMessage (TEXT ("ERROR: Reading the SD from <" + sObjectPath + "> failed with: " + GetLastErrorMessage (m_nAPIError)));
		}

		goto CleanUp;
	}
	else if (csdSD->m_psdSD == NULL)
	{
		LogMessage (TEXT ("INFO: The object <" + m_sObjectPath + "> has a NULL security descriptor (granting full control to everyone) and is being ignored"));
		nError	=	RTN_ERR_IGNORED;

		goto CleanUp;
	}

	// Get the SD's control information, which contains the protection flags
	DWORD									nSDRevision	=	0;
	SECURITY_DESCRIPTOR_CONTROL	sdControl;

	if (! GetSecurityDescriptorControl (csdSD->m_psdSD, &sdControl, &nSDRevision))
	{
		nError			=	RTN_ERR_GET_SD_CONTROL;
		m_nAPIError		=	GetLastError ();

		goto CleanUp;
	}

	// Build the SD control strings: DACL ...
	if (sdControl & SE_DACL_PROTECTED)
	{
		sDACLControl	=	TEXT ("(protected");
	}
	else
	{
		sDACLControl	=	TEXT ("(not_protected");
	}
	if (sdControl & SE_DACL_AUTO_INHERITED)
	{
		sDACLControl	+=	TEXT ("+auto_inherited)");
	}
	else
	{
		sDACLControl	+=	TEXT (")");
	}

	// ... and SACL
	if (sdControl & SE_SACL_PROTECTED)
	{
		sSACLControl	=	TEXT ("(protected");
	}
	else
	{
		sSACLControl	=	TEXT ("(not_protected");
	}
	if (sdControl & SE_SACL_AUTO_INHERITED)
	{
		sSACLControl	+=	TEXT ("+auto_inherited)");
	}
	else
	{
		sSACLControl	+=	TEXT (")");
	}

	// Which list format is requested: SDDL or our own?
	if (m_nListFormat == LIST_SDDL)
	{
		// Convert the SD into the SDDL format
		if (! ConvertSecurityDescriptorToStringSecurityDescriptor (csdSD->m_psdSD, SDDL_REVISION_1, siSecInfo, &sSDDL, NULL))
		{
			nError		=	RTN_ERR_CONVERT_SD;
			m_nAPIError	=	GetLastError ();

			goto CleanUp;
		}

		//
		// Format and log the result
		//
		if (sObjectPath.Left (1) != TEXT ("\""))
		{
			sLineOut		=	TEXT ("\"") + sObjectPath + TEXT ("\",") + sObjectType + TEXT (",\"") + sSDDL + TEXT ("\"");
		}
		else
		{
			sLineOut		=	sObjectPath + TEXT (",") + sObjectType + TEXT (",\"") + sSDDL + TEXT ("\"");
		}
	}
	else if (m_nListFormat == LIST_CSV)
	{
		//
		// A listing in csv format is desired
		//
		if (sObjectPath.Left (1) != TEXT ("\""))
		{
			sLineOut		=	TEXT ("\"") + sObjectPath + TEXT ("\",") + sObjectType + TEXT (",");
		}
		else
		{
			sLineOut		=	sObjectPath + TEXT (",") + sObjectType + TEXT (",");
		}

		// Process DACL if necessary
		if (m_nListWhat & ACL_DACL && csdSD->m_paclDACL)
		{
			CString sDACL			=	ListACL (csdSD->m_paclDACL);

			if (! sDACL.IsEmpty ())
			{
				sSD			=	TEXT ("DACL") + sDACLControl + TEXT (":") + sDACL;
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Process SACL if necessary
		if (m_nListWhat & ACL_SACL && csdSD->m_paclSACL)
		{
			CString sSACL			=	ListACL (csdSD->m_paclSACL);

			if (! sSD.IsEmpty () && ! sSACL.IsEmpty ())
			{
				sSD			+=	TEXT (";");
			}

			if (! sSACL.IsEmpty ())
			{
				sSD			+=	TEXT ("SACL") + sSACLControl + TEXT (":") + sSACL;
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Process owner if necessary
		if (m_nListWhat & SD_OWNER && csdSD->m_psidOwner)
		{
			CString sOwner	=	GetTrusteeFromSID (csdSD->m_psidOwner);

			if (! sSD.IsEmpty () && ! sOwner.IsEmpty ())
			{
				sSD			+=	TEXT (";");
			}

			if (! sOwner.IsEmpty ())
			{
				sSD			+=	TEXT ("Owner:") + sOwner;
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Process primary group if necessary
		if (m_nListWhat & SD_GROUP && csdSD->m_psidGroup)
		{
			CString sGroup	=	GetTrusteeFromSID (csdSD->m_psidGroup);

			if (! sSD.IsEmpty () && ! sGroup.IsEmpty ())
			{
				sSD			+=	TEXT (";");
			}

			if (! sGroup.IsEmpty ())
			{
				sSD			+=	TEXT ("Group:") + sGroup;
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Format the result
		if (! sSD.IsEmpty ())
		{
			sLineOut		+=	TEXT ("\"") + sSD + TEXT ("\"");
		}
		else
		{
			sLineOut		=	TEXT ("");
		}
	}
	else if (m_nListFormat == LIST_TAB)
	{
		//
		// A listing in tabular format is desired
		//
		sLineOut		=	sObjectPath + TEXT ("\n");
		sSD			=	TEXT ("");

		// Process owner if necessary
		if (m_nListWhat & SD_OWNER && csdSD->m_psidOwner)
		{
			CString sOwner	=	GetTrusteeFromSID (csdSD->m_psidOwner);

			if (! sOwner.IsEmpty ())
			{
				sSD			+=	TEXT ("\n   Owner: ") + sOwner + TEXT ("\n");
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Process primary group if necessary
		if (m_nListWhat & SD_GROUP && csdSD->m_psidGroup)
		{
			CString sGroup	=	GetTrusteeFromSID (csdSD->m_psidGroup);

			if (! sGroup.IsEmpty ())
			{
				sSD			+=	TEXT ("\n   Group: ") + sGroup + TEXT ("\n");
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Process DACL if necessary
		if (m_nListWhat & ACL_DACL && csdSD->m_paclDACL)
		{
			CString sDACL	=	ListACL (csdSD->m_paclDACL);

			sDACL.Replace (TEXT (":"), TEXT ("\n   "));
			sDACL.Replace (TEXT (","), TEXT ("   "));

			if (! sDACL.IsEmpty ())
			{
				sSD			+=	TEXT ("\n   DACL") + sDACLControl + (":\n   ") + sDACL + TEXT ("\n");
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Process SACL if necessary
		if (m_nListWhat & ACL_SACL && csdSD->m_paclSACL)
		{
			CString sSACL	=	ListACL (csdSD->m_paclSACL);

			sSACL.Replace (TEXT (":"), TEXT ("\n   "));
			sSACL.Replace (TEXT (","), TEXT ("   "));

			if (! sSACL.IsEmpty ())
			{
				sSD			+=	TEXT ("\n   SACL") + sSACLControl + (":\n   ") + sSACL + TEXT ("\n");
			}

			if (m_nAPIError)
			{
				nError		=	RTN_ERR_LIST_ACL;

				goto CleanUp;
			}
		}

		// Format the result
		if (! sSD.IsEmpty ())
		{
			sLineOut		+=	sSD;
		}
		else
		{
			sLineOut		=	TEXT ("");
		}
	}

	// Save the result
	if (! sLineOut.IsEmpty ())
	{
		if (m_fhBackupRestoreFile)
		{
			if (_ftprintf (m_fhBackupRestoreFile, TEXT ("%s\r\n"), sLineOut.GetString ()) < 0)
			{
				return RTN_ERR_WRITE_LOGFILE;
			}
		}

		// Print the result
		LogMessage (sLineOut);
	}

CleanUp:

	// If errors are to be ignored (param "-ignoreerr" set), then do NOT return an error
	if (nError == RTN_ERR_IGNORED)
	{
		m_nAPIError	=	ERROR_SUCCESS;
		nError		=	RTN_OK;
	}

	if (sSDDL)			LocalFree (sSDDL);
	if (csdSD)			delete csdSD;

	return nError;
}


//
// GetTrusteeFromSID: Convert a binary SID into a trustee name
//
CString CSetACL::GetTrusteeFromSID (PSID psidSID)
{
	DWORD				nError			=	RTN_OK;
	DWORD				nAccountName	=	1024;
	DWORD				nDomainName		=	1024;
	LPTSTR			pcSID				=	NULL;
	CString			sSID;
	CString			sDomainName;
	CString			sAccountName;
	CString			sTrustee;
	CString			sTrusteeName;
	SID_NAME_USE	snuSidType;

	if (m_nListNameSID & LIST_NAME)
	{
		// Try to look up the account name
		if (! LookupAccountSid (m_sTargetSystemName.IsEmpty () ? NULL : m_sTargetSystemName.GetBuffer (m_sTargetSystemName.GetLength ()), psidSID, sAccountName.GetBuffer (1024), &nAccountName, sDomainName.GetBuffer (1024), &nDomainName, &snuSidType))
		{
			nError			=	GetLastError ();

			m_sTargetSystemName.ReleaseBuffer ();
			sAccountName.ReleaseBuffer ();
			sDomainName.ReleaseBuffer ();

			if (nError != ERROR_NONE_MAPPED)
			{
				m_nAPIError		=	nError;
				return sTrustee;
			}

			// The account name was not found: display the SID instead
			if (! ConvertSidToStringSid (psidSID, &pcSID))
			{
				m_nAPIError		=	GetLastError ();
				return sTrustee;
			}

			sSID					=	pcSID;
		}

		m_sTargetSystemName.ReleaseBuffer ();
		sAccountName.ReleaseBuffer ();
		sDomainName.ReleaseBuffer ();

		// Build the trustee name string
		if (! sDomainName.IsEmpty ())
		{
			sTrusteeName		=	sDomainName + TEXT ("\\") + sAccountName;
		}
		else
		{
			sTrusteeName		=	sAccountName;
		}
	}

	// Get the string SID if requested
	if (m_nListNameSID & LIST_SID && sSID.IsEmpty ())
	{
		if (! ConvertSidToStringSid (psidSID, &pcSID))
		{
			m_nAPIError	=	GetLastError ();
			return sTrustee;
		}

		sSID						=	pcSID;
	}

	// Build the trustee string
	if (sTrusteeName.IsEmpty ())
	{
		// We could not or were not asked to find the name -> display the SID only
		sTrustee				=	sSID;
	}
	else if (m_nListNameSID == LIST_NAME_SID)
	{
		// Display the name and the SID
		sTrustee				=	sTrusteeName + TEXT ("=") + sSID;
	}
	else if (m_nListNameSID & LIST_NAME)
	{
		// Display the name only
		sTrustee				=	sTrusteeName;
	}

	if (pcSID)
	{
		LocalFree (pcSID);
	}

	return sTrustee;
}


//
// ListACL: Return the contents of an ACL as a string
//
CString CSetACL::ListACL (PACL paclACL)
{
	ACL_SIZE_INFORMATION				asiACLSize;
	ACCESS_ALLOWED_ACE*				paceACE			=	NULL;
	CString								sOut;

	// If this is a NULL ACL, do nothing
	if (! paclACL)
	{
		return sOut;
	}

	// Get the number of entries in the ACL
	if (! GetAclInformation (paclACL, &asiACLSize, sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		m_nAPIError	=	GetLastError ();
		return sOut;
	}

	// Loop through the ACEs
	for (WORD i = 0; i < asiACLSize.AceCount; i++)
	{
		CString			sTrustee;
		CString			sPermissions;
		CString			sFlags;
		CString			sACEType;

		// Get the current ACE
		if (! GetAce (paclACL, i, (LPVOID*) &paceACE))
		{
			m_nAPIError	=	GetLastError ();
			return sOut;
		}

		// Omit inherited ACEs?
		if (paceACE->Header.AceFlags & INHERITED_ACE && m_fListInherited == false)
		{
			continue;
		}

		// Find the name corresponding to the SID in the ACE
		sTrustee				=	GetTrusteeFromSID ((PSID) &(paceACE->SidStart));

		if (m_nAPIError)
		{
			return sOut;
		}

		// Get the permissions
		sPermissions		=	GetPermissions (paceACE->Mask);

		// Get the ACE type
		sACEType				=	GetACEType (paceACE->Header.AceType);

		// Get the ACE flags
		sFlags				=	GetACEFlags (paceACE->Header.AceFlags);

		sOut					+=	sTrustee + TEXT (",") + sPermissions + TEXT (",") + sACEType + TEXT (",") + sFlags + TEXT (":");
	}

	sOut.TrimRight (TEXT (":"));

	return sOut;
}


//
// ProcessACEsOfGivenTrustees: Process (delete, replace, copy) all ACEs belonging to trustees specified
//
DWORD CSD::ProcessACEsOfGivenTrustees (DWORD nWhere)
{
	DWORD									nError			=	RTN_OK;
	ACL_SIZE_INFORMATION				asiACLSize;
	ACCESS_ALLOWED_ACE*				paceACE			=	NULL;
	DWORD									nACECount		=	0;
	PACL									paclACL			=	NULL;
	BOOL									fIsSACL			=	false;

	if (nWhere == ACL_DACL)
	{
		paclACL	=	m_paclDACL;
		fIsSACL	=	false;
	}
	else if (nWhere == ACL_SACL)
	{
		paclACL	=	m_paclSACL;
		fIsSACL	=	true;
	}
	else
	{
		return RTN_ERR_PARAMS;
	}

	// If this is a NULL ACL, do nothing
	if (! paclACL)
	{
		return nError;
	}

	// Get the number of entries in the ACL
	if (! GetAclInformation (paclACL, &asiACLSize, sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		m_nAPIError	=	GetLastError ();
		return RTN_ERR_LOOP_ACL;
	}

	nACECount	=	asiACLSize.AceCount;

	// Loop through the ACEs
	for (DWORD i = 0; i < nACECount; i++)
	{
		// Get the current ACE
		if (! GetAce (paclACL, i, (LPVOID*) &paceACE))
		{
			m_nAPIError	=	GetLastError ();
			return RTN_ERR_LOOP_ACL;
		}

		// Do NOT change inherited ACEs
		if (paceACE->Header.AceFlags & INHERITED_ACE)
		{
			continue;
		}

		// Iterate through the Trustees specified
		POSITION	pos	=	m_setaclMain->m_lstTrustees.GetHeadPosition ();

		while (pos)
		{
			CTrustee*	pTrustee	=	m_setaclMain->m_lstTrustees.GetNext (pos);

			// Process this ACE only if the specified options match
			if (fIsSACL && !pTrustee->m_fSACL)
			{
				continue;
			}
			if (!fIsSACL && !pTrustee->m_fDACL)
			{
				continue;
			}

			//
			// Process the ACE if it belongs to a SID on the list.
			//
			if (EqualSid ((PSID) &(paceACE->SidStart), pTrustee->m_psidTrustee))
			{
				PACL	paclACLNew	=	NULL;

				if (pTrustee->m_nAction & ACTN_REMOVETRUSTEE)
				{
					if (! DeleteAce (paclACL, i))
					{
						m_nAPIError	=	GetLastError ();
						return RTN_ERR_DEL_ACE;
					}

					// The ACECount is now reduced by one!
					nACECount--;
					i--;
				}
				else if (pTrustee->m_nAction & ACTN_REPLACETRUSTEE)
				{
					// Replace the ACE into a new ACL
					paclACLNew		=	ACLReplaceACE (paclACL, i, pTrustee->m_oNewTrustee->m_psidTrustee);

					if (! paclACLNew)
					{
						m_nAPIError	=	GetLastError ();
						return RTN_ERR_COPY_ACL;
					}
				}
				else if (pTrustee->m_nAction & ACTN_COPYTRUSTEE)
				{
					// Copy the ACE into a new ACL
					paclACLNew		=	ACLCopyACE (paclACL, i, pTrustee->m_oNewTrustee->m_psidTrustee);

					if (! paclACLNew)
					{
						m_nAPIError	=	GetLastError ();
						return RTN_ERR_COPY_ACL;
					}

					// The ACE count has increased
					nACECount++;
				}

				if (paclACLNew)
				{
					// Free the old ACL
					if (nWhere == ACL_DACL)
					{
						DeleteBufDACL ();
					}
					else if (nWhere == ACL_SACL)
					{
						DeleteBufSACL ();
					}

					// Continue with the new ACL
					paclACL	=	paclACLNew;

					// Save the new ACL
					if (nWhere == ACL_DACL)
					{
						m_paclDACL			=	paclACL;
						m_fBufDACLAlloc	=	true;
					}
					else if (nWhere == ACL_SACL)
					{
						m_paclSACL			=	paclACL;
						m_fBufSACLAlloc	=	true;
					}
				}

				// Break out of the while loop and continue with next ACE
				break;
			}	// if (EqualSid)
		}		// while (pos)
	}			//	for (int i = 0; i < nACECount; i++)

	return nError;
}


//
// ProcessACEsOfGivenDomains: Process (delete, replace, copy) all ACEs belonging to domains specified
//
DWORD CSD::ProcessACEsOfGivenDomains (DWORD nWhere)
{
	DWORD									nError			=	RTN_OK;
	ACL_SIZE_INFORMATION				asiACLSize;
	ACCESS_ALLOWED_ACE*				paceACE			=	NULL;
	DWORD									nACECount		=	0;
	PACL									paclACL			=	NULL;
	BOOL									fIsSACL			=	false;

	if (nWhere == ACL_DACL)
	{
		paclACL	=	m_paclDACL;
		fIsSACL	=	false;
	}
	else if (nWhere == ACL_SACL)
	{
		paclACL	=	m_paclSACL;
		fIsSACL	=	true;
	}
	else
	{
		return RTN_ERR_PARAMS;
	}

	// If this is a NULL ACL, do nothing
	if (! paclACL)
	{
		return nError;
	}

	// Get the number of entries in the ACL
	if (! GetAclInformation (paclACL, &asiACLSize, sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		m_nAPIError	=	GetLastError ();
		return RTN_ERR_LOOP_ACL;
	}

	nACECount	=	asiACLSize.AceCount;

	// Loop through the ACEs
	for (DWORD i = 0; i < nACECount; i++)
	{
		// Get the current ACE
		if (! GetAce (paclACL, i, (LPVOID*) &paceACE))
		{
			m_nAPIError	=	GetLastError ();
			return RTN_ERR_LOOP_ACL;
		}

		// Check for validity of SID
		if (! IsValidSid ((PSID) &(paceACE->SidStart)))
		{
			return RTN_ERR_LOOP_ACL;
		}

		// Do NOT change inherited ACEs
		if (paceACE->Header.AceFlags & INHERITED_ACE)
		{
			continue;
		}

		//
		// Get domain and trustee of the SID in the ACE
		//
		DWORD				nAccountName	=	1024;
		DWORD				nDomainName		=	1024;
		CString			sDomainName;
		CString			sAccountName;
		SID_NAME_USE	snuSidType;
		LPCTSTR			lpSystemName	=	m_setaclMain->m_sTargetSystemName.IsEmpty () ? NULL : m_setaclMain->m_sTargetSystemName.GetBuffer (m_setaclMain->m_sTargetSystemName.GetLength ());

		// Try to look up account and domain name
		if (! LookupAccountSid (lpSystemName, (PSID) &(paceACE->SidStart), sAccountName.GetBuffer (1024), &nAccountName, sDomainName.GetBuffer (1024), &nDomainName, &snuSidType))
		{
			m_setaclMain->m_sTargetSystemName.ReleaseBuffer ();
			sAccountName.ReleaseBuffer ();
			sDomainName.ReleaseBuffer ();

			// Ignore SIDs that cannot be looked up
			continue;
		}

		m_setaclMain->m_sTargetSystemName.ReleaseBuffer ();
		sAccountName.ReleaseBuffer ();
		sDomainName.ReleaseBuffer ();

		if (snuSidType == SidTypeDeletedAccount || snuSidType == SidTypeInvalid || snuSidType == SidTypeUnknown)
		{
			// Ignore invalid or deleted SIDs
			continue;
		}

		if (sAccountName.IsEmpty () || sDomainName.IsEmpty ())
		{
			// Ignore dubious errors
			continue;
		}

		//
		// Iterate through the domains specified
		//
		POSITION	pos	=	m_setaclMain->m_lstDomains.GetHeadPosition ();

		while (pos)
		{
			CDomain*	pDomain	=	m_setaclMain->m_lstDomains.GetNext (pos);

			// Process this ACE only if the specified options match
			if (fIsSACL && !pDomain->m_fSACL)
			{
				continue;
			}
			if (!fIsSACL && !pDomain->m_fDACL)
			{
				continue;
			}

			//
			// Process the ACE if it belongs to a domain on the list.
			//
			if (sDomainName.CompareNoCase (pDomain->m_sDomain) == 0)
			{
				PACL	paclACLNew	=	NULL;

				if (pDomain->m_nAction & ACTN_REMOVETRUSTEE)
				{
					if (! DeleteAce (paclACL, i))
					{
						m_nAPIError	=	GetLastError ();
						return RTN_ERR_DEL_ACE;
					}

					// The ACECount is now reduced by one!
					nACECount--;
					i--;
				}
				else
				{
					// Make sure a new domain is set
					if (! pDomain->m_oNewDomain || pDomain->m_oNewDomain->m_sDomain.IsEmpty	())
					{
						return RTN_ERR_PARAMS;
					}

					// Build a Trustee object with the new domain, old trustee name, and look up the SID
					CTrustee*	pTrusteeNew	=	new CTrustee  (pDomain->m_oNewDomain->m_sDomain + TEXT ("\\") + sAccountName, false, 0, false, false);

					if (pTrusteeNew->LookupSID () != RTN_OK)
					{
						m_setaclMain->LogMessage (TEXT ("   Account <" + sAccountName + "> was not found in domain <" + pDomain->m_oNewDomain->m_sDomain + ">."));

						continue;
					}

					if (pDomain->m_nAction & ACTN_REPLACETRUSTEE)
					{
						// Replace the ACE into a new ACL
						paclACLNew		=	ACLReplaceACE (paclACL, i, pTrusteeNew->m_psidTrustee);

						if (! paclACLNew)
						{
							m_nAPIError	=	GetLastError ();
							delete pTrusteeNew;
							return RTN_ERR_COPY_ACL;
						}
					}
					else if (pDomain->m_nAction & ACTN_COPYTRUSTEE)
					{
						// Copy the ACE into a new ACL
						paclACLNew		=	ACLCopyACE (paclACL, i, pTrusteeNew->m_psidTrustee);

						if (! paclACLNew)
						{
							m_nAPIError	=	GetLastError ();
							delete pTrusteeNew;
							return RTN_ERR_COPY_ACL;
						}

						// The ACE count has increased
						nACECount++;
					}

					delete pTrusteeNew;
				}

				if (paclACLNew)
				{
					// Free the old ACL
					if (nWhere == ACL_DACL)
					{
						DeleteBufDACL ();
					}
					else if (nWhere == ACL_SACL)
					{
						DeleteBufSACL ();
					}

					// Continue with the new ACL
					paclACL	=	paclACLNew;

					// Save the new ACL
					if (nWhere == ACL_DACL)
					{
						m_paclDACL			=	paclACL;
						m_fBufDACLAlloc	=	true;
					}
					else if (nWhere == ACL_SACL)
					{
						m_paclSACL			=	paclACL;
						m_fBufSACLAlloc	=	true;
					}
				}

				// Break out of the while loop and continue with next ACE
				break;
			}	// if (EqualSid)
		}		// while (pos)
	}			//	for (int i = 0; i < nACECount; i++)

	return nError;
}


//
// ACLReplaceACE: Replace an ACE in an ACL
//
PACL CSD::ACLReplaceACE (PACL paclACL, DWORD nACE, PSID psidNewTrustee)
{
	ACL_SIZE_INFORMATION				asiACLSize;
	ACCESS_ALLOWED_ACE*				paceACE			=	NULL;
	ACCESS_ALLOWED_ACE*				paceACE2			=	NULL;

	// Do a few checks
	if (! paclACL)
	{
		return NULL;
	}
	if (! IsValidAcl (paclACL))
	{
		return NULL;
	}

	// Get the ACE
	if (! GetAce (paclACL, nACE, (LPVOID*) &paceACE))
	{
		m_nAPIError				=	GetLastError ();
		return NULL;
	}

	// Store the ACE flags for later use
	BYTE			nACEType		=	((PACE_HEADER) paceACE)->AceType;
	BYTE			nACEFlags	=	((PACE_HEADER) paceACE)->AceFlags;
	ACCESS_MASK	nACEMask		=	((ACCESS_ALLOWED_ACE*) paceACE)->Mask;

	// Now delete the ACE
	if (! DeleteAce (paclACL, nACE))
	{
		m_nAPIError				=	GetLastError ();
		return NULL;
	}

	//
	// The new ACE may (!) be of different length (due to the variable length SID) than the deleted one ->
	// We have to initialize a new ACL of correct size and copy all ACEs over; then we can add the new ACE.
	//
	// Get the current size of the ACL excluding the deleted ACE
	if (! GetAclInformation (paclACL, &asiACLSize, sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		m_nAPIError				=	GetLastError ();
		return NULL;
	}

	// Allocate memory for the new ACL
	DWORD	nACLNewSize			=	asiACLSize.AclBytesInUse + sizeof (ACCESS_ALLOWED_ACE) + GetLengthSid (psidNewTrustee) - sizeof (DWORD);
	PACL	paclACLNew			=	(PACL) LocalAlloc (LPTR, nACLNewSize);

	if (! paclACLNew)
	{
		m_nAPIError				=	GetLastError ();
		return NULL;
	}

	// Initialize the new ACL
	if(! InitializeAcl (paclACLNew, nACLNewSize, ACL_REVISION))
	{
		m_nAPIError				=	GetLastError ();
		LocalFree (paclACLNew);
		return NULL;
	}

	//
	// Copy all ACEs from the old to the new ACL; insert our new ACE at the correct position
	//
	// The new ACE might belong at the end of the ACL.
	BOOL	fNewACEInserted	=	false;
	WORD	j;

	for (j = 0; j < asiACLSize.AceCount; j++)
	{
		if (j == nACE)
		{
			// Insert the new ACE
			if (! AddAccessAllowedAce (paclACLNew, ACL_REVISION, nACEMask, psidNewTrustee))
			{
				m_nAPIError	=	GetLastError ();
				LocalFree (paclACLNew);
				return NULL;
			}

			// Get the ACE we just added in order to change the ACE flags
			if (! GetAce (paclACLNew, j, (LPVOID*) &paceACE2))
			{
				m_nAPIError		=	GetLastError ();
				LocalFree (paclACLNew);
				return NULL;
			}

			// Set the original ACE flags on the new ACE
			((PACE_HEADER) paceACE2)->AceType	=	nACEType;
			((PACE_HEADER) paceACE2)->AceFlags	=	nACEFlags;

			// The new ACE does not belong at the end.
			fNewACEInserted	=	true;
		}

		// Get the current ACE from the old ACL
		if (! GetAce (paclACL, j, (LPVOID*) &paceACE2))
		{
			m_nAPIError			=	GetLastError ();
			LocalFree (paclACLNew);
			return NULL;
		}

		// Copy the current ACE from the old to the new ACL
		if(! AddAce (paclACLNew, ACL_REVISION, MAXDWORD, paceACE2, ((PACE_HEADER) paceACE2)->AceSize))
		{
			m_nAPIError			=	GetLastError ();
			LocalFree (paclACLNew);
			return NULL;
		}
	}

	// If the new ACE was not yet been inserted, it belongs at the end of the ACL. Append it now.
	if (! fNewACEInserted)
	{
		// Insert the new ACE
		if (! AddAccessAllowedAce (paclACLNew, ACL_REVISION, nACEMask, psidNewTrustee))
		{
			m_nAPIError			=	GetLastError ();
			LocalFree (paclACLNew);
			return NULL;
		}

		// Get the ACE we just added in order to change the ACE flags
		if (! GetAce (paclACLNew, j, (LPVOID*) &paceACE2))
		{
			m_nAPIError			=	GetLastError ();
			LocalFree (paclACLNew);
			return NULL;
		}

		// Set the original ACE flags on the new ACE
		((PACE_HEADER) paceACE2)->AceType	=	nACEType;
		((PACE_HEADER) paceACE2)->AceFlags	=	nACEFlags;
	}

	// Check for validity
	if (! IsValidAcl (paclACLNew))
	{
		return NULL;
	}

	// return the new ACL
	return paclACLNew;
}


//
// ACLCopyACE: Copy an ACE in an ACL
//
PACL CSD::ACLCopyACE (PACL paclACL, DWORD nACE, PSID psidNewTrustee)
{
	ACL_SIZE_INFORMATION				asiACLSize;
	ACCESS_ALLOWED_ACE*				paceACE			=	NULL;
	ACCESS_ALLOWED_ACE*				paceACE2			=	NULL;

	// Do a few checks
	if (! paclACL)
	{
		return NULL;
	}
	if (! IsValidAcl (paclACL))
	{
		return NULL;
	}

	//
	// We have to initialize a new ACL of correct size and copy all ACEs over; then we can add the new ACE.
	//
	// Get the current size of the ACL
	if (! GetAclInformation (paclACL, &asiACLSize, sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		m_nAPIError		=	GetLastError ();
		return NULL;
	}

	// Allocate memory for the new ACL
	DWORD	nACLNewSize	=	asiACLSize.AclBytesInUse + sizeof (ACCESS_ALLOWED_ACE) + GetLengthSid (psidNewTrustee) - sizeof (DWORD);
	PACL	paclACLNew	=	(PACL) LocalAlloc (LPTR, nACLNewSize);

	if (! paclACLNew)
	{
		m_nAPIError		=	GetLastError ();
		return NULL;
	}

	// Initialize the new ACL
	if(! InitializeAcl (paclACLNew, nACLNewSize, ACL_REVISION))
	{
		m_nAPIError	=	GetLastError ();
		LocalFree (paclACLNew);
		return NULL;
	}

	//
	// Copy all ACEs from the old to the new ACL; insert our new ACE at the correct position
	//
	for (WORD j = 0; j < asiACLSize.AceCount; j++)
	{
		// Get the current ACE from the old ACL
		if (! GetAce (paclACL, j, (LPVOID*) &paceACE))
		{
			m_nAPIError	=	GetLastError ();
			LocalFree (paclACLNew);
			return NULL;
		}

		// Copy the current ACE from the old to the new ACL
		if(! AddAce (paclACLNew, ACL_REVISION, MAXDWORD, paceACE, ((PACE_HEADER) paceACE)->AceSize))
		{
			m_nAPIError	=	GetLastError ();
			LocalFree (paclACLNew);
			return NULL;
		}

		if (j == nACE)
		{
			// Insert the new ACE
			if (! AddAccessAllowedAce (paclACLNew, ACL_REVISION, ((ACCESS_ALLOWED_ACE*) paceACE)->Mask, psidNewTrustee))
			{
				m_nAPIError	=	GetLastError ();
				LocalFree (paclACLNew);
				return NULL;
			}

			// Get the ACE we just added in order to change the ACE flags
			if (! GetAce (paclACLNew, j + 1, (LPVOID*) &paceACE2))
			{
				m_nAPIError	=	GetLastError ();
				LocalFree (paclACLNew);
				return NULL;
			}

			// Set the original ACE flags on the new ACE
			((PACE_HEADER) paceACE2)->AceType	=	((PACE_HEADER) paceACE)->AceType;
			((PACE_HEADER) paceACE2)->AceFlags	=	((PACE_HEADER) paceACE)->AceFlags;
		}
	}

	// Check for validity
	if (! IsValidAcl (paclACLNew))
	{
		return NULL;
	}

	// return the new ACL
	return paclACLNew;
}


//
// DeleteACEsByHeaderFlags: Delete all ACEs from an ACL that have certain header flags set
//
DWORD CSD::DeleteACEsByHeaderFlags (DWORD nWhere, BYTE nFlags, BOOL fFlagsSet)
{
	DWORD									nError			=	RTN_OK;
	ACL_SIZE_INFORMATION				asiACLSize;
	ACCESS_ALLOWED_ACE*				paceACE			=	NULL;
	DWORD									nACECount;
	PACL									paclACL			=	NULL;

	if (nWhere == ACL_DACL)
	{
		paclACL			=	m_paclDACL;
	}
	else if (nWhere == ACL_SACL)
	{
		paclACL			=	m_paclSACL;
	}
	else
	{
		return RTN_ERR_PARAMS;
	}

	// If this is a NULL ACL, do nothing
	if (! paclACL)
	{
		return nError;
	}

	// Get the number of entries in the ACL
	if (! GetAclInformation (paclACL, &asiACLSize, sizeof (ACL_SIZE_INFORMATION), AclSizeInformation))
	{
		m_nAPIError		=	GetLastError ();
		return RTN_ERR_LOOP_ACL;
	}

	nACECount			=	asiACLSize.AceCount;

	// Loop through the ACEs
	for (DWORD i = 0; i < nACECount; i++)
	{
		BOOL fDelete	=	false;

		// Get the current ACE
		if (! GetAce (paclACL, i, (LPVOID*) &paceACE))
		{
			m_nAPIError	=	GetLastError ();
			return RTN_ERR_LOOP_ACL;
		}

		// Determine whether to delete the ACE depending on the parameters passed
		if (fFlagsSet)
		{
			fDelete		=	paceACE->Header.AceFlags & nFlags;
		}
		else
		{
			fDelete		=	! (paceACE->Header.AceFlags & nFlags);
		}

		if (fDelete)
		{
			if (! DeleteAce (paclACL, i))
			{
				m_nAPIError	=	GetLastError ();
				return RTN_ERR_DEL_ACE;
			}

			// The ACECount is now reduced by one!
			nACECount--;
			i--;
		}
	}

	return nError;
}


//
// GetPermissions: Return a string with the permissions in an access mask
//
CString CSetACL::GetPermissions (ACCESS_MASK nAccessMask)
{
	CString	sPermissions;
	DWORD		aGenericMappingFile[4]		=	{0x120089, 0x120116, 0x1200A0, 0x1F01FF};
	DWORD		aGenericMappingRegistry[4]	=	{0x120019, 0x20006, 0x20019, 0xF003F};

	if (m_nObjectType	==	SE_FILE_OBJECT)
	{
		// Map generic rights to standard and specific rights
		MapGenericMask (&nAccessMask, (GENERIC_MAPPING*) aGenericMappingFile);

		// Mask out the SYNCHRONIZE flag which is not always set (ie. not on audit ACEs)
		nAccessMask			&=	~SYNCHRONIZE;

		if ((nAccessMask & MY_DIR_FULL_ACCESS) == MY_DIR_FULL_ACCESS)
		{
			sPermissions	=	TEXT ("full+");
			nAccessMask		&= ~MY_DIR_FULL_ACCESS;
		}
		if ((nAccessMask & MY_DIR_CHANGE_ACCESS) == MY_DIR_CHANGE_ACCESS)
		{
			sPermissions	=	TEXT ("change+");
			nAccessMask		&= ~MY_DIR_CHANGE_ACCESS;
		}
		if ((nAccessMask & MY_DIR_READ_EXECUTE_ACCESS) == MY_DIR_READ_EXECUTE_ACCESS)
		{
			sPermissions	=	TEXT ("read_execute+");
			nAccessMask		&= ~MY_DIR_READ_EXECUTE_ACCESS;
		}
		if ((nAccessMask & MY_DIR_WRITE_ACCESS) == MY_DIR_WRITE_ACCESS)
		{
			sPermissions	=	TEXT ("write+");
			nAccessMask		&= ~MY_DIR_WRITE_ACCESS;
		}
		if ((nAccessMask & MY_DIR_READ_ACCESS) == MY_DIR_READ_ACCESS)
		{
			sPermissions	=	TEXT ("read+");
			nAccessMask		&= ~MY_DIR_READ_ACCESS;
		}

		if (nAccessMask & FILE_LIST_DIRECTORY)
			  sPermissions += TEXT ("FILE_LIST_DIRECTORY+");
		if (nAccessMask & FILE_ADD_FILE)
			  sPermissions += TEXT ("FILE_ADD_FILE+");
		if (nAccessMask & FILE_ADD_SUBDIRECTORY)
			  sPermissions += TEXT ("FILE_ADD_SUBDIRECTORY+");
		if (nAccessMask & FILE_READ_EA)
			  sPermissions += TEXT ("FILE_READ_EA+");
		if (nAccessMask & FILE_WRITE_EA)
			  sPermissions += TEXT ("FILE_WRITE_EA+");
		if (nAccessMask & FILE_TRAVERSE)
			  sPermissions += TEXT ("FILE_TRAVERSE+");
		if (nAccessMask & FILE_DELETE_CHILD)
			  sPermissions += TEXT ("FILE_DELETE_CHILD+");
		if (nAccessMask & FILE_READ_ATTRIBUTES)
			  sPermissions += TEXT ("FILE_READ_ATTRIBUTES+");
		if (nAccessMask & FILE_WRITE_ATTRIBUTES)
			  sPermissions += TEXT ("FILE_WRITE_ATTRIBUTES+");
		if (nAccessMask & READ_CONTROL)
			  sPermissions += TEXT ("READ_CONTROL+");
		if (nAccessMask & WRITE_OWNER)
			  sPermissions += TEXT ("WRITE_OWNER+");
		if (nAccessMask & WRITE_DAC)
			  sPermissions += TEXT ("WRITE_DAC+");
		if (nAccessMask & DELETE)
			  sPermissions += TEXT ("DELETE+");
		if (nAccessMask & SYNCHRONIZE)
			  sPermissions += TEXT ("SYNCHRONIZE+");
		if (nAccessMask & ACCESS_SYSTEM_SECURITY)
			  sPermissions += TEXT ("ACCESS_SYSTEM_SECURITY+");
		if (nAccessMask & GENERIC_ALL)
			  sPermissions += TEXT ("GENERIC_ALL+");
		if (nAccessMask & GENERIC_EXECUTE)
			  sPermissions += TEXT ("GENERIC_EXECUTE+");
		if (nAccessMask & GENERIC_READ)
			  sPermissions += TEXT ("GENERIC_READ+");
		if (nAccessMask & GENERIC_WRITE)
			  sPermissions += TEXT ("GENERIC_WRITE+");

		sPermissions.TrimRight (TEXT ("+"));
	}
	else if (m_nObjectType	==	SE_REGISTRY_KEY)
	{
		// Map generic rights to standard and specific rights
		MapGenericMask (&nAccessMask, (GENERIC_MAPPING*) aGenericMappingRegistry);

		if ((nAccessMask & MY_REG_FULL_ACCESS) == MY_REG_FULL_ACCESS)
		{
			sPermissions	=	TEXT ("full+");
			nAccessMask		&= ~MY_REG_FULL_ACCESS;
		}
		if ((nAccessMask & MY_REG_READ_ACCESS) == MY_REG_READ_ACCESS)
		{
			sPermissions	=	TEXT ("read+");
			nAccessMask		&= ~MY_REG_READ_ACCESS;
		}

		if (nAccessMask & KEY_CREATE_LINK)
			  sPermissions += TEXT ("KEY_CREATE_LINK+");
		if (nAccessMask & KEY_CREATE_SUB_KEY)
			  sPermissions += TEXT ("KEY_CREATE_SUB_KEY+");
		if (nAccessMask & KEY_ENUMERATE_SUB_KEYS)
			  sPermissions += TEXT ("KEY_ENUMERATE_SUB_KEYS+");
		if (nAccessMask & KEY_EXECUTE)
			  sPermissions += TEXT ("KEY_EXECUTE+");
		if (nAccessMask & KEY_NOTIFY)
			  sPermissions += TEXT ("KEY_NOTIFY+");
		if (nAccessMask & KEY_QUERY_VALUE)
			  sPermissions += TEXT ("KEY_QUERY_VALUE+");
		if (nAccessMask & KEY_READ)
			  sPermissions += TEXT ("KEY_READ+");
		if (nAccessMask & KEY_SET_VALUE)
			  sPermissions += TEXT ("KEY_SET_VALUE+");
		if (nAccessMask & KEY_WRITE)
			  sPermissions += TEXT ("KEY_WRITE+");
		if (nAccessMask & READ_CONTROL)
			  sPermissions += TEXT ("READ_CONTROL+");
		if (nAccessMask & WRITE_OWNER)
			  sPermissions += TEXT ("WRITE_OWNER+");
		if (nAccessMask & WRITE_DAC)
			  sPermissions += TEXT ("WRITE_DAC+");
		if (nAccessMask & DELETE)
			  sPermissions += TEXT ("DELETE+");
		if (nAccessMask & SYNCHRONIZE)
			  sPermissions += TEXT ("SYNCHRONIZE+");
		if (nAccessMask & ACCESS_SYSTEM_SECURITY)
			  sPermissions += TEXT ("ACCESS_SYSTEM_SECURITY+");
		if (nAccessMask & GENERIC_ALL)
			  sPermissions += TEXT ("GENERIC_ALL+");
		if (nAccessMask & GENERIC_EXECUTE)
			  sPermissions += TEXT ("GENERIC_EXECUTE+");
		if (nAccessMask & GENERIC_READ)
			  sPermissions += TEXT ("GENERIC_READ+");
		if (nAccessMask & GENERIC_WRITE)
			  sPermissions += TEXT ("GENERIC_WRITE+");

		sPermissions.TrimRight (TEXT ("+"));
	}
	else if (m_nObjectType	==	SE_SERVICE)
	{
		if ((nAccessMask & MY_SVC_FULL_ACCESS) == MY_SVC_FULL_ACCESS)
		{
			sPermissions	=	TEXT ("full+");
			nAccessMask		&= ~MY_SVC_FULL_ACCESS;
		}
		if ((nAccessMask & MY_SVC_STARTSTOP_ACCESS) == MY_SVC_STARTSTOP_ACCESS)
		{
			sPermissions	=	TEXT ("start_stop+");
			nAccessMask		&= ~MY_SVC_STARTSTOP_ACCESS;
		}
		if ((nAccessMask & MY_SVC_READ_ACCESS) == MY_SVC_READ_ACCESS)
		{
			sPermissions	=	TEXT ("read+");
			nAccessMask		&= ~MY_SVC_READ_ACCESS;
		}

		if (nAccessMask & SERVICE_CHANGE_CONFIG)
			  sPermissions += TEXT ("SERVICE_CHANGE_CONFIG+");
		if (nAccessMask & SERVICE_ENUMERATE_DEPENDENTS)
			  sPermissions += TEXT ("SERVICE_ENUMERATE_DEPENDENTS+");
		if (nAccessMask & SERVICE_INTERROGATE)
			  sPermissions += TEXT ("SERVICE_INTERROGATE+");
		if (nAccessMask & SERVICE_PAUSE_CONTINUE)
			  sPermissions += TEXT ("SERVICE_PAUSE_CONTINUE+");
		if (nAccessMask & SERVICE_QUERY_CONFIG)
			  sPermissions += TEXT ("SERVICE_QUERY_CONFIG+");
		if (nAccessMask & SERVICE_QUERY_STATUS)
			  sPermissions += TEXT ("SERVICE_QUERY_STATUS+");
		if (nAccessMask & SERVICE_START)
			  sPermissions += TEXT ("SERVICE_START+");
		if (nAccessMask & SERVICE_STOP)
			  sPermissions += TEXT ("SERVICE_STOP+");
		if (nAccessMask & SERVICE_USER_DEFINED_CONTROL)
			  sPermissions += TEXT ("SERVICE_USER_DEFINED_CONTROL+");
		if (nAccessMask & READ_CONTROL)
			  sPermissions += TEXT ("READ_CONTROL+");
		if (nAccessMask & WRITE_OWNER)
			  sPermissions += TEXT ("WRITE_OWNER+");
		if (nAccessMask & WRITE_DAC)
			  sPermissions += TEXT ("WRITE_DAC+");
		if (nAccessMask & DELETE)
			  sPermissions += TEXT ("DELETE+");
		if (nAccessMask & SYNCHRONIZE)
			  sPermissions += TEXT ("SYNCHRONIZE+");
		if (nAccessMask & ACCESS_SYSTEM_SECURITY)
			  sPermissions += TEXT ("ACCESS_SYSTEM_SECURITY+");
		if (nAccessMask & GENERIC_ALL)
			  sPermissions += TEXT ("GENERIC_ALL+");
		if (nAccessMask & GENERIC_EXECUTE)
			  sPermissions += TEXT ("GENERIC_EXECUTE+");
		if (nAccessMask & GENERIC_READ)
			  sPermissions += TEXT ("GENERIC_READ+");
		if (nAccessMask & GENERIC_WRITE)
			  sPermissions += TEXT ("GENERIC_WRITE+");

		sPermissions.TrimRight (TEXT ("+"));
	}
	else if (m_nObjectType	==	SE_PRINTER)
	{
		if ((nAccessMask & MY_PRINTER_MAN_PRINTER_ACCESS) == MY_PRINTER_MAN_PRINTER_ACCESS)
		{
			sPermissions	=	TEXT ("manage_printer+");
			nAccessMask		&= ~MY_PRINTER_MAN_PRINTER_ACCESS;
		}
		if ((nAccessMask & MY_PRINTER_MAN_DOCS_ACCESS) == MY_PRINTER_MAN_DOCS_ACCESS)
		{
			sPermissions	=	TEXT ("manage_documents+");
			nAccessMask		&= ~MY_PRINTER_MAN_DOCS_ACCESS;
		}
		if ((nAccessMask & MY_PRINTER_PRINT_ACCESS) == MY_PRINTER_PRINT_ACCESS)
		{
			sPermissions	=	TEXT ("print+");
			nAccessMask		&= ~MY_PRINTER_PRINT_ACCESS;
		}

		if (nAccessMask & PRINTER_ACCESS_ADMINISTER)
			  sPermissions += TEXT ("PRINTER_ACCESS_ADMINISTER+");
		if (nAccessMask & PRINTER_ACCESS_USE)
			  sPermissions += TEXT ("PRINTER_ACCESS_USE+");
		if (nAccessMask & JOB_ACCESS_ADMINISTER)
			  sPermissions += TEXT ("JOB_ACCESS_ADMINISTER+");
		if (nAccessMask & JOB_ACCESS_READ)
			  sPermissions += TEXT ("JOB_ACCESS_READ+");
		if (nAccessMask & READ_CONTROL)
			  sPermissions += TEXT ("READ_CONTROL+");
		if (nAccessMask & WRITE_OWNER)
			  sPermissions += TEXT ("WRITE_OWNER+");
		if (nAccessMask & WRITE_DAC)
			  sPermissions += TEXT ("WRITE_DAC+");
		if (nAccessMask & DELETE)
			  sPermissions += TEXT ("DELETE+");
		if (nAccessMask & SYNCHRONIZE)
			  sPermissions += TEXT ("SYNCHRONIZE+");
		if (nAccessMask & ACCESS_SYSTEM_SECURITY)
			  sPermissions += TEXT ("ACCESS_SYSTEM_SECURITY+");
		if (nAccessMask & GENERIC_ALL)
			  sPermissions += TEXT ("GENERIC_ALL+");
		if (nAccessMask & GENERIC_EXECUTE)
			  sPermissions += TEXT ("GENERIC_EXECUTE+");
		if (nAccessMask & GENERIC_READ)
			  sPermissions += TEXT ("GENERIC_READ+");
		if (nAccessMask & GENERIC_WRITE)
			  sPermissions += TEXT ("GENERIC_WRITE+");

		sPermissions.TrimRight (TEXT ("+"));
	}
	else if (m_nObjectType	==	SE_LMSHARE)
	{
		if ((nAccessMask & MY_SHARE_FULL_ACCESS) == MY_SHARE_FULL_ACCESS)
		{
			sPermissions	=	TEXT ("full+");
			nAccessMask		&= ~MY_SHARE_FULL_ACCESS;
		}
		if ((nAccessMask & MY_SHARE_CHANGE_ACCESS) == MY_SHARE_CHANGE_ACCESS)
		{
			sPermissions	=	TEXT ("change+");
			nAccessMask		&= ~MY_SHARE_CHANGE_ACCESS;
		}
		if ((nAccessMask & MY_SHARE_READ_ACCESS) == MY_SHARE_READ_ACCESS)
		{
			sPermissions	=	TEXT ("read+");
			nAccessMask		&= ~MY_SHARE_READ_ACCESS;
		}

		if (nAccessMask & SHARE_READ)
			  sPermissions += TEXT ("SHARE_READ+");
		if (nAccessMask & SHARE_CHANGE)
			  sPermissions += TEXT ("SHARE_CHANGE+");
		if (nAccessMask & SHARE_WRITE)
			  sPermissions += TEXT ("SHARE_WRITE+");
		if (nAccessMask & READ_CONTROL)
			  sPermissions += TEXT ("READ_CONTROL+");
		if (nAccessMask & WRITE_OWNER)
			  sPermissions += TEXT ("WRITE_OWNER+");
		if (nAccessMask & WRITE_DAC)
			  sPermissions += TEXT ("WRITE_DAC+");
		if (nAccessMask & DELETE)
			  sPermissions += TEXT ("DELETE+");
		if (nAccessMask & SYNCHRONIZE)
			  sPermissions += TEXT ("SYNCHRONIZE+");
		if (nAccessMask & ACCESS_SYSTEM_SECURITY)
			  sPermissions += TEXT ("ACCESS_SYSTEM_SECURITY+");
		if (nAccessMask & GENERIC_ALL)
			  sPermissions += TEXT ("GENERIC_ALL+");
		if (nAccessMask & GENERIC_EXECUTE)
			  sPermissions += TEXT ("GENERIC_EXECUTE+");
		if (nAccessMask & GENERIC_READ)
			  sPermissions += TEXT ("GENERIC_READ+");
		if (nAccessMask & GENERIC_WRITE)
			  sPermissions += TEXT ("GENERIC_WRITE+");

		sPermissions.TrimRight (TEXT ("+"));
	}

	return sPermissions;
}


//
// GetACEType: Return a string with the type of an ACE
//
CString CSetACL::GetACEType (BYTE nACEType)
{
	CString	sACEType;

	switch(nACEType)
	{
	case ACCESS_ALLOWED_ACE_TYPE:
		sACEType	=	TEXT ("allow");
		break;
	case ACCESS_ALLOWED_CALLBACK_ACE_TYPE:
		sACEType	=	TEXT ("allow_callback");
		break;
	case ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("allow_callback_object");
		break;
	case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
		sACEType	=	TEXT ("allow_compound");
		break;
	case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("allow_object");
		break;
	case ACCESS_DENIED_ACE_TYPE:
		sACEType	=	TEXT ("deny");
		break;
	case ACCESS_DENIED_CALLBACK_ACE_TYPE:
		sACEType	=	TEXT ("deny_callback");
		break;
	case ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("deny_callback_object");
		break;
	case ACCESS_DENIED_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("deny_object");
		break;
	case SYSTEM_ALARM_ACE_TYPE:
		sACEType	=	TEXT ("alarm");
		break;
	case SYSTEM_ALARM_CALLBACK_ACE_TYPE:
		sACEType	=	TEXT ("alarm_callback");
		break;
	case SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("alarm_callback_object");
		break;
	case SYSTEM_ALARM_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("alarm_object");
		break;
	case SYSTEM_AUDIT_ACE_TYPE:
		sACEType	=	TEXT ("audit");
		break;
	case SYSTEM_AUDIT_CALLBACK_ACE_TYPE:
		sACEType	=	TEXT ("audit_callback");
		break;
	case SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("audit_callback_object");
		break;
	case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
		sACEType	=	TEXT ("audit_object");
		break;
	case SYSTEM_MANDATORY_LABEL_ACE_TYPE:
		sACEType	=	TEXT ("mandatory_label");
		break;
	}

	return sACEType;
}


//
// GetACEFlags: Return a string with the flags of an ACE
//
CString CSetACL::GetACEFlags (BYTE nACEFlags)
{
	CString	sFlags;

	if (nACEFlags & CONTAINER_INHERIT_ACE)
		 sFlags	+=	TEXT ("container_inherit+");
	if (nACEFlags & OBJECT_INHERIT_ACE)
		 sFlags	+= TEXT ("object_inherit+");
	if (nACEFlags & INHERIT_ONLY_ACE)
		 sFlags	+= TEXT ("inherit_only+");
	if (nACEFlags & NO_PROPAGATE_INHERIT_ACE)
		 sFlags	+= TEXT ("no_propagate_inherit+");
	if (nACEFlags & INHERITED_ACE)
		 sFlags	+= TEXT ("inherited+");
	if (nACEFlags & SUCCESSFUL_ACCESS_ACE_FLAG)
		 sFlags	+= TEXT ("audit_success+");
	if (nACEFlags & FAILED_ACCESS_ACE_FLAG)
		 sFlags	+= TEXT ("audit_fail+");

	sFlags.TrimRight (TEXT ("+"));

	if (sFlags.IsEmpty ())
	{
		sFlags	=	TEXT ("no_inheritance");
	}

	return sFlags;
}


//
// SetPrivilege: Enable a privilege (user right) for the current process
//
DWORD CSetACL::SetPrivilege (CString sPrivilege, BOOL fEnable)
{
	HANDLE				hToken	=	NULL;		// handle to process token
	TOKEN_PRIVILEGES	tkp;						// pointer to token structure

	// Get the current process token handle
	if (! OpenProcessToken (GetCurrentProcess (), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return RTN_ERR_EN_PRIV;
	}

	// Get the LUID for the privilege on the target (or local) system
	if (! LookupPrivilegeValue (m_sTargetSystemName, sPrivilege, &tkp.Privileges[0].Luid))
	{
		CloseHandle (hToken);
		return RTN_ERR_EN_PRIV;
	}

	// One privilege to set
	tkp.PrivilegeCount					=	1;
	tkp.Privileges[0].Attributes		=	(fEnable ? SE_PRIVILEGE_ENABLED : 0);

	// Enable the privilege
	if (! AdjustTokenPrivileges (hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0) || GetLastError ())
	{
		CloseHandle (hToken);
		return RTN_ERR_EN_PRIV;
	}

	CloseHandle (hToken);
	return RTN_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Class CTrustee
//
//////////////////////////////////////////////////////////////////////


//
// Constructor: initialize all member variables
//
CTrustee::CTrustee (CString sTrustee, BOOL fTrusteeIsSID, DWORD nAction, BOOL fDACL, BOOL fSACL)
{
	m_sTrustee			=	sTrustee;
	m_fTrusteeIsSID	=	fTrusteeIsSID;
	m_psidTrustee		=	NULL;
	m_nAction			=	nAction;
	m_fDACL				=	fDACL;
	m_fSACL				=	fSACL;
	m_oNewTrustee		=	NULL;
}


CTrustee::CTrustee ()
{
	m_sTrustee			=	TEXT ("");
	m_fTrusteeIsSID	=	false;
	m_psidTrustee		=	NULL;
	m_nAction			=	0;
	m_fDACL				=	false;
	m_fSACL				=	false;
	m_oNewTrustee		=	NULL;
}


//
// Destructor: clean up
//
CTrustee::~CTrustee ()
{
	if (m_psidTrustee)
	{
		LocalFree (m_psidTrustee);
	}
}


//
// LookupSID: Lookup/calculate the binary SID of a given trustee
//
DWORD CTrustee::LookupSID ()
{
	// Is there a valid trustee object?
	if (m_sTrustee.IsEmpty ())
	{
		return RTN_OK;
	}

	// If there already is a binary SID, delete it (playing it safe)
	if (m_psidTrustee)
	{
		LocalFree (m_psidTrustee);
		m_psidTrustee		=	NULL;
	}

	// If the trustee was specified as a string SID, it has to be converted into a binary SID.
	if (m_fTrusteeIsSID)
	{
		if (! ConvertStringSidToSid (m_sTrustee, &m_psidTrustee))
		{
			m_psidTrustee	=	NULL;

			return RTN_ERR_LOOKUP_SID;
		}

		return RTN_OK;
	}

	//
	// The trustee is not a (string) SID. Look up the SID for the account name.
	//
	CString							sSystemName;
	CString							sAccountDomain;
	CString							sAccountName;
	CString							sDomainName;
	SID_NAME_USE					eUse;
	DWORD								nBufTrustee			=	4096;
	PSID								psidTrustee			=	LocalAlloc (LPTR, nBufTrustee);
	DWORD								nBufDomain			=	4096;
	PDOMAIN_CONTROLLER_INFO		pdcInfo				=	NULL;
	BOOL								fIsDomainName		=	false;

	// The trustee name can have a preceding machine/domain name, or not. In the latter case the local machine or the domain it is a member of is used by the API function.
	int nBackslashPos		= m_sTrustee.Find (TEXT ("\\"));
	if (nBackslashPos != -1)
	{
		sSystemName			=	m_sTrustee.Left (nBackslashPos);
		sAccountDomain		=	sSystemName;
		sAccountName		=	m_sTrustee.Right (m_sTrustee.GetLength () - nBackslashPos - 1);
	}
	else
	{
		sAccountName		=	m_sTrustee;
	}

	//
	// If a machine/domain name was specified, we assume it is a domain name, and search for a DC.
	//
	if (! sSystemName.IsEmpty ())
	{
		// If a DC can be found, we use it. If not, the name might be a computer name.
		if (DsGetDcName (NULL, sSystemName, NULL, NULL, DS_RETURN_FLAT_NAME, &pdcInfo) == ERROR_SUCCESS)
		{
			sSystemName		=	pdcInfo->DomainControllerName;
			sAccountDomain	=	pdcInfo->DomainName;
			fIsDomainName	=	true;

			// Remove the leading backslashes in the name
			if (sSystemName.Left (2) == TEXT ("\\\\"))
			{
				sSystemName	=	sSystemName.Right (sSystemName.GetLength () - 2);
			}

			// Free the buffer allocated by the API function
			if (pdcInfo)
			{
				NetApiBufferFree (pdcInfo);
			}
		}
		else
		{
			// No DC was found. The system name must be a computer name. Check whether it is a DC
			DSROLE_PRIMARY_DOMAIN_INFO_BASIC*	dsrolebasicDCInfo;

			if (DsRoleGetPrimaryDomainInformation (sSystemName, DsRolePrimaryDomainInfoBasic, (PBYTE*) &dsrolebasicDCInfo) == ERROR_SUCCESS)
			{
				if (dsrolebasicDCInfo->MachineRole == DsRole_RoleBackupDomainController || dsrolebasicDCInfo->MachineRole == DsRole_RolePrimaryDomainController)
				{
					sAccountDomain	=	dsrolebasicDCInfo->DomainNameFlat;
					fIsDomainName	=	true;
				}

				DsRoleFreeMemory (dsrolebasicDCInfo);
			}
		}
	}

	//
	// We'll search for the account on either a DC found, a remote machine specified or on the local computer.
	//
	if (LookupAccountName (sSystemName.IsEmpty () ? NULL : sSystemName.GetBuffer (sSystemName.GetLength ()), sAccountName, psidTrustee, &nBufTrustee, sDomainName.GetBuffer (4096), &nBufDomain, &eUse))
	{
		sSystemName.ReleaseBuffer ();
		sDomainName.ReleaseBuffer ();

		// The account name was found, but it might be in a trusted domain instead of the domain specified -> check the domain
		if (sSystemName.IsEmpty () || (! fIsDomainName) || sDomainName.CompareNoCase (sAccountDomain) == 0)
		{
			m_psidTrustee			=	CopySID (psidTrustee);
		}

		if (psidTrustee) LocalFree (psidTrustee);

		if (! m_psidTrustee)
		{
			return RTN_ERR_LOOKUP_SID;
		}
		else
		{
			return RTN_OK;
		}
	}
	else
	{
		sSystemName.ReleaseBuffer ();
		sDomainName.ReleaseBuffer ();

		if (psidTrustee) LocalFree (psidTrustee);

		return RTN_ERR_LOOKUP_SID;
	}
}


//////////////////////////////////////////////////////////////////////
//
// Class CDomain
//
//////////////////////////////////////////////////////////////////////


//
// Constructor: initialize all member variables
//
CDomain::CDomain ()
{
	m_sDomain			=	TEXT ("");
	m_nAction			=	0;
	m_fDACL				=	false;
	m_fSACL				=	false;
	m_oNewDomain		=	NULL;
}


//
// Destructor: clean up
//
CDomain::~CDomain ()
{
}


//
// SetDomain: Set the domain and do some initialization
//
DWORD CDomain::SetDomain (CString sDomain, DWORD nAction, BOOL fDACL, BOOL fSACL)
{
	PDOMAIN_CONTROLLER_INFO		pdcInfo				=	NULL;

	// Reset all member variables
	m_sDomain			=	TEXT ("");
	m_nAction			=	0;
	m_fDACL				=	false;
	m_fSACL				=	false;

	if (m_oNewDomain)
	{
		delete m_oNewDomain;
		m_oNewDomain		=	NULL;
	}

	if (sDomain.IsEmpty ())
	{
		return RTN_ERR_INV_DOMAIN;
	}

	// Get the NetBIOS name of the domain
	if (DsGetDcName (NULL, sDomain, NULL, NULL, DS_RETURN_FLAT_NAME, &pdcInfo) == ERROR_SUCCESS)
	{
		m_sDomain		=	pdcInfo->DomainName;

		// Free the buffer allocated by the API function
		if (pdcInfo)
		{
			NetApiBufferFree (pdcInfo);
		}
	}
	else
	{
		return RTN_ERR_INV_DOMAIN;
	}

	m_nAction			=	nAction;
	m_fDACL				=	fDACL;
	m_fSACL				=	fSACL;
	m_oNewDomain		=	NULL;

	return RTN_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Class CACE
//
//////////////////////////////////////////////////////////////////////


//
// Constructor: initialize all member variables
//
CACE::CACE (CTrustee* pTrustee, CString sPermission, DWORD nInheritance, BOOL fInhSpecified, ACCESS_MODE nAccessMode, DWORD nACLType)
{
	if (pTrustee)
	{
		m_pTrustee		=	pTrustee;
	}
	else
	{
		m_pTrustee		=	NULL;
	}

	m_sPermission		=	sPermission;
	m_nInheritance		=	nInheritance;
	m_fInhSpecified	=	fInhSpecified;
	m_nAccessMode		=	nAccessMode;
	m_nACLType			=	nACLType;
	m_nAccessMask		=	0;
}


CACE::CACE ()
{
	m_pTrustee			=	NULL;
	m_sPermission		=	TEXT ("");
	m_nInheritance		=	0;
	m_fInhSpecified	=	false;
	m_nAccessMode		=	NOT_USED_ACCESS;
	m_nACLType			=	0;
	m_nAccessMask		=	0;
}


//
// Destructor: clean up
//
CACE::~CACE ()
{
	if (m_pTrustee)
	{
		delete m_pTrustee;
	}
}


//////////////////////////////////////////////////////////////////////
//
// Class CSD
//
//////////////////////////////////////////////////////////////////////


//
// Constructor: initialize all member variables
//
CSD::CSD (CSetACL* setaclMain)
{
	m_setaclMain				=	setaclMain;

	m_sObjectPath				=	TEXT ("");
	m_nAPIError					=	0;
	m_nObjectType				=	SE_UNKNOWN_OBJECT_TYPE;
	m_siSecInfo					=	NULL;
	m_psdSD						=	NULL;
	m_paclDACL					=	NULL;
	m_paclSACL					=	NULL;
	m_psidOwner					=	NULL;
	m_psidGroup					=	NULL;
	m_fBufSDAlloc				=	false;
	m_fBufDACLAlloc			=	false;
	m_fBufSACLAlloc			=	false;
	m_fBufOwnerAlloc			=	false;
	m_fBufGroupAlloc			=	false;
	m_fUseLowLevelWrites2SD	=	false;
}


//
// Destructor: clean up
//
CSD::~CSD ()
{
	DeleteBufSD ();
	DeleteBufDACL ();
	DeleteBufSACL ();
	DeleteBufOwner ();
	DeleteBufGroup ();
}


//
// DeleteBufSD: Delete the buffer for the SD
//
void CSD::DeleteBufSD ()
{
	if (m_psdSD && m_fBufSDAlloc)
	{
		LocalFree (m_psdSD);
		m_psdSD			=	NULL;
		m_fBufSDAlloc	=	false;
	}
}


//
// DeleteBufDACL: Delete the buffer for the DACL
//
void CSD::DeleteBufDACL ()
{
	if (m_paclDACL && m_fBufDACLAlloc)
	{
		LocalFree (m_paclDACL);
		m_paclDACL			=	NULL;
		m_fBufDACLAlloc	=	false;
	}
}


//
// DeleteBufSACL: Delete the buffer for the SACL
//
void CSD::DeleteBufSACL ()
{
	if (m_paclSACL && m_fBufSACLAlloc)
	{
		LocalFree (m_paclSACL);
		m_paclSACL			=	NULL;
		m_fBufSACLAlloc	= false;
	}
}


//
// DeleteBufOwner: Delete the buffer for the Owner
//
void CSD::DeleteBufOwner ()
{
	if (m_psidOwner && m_fBufOwnerAlloc)
	{
		LocalFree (m_psidOwner);
		m_psidOwner			=	NULL;
		m_fBufOwnerAlloc	=	false;
	}
}


//
// DeleteBufGroup: Delete the buffer for the Group
//
void CSD::DeleteBufGroup ()
{
	if (m_psidGroup && m_fBufGroupAlloc)
	{
		LocalFree (m_psidGroup);
		m_psidGroup			=	NULL;
		m_fBufGroupAlloc	=	false;
	}
}


//
// GetSD: Get the SD and other information indicated by siSecInfo
//
DWORD CSD::GetSD (CString sObjectPath, SE_OBJECT_TYPE nObjectType, SECURITY_INFORMATION siSecInfo)
{
	BOOL									fSDRead			=	false;
	BOOL									fOK				=	true;
	DWORD									nError			=	RTN_OK;
	DWORD									nDesiredAccess	=	READ_CONTROL;
	HANDLE								hFile				=	NULL;
	HKEY									hKey				=	NULL;
	HANDLE								hAny				=	NULL;
	PSECURITY_DESCRIPTOR				pSDSelfRel		=	NULL;
	DWORD									nBufSD			=	0;
	DWORD									nBufDACL			=	0;
	DWORD									nBufSACL			=	0;
	DWORD									nBufOwner		=	0;
	DWORD									nBufGroup		=	0;

	// Check the parameters
	if (sObjectPath.IsEmpty ())
	{
		return RTN_ERR_PARAMS;
	}
	if (nObjectType == 0)
	{
		return RTN_ERR_PARAMS;
	}

	m_sObjectPath	=	sObjectPath;
	m_nObjectType	=	nObjectType;
	m_siSecInfo		=	siSecInfo;

	if (siSecInfo & SACL_SECURITY_INFORMATION)
	{
		// Enable a privilege needed to read the SACL
		if (m_setaclMain->SetPrivilege (SE_SECURITY_NAME, true) != ERROR_SUCCESS)
		{
			return RTN_ERR_EN_PRIV;
		}

		// We need to have more access
		nDesiredAccess	|=	ACCESS_SYSTEM_SECURITY;
	}

	//
	// Use black magic (like backup programs) to read ALL SDs even if access is (normally) denied.
	//
	if (m_nObjectType	==	SE_FILE_OBJECT)
	{
		// Get a handle to the file/dir with special backup access (privilege SeBackupName has to be enabled for this)
		hFile		=	CreateFile (m_sObjectPath, nDesiredAccess, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

		if (hFile == INVALID_HANDLE_VALUE || hFile == NULL)
		{
			hFile		=	NULL;
			goto GetSD2ndTry;
		}
		else
		{
			hAny		=	hFile;
		}
	}
	else if (m_nObjectType == SE_REGISTRY_KEY)
	{
		// Open the key
		nError	=	m_setaclMain->OpenRegKey (&m_sObjectPath, &hKey);

		if (nError != RTN_OK)
		{
			hKey		=	NULL;
			goto GetSD2ndTry;
		}
		else
		{
			hAny		=	hKey;
		}
	}

	if (! hAny)
	{
		goto GetSD2ndTry;
	}

	// Get the size of the buffer needed
	GetKernelObjectSecurity (hAny, siSecInfo, &pSDSelfRel, 0, &nBufSD);
	if (! nBufSD)
	{
		goto GetSD2ndTry;
	}

	// Allocate a buffer for the SD.
	pSDSelfRel			=	LocalAlloc (LPTR, nBufSD);
	if (pSDSelfRel == NULL)
	{
		m_nAPIError	=	GetLastError ();

		return RTN_ERR_OUT_OF_MEMORY;
	}

	// Get the SD
	if (! GetKernelObjectSecurity (hAny, siSecInfo, pSDSelfRel, nBufSD, &nBufSD))
	{
		LocalFree (pSDSelfRel);
		pSDSelfRel		=	NULL;
		fSDRead			=	false;

		goto GetSD2ndTry;
	}
	else
	{
		fSDRead			=	true;
	}

GetSD2ndTry:

	// Close the open handles
	if (hFile)
	{
		CloseHandle (hFile);
		hFile			=	NULL;
	}
	if (hKey)
	{
		RegCloseKey (hKey);
		hKey			=	NULL;
	}

	if (! fSDRead)
	{
		// Above procedure did not work or this is an object type other than file system or registry: use the regular method
		m_nAPIError	=	GetNamedSecurityInfo (m_sObjectPath.GetBuffer (m_sObjectPath.GetLength () + 1), m_nObjectType, siSecInfo, NULL, NULL, NULL, NULL, &pSDSelfRel);
		m_sObjectPath.ReleaseBuffer ();

		if (m_nAPIError != ERROR_SUCCESS)
		{
			return RTN_ERR_GETSECINFO;
		}
	}

	// Does the object have an SD?
	if (pSDSelfRel == NULL)
	{
		// The SD is NULL -> no need to do further processing
		return RTN_OK;
	}

	//
	// We have got the SD.
	//
	// Convert the self-relative SD into an absolute SD:
	//
	// 1. Determine the size of the buffers needed
	nBufSD			=	0;

	MakeAbsoluteSD (pSDSelfRel, NULL, &nBufSD, NULL, &nBufDACL, NULL, &nBufSACL, NULL, &nBufOwner, NULL, &nBufGroup);
	if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
	{
		nError		=	RTN_ERR_CONVERT_SD;
		m_nAPIError	=	GetLastError ();

		return nError;
	}

	// 2.1 Delete old buffers
	DeleteBufSD ();
	DeleteBufDACL ();
	DeleteBufSACL ();
	DeleteBufOwner ();
	DeleteBufGroup ();

	// 2.2 Allocate new buffers
	m_psdSD					=	(PSECURITY_DESCRIPTOR) LocalAlloc (LPTR, nBufSD);
	m_fBufSDAlloc			=	true;

	if (nBufDACL)
	{
		m_paclDACL			=	(PACL) LocalAlloc (LPTR, nBufDACL);
		m_fBufDACLAlloc	=	true;
	}
	if (nBufSACL)
	{
		m_paclSACL			=	(PACL) LocalAlloc (LPTR, nBufSACL);
		m_fBufSACLAlloc	=	true;
	}
	if (nBufOwner)
	{
		m_psidOwner			=	(PSID) LocalAlloc (LPTR, nBufOwner);
		m_fBufOwnerAlloc	=	true;
	}
	if (nBufGroup)
	{
		m_psidGroup			=	(PSID) LocalAlloc (LPTR, nBufGroup);
		m_fBufGroupAlloc	=	true;
	}

	// 3. Do the conversion
	fOK	=	MakeAbsoluteSD (pSDSelfRel, m_psdSD, &nBufSD, m_paclDACL, &nBufDACL, m_paclSACL, &nBufSACL, m_psidOwner, &nBufOwner, m_psidGroup, &nBufGroup);

	// Free the temporary self-relative SD
	LocalFree (pSDSelfRel);
	pSDSelfRel				=	NULL;

	if (! fOK || ! IsValidSecurityDescriptor (m_psdSD))
	{
		nError				=	RTN_ERR_CONVERT_SD;
		m_nAPIError			=	GetLastError ();

		return nError;
	}

	return nError;
}


//
// SetSD: Set the SD and other information indicated by siSecInfo
//
DWORD CSD::SetSD (CString sObjectPath, SE_OBJECT_TYPE nObjectType, SECURITY_INFORMATION siSecInfo, PACL paclDACL, PACL paclSACL, PSID psidOwner, PSID psidGroup)
{
	DWORD									nError			=	RTN_OK;
	DWORD									nDesiredAccess	=	WRITE_OWNER | WRITE_DAC;
	HANDLE								hFile				=	NULL;
	HKEY									hKey				=	NULL;
	HANDLE								hAny				=	NULL;

	// Check the parameters
	if (sObjectPath.IsEmpty ())
	{
		return RTN_ERR_PARAMS;
	}
	if (nObjectType == 0)
	{
		return RTN_ERR_PARAMS;
	}

	if (m_psdSD == NULL)
	{
		if (m_sObjectPath.IsEmpty () || m_nObjectType == 0)
		{
			// The SD has not yet been fetched -> get it
			nError			=	GetSD (sObjectPath, nObjectType, siSecInfo);

			if (nError)
			{
				return nError;
			}
		}
		else
		{
			// The object has a NULL SD -> we have to create a new SD
			m_psdSD			=	(PSECURITY_DESCRIPTOR) LocalAlloc (LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

			if (m_psdSD == NULL)
			{
				m_fBufSDAlloc	=	false;

				return RTN_ERR_OUT_OF_MEMORY;
			}
			else
			{
				m_fBufSDAlloc	=	true;
			}

			if (! InitializeSecurityDescriptor (m_psdSD, SECURITY_DESCRIPTOR_REVISION))
			{
				return RTN_ERR_CREATE_SD;
			}
		}
	}

	if (siSecInfo & OWNER_SECURITY_INFORMATION)
	{
		// Enable a privilege needed to set the Owner
		if (m_setaclMain->SetPrivilege (SE_TAKE_OWNERSHIP_NAME, true) != ERROR_SUCCESS)
		{
			return RTN_ERR_EN_PRIV;
		}
	}
	if (siSecInfo & SACL_SECURITY_INFORMATION)
	{
		// Enable a privilege needed to read the SACL
		if (m_setaclMain->SetPrivilege (SE_SECURITY_NAME, true) != ERROR_SUCCESS)
		{
			return RTN_ERR_EN_PRIV;
		}

		// We need to have more access
		nDesiredAccess	|=	ACCESS_SYSTEM_SECURITY;
	}

	// Set owner, group, DACL and SACL in the SD
	if (siSecInfo & DACL_SECURITY_INFORMATION)
	{
		if (SetSecurityDescriptorDacl (m_psdSD, true, paclDACL, false))
		{
			if (m_paclDACL != paclDACL)
			{
				DeleteBufDACL ();

				m_paclDACL				=	paclDACL;
				m_fBufDACLAlloc		=	false;
			}
		}
		else
		{
			m_nAPIError					=	GetLastError ();
			return RTN_ERR_SET_SD_DACL;
		}
	}
	if (siSecInfo & SACL_SECURITY_INFORMATION)
	{
		if (SetSecurityDescriptorSacl (m_psdSD, true, paclSACL, false))
		{
			if (m_paclSACL != paclSACL)
			{
				DeleteBufSACL ();

				m_paclSACL				=	paclSACL;
				m_fBufSACLAlloc		=	false;
			}
		}
		else
		{
			m_nAPIError					=	GetLastError ();
			return RTN_ERR_SET_SD_SACL;
		}
	}
	if (siSecInfo & OWNER_SECURITY_INFORMATION)
	{
		if (SetSecurityDescriptorOwner (m_psdSD, psidOwner, false))
		{
			if (m_psidOwner != psidOwner)
			{
				DeleteBufOwner ();

				m_psidOwner				=	psidOwner;
				m_fBufOwnerAlloc		=	false;
			}
		}
		else
		{
			m_nAPIError					=	GetLastError ();
			return RTN_ERR_SET_SD_OWNER;
		}
	}
	if (siSecInfo & GROUP_SECURITY_INFORMATION)
	{
		if (SetSecurityDescriptorGroup (m_psdSD, psidGroup, false))
		{
			if (m_psidGroup != psidGroup)
			{
				DeleteBufGroup ();

				m_psidGroup				=	psidGroup;
				m_fBufGroupAlloc		=	false;
			}
		}
		else
		{
			m_nAPIError					=	GetLastError ();
			return RTN_ERR_SET_SD_GROUP;
		}
	}

	// Check: we now should have a valid SD and no error
	if (! IsValidSecurityDescriptor (m_psdSD))
	{
		return RTN_ERR_INVALID_SD;
	}

	//
	// The code below should be used only after VERY thorough consideration:
	//
	// It tries to set the SD using SetKernelObjectSecurity (), which is a low-level security API function
	// and does not handle inheritance at all: permissions are not propagated to sub-objects, and the "inherited"
	// flag of the object's ACEs is not set/cleared as necessary. Since SetACL does not do this either, this
	// normally results in incorrect ACLs.
	//
	// The BIG advantage of SetKernelObjectSecurity is, given appropriate, usually admin, privileges, that it
	// does not perform access checks but sets the SD on any object, regardless of it's current permissions and owner.
	//
	if (m_fUseLowLevelWrites2SD)
	{
		// Use black magic (like backup programs) to write ALL SDs even if access is (normally) denied.

		if (m_nObjectType	==	SE_FILE_OBJECT)
		{
			// Get a handle to the file/dir with special backup access (privilege SeBackupName has to be enabled for this)
			hFile		=	CreateFile (m_sObjectPath, nDesiredAccess, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

			if (hFile == INVALID_HANDLE_VALUE || hFile == NULL)
			{
				hFile		=	NULL;
				goto SetSD2ndTry;
			}
			else
			{
				hAny		=	hFile;
			}
		}
		else if (m_nObjectType == SE_REGISTRY_KEY)
		{
			// Open the key
			nError	=	m_setaclMain->OpenRegKey (&m_sObjectPath, &hKey);

			if (nError != RTN_OK)
			{
				hKey		=	NULL;
				goto SetSD2ndTry;
			}
			else
			{
				hAny		=	hKey;
			}
		}

		if (! hAny)
		{
			goto SetSD2ndTry;
		}

		// Set protection and auto inheritance flags correctly in the security descriptor
		if (siSecInfo & DACL_SECURITY_INFORMATION)
		{
			if (! SetSecurityDescriptorControl (m_psdSD, SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED, SE_DACL_AUTO_INHERIT_REQ | SE_DACL_AUTO_INHERITED))
			{
				goto SetSD2ndTry;
			}
		}
		if (siSecInfo & SACL_SECURITY_INFORMATION)
		{
			if (! SetSecurityDescriptorControl (m_psdSD, SE_SACL_AUTO_INHERIT_REQ | SE_SACL_AUTO_INHERITED, SE_SACL_AUTO_INHERIT_REQ | SE_SACL_AUTO_INHERITED))
			{
				goto SetSD2ndTry;
			}
		}
		if (siSecInfo & PROTECTED_DACL_SECURITY_INFORMATION)
		{
			if (! SetSecurityDescriptorControl (m_psdSD, SE_DACL_PROTECTED, SE_DACL_PROTECTED))
			{
				goto SetSD2ndTry;
			}
		}
		if (siSecInfo & PROTECTED_SACL_SECURITY_INFORMATION)
		{
			if (! SetSecurityDescriptorControl (m_psdSD, SE_SACL_PROTECTED, SE_SACL_PROTECTED))
			{
				goto SetSD2ndTry;
			}
		}
		if (siSecInfo & UNPROTECTED_DACL_SECURITY_INFORMATION)
		{
			if (! SetSecurityDescriptorControl (m_psdSD, SE_DACL_PROTECTED, 0))
			{
				goto SetSD2ndTry;
			}
		}
		if (siSecInfo & UNPROTECTED_SACL_SECURITY_INFORMATION)
		{
			if (! SetSecurityDescriptorControl (m_psdSD, SE_SACL_PROTECTED, 0))
			{
				goto SetSD2ndTry;
			}
		}


		// Set the SD
		if (! SetKernelObjectSecurity (hAny, siSecInfo, m_psdSD))
		{
			goto SetSD2ndTry;
		}
		else
		{
			m_nAPIError	=	ERROR_SUCCESS;

			return RTN_OK;
		}
	}	// if (m_fUseLowLevelWrites2SD)

SetSD2ndTry:

	// Close the open handles
	if (hFile)
	{
		CloseHandle (hFile);
		hFile			=	NULL;
	}
	if (hKey)
	{
		RegCloseKey (hKey);
		hKey			=	NULL;
	}

	// m_fUseLowLevelWrites2SD is set to false (the default), above procedure did not work, or this is an object type other than file system or registry: use the regular method to set the SD
	m_nAPIError	=	SetNamedSecurityInfo (m_sObjectPath.GetBuffer (m_sObjectPath.GetLength () + 1), nObjectType, siSecInfo, psidOwner, psidGroup, paclDACL, paclSACL);
	m_sObjectPath.ReleaseBuffer ();

	if (m_nAPIError != ERROR_SUCCESS)
	{
		nError		=	RTN_ERR_SETSECINFO;
	}

	return nError;
}


//////////////////////////////////////////////////////////////////////
//
// Global functions
//
//////////////////////////////////////////////////////////////////////


//
// Split: Called by various CSetACL functions. Emulation of Perl's split function.
//
BOOL Split (CString sDelimiter, CString sInput, CStringArray* asOutput)
{
	try
	{
		// Delete contents of the output array
		asOutput->RemoveAll ();

		// Remove leading and trailing whitespace
		sInput.TrimLeft ();
		sInput.TrimRight ();

		// Find each substring and add it to the output array
		int i = 0, j;
		while ((j = sInput.Find (sDelimiter, i)) != -1)
		{
			asOutput->Add (sInput.Mid (i, j - i));
			i = j + 1;
		}

		// Is there an element left at the end?
		if (sInput.GetLength () > i)
		{
			asOutput->Add (sInput.Mid (i, sInput.GetLength () - i));
		}

		return TRUE;
	}
	catch (CMemoryException* exc)
	{
		exc->Delete ();

		return FALSE;
	}
}


//
// CopySID: Copy a SID
//
PSID CopySID (PSID pSID)
{
	PSID		pSIDNew				=	NULL;

	if (! pSID)
	{
		return NULL;
	}

	// Allocate memory for the SID
	pSIDNew	= (PSID) LocalAlloc (LPTR, GetLengthSid (pSID));

	if (! pSIDNew)
	{
		return NULL;
	}

	// Copy the SID
	if (! CopySid (GetLengthSid (pSID), pSIDNew, pSID))
	{
		LocalFree (pSIDNew);
		return NULL;
	}

	return pSIDNew;
}


//////////////////////////////////////////////////////////////////////
//
// BuildLongUnicodePath: Take any path any turn it into a path that is not limited to MAX_PATH, if possible
//
//////////////////////////////////////////////////////////////////////


void BuildLongUnicodePath (CString* sPath)
{
	// Check this is not already a "long" path
	if (sPath->Left (3) == TEXT ("\\\\?"))
	{
		return;
	}

	#ifdef UNICODE
		if (sPath->Left (2) == TEXT ("\\\\"))
		{
			// This is a UNC path. Build a path in the following format: \\?\UNC\<server>\<share>
			sPath->Insert (2, TEXT ("?\\UNC\\"));
		}
		else
		{
			// Check this is an absolute path
			if (sPath->Mid (1, 2) == TEXT (":\\"))
			{
				// This is an absolute non-UNC. Build a path in the following format: \\?\<drive letter>:\<path>
				sPath->Insert (0, TEXT ("\\\\?\\"));
			}
		}
	#endif
}
