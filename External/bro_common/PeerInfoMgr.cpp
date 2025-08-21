#include "StdAfx.h"
#include ".\peerinfomgr.h"
#include "svccommons.h"
#include "types.h"

PeerInfoMgr::PeerInfoMgr(void)
{
}

PeerInfoMgr::~PeerInfoMgr(void)
{
}

bool PeerInfoMgr::AddPeer(KeyType key, STRUCT_PEER_INFO* pi)
{
	SystemPrint(ERROR_LOG, "-MBS-[PeerInfoMgr::AddPeer] pi=0x%x\n", pi);
	return m_map.insert(PeerInfoMap::value_type(key, pi)).second;
}

void PeerInfoMgr::RemovePeer(KeyType key)
{
	PeerInfoMapIT it = m_map.find(key);
	if (it != m_map.end())
	{
		STRUCT_PEER_INFO* pi = it->second;
		SAFE_DELETE(pi);
		m_map.erase(it);
	}
}

void PeerInfoMgr::RemoveAllPeer()
{
	for(PeerInfoMapIT it = m_map.begin(); it != m_map.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		SAFE_DELETE(pi);
	}
	m_map.clear();

	SystemPrint(ERROR_LOG, "-MBS-[PeerInfoMgr::RemoveAllPeer] m_map.clear()\n");
	
}

const PeerInfoMgr::PeerInfoMap& PeerInfoMgr::GetMap()
{
	return m_map;
}

STRUCT_PEER_INFO* PeerInfoMgr::GetPeerInfo(KeyType key)
{
	PeerInfoMapIT it = m_map.find(key);
	if (it != m_map.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}
