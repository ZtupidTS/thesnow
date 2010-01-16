/////////////////////////////////////////////////////////////////////////////
//
//
//	CSetACL.h
//
//
//	Description:	Main include file needed by all SetACL .cpp/.h files
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
// Definitions that must come before anything else, normally in stdafx.h
//
//////////////////////////////////////////////////////////////////////


// Windows >= XP needed
#define	_WIN32_WINNT				0x05010000


// Do not include rarely used parts of the Windows headers
#define VC_EXTRALEAN


// We ONLY use unicode
#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

//////////////////////////////////////////////////////////////////////
//
// Includes
//
//////////////////////////////////////////////////////////////////////


// Includes normally in stdafx.h
#ifdef I_AM_AN_OCX
	#include <afxctl.h>
	#include <afxext.h>
#else
	#include <afxwin.h>
#endif

// Always needed for certain "advanced" MFC classes
#include <afxtempl.h>

// More includes
#include <windows.h>
#include <iostream>
#include <aclapi.h>
#include <lmerr.h>
#include <winspool.h>
#include <winsvc.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <sddl.h>
#include <dsgetdc.h>
#include <dsrole.h>

#include <new>

//////////////////////////////////////////////////////////////////////
//
// Defines
//
//////////////////////////////////////////////////////////////////////


// ACL/SD information types
#define ACL_DACL								1			// The ACL is a DACL (permission information)
#define ACL_SACL								2			// The ACL is a SACL (auditing information)
#define SD_OWNER								4			//	Owner information
#define SD_GROUP								8			// Primary group information


// Actions the program can perform
#define ACTN_ADDACE							1			// Add an ACE
#define ACTN_LIST								2			// List the entries in the security descriptor
#define ACTN_SETOWNER						4			// Set the owner
#define ACTN_SETGROUP						8			// Set the primary group
#define ACTN_CLEARDACL						16			// Clear the DACL of any non-inherited ACEs
#define ACTN_CLEARSACL						32			// Clear the SACL of any non-inherited ACEs
#define ACTN_SETINHFROMPAR					64			// Set the flag 'allow inheritable permissions from the parent object to propagate to this object'
#define ACTN_RESETCHILDPERMS				128		// Reset permissions on all sub-objects and enable propagation of inherited permissions
#define ACTN_REPLACETRUSTEE				256		// Replace one trustee by another in all ACEs
#define ACTN_REMOVETRUSTEE					512		// Remove all ACEs belonging to a certain trustee
#define ACTN_COPYTRUSTEE					1024		//	Copy the permissions for one trustee to another
#define ACTN_REPLACEDOMAIN					256		// Replace one domain by another in all ACEs
#define ACTN_REMOVEDOMAIN					512		// Remove all ACEs belonging to a certain domain
#define ACTN_COPYDOMAIN						1024		//	Copy the permissions for one domain to another
#define ACTN_RESTORE							2048		//	Restore entire security descriptors backup up with the list function
#define ACTN_TRUSTEE							4096		//	Process all trustee actions
#define ACTN_DOMAIN							8192		//	Process all domain actions

// Return/Error codes
#define RTN_OK									0			// OK
#define RTN_USAGE								1			// Usage instructions were printed
#define RTN_ERR_GENERAL						2			// General error
#define RTN_ERR_PARAMS						3			// Parameter(s) incorrect
#define RTN_ERR_OBJECT_NOT_SET			4			// The object was not set
#define RTN_ERR_GETSECINFO					5			// The call to GetNamedSecurityInfo () failed
#define RTN_ERR_LOOKUP_SID					6			// The SID for a trustee could not be found
#define RTN_ERR_INV_DIR_PERMS				7			// Directory permissions specified are invalid
#define RTN_ERR_INV_PRN_PERMS				8			// Printer permissions specified are invalid
#define RTN_ERR_INV_REG_PERMS				9			// Registry permissions specified are invalid
#define RTN_ERR_INV_SVC_PERMS				10			// Service permissions specified are invalid
#define RTN_ERR_INV_SHR_PERMS				11			// Share permissions specified are invalid
#define RTN_ERR_EN_PRIV						12			// A privilege could not be enabled
#define RTN_ERR_DIS_PRIV					13			// A privilege could not be disabled
#define RTN_ERR_NO_NOTIFY					14			// No notification function was given
#define RTN_ERR_LIST_FAIL					15			// An error occured in the list function
#define RTN_ERR_FINDFILE					16			// FindFile reported an error
#define RTN_ERR_GET_SD_CONTROL			17			//	The call to GetSecurityDescriptorControl () failed
#define RTN_ERR_INTERNAL					18			// An internal program error occured
#define RTN_ERR_SETENTRIESINACL			19			// SetEntriesInAcl () failed
#define RTN_ERR_REG_PATH					20			//	A registry path is incorrect
#define RTN_ERR_REG_CONNECT				21			//	Connect to a remote registry failed
#define RTN_ERR_REG_OPEN					22			//	Opening a registry key failed
#define RTN_ERR_REG_ENUM					23			//	Enumeration of registry keys failed
#define RTN_ERR_PREPARE						24			//	Preparation failed
#define RTN_ERR_SETSECINFO					25			// The call to SetNamedSecurityInfo () failed
#define RTN_ERR_LIST_OPTIONS				26			// Incorrect list options specified
#define RTN_ERR_CONVERT_SD					27			// A SD could not be converted to/from string format
#define RTN_ERR_LIST_ACL					28			// ACL listing failed
#define RTN_ERR_LOOP_ACL					29			// Looping through an ACL failed
#define RTN_ERR_DEL_ACE						30			// Deleting an ACE failed
#define RTN_ERR_COPY_ACL					31			//	Copying an ACL failed
#define RTN_ERR_ADD_ACE						32			// Adding an ACE failed
#define RTN_ERR_NO_LOGFILE					33			// No backup/restore file was specified
#define RTN_ERR_OPEN_LOGFILE				34			// The backup/restore file could not be opened
#define RTN_ERR_READ_LOGFILE				35			// A read operation from the backup/restore file failed
#define RTN_ERR_WRITE_LOGFILE				36			// A write operation from the backup/restore file failed
#define RTN_ERR_OS_NOT_SUPPORTED			37			// The operating system is not supported
#define RTN_ERR_INVALID_SD					38			//	The security descriptor is invalid
#define RTN_ERR_SET_SD_DACL				39			//	The call to SetSecurityDescriptorDacl () failed
#define RTN_ERR_SET_SD_SACL				40			//	The call to SetSecurityDescriptorSacl () failed
#define RTN_ERR_SET_SD_OWNER				41			//	The call to SetSecurityDescriptorOwner () failed
#define RTN_ERR_SET_SD_GROUP				42			//	The call to SetSecurityDescriptorGroup () failed
#define RTN_ERR_INV_DOMAIN					43			//	The domain specified is invalid
#define RTN_ERR_IGNORED						44			//	An error occured, but it was ignored
#define RTN_ERR_CREATE_SD					45			//	The creation of an SD failed
#define RTN_ERR_OUT_OF_MEMORY				46			//	Memory allocation failed

// For inheritance from the parent
#define INHPARNOCHANGE						0			// Do not change settings
#define INHPARYES								1			// Inherit from parent
#define INHPARCOPY							2			// Do not inherit, copy inheritable permissions
#define INHPARNOCOPY							4			// Do not inherit, do not copy inheritable permissions

// List formats
#define LIST_SDDL								0			// SDDL format
#define LIST_CSV								1			// CSV format
#define LIST_TAB								2			// Tabular format

// How to list trustees
#define LIST_NAME								1			// List names
#define LIST_SID								2			// List SIDs
#define LIST_NAME_SID						3			// List names and SIDs

//
// Access permissions.  These are based on the values set by explorer
//
// Directories / files
#define MY_DIR_READ_ACCESS					(FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_READ_ATTRIBUTES | READ_CONTROL)
#define MY_DIR_WRITE_ACCESS				(FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES)
#define MY_DIR_LIST_FOLDER_ACCESS		(FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_TRAVERSE | FILE_READ_ATTRIBUTES | READ_CONTROL)
#define MY_DIR_READ_EXECUTE_ACCESS		(FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_TRAVERSE | FILE_READ_ATTRIBUTES | READ_CONTROL)
#define MY_DIR_CHANGE_ACCESS				(FILE_LIST_DIRECTORY | FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY | FILE_READ_EA | FILE_WRITE_EA | FILE_TRAVERSE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | READ_CONTROL | DELETE)
#define MY_DIR_FULL_ACCESS					(FILE_LIST_DIRECTORY | FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY | FILE_READ_EA | FILE_WRITE_EA | FILE_TRAVERSE | FILE_DELETE_CHILD | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | READ_CONTROL | WRITE_OWNER | WRITE_DAC | DELETE)

// Printers
#define MY_PRINTER_PRINT_ACCESS			(PRINTER_ACCESS_USE | READ_CONTROL)
#define MY_PRINTER_MAN_PRINTER_ACCESS	(PRINTER_ACCESS_ADMINISTER | PRINTER_ACCESS_USE | READ_CONTROL | WRITE_OWNER | WRITE_DAC | DELETE)
#define MY_PRINTER_MAN_DOCS_ACCESS		(JOB_ACCESS_ADMINISTER | JOB_ACCESS_READ | READ_CONTROL | WRITE_OWNER | WRITE_DAC | DELETE)

// Registry
#define MY_REG_READ_ACCESS					(KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_NOTIFY | KEY_QUERY_VALUE | KEY_READ | READ_CONTROL)
#define MY_REG_FULL_ACCESS					(KEY_CREATE_LINK | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_NOTIFY | KEY_QUERY_VALUE | KEY_READ | KEY_SET_VALUE | KEY_WRITE | READ_CONTROL | WRITE_OWNER | WRITE_DAC | DELETE)

// Services
#define MY_SVC_READ_ACCESS					(SERVICE_ENUMERATE_DEPENDENTS | SERVICE_INTERROGATE | SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_USER_DEFINED_CONTROL | READ_CONTROL)
#define MY_SVC_STARTSTOP_ACCESS			(SERVICE_ENUMERATE_DEPENDENTS | SERVICE_INTERROGATE | SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP | SERVICE_USER_DEFINED_CONTROL | READ_CONTROL)
#define MY_SVC_FULL_ACCESS					(SERVICE_CHANGE_CONFIG | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_INTERROGATE | SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP | SERVICE_USER_DEFINED_CONTROL | READ_CONTROL | WRITE_OWNER | WRITE_DAC | DELETE)

// Shares
// The following three lines are from Check_SD:
#define SHARE_READ							0x000001BF
#define SHARE_CHANGE							0x000000A9
#define SHARE_WRITE							0x00000040
#define MY_SHARE_READ_ACCESS				(SHARE_CHANGE | READ_CONTROL | SYNCHRONIZE)
#define MY_SHARE_CHANGE_ACCESS			(SHARE_READ | DELETE | READ_CONTROL | SYNCHRONIZE)
#define MY_SHARE_FULL_ACCESS				(SHARE_READ | SHARE_CHANGE | SHARE_WRITE | READ_CONTROL | WRITE_OWNER | WRITE_DAC | DELETE | SYNCHRONIZE)

// For recursion
#define RECURSE_NO							1			// No recursion
#define RECURSE_CONT							2			// Recurse containers
#define RECURSE_OBJ							4			// Recurse objects
#define RECURSE_CONT_OBJ					6			// Recurse containers and objects


//////////////////////////////////////////////////////////////////////
//
// Class CTrustee
//
//////////////////////////////////////////////////////////////////////


class CTrustee
{

public:

							CTrustee ();
							CTrustee (CString sTrustee, BOOL fTrusteeIsSID, DWORD nAction, BOOL fDACL, BOOL fSACL);
	virtual				~CTrustee ();
	DWORD					LookupSID ();							// Lookup/calculate the binary SID of a given trustee

	CString				m_sTrustee;								// Name of the trustee
	BOOL					m_fTrusteeIsSID;						// Is the trustee a (string) SID?
	PSID					m_psidTrustee;							// Pointer to the binary SID
	DWORD					m_nAction;								//	Which action(s) are to be performed on this trustee?
	BOOL					m_fDACL;									//	Use this trustee in DACL processing?
	BOOL					m_fSACL;									//	Use this trustee in SACL processing?
	CTrustee*			m_oNewTrustee;							//	Pointer to another trustee this one should be replaced with

};


//////////////////////////////////////////////////////////////////////
//
// Class CDomain
//
//////////////////////////////////////////////////////////////////////


class CDomain
{

public:

							CDomain ();
	virtual				~CDomain ();
	DWORD					SetDomain (CString sDomain, DWORD nAction, BOOL fDACL, BOOL fSACL);

	CString				m_sDomain;								// DNS name of the domain
	DWORD					m_nAction;								//	Which action(s) are to be performed on this domain?
	BOOL					m_fDACL;									//	Use this domain in DACL processing?
	BOOL					m_fSACL;									//	Use this domain in SACL processing?
	CDomain*				m_oNewDomain;							//	Pointer to another domain this one should be replaced with

};


//////////////////////////////////////////////////////////////////////
//
// Class CACE
//
//////////////////////////////////////////////////////////////////////


class CACE
{

public:

							CACE ();
							CACE (CTrustee* pTrustee, CString sPermission, DWORD nInheritance, BOOL fInhSpecified, ACCESS_MODE nAccessMode, DWORD nACLType);
	virtual				~CACE ();

	CTrustee*			m_pTrustee;								// Pointer to instance of CTrustee which specifies the trustee
	CString				m_sPermission;							// Permission to set for this trustee (string value)
	DWORD					m_nInheritance;						// Inheritance flags to use
	BOOL					m_fInhSpecified;						// Inheritance specified by caller, or do we use default (working) values?
	ACCESS_MODE			m_nAccessMode;							// Access mode: set, grant, deny,  revoke, audit_success, audit_failure
	DWORD					m_nACLType;								// DACL or SACL
	DWORD					m_nAccessMask;							// Permission to set for this trustee (bitmask value)

};


//////////////////////////////////////////////////////////////////////
//
// Class CSetACL
//
//////////////////////////////////////////////////////////////////////


class CSetACL  
{

public:

													CSetACL (void (* funcNotify) (CString) = NULL);						// Constructor. The optional callback function will be called for every message generated.
	virtual										~CSetACL ();																	// Destructor

	DWORD											SetObject (CString sObjectPath, SE_OBJECT_TYPE nObjectType);	// Set the object on which all actions are to be performed
	DWORD											SetAction (DWORD nAction);													// Set the action to be performed
	DWORD											AddAction (DWORD nAction);													// Add an action to be performed
	DWORD											AddACE (CString sTrustee, BOOL fTrusteeIsSID, CString sPermission, DWORD nInheritance, BOOL fInhSpecified, DWORD nAccessMode, DWORD nACLType);	// Add an ACE to a DACL or SACL
	DWORD											AddTrustee (CString sTrustee, CString sNewTrustee, BOOL fTrusteeIsSID, BOOL fNewTrusteeIsSID, DWORD nAction, BOOL fDACL, BOOL fSACL);				// Add a trustee: used by the functions to remove/replace (a) specified trustee(s) from the ACL
	DWORD											AddDomain (CString sDomain, CString sNewDomain, DWORD nAction, BOOL fDACL, BOOL fSACL);																			// Add a domain: used by the functions to remove/replace (a) specified domain(s) from the ACL
	DWORD											SetOwner (CString sTrustee, BOOL fTrusteeIsSID);					// Set the owner
	DWORD											SetPrimaryGroup (CString sTrustee, BOOL fTrusteeIsSID);			// Set the primary group
	DWORD											Run ();																			// Do it: apply all settings.
	DWORD											SetRecursion (DWORD nRecursionType);									// Should we recurse?
	DWORD											SetObjectFlags (DWORD nDACLProtected, DWORD nSACLProtected, BOOL fDACLResetChildObjects, BOOL fSACLResetChildObjects);									// Set flags specific to the object
	CString										GetLastErrorMessage (DWORD nError = 0);								// Return the error message generated during the last Run ()
	DWORD											SetListOptions (DWORD nListFormat, DWORD nListWhat, BOOL fListInherited, DWORD nListNameSID);																	// Set the options for ACL listing
	DWORD											SetBackupRestoreFile (CString sBackupRestoreFile);					// Specify a (unicode) file to be used for backup/restore operations
	DWORD											SetLogFile (CString sLogFile);											// Specify a (unicode) file to be used for logging
	void											AddObjectFilter (CString sKeyword);										// Add a keyword to be filtered out - objects containing this keyword are not processed
	DWORD											OpenRegKey (CString* sObjectPath, PHKEY hSubKey);					//	Open a registry key
	DWORD											SetPrivilege (CString sPrivilege, BOOL fEnable);					// Set a privilege (user right)
	BOOL											SetIgnoreErrors (BOOL fIgnoreError);									//	Ignore errors, do NOT stop execution (unknown consequences!)
	DWORD											LogMessage (CString sMessage);											// Log a message string

public:

	void											(*m_funcNotify) (CString);													// Pointer to function that receives messages from us
	CTypedPtrList<CPtrList, CTrustee*>	m_lstTrustees;																	// List that holds trustees to be processed
	CTypedPtrList<CPtrList, CDomain*>	m_lstDomains;																	// List that holds domains to be processed
	CString										m_sTargetSystemName;															// Name of the computer the object path points to (empty for local system)
	DWORD											m_nAPIError;																	// Error code of the last API function called
	BOOL											m_fIgnoreErrors;																// Ignore errors?

private:

	DWORD											Prepare ();																		// Prepare for execution.
	DWORD											DetermineACEAccessMasks ();												// Set the access masks and inheritance values for all ACE
	BOOL											CheckAction (DWORD nAction);												// Check if an action is valid
	BOOL											CheckInheritance (DWORD nInheritance);									// Check if inheritance flags are valid
	BOOL											CheckInhFromParent (DWORD nInheritance);								// Check if inheritance from parent flags are valid
	BOOL											CheckACEAccessMode (DWORD nAccessMode, DWORD nACLType);			// Check if an ACE access mode (deny, set...) is valid
	CACE*											CopyACE (CACE* pACE);														// Copy an existing ACE
	DWORD											DoActionList ();																// Create a permission listing
	DWORD											DoActionRestore ();															// Restore permissions from a file
	DWORD											DoActionWrite ();																// Process actions that write to the SD
	DWORD											Write2SD (CString  sObjectPath);											// Set/Add the ACEs, owner and primary group specified to the ACLs
	DWORD											ListSD (CString sObjectPath);												// List the contents of a SD in text format
	CString										ListACL (PACL paclACL);														// Return the contents of an ACL as a string
	DWORD											RecurseDirs (CString  sObjectPath, DWORD (CSetACL::*funcProcess) (CString sObjectPath));		// Recurse a directory structure and call the function for every file / dir
	DWORD											RecurseRegistry (CString  sObjectPath, DWORD (CSetACL::*funcProcess) (CString sObjectPath));	// Recurse the registry and call the function for every key
	CString										GetPermissions (ACCESS_MASK nAccessMask);								// Return a string with the permissions in an access mask
	CString										GetACEType (BYTE nACEType);												// Return a string with the type of an ACE
	CString										GetACEFlags (BYTE nACEFlags);												// Return a string with the flags of an ACE
	BOOL											CheckFilterList (CString sObjectPath);									// Check whether a certain path needs to be filtered out
	CString										GetTrusteeFromSID (PSID psidSID);										// Convert a binary SID into a trustee name

private:

	CString										m_sObjectPath;							// Path to the object to be processed
	SE_OBJECT_TYPE								m_nObjectType;							// Type of the object to be processed
	DWORD											m_nAction;								// Action to be performed
	DWORD											m_nDACLProtected;						// Set the protected (from propagation from the parent object) flag for the DACL
	DWORD											m_nSACLProtected;						// Set the protected (from propagation from the parent object) flag for the SACL
	BOOL											m_fDACLResetChildObjects;			// Reset permissions on child objects and enable propagation of inheritable permissions for the DACL
	BOOL											m_fSACLResetChildObjects;			// Reset permissions on child objects and enable propagation of inheritable permissions for the SACL
	DWORD											m_nRecursionType;						// Type of recursion to be performed
	CTrustee*									m_pOwner;								// Owner to set
	CTrustee*									m_pPrimaryGroup;						//	Primary group to set
	CTypedPtrList<CPtrList, CACE*>		m_lstACEs;								//	List that holds the ACEs to be processed
	CStringList									m_lstObjectFilter;					//	List that holds object names to be filtered out

	PACL											m_paclExistingDACL;					// Receives a pointer to the existing DACL of the object
	PACL											m_paclExistingSACL;					// Receives a pointer to the existing SACL of the object
	PSID											m_psidExistingOwner;					// Receives a pointer to the SID of the existing owner of the object
	PSID											m_psidExistingGroup;					// Receives a pointer to the SID of the existing primary group of the object

	DWORD											m_nDACLEntries;						//	Count of ACEs that apply to the DACL
	DWORD											m_nSACLEntries;						//	Count of ACEs that apply to the SACL

	DWORD											m_nListFormat;							// Use SDDL format when listing permissions?
	DWORD											m_nListWhat;							// What to list (DACL, SACL, owner, group)
	DWORD											m_nListNameSID;						// List names, SIDs, or both?
	BOOL											m_fListInherited;						// List inherited permissions?

	CString										m_sBackupRestoreFile;				// Name of a (unicode) file to be used for backup/restore operations
	FILE*											m_fhBackupRestoreFile;				// File handle of a (unicode) file to be used for backup/restore operations

	BOOL											m_fProcessSubObjectsOnly;			//	Only process sub-objects of the object specified.

	BOOL											m_fUseLowLevelWrites2SD;			// Use low-level functions (no automatic propagation of inheritance!) to write the SD?

	CString										m_sLogFile;								// File to use for logging
	FILE*											m_fhLog;									// Filehandle to log file

	BOOL											m_fTrusteesProcessDACL;				//	Do we need to process the DACL when dealing with trustees?
	BOOL											m_fTrusteesProcessSACL;				//	Do we need to process the SACL when dealing with trustees?
	BOOL											m_fDomainsProcessDACL;				//	Do we need to process the DACL when dealing with domains?
	BOOL											m_fDomainsProcessSACL;				//	Do we need to process the SACL when dealing with domains?
};



//////////////////////////////////////////////////////////////////////
//
// Class CSD
//
//////////////////////////////////////////////////////////////////////


class CSD
{

public:

								CSD (CSetACL* setaclMain);
	virtual					~CSD ();
	DWORD						GetSD (CString sObjectPath, SE_OBJECT_TYPE nObjectType, SECURITY_INFORMATION siSecInfo);	// Get the SD and other information indicated by siSecInfo
	DWORD						SetSD (CString sObjectPath, SE_OBJECT_TYPE nObjectType, SECURITY_INFORMATION siSecInfo, PACL paclDACL, PACL paclSACL, PSID psidOwner, PSID psidGroup);	// Set the SD and other information indicated by siSecInfo
	DWORD						DeleteACEsByHeaderFlags (DWORD nWhere, BYTE nFlags, BOOL fFlagsSet);								//	Delete all ACEs from an ACL that have certain header flags set
	DWORD						ProcessACEsOfGivenTrustees (DWORD nWhere);																//	Process (delete, replace, copy) all ACEs belonging to trustees specified
	DWORD						ProcessACEsOfGivenDomains (DWORD nWhere);																	//	Process (delete, replace, copy) all ACEs belonging to domains specified

public:

	PACL						m_paclDACL;							// Pointer to the DACL in the SD
	PACL						m_paclSACL;							// Pointer to the SACL in the SD
	PSID						m_psidOwner;						// Pointer to the owner in the SD
	PSID						m_psidGroup;						// Pointer to the primary group in the SD
	PSECURITY_DESCRIPTOR	m_psdSD;								//	Pointer to the security descriptor this is all about
	DWORD						m_nAPIError;						// Error code of the last API function called

private:

	void						DeleteBufSD ();					//	Delete internal buffer for SD
	void						DeleteBufDACL ();					//	Delete internal buffer for DACL
	void						DeleteBufSACL ();					//	Delete internal buffer for SACL
	void						DeleteBufOwner ();				//	Delete internal buffer for Owner
	void						DeleteBufGroup ();				//	Delete internal buffer for primary Group

	PACL						ACLCopyACE (PACL paclACL, DWORD nACE, PSID psidNewTrustee);											// Copy an ACE in an ACL
	PACL						ACLReplaceACE (PACL paclACL, DWORD nACE, PSID psidNewTrustee);										// Replace an ACE in an ACL

private:

	CString					m_sObjectPath;						// Path to the object who's SD we're interested in
	SE_OBJECT_TYPE			m_nObjectType;						// Type of the object to be processed
	SECURITY_INFORMATION	m_siSecInfo;						// What are we interested in (DACL, SACL, owner, group)
	CSetACL*					m_setaclMain;						// Pointer to the main class; we need some of it's methods
	BOOL						m_fBufSDAlloc;						// Did we allocate the buffer for the SD ourselves?
	BOOL						m_fBufDACLAlloc;					// Did we allocate the buffer for the SD ourselves?
	BOOL						m_fBufSACLAlloc;					// Did we allocate the buffer for the SD ourselves?
	BOOL						m_fBufOwnerAlloc;					// Did we allocate the buffer for the SD ourselves?
	BOOL						m_fBufGroupAlloc;					// Did we allocate the buffer for the SD ourselves?

	BOOL						m_fUseLowLevelWrites2SD;		// Use low-level functions (no automatic propagation of inheritance!) to write the SD?

};


//////////////////////////////////////////////////////////////////////
//
// Global functions
//
//////////////////////////////////////////////////////////////////////

BOOL		Split		(CString sDelimiter, CString sInput, CStringArray* saOutput);		// Split a string into multiple parts at a given delimiter
PSID		CopySID	(PSID pSID);																		// Copy a SID
void		BuildLongUnicodePath (CString* sPath);													// Take any path any turn it into a path that is not limited to MAX_PATH, if possible
