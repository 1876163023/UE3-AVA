/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: p2pHandler.cpp

	Description: Implementation of CavaP2PHandler

***/
#include "avaNet.h"
#include "avaNetStateController.h"
#include "avaMsgSend.h"
#include "p2pHandler.h"

#ifdef EnableHostMigration
#include "hostMigration.h" // 20070222 dEAthcURe|HM
#endif


void CavaP2PHandler::OnInitListen(FSocket *Socket)
{
#ifdef EnableP2pConn
	if (GavaNetClient->IsValid() && _StateController->RoomInfo.IsValid())
	{
		PM::GAME::READY_NTF::Send();// 20070224 [-] 20070123 dEAthcURe
	}

	GetAvaNetHandler()->StartLoadingCheck();
#endif
}


void CavaP2PHandler::OnInitConnect(FSocket *Socket, FURL &ConnectURL)
{
	// {{ 20070222 dEAthcURe|HM
	#ifdef EnableHostMigration
	if ( _StateController->RoomInfo.IsValid() &&  _StateController->RoomInfo.IsValid() &&
		_StateController->RoomInfo.HostIdx >= 0 && _StateController->RoomInfo.HostIdx < Def::MAX_ALL_PLAYER_PER_ROOM) {
		FRoomPlayerInfo &HostInfo = _StateController->RoomInfo.PlayerList.PlayerList[_StateController->RoomInfo.HostIdx];
		g_hostMigration.setHostName(HostInfo.PlayerInfo.nickname, FRoomInfo::SlotToTeam(_StateController->RoomInfo.HostIdx));
	}
	#endif
	// }} 20070222 dEAthcURe|HM

#ifdef EnableP2pConn
	if (GavaNetClient->IsValid() && _StateController->RoomInfo.IsValid())
	{

	}
#endif
}

void CavaP2PHandler::OnTick()
{
#ifdef EnableP2pConn
	if (GavaNetClient->IsValid() && GavaNetClient->pp2pConn)
	{
		GavaNetClient->pp2pConn->tick();		
	}
#endif
}


bool CavaP2PHandler::OnRecvFrom(char *Data, INT BytesRead, FInternetIpAddr &FromAddr)
{
#ifdef EnableP2pConn	
	if (GavaNetClient->IsValid() && GavaNetClient->pp2pConn)
	{
		if(GavaNetClient->pp2pConn->onRecvFrom(Data, BytesRead, (sockaddr*)&(FromAddr.Addr), sizeof(FromAddr.Addr))) {
			return true;
		}
	}	
#endif
	return false;
}
