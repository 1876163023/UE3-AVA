// Thread.cpp: implementation of the Thread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Thread.h"
#include <process.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommonThread::CCommonThread()
{
	m_bRunning = false;
	m_bStop = false;
	m_strDesc[0] ='\0';
}

CCommonThread::~CCommonThread()
{

}


UINT  __stdcall CCommonThread::StaticThreadProc(PVOID pParam)
{
	return ((CCommonThread*)pParam)->OnRunThreadProc();
}
bool CCommonThread::Begin(char* strDesc,FPTHREADPROC pThreadProc,PVOID pvContext)
{
	FPTHREADPROC pProc = CCommonThread::StaticThreadProc;
	PVOID pvCon = this;
	//strcpy(m_strDesc,strDesc);
	if (FAILED(StringCchCopyA(m_strDesc, 256, strDesc)) ) {
		assert(!"CCommonThread::Begin(strcpy fail)");
		return false;
	}
	if(pThreadProc)
		pProc = pThreadProc;
	if(pvContext)
		pvCon = pvContext;
	m_hThread = (HANDLE) _beginthreadex(NULL,0,pProc,pvCon,0,&m_dwID);
	if( m_hThread )
	{
		m_bRunning = true;
		return true;
	}
	return false;

}
bool CCommonThread::End(bool bWait)
{
	m_bStop = true;
	if(false == bWait)
		return true;
	while(m_bRunning == true)
	{
		Sleep(100);

	}
	m_bStop = false;
	return true;
}
int CCommonThread::OnRunThreadProc()
{
	return 0;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThread::CThread()
{
	m_bDeleted = FALSE;
	m_nExecuteCount = 0;
	InitializeCriticalSection(&m_csExcuteCount);
	for (int i = 0; i < MAX_CONCURRENT_THREAD; i++) {
		m_pRun[i] = NULL;
	}
}

CThread::~CThread()
{
	m_bDeleted = TRUE;
	//m_pbRun = NULL;
	DeleteCriticalSection(&m_csExcuteCount);
	for ( int i = 0; i < MAX_CONCURRENT_THREAD; i++) {
		if (m_pRun[i]) {
			if (IsBadWritePtr(m_pRun[i], sizeof(int)) ) continue;
			if (THREAD_RUN == *(m_pRun[i])) {
				*(m_pRun[i]) = THREAD_TERMINATING;
			}
		}
	}
}

BOOL CThread::StopThread(int nID)
{
	int		i;
	//m_bRun = false;
	if (-1 == nID) {
		bool	bLive = true;
		for ( i = 0; i < MAX_CONCURRENT_THREAD; i++) {
			if (m_pRun[i]) {
				if (IsBadWritePtr(m_pRun[i], sizeof(int)) ) continue;
				if (THREAD_RUN == *(m_pRun[i])) {
					*(m_pRun[i]) = THREAD_TERMINATING;
				}
			}
		}
		while (bLive) {
			bLive = false;
			for ( i = 0; i < MAX_CONCURRENT_THREAD; i++) {
				if (m_pRun[i]) {
					if (IsBadWritePtr(m_pRun[i], sizeof(int)) ) continue;
					if (THREAD_TERMINATING == *(m_pRun[i]) ) {
						bLive = true;
						break;
					}
				}
			}
			Sleep(50);
		}
	}else {
		if (m_pRun[nID]) {
			if (IsBadWritePtr(m_pRun[nID], sizeof(int)) ) return TRUE;
			if (THREAD_RUN == *(m_pRun[nID])) {
				*(m_pRun[nID]) = THREAD_TERMINATING;
			}
		}
	}
	//while(m_nExecuteCount) {
	//	Sleep(50);
	//}
	return TRUE;
}

BOOL CThread::BeginThread()
{
	//AfxBeginThread( &CThread::StartPoint, (void *)this);
	unsigned int dwOutputThreadID;
	//m_bRun = true;
	_beginthreadex( NULL, 0, &CThread::StartPoint, this, 0, &dwOutputThreadID );
	return TRUE;
}
