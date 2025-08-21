#pragma once

#ifdef EnableRttTest

#include "p2pConn.h"
// {{ 20070410 dEAthcURe|HP RTT test
#include "p2pNtImpl.h"
//#include "chsNtImpl.h"
#include "Ichs.h"
#include "p2pNetTester.h"
// }} 20070410 dEAthcURe|HP RTT test


struct CavaChsImpl : public Ichs_t
{
	struct FTestResultItem
	{
		SLIST_ENTRY itemEntry;
		DWORD idToTest;
		Ichs_t::playerInfo_t playerInfo;
	};

	struct FRttResultInfo
	{
		FLOAT RttValue;
		DOUBLE CheckTime;
	};

	SLIST_HEADER testResultList;

	DWORD RttRecheckThreshold;
	FLOAT RttRecheckTime;

	TMap<DWORD, FRttResultInfo> CachedInfoList;

	virtual int getIdAccount(int idx);
	virtual pplayerInfo_t getPlayerInfo(int idAccount);
	virtual bool send_ntfMyAddr(int idAccount, SOCKADDR_IN addrLocal, SOCKADDR_IN addrPublic);
	virtual bool send_reqPeerList(int idAccRequester, int idChannel, int idRoom) { return true; }
	virtual int recv_ansPeerList(DWORD ms) { return 1/*TestList.Num()*/; }
	virtual bool send_reqConnect(int idAccRequester, int idAccPeer);
	virtual bool recv_ansConnect(int sec) { return true; }
	virtual void recv(void) {}

	virtual bool connect(char* ipAddr, int port);
	virtual void disconnect(void) {}

	virtual void procUpdate(int idAccount);
	virtual void endUpdate(int idAccount, Ichs_t::pplayerInfo_t ppi);

	virtual void onSessionDisconnected(int idAccount);

	virtual void onTick(void);
	virtual void onConnectionFailed(void);

	FLOAT CheckAndGetRttValue(DWORD idAccount);

	void ProcTestResults();
	void EmptyTestResultList();

	CavaChsImpl(pIp2pNetTester_t _pParent);
	virtual ~CavaChsImpl(void);
};

#endif
