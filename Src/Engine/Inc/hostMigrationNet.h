//
// hostMigrationNet.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifdef UseNativeServer
//---------------------------------------------------------------------------
#ifndef hostMigrationClientH
#define hostMigrationClientH
//---------------------------------------------------------------------------
#include <winsock.h>

#include <list>
#include <string>
using namespace std;
//---------------------------------------------------------------------------
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
};
//---------------------------------------------------------------------------
#define LOG(a) if(log) log(a)
//---------------------------------------------------------------------------
struct hmPeerInfo_t {
	int id;
	unsigned short port;
	char ipAddr[32];
};
typedef struct hmPeerInfo_t hmPeerInfo_t;
typedef struct hmPeerInfo_t* phmPeerInfo_t;
//---------------------------------------------------------------------------
struct hmNetCallback_t {
	virtual void onPacketComplete(struct hostMigrationNet_t* pSender, int type, char* payload)=0;
};
typedef struct hmNetCallback_t hmNetCallback_t;
typedef struct hmNetCallback_t* phmNetCallback_t;
//---------------------------------------------------------------------------
struct hostMigrationNet_t {
	list<hmPeerInfo_t> peers;
	unsigned short udpPort;

	float avgRtt;

	int sdUdpServer;
	int sdUdpClient;

	int nByteReceived;
	SOCKET sdTcp;
	int id;
	int idHost;
	int port;
	wstring cpuId;
	int memSizeMB;

	char sendBuffer[65536];
	char recvBuffer[65536];
	char payload[1024];

	bool sendPacket(int type, char* payload = 0x0, int len = 0);
	bool parsePacket(int nByteIncomming);
	virtual void onPacketComplete(int type, char* optionString);

	void (*log)(char* format, ...);

	int recv(int nSec);
	void onTickTcp(void);

	bool reqPeerList(void);
	bool reqCreateRoom(void);
	bool reqJoin(void);
	bool reqNewHost(void);
	bool ntfLinkLost(void);
	bool ntfHardwareInfo(void);
	bool ntfNetInfo(void);
	bool ntfBeNewHost(void);

	bool connect(char* ipAddr, int port);
	bool disconnect(void);

	bool getMyIpAddress(char *address);
	wstring scanFromReg(wstring path, TCHAR* name);

	void onTickUdpEchoServer(void);
	bool initUdpEchoServer(unsigned short port);
	float testRtt(char* addr, int port, int nTrial);
	bool testNet(int nTrial = 10);
	void onTick(void);

	phmNetCallback_t pCallback;
	void setCallback(phmNetCallback_t pCb);

	hostMigrationNet_t(void);
	~hostMigrationNet_t(void);
};
typedef struct hostMigrationNet_t hostMigrationNet_t;
typedef struct hostMigrationNet_t* phostMigrationNet_t;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#endif