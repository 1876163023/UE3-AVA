#pragma once

#include "Thread.h"
#include "GameCommons.h"
#include "TimerManager.h"

#pragma comment(lib, "winmm.lib")

#define	DEFAULT_MAX_PEER		8
#define	DEFAULT_P2P_BUFFER_SIZE	4096

#define	P2P_WINDOW_SIZE					16

#define P2P_FLAG_HAND_SHAKE				1	//던지는 쪽에서 hand shaking을 위해 buffer에 저장하고 timer를 동작시킨다.
#define P2P_FLAG_USER_DEFINED_ACK		2	//hand shaking시 받는 쪽에서 별도의 ACK처리 루틴을 가지고 있을때
#define	P2P_FLAG_PIGGY_BACK				4
#define	P2P_FLAG_TX_SERVER				8	//Server에게 던지라는 의미이다. ex) 이 flag가 켜져 있고 dest key가 다른 peer이면 key에 해당하는 peer에게도 던지고, server에게도 던진다.
#define	P2P_FLAG_RELAY					16	//Server를 거쳐서 던지라는 의미이다.	ex) 이 flag가 켜져 있으면 dest key가 다른 peer라도 Server에게만 던진다. P2P_FLAG_TX_SERVER와 같이 사용하면 안된다.

/////////////////////////////////////////
#define P2P_PROCESS_OK					1	//Handler에서 특별히 응답 패킷을 처리하지 않을 경우
#define P2P_PROCESS_SEND_ACK		2	//Handler에서 PiggyBack으로 ACK를 보태서 처리했을경우 이값을 return한다.
#define P2P_PROCESS_NOTIFY_USER		4	//User Handler를 실행 시킬 필요가 있을때 default Proc에서 처리
#define P2P_PROCESS_TIMER				8	//Handler에서 Timer를 처리하여 더이상 Timer를 처리할 필요가 없다.
#define P2P_PROCESS_IGNORE_TIME_ADJUST				16	//Handler에서 Timer를 처리하여 더이상 Timer를 처리할 필요가 없다.
/////////////////////////////////////////

#define P2P_TX_RETRY_COUNT					3
#define P2P_RETRANSMIT_INTERVAL			1000 // [!] 20070502 3000 // [!] 20070410 // 200
#define P2P_PUNCHHOLE_RETRY_INTERVAL	1000 // [!] 20070502 1000
#define P2P_DIRECT_RQ_INTERVAL				5000 // [!] 20070527 500
#define P2P_RELAY_PREPARE_INTERVAL			5000 // [!] 20070502 3000

enum	P2P_SESSION_STAGE {
	//////////////////////////////////////
	//Peer Session상태
	P2P_SESSION_INIT=0,
	P2P_SESSION_DIRECT_TRY0,
	P2P_SESSION_DIRECT_TRY1,
	P2P_SESSION_DIRECT_TRY2,
	P2P_SESSION_HOLE_PUNCHING_REQ,	//최초의 요구 packet을 보낸 상태
	P2P_SESSION_HOLE_PUNCHING_TRY0,
	P2P_SESSION_HOLE_PUNCHING_TRY1,
	P2P_SESSION_HOLE_PUNCHING_TRY2,
	P2P_SESSION_HOLE_SCAN0,
	P2P_SESSION_HOLE_SCAN1,
	P2P_SESSION_HOLE_SCAN2,
	P2P_SESSION_DIRECT_ESTABLISHED,
	P2P_SESSION_HOLE_PUNCHING_ESTABLISHED,
	P2P_SESSION_RELAY_PREPARE,
	P2P_SESSION_RELAY_ESTABLISHED,
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

/* forward decls. */
class PeerInfoMgr;
class CP2PHandler;
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
		heartbeatCount = 0; // 20070129 dEAthcURe
		timeBeginHeartbeat = 0x0; // 20070130 dEAthcURe
		bUpdated = false; // 20070220 dEAthcURe
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
		m_nPunchHoleTimerID = 0;
		m_nHoleScanTimerID = 0;
		m_nRelayPrepareTimer = 0;
		m_nStage = P2P_SESSION_INIT;
		m_bInSameNAT = false;//같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
		m_nSeq = 0;
		m_nAckSeq = 0;
		m_nRxSeq = 255;
		m_nRetryCount = 0;
		m_nBeginPort = 0;
		m_nEndPort = 0;
		memset(m_pBuffer, 0, 256*sizeof(LPCP2PPacketBuffer));
		heartbeatCount = 0; // 20070129 dEAthcURe
		timeBeginHeartbeat = 0; // 20070130 dEAthcURe
		bUpdated = false; // 20070220 dEAthcURe
	}
	bool	IsConnected();
	bool	IsDisconnected()
	{
		if (P2P_SESSION_DISCONNECTED == m_nStage) {
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
	void	KillAckTimer();
	void	KillAllTimer();
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
		bUpdated = value.bUpdated; // 20070220
		return *this;
	}
	int			m_nKey;
	SOCKADDR	m_addrLocal;
	SOCKADDR	m_addrPublic;
	int			m_nTurnArroundTime;
	int			m_nTimerID;
	int			m_nDirectRqTimerID;
	int			m_nPunchHoleTimerID;
	int			m_nHoleScanTimerID;
	int			m_nRelayPrepareTimer;
	int			m_nStage;
	bool		m_bInSameNAT;//같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
	BYTE		m_nSeq;		// 다음에 보낼 패킷의 sequence number (앞의 패킷이 아직 전송 확인된 상황이 아닌경우 가장 최근에 큐에 달아 놓은 패킷의 번호이다.)
	BYTE		m_nAckSeq;	// Ack를 기다리고 있는 패킷의 번호.(실제로 직전에 네트워크를 타고 날라간 패킷의 번호이다)
	BYTE		m_nRxSeq;	// 마지막으로 수신한 패킷의 번호(상대방이 ACK를 받지 못해 재전송 하더라도 중복 처리하지 않기 위해 사용한다.)
	BYTE		m_nRetryCount;
	unsigned short	m_nBeginPort;
	unsigned short	m_nEndPort;
	LPCP2PPacketBuffer	m_pBuffer[256];
	int heartbeatCount; // 20070129 dEAthcURe
	time_t timeBeginHeartbeat; // 20070130 dEAthcURe
	bool bUpdated; // 20070220 dEAthcURe
};

#ifdef EnableP2pConn
#define EnableEmbedding
#endif

// {{ 20070130 dEAthcURe log stuff
#define LogFileName ".\\log\\p2pConn.txt"
#define LogToFile _logToFile
// }} 20070130 dEAthcURe log stuff

#define UseHeartbeatV2
#ifdef UseHeartbeatV2
#define Heartbeat() TxHeartBeatV2()
#else
#define Heartbeat() TxHeartBeat()
#endif

#define NumAppPacketToStopHeartbeating 3 // [+] 20070130 dEAthcURe

class CP2PClientEx : public CThread, public CCriticalSection
{
public:
	#ifdef EnableEmbedding
	int nAppPacketToStopHeartbeating; // [+] 20070130 dEAthcURe
	// {{ [+] 20060915 dEAthcURe	
	bool bEmbedded;
	bool bGetPeerAddr;
	bool bHeartbeat;
	// }} [+] 20060915 dEAthcURe		
	void RxPacket(void);
	void TxHeartbeatToServer(void); // 20070221 dEAthcURe
	void TxHeartBeat(void);
	void TxHeartBeatV2(void);	
	void TxRttProbReq(int idAccount); // [+] 20070410 dEAthcURe
	void TxRttProbAck(void); // [+] 20070410 dEAthcURe
	#endif

	static void CALLBACK StaticOnTimer(UINT uID, UINT/* uMsg*/, DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/);
	virtual bool		ProcessP2PPacket(PACKET_P2P_GENERIC *pPacket, SOCKADDR* pAddr);
	int					GetServerTick()
	{
		return (m_tServerOffset + m_watch.GetTick());
	}
	void				SetRetransmitInterval(int val) { m_dwRetransmitInterval = val; }
	void				SetUseTurnMode(bool bUseTurn)
	{
		m_bUseTURN = bUseTurn;
	}
	bool				IsTryingTurn() { return m_bUseTURN; }
	void				OnTimer(UINT nID);
	void				RegisterPeer(int nKey, SOCKADDR* pAddr=NULL);	//통신할 Peer등록
	void				RegisterPeer(int nKey, SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic); // [+] 20070410 dEAthcURe
	void				RemovePeer(int nKey);
	void				RemoveAllPeer();
	int					RequestPeerAddr(int nKey=-1);
	int					RequestConnectServer(void);
	bool				RequestConnectPeer(int idPeer);
	void				RequestConnectPeers();
	int					RequestExit();
	int					RequestExitWithAllPeers();
	void				Disconnect(int nKey);
	void				Disconnect(SOCKADDR *pAddr);
	void				StopOperation();
	bool				Init(int nKey, char* strServerIP, int nServerPort, CP2PHandler* pHandler, CP2PHandler* pLossHandler, CP2PHandler* pNotifier);

	// {{ [+] 20060918 dEAthcURe 
	#ifdef EnableEmbedding
	void				_logToFile(char* fmt, ...) ;
	bool				InitEmbedding(int nKey, unsigned int nServerIP, int nServerPort, SOCKET sd, int localPort, CP2PHandler* pHandler, CP2PHandler* pLossHandler, CP2PHandler* pNotifier); // [+] 20060915
	#endif
	// }} [+] 20060918 dEAthcURe 

	UINT				Run(int* pRun, int nID);
	int					TxP2pPacket(PACKET_P2P_GENERIC* pPacket,int bLock =true);
	int					TxP2pPacket(STRUCT_PEER_INFO* pi, PACKET_P2P_GENERIC* pPacket, int nServerTick);
	int					TxP2pPacket(int destKey, PACKET_P2P_GENERIC* pPacket, int nServerTick);
	STRUCT_PEER_INFO*	GetPeerInfo(int nPeerKey);
	int					GetKey() { return m_nKey;}
	int					GetPeerCount();

	CP2PClientEx(int nPort, int nMaxPeer, int retransmitInterval);
	~CP2PClientEx(void);
protected: // [!] 20060918 dEAthcURe private 
	void	ClearSessionBuffer(STRUCT_PEER_INFO* pStruct);
	void	RxPeerAddrOk(PACKET_P2P_PEER_INFO_OK *pPacket);
	void	RxDirectRq(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pStruct, SOCKADDR* pAddr);
	void	RxDirectAck(STRUCT_PEER_INFO* pStruct, SOCKADDR* pAddr);
	void	RxDirectConfirm(STRUCT_PEER_INFO* pStruct, SOCKADDR* pAddr);
	void	RequestHolePunching(STRUCT_PEER_INFO* pStruct);	//1.peer에게 MSGID_P2P_REQ_PUNCHHOLE 를 날려서 hole punching을 요구한다.
	void	RxRqHolePunching(PACKET_P2P_RQ_HOLE_PUNCH* pPacket, STRUCT_PEER_INFO* pStruct);	//2. MSGID_P2P_REQ_PUNCHHOLE을 받으면 Server의 Probe port들과 요구한 peer들에 packet을 날린다.
	void	RxPunch(STRUCT_PEER_INFO* pPeerInfo, PACKET_P2P_HOLE_PUNCH* pPacket);	// 3. MSGID_P2P_PUNCH를 받으면 port범위를 살펴보고 둟는다.
	void	RxHoleScan(STRUCT_PEER_INFO* pPeerInfo, PACKET_P2P_HOLE_SCAN* pPacket, SOCKADDR* pAddr);	// 4. MSGID_P2P_SCAN을 받으면 connect되지 않은 session이라면 connect상태로 설정하고 ACK를 보낸다.
	void	RxHolePunchAck(STRUCT_PEER_INFO* pPeerInfo, SOCKADDR* pAddr);
	void	RxTimeAdjust(STRUCT_PEER_INFO* pSender, PACKET_P2P_TIME * pPacket);	//time sync packet처리
	void	RxPacket(int* pRun);
	void	TxHeartBeat(int* pRun);	
	int		TxDirectRq(STRUCT_PEER_INFO* pStruct);
	void	TxPunchHoleProbePacket(STRUCT_PEER_INFO* pStruct);
	void	TxHoleScan(STRUCT_PEER_INFO* pPeerInfo);
	int		DefaultPacketProc(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pSender, SOCKADDR* pAddr);
	int		DefaultTimerProc(UINT nID, STRUCT_PEER_INFO* pPeerInfo);
	int		DefaultPacketLossProc(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pPeer);
	int		TxReliablePacket(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pDest, int nCurrent);
	int		TxVolatilePacket(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pDest);
	int		TxPacket(PACKET_P2P_GENERIC *pOutPacket, SOCKADDR* pAddr);	
	inline bool	IsPeerSpecified(int key);
	int		CreateTimeSetEventForTx();
	int					m_nKey;
	int					m_nCurrentPeer;
	int					m_nMaxPeer;
public: // 20070122 dEAthcURe
	int					m_nPort;
private: // 20070122 dEAthcURe

public: // 20070122 dEAthcURe
	int					m_nDropRx;		//for Test
	int					m_nDropTx;		//for Test
	bool				m_bSuspendRx;	//for Test
	bool				m_bSuspendTx;	//for Test
protected: // 20070122 dEAthcURe // private
	bool				m_bUseTURN;
	int					m_dwRetransmitInterval;
	//////////////////////////////////////////////
	// time sync 관련
	int					m_nTimeAdjustRetry;
	int					m_tOrg;
	int					m_tRec;
	int					m_tXmt;

	int					m_tRxPacket;
	int					m_tDelta0;
	//int				m_tTheta0;
	int					m_tRoundTripDelay[8];
	//int				m_tClockOffset[8];
	int					m_tServerOffset;
	int					m_nTickIndex;
	//////////////////////////////////////////////
	HANDLE				m_evtHeartbeat;
	SOCKET				m_sRcvSocket;
	SOCKADDR			m_addrLocal;	//같은 NAT인지 판별하기위해 사용
	SOCKADDR			m_addrPublic;	//같은 NAT인지 판별하기위해 사용
	CP2PHandler*		m_pHandler;
	CP2PHandler*		m_pLossHandler;
	CP2PHandler*		m_pNotifier;
	STRUCT_PEER_INFO	m_serverInfo;
	STRUCT_PEER_INFO	m_serverProbe1Info;
	STRUCT_PEER_INFO	m_serverProbe2Info;
	PeerInfoMgr*		m_pPeerInfoMgr;
	HANDLE				m_heartbeatThreadTerminateEvent;
	CPool<CP2PPacketBuffer>		m_packetBuffer;
	CMiliSecResolutionTick		m_watch;
	void ClearSession(STRUCT_PEER_INFO* pStruct);
	void GetLogMsgByMsgID(MSGID msgid, TCHAR* buf, size_t size);
public:
	bool IsAllPeerConnected(void);
};
