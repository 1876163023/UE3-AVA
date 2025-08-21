#include "avaNet.h"

#include "p2pNetTester.h"

#include "ComDef/Def.h"
#include "avaMsgSend.h"
#include "avaNetStateController.h"

using namespace Def;


#ifdef EnableRttTest

CavaChsImpl::CavaChsImpl(pIp2pNetTester_t _pParent) : Ichs_t(_pParent), RttRecheckThreshold(100), RttRecheckTime(60)
{
	::InitializeSListHead(&testResultList);
}

CavaChsImpl::~CavaChsImpl()
{
	EmptyTestResultList();
	CachedInfoList.Empty();
}

bool CavaChsImpl::connect(char* ipAddr, int port)
{
	return true; // 20070517 dEAthcURe|RTT
}

bool CavaChsImpl::send_ntfMyAddr(int idAccount, SOCKADDR_IN addrLocal, SOCKADDR_IN addrPublic)
{
	UDP_HOST_INFO HostInfo;
	HostInfo.intAddr.ipAddress = addrLocal.sin_addr.S_un.S_addr;
	HostInfo.intAddr.port = addrLocal.sin_port;
	HostInfo.extAddr.ipAddress = addrPublic.sin_addr.S_un.S_addr;
	HostInfo.extAddr.port = addrPublic.sin_port;
	PM::CLIENT::SET_RTTTEST_ADDR_NTF::Send(HostInfo);
	return true;
}

int CavaChsImpl::getIdAccount(int idx)
{
	return ID_INVALID_ACCOUNT;
}

Ichs_t::pplayerInfo_t CavaChsImpl::getPlayerInfo(int idAccount)
{
	return NULL;
}

bool CavaChsImpl::send_reqConnect(int idAccRequester, int idAccPeer)
{
	if (idAccRequester == ID_INVALID_ACCOUNT || idAccPeer == ID_INVALID_ACCOUNT)
		return false;

	UDP_HOST_INFO udpInfo;
	udpInfo.intAddr.ipAddress = GavaNetClient->p2pNtImpl.addrLocal.sin_addr.S_un.S_addr;
	udpInfo.intAddr.port = GavaNetClient->p2pNtImpl.addrLocal.sin_port;
	udpInfo.extAddr.ipAddress = GavaNetClient->p2pNtImpl.addrPublic.sin_addr.S_un.S_addr;;
	udpInfo.extAddr.port = GavaNetClient->p2pNtImpl.addrPublic.sin_port;
	PM::ROOM::RTTT_START_REQ::Send(idAccPeer, udpInfo);

	return true;
}

void CavaChsImpl::procUpdate(int idAccount)
{
	_LOG(TEXT("idAccount = %d"), idAccount);
}

void CavaChsImpl::endUpdate(int idAccount, Ichs_t::pplayerInfo_t ppi)
{
	// called from recv thread

	_LOG(TEXT("idAccount = %d"), idAccount);

	FTestResultItem *pItem = new FTestResultItem();
	check(pItem);

	pItem->idToTest = idAccount;
	appMemcpy(&pItem->playerInfo, ppi, sizeof(Ichs_t::playerInfo_t));

	::InterlockedPushEntrySList(&testResultList, &pItem->itemEntry);
}

void CavaChsImpl::onSessionDisconnected(int idAccount)
{
	_LOG(TEXT("idAccount = %d"), idAccount);

	//FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find(idAccount);
	//if (!Info && Info->RoomPlayerInfo.idSlot == _StateController->RoomInfo.HostIdx)
	//{
	//	GavaNetClient->bInvalidPing = TRUE;
	//}
}

void CavaChsImpl::onTick()
{
}

void CavaChsImpl::onConnectionFailed()
{
	//GavaNetClient->bInvalidPing = TRUE;
}

FLOAT CavaChsImpl::CheckAndGetRttValue(DWORD idAccount)
{
	FRttResultInfo *pInfo = CachedInfoList.Find(idAccount);
	if (pInfo && appSeconds() <= pInfo->CheckTime + RttRecheckTime && pInfo->RttValue > 0 && pInfo->RttValue <= RttRecheckThreshold)
	{
		// 테스트하고자 하는 대상이 이미 캐쉬되어 있고, 테스트한지 RttRecheckTime 만큼 지나지 않았고,
		// 테스트한 RTT 값이 RttRecheckThreshold를 넘지 않으면 새로 테스트할 필요가 없으므로 값을 통보
		return pInfo->RttValue;
	}

	return -1;
}

void CavaChsImpl::ProcTestResults()
{
	// called from main thread

	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&testResultList);
	while (pEntry)
	{
		FTestResultItem *pItem = (FTestResultItem*)pEntry;
		FLOAT RttValue = (pItem->playerInfo.connInfo.nSuccess > 0 && !pItem->playerInfo.connInfo.bCanceled) ? (FLOAT)pItem->playerInfo.connInfo.timeAvgRtt : -2.0f;

		_LOG(TEXT("RTT Test finished. id = %d, rtt = %.3f"), pItem->idToTest, RttValue);

		if (RttValue >= 0)
		{
			FRttResultInfo *pResult = CachedInfoList.Find(pItem->idToTest);
			if (!pResult)
			{
				CachedInfoList.Set(pItem->idToTest, FRttResultInfo());
				pResult = CachedInfoList.Find(pItem->idToTest);
			}

			if (pResult)
			{
				pResult->RttValue = RttValue;
				pResult->CheckTime = appSeconds();
			}
		}

		GetAvaNetRequest()->SetMyRttValue(RttValue);

		delete pItem;
		pEntry = ::InterlockedPopEntrySList(&testResultList);
	}
}

void CavaChsImpl::EmptyTestResultList()
{
	PSLIST_ENTRY pEntry;
	pEntry = ::InterlockedPopEntrySList(&testResultList);
	while (pEntry)
	{
		FTestResultItem *pItem = (FTestResultItem*)pEntry;
		delete pItem;

		pEntry = ::InterlockedPopEntrySList(&testResultList);
	}
}


#endif

