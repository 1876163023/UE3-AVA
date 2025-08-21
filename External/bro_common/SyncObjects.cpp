#include "stdafx.h"
#include "SyncObjects.h"
CCriticalSection::CCriticalSection()
{
	//InitializeCriticalSectionAndSpinCount( &m_CritSec, 4000 );
	InitializeCriticalSection( &m_CritSec );
}
CCriticalSection::~CCriticalSection()
{
   DeleteCriticalSection( &m_CritSec );
}
