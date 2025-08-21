#pragma once

#include "Thread.h"
#include "GameCommons.h"
#include "Pool.h"
#include "P2PClientEx.h"
#include "TimerManager.h"
#include "ConnectionChecker.h"

#pragma warning(disable:4702) // warning C4702: unreachable code
#include <map>

#include "CustomAllocator.h"

enum	P2P_CLIENT_STATE {
	P2P_INFO_INIT=0,
	P2P_CONNECT,
};

struct CP2PInfoEx : public PoolEntry, public CLRUEntry
{
	CP2PInfoEx()
	{
		Init();
	}
	void Init()
	{
		m_nKey = -1;
		m_nRxSeq = 255;
		m_nProbePort1 = 0;
		m_nProbePort2 = 0;
		m_nState = P2P_INFO_INIT;
		memset(&m_addrLocal, 0, sizeof(SOCKADDR));
		memset(&m_addrPublic, 0, sizeof(SOCKADDR));
		m_tOrg = 0;
		m_tRec = 0;
		m_tXmt = 0;
	}
	int		SecondsConnection() { return 0;}
	void	SetAddrLocal(SOCKADDR* pAddr)
	{
		memcpy(&m_addrLocal, pAddr, sizeof(SOCKADDR));
	}
	void	SetAddrPublic(SOCKADDR* pAddr)
	{
		memcpy(&m_addrPublic, pAddr, sizeof(SOCKADDR));
	}
	//void		OnTimer(HANDLE nID);
	void		RemovePeer(int nKey);
	static	int	TxPacket(PACKET_GENERIC *pPacket, SOCKADDR *pAddr);
	int			TxPacket(PACKET_GENERIC *pPacket);
	int			TxPacketToAllPeer(PACKET_GENERIC *pPacket, bool bIncludeMe=true);
	int			TxExpelPartyPacket();
	int			TxPeerAddrOkPacket(bool bAllParty);
	int			TxPeerAddrFailPacket(bool bAllParty);
	int			m_nKey;
	int			m_nState;
	int			m_nRxSeq;
	///////////////////////////
	// Time Sync 관련
	int			m_tOrg;
	int			m_tRec;
	int			m_tXmt;
	// Time Sync 관련
	///////////////////////////
	SOCKADDR	m_addrLocal;
	SOCKADDR	m_addrPublic;
	unsigned short	m_nProbePort1;
	unsigned short	m_nProbePort2;
	in_addr 	m_Saddr1;
	in_addr 	m_Saddr2;
	SOCKADDR	m_addrProbe1;
	SOCKADDR	m_addrProbe2;
};

typedef CP2PInfoEx*	LPCP2PInfoEx;

class CP2PServerEx : public CThread, public CCriticalSection, public CConnectionChecker
{
public:
	static	SOCKET		GetServerSocket() { return s_pP2PServer->m_sServerSock;	}
	CP2PInfoEx*			AllocP2PInfo(int nKey, SOCKADDR *pAddrPublic);
	CP2PInfoEx*			GetP2PInfo(int nKey);
	void				FreeP2PInfo(int nKey);

	void				OnConnectionTimeOut(CLRUEntry* pIdleEntry);
	void				OnAcceptTimeOut(CLRUEntry* /* pIdleEntry*/){}
	bool				Init(int nServerPort, int nMAxUser);
	UINT				Run(int* pRun, int nID);
	CP2PServerEx(void);
	~CP2PServerEx(void);

	static CP2PServerEx*	s_pP2PServer;
private:
	UINT				RcvThread(int* pRun, SOCKET sServerSock);
	UINT				ChkThread(int* pRun);
	bool				ProcessPacket(PACKET_P2P_GENERIC* pPacket, SOCKADDR* pAddr);
	int					PacketHandler(PACKET_P2P_GENERIC* pPacket, SOCKADDR* pAddr);
	int					RxConnectRq(PACKET_P2P_REQ_CONNECT* pPacket, SOCKADDR* pAddr);
	int					RxHeartBeat(PACKET_P2P_GENERIC* pPacket);
	int					RxTimeAdjust(PACKET_P2P_TIME* pPacket);
	int					RxReqPeerAddr(PACKET_P2P_REQ_PEER_INFO* pPacket);
	void				RxProbe1(PACKET_P2P_HOLE_PROBE* pPacket, SOCKADDR* pAddr);
	void				RxProbe2(PACKET_P2P_HOLE_PROBE* pPacket, SOCKADDR* pAddr);
	void				StopOperation();


	CPool<CP2PInfoEx>*	m_pPeerInfoPool;
	typedef std::map<int, LPCP2PInfoEx, std::less<int>, BroCustomAllocator<int> > P2PINFOMAP;
	P2PINFOMAP			m_pP2PInfoMap;
	SOCKET				m_sServerSock;
	int					m_nPort;
	SOCKET				m_sServerProbeSock1;
	int					m_nProbePort1;
	SOCKET				m_sServerProbeSock2;
	int					m_nProbePort2;
	int					m_nMaxUser;
//	CTimerManager		m_timerManager;
//	CConnectionChecker	m_connectionChecker;
	CMiliSecResolutionTick		m_watch;
};
