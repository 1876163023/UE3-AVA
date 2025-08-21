/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetClient.h

	Description: Declarations of CavaNetClient; main avaNet network system class

***/
#ifndef __AVANETCLIENT_H__
#define __AVANETCLIENT_H__




#include "../../IpDrv/Inc/UnIpDrv.h"
#include "RxGateTranslator/RxGateTranslator.h"


#define _LH_	_T("[") _T(__FUNCTION__) _T("]")

#if NO_LOGGING || FINAL_RELEASE

	#define _LOG	__noop

#else

	#if SUPPORTS_VARIADIC_MACROS

		#define _LOG(...)	GavaNetClient->Logf(_LH_, __VA_ARGS__)

	#elif _MSC_VER >= 1300

		#define _LOG(_s,a,b,c,d,e,f,g,h)	GavaNetClient->Logf(_LH_, _s, VARG(a), VARG(b), VARG(c), VARG(d), VARG(e), VARG(f), VARG(g), VARG(h))

	#else

		#define _LOG	GLog->Logf

	#endif

#endif

#define _DUMP	_LOG

#if SUPPORTS_VARIADIC_MACROS

	#define _CLOG(...)	GavaNetClient->LogConsole(TRUE, __VA_ARGS__)
	#define _CNLOG(...)	GavaNetClient->LogConsole(FALSE, __VA_ARGS__)

#elif _MSC_VER >= 1300

	#define _CLOG(_s,a,b,c,d,e,f,g,h)	GavaNetClient->LogConsole(TRUE, _s, VARG(a), VARG(b), VARG(c), VARG(d), VARG(e), VARG(f), VARG(g), VARG(h))
	#define _CNLOG(_s,a,b,c,d,e,f,g,h)	GavaNetClient->LogConsole(FALSE, _s, VARG(a), VARG(b), VARG(c), VARG(d), VARG(e), VARG(f), VARG(g), VARG(h))

#else

	#define _CLOG	_LOG
	#define _CNLOG	_LOG

#endif



enum EavaNetClientState
{
	CLS_None,
	CLS_Connecting,
	CLS_CreatingSession,
	CLS_Authenticating,
	CLS_Connected,
	CLS_ChangingSession,
};


class CavaConnection;
class IavaNetEventHandlerInterface;
class CavaNetStateController;

struct hwPerformanceCounterClient_t;
struct FPendingMsg;


#define EnableP2pConn // 20060929 dEAthcURe

#ifdef EnableP2pConn

#include "p2pConn.h"

struct p2pConn_t;
class CavaP2PHandler;

#define EnableRttTest

#endif

#ifdef EnableRttTest

#include "p2pNtImpl.h"
#include "Ichs.h"
#include "p2pNetTester.h"
#include "p2pNetTesterImpl.h"

#endif


class CavaNetClient : public FTickableObject, public FExec, public Ip2pConn_t // 20070413 nonblock TEST!! // class CavaNetClient : public FTickableObject, public FExec
{
private:
    IavaNetEventHandlerInterface* EventHandler;
	CavaConnection *CurrConn;			// current connection
	FOutputDevice *Output;				// console logger; used for admin(GM) commands

	FLOAT CheckDelta;

	DWORD ExitCode;		// set by CloseConnection(); if ExitCode == EXIT_GAME_END or EXIT_SERVER_SIDE_EXIT, MsgGameCloseNtf is not sent when application exits.

public:
	UBOOL bStarted;

	float timeHostBegin; // [+] 20070531 dEAthcURe|HM

	RxGate::RXNERVE_ADDRESS CurrentChannelAddress;		// normal channel's M address; "CHM" + channel id
	RxGate::RXNERVE_ADDRESS CurrentGuildAddress;		// clan channel's M address; "CLAN" + clan id
	RxGate::RXNERVE_ADDRESS CurrentClientAddress;		// this client's M address; "AVAC" + usn
	WORD SessionKeyChannel;				// session key to the normal channel (it must be always valid)
	WORD SessionKeyGuild;				// session key to the clan channel (it should be valid if the player is member of a clan
	DWORD ClientKey;					// client key
	DWORD ClientIP;						// client ip

	TArray<FString> P2PServers;			// available p2p server list
	TArray<FString> RtttServers;		// available rttt server list

	EavaNetClientState CLState;

	TMap<FString, FString> Settings;
	CavaNetStateController *StateController;

	//UBOOL bPendingConnection;
	TArray<FString> PendingConnections;	// gate ip address list; select a ip from this list randomly until connection try succeeds
	TArray<INT> PendingChannels;		// initial channel id list; select a channel from this list randomly until initial connection try succeeds

	UBOOL bForceConnect;				// checked for second connection try, which would kick already connected player whose usn is same as this player

	TDynamicMap<WORD, FPendingMsg> PendingMsgs;			// saved request message list; checked for time-out

	DOUBLE ConnectionTimeOutDue;						// time-out due for the initial connection try

	hwPerformanceCounterClient_t *PerformanceCounter;	// hardware information storage; h/w rating calculator is removed from the client (server only)

	FLOAT TimeKeepAliveCheck;			// time to send keep-alive message to the gate

	DOUBLE TimePlayQuota;				// 1-week play quota
	FLOAT TimeToDieTick;				// turned on when connection to the server is closed; time to close the client
	INT TimeToDieCount;					// turned on when connection to the server is closed; count down to close the client
	FLOAT TimeGracefulQuit;				// turned on when "NET QUIT" request is sent to server;
										// if the server does not close connection before this time, the client close the connection forcefully

	DOUBLE EmergencyCheckTime;			// used for the emergency situation handling

private:
	void InitializeSettings();
	void FinalizeSettings();
	void Start();

	void ProcKeepAliveCheck();
	void ProcTimeToDie();
	void ProcPendingConnections();
	void ProcTimeOutPendingMsgs();
	void ProcEmergencyState();

public:
	CavaNetClient();
	virtual ~CavaNetClient();

	void SetEventHandler(IavaNetEventHandlerInterface *Handler)
	{
		EventHandler = Handler;
	}
	IavaNetEventHandlerInterface* GetEventHandler()
	{
		return EventHandler;
	}

	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar = *GLog);

	/**
	 * Tells the tickable object system this object is ok to tick
	 */
	virtual UBOOL IsTickable(void)
	{
		return TRUE;
	}

	/**
	 * Used to determine if an object should be ticked when the game is paused.
	 *
	 * @return always TRUE as networking needs to be ticked even when paused
	 */
	virtual UBOOL IsTickableWhenPaused(void)
	{
		return TRUE;
	}

	virtual void Tick(FLOAT DeltaTime);

	VARARG_DECL(void, void, {}, Logf, VARARG_NONE, const TCHAR*, VARARG_EXTRA(const TCHAR *Header), VARARG_EXTRA(Header));
	VARARG_DECL(void, void, {}, LogConsole, VARARG_NONE, const TCHAR*, VARARG_EXTRA(UBOOL bPrefix), VARARG_EXTRA(bPrefix));

	UBOOL AutoConnect();
	UBOOL OpenConnection(FInternetIpAddr &addr);
	UBOOL OpenConnection(const TCHAR *addr);
	void CloseConnection(DWORD ExitCode);
	void DestroyConnection();
	CavaConnection* CurrentConnection()
	{
		return CurrConn;
	}

	void Quit(UBOOL bGraceful = TRUE);

	UBOOL CreateRxGateSession(RxGate::RXNERVE_ADDRESS &addr);

	void StartConnectionTimeOutCheck();
	void EndConnectionTimeOutCheck();
	void ProcConnectionTimeOutDue();

	UBOOL IsValid()
	{
		return CurrConn != NULL;
	}

	UBOOL ParseAdminCommand(const TCHAR *Cmd, FOutputDevice& Ar = *GLog);

	static void CreateNew();
	static void Delete();

public:
	static DOUBLE TimeOutSec;
	static DOUBLE ConnectionTimeOutSec;

#ifdef EnableP2pConn
	virtual void onP2pNotification(DWORD nParam1, DWORD nParam2);
	void SelectP2PServer(INT Index);

	FURL ConnectURL;
	SOCKADDR_IN addrLocal;
	SOCKADDR_IN addrPublic;
	p2pConn_t *pp2pConn; // dEAthcURe
	CavaP2PHandler *pP2PHandler;

	SOCKADDR HostAddrLocal;
	SOCKADDR HostAddrPublic;

	INT P2PLocalPort;

	// {{ 20070410 dEAthcURe|HP RTT test
#ifdef EnableRttTest
	p2pNtImpl_t p2pNtImpl;
	//chsNtImpl_t chsNtImpl;
	CavaChsImpl chsImpl;
	p2pNetTester_t p2pNetTester;
#endif
	// }} 20070410 dEAthcURe|HP RTT test
#endif

	// for RTT tester error handling
	volatile UBOOL bP2pSessionDisconnected;
	volatile UBOOL bInvalidPing;

public:
	// MT flags
	void ProcMTFlags();

	// Rtt test helper
	void RtttInit();
	void RtttSetActive(DWORD idAccount);
};


extern CavaNetClient *GavaNetClient;


struct FPendingMsg
{
	BYTE *pMsg;
	//DOUBLE SentTime;
	DOUBLE CheckTime;

	FPendingMsg(DOUBLE TimeOutDelta = CavaNetClient::TimeOutSec) : pMsg(NULL)
	{
		SetCheckTime(TimeOutDelta);
	}
	FPendingMsg(BYTE *MsgBuf, DOUBLE TimeOutDelta = CavaNetClient::TimeOutSec) : pMsg(MsgBuf)
	{
		SetCheckTime(TimeOutDelta);
	}

	void SetCheckTime(DOUBLE TimeOutDelta)
	{
		CheckTime = appSeconds() + TimeOutDelta;
	}
	void Dump();
};




class UavaNetHandler;
class UavaNetRequest;

UavaNetHandler* GetAvaNetHandler();
UavaNetRequest* GetAvaNetRequest();

void _DumpMemory(BYTE *buf, INT len);



#endif
