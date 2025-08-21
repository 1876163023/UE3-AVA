#include "stdafx.h"
#include "limits.h"
#pragma warning(disable:4995)
#include <assert.h>

#pragma warning(disable:4201)
#include "MMSystem.h"
#pragma warning(default:4201)
#include "p2pclientex.h"
#include "P2PHandler.h"
#include "PeerInfoMgr.h"
#ifdef _BF_CLIENT
#include <NtixCore/LogManager.h>
#endif

bool STRUCT_PEER_INFO::IsConnected()
{	
	if ((P2P_SESSION_DIRECT_ESTABLISHED == m_nStage) ||
		(P2P_SESSION_RELAY_ESTABLISHED == m_nStage) ||
		(P2P_SESSION_HOLE_PUNCHING_ESTABLISHED == m_nStage) ) {
			return true;
		}
		return false;
}

void STRUCT_PEER_INFO::KillAckTimer()
{
	if (m_nTimerID) {
		timeKillEvent(m_nTimerID);
		m_nTimerID = 0;
	}
}

void STRUCT_PEER_INFO::KillAllTimer()
{
	if (m_nTimerID) {
		timeKillEvent(m_nTimerID);
		m_nTimerID = 0;
	}
	if (m_nDirectRqTimerID) {
		timeKillEvent(m_nDirectRqTimerID);
		m_nDirectRqTimerID = 0;
	}
	if (m_nPunchHoleTimerID) {
		timeKillEvent(m_nPunchHoleTimerID);
		m_nPunchHoleTimerID = 0;
	}
	if (m_nHoleScanTimerID) {
		timeKillEvent(m_nHoleScanTimerID);
		m_nHoleScanTimerID = 0;
	}
	if (m_nRelayPrepareTimer) {
		timeKillEvent(m_nRelayPrepareTimer);
		m_nRelayPrepareTimer = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
CP2PClientEx::CP2PClientEx(int nPort, int nMaxPeer, int retransmitInterval):m_packetBuffer(nMaxPeer*64)
{
	#ifdef EnableEmbedding
	nAppPacketToStopHeartbeating = NumAppPacketToStopHeartbeating;
	bHeartbeat = false;	
	bEmbedded = false; 
	#endif

	m_dwRetransmitInterval = -1;
	m_dwRetransmitInterval = retransmitInterval;
	m_nCurrentPeer = 0;
	m_nKey = -1;
	m_nPort = nPort;
	m_sRcvSocket = INVALID_SOCKET;
	m_nMaxPeer = nMaxPeer;
	m_pHandler = NULL;
	m_pLossHandler = NULL;
	m_pNotifier = NULL;
	m_nDropRx = 0;
	m_nDropTx = 0;
	m_bSuspendRx = false;
	m_bSuspendTx = false;
	m_tServerOffset = 0;
	m_bUseTURN = false;
	m_pPeerInfoMgr = new PeerInfoMgr;
	m_heartbeatThreadTerminateEvent = CreateEvent(NULL, TRUE, FALSE, _T(""));
}

CP2PClientEx::~CP2PClientEx(void)
{
	TLock lo(this);

	if (m_serverInfo.m_nStage >= P2P_SVR_CONNECT) {
		RequestExit();
		//Sleep(1000);
		StopOperation();
	}
	SAFE_DELETE(m_pPeerInfoMgr);
	SAFE_DELETE(m_pHandler);
	SAFE_DELETE(m_pLossHandler);
	SAFE_DELETE(m_pNotifier);
	CloseHandle(m_heartbeatThreadTerminateEvent);
}
STRUCT_PEER_INFO* CP2PClientEx::GetPeerInfo(int nPeerKey)
{
	if (P2P_DEST_NONE == nPeerKey) {
		return &m_serverInfo;
	}

	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0x0;
	}
	return m_pPeerInfoMgr->GetPeerInfo(nPeerKey);
}

void CP2PClientEx::StaticOnTimer(UINT uID, UINT /*uMsg*/, DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
{
	CP2PClientEx*	pP2pClient = (CP2PClientEx *)dwUser;
	if (NULL == pP2pClient) return;

	__try {
		pP2pClient->OnTimer(uID);	
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		unsigned long ec = GetExceptionCode();
		if(ec == EXCEPTION_ACCESS_VIOLATION) 
			SystemPrint(ERROR_LOG, "-SEH-[CP2PClientEx::StaticOnTimer] EXCEPTION_ACCESS_VIOLATION has occured\n");
		else 
			SystemPrint(ERROR_LOG, "-SEH-[CP2PClientEx::StaticOnTimer] exception:%d has occured\n", ec);
	}	
}

bool CP2PClientEx::Init(int nKey, char* strServerIP, int nServerPort, CP2PHandler* pHandler, CP2PHandler* pLossHandler, CP2PHandler* pNotifier)
{
	SOCKADDR_IN	addr;
	SAFE_DELETE(m_pHandler);
	m_pHandler = pHandler;
	SAFE_DELETE(m_pLossHandler);
	m_pLossHandler = pLossHandler;
	SAFE_DELETE(m_pNotifier);
	m_pNotifier = pNotifier;

	m_nKey = nKey;
	m_sRcvSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_sRcvSocket == INVALID_SOCKET){
		StopOperation();
		return false;
	}

	SystemPrint(ERROR_LOG, "sd=%d\n", m_sRcvSocket);

	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	DWORD status;
	// disable  new behavior using
	// IOCTL: SIO_UDP_CONNRESET
	status = WSAIoctl(m_sRcvSocket, SIO_UDP_CONNRESET,
		&bNewBehavior, sizeof(bNewBehavior),
		NULL, 0, &dwBytesReturned,
		NULL, NULL);


	////////////////////////////////////////////////////////////////////////////////////////////
	// Socket Option 설정 2005.08.11
	int opt = 16*1024;
	int	nTimeOut = 10;
	int	nReuse = TRUE;
	if(setsockopt(m_sRcvSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nReuse, sizeof(int)) < 0)
	{
		SystemPrint(ERROR_LOG, "CServerInterface::PacketTransmitter, SO_REUSEADDR error\n");
	}
	if(setsockopt(m_sRcvSocket, SOL_SOCKET, SO_RCVBUF, (char *)&opt, sizeof(int)) < 0)
	{
		SystemPrint(ERROR_LOG, "CServerInterface::PacketTransmitter, enlarging SNDBUF error\n");
	}
	if(setsockopt(m_sRcvSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeOut, sizeof(int)) < 0)
	{
		SystemPrint(ERROR_LOG, "CServerInterface::PacketTransmitter, SO_SNDTIMEO error\n");
	}
	{
		int		sndBufSize = 0;
		int		nOptLen = sizeof(int);
		getsockopt(m_sRcvSocket,SOL_SOCKET, SO_SNDBUF, (char *)&sndBufSize, &nOptLen);
		SystemPrint(2,"P2P Socket Send buffer Size = %d\n", sndBufSize );

		sndBufSize = 8096*2;
		setsockopt(m_sRcvSocket,SOL_SOCKET, SO_SNDBUF, (char *)&sndBufSize, sizeof(int));

		getsockopt(m_sRcvSocket,SOL_SOCKET, SO_SNDBUF, (char *)&sndBufSize, &nOptLen);
		SystemPrint(2,"P2P Socket Send buffer Size = %d\n", sndBufSize );
	}
	// Socket Option 설정 2005.08.11
	////////////////////////////////////////////////////////////////////////////////////////////

	memset(&addr, 0, sizeof(SOCKADDR));
	addr.sin_family				= AF_INET;
	addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	addr.sin_port				= htons((u_short)m_nPort);

	if (bind(m_sRcvSocket, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){
		StopOperation();
		return false;
	}
	SOCKADDR_IN	addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(strServerIP);
	addrServer.sin_port = htons((u_short)nServerPort);

	m_serverInfo.SetAddrLocal((SOCKADDR *)&addrServer);
	m_serverInfo.SetAddrPublic((SOCKADDR *)&addrServer);
	m_serverInfo.m_nKey = P2P_DEST_NONE;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(strServerIP);
	addrServer.sin_port = htons((u_short)(nServerPort+1));
	m_serverProbe1Info.SetAddrLocal((SOCKADDR *)&addrServer);
	m_serverProbe1Info.SetAddrPublic((SOCKADDR *)&addrServer);

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(strServerIP);
	addrServer.sin_port = htons((u_short)(nServerPort+2));
	m_serverProbe2Info.SetAddrLocal((SOCKADDR *)&addrServer);
	m_serverProbe2Info.SetAddrPublic((SOCKADDR *)&addrServer);
	//////////////////////////////////////////////////////////////////////////////
	// 같은 NAT내 통신을 위해 나의 local address를 구한다.
	char strHostname[256];
	if (SOCKET_ERROR == gethostname(strHostname, 256)) {
		assert(!"getsockname에서 오류발생,무슨 에러지?");
		return false;
	}
	SystemPrint(2, "hostname=%s\n", strHostname);
	hostent*	pHostEnt = gethostbyname(strHostname);
	if (NULL == pHostEnt) {
		assert(!"gethostbyname에서 오류발생,무슨 에러지?");
		return false;
	}
	SystemPrint(2, "h_name=%s, h_aliases=%s\n", pHostEnt->h_name, pHostEnt->h_aliases);

	SOCKADDR_IN*	pSockAddr = (SOCKADDR_IN *)&m_addrLocal;
	memset(pSockAddr->sin_zero, 0, 8*sizeof(char));
	pSockAddr->sin_addr.s_addr = *((int*)pHostEnt->h_addr_list[0]);
	SystemPrint(2, "IP address=%s\n", inet_ntoa(pSockAddr->sin_addr));
	pSockAddr->sin_port = htons((u_short)m_nPort);
	pSockAddr->sin_family				= AF_INET;
	// 같은 NAT내 통신을 위해 나의 local address를 구한다.
	//////////////////////////////////////////////////////////////////////////////
	BeginThread();
	RequestConnectServer();
	return true;
}

#ifdef EnableEmbedding

void CP2PClientEx::_logToFile(char* fmt, ...) 
{
	FILE* fp = fopen(LogFileName, "at");
	if(fp) {		
		va_list argPtr;
		va_start( argPtr, fmt );
		vfprintf(fp, fmt, argPtr);
		fprintf(fp, "\n");
		va_end( argPtr );
		fclose(fp);
	}
}

bool CP2PClientEx::InitEmbedding(int nKey, unsigned int nServerIP, int nServerPort, SOCKET sd, int localPort, CP2PHandler* pHandler, CP2PHandler* pLossHandler, CP2PHandler* pNotifier)
{
	// 20060915 dEAthcURe
	// Unreal과 같은 외부 엔진에 임베딩하기 위해 사용한다.

	m_nPort = localPort;
	SOCKADDR_IN	addr;
	SAFE_DELETE(m_pHandler);
	m_pHandler = pHandler;
	SAFE_DELETE(m_pLossHandler);
	m_pLossHandler = pLossHandler;
	SAFE_DELETE(m_pNotifier);
	m_pNotifier = pNotifier;

	m_nKey = nKey;
	if (sd == INVALID_SOCKET) {
		StopOperation();
		return false;
	}
	m_sRcvSocket = sd;	
	SystemPrint(ERROR_LOG, "sd=%d\n", m_sRcvSocket);

	bEmbedded = true;

	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	DWORD status;

	SOCKADDR_IN	addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = nServerIP ? ::htonl(nServerIP) : INADDR_ANY;
	addrServer.sin_port = htons((u_short)nServerPort);

	m_serverInfo.SetAddrLocal((SOCKADDR *)&addrServer);
	m_serverInfo.SetAddrPublic((SOCKADDR *)&addrServer);
	m_serverInfo.m_nKey = P2P_DEST_NONE;

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = nServerIP ? ::htonl(nServerIP) : INADDR_ANY;
	addrServer.sin_port = htons((u_short)(nServerPort+1));
	m_serverProbe1Info.SetAddrLocal((SOCKADDR *)&addrServer);
	m_serverProbe1Info.SetAddrPublic((SOCKADDR *)&addrServer);

	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = nServerIP ? ::htonl(nServerIP) : INADDR_ANY;
	addrServer.sin_port = htons((u_short)(nServerPort+2));
	m_serverProbe2Info.SetAddrLocal((SOCKADDR *)&addrServer);
	m_serverProbe2Info.SetAddrPublic((SOCKADDR *)&addrServer);
	//////////////////////////////////////////////////////////////////////////////
	// 같은 NAT내 통신을 위해 나의 local address를 구한다.
	char strHostname[256];
	if (SOCKET_ERROR == gethostname(strHostname, 256)) {
		assert(!"getsockname에서 오류발생,무슨 에러지?");
		return false;
	}
	SystemPrint(2, "hostname=%s", strHostname);
	hostent*	pHostEnt = gethostbyname(strHostname);
	if (NULL == pHostEnt) {
		assert(!"gethostbyname에서 오류발생,무슨 에러지?");
		return false;
	}
	SystemPrint(2, "h_name=%s, h_aliases=%s", pHostEnt->h_name, pHostEnt->h_aliases);

	SOCKADDR_IN*	pSockAddr = (SOCKADDR_IN *)&m_addrLocal;
	memset(pSockAddr->sin_zero, 0, 8*sizeof(char));
	pSockAddr->sin_addr.s_addr = *((int*)pHostEnt->h_addr_list[0]);
	SystemPrint(2, "IP address=%s", inet_ntoa(pSockAddr->sin_addr));
	pSockAddr->sin_port = htons((u_short)localPort);
	pSockAddr->sin_family				= AF_INET;
	// 같은 NAT내 통신을 위해 나의 local address를 구한다.
	//////////////////////////////////////////////////////////////////////////////
	// BeginThread(); // [-] 20060915 dEAthcURe 이 부분이 thread가 아니라 tick으로 대체된다.
	RequestConnectServer();
	return true;
}
#endif

int CP2PClientEx::TxP2pPacket(PACKET_P2P_GENERIC* pPacket,int /*bLock*/)
{
	bool	bTimeStamp=false;
	int		nCurrent=GetServerTick();
	TLock lo(this);

	if(pPacket == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	BYTE	m_bFlagOrg = pPacket->m_bFlag;
	if (P2P_FLAG_HAND_SHAKE&(pPacket->m_bFlag)) {
		bTimeStamp=true;
		//nCurrent=GetServerTick();
	}
	pPacket->m_nTick = nCurrent;	//모든 packet에 tick이 찍히도록하자.
	if (P2P_DEST_ALL == pPacket->m_nDestPeerKey) {
		const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
		{
			STRUCT_PEER_INFO* pi = it->second;

			if(pi) { // 20070522 dEAthcURe pi null check
				if (P2P_SESSION_DISCONNECTED == pi->m_nStage) continue;
				if (P2P_SESSION_RELAY_ESTABLISHED == pi->m_nStage) {
					pPacket->m_bFlag |= P2P_FLAG_RELAY;
					pPacket->m_nDestPeerKey = pi->m_nKey;        //추가
				}
				if (bTimeStamp) {
					TxReliablePacket(pPacket, pi,nCurrent);
				} else {
					TxVolatilePacket(pPacket, pi);
				}
				pPacket->m_bFlag = m_bFlagOrg;
			} // 20070522 dEAthcURe pi null check
		}		
		pPacket->m_nDestPeerKey = P2P_DEST_ALL;         //추가
	} else if (P2P_DEST_NONE != pPacket->m_nDestPeerKey) {
		STRUCT_PEER_INFO*	pPeerInfo = GetPeerInfo(pPacket->m_nDestPeerKey);
		if (NULL == pPeerInfo){ 
			return 0;
		}
		if (P2P_SESSION_DISCONNECTED == pPeerInfo->m_nStage) {
			return 0;
		}
		if (P2P_SESSION_RELAY_ESTABLISHED == pPeerInfo->m_nStage) {
			pPacket->m_bFlag |= P2P_FLAG_RELAY;
		}
		if (bTimeStamp) {
			TxReliablePacket(pPacket, pPeerInfo, nCurrent);
		} else {
			TxVolatilePacket(pPacket, pPeerInfo);
		}
		pPacket->m_bFlag = m_bFlagOrg;
	}
	if (P2P_FLAG_TX_SERVER&pPacket->m_bFlag) {
		if (P2P_SESSION_DISCONNECTED == m_serverInfo.m_nStage){
			return 0;
		}
		if (bTimeStamp) {
			TxReliablePacket(pPacket, &m_serverInfo, nCurrent);
		} else {
			TxVolatilePacket(pPacket, &m_serverInfo);
		}
	}
	return pPacket->Length();
}
int CP2PClientEx::TxP2pPacket(STRUCT_PEER_INFO* pi, PACKET_P2P_GENERIC* pPacket, int nServerTick)
{
	if(pi == 0x0 || pPacket == 0x0) {	// [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	TLock lo(this);
	if (P2P_SESSION_DISCONNECTED == pi->m_nStage) 
	{
		return 0;
	}

	if (P2P_FLAG_HAND_SHAKE & pPacket->m_bFlag) 
	{
		TxReliablePacket(pPacket, pi, nServerTick);
	} 
	else 
	{
		TxVolatilePacket(pPacket, pi);
	}

	return pPacket->Length();
}

int CP2PClientEx::TxP2pPacket(int destKey, PACKET_P2P_GENERIC* pPacket, int nServerTick)
{
	TLock lo(this);

	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	STRUCT_PEER_INFO* pi = m_pPeerInfoMgr->GetPeerInfo(destKey);
	if (!pi) 
	{
		return 0;
	}
	int ret = TxP2pPacket(pi, pPacket, nServerTick);
	return ret;
}

bool CP2PClientEx::IsPeerSpecified(int key)
{
	return key > P2P_DEST_ALL;
}

int CP2PClientEx::CreateTimeSetEventForTx()
{
#ifdef _BT_DEVELOP
	UINT val = P2P_RETRANSMIT_INTERVAL;
	if (-1 != m_dwRetransmitInterval) 
		val = m_dwRetransmitInterval;
	return timeSetEvent(val, 50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
#else	
	return timeSetEvent(P2P_RETRANSMIT_INTERVAL, 50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
#endif
}

int CP2PClientEx::TxReliablePacket(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pDest, int nCurrent)
{
	if(pPacket == 0x0 || pDest == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	int		nSeq;	
	nSeq = (pDest->m_nSeq + 1)%256;
	pDest->m_nSeq = (BYTE)nSeq;
	pPacket->m_nSeq = (BYTE)nSeq;
	pPacket->m_nTick = nCurrent;
	pDest->m_pBuffer[nSeq] = m_packetBuffer.Alloc();
	if(pDest->m_pBuffer[nSeq]) {
		pDest->m_pBuffer[nSeq]->m_nTxTime = nCurrent;
		memcpy(pDest->m_pBuffer[nSeq]->m_pBuffer, pPacket, pPacket->Length());
		
		if (pDest->m_nTimerID) {
			SystemPrint(2,"CP2PClientEx::TxReliablePacket(busy. Storing packet instead for next transmission(MsgID=%d, seq=%d))\n", 
				pPacket->MsgID(), nSeq);
			return	pPacket->Length();
		}
		pDest->m_nRetryCount = 0;
		pDest->m_nAckSeq = (BYTE)nSeq;
		pDest->m_nTimerID = CreateTimeSetEventForTx();
		if (NULL == pDest->m_nTimerID) {
			SystemPrint(2, "TxReliablePacket(timeSetEvent fail)\n");
		}
		return TxVolatilePacket(pPacket, pDest);
	}
	return 0;
}

int CP2PClientEx::TxVolatilePacket(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pDest)
{
	SOCKADDR*       pAddr = NULL;

	if(pPacket && pDest) {
		if (pDest->m_bInSameNAT) {
			pAddr = &(pDest->m_addrLocal);
		} else {
			pAddr = &(pDest->m_addrPublic);
		}

		if (P2P_FLAG_RELAY&pPacket->m_bFlag) {
			pAddr = &(m_serverInfo.m_addrPublic);
			return TxPacket(pPacket, pAddr);
		}

		return TxPacket(pPacket, pAddr );
	}
	return 0;
}

#ifndef case_msg
#define case_msg(msgid, msg) \
	case msgid : \
		StringCchPrintf(buf, size, _T("MSGID_P2P_REQ_RESPONSE")); break;
#endif

void CP2PClientEx::GetLogMsgByMsgID(MSGID msgid, TCHAR* buf, size_t size)
{
	switch (msgid) {
		case_msg(MSGID_P2P_I_OPEND_DOOR, _T("MSGID_P2P_I_OPEND_DOOR"));
		case_msg(MSGID_P2P_REQ_RESPONSE, _T("MSGID_P2P_REQ_RESPONSE"));
		case_msg(MSGID_P2P_ACK_RESPONSE, _T("MSGID_P2P_ACK_RESPONSE"));
		case_msg(MSGID_P2P_CONFIRM_RESPONSE, _T("MSGID_P2P_CONFIRM_RESPONSE"));
		case_msg(MSGID_P2P_REQ_PUNCHHOLE, _T("MSGID_P2P_REQ_PUNCHHOLE")); 
		case_msg(MSGID_P2P_PUNCHHOLE_PROBE1, _T("MSGID_P2P_PUNCHHOLE_PROBE1"));
		case_msg(MSGID_P2P_PUNCHHOLE_PROBE2, _T("MSGID_P2P_PUNCHHOLE_PROBE2")); 
		case_msg(MSGID_P2P_PUNCH, _T("MSGID_P2P_PUNCH"));
		case_msg(MSGID_P2P_HOLE_SCAN, _T("MSGID_P2P_HOLE_SCAN"));
		case_msg(MSGID_P2P_ACK_PUNCHHOLE, _T("MSGID_P2P_ACK_PUNCHHOLE")); 
		case_msg(MSGID_P2P_HEARTBEAT, _T("MSGID_P2P_HEARTBEAT")); 
		case_msg(MSGID_P2P_REQ_CONNECT, _T("MSGID_P2P_REQ_CONNECT")); 
		case_msg(MSGID_P2P_ACK_CONNECT, _T("MSGID_P2P_ACK_CONNECT")); 
		case_msg(MSGID_P2P_REQ_PEER_ADDR, _T("MSGID_P2P_REQ_PEER_ADDR")); 
		case_msg(MSGID_P2P_PEER_ADDR_OK, _T("MSGID_P2P_PEER_ADDR_OK")); 
		case_msg(MSGID_P2P_ACK_SEQ, _T("MSGID_P2P_ACK_SEQ")); 
		case_msg(MSGID_P2P_EXIT, _T("MSGID_P2P_EXIT)")); 
		default : 
			break;
	}
}

int CP2PClientEx::TxPacket(PACKET_P2P_GENERIC *pOutPacket, SOCKADDR* pAddr)
{
	if(m_sRcvSocket == INVALID_SOCKET || pOutPacket == 0x0 || pAddr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	int		nResult;
	///////////////////////////////////////////////////
	// for debug Log Packet
	TCHAR strTemp[64];
	TCHAR strFlag[16];
	strTemp[0] = 0;
	GetLogMsgByMsgID(pOutPacket->MsgID(), strTemp, sizeof(strTemp));

	if (strTemp[0]) {
		strFlag[0] = 0;
		if (pOutPacket->m_bFlag & P2P_FLAG_HAND_SHAKE) {
			StringCchCopy(strFlag, 16, _T("|H"));
		}
		if (pOutPacket->m_bFlag & P2P_FLAG_USER_DEFINED_ACK) {
			StringCchCat(strFlag, 16, _T("|U"));
		}
		if (pOutPacket->m_bFlag & P2P_FLAG_PIGGY_BACK) {
			StringCchCat(strFlag, 16, _T("|P"));
		}
		if (pOutPacket->m_bFlag & P2P_FLAG_TX_SERVER) {
			StringCchCat(strFlag, 16, _T("|S"));
		}
		if (pOutPacket->m_bFlag & P2P_FLAG_RELAY) {
			StringCchCat(strFlag, 16, _T("|R"));
		}
		if (0 == strFlag[0]) {
			StringCchCopy(strFlag, 16, _T("|"));
		}
		int level = 2;
		if (pOutPacket->MsgID() == MSGID_P2P_ACK_SEQ || pOutPacket->MsgID() == MSGID_P2P_HEARTBEAT)
			level = HEARTBEAT_LOG;


		SystemPrint(level, "Tx:%d(m_nKey=%d, %s(dest=%d(%s:%d),flag=%s|,seq=%d,AckSeq=%d))", 
			pOutPacket->MsgID(), m_nKey, 
			strTemp, pOutPacket->m_nDestPeerKey, inet_ntoa( ((SOCKADDR_IN *)pAddr)->sin_addr), ntohs(((SOCKADDR_IN *)pAddr)->sin_port), strFlag,
			pOutPacket->m_nSeq, pOutPacket->m_nAckSeq);
	}
	// for debug Log Packet
	///////////////////////////////////////////////////
	if (NULL == pAddr) {
		return -1;
	}
	/////////////////////////////////////////
	//for Test
	if (m_nDropTx > 0) {
		m_nDropTx--;
		return pOutPacket->Length();
	}
	while (m_bSuspendTx) {
		Sleep(40);
	}
	/////////////////////////////////////////
	nResult = sendto(m_sRcvSocket, (char *)pOutPacket, pOutPacket->Length(), 0,
		pAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == nResult) {
		int	nErrorCode = WSAGetLastError();
		SystemPrint(ERROR_LOG, "nErrorCode=%d, sd=%d, %s:%d\n", nErrorCode, m_sRcvSocket, inet_ntoa(((SOCKADDR_IN*)pAddr)->sin_addr), ((SOCKADDR_IN*)pAddr)->sin_port);
		switch(nErrorCode) {
			case WSAEWOULDBLOCK:
				nResult = 0; break;
			case WSAETIMEDOUT:
				nResult = 0; break;
			default : nResult = -1; break;
		}
		//LPVOID lpMsgBuf;
		//FormatMessage(
		//	FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		//	FORMAT_MESSAGE_FROM_SYSTEM | 
		//	FORMAT_MESSAGE_IGNORE_INSERTS,
		//	NULL,
		//	nErrorCode,
		//	0, // Default language
		//	(LPTSTR) &lpMsgBuf,
		//	0,
		//	NULL 
		//	);
		//if (lpMsgBuf) {
		//	SystemPrint(ERROR_LOG, "P2p socket send to (%s:%s) error(%s) ", 
		//		inet_ntoa( ((SOCKADDR_IN *)pAddr)->sin_addr),ntohs(((SOCKADDR_IN *)pAddr)->sin_port),
		//		lpMsgBuf);
		//}
	} else if ( 0 == nResult) {		
		SystemPrint(2,"P2p socket send to (%s:%s) error(0bytes send) \n",
			inet_ntoa( ((SOCKADDR_IN *)pAddr)->sin_addr),ntohs( ((SOCKADDR_IN *)pAddr)->sin_port ) );
	}
	else if( nResult != pOutPacket->Length()) {
		SystemPrint(2,"sendto length mismatch, %d bytes sent, but %d bytes expected", nResult, pOutPacket->Length());
	}
	return nResult;
}

int CP2PClientEx::RequestConnectServer(void)
{
	PACKET_P2P_REQ_CONNECT		packet;

	packet.InitData();
	packet.Init(GetKey(), MSGID_P2P_REQ_CONNECT);
	packet.m_bFlag = P2P_FLAG_TX_SERVER|P2P_FLAG_HAND_SHAKE|P2P_FLAG_USER_DEFINED_ACK;
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	memcpy(&(packet.m_addrLocal), &m_addrLocal, sizeof(SOCKADDR));
	packet.Complete(sizeof(PACKET_P2P_REQ_CONNECT));

	m_serverInfo.m_nStage = P2P_SVR_CONNECT_TRY;
	//SOCKADDR_IN	addrSelf;
	//addrSelf.sin_family = AF_INET;
	//addrSelf.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//addrSelf.sin_port = htons((u_short)6800);
	//TxPacket(&packet,(SOCKADDR *)&addrSelf);
	return	TxP2pPacket(&packet);
}

int	CP2PClientEx::RequestPeerAddr(int nKey)
{
	char						pBuffer[512];
	PACKET_P2P_REQ_PEER_INFO*	pPacket = (PACKET_P2P_REQ_PEER_INFO *)pBuffer;

	if(pPacket == 0x0 || m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	bGetPeerAddr = false; // 20070123 dEAthcURe

	pPacket->Init(GetKey(), MSGID_P2P_REQ_PEER_ADDR);
	pPacket->InitData();
	pPacket->m_bFlag = P2P_FLAG_TX_SERVER|P2P_FLAG_HAND_SHAKE|P2P_FLAG_USER_DEFINED_ACK;
	pPacket->m_nDestPeerKey = P2P_DEST_NONE;
	if (-1 == nKey) {
		pPacket->m_nNumberOfPeer = m_nCurrentPeer;
		const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
		int idx = 0;
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
		{
			STRUCT_PEER_INFO* pi = it->second;
			if(pi) { // 20070522 dEAthcURe pi null check
				pPacket->SetPeerKey(idx++, pi->m_nKey);
				SystemPrint(2, "RequestPeerAddr(idx=%d, key=%d)\n", idx, pi->m_nKey);
			} // 20070522 dEAthcURe pi null check
		}
	} else {
		pPacket->m_nNumberOfPeer = 1;
		pPacket->SetPeerKey(0, nKey);
		SystemPrint(2, "RequestPeerAddr_(0(%d))\n",nKey);
	}
	pPacket->Complete(pPacket->GetSize());
	m_serverInfo.m_nStage = P2P_SVR_ADDR_TRY;
	return TxP2pPacket(pPacket);
}

int	CP2PClientEx::RequestExit()
{
	if (P2P_SVR_CONNECT < m_serverInfo.m_nStage) {
		return 1;
	}
	// TODO jovial, 06-10-04, 아래 코드 브로 팀에 문의 필요
	//if (P2P_SVR_CONNECT < m_serverInfo.m_nStage) {
	//	return 1;
	//}
	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(GetKey(), MSGID_P2P_EXIT);
	packet.m_bFlag = P2P_FLAG_TX_SERVER;
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	//m_serverInfo.m_nStage = P2P_SVR_EXIT_TRY;
	return TxP2pPacket(&packet);
}

int	CP2PClientEx::RequestExitWithAllPeers()
{
	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(GetKey(), MSGID_P2P_EXIT_ALL_PEER);
	packet.m_bFlag = P2P_FLAG_TX_SERVER;
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	//m_nP2pServerStage = P2P_SVR_EXIT_ALL_TRY0;
	return TxP2pPacket(&packet);
}
void CP2PClientEx::RxPeerAddrOk(PACKET_P2P_PEER_INFO_OK *pPacket)
{
	if(pPacket == 0x0 || m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	STRUCT_PEER_INFO* piResolved = 0x0; // 20070220 dEAthcURe
	/////////////////////////////////////////////////////////////////
	//수신한 값으로 peer들의 addr필드를 setting
	for (int i = 0; i < pPacket->m_nNumberOfPeer; i++) {
		STRUCT_PEER_ADDR*	pPeerAddr = pPacket->GetPeerAddr(i);
		if(pPeerAddr) { // 20070522 dEAthcURe null check
			SystemPrint(2,"pPeerAddr[%d]->key(%d)\n", i, pPeerAddr->m_nKey);
			////////////////////////////////////////////////////////////////
			// 나의 public addr값 setting
			if (GetKey() == pPeerAddr->m_nKey) {
				memcpy(&m_addrPublic,&(pPeerAddr->m_addrPublic), sizeof(SOCKADDR));
			}
			////////////////////////////////////////////////////////////////
			const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
			for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++) {
				char strLocalAddr[64];
				char strPublicAddr[64];
				STRUCT_PEER_INFO* pi = it->second;
				if(pi) { // 20070522 dEAthcURe pi null check
					//if (pi->IsConnected()) continue; // [-] 20070124 dEAthcURe
					if (pPeerAddr->m_nKey == pi->m_nKey) {
						pi->SetAddrLocal(&(pPeerAddr->m_addrLocal));	//같은 NAT안의 클라이언트들끼리의 통신도 가능하게하기 위해 수정
						pi->SetAddrPublic(&(pPeerAddr->m_addrPublic));
						// {{ 20070123 dEAthcURe
						bGetPeerAddr = true;
						pi->bUpdated = true; // 20070220
						piResolved = pi; // 20070220	
						// }} 20070123 dEAthcURe

						StringCchPrintfA(strLocalAddr,64,"%s",inet_ntoa( ((SOCKADDR_IN *)&(pi->m_addrLocal))->sin_addr));
						StringCchPrintfA(strPublicAddr,64,"%s",inet_ntoa( ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_addr));
						SystemPrint(2,"peerMap. key(%d),Laddr(%s:%d), Paddr(%s:%d)", pi->m_nKey, strLocalAddr, ((SOCKADDR_IN *)&(pi->m_addrLocal))->sin_port, strPublicAddr, ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_port);
						break;
					}
				} // 20070522 dEAthcURe pi null check
			}
		} // 20070522 dEAthcURe null check
	}
	//수신한 값으로 peer들의 addr필드를 setting
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// for same NAT
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe pi null check
			//if (memcmp(&(m_pPeerInfo[i].m_addrPublic), &m_addrPublic, sizeof(SOCKADDR)) == 0 ) {
			if ( ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_addr.S_un.S_addr == ((SOCKADDR_IN *)&m_addrPublic)->sin_addr.S_un.S_addr) {
				SystemPrint(2,"peer(%d:%s) is in same NAT\n", pi->m_nKey, inet_ntoa( ((SOCKADDR_IN *)&(pi->m_addrLocal))->sin_addr) );
				pi->m_bInSameNAT = true;
			} else {
				SystemPrint(2,"peer(%d:%s) is not in same NAT\n", pi->m_nKey, inet_ntoa( ((SOCKADDR_IN *)&(pi->m_addrLocal))->sin_addr) );
				pi->m_bInSameNAT = false;
			}
		} // 20070522 dEAthcURe pi null check
	}
	////////////////////////////////////////////////////////////////
	if (m_pNotifier) {
		m_pNotifier->Execute((DWORD)P2P_SVR_ADDR_OK, (DWORD)piResolved); // [!] 20070220 dEAthcURe // m_pNotifier->Execute((DWORD)P2P_SVR_ADDR_OK, (DWORD)0);
	}
}

void CP2PClientEx::RequestConnectPeers()
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;

		if(pi) { // 20070522 dEAthcURe pi null check
			if ( ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_addr.S_un.S_addr == 0 ) {
				continue;
			}
			if (false == pi->IsConnected()) {
				pi->m_nStage = P2P_SESSION_DIRECT_TRY0;
				pi->m_nDirectRqTimerID = timeSetEvent(P2P_DIRECT_RQ_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
				TxDirectRq(pi);
			}
		} // 20070522 dEAthcURe pi null check
	}
}

bool CP2PClientEx::RequestConnectPeer(int idPeer)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return false;
	}

	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;

		if(pi) { // 20070522 dEAthcURe pi null check
			if(pi->m_nKey == idPeer) {
				if ( ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_addr.S_un.S_addr == 0 ) {
					SystemPrint(ERROR_LOG, "[CP2PClientEx::RequestConnectPeer] invalid address for %d", pi->m_nKey);
					return false;
				}
				if (false == pi->IsConnected()) {
					pi->m_nStage = P2P_SESSION_DIRECT_TRY0;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::RequestConnectPeer] timeSetEvent for %d\n", pi->m_nKey);
					pi->m_nDirectRqTimerID = timeSetEvent(P2P_DIRECT_RQ_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxDirectRq(pi);
					return true;
				}
				else {
					SystemPrint(ERROR_LOG, "[CP2PClientEx::RequestConnectPeer] already connected %d", pi->m_nKey);
					return false;
				}
			}
		} // 20070522 dEAthcURe pi null check
	}
	return false;
}

int CP2PClientEx::TxDirectRq(STRUCT_PEER_INFO* pStruct)
{
	if(pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return 0;
	}

	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(GetKey(), MSGID_P2P_I_OPEND_DOOR);
	packet.m_bFlag = 0;
	packet.m_nDestPeerKey = pStruct->m_nKey;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	//TxVolatilePacket(&packet, pStruct);
	TxPacket(&packet, &m_serverInfo.m_addrPublic);
	TxPacket(&packet, &pStruct->m_addrLocal );
	TxPacket(&packet, &pStruct->m_addrPublic );

	packet.InitData();
	packet.Init(GetKey(), MSGID_P2P_REQ_RESPONSE);
	//packet.m_bFlag = P2P_FLAG_HAND_SHAKE|P2P_FLAG_USER_DEFINED_ACK|P2P_FLAG_RELAY;
	packet.m_bFlag = P2P_FLAG_RELAY;
	packet.m_nDestPeerKey = pStruct->m_nKey;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	SystemPrint(2,"TxVolatilePacket(MSGID_P2P_REQ_RESPONSE, key(%d))", pStruct->m_nKey);
	return TxVolatilePacket(&packet, &m_serverInfo);
}

void CP2PClientEx::RxDirectRq(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pStruct, SOCKADDR* /*pAddr*/)
{
	if(pPacket == 0x0 || pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2, "RxDirectRq\n");
	if (pStruct->IsConnected()) {
		SystemPrint(2, "[CP2PClientEx::RxDirectRq] already connected");
		//return; // test disable 20070405 나는 등록되어있어도 남은 등록 안되있을수있다.
	}

	// {{ 20070620 test 
	{
		PACKET_P2P_GENERIC	packet;
		packet.InitData();
		packet.Init(GetKey(), MSGID_P2P_I_OPEND_DOOR);
		packet.m_bFlag = 0;
		packet.m_nDestPeerKey = pStruct->m_nKey;
		packet.Complete(sizeof(PACKET_P2P_GENERIC));
		//TxVolatilePacket(&packet, pStruct);
		TxPacket(&packet, &m_serverInfo.m_addrPublic);
		TxPacket(&packet, &pStruct->m_addrLocal );
		TxPacket(&packet, &pStruct->m_addrPublic );
	}
	// }} 20070620 test 

	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(m_nKey, MSGID_P2P_ACK_RESPONSE);
	//packet.m_bFlag = P2P_FLAG_PIGGY_BACK;	
	packet.m_nAckSeq = pPacket->m_nSeq;
	packet.m_nDestPeerKey = pPacket->ConnectID();
	packet.m_nTick = GetServerTick();
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	TxVolatilePacket(&packet,pStruct);	//다시 이걸로 변경. 문열기 packet을 추가 하면서 accept addr이 아닌 public ip로 보내는 version으로 되돌아간다.
}
void CP2PClientEx::RxDirectAck(STRUCT_PEER_INFO* pStruct, SOCKADDR* pAddr)
{
// requester function
	if(pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

//	return; // NOTICE 서버 릴레이로 테스트시에는 여기서 바로 return
	PACKET_P2P_GENERIC	packet;

	SystemPrint(2, "RxDirectAck\n");

	bool bAlreadyConnected = pStruct->IsConnected();
	if(bAlreadyConnected) {
		SystemPrint(2, "[CP2PClientEx::RxDirectAck] already connected");
	}
	if (!bAlreadyConnected) { // 이미 접속되어 있으면 상대방이 접속 과정을 수행할 수 있도록 패킷만 보내주고 나는 현상 유지
		ClearSessionBuffer(pStruct);
		pStruct->SetAddrPublic(pAddr);
		pStruct->m_nStage = P2P_SESSION_DIRECT_ESTABLISHED;
		SystemPrint(2, "mykey=%d, P2P_SESSION_DIRECT_ESTABLISHED(%d)\n", m_nKey, pStruct->m_nKey);
	}	

	// {{ 20070621 test 
	{
		PACKET_P2P_GENERIC	packet;
		packet.InitData();
		packet.Init(GetKey(), MSGID_P2P_I_OPEND_DOOR);
		packet.m_bFlag = 0;
		packet.m_nDestPeerKey = pStruct->m_nKey;
		packet.Complete(sizeof(PACKET_P2P_GENERIC));		
		TxPacket(&packet, &pStruct->m_addrLocal );
		TxPacket(&packet, &pStruct->m_addrPublic );
	}
	// }} 20070621 test 

	packet.InitData();
	packet.Init(m_nKey, MSGID_P2P_CONFIRM_RESPONSE);
	packet.m_bFlag = P2P_FLAG_HAND_SHAKE;	
	packet.m_nDestPeerKey = pStruct->m_nKey;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	TxP2pPacket(&packet);
	if (!bAlreadyConnected && m_pNotifier) {
		m_pNotifier->Execute((DWORD)P2P_SESSION_DIRECT_ESTABLISHED, (DWORD)pStruct->m_nKey);
	}
}

void CP2PClientEx::RxDirectConfirm(STRUCT_PEER_INFO* pStruct, SOCKADDR* pAddr)
{
	if(pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	PACKET_P2P_GENERIC	packet;

	SystemPrint(2, "RxDirectConfirm\n");
	if (pStruct->IsConnected()) {
		SystemPrint(2, "[CP2PClientEx::RxDirectConfirm] already connected, return");		
		return;
	}
	ClearSessionBuffer(pStruct);
	pStruct->SetAddrPublic(pAddr);
	pStruct->m_nStage = P2P_SESSION_DIRECT_ESTABLISHED;
	SystemPrint(2, "mykey=%d, P2P_SESSION_DIRECT_ESTABLISHED(%d)\n", m_nKey, pStruct->m_nKey);
	if (m_pNotifier) {
		m_pNotifier->Execute((DWORD)P2P_SESSION_DIRECT_ESTABLISHED, (DWORD)pStruct->m_nKey);
	}
}

///////////////////////////////////////////////////////////////
// Requester쪽에서 부르는 method이다
void CP2PClientEx::RequestHolePunching(STRUCT_PEER_INFO* pStruct)
{
	if(pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	// {{ 20070621 dEAthcURe
	{
		PACKET_P2P_GENERIC	packet;
		packet.InitData();
		packet.Init(GetKey(), MSGID_P2P_I_OPEND_DOOR);
		packet.m_bFlag = 0;
		packet.m_nDestPeerKey = pStruct->m_nKey;
		packet.Complete(sizeof(PACKET_P2P_GENERIC));
		//TxVolatilePacket(&packet, pStruct);
		TxPacket(&packet, &m_serverInfo.m_addrPublic);
		TxPacket(&packet, &pStruct->m_addrLocal );
		TxPacket(&packet, &pStruct->m_addrPublic );
	}
	// }} 20070621 dEAthcURe

	PACKET_P2P_RQ_HOLE_PUNCH	packet;
	packet.Init(GetKey(), MSGID_P2P_REQ_PUNCHHOLE);
	packet.InitData();
	packet.m_nDestPeerKey = pStruct->m_nKey;
	packet.m_bFlag = P2P_FLAG_HAND_SHAKE|P2P_FLAG_USER_DEFINED_ACK|P2P_FLAG_RELAY;
	memcpy(&(packet.m_RequsterPublicAddr), &m_addrPublic, sizeof(SOCKADDR));
	packet.Complete(sizeof(PACKET_P2P_RQ_HOLE_PUNCH));
	pStruct->m_nStage = P2P_SESSION_HOLE_PUNCHING_REQ;
	SystemPrint(2, "RequestHolePunching(my addr=(%s:%d))\n",
		inet_ntoa( ((SOCKADDR_IN*)&m_addrPublic)->sin_addr ), 
		ntohs( ((SOCKADDR_IN*)&m_addrPublic)->sin_port ) );
	TxP2pPacket(&packet);
}

/////////////////////////////////////////////////////////////
// Request받은 쪽(NAT안에 있는 놈)에서 부르는 method이다.
void CP2PClientEx::RxRqHolePunching(PACKET_P2P_RQ_HOLE_PUNCH* pPacket, STRUCT_PEER_INFO* pStruct)
{
	if(pPacket == 0x0 || pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2, "RxRqHolePunching\n");
	if (pStruct->IsConnected()) {	//connected되어 있다는 말은 양방향 으로 통신가능 하다는 얘기이므로 더이상 진행 할 필요가 없다.
		SystemPrint(2, "[CP2PClientEx::RxRqHolePunching] already connected");
		//return; // 20070613 dEAthcURe 상대가 원한다면 응해준다.		
	}
	pStruct->m_nStage = P2P_SESSION_HOLE_PUNCHING_TRY0;
	SystemPrint(ERROR_LOG, "[CP2PClientEx::RxRqHolePunching] timeSetEvent for %d\n", pStruct->m_nKey);
	pStruct->m_nPunchHoleTimerID = timeSetEvent(P2P_PUNCHHOLE_RETRY_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
	pStruct->SetAddrPublic(&(pPacket->m_RequsterPublicAddr));
	SystemPrint(2, "RxRqHolePunching(requester addr=(%s:%d))\n",
		inet_ntoa( ((SOCKADDR_IN*)&(pPacket->m_RequsterPublicAddr))->sin_addr ), 
		ntohs( ((SOCKADDR_IN*)&(pPacket->m_RequsterPublicAddr))->sin_port ) );
	TxPunchHoleProbePacket(pStruct);
}

void CP2PClientEx::TxPunchHoleProbePacket(STRUCT_PEER_INFO* pStruct)
{
	if(pStruct == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	PACKET_P2P_HOLE_PROBE	packetTunneling;
	if (pStruct->IsConnected()) {
		SystemPrint(2, "[CP2PClientEx::TxPunchHoleProbePacket] already connected");
		//return; // 20070613 dEAthcURe 상대가 원한다면 응해준다.
	}
	packetTunneling.Init(GetKey(), MSGID_P2P_PUNCHHOLE_PROBE1);
	packetTunneling.InitData();
	packetTunneling.m_nDestPeerKey = pStruct->m_nKey;
	memcpy(&(packetTunneling.m_RequsteePublicAddr), &m_addrPublic, sizeof(SOCKADDR));
	packetTunneling.Complete(sizeof(PACKET_P2P_HOLE_PROBE));
	TxVolatilePacket(&packetTunneling, &m_serverProbe1Info);
	packetTunneling.Init(GetKey(), MSGID_P2P_PUNCHHOLE_PROBE1);	//이건 더미로 상대 피어한테 보낸다. 
	TxVolatilePacket(&packetTunneling, pStruct);
	packetTunneling.Init(GetKey(), MSGID_P2P_PUNCHHOLE_PROBE2);
	TxVolatilePacket(&packetTunneling, &m_serverProbe2Info);
	SystemPrint(2, "TxPunchHoleProbePacket(my addr=(%s:%d))\n",
		inet_ntoa( ((SOCKADDR_IN*)&(packetTunneling.m_RequsteePublicAddr))->sin_addr ), 
		ntohs( ((SOCKADDR_IN*)&(packetTunneling.m_RequsteePublicAddr))->sin_port ) );
}
///////////////////////////////////////////////////////////////
// Requester쪽에서 부르는 method이다
void CP2PClientEx::RxPunch(STRUCT_PEER_INFO* pPeerInfo, PACKET_P2P_HOLE_PUNCH* pPacket)
{
	if(pPeerInfo == 0x0 || pPacket == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2, "RxPunch\n");
	if (NULL == pPeerInfo) {
		SystemPrint(2, "peer(%d) is not exist\n", pPacket->ConnectID());
		return;
	}
	if (pPacket->m_nDestPeerKey != m_nKey) {
		SystemPrint(2, "I(%d) received packet to (%d)\n", m_nKey, pPacket->m_nDestPeerKey);
		return;
	}
	/* [-] 20070621 dEAthcURe 따질필요없3
	if (P2P_SESSION_HOLE_PUNCHING_REQ != pPeerInfo->m_nStage) {
		SystemPrint(2, "P2P_SESSION_HOLE_PUNCHING_REQ 상태가 아닌데 RxPunch를 받았다는 말은 Network delay가 비정상적으로 길어 이미 상태가 disconnect혹은 relay준비 상태로 바뀌었다는  의미이므로 이경우 내가 punchhole try하는것은 실패한것으로 간주하고 아무 처리를 하지 않는다");
		return;
	}
	*/

	pPeerInfo->SetAddrPublic(&(pPacket->m_RequsteePublicAddr));
	pPeerInfo->m_nBeginPort = pPacket->m_nBeginPort;
	pPeerInfo->m_nEndPort = pPacket->m_nEndPort;
	SystemPrint(ERROR_LOG, "[CP2PClientEx::RxPunch] timeSetEvent for %d\n", pPeerInfo->m_nKey);
	pPeerInfo->m_nHoleScanTimerID = timeSetEvent(P2P_PUNCHHOLE_RETRY_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
	pPeerInfo->m_nStage = P2P_SESSION_HOLE_SCAN0;
	TxHoleScan(pPeerInfo); 
}

void CP2PClientEx::TxHoleScan(STRUCT_PEER_INFO* pPeerInfo)
{
	if(pPeerInfo == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	if (pPeerInfo->m_nBeginPort == pPeerInfo->m_nEndPort) {	//Cone NAT에 있는 경우다.
		if ( ntohs( ((SOCKADDR_IN *)&(pPeerInfo->m_addrPublic))->sin_port) != pPeerInfo->m_nBeginPort ) {
			SystemPrint(2,"peer(%d(%s:%d)) is in Cone NAT but has fault\n", pPeerInfo->m_nKey,
				inet_ntoa(((SOCKADDR_IN *)&(pPeerInfo->m_addrPublic))->sin_addr),
				ntohs( ((SOCKADDR_IN *)&(pPeerInfo->m_addrPublic))->sin_port ) );
			return;
		}
		PACKET_P2P_HOLE_SCAN	packet;
		packet.Init(GetKey(), MSGID_P2P_HOLE_SCAN);
		packet.InitData();
		packet.m_nDestPeerKey = pPeerInfo->m_nKey;
		packet.m_nPort = pPeerInfo->m_nBeginPort;
		packet.Complete(sizeof(PACKET_P2P_HOLE_SCAN));
		TxVolatilePacket(&packet, pPeerInfo);
	} else if ( (pPeerInfo->m_nBeginPort < pPeerInfo->m_nEndPort) && (pPeerInfo->m_nBeginPort + 1000 < pPeerInfo->m_nEndPort) ) {	//Symmetric NAT에 있는 경우다.
		SOCKADDR_IN	addrDest;
		PACKET_P2P_HOLE_SCAN	packet;

		SystemPrint(2,"peer(%d:%s) is in symmetric NAT(%d:%d)\n",pPeerInfo->m_nKey,
			inet_ntoa(((SOCKADDR_IN *)&(pPeerInfo->m_addrPublic))->sin_addr), pPeerInfo->m_nBeginPort, pPeerInfo->m_nEndPort );
		packet.Init(GetKey(), MSGID_P2P_HOLE_SCAN);
		packet.InitData();
		packet.m_nDestPeerKey = pPeerInfo->m_nKey;
		packet.Complete(sizeof(PACKET_P2P_HOLE_SCAN));
		memcpy(&addrDest, &(pPeerInfo->m_addrPublic), sizeof(SOCKADDR_IN));
		////////////////////////////////////////////////////////////////////
		// Begin Port에서 End Port사이에 있는 Port에 MSGID_P2P_HOLE_SCAN
		// 을 던진다.
		for (int nPort = pPeerInfo->m_nBeginPort-16; nPort < pPeerInfo->m_nEndPort+16; nPort++) { // for (int nPort = pPeerInfo->m_nBeginPort+1; nPort < pPeerInfo->m_nEndPort; nPort++) {
			addrDest.sin_port = htons((unsigned short)nPort);
			packet.m_nPort = (unsigned short)nPort;
			TxPacket(&packet, (SOCKADDR *)&addrDest);	//TxVolatilePacket외에서 직접 TxPacket은 여기서만 부른다.
		}
		////////////////////////////////////////////////////////////////////
	} else {
		SystemPrint(2, "Wrong packet(begin port(%d) > end port(%d) from peer(%d)\n", pPeerInfo->m_nBeginPort, pPeerInfo->m_nEndPort, pPeerInfo->m_nKey);
	}
}

/////////////////////////////////////////////////////////////
// Request받은 쪽(NAT안에 있는 놈)에서 부르는 method이다.
void CP2PClientEx::RxHoleScan(STRUCT_PEER_INFO* pPeerInfo, PACKET_P2P_HOLE_SCAN* pPacket, SOCKADDR* pAddr)
{
	if(pPeerInfo == 0x0 || pPacket == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	PACKET_P2P_HOLE_SCAN	packetAck;
	//if (P2P_SESSION_HOLE_PUNCHING_ESTABLISHED > pPeerInfo->m_nStage) {
	//	pPeerInfo->m_nStage = P2P_SESSION_HOLE_PUNCHING_ESTABLISHED;
	//}
	SystemPrint(2, "RxHoleScan\n");

	bool bAlreadyConnected = pPeerInfo->IsConnected(); // 20070613 dEAthcURe 이미 접속되어있으면 packet만 전해주고 상태유지.
	if(bAlreadyConnected) {
		SystemPrint(2, "[CP2PClientEx::RxHoleScan] already connected");
	}
	if(!bAlreadyConnected) {
		ClearSessionBuffer(pPeerInfo);
		pPeerInfo->SetAddrPublic(pAddr);
		pPeerInfo->m_nStage = P2P_SESSION_HOLE_PUNCHING_ESTABLISHED;
	}

	packetAck.Init(GetKey(), MSGID_P2P_ACK_PUNCHHOLE);
	packetAck.InitData();
	packetAck.m_nDestPeerKey = pPacket->ConnectID();
	packetAck.m_bFlag = P2P_FLAG_HAND_SHAKE;
	packetAck.m_nPort = pPacket->m_nPort;
	packetAck.Complete(sizeof(PACKET_P2P_HOLE_SCAN));	
	TxP2pPacket(&packetAck);

	if (!bAlreadyConnected && m_pNotifier) {
		m_pNotifier->Execute((DWORD)P2P_SESSION_HOLE_PUNCHING_ESTABLISHED, (DWORD)pPeerInfo->m_nKey);
	}
}

/////////////////////////////////////////////////////////////////
//Requester 쪽에서 부르,는 method이다
void CP2PClientEx::RxHolePunchAck(STRUCT_PEER_INFO* pPeerInfo, SOCKADDR* pAddr)
{
	if(pPeerInfo == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2, "RxHolePunchAck\n");
	if (pPeerInfo->IsConnected()) {
		SystemPrint(2, "[CP2PClientEx::RxHolePunchAck] already connected, return");
		return;
	}
	ClearSessionBuffer(pPeerInfo);
	pPeerInfo->SetAddrPublic(pAddr);
	pPeerInfo->m_nStage = P2P_SESSION_HOLE_PUNCHING_ESTABLISHED;
	if (m_pNotifier) {
		m_pNotifier->Execute((DWORD)P2P_SESSION_HOLE_PUNCHING_ESTABLISHED, (DWORD)pPeerInfo->m_nKey);
	}
}

void CP2PClientEx::RxTimeAdjust(STRUCT_PEER_INFO* pSender, PACKET_P2P_TIME * pPacket)
{
	if(pPacket == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	if (pSender != &m_serverInfo) {
		SystemPrint(2,"Server가 아닌 놈(%d)이 MSGID_P2P_TIME_ADJUST를 보내왔다\n",pPacket->ConnectID());
		return ;
	}
	if (pPacket->m_nT1 != m_tXmt) {
		SystemPrint(2,"%d(org:%d, rec:%d, xmt:%d)한테서 온 MSGID_P2P_TIME_ADJUST(%d,%d,%d,%d)이 위조된 것 같다\n",pPacket->ConnectID(),
			m_tOrg, m_tRec, m_tXmt,pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);
		return ;
	}
	if (pPacket->m_nT3 != m_tOrg) {	//중복 Packet이 아니라면
		SystemPrint(GAME_LOG, "src key=%d(pInfo: org:%d, rec:%d, xmt:%d)한테서 온 MSGID_P2P_TIME_ADJUST(pPacket: %d,%d,%d,%d)정보\n",
			pPacket->ConnectID(), 
			m_tOrg, m_tRec, m_tXmt,pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);

		pPacket->m_nT4 = m_watch.GetTick();//GetTickCount();
		m_tOrg = pPacket->m_nT3;
		m_tRec = pPacket->m_nT4;
//		LOGPRT(_T("RxTimeAdjust. m_tOrg=%d, m_tRec=%d\n"), m_tOrg, m_tRec);
		int	nTurnArround = m_tRec - m_tXmt;
		int	nServerProcTime = pPacket->m_nT3 - pPacket->m_nT2;
		int	nRoundTripdelay = nTurnArround - nServerProcTime;
		//int	nClockOffset = (pPacket->m_nT2 - pPacket->m_nT1 + pPacket->m_nT3 -pPacket->m_nT4)/2;
		//int nServerClockOffset = nClockOffset - nRoundTripdelay/2;
		if (-1 == nRoundTripdelay) {
			nRoundTripdelay = 0;
		}
		if (m_tDelta0 == m_tRoundTripDelay[m_nTickIndex]) {
			for (int i = m_nTickIndex + 1; i < m_nTickIndex + 8; i++) {
				if (m_tRoundTripDelay[i % 8] <= m_tDelta0) {
					m_tDelta0 = m_tRoundTripDelay[i % 8];
					//m_tTheta0 = m_tClockOffset[i%8];
				}
			}
		}
		//m_tClockOffset[m_nTickIndex] = nClockOffset;
		m_tRoundTripDelay[m_nTickIndex] = nRoundTripdelay;
		m_nTickIndex = (m_nTickIndex + 1) % 8;
		if (nRoundTripdelay <= m_tDelta0 ) {
			m_tDelta0 = nRoundTripdelay;
			//m_tTheta0 = nClockOffset;
			m_tServerOffset = (m_tOrg-m_tRec) + (m_tDelta0/2); 
		}
	}
	else
	{
		SystemPrint(2, "src key=%d(pInfo: org:%d, rec:%d, xmt:%d)한테서 온 MSGID_P2P_TIME_ADJUST(pPacket: %d,%d,%d,%d)이 중복 인 것 같다\n",
			pPacket->ConnectID(), 
			m_tOrg, m_tRec, m_tXmt,pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);
	}
	//SystemPrint(HEARTBEAT_LOG,"Rx(m_nKey=%d, MSGID_P2P_TIME_ADJUST(%d,%d,%d,%d). src=%d)",
	//	m_nKey,
	//	pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4 ,pPacket->ConnectID());
	m_nTimeAdjustRetry = 0;
}

UINT CP2PClientEx::Run(int* pRun, int nID)
{
	if (0 == nID) {
		RxPacket(pRun);
	} else if (1 == nID) {
		TxHeartBeat(pRun);
	}
	return 1;
}

#ifdef EnableEmbedding

void CP2PClientEx::RxPacket(void)
{
	// test for single thread

	int nResult;
	char pBuffer[DEFAULT_P2P_BUFFER_SIZE];
	SOCKADDR addr;
	int addr_len = sizeof(SOCKADDR);
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)pBuffer;

	memset((char *) &addr, 0, sizeof(SOCKADDR));
	int nError;
	bool bClosed = false; 

	// select?
	fd_set readFds, writeFds, errorFds;
	FD_ZERO(&readFds);
	FD_SET(m_sRcvSocket, &readFds);

	struct timeval timevalue;

	timevalue.tv_sec = 0;
	timevalue.tv_usec = 1;

	::select(m_sRcvSocket+1, &readFds, 0x0, 0x0, &timevalue);
	if (!FD_ISSET(m_sRcvSocket, &readFds)) {
		return;
	}
	nResult = recvfrom(m_sRcvSocket, pBuffer, DEFAULT_P2P_BUFFER_SIZE, 0, &addr, &addr_len);
	
	if (SOCKET_ERROR == nResult) {
		switch(nError = WSAGetLastError())
		{
		case WSAEWOULDBLOCK:
		case WSAEINPROGRESS:
		case WSAEMSGSIZE:
		case WSAENOBUFS:
			bClosed = false;
			break;
		default :
			bClosed = true;
			break;
		}
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			nError,
			0, // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);
		if (lpMsgBuf) {
//				SystemPrint(2,"P2p socket read error(%s) ", lpMsgBuf);
		}
		if (!bClosed) {
			return;
		}
	} else if ( 0 == nResult ) {
		bClosed = true;
	}
	if (bClosed) {
		Disconnect(&addr);
		return; // break;
	}
	{
		//////////////////////////////
		// for Test
		if (m_nDropRx > 0) {
			m_nDropRx--;
			return;
		}
		while (m_bSuspendRx) {
			Sleep(40);
		}
		//////////////////////////////
	}
	ProcessP2PPacket(pPacket, &addr);
}

void CP2PClientEx::TxHeartbeatToServer(void)
{
	PACKET_P2P_GENERIC packet;
	packet.InitData();
	packet.Init(m_nKey,MSGID_P2P_HEARTBEAT);
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.m_bFlag = P2P_FLAG_TX_SERVER;	//Server와의 
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	TxP2pPacket(&packet);	
}

void CP2PClientEx::TxHeartBeat(void)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	#ifndef NO_P2P_PEER_HEARTBEAT
	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(m_nKey,MSGID_P2P_HEARTBEAT);
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.m_bFlag = 0x0; // P2P_FLAG_HAND_SHAKE; // [-] 20070127 dEAthcURe
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	#endif

	/* [-] 20070127 dEAthcURe
	PACKET_P2P_TIME		packetTime;
	packetTime.InitData();
	packetTime.Init(m_nKey,MSGID_P2P_TIME_ADJUST);
	packetTime.m_nDestPeerKey = P2P_DEST_NONE;
	packetTime.m_bFlag = P2P_FLAG_HAND_SHAKE|P2P_FLAG_TX_SERVER|P2P_FLAG_USER_DEFINED_ACK;	//Server와의 
	packetTime.Complete(sizeof(PACKET_P2P_TIME));
	*/
	
	#ifndef P2P_PEER_HEARTBEAT
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe pi null check
			//if (pi->IsConnected()) { // [-] 20070124 dEAthcURe

			// {{ 20070220
			if(pi->heartbeatCount == nAppPacketToStopHeartbeating) {
				continue;
			}
			// }} 20070220

			if(pi->bUpdated) { // [+] 20070220
				packet.m_nDestPeerKey = pi->m_nKey;
				SOCKADDR_IN* pAddr = (SOCKADDR_IN*)(pi->m_bInSameNAT ? &pi->m_addrLocal : &pi->m_addrPublic);
				LogToFile("[TxHeartBeat] to %d %s:%d", pi->m_nKey, inet_ntoa(pAddr->sin_addr), ntohs(pAddr->sin_port));
				TxP2pPacket(&packet);
			}
			else {
				LogToFile("[TxHeartBeat] to %d, No addr info (%d)", pi->m_nKey, pi->m_nStage);
			}// [+] 20070220 [-] 20070124 dEAthcURe
		} // 20070522 dEAthcURe pi null check
	}
	#endif

	/* [-] 20070127 dEAthcURe
	packetTime.m_nT1 = m_tOrg;
	packetTime.m_nT2 = m_tRec;
	packetTime.m_nT3 = m_watch.GetTick();//GetTickCount();
	packetTime.m_nT4 = 0;
	m_tXmt = packetTime.m_nT3;
	TxP2pPacket(&packetTime);	
	*/
}

// {{ 20070129 dEAthcURe
void CP2PClientEx::TxHeartBeatV2(void)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	#ifndef NO_P2P_PEER_HEARTBEAT
	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(m_nKey,MSGID_P2P_HEARTBEAT);
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.m_bFlag = 0x0; // P2P_FLAG_HAND_SHAKE; // [-] 20070127 dEAthcURe
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
	#endif

	/* [-] 20070127 dEAthcURe
	PACKET_P2P_TIME		packetTime;
	packetTime.InitData();
	packetTime.Init(m_nKey,MSGID_P2P_TIME_ADJUST);
	packetTime.m_nDestPeerKey = P2P_DEST_NONE;
	packetTime.m_bFlag = P2P_FLAG_HAND_SHAKE|P2P_FLAG_TX_SERVER|P2P_FLAG_USER_DEFINED_ACK;	//Server와의 
	packetTime.Complete(sizeof(PACKET_P2P_TIME));
	*/
	
	#ifndef P2P_PEER_HEARTBEAT
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe
			// {{ 20070220
			if(pi->heartbeatCount == nAppPacketToStopHeartbeating) {
				continue;
			}
			// }} 20070220

			if(pi->bUpdated) { // [+] 20070220
				if(pi->heartbeatCount < nAppPacketToStopHeartbeating) {		
					packet.m_nDestPeerKey = pi->m_nKey;
					SOCKADDR_IN* pAddr = (SOCKADDR_IN*)(pi->m_bInSameNAT ? &pi->m_addrLocal : &pi->m_addrPublic);
					LogToFile("[TxHeartBeat] to %d %s:%d %d/%d", pi->m_nKey, inet_ntoa(pAddr->sin_addr), ntohs(pAddr->sin_port), pi->heartbeatCount, nAppPacketToStopHeartbeating);
					if(pi->timeBeginHeartbeat == 0x0) {
						time(&pi->timeBeginHeartbeat);
					}
					TxP2pPacket(&packet);
				}
			}
			else { // [+] 20070220
				LogToFile("[TxHeartBeat] to %d, No addr info (%d)", pi->m_nKey, pi->m_nStage);
			} // [+] 20070220
		} // 20070522 dEAthcURe
	}
	#endif

	/* [-] 20070127 dEAthcURe
	packetTime.m_nT1 = m_tOrg;
	packetTime.m_nT2 = m_tRec;
	packetTime.m_nT3 = m_watch.GetTick();//GetTickCount();
	packetTime.m_nT4 = 0;
	m_tXmt = packetTime.m_nT3;
	TxP2pPacket(&packetTime);	
	*/
}
// }} 20070129 dEAthcURe

//---------------------------------------------------------------------------
void CP2PClientEx::TxRttProbReq(int idAccount)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(m_nKey,MSGID_P2P_REQ_RTTPROB);
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.m_bFlag = 0x0;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));	
		
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe pi null check
			if (pi->m_nKey == idAccount && pi->IsConnected()) {
				packet.m_nDestPeerKey = pi->m_nKey;
				TxP2pPacket(&packet);
			}
		} // 20070522 dEAthcURe pi null check
	}	
}
//---------------------------------------------------------------------------
void CP2PClientEx::TxRttProbAck(void)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(m_nKey,MSGID_P2P_ACK_RTTPROB);
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.m_bFlag = 0x0;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));	
		
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe
			if (pi->IsConnected()) {
				packet.m_nDestPeerKey = pi->m_nKey;
				TxP2pPacket(&packet);
			}
		} // 20070522 dEAthcURe
	}	
}

#endif

void CP2PClientEx::RxPacket(int* pRun)
{
	int					nResult;
	char				pBuffer[DEFAULT_P2P_BUFFER_SIZE];
	SOCKADDR			addr;
	int					addr_len = sizeof(SOCKADDR);
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)pBuffer;

	memset((char *) &addr, 0, sizeof(SOCKADDR));
	while (THREAD_RUN == *pRun) {
		int		nError;
		bool bClosed = false; 
		nResult = recvfrom(m_sRcvSocket, pBuffer, DEFAULT_P2P_BUFFER_SIZE, 0, &addr, &addr_len);
		if (THREAD_RUN != *pRun) {
			return;
		}
		if (SOCKET_ERROR == nResult) {
			switch(nError = WSAGetLastError())
			{
			case WSAEWOULDBLOCK:
			case WSAEINPROGRESS:
			case WSAEMSGSIZE:
			case WSAENOBUFS:
				bClosed = false;
				break;
			default :
				bClosed = true;
				break;
			}
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				nError,
				0, // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			if (lpMsgBuf) {
				//SystemPrint(2,"P2p socket read error(%s) ", lpMsgBuf);
			}
			if (!bClosed) {
				continue;
			}
		} else if ( 0 == nResult ) {
			bClosed = true;
		}
		if (bClosed) {
			Disconnect(&addr);
			continue;
		}

		//////////////////////////////
		// for Test
		if (m_nDropRx > 0) {
			m_nDropRx--;
			continue;
		}
		while (m_bSuspendRx) {
			Sleep(40);
		}
		//////////////////////////////
		ProcessP2PPacket(pPacket, &addr);
	}
}
void CP2PClientEx::TxHeartBeat(int* pRun)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

#ifndef NO_P2P_PEER_HEARTBEAT
	PACKET_P2P_GENERIC	packet;
	packet.InitData();
	packet.Init(m_nKey,MSGID_P2P_HEARTBEAT);
	packet.m_nDestPeerKey = P2P_DEST_NONE;
	packet.m_bFlag = P2P_FLAG_HAND_SHAKE;
	packet.Complete(sizeof(PACKET_P2P_GENERIC));
#endif

	PACKET_P2P_TIME		packetTime;
	packetTime.InitData();
	packetTime.Init(m_nKey,MSGID_P2P_TIME_ADJUST);
	packetTime.Complete(sizeof(PACKET_P2P_TIME));
	packetTime.m_nDestPeerKey = P2P_DEST_NONE;
	packetTime.m_bFlag = P2P_FLAG_HAND_SHAKE|P2P_FLAG_TX_SERVER|P2P_FLAG_USER_DEFINED_ACK;	//Server와의 
	while (THREAD_RUN == *pRun) {
#ifndef P2P_PEER_HEARTBEAT
		const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
		{
			STRUCT_PEER_INFO* pi = it->second;
			if(pi) { // 20070522 dEAthcURe pi null check
				if (pi->IsConnected()) { 
					packet.m_nDestPeerKey = pi->m_nKey; 
					TxP2pPacket(&packet);
				}
			} // 20070522 dEAthcURe pi null check
		}
#endif
		{
			TLock lo(this);
			packetTime.m_nT1 = m_tOrg;
			packetTime.m_nT2 = m_tRec;
			if (m_nTimeAdjustRetry > 0) {
				packetTime.m_nT3 = m_tXmt;//GetTickCount();
			} else {
				packetTime.m_nT3 = m_watch.GetTick();//GetTickCount();
			}
			packetTime.m_nT4 = 0;
			m_tXmt = packetTime.m_nT3;
			SystemPrint(GAME_LOG, "Tx(m_nKey=%d, MSGID_P2P_TIME_ADJUST(%d,%d,%d,%d):(org=%d, rec=%d, xmt=%d)\n", 
					m_nKey, 
					packetTime.m_nT1, packetTime.m_nT2, packetTime.m_nT3, packetTime.m_nT4, 
					m_tOrg, m_tRec, m_tXmt);
			TxP2pPacket(&packetTime, false);
		}

		//SystemPrint(HEARTBEAT_LOG, "Tx(m_nKey=%d, MSGID_P2P_TIME_ADJUST(%d,%d,%d,%d):(org=%d, rec=%d, xmt=%d)", 
		//	m_nKey, 
		//	packetTime.m_nT1, packetTime.m_nT2, packetTime.m_nT3, packetTime.m_nT4, 
		//	m_tOrg, m_tRec, m_tXmt);
		WaitForSingleObject(m_heartbeatThreadTerminateEvent, 5000);//5초마다 보내보자.
	}
}
bool CP2PClientEx::ProcessP2PPacket(PACKET_P2P_GENERIC *pPacket, SOCKADDR* pAddr)
{
	if(pPacket == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return false;
	}

	int					nResult = P2P_PROCESS_OK;
	STRUCT_PEER_INFO*	pPeer = NULL;
	int					nTurnArround = 0;

	///////////////////////////////////////////////////
	// for debug Log Packet
	TCHAR strTemp[64];
	TCHAR strFlag[16];
	strTemp[0] = 0;
	GetLogMsgByMsgID(pPacket->MsgID(), strTemp, sizeof(strTemp));

	if (strTemp[0]) {
		strFlag[0] = 0;
		if (pPacket->m_bFlag & P2P_FLAG_HAND_SHAKE) {
			StringCchCopy(strFlag, 16, _T("|H"));
		}
		if (pPacket->m_bFlag & P2P_FLAG_USER_DEFINED_ACK) {
			StringCchCat(strFlag, 16, _T("|U"));
		}
		if (pPacket->m_bFlag & P2P_FLAG_PIGGY_BACK) {
			StringCchCat(strFlag, 16, _T("|P"));
		}
		if (pPacket->m_bFlag & P2P_FLAG_TX_SERVER) {
			StringCchCat(strFlag, 16, _T("|S"));
		}
		if (pPacket->m_bFlag & P2P_FLAG_RELAY) {
			StringCchCat(strFlag, 16, _T("|R"));
		}
		if (0 == strFlag[0]) {
			StringCchCopy(strFlag, 16, _T("|"));
		}
		if (pPacket->MsgID() != MSGID_P2P_HEARTBEAT && pPacket->MsgID() != MSGID_P2P_ACK_SEQ)
		{
			SystemPrint(2, "Rx:%d(%s(src=%d(%s:%d),flag=%s|,seq=%d,AckSeq=%d))\n", 
				pPacket->MsgID(), strTemp,
				pPacket->ConnectID(), inet_ntoa( ((SOCKADDR_IN *)pAddr)->sin_addr), ntohs(((SOCKADDR_IN *)pAddr)->sin_port), strFlag,
				pPacket->m_nSeq, pPacket->m_nAckSeq);
		}
	}
	// for debug Log Packet
	///////////////////////////////////////////////////
	TLock lo(this);

	pPeer = GetPeerInfo(pPacket->ConnectID());
	//if (m_serverInfo.m_nStage != P2P_SVR_ADDR_OK) {
	//	if (pPeer != &m_serverInfo) {
	//		Unlock();
	//		return true;
	//	}
	//}
	if (NULL == pPeer) {
		//printf("invalid peer %d msg:%d %s:%d\n", pPacket->ConnectID(), pPacket->MsgID(), inet_ntoa(((SOCKADDR_IN*)pAddr)->sin_addr), htons(((SOCKADDR_IN*)pAddr)->sin_port)); // [+] 20070612
		return true;
	}

	if (P2P_FLAG_PIGGY_BACK&pPacket->m_bFlag) {	//ACK정보가 포함된 Packet이 도착했으면
		int	nAckSeq = pPacket->m_nAckSeq;
		int	nNextSeq = (nAckSeq + 1)%256;
		//SystemPrint(2,"Rx ACK :(AckSeq=%d)",nAckSeq);
		if (NULL == pPeer) {
			return false;
		}
		m_tRxPacket = GetServerTick();
		if (nAckSeq == pPeer->m_nAckSeq) {	//기다리고 있던 ACK가 도착했으면
			pPeer->KillAckTimer();
			pPeer->m_nRetryCount = 0;
			if (pPeer->m_pBuffer[nAckSeq]) {	//ClearSessionBuffer가 불리기직전에 날린 패킷의 ACK가 오는경우 null일수 있다.
				pPeer->m_nTurnArroundTime = m_tRxPacket - pPeer->m_pBuffer[nAckSeq]->m_nTxTime;
				nTurnArround = pPeer->m_nTurnArroundTime;	// 다시 검색해서 찾을려면 귀찮으니 TurArroundTime도 알려주자.
				m_packetBuffer.Free(pPeer->m_pBuffer[nAckSeq]);
				pPeer->m_pBuffer[nAckSeq] = NULL;
			} else {
				SystemPrint(2, "직전에 ClearSessionBuffer(%d)가 불렸는지, ACK온 패킷에 해당하는 buffer가 없다\n", pPeer->m_nKey);
			}
			pPeer->m_nAckSeq = (BYTE)nNextSeq;	//다음에 보낼패킷이 있는지 여부에 상관없이 일단 nAckSeq는 처리 되었으므로
												//혹시나 한번더 같은 ack가 날아오더라도 오동작 하지 않도록 하기 위해 
												//pPeer->m_nAckSeq를 1 증가 시켜 놓는다. 어차피 TxPacket할때 다시 세팅되더라도 아무런 
												//문제가 없다.
			if (pPeer->m_pBuffer[nNextSeq]) {
				//보낼 패킷이 있다.
				PACKET_P2P_GENERIC* pPkt = (PACKET_P2P_GENERIC *)(pPeer->m_pBuffer[nNextSeq]->m_pBuffer);
				SystemPrint(2, "Tx Sending Booked packet(MsgID=%d, seq=%d)\n", pPkt->MsgID(), nNextSeq);
				pPeer->m_nTimerID = CreateTimeSetEventForTx();
				TxVolatilePacket(pPkt, pPeer);
			}
		}	//if (nAckSeq == pPeer->m_nAckSeq) {
	}	//	if (P2P_FLAG_PIGGY_BACK&pPacket->m_bFlag)

	if (P2P_FLAG_HAND_SHAKE&pPacket->m_bFlag) {	//Reliable Packet이 도착했으면
		if (NULL == pPeer) {
			return false;
		}
		if ((P2P_FLAG_USER_DEFINED_ACK&pPacket->m_bFlag) == 0) {
			PACKET_P2P_GENERIC	packet;
			packet.InitData();
			packet.Init(m_nKey, MSGID_P2P_ACK_SEQ);
			packet.m_bFlag = P2P_FLAG_PIGGY_BACK;
			if (P2P_FLAG_RELAY&pPacket->m_bFlag) {	//relay로 온놈은 relay로 ACK를 보낸다.
				packet.m_bFlag |= P2P_FLAG_RELAY;
			}
			packet.m_nAckSeq = pPacket->m_nSeq;
			packet.m_nDestPeerKey = pPacket->ConnectID();
			packet.m_nTick = GetServerTick();
			packet.Complete(sizeof(PACKET_P2P_GENERIC));
			TxVolatilePacket(&packet,pPeer);
			if (pPeer->m_nRxSeq == pPacket->m_nSeq) {	//이미 처리한 패킷이 재전송되어 온경우 무시한다.
				SystemPrint(2, "이미 ACK를 보낸 packet(seq=%d)이 peer(%d)로 부터 다시 전송되어왔습니다\n", pPacket->m_nSeq, pPacket->ConnectID());
				return true;
			}
			pPeer->m_nRxSeq = pPacket->m_nSeq;
#ifdef _BF_SERVER
			nxlogmsg(_T("Setting pPeer->m_nRxSeq=%d"), pPacket->m_nRxSeq);
#endif

		} else {		//사용자가 ACK를 처리 하는 경우에는 이미 도착한 패킷이라도 NOTI해줘서 사용자가 적절한 ACK를 보낼 수 있도록한다.
			nResult = DefaultPacketProc(pPacket, pPeer, pAddr);
			if ((P2P_PROCESS_NOTIFY_USER & nResult) && m_pHandler){
				if (pPeer->IsConnected()) {
					nResult = m_pHandler->Execute((DWORD)(DWORD_PTR)pPacket, (DWORD)nTurnArround);
				}
			}
			if (pPeer->m_nRxSeq == pPacket->m_nSeq) {	//이미 처리한 패킷이 재전송되어 온경우 무시한다.
				SystemPrint(2, "이미 ACK를 보낸 packet(seq=%d)이 peer(%d)로 부터 다시 전송되어왔습니다\n", pPacket->m_nSeq, pPacket->ConnectID());
				return true;
			}
			pPeer->m_nRxSeq = pPacket->m_nSeq;
#ifdef _BF_SERVER
			nxlogmsg(_T("Setting pPeer->m_nRxSeq=%d"), pPacket->m_nRxSeq);
#endif
			return true;
		}
	}
	nResult = DefaultPacketProc(pPacket,pPeer, pAddr);
	if ((P2P_PROCESS_NOTIFY_USER & nResult) && m_pHandler){
		if (pPeer->IsConnected()) {
			nResult = m_pHandler->Execute((DWORD)(DWORD_PTR)pPacket, (DWORD)nTurnArround);
		}
	}

	return true;
}

void CP2PClientEx::OnTimer(UINT nID)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	int					nResult=P2P_PROCESS_OK;
	STRUCT_PEER_INFO*	pPeerInfo=NULL;
	SystemPrint(2, "CP2PClientEx::OnTimer\n");
	TLock lo(this);

	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();

	if ((int)nID == m_serverInfo.m_nTimerID) {
		pPeerInfo = &m_serverInfo;
	} else {
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
		{
			STRUCT_PEER_INFO* pi = it->second;
			if(pi) { // 20070522 dEAthcURe pi null check
				if ( (pi->m_nTimerID == (int)nID ) ||
					(pi->m_nDirectRqTimerID == (int)nID ) ||
					(pi->m_nPunchHoleTimerID == (int)nID ) ||
					(pi->m_nHoleScanTimerID == (int)nID ) ||
					(pi->m_nRelayPrepareTimer == (int)nID ) )
				{
					pPeerInfo = pi;
					break;
				}
			} // 20070522 dEAthcURe pi null check
		}
	}
	if (NULL == pPeerInfo) {
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
		{
			STRUCT_PEER_INFO* pi = it->second;
			if(pi) { // 20070522 dEAthcURe pi null check
				SystemPrint(2,"peer(key=%d, timerid = %d)\n", pi->m_nKey, pi->m_nTimerID);
			} // 20070522 dEAthcURe pi null check
		}
		SystemPrint(2, "(NULL == pPeerInfo) CP2PClientEx::OnTimer\n");
		return;
	}
	nResult = DefaultTimerProc(nID, pPeerInfo);	//일반적인 Packet재전송용 Timer가 아닌 다른 용도로 Timer가 사용됐는지 점검하고
	if (P2P_PROCESS_TIMER&nResult) {			//그렇다면
		SystemPrint(2, "}//(P2P_PROCESS_TIMER&nResult) CP2PClientEx::OnTimer\n");
		return;									//린턴한다.
	}
	int	nSeq = pPeerInfo->m_nAckSeq;
	if ((int)nID == pPeerInfo->m_nTimerID) {
		if (pPeerInfo->m_nRetryCount < P2P_TX_RETRY_COUNT) {
			pPeerInfo->m_nRetryCount++;
			pPeerInfo->m_nTimerID = CreateTimeSetEventForTx();
			SystemPrint(2, "Timer Expire retransmit. retrycnt=%d. dest=%d. seq=%d\n", 
				pPeerInfo->m_nRetryCount, pPeerInfo->m_nKey, nSeq);

			TxVolatilePacket( (PACKET_P2P_GENERIC *)(pPeerInfo->m_pBuffer[nSeq]->m_pBuffer), pPeerInfo);
		} else {	
			//////////////////////////////////
			//3회 재전송 시도가 실패했을경우
			SystemPrint(2, "Packet(to:%d) Loss detected\n", pPeerInfo->m_nKey);
			pPeerInfo->m_nTimerID = 0;
			nResult = DefaultPacketLossProc((PACKET_P2P_GENERIC *)(pPeerInfo->m_pBuffer[nSeq]->m_pBuffer), pPeerInfo);
			if ( (P2P_PROCESS_NOTIFY_USER & nResult) && m_pLossHandler) {
				m_pLossHandler->Execute((DWORD)(DWORD_PTR)(pPeerInfo->m_pBuffer[nSeq]->m_pBuffer), (DWORD)pPeerInfo->m_nKey);
			} else if ( P2P_PROCESS_IGNORE_TIME_ADJUST &  nResult) {
				int	nNextSeq = (nSeq + 1)%256;
				m_packetBuffer.Free(pPeerInfo->m_pBuffer[nSeq]);
				pPeerInfo->m_pBuffer[nSeq] = NULL;
				pPeerInfo->m_nAckSeq = (BYTE)nNextSeq;	//다음에 보낼패킷이 있는지 여부에 상관없이 일단 nAckSeq는 처리 되었으므로
				if (pPeerInfo->m_pBuffer[nNextSeq]) {
					//보낼 패킷이 있다.
					PACKET_P2P_GENERIC* pPkt = (PACKET_P2P_GENERIC *)(pPeerInfo->m_pBuffer[nNextSeq]->m_pBuffer);
					SystemPrint(2, "Tx Sending Booked packet(MsgID=%d, seq=%d)\n", pPkt->MsgID(), nNextSeq);
					pPeerInfo->m_nTimerID = CreateTimeSetEventForTx();
					TxVolatilePacket(pPkt, pPeerInfo);
				}
			}
			//////////////////////////////////
		}
	}
	SystemPrint(2, "} CP2PClientEx::OnTimer\n");
}

int CP2PClientEx::DefaultPacketProc(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pSender, SOCKADDR* pAddr)
{
	if (NULL == pSender || pPacket == 0x0) {
		return P2P_PROCESS_OK;
	}

	switch (pPacket->MsgID()) {		
		case MSGID_P2P_REQ_RESPONSE :
			{
				SystemPrint(2, "Rx(MSGID_P2P_REQ_RESPONSE(seq=%d) from :%d)\n",pPacket->m_nSeq, pPacket->ConnectID());
				RxDirectRq(pPacket, pSender, pAddr);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_ACK_RESPONSE :
			{
				SystemPrint(2, "Rx(MSGID_P2P_ACK_RESPONSE(Ack seq=%d) from :%d)\n",pPacket->m_nAckSeq, pPacket->ConnectID());
				RxDirectAck(pSender, pAddr);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_CONFIRM_RESPONSE :
			{
				SystemPrint(2, "Rx(MSGID_P2P_CONFIRM_RESPONSE from :%d)\n", pPacket->ConnectID());
				RxDirectConfirm(pSender, pAddr);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_REQ_PUNCHHOLE:
			{
				SystemPrint(2, "Rx(MSGID_P2P_REQ_PUNCHHOLE from :%d)\n", pPacket->ConnectID());
				/////////////////////////////////////////////////////////////////
				// P2P_FLAG_PIGGY_BACK을 켠 응답packet을 일단 relay로 응답한다.
				PACKET_P2P_GENERIC	packet;
				packet.InitData();
				packet.Init(m_nKey, MSGID_P2P_ACK_REQ_PUNCHHOLE);
				packet.m_bFlag = P2P_FLAG_PIGGY_BACK;
				packet.m_bFlag |= P2P_FLAG_RELAY;	//예는 무조건 relay로만 온다.
				packet.m_nAckSeq = pPacket->m_nSeq;
				packet.m_nDestPeerKey = pPacket->ConnectID();
				packet.m_nTick = GetServerTick();
				packet.Complete(sizeof(PACKET_P2P_GENERIC));
				TxVolatilePacket(&packet,pSender);
				/////////////////////////////////////////////////////////////////
				RxRqHolePunching((PACKET_P2P_RQ_HOLE_PUNCH *)pPacket, pSender);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_PUNCH :
			{
				SystemPrint(2, "Rx(MSGID_P2P_PUNCH from :%d)\n", pPacket->ConnectID());
				RxPunch(pSender, (PACKET_P2P_HOLE_PUNCH *)pPacket);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_HOLE_SCAN :
			{
				SystemPrint(2, "Rx(MSGID_P2P_HOLE_SCAN from :%d)\n", pPacket->ConnectID());
				RxHoleScan(pSender, (PACKET_P2P_HOLE_SCAN *)pPacket, pAddr);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_ACK_PUNCHHOLE :
			{
				SystemPrint(2, "Rx(MSGID_P2P_ACK_PUNCHHOLE from :%d)\n", pPacket->ConnectID());
				RxHolePunchAck(pSender, pAddr);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_ACK_CONNECT :
			{
				SystemPrint(2, "Rx(MSGID_P2P_ACK_CONNECT from :%d)\n", pPacket->ConnectID());
				if (P2P_SVR_CONNECT_TRY == pSender->m_nStage) {
					PACKET_P2P_ACK_CONNECT*	pConnectAckPacket = (PACKET_P2P_ACK_CONNECT *)pPacket;
					pSender->m_nStage = P2P_SVR_CONNECT;
					memcpy(&m_addrPublic, &(pConnectAckPacket->m_addrPublic), sizeof(SOCKADDR));
					SystemPrint(2,"My Public IP=%s:%d\n",inet_ntoa( ((SOCKADDR_IN *)&(m_addrPublic))->sin_addr), ntohs(((SOCKADDR_IN *)&(m_addrPublic))->sin_port ));
					///////////////////////////////////////////////
					// time sync관련
					m_tOrg = 0;
					m_tRec = 0;
					m_tXmt = 0;
					m_nTickIndex = 0;
					m_tServerOffset = 0;
					m_nTimeAdjustRetry = 0;
					for (int i = 0; i < 8; i++) {
						m_tRoundTripDelay[i] = INT_MAX;
						//m_tClockOffset[i] = INT_MAX;
					}
					m_tDelta0 = INT_MAX-1;
					//m_tTheta0 = INT_MAX-1;
					///////////////////////////////////////////////
					#ifdef EnableEmbedding
					bHeartbeat = true;	
					#else					
					BeginThread();					
					#endif
					if (m_pNotifier) {
						m_pNotifier->Execute((DWORD)P2P_SVR_CONNECT, NULL);
					}
				}
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_PEER_ADDR_OK :
			{
				SystemPrint(2, "Rx(MSGID_P2P_PEER_ADDR_OK(seq=%d) from :%d)\n",pPacket->m_nSeq, pPacket->ConnectID());
				if (P2P_SVR_ADDR_TRY == pSender->m_nStage) {
					pSender->m_nStage = P2P_SVR_ADDR_OK;
					RxPeerAddrOk((PACKET_P2P_PEER_INFO_OK *)pPacket);
				}
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_TIME_ADJUST :
			{
				SystemPrint(2, "Rx(MSGID_P2P_TIME_ADJUST from :%d)\n", pPacket->ConnectID());
				RxTimeAdjust(pSender, (PACKET_P2P_TIME *) pPacket);
				return P2P_PROCESS_OK;
			}
		case MSGID_P2P_ACK_REQ_PUNCHHOLE:
			{
				if (0 == pSender->m_nRelayPrepareTimer) {
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultPacketProc|MSGID_P2P_ACK_REQ_PUNCHHOLE] timeSetEvent for %d\n", pSender->m_nKey);
					pSender->m_nRelayPrepareTimer = timeSetEvent(4*P2P_RETRANSMIT_INTERVAL+P2P_PUNCHHOLE_RETRY_INTERVAL*4+P2P_RELAY_PREPARE_INTERVAL, 50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
				}
				return P2P_PROCESS_OK;
			}
			/*
		// {{ 20070129 dEAthcURe
		case MSGID_P2P_HeartbeatV2:
			{
				const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
				for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++) {
					STRUCT_PEER_INFO* pi = it->second;
					if (pi && pPacket->ConnectID() == pi->m_nKey) {
						PACKET_HeartbeatV2* pHeartbeat = (PACKET_HeartbeatV2*)pPacket;
						if(pHeartbeat) {
							pi->heartbeatCount = pHeartbeat->count + 1;
						}
						return P2P_PROCESS_OK;
					}
				}
			}
		// }} 20070129 dEAthcURe
		*/
	}
	return P2P_PROCESS_NOTIFY_USER;
}

int CP2PClientEx::DefaultTimerProc(UINT nID, STRUCT_PEER_INFO* pPeerInfo)
{
	if (pPeerInfo == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return P2P_PROCESS_OK;
	}

	switch( pPeerInfo->m_nStage ) {
		case P2P_SESSION_DIRECT_TRY0 :
			{
				if ((int)nID == pPeerInfo->m_nDirectRqTimerID) {
					pPeerInfo->m_nStage = P2P_SESSION_DIRECT_TRY1;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_DIRECT_TRY0 timeSetEvent for %d\n", pPeerInfo->m_nKey);
					pPeerInfo->m_nDirectRqTimerID = timeSetEvent(P2P_DIRECT_RQ_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxDirectRq(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_DIRECT_TRY1 :
			{
				if ((int)nID == pPeerInfo->m_nDirectRqTimerID) {
					pPeerInfo->m_nStage = P2P_SESSION_DIRECT_TRY2;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_DIRECT_TRY1 timeSetEvent for %d\n", pPeerInfo->m_nKey);
					pPeerInfo->m_nDirectRqTimerID = timeSetEvent(P2P_DIRECT_RQ_INTERVAL*2,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxDirectRq(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_DIRECT_TRY2 :
			{
				if ((int)nID == pPeerInfo->m_nDirectRqTimerID) {
					pPeerInfo->m_nDirectRqTimerID = 0;
					RequestHolePunching(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_PUNCHING_TRY0 :
			{
				if ((int)nID == pPeerInfo->m_nPunchHoleTimerID) {
					pPeerInfo->m_nStage = P2P_SESSION_HOLE_PUNCHING_TRY1;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_HOLE_PUNCHING_TRY0 timeSetEvent for %d\n", pPeerInfo->m_nKey);
					pPeerInfo->m_nPunchHoleTimerID = timeSetEvent(P2P_PUNCHHOLE_RETRY_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxPunchHoleProbePacket(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_PUNCHING_TRY1 :
			{
				if ((int)nID == pPeerInfo->m_nPunchHoleTimerID) {
					pPeerInfo->m_nStage = P2P_SESSION_HOLE_PUNCHING_TRY2;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_HOLE_PUNCHING_TRY1 timeSetEvent for %d\n", pPeerInfo->m_nKey);
					pPeerInfo->m_nPunchHoleTimerID = timeSetEvent(P2P_PUNCHHOLE_RETRY_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxPunchHoleProbePacket(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_PUNCHING_TRY2 :
			{
				if ((int)nID == pPeerInfo->m_nPunchHoleTimerID) {
					pPeerInfo->m_nPunchHoleTimerID = 0;
					if (IsTryingTurn()) {
						pPeerInfo->m_nStage = P2P_SESSION_RELAY_PREPARE;						
						SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_HOLE_PUNCHING_TRY2 timeSetEvent for %d\n", pPeerInfo->m_nKey);
						pPeerInfo->m_nRelayPrepareTimer = timeSetEvent(P2P_RELAY_PREPARE_INTERVAL*2,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);

					} else {
						pPeerInfo->m_nStage = P2P_SESSION_DISCONNECTED;	//처리하고 사용자에게는 알려만 주자.						
						if (m_pNotifier) {
							m_pNotifier->Execute((DWORD)P2P_SESSION_DISCONNECTED, (DWORD)pPeerInfo->m_nKey);
						}
					}
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_SCAN0 :
			{
				if ((int)nID == pPeerInfo->m_nHoleScanTimerID) {
					pPeerInfo->m_nStage = P2P_SESSION_HOLE_SCAN1;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_HOLE_SCAN0 timeSetEvent for %d\n", pPeerInfo->m_nKey);
					pPeerInfo->m_nHoleScanTimerID = timeSetEvent(P2P_PUNCHHOLE_RETRY_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxHoleScan(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_SCAN1 :
			{
				if ((int)nID == pPeerInfo->m_nHoleScanTimerID) {
					pPeerInfo->m_nStage = P2P_SESSION_HOLE_SCAN2;
					SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_HOLE_SCAN1 timeSetEvent for %d\n", pPeerInfo->m_nKey);
					pPeerInfo->m_nHoleScanTimerID = timeSetEvent(P2P_PUNCHHOLE_RETRY_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					TxHoleScan(pPeerInfo);
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_SCAN2 :
			{
				if ((int)nID == pPeerInfo->m_nHoleScanTimerID) {
					pPeerInfo->m_nHoleScanTimerID = 0;
					if (IsTryingTurn()) {
						pPeerInfo->m_nStage = P2P_SESSION_RELAY_PREPARE;						
						SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultTimerProc] P2P_SESSION_HOLE_SCAN2 timeSetEvent for %d\n", pPeerInfo->m_nKey);
						pPeerInfo->m_nRelayPrepareTimer = timeSetEvent(P2P_RELAY_PREPARE_INTERVAL,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					} else {
						pPeerInfo->m_nStage = P2P_SESSION_DISCONNECTED;	//처리하고 사용자에게는 알려만 주자.						
						if (m_pNotifier) {
							m_pNotifier->Execute((DWORD)P2P_SESSION_DISCONNECTED, (DWORD)pPeerInfo->m_nKey);
						}
					}
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_RELAY_PREPARE :
			{
				if ((int)nID == pPeerInfo->m_nRelayPrepareTimer) {
					pPeerInfo->m_nRelayPrepareTimer = 0;
					if (IsTryingTurn()) { // [+] 20070621 dEAthcURe						
						pPeerInfo->m_nStage = P2P_SESSION_RELAY_ESTABLISHED;
						ClearSessionBuffer(pPeerInfo);
						if (m_pNotifier) {
							m_pNotifier->Execute((DWORD)P2P_SESSION_RELAY_ESTABLISHED, (DWORD)pPeerInfo->m_nKey);
						}
					}
					else { // [+] 20070621 dEAthcURe
						pPeerInfo->m_nStage = P2P_SESSION_DISCONNECTED;	//처리하고 사용자에게는 알려만 주자.						
						if (m_pNotifier) {
							m_pNotifier->Execute((DWORD)P2P_SESSION_DISCONNECTED, (DWORD)pPeerInfo->m_nKey);
						}
					}
					return P2P_PROCESS_TIMER;
				}
				break;
			}
		case P2P_SESSION_HOLE_PUNCHING_REQ:
			{
				if ((int)nID == pPeerInfo->m_nRelayPrepareTimer) {
					pPeerInfo->m_nRelayPrepareTimer = 0;
					pPeerInfo->m_nStage = P2P_SESSION_RELAY_ESTABLISHED;
					SystemPrint(2,"P2P_SESSION_RELAY_ESTABLISHED\n");
					ClearSessionBuffer(pPeerInfo);
					if (m_pNotifier) {
						m_pNotifier->Execute((DWORD)P2P_SESSION_RELAY_ESTABLISHED, (DWORD)pPeerInfo->m_nKey);
					}
					return P2P_PROCESS_TIMER;
				}
				break;
			}
	}
	return P2P_PROCESS_OK;
}

int CP2PClientEx::DefaultPacketLossProc(PACKET_P2P_GENERIC* pPacket, STRUCT_PEER_INFO* pPeer)
{
	SystemPrint(2, "DefaultPacketLossProc\n");
	if (NULL == pPeer || pPacket == 0x0) {
		return P2P_PROCESS_OK;
	}
	if(P2P_DEST_NONE == pPeer->m_nKey) {
		if (pPacket->MsgID() ==  MSGID_P2P_TIME_ADJUST) {
			//
			m_nTimeAdjustRetry++;
			SystemPrint(2, "m_nTimeAdjustRetry=%d\n", m_nTimeAdjustRetry);
			return P2P_PROCESS_IGNORE_TIME_ADJUST;
		}
		pPeer->m_nStage = P2P_SVR_DISCONNECT;
		if (m_pNotifier) {
			if (MSGID_P2P_REQ_CONNECT == pPacket->MsgID()) {
				m_pNotifier->Execute((DWORD)P2P_SVR_DISCONNECT, (DWORD)MSGID_P2P_REQ_CONNECT);
			} else {
				m_pNotifier->Execute((DWORD)P2P_SVR_DISCONNECT, (DWORD)P2P_DEST_NONE);
			}
		}
		return P2P_PROCESS_OK;
	} else {
		switch (pPacket->MsgID()) {
			case MSGID_P2P_CONFIRM_RESPONSE :
				SystemPrint(2,"MSGID_P2P_CONFIRM_RESPONSE Loss detect(stage=%d)\n", pPeer->m_nStage);
				if (P2P_SESSION_DIRECT_ESTABLISHED == pPeer->m_nStage) {
					RequestHolePunching(pPeer);
				}
				return P2P_PROCESS_OK;
			case MSGID_P2P_REQ_PUNCHHOLE :
				if (P2P_SESSION_HOLE_PUNCHING_REQ == pPeer->m_nStage) {
					if (IsTryingTurn()) {
						pPeer->m_nStage = P2P_SESSION_RELAY_PREPARE;
						SystemPrint(ERROR_LOG, "[CP2PClientEx::DefaultPacketLossProc] MSGID_P2P_REQ_PUNCHHOLE timeSetEvent for %d\n", pPeer->m_nKey);
						pPeer->m_nRelayPrepareTimer = timeSetEvent(P2P_RELAY_PREPARE_INTERVAL*2,50,StaticOnTimer,(DWORD_PTR)this,TIME_ONESHOT);
					} else {
						pPeer->m_nStage = P2P_SESSION_DISCONNECTED;	//처리하고 사용자에게는 알려만 주자.
						if (m_pNotifier) {
							m_pNotifier->Execute((DWORD)P2P_SESSION_DISCONNECTED, (DWORD)pPeer->m_nKey);
						}
					}
				} else {
					SystemPrint(2, "MSGID_P2P_REQ_RESPONSE가 소실되었는대, 상태는 P2P_SESSION_DIRECT_TRY이 아니다.무슨일일까?\n");
				}
				return P2P_PROCESS_OK;
			default :
				if (false == pPeer->IsConnected()) {
					SystemPrint(2, "연결이되지 않았는데, packet(k=%d, M=%d)가 손실되었다\n", pPacket->ConnectID(), pPacket->MsgID());
					return P2P_PROCESS_OK;
				}
		}
	}
	return P2P_PROCESS_NOTIFY_USER;
}
void CP2PClientEx::Disconnect(int nKey)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2, "Disconnect(%d)\n", nKey);
	m_pPeerInfoMgr->RemovePeer(nKey);

	m_nCurrentPeer--;

	if (0 <= m_nCurrentPeer) {
		RequestExit();
	}
}
void CP2PClientEx::Disconnect(SOCKADDR *pAddr)
{
	if(pAddr == 0x0 || m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2, "Disconnect(%s)\n", inet_ntoa( ((SOCKADDR_IN*)pAddr)->sin_addr )  );
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe pi null check
			if (memcmp(&(pi->m_addrPublic), pAddr, sizeof(SOCKADDR)) == 0) {
				Disconnect(pi->m_nKey);
				break;
			}
		} // 20070522 dEAthcURe pi null check
	}
}

void CP2PClientEx::StopOperation()
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	__try {
		if (!bEmbedded && INVALID_SOCKET != m_sRcvSocket) {
			closesocket(m_sRcvSocket);
			m_sRcvSocket = INVALID_SOCKET;
		}

		if (m_serverInfo.m_nTimerID) {
			timeKillEvent(m_serverInfo.m_nTimerID);
			m_serverInfo.m_nTimerID = 0;
		}

	
		const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
		{
			STRUCT_PEER_INFO* pi = it->second;
			if(pi) { // 20070522 dEAthcURe pi null check 
				//__try { // [!] 20070529
					if (pi->m_nTimerID) {
						timeKillEvent(pi->m_nTimerID);
						pi->m_nTimerID = 0;						
					}
					// {{ 20070621 dEAthcURe
					if (pi->m_nDirectRqTimerID) {
						timeKillEvent(pi->m_nDirectRqTimerID);
						pi->m_nDirectRqTimerID = 0;						
					}
					if (pi->m_nPunchHoleTimerID) {
						timeKillEvent(pi->m_nPunchHoleTimerID);
						pi->m_nPunchHoleTimerID = 0;						
					}
					if (pi->m_nHoleScanTimerID) {
						timeKillEvent(pi->m_nHoleScanTimerID);
						pi->m_nHoleScanTimerID = 0;						
					}
					if (pi->m_nRelayPrepareTimer) {
						timeKillEvent(pi->m_nRelayPrepareTimer);
						pi->m_nRelayPrepareTimer = 0;						
					}
					// }} 20070621 dEAthcURe
					pi->m_bInSameNAT = true;
				/* // [!] 20070529
				}
				__except(EXCEPTION_EXECUTE_HANDLER) {
					unsigned long ec = GetExceptionCode();
					if(ec == EXCEPTION_ACCESS_VIOLATION) SystemPrint(ERROR_LOG, "-SEH-[CP2PClientEx::StopOperation] EXCEPTION_ACCESS_VIOLATION has occured while killing event of pi=0x%x\n", pi);
					else SystemPrint(ERROR_LOG, "-SEH-[CP2PClientEx::StopOperation] exception:%d has occured while killing event of pi=0x%x\n", ec, pi);
				}
				*/
			} // 20070522 dEAthcURe pi null check 
		}

		m_pPeerInfoMgr->RemoveAllPeer();

		SystemPrint(2, "CP2PClientEx::StopOperation\n");
		SetEvent(m_heartbeatThreadTerminateEvent);
		StopThread();
		m_nCurrentPeer = 0;	
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		unsigned long ec = GetExceptionCode();
		if(ec == EXCEPTION_ACCESS_VIOLATION) SystemPrint(ERROR_LOG, "-SEH-[CP2PClientEx::StopOperation] EXCEPTION_ACCESS_VIOLATION has occured\n");
		else SystemPrint(ERROR_LOG, "-SEH-[CP2PClientEx::StopOperation] exception:%d has occured\n", ec);
	}
}

void CP2PClientEx::ClearSessionBuffer(STRUCT_PEER_INFO* pStruct)
{
	int	nSeq;
	if (NULL == pStruct) return;
	TLock lo(this);

	pStruct->KillAllTimer();

	//nSeq = (pStruct->m_nAckSeq + 1)%256;
	nSeq = pStruct->m_nAckSeq;	//타이머를 죽였으므로 이미 날리고 ACK를 기다리는 중인 packet이 있더라도 얘도 날려버리자.
	while(pStruct->m_pBuffer[nSeq]) {
		m_packetBuffer.Free(pStruct->m_pBuffer[nSeq]);
		pStruct->m_pBuffer[nSeq] = NULL;
		nSeq = (nSeq + 1)%256;
	}
	//pStruct->m_nAckSeq = pStruct->m_nSeq;
	pStruct->m_nSeq = pStruct->m_nAckSeq;	//어차피 m_nAckSeq < n <= m_nSeq인 놈들은 날라가지도 못했던 놈이니 m_nSeq를 내리자.
	SystemPrint(2, "ClearSessionBuffer(%d(m_nSeq(%d), m_nRxSeq(%d))\n",pStruct->m_nKey, pStruct->m_nSeq, pStruct->m_nRxSeq);
}

void CP2PClientEx::RegisterPeer(int nKey, SOCKADDR* pAddr)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2,"RegisterPeer(%d)\n", nKey);
	TLock lo(this);

	if (m_nCurrentPeer+1 >= m_nMaxPeer) {
		return;
	}
	STRUCT_PEER_INFO* pOrg = m_pPeerInfoMgr->GetPeerInfo(nKey);
	if (pOrg)
	{
		return;
	}


	STRUCT_PEER_INFO* pi = new STRUCT_PEER_INFO;

	if(pi) { // [+] 20070531 dEAthcURe|P2p
		SystemPrint(2, "(CP2PClientEx::RegisterPeer) key=%d, m_nStage=%d\n", 
			nKey, pi->m_nStage);
		
		pi->m_nKey = nKey;
		pi->SetAddrLocal(pAddr);
		pi->SetAddrPublic(pAddr);

		m_pPeerInfoMgr->AddPeer(nKey, pi);

		m_nCurrentPeer++;
	}
}

void CP2PClientEx::RegisterPeer(int nKey, SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	SystemPrint(2,"RegisterPeer2(%d)\n", nKey);
	TLock lo(this);

	if (m_nCurrentPeer+1 >= m_nMaxPeer) {
		return;
	}
	STRUCT_PEER_INFO* pOrg = m_pPeerInfoMgr->GetPeerInfo(nKey);
	if (pOrg)
	{
		return;
	}


	STRUCT_PEER_INFO* pi = new STRUCT_PEER_INFO;
	if(pi) { // [+] 20070531 dEAthcURe|P2p
		SystemPrint(2, "(CP2PClientEx::RegisterPeer2) key=%d, m_nStage=%d\n", nKey, pi->m_nStage);
		
		pi->m_nKey = nKey;
		pi->SetAddrLocal(pAddrLocal);
		pi->SetAddrPublic(pAddrPublic);

		// {{ 20070405
		if ( ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_addr.S_un.S_addr == ((SOCKADDR_IN *)&m_addrPublic)->sin_addr.S_un.S_addr) {
			SystemPrint(2,"peer(%d:%s) is in same NAT\n", pi->m_nKey, inet_ntoa( ((SOCKADDR_IN *)&(pi->m_addrLocal))->sin_addr) );
			pi->m_bInSameNAT = true;
		} else {
			SystemPrint(2,"peer(%d:%s) is not in same NAT\n", pi->m_nKey, inet_ntoa( ((SOCKADDR_IN *)&(pi->m_addrLocal))->sin_addr) );
			pi->m_bInSameNAT = false;
		}
		// }} 20070405

		m_pPeerInfoMgr->AddPeer(nKey, pi);

		m_nCurrentPeer++;
	}
}

void CP2PClientEx::RemovePeer(int nKey)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	TLock lo(this);

	STRUCT_PEER_INFO* pi = m_pPeerInfoMgr->GetPeerInfo(nKey);
	assert(pi);

	SystemPrint(2, "mykey=%d, RemovePeer key=%d\n", m_nKey, nKey);
	ClearSessionBuffer(pi);
	m_pPeerInfoMgr->RemovePeer(nKey);
	m_nCurrentPeer--;
}

void CP2PClientEx::RemoveAllPeer()
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return;
	}

	TLock lo(this);

	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe pi null check
			ClearSessionBuffer(pi);
			SystemPrint(ERROR_LOG, "-MBS-[CP2PClientEx::RemoveAllPeer] ClearSessionBuffer cleared for pi=0x%x\n", pi);
		} // 20070522 dEAthcURe pi null check
		else {
			SystemPrint(ERROR_LOG, "-MBS-[CP2PClientEx::RemoveAllPeer] Invalid pi\n");
		}
	}

	m_pPeerInfoMgr->RemoveAllPeer();
	m_nCurrentPeer = 0;
}

bool CP2PClientEx::IsAllPeerConnected(void)
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return false;
	}

	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++)
	{
		STRUCT_PEER_INFO* pi = it->second;
		if(pi) { // 20070522 dEAthcURe pi null check
			if (pi->m_nKey < 0) continue;
			if (false == pi->IsConnected()) {
				if (false == pi->IsDisconnected()) {
					return false;
				}
			}
		} // 20070522 dEAthcURe pi null check
	}

	return true;
}

int CP2PClientEx::GetPeerCount()
{
	if(m_pPeerInfoMgr == 0x0) { // [+] 20070531 dEAthcURe|P2p
		return -1;
	}

	return (int)m_pPeerInfoMgr->GetMap().size();
}