//
// p2pConn.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef p2pConnH
#define p2pConnH
//---------------------------------------------------------------------------
#include "bro_common/P2PClientEx.h"
#include "bro_common/PeerInfoMgr.h"
#include "time.h"

//#include <list>
//using namespace std;
//---------------------------------------------------------------------------

class CP2PClientEx;

#define DefaultGmsIpAddr	"10.20.16.90"
#define DefaultGmsPort		62000

#define DefaultP2psIpAddr	"203.215.208.51"
#define DefaultP2psPort		28020

#define HeartbeatTimeout 20

// {{ 20070410
struct Ip2pConn_t {
	virtual void processP2pPacket(DWORD nParam1, DWORD nParam2) {}
	virtual void onLossP2pPacket(DWORD nParam1, DWORD nParam2) {}
	virtual void onP2pNotification(DWORD nParam1, DWORD nParam2) {}
};
typedef struct Ip2pConn_t Ip2pConn_t;
typedef struct Ip2pConn_t* pIp2pConn_t;
// }} 20070410

struct p2pConn_t : CP2PClientEx {
	volatile bool bWaitForConnect;
	volatile bool bWaitForSvrAddr;
	pIp2pConn_t pParent; // [+] 20070410

	volatile bool bConnected;
	SOCKET gmsSd;
	char gmsIpAddr[256];
	int gmsPort;
	char sendBuffer[65536];
	char recvBuffer[65536];	
	char p2pBuffer[65536];
	int id;

	float timeNextHeartbeat;
	float intervalHeartbeat;

	// {{ 20070221 새 server에 맞추기 위해 server로 heartbeat
	float timeNextHeartbeatToServer;
	float intervalHeartbeatToServer;
	// }} 20070221 새 server에 맞추기 위해 server로 heartbeat

	SOCKET p2psSd;
	char p2psIpAddr[256];
	int p2psPort;
	int idHost;	
	TArray<int> peers;

	static void setP2pServerOnStartup(char* ipAddr, int port);
	void setP2pServer(char* ipAddr, int port, bool bFromIni = false);

	void (*_cb_log)(char* string);
	
	sockaddr mySockAddr;
	sockaddr hostSockAddr;
	bool waitForConnect(DWORD milisec = 30000);
	bool getPeerAddress(int idPeer, DWORD milisec, sockaddr* addr);	
	//bool bConnected;
	bool getHostAddress(DWORD milisec, sockaddr* pAddr = 0x0);	
	bool requestConnectToHost(DWORD milisec, SOCKADDR* addr);
	bool getMyAddress(DWORD milisec, sockaddr* pAddr = 0x0);
	bool getMyAddress(DWORD milisec, sockaddr* pAddrLocal, sockaddr* pAddrPublic);
	bool getPeerAddress(int id, SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic);
	bool setPeerAddress(int id, SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic, SOCKADDR* addrAgainstI = 0x0);
	SOCKADDR hostAddrLocal, hostAddrPublic;
	bool bHostAddrPended;
	bool setHostAddress(SOCKADDR* pAddrLocal, SOCKADDR* pAddrPublic);

	#ifdef P2pConnNativeGms
	virtual int gmsRecv(int nSec);
	virtual bool gmsBeHost(void);
	virtual bool gmsGetHost(void);
	#ifdef EnableUserList
	virtual bool gmsUserList(void);
	#endif
	virtual bool gmsJoin(void);
	virtual int gmsGetId(void);
	virtual bool gmsConnect(char* ipAddr = DefaultGmsIpAddr, int port = DefaultGmsPort);
	virtual bool gmsDisconnect(void);
	#endif

	virtual int processP2pPacket(DWORD nParam1, DWORD nParam2);
	virtual int onLossP2pPacket(DWORD nParam1, DWORD nParam2);
	virtual int onP2pNotification(DWORD nParam1, DWORD nParam2);

	STRUCT_PEER_INFO* findPeerByAddr(sockaddr* pAddr); // [+] 20070130
	void timeoutHeartbeat(void); // [+] 20070130
	bool onRecvFrom(char* buffer, int nByte, sockaddr* from, int fromLen);
	bool onRecv(char* buffer, int nByte);
	void tick(bool bRecvFrom = false);

	void setHost(int idHost);
	bool setHost(int idHost, SOCKADDR* pAddr); // 20070224

	bool initSocket(void);
	bool initUdp(SOCKET sd = INVALID_SOCKET, int localPort = 7777);
	bool initUdp(char* ipAddr, int port, SOCKET sd = INVALID_SOCKET, int localPort = 7777);	// [!] 20070410 // bool initUdp(char* ipAddr, int port, SOCKET sd = INVALID_SOCKET);	
	void setId(int _id) {id = _id;}

	bool init(void); // [+] 20070410
	void deinit(void);

	p2pConn_t(pIp2pConn_t pParent = 0x0, bool bGlobal = true, int localPort = 5001, int nMaxPeer = 32, int retransmitInterval = 10000); // [!] 20070410 //p2pConn_t(int localPort = 5001, int nMaxPeer = 16, int retransmitInterval = 10000);
	~p2pConn_t(void);
};
typedef struct p2pConn_t p2pConn_t;
typedef struct p2pConn_t* pp2pConn_t;
//---------------------------------------------------------------------------
extern char _gini_ipAddr[256];
extern int _gini_port;
//---------------------------------------------------------------------------
#endif
