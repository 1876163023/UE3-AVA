#pragma once

#pragma warning(push)
#pragma warning (disable: 4702)
#include <map>
#pragma warning(pop)

#include "Utils.h"

class NumberPool;
class Session;
class CP2PClientEx;

class SessionMgr
{
public:
	typedef __int32 SessionKeyType;
	typedef std::map<SessionKeyType, Session*> SessionMap;
	typedef SessionMap::iterator SessionMapIT;
	typedef SessionMap::const_iterator SessionMapCIT;

public:
	SessionMgr(CP2PClientEx* p2pCli);
	virtual ~SessionMgr(void);

	Session*	CreateSession();
	void		RemoveSession(Session* session);
	/* GetLock()�� TLock�� �̿��ؼ� lock�� �ɰ� GetSession���� ���� session�� ����ؾ� �Ѵ� */
	Session*	GetSession(SessionKeyType sessionKey);
	CCriticalSection* GetLock() { return m_cs; }

private:
	typedef NumberPoolT<SessionKeyType> NumberPool;
	SessionMap			m_map;
	NumberPool			m_numberPool;
	CCriticalSection*	m_cs;
	CP2PClientEx*		m_p2pCli;
};


#include "SessionMgr.inl"