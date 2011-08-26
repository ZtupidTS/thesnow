// CodeSign.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "CodeSign.h"


// 这是导出函数的一个示例。
//-------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Example of verifying the embedded signature of a PE file by using 
// the WinVerifyTrust function.

#define _UNICODE 1
#define UNICODE 1

//#include <tchar.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>

// Link with the Wintrust.lib file.

LPTSTR GetSubjectname(PCCERT_CONTEXT pCertContext);


int fnCodeSign(LPCWSTR pwszSourceFile)
{
    LONG lStatus;
    DWORD dwLastError;
	int code=1;
    // Initialize the WINTRUST_FILE_INFO structure.

    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = pwszSourceFile;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    /*
    WVTPolicyGUID specifies the policy to apply on the file
    WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:
    
    1) The certificate used to sign the file chains up to a root 
    certificate located in the trusted root certificate store. This 
    implies that the identity of the publisher has been verified by 
    a certification authority.
    
    2) In cases where user interface is displayed (which this example
    does not do), WinVerifyTrust will check for whether the  
    end entity certificate is stored in the trusted publisher store,  
    implying that the user trusts content from this publisher.
    
    3) The end entity certificate has sufficient permission to sign 
    code, as indicated by the presence of a code signing EKU or no 
    EKU.
    */

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;

    // Initialize the WinVerifyTrust input data structure.

    // Default all fields to 0.
    memset(&WinTrustData, 0, sizeof(WinTrustData));

    WinTrustData.cbStruct = sizeof(WinTrustData);
    
    // Use default code signing EKU.
    WinTrustData.pPolicyCallbackData = NULL;

    // No data to pass to SIP.
    WinTrustData.pSIPClientData = NULL;

    // Disable WVT UI.
    WinTrustData.dwUIChoice = WTD_UI_NONE;

    // No revocation checking.
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

    // Verify an embedded signature on a file.
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

    // Default verification.
    WinTrustData.dwStateAction = 0;

    // Not applicable for default verification of embedded signature.
    WinTrustData.hWVTStateData = NULL;

    // Not used.
    WinTrustData.pwszURLReference = NULL;

    // Default.
    WinTrustData.dwProvFlags = WTD_SAFER_FLAG;

    // This is not applicable if there is no UI because it changes 
    // the UI to accommodate running applications instead of 
    // installing applications.
    WinTrustData.dwUIContext = 0;

    // Set pFile.
    WinTrustData.pFile = &FileData;

    // WinVerifyTrust verifies signatures as specified by the GUID 
    // and Wintrust_Data.
    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    switch (lStatus) 
    {
        case ERROR_SUCCESS:
            /*
            Signed file:
                - Hash that represents the subject is trusted.

                - Trusted publisher without any verification errors.

                - UI was disabled in dwUIChoice. No publisher or 
                    time stamp chain errors.

                - UI was enabled in dwUIChoice and the user clicked 
                    "Yes" when asked to install and run the signed 
                    subject.
            */

//            wprintf_s(L"The file \"%s\" is signed and the signature "
//              L"was verified.\n",
//                pwszSourceFile);
			code=-1;
            break;
        
        case TRUST_E_NOSIGNATURE:
            // The file was not signed or had a signature 
            // that was not valid.

            // Get the reason for no signature.
            dwLastError = GetLastError();
            if (TRUST_E_NOSIGNATURE == dwLastError ||
                    TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
                    TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
            {
                // The file was not signed.
 //               wprintf_s(L"The file \"%s\" is not signed.\n",
 //                   pwszSourceFile);
				code=-2;
            } 
            else 
            {
                // The signature was not valid or there was an error 
                // opening the file.
//                wprintf_s(L"An unknown error occurred trying to "
  //                  L"verify the signature of the \"%s\" file.\n",
    //                pwszSourceFile);
				code=-3;
            }

            break;

        case TRUST_E_EXPLICIT_DISTRUST:
            // The hash that represents the subject or the publisher 
            // is not allowed by the admin or user.
 //           wprintf_s(L"The signature is present, but specifically "
 //               L"disallowed.\n");
			code=-4;
            break;

        case TRUST_E_SUBJECT_NOT_TRUSTED:
            // The user clicked "No" when asked to install and run.
 //           wprintf_s(L"The signature is present, but not "
 //               L"trusted.\n");
			code=-5;
            break;

        case CRYPT_E_SECURITY_SETTINGS:
            /*
            The hash that represents the subject or the publisher 
            was not explicitly trusted by the admin and the 
            admin policy has disabled user trust. No signature, 
            publisher or time stamp errors.
            
            wprintf_s(L"CRYPT_E_SECURITY_SETTINGS - The hash "
                L"representing the subject or the publisher wasn't "
                L"explicitly trusted by the admin and admin policy "
                L"has disabled user trust. No signature, publisher "
                L"or timestamp errors.\n");
			*/
			code=-6;
            break;

        default:
            // The UI was disabled in dwUIChoice or the admin policy 
            // has disabled user trust. lStatus contains the 
            // publisher or time stamp chain error.
 //           wprintf_s(L"Error is: 0x%x.\n",
 //               lStatus);
			code=-7;
            break;
    }
    return code;
}

LPCWSTR fnSignWhois(LPCWSTR pwszSourceFile)
{
	LPCWSTR SignWhois;
	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL; 
	PCCERT_CONTEXT pCertContext = NULL;
	BOOL fResult;   
	DWORD dwEncoding, dwContentType, dwFormatType;
	PCMSG_SIGNER_INFO pSignerInfo = NULL;
	PCMSG_SIGNER_INFO pCounterSignerInfo = NULL;
	DWORD dwSignerInfo;
	CERT_INFO CertInfo;     

	__try
	{
		// Get message handle and store handle from the signed file.
		fResult = CryptQueryObject(CERT_QUERY_OBJECT_FILE,
			pwszSourceFile,
			CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
			CERT_QUERY_FORMAT_FLAG_BINARY,
			0,
			&dwEncoding,
			&dwContentType,
			&dwFormatType,
			&hStore,
			&hMsg,
			NULL);
		if (!fResult)
		{
			return L"-1";
			__leave;
		}

		// Get signer information size.
		fResult = CryptMsgGetParam(hMsg, 
			CMSG_SIGNER_INFO_PARAM, 
			0, 
			NULL, 
			&dwSignerInfo);
		if (!fResult)
		{
			return L"-2";
			__leave;
		}

		// Allocate memory for signer information.
		pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
		if (!pSignerInfo)
		{
			return L"-3";
			__leave;
		}

		// Get Signer Information.
		fResult = CryptMsgGetParam(hMsg, 
			CMSG_SIGNER_INFO_PARAM, 
			0, 
			(PVOID)pSignerInfo, 
			&dwSignerInfo);
		if (!fResult)
		{
			return L"-4";
			__leave;
		}

		// Search for the signer certificate in the temporary 
		// certificate store.
		CertInfo.Issuer = pSignerInfo->Issuer;
		CertInfo.SerialNumber = pSignerInfo->SerialNumber;

		pCertContext = CertFindCertificateInStore(hStore,
			ENCODING,
			0,
			CERT_FIND_SUBJECT_CERT,
			(PVOID)&CertInfo,
			NULL);
		if (!pCertContext)
		{
			return L"-5";
			__leave;
		}

		SignWhois=GetSubjectname(pCertContext);
	}
	__finally
	{               
		if (pSignerInfo != NULL) LocalFree(pSignerInfo);
		if (pCounterSignerInfo != NULL) LocalFree(pCounterSignerInfo);
		if (pCertContext != NULL) CertFreeCertificateContext(pCertContext);
		if (hStore != NULL) CertCloseStore(hStore, 0);
		if (hMsg != NULL) CryptMsgClose(hMsg);
	}
	return SignWhois;
}

LPTSTR GetSubjectname(PCCERT_CONTEXT pCertContext)
{
	LPTSTR fReturn;
	WCHAR szName[4096];
	DWORD dwData;
	// Get Subject name size.
	if (!(dwData = CertGetNameString(pCertContext, 
		CERT_NAME_SIMPLE_DISPLAY_TYPE,
		0,
		NULL,
		NULL,
		0)))
	{
		fReturn=L"-6";
	}
	// Get subject name.
	if (!(CertGetNameString(pCertContext, 
		CERT_NAME_SIMPLE_DISPLAY_TYPE,
		0,
		NULL,
		szName,
		dwData)))
	{
		fReturn=L"-6";
	}
	fReturn = szName;
	return fReturn;
}
