/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaEventHandler.cpp

	Description: Implementation of CavaNetEventHandler; derivation of interface IavaEventHandlerInterface

***/
#include "avaNet.h"
#include "avaNetEventHandler.h"
#include "avaConnection.h"
#include "avaNetStateController.h"
#include "avaCommunicator.h"


#ifndef ASSERT
	#define ASSERT check
#endif

#include "avaMsgSend.h"
#include "avaMsgProc.h"

#include "nProtect.h"

#include "RxGateTranslator/RxGateTranslator.h"
#include "ComDef/Version.h"


extern INT GAVABuiltFromChangeList;




/////////////////////////////////////////////////////////////////////////////////////////////////////
// CavaUNErrorHandler


void CavaUNErrorHandler::OnFailure()
{
	_LOG(TEXT("Rejected by host"));
	GetAvaNetRequest()->LeaveRoom(ELR_RejectedByHost);
}

void CavaUNErrorHandler::OnBrawl()
{
	_LOG(TEXT("MD5 authentication failed"));
	GetAvaNetRequest()->LeaveRoom(ELR_MD5Failed);
}

void CavaUNErrorHandler::OnPackageMismatch(const TCHAR *PackageName)
{
	_LOG(TEXT("Package version mismatch (%s)"), PackageName);
	GetAvaNetRequest()->LeaveRoom(ELR_PackageMismatch);
}

void CavaUNErrorHandler::OnPackageNotFound(const TCHAR *PackageName)
{
	_LOG(TEXT("Package not found (%s)"), PackageName);
	GetAvaNetRequest()->LeaveRoom(ELR_PackageNotFound);
}
void CavaUNErrorHandler::OnConnectionError(const TCHAR *Error)
{
	_LOG(TEXT("Failed to connect"));
	GetAvaNetRequest()->LeaveRoom(ELR_FailedToConnectHost);
}



///////////////////////////////////////////////////////////////////////////////////////
// CavaRxGateTranslatorProc

class CavaRxGateTranslatorProc : public IRxGateTranslateProc
{
public:
	// RxGate -> client
	void MsgConnectionAns(void *info, WORD errCode, DWORD clientKey, DWORD clientIP, RxGate::LPRXNERVE_ADDRESS pGateAddr)
	{
		CavaConnection *Connection = (CavaConnection*)info;
		if (!Connection || Connection != GavaNetClient->CurrentConnection())
		{
			_LOG(TEXT("Invalid connection handler."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
			return;
		}

		TCHAR Addr[9];
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pGateAddr->address, 8, Addr, 8);
		Addr[8] = 0;
		_LOG(TEXT("[MsgConnectionAns] received from %s; Gate = %s"), *Connection->RemoteAddress.ToString(TRUE), Addr);

		_Communicator().SetGateAddress(pGateAddr);

		if (GavaNetClient->CLState != CLS_Connecting)
		{
			_LOG(TEXT("Connection(%s) state is not CS_Connecting."), *Connection->RemoteAddress.ToString(TRUE));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Client_InvalidState);
			GavaNetClient->CloseConnection(EXIT_INVALID_STATE);
			return;
		}

		if (errCode > 0 || clientKey == RXGATE_INVALID_CLIENT_KEY)
		{
			_LOG(TEXT("Error connecting to %s"), *Connection->RemoteAddress.ToString(TRUE));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Session_FailedToConnect);
			GavaNetClient->CloseConnection(EXIT_FAILED_TO_CONNECT);
			return;
		}

		GavaNetClient->ClientKey = clientKey;
		GavaNetClient->ClientIP = clientIP;

		_LOG(TEXT("Requesting session creation; ClientKey = %d, ClientIP = %d"), clientKey, clientIP);

		// select a channel
		FChannelInfo *Channel = _StateController->SelectChannel(TRUE);
		if (!Channel)
		{
			_LOG(TEXT("Error creating CreateSessionReq message."));
			GavaNetClient->CloseConnection(EXIT_FAILED_TO_CREATE_SESSION);
			return;
		}

		GavaNetClient->PendingChannels.Empty();
		for (INT i = 0; i < _StateController->ChannelList.ChannelList.Num(); ++i)
		{
			FChannelInfo &Info = _StateController->ChannelList.ChannelList(i);
			if (Info.idChannel == Channel->idChannel)
				continue;

			if (Info.Flag != _CF_NORMAL)
			{
				if (Info.Flag != _CF_PCBANG || !_StateController->PlayerInfo.IsPcBang())
					continue;
			}

			GavaNetClient->PendingChannels.Push(Info.idChannel);
		}

		CREATE_RXADDRESS(RxAddr, (*Channel));

		if ( GavaNetClient->CreateRxGateSession(RxAddr) )
		{
			GavaNetClient->CLState = CLS_CreatingSession;
		}
	}

	void MsgCreateSessionAns(void *info, WORD errCode, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr)
	{
		CavaConnection *Connection = (CavaConnection*)info;
		if (!Connection || Connection != GavaNetClient->CurrentConnection())
		{
			_LOG(TEXT("Invalid connection handler."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
			return;
		}

		_LOG(TEXT("[MsgCreateSessionAns] received from %s."), *Connection->RemoteAddress.ToString(TRUE));

		if (pServerAddr->address64 == GavaNetClient->CurrentGuildAddress.address64)
		{
			// guild
			if (!_StateController->PlayerInfo.IsValid())
			{
				_LOG(TEXT("Invalid player info."));
				return;
			}
			if (errCode > 0 || sessionKey == RXGATE_INVALID_SESSION_KEY)
			{
				_LOG(TEXT("Error creating guild session. errCode = %d, SessionKey = %d"), errCode, sessionKey);
				GavaNetClient->SessionKeyGuild = 0xff;
				GavaNetClient->CurrentGuildAddress.address64 = 0;
				return;
			}

			GavaNetClient->SessionKeyGuild = sessionKey;

			_LOG(TEXT("Requesting authentication to guild channel."));

			FString *Key = GavaNetClient->Settings.Find(CFG_KEYSTRING);
			PM::CLIENT::GUILD_CONNECT_REQ::Send(Def::VERSION_PROTOCOL, (WORD)GAVABuiltFromChangeList,
							_StateController->PlayerInfo.PlayerInfo.idAccount, _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild,
							Key ? *Key : TEXT(""), _RF_NORMAL, GavaNetClient->CurrentChannelAddress.address64);
		}
		else
		{
			// channel
			if (GavaNetClient->CLState != CLS_CreatingSession)
			{
				_LOG(TEXT("Client state is not CLS_CreatingSession."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Client_InvalidState);
				GavaNetClient->CloseConnection(EXIT_INVALID_STATE);
				return;
			}

			if (errCode > 0 /*|| appMemcmp(&Connection->CurrentChannelAddress, pServerAddr, sizeof(RxGate::RXNERVE_ADDRESS)) != 0*/ || sessionKey == RXGATE_INVALID_SESSION_KEY)
			{
				TCHAR Addr[9];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pServerAddr->address, 8, Addr, 8);
				Addr[8] = 0;

				_LOG(TEXT("Error creating channel session to %s."), Addr);

				FChannelInfo *Channel = NULL;

				if (GavaNetClient->PendingChannels.Num() > 0)
				{
					INT ListIdx = appRand() % GavaNetClient->PendingChannels.Num();
					Channel = _StateController->ChannelList.Find( GavaNetClient->PendingChannels(ListIdx) );
					GavaNetClient->PendingChannels.Remove(ListIdx);
				}

				if (Channel)
				{
					_LOG(TEXT("Trying channel %d"), Channel->idChannel);

					CREATE_RXADDRESS(RxAddr, (*Channel));

					GavaNetClient->CreateRxGateSession(RxAddr);
				}
				else
				{
					_LOG(TEXT("No more channel to create session."));
					if (GavaNetClient && GavaNetClient->GetEventHandler())
						GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Session_FailedToCreate);
					GavaNetClient->CloseConnection(EXIT_FAILED_TO_CREATE_SESSION);
				}

				return;
			}

			GavaNetClient->SessionKeyChannel = sessionKey;

			_LOG(TEXT("Requesting authentication to game channel."));

			FString *Key = GavaNetClient->Settings.Find(CFG_KEYSTRING);
			if (Key)
				PM::CLIENT::CONNECT_REQ::Send(Def::VERSION_PROTOCOL, (WORD)GAVABuiltFromChangeList, *Key, (GavaNetClient->bForceConnect ? _RF_FORCED_CONNECT : _RF_NORMAL));
			else
				PM::CLIENT::CONNECT_REQ::Send(Def::VERSION_PROTOCOL, (WORD)GAVABuiltFromChangeList, TEXT(""), (GavaNetClient->bForceConnect ? _RF_FORCED_CONNECT : _RF_NORMAL));
			GavaNetClient->bForceConnect = FALSE;
		}
	}

	void MsgJoinGroupNtf(void *info, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr)
	{
	}

	void MsgExitGroupNtf(void *info, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr)
	{
	}

	void MsgChangeSessionAns(void *info, WORD errCode, WORD reqSessionKey, WORD newSessionKey, RxGate::LPRXNERVE_ADDRESS pServerAddr)
	{
		CavaConnection *Connection = (CavaConnection*)info;
		if (!Connection || Connection != GavaNetClient->CurrentConnection())
		{
			_LOG(TEXT("Invalid connection handler."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
			return;
		}

		_LOG(TEXT("[MsgChangeSessionAns] received from %s."), *Connection->RemoteAddress.ToString(TRUE));

		if (GavaNetClient->CLState != CLS_ChangingSession)
		{
			_LOG(TEXT("Connection(%s) state is not CS_ChangingSession."), *Connection->RemoteAddress.ToString(TRUE));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Client_InvalidState);
			GavaNetClient->CloseConnection(EXIT_INVALID_STATE);
			return;
		}

		//__int64 OldAddr = pServerAddr->address64;

		TCHAR Addr[9];
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pServerAddr->address, 8, Addr, 8);
		Addr[8] = 0;

		if (errCode > 0 || GavaNetClient->SessionKeyChannel != reqSessionKey)
		{
			_LOG(TEXT("Error changing session to %s."), Addr);
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Session_FailedToChange);
			GavaNetClient->CloseConnection(EXIT_FAILED_TO_CHANGE_SESSION);
			return;
		}

		//appMemcpy(&Connection->CurrentChannelAddress, pServerAddr, sizeof(RxGate::RXNERVE_ADDRESS));
		GavaNetClient->SessionKeyChannel = newSessionKey;

		_LOG(TEXT("Requesting authentication."));

		FString *Key = GavaNetClient->Settings.Find(CFG_KEYSTRING);
		BYTE Rf = (_StateController->AutoMoveDest.IsMoving() && _StateController->AutoMoveDest.IsFollowing() ? _RF_FOLLOWING_PLAYER : _RF_CHANGING_SESSION);
		PM::CLIENT::CONNECT_REQ::Send(Def::VERSION_PROTOCOL, (WORD)GAVABuiltFromChangeList, Key ? *Key : TEXT(""), Rf, GavaNetClient->CurrentChannelAddress.address64);
	}

	void MsgGroupData(void *info, RxGate::LPRXNERVE_ADDRESS pGroupAddr, _LPMSGBUF pData)
	{
		BREAK_SECTION_BEGIN()
		{
			CavaConnection *Connection = (CavaConnection*)info;
			if (!Connection || Connection != GavaNetClient->CurrentConnection())
			{
				_LOG(TEXT("Invalid connection handler."));
				if (GavaNetClient && GavaNetClient->GetEventHandler())
					GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
				break;
			}

			//if (sessionKey != GavaNetClient->SessionKeyChannel)
			//{
			//	_LOG(TEXT("Invalid session key."));
			//	//GavaNetClient->GetEventHandler()->Error(Connection, 0);
			//	break;
			//}

			PM::Proc(pData);
		}
		BREAK_SECTION_END()

		pData->Detach();
		pData->Delete();
	}

	void MsgCloseSessionNtf(void *info, WORD sessionKey, WORD reasonCode)
	{
		CavaConnection *Connection = (CavaConnection*)info;
		if (!Connection || Connection != GavaNetClient->CurrentConnection())
		{
			_LOG(TEXT("Invalid connection handler."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
			return;
		}

		_LOG(TEXT("[MsgCloseSessionNtf] received from %s."), *Connection->RemoteAddress.ToString(TRUE));

		if (sessionKey == GavaNetClient->SessionKeyChannel)
		{
			_LOG(TEXT("Session to channel server is closed."));

			if (GavaNetClient->CLState == CLS_ChangingSession)
			{
				_LOG(TEXT("Currently changing session; ignoring it."));
				return;
			}

			GavaNetClient->CloseConnection(EXIT_SERVER_SIDE_EXIT);
		}
		else if (sessionKey == GavaNetClient->SessionKeyGuild)
		{
			_LOG(TEXT("Session to guild server is closed."));

			GavaNetClient->SessionKeyGuild = 0xff;
			GavaNetClient->CurrentGuildAddress.address64 = 0;

			GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_GuildDisconnect, TEXT(""), TEXT(""), 0, 0);

			if (_StateController->ChannelInfo.IsMyClanChannel())
			{
				if (/*_StateController->GetNetState() != _AN_INGAME && */_StateController->GetNetState() != _AN_CHANNELLIST)
				{
					GavaNetClient->CloseConnection(EXIT_SERVER_SIDE_EXIT);

					// move to channel list scene
					//_StateController->AutoMoveDest.SetMoveDestToNormal(ID_INVALID_CHANNEL);
					//_StateController->GoToState(_AN_CHANNELLIST);
					//GetAvaNetHandler()->ProcMessage(EMsg_Channel, EMsg_Channel_Leave, TEXT("ok"), TEXT(""), 0, 0);
				}
				else
				{
				}
			}
		}
		else
		{
			_LOG(TEXT("Session key is not matching; ignoring it."));
			//GavaNetClient->GetEventHandler()->Error(Connection, 0);
			return;
		}
	}

	void MsgGameGuardAuthReq(void *Info, DWORD clientKey, DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3)
	{
		CavaConnection *Connection = (CavaConnection*)Info;
		if (!Connection || Connection != GavaNetClient->CurrentConnection())
		{
			_LOG(TEXT("Invalid connection handler."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
			return;
		}

		_LOG(TEXT("[MsgGameGuardAuthReq] received from %s; ClientKey = %d, dwIndex = %d, dwValue1 = %d, dwValue2 = %d, dwValue3 = %d"),
					*Connection->RemoteAddress.ToString(TRUE), clientKey, dwIndex, dwValue1, dwValue2, dwValue3);

		if (clientKey != GavaNetClient->ClientKey)
		{
			_LOG(TEXT("Invalid client key. (clientKey[%d] != GavaNetClient->ClientKey[%d])"), clientKey, GavaNetClient->ClientKey);
			//GavaNetClient->GetEventHandler()->Error(Connection, 0);
			return;
		}

		NPGameProcAuth(dwIndex, dwValue1, dwValue2, dwValue3);
	}

	void MsgGameGuardErrorNtf(void *Info)
	{
		CavaConnection *Connection = (CavaConnection*)Info;
		if (!Connection || Connection != GavaNetClient->CurrentConnection())
		{
			_LOG(TEXT("Invalid connection handler."));
			if (GavaNetClient && GavaNetClient->GetEventHandler())
				GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
			return;
		}

		_LOG(TEXT("[MsgGameGuardErrorNtf] received from %s"), *Connection->RemoteAddress.ToString(TRUE));

		_StateController->SetLastConnectResult(TEXT("gameguard error"));
	}

	// RxGate -> server
	void MsgSessionInfoReq(void *info, DWORD clientKey, WORD sessionKey,LONG clientaddr) {}
	void MsgJoinGroupAns(void *info, BOOL join, WORD sessionKey, RxGate::LPRXNERVE_ADDRESS pGroupAddr) {}
	void MsgExitGroupAns(void *info,WORD errCode) {}
	void MsgChangeSessionNtf(void *info, RxGate::LPRXNERVE_ADDRESS pServerAddr) {}
	void MsgGameGuardAuthAns(void *Info, DWORD clientKey, DWORD dwIndex, DWORD dwValue1, DWORD dwValue2, DWORD dwValue3) {}

	// RxGate -> client, server
	void MsgSessionData(void *info, WORD sessionKey, _LPMSGBUF pData)
	{
		BREAK_SECTION_BEGIN()
		{
			CavaConnection *Connection = (CavaConnection*)info;
			if (!Connection || Connection != GavaNetClient->CurrentConnection())
			{
				_LOG(TEXT("Invalid connection handler."));
				if (GavaNetClient && GavaNetClient->GetEventHandler())
					GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
				break;
			}

			if (sessionKey != GavaNetClient->SessionKeyChannel && sessionKey != GavaNetClient->SessionKeyGuild)
			{
				_LOG(TEXT("Invalid session key."));
				//GavaNetClient->GetEventHandler()->Error(Connection, 0);
				break;
			}

			PM::Proc(pData);
		}
		BREAK_SECTION_END()

		pData->Detach();
		pData->Delete();
	}

	void MsgData(void *info, DWORD errCode, DWORD msgTag, DWORD clientKey, RxGate::LPRXNERVE_ADDRESS pServerAddr, _LPMSGBUF pData, RxGate::LPRXNERVE_ADDRESS pSrcAddr)
	{
		BREAK_SECTION_BEGIN()
		{
			CavaConnection *Connection = (CavaConnection*)info;
			if (!Connection || Connection != GavaNetClient->CurrentConnection())
			{
				_LOG(TEXT("Invalid connection handler."));
				if (GavaNetClient && GavaNetClient->GetEventHandler())
					GavaNetClient->GetEventHandler()->Error(Connection, EavaE_Socket_NoConnection);
				break;
			}

			if (errCode == 105)
			{
				// error control
			}
			else
			{
				if (clientKey != GavaNetClient->ClientKey)
				{
					_LOG(TEXT("Invalid session key."));
					//GavaNetClient->GetEventHandler()->Error(Connection, 0);
					break;
				}

				if (pSrcAddr->address64 == _Communicator().GetCMSAddress()->address64)
				{
					_Communicator().ProcMsg(pData->GetData(), pData->GetLength());
				}
				else
				{
					PM::Proc(pData);
				}
			}
		}
		BREAK_SECTION_END()

		pData->Detach();
		pData->Delete();
	}
};

class CavaRxGateTranslatorProc GavaRxGateTranslatorProc;

///////////////////////////////////////////////////////////////////////////////////////
// CavaNetEventHandler

void CavaNetEventHandler::Connected(CavaConnection* Connection)
{
	ASSERT(Connection->ConnState == CS_Connected);

	if (!Connection || !GavaNetClient || Connection != GavaNetClient->CurrentConnection())
	{
		GError->Logf(TEXT("avaNet: Invalid connection handler."));
		Error(Connection, 0);
		return;
	}

	if (GavaNetClient->CLState != CLS_Connecting)
	{
		_LOG(TEXT("Connection(%s) state is not CS_Connecting."), *Connection->RemoteAddress.ToString(TRUE));
		Error(Connection, 0);
		return;
	}

	GLog->Logf(TEXT("avaNet: Connection established to %s."), *Connection->RemoteAddress.ToString(TRUE));

	FString *USN = GavaNetClient->Settings.Find(CFG_USERSN);
	if (!USN)
	{
		Error(Connection, 0);
		return;
	}

	// Request connection
	ScopedMsgBufPtr msgbuf = CreateMsgBufN(1024);
	if ( RxGateTranslator::MsgConnectionReq(msgbuf, appAtoi(**USN)) )
	{
		Connection->Send(msgbuf);
		//Connection->ConnState = CS_CreatingSession;
		GavaNetClient->StartConnectionTimeOutCheck();
	}
	else
	{
		_LOG(TEXT("Error creating ConnectionReq message."));
		Error(Connection, 0);
	}	
}

void CavaNetEventHandler::Disconnected(CavaConnection* Connection)
{
	if (!GavaNetClient)
	{
		GEngine->Exec(TEXT("QUIT"));
	}

	GavaNetClient->PendingMsgs.Empty();
	GavaNetClient->EndConnectionTimeOutCheck();

	if (GavaNetClient->CurrentConnection() == Connection)
	{
		//INT NetState = _StateController->GetNetState();

		_LOG(TEXT("Connection closed."));
		GavaNetClient->DestroyConnection();
		GavaNetClient->PendingMsgs.Empty();

		FString Param = TEXT("server");

		if (_StateController->LastConnectResult == TEXT("already connected"))
			Param = TEXT("already connected");
		else if (_StateController->LastConnectResult == TEXT("connecting"))
			_StateController->LastConnectResult = TEXT("failed");

		if (GavaNetClient->TimeGracefulQuit > 0)
		{
			GavaNetClient->TimeGracefulQuit = 0;
			GEngine->Exec(TEXT("QUIT"));
		}
		else //if (_StateController->GetNetState() >= _AN_CHANNELLIST)
		{
			GavaNetClient->TimeToDieTick = appSeconds();
			GavaNetClient->TimeToDieCount = 20;
			_LOG(TEXT("TimeToDie set to %u"), appSeconds() + 20);

			GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_Disconnect, *Param, TEXT(""), 0, 0);
		}
		//else
		//{
		//	if ( _StateController->AmIHost() )
		//	{
		//		for (INT i = GetAvaNetHandler()->RoomStartingPlayerList.Num() - 1; i >= 0 ; --i)
		//		{
		//			GetAvaNetHandler()->RoomStartingPlayerList.Remove(i);
		//			GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("cancel"), TEXT(""), i, 0);
		//		}
		//	}

		//	//FString Msg = Localize(TEXT("UIPopUpMsg"), TEXT("Msg_Client_ServerConnectionLost"), TEXT("AVANET"));
		//	//GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_Notice, MsgStr.GetString(), TEXT(""), 0, 0);
		//}

		_StateController->GoToState(_AN_DISCONNECTED);
	}
}


#pragma pack(push)
#pragma pack(2)
struct _AVAMSGHDR
{
	DWORD len;
	WORD id;
};
#pragma pack(pop)

void CavaNetEventHandler::MsgReceived(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen)
{
	if (!(Connection->ConnState >= CS_Connecting && Connection->ConnState <= CS_ClosePending))
		return;

	if (!Connection || Connection != GavaNetClient->CurrentConnection())
	{
		GError->Logf(TEXT("avaNet: Invalid connection handler."));
		Error(Connection, 0);
		return;
	}

	if ( !RxGateTranslator::Proc(Connection, &GavaRxGateTranslatorProc, (LPBYTE)Buffer, (DWORD)BufferLen) )
	{
		_AVAMSGHDR *pHdr = (_AVAMSGHDR*)Buffer;
		_LOG(TEXT("Error translating RxGate message. id = %d, length = %d, BufferLen = %d"), pHdr->id, pHdr->len, BufferLen);
		if (pHdr->id == 13)
		{
			_LOG(TEXT("Message was MsgSessionData. recorded data length = %d, real buffer length = %d"),
					*((DWORD*)(Buffer + sizeof(_AVAMSGHDR) + sizeof(WORD))),
					BufferLen - (sizeof(_AVAMSGHDR) + sizeof(WORD) + sizeof(DWORD)));
		}
		_DUMP(TEXT("Dumping buffer..."));
		FString DumpBuf;
		for (INT i = 0; i < BufferLen; ++i)
		{
			DumpBuf += FString::Printf(TEXT(" %02x"), Buffer[i]);
			if (i % 16 == 15)
			{
				_DUMP(*DumpBuf);
				DumpBuf = TEXT("");
			}
		}
		if (DumpBuf.Len() > 0)
		{
			_DUMP(*DumpBuf);
		}
		Error(Connection, 0);
	}
}

void CavaNetEventHandler::MsgSent(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen)
{
}

void CavaNetEventHandler::MsgTimeOut(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen)
{
	PM::ProcTimeOut(Buffer, BufferLen);
}

void CavaNetEventHandler::Error(CavaConnection* Connection, INT Code)
{
	_StateController->SetLastConnectResult(TEXT("failed"));

	switch (Code)
	{
	case EavaE_None:
		break;
	case EavaE_Socket_FailedToConnect:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("socket failed to connect"));
		break;
	case EavaE_Socket_FailedToRecv:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("socket failed to receive"));
		break;
	case EavaE_Socket_FailedToSend:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("socket failed to send"));
		break;
	case EavaE_Socket_NoConnection:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("no connection"));
		break;
	case EavaE_Session_FailedToConnect:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("session failed to connect"));
		break;
	case EavaE_Session_FailedToCreate:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("session failed to create"));
		break;
	case EavaE_Session_FailedToChange:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("session failed to change"));
		break;
	case EavaE_Client_InvalidState:
		GetAvaNetHandler()->eventProcCriticalError(TEXT("client invalid state"));
		break;
	}
}


INT CavaNetEventHandler::CheckMsgLength(CavaConnection* Connection, const BYTE *Buffer, INT BufferLen)
{
	if (BufferLen < sizeof(_AVAMSGHDR)) return 0;
	_AVAMSGHDR* mhdr = (_AVAMSGHDR*)Buffer;
	if (mhdr == NULL) return 0;
	INT mlen = (INT)mhdr->len + sizeof(_AVAMSGHDR);
	if (BufferLen < mlen) return 0;
	return mlen;
}


void CavaNetEventHandler::Send(ECallbackEventType InType)
{
	if (InType == CALLBACK_ReadyToPlay)
	{
		FinishLoadingProgress();
	}
}


DOUBLE CavaNetEventHandler::NoticedTime = 0.0f;
INT CavaNetEventHandler::NoticedProgress = -1;
INT CavaNetEventHandler::NoticedStep = -1;


void CavaNetEventHandler::StartLoadingProgress(UBOOL bNotify)
{
	// Set default map configuration to loading map to reduce loading time
	if (!_StateController->IsStatePlaying())
		return;

	if (bNotify)
	{
		NoticedProgress = 0;
		NoticedStep = 0;
		NoticedTime = appSeconds();
	}

	_LOG(TEXT("Loading started."));
}

void CavaNetEventHandler::FinishLoadingProgress(UBOOL bNotify)
{
	//if (/*NoticedProgress < 0 ||*/ NoticedTime == 0)
	//	return;

	NoticedProgress = -1;
	NoticedStep = -1;
	NoticedTime = 0;

	if (!_StateController->AmIHost())
	{
		GetAvaNetHandler()->LoadingCheckTime = 0.0;
		GetAvaNetHandler()->LoadingCutOffPerc = -1;
	}

	if (!_StateController->IsStatePlaying())
		return;

	_LOG(TEXT("Loading finished."));

	if (bNotify)
	{
		PM::GAME::LOADING_PROGRESS_NTF::Send(100, -1);
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("complete"), TEXT("me"), 0, 100);

		FRoomPlayerInfo *Info = _StateController->GetMyRoomPlayerInfo();
		if (Info && Info->RttValue < 0)
		{
			GetAvaNetRequest()->SetMyRttValue(Info->RttValue == -1 ? 10 : 100);
		}
	}
}

void CavaNetEventHandler::LoadingProgressCallback(INT idStep, INT CurStep, INT MaxStep, bool bTickNetClient) // void CavaNetEventHandler::LoadingProgressCallback(const TCHAR *StepName, FLOAT Sec, INT CurStep, INT MaxStep) // [!] 20070510 dEAthcURe|LP
{
	if(bTickNetClient) GavaNetClient->Tick(0.f); // dEAthcURe|LP loading중에도 peers의 progress ntf를 받아야한다.

	if (!_StateController->IsStatePlaying())
		return;
	if (!_StateController->IsStatePlaying())
		return;

	// {{ [!] 20070510 dEAthcURe|LP history file이 없으면 MaxStep이 0이다.
	if(0 == MaxStep) {
		return;
	}
	//_LOG(TEXT("[CavaNetEventHandler::LoadingProgressCallback] %d:%d/%d"), idStep, CurStep, MaxStep);
	// }} [!] 20070510 dEAthcURe|LP history file이 없으면 MaxStep이 0이다.

	//if (/*NoticedProgress < 0 ||*/ NoticedTime == 0) // [-] dEAthcURe|LP
	//	return; // [-] dEAthcURe|LP

	if (CurStep >= MaxStep)
		CurStep = MaxStep - 1;

	UBOOL bNotice = FALSE;
	BYTE CurProgress = 100 * CurStep / MaxStep;

	if (NoticedProgress < 30)
	{
		if (CurProgress >= 30 || appSeconds() - NoticedTime > 10.0f)
		{
			bNotice = TRUE;
		}
	}
	else
	{
		if ((CurProgress - NoticedProgress >= 20) || (/*CurProgress > NoticedProgress &&*/ CurStep > NoticedStep && appSeconds() - NoticedTime > 10.0f))
		{
			bNotice = TRUE;
		}
	}

	if (bNotice)
	{
		NoticedProgress = CurProgress;
		NoticedStep = CurStep;
		NoticedTime = appSeconds();

		PM::GAME::LOADING_PROGRESS_NTF::Send(NoticedProgress, NoticedStep);

		_LOG(TEXT("Loading... %d%%(%d)"), NoticedProgress, NoticedStep);

		//GavaNetClient->Tick(0.f);
	}
}

