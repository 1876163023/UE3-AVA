//
// Ip2p.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef Ip2pH
#define Ip2pH
//---------------------------------------------------------------------------
#include "Ip2pNetTester.h"
#include <winsock2.h>
//---------------------------------------------------------------------------
//extern void log(char* fmt, ...);
//---------------------------------------------------------------------------
struct Ip2p_t {	
	int localPort;
	pIp2pNetTester_t pParent;
	SOCKET sd;
	SOCKADDR_IN addrLocal;
	SOCKADDR_IN addrPublic;
	DWORD timeServerConnect;
	DWORD timeGetAddr;

	HANDLE hEventP2pSvrConnected;
	HANDLE hEventP2pSvrAddr;

	virtual bool connectToServer(char* ipAddr, int port, int idAccount) {
		ResetEvent(hEventP2pSvrConnected);
		return false; // 실제 send구현을 안했으므로 항상 false 리턴.
	}

	DWORD timeIntervalHeartbeatToServer;
	virtual bool heartbeatToServer(void)=0;
	virtual bool send_rttProb(int idAccount)=0;
	virtual void recv(void)=0;
	virtual bool registerPeer(int idAccount, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic)=0;	
	virtual bool requestConnectPeer(int idAccount)=0;
	virtual void removeAllPeer(void)=0;
	virtual void removePeer(int idPeer)=0;
	virtual bool disconnect(void)=0;

	virtual bool init(int localPort) {return false;}
	virtual void deinit(void) {}	

	Ip2p_t(pIp2pNetTester_t _pParent) {
		localPort = 8799;
		timeServerConnect = 5000;
		timeGetAddr = 2000;
		timeIntervalHeartbeatToServer = 30000;
		pParent = _pParent;		
		sd = INVALID_SOCKET;				
		hEventP2pSvrConnected = CreateEvent(0x0, true, false, TEXT("hEventP2pSvrConnected"));
		hEventP2pSvrAddr = CreateEvent(0x0, true, false, TEXT("hEventP2pSvrAddr"));		
	}

	virtual ~Ip2p_t(void) {
		CloseHandle(hEventP2pSvrConnected);
		CloseHandle(hEventP2pSvrAddr);
	}
};
typedef struct Ip2p_t Ip2p_t;
typedef struct Ip2p_t* pIp2p_t;
//---------------------------------------------------------------------------
#endif