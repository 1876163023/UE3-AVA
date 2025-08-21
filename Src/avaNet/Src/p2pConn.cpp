//
// p2pConn.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "avaNet.h"
#include "p2pConn.h"
#include "bro_common\P2PHandler.h"
#include <string.h>
#define LOG(a) if(_cb_log) _cb_log(a)

#ifdef LogToFile
#undef LogToFile
void LogToFile(char *str, ...) {}
#endif

//---------------------------------------------------------------------------
#ifdef EnableP2pConn
char _gini_ipAddr[256] = "";
int _gini_port = 0;
pp2pConn_t g_p2pConn = 0x0;
//---------------------------------------------------------------------------
void p2pConn_t::setP2pServerOnStartup(char* ipAddr, int port)
{
	if(ipAddr) {
		strncpy(_gini_ipAddr, ipAddr, 255);	
		_gini_port = port;		
	}
}
//---------------------------------------------------------------------------
void p2pConn_t::setP2pServer(char* ipAddr, int port, bool bFromIni)
{	
	if(ipAddr) {
		strncpy(p2psIpAddr, ipAddr, 255);	
		p2psPort = port;

		if(bFromIni) {
			strncpy(_gini_ipAddr, ipAddr, 255);	
			_gini_port = port;
		}	
	}
}
//---------------------------------------------------------------------------
bool p2pConn_t::waitForConnect(DWORD milisec)
{	
	fd_set readFds, writeFds, exceptFds;
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	struct timeval timevalue;

	timevalue.tv_sec = 1;
	timevalue.tv_usec = 0;
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)p2pBuffer;
	
	DWORD timeStart = timeGetTime();
	DWORD timeEnd = timeStart;

	bWaitForConnect = true;
	while(bWaitForConnect && !bHeartbeat && timeEnd - timeStart < milisec) {
		FD_SET(p2psSd, &readFds);
		::select(0, &readFds, &writeFds, &exceptFds, &timevalue);
		if (FD_ISSET(p2psSd, &readFds)) {
			SOCKADDR addr;
			int addr_len = sizeof(SOCKADDR);
			int nByteReceived = recvfrom(p2psSd, p2pBuffer, 65536, 0, &addr, &addr_len);
			if(nByteReceived>0) {
				ProcessP2PPacket(pPacket, &addr);
				if(bHeartbeat) {		
					bConnected = true;
					return true;
				}			
			}		
		}
		timeEnd = timeGetTime();
	}

	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::getPeerAddress(int idPeer, DWORD milisec, sockaddr* pAddr)
{
	fd_set readFds, writeFds, exceptFds;
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	struct timeval timevalue;

	timevalue.tv_sec = 1;
	timevalue.tv_usec = 0;
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)p2pBuffer;
	
	DWORD timeStart = timeGetTime();
	DWORD timeEnd = timeStart;

	bWaitForSvrAddr = true;
	while(bWaitForSvrAddr && !bGetPeerAddr && timeEnd - timeStart < milisec) {		
		FD_SET(p2psSd, &readFds);
		::select(0, &readFds, &writeFds, &exceptFds, &timevalue);
		if (FD_ISSET(p2psSd, &readFds)) {
			SOCKADDR addr;
			int addr_len = sizeof(SOCKADDR);
			int nByteReceived = recvfrom(p2psSd, p2pBuffer, 65536, 0, &addr, &addr_len);
			if(nByteReceived>0) {
				ProcessP2PPacket(pPacket, &addr);
				if(bGetPeerAddr) {
					STRUCT_PEER_INFO* pi = GetPeerInfo(idPeer);
					if(pi->m_bInSameNAT) {
						memcpy(pAddr, &pi->m_addrLocal, sizeof(sockaddr));
					}
					else {
						memcpy(pAddr, &pi->m_addrPublic, sizeof(sockaddr));
					}

					SOCKADDR_IN* pAddr = (SOCKADDR_IN*) ( pi->m_bInSameNAT ? &pi->m_addrLocal : &pi->m_addrPublic);			
					LogToFile("[getPeerAddress:%d] %s:%d %s", pi->m_nKey, inet_ntoa(pAddr->sin_addr), htons(pAddr->sin_port), pi->m_bInSameNAT ? "(SameNAT)": "");					

					// {{ incoming peer에 대해서 동일 NAT 내에 있는지 테스트한다.
					if(idPeer == m_nKey) {
						m_addrPublic = pi->m_addrPublic;
					}
					// }} incoming peer에 대해서 동일 NAT 내에 있는지 테스트한다.
					
					return true;
				}			
			}		
		}
		timeEnd = timeGetTime();
	}
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::getHostAddress(DWORD milisec, sockaddr* pAddr)
{	
	if(bConnected == false) 
		return false;
	RegisterPeer(idHost);
	RequestPeerAddr(idHost);
	bool ret = getPeerAddress(idHost, milisec, &hostSockAddr);
	if(pAddr) memcpy(pAddr, &hostSockAddr, sizeof(mySockAddr));	
	return ret;
}
//---------------------------------------------------------------------------
bool p2pConn_t::requestConnectToHost(DWORD milisec, SOCKADDR* pAddr)
{
	fd_set readFds, writeFds, exceptFds;
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	struct timeval timevalue;

	timevalue.tv_sec = 1;
	timevalue.tv_usec = 0;
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)p2pBuffer;
	
	bConnected = false;
	if(RequestConnectPeer(idHost) == false) {
		return false;
	}

	DWORD timeStart = timeGetTime();
	DWORD timeEnd = timeStart;

	while(timeEnd - timeStart < milisec) {
		FD_SET(p2psSd, &readFds);
		::select(0, &readFds, &writeFds, &exceptFds, &timevalue);
		if (FD_ISSET(p2psSd, &readFds)) {
			SOCKADDR addr;
			int addr_len = sizeof(SOCKADDR);
			int nByteReceived = recvfrom(p2psSd, p2pBuffer, 65536, 0, &addr, &addr_len);
			if(nByteReceived>0) {
				ProcessP2PPacket(pPacket, &addr);
				if(bConnected) {
					STRUCT_PEER_INFO* pi = GetPeerInfo(idHost);
					if(pi->m_bInSameNAT) {
						memcpy(pAddr, &pi->m_addrLocal, sizeof(sockaddr));
					}
					else {
						memcpy(pAddr, &pi->m_addrPublic, sizeof(sockaddr));
					}					
					return true;
				}
			}
		}		
		timeEnd = timeGetTime();
	}
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::getMyAddress(DWORD milisec, sockaddr* pAddr)
{	
	//if(bConnected == false)	// [-] 20070410 검증필요
	//	return false;			// [-] 20070410 검증필요

	RegisterPeer(id);
	RequestPeerAddr(id);
	bool ret = getPeerAddress(id, milisec, &mySockAddr);
	if(pAddr) memcpy(pAddr, &mySockAddr, sizeof(mySockAddr));		
	RemovePeer(id);
	return ret;
}
//---------------------------------------------------------------------------
bool p2pConn_t::getMyAddress(DWORD milisec, sockaddr* pAddrLocal, sockaddr* pAddrPublic)
{	
	//if(bConnected == false)	// [-] 20070410 검증필요
	//	return false;			// [-] 20070410 검증필요

	bool bSuccess = false;
	RegisterPeer(id);
	RequestPeerAddr(id);
	if(getPeerAddress(id, milisec, &mySockAddr)) { // [!] 20070601
		STRUCT_PEER_INFO* pi = GetPeerInfo(id);
		if(pi) {		
			memcpy(pAddrLocal, &pi->m_addrLocal, sizeof(SOCKADDR));		
			memcpy(pAddrPublic, &pi->m_addrPublic, sizeof(SOCKADDR));		
			bSuccess = true;
		}	
	}
	//RemovePeer(id);// [-] 20070410 검증필요
	return bSuccess;
}
//---------------------------------------------------------------------------
bool p2pConn_t::getPeerAddress(int id, SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic)
{
	STRUCT_PEER_INFO* pi = GetPeerInfo(id);
	if(pi) {
		memcpy(pAddrLocal, &pi->m_addrLocal, sizeof(SOCKADDR));
		memcpy(pAddrPublic, &pi->m_addrPublic, sizeof(SOCKADDR));		
		return true;
	}	
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::setPeerAddress(int id, SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic, SOCKADDR* addrAgainstI)
{
	STRUCT_PEER_INFO* pi = GetPeerInfo(id);
	if(pi && pAddrLocal && pAddrPublic) {
		memcpy(&pi->m_addrLocal, pAddrLocal, sizeof(SOCKADDR));
		memcpy(&pi->m_addrPublic, pAddrPublic, sizeof(SOCKADDR));

		if ( ((SOCKADDR_IN *)&(pi->m_addrPublic))->sin_addr.S_un.S_addr == ((SOCKADDR_IN *)&m_addrPublic)->sin_addr.S_un.S_addr) {			
			pi->m_bInSameNAT = true;
			if(addrAgainstI) memcpy(addrAgainstI, pAddrLocal, sizeof(SOCKADDR));
		} else {
			pi->m_bInSameNAT = false;
			if(addrAgainstI) memcpy(addrAgainstI, pAddrPublic, sizeof(SOCKADDR));
		}
		return true;
	}	
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::setHostAddress(SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic)
{
	if(!setPeerAddress(idHost, pAddrLocal, pAddrPublic)) {
		memcpy(&hostAddrLocal, pAddrLocal, sizeof(SOCKADDR));
		memcpy(&hostAddrPublic, pAddrPublic, sizeof(SOCKADDR));
		bHostAddrPended = true;
	}
	return true;
}
//---------------------------------------------------------------------------
int p2pConn_t::processP2pPacket(DWORD nParam1, DWORD nParam2)
{
	char str[1024] = "";
	switch(nParam1) {
		case P2P_SVR_INIT:
			LOG("processP2pPacket P2P_SVR_INIT");
			break;

		case P2P_SVR_CONNECT_TRY:
			LOG("processP2pPacket P2P_SVR_CONNECT_TRY");
			break;

		case P2P_SVR_CONNECT:
			LOG("processP2pPacket P2P_SVR_CONNECT");
			bWaitForConnect = false;
			break;

		case P2P_SVR_ADDR_TRY:
			LOG("processP2pPacket P2P_SVR_ADDR_TRY");
			break;

		case P2P_SVR_ADDR_OK:
			LOG("processP2pPacket P2P_SVR_ADDR_OK");
			bWaitForSvrAddr = false;
			break;

		case P2P_SVR_ADDR_FAIL:
			LOG("processP2pPacket P2P_SVR_ADDR_FAIL");
			bWaitForSvrAddr = false;
			break;

		case P2P_SVR_EXIT_TRY:
			LOG("processP2pPacket P2P_SVR_EXIT_TRY");
			break;

		case P2P_SVR_EXIT_OK:
			LOG("processP2pPacket P2P_SVR_EXIT_OK");
			break;

		case P2P_SVR_EXIT_FAIL:
			LOG("processP2pPacket P2P_SVR_EXIT_FAIL");
			break;

		case P2P_SVR_EXIT_ALL_TRY:
			LOG("processP2pPacket P2P_SVR_EXIT_ALL_TRY");
			break;

		case P2P_SVR_EXIT_ALL_OK:
			LOG("processP2pPacket P2P_SVR_EXIT_ALL_OK");
			break;

		case P2P_SVR_EXIT_ALL_FAIL:
			LOG("processP2pPacket P2P_SVR_EXIT_ALL_FAIL");
			break;

		case P2P_SVR_DISCONNECT:
			LOG("processP2pPacket P2P_SVR_DISCONNECT");
			bWaitForConnect = false;
			break;

		case P2P_SVR_EXPELED:
			LOG("processP2pPacket P2P_SVR_EXPELED");
			break;	

		case P2P_SVR_ADDR_PARTIALLY_OK:
			LOG("processP2pPacket P2P_SVR_ADDR_PARTIALLY_OK");
			break;	

		case P2P_SESSION_INIT:
			LOG("processP2pPacket P2P_SESSION_INIT");
			break;
		//case P2P_SESSION_SET_ADDR:
		//	LOG("processP2pPacket P2P_SESSION_SET_ADDR");
		//	break;
		//case P2P_SESSION_DIRECT_TRY:
		//	LOG("processP2pPacket P2P_SESSION_DIRECT_TRY");
		//	break;
		case P2P_SESSION_HOLE_PUNCHING_REQ:	//최초의 요구 packet을 보낸 상태
			LOG("processP2pPacket P2P_SESSION_HOLE_PUNCHING_REQ");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY0:
			LOG("processP2pPacket P2P_SESSION_HOLE_PUNCHING_TRY0");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY1:
			LOG("processP2pPacket P2P_SESSION_HOLE_PUNCHING_TRY1");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY2:
			LOG("processP2pPacket P2P_SESSION_HOLE_PUNCHING_TRY2");
			break;
		case P2P_SESSION_HOLE_SCAN0:
			LOG("processP2pPacket P2P_SESSION_HOLE_SCAN0");
			break;
		case P2P_SESSION_HOLE_SCAN1:
			LOG("processP2pPacket P2P_SESSION_HOLE_SCAN1");
			break;
		case P2P_SESSION_HOLE_SCAN2:
			LOG("processP2pPacket P2P_SESSION_HOLE_SCAN2");
			break;
		case P2P_SESSION_DIRECT_ESTABLISHED:
			LOG("processP2pPacket P2P_SESSION_DIRECT_ESTABLISHED");
			bConnected = true; // [+] 20070223
			break;
		case P2P_SESSION_HOLE_PUNCHING_ESTABLISHED:
			LOG("processP2pPacket P2P_SESSION_HOLE_PUNCHING_ESTABLISHED");
			bConnected = true; // [+] 20070223
			break;
		case P2P_SESSION_DISCONNECTED:
			LOG("processP2pPacket P2P_SESSION_DISCONNECTED");
			break;
		default:
			sprintf(str, "processP2pPacket %x %x", nParam1, nParam2);
			LOG(str);

	}
	if(pParent) pParent->processP2pPacket(nParam1, nParam2); // [+] 20070410
	return 9;
}
//---------------------------------------------------------------------------
int p2pConn_t::onLossP2pPacket(DWORD nParam1, DWORD nParam2)
{
	char str[1024] = "";
	switch(nParam1) {
		case P2P_SVR_INIT:
			LOG("onLossP2pPacket P2P_SVR_INIT");
			break;

		case P2P_SVR_CONNECT_TRY:
			LOG("onLossP2pPacket P2P_SVR_CONNECT_TRY");
			break;

		case P2P_SVR_CONNECT:
			LOG("onLossP2pPacket P2P_SVR_CONNECT");
			bWaitForConnect = false;
			break;

		case P2P_SVR_ADDR_TRY:
			LOG("onLossP2pPacket P2P_SVR_ADDR_TRY");
			break;

		case P2P_SVR_ADDR_OK:
			LOG("onLossP2pPacket P2P_SVR_ADDR_OK");
			bWaitForSvrAddr = false;
			break;

		case P2P_SVR_ADDR_FAIL:
			LOG("onLossP2pPacket P2P_SVR_ADDR_FAIL");
			bWaitForSvrAddr = false;
			break;

		case P2P_SVR_EXIT_TRY:
			LOG("onLossP2pPacket P2P_SVR_EXIT_TRY");
			break;

		case P2P_SVR_EXIT_OK:
			LOG("onLossP2pPacket P2P_SVR_EXIT_OK");
			break;

		case P2P_SVR_EXIT_FAIL:
			LOG("onLossP2pPacket P2P_SVR_EXIT_FAIL");
			break;

		case P2P_SVR_EXIT_ALL_TRY:
			LOG("onLossP2pPacket P2P_SVR_EXIT_ALL_TRY");
			break;

		case P2P_SVR_EXIT_ALL_OK:
			LOG("onLossP2pPacket P2P_SVR_EXIT_ALL_OK");
			break;

		case P2P_SVR_EXIT_ALL_FAIL:
			LOG("onLossP2pPacket P2P_SVR_EXIT_ALL_FAIL");
			break;

		case P2P_SVR_DISCONNECT:
			LOG("onLossP2pPacket P2P_SVR_DISCONNECT");
			bWaitForConnect = false;
			break;

		case P2P_SVR_EXPELED:
			LOG("onLossP2pPacket P2P_SVR_EXPELED");
			break;	

		case P2P_SVR_ADDR_PARTIALLY_OK:
			LOG("onLossP2pPacket P2P_SVR_ADDR_PARTIALLY_OK");
			break;	

		case P2P_SESSION_INIT:
			LOG("onLossP2pPacket P2P_SESSION_INIT");
			break;
		//case P2P_SESSION_SET_ADDR:
		//	LOG("onLossP2pPacket P2P_SESSION_SET_ADDR");
		//	break;
		//case P2P_SESSION_DIRECT_TRY:
		//	LOG("onLossP2pPacket P2P_SESSION_DIRECT_TRY");
		//	break;
		case P2P_SESSION_HOLE_PUNCHING_REQ:	//최초의 요구 packet을 보낸 상태
			LOG("onLossP2pPacket P2P_SESSION_HOLE_PUNCHING_REQ");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY0:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_PUNCHING_TRY0");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY1:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_PUNCHING_TRY1");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY2:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_PUNCHING_TRY2");
			break;
		case P2P_SESSION_HOLE_SCAN0:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_SCAN0");
			break;
		case P2P_SESSION_HOLE_SCAN1:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_SCAN1");
			break;
		case P2P_SESSION_HOLE_SCAN2:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_SCAN2");
			break;
		case P2P_SESSION_DIRECT_ESTABLISHED:
			LOG("onLossP2pPacket P2P_SESSION_DIRECT_ESTABLISHED");
			break;
		case P2P_SESSION_HOLE_PUNCHING_ESTABLISHED:
			LOG("onLossP2pPacket P2P_SESSION_HOLE_PUNCHING_ESTABLISHED");
			break;
		case P2P_SESSION_DISCONNECTED:
			LOG("onLossP2pPacket P2P_SESSION_DISCONNECTED");
			break;
		default:
			sprintf(str, "onLossP2pPacket %x %x", nParam1, nParam2);
			LOG(str);
	}
	return 9;
}
//---------------------------------------------------------------------------
int p2pConn_t::onP2pNotification(DWORD nParam1, DWORD nParam2)
{
	char str[1024] = "";
	switch(nParam1) {
		case P2P_SVR_INIT:
			LOG("onP2pNotification P2P_SVR_INIT");
			break;

		case P2P_SVR_CONNECT_TRY:
			LOG("onP2pNotification P2P_SVR_CONNECT_TRY");
			break;

		case P2P_SVR_CONNECT:
			LOG("onP2pNotification P2P_SVR_CONNECT");
			bWaitForConnect = false;
			break;

		case P2P_SVR_ADDR_TRY:
			LOG("onP2pNotification P2P_SVR_ADDR_TRY");
			break;

		case P2P_SVR_ADDR_OK: {
			LOG("onP2pNotification P2P_SVR_ADDR_OK");
			STRUCT_PEER_INFO* piResolved = (STRUCT_PEER_INFO*)nParam2;
			if(piResolved && piResolved->m_nKey != id) {
				RequestConnectPeer(piResolved->m_nKey);
			}			
			bWaitForSvrAddr = false;
		} break;

		case P2P_SVR_ADDR_FAIL:
			LOG("onP2pNotification P2P_SVR_ADDR_FAIL");
			bWaitForSvrAddr = false;
			break;

		case P2P_SVR_EXIT_TRY:
			LOG("onP2pNotification P2P_SVR_EXIT_TRY");
			break;

		case P2P_SVR_EXIT_OK:
			LOG("onP2pNotification P2P_SVR_EXIT_OK");
			break;

		case P2P_SVR_EXIT_FAIL:
			LOG("onP2pNotification P2P_SVR_EXIT_FAIL");
			break;

		case P2P_SVR_EXIT_ALL_TRY:
			LOG("onP2pNotification P2P_SVR_EXIT_ALL_TRY");
			break;

		case P2P_SVR_EXIT_ALL_OK:
			LOG("onP2pNotification P2P_SVR_EXIT_ALL_OK");
			break;

		case P2P_SVR_EXIT_ALL_FAIL:
			LOG("onP2pNotification P2P_SVR_EXIT_ALL_FAIL");
			break;

		case P2P_SVR_DISCONNECT:
			LOG("onP2pNotification P2P_SVR_DISCONNECT");
			bWaitForConnect = false;
			break;

		case P2P_SVR_EXPELED:
			LOG("onP2pNotification P2P_SVR_EXPELED");
			break;	

		case P2P_SVR_ADDR_PARTIALLY_OK:
			LOG("onP2pNotification P2P_SVR_ADDR_PARTIALLY_OK");
			break;	

		case P2P_SESSION_INIT:
			LOG("onP2pNotification P2P_SESSION_INIT");
			break;
		//case P2P_SESSION_SET_ADDR:
		//	LOG("onP2pNotification P2P_SESSION_SET_ADDR");
		//	break;
		//case P2P_SESSION_DIRECT_TRY:
		//	LOG("onP2pNotification P2P_SESSION_DIRECT_TRY");
		//	break;
		case P2P_SESSION_HOLE_PUNCHING_REQ:	//최초의 요구 packet을 보낸 상태
			LOG("onP2pNotification P2P_SESSION_HOLE_PUNCHING_REQ");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY0:
			LOG("onP2pNotification P2P_SESSION_HOLE_PUNCHING_TRY0");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY1:
			LOG("onP2pNotification P2P_SESSION_HOLE_PUNCHING_TRY1");
			break;
		case P2P_SESSION_HOLE_PUNCHING_TRY2:
			LOG("onP2pNotification P2P_SESSION_HOLE_PUNCHING_TRY2");
			break;
		case P2P_SESSION_HOLE_SCAN0:
			LOG("onP2pNotification P2P_SESSION_HOLE_SCAN0");
			break;
		case P2P_SESSION_HOLE_SCAN1:
			LOG("onP2pNotification P2P_SESSION_HOLE_SCAN1");
			break;
		case P2P_SESSION_HOLE_SCAN2:
			LOG("onP2pNotification P2P_SESSION_HOLE_SCAN2");
			break;
		case P2P_SESSION_DIRECT_ESTABLISHED:
			LOG("onP2pNotification P2P_SESSION_DIRECT_ESTABLISHED");
			bConnected = true; // [+] 20070223
			break;
		case P2P_SESSION_HOLE_PUNCHING_ESTABLISHED:
			LOG("onP2pNotification P2P_SESSION_HOLE_PUNCHING_ESTABLISHED");
			bConnected = true; // [+] 20070223
			break;
		case P2P_SESSION_DISCONNECTED:
			LOG("onP2pNotification P2P_SESSION_DISCONNECTED");
			break;
		default:
			sprintf(str, "onP2pNotification %x %x", nParam1, nParam2);
			LOG(str);
	}
	if(pParent) pParent->onP2pNotification(nParam1, nParam2); // [+] 20070410
	return 9;
}
//---------------------------------------------------------------------------
#ifdef P2pConnNativeGms // 20061009 dEAthcURe default off
int p2pConn_t::gmsRecv(int nSec)
{
	fd_set readFds, writeFds, exceptFds;
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	FD_SET(gmsSd, &readFds);

	struct timeval timevalue;

	timevalue.tv_sec = nSec;
	timevalue.tv_usec = 0;

	::select(gmsSd+1, &readFds, &writeFds, &exceptFds, &timevalue);
	if (FD_ISSET(gmsSd, &readFds)) {
		int nByteReceived = recv(gmsSd, recvBuffer, 256, 0); recvBuffer[nByteReceived] = 0x0;
		return nByteReceived;
	}
	return -1;
}
//---------------------------------------------------------------------------
bool p2pConn_t::gmsBeHost(void)	
{
	sprintf(sendBuffer, "req behost %d", id);
	int err = send(gmsSd, sendBuffer, strlen(sendBuffer), 0 );
	
	if(gmsRecv(10)) {
		if(!strncmp(recvBuffer, "ack", 3)) {			
			LOG("Being host accepted");
			return true;
		}	
		LOG("Being host request denied by room server");		
	}
	LOG("Recv failed");
	return false;	
}

//---------------------------------------------------------------------------
bool p2pConn_t::gmsGetHost(void)
{
	sprintf(sendBuffer, "req gethost"); // sprintf(sendBuffer, "req userlist");
	int err = send(gmsSd, sendBuffer, strlen(sendBuffer), 0 );

	if(gmsRecv(10)<=0) {
		LOG("Recv failed.");
		return false;
	}

	if(strncmp(recvBuffer, "ack", 3)) {
		LOG("Gethost request denied by room server");
		return false;
	}

	sscanf(recvBuffer, "ack gethost %d", &idHost);			

	char str[1024] = "";
	sprintf(str, "Host registered %d", idHost);
	LOG(str);

	RegisterPeer(idHost);
	RequestPeerAddr(idHost);

	return true;
}
//---------------------------------------------------------------------------
#ifdef EnableUserList
bool p2pConn_t::gmsUserList(void)
{
	sprintf(sendBuffer, "req ul"); // sprintf(sendBuffer, "req userlist");
	int err = send(gmsSd, sendBuffer, strlen(sendBuffer), 0 );

	if(gmsRecv(10)<=0) {
		LOG("Recv failed.");
		return false;
	}

	if(strncmp(recvBuffer, "ack", 3)) {
		LOG("Userlist request denied by room server");
		return false;
	}

	int nPeer = -1;
	int user[16];

	sscanf(recvBuffer, "ack userlist %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
		&nPeer, &user[0], &user[1], &user[2], &user[3], &user[4], &user[5], &user[6], &user[7], 
		&user[8], &user[9], &user[10], &user[11], &user[12], &user[13], &user[14], &user[15]);

	for(int lpp=0;lpp<nPeer;lpp++) {
		//peers.push_back(user[lpp]);
		peers.Push(user[lpp]);
		char str[1024] = "";
		sprintf(str, "User registered %d", user[lpp]);
		LOG(str);
	}		
	return true;
}
#endif
//---------------------------------------------------------------------------
bool p2pConn_t::gmsJoin(void)
{	
	sprintf(sendBuffer, "req join %d", id);
	int err = send(gmsSd, sendBuffer, strlen(sendBuffer), 0 );
	
	if(gmsRecv(10)<0) {
		LOG("Recv failed.");
		return false;
	}

	if(strncmp(recvBuffer, "ack", 3)) {
		LOG("Join request denied by room server"); //logListBox.AddString("Join request denied by room server");		
		return false;
	}	

	LOG("Join succeeded"); //logListBox.AddString("Join succeeded");
	LOG(recvBuffer); //logListBox.AddString(recvBuffer);

	return true;
}
//---------------------------------------------------------------------------
int p2pConn_t::gmsGetId(void)
{
	id = -1;

	sprintf(sendBuffer, "req getid");
	int err = send(gmsSd, sendBuffer, strlen(sendBuffer), 0 );
	
	if(gmsRecv(10)<0) {
		LOG("Recv failed.");
		return id;
	}

	if(strncmp(recvBuffer, "ack", 3)) {
		LOG("GetId request denied by room server"); //logListBox.AddString("Join request denied by room server");		
		return id;
	}	

	
	sscanf(recvBuffer, "ack getid %d", &id);

	LOG("GetId succeeded"); //logListBox.AddString("Join succeeded");
	LOG(recvBuffer); //logListBox.AddString(recvBuffer);

	return id;
}
//---------------------------------------------------------------------------
bool p2pConn_t::gmsConnect(char* ipAddr, int port)
{
	if(gmsSd!=INVALID_SOCKET) {
		closesocket(gmsSd);
		gmsSd = INVALID_SOCKET;
	}

	gmsSd = socket(AF_INET, SOCK_STREAM, 0);
	if(gmsSd == INVALID_SOCKET) 
		goto cleanup;
	
	SOCKADDR_IN addr;
	memset (&addr, '\0', sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipAddr);
	addr.sin_port = htons(port);
	int err = connect(gmsSd, (struct sockaddr *)&addr, sizeof(addr)); 
	if(err) 
		goto cleanup;
	return true;

cleanup:
	if(gmsSd!=INVALID_SOCKET) {
		closesocket(gmsSd);
		gmsSd = INVALID_SOCKET;
	}
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::gmsDisconnect(void)
{
	if(gmsSd!=INVALID_SOCKET) {
		closesocket(gmsSd);
		gmsSd = INVALID_SOCKET;
		return true;
	}
	return false;
}
#endif
//---------------------------------------------------------------------------
class PeerInfoMgr;
//---------------------------------------------------------------------------
STRUCT_PEER_INFO* p2pConn_t::findPeerByAddr(sockaddr* pAddr)
{		
	// 동NAT이면 이렇게 peer를 찾을 수 있지만 아니면 timeout할수밖에 없3

	if(pAddr) {
		SOCKADDR_IN* pAddrReceived = (SOCKADDR_IN*)pAddr;
		//LogToFile("[findPeerByAddr|recevied] %s:%d", inet_ntoa(pAddrReceived->sin_addr), ntohs(pAddrReceived->sin_port));

		const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
		for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++) {
			STRUCT_PEER_INFO* pi = it->second;
			if(pi) {				
				SOCKADDR_IN* pAddrPi = (SOCKADDR_IN*)(pi->m_bInSameNAT ? &pi->m_addrLocal : &pi->m_addrPublic);
				if(pAddrReceived->sin_addr.S_un.S_addr == pAddrPi->sin_addr.S_un.S_addr && 
					pAddrReceived->sin_port == pAddrPi->sin_port) {
					//LogToFile("--|matched| id:%d %s:%d", pi->m_nKey, inet_ntoa(pAddrPi->sin_addr), ntohs(pAddrPi->sin_port));
					return pi;
				}	
				/*
				else {
					SOCKADDR_IN* pAddrPi = (SOCKADDR_IN*)&pi->m_addrLocal;
					LogToFile("--|mismatched| %s ", pi->m_bInSameNAT ? "SameNAT" : "");
					LogToFile("local id:%d %s:%d", pi->m_nKey, inet_ntoa(pAddrPi->sin_addr), ntohs(pAddrPi->sin_port));
					pAddrPi = (SOCKADDR_IN*)&pi->m_addrPublic;
					LogToFile("public id:%d %s:%d", pi->m_nKey, inet_ntoa(pAddrPi->sin_addr), ntohs(pAddrPi->sin_port));
				}
				*/
			}
		}

	}
	return 0x0;
}
//---------------------------------------------------------------------------
void p2pConn_t::timeoutHeartbeat(void)
{
	time_t timeCurrent;
	time(&timeCurrent);
	const PeerInfoMgr::PeerInfoMap& peerMap = m_pPeerInfoMgr->GetMap();
	for(PeerInfoMgr::PeerInfoMapCIT it = peerMap.begin(); it != peerMap.end(); it++) {
		STRUCT_PEER_INFO* pi = it->second;
		if(pi && pi->timeBeginHeartbeat && timeCurrent - pi->timeBeginHeartbeat > HeartbeatTimeout) { // if(pi && pi->heartbeatCount == 0x0 && pi->timeBeginHeartbeat && timeCurrent - pi->timeBeginHeartbeat > HeartbeatTimeout) { // [!] 20070220
			pi->heartbeatCount = nAppPacketToStopHeartbeating;
			LogToFile("Timeout heartbeating %d (%d sec)", pi->m_nKey, HeartbeatTimeout);
			pi->timeBeginHeartbeat = 0x0; // [+] 20070220
		}
	}
}
//---------------------------------------------------------------------------
bool p2pConn_t::onRecvFrom(char* buffer, int nByteReceived, sockaddr* from, int fromLen)
{
	//int nResult;	
	PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)buffer;	
	//int nError;
	bool bClosed = false; 		
	
	{
		//////////////////////////////
		// for Test		
		//if (m_nDropRx > 0) {
		//	m_nDropRx--;
		//	continue;
		//}		
		while (m_bSuspendRx) {
			Sleep(40);
		}
		//////////////////////////////
	}
	// check if qVDs pacekt
	if(nByteReceived < sizeof(PACKET_HEADER) || memcmp(pPacket->header.heading, PacketHeading, SizeOfHeading)) {
		// {{ 20070130 stop heartbeating
		
		STRUCT_PEER_INFO* pi = findPeerByAddr(from);

		/*
		SOCKADDR_IN* pAddr = (SOCKADDR_IN*)from;
		LogToFile("[chk] from %s:%d", inet_ntoa(pAddr->sin_addr), ntohs(pAddr->sin_port));

		if(pi) {
			LogToFile("[chk] pi->local %s:%d = %d", inet_ntoa(((SOCKADDR_IN*)&pi->m_addrLocal)->sin_addr), ntohs(((SOCKADDR_IN*)&pi->m_addrLocal)->sin_port), pi->heartbeatCount);
			LogToFile("[chk] pi->public %s:%d = %d", inet_ntoa(((SOCKADDR_IN*)&pi->m_addrPublic)->sin_addr), ntohs(((SOCKADDR_IN*)&pi->m_addrPublic)->sin_port), pi->heartbeatCount);
		}
		*/

		if(pi) {
			if(pi->heartbeatCount < nAppPacketToStopHeartbeating) {
				SOCKADDR_IN* pAddr = (SOCKADDR_IN*) ( pi->m_bInSameNAT ? &pi->m_addrLocal : &pi->m_addrPublic);			
				LogToFile("[Appl:%d] %s:%d = %d %s %d/%d", pi->m_nKey, inet_ntoa(pAddr->sin_addr), htons(pAddr->sin_port), pi->heartbeatCount, pi->m_bInSameNAT ? "(SameNAT)": "", pi->heartbeatCount, nAppPacketToStopHeartbeating-1);
				//LogToFile("[Heartbeat] --%s:%d = %d", inet_ntoa(((SOCKADDR_IN*)&pi->m_addrLocal)->sin_addr), ntohs(((SOCKADDR_IN*)&pi->m_addrLocal)->sin_port), pi->heartbeatCount);
				//LogToFile("[Heartbeat] --%s:%d = %d", inet_ntoa(((SOCKADDR_IN*)&pi->m_addrPublic)->sin_addr), ntohs(((SOCKADDR_IN*)&pi->m_addrPublic)->sin_port), pi->heartbeatCount);
				pi->heartbeatCount++;

				if(pi->heartbeatCount>=nAppPacketToStopHeartbeating) {
					LogToFile("Heartbeat stopped %d (app packet detected)", pi->m_nKey);
				}
			}			
		}
		else {
			//SOCKADDR_IN* pAddr = (SOCKADDR_IN*)from;
			//LogToFile("[Appl] no match from %s:%d", inet_ntoa(pAddr->sin_addr), htons(pAddr->sin_port));
		}		

		timeoutHeartbeat();
		// }} 20070130 stop heartbeating
		return false; // not a qVDs packet
	}
	// check if qVDs pacekt

	// {{ 20070130 dEAthcURe
	//SOCKADDR_IN* pAddr = (SOCKADDR_IN*)from;
	//LogToFile("[qVDs] packet:%d from %s:%d", pPacket->MsgID(), inet_ntoa(pAddr->sin_addr), htons(pAddr->sin_port));
	//TCHAR waddr[256];
	//mbstowcs(waddr, inet_ntoa(pAddr->sin_addr), 255);
	//debugf(TEXT("[qVDs] packet:%d from %s:%d"), pPacket->MsgID(), waddr, htons(pAddr->sin_port));
	// }} 20070130 dEAthcURe

	ProcessP2PPacket(pPacket, from);
	return true;
}
//---------------------------------------------------------------------------
bool p2pConn_t::onRecv(char* buffer, int nByte)
{
	// parse packet
	char msgKind[16];
	char cmd[16];
	int idPeer;
	sscanf(buffer, "%s %s %d", msgKind, cmd, &idPeer);
	if(!strcmp(cmd, "join")) {
		//peers.push_back(idPeer);				
		peers.Push(idPeer);
		LOG(recvBuffer);
		RegisterPeer(idPeer, 0x0);
		RequestPeerAddr(idPeer);
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void p2pConn_t::tick(bool bRecvFrom)
{	
	#ifdef P2pConnNativeGms
	int nByteReceived = gmsRecv(0);
	if(nByteReceived > 0) {
		onRecv(recvBuffer, nByteReceived);
	}	
	#endif

	if(bRecvFrom) RxPacket(); // onRecvFrom이 추가되면 없애라
	
	float timeCurr = (float)timeGetTime()/1000;	
	if(bHeartbeat && intervalHeartbeat && timeNextHeartbeat < timeCurr) { // 20061009 dEAthcURe intervalHeartbeat가 0이면 heartbeat하지 않는다
		//Heartbeat(); // 20070223 testdisable // TxHeartBeat(); // [-] 20070129
		timeNextHeartbeat = timeCurr + intervalHeartbeat;
	}

	if(timeNextHeartbeatToServer < timeCurr) {
		TxHeartbeatToServer();
		timeNextHeartbeatToServer = timeCurr + intervalHeartbeatToServer;
	}
}
//---------------------------------------------------------------------------
void p2pConn_t::setHost(int _idHost)
{
	idHost = _idHost;
	RegisterPeer(idHost);	
}
//---------------------------------------------------------------------------
bool p2pConn_t::setHost(int _idHost, SOCKADDR* pAddr)
{
	idHost = _idHost;
	RegisterPeer(idHost);	
	if(bHostAddrPended) {
		return setPeerAddress(idHost, &hostAddrLocal, &hostAddrPublic, pAddr);				
	}
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::initSocket(void)
{
	if(p2psSd!=INVALID_SOCKET) {
		closesocket(p2psSd);
		p2psSd = INVALID_SOCKET;
	}
	
	SOCKADDR_IN	addr;
	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	DWORD status;
	
	p2psSd = socket(AF_INET, SOCK_DGRAM, 0);
	if (p2psSd == INVALID_SOCKET){
		StopOperation();
		goto cleanup;
	} 

	// disable  new behavior using
	// IOCTL: SIO_UDP_CONNRESET
	status = WSAIoctl(p2psSd, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);

	////////////////////////////////////////////////////////////////////////////////////////////
	// Socket Option 설정 2005.08.11
	int opt = 16*1024;
	int	nTimeOut = 10000;
	if(setsockopt(p2psSd, SOL_SOCKET, SO_RCVBUF, (char *)&opt, sizeof(int)) < 0) goto cleanup;
	if(setsockopt(p2psSd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeOut, sizeof(int)) < 0) goto cleanup;
	// Socket Option 설정 2005.08.11
	////////////////////////////////////////////////////////////////////////////////////////////

	memset(&addr, 0, sizeof(SOCKADDR));
	addr.sin_family				= AF_INET;
	addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);
	addr.sin_port				= htons((u_short)m_nPort);

	if (bind(p2psSd, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){
		StopOperation();
		goto cleanup;
	}

	return true;

cleanup:
	if(p2psSd!=INVALID_SOCKET) {
		closesocket(p2psSd);
		p2psSd = INVALID_SOCKET;
	}
	return false;
}
//---------------------------------------------------------------------------
bool p2pConn_t::initUdp(SOCKET sd, int localPort)
{
	bConnected = false;
	p2psSd = sd;
	if(p2psSd==INVALID_SOCKET) {
		if(!initSocket()) return false;
	}

	RemoveAllPeer();

	return InitEmbedding(id, htonl(inet_addr(p2psIpAddr)), p2psPort, p2psSd, localPort,
		new CP2PHandlerInvoker<p2pConn_t>(this, &p2pConn_t::processP2pPacket),
		new CP2PHandlerInvoker<p2pConn_t>(this, &p2pConn_t::onLossP2pPacket),
		new CP2PHandlerInvoker<p2pConn_t>(this, &p2pConn_t::onP2pNotification));	
}
//---------------------------------------------------------------------------
bool p2pConn_t::initUdp(char* ipAddr, int port, SOCKET sd, int localPort) // [!] 20070410 // bool p2pConn_t::initUdp(char* ipAddr, int port, SOCKET sd)
{
	strcpy(p2psIpAddr, ipAddr);
	p2psPort = port;
	return initUdp(sd, localPort); // [!] 20070410 // return initUdp(sd);
}
//---------------------------------------------------------------------------
bool p2pConn_t::init(void)
{
	// 20070410 dEAthcURe
	bHeartbeat = false;
	return true;
}
//---------------------------------------------------------------------------
void p2pConn_t::deinit(void)
{	
	if (m_serverInfo.m_nStage >= P2P_SVR_CONNECT) {
		RequestExit();		
	}	
	StopOperation();
}
//---------------------------------------------------------------------------
p2pConn_t::p2pConn_t(pIp2pConn_t _pParent, bool bGlobal, int localPort, int nMaxPeer, int retransmitInterval) // [!] 20070410 // p2pConn_t::p2pConn_t(int localPort, int nMaxPeer, int retransmitInterval)
: CP2PClientEx(localPort, nMaxPeer, retransmitInterval)
, bWaitForSvrAddr(false)
, bWaitForConnect(false)
, gmsSd(INVALID_SOCKET)
, gmsPort(62000)

, p2psSd(INVALID_SOCKET)
, _cb_log(0x0)
, timeNextHeartbeat(0)
, intervalHeartbeat(2) // 20061009 dEAthcURe intervalHeartbeat initial value? heartbeat on/off정책
, timeNextHeartbeatToServer(0) // 20070221 dEAthcURe
, intervalHeartbeatToServer(30) // [!] 20070630 10 -> 30 // 20070221 dEAthcURe
, idHost(-1)
, id(-1)
, bConnected(false)
{
	pParent = _pParent; // 20070410

	// {{ 20070224
	memset(&hostAddrLocal, 0, sizeof(SOCKADDR));
	memset(&hostAddrPublic, 0, sizeof(SOCKADDR));	
	bHostAddrPended = false;
	// }} 20070224

	memset(&mySockAddr, 0, sizeof(mySockAddr));
	memset(&hostSockAddr, 0, sizeof(hostSockAddr));

	strcpy(gmsIpAddr, "127.0.0.1");	

	if(_gini_ipAddr[0]) {
		setP2pServer(_gini_ipAddr, _gini_port);
	}
	else {
		setP2pServer(DefaultP2psIpAddr, DefaultP2psPort);
	}

	#ifdef P2pConnNativeIni // 20061009 dEAthcURe default off
	FILE* fp = fopen("udpServer.ini", "rt");
	if(fp) {
		fscanf(fp, "%s %d", p2psIpAddr, &p2psPort);
		fclose(fp);
	}
	#endif
	timeNextHeartbeatToServer = timeNextHeartbeat = (float)timeGetTime()/1000;	
	if(bGlobal) {
		g_p2pConn = this;
	}
}
//---------------------------------------------------------------------------
p2pConn_t::~p2pConn_t(void)
{
	#ifdef P2pConnNativeGms
	gmsDisconnect();
	#endif
	if(g_p2pConn == this) {
		g_p2pConn = 0x0;
	}
}
//---------------------------------------------------------------------------
void TickP2pConn(bool bRecvFrom) // 20070413 dEAthcURe|HP nonblock p2p client // void TickP2pConn(void)
{
	static DWORD timePrev = 0;
	DWORD timeCurr = timeGetTime();

	if(timePrev && timeCurr - timePrev > 1000) {
		debugf(TEXT("          <---------- [TickP2pConn] Interval of Tick is too slow to establish p2p connection (%d ms)"), timeCurr - timePrev);		
	}
	timePrev = timeCurr;

	if(g_p2pConn) {
		g_p2pConn->tick(bRecvFrom); // 20070413 dEAthcURe|HP nonblock p2p client // g_p2pConn->tick();
	}
}
//---------------------------------------------------------------------------
#endif
