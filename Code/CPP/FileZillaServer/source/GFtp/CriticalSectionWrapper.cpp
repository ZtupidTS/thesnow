#include "CriticalSectionWrapper.h"

CCriticalSectionWrapper::CCriticalSectionWrapper()
{
	InitializeCriticalSection(&m_criticalSection);
}

CCriticalSectionWrapper::~CCriticalSectionWrapper()
{
	DeleteCriticalSection(&m_criticalSection);
}

void CCriticalSectionWrapper::Lock()
{
	EnterCriticalSection(&m_criticalSection);
}
void CCriticalSectionWrapper::Unlock()
{
	LeaveCriticalSection(&m_criticalSection);
}


CLock::CLock(CCriticalSectionWrapper *pCritSection, BOOL bLocked /*=TRUE*/)
{
	m_pCritSection = pCritSection;
	if (bLocked)
	{
		m_bLocked = TRUE;
		m_pCritSection->Lock();
	}
	else
		m_bLocked = FALSE;
}

CLock::~CLock()
{
	if (m_bLocked)
		m_pCritSection->Unlock();
}

void CLock::Lock()
{
	m_pCritSection->Lock();
	m_bLocked = TRUE;
}

void CLock::Unlock()
{
	m_pCritSection->Unlock();
	m_bLocked = FALSE;
}
