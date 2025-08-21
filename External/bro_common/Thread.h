// Thread.h: interface for the Thread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_H__1C7E44B0_C9EF_46A4_93D8_7B8994FD627C__INCLUDED_)
#define AFX_THREAD_H__1C7E44B0_C9EF_46A4_93D8_7B8994FD627C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define	MAX_CONCURRENT_THREAD	128

typedef UINT  (__stdcall *FPTHREADPROC)(PVOID);
class CCommonThread  
{
public:
	HANDLE	m_hThread;
	UINT	m_dwID;
	bool	m_bRunning;
	bool	m_bStop;
	char	m_strDesc[256];
	CCommonThread();
	virtual ~CCommonThread();

	static UINT  __stdcall StaticThreadProc(PVOID pParam);
	bool Begin(char* strDesc,FPTHREADPROC pThreadProc = NULL,PVOID pvContext = NULL);
	bool End(bool bWait=false);
	virtual int OnRunThreadProc();
};

enum THREAD_STATE {
	THREAD_INIT = 0,
	THREAD_RUN,
	THREAD_TERMINATING,
	THREAD_TERMINATED,
};
class CThread  
{
public:
	BOOL	StopThread(int nID=-1);
	BOOL	BeginThread();
	static	UINT WINAPI	StartPoint(LPVOID lpParam)
	{
		{
			DWORD procmask;
			DWORD sysmask;
			GetProcessAffinityMask(GetCurrentProcess(), &procmask, &sysmask);
			DWORD fprocmask = 1;
			while (!(fprocmask & sysmask))
				fprocmask <<= 1;
			SetThreadAffinityMask(GetCurrentThread(), fprocmask);
			Sleep(1);
		}
		CThread *pThread = (CThread *)lpParam;
		int		nRun = THREAD_RUN;
		int		nID;
		UINT	nResult;
		EnterCriticalSection(&(pThread->m_csExcuteCount));
		nID = pThread->m_nExecuteCount;
		if ( pThread->m_pRun[nID] && (THREAD_RUN == *(pThread->m_pRun[nID])) ) {
			LeaveCriticalSection(&(pThread->m_csExcuteCount));
			return 0;
		}
		pThread->m_pRun[nID] = &nRun;
		//m_nExecuteCount++;
		pThread->m_nExecuteCount = (pThread->m_nExecuteCount + 1) % MAX_CONCURRENT_THREAD;
		LeaveCriticalSection(&(pThread->m_csExcuteCount));
		nResult =  pThread->Run(&nRun, nID);
		nRun = THREAD_TERMINATED;
		return nResult;
	}

	virtual	UINT	Run(int* /*pRun*/, int /*nID*/){ return 1; }
	CThread();
	virtual ~CThread();
protected:
	BOOL	m_bDeleted;
	int		m_nExecuteCount;
	//bool	m_bRun;
	LPINT	m_pRun[MAX_CONCURRENT_THREAD];
private:
	CRITICAL_SECTION	m_csExcuteCount;
};

#endif // !defined(AFX_THREAD_H__1C7E44B0_C9EF_46A4_93D8_7B8994FD627C__INCLUDED_)
