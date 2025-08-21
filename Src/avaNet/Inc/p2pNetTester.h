//
// p2pNetTester.h
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#ifndef p2pNetTesterH
#define p2pNetTesterH
//---------------------------------------------------------------------------
#include "Ip2p.h"
#include "Ichs.h"

#include <process.h>
//---------------------------------------------------------------------------
struct p2pNetTester_t : Ip2pNetTester_t {

	struct p2pTestItem_t {
		SLIST_ENTRY itemEntry;
		DWORD idToTest;
		SOCKADDR_IN addrLocal;
		SOCKADDR_IN addrPublic;

		p2pTestItem_t() : idToTest(0) {}
	};
	typedef struct p2pTestItem_t *pp2pTestItem_t;

	enum threadState_t {
		tsNone,
		tsCreated,
		tsActive,
		tsTerminating,
		tsTerminated,		
		tsEndOfType
	};

	enum testState_t {
		testNone,
		testInit,
		testStart,
		testProc,
		testStop,
	};

	volatile bool bKeepServerConnection; // [+] 20070627
	//volatile bool bTestRtt;
	//ini_t ini;
	pIp2p_t pP2p;
	pIchs_t pChs;

	char p2psIpAddr[16];
	int p2psPort;

	int nTrial;
	int nFailToCancel;

	DWORD idAccount;
	volatile threadState_t hostThreadState, clientThreadState;
	uintptr_t hHostThread, hRecvThread;
	HANDLE hEventChangeHostThreadState, hEventChangeClientThreadState;
	HANDLE hEventHostThreadTerminated, hEventClientThreadTerminated;
	DWORD timeSleep;
	DWORD timeThreadControl;
	DWORD startDelay;

	volatile testState_t hostTestState;
	SLIST_HEADER toTestList, hostTestList, clientTestList;
	TMap<DWORD,Ichs_t::playerInfo_t> testingItemList;

	void hostThreadFunc(void);

	static void hostThreadProc(void* lpParam) {		
		struct p2pNetTester_t* pParent = (struct p2pNetTester_t*)lpParam;
		
		//__try { // [!] 20070529
			if(pParent) pParent->hostThreadFunc();
		/* // [!] 20070529
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {			
			//log("[hostThreadProc] exception has occured %d\n", GetExceptionCode());
		}
		*/
		_endthread();
	}

	inline void setHostThreadState(threadState_t state, bool bSetEvent = true) {
		if(hostThreadState != state) {
			hostThreadState = state;
			if(bSetEvent) SetEvent(hEventChangeHostThreadState);
		}
	}

	inline bool getHostThreadStateChanging(threadState_t& state, DWORD ms) {
		if(WaitForSingleObject(hEventChangeHostThreadState, ms) == WAIT_OBJECT_0) {
			state = hostThreadState;
			ResetEvent(hEventChangeHostThreadState);
			return true;
		}
		return false;
	}

	inline DWORD _hostThread_WaitForEvent(HANDLE hEvent, DWORD ms) {
		HANDLE handles[2] = {hEventChangeHostThreadState, hEvent};
		return WaitForMultipleObjects(2, handles, false, ms);
	}

	void recvThreadFunc(void);

	static void recvThreadProc(void* lpParam) {
		struct p2pNetTester_t* pParent = (struct p2pNetTester_t*)lpParam;
		//__try { // [!] 20070529
			if(pParent) pParent->recvThreadFunc();
		/* // [!] 20070529
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
			//log("[recvThreadProc] exception has occured %d\n", GetExceptionCode());
		}
		*/
		
		_endthread();
	}

	void setRecvThreadState(threadState_t state) {
		if(clientThreadState != state) {
			clientThreadState = state;
			SetEvent(hEventChangeClientThreadState);
		}
	}

	bool setActive(bool bActive = true, bool bBlocking = true, DWORD timeout = INFINITE);
	void activateRttTest(bool bActive = true, bool bKeepServerConnection = false, DWORD startDelay = 0);
	void initRttTest();

	void dispatch();

	void addToTestList(DWORD idToTest);
	void emptyToTestList();
	void emptyHostTestList();
	void emptyClientTestList();

	virtual void onAckP2pConnect(DWORD idPeer, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic);
	virtual void onNtfP2pConnect(DWORD idRequester, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic);	
	virtual void onSessionEstablished(DWORD idPeer);
	virtual void onSessionDisconnected(DWORD idPeer);
	virtual void onRttProbAck(DWORD idPeer);

	bool init(pIp2p_t pP2p, pIchs_t pChs);

	void procOnAck();
	void procOnNtf();

	p2pNetTester_t(void);
	~p2pNetTester_t(void);
};
typedef struct p2pNetTester_t p2pNetTester_t;
typedef struct p2pNetTester_t* pp2pNetTester_t;
//---------------------------------------------------------------------------
#endif

