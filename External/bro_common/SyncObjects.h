// SyncObjects.h: interface for the SyncObjects class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYNCOBJECTS_H__739C7D2A_4D0F_4134_A09F_8A893C66F779__INCLUDED_)
#define AFX_SYNCOBJECTS_H__739C7D2A_4D0F_4134_A09F_8A893C66F779__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
class CCriticalSection  
{
public:
	//////////////////////////////////////////////////////////////////////

	CCriticalSection();

	
	virtual ~CCriticalSection();
	//////////////////////////////////////////////////////////////////////

	void Lock()
	{
		EnterCriticalSection( &m_CritSec );
	};

	void Unlock()
	{
		LeaveCriticalSection( &m_CritSec );
	};

    CRITICAL_SECTION    m_CritSec;
};

class TLock
{
public:
	TLock(CCriticalSection& s) : m_cs(&s) { m_cs->Lock(); }
	TLock(CCriticalSection* s) : m_cs(s)  { m_cs->Lock(); }
	~TLock() { m_cs->Unlock(); }
private:
	CCriticalSection* m_cs;
};
#endif // !defined(AFX_SYNCOBJECTS_H__739C7D2A_4D0F_4134_A09F_8A893C66F779__INCLUDED_)
