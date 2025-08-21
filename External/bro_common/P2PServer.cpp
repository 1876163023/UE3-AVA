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
	//Peer���� ����(key, public addr) setting
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
	//���� ����(key, public addr) setting
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
	//Peer���� ����(key, public addr) setting
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
	//���� ����(key, public addr) setting
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

CP2PServer::CP2PServer():m_timerManager(50)	// timer manager�� resolution�� 50mSec�� �ʱ�ȭ�Ѵ�.
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
	// �ϴ� Party���� nKEy�� ���� �Ѵ�.
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
	// �ϴ� Party���� nKEy�� ���� �Ѵ�.
	///////////////////////////////////////////////////////////////

	if (pInfo->m_hAddrRequestExpireTimer) {
		nRemainMiliSec = pInfo->KillReqAddrTimerOfParty();
	}

	if ( (P2P_RX_REQ_ADDR == pInfo->m_nState) || (P2P_RX_PARTY_EXIT == pInfo->m_nState) ) {
		CP2PInfo*		pLastPeer = NULL;	//�� ������ peer�� �ʿ�� ������ ���Ƿ� �ϳ� ���ϱ⿡ �������� �������ؼ� ������ peer�� ����Ѵ�.
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
		if (pLastPeer) {	// <=== ���� �ִ� �Ǿ �ϳ��� �ִٸ�
			if (P2P_SOME_PEER_DISCONNECTED == pLastPeer->m_nState) {
				pLastPeer->TxPeerAddrFailPacket(true);
			}else if (P2P_RX_PARTY_EXIT == pLastPeer->m_nState) {
				if (nRemainMiliSec > 0) {
					pLastPeer->m_hAddrRequestExpireTimer = pLastPeer->SetTimer(nRemainMiliSec);
				}
			}
		}	// <=== ���� �ִ� �Ǿ �ϳ��� �ִٸ�
	} else {
		// �̷� ��� �ϴ� ������ �Ѵ�. ����ϴ� �߰��� �߷��������� �ִ� ��� 
		// �׳� �����ڷᱸ������ ��⸸ �ص� ������� ������?
		// Ŭ���̾�Ʈ���� �� ������ �������� �˷��� �ʿ�� ���°� ����.
		if (nRemainMiliSec > 0) {
			ASSERT(!"�̷���찡 ����ٸ� ��Ư���ϴ�");
		}
	}
	FreeP2PInfo(nKey);
}

void CP2PServer::RxReqPeerAddr(PACKET_P2P_REQ_PEER_ADDR *pInPacket, SOCKADDR* pAddr)
{
	int			i, nNewState;
	CP2PInfo*	pInfo = GetP2PInfo(pInPacket->ConnectID());	// ��õ��� ���� ������� ��쵵 ������ ������ alloc���� �ʰ� �ϴ� �ִ��� ���캻��. 2005.7.22
	if (NULL == pInfo) {	// <===ó������ �õ��ؼ� ������ PeerInfo�� �Ҵ��ϴ� ���;
		pInfo = AllocP2PInfo(pInPacket->ConnectID(), pAddr);
		if (NULL == pInfo) {
			ASSERT(!"Peer Info allocation fail");
			return;
		}
		//////////////////////////////////////////////////
		// �Ҵ�� peer info �ʱ�ȭ
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
		// �Ҵ�� peer info �ʱ�ȭ
		//////////////////////////////////////////////////
		int	nOthersState = pInfo->GetPartyState();
		if (P2P_INFO_INIT == nOthersState) {	// ��Ƽ������ ó������ ReqAddr�ѳ��� ���
			//4��(3�� ��õ� �ҽð�)�ȿ� ��� Ŭ���̾�Ʈ���Լ� MSGID_P2P_REQ_PEER_ADDR�� ���� �ʾҴٸ�
			//��� Ŭ���̾�Ʈ�� ��Ƽ �������� �����Ѱ��̴�. �̸� �����ϱ����� setTimer�Ѵ�.
			pInfo->m_hAddrRequestExpireTimer = pInfo->SetTimer(4000);	
			pInfo->m_nState = P2P_RX_REQ_ADDR;
		} else {
			pInfo->m_nState = nOthersState;
			if (P2P_SOME_PEER_DISCONNECTED == nOthersState) {
				pInfo->TxExpelPartyPacket();
				FreeP2PInfo(pInfo->m_nKey);
				return;
			} else if(P2P_RX_ALL_PEER_ADDR == nOthersState) {
				ASSERT(!"�ٵ鼺��������  address�� �޾Ҵµ� �űԷ� �����ϴ³��� �ִ�?");
				pInfo->TxExpelPartyPacket();
				return;
			}
		}
	}	// <====ó������ �õ��ؼ� ������ PeerInfo�� �Ҵ��ϴ� ���;
	if (pInfo->UpdatePartyState(&nNewState)) {	//state�� õ�̵� ���
		pInfo->KillReqAddrTimerOfParty();
		if (P2P_SOME_PEER_DISCONNECTED == nNewState) {
			pInfo->TxPeerAddrFailPacket(true);
		} else if (P2P_RX_ALL_PEER_ADDR == nNewState) {
			pInfo->TxPeerAddrOkPacket(true);
		} else {
			ASSERT(!"�̷����� ������ �ȵȴ�1");
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
				ASSERT(!"�̹� NULL�� �� �ٻ��µ� �� �� NULL?");
			}
		}
		pInfo->TxPeerAddrFailPacket(true);
	} else {
		SystemPrint(ERROR_LOG,"�̻��¿��� timer�� expire�� �Ǹ� ���� bug�� �ִ°Ŵ�.");
		ASSERT(!"�̻��¿��� timer�� expire�� �Ǹ� ���� bug�� �ִ°Ŵ�.");
	}
	LeaveCriticalSection(&m_csP2p);
}

// client�� ���Ե� P2P party�� peer���� ���¸� �����´�.(�������� ��� ��� party������ ���°� ����)
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


//  ��Ƽ�� �������� peer�� �߿� Requst Addr Expire timer�� �������γ��� ������ ���� ���� �ð��� ��ȯ�Ѵ�.
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
