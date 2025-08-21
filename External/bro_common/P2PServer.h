// P2PServer.h: interface for the CP2PServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_P2PSERVER_H__64356E27_13EB_494B_9E84_3E005A2F9E74__INCLUDED_)
#define AFX_P2PSERVER_H__64356E27_13EB_494B_9E84_3E005A2F9E74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Thread.h"
#include "Pool.h"
#include "P2PClient.h"
#include "TimerManager.h"
//UDP Transponder
struct CP2PInfo : public PoolEntry, public CTimerEntry
{
	CP2PInfo()
	{
		m_nKey = -1;
		m_bNeedToRelay = false;
		m_hAddrRequestExpireTimer = NULL;
	}
	void	SetAddrLocal(SOCKADDR* pAddr)
	{
		memcpy(&m_addrLocal, pAddr, sizeof(SOCKADDR));
	}
	void	SetAddrPublic(SOCKADDR* pAddr)
	{
		memcpy(&m_addrPublic, pAddr, sizeof(SOCKADDR));
	}
	// client가 포함된 P2P party의 peer들의 상태를 가져온다.(정상적인 경우 모든 party원들의 상태가 같다)
	int			GetPartyState(void);
	// P2P party의 peer들의 상태를 갱신하고 갱신된 값을 반환한다.
	bool		UpdatePartyState(int* pState);
	//  파티에 참여중인 peer들 중에 Requst Addr Expire timer가 동작중인놈이 있으면 끄고 남은 시간을 반환한다.
	int			KillReqAddrTimerOfParty(void);
	void		OnTimer(HANDLE nID);
	void		RemovePeer(int nKey);
	static	int	TxPacket(PACKET_GENERIC *pPacket, SOCKADDR *pAddr);
	int			TxPacket(PACKET_GENERIC *pPacket);
	int			TxPacketToAllPeer(PACKET_GENERIC *pPacket, bool bIncludeMe=true);
	int			TxExpelPartyPacket();
	int			TxPeerAddrOkPacket(bool bAllParty);
	int			TxPeerAddrFailPacket(bool bAllParty);
	int			m_nKey;
	int			m_nState;
	bool		m_bNeedToRelay;
	SOCKADDR	m_addrLocal;
	SOCKADDR	m_addrPublic;
	int			m_nNumberOfPeer;
	int			m_nPeerKeys[DEFAULT_MAX_PEER];
	HANDLE		m_hAddrRequestExpireTimer;
};

typedef CP2PInfo*	LPCP2PInfo;
class CP2PServer : public CThread
{
public:
	static	SOCKET		GetServerSocket() { return s_pP2PServer->m_sServerSock;	}
	static	CP2PInfo*	GetP2PInfo(int nKey);
	void		OnExpireReqAddrTimer(CP2PInfo* pInfo);
	void		RelayP2pPacket(PACKET_P2P_GENERIC* pPacket);
	CP2PInfo*	AllocP2PInfo(int nKey, SOCKADDR *pAddrPublic);
	void		FreeP2PInfo(int nKey);
	void		StopOperation();
	bool		ProcessPacket(PACKET_GENERIC* pPacket, SOCKADDR* pAddr);
	UINT		Run();
	bool		Init(int nServerPort, int nMAxUser);
	CP2PServer();
	virtual ~CP2PServer();

	static CP2PServer*	s_pP2PServer;

private:
	void	ExitAllPeer(int nKey);	//같이 통신하는 모든 Peer들을 같이 제거한다.
	void	Exit(int nKey);	//P2p통신을 그만둘때,.
	void	RxReqPeerAddr(PACKET_P2P_REQ_PEER_ADDR* pInPacket, SOCKADDR* pAddr);
	SOCKET				m_sServerSock;
	int					m_nMaxUser;
	int					m_nPort;
	CPool<CP2PInfo>*	m_pPeerInfoPool;
	LPCP2PInfo*			m_ppP2PInfo;
	CRITICAL_SECTION	m_csP2p;
	CTimerManager		m_timerManager;
};

#endif // !defined(AFX_P2PSERVER_H__64356E27_13EB_494B_9E84_3E005A2F9E74__INCLUDED_)
