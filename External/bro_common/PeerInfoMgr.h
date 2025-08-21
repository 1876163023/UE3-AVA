#pragma once

#pragma warning(push)
#pragma warning (disable: 4702)
#include <map>
#include "CustomAllocator.h"
#pragma warning(pop)

struct STRUCT_PEER_INFO;

class PeerInfoMgr
{
public:
	typedef int KeyType;
	typedef std::map<KeyType, STRUCT_PEER_INFO*, std::less<KeyType>, BroCustomAllocator<KeyType> > PeerInfoMap;
	typedef PeerInfoMap::iterator PeerInfoMapIT;
	typedef PeerInfoMap::const_iterator PeerInfoMapCIT;

public:
	PeerInfoMgr(void);
	~PeerInfoMgr(void);

	bool AddPeer(KeyType key, STRUCT_PEER_INFO* pi); 
	void RemovePeer(KeyType key);
	void RemoveAllPeer();
	const PeerInfoMap& GetMap();
	STRUCT_PEER_INFO* GetPeerInfo(KeyType key);

private:
	PeerInfoMap m_map;
};
