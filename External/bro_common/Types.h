#pragma once
#include "Pool.h"

enum	P2P_SESSION_STAGE {
	//////////////////////////////////////
	//Peer Session상태
	P2P_SESSION_INIT=0,
	P2P_SESSION_SET_ADDR,
	P2P_SESSION_DIRECT_TRY,
	P2P_SESSION_HOLE_PUNCHING_REQ,	//최초의 요구 packet을 보낸 상태
	P2P_SESSION_HOLE_PUNCHING_TRY0,
	P2P_SESSION_HOLE_PUNCHING_TRY1,
	P2P_SESSION_HOLE_PUNCHING_TRY2,
	P2P_SESSION_HOLE_SCAN0,
	P2P_SESSION_HOLE_SCAN1,
	P2P_SESSION_HOLE_SCAN2,
	P2P_SESSION_DIRECT_ESTABLISHED,
	P2P_SESSION_HOLE_PUNCHING_ESTABLISHED,
	P2P_SESSION_DISCONNECTED,
	//////////////////////////////////////
	//Server session상태
	P2P_SVR_INIT,
	P2P_SVR_CONNECT_TRY,
	P2P_SVR_CONNECT,
	P2P_SVR_ADDR_TRY,
	P2P_SVR_ADDR_OK,
	P2P_SVR_ADDR_FAIL,
	P2P_SVR_EXIT_TRY,
	P2P_SVR_EXIT_OK,
	P2P_SVR_EXIT_FAIL,
	P2P_SVR_EXIT_ALL_TRY,
	P2P_SVR_EXIT_ALL_OK,
	P2P_SVR_EXIT_ALL_FAIL,
	P2P_SVR_DISCONNECT,
	P2P_SVR_EXPELED,
	P2P_SVR_ADDR_PARTIALLY_OK,
};

class CP2PPacketBuffer : public PoolEntry
{
public:
	char	m_pBuffer[512];
	int		m_nTxTime;
};

typedef	CP2PPacketBuffer*	LPCP2PPacketBuffer;

struct STRUCT_PEER_INFO{
	STRUCT_PEER_INFO(STRUCT_PEER_INFO& a)
	{
		m_nKey = a.m_nKey;
		memcpy(&m_addrLocal, &(a.m_addrLocal), sizeof(SOCKADDR));
		memcpy(&m_addrPublic, &(a.m_addrPublic), sizeof(SOCKADDR));
		m_nTurnArroundTime = a.m_nTurnArroundTime;
		m_nTimerID = a.m_nTimerID;
		m_nStage = a.m_nStage;
		m_bInSameNAT = a.m_bInSameNAT;//같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
		m_nSeq = a.m_nSeq;
		m_nAckSeq = a.m_nAckSeq;
		m_nRxSeq = a.m_nRxSeq;
		m_nRetryCount = a.m_nRetryCount;
		m_nBeginPort = a.m_nBeginPort;
		m_nEndPort = a.m_nEndPort;
		memcpy(m_pBuffer, a.m_pBuffer, 256*sizeof(LPCP2PPacketBuffer));
	}
	STRUCT_PEER_INFO()
	{
		Init();
	}

	void Init()
	{
		m_nKey = -1;
		memset(&m_addrLocal, 0, sizeof(SOCKADDR));
		memset(&m_addrPublic, 0, sizeof(SOCKADDR));
		m_nTurnArroundTime = 0;
		m_nTimerID = 0;
		m_nStage = P2P_SESSION_INIT;
		m_bInSameNAT = false;//같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
		m_nSeq = 0;
		m_nAckSeq = 0;
		m_nRxSeq = 255;
		m_nRetryCount = 0;
		m_nBeginPort = 0;
		m_nEndPort = 0;
		memset(m_pBuffer, 0, 256*sizeof(LPCP2PPacketBuffer));
	}
	bool	IsConnected()
	{
		if ((P2P_SESSION_DIRECT_ESTABLISHED == m_nStage) ||
			(P2P_SESSION_HOLE_PUNCHING_ESTABLISHED == m_nStage) ) {
				return true;
			}
			return false;
	}
	bool	IsDisconnected()
	{
		if (P2P_SESSION_DISCONNECTED == m_nStage) {
			return true;
		}
		return false;
	}
	bool	IsAddressSet()
	{
		if (P2P_SESSION_SET_ADDR <= m_nStage) {
			return true;
		}
		return false;
	}
	void	SetAddrLocal(SOCKADDR* pAddr)
	{
		if (NULL == pAddr) return;
		memcpy(&m_addrLocal, pAddr, sizeof(SOCKADDR));
	}
	void	SetAddrPublic(SOCKADDR* pAddr)
	{
		if (NULL == pAddr) return;
		memcpy(&m_addrPublic, pAddr, sizeof(SOCKADDR));
	}
	void	KillTimer();
	STRUCT_PEER_INFO operator=(STRUCT_PEER_INFO& value)
	{
		m_nKey = value.m_nKey;
		memcpy(&m_addrLocal, &(value.m_addrLocal), sizeof(SOCKADDR));
		memcpy(&m_addrPublic, &(value.m_addrPublic), sizeof(SOCKADDR));
		m_nTurnArroundTime = value.m_nTurnArroundTime;
		m_nTimerID = value.m_nTimerID;
		m_nStage = value.m_nStage;
		m_bInSameNAT = value.m_bInSameNAT;	////같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
		m_nSeq = value.m_nSeq;
		m_nAckSeq = value.m_nAckSeq;
		m_nRxSeq = value.m_nRxSeq;
		m_nRetryCount = value.m_nRetryCount;
		m_nBeginPort = value.m_nBeginPort;
		m_nEndPort = value.m_nEndPort;
		memcpy(m_pBuffer, value.m_pBuffer, 256*sizeof(LPCP2PPacketBuffer));
		return *this;
	}
	int			m_nKey;
	SOCKADDR	m_addrLocal;
	SOCKADDR	m_addrPublic;
	int			m_nTurnArroundTime;
	int			m_nTimerID;
	int			m_nStage;
	bool		m_bInSameNAT;//같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
	BYTE		m_nSeq;		// 다음에 보낼 패킷의 sequence number (앞의 패킷이 아직 전송 확인된 상황이 아닌경우 가장 최근에 큐에 달아 놓은 패킷의 번호이다.)
	BYTE		m_nAckSeq;	// Ack를 기다리고 있는 패킷의 번호.(실제로 직전에 네트워크를 타고 날라간 패킷의 번호이다)
	BYTE		m_nRxSeq;	// 마지막으로 수신한 패킷의 번호(상대방이 ACK를 받지 못해 재전송 하더라도 중복 처리하지 않기 위해 사용한다.)
	BYTE		m_nRetryCount;
	unsigned short	m_nBeginPort;
	unsigned short	m_nEndPort;
	LPCP2PPacketBuffer	m_pBuffer[256];
};
