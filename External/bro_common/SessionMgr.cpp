#include "StdAfx.h"
#include ".\sessionmgr.h"
#include "P2PClientEx.h"
#include "Session.h"

//////////////////////////////////////////////////////////////////////////
// SessionMgr
//////////////////////////////////////////////////////////////////////////
SessionMgr::SessionMgr(CP2PClientEx* p2pCli) : m_p2pCli(p2pCli)
{
	m_cs = new CCriticalSection;
}

SessionMgr::~SessionMgr(void)
{
	SAFE_DELETE(m_cs);
}

Session* SessionMgr::CreateSession()
{
	TLock lo(m_cs);

	SessionKeyType key = m_numberPool.GenerateNumber();
	Session* tmpSession = GetSession(key);
	if (tmpSession)
	{
		m_numberPool.ReturnToPool(key);
		return NULL;
	}

	Session* session = new Session(m_p2pCli, key);
	if (false == m_map.insert(SessionMap::value_type(key, session)).second)
	{
		SAFE_DELETE(session);
		return NULL;
	}

	return session;
}

void SessionMgr::RemoveSession(Session* session)
{
	TLock lo(m_cs);

	SessionKeyType key = session->GetSessionKey();
	session->SetSessionHandler(NULL);
	m_numberPool.ReturnToPool(key);
	SAFE_DELETE(session);
	m_map.erase(key);
}

Session* SessionMgr::GetSession(SessionKeyType sessionKey)
{
	TLock lo(m_cs);

	SessionMapIT it = m_map.find(sessionKey);
	if (it == m_map.end())
	{
		return NULL;
	}

	return it->second;
}

