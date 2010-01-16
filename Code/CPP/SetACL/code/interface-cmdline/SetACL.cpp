/////////////////////////////////////////////////////////////////////////////
//
//
//	SetACL.cpp
//
//
//	Description:	Command line interface for SetACL
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


#include	"SetACL.h"


//////////////////////////////////////////////////////////////////////
//
// Defines
//
//////////////////////////////////////////////////////////////////////


#define	TXTPRINTHELP	TEXT("\nType 'SetACL -help' for help.\n")


//////////////////////////////////////////////////////////////////////
//
// Global variables
//
//////////////////////////////////////////////////////////////////////


// The application object
CWinApp	theApp;

// Are we in silent mode?
BOOL		fSilent;


//////////////////////////////////////////////////////////////////////
//
// Functions
//
//////////////////////////////////////////////////////////////////////


//
// main
//
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	UNUSED_ALWAYS(envp);		// suppress warning (unreferenced formal parameter)

	// Our SetACL class object
	CSetACL	oSetACL (PrintMsg);

	// Return code to pass to caller
	int		nRetCode	= 0;

	// Other variable initializations
	fSilent	=	false;


	// Initialize MFC
	if (!AfxWinInit (::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		printf ("ERROR: MFC initialization failed!\n");

		return RTN_ERR_GENERAL;
	}

	// Process the command line
	if ((nRetCode = ProcessCmdLine (argc, argv, PrintMsg, &oSetACL)) != RTN_OK)
	{
		return nRetCode;
	}

	// Run the program
	nRetCode	=	oSetACL.Run ();

	// Return status
	return nRetCode;
}


//
// ProcessCmdLine: Process the command line
//
DWORD ProcessCmdLine (int argc, TCHAR* argv[], void (* funcNotify) (CString), CSetACL* oSetACL)
{
	// Check arguments
	if (! oSetACL || ! argv)
	{
		return RTN_ERR_GENERAL;
	}

	int		nRetCode				=	0;

	CString	sObjectName;
	DWORD		nObjectType			=	0;
	BOOL		fActionClear		=	false;
	BOOL		fActionSetProt		=	false;
	BOOL		fActionReset		=	false;
	DWORD		nRecursionType		=	RECURSE_NO;
	BOOL		fDACLReset			=	false;
	BOOL		fSACLReset			=	false;
	BOOL		fClearDACL			=	false;
	BOOL		fClearSACL			=	false;
	DWORD		nDACLProtected		=	INHPARNOCHANGE;
	DWORD		nSACLProtected		=	INHPARNOCHANGE;
	CString	sArg;
	CString	sParam;


	// Invocation without arguments -> message and quit
	if (argc	==	1)
	{
		if (funcNotify)
		{
			(*funcNotify) (TXTPRINTHELP);
		}

		return RTN_ERR_PARAMS;
	}

	// Should we print the help page?
	if (argc > 1)
	{
		sArg			=	argv[1];

		if (sArg.CompareNoCase (TEXT ("-help")) == 0 || sArg.CompareNoCase (TEXT ("-?")) == 0)
		{
			PrintHelp ();

			return RTN_USAGE;
		}
	}

	//
	// Loop over all arguments
	//
	for (int i = 1; i < argc; i++)
	{
		sArg			=	argv[i];

		// Is this an argument that must be followed by a parameter?
		if (sArg.CompareNoCase (TEXT ("-silent")) == 0)
		{
			fSilent	=	true;
		}
		else if (sArg.CompareNoCase (TEXT ("-ignoreerr")) == 0)
		{
			oSetACL->SetIgnoreErrors (true);
		}
		else
		{
			if (i == argc - 1)
			{
				if (funcNotify)
				{
					(*funcNotify) (TEXT ("ERROR in command line: No parameter found for option ") + sArg + TEXT ("!\n") + TXTPRINTHELP);
				}

				return RTN_ERR_PARAMS;
			}
			else
			{
				sParam	=	argv[i + 1];
			}

			// Check if the following argument begins with a '-'. If yes -> error
			if (sParam.Left (1) == TEXT ("-"))
			{
				if (funcNotify)
				{
					(*funcNotify) (TEXT ("ERROR in command line: No parameter found for option ") + sArg + TEXT ("!\n") + TXTPRINTHELP);
				}

				return RTN_ERR_PARAMS;
			}

			// Do not process the parameter during the next loop - this is done now
			i++;

			//
			// Process the parameter
			//
			if (sArg.CompareNoCase (TEXT ("-on")) == 0)
			{
				sObjectName		=	sParam;

				if (nObjectType)
				{
					if (oSetACL->SetObject (sObjectName, (SE_OBJECT_TYPE) nObjectType) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR while processing command line: object (name, type): ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-ot")) == 0)
			{
				if (sParam.CompareNoCase (TEXT ("file")) == 0)
				{
					nObjectType	=	SE_FILE_OBJECT;
				}
				else if (sParam.CompareNoCase (TEXT ("reg")) == 0)
				{
					nObjectType	=	SE_REGISTRY_KEY;
				}
				else if (sParam.CompareNoCase (TEXT ("srv")) == 0)
				{
					nObjectType	=	SE_SERVICE;
				}
				else if (sParam.CompareNoCase (TEXT ("prn")) == 0)
				{
					nObjectType	=	SE_PRINTER;
				}
				else if (sParam.CompareNoCase (TEXT ("shr")) == 0)
				{
					nObjectType	=	SE_LMSHARE;
				}
				else
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid object type specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				if (! sObjectName.IsEmpty ())
				{
					if (oSetACL->SetObject (sObjectName, (SE_OBJECT_TYPE) nObjectType) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR while processing command line: object (name, type): ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-actn")) == 0)
			{
				if (sParam.CompareNoCase (TEXT ("ace")) == 0)
				{
					if (oSetACL->AddAction (ACTN_ADDACE) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("trustee")) == 0)
				{
					if (oSetACL->AddAction (ACTN_TRUSTEE) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("domain")) == 0)
				{
					if (oSetACL->AddAction (ACTN_DOMAIN) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("list")) == 0)
				{
					if (oSetACL->AddAction (ACTN_LIST) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("restore")) == 0)
				{
					if (oSetACL->AddAction (ACTN_RESTORE) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("setowner")) == 0)
				{
					if (oSetACL->AddAction (ACTN_SETOWNER) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("setgroup")) == 0)
				{
					if (oSetACL->AddAction (ACTN_SETGROUP) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: ") + sParam + TEXT (" could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("clear")) == 0)
				{
					fActionClear	=	true;

					if (fClearDACL)
					{
						if (oSetACL->AddAction (ACTN_CLEARDACL) != RTN_OK)
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: CLEARDACL could not be set!\n"));
							}

							return RTN_ERR_PARAMS;
						}
					}

					if (fClearSACL)
					{
						if (oSetACL->AddAction (ACTN_CLEARSACL) != RTN_OK)
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: CLEARSACL could not be set!\n"));
							}

							return RTN_ERR_PARAMS;
						}
					}
				}
				else if (sParam.CompareNoCase (TEXT ("setprot")) == 0)
				{
					fActionSetProt	=	true;

					if (oSetACL->AddAction (ACTN_SETINHFROMPAR) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: SETINHFROMPAR could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else if (sParam.CompareNoCase (TEXT ("rstchldrn")) == 0)
				{
					fActionReset	=	true;

					if (oSetACL->AddAction (ACTN_RESETCHILDPERMS) != RTN_OK)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: RESETCHILDPERMS could not be set!\n"));
						}

						return RTN_ERR_PARAMS;
					}
				}
				else
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid action specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-rec")) == 0)
			{
				if (sParam.CompareNoCase (TEXT ("no")) == 0)
				{
					nRecursionType	=	RECURSE_NO;
				}
				else if (sParam.CompareNoCase (TEXT ("cont")) == 0)
				{
					nRecursionType	=	RECURSE_CONT;
				}
				else if (sParam.CompareNoCase (TEXT ("yes")) == 0)
				{
					nRecursionType	=	RECURSE_CONT;
				}
				else if (sParam.CompareNoCase (TEXT ("obj")) == 0)
				{
					nRecursionType	=	RECURSE_OBJ;
				}
				else if (sParam.CompareNoCase (TEXT ("cont_obj")) == 0)
				{
					nRecursionType	=	RECURSE_CONT_OBJ;
				}
				else
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid recursion type specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				if (oSetACL->SetRecursion (nRecursionType) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR (internal) while processing command line: recursion type could not be set!\n"));
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-rst")) == 0)
			{
				if (sParam.CompareNoCase (TEXT ("dacl")) == 0)
				{
					fDACLReset		=	true;
				}
				else if (sParam.CompareNoCase (TEXT ("sacl")) == 0)
				{
					fSACLReset		=	true;
				}
				else if (sParam.CompareNoCase (TEXT ("dacl,sacl")) == 0 || sParam.CompareNoCase (TEXT ("sacl,dacl")) == 0)
				{
					fDACLReset		=	true;
					fSACLReset		=	true;
				}
				else
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -rst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Now set the object flags
				if (oSetACL->SetObjectFlags (nDACLProtected, nSACLProtected, fDACLReset, fSACLReset) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Backup/Restore file: ") + sParam + TEXT (" could not be set!\n"));
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-bckp")) == 0)
			{
				if (oSetACL->SetBackupRestoreFile (sParam) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Backup/Restore file: ") + sParam + TEXT (" could not be set!\n"));
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-log")) == 0)
			{
				oSetACL->SetLogFile (sParam);
			}
			else if (sArg.CompareNoCase (TEXT ("-fltr")) == 0)
			{
				oSetACL->AddObjectFilter (sParam);
			}
			else if (sArg.CompareNoCase (TEXT ("-clr")) == 0)
			{
				if (sParam.CompareNoCase (TEXT ("dacl")) == 0)
				{
					fClearDACL		=	true;

					if (fActionClear)
					{
						if (oSetACL->AddAction (ACTN_CLEARDACL) != RTN_OK)
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: CLEARDACL could not be set!\n"));
							}

							return RTN_ERR_PARAMS;
						}
					}
				}
				else if (sParam.CompareNoCase (TEXT ("sacl")) == 0)
				{
					fClearSACL		=	true;

					if (fActionClear)
					{
						if (oSetACL->AddAction (ACTN_CLEARSACL) != RTN_OK)
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: CLEARSACL could not be set!\n"));
							}

							return RTN_ERR_PARAMS;
						}
					}
				}
				else if (sParam.CompareNoCase (TEXT ("dacl,sacl")) == 0 || sParam.CompareNoCase (TEXT ("sacl,dacl")) == 0)
				{
					fClearDACL		=	true;
					fClearSACL		=	true;

					if (fActionClear)
					{
						if (oSetACL->AddAction (ACTN_CLEARDACL) != RTN_OK)
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: CLEARDACL could not be set!\n"));
							}

							return RTN_ERR_PARAMS;
						}

						if (oSetACL->AddAction (ACTN_CLEARSACL) != RTN_OK)
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR (internal) while processing command line: Action: CLEARSACL could not be set!\n"));
							}

							return RTN_ERR_PARAMS;
						}
					}
				}
				else
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -clr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-ace")) == 0)
			{
				CStringArray	asElements;
				CString			sTrusteeName;
				CString			sPermission;
				BOOL				fSID				=	false;
				DWORD				nInheritance	=	0;
				DWORD				nAccessMode		=	SET_ACCESS;
				DWORD				nACLType			=	ACL_DACL;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Check the number of entries in the parameter
				if (asElements.GetSize () < 2 || asElements.GetSize () > 6)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid number of entries in parameter for option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("n")) == 0)
					{
						sTrusteeName	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("p")) == 0)
					{
						sPermission	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("s")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							fSID			=	true;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							fSID			=	false;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid SID entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("i")) == 0)
					{
						CStringArray	asInheritance;

						// Split the inheritance list
						if (! Split (TEXT (","), asEntry[1], &asInheritance))
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid inheritance entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}

						// Loop through the inheritance list
						for (int k = 0; k < asInheritance.GetSize (); k++)
						{
							if (asInheritance[k].CompareNoCase (TEXT ("so")) == 0)
							{
								nInheritance	|=	SUB_OBJECTS_ONLY_INHERIT;
							}
							else if (asInheritance[k].CompareNoCase (TEXT ("sc")) == 0)
							{
								nInheritance	|=	SUB_CONTAINERS_ONLY_INHERIT;
							}
							else if (asInheritance[k].CompareNoCase (TEXT ("np")) == 0)
							{
								nInheritance	|=	INHERIT_NO_PROPAGATE;
							}
							else if (asInheritance[k].CompareNoCase (TEXT ("io")) == 0)
							{
								nInheritance	|=	INHERIT_ONLY;
							}
							else
							{
								if (funcNotify)
								{
									(*funcNotify) (TEXT ("ERROR in command line: Invalid inheritance entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
								}

								return RTN_ERR_PARAMS;
							}
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("m")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("set")) == 0)
						{
							nAccessMode			=	SET_ACCESS;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("grant")) == 0)
						{
							nAccessMode			=	GRANT_ACCESS;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("deny")) == 0)
						{
							nAccessMode			=	DENY_ACCESS;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("revoke")) == 0)
						{
							nAccessMode			=	REVOKE_ACCESS;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("aud_succ")) == 0)
						{
							nAccessMode			=	SET_AUDIT_SUCCESS;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("aud_fail")) == 0)
						{
							nAccessMode			=	SET_AUDIT_FAILURE;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("aud_fail,aud_succ")) == 0 || asEntry[1].CompareNoCase (TEXT ("aud_succ,aud_fail")) == 0)
						{
							nAccessMode			=	SET_AUDIT_FAILURE + SET_AUDIT_SUCCESS;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid access mode entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("w")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("dacl")) == 0)
						{
							nACLType			=	ACL_DACL;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("sacl")) == 0)
						{
							nACLType			=	ACL_SACL;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid ACL type (where) entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now add the ace
				if (oSetACL->AddACE (sTrusteeName, fSID, sPermission, nInheritance, nInheritance != 0, nAccessMode, nACLType) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR while processing command line: ACE: ") + sParam + TEXT (" could not be set!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-trst")) == 0)
			{
				CStringArray	asElements;
				CString			sTrusteeName1;
				CString			sTrusteeName2;
				BOOL				fSID1					=	false;
				BOOL				fSID2					=	false;
				BOOL				fDACL					=	false;
				BOOL				fSACL					=	false;
				DWORD				nTrusteeAction		=	0;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Check the number of entries in the parameter
				if (asElements.GetSize () < 3 || asElements.GetSize () > 6)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid number of entries in parameter for option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("n1")) == 0)
					{
						sTrusteeName1	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("n2")) == 0)
					{
						sTrusteeName2	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("s1")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							fSID1			=	true;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							fSID1			=	false;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid SID entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("s2")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							fSID2			=	true;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							fSID2			=	false;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid SID entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("ta")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("remtrst")) == 0)
						{
							nTrusteeAction		=	ACTN_REMOVETRUSTEE;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("repltrst")) == 0)
						{
							nTrusteeAction		=	ACTN_REPLACETRUSTEE;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("cpytrst")) == 0)
						{
							nTrusteeAction		=	ACTN_COPYTRUSTEE;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid trustee action entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("w")) == 0)
					{
						CStringArray	asACLType;

						// Split the ACL type list
						if (! Split (TEXT (","), asEntry[1], &asACLType))
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid ACL type (where) entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}

						// Loop through the ACL type list
						for (int k = 0; k < asACLType.GetSize (); k++)
						{
							if (asACLType[k].CompareNoCase (TEXT ("dacl")) == 0)
							{
								fDACL				=	true;
							}
							else if (asACLType[k].CompareNoCase (TEXT ("sacl")) == 0)
							{
								fSACL				=	true;
							}
							else
							{
								if (funcNotify)
								{
									(*funcNotify) (TEXT ("ERROR in command line: Invalid ACL type (where) entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
								}

								return RTN_ERR_PARAMS;
							}
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -trst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now add the trustee
				if (oSetACL->AddTrustee (sTrusteeName1, sTrusteeName2, fSID1, fSID2, nTrusteeAction, fDACL, fSACL) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR while processing command line: Trustee: ") + sParam + TEXT (" could not be set!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-dom")) == 0)
			{
				CStringArray	asElements;
				CString			sDomainName1;
				CString			sDomainName2;
				BOOL				fDACL					=	false;
				BOOL				fSACL					=	false;
				DWORD				nDomainAction		=	0;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Check the number of entries in the parameter
				if (asElements.GetSize () < 3 || asElements.GetSize () > 4)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid number of entries in parameter for option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("n1")) == 0)
					{
						sDomainName1	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("n2")) == 0)
					{
						sDomainName2	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("da")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("remdom")) == 0)
						{
							nDomainAction		=	ACTN_REMOVEDOMAIN;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("repldom")) == 0)
						{
							nDomainAction		=	ACTN_REPLACEDOMAIN;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("cpydom")) == 0)
						{
							nDomainAction		=	ACTN_COPYDOMAIN;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid domain action entry ") + asElements[j] + TEXT (" in a parameter option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("w")) == 0)
					{
						CStringArray	asACLType;

						// Split the ACL type list
						if (! Split (TEXT (","), asEntry[1], &asACLType))
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid ACL type (where) entry ") + asElements[j] + TEXT (" in a parameter option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}

						// Loop through the ACL type list
						for (int k = 0; k < asACLType.GetSize (); k++)
						{
							if (asACLType[k].CompareNoCase (TEXT ("dacl")) == 0)
							{
								fDACL				=	true;
							}
							else if (asACLType[k].CompareNoCase (TEXT ("sacl")) == 0)
							{
								fSACL				=	true;
							}
							else
							{
								if (funcNotify)
								{
									(*funcNotify) (TEXT ("ERROR in command line: Invalid ACL type (where) entry ") + asElements[j] + TEXT (" in a parameter option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
								}

								return RTN_ERR_PARAMS;
							}
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -dom specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now add the domain
				if (oSetACL->AddDomain (sDomainName1, sDomainName2, nDomainAction, fDACL, fSACL) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR while processing command line: Domain: ") + sParam + TEXT (" could not be set!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-ownr")) == 0)
			{
				CStringArray	asElements;
				CString			sTrusteeName;
				BOOL				fSID					=	false;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -ownr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Check the number of entries in the parameter
				if (asElements.GetSize () < 1 || asElements.GetSize () > 2)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid number of entries in parameter for option -ownr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -ownr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -ownr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("n")) == 0)
					{
						sTrusteeName	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("s")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							fSID			=	true;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							fSID			=	false;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid SID entry ") + asElements[j] + TEXT (" in a parameter option -ownr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -ownr specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now set the owner
				if (oSetACL->SetOwner (sTrusteeName, fSID) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR while processing command line: Owner: ") + sParam + TEXT (" could not be set!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-grp")) == 0)
			{
				CStringArray	asElements;
				CString			sTrusteeName;
				BOOL				fSID					=	false;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -grp specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Check the number of entries in the parameter
				if (asElements.GetSize () < 1 || asElements.GetSize () > 2)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid number of entries in parameter for option -grp specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -grp specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -grp specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("n")) == 0)
					{
						sTrusteeName	=	asEntry[1];
					}
					else if (asEntry[0].CompareNoCase (TEXT ("s")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							fSID			=	true;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							fSID			=	false;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid SID entry ") + asElements[j] + TEXT (" in a parameter option -grp specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -grp specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now set the primary group
				if (oSetACL->SetPrimaryGroup (sTrusteeName, fSID) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR while processing command line: primary group: ") + sParam + TEXT (" could not be set!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-op")) == 0)
			{
				CStringArray	asElements;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Check the number of entries in the parameter
				if (asElements.GetSize () < 1 || asElements.GetSize () > 2)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid number of entries in parameter for option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("dacl")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("nc")) == 0)
						{
							nDACLProtected	=	INHPARNOCHANGE;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("np")) == 0)
						{
							nDACLProtected	=	INHPARYES;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("p_c")) == 0)
						{
							nDACLProtected	=	INHPARCOPY;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("p_nc")) == 0)
						{
							nDACLProtected	=	INHPARNOCOPY;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid protection entry ") + asElements[j] + TEXT (" in a parameter option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("sacl")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("nc")) == 0)
						{
							nSACLProtected	=	INHPARNOCHANGE;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("np")) == 0)
						{
							nSACLProtected	=	INHPARYES;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("p_c")) == 0)
						{
							nSACLProtected	=	INHPARCOPY;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("p_nc")) == 0)
						{
							nSACLProtected	=	INHPARNOCOPY;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid protection entry ") + asElements[j] + TEXT (" in a parameter option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -op specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now set the object flags
				if (oSetACL->SetObjectFlags (nDACLProtected, nSACLProtected, fDACLReset, fSACLReset) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR (internal) while processing command line: object flags: ") + sParam + TEXT (" could not be set!\n"));
					}

					return RTN_ERR_PARAMS;
				}
			}
			else if (sArg.CompareNoCase (TEXT ("-lst")) == 0)
			{
				CStringArray	asElements;
				DWORD				nListFormat		=	LIST_CSV;
				DWORD				nListWhat		=	ACL_DACL;
				DWORD				nListNameSID	=	LIST_NAME;
				BOOL				fListInherited	=	false;

				// Split the parameter into its components
				if (! Split (TEXT (";"), sParam, &asElements))
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR in command line: Invalid parameter for option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
					}

					return RTN_ERR_PARAMS;
				}

				// Loop through the entries in the parameter
				for (int j = 0; j < asElements.GetSize (); j++)
				{
					CStringArray	asEntry;

					// Split the entry into value and data
					if (! Split (TEXT (":"), asElements[j], &asEntry))
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry.GetSize () != 2)
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}

					if (asEntry[0].CompareNoCase (TEXT ("f")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("sddl")) == 0)
						{
							nListFormat		=	LIST_SDDL;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("csv")) == 0 || asEntry[1].CompareNoCase (TEXT ("own")) == 0)
						{
							nListFormat		=	LIST_CSV;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("tab")) == 0)
						{
							nListFormat		=	LIST_TAB;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid list format entry ") + asElements[j] + TEXT (" in a parameter option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("i")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							fListInherited	=	true;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							fListInherited	=	false;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid list format entry ") + asElements[j] + TEXT (" in a parameter option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("s")) == 0)
					{
						if (asEntry[1].CompareNoCase (TEXT ("y")) == 0)
						{
							nListNameSID	=	LIST_SID;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("n")) == 0)
						{
							nListNameSID	=	LIST_NAME;
						}
						else if (asEntry[1].CompareNoCase (TEXT ("b")) == 0)
						{
							nListNameSID	=	LIST_NAME_SID;
						}
						else
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid list format entry ") + asElements[j] + TEXT (" in a parameter option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}
					}
					else if (asEntry[0].CompareNoCase (TEXT ("w")) == 0)
					{
						CStringArray	asListWhat;

						// Split the list
						if (! Split (TEXT (","), asEntry[1], &asListWhat))
						{
							if (funcNotify)
							{
								(*funcNotify) (TEXT ("ERROR in command line: Invalid list what entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
							}

							return RTN_ERR_PARAMS;
						}

						// Loop through the list
						for (int k = 0; k < asListWhat.GetSize (); k++)
						{
							if (asListWhat[k].CompareNoCase (TEXT ("d")) == 0)
							{
								nListWhat		|=	ACL_DACL;
							}
							else if (asListWhat[k].CompareNoCase (TEXT ("s")) == 0)
							{
								nListWhat		|=	ACL_SACL;
							}
							else if (asListWhat[k].CompareNoCase (TEXT ("o")) == 0)
							{
								nListWhat		|=	SD_OWNER;
							}
							else if (asListWhat[k].CompareNoCase (TEXT ("g")) == 0)
							{
								nListWhat		|=	SD_GROUP;
							}
							else
							{
								if (funcNotify)
								{
									(*funcNotify) (TEXT ("ERROR in command line: Invalid list what entry ") + asElements[j] + TEXT (" in a parameter option -ace specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
								}

								return RTN_ERR_PARAMS;
							}
						}
					}
					else
					{
						if (funcNotify)
						{
							(*funcNotify) (TEXT ("ERROR in command line: Invalid entry ") + asElements[j] + TEXT (" in a parameter option -lst specified: ") + sParam + TEXT ("!\n") + TXTPRINTHELP);
						}

						return RTN_ERR_PARAMS;
					}
				}

				// Now set the list options
				if (oSetACL->SetListOptions (nListFormat, nListWhat, fListInherited, nListNameSID) != RTN_OK)
				{
					if (funcNotify)
					{
						(*funcNotify) (TEXT ("ERROR (internal) while processing command line: list options: ") + sParam + TEXT (" could not be set!\n"));
					}

					return RTN_ERR_PARAMS;
				}
			}
			else
			{
				if (funcNotify)
				{
					(*funcNotify) (TEXT ("ERROR in command line: Invalid option specified: ") + sArg + TEXT ("!\n") + TXTPRINTHELP);
				}

				return RTN_ERR_PARAMS;
			}
		}
	}	//	for (int i = 1; i < argc; i++)

	return nRetCode;
}


//
// PrintMsg: Called by various CSetACL functions. Prints status text to stdout and/or log file
//
void PrintMsg (CString sMessage)
{
	if (! fSilent)
	{
		// Print to the screen
		_tprintf (TEXT ("%s\n"), sMessage.GetString ());
	}
}


//
// PrintHelp: Print help text
//
void PrintHelp ()
{
	printf ("SetACL by Helge Klein\n");
	printf ("\n");
	printf ("Homepage:        http://setacl.sourceforge.net\n");
	printf ("Version:         2.0.3.0\n");
	printf ("Copyright:       Helge Klein\n");
	printf ("License:         GPL\n");
	printf ("\n");
	printf ("-O-P-T-I-O-N-S--------------------------------------------------------\n");
	printf ("\n");
	printf ("-on    ObjectName\n");
	printf ("\n");
	printf ("-ot    ObjectType\n");
	printf ("\n");
	printf ("-actn  Action\n");
	printf ("\n");
	printf ("-ace   \"n:Trustee;p:Permission;s:IsSID;i:Inheritance;m:Mode;w:Where\"\n");
	printf ("\n");
	printf ("-trst  \"n1:Trustee;n2:Trustee;s1:IsSID;s2:IsSID;ta:TrusteeAction;w:Where\"\n");
	printf ("\n");
	printf ("-dom   \"n1:Domain;n2:Domain;da:DomainAction;w:Where\"\n");
	printf ("\n");
	printf ("-ownr  \"n:Trustee;s:IsSID\"\n");
	printf ("\n");
	printf ("-grp   \"n:Trustee;s:IsSID\"\n");
	printf ("\n");
	printf ("-rec   Recursion\n");
	printf ("\n");
	printf ("-op    \"dacl:Protection;sacl:Protection\"\n");
	printf ("\n");
	printf ("-rst   Where\n");
	printf ("\n");
	printf ("-lst   \"f:Format;w:What;i:ListInherited;s:DisplaySID\"\n");
	printf ("\n");
	printf ("-bckp  Filename\n");
	printf ("\n");
	printf ("-log   Filename\n");
	printf ("\n");
	printf ("-fltr  Keyword\n");
	printf ("\n");
	printf ("-clr   Where\n");
	printf ("\n");
	printf ("-silent\n");
	printf ("\n");
	printf ("-ignoreerr\n");
	printf ("\n");
	printf ("-P-A-R-A-M-E-T-E-R-S-------------------------------------------------\n");
	printf ("\n");
	printf ("ObjectName:      Name of the object to process (e.g. 'c:\\mydir')\n");
	printf ("\n");
	printf ("ObjectType:      Type of object:\n");
	printf ("\n");
	printf ("                 file:       Directory/file\n");
	printf ("                 reg:        Registry key\n");
	printf ("                 srv:        Service\n");
	printf ("                 prn:        Printer\n");
	printf ("                 shr:        Network share\n");
	printf ("\n");
	printf ("Action:          Action(s) to perform:\n");
	printf ("\n");
	printf ("                 ace:        Process ACEs specified by parameter(s) '-ace'\n");
	printf ("                 trustee:    Process trustee(s) specified by parameter(s)\n");
	printf ("                             '-trst'.\n");
	printf ("                 domain:     Process domain(s) specified by parameter(s)\n");
	printf ("                             '-dom'.\n");
	printf ("                 list:       List permissions. A backup file can be\n");
	printf ("                             specified by parameter '-bckp'. Controlled by\n");
	printf ("                             parameter '-lst'.\n");
	printf ("                 restore:    Restore entire security descriptors backed up\n");
	printf ("                             using the list function. A file containing the\n");
	printf ("                             backup has to be specified using the parameter\n");
	printf ("                             '-bckp'. The listing has to be in SDDL format.\n");
	printf ("                 setowner:   Set the owner to trustee specified by parameter\n");
	printf ("                             '-ownr'.\n");
	printf ("                 setgroup:   Set the primary group to trustee specified by\n");
	printf ("                             parameter '-grp'.\n");
	printf ("                 clear:      Clear the ACL of any non-inherited ACEs. The\n");
	printf ("                             parameter '-clr' controls whether to do this for\n");
	printf ("                             the DACL, the SACL, or both.\n");
	printf ("                 setprot:    Set the flag 'allow inheritable permissions from\n");
	printf ("                             the parent object to propagate to this object' to\n");
	printf ("                             the value specified by parameter '-op'.\n");
	printf ("                 rstchldrn:  Reset permissions on all sub-objects and enable\n");
	printf ("                             propagation of inherited permissions. The\n");
	printf ("                             parameter '-rst' controls whether to do this for\n");
	printf ("                             the DACL, the SACL, or both.\n");
	printf ("\n");
	printf ("TrusteeAction:   Action to perform on trustee specified:\n");
	printf ("\n");
	printf ("                 remtrst:    Remove all ACEs belonging to trustee specified.\n");
	printf ("                 repltrst:   Replace trustee 'n1' by 'n2' in all ACEs.\n");
	printf ("                 cpytrst:    Copy the permissions for trustee 'n1' to 'n2'.\n");
	printf ("\n");
	printf ("DomainAction:    Action to perform on domain specified:\n");
	printf ("\n");
	printf ("                 remdom:     Remove all ACEs belonging to trustees of domain\n");
	printf ("                             specified.\n");
	printf ("                 repldom:    Replace trustees from domain 'n1' by trustees with\n");
	printf ("                             same name from domain 'n2' in all ACEs.\n");
	printf ("                 cpydom:     Copy permissions from trustees from domain 'n1' to\n");
	printf ("                             trustees with same name from domain 'n2' in all\n");
	printf ("                             ACEs.\n");
	printf ("\n");
	printf ("Trustee:         Name or SID of trustee (user or group). Format:\n");
	printf ("                 \n");
	printf ("                 a) [(computer | domain)\\]name\n");
	printf ("                 \n");
	printf ("                 Where:\n");
	printf ("                 \n");
	printf ("                 computer:   DNS or NetBIOS name of a computer -> 'name' must\n");
	printf ("                             be a local account on that computer.\n");
	printf ("                 domain:     DNS or NetBIOS name of a domain -> 'name' must\n");
	printf ("                             be a domain user or group.\n");
	printf ("                 name:       user or group name\n");
	printf ("                 \n");
	printf ("                 If no computer or domain name is given, SetACL tries to find\n");
	printf ("                 a SID for 'name' in the following order:\n");
	printf ("                 \n");
	printf ("                 1. built-in accounts and well-known SIDs\n");
	printf ("                 2. local accounts\n");
	printf ("                 3. primary domain\n");
	printf ("                 4. trusted domains\n");
	printf ("                 \n");
	printf ("                 b) SID string\n");
	printf ("\n");
	printf ("Domain:          Name of a domain (NetBIOS or DNS name).\n");
	printf ("\n");
	printf ("Permission:      Permission to set. Validity of permissions depends on the\n");
	printf ("                 object type (see below). Comma separated list.\n");
	printf ("\n");
	printf ("                 Example:    'read,write_ea,write_dacl'\n");
	printf ("\n");
	printf ("IsSID:           Is the trustee name a SID?\n");
	printf ("\n");
	printf ("                 y:          Yes\n");
	printf ("                 n:          No\n");
	printf ("\n");
	printf ("DisplaySID:      Display trustee names as SIDs?\n");
	printf ("\n");
	printf ("                 y:          Yes\n");
	printf ("                 n:          No\n");
	printf ("                 b:          Both (names and SIDs)\n");
	printf ("\n");
	printf ("Inheritance:     Inheritance flags for the ACE. This may be a comma separated\n");
	printf ("                 list containing the following:\n");
	printf ("\n");
	printf ("                 so:         sub-objects\n");
	printf ("                 sc:         sub-containers\n");
	printf ("                 np:         no propagation\n");
	printf ("                 io:         inherit only\n");
	printf ("                 \n");
	printf ("                 Example:    'io,so'\n");
	printf ("\n");
	printf ("Mode:            Access mode of this ACE:\n");
	printf ("\n");
	printf ("                 a) DACL:\n");
	printf ("\n");
	printf ("                 set:        Replace all permissions for given trustee by\n");
	printf ("                             those specified.\n");
	printf ("                 grant:      Add permissions specified to existing permissions\n");
	printf ("                             for given trustee.\n");
	printf ("                 deny:       Deny permissions specified.\n");
	printf ("                 revoke:     Remove permissions specified from existing\n");
	printf ("                             permissions for given trustee.\n");
	printf ("\n");
	printf ("                 b) SACL:\n");
	printf ("\n");
	printf ("                 aud_succ:   Add an audit success ACE.\n");
	printf ("                 aud_fail:   Add an audit failure ACE.\n");
	printf ("                 revoke:     Remove permissions specified from existing\n");
	printf ("                             permissions for given trustee.\n");
	printf ("\n");
	printf ("Where:           Apply settings to DACL, SACL, or both (comma separated list):\n");
	printf ("\n");
	printf ("                 dacl\n");
	printf ("                 sacl\n");
	printf ("                 dacl,sacl\n");
	printf ("\n");
	printf ("Recursion:       Recursion settings, depends on object type:\n");
	printf ("\n");
	printf ("                 a) file:\n");
	printf ("                 \n");
	printf ("                 no:         No recursion.\n");
	printf ("                 cont:       Recurse, and process directories only.\n");
	printf ("                 obj:        Recurse, and process files only.\n");
	printf ("                 cont_obj:   Recurse, and process directories and files.\n");
	printf ("                 \n");
	printf ("                 b) reg:\n");
	printf ("                 \n");
	printf ("                 no:         Do not recurse.\n");
	printf ("                 yes:        Do Recurse.\n");
	printf ("\n");
	printf ("Protection:      Controls the flag 'allow inheritable permissions from the\n");
	printf ("                 parent object to propagate to this object':\n");
	printf ("\n");
	printf ("                 nc:         Do not change the current setting.\n");
	printf ("                 np:         Object is not protected, i.e. inherits from\n");
	printf ("                             parent.\n");
	printf ("                 p_c:        Object is protected, ACEs from parent are\n");
	printf ("                             copied.\n");
	printf ("                 p_nc:       Object is protected, ACEs from parent are not\n");
	printf ("                             copied.\n");
	printf ("\n");
	printf ("Format:          Which list format to use:\n");
	printf ("\n");
	printf ("                 sddl:       Standardized SDDL format. Only listings in this\n");
	printf ("                             format can be restored.\n");
	printf ("                 csv:        SetACL's csv format.\n");
	printf ("                 tab:        SetACL's tabular format.\n");
	printf ("\n");
	printf ("What:            Which components of security descriptors to include in the\n");
	printf ("                 listing. (comma separated list):\n");
	printf ("\n");
	printf ("                 d:          DACL\n");
	printf ("                 s:          SACL\n");
	printf ("                 o:          Owner\n");
	printf ("                 g:          Primary group\n");
	printf ("                 \n");
	printf ("                 Example:    'd,s'\n");
	printf ("\n");
	printf ("ListInherited:   List inherited permissions?\n");
	printf ("\n");
	printf ("                 y:          Yes\n");
	printf ("                 n:          No\n");
	printf ("\n");
	printf ("Filename:        Name of a (unicode) file used for list/backup/restore\n");
	printf ("                 operations or logging.\n");
	printf ("\n");
	printf ("Keyword:         Keyword to filter object names by. Names containing this\n");
	printf ("                 keyword are not processed.\n");
	printf ("\n");
	printf ("-R-E-M-A-R-K-S--------------------------------------------------------\n");
	printf ("\n");
	printf ("Required parameters (all others are optional):\n");
	printf ("\n");
	printf ("                 -on         (Object name)\n");
	printf ("                 -ot         (Object type)\n");
	printf ("\n");
	printf ("Parameters that may be specified more than once:\n");
	printf ("\n");
	printf ("                 -actn       (Action)\n");
	printf ("                 -ace        (Access control entry)\n");
	printf ("                 -trst       (Trustee)\n");
	printf ("                 -dom        (Domain)\n");
	printf ("                 -fltr       (Filter keyword)\n");
	printf ("\n");
	printf ("Only actions specified by parameter(s) '-actn' are actually performed,\n");
	printf ("regardless of the other options set.\n");
	printf ("\n");
	printf ("Order in which multiple actions are processed:\n");
	printf ("\n");
	printf ("                 1.          restore\n");
	printf ("                 2.          clear\n");
	printf ("                 3.          trustee\n");
	printf ("                 4.          domain\n");
	printf ("                 5.          ace, setowner, setgroup, setprot\n");
	printf ("                 6.          rstchldrn\n");
	printf ("                 7.          list\n");
	printf ("\n");
	printf ("-V-A-L-I-D--P-E-R-M-I-S-S-I-O-N-S-------------------------------------\n");
	printf ("\n");
	printf ("a) Standard permission sets (combinations of specific permissions)\n");
	printf ("\n");
	printf ("Files / Directories:\n");
	printf ("\n");
	printf ("              read:          Read\n");
	printf ("              write:         Write\n");
	printf ("              list_folder:   List folder\n");
	printf ("              read_ex:       Read, execute\n");
	printf ("              change:        Change\n");
	printf ("              profile:       = change + write_dacl\n");
	printf ("              full:          Full access\n");
	printf ("\n");
	printf ("Printers:\n");
	printf ("\n");
	printf ("              print:         Print\n");
	printf ("              man_printer:   Manage printer\n");
	printf ("              man_docs:      Manage documents\n");
	printf ("              full:          Full access\n");
	printf ("\n");
	printf ("Registry:\n");
	printf ("\n");
	printf ("              read:          Read\n");
	printf ("              full:          Full access\n");
	printf ("\n");
	printf ("Service:\n");
	printf ("\n");
	printf ("              read:          Read\n");
	printf ("              start_stop:    Start / Stop\n");
	printf ("              full:          Full access\n");
	printf ("\n");
	printf ("Share:\n");
	printf ("\n");
	printf ("              read:          Read\n");
	printf ("              change:        Change\n");
	printf ("              full:          Full access\n");
	printf ("\n");
	printf ("b) Specific permissions\n");
	printf ("\n");
	printf ("Files / Directories:\n");
	printf ("\n");
	printf ("              traverse:      Traverse folder / execute file\n");
	printf ("              list_dir:      List folder / read data\n");
	printf ("              read_attr:     Read attributes\n");
	printf ("              read_ea:       Read extended attributes\n");
	printf ("              add_file:      Create files / write data\n");
	printf ("              add_subdir:    Create folders / append data\n");
	printf ("              write_attr:    Write attributes\n");
	printf ("              write_ea:      Write extended attributes\n");
	printf ("              del_child:     Delete subfolders and files\n");
	printf ("              delete:        Delete\n");
	printf ("              read_dacl:     Read permissions\n");
	printf ("              write_dacl:    Write permissions\n");
	printf ("              write_owner:   Take ownership\n");
	printf ("\n");
	printf ("Registry:\n");
	printf ("\n");
	printf ("              query_val:     Query value\n");
	printf ("              set_val:       Set value\n");
	printf ("              create_subkey: Create subkeys\n");
	printf ("              enum_subkeys:  Enumerate subkeys\n");
	printf ("              notify:        Notify\n");
	printf ("              create_link:   Create link\n");
	printf ("              delete:        Delete\n");
	printf ("              write_dacl:    Write permissions\n");
	printf ("              write_owner:   Take ownership\n");
	printf ("              read_access:   Read control\n");
	printf ("\n");
}