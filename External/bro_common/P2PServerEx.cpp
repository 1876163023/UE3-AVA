#include "stdafx.h"
#include ".\p2pserverex.h"

CP2PServerEx* CP2PServerEx::s_pP2PServer = NULL;

int CP2PInfoEx::TxPacket(PACKET_GENERIC *pPacket, SOCKADDR *pAddr)
{
	int nResult = sendto(CP2PServerEx::GetServerSocket(), (char *)pPacket, pPacket->Length(), 0, pAddr, sizeof(SOCKADDR));
	if (SOCKET_ERROR == nResult || 0 == nResult ) {
		if(WSAEWOULDBLOCK != WSAGetLastError()) {
			nResult = -1;
			SystemPrint(2,"IoSocketSendComplete(SOCKET_ERROR & !WSAEWOULDBLOCK)");
		} else {
			SystemPrint(1,"SOCKET_ERROR & WSAEWOULDBLOCK");
		}
	}
	return nResult;
}
int CP2PInfoEx::TxPacket(PACKET_GENERIC *pPacket)
{
	int nResult = sendto(CP2PServerEx::GetServerSocket(), (char *)pPacket, pPacket->Length(), 0, &m_addrPublic, sizeof(SOCKADDR));
	if (SOCKET_ERROR == nResult || 0 == nResult ) {
		if(WSAEWOULDBLOCK != WSAGetLastError()) {
			nResult = -1;
			SystemPrint(2,"IoSocketSendComplete(SOCKET_ERROR & !WSAEWOULDBLOCK)");
		} else {
			SystemPrint(1,"SOCKET_ERROR & WSAEWOULDBLOCK");
		}
	}
	return nResult;
}
/////////////////////////////////////////////////////////
CP2PServerEx::CP2PServerEx(void)//:m_watch(100000.0)//:m_timerManager(50)	// timer manager의 resolution을 50mSec로 초기화한다.
{
	s_pP2PServer = this;
	m_sServerSock = INVALID_SOCKET;
	m_sServerProbeSock1 = INVALID_SOCKET;
	m_sServerProbeSock2 = INVALID_SOCKET;
	m_pPeerInfoPool = NULL;
	SystemPrint(ERROR_LOG, "P2PServerEx started");
}

CP2PServerEx::~CP2PServerEx(void)
{
	SAFE_DELETE(m_pPeerInfoPool);
}

CP2PInfoEx* CP2PServerEx::AllocP2PInfo(int nKey, SOCKADDR *pAddrPublic)
{
	CP2PInfoEx*	pInfo;
	if (m_pP2PInfoMap.size() >= (size_t)m_nMaxUser)
		return NULL;
	pInfo = m_pPeerInfoPool->Alloc();
	if (NULL == pInfo) return NULL;
	pInfo->Init();
	pInfo->m_nKey = nKey;
	pInfo->SetAddrPublic(pAddrPublic);
	if (!m_pP2PInfoMap.insert(P2PINFOMAP::value_type(nKey, pInfo)).second)
	{
		pInfo->Init();
		m_pPeerInfoPool->Free(pInfo);
		return NULL;
	}
	m_pP2PInfoMap[nKey] = pInfo;
	return pInfo;
}

CP2PInfoEx* CP2PServerEx::GetP2PInfo(int nKey)
{
	if (NULL==s_pP2PServer) return NULL;
	P2PINFOMAP::iterator it = s_pP2PServer->m_pP2PInfoMap.find(nKey);
	if (it == s_pP2PServer->m_pP2PInfoMap.end())
		return NULL;
	return it->second;
}
void CP2PServerEx::FreeP2PInfo(int nKey)
{
	if (nKey < 0) {
		SystemPrint(2, "FreeP2PInfo[%d] Key out of range", nKey);
		return;
	}
	P2PINFOMAP::iterator it = m_pP2PInfoMap.find(nKey);
	if (it == m_pP2PInfoMap.end()) return;
	SystemPrint(2, "PeerInfo[%d] is deleted", nKey);
	CP2PInfoEx* pInfo = it->second;
	m_pP2PInfoMap.erase(it);
	pInfo->Init();
	m_pPeerInfoPool->Free(pInfo);
}

void SetSocketReuse(SOCKET s)
{
	int	nReuse = TRUE;
	if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&nReuse, sizeof(int)) < 0)
	{
		SystemPrint(ERROR_LOG, "CServerInterface::PacketTransmitter, SO_REUSEADDR error");
	}
}

bool CP2PServerEx::Init(int nServerPort, int nMAxUser)
{
	SOCKADDR_IN	addr;
	m_nMaxUser = nMAxUser;
	m_pPeerInfoPool = new CPool<CP2PInfoEx>(nMAxUser);
	//////////////////////////////////////////////////////////////////////
	// main Port
	m_nPort = nServerPort;
	m_sServerSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_sServerSock == INVALID_SOCKET){
		StopOperation();
		return false;
	}
	memset(&addr, 0, sizeof(SOCKADDR));
	addr.sin_family				= AF_INET;
	addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	addr.sin_port				= htons((u_short)m_nPort);

	SetSocketReuse(m_sServerSock);
	if (bind(m_sServerSock, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){
		StopOperation();
		return false;
	}
	// main Port
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// probe Port1
	m_nProbePort1 = nServerPort+1;
	m_sServerProbeSock1 = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_sServerProbeSock1 == INVALID_SOCKET){
		StopOperation();
		return false;
	}
	memset(&addr, 0, sizeof(SOCKADDR));
	addr.sin_family				= AF_INET;
	addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	addr.sin_port				= htons((u_short)m_nProbePort1);

	SetSocketReuse(m_sServerProbeSock1);
	if (bind(m_sServerProbeSock1, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){
		StopOperation();
		return false;
	}
	// probe Port1
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// probe Port2
	m_nProbePort2 = nServerPort+2;
	m_sServerProbeSock2 = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_sServerProbeSock2 == INVALID_SOCKET){
		StopOperation();
		return false;
	}
	memset(&addr, 0, sizeof(SOCKADDR));
	addr.sin_family				= AF_INET;
	addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	addr.sin_port				= htons((u_short)m_nProbePort2);

	SetSocketReuse(m_sServerProbeSock2);
	if (bind(m_sServerProbeSock2, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){
		StopOperation();
		return false;
	}
	// probe Port2
	//////////////////////////////////////////////////////////////////////
//	m_timerManager.BeginThread();
#ifndef _BT_DEVELOP
	CConnectionChecker::Init(15,15);
#else
	CConnectionChecker::Init(1500,1500);
#endif
	BeginThread();
	BeginThread();
	BeginThread();
	BeginThread();
	return true;
}
UINT CP2PServerEx::Run(int* pRun, int nID)
{
	switch(nID) {
		case 0 : return RcvThread(pRun, m_sServerSock); break;
		case 1 : return RcvThread(pRun, m_sServerProbeSock1); break;
		case 2 : return RcvThread(pRun, m_sServerProbeSock2); break;
		case 3 : return ChkThread(pRun); break;
		default : SystemPrint(ERROR_LOG, "Id(%d) out of range", nID); break;
	}
	return 0;
}

void CP2PServerEx::OnConnectionTimeOut(CLRUEntry* pIdleEntry)
{
	CP2PInfoEx* pInfo = (CP2PInfoEx *)pIdleEntry;
	FreeP2PInfo(pInfo->m_nKey);
}
UINT CP2PServerEx::ChkThread(int* pRun)
{
	while (THREAD_RUN == *pRun) {
		Lock();
		CheckConnection();	//chain Lock 진입
		Unlock();
		Sleep(5000);
	}
	return 1;
}
UINT CP2PServerEx::RcvThread(int* pRun, SOCKET sServerSock)
{
	char			pBuffer[DEFAULT_P2P_BUFFER_SIZE];
	SOCKADDR		addr;
	int				result;
	int				addr_len = sizeof(SOCKADDR);
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)pBuffer;

	memset((char *) &addr, 0, sizeof(SOCKADDR));
	while (THREAD_RUN == *pRun) {
		bool bClosed = false; 
		result = recvfrom(sServerSock, pBuffer, DEFAULT_P2P_BUFFER_SIZE, 0, &addr, &addr_len);
		if (THREAD_RUN != *pRun) {
			return 1;
		}
		if (SOCKET_ERROR == result) {
			switch(WSAGetLastError())
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
			if (!bClosed) {
				continue;
			}
		} else if ( 0 == result ) {
			bClosed = true;
		}
		if (bClosed) {
			//Disconnect(&addr);
			continue;
		}
		ProcessPacket(pPacket, &addr);
	}
	m_nExecuteCount--;
	return 1;
}

bool CP2PServerEx::ProcessPacket(PACKET_P2P_GENERIC* pPacket, SOCKADDR* pAddr)
{
	Lock();
	CP2PInfoEx*	pInfo = GetP2PInfo(pPacket->ConnectID());
	if (P2P_FLAG_RELAY&pPacket->m_bFlag) {
		CP2PInfoEx*	pDestInfo = GetP2PInfo(pPacket->m_nDestPeerKey);
		if (NULL == pDestInfo) {
			Unlock();
			return false;
		}
		pDestInfo->TxPacket(pPacket);//////////////
		Unlock();
		return true;
	}
	if (P2P_FLAG_HAND_SHAKE&pPacket->m_bFlag) {	//Reliable Packet이 도착했으면
		if (P2P_FLAG_USER_DEFINED_ACK&pPacket->m_bFlag) {
			PacketHandler(pPacket, pAddr);
			if (NULL == pInfo) {
				Unlock();
				return true;
			}
			if (pInfo->m_nRxSeq == pPacket->m_nSeq) {	//이미 처리한 패킷이 재전송되어 온경우 무시한다.
				SystemPrint(ERROR_LOG, "pInfo->m_nRxSeq == pPacket->m_nSeq. ignoring seq=%d", pInfo->m_nRxSeq);
				Unlock();
				return true;
			}

			pInfo->m_nRxSeq = pPacket->m_nSeq;
			SystemPrint(ERROR_LOG, "pInfo->m_nRxSeq=%d", pInfo->m_nRxSeq);
			Unlock();
			return true;
		} else {
			PACKET_P2P_GENERIC	packet;
			packet.InitData();
			packet.Init(P2P_DEST_NONE, MSGID_P2P_ACK_SEQ);
			packet.m_bFlag = P2P_FLAG_PIGGY_BACK;
			packet.m_nAckSeq = pPacket->m_nSeq;
			packet.m_nDestPeerKey = pPacket->ConnectID();
			packet.m_nTick = m_watch.GetTick();//GetTickCount();
			packet.Complete(sizeof(PACKET_P2P_GENERIC));
			CP2PInfoEx::TxPacket(&packet, pAddr);
			if (NULL == pInfo) {
				Unlock();
				return true;
			}
			if (pInfo->m_nRxSeq == pPacket->m_nSeq) {	//이미 처리한 패킷이 재전송되어 온경우 무시한다.
				SystemPrint(ERROR_LOG, "pInfo->m_nRxSeq == pPacket->m_nSeq. ignoring seq=%d", pInfo->m_nRxSeq);
				Unlock();
				return true;
			}
			pInfo->m_nRxSeq = pPacket->m_nSeq;
			SystemPrint(ERROR_LOG, "Setting pInfo->m_nRxSeq=%d", pInfo->m_nRxSeq);
		}
	}
	PacketHandler(pPacket, pAddr);
	Unlock();
	return true;
}
int CP2PServerEx::PacketHandler(PACKET_P2P_GENERIC* pPacket, SOCKADDR* pAddr)
{
	switch(pPacket->MsgID()) {
		case MSGID_P2P_REQ_CONNECT :
			{
				RxConnectRq((PACKET_P2P_REQ_CONNECT *)pPacket, pAddr);
				break;
			}
		case MSGID_P2P_TIME_ADJUST :
			{
				RxTimeAdjust((PACKET_P2P_TIME *)pPacket);
				break;
			}
		case MSGID_P2P_HEARTBEAT :
			{
				RxHeartBeat(pPacket);	//chain Lock 진입
				break;
			}
		case MSGID_P2P_REQ_PEER_ADDR :
			{
				SystemPrint(ERROR_LOG, "Rx([key(%d)|MSGID_P2P_REQ_PEER_ADDR|])", pPacket->ConnectID() );
				RxReqPeerAddr((PACKET_P2P_REQ_PEER_INFO *)pPacket);
				break;
			}
		case MSGID_P2P_EXIT:
			{
				CP2PInfoEx*	pInfo = GetP2PInfo(pPacket->ConnectID());	// 재시도를 위해 날라오는 경우도 있으니 무조건 alloc하지 않고 일단 있는지 살펴본다. 2005.7.22
				if (pInfo) {
					Delete(pInfo);	//chain Lock 진입
					FreeP2PInfo(pPacket->ConnectID());
				}
				break;
			}
			/*
		case MSGID_P2P_EXIT:
			{
				PACKET_GENERIC	packet;
				SystemPrint(ERROR_LOG, "Rx([key(%d)|MSGID_P2P_EXIT|])", pPacket->ConnectID() );
				packet.Init(-1, MSGID_P2P_EXIT_OK);
				packet.Complete(sizeof(PACKET_GENERIC));
				CP2PInfoEx::TxPacket(&packet, pAddr);
				Exit(pPacket->ConnectID());
				break;
			}
		case MSGID_P2P_EXIT_ALL_PEER:
			{
				PACKET_GENERIC	packet;
				SystemPrint(ERROR_LOG, "Rx([key(%d)|MSGID_P2P_EXIT_ALL_PEER|])", pPacket->ConnectID() );
				packet.Init(-1, MSGID_P2P_EXIT_ALL_OK);
				packet.Complete(sizeof(PACKET_GENERIC));
				CP2PInfoEx::TxPacket(&packet, pAddr);
				ExitAllPeer(pPacket->ConnectID());
				break;
			}
		case MSGID_P2P_REMOVE_USER :
			{
				break;
			}
			//case MSGID_P2P_REQ_PUNCHHALL :
			*/
		case MSGID_P2P_PUNCHHOLE_PROBE1 :
			{
				RxProbe1((PACKET_P2P_HOLE_PROBE *)pPacket, pAddr);
				break;
			}
		case MSGID_P2P_PUNCHHOLE_PROBE2 :
			{
				RxProbe2((PACKET_P2P_HOLE_PROBE *)pPacket, pAddr);
				break;
			}
		default : break;
	}
	return	true;
}
int CP2PServerEx::RxConnectRq(PACKET_P2P_REQ_CONNECT* pPacket, SOCKADDR* pAddr)
{
	CP2PInfoEx*	pInfo = GetP2PInfo(pPacket->ConnectID());	// 재시도를 위해 날라오는 경우도 있으니 무조건 alloc하지 않고 일단 있는지 살펴본다. 2005.7.22
	if (NULL == pInfo) {
#ifdef _BF_SERVER
		nxlogmsg(_S("RxConnectRq. NULL == pInfo. Allocating..."));
#endif
		pInfo = AllocP2PInfo(pPacket->ConnectID(), pAddr);
		if (NULL == pInfo) {
			assert(!"Peer Info allocation fail");
			return 0;
		}
		Add(pInfo);
	} else {
		pInfo->Init();
		pInfo->SetAddrPublic(pAddr);
	}
	//////////////////////////////////////////////////
	//  peer info 초기화
	char strDebugMsg[256];
	char strTemp[64];
	StringCchCopyA(strTemp, 64, inet_ntoa( ((SOCKADDR_IN*)&(pInfo->m_addrPublic))->sin_addr));
	
	pInfo->SetAddrLocal(&(pPacket->m_addrLocal));
	pInfo->m_nState = P2P_CONNECT;
	StringCchPrintfA(strDebugMsg, 256,"RxConnectRq- from NewPeer(tOrg=%d, k=%d, publicIP=%s, localIP=%s, port=%d, st=P2P_INFO_INIT)", pInfo->m_tOrg,
		pPacket->ConnectID(), 
		strTemp, 
		inet_ntoa( ((SOCKADDR_IN*)&(pInfo->m_addrLocal))->sin_addr),
		((SOCKADDR_IN*)&(pInfo->m_addrPublic))->sin_port);
	SystemPrint(ERROR_LOG, "%s", strDebugMsg);
	//////////////////////////////////////////////////
	PACKET_P2P_ACK_CONNECT       packetOut;
	packetOut.Init(P2P_DEST_NONE, MSGID_P2P_ACK_CONNECT);
	packetOut.InitData();
	memcpy(&(packetOut.m_addrPublic), pAddr, sizeof(SOCKADDR));
	packetOut.m_bFlag = P2P_FLAG_PIGGY_BACK;
	packetOut.m_nAckSeq = pPacket->m_nSeq;
	packetOut.m_nDestPeerKey = pPacket->ConnectID();
	packetOut.m_nTick = m_watch.GetTick();//GetTickCount();
	packetOut.Complete(sizeof(PACKET_P2P_ACK_CONNECT));
	CP2PInfoEx::TxPacket(&packetOut, pAddr);
	SystemPrint(ERROR_LOG, "Public IP (%s) connect", inet_ntoa( ((SOCKADDR_IN*)&(packetOut.m_addrPublic))->sin_addr));
	return 1;
}
/////////////////////////////////////////////////////////
// Server에서 굳이 LRU chain으로 살아있는놈 관리할 필요가 없다.
// reply보내서 뚫려 잇는 홀 관리나 하자.
// 이 메소드에서는 그저 이상한놈 안오나 디버깅 목적으로 찍어나 보자.
int CP2PServerEx::RxHeartBeat(PACKET_P2P_GENERIC* pPacket)
{
	CP2PInfoEx*	pInfo = GetP2PInfo(pPacket->ConnectID());	// 재시도를 위해 날라오는 경우도 있으니 무조건 alloc하지 않고 일단 있는지 살펴본다. 2005.7.22
	if (NULL == pInfo) {
		SystemPrint(2,"없는놈(%d)한테서 Heartbeat이 왔다",pPacket->ConnectID());
		return 0;
	}
	Touch(pInfo);
	SystemPrint(2,"Rx(MSGID_P2P_HEARTBEAT from %d)",pPacket->ConnectID());
	return 1;
}
/////////////////////////////////////////////////////////
// Time Sync를 위해서
int CP2PServerEx::RxTimeAdjust(PACKET_P2P_TIME* pPacket)
{
	PACKET_P2P_TIME		outPacket;
	CP2PInfoEx*	pInfo = GetP2PInfo(pPacket->ConnectID());	// 재시도를 위해 날라오는 경우도 있으니 무조건 alloc하지 않고 일단 있는지 살펴본다. 2005.7.22
	if (NULL == pInfo) {
		SystemPrint(2,"없는놈(%d)한테서 MSGID_P2P_TIME_ADJUST(%d,%d,%d,%d)이 왔다",pPacket->ConnectID(), pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);
		return 0;
	}
	if (pPacket->m_nT1 != pInfo->m_tXmt) {
		if (pPacket->m_nT3 != pInfo->m_tOrg)
		{
			SystemPrint(2,"src key=%d(pInfo: org:%d, rec:%d, xmt:%d, ip = %s)한테서 온 MSGID_P2P_TIME_ADJUST(pPacket: %d,%d,%d,%d)이 위조된 것 같다",
				pPacket->ConnectID(), pInfo->m_tOrg, pInfo->m_tRec, pInfo->m_tXmt,inet_ntoa( ((SOCKADDR_IN*)&(pInfo->m_addrLocal))->sin_addr),
				pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);
			return 1;
		}
	}

	SystemPrint(ERROR_LOG, "src key=%d(pInfo: org:%d, rec:%d, xmt:%d)한테서 온 MSGID_P2P_TIME_ADJUST(pPacket: %d,%d,%d,%d)",
		pPacket->ConnectID(), pInfo->m_tOrg, pInfo->m_tRec, pInfo->m_tXmt, 
		pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);

	if (pPacket->m_nT3 != pInfo->m_tOrg) {	//중복 Packet이 아니라면
		pPacket->m_nT4 = m_watch.GetTick();//GetTickCount();
		pInfo->m_tOrg = pPacket->m_nT3;
		pInfo->m_tRec = pPacket->m_nT4;
		SystemPrint(ERROR_LOG, "after pInfo->m_tOrg=%d, pInfo->m_tRec=%d", pInfo->m_tOrg, pInfo->m_tRec);
	} else {
		SystemPrint(2,"src key=%d(pInfo: org:%d, rec:%d, xmt:%d)한테서 온 MSGID_P2P_TIME_ADJUST(pPacket: %d,%d,%d,%d)이 중복 인 것 같다",pPacket->ConnectID(), 
			pInfo->m_tOrg, pInfo->m_tRec, pInfo->m_tXmt,
			pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);
	}
	//SystemPrint(2,"src key=%d(pInfo: org:%d, rec:%d, xmt:%d)한테서 MSGID_P2P_TIME_ADJUST(pPacket: %d,%d,%d,%d)이 정상",pPacket->ConnectID(), 
	//	pInfo->m_tOrg, pInfo->m_tRec, pInfo->m_tXmt,
	//	pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4);
	Touch(pInfo);
	//SystemPrint(2,"Rx(MSGID_P2P_TIME_ADJUST from %d(%d,%d,%d,%d) : (org:%d, rec:%d, xmt:%d)",pPacket->ConnectID(), 
	//	pPacket->m_nT1, pPacket->m_nT2, pPacket->m_nT3, pPacket->m_nT4, pInfo->m_tOrg, pInfo->m_tRec, pInfo->m_tXmt);
	outPacket.InitData();
	outPacket.Init(P2P_DEST_NONE,MSGID_P2P_TIME_ADJUST);
	outPacket.m_bFlag = P2P_FLAG_PIGGY_BACK;
	outPacket.m_nAckSeq = pPacket->m_nSeq;
	outPacket.m_nDestPeerKey = pPacket->ConnectID();
	outPacket.m_nT1 = pInfo->m_tOrg;
	outPacket.m_nT2 = pInfo->m_tRec;
	outPacket.m_nT3 = m_watch.GetTick();//GetTickCount();
	pInfo->m_tXmt = outPacket.m_nT3;
//	SystemPrint(2,"%d(new pInfo->m_tXmt=%d)", pPacket->ConnectID(), pInfo->m_tXmt);
	outPacket.m_nT4 = 0;
	outPacket.m_nTick = outPacket.m_nT3;
	outPacket.Complete(sizeof(PACKET_P2P_TIME));
	//SystemPrint(2,"Tx(MSGID_P2P_TIME_ADJUST to %d(%d,%d,%d,%d)",pPacket->ConnectID(), 
	//	outPacket.m_nT1, outPacket.m_nT2, outPacket.m_nT3, outPacket.m_nT4);

	return pInfo->TxPacket(&outPacket);
}
/////////////////////////////////////////////////////////
int CP2PServerEx::RxReqPeerAddr(PACKET_P2P_REQ_PEER_INFO* pPacket)
{
	STRUCT_PEER_ADDR*			pStruct;
	TCHAR						pBuffer[1024];
	PACKET_P2P_PEER_INFO_OK*	pOutPacket = (PACKET_P2P_PEER_INFO_OK *)pBuffer;
	CP2PInfoEx*					pInfo = NULL;
	CP2PInfoEx*					pRequsterInfo = GetP2PInfo(pPacket->ConnectID());	// 재시도를 위해 날라오는 경우도 있으니 무조건 alloc하지 않고 일단 있는지 살펴본다. 2005.7.22
	if (NULL == pRequsterInfo) {
		SystemPrint(2,"없는놈(%d)한테서 MSGID_P2P_REQ_PEER_ADDR이 왔다",pPacket->ConnectID());
		return 0;
	}
	SystemPrint(2, "RxReqPeerAddr from %d. m_nNumberOfPeer=%d", pPacket->ConnectID(), pPacket->m_nNumberOfPeer);
	pOutPacket->Init(P2P_DEST_NONE, MSGID_P2P_PEER_ADDR_OK);
	pOutPacket->InitData();
	pOutPacket->m_bFlag = P2P_FLAG_PIGGY_BACK;
	pOutPacket->m_nAckSeq = pPacket->m_nSeq;
	pOutPacket->m_nDestPeerKey = pPacket->ConnectID();
	pOutPacket->m_nTick = m_watch.GetTick();
	pOutPacket->m_nNumberOfPeer = pPacket->m_nNumberOfPeer;
	/////////////////////////////////////////////////////
	//Peer들의 정보(key, public addr) setting
	for ( int i = 0 ; i < pPacket->m_nNumberOfPeer; i++) {
		pStruct = pOutPacket->GetPeerAddr(i);
		pStruct->Init();
		pStruct->m_nKey = pPacket->GetPeerKey(i);
		pInfo = CP2PServerEx::GetP2PInfo(pStruct->m_nKey);
		if (pInfo) {
			if (P2P_CONNECT == pInfo->m_nState) {
				pStruct->SetAddrLocal(&(pInfo->m_addrLocal) );
				pStruct->SetAddrPublic(&(pInfo->m_addrPublic) );
				SystemPrint(2, "RxReqPeerAddr i=%d, pStruct->m_nKey=%d, addrLocal=%s, addrPublic=%s", 
				i, pStruct->m_nKey, inet_ntoa( ((SOCKADDR_IN *)&(pInfo->m_addrLocal) )->sin_addr),
				 inet_ntoa( ((SOCKADDR_IN *)&(pInfo->m_addrPublic) )->sin_addr) );
			}
			else
			{
				SystemPrint(2, "m_nState is not P2P_CONNECT. key=%d", pStruct->m_nKey);
			}
		}
		else
		{
			SystemPrint(2, "pInfo NOT FOUNC. key=%d", pStruct->m_nKey);
		}
	}

	//TODO [2006-4-25 jovial] :내정보가 p2pclientex에서 필요한 경우, 새 필드를 pOutPacket에 추가
	pOutPacket->Complete(pOutPacket->GetSize());
	return pRequsterInfo->TxPacket(pOutPacket);
}

void CP2PServerEx::RxProbe1(PACKET_P2P_HOLE_PROBE* pPacket, SOCKADDR* pAddr)
{
	CP2PInfoEx*	pPeerInfo = GetP2PInfo(pPacket->m_nDestPeerKey);
	if (NULL == pPeerInfo) {
		return;
	}
	SystemPrint(2, "RxProbe1 from %d(%s:%d)==>peer(%d)", pPacket->ConnectID(),
		inet_ntoa( ((SOCKADDR_IN*)&(pPacket->m_RequsteePublicAddr))->sin_addr ), 
		ntohs( ((SOCKADDR_IN*)&(pPacket->m_RequsteePublicAddr))->sin_port ),pPacket->m_nDestPeerKey);
	pPeerInfo->m_Saddr1 = ((SOCKADDR_IN*)pAddr)->sin_addr;
	pPeerInfo->m_nProbePort1 = ntohs( ((SOCKADDR_IN*)pAddr)->sin_port );
	//memcpy(&(pPeerInfo->m_addrProbe1), pAddr, sizeof(SOCKADDR));
}
void CP2PServerEx::RxProbe2(PACKET_P2P_HOLE_PROBE* pPacket, SOCKADDR* pAddr)
{
	CP2PInfoEx*	pPeerInfo = GetP2PInfo(pPacket->m_nDestPeerKey);
	if (NULL == pPeerInfo) {
		return;
	}
	SystemPrint(2, "RxProbe2 from %d(%s:%d)==>peer(%d)", pPacket->ConnectID(),
		inet_ntoa( ((SOCKADDR_IN*)&(pPacket->m_RequsteePublicAddr))->sin_addr ), 
		ntohs( ((SOCKADDR_IN*)&(pPacket->m_RequsteePublicAddr))->sin_port ),pPacket->m_nDestPeerKey);
	
	if (0 == pPeerInfo->m_nProbePort1) {
		pPeerInfo->m_nProbePort1 = 0;
		SystemPrint(2, "Probe1 packet을 못받고 probe2만 받은것 같다");
		return;
	}
	
	pPeerInfo->m_Saddr2 = ((SOCKADDR_IN*)pAddr)->sin_addr;
	pPeerInfo->m_nProbePort2 = ntohs( ((SOCKADDR_IN*)pAddr)->sin_port);
	if (pPeerInfo->m_Saddr1.S_un.S_addr == pPeerInfo->m_Saddr2.S_un.S_addr) {
		PACKET_P2P_HOLE_PUNCH	outPacket;
		outPacket.InitData();
		outPacket.Init(pPacket->ConnectID(), MSGID_P2P_PUNCH);
		outPacket.m_nDestPeerKey = pPacket->m_nDestPeerKey;
		memcpy(&(outPacket.m_RequsteePublicAddr), &(pPacket->m_RequsteePublicAddr), sizeof(SOCKADDR));
		outPacket.m_nBeginPort = pPeerInfo->m_nProbePort1;
		outPacket.m_nEndPort = pPeerInfo->m_nProbePort2;
		outPacket.Complete(sizeof(PACKET_P2P_HOLE_PUNCH));
		SystemPrint(2, "Peer(%d)==>Tx(MSGID_P2P_PUNCH(%s:%d[%d:%d])==>peer(%d)", pPacket->ConnectID(),
			inet_ntoa( ((SOCKADDR_IN*)&(outPacket.m_RequsteePublicAddr))->sin_addr ), ntohs( ((SOCKADDR_IN*)&(outPacket.m_RequsteePublicAddr))->sin_port ),
			outPacket.m_nBeginPort, outPacket.m_nEndPort, outPacket.m_nDestPeerKey);
		pPeerInfo->TxPacket(&outPacket);
	}
	//memcpy(&(pPeerInfo->m_addrProbe2), pAddr, sizeof(SOCKADDR));
}
void CP2PServerEx::StopOperation()
{	
	if (INVALID_SOCKET != m_sServerSock) {
		closesocket(m_sServerSock);
		m_sServerSock = INVALID_SOCKET;
	}
	if (INVALID_SOCKET != m_sServerProbeSock1) {
		closesocket(m_sServerProbeSock1);
		m_sServerProbeSock1 = INVALID_SOCKET;
	}
	if (INVALID_SOCKET != m_sServerProbeSock2) {
		closesocket(m_sServerProbeSock2);
		m_sServerProbeSock2 = INVALID_SOCKET;
	}
	StopThread();
}
