// P2PServer.cpp: implementation of the CP2PServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "P2PServer.h"
#pragma warning(disable:4995)
#include "BugSlayerUtil.h"
#pragma warning(default:4995)
CP2PServer* CP2PServer::s_pP2PServer = NULL;


int CP2PInfo::TxPacket(PACKET_GENERIC *pPacket, SOCKADDR *pAddr)
{
	int nResult = sendto(CP2PServer::GetServerSocket(), (char *)pPacket, pPacket->Length(), 0, pAddr, sizeof(SOCKADDR));
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

int CP2PInfo::TxPacket(PACKET_GENERIC *pPacket)
{
	int nResult = sendto(CP2PServer::GetServerSocket(), (char *)pPacket, pPacket->Length(), 0, &m_addrPublic, sizeof(SOCKADDR));
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

int CP2PInfo::TxPacketToAllPeer(PACKET_GENERIC *pPacket, bool bIncludeMe)
{
	int	i;
	int	nResult = 0;
	if (bIncludeMe) {
		if (TxPacket(pPacket, &m_addrPublic) > 0 ) {
			nResult++;
		}
	}
	for ( i = 0 ; i < m_nNumberOfPeer; i++) {
		CP2PInfo*	pOtherInfo = CP2PServer::GetP2PInfo(m_nPeerKeys[i]);
		if (TxPacket(pPacket, &(pOtherInfo->m_addrPublic) ) > 0 ) {
			nResult++;
		}
	}
	return nResult;
}

int CP2PInfo::TxExpelPartyPacket()
{
	PACKET_GENERIC	packet;
	packet.Init(-1,MSGID_P2P_EXPELED);
	packet.Complete(sizeof(PACKET_GENERIC));
	return TxPacket(&packet);
}

int CP2PInfo::TxPeerAddrOkPacket(bool bAllParty)
{
	STRUCT_PEER_ADDR*			pStruct;
	TCHAR						pBuffer[1024];
	PACKET_P2P_PEER_ADDR_OK*	pOutPacket = (PACKET_P2P_PEER_ADDR_OK *)pBuffer;
	CP2PInfo*					pPeerInfo;
	pOutPacket->Init(-1, MSGID_P2P_PEER_ADDR_OK);
	pOutPacket->m_nNumberOfPeer = m_nNumberOfPeer + 1;
	/////////////////////////////////////////////////////
	//Peer들의 정보(key, public addr) setting
	for ( int i = 0 ; i < m_nNumberOfPeer; i++) {
		pStruct = pOutPacket->GetPeerAddr(i);
		pStruct->m_nKey = m_nPeerKeys[i];
		pPeerInfo = CP2PServer::GetP2PInfo(pStruct->m_nKey);
		if (pPeerInfo) {
			pStruct->SetAddrLocal(&(pPeerInfo->m_addrLocal) );
			pStruct->SetAddrPublic(&(pPeerInfo->m_addrPublic) );
		}
	}
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	//나의 정보(key, public addr) setting
	pStruct = pOutPacket->GetPeerAddr(i);
	pStruct->m_nKey = m_nKey;
	pStruct->SetAddrLocal(&m_addrLocal);
	pStruct->SetAddrPublic(&m_addrPublic);
	/////////////////////////////////////////////////////
	pOutPacket->Complete(pOutPacket->GetSize());
	if (bAllParty) {
		return TxPacketToAllPeer(pOutPacket);
	} else {
		return TxPacket(pOutPacket);
	}
}

int CP2PInfo::TxPeerAddrFailPacket(bool bAllParty)
{
	STRUCT_PEER_ADDR*			pStruct;
	TCHAR						pBuffer[1024];
	PACKET_P2P_PEER_ADDR_OK*	pOutPacket = (PACKET_P2P_PEER_ADDR_OK *)pBuffer;
	CP2PInfo*					pPeerInfo;
	pOutPacket->Init(-1, MSGID_P2P_PEER_ADDR_FAIL);
	pOutPacket->m_nNumberOfPeer = m_nNumberOfPeer + 1;
	/////////////////////////////////////////////////////
	//Peer들의 정보(key, public addr) setting
	for ( int i = 0 ; i < m_nNumberOfPeer; i++) {
		pStruct = pOutPacket->GetPeerAddr(i);
		pStruct->m_nKey = m_nPeerKeys[i];
		pPeerInfo = CP2PServer::GetP2PInfo(pStruct->m_nKey);
		if (pPeerInfo) {
			pStruct->SetAddrLocal(&(pPeerInfo->m_addrLocal) );
			pStruct->SetAddrPublic(&(pPeerInfo->m_addrPublic) );
		}
	}
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	//나의 정보(key, public addr) setting
	pStruct = pOutPacket->GetPeerAddr(i);
	pStruct->m_nKey = m_nKey;
	pStruct->SetAddrLocal(&m_addrLocal);
	pStruct->SetAddrPublic(&m_addrPublic);
	/////////////////////////////////////////////////////
	pOutPacket->Complete(pOutPacket->GetSize());
	if (bAllParty) {
		return TxPacketToAllPeer(pOutPacket);
	} else {
		return TxPacket(pOutPacket);
	}
}

void CP2PInfo::RemovePeer(int nKey)
{
	for (int i = 0; i < m_nNumberOfPeer; i++) {
		if (nKey ==m_nPeerKeys[i]) {
			m_nState = P2P_INFO_INIT;
			m_nNumberOfPeer--;
			break;
		}
	}
	for (; i < m_nNumberOfPeer; i++) {
		m_nPeerKeys[i] = m_nPeerKeys[i+1];
	}
}
void CP2PInfo::OnTimer(HANDLE nID)
{
	if (m_hAddrRequestExpireTimer == nID) {
		KillTimer(nID);
		m_hAddrRequestExpireTimer = 0;
		CP2PServer::s_pP2PServer->OnExpireReqAddrTimer(this);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CP2PServer::CP2PServer():m_timerManager(50)	// timer manager의 resolution을 50mSec로 초기화한다.
{
	s_pP2PServer = this;
	InitializeCriticalSection(&m_csP2p);
}

CP2PServer::~CP2PServer()
{
	DeleteCriticalSection(&m_csP2p);
}

bool CP2PServer::Init(int nServerPort, int nMAxUser)
{
	SOCKADDR_IN	addr;
	m_nMaxUser = nMAxUser;
	m_pPeerInfoPool = (CPool<CP2PInfo> *)(new CPool<CP2PInfo>(nMAxUser));
	m_ppP2PInfo = new LPCP2PInfo[nMAxUser];
	for (int i = 0; i < nMAxUser; i++) {
		m_ppP2PInfo[i] = NULL;
	}
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

	if (bind(m_sServerSock, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){
		StopOperation();
		return false;
	}
	m_timerManager.BeginThread();
	BeginThread();
	return true;
}
void CP2PServer::StopOperation()
{	
	if (INVALID_SOCKET != m_sServerSock) {
		closesocket(m_sServerSock);
		m_sServerSock = INVALID_SOCKET;
	}
	StopThread();
}

UINT CP2PServer::Run()
{
	char			pBuffer[DEFAULT_P2P_BUFFER_SIZE];
	int				result;
	SOCKADDR		addr;
	int				addr_len = sizeof(SOCKADDR);
	PACKET_GENERIC*	pPacket = (PACKET_GENERIC *)pBuffer;

	m_nExecuteCount++;
	memset((char *) &addr, 0, sizeof(SOCKADDR));
	while (m_bRun) {
		bool bClosed = false; 
		result = recvfrom(m_sServerSock, pBuffer, DEFAULT_P2P_BUFFER_SIZE, 0, &addr, &addr_len);
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

bool CP2PServer::ProcessPacket(PACKET_GENERIC *pPacket, SOCKADDR* pAddr)
{
	EnterCriticalSection(&m_csP2p);
	switch(pPacket->MsgID()) {
		case MSGID_P2P_REQ_PEER_ADDR :
			SystemPrint(ERROR_LOG, "Rx([key(%d)|MSGID_P2P_REQ_PEER_ADDR|])", pPacket->ConnectID() );
			RxReqPeerAddr((PACKET_P2P_REQ_PEER_ADDR *)pPacket, pAddr);
			break;
		case MSGID_P2P_EXIT:
			{
				PACKET_GENERIC	packet;
				SystemPrint(ERROR_LOG, "Rx([key(%d)|MSGID_P2P_EXIT|])", pPacket->ConnectID() );
				packet.Init(-1, MSGID_P2P_EXIT_OK);
				packet.Complete(sizeof(PACKET_GENERIC));
				CP2PInfo::TxPacket(&packet, pAddr);
				Exit(pPacket->ConnectID());
				break;
			}
		case MSGID_P2P_EXIT_ALL_PEER:
			{
				PACKET_GENERIC	packet;
				SystemPrint(ERROR_LOG, "Rx([key(%d)|MSGID_P2P_EXIT_ALL_PEER|])", pPacket->ConnectID() );
				packet.Init(-1, MSGID_P2P_EXIT_ALL_OK);
				packet.Complete(sizeof(PACKET_GENERIC));
				CP2PInfo::TxPacket(&packet, pAddr);
				ExitAllPeer(pPacket->ConnectID());
				break;
			}
		case MSGID_P2P_REMOVE_USER :
			{
				break;
			}
		case MSGID_P2P_REQ_PUNCHHOLE :
		case MSGID_P2P_REQ_RELAY :
			RelayP2pPacket((PACKET_P2P_GENERIC *)pPacket);
			break;
	}
	LeaveCriticalSection(&m_csP2p);
	return	true;
}

void CP2PServer::ExitAllPeer(int nKey)
{
	CP2PInfo*	pInfo = GetP2PInfo(nKey);
	if (NULL == pInfo) return;
	for ( int i = 0; i < pInfo->m_nNumberOfPeer; i++) {
		FreeP2PInfo(pInfo->m_nPeerKeys[i]);
	}
	FreeP2PInfo(nKey);
}

void CP2PServer::Exit(int nKey)
{
	int			i, j;
	int			nStillNotRequestPeers = 0;
	int			nRemainMiliSec = 0;
	CP2PInfo*	pInfo = GetP2PInfo(nKey);
	if (NULL == pInfo) return;
	///////////////////////////////////////////////////////////////
	// 일단 Party에서 nKEy를 제거 한다.
	for ( i = 0; i < pInfo->m_nNumberOfPeer; i++) {
		CP2PInfo*	pPeerInfo = GetP2PInfo(pInfo->m_nPeerKeys[i]);
		if (pPeerInfo) {
			for (j = 0; j < pPeerInfo->m_nNumberOfPeer; j++ ) {
				if (pPeerInfo->m_nPeerKeys[j] == nKey) {
					pPeerInfo->m_nNumberOfPeer--;
					break;
				}
			}
			for (; j < pPeerInfo->m_nNumberOfPeer; j++ ) {
				pPeerInfo->m_nPeerKeys[j] = pPeerInfo->m_nPeerKeys[j+1];
			}
		} else {
			nStillNotRequestPeers++;
		}
	}
	// 일단 Party에서 nKEy를 제거 한다.
	///////////////////////////////////////////////////////////////

	if (pInfo->m_hAddrRequestExpireTimer) {
		nRemainMiliSec = pInfo->KillReqAddrTimerOfParty();
	}

	if ( (P2P_RX_REQ_ADDR == pInfo->m_nState) || (P2P_RX_PARTY_EXIT == pInfo->m_nState) ) {
		CP2PInfo*		pLastPeer = NULL;	//꼭 마지막 peer일 필요는 없으나 임의로 하나 정하기에 마지막이 제일편해서 마지막 peer를 사용한다.
		for ( i = 0; i < pInfo->m_nNumberOfPeer; i++) {
			CP2PInfo*	pPeerInfo = GetP2PInfo(pInfo->m_nPeerKeys[i]);
			if (pPeerInfo) {
				pLastPeer = pPeerInfo;
				if (0 == nStillNotRequestPeers) {
					pPeerInfo->m_nState = P2P_SOME_PEER_DISCONNECTED;
				} else {
					pPeerInfo->m_nState = P2P_RX_PARTY_EXIT;
				}
			}
		}
		if (pLastPeer) {	// <=== 남아 있는 피어가 하나라도 있다면
			if (P2P_SOME_PEER_DISCONNECTED == pLastPeer->m_nState) {
				pLastPeer->TxPeerAddrFailPacket(true);
			}else if (P2P_RX_PARTY_EXIT == pLastPeer->m_nState) {
				if (nRemainMiliSec > 0) {
					pLastPeer->m_hAddrRequestExpireTimer = pLastPeer->SetTimer(nRemainMiliSec);
				}
			}
		}	// <=== 남아 있는 피어가 하나라도 있다면
	} else {
		// 이런 경우 일단 생까기로 한다. 통신하다 중간에 잘려나간놈이 있는 경우 
		// 그냥 서버자료구조에서 배기만 해도 충분하지 않을까?
		// 클라이언트한테 잘 빠졌네 못빠졌네 알려줄 필요는 없는것 같다.
		if (nRemainMiliSec > 0) {
			ASSERT(!"이런경우가 생긴다면 좀특이하다");
		}
	}
	FreeP2PInfo(nKey);
}

void CP2PServer::RxReqPeerAddr(PACKET_P2P_REQ_PEER_ADDR *pInPacket, SOCKADDR* pAddr)
{
	int			i, nNewState;
	CP2PInfo*	pInfo = GetP2PInfo(pInPacket->ConnectID());	// 재시도를 위해 날라오는 경우도 있으니 무조건 alloc하지 않고 일단 있는지 살펴본다. 2005.7.22
	if (NULL == pInfo) {	// <===처음으로 시도해서 새로이 PeerInfo를 할당하는 경우;
		pInfo = AllocP2PInfo(pInPacket->ConnectID(), pAddr);
		if (NULL == pInfo) {
			ASSERT(!"Peer Info allocation fail");
			return;
		}
		//////////////////////////////////////////////////
		// 할당된 peer info 초기화
		TCHAR	strDebugMsg[256];
		TCHAR	strTemp[64];
		StringCchCopy(strTemp, 64, inet_ntoa( ((SOCKADDR_IN*)&(pInfo->m_addrPublic))->sin_addr));
		pInfo->m_nNumberOfPeer = pInPacket->m_nNumberOfPeer;
		pInfo->SetAddrLocal(&(pInPacket->m_addrLocal));
		pInfo->m_bNeedToRelay = false;
		pInfo->m_hAddrRequestExpireTimer = NULL;
		pInfo->m_nState = P2P_INFO_INIT;
		StringCchPrintf(strDebugMsg, 256,"RxReqAddr from NewPeer(k=%d, publicIP=%s, localIP=%s, st=P2P_INFO_INIT, peer%d(", 
			pInPacket->ConnectID(), 
			strTemp, 
			inet_ntoa( ((SOCKADDR_IN*)&(pInfo->m_addrLocal))->sin_addr), 
			pInfo->m_nNumberOfPeer);
		for ( i = 0; i < pInPacket->m_nNumberOfPeer; i++) {
			pInfo->m_nPeerKeys[i] = pInPacket->GetPeerKey(i);
			StringCchPrintf(strTemp,64," %d",pInfo->m_nPeerKeys[i]);
			StringCchCat(strDebugMsg, 256, strTemp);
		}
		StringCchCat(strDebugMsg, 256, "))");
		SystemPrint(2, "%s", strDebugMsg);
		// 할당된 peer info 초기화
		//////////////////////////////////////////////////
		int	nOthersState = pInfo->GetPartyState();
		if (P2P_INFO_INIT == nOthersState) {	// 파티내에서 처음으로 ReqAddr한놈인 경우
			//4초(3번 재시도 할시간)안에 모든 클라이언트에게서 MSGID_P2P_REQ_PEER_ADDR를 받지 않았다면
			//몇몇 클라이언트는 파티 참여도중 실패한것이다. 이를 감지하기위해 setTimer한다.
			pInfo->m_hAddrRequestExpireTimer = pInfo->SetTimer(4000);	
			pInfo->m_nState = P2P_RX_REQ_ADDR;
		} else {
			pInfo->m_nState = nOthersState;
			if (P2P_SOME_PEER_DISCONNECTED == nOthersState) {
				pInfo->TxExpelPartyPacket();
				FreeP2PInfo(pInfo->m_nKey);
				return;
			} else if(P2P_RX_ALL_PEER_ADDR == nOthersState) {
				ASSERT(!"다들성공적으로  address를 받았는데 신규로 참여하는놈이 있다?");
				pInfo->TxExpelPartyPacket();
				return;
			}
		}
	}	// <====처음으로 시도해서 새로이 PeerInfo를 할당하는 경우;
	if (pInfo->UpdatePartyState(&nNewState)) {	//state가 천이된 경우
		pInfo->KillReqAddrTimerOfParty();
		if (P2P_SOME_PEER_DISCONNECTED == nNewState) {
			pInfo->TxPeerAddrFailPacket(true);
		} else if (P2P_RX_ALL_PEER_ADDR == nNewState) {
			pInfo->TxPeerAddrOkPacket(true);
		} else {
			ASSERT(!"이런경우는 있으면 안된다1");
		}
	} else {
		if (P2P_SOME_PEER_DISCONNECTED == nNewState) {
			pInfo->TxPeerAddrFailPacket(false);
		} else if (P2P_RX_ALL_PEER_ADDR == nNewState) {
			pInfo->TxPeerAddrOkPacket(false);
		}
	}
}

void CP2PServer::FreeP2PInfo(int nKey)
{
	if (NULL==m_ppP2PInfo[nKey]) return;
	m_pPeerInfoPool->Free(m_ppP2PInfo[nKey]);
	m_ppP2PInfo[nKey] = NULL;
}

CP2PInfo* CP2PServer::AllocP2PInfo(int nKey, SOCKADDR *pAddrPublic)
{
	CP2PInfo*	pInfo;
	if (nKey < 0) return NULL;
	if (nKey >= m_nMaxUser) return NULL;
	pInfo = m_pPeerInfoPool->Alloc();
	if (NULL == pInfo) return NULL;
	pInfo->m_nKey = nKey;
	pInfo->SetAddrPublic(pAddrPublic);
	m_ppP2PInfo[nKey] = pInfo;
	return pInfo;
}

CP2PInfo* CP2PServer::GetP2PInfo(int nKey)
{
	if (NULL==s_pP2PServer) return NULL;
	if (nKey < 0) return NULL;
	if (nKey >= s_pP2PServer->m_nMaxUser) return NULL;
	return s_pP2PServer->m_ppP2PInfo[nKey];
}

void CP2PServer::RelayP2pPacket(PACKET_P2P_GENERIC *pPacket)
{
	CP2PInfo*	pPeerInfo = GetP2PInfo(pPacket->m_nDestPeerKey);
	pPeerInfo->TxPacket(pPacket, &(pPeerInfo->m_addrPublic) );
	SystemPrint(ERROR_LOG, "RelayP2pPacket([key(%d)|Msg(%d)|len(%d)] to user%d", pPacket->ConnectID(), pPacket->MsgID(), pPacket->Length(),
		pPacket->m_nDestPeerKey);
}


void CP2PServer::OnExpireReqAddrTimer(CP2PInfo* pInfo)
{
	CP2PInfo*					pPeerInfo;
	int							i, nPeerKey;
	int							nDontRespondingPeers = 0;
	int							nDontRespondingPeerKeys[DEFAULT_MAX_PEER];

	SystemPrint(2, "ReqAddr timer Expire!!!");
	EnterCriticalSection(&m_csP2p);
	if ( (P2P_RX_REQ_ADDR == pInfo->m_nState) || (P2P_RX_PARTY_EXIT == pInfo->m_nState) ) {
		pInfo->m_nState = P2P_SOME_PEER_DISCONNECTED;
		for (i = 0 ; i < pInfo->m_nNumberOfPeer; i++) {
			nPeerKey = pInfo->m_nPeerKeys[i];
			pPeerInfo = GetP2PInfo(nPeerKey);
			if (NULL == pPeerInfo) {
				nDontRespondingPeerKeys[nDontRespondingPeers] = nPeerKey;
				nDontRespondingPeers++;
			}
		}
		for (i = 0; i < nDontRespondingPeers; i++) {
			pInfo->RemovePeer(nDontRespondingPeerKeys[i]);
		}
		for (i = 0; i < pInfo->m_nNumberOfPeer; i++) {
			pPeerInfo = GetP2PInfo(pInfo->m_nPeerKeys[i]);
			if (pPeerInfo) {
				pPeerInfo->m_nState = P2P_SOME_PEER_DISCONNECTED;
				for (int nDr = 0; nDr < nDontRespondingPeers; nDr++) {
					pPeerInfo->RemovePeer(nDontRespondingPeerKeys[nDr]);
				}
			} else {
				ASSERT(!"이미 NULL인 놈 다뺐는데 왠 또 NULL?");
			}
		}
		pInfo->TxPeerAddrFailPacket(true);
	} else {
		SystemPrint(ERROR_LOG,"이상태에서 timer가 expire가 되면 뭔가 bug가 있는거다.");
		ASSERT(!"이상태에서 timer가 expire가 되면 뭔가 bug가 있는거다.");
	}
	LeaveCriticalSection(&m_csP2p);
}

// client가 포함된 P2P party의 peer들의 상태를 가져온다.(정상적인 경우 모든 party원들의 상태가 같다)
int CP2PInfo::GetPartyState(void)
{
	int	nState = m_nState;
	if (P2P_INFO_INIT == nState) {
		for (int i = 0; i < m_nNumberOfPeer; i++) {
			CP2PInfo*	pPeer = CP2PServer::GetP2PInfo(m_nPeerKeys[i]);
			if (pPeer) {
				nState = pPeer->m_nState;
			}
		}
	}
	return nState;
}

bool CP2PInfo::UpdatePartyState(int* pState)
{
	int		nState = m_nState;
	bool	bAllSameState = true;
	bool	bAllPartyJoined = true;
	bool	bChangeState = false;
	for (int i = 0; i < m_nNumberOfPeer; i++) {
		CP2PInfo*	pPeer = CP2PServer::GetP2PInfo(m_nPeerKeys[i]);
		if (NULL == pPeer) {
			bAllPartyJoined = false;
		}else {
			if (nState != pPeer->m_nState) {
				ASSERT(!"Participants have different state");
				bAllSameState = false;
			}
		}
	}
	if (bAllPartyJoined && bAllSameState) {
		if (P2P_RX_REQ_ADDR == nState) {
			nState = P2P_RX_ALL_PEER_ADDR;
			bChangeState = true;
		} else if (P2P_RX_PARTY_EXIT == nState) {
			nState = P2P_SOME_PEER_DISCONNECTED;
			bChangeState = true;
		}
		if (bChangeState) {
			m_nState = nState;
			for ( i = 0; i < m_nNumberOfPeer; i++) {
				CP2PInfo*	pPeer = CP2PServer::GetP2PInfo(m_nPeerKeys[i]);
				pPeer->m_nState = nState;
			}
		}
	}
	*pState = nState;
	return bChangeState;
}


//  파티에 참여중인 peer들 중에 Requst Addr Expire timer가 동작중인놈이 있으면 끄고 남은 시간을 반환한다.
int CP2PInfo::KillReqAddrTimerOfParty(void)
{
	int		nRemainMiliSec = 0;
	if (m_hAddrRequestExpireTimer) {
		nRemainMiliSec = KillTimer(m_hAddrRequestExpireTimer);
		m_hAddrRequestExpireTimer = NULL;
		return nRemainMiliSec;
	}
	for (int i = 0; i < m_nNumberOfPeer; i++) {
		CP2PInfo*	pPeer = CP2PServer::GetP2PInfo(m_nPeerKeys[i]);
		if (NULL == pPeer) {
			continue;
		}
		if (pPeer->m_hAddrRequestExpireTimer) {
			nRemainMiliSec = pPeer->KillTimer(pPeer->m_hAddrRequestExpireTimer);
			pPeer->m_hAddrRequestExpireTimer = NULL;
			break;
		}
	}
	return nRemainMiliSec;
}
