//
// Ichs.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef IchsH
#define IchsH
//---------------------------------------------------------------------------
#include "Ip2pNetTester.h"
#include <winsock2.h>
//---------------------------------------------------------------------------
struct Ichs_t {
	pIp2pNetTester_t pParent;

	SOCKET sd;
	HANDLE hEventRecvPeerList;
	CRITICAL_SECTION csInfo;

	struct connInfo_t {

		enum state_t {
			csIdle,
			csReserved,
			csStarting,
			csActive,			
		} state;
		
		bool bTestRtt;
		DWORD timeMinRtt;
		DWORD timeMaxRtt;
		DWORD timeSumRtt;
		DWORD timeAvgRtt;	
		DWORD timeBeginRtt;
		DWORD timeBeginProb;
		DWORD timeEndProb;
		int nSuccess, nFail;
		bool bCanceled;

		void initTest(void) {		
			timeBeginRtt = timeGetTime();
			timeMinRtt = 99999999;
			timeMaxRtt = 0;
			timeSumRtt = 0;
			timeAvgRtt = 0;
			timeBeginProb = 0;
			timeEndProb = 0;
			nSuccess = nFail = 0;
			bCanceled = false;			
		}

		void init(void) {
			state = csIdle; // [!] 20070509 contructor¿¡¼­ ÀÌ»ç¿È
			initTest();
		}

		connInfo_t(void) {			
			init();			
		}		
	};
	typedef struct connInfo_t connInfo_t;
	typedef struct connInfo_t* pconnInfo_t;

	struct playerInfo_t {
		int idAccount;
		SOCKADDR_IN addrLocal;
		SOCKADDR_IN addrPublic;

		connInfo_t connInfo;
		connInfo_t connInfoLastSuccessful;

		void init(void) {
			idAccount = -1;
			memset(&addrLocal, 0, sizeof(addrLocal));
			memset(&addrPublic, 0, sizeof(addrPublic));		

			connInfo.init();
			connInfoLastSuccessful.init();
		}

		inline void endUpdate(void) {
			connInfoLastSuccessful = connInfo;			
		}

		inline void getConnectionInfo(connInfo_t& out) {			
			out = connInfoLastSuccessful;			
		}

		playerInfo_t(void) {			
			init();
		}
	};
	typedef struct playerInfo_t playerInfo_t;
	typedef struct playerInfo_t* pplayerInfo_t;
	int nPlayers;

	virtual int getIdAccount(int idx)=0;
	virtual pplayerInfo_t getPlayerInfo(int idAccount)=0;
	virtual bool send_ntfMyAddr(int idAccount, SOCKADDR_IN addrLocal, SOCKADDR_IN addrPublic)=0;	
	virtual bool send_reqPeerList(int idAccRequester, int idChannel, int idRoom)=0;
	virtual int recv_ansPeerList(DWORD ms) {
		if(WaitForSingleObject(hEventRecvPeerList, ms)==WAIT_OBJECT_0) {
			return nPlayers;
		}
		return -1;
	}
	virtual bool send_reqConnect(int idAccRequester, int idAccPeer)=0;
	virtual bool recv_ansConnect(int sec)=0;	
	virtual void recv(void)=0;

	virtual bool connect(char* ipAddr, int port) {return false;}
	virtual void disconnect(void) {}

	virtual void procUpdate(int idAccount)=0;
	virtual void endUpdate(int idAccount, pplayerInfo_t ppi)=0;

	virtual void onSessionDisconnected(int idPeer) {}

	virtual void onTick(void) {}
	virtual void onConnectionFailed(void) {}
	virtual bool IsTestIdle(void) {return false;}

	Ichs_t(pIp2pNetTester_t _pParent) {
		pParent = _pParent;
		sd = INVALID_SOCKET;
		nPlayers = 0;
		hEventRecvPeerList = CreateEvent(0x0, true, false, TEXT("hEventRecvPeerList"));
		InitializeCriticalSection(&csInfo);
	}

	virtual ~Ichs_t(void) {
		CloseHandle(hEventRecvPeerList);
		DeleteCriticalSection(&csInfo);
	}
};
typedef struct Ichs_t Ichs_t;
typedef struct Ichs_t* pIchs_t;
//---------------------------------------------------------------------------
#endif