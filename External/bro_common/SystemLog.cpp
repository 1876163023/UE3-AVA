// SystemLog.cpp: implementation of the CSystemLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tchar.h>
#include <stdarg.h>
#include <stdio.h>
#include "SystemLog.h"

#ifdef _BF_SERVER
#include <NXCore/Str.h>
#endif

#ifdef _BF_CLIENT
#include <NtixCore/LogManager.h>
#include <NXCore/Str.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class CSystemLog  : public CCriticalSection
{
public:
	LOG_LINE	m_LogLines[MAX_STORED_LOG_LINE];
	int			m_nFirstFree;
	int			m_nLastInsert;
	char		m_strTime[256];
	char		m_strDate[256];
public:
	CSystemLog()
	{
		m_nLastInsert = 1;
		m_nFirstFree = 0;
		m_strTime[0] = 0;
		m_strDate[0] = 0;
		_strtime(m_strTime);
		_strdate(m_strDate);
		int i = 0;
		for (;m_strDate[i]; i++) {
			if (m_strDate[i] == '/') {
				m_strDate[i] = '_';
			}
		}
		m_strDate[i] = '_';
		m_strDate[i+1] = m_strTime[0];
		m_strDate[i+2] = m_strTime[1];
		m_strDate[i+3] = 0;
		for( i = 0; i < MAX_STORED_LOG_LINE;i++)
			m_LogLines[i].m_bFilled = false;
	}
	virtual ~CSystemLog()
	{
	}
};

class CExceptionLog
{
public:
	void	Add(char *strContext, DWORD dwExceptionCode)
	{
		if (0 == m_strLog[0]) {
			switch(dwExceptionCode) {
				case EXCEPTION_ACCESS_VIOLATION :
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_ACCESS_VIOLATION)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
					//sprintf(m_strLog, "%s(EXCEPTION_ACCESS_VIOLATION)", strContext); break;
				case EXCEPTION_DATATYPE_MISALIGNMENT:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_DATATYPE_MISALIGNMENT)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_IN_PAGE_ERROR:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_IN_PAGE_ERROR)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_GUARD_PAGE:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_GUARD_PAGE)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_STACK_OVERFLOW:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_STACK_OVERFLOW)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_ILLEGAL_INSTRUCTION:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_ILLEGAL_INSTRUCTION)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_PRIV_INSTRUCTION:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_PRIV_INSTRUCTION)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_INT_DIVIDE_BY_ZERO:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_INT_DIVIDE_BY_ZERO)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				case EXCEPTION_INT_OVERFLOW:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(EXCEPTION_INT_OVERFLOW)", strContext)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
				default:
					if (FAILED(StringCchPrintfA(m_strLog, BUF_SIZE, "%s(%d)", strContext, dwExceptionCode)) ) {
						assert(!"CExceptionLog::Add(m_strLog overrun)");
						return;
					}
					break;
			}
		} else {
			//strcat(m_strLog, "<==");
			if (FAILED(StringCchCatA(m_strLog, BUF_SIZE, "<=="))) {
				assert(!"CExceptionLog::Add(m_strLog overrun)");
				return;
			}
			//strcat(m_strLog, strContext);
			if (FAILED(StringCchCatA(m_strLog, BUF_SIZE, strContext)) ) {
				assert(!"CExceptionLog::Add(m_strLog overrun)");
				return;
			}

		}
	}
	CExceptionLog()
	{
		m_strLog[0] = 0;
	}
	virtual ~CExceptionLog(){}
	const static DWORD BUF_SIZE = 4096;
	char m_strLog[BUF_SIZE];
	
};

CSystemLog			s_SystemLog;
CExceptionLog		s_exceptionLog;

void SystemPrint(int nLevel,const char* fmt,...)
{
	FILE	*fp=NULL;
	char	strTime[60];
	char	strMsg[MAX_LOG_LENGTH*10];
	char	strFilename[256];
    s_SystemLog.Lock();
	if ( FAILED(StringCchVPrintfA(strMsg,MAX_LOG_LENGTH*10, fmt,(CHAR*)((&fmt)+1)) ) ) {
		assert(!"strMsg over run");
		return;
	}

	SYSTEMTIME st; 
	::GetLocalTime(&st);
	DWORD pid = ::GetCurrentProcessId();
	DWORD thid = ::GetCurrentThreadId();

	StringCchPrintfA(strTime, sizeof(strTime), "[%04d/%02d/%02d %02d:%02d:%02d.%03d|pid=%d|th=%d] ", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, 
		pid, thid);

	if (ERROR_LOG==nLevel) {
		StringCchPrintfA(strFilename, 256, "WarningLog/ErrorLog%s.txt", s_SystemLog.m_strDate);
	} else if (BULLET_LOG==nLevel) {
		StringCchPrintfA(strFilename, 256, "BulletLog/BulletLog%s.txt", s_SystemLog.m_strDate);
	} else if (LOGIN_LOG==nLevel) {
		StringCchPrintfA(strFilename, 256, "LoginLog/LogInLog%s.txt", s_SystemLog.m_strDate);
	} else if (GAME_LOG==nLevel) {
		StringCchPrintfA(strFilename, 256, "GameLog/GameLog%s.txt", s_SystemLog.m_strDate);
	} else if (EVENT_LOG==nLevel) {
		StringCchPrintfA(strFilename, 256, "GameLog/EventLog%s.txt", s_SystemLog.m_strDate);
	} else if (TRACE_LOG==nLevel) {
		StringCchPrintfA(strFilename, 256, "WarningLog/TraceLog%s.txt", s_SystemLog.m_strDate);
	} else {
		StringCchPrintfA(strFilename, 256, "WarningLog/WarningLog%s.txt", s_SystemLog.m_strDate);
	}

#ifdef _BF_SERVER
	nxlogmsg(nxc::Str(strMsg));
#endif

#ifdef _BF_CLIENT
	if (ERROR_LOG==nLevel)
	{
		LOGPRT(nxc::Str(strMsg));
	}
#endif

	fp = fopen(strFilename, "a");
	if (fp) {
		fprintf(fp, "%s", strTime);
		if (GAME_LOG==nLevel) {
			fprintf(fp, "%s\n", strMsg);
			char szBuffer[40960];

			//sprintf(szBuffer,"%s\n", strMsg);
			if ( FAILED(StringCchPrintfA( szBuffer, 40960, "%s\n", strMsg)) ) {
				assert(!"szBuffer[40960] over run");
				return;
			}
			fputs(strMsg,fp);
		} else {
			char szBuffer[MAX_LOG_LENGTH];
			//sprintf(szBuffer,"[%s]%s\n", s_SystemLog.m_strTime, strMsg);
			if ( FAILED(StringCchPrintfA( szBuffer, MAX_LOG_LENGTH, "%s\n", strMsg)) ) {
				assert(!"szBuffer[MAX_LOG_LENGTH] over run");
				return;
			}
			fputs(szBuffer,fp);
			//fprintf(fp, "[%s]%s\n", s_SystemLog.m_strTime, strMsg);
		}
		fclose(fp);
	}
	if (nLevel == ERROR_LOG) {
		strMsg[MAX_LOG_LENGTH-1] = 0;
		//strcpy(s_SystemLog.m_LogLines[s_SystemLog.m_nFirstFree].m_strLog,strMsg);
		if (FAILED(StringCchCopyA(s_SystemLog.m_LogLines[s_SystemLog.m_nFirstFree].m_strLog,MAX_LOG_LENGTH,strMsg)) ) {
			assert(!"s_SystemLog.m_LogLines[MAX_LOG_LENGTH] over run");
			return;
		}
		s_SystemLog.m_LogLines[s_SystemLog.m_nFirstFree].m_bFilled = true;
		s_SystemLog.m_LogLines[s_SystemLog.m_nFirstFree].m_nLevel = nLevel;
		s_SystemLog.m_nLastInsert = s_SystemLog.m_nFirstFree;
		s_SystemLog.m_nFirstFree = (s_SystemLog.m_nFirstFree + 1) % MAX_STORED_LOG_LINE;
	}



	s_SystemLog.Unlock();
}

int RemoveLogLines(LOG_LINE* pOutLines)
{
	s_SystemLog.Lock();

	int	nLines = MAX_STORED_LOG_LINE-1;
	int nStart = (s_SystemLog.m_nFirstFree  + 1) % MAX_STORED_LOG_LINE;
	int idx = 0;
	_strtime(s_SystemLog.m_strTime);
	_strdate(s_SystemLog.m_strDate);
	int i = 0;
	for (;s_SystemLog.m_strDate[i]; i++) {
		if (s_SystemLog.m_strDate[i] == '/') {
			s_SystemLog.m_strDate[i] = '_';
		}
	}
	s_SystemLog.m_strDate[i] = '_';
	s_SystemLog.m_strDate[i+1] = s_SystemLog.m_strTime[0];
	s_SystemLog.m_strDate[i+2] = s_SystemLog.m_strTime[1];
	s_SystemLog.m_strDate[i+3] = 0;
	while (nLines)
	{
		if(s_SystemLog.m_LogLines[nStart].m_bFilled == true)
		{
			s_SystemLog.m_LogLines[nStart].m_bFilled = false;
			memcpy(&pOutLines[idx],&s_SystemLog.m_LogLines[nStart],sizeof(LOG_LINE));
			idx++;
		}
		nStart = (nStart + 1) % MAX_STORED_LOG_LINE;
		nLines--;

	}
	s_SystemLog.Unlock();
	return idx;
}
int SystemLogLines(int nLines,LOG_LINE* pOutLines)
{
	s_SystemLog.Lock();

	int nStart = (s_SystemLog.m_nFirstFree  + MAX_STORED_LOG_LINE - nLines) % MAX_STORED_LOG_LINE;
	int idx = 0;
	_strtime(s_SystemLog.m_strTime);
	_strdate(s_SystemLog.m_strDate);
	int i = 0;
	for (;s_SystemLog.m_strDate[i]; i++) {
		if (s_SystemLog.m_strDate[i] == '/') {
			s_SystemLog.m_strDate[i] = '_';
		}
	}
	s_SystemLog.m_strDate[i] = '_';
	s_SystemLog.m_strDate[i+1] = s_SystemLog.m_strTime[0];
	s_SystemLog.m_strDate[i+2] = s_SystemLog.m_strTime[1];
	s_SystemLog.m_strDate[i+3] = 0;
	while (nLines)
	{
		if(s_SystemLog.m_LogLines[nStart].m_bFilled == true)
		{
			memcpy(&pOutLines[idx],&s_SystemLog.m_LogLines[nStart],sizeof(LOG_LINE));
			idx++;
		}
		nStart = (nStart + 1) % MAX_STORED_LOG_LINE;
		nLines--;

	}
	s_SystemLog.Unlock();
	return nStart;
};

LONG	expLogFilter(DWORD dwExceptionCode, char* strException, BOOL bTopLevel)
{
	s_exceptionLog.Add(strException, dwExceptionCode);
	if (bTopLevel) return EXCEPTION_EXECUTE_HANDLER;
	return EXCEPTION_CONTINUE_SEARCH;
}

void	expFlushLog()
{
	SystemPrint(2, s_exceptionLog.m_strLog);
	s_exceptionLog.m_strLog[0] = 0;
}