
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#pragma comment(lib, "setupapi.lib")


/////////////////////////////////////////////////////////////////////////////

struct DiskDriveInfo
{
    CString		DriveLetter;	// �h���C�u���^�[
	CString		DevicePath;		// �f�o�C�X�p�X
	CString		VolumeName;		// �{�����[����
	CString		ParentDevId;	// �e�K�w�f�o�C�X�̃f�o�C�XID
};


/////////////////////////////////////////////////////////////////////////////

// �f�B�X�N�h���C�u���
CAtlArray< DiskDriveInfo >	gDriveInfos;


/////////////////////////////////////////////////////////////////////////////

BOOL GetVolumeNameFromDriveLetter()
{
	TCHAR szDriveLetter[] = _T("A:\\");
	TCHAR szVolumeName[MAX_PATH];
	BOOL bRet;

	for ( int i = 0; i < 25; i++ )
	{
		szDriveLetter[0] = _T('A') + i;

		// �h���C�u���^�[����{�����[�������擾
		bRet = GetVolumeNameForVolumeMountPoint(
						szDriveLetter,
						szVolumeName,
						sizeof( szVolumeName ) / sizeof( TCHAR ) );

		// �擾����
		if ( bRet )
		{
			DiskDriveInfo info;

			info.DriveLetter = szDriveLetter;
			info.VolumeName  = szVolumeName;

			gDriveInfos.Add( info );
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////

BOOL GetVolumeNameFromDiskDevicePath()
{
	IID									iidVolumeClass;
	HDEVINFO							hDevInfo;
	SP_DEVICE_INTERFACE_DATA			sDevIfData;
	SP_DEVINFO_DATA						sDevInfoData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pDevIfDetail;
	ULONG								ulLength;
	TCHAR								szDevicePath[MAX_PATH];
	TCHAR								szVolumeName[MAX_PATH];
	BOOL								bRet;

	// Volume Device Class
	IIDFromString( L"{53f5630d-b6bf-11d0-94f2-00a0c91efb8b}", &iidVolumeClass);

	hDevInfo = SetupDiGetClassDevs(
						&iidVolumeClass,
						NULL,
						NULL,
						DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

	if ( hDevInfo == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	sDevIfData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );

	// �f�B�X�N�f�o�C�X��񋓂��A�f�o�C�X�p�X����{�����[�������擾����B
	for (ULONG nIndex = 0; ; nIndex++)
	{
		bRet = SetupDiEnumDeviceInterfaces(
							hDevInfo,
							0,
							&iidVolumeClass,
							nIndex,
							&sDevIfData );

		if ( !bRet )
		{
			break;	// �񋓏I��
		}

		ZeroMemory( &sDevInfoData, sizeof( SP_DEVINFO_DATA ) );

		sDevInfoData.cbSize = sizeof( SP_DEVINFO_DATA );

		// �f�o�C�X�ڍ׏��̒������擾
		SetupDiGetDeviceInterfaceDetail(
							hDevInfo,
							&sDevIfData,
							NULL,
							0,
							&ulLength,
							&sDevInfoData );

		// �f�o�C�X�ڍ׏��p�������m��
		pDevIfDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc( ulLength );

		assert( pDevIfDetail != NULL );

		pDevIfDetail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

		// �f�o�C�X�ڍ׏��擾
		bRet = SetupDiGetDeviceInterfaceDetail(
							hDevInfo,
							&sDevIfData,
							pDevIfDetail,
							ulLength,
							&ulLength,
							&sDevInfoData );

		// �f�o�C�X�ڍ׏��擾����
		if ( bRet )
		{
			// �f�o�C�X�p�X�i������'\'��t����j
			_tcscpy( szDevicePath, pDevIfDetail->DevicePath );
			_tcscat( szDevicePath, _T("\\") );

			// �f�o�C�X�p�X����{�����[�������擾
			bRet = GetVolumeNameForVolumeMountPoint(
								szDevicePath,
								szVolumeName,
								sizeof( szVolumeName ) / sizeof( TCHAR ) );

			// �{�����[�����擾����
			if ( bRet )
			{
				// ���h���C�u���^�[����擾�����{�����[�����Ɣ�r��
				//   �h���C�u���^�[�Ɗ֘A�t����B
				for ( size_t i = 0; i < gDriveInfos.GetCount(); i++ )
				{
					DiskDriveInfo& rInfo = gDriveInfos[i];

					// �{�����[��������v
					if ( rInfo.VolumeName.CompareNoCase( szVolumeName ) == 0 )
					{
						rInfo.DevicePath = szDevicePath;

						// ���{�����[���f�o�C�X�̐e�K�w(�f�B�X�N�f�o�C�X?)��
						// �@�f�o�C�XID���擾����B
						{
							DEVINST hDevInst = sDevInfoData.DevInst;
							DEVINST hParentDev;
							ULONG ulParentDevIdLen;
							LPTSTR pParentDevId;
							CONFIGRET cRet;

							cRet = CM_Get_Parent( &hParentDev, hDevInst, 0 );

							if ( cRet == CR_SUCCESS )
							{
								CM_Get_Device_ID_Size( &ulParentDevIdLen, hParentDev, 0 );
								++ulParentDevIdLen; // �k��������

								pParentDevId = (LPTSTR)malloc( ulParentDevIdLen * sizeof( TCHAR ) );

								assert( pParentDevId != NULL );

								ZeroMemory( pParentDevId, ulParentDevIdLen * sizeof( TCHAR ) );

								cRet = CM_Get_Device_ID( hParentDev, pParentDevId, ulParentDevIdLen, 0 );

								if ( cRet == CR_SUCCESS )
								{
									rInfo.ParentDevId = pParentDevId;
								}

								free( pParentDevId );
							}
						}
					}
				}
			}
		}

		free( pDevIfDetail );
	}

	SetupDiDestroyDeviceInfoList( hDevInfo );

	return TRUE;
}

void ShowData()
{
	CString cstr, clip;

	for ( size_t i = 0; i < gDriveInfos.GetCount(); i++ )
	{
		DiskDriveInfo& rInfo = gDriveInfos[i];

		cstr.Format(_T("Drive Letter : %s\nDevice Path  : %s\nVolume Name  : %s\nParent DevId : %s\n\n"), (LPCTSTR)rInfo.DriveLetter, (LPCTSTR)rInfo.DevicePath, (LPCTSTR)rInfo.VolumeName, (LPCTSTR)rInfo.ParentDevId);
		clip += cstr;
	}
	CStdioFile file;
	file.Open(_T("test.txt"), CFile::modeCreate | CFile::modeWrite | CFile::typeText );
	file.WriteString(clip);
	file.Close();
}
