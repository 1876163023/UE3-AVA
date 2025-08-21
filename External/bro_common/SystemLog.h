// SystemLog.h: interface for the CSystemLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEMLOG_H__CB936DB0_7A04_4BD1_BE57_F68512D18371__INCLUDED_)
#define AFX_SYSTEMLOG_H__CB936DB0_7A04_4BD1_BE57_F68512D18371__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SyncObjects.h"
#include "time.h"

#define ERROR_LOG	2
#define WARNING_LOG	1
#define	BULLET_LOG	3
#define LOGIN_LOG	4
#define GAME_LOG	5
#define EVENT_LOG	6
#define	TRACE_LOG	7
#define	HEARTBEAT_LOG	8

#define	MAX_STORED_LOG_LINE	2048
#define	MAX_LOG_LENGTH		4096
struct LOG_LINE
{
	bool	m_bFilled;
	int		m_nLevel;
	char	m_strLog[MAX_LOG_LENGTH];
};

struct LOG_STATUS
{
	clock_t	m_LastClock;
	void UpdateClock(int clockIn)
	{
		m_LastClock = clockIn;
	}
	clock_t LastClock()
	{
		return m_LastClock;
	}

};
struct LOG_IOSERVER_STATUS : public LOG_STATUS
{
public:
	//update during running
	long	m_nInBytes;
	long	m_nOutBytes;
	long	m_nInPackets;
	long	m_nOutPackets;
	long	m_nLoginPerSec;
	long	m_nLoginFailPerSec;
public:
	bool m_bOneTimeTag;
	// onetimeinit
	long m_nMaxClients;
	long m_nConnectedClients;
	int		m_nReservedSelect;
	int		m_nReservedUpdate;
	long	m_nLoginPC;

	void OneTimeInit(int nMaxClients)
	{
		ZeroMemory(this,sizeof(LOG_IOSERVER_STATUS));
		m_nMaxClients = nMaxClients;
		UpdateClock(clock());

	}
	void IncrementLoginPC()
	{
		InterlockedIncrement(&m_nLoginPC);
	}
	void DecrementLoginPC()
	{
		if(m_nLoginPC > 0)
			InterlockedDecrement(&m_nLoginPC);
	}
	void IncrementClient()
	{
		InterlockedIncrement(&m_nConnectedClients);
	}
	void DecrementClient()
	{
		if(m_nConnectedClients > 0)
			InterlockedDecrement(&m_nConnectedClients);
	}
	void UpdateInBytes(int nBytes)
	{
		InterlockedExchangeAdd(&m_nInBytes,nBytes);
	}
	void UpdateOutBytes(int nBytes)
	{
		InterlockedExchangeAdd(&m_nOutBytes,nBytes);

	}
	void IncrementLoginPerSec()
	{
		InterlockedIncrement(&m_nLoginPerSec);
	}
	void IncrementLoginFailPerSec()
	{
		InterlockedIncrement(&m_nLoginFailPerSec);
	}
	//
	void IncrementInPacket()
	{
		InterlockedIncrement(&m_nInPackets);

	}
	void IncrementOutPacket()
	{
		InterlockedDecrement(&m_nOutPackets);
	}
	void Reset(clock_t clockIn)
	{
		ZeroMemory(this,((LONG_PTR)&(this->m_bOneTimeTag) - (LONG_PTR)this));
		UpdateClock(clockIn);
	}

};
//extern LOG_IOSERVER_STATUS g_IoStatusFront;
void SystemPrint(int nLevel,const char* fmt,...);
int SystemLogLines(int nLines,LOG_LINE* pOutLines);
int		RemoveLogLines(LOG_LINE* pOutLines);
LONG	expLogFilter(DWORD dwExceptionCode, char* strException, BOOL bTopLevel=false);
void	expFlushLog();
#endif // !defined(AFX_SYSTEMLOG_H__CB936DB0_7A04_4BD1_BE57_F68512D18371__INCLUDED_)
