#include "windows.h"

class CCriticalSectionWrapper
{
public:
	CCriticalSectionWrapper();
	~CCriticalSectionWrapper();

	void Lock();
	void Unlock();

protected:
	CRITICAL_SECTION m_criticalSection;
};

class CLock
{
public:
	CLock(CCriticalSectionWrapper *pCritSection, BOOL bLocked = TRUE);
	virtual ~CLock();

	void Lock();
	void Unlock();

protected:
	CCriticalSectionWrapper *m_pCritSection;
	BOOL m_bLocked;
};