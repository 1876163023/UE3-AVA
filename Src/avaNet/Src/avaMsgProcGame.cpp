/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcGame.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefGame.h"

#include "avaNetClient.h"
#include "avaMsgProc.h"
#include "avaMsgSend.h"
#include "avaNetEventHandler.h"
#include "avaNetStateController.h"
#include "avaCommunicator.h"

#ifdef EnableP2pConn
#include "p2pConn.h"
#endif

// {{ 20070212 dEAthcURe|HM
#ifdef EnableHostMigration
#include "hostMigration.h"
#endif
// }} 20070212 dEAthcURe|HM



//#define _TEMP_RESULT_LOG


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Game

#ifdef EnableHostMigration
//void GetOpenURL(FURL &OpenURL)
//{
//	FRoomInfo &Room = _StateController->RoomInfo;
//	check(Room.IsValid());
//
//	OpenURL.AddOption(TEXT("LAN"));
//
//	if (_StateController->AmIHost())
//	{
//		OpenURL.AddOption(TEXT("Listen"));
//
//		//OpenURL.AddOption(TEXT("MinNetPlayers=2"));
//		OpenURL.AddOption(*FString::Printf(TEXT("WinCondition=%d"), Room.RoomInfo.setting.roundToWin));
//		OpenURL.AddOption(*FString::Printf(TEXT("FFType=%d"), Room.RoomInfo.setting.tkLevel));
//
//		FMapInfo *Info = _StateController->GetCurrentMap();
//		if (Info)
//		{
//			OpenURL.Map = Info->FileName;
//		}
//		else
//		{
//			_LOG(TEXT("Error! Map not found."));
//			OpenURL.Map = TEXT("blahblah.ut3");
//		}
//	}
//	else
//	{
//		FString *HostAddress = GavaNetClient->Settings.Find(CFG_HOSTADDRESS);
//
//		if (HostAddress)
//			OpenURL.Host = *HostAddress;
//		else
//			OpenURL.Host = TEXT("127.0.0.1");
//
//		#ifdef EnableP2pConn
//		// {{ 20070124 dEAthcURe|HP
//		if(GavaNetClient->pp2pConn) {
//			SOCKADDR_IN* pAddr = (SOCKADDR_IN*)&GavaNetClient->pp2pConn->hostSockAddr;
//			if(pAddr->sin_addr.S_un.S_addr) {			
//				OpenURL.Host = FString::Printf(TEXT("%d.%d.%d.%d"), pAddr->sin_addr.S_un.S_un_b.s_b1,pAddr->sin_addr.S_un.S_un_b.s_b2,pAddr->sin_addr.S_un.S_un_b.s_b3,pAddr->sin_addr.S_un.S_un_b.s_b4);
//			}
//			else {
//				OpenURL.Host = TEXT("127.0.0.1");
//			}
//		}
//		// }} 20070124 dEAthcURe|HP
//		#endif
//
//		OpenURL.Map = TEXT("");
//
//		FMapInfo *Info = _StateController->GetCurrentMap();
//		if (Info)
//		{
//			OpenURL.AddOption(*FString::Printf(TEXT("PreLoad=%s"), *Info->FileName));
//		}
//		else
//		{
//			_LOG(TEXT("Error! Map not found."));
//		}
//	}
//
//	// add option strings
//	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
//	check(MyInfo);
//
//	OpenURL.AddOption(*FString::Printf(TEXT("Name=%s"), _StateController->PlayerInfo.PlayerInfo.nickname));
//	OpenURL.AddOption(*FString::Printf(TEXT("AID=%d"), _StateController->PlayerInfo.PlayerInfo.idAccount));
//	OpenURL.AddOption(*FString::Printf(TEXT("ChLevel=%d"), _StateController->PlayerInfo.PlayerInfo.level));
//
//	if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD)
//	{
//		OpenURL.AddOption(*FString::Printf(TEXT("Guild=%s"), _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName));
//	}
//
//	if (MyInfo->GetTeamID() == RT_SPECTATOR)
//	{
//		OpenURL.AddOption(TEXT("SpectatorOnly=1"));
//		return;
//	}
//
//	OpenURL.AddOption(*FString::Printf(TEXT("Team=%d"), MyInfo->GetTeamID()));//(MyInfo->RoomPlayerInfo.idTeam == RT_EU ? 0 : 1)));
//	OpenURL.AddOption(*FString::Printf(TEXT("Class=%d"), MyInfo->RoomPlayerInfo.currentClass));
//
//	// {{ 20070509 dEAthcURe|HM HM접속인지 구분
//	if(g_hostMigration.state == hmsNewClientPrepare) {
//		OpenURL.AddOption(TEXT("HM"));
//		debugf(TEXT("Add option 'HM' to OpenURL."));
//	}
//	// }} 20070509 dEAthcURe|HM HM접속인지 구분
//}
#endif

// 게임 시작
void PM::GAME::START_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_ROOM)
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	_StateController->RoomHostKickCount = -1;
	_StateController->LastResultInfo.Clear();

	UBOOL bWasPlaying = (Room.RoomInfo.state.playing == RIP_PLAYING);

	_StateController->CountDown = -1;
	Room.RoomInfo.state.playing = RIP_PLAYING;

	GetAvaNetHandler()->RoomStartingPlayerList.Empty();
	GCallbackEvent->Send(CALLBACK_PlayerListUpdated);

	if (!bWasPlaying)
	{
		// 카운트 끝나고 게임 시작
		Room.PlayerList.StartPlayerList.Empty();

		for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
		{
			if (Room.PlayerList.IsEmpty(i))
				continue;

			ROOM_PLAYER_INFO &Info = Room.PlayerList.PlayerList[i].RoomPlayerInfo;

			//Room.PlayerList.PlayerList[i].LoadingProgress = 0;

			if (Room.HostIdx == i || Info.bReady == _READY_WAIT)
			{
				// 방장이나 레디 상태인 플레이어은 모두 로딩 상태로
				Info.bReady = _READY_LOADING;

				if (def.bStart)
				{
					// 나도 게임을 시작하는 상태라면, StartingPlayerList 업데이트

					_LOG(TEXT("Adding [%d]%s into RoomStartingPlayerList"), i, Info.nickname);

					Room.PlayerList.StartPlayerList.Push(Info.idAccount);

					FavaRoomPlayerInfo *pInfo = new(GetAvaNetHandler()->RoomStartingPlayerList) FavaRoomPlayerInfo();
					
					pInfo->AccountID = Info.idAccount;
					((FString*)(&(pInfo->NickName)))->FString::FString();
					pInfo->NickName = Info.nickname;
					pInfo->NickFName = FName(Info.nickname);
					pInfo->Level = Info.level;
					pInfo->TeamID = Info.GetTeamID();
					pInfo->LoadingProgress = 0;
					pInfo->LoadingStepCount = 0;
					pInfo->UpdateCount = 0;
					pInfo->UpdateTime = appSeconds();

					GCallbackEvent->Send(CALLBACK_PlayerListUpdated);
				}
			}
			else
			{
				_LOG(TEXT("[%d]%s is not ready, so he is not starting the game"), i, Info.nickname);
			}
		}
	}

	if (!def.bStart || _StateController->AmIStealthMode())
	{
		_LOG(TEXT("Game started; waiting..."));
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 0, 0);
		return;
	}

	// 나도 게임을 시작하는 상태

	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (MyInfo)
		MyInfo->RoomPlayerInfo.bReady = _READY_LOADING;

	TCHAR IPAddress[20];

	def.hostInfo.extAddr.GetAddress(IPAddress);
	GavaNetClient->Settings.Set(CFG_HOSTADDRESS, IPAddress);

#ifdef EnableP2pConn
	// p2p 라이브러리 적용 -> 방장은 맵 로딩 시작, 클라이언트는 방장 로딩 대기
	// p2p 라이브러리 미적용 -> 전원 맵 로딩 시작
	if(!_StateController->AmIHost()) {
		((SOCKADDR_IN*)&GavaNetClient->HostAddrLocal)->sin_family = AF_INET;
		((SOCKADDR_IN*)&GavaNetClient->HostAddrLocal)->sin_addr.S_un.S_addr = def.hostInfo.intAddr.ipAddress;
		((SOCKADDR_IN*)&GavaNetClient->HostAddrLocal)->sin_port = def.hostInfo.intAddr.port;

		((SOCKADDR_IN*)&GavaNetClient->HostAddrPublic)->sin_family = AF_INET;
		((SOCKADDR_IN*)&GavaNetClient->HostAddrPublic)->sin_addr.S_un.S_addr = def.hostInfo.extAddr.ipAddress;
		((SOCKADDR_IN*)&GavaNetClient->HostAddrPublic)->sin_port = def.hostInfo.extAddr.port;	
	}

	if (_StateController->AmIHost() || bWasPlaying)
#endif
	{
		FURL OpenURL;
		_StateController->GetOpenURL(OpenURL);

		if (OpenURL.IsLocalInternal())
		{
			FString PackageFilename;
			if (!GPackageFileCache->FindPackageFile(*OpenURL.Map, NULL, PackageFilename))
			{
				_LOG(TEXT("ERROR: The map '%s' does not exist."), *OpenURL.Map);

				CavaNetEventHandler::FinishLoadingProgress(FALSE);

				GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT("failed"), TEXT(""), 0, 0);

				// disconnect?
				//GetAvaNetRequest()->LeaveGame();
				return;
			}
		}

		_StateController->GoToState(_AN_INGAME);
		CavaNetEventHandler::StartLoadingProgress(!bWasPlaying);

		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);
		_LOG(TEXT("Opening URL... %s"), *(OpenURL.String()));

		// {{ 20070413 dEAthcURe|HP nonblock p2p client
		if(!_StateController->AmIHost()) {
			GavaNetClient->ConnectURL = OpenURL;
			_ahead_deinitSocket();			
			if(_ahead_initSocket() && GavaNetClient) {
				if(GavaNetClient->pp2pConn) delete GavaNetClient->pp2pConn;
				GavaNetClient->pp2pConn = new p2pConn_t(GavaNetClient);
				if(GavaNetClient->pp2pConn) {
					GavaNetClient->pp2pConn->setId(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);			
					debugf(TEXT("          <---------- [PM::GAME::START_NTF::Proc] localPort %d"), _ahead_port);
					GavaNetClient->pp2pConn->initUdp(_ahead_socket->Socket, _ahead_port);				
				}
				OpenURL.AddOption(TEXT("p2p_wait_svr_connect"));
			}
		}
		// }} 20070413 dEAthcURe|HP nonblock p2p client

#ifdef _BT_TEST_BY_CRAZY
		// ignore network saturation check during loading the game
		_LOG(TEXT("======== WARNING! Network saturation check is ignored now ========"));
		UNetConnection::bIgnoreNetReadyCheck = TRUE;
#endif

		GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
		return;
	}

	if (!_StateController->AmIHost() /*&& !bWasPlaying*/)
	{
		GetAvaNetHandler()->LoadingCheckTime = appSeconds() + LOADING_CHECK_TIME;
		GetAvaNetHandler()->LoadingCutOffPerc = 0;
	}

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);

	// {{ 20071123 dEAthcURe|PL preloadClient 
	#ifdef EnableClientPreloading
	FURL OpenURL = FURL(0x0);
	FMapInfo *Info = _StateController->GetCurrentMap();
	if (Info) {
		OpenURL.Map = Info->FileName;
	}	
	OpenURL.AddOption(TEXT("preloadingMapClient"));
	debugf(TEXT("%s"), *OpenURL.String());
	GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
	#endif
	// }} 20071123 dEAthcURe|PL preloadClient 
}

// {{ 20080109 dEAthcURe|HP preloading client는 udp connect to server를 preloading 후에 한다.
#ifdef EnableClientPreloading
extern bool GbClientPreloading;
extern bool GbClientPreloaded;
extern bool GbHostReadyNtfPended;
#endif
// }} 20080109 dEAthcURe|HP preloading client는 udp connect to server를 preloading 후에 한다.

// 방장이 접속을 받을 준비가 끝났음. 클라이언트 로딩 시작.
void PM::GAME::READY_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_ROOM || _StateController->AmIHost() || _StateController->RoomInfo.RoomInfo.state.playing != RIP_PLAYING) {
		// {{ 20070226 dEAthcURe|HM
		#ifdef EnableHostMigration
		if(_StateController->GetNetState() == _AN_INGAME) { // HM?
			FURL OpenURL;
			_StateController->GetOpenURL(OpenURL);			

			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);
			_LOG(TEXT("Opening URL... %s"), *(OpenURL.String()));

			// {{ 20070501 dEAthcURe|HM nonblock p2p client
			GavaNetClient->ConnectURL = OpenURL;
			_ahead_deinitSocket();			
			if(_ahead_initSocket() && GavaNetClient) {
				if(GavaNetClient->pp2pConn) delete GavaNetClient->pp2pConn;
				GavaNetClient->pp2pConn = new p2pConn_t(GavaNetClient);
				if(GavaNetClient->pp2pConn) {
					GavaNetClient->pp2pConn->setId(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);			
					debugf(TEXT("          <---------- [PM::GAME::READY_NTF::Proc] localPort %d"), _ahead_port);
					GavaNetClient->pp2pConn->initUdp(_ahead_socket->Socket, _ahead_port);				
				}
				OpenURL.AddOption(TEXT("p2p_wait_svr_connect"));
			}				
			// }} 20070501 dEAthcURe|HM nonblock p2p client

			GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
		}
		#endif
		// }} 20070226 dEAthcURe|HM
		return;
	}

	// Ignore it if I am not ready
	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (!MyInfo || !MyInfo->IsPlaying())
		return;

	// Set default map configuration to the map to load to reduce loading time
	//FMapInfo *Info = _StateController->GetCurrentMap();
	//if (Info)
	//{
	//	FURL::DefaultMap = Info->FileName;
	//}

	FURL OpenURL;
	_StateController->GetOpenURL(OpenURL);

	if (OpenURL.IsLocalInternal())
	{
		FString PackageFilename;
		if (!GPackageFileCache->FindPackageFile(*OpenURL.Map, NULL, PackageFilename))
		{
			_LOG(TEXT("ERROR: The map '%s' does not exist."), *OpenURL.Map);

			CavaNetEventHandler::FinishLoadingProgress(FALSE);

			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT("failed"), TEXT(""), 0, 0);

			// disconnect?
			//GetAvaNetRequest()->LeaveGame();
			return;
		}
	}

	// {{ 20080109 dEAthcURe|HP preloading client는 udp connect to server를 preloading 후에 한다.
	if(GbClientPreloading && !GbClientPreloaded) {
		debugf(TEXT("          <---------- [dEAthcURe|HP] udp connect to server pended due to preloading client"));
		GbHostReadyNtfPended = true;
		return;
	}
	// }} 20080109 dEAthcURe|HP preloading client는 udp connect to server를 preloading 후에 한다.

	_StateController->GoToState(_AN_INGAME);
	CavaNetEventHandler::StartLoadingProgress();

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);
	_LOG(TEXT("Opening URL... %s"), *(OpenURL.String()));

	// {{ 20070413 dEAthcURe|HP nonblock p2p client	
	GavaNetClient->ConnectURL = OpenURL;
	_ahead_deinitSocket();			
	if(_ahead_initSocket() && GavaNetClient) {
		if(GavaNetClient->pp2pConn) delete GavaNetClient->pp2pConn;
		GavaNetClient->pp2pConn = new p2pConn_t(GavaNetClient);
		if(GavaNetClient->pp2pConn) {
			GavaNetClient->pp2pConn->setId(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);			
			debugf(TEXT("          <---------- [PM::GAME::READY_NTF::Proc] localPort %d"), _ahead_port);
			GavaNetClient->pp2pConn->initUdp(_ahead_socket->Socket, _ahead_port);				
		}
		OpenURL.AddOption(TEXT("p2p_wait_svr_connect"));
	}	
	// }} 20070413 dEAthcURe|HP nonblock p2p client

	GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
	//FString Cmd = FString::Printf(TEXT("OPEN %s"), *(OpenURL.String()));
	//GEngine->Exec(*Cmd);
}
// {{ 20080109 dEAthcURe|HP preloading client는 udp connect to server를 preloading 후에 한다.
void _browsePreloadedClient(void)
{
	FURL OpenURL;
	_StateController->GetOpenURL(OpenURL);

	_StateController->GoToState(_AN_INGAME);
	CavaNetEventHandler::StartLoadingProgress();

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);
	_LOG(TEXT("Opening URL... %s"), *(OpenURL.String()));

	// {{ 20070413 dEAthcURe|HP nonblock p2p client	
	GavaNetClient->ConnectURL = OpenURL;
	_ahead_deinitSocket();			
	if(_ahead_initSocket() && GavaNetClient) {
		if(GavaNetClient->pp2pConn) delete GavaNetClient->pp2pConn;
		GavaNetClient->pp2pConn = new p2pConn_t(GavaNetClient);
		if(GavaNetClient->pp2pConn) {
			GavaNetClient->pp2pConn->setId(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);			
			debugf(TEXT("          <---------- [PM::GAME::READY_NTF::Proc] localPort %d"), _ahead_port);
			GavaNetClient->pp2pConn->initUdp(_ahead_socket->Socket, _ahead_port);				
		}
		OpenURL.AddOption(TEXT("p2p_wait_svr_connect"));
	}	
	// }} 20070413 dEAthcURe|HP nonblock p2p client

	GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
}
// }} 20080109 dEAthcURe|HP preloading client는 udp connect to server를 preloading 후에 한다.
#ifdef org
void PM::GAME::READY_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_ROOM || _StateController->AmIHost() || _StateController->RoomInfo.RoomInfo.state.playing != 2) {
		// {{ 20070226 dEAthcURe|HM
		#ifdef EnableHostMigration
		if(_StateController->GetNetState() == _AN_INGAME) { // HM?
			FURL OpenURL;
			_StateController->GetOpenURL(OpenURL);			

			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);
			_LOG(TEXT("Opening URL... %s"), *(OpenURL.String()));

			GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
		}
		#endif
		// }} 20070226 dEAthcURe|HM
		return;
	}

	// Set default map configuration to the map to load to reduce loading time
	//FMapInfo *Info = _StateController->GetCurrentMap();
	//if (Info)
	//{
	//	FURL::DefaultMap = Info->FileName;
	//}

	CavaNetEventHandler::StartLoadingProgress();

	FURL OpenURL;
	_StateController->GetOpenURL(OpenURL);

	if (OpenURL.IsLocalInternal())
	{
		FString PackageFilename;
		if (!GPackageFileCache->FindPackageFile(*OpenURL.Map, NULL, PackageFilename))
		{
			_LOG(TEXT("ERROR: The map '%s' does not exist."), *OpenURL.Map);

			CavaNetEventHandler::FinishLoadingProgress(FALSE);

			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT("failed"), TEXT(""), 0, 0);
			// disconnect?
			return;
		}
	}

	_StateController->GoToState(_AN_INGAME);

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, TEXT(""), TEXT(""), 1, 0);
	_LOG(TEXT("Opening URL... %s"), *(OpenURL.String()));

	GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
	//FString Cmd = FString::Printf(TEXT("OPEN %s"), *(OpenURL.String()));
	//GEngine->Exec(*Cmd);
}
#endif


// 방장에게 클라이언트가 p2p 접속 요청
void PM::GAME::JOIN_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStatePlaying() || !_StateController->AmIHost()) {
		_LOG(TEXT("NetState=%d, bHost"), _StateController->GetNetState(), _StateController->AmIHost());
		return;
	}

	TMSG msg(pData);

	SOCKADDR_IN addrLocal, addrPublic;
	addrLocal.sin_family = AF_INET;
	addrLocal.sin_addr.S_un.S_addr = msg.Data().hostInfo.intAddr.ipAddress;
	addrLocal.sin_port = msg.Data().hostInfo.intAddr.port;

	addrPublic.sin_family = AF_INET;
	addrPublic.sin_addr.S_un.S_addr = msg.Data().hostInfo.extAddr.ipAddress;
	addrPublic.sin_port = msg.Data().hostInfo.extAddr.port;

	// {{ 20060928 dEAthcURe
#ifdef EnableP2pConn
	debugf(TEXT("[PM::GAME::JOIN_NTF::Proc] p2pConn=0x%x bConnected=%s"), GavaNetClient->pp2pConn, GavaNetClient->pp2pConn->bConnected?TEXT("true"):TEXT("false"));
	if(GavaNetClient->pp2pConn) { // if(GavaNetClient->pp2pConn && GavaNetClient->pp2pConn->bConnected) {
		GavaNetClient->pp2pConn->RemovePeer(msg.Data().idAccount);
		GavaNetClient->pp2pConn->RegisterPeer(msg.Data().idAccount, 0x0);
		GavaNetClient->pp2pConn->setPeerAddress(msg.Data().idAccount, (SOCKADDR*)&addrLocal, (SOCKADDR*)&addrPublic); //GavaNetClient->pp2pConn->RequestPeerAddr(msg.Data().idAccount); // [!] 20070224
		TCHAR localstr[256], publicstr[256];
		mbstowcs(localstr, inet_ntoa(addrLocal.sin_addr), 255);
		mbstowcs(publicstr, inet_ntoa(addrPublic.sin_addr), 255);
		debugf(TEXT("          <---------- [PM::GAME::JOIN_NTF::Proc] setPeerAddr %d %s:%d %s:%d"), 
			msg.Data().idAccount, localstr, addrLocal.sin_port, publicstr, addrPublic.sin_port);

		GavaNetClient->pp2pConn->RequestConnectPeer(msg.Data().idAccount);
		debugf(TEXT("          <---------- [PM::GAME::JOIN_NTF::Proc] RequestConnectPeer %d"), msg.Data().idAccount);
		
		/* 20070220 disabled 굳이 확인안해도 heartbeat loop에서 거시기 된다
		sockaddr addr;
		if(GavaNetClient->pp2pConn->getPeerAddress(msg.Data().idAccount, 20000, &addr)) {
			TCHAR str[256];
			mbstowcs(str, inet_ntoa(((SOCKADDR_IN*)&addr)->sin_addr), 255);
			debugf(TEXT("          <---------- [PM::GAME::JOIN_NTF::Proc] peer %d registered %s %d"), msg.Data().idAccount, str, ntohs(((SOCKADDR_IN*)&addr)->sin_port));
		}
		else {
			debugf(TEXT("          <---------- [PM::GAME::JOIN_NTF::Proc] getPeerAddr of %d failed"), msg.Data().idAccount);
		}
		*/
	}
	else {
		debugf(TEXT("          <---------- [PM::GAME::JOIN_NTF::Proc] UDP server not connected"));
	}
#endif
	// }} 20060928 dEAthcURe
}


void PM::GAME::START_COUNT_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_ROOM || !_StateController->RoomInfo.IsValid())
		return;
	if (_StateController->IsCountingDown())
		return;

	_StateController->RoomHostKickCount = -1;

	if (_StateController->RoomInfo.RoomInfo.state.playing == RIP_WAIT && _StateController->RoomInfo.RoomInfo.setting.autoBalance > 0)
	{
		FString Msg = Localize(TEXT("UIRoomScene"), TEXT("Msg_AutoBalancing"), TEXT("AVANET"));
		_StateController->LogChatConsole(*Msg, EChat_ReadyRoom);
	}

	_StateController->StartCountDown();
}


void PM::GAME::CANCEL_COUNT_NTF::Proc(_LPMSGBUF pData)
{
	if (_StateController->GetNetState() != _AN_ROOM)
		return;

	if (!_StateController->RoomInfo.IsValid())
		return;

	if (_StateController->IsCountingDown())
	{
		_StateController->CancelCountDown();
		_StateController->RoomInfo.RoomInfo.state.playing = RIP_WAIT;
	}

	_StateController->CheckRoomHostKickCondition();
}


void _UpdateItemGauge(void *Info, _ITEM_UPDATE_TYPE_ Flag, void *Result, TID_EQUIP_SLOT idSlot)
{
	switch (Flag)
	{
	case _IUT_ITEM_INSERT_:		// insert
		break;
	case _IUT_ITEM_DELETE_:		// delete
		//if (Result)
		//{
		//	ITEM_INFO *Item = (ITEM_INFO*)Result;
		//	ITEM_DESC *Desc = _ItemDesc().GetItem(Item->id);

		//	if (Desc)
		//		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_DeleteItem, Desc->GetName(), TEXT(""), Item->id, 0);

		//	_LOG(TEXT("Item deleted (%s); slot = %d, id = %d, sn = %I64d"),
		//					(Desc ? Desc->GetName() : TEXT("Unknown")), idSlot, Item->id, Item->sn);
		//}
		break;
	case _IUT_ITEM_GAUGE_UPDATE_:		// update gauge
		if (Result)
		{
			ITEM_INFO *Item = (ITEM_INFO*)Result;
			ITEM_DESC *Desc = _ItemDesc().GetItem(Item->id);

			//_LOG(TEXT("Item gauge updated (%s); id = %d, sn = %I64d, gauge type = %d, gauge = %d"),
			//				(Desc ? Desc->GetName() : TEXT("Unknown")), Item->id, Item->sn, Desc->gaugeType, Item->limit);

			if ((Desc->gaugeType == _IGT_MAINTENANCE || Desc->gaugeType == _IGT_DURABILITY))
			{
				Item->limit = Clamp<LONG>(Item->limit, 0, ITEM_LIMIT_INITED);
			}

			_LOG(TEXT("Item gauge updated (%s); id = %d, sn = %I64d, gauge type = %d, gauge = %d"),
							(Desc ? Desc->GetName() : TEXT("Unknown")), Item->id, Item->sn, Desc->gaugeType, Item->limit);
		}
		break;
	case _IUT_CUSTOM_ITEM_DELETE_:		// delete custom slot
		//if (Result)
		//{
		//	CUSTOM_ITEM_INFO *Item = (CUSTOM_ITEM_INFO*)Result;
		//	CUSTOM_ITEM_DESC *Desc = _ItemDesc().GetCustomItem(Item->id);
		//	ITEM_DESC *Weapon = _StateController->PlayerInfo.IsValid() ?
		//							_ItemDesc().GetItem( _StateController->PlayerInfo.Inven.GetItemIdToSN(Item->item_sn) ) :
		//							NULL;

		//	if (Desc && Weapon)
		//		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_DeleteCustom, Desc->GetName(), Weapon->GetName(), Item->id, Weapon->id );

		//	_LOG(TEXT("Custom item deleted (%s @ %s); slot = %d, item sn = %I64d, custom slot = %d, custom item id = %d"),
		//				(Weapon ? Weapon->GetName() : TEXT("Unknown")), (Desc ? Desc->GetName() : TEXT("Unknown")), idSlot, Item->item_sn, Item->slot, Item->id);
		//}
		break;
	case _IUT_CUSTOM_ITEM_GAUGE_UPDATE_:		// update custom slot
		if (Result)
		{
			CUSTOM_ITEM_INFO *Item = (CUSTOM_ITEM_INFO*)Result;
			CUSTOM_ITEM_DESC *Desc = _ItemDesc().GetCustomItem(Item->id);
			ITEM_DESC *Weapon = _StateController->PlayerInfo.IsValid() ?
									_ItemDesc().GetItem( _StateController->PlayerInfo.Inven.GetItemIdToSN(Item->item_sn) ) :
									NULL;

			//_LOG(TEXT("Custom item gauge updated (%s @ %s); slot = %d, item sn = %I64d, custom slot = %d, custom item id = %d, gauge type = %d, gauge = %d"),
			//			(Weapon ? Weapon->GetName() : TEXT("Unknown")), (Desc ? Desc->GetName() : TEXT("Unknown")), idSlot, Item->item_sn, Item->slot, Item->id, Desc->gaugeType, Item->limit);

			if ((Desc->gaugeType == _IGT_MAINTENANCE || Desc->gaugeType == _IGT_DURABILITY))
			{
				Item->limit = Clamp<LONG>(Item->limit, 0, ITEM_LIMIT_INITED);
			}

			_LOG(TEXT("Custom item gauge updated (%s @ %s); slot = %d, item sn = %I64d, custom slot = %d, custom item id = %d, gauge type = %d, gauge = %d"),
						(Weapon ? Weapon->GetName() : TEXT("Unknown")), (Desc ? Desc->GetName() : TEXT("Unknown")), idSlot, Item->item_sn, Item->slot, Item->id, Desc->gaugeType, Item->limit);
		}
		break;
	case _IUT_EFFECT_ITEM_DELETE_:
//		if (Result)
//		{
//			EFFECT_ITEM_INFO *Item = (EFFECT_ITEM_INFO*)Result;
//			EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(Item->id);
//
//			if (Desc && Item)
//				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_DeleteEffect, Desc->GetName(), TEXT(""), Item->id, 0);
//
//			_LOG(TEXT("Effect item deleted (%s); id = %d, item sn = %I64d, effect type = %d, effect value = %d"),
//						(Desc ? Desc->GetName() : TEXT("Unknown")), Item->id, Item->item_sn, Desc->effectInfo.effectType, Desc->effectInfo.effectValue);
//
//#ifdef _TEMP_RESULT_LOG
//			_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Effect item deleted (%s)"), (Desc ? Desc->GetName() : TEXT("Unknown"))),
//							EChat_InGameSystem);
//#endif
//		}
		break;
	case _IUT_EFFECT_ITEM_GAUGE_UPDATE_:
		if (Result)
		{
			EFFECT_ITEM_INFO *Item = (EFFECT_ITEM_INFO*)Result;
			EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(Item->id);

			Item->limit = Clamp<LONG>(Item->limit, 0, ITEM_LIMIT_INITED);

			_LOG(TEXT("Effect item gauge updated (%s); effect type = %d, effect value = %d, gauge = %d"),
						(Desc ? Desc->GetName() : TEXT("Unknown")), Desc->effectInfo.effectType, Desc->effectInfo.effectValue, Item->limit);

#ifdef _TEMP_RESULT_LOG
			EFFECT_ITEM_INFO *OldItem = NULL;
			for (INT i = 0; i < MAX_EFFECT_INVENTORY_SIZE; ++i)
			{
				if (_StateController->PlayerInfo.OldItemInfo.effectInven[i].id == Item->id)
					OldItem = &_StateController->PlayerInfo.OldItemInfo.effectInven[i];
			}
			if (OldItem)
				_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Effect item gauge updated (%s) : %d -> %d (%+d)"),
													(Desc ? Desc->GetName() : TEXT("Unknown")), OldItem->limit, Item->limit, Item->limit - OldItem->limit),
								EChat_InGameSystem);
			else
				_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Effect item gauge updated (%s) : %d"),
													(Desc ? Desc->GetName() : TEXT("Unknown")), Item->limit),
								EChat_InGameSystem);
#endif
		}
		break;
	}
}

void PM::GAME::UPDATE_SCORE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	//if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INGAME)
	//	return;
	//if (!_StateController->RoomInfo.IsValid())
	//	return;
	if (_StateController->GetChannelSetting(EChannelSetting_UpdatePlayerScore) == 0)
		return;

	TMSG msg(pData);
	PM::_MsgBuffer<PLAYER_RESULT_INFO> ScoreBuf(msg, msg.Data().playerScore);

	if (ScoreBuf.GetCount() != 1)
		return;

	PLAYER_RESULT_INFO &Info = ScoreBuf[0];

	if (_StateController->PlayerInfo.PlayerInfo.idAccount != Info.idAccount)
		return;

	_StateController->LastResultInfo.PlayerResultInfo = Info;
	if (_StateController->LastResultInfo.InfoLevel < 1)
		_StateController->LastResultInfo.InfoLevel = 1;

	_StateController->PlayerInfo.PlayerInfo.scoreInfo.AddResult(Info);

	_StateController->PlayerInfo.PlayerInfo.currentClass = Info.currentClass;
	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
	if (MyInfo)
	{
		MyInfo->RoomPlayerInfo.currentClass = Info.currentClass;
		//MyInfo->PlayerInfo.currentClass = Info.currentClass;
	}

	//////////////////////////////////////////////////////////////////////////////
#ifdef _GAME_ITEM_PROMOTION
	if (_StateController->GetChannelSetting(EChannelSetting_AllowEventBonus) > 0 &&
		Info.classResultInfo[0].playTime + Info.classResultInfo[1].playTime + Info.classResultInfo[2].playTime >= 3 * 60)
	{
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_EventCoin, 1);
	}
#endif
	//////////////////////////////////////////////////////////////////////////////

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("score"), TEXT(""), 0, 0);

	_StateController->PlayerInfo.BackupItemInfo();

	CInventory &Inven = _StateController->PlayerInfo.Inven;
	Inven.UpdateItemGauge((void*)MyInfo, Info.classResultInfo[_CLASS_POINTMAN].playTime,
						Info.classResultInfo[_CLASS_RIFLEMAN].playTime, Info.classResultInfo[_CLASS_SNIPER].playTime, _UpdateItemGauge,
						_StateController->ChannelInfo.IsMyClanChannel());

	// empty invalidated grenade slots
	_StateController->PlayerInfo.Inven.RebuildEffect();
	_StateController->PlayerInfo.CheckGrenadeSlots();

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("item"), TEXT(""), 0, 0);

#if !FINAL_RELEASE
	_LOG(TEXT("Updating score... gameWin = %d, roundWin = %d, roundDefeat = %d, disconnectCount = %d"), Info.gameWin, Info.roundWin, Info.roundDefeat, Info.disconnectCount);
	_LOG(TEXT("score = (%d, %d, %d, %d)"), Info.score.attacker, Info.score.defender, Info.score.leader, Info.score.tactic);
	_LOG(TEXT("Pointman; playTime = %d, sprintTime = %d, takenDamage = %d, playRound = %d, headshotCount = %d, headshotKillCount = %d, killCount %d"),
				Info.classResultInfo[0].playTime, Info.classResultInfo[0].sprintTime, Info.classResultInfo[0].takenDamage, Info.classResultInfo[0].playRound,
				Info.classResultInfo[0].headshotCount, Info.classResultInfo[0].headshotKillCount, Info.classResultInfo[0].killCount);
	_LOG(TEXT("Rifleman; playTime = %d, sprintTime = %d, takenDamage = %d, playRound = %d, headshotCount = %d, headshotKillCount = %d, killCount %d"),
				Info.classResultInfo[1].playTime, Info.classResultInfo[1].sprintTime, Info.classResultInfo[1].takenDamage, Info.classResultInfo[1].playRound,
				Info.classResultInfo[1].headshotCount, Info.classResultInfo[1].headshotKillCount, Info.classResultInfo[1].killCount);
	_LOG(TEXT("Sniper; playTime = %d, sprintTime = %d, takenDamage = %d, playRound = %d, headshotCount = %d, headshotKillCount = %d, killCount %d"),
				Info.classResultInfo[2].playTime, Info.classResultInfo[2].sprintTime, Info.classResultInfo[2].takenDamage, Info.classResultInfo[2].playRound,
				Info.classResultInfo[2].headshotCount, Info.classResultInfo[2].headshotKillCount, Info.classResultInfo[2].killCount);
	_LOG(TEXT("deathCount = %d, helmetDropCount = %d, bulletMultiKillCount = %d, grenadeMultiKillCount = %d"),
				Info.deathCount, Info.helmetDropCount, Info.bulletMultiKillCount, Info.grenadeMultiKillCount);
	_LOG(TEXT("roundTopKillCount = %d, roundTopHeadshotKillCount = %d, topLevelKillCount = %d, higherLevelKillCount = %d"),
				Info.roundTopKillCount, Info.roundTopHeadshotKillCount, Info.topLevelKillCount, Info.higherLevelKillCount);
	_LOG(TEXT("noDamageWinCount = %d, topScoreCount = %d, noDeathRoundCount = %d, teamKillCount = %d"),
				Info.noDamageWinCount, Info.topScoreCount, Info.noDeathRoundCount, Info.teamKillCount);
	_LOG(TEXT("weaponFireCount; Pistol = %d, SMG = %d, AR = %d, SR = %d"), Info.weaponFireCount[0], Info.weaponFireCount[1], Info.weaponFireCount[2], Info.weaponFireCount[3]);
	_LOG(TEXT("weaponHitCount; Pistol = %d, SMG = %d, AR = %d, SR = %d"), Info.weaponHitCount[0], Info.weaponHitCount[1], Info.weaponHitCount[2], Info.weaponHitCount[3]);
	_LOG(TEXT("weaponHeadshotCount; Pistol = %d, SMG = %d, AR = %d, SR = %d"),
				Info.weaponHeadshotCount[0], Info.weaponHeadshotCount[1], Info.weaponHeadshotCount[2], Info.weaponHeadshotCount[3]);
#endif
}


void PM::GAME::LOADING_PROGRESS_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;

	if (!Room.PlayerList.IsEmpty(def.idSlot))
	{
		FRoomPlayerInfo &Info = Room.PlayerList.PlayerList[def.idSlot];
		_LOG(TEXT("[%d]%s -> %d%%"), def.idSlot, Info.RoomPlayerInfo.nickname, def.progress);
		if (def.progress >= 100)
		{
			// 로딩 완료; Playing 상태로
			Info.RoomPlayerInfo.bReady = _READY_PLAYING;
			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Ready, TEXT(""), TEXT(""), def.idSlot, _READY_PLAYING);

#ifdef EnableRttTest
			FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
			if (MyInfo && MyInfo->RttValue < 0 && !_StateController->IsStatePlaying() && !_StateController->AmIStealthMode() && def.idSlot == Room.HostIdx)
			{
				_LOG(TEXT("Restarting RTT test..."));
				// 방장의 로딩이 끝났으므로 RTT 테스트 재개
				GavaNetClient->p2pNetTester.addToTestList(Info.RoomPlayerInfo.idAccount);
				_LOG(TEXT("[dEAthcURe] activating RTT test..."));
				GavaNetClient->p2pNetTester.activateRttTest(true, _StateController->AmIHost()==TRUE?true:false, 5000); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive();
			}
#endif
		}

		//if (_StateController->RoomInfo.HostIdx == msg.Data().idSlot)
		//{
		//}
		//else
		{
			for (INT i = 0; i < GetAvaNetHandler()->RoomStartingPlayerList.Num(); ++i)
			{
				if (GetAvaNetHandler()->RoomStartingPlayerList(i).AccountID == Info.RoomPlayerInfo.idAccount)
				{
					_LOG(TEXT("Loading Progress: [%d]%d%%(%d)"), def.idSlot, def.progress, def.step);
					++GetAvaNetHandler()->RoomStartingPlayerList(i).UpdateCount;
					GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingProgress = def.progress;

					if (def.progress >= 100)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress,
												def.progress == 255 ? TEXT("cancel") : TEXT("complete"), TEXT(""), i, def.progress);
						GetAvaNetHandler()->RoomStartingPlayerList.Remove(i);
						break;
					}

					if (GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingStepCount < def.step)
					{
						GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingStepCount = def.step;
						GetAvaNetHandler()->RoomStartingPlayerList(i).UpdateTime = appSeconds();

						GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("progress"), TEXT(""), i, def.progress);
					}

					//if (GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingStepCount >= def.step)
					//{
					//	// this client's loading sequence is not progressing, so skip it
					//	GetAvaNetHandler()->RoomStartingPlayerList.Remove(i);
					//	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("skip check"), TEXT(""), i, def.progress);
					//}
					//else
					//{
					//	GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingStepCount = def.step;
					//	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("progress"), TEXT(""), i, def.progress);
					//}
					break;
				}
			}
		}

		GCallbackEvent->Send(CALLBACK_PlayerListUpdated);
	}

	if (_StateController->AmIHost() && GetAvaNetHandler()->RoomStartingPlayerList.Num() == 0)
	{
		GetAvaNetHandler()->LoadingCheckTime = 0.0;
		GetAvaNetHandler()->LoadingCutOffPerc = -1;

#ifdef _BT_TEST_BY_CRAZY
		_LOG(TEXT("======== Network saturation check is turned on now ========"));
		UNetConnection::bIgnoreNetReadyCheck = FALSE;
#endif
	}
}


void PM::GAME::LEAVE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	TMSG msg(pData);

	FRoomPlayerInfo *PlayerInfo = _StateController->RoomInfo.PlayerList.Find(msg.Data().idAccount);
	if (!PlayerInfo)
		return;

	PlayerInfo->RoomPlayerInfo.bReady = _READY_NONE;

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Leave, PlayerInfo->RoomPlayerInfo.nickname, TEXT(""), 0, 0);

	for (INT i = 0; i < GetAvaNetHandler()->RoomStartingPlayerList.Num(); ++i)
	{
		if (GetAvaNetHandler()->RoomStartingPlayerList(i).AccountID == PlayerInfo->RoomPlayerInfo.idAccount)
		{
			GetAvaNetHandler()->RoomStartingPlayerList.Remove(i);
			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("cancel"), TEXT(""), i, 0);
			break;
		}
	}

	GCallbackEvent->Send(CALLBACK_PlayerListUpdated);
}


void PM::GAME::SKILL_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	//if (!_StateController->IsStateInRoom())
	//	return;
	//if (!_StateController->RoomInfo.IsValid())
	//	return;

	FRoomPlayerInfo *PlayerInfo = _StateController->GetMyRoomPlayerInfo();
	//if (!PlayerInfo)
	//	return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	for (INT i = 0; i < _CLASS_MAX; ++i)
	{
		if (def.skillInfo.skill[i] > 0)
		{
			_StateController->PlayerInfo.PlayerInfo.skillInfo.skill[i] |= def.skillInfo.skill[i];
			if (PlayerInfo)
				PlayerInfo->RoomPlayerInfo.skillInfo.skill[i] |= def.skillInfo.skill[i];

			for (INT j = 0; j < 16; ++j)
			{
				if (def.skillInfo.skill[i] & (1 << j))
				{
					_StateController->LastResultInfo.SkillList[i].Push(j);
					_LOG(TEXT("Skill received = %s"), *GetAvaNetRequest()->GetSkillName(i, j+1));
#ifdef _TEMP_RESULT_LOG
					_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Skill received = %s"), *GetAvaNetRequest()->GetSkillName(i, j+1), EChat_InGameSystem));
#endif

					GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("skill"), TEXT(""), i, j);
				}
			}
		}
	}

	if (_StateController->LastResultInfo.InfoLevel < 1)
		_StateController->LastResultInfo.InfoLevel = 1;
}


void PM::GAME::RESULT_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	//if (!_StateController->IsStateInRoom())
	//	return;
	//if (!_StateController->RoomInfo.IsValid())
	//	return;

	FRoomPlayerInfo *PlayerInfo = _StateController->GetMyRoomPlayerInfo();
	//if (!PlayerInfo)
	//	return;

	//GetAvaNetHandler()->CollectLastResultInfo();

	TMSG msg(pData);
	DEF &def = msg.Data();

	////////////////////////////////////////////////////////////////////////////////////
	// 레벨 업
	_StateController->LastResultInfo.Level = def.level;
	if (def.level > 0)
	{
		_StateController->PlayerInfo.PlayerInfo.level += def.level;
		if (PlayerInfo)
		{
			PlayerInfo->PlayerInfo.level += def.level;
			PlayerInfo->RoomPlayerInfo.level += def.level;
		}

		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_Level, 1);

		_LOG(TEXT("Level up!"));
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("level"), TEXT(""), _StateController->PlayerInfo.PlayerInfo.level, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// 이벤트

#ifdef _HAPPY_NEW_YEAR_EVENT
	if (_StateController->GetChannelSetting(EChannelSetting_AllowEventBonus) > 0)
	{
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_EventXP, 30);
	}
#endif

	////////////////////////////////////////////////////////////////////////////////////
	// 경험치
	_StateController->PlayerInfo.PlayerInfo.xpProgress = def.xpProgress;
	_StateController->LastResultInfo.xp = def.xp;

	// 경험치 부스트 아이템
	if (def.boostXP > 0)
	{
		_StateController->LastResultInfo.xp += def.boostXP;
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_XP, def.boostXP);
	}

	// PC방 경험치 보너스
	if (def.pcBangXP > 0)
	{
		_StateController->LastResultInfo.xp += def.pcBangXP;
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_PCBangXP, def.pcBangXP);
	}

	_StateController->PlayerInfo.PlayerInfo.xp += _StateController->LastResultInfo.xp;

	INT BonusXPPerc = _StateController->PlayerInfo.GetBonusXPPerc();
	INT PcBangBonusXPPerc = _StateController->PlayerInfo.GetPcBangBonusXPPerc();

	_LOG(TEXT("XP; Base = %+d, XPBoost = %+d(%d%%), PcBangBonus = %+d(%d%%)]"),
		def.xp, def.boostXP, BonusXPPerc, def.pcBangXP, PcBangBonusXPPerc);
#ifdef _TEMP_RESULT_LOG
	_StateController->ChatMsgList.Add(*FString::Printf(TEXT("XP; Base = %+d, XPBoost = %+d(%d%%), PcBangBonus = %+d(%d%%)]"),
											def.xp, def.boostXP, BonusXPPerc, def.pcBangXP, PcBangBonusXPPerc),
									EChat_InGameSystem);
#endif

	// 전우 경험치
	_StateController->LastResultInfo.BIAFlag = def.biaXPFlag;
	if (def.biaXPFlag > _BIAXP_NONE)
	{
		_StateController->PlayerInfo.PlayerInfo.biaXP += def.biaXP;
		_LOG(TEXT("BIA XP = %+d"), def.biaXP);
#ifdef _TEMP_RESULT_LOG
		_StateController->ChatMsgList.Add(*FString::Printf(TEXT("BIA Xp = %+d"), def.biaXP), EChat_InGameSystem);
#endif

		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("bia xp"), TEXT(""), def.biaXP, 0);

		// 경험치 업
		if (def.biaXPFlag == _BIAXP_FULL && _StateController->PlayerInfo.PlayerInfo.biaXP >= BIA_EXP_MAX)
		{
			_StateController->PlayerInfo.PlayerInfo.biaXP -= BIA_EXP_MAX;

			_LOG(TEXT("BIA XP reached full! Receiving XP = %+d"), BIA_EXP_MAX);
#ifdef _TEMP_RESULT_LOG
			_StateController->ChatMsgList.Add(*FString::Printf(TEXT("BIA XP reached full! Receiving XP = %+d"), BIA_EXP_MAX), EChat_InGameSystem);
#endif

			_StateController->LastResultInfo.xp += BIA_EXP_MAX;
			_StateController->PlayerInfo.PlayerInfo.xp += BIA_EXP_MAX;

			_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_Brother, BIA_EXP_MAX);

			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("bia xp full"), TEXT(""), BIA_EXP_MAX, 0);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// 게임 머니
	_StateController->LastResultInfo.Money = def.money;
	if (def.money > 0)
	{
		_StateController->LastResultInfo.AddMsgInfo( LastResultMsgType_Money, def.money );

		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("money"), TEXT(""), def.money, 0);
	}

	// 유로 지급 아이템
	if (def.boostMoney > 0)
	{
		_StateController->LastResultInfo.Money += def.boostMoney;
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_MoneyBoost, def.boostMoney);

		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("money boost"), TEXT(""), def.boostMoney, 0);
	}
	if (def.pcBangMoney > 0)
	{
		_StateController->LastResultInfo.Money += def.pcBangMoney;
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_PCBangMoney, def.pcBangMoney);

		//GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("pcbang money bonus"), TEXT(""), def.pcBangMoney, 0);
	}

	_StateController->PlayerInfo.PlayerInfo.money += _StateController->LastResultInfo.Money;

	INT BonusMoneyPerc = _StateController->PlayerInfo.GetBonusMoneyPerc();
	INT PcBangBonusMoneyPerc = _StateController->PlayerInfo.GetPcBangBonusMoneyPerc();

	_LOG(TEXT("Money earned = %+d, MoneyBoost = %+d(%d%%), PCBangBonus = %+d(%d%%)"), def.money, def.boostMoney, BonusMoneyPerc, def.pcBangMoney, PcBangBonusMoneyPerc);
#ifdef _TEMP_RESULT_LOG
	_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Money earned = %+d, MoneyBoost = %+d(%d%%), PCBangBonus = %+d(%d%%)"),
													def.money, def.boostMoney, BonusMoneyPerc, def.pcBangMoney, PcBangBonusMoneyPerc),
										EChat_InGameSystem);
#endif


	////////////////////////////////////////////////////////////////////////////////////
	// 보급치
	_StateController->LastResultInfo.SupplyPoint = def.supplyPoint;

	// 보급치 부스트 아이템
	if (def.boostSupply > 0)
	{
		_StateController->LastResultInfo.SupplyPoint += def.boostSupply;
		_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_Supply, def.boostSupply);
	}

	INT BonusSupplyPerc = _StateController->PlayerInfo.GetBonusSupplyPerc();

	_LOG(TEXT("Supply; Base = %+d, Bonus = %+d(%d%%)"), def.supplyPoint, def.boostSupply, BonusSupplyPerc);
#ifdef _TEMP_RESULT_LOG
	_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Supply; Base = %+d, Bonus = %+d(%d%%)"),
											def.supplyPoint, def.boostSupply, BonusSupplyPerc),
									EChat_InGameSystem);
#endif

	//GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("supply"), TEXT(""), def.supplyPoint, 0);

	_StateController->PlayerInfo.PlayerInfo.supplyPoint += _StateController->LastResultInfo.SupplyPoint;

	// 보급 지급
	_StateController->LastResultInfo.SupplyCount = def.supplyCnt;
	if (def.supplyCnt > 0)
	{
		//for (INT i = 0; i < def.supplyCnt; ++i)
		{
			_StateController->PlayerInfo.PlayerInfo.supplyPoint -= SUPPLYPOINT_MAX;
			if (_StateController->PlayerInfo.PlayerInfo.supplyPoint < 0)
			{
				_StateController->PlayerInfo.PlayerInfo.supplyPoint = 0;
				//break;
			}

			INT Remain = 3 - (def.supplyCnt % 3);
			if (Remain == 3)
			{
				// 보급 물자
				_LOG(TEXT("Supply money and item received!"));
#ifdef _TEMP_RESULT_LOG
				_StateController->ChatMsgList.Add(TEXT("Supply money and item received!"), EChat_InGameSystem);
#endif

				_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_SupplyItem);

				GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("supply item"), TEXT(""), 0, 0);
			}
			else
			{
				// 보급 머니
				_LOG(TEXT("Supply money received! Next item supply = %d"), Remain);
#ifdef _TEMP_RESULT_LOG
				_StateController->ChatMsgList.Add(TEXT("Supply money received!"), EChat_InGameSystem);
#endif

				_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_SupplyMoney, Remain);

				GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("supply money"), TEXT(""), Remain, 0);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// 클랜 포인트
	if (_StateController->ChannelInfo.IsFriendlyGuildChannel())
	{
		if (_StateController->PlayerInfo.IsPcBang() || def.pcBangMoney > 0 || def.pcBangXP > 0)
		{
			_StateController->LastResultInfo.AddMsgInfo(LastResultMsgType_PCBangClanPoint, 0);
		}
	}

	if (_StateController->LastResultInfo.InfoLevel < 1)
		_StateController->LastResultInfo.InfoLevel = 1;

	_StateController->LastResultInfo.Dump();

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("player result"), TEXT(""), def.money, 0);
}


void PM::GAME::AWARD_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;
	//if (!_StateController->IsStateInRoom())
	//	return;
	//if (!_StateController->RoomInfo.IsValid())
	//	return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	if (def.award_id < 0 || def.award_id >= MAX_AWARD_PER_PLAYER)
		return;

	if (_StateController->PlayerInfo.PlayerInfo.awardInfo.info[def.award_id] < MAX_COUNT_PER_AWARD)
		++_StateController->PlayerInfo.PlayerInfo.awardInfo.info[def.award_id];
	_StateController->LastResultInfo.AwardList.Push(def.award_id);

	if (_StateController->LastResultInfo.InfoLevel < 1)
		_StateController->LastResultInfo.InfoLevel = 1;

	_LOG(TEXT("Award received = %d"), def.award_id);
#ifdef _TEMP_RESULT_LOG
	_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Award received = %d"), def.award_id), EChat_InGameSystem);
#endif

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("award"), TEXT(""), def.award_id, 0);
}


void PM::GAME::ITEM_UPDATE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	//if (!_StateController->RoomInfo.IsValid())
	//	return;

	TMSG msg(pData);

	PM::_MsgBuffer<ITEM_BASE_INFO> ItemBuf(msg, msg.Data().itemInfo);
	INT UpdateType = msg.Data().updateType;
	// UpdateType == 0 -> 보급 물자 지급 받음
	// UpdateType == 1 -> 진급 축하 아이템 지급
	// UpdateType == 2 -> 진급 축하 아이템(부스터) 지급 --> 부스터 아이템이 마지막에 내려오므로, 이 경우에 진급 축하 메시지 띄움

	for (INT i = 0; i < ItemBuf.GetCount(); ++i)
	{
		ITEM_BASE_INFO &Item = ItemBuf[i];

		if (IsEffectItem(Item.id))
		{
			EFFECT_ITEM_INFO *ItemTo = _StateController->PlayerInfo.Inven.GetEffectInven(Item.sn);

			if (ItemTo)
			{
				// current effect item is updated
				ItemTo->limit = ITEM_LIMIT_INITED;
				EFFECT_ITEM_DESC *ItemDesc = _ItemDesc().GetEffectItem(Item.id);
				if (ItemDesc)
				{
					_LOG(TEXT("Effect item updated; [%d]%s"), Item.id, ItemDesc->GetName());
#ifdef _TEMP_RESULT_LOG
					_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Effect item updated; [%d]%s"), Item.id, ItemDesc->GetName()), EChat_InGameSystem);
#endif

					if (UpdateType == 0)
						GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("effect item received"), ItemDesc->GetName(), Item.id, 0);
				}
			}
			else
			{
				// new item added
				EFFECT_ITEM_INFO NewItem;
				NewItem.id = Item.id;
				NewItem.item_sn = Item.sn;
				NewItem.limit = ITEM_LIMIT_INITED;
				if ( _StateController->PlayerInfo.Inven.AddEffectInven(&NewItem) )
				{
					EFFECT_ITEM_DESC *ItemDesc = _ItemDesc().GetEffectItem(Item.id);
					if (ItemDesc)
					{
						_LOG(TEXT("Effect item received; [%d]%s"), Item.id, ItemDesc->GetName());
#ifdef _TEMP_RESULT_LOG
						_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Effect item received; [%d]%s"), Item.id, ItemDesc->GetName()), EChat_InGameSystem);
#endif

						if (UpdateType == 0)
							GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("effect item received"), ItemDesc->GetName(), Item.id, 0);
					}
				}
			}
		}
		else if (IsEquipItem(Item.id) || IsWeaponItem(Item.id))
		{
			ITEM_INFO *ItemTo = NULL;
			if (IsEquipItem(Item.id))
			{
				ItemTo = _StateController->PlayerInfo.Inven.GetEquipInven(Item.sn);
			}
			else if (IsWeaponItem(Item.id))
			{
				ItemTo = _StateController->PlayerInfo.Inven.GetWeaponInven(Item.sn);
			}

			if (ItemTo)
			{
				// current item is updated
				ItemTo->limit = ITEM_LIMIT_INITED;
				ITEM_DESC *ItemDesc = _ItemDesc().GetItem(Item.id);
				if (ItemDesc)
				{
					_LOG(TEXT("Item updated; [%d]%s"), Item.id, ItemDesc->GetName());
#ifdef _TEMP_RESULT_LOG
					_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Item updated; [%d]%s"), Item.id, ItemDesc->GetName()), EChat_InGameSystem);
#endif

					if (UpdateType == 0)
						GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("item received"), ItemDesc->GetName(), Item.id, 0);
				}
			}
			else
			{
				// new item added
				ITEM_INFO NewItem;
				NewItem.id = Item.id;
				NewItem.sn = Item.sn;
				NewItem.limit = ITEM_LIMIT_INITED;
				NewItem.tag = _StateController->PlayerInfo.Inven.GetCustomItemTag(Item.id);
				if ( _StateController->PlayerInfo.Inven.AddInven(&NewItem) )
				{
					ITEM_DESC *ItemDesc = _ItemDesc().GetItem(Item.id);
					if (ItemDesc)
					{
						_LOG(TEXT("Item received; [%d]%s"), Item.id, ItemDesc->GetName());
#ifdef _TEMP_RESULT_LOG
						_StateController->ChatMsgList.Add(*FString::Printf(TEXT("Item received; [%d]%s"), Item.id, ItemDesc->GetName()), EChat_InGameSystem);
#endif

					if (UpdateType == 0)
						GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("item received"), ItemDesc->GetName(), Item.id, 0);
					}
				}
			}
		}
		else
		{
			_LOG(TEXT("Invalid item type received. id = %d"), Item.id);
		}
	}

	_StateController->PlayerInfo.Inven.RebuildEffect();
	_StateController->PlayerInfo.CheckGrenadeSlots();

	if (UpdateType == 2)
	{
		// 진급 축하 아이템
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LevelUpGrats, TEXT(""), TEXT(""), 0, 0);
	}
}

void PM::GAME::RESULT_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;
	if (!_StateController->RoomInfo.IsValid())
		return;

	///GetAvaNetHandler()->CollectLastResultInfo();

	TMSG msg(pData);
	DEF &def = msg.Data();

	_LOG(TEXT("Updating game result"));

	//appMemcpy(_StateController->LastResultInfo.RoomResultInfo, msg.Data().resultInfo, MAX_PLAYER_PER_ROOM * sizeof(ROOM_RESULT_INFO));
	_LOG(TEXT("LastResultInfo.RoomResultInfo"));
	//for (INT i = 0; i < _StateController->LastResultInfo.RoomResultInfo.Num(); ++i)
	//{
	//	if (_StateController->LastResultInfo.RoomResultInfo(i).IsValid())
	//	{
	//		_StateController->LastResultInfo.RoomResultInfo(i).xp = def.resultInfo[_StateController->LastResultInfo.RoomResultInfo(i).idSlot].xp;
	//		_StateController->LastResultInfo.RoomResultInfo(i).SupplyPoint = def.resultInfo[_StateController->LastResultInfo.RoomResultInfo(i).idSlot].supplyPoint;
	//		_LOG(TEXT("[%d] %s; XP = %+d, SupplyPoint = %+d"), _StateController->LastResultInfo.RoomResultInfo(i).idSlot,
	//														*_StateController->LastResultInfo.RoomResultInfo(i).Nickname,
	//														_StateController->LastResultInfo.RoomResultInfo(i).xp,
	//														_StateController->LastResultInfo.RoomResultInfo(i).SupplyPoint);
	//		break;
	//	}
	//}

#if !FINAL_RELEASE
	//for (INT i = 0; i < _StateController->LastResultInfo.RoomResultInfo.Num(); ++i)
	//{
	//	_LOG(TEXT("[%d] %s; Level = %d, Score = %d, Death = %d, XP = %+d, SupplyPoint = %+d %s %s"),
	//													_StateController->LastResultInfo.RoomResultInfo(i).idSlot,
	//													*_StateController->LastResultInfo.RoomResultInfo(i).Nickname,
	//													_StateController->LastResultInfo.RoomResultInfo(i).Level,
	//													_StateController->LastResultInfo.RoomResultInfo(i).Score,
	//													_StateController->LastResultInfo.RoomResultInfo(i).Death,
	//													_StateController->LastResultInfo.RoomResultInfo(i).xp,
	//													_StateController->LastResultInfo.RoomResultInfo(i).SupplyPoint,
	//													_StateController->LastResultInfo.RoomResultInfo(i).bLeader ? TEXT("(Leader)") : TEXT(""),
	//													_StateController->LastResultInfo.RoomResultInfo(i).bPcBang ? TEXT("(PC)") : TEXT("")
	//													);
	//}
#endif

	_StateController->LastResultInfo.RoomResultInfo.Empty();

	for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	{
		//if (_StateController->RoomInfo.PlayerList.IsEmpty(i))
		//	continue;
		if (def.resultInfo[i].level == 0)
			continue;

		//FLastResultInfo::FPlayerResultInfo *pInfo = _StateController->LastResultInfo.FindPlayer(i);
		//if (!pInfo)
		//	pInfo = new(_StateController->LastResultInfo.RoomResultInfo) FLastResultInfo::FPlayerResultInfo;
		//check(pInfo);
		FLastResultInfo::FPlayerResultInfo *pInfo = new(_StateController->LastResultInfo.RoomResultInfo) FLastResultInfo::FPlayerResultInfo;

		pInfo->idSlot = i;
		if (!_StateController->RoomInfo.PlayerList.IsEmpty(i))
		{
			pInfo->LastLevel = _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.level;
			pInfo->Nickname = _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.nickname;
		}
		pInfo->Score = def.resultInfo[i].score;
		pInfo->Death = def.resultInfo[i].death;
		pInfo->Level = def.resultInfo[i].level;
		pInfo->xp = def.resultInfo[i].xp;
		pInfo->SupplyPoint = def.resultInfo[i].supplyPoint;
		pInfo->bLeader = def.resultInfo[i].bLeader > 0;
		pInfo->biaXPFlag = def.resultInfo[i].biaXPFlag;

//#ifdef _TEMP_EFFECTITEM_SUPPORT
		for (INT x = 0; x < MAX_EFFECTSET_SIZE; ++x)
		{
			pInfo->AddEffect(def.resultInfo[i].effectList[x]);
		}
//#endif

		if (!_StateController->RoomInfo.PlayerList.IsEmpty(i))
		{
			FRoomPlayerInfo &Info = _StateController->RoomInfo.PlayerList.PlayerList[i];

			pInfo->bPcBang = (Info.RoomPlayerInfo.pcBang > 0);

			//if (def.resultInfo[i].currentClass != _CLASS_NONE)
			{
				Info.RoomPlayerInfo.currentClass = def.resultInfo[i].currentClass;
				if (_StateController->MyRoomSlotIdx == i)
				{
					// it's me
					_StateController->PlayerInfo.PlayerInfo.currentClass = def.resultInfo[i].currentClass;
					_StateController->PlayerInfo.PlayerInfo.lastTeam = Info.GetTeamID();

					// got full result
					if (_StateController->LastResultInfo.InfoLevel < 2)
						_StateController->LastResultInfo.InfoLevel = 2;
				}
			}
			if (def.resultInfo[i].level > pInfo->LastLevel)
			{
				Info.PlayerInfo.level = def.resultInfo[i].level;
				Info.RoomPlayerInfo.level = def.resultInfo[i].level;
			}

			appMemcpy(Info.RoomPlayerInfo.equipItem, def.resultInfo[i].equipItem, MAX_EQUIPSET_SIZE * sizeof(TID_ITEM));
			appMemcpy(Info.RoomPlayerInfo.weaponItem, def.resultInfo[i].weaponItem, MAX_WEAPONSET_SIZE * sizeof(ROOM_ITEM_INFO));
			appMemcpy(Info.RoomPlayerInfo.customItem, def.resultInfo[i].customItem, _CLASS_MAX * _CSI_MAX * sizeof(TID_ITEM));

			appMemcpy(&Info.RoomPlayerInfo.skillInfo, &def.resultInfo[i].skillInfo, sizeof(PLAYER_SKILL_INFO));

			// find if the player is my buddy or guild member and reset his full info flag to retrieve his full info again.
			Info.SetFullInfo(FALSE);
			if (_StateController->GuildInfo.IsValid())
			{
				FGuildPlayerInfo *GuildPlayer = _StateController->GuildInfo.PlayerList.Find(Info.PlayerInfo.idAccount);
				if (GuildPlayer)
				{
					GuildPlayer->GuildPlayerInfo.level = Info.PlayerInfo.level;
					GuildPlayer->SetFullInfo(FALSE);
				}
			}

			FBuddyInfo *Buddy = _Communicator().BuddyList.Find(Info.PlayerInfo.idAccount);
			if (Buddy)
			{
				Buddy->Level = Info.PlayerInfo.level;
				Buddy->SetFullInfo(FALSE);
			}

		}

		//if (_StateController->RoomInfo.PlayerList.IsEmpty(i))
		//{
		//	_LOG(TEXT("[%d] ** Empty **"), i);
		//}
		//else
		//{
			_LOG(TEXT("packet[%d] Score = %d, Death = %d, Level = %d, XP = %+d, SupplyPoint = %+d %s"),
				i, def.resultInfo[i].score, def.resultInfo[i].death, def.resultInfo[i].level,
				def.resultInfo[i].xp, def.resultInfo[i].supplyPoint, def.resultInfo[i].bLeader > 0 ? TEXT("(Leader") : TEXT(""));
			_LOG(TEXT("game[%d] Score = %d, Death = %d, Level = %d, XP = %+d, SupplyPoint = %+d %s"),
				i, pInfo->Score, pInfo->Death, pInfo->Level, pInfo->xp, pInfo->SupplyPoint, pInfo->bLeader > 0 ? TEXT("(Leader") : TEXT(""));
		//}
	}

	_StateController->LastResultInfo.TeamScore[0] = def.teamWinCount[0];
	_StateController->LastResultInfo.TeamScore[1] = def.teamWinCount[1];
	_LOG(TEXT("TeamScore[0] = %d, TeamScore[1] = %d"), def.teamWinCount[0], def.teamWinCount[1]);

	_StateController->LastResultInfo.Sort();

	_StateController->LastResultInfo.Dump();

	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_ResultUpdate, TEXT("room result"), TEXT(""), 0, 0);
}


void PM::GAME::CHAT_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	FRoomPlayerInfo *Player = Room.PlayerList.Find(def.idAccount);
	if (Player)
	{
		PM::_MsgString ChatMsg(msg, def.chatmsg);
		FString FilteredMsg = (LPCWSTR)ChatMsg;
		_WordCensor().ReplaceChatMsg((TCHAR*)*FilteredMsg);

		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Chat, Player->PlayerInfo.nickname, *FilteredMsg, Player->PlayerInfo.idAccount, def.team);

		//if (g_hostMigration.state != hmsNewClientPrepare)
		//{
		//	FString ChatStr = FString::Printf(TEXT("%s : %s"), Player->RoomPlayerInfo.nickname, *FilteredMsg);
		//	_StateController->LogChatConsole(*ChatStr);
		//}
	}
	else
	{
		_LOG(TEXT("Invalid room chat notification received."));
	}
}

void PM::GAME::REPORT_VOTE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->IsStateInRoom())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	FRoomInfo &Room = _StateController->RoomInfo;
	check(Room.IsValid());

	switch (def.command)
	{
	case VC_KICK:
		{
			UBOOL bVoted = (def.idCaller == _StateController->PlayerInfo.PlayerInfo.idAccount);
			if (!bVoted)
			{
				for (INT i = 0; i < MAX_PLAYER_PER_TEAM-1; ++i)
				{
					if (def.idVoter[i] == _StateController->PlayerInfo.PlayerInfo.idAccount)
					{
						bVoted = TRUE;
						break;
					}
				}
			}

#ifdef _APPLY_VOTE_FEE
			if (bVoted)
			{
				// 강퇴 투표에 참여했다. 강퇴비용 삭감.
				_StateController->PlayerInfo.PlayerInfo.money -= VOTE_FEE;
				if (_StateController->PlayerInfo.PlayerInfo.money < 0)
					_StateController->PlayerInfo.PlayerInfo.money = 0;

				GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
			}
#endif
		}
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::GAME::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(START_NTF)
		CASE_MSG_PROC(READY_NTF)
		CASE_MSG_PROC(JOIN_NTF)
		CASE_MSG_PROC(START_COUNT_NTF)
		CASE_MSG_PROC(CANCEL_COUNT_NTF)
		CASE_MSG_PROC(UPDATE_SCORE_NTF)
		CASE_MSG_PROC(LOADING_PROGRESS_NTF)
		CASE_MSG_PROC(LEAVE_NTF)
		CASE_MSG_PROC(SKILL_UPDATE_NTF)
		CASE_MSG_PROC(RESULT_UPDATE_NTF)
		CASE_MSG_PROC(AWARD_UPDATE_NTF)
		CASE_MSG_PROC(ITEM_UPDATE_NTF)
		CASE_MSG_PROC(RESULT_NTF)
		CASE_MSG_PROC(CHAT_NTF)
		CASE_MSG_PROC(REPORT_VOTE_NTF)

	default:
		_LOG(TEXT("Invalid GAME message received. ID = %d"), pHeader->msg_id);
	}
}

