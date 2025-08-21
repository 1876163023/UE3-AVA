//
// p2pNetTester.cpp
//
// deathcure@redduck.com
//---------------------------------------------------------------------------
#include "avaNet.h"
#include "p2pNetTester.h"

using namespace Def;

//---------------------------------------------------------------------------
//extern void log(char* fmt, ...);
//
//void log(char* fmt, ...)
//{
//	return;
//	// implement this
//	va_list argPtr;
//	va_start( argPtr, fmt );		
//	char str[1024];
//	_vsnprintf( str, 1024, fmt, argPtr );
//
//	FILE* fp = fopen("p2pNetTester.log", "at");
//	if(fp) {
//		fputs(str, fp);
//		fclose(fp);
//	}	
//	//printf(str);	
//	va_end( argPtr );
//}
//---------------------------------------------------------------------------
// p2pNetTester_t
//---------------------------------------------------------------------------
void p2pNetTester_t::onAckP2pConnect(Def::TID_ACCOUNT idPeer, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic)
{
	// called from main thread

	//if (hostTestState != testProc)
	//{
	//	debugf(TEXT("[p2pNetTester_t::onAckP2pConnect] Invalid test state(%d)"), (INT)hostTestState);
	//	return;
	//}

#if !FINAL_RELEASE
	TCHAR wAddrLocal[256];
	TCHAR wAddrPublic[256];
	mbstowcs(wAddrLocal, inet_ntoa(addrLocal->sin_addr), 255);
	mbstowcs(wAddrPublic, inet_ntoa(addrPublic->sin_addr), 255);
	_LOG(TEXT("id=%d Ladr=%s:%d Padr=%s:%d"), idPeer, wAddrLocal, ntohs(addrLocal->sin_port), wAddrPublic, ntohs(addrPublic->sin_port));
	//log(">onAckP2pConnect> id=%d Ladr=%s:%d Padr=%s:%d\n", idPeer, inet_ntoa(addrLocal->sin_addr), ntohs(addrLocal->sin_port), inet_ntoa(addrPublic->sin_addr), ntohs(addrPublic->sin_port));
#endif

	if(addrLocal->sin_addr.S_un.S_addr == 0x0 || addrPublic->sin_addr.S_un.S_addr == 0x0) {
		_LOG(TEXT("Invalid address for %d"), idPeer);
		return;
	}

	//idToTest = idPeer;
	//appMemcpy(&addrToTestLocal, addrLocal, sizeof(SOCKADDR_IN));
	//appMemcpy(&addrToTestPublic, addrPublic, sizeof(SOCKADDR_IN));

	pp2pTestItem_t pItem = new p2pTestItem_t();
	check(pItem);

	pItem->idToTest = idPeer;
	appMemcpy(&pItem->addrLocal, addrLocal, sizeof(SOCKADDR_IN));
	appMemcpy(&pItem->addrPublic, addrPublic, sizeof(SOCKADDR_IN));

	::InterlockedPushEntrySList(&clientTestList, &pItem->itemEntry);

	//::InterlockedCompareExchange((volatile LONG*)&hostTestState, testOnAck, testProc);
}
//---------------------------------------------------------------------------
void p2pNetTester_t::procOnAck()
{
	// called from recv thread

	//if (hostTestState != testOnAck)
	//{
	//	debugf(TEXT(">procOnAck> invalid test state(%d)"), (INT)hostTestState);
	//	return;
	//}

	//if (idToTest == 0)
	//{
	//	debugf(TEXT(">procOnAckt> invalid test id"));
	//	return;
	//}

//#if !FINAL_RELEASE
//	TCHAR wAddrLocal[256];
//	TCHAR wAddrPublic[256];
//	mbstowcs(wAddrLocal, inet_ntoa(addrToTestLocal.sin_addr), 255);
//	mbstowcs(wAddrPublic, inet_ntoa(addrToTestPublic.sin_addr), 255);
//	debugf(TEXT(">procOnAck> id=%d Ladr=%s:%d Padr=%s:%d"), idToTest, wAddrLocal, ntohs(addrToTestLocal.sin_port), wAddrPublic, ntohs(addrToTestPublic.sin_port));
//#endif

	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&clientTestList);
	while (pEntry)
	{
		pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;

#if !FINAL_RELEASE
		TCHAR wAddrLocal[256];
		TCHAR wAddrPublic[256];
		mbstowcs(wAddrLocal, inet_ntoa(pItem->addrLocal.sin_addr), 255);
		mbstowcs(wAddrPublic, inet_ntoa(pItem->addrPublic.sin_addr), 255);
		_LOG(TEXT("id=%d Ladr=%s:%d Padr=%s:%d"), pItem->idToTest, wAddrLocal, ntohs(pItem->addrLocal.sin_port), wAddrPublic, ntohs(pItem->addrPublic.sin_port));
#endif

		if (pItem->idToTest == ID_INVALID_ACCOUNT)
		{
			_LOG(TEXT("invalid test id"));
			delete pItem;
			continue;
		}

		Ichs_t::pplayerInfo_t ppi = testingItemList.Find(pItem->idToTest);
		if (!ppi)
		{
			testingItemList.Set(pItem->idToTest, Ichs_t::playerInfo_t());
			ppi = testingItemList.Find(pItem->idToTest);
		}
		if (!ppi)
		{
			_LOG(TEXT("failed to alloc playerInfo_t"));
			delete pItem;
			continue;
		}

		ppi->init();
		ppi->idAccount = idAccount;
		//ppi->connInfo.state = Ichs_t::connInfo_t::csReserved;

		//if(ppi->connInfo.state == Ichs_t::connInfo_t::csReserved ) {
			//ppi->connInfo.initTest(); // [-] 20070621 timeout detect
			ppi->connInfo.state = Ichs_t::connInfo_t::csStarting;
			pP2p->registerPeer(pItem->idToTest, &pItem->addrLocal, &pItem->addrPublic);
			if(pP2p->requestConnectPeer(pItem->idToTest)) {
				_LOG(TEXT("requestConnectPeer(%d)"), pItem->idToTest);
			}
			else {
				// already connected
				ppi->connInfo.state = Ichs_t::connInfo_t::csActive;
				_LOG(TEXT("%d->%d...(already connected:%d)"), idAccount, pItem->idToTest, ppi->connInfo.timeBeginRtt);
				ppi->connInfo.timeBeginProb = timeGetTime();
				pP2p->send_rttProb(pItem->idToTest);
			}
		//}
		//else {
		//	debugf(TEXT(">procOnAck> invalid state %d of %d "), (INT)ppi->connInfo.state, idToTest);
		//	ppi->connInfo.bCanceled = true;		
		//	ppi->endUpdate();
		//	hostTestState = testEnd;
		//}

		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&clientTestList);
	}
}
//---------------------------------------------------------------------------
void p2pNetTester_t::onNtfP2pConnect(Def::TID_ACCOUNT idRequester, SOCKADDR_IN* addrLocal, SOCKADDR_IN* addrPublic)
{	
	// called from main thread

	//if (hostTestState != testNone && hostTestState != testOnNtf)
	//{
	//	debugf(TEXT(">onNtfP2pConnect> invalid test state(%d)"), (INT)hostTestState);
	//	return;
	//}

	//if(addrLocal->sin_addr.S_un.S_addr == 0x0 || addrPublic->sin_addr.S_un.S_addr == 0x0) {
	//	debugf(TEXT("[p2pNetTester_t::onNtfP2pConnect] Invalid address for %d"), idRequester);
	//	hostTestState = testNone;
	//	return;
	//}

#if !FINAL_RELEASE
	TCHAR wAddrLocal[256];
	TCHAR wAddrPublic[256];
	mbstowcs(wAddrLocal, inet_ntoa(addrLocal->sin_addr), 255);
	mbstowcs(wAddrPublic, inet_ntoa(addrPublic->sin_addr), 255);
	_LOG(TEXT("id=%d Ladr=%s:%d Padr=%s:%d"), idRequester, wAddrLocal, ntohs(addrLocal->sin_port), wAddrPublic, ntohs(addrPublic->sin_port));
#endif

	pp2pTestItem_t pItem = new p2pTestItem_t();
	check(pItem);

	pItem->idToTest = idRequester;
	appMemcpy(&pItem->addrLocal, addrLocal, sizeof(SOCKADDR_IN));
	appMemcpy(&pItem->addrPublic, addrPublic, sizeof(SOCKADDR_IN));

	::InterlockedPushEntrySList(&hostTestList, &pItem->itemEntry);

	//::InterlockedCompareExchange((volatile LONG*)&hostTestState, testOnNtf, testNone);
	//hostTestState = testOnNtf;
}
//---------------------------------------------------------------------------
void p2pNetTester_t::procOnNtf()
{
	// called from recv thread

	//if (hostTestState != testOnNtf)
	//{
	//	debugf(TEXT(">procOnNtf> invalid test state(%d)"), (INT)hostTestState);
	//	return;
	//}

	//log(">onNtfP2pConnect> id=%d Ladr=%s:%d Padr=%s:%d\n", idRequester, inet_ntoa(addrLocal->sin_addr), ntohs(addrLocal->sin_port), inet_ntoa(addrPublic->sin_addr), ntohs(addrPublic->sin_port));	

	/* [-] 20070601 dEAthcURe|RTT 이게 없어도 registerPeer에서 적당히 거시기 된다.
	Ichs_t::pplayerInfo_t ppi = pChs->getPlayerInfo(idRequester);

	if(ppi && ppi->connInfo.state != Ichs_t::connInfo_t::csIdle) {
		debugf(TEXT(">onNtfP2pConnect> already registered %d"), idRequester);
		return;
	}
	*/

	//::InterlockedCompareExchange((volatile LONG*)&hostTestState, testNone, testOnNtf);

	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&hostTestList);
	while (pEntry)
	{
		pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;

#if !FINAL_RELEASE
		TCHAR wAddrLocal[256];
		TCHAR wAddrPublic[256];
		mbstowcs(wAddrLocal, inet_ntoa(pItem->addrLocal.sin_addr), 255);
		mbstowcs(wAddrPublic, inet_ntoa(pItem->addrPublic.sin_addr), 255);
		_LOG(TEXT("id=%d Ladr=%s:%d Padr=%s:%d"), pItem->idToTest, wAddrLocal, ntohs(pItem->addrLocal.sin_port), wAddrPublic, ntohs(pItem->addrPublic.sin_port));
#endif

		if (pItem->idToTest == 0)
		{
			_LOG(TEXT("invalid test id"));
			delete pItem;
			continue;
		}

		pP2p->removePeer(pItem->idToTest); // [-] 20070621 진행중인 내검사가 취소될수있음 // [+] 20070604
		pP2p->registerPeer(pItem->idToTest, &pItem->addrLocal, &pItem->addrPublic);	
		if(pP2p->requestConnectPeer(pItem->idToTest)) {
			_LOG(TEXT("requestConnectPeer(%d)"), pItem->idToTest);
		}
		else {
			_LOG(TEXT("already connected(%d)"), pItem->idToTest);
		}

		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&hostTestList);
	}
}
//---------------------------------------------------------------------------
void p2pNetTester_t::onSessionEstablished(Def::TID_ACCOUNT idPeer)
{
	// called from recv thread

	_LOG(TEXT("%d->%d..."), idAccount, idPeer);
	Ichs_t::pplayerInfo_t ppi = testingItemList.Find(idPeer);//pChs->getPlayerInfo(idPeer);
	if(ppi) {
		ppi->connInfo.state = Ichs_t::connInfo_t::csActive;
		ppi->connInfo.initTest();

		ppi->connInfo.timeBeginProb = timeGetTime();
		pP2p->send_rttProb(idPeer);
		/* connInfo.state를 믿지 말아보쟈.
		if(ppi->connInfo.state == Ichs_t::connInfo_t::csStarting) {
			ppi->connInfo.state = Ichs_t::connInfo_t::csActive;
			ppi->connInfo.initTest();

			debugf(TEXT("[p2pNetTester_t::onSessionEstablished] %d->%d..."), idAccount, idPeer);
			ppi->connInfo.timeBeginProb = timeGetTime();
			pP2p->send_rttProb(idPeer);
		}
		else {
			debugf(TEXT("[onSessionEstablished] invalid state=%d of %d"), (INT)ppi->connInfo.state, idPeer);
		}	
		*/
	}
	return;
}
//---------------------------------------------------------------------------
void p2pNetTester_t::onSessionDisconnected(Def::TID_ACCOUNT idPeer)
{
	// called from recv thread

	_LOG(TEXT("%d->%d"), idAccount, idPeer);		
	pP2p->removePeer(idPeer);

	Ichs_t::pplayerInfo_t ppi = testingItemList.Find(idPeer);//pChs->getPlayerInfo(idPeer);
	if(ppi) {
		ppi->connInfo.state = Ichs_t::connInfo_t::csIdle;
		ppi->connInfo.bCanceled = true;		
		//pChs->onSessionDisconnected(idPeer);
		pChs->endUpdate(idPeer, ppi);
		testingItemList.Remove(idPeer);
	}	
	return;
}
//---------------------------------------------------------------------------
void p2pNetTester_t::onRttProbAck(Def::TID_ACCOUNT idPeer)
{
	// called from recv thread

	_LOG(TEXT("idPeer=%d"), idPeer);

	Ichs_t::pplayerInfo_t ppi = testingItemList.Find(idPeer);//pChs->getPlayerInfo(idPeer);
	if(ppi && ppi->connInfo.state == Ichs_t::connInfo_t::csActive) {
		++ppi->connInfo.nSuccess;
		ppi->connInfo.timeEndProb = timeGetTime();
		DWORD timeDiff = ppi->connInfo.timeEndProb - ppi->connInfo.timeBeginProb;
		if(timeDiff > ppi->connInfo.timeMaxRtt) ppi->connInfo.timeMaxRtt = timeDiff;
		if(timeDiff < ppi->connInfo.timeMinRtt) ppi->connInfo.timeMinRtt = timeDiff;
		ppi->connInfo.timeSumRtt += timeDiff;
		ppi->connInfo.timeAvgRtt = ppi->connInfo.timeSumRtt / ppi->connInfo.nSuccess;
		_LOG(TEXT("RTT %d->%d %d/%d %d,%d,%d\r"), idAccount, ppi->idAccount, ppi->connInfo.nSuccess+ppi->connInfo.nFail, nTrial, ppi->connInfo.timeMinRtt, ppi->connInfo.timeMaxRtt, ppi->connInfo.timeAvgRtt);

		if(ppi->connInfo.nSuccess + ppi->connInfo.nFail < nTrial) {
			pChs->procUpdate(idPeer);

			ppi->connInfo.timeBeginProb = timeGetTime();
			pP2p->send_rttProb(idPeer);
		}
		else {
			ppi->connInfo.state = Ichs_t::connInfo_t::csIdle;
			ppi->connInfo.bCanceled = ppi->connInfo.nFail >= nFailToCancel;
			ppi->endUpdate();

			Ichs_t::connInfo_t ci;
			ppi->getConnectionInfo(ci);
			_LOG(TEXT("%d->%d %d=%ds+%df trials min=%d, max=%d, avg=%d %s\t"), idAccount, ppi->idAccount, ci.nSuccess + ci.nFail, ci.nSuccess, ci.nFail, ci.timeMinRtt, ci.timeMaxRtt, ci.timeAvgRtt, ci.bCanceled?TEXT("*canceled*"):TEXT(""));
			// ci.timeAvgRtt 저장

			//pChs->endUpdate(idPeer);
			pChs->endUpdate(idPeer, ppi);
			testingItemList.Remove(idPeer);
		}
	}
	else {
		_LOG(TEXT("Invalid ppi=0x%x for %d state=%d"), ppi, idPeer, ppi?ppi->connInfo.state:-1);
	}
}
//---------------------------------------------------------------------------
void p2pNetTester_t::hostThreadFunc(void) 
{
	if (strlen(p2psIpAddr) == 0)
		strcpy(p2psIpAddr, DefaultP2psIpAddr);
	if (p2psPort == 0)
		p2psPort = DefaultP2psPort;	

	int localPort = 27888;//8799; // !

	if(pP2p && pChs && idAccount != 0) {
		
		setHostThreadState(tsActive, false);
		_LOG(TEXT("Host thread activated"));
		//if(pChs->connect(0, 0)) {
			//debugf(TEXT("[p2pNetTester_t::hostThreadFunc] chs connected"));
			pP2p->init(localPort);
			while(hostThreadState!=tsTerminating) { //log("[p2pNetTester_t::hostThreadFunc] requesting peer list...\n");
				//if(bTestRtt) {
				bool _local_bKeepServerConnection = bKeepServerConnection; // [+] 20070628

				switch (hostTestState)
				{
				case testInit:
					_LOG(TEXT("hostTestState = testInit"));
					{
						::InterlockedCompareExchange((volatile LONG*)&hostTestState, testNone, testInit);

						emptyToTestList();
						emptyHostTestList();
						emptyClientTestList();

						if(pP2p->connectToServer(p2psIpAddr, p2psPort, idAccount)) {
							_LOG(TEXT("p2p connected %s:%d for %d "), ANSI_TO_TCHAR(p2psIpAddr), p2psPort, idAccount);
#if !FINAL_RELEASE
							TCHAR wAddrLocal[256];
							TCHAR wAddrPublic[256];
							mbstowcs(wAddrLocal, inet_ntoa(pP2p->addrLocal.sin_addr), 255);
							mbstowcs(wAddrPublic, inet_ntoa(pP2p->addrPublic.sin_addr), 255);
							_LOG(TEXT("send_ntfMyAddr id=%d Ladr=%s:%d Padr=%s:%d"),
									idAccount, wAddrLocal, ntohs(pP2p->addrLocal.sin_port), wAddrPublic, ntohs(pP2p->addrPublic.sin_port));
#endif
							pChs->send_ntfMyAddr(idAccount, pP2p->addrLocal, pP2p->addrPublic);

							pP2p->removeAllPeer();
						}
						else {
							_LOG(TEXT("Connecting to p2ps %s %d failed"), ANSI_TO_TCHAR(p2psIpAddr), p2psPort);
							pChs->onConnectionFailed();
						}
						pP2p->disconnect();					
					}
					break;

				case testStart:
					_LOG(TEXT("hostTestState = testStart"));
					{
						::InterlockedCompareExchange((volatile LONG*)&hostTestState, testProc, testStart);

						emptyHostTestList();
						emptyClientTestList();

						Sleep(startDelay);

						if(pP2p->connectToServer(p2psIpAddr, p2psPort, idAccount)) {
							_LOG(TEXT("p2p connected %s:%d for %d"), ANSI_TO_TCHAR(p2psIpAddr), p2psPort, idAccount);

#if !FINAL_RELEASE
							TCHAR wAddrLocal[256];
							TCHAR wAddrPublic[256];
							mbstowcs(wAddrLocal, inet_ntoa(pP2p->addrLocal.sin_addr), 255);
							mbstowcs(wAddrPublic, inet_ntoa(pP2p->addrPublic.sin_addr), 255);
							_LOG(TEXT("send_ntfMyAddr id=%d Ladr=%s:%d Padr=%s:%d"),
									idAccount, wAddrLocal, ntohs(pP2p->addrLocal.sin_port), wAddrPublic, ntohs(pP2p->addrPublic.sin_port));
#endif
							pChs->send_ntfMyAddr(idAccount, pP2p->addrLocal, pP2p->addrPublic);

							pP2p->removeAllPeer();

							if (hostTestState == testProc)
							{
								PSLIST_ENTRY pEntry;
								pEntry = ::InterlockedPopEntrySList(&toTestList);
								while (pEntry)
								{
									pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;

									if(!pChs->send_reqConnect(idAccount, pItem->idToTest))
										_LOG(TEXT("reqConnect failed; idPeer = %d"), pItem->idToTest);

									delete pItem;
									pEntry = ::InterlockedPopEntrySList(&toTestList);
								}

								while (hostTestState == testProc) {
									threadState_t ts;
									if(getHostThreadStateChanging(ts, timeSleep) && ts == tsTerminating) {
										break;
									}
								}
							}
							else
							{
								if (hostTestState != testStart)
									emptyToTestList();
							}
						}
						else {
							_LOG(TEXT("Connecting to p2ps %s %d failed"), ANSI_TO_TCHAR(p2psIpAddr), p2psPort);
							pChs->onConnectionFailed();
						}
						pP2p->disconnect();					
					}
					break;

				//case testStop:
				//	// recv thread 에서 상태를 testNone으로 리셋
				//	emptyToTestList();
				//	emptyHostTestList();
				//	emptyClientTestList();
				//	break;
				}

				if(_local_bKeepServerConnection) {
					pP2p->heartbeatToServer();
				}

				// {{ sleep until next trial
				threadState_t ts;
				if(getHostThreadStateChanging(ts, timeSleep) && ts == tsTerminating) {
					break;
				}
				// }} sleep until next trial
			}			
			pP2p->deinit();
			//pChs->disconnect();
		//}
		//else debugf(TEXT("[p2pNetTester_t::hostThreadFunc] connecting to chs failed"));
	}
//_endHostThreadProc:

	emptyToTestList();
	emptyHostTestList();
	emptyClientTestList();

	setHostThreadState(tsTerminated);
	_LOG(TEXT("Host thread terminated"));
	SetEvent(hEventHostThreadTerminated);
}
//---------------------------------------------------------------------------
void p2pNetTester_t::recvThreadFunc(void) 
{
	if(pP2p) {
		setHostThreadState(tsActive, false);
		_LOG(TEXT("recv thread activated"));
		while(clientThreadState!=tsTerminating) {			
			fd_set readFds, writeFds, exceptFds;

			FD_ZERO(&readFds);
			FD_ZERO(&writeFds);
			FD_ZERO(&exceptFds);
			
			int nSet = 0;
			if(pP2p && pP2p->sd != INVALID_SOCKET) {
				FD_SET(pP2p->sd, &readFds);
				++nSet;
			}
			//if(pChs && pChs->sd != INVALID_SOCKET) {
			//	FD_SET(pChs->sd, &readFds);
			//	++nSet;
			//}

			if(nSet) {
				struct timeval timevalue;
				DWORD ms = 1; // tcp/udp중 하나만 valid하면 다른 하나를 기다리기 위해 select의 wait 시간을 단축한다.
				if(nSet==1) ms = 1000; // chs 통신 배제 if(nSet==2) ms = 1000;
				timevalue.tv_sec = ms/1000;				
				timevalue.tv_usec = (ms%1000)*1000;				

				::select(0, &readFds, &writeFds, &exceptFds, &timevalue);
				if(pP2p && FD_ISSET(pP2p->sd, &readFds)) {
					//__try { // [!] 20070529
						pP2p->recv();
					/* // [!] 20070529
					}					
					__except(EXCEPTION_EXECUTE_HANDLER) {
						int e = GetExceptionCode();
						debugf(TEXT("p2p exception %d"), e);
					}
					*/
				}
				//if(pChs && FD_ISSET(pChs->sd, &readFds)) {					
				//	__try {
				//		pChs->recv();
				//	}
				//	__except(EXCEPTION_EXECUTE_HANDLER) {
				//		int e = GetExceptionCode();
				//		debugf(TEXT("chs exception %d\n"), e);
				//	}
				//}
			}
			else Sleep(10);

			if(pChs) {
				__try {
					//pChs->onTick();
					dispatch();
				}
				__except(EXCEPTION_EXECUTE_HANDLER) {
					int e = GetExceptionCode();
					_LOG(TEXT("onTick exception %d\n"), e);
				}
			}
		}
	}
	setRecvThreadState(tsTerminated);
	_LOG(TEXT("recv thread terminated"));
	SetEvent(hEventClientThreadTerminated);
}
//---------------------------------------------------------------------------
bool p2pNetTester_t::setActive(bool bActive, bool bBlocking, DWORD timeout) 
{
	//return false; // [!] 20070528 dEAthcURe|TEST
	if(bActive) {
		if(hostThreadState == tsNone || hostThreadState == tsTerminated) {
			// starting new thread
			hRecvThread = _beginthread(recvThreadProc, 0, (void*)this);
			if(!hRecvThread) return false;

			hHostThread = _beginthread(hostThreadProc, 0, (void*)this); 
			if(!hHostThread) {
				setRecvThreadState(tsTerminating);
				return false;
			}
			_LOG(TEXT("threads started 0x%x 0x%x"), hHostThread, hRecvThread);
			return true;
		}
	}		
	else if(!bActive) {
		if(hostThreadState == tsCreated || hostThreadState == tsActive) {
			setHostThreadState(tsTerminating);
			setRecvThreadState(tsTerminating);

			if(bBlocking) {
				WaitForSingleObject(hEventHostThreadTerminated, timeout); // INFINITE);
				WaitForSingleObject(hEventClientThreadTerminated, timeout); // INFINITE);
			}
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------	
void p2pNetTester_t::activateRttTest(bool bActive, bool _bKeepServerConnection, DWORD _startDelay)
{
	// called from main thread
	bKeepServerConnection = _bKeepServerConnection;
	//bTestRtt = bActive;
	if (bActive)
	{
		startDelay = _startDelay;
		hostTestState = testStart;
		_LOG(TEXT("host test state is set to %d"), (INT)hostTestState);
	}
	else
	{
		hostTestState = testStop;
		_LOG(TEXT("test state is set to testNone"));
	}
}
//---------------------------------------------------------------------------
void p2pNetTester_t::initRttTest()
{
	hostTestState = testInit;
	_LOG(TEXT("Test state is set to testInit"));
}
//---------------------------------------------------------------------------
void p2pNetTester_t::dispatch()
{
	// called from recv thread

	if (hostTestState == testStop)
	{
		::InterlockedCompareExchange((volatile LONG*)&hostTestState, testNone, testStop);
		emptyToTestList();
		//emptyHostTestList();
		emptyClientTestList();
		pP2p->removeAllPeer();
		testingItemList.Empty();
		return;
	}

	procOnNtf();
	procOnAck();

	for (TMap<Def::TID_ACCOUNT,Ichs_t::playerInfo_t>::TIterator it(testingItemList); it; ++it)
	{
		TID_ACCOUNT idPeer= it.Key();
		Ichs_t::playerInfo_t &pi = it.Value();

		{
			DWORD timeCurrent = timeGetTime();
			if(pi.connInfo.state != Ichs_t::connInfo_t::csIdle && pi.connInfo.timeBeginRtt + 30000 < timeCurrent) {
				_LOG(TEXT("RTT probing timeout for %d(st:%d:%d)<-------------\n"), idPeer, (INT)pi.connInfo.state, pi.connInfo.timeBeginRtt);
				pi.connInfo.state = Ichs_t::connInfo_t::csIdle;
				pi.connInfo.bCanceled = true;
				pi.connInfo.timeBeginRtt = timeCurrent; // [+] 20070621 dEAthcURe initialize timeout 

				pChs->endUpdate(idPeer, &pi);
				it.RemoveCurrent();
			}
		}
	}
}
//---------------------------------------------------------------------------
void p2pNetTester_t::addToTestList( Def::TID_ACCOUNT idToTest )
{
	if (idToTest == ID_INVALID_ACCOUNT)
		return;

	pp2pTestItem_t pItem = new p2pTestItem_t();
	check(pItem);

	pItem->idToTest = idToTest;

	::InterlockedPushEntrySList(&toTestList, &pItem->itemEntry);
}
//---------------------------------------------------------------------------	
void p2pNetTester_t::emptyToTestList()
{
	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&toTestList);
	while (pEntry)
	{
		pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;
		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&toTestList);
	}
}
//---------------------------------------------------------------------------	
void p2pNetTester_t::emptyHostTestList()
{
	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&hostTestList);
	while (pEntry)
	{
		pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;
		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&hostTestList);
	}
}
//---------------------------------------------------------------------------	
void p2pNetTester_t::emptyClientTestList()
{
	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&clientTestList);
	while (pEntry)
	{
		pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;
		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&clientTestList);
	}
}
//---------------------------------------------------------------------------	
bool p2pNetTester_t::init(pIp2p_t _pP2p, pIchs_t _pChs)
{	
	pP2p = _pP2p; if(pP2p) pP2p->pParent = this;
	pChs = _pChs; if(pChs) pChs->pParent = this;
	_LOG(TEXT("Ip2p=0x%x Ichs=0x%x"), pP2p, pChs);
	
	return true;
}
//---------------------------------------------------------------------------	
p2pNetTester_t::p2pNetTester_t(void) 
{
	//bTestRtt = false;
	appMemzero(p2psIpAddr, 16);
	p2psPort = 0;
	timeSleep = 1000;
	idAccount = 0;
	nTrial = 5;//10;
	nFailToCancel = 3;//5;
	timeThreadControl = 3000;
	startDelay = 0;

	hostThreadState = tsNone;
	clientThreadState = tsNone;

	hEventChangeHostThreadState = CreateEvent(0x0, false, false, TEXT("hEventChangeHostThreadState"));
	hEventChangeClientThreadState = CreateEvent(0x0, false, false, TEXT("hEventChangeClientThreadState"));	
	hEventHostThreadTerminated = CreateEvent(0x0, false, false, TEXT("hEventHostThreadTerminated"));
	hEventClientThreadTerminated = CreateEvent(0x0, false, false, TEXT("hEventClientThreadTerminated"));

	hostTestState = testNone;
	//idToTest = 0;
	//addrToTestLocal.sin_family = AF_INET;
	//addrToTestLocal.sin_addr.S_un.S_addr = 0;
	//addrToTestLocal.sin_port = 0;
	//addrToTestPublic.sin_family = AF_INET;
	//addrToTestPublic.sin_addr.S_un.S_addr = 0;
	//addrToTestPublic.sin_port = 0;

	::InitializeSListHead(&toTestList);
	::InitializeSListHead(&hostTestList);
	::InitializeSListHead(&clientTestList);
}
//---------------------------------------------------------------------------	
p2pNetTester_t::~p2pNetTester_t(void) 
{
	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&hostTestList);
	while (pEntry)
	{
		pp2pTestItem_t pItem = (pp2pTestItem_t)pEntry;
		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&hostTestList);
	}

	emptyToTestList();
	emptyHostTestList();
	emptyClientTestList();

	setHostThreadState(tsTerminating);
	setRecvThreadState(tsTerminating);

	//WaitForSingleObject(hEventHostThreadTerminated, INFINITE); // [!-] 20070507
	//WaitForSingleObject(hEventClientThreadTerminated, INFINITE); // [!-] 20070507

	CloseHandle(hEventChangeHostThreadState);
	CloseHandle(hEventChangeClientThreadState);

	CloseHandle(hEventHostThreadTerminated);
	CloseHandle(hEventClientThreadTerminated);
}
//---------------------------------------------------------------------------	
