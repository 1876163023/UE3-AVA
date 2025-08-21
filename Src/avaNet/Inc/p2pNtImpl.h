//
// p2pNtImpl.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef p2pNtImplH
#define p2pNtImplH
//---------------------------------------------------------------------------
#include "Ip2p.h"
//---------------------------------------------------------------------------
//extern void log(char* fmt, ...);
//---------------------------------------------------------------------------
struct p2pNtImpl_t : Ip2p_t, Ip2pConn_t {
	pp2pConn_t pp2pConn;	
	bool bConnected; // [+] 20070627 dEAthcURe

	char sendBuffer[65536];
	char recvBuffer[65536];	

	virtual bool connectToServer(char* ipAddr, int port, int idAccount) {		
		// {{ [+] 20070627
		if(bConnected) {
			return true;
		}
		bConnected = true;
		// }} [+] 20070627

		__super::connectToServer(ipAddr, port, idAccount);

		pp2pConn->setId(idAccount);
		pp2pConn->initUdp(ipAddr, port, sd, localPort);		

		if(WaitForSingleObject(hEventP2pSvrConnected, timeServerConnect)==WAIT_OBJECT_0) {
			//log("[p2pNtImpl_t::connectToServer] p2p server connected.\n");

			ResetEvent(hEventP2pSvrAddr);
			pp2pConn->RegisterPeer(idAccount);
			pp2pConn->RequestPeerAddr(idAccount);

			if(WaitForSingleObject(hEventP2pSvrAddr, timeGetAddr)==WAIT_OBJECT_0) {
				//log("[p2pNtImpl_t::connectToServer] p2p server addr succeeded.\n");

				STRUCT_PEER_INFO* pi = pp2pConn->GetPeerInfo(idAccount);
				if(pi) {
					memcpy(&addrLocal, &pi->m_addrLocal, sizeof(SOCKADDR));
					memcpy(&addrPublic, &pi->m_addrPublic, sizeof(SOCKADDR));					
					return true;
				}
				else { //log("[p2pNtImpl_t::connectToServer] GetPeerInfo failed.\n");
				}
			}
			else { //log("[p2pNtImpl_t::connectToServer] WaitForSingleObject(hEventP2pSvrAddr) timeout.\n");
			}
		}
		else { //log("[p2pNtImpl_t::connectToServer] WaitForSingleObject(hEventP2pSvrConnected) timeout.\n");
		}
		return false;
	}

	
	virtual bool heartbeatToServer(void) {
		if(bConnected) {
			static DWORD timeNext=0;
			DWORD timeCurrent = timeGetTime();
			if(timeCurrent > timeNext) {
				pp2pConn->TxHeartbeatToServer();
				timeNext = timeCurrent + timeIntervalHeartbeatToServer;
				return true;
			}
		}
		return false;
	}
	
	virtual bool send_rttProb(int idAccount) {		
		pp2pConn->TxRttProbReq(idAccount);
		return true;
	}	

	virtual void recv(void) {		
		PACKET_P2P_GENERIC*	pPacket = (PACKET_P2P_GENERIC *)recvBuffer;			
		
		SOCKADDR addr;
		int addr_len = sizeof(SOCKADDR);
		int nByteReceived = recvfrom(sd, recvBuffer, 65536, 0, &addr, &addr_len);
		if(nByteReceived>0) { //log("\np2pNtimpl::recv\n");
			pp2pConn->ProcessP2PPacket(pPacket, &addr);			
		}
	}

	virtual bool registerPeer(int idAccount, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic) {
		pp2pConn->RegisterPeer(idAccount, (SOCKADDR*)addrLocal, (SOCKADDR*)addrPublic);
		return true;		
	}

	virtual bool requestConnectPeer(int idAccount) {		
		return pp2pConn->RequestConnectPeer(idAccount);
	}

	virtual void removeAllPeer(void) {
		pp2pConn->RemoveAllPeer();
	}

	virtual void removePeer(int idPeer) {
		pp2pConn->RemovePeer(idPeer);
	}

	virtual bool disconnect(void) {
		if(bConnected) {
			pp2pConn->deinit();	
			bConnected = false; // [+] 20070627 dEAthcURe
		}
		return true;
	}

	bool init(int localPort) {
		__super::init(localPort);
		this->localPort = localPort;

		//WORD wVersionRequested = MAKEWORD( 2, 2 );
		//WSADATA wsaData;
		//int err;err = WSAStartup( wVersionRequested, &wsaData );		

		sd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		int	nReuse = 1;
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&nReuse, sizeof(int));		

		SOCKADDR_IN	addr;
		memset(&addr, 0, sizeof(SOCKADDR));
		addr.sin_family				= AF_INET;
		addr.sin_addr.S_un.S_addr	= htonl(INADDR_ANY);

		//int localPort = 5000 + idAccount;
		addr.sin_port				= htons((u_short)localPort);

		if (bind(sd, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) < 0){			
			return false;
		}	
		return true;
	}

	void deinit(void) {
		disconnect(); // [+] 20070628
		closesocket(sd);
		//WSACleanup();
	}

	virtual void processP2pPacket(DWORD nParam1, DWORD nParam2) {
		//__try { // [!] 20070529
			PACKET_P2P_GENERIC* pPacket = (PACKET_P2P_GENERIC*)nParam1; if(!pPacket) return;		
			switch(pPacket->MsgID()) {
				case MSGID_P2P_REQ_RTTPROB: { //log("MSGID_P2P_REQ_RTTPROB\n");
					pp2pConn->TxRttProbAck();
				} break;
				case MSGID_P2P_ACK_RTTPROB: { //log("MSGID_P2P_ACK_RTTPROB\n");
					pParent->onRttProbAck(pPacket->ConnectID());
				} break;
				case MSGID_P2P_ACK_SEQ : case MSGID_P2P_I_OPEND_DOOR: { //log¹èÁ¦
				} break;
				default: {
					//log("processP2pPacket unknown %d from %d \n", pPacket->MsgID(), pPacket->ConnectID());
				}			
			}
		/* // [!] 20070529
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
			int e = GetExceptionCode();
			//log("processP2pPacket exception %d\n", e);
		}
		*/
	}

	virtual void onP2pNotification(DWORD nParam1, DWORD nParam2) {
		//__try { // [!] 20070529
			switch(nParam1) {
				case P2P_SVR_ADDR_OK: { //debugf(TEXT("P2P_SVR_ADDR_OK"));//log("P2P_SVR_ADDR_OK\n");
					SetEvent(hEventP2pSvrAddr);
				} break;

				case P2P_SVR_CONNECT: { //debugf(TEXT("P2P_SVR_CONNECT"));//log("P2P_SVR_CONNECT\n");
					SetEvent(hEventP2pSvrConnected);
				} break;
				case P2P_SVR_DISCONNECT: { //debugf(TEXT("P2P_SVR_DISCONNECT"));
					//log("P2P_SVR_DISCONNECT\n");
				} break;

				case P2P_SESSION_DIRECT_ESTABLISHED: case P2P_SESSION_HOLE_PUNCHING_ESTABLISHED: case P2P_SESSION_RELAY_ESTABLISHED: { //log("Session established\n");					
					pParent->onSessionEstablished(nParam2);
				} break;

				case P2P_SESSION_DISCONNECTED: {
					//debugf(TEXT("Session disconnected %d"), nParam2);//log("Session disconnected %d\n", nParam2);
					pParent->onSessionDisconnected(nParam2);
				} break;
				default: {
					//debugf(TEXT("onP2pNotification unknown %d"), nParam2);//log("onP2pNotification unknown %d\n", nParam1);				
				}			
			}
		/* // [!] 20070529
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
			int e = GetExceptionCode();
			//debugf(TEXT("onP2pNotification exception %d"), e); //log("onP2pNotification exception %d\n", e);
		}
		*/
	}

	p2pNtImpl_t(pIp2pNetTester_t _pParent=0x0) : Ip2p_t(_pParent) {		
		pp2pConn = new p2pConn_t(this, false);
		bConnected = false; // [+] 20070627 dEAthcURe
	}

	virtual ~p2pNtImpl_t(void) {
		deinit();
		delete pp2pConn;
	}
};
typedef struct p2pNtImpl_t p2pNtImpl_t;
typedef struct p2pNtImpl_t* pp2pNtImpl_t;
//---------------------------------------------------------------------------
#endif