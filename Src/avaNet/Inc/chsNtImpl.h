//
// chsNtImpl.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef chsNtImplH
#define chsNtImplH
//---------------------------------------------------------------------------
#include "Ichs.h"
//---------------------------------------------------------------------------
//extern void log(char* fmt, ...);
//---------------------------------------------------------------------------
struct chsNtImpl_t : Ichs_t {
	enum packetDef {
		pdReqAnonymousJoin, // 'req anonymousjoin',
		pdAckAnonymousJoin, // 'ack anonymousjoin',
		pdNakAnonymousJoin, // 'nak anonymousjoin',
		pdNtfAnonymousJoin, // 'ntf anonymousjoin',
	    
		pdReqBeHost, // 'req behost',
		pdAckBeHost, // 'ack behost',
		pdNakBeHost, // 'nak behost',
		pdNtfBeHost, // 'ntf behost', # 7

		pdReqBeNewHost,
		pdAckBeNewHost,
		pdNakBeNewHost,
		pdNtfBeNewHost,
	    
		pdNtfLinkLost, // 'ntf linklost', # 8
		pdNtfHwInfo, // 'ntf hwinfo',
		pdNtfNetInfo, // 'ntf netinfo',
		pdNtfMigratedHost, // 'ntf migratedhost'
		pdNtfDeprive, // 'ntf deprive',
		pdNtfNoParticipant, // 'ntf noparticipant'
		pdNtfEndGame, // 'ntf endgame'

		pdReqPeerList,
		pdAckPeerList,
		pdNakPeerList,
		pdNtfPeerList,

		pdNtfMyAddr,	

		pdReqPeerList2,
		pdAckPeerList2,
		pdNakPeerList2,

		pdReqConnect,
		pdAckConnect,
		pdNakConnect,
		pdNtfConnect,
	};

	struct packetHeader_t {
		short type;
		short len;	
	};
	typedef struct packetHeader_t packetHeader_t;
	typedef struct packetHeader_t* ppacketHeader_t;

	struct payload_t {
	};
	typedef struct payload_t payload_t;
	typedef struct payload_t* ppayload_t;

	struct payloadReqPeerList_t : payload_t {
		int idAccRequester;
		int idChannel;
		int idRoom;
	};
	typedef struct payloadReqPeerList_t payloadReqPeerList_t;
	typedef struct payloadReqPeerList_t* ppayloadReqPeerList_t;

	struct payloadPeerInfo_t : payload_t {
		int idAcc;
		unsigned long localAddr;
		unsigned int localPort;
		unsigned long publicAddr;
		unsigned int publicPort;
	};
	typedef struct payloadPeerInfo_t payloadPeerInfo_t;
	typedef struct payloadPeerInfo_t* ppayloadPeerInfo_t;

#define MaxUser	32
	
	playerInfo_t playerInfo[MaxUser];
	virtual int getIdAccount(int idx) {
		if(idx<nPlayers) {
			return playerInfo[idx].idAccount;
		}
		return -1;
	}

	virtual pplayerInfo_t getPlayerInfo(int idAccount) {
		for(int lpp=0;lpp<nPlayers;lpp++) {
			if(playerInfo[lpp].idAccount == idAccount) {
				return &playerInfo[lpp];
			}			
		}	
		return 0x0;
	}
	
	char sendBuffer[65536];
	char recvBuffer[65536];	
	int nByteReceived;

	bool sendPacket(int type, char* payload, int len) {
		ppacketHeader_t pHeader = (ppacketHeader_t)sendBuffer;
		pHeader->type = type;
		pHeader->len = len;
		if(payload) {		
			memcpy(pHeader+1, payload, pHeader->len);
		}
		if(send(sd, sendBuffer, sizeof(packetHeader_t) + pHeader->len, 0x0)>0) {
			return true;
		}
		return false;
	}

	virtual bool send_ntfMyAddr(int idAccount, SOCKADDR_IN addrLocal, SOCKADDR_IN addrPublic) {
		//log("[send_ntfMyAddr]\n");
		payloadPeerInfo_t payload;
		payload.idAcc = idAccount;
		payload.localAddr = addrLocal.sin_addr.S_un.S_addr;
		payload.localPort = addrLocal.sin_port;
		payload.publicAddr = addrPublic.sin_addr.S_un.S_addr;
		payload.publicPort = addrPublic.sin_port;		
		return sendPacket(pdNtfMyAddr, (char*)&payload, sizeof(payload));
	}

	virtual bool send_reqPeerList(int idAccRequester, int idChannel, int idRoom) {
		//log("[send_reqPeerList]\n");
		ResetEvent(hEventRecvPeerList);
		payloadReqPeerList_t payloadReqPeerList;
		payloadReqPeerList.idAccRequester = idAccRequester;
		payloadReqPeerList.idChannel = idChannel;
		payloadReqPeerList.idRoom = idRoom;		
		return sendPacket(pdReqPeerList2, (char*)&payloadReqPeerList, sizeof(payloadReqPeerList));
	}

	struct payloadReqConnect_t : payload_t {
		int idAccRequester;
		int idAccPeer;
	};
	typedef struct payloadReqConnect_t payloadReqConnect_t;
	typedef struct payloadReqConnect_t* ppayloadReqConnect_t;

	bool bAckConnect, bNakConnect;
	virtual bool send_reqConnect(int idAccRequester, int idAccPeer) {
		//log("[send_reqConnect]\n");
		bAckConnect = bNakConnect = false;
		payloadReqConnect_t payload;
		payload.idAccRequester = idAccRequester;
		payload.idAccPeer = idAccPeer;		
		return sendPacket(pdReqConnect, (char*)&payload, sizeof(payload));
	}
	virtual bool recv_ansConnect(int sec) {
		time_t timeCurrent;
		time(&timeCurrent);
		time_t timeToWait = timeCurrent + sec;
		while(!bAckConnect && !bNakConnect && timeToWait > timeCurrent) {
			Sleep(1);
			time(&timeCurrent);
		}		
		return bAckConnect;	
	}	

	bool parsePacket(int nByteIncomming) {
		//log("[parsePacket] nByte:%d", nByteIncomming);

		nByteReceived += nByteIncomming;
		ppacketHeader_t pHeader = (ppacketHeader_t)recvBuffer;		
		while(nByteReceived && (int)sizeof(packetHeader_t) + pHeader->len <= nByteReceived) {
			if(onPacketComplete(pHeader->type, (char*)(pHeader+1), pHeader->len)) {
				nByteReceived = 0;
				_LOG(TEXT("[parsePacket] buffer is broken. cleared\n"));
				break;
			}
			int packetSize = pHeader->len + sizeof(packetHeader_t);
			memmove(recvBuffer, recvBuffer + packetSize, 65536 - packetSize);
			nByteReceived -= packetSize;
		}
		return true;
	}

	virtual void recv(void) {
		int nByteReceived = ::recv(sd, recvBuffer, 65536, 0);
		if(nByteReceived>0) {
			parsePacket(nByteReceived);
		}
		else {
			// disconnect
		}
	}

	bool onPacketComplete(int type, char* payload, int lenPayload) {
		switch(type) {
			case pdAckPeerList2: {
				//log("[chsNtImpl_t::onPacketComplete] pdAckPeerList2\n");
				int nItem = lenPayload / sizeof(payloadPeerInfo_t);
				if(nItem<MaxUser) {					
					ppayloadPeerInfo_t ppi = (ppayloadPeerInfo_t)payload;
					EnterCriticalSection(&csInfo);
					for(int lpp=0;lpp<nItem;lpp++) {
						playerInfo[lpp].idAccount = ppi->idAcc;
						memset(&playerInfo[lpp].addrLocal, 0, sizeof(SOCKADDR));
						playerInfo[lpp].addrLocal.sin_family = AF_INET;
						playerInfo[lpp].addrLocal.sin_addr.S_un.S_addr = ppi->localAddr;
						playerInfo[lpp].addrLocal.sin_port = ppi->localPort;
						memset(&playerInfo[lpp].addrPublic, 0, sizeof(SOCKADDR));
						playerInfo[lpp].addrPublic.sin_family = AF_INET;
						playerInfo[lpp].addrPublic.sin_addr.S_un.S_addr = ppi->publicAddr;
						playerInfo[lpp].addrPublic.sin_port = ppi->publicPort;
						ppi++;
					}
					LeaveCriticalSection(&csInfo);
					nPlayers = nItem;
					SetEvent(hEventRecvPeerList);
				}
			} break;

			case pdNakPeerList2: {
				//log("[chsNtImpl_t::onPacketComplete] pdNakPeerList2\n");
				nPlayers = 0;				
				SetEvent(hEventRecvPeerList);
			} break;

			case pdAckConnect: {
				//log("[chsNtImpl_t::onPacketComplete] pdAckConnect\n");
				ppayloadPeerInfo_t ppi = (ppayloadPeerInfo_t)payload;
				if(pParent && ppi) {					
					SOCKADDR_IN addrLocal;
					addrLocal.sin_family = AF_INET;
					addrLocal.sin_addr.S_un.S_addr = ppi->localAddr;
					addrLocal.sin_port = ppi->localPort;

					SOCKADDR_IN addrPublic;
					addrPublic.sin_family = AF_INET;
					addrPublic.sin_addr.S_un.S_addr = ppi->publicAddr;
					addrPublic.sin_port = ppi->publicPort;

					pParent->onAckP2pConnect(ppi->idAcc, &addrLocal, &addrPublic);
				}
				bAckConnect = true;
			} break;

			case pdNtfConnect: {
				//log("[chsNtImpl_t::onPacketComplete] pdNtfConnect\n");
				ppayloadPeerInfo_t ppi = (ppayloadPeerInfo_t)payload;
				if(pParent && ppi) {					
					SOCKADDR_IN addrLocal;
					addrLocal.sin_family = AF_INET;
					addrLocal.sin_addr.S_un.S_addr = ppi->localAddr;
					addrLocal.sin_port = ppi->localPort;

					SOCKADDR_IN addrPublic;
					addrPublic.sin_family = AF_INET;
					addrPublic.sin_addr.S_un.S_addr = ppi->publicAddr;
					addrPublic.sin_port = ppi->publicPort;

					pParent->onNtfP2pConnect(ppi->idAcc, &addrLocal, &addrPublic);
				}				
			} break;
			default: {
				_LOG(TEXT("[chsNtImpl_t::onPacketComplete] unknown packet type %d\n"), type);
				return true;
			}
		}	
		return false;
	}	

	virtual bool connect(char* ipAddr, int port) {		
		if(sd!=INVALID_SOCKET) {
			closesocket(sd);
			sd = INVALID_SOCKET;
		}

		sd = socket(AF_INET, SOCK_STREAM, 0);
		if(sd == INVALID_SOCKET) 
			goto cleanup;
		
		SOCKADDR_IN addr;
		memset (&addr, '\0', sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(ipAddr);
		addr.sin_port = htons(port);
		int err = ::connect(sd, (struct sockaddr *)&addr, sizeof(addr)); 
		if(err) 
			goto cleanup;
		return true;

	cleanup:
		if(sd!=INVALID_SOCKET) {
			closesocket(sd);
			sd= INVALID_SOCKET;
		}
		return false;
	}

	virtual void disconnect(void) {
		closesocket(sd);
	}

	virtual void onTick(void) {
		DWORD timeCurrent = timeGetTime();
		for(int lpp=0;lpp<nPlayers;lpp++) {
			if(playerInfo[lpp].connInfo.state != connInfo_t::csIdle && playerInfo[lpp].connInfo.timeBeginRtt + 5000 < timeCurrent) {
				_LOG(TEXT("RTT probing timeout for %d(st:%d:%d)<-------------\n"), playerInfo[lpp].idAccount, playerInfo[lpp].connInfo.state, playerInfo[lpp].connInfo.timeBeginRtt);
				playerInfo[lpp].connInfo.state = connInfo_t::csIdle;
				playerInfo[lpp].connInfo.bCanceled = true;
				endUpdate(playerInfo[lpp].idAccount);
			}
		}
	}

	chsNtImpl_t(pIp2pNetTester_t _pParent=0x0) : Ichs_t(_pParent) {
		nByteReceived = 0;
		
		//WORD wVersionRequested = MAKEWORD( 2, 2 );
		//WSADATA wsaData;
		//int err;err = WSAStartup( wVersionRequested, &wsaData );
	}

	~chsNtImpl_t(void) {
		//WSACleanup();
	}
};
typedef struct chsNtImpl_t chsNtImpl_t;
typedef struct chsNtImpl_t* pchsNtImpl_t;
//---------------------------------------------------------------------------
#endif
