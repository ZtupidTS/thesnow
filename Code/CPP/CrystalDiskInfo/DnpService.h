// http://www.dinop.com/vc/service_ctrl.html (ja)

#pragma once

#include <winsvc.h>
#include "atlstr.h"

class	CDnpService
{
	//
	//	�T�[�r�X�̋N���^��~�p�X���b�h�N���X
	//
	class CServiceThread
	{
	public:
		CServiceThread()
		{
			_bCancel = false;
		}

	private:

		bool					_bCancel;			//�T�[�r�X�̋N���^��~�������f�p�ϐ��Atrue�Ȃ璆�f�J�n
		CComAutoCriticalSection	_secbCancel;		//�T�[�r�X�̋N���^��~�������f�p�N���e�B�J���Z�N�V����

	public:

		//
		//	�T�[�r�X�̋N���^��~�������f�p�֐�
		//
		//	���f�������ꍇ��IsCancel(true,true)���Ăяo��
		//
		bool	IsCancel(bool bSave=false,bool bNewValue=false)
		{
			bool	ret;

			_secbCancel.Lock();
				if(bSave)
				{
					_bCancel = bNewValue;
					ret = true;
				}
				else
					ret = _bCancel;
			_secbCancel.Unlock();

			return	ret;
		}



		//
		//	�T�[�r�X�̊ȈՃR���g���[��
		//
		//	���̂܂܌Ăяo���ƃT�[�r�X���N���^��~����܂Ŗ������[�v�őҋ@����B
		//	�X���b�h���ŌĂяo���AIsCancel()�𗘗p���邱�ƂŖ������[�v�Ɋׂ�Ȃ�
		//	�R���g���[�����\�B
		//
		bool	EasyStartStop(LPCTSTR pszName,bool bStart)
		{
			bool			ret;
			BOOL			bRet;
			SC_HANDLE		hManager;
			SC_HANDLE		hService;
			SERVICE_STATUS	sStatus;

			ret = false;
			hManager = NULL;
			hService = NULL;
			while(1)			//�������[�v�ł͂Ȃ��I
			{
				hManager = ::OpenSCManager(NULL,NULL,GENERIC_EXECUTE);
				if(hManager == NULL)
					break;

				if(bStart)
					hService = ::OpenService(hManager,pszName,SERVICE_START | SERVICE_QUERY_STATUS);
				else
					hService = ::OpenService(hManager,pszName,SERVICE_STOP | SERVICE_QUERY_STATUS);
				if(hService == NULL)
					break;

				::ZeroMemory(&sStatus,sizeof(SERVICE_STATUS));
				bRet = ::QueryServiceStatus(hService,&sStatus);
				if(bRet == FALSE)
					break;

				if(bStart && sStatus.dwCurrentState == SERVICE_RUNNING)
				{
					//���ɃT�[�r�X�͓����Ă���
					ret = true;
					break;
				}
				if((bStart == false) && sStatus.dwCurrentState == SERVICE_STOPPED)
				{
					//���ɃT�[�r�X�͎~�܂��Ă���
					ret = true;
					break;
				}

				CString cstr;
				cstr.Format(_T("sStatus.dwCurrentState:%08X"), sStatus.dwCurrentState);
				DebugPrint(cstr);

				if(bStart)
				{
					////////////////////////////
					//	�T�[�r�X�J�n
					//

					if(sStatus.dwCurrentState == SERVICE_STOPPED)
					{
						//�T�[�r�X�J�n�v��
						DebugPrint(_T("StartService"));
						bRet = ::StartService(hService,NULL,NULL);
						// ERROR_ALREADY_EXISTS
						/*
						if(bRet == FALSE)
						{
							cstr.Format(_T("%08X"), GetLastError());
							DebugPrint(_T("NG:StartService"));
							DebugPrint(cstr);
							break;
						}
						else
						{
							DebugPrint(_T("OK:StartService"));
						}
						*/

						//�J�n�܂Ŗ������[�v�őҋ@
						//IsCancel�𗘗p����Ζ������[�v����̒E�o���\
						DebugPrint(_T("QueryServiceStatus"));
						int count = 0;
						while(::QueryServiceStatus(hService,&sStatus))
						{
							// �������[�v����� (�ő� 5 �b�� WMI �̏�������҂�)
							if(count >= 10)
							{
								break;
							}

							if(sStatus.dwCurrentState == SERVICE_RUNNING)
							{
								ret = true;
								break;
							}
							DebugPrint(_T("sStatus.dwCurrentState != SERVICE_RUNNING"));

							if(IsCancel())
								break;

							::Sleep(500);
							DebugPrint(_T("Sleep"));
							count++;
							continue;
						}
						cstr.Format(_T("GetLastError():%08X"), GetLastError());
						DebugPrint(cstr);
					}
					break;
				}


				////////////////////////////
				//	�T�[�r�X��~
				//

				if(sStatus.dwCurrentState == SERVICE_RUNNING)
				{
					//�T�[�r�X��~�v��
					bRet = ::ControlService(hService,SERVICE_CONTROL_STOP,&sStatus);
					/*
					if(bRet == FALSE)
						break;
					*/

					//��~�܂Ŗ������[�v�őҋ@
					//IsCancel�𗘗p����Ζ������[�v����̒E�o���\
					while(::QueryServiceStatus(hService,&sStatus))
					{
						if(sStatus.dwCurrentState == SERVICE_STOPPED)
						{
							ret = true;
							break;
						}

						if(IsCancel())
							break;

						//::Sleep(sStatus.dwWaitHint);
						//�{���Ȃ��dwWaitHit����Sleep�������邪�A���f���������邽��
						//500msec�������Sleep����
						::Sleep((sStatus.dwWaitHint > 500) ? 500 : sStatus.dwWaitHint);
						continue;
					}
				}

				break;		//�K�{�I���̍s���Ȃ��Ɩ������[�v�ɂȂ邩��
			}

			DebugPrint(_T("EasyStartStop"));
			//ATLASSERT(ret);

			if(hService)
				::CloseServiceHandle(hService);
			if(hManager)
				::CloseServiceHandle(hManager);

			return	ret;
		}
	};


public:


	//
	//	�T�[�r�X�̊ȈՃR���g���[��
	//
	//	�T�[�r�X���N��/��~����܂Ŗ������[�v�őҋ@����B
	//
	bool	EasyStartStop(LPCTSTR pszName,bool bStart)
	{
		CServiceThread	cThread;

		return	cThread.EasyStartStop(pszName,bStart);
	}


	//
	//	�T�[�r�X�̊ȈՋN��
	//
	//	�T�[�r�X���N������܂Ŗ������[�v�őҋ@����B
	//
	bool	EasyStart(LPCTSTR pszName)
	{
		return	EasyStartStop(pszName,true);
	}

	//
	//	�T�[�r�X�̊ȈՒ�~
	//
	//	�T�[�r�X����~����܂Ŗ������[�v�őҋ@����B
	//
	bool	EasyStop(LPCTSTR pszName)
	{
		return	EasyStartStop(pszName,false);
	}


	//
	//	�T�[�r�X�̊ȈՍċN��
	//
	//	�T�[�r�X���ċN������܂Ŗ������[�v�őҋ@����B
	//
	bool	EasyRestart(LPCTSTR pszName)
	{
		bool			ret;
		CServiceThread	cThread;

		ret = cThread.EasyStartStop(pszName,false);
		if(ret)
			ret = cThread.EasyStartStop(pszName,true);

		return	ret;
	}



	//
	//	�w�肷��T�[�r�X�������Ă��邩�̃`�F�b�N
	//
	//	false�̏ꍇ��"��~"�Ƃ͌���Ȃ��B�T�[�r�X�����݂��Ȃ��ꍇ�Ȃǂ�false�ƂȂ�B
	//
	bool	IsServiceRunning(LPCTSTR pszName)
	{
		bool			ret;
		BOOL			bRet;
		SC_HANDLE		hManager;
		SC_HANDLE		hService;
		SERVICE_STATUS	sStatus;

		ret = false;
		hManager = NULL;
		hService = NULL;
		while(1)			//�������[�v�ł͂Ȃ��I
		{
			hManager = ::OpenSCManager(NULL,NULL,GENERIC_EXECUTE);
			ATLASSERT(hManager);
			if(hManager == NULL)
				break;

			hService = ::OpenService(hManager,pszName,SERVICE_QUERY_STATUS);
			ATLASSERT(hService);
			if(hService == NULL)
				break;

			::ZeroMemory(&sStatus,sizeof(SERVICE_STATUS));
			bRet = ::QueryServiceStatus(hService,&sStatus);
			ATLASSERT(bRet);
			if(bRet == FALSE)
				break;

			if(sStatus.dwCurrentState == SERVICE_RUNNING)
				ret = true;

			break;		//�K�{
		}

		if(hService)
			::CloseServiceHandle(hService);
		if(hManager)
			::CloseServiceHandle(hManager);

		return	ret;
	}
};
