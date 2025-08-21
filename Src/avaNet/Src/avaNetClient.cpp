/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaNetClient.cpp

	Description: Implementation of avaNetClient

***/
#include "avaNet.h"
#include "avaNetEventHandler.h"
#include "avaConnection.h"
#include "avaNetStateController.h"
#include "avaMsgSend.h"

#include "avaStaticData.h"
#include "avaWebInClient.h"


#ifndef ASSERT
	#define ASSERT check
#endif

#include "ComDef/Def.h"
#include "ComDef/ItemDesc.h"
#include "ComDef/MsgDef.h"
using namespace Def;

#ifdef EnableP2pConn
//#include "p2pConn.h"
#include "p2pHandler.h"

#endif

#include "RxGateTranslator/RxGateTranslator.h"
#include "hwPerformanceCounter.h"


#ifdef EnableHostMigration
#include "hostmigration.h"
#endif
 


CavaNetClient *GavaNetClient = NULL;
CavaNetEventHandler *GavaEventHandler = NULL;



/////////////////////////////////////////////////////////////////////////////////////////////////////
// CavaMemAlloc

class CavaMemAlloc : public IMemAlloc
{
public:
	void* Alloc(unsigned long size)
	{
		//void *p = appMalloc(size);
		//_LOG(TEXT(">>>>>>>> Alloc(%d) = %p, Cnt = %d"), size, p, ++Cnt);
		//return p;
		return appMalloc(size);
	}
	void Free(void *mem)
	{
		//_LOG(TEXT(">>>>>>>> Free = %p, Cnt = %d"), mem, --Cnt);
		appFree(mem);
	}

	//CavaMemAlloc() : Cnt(0) {}

//private:
//	int Cnt;
};

class CavaBroMemAlloc : public IbroMemAlloc
{
public:
	void* Alloc(unsigned long size)
	{
		void *p = appMalloc(size);
		_LOG(TEXT(">>>>>>>> Alloc(%d) = %p, Cnt = %d"), size, p, ++Cnt);
		return p;
		//return appMalloc(size);
	}
	void Free(void *mem)
	{
		_LOG(TEXT(">>>>>>>> Free = %p, Cnt = %d"), mem, --Cnt);
		appFree(mem);
	}

	CavaBroMemAlloc() : Cnt(0) {}

private:
	int Cnt;
};




struct FavaNetClientLocal
{
	CavaMemAlloc MemAlloc;
	CavaBroMemAlloc BroMemAlloc;
	CavaUNErrorHandler UNErrorHandler;
};


FavaNetClientLocal _Locals;


UavaNetHandler* GetAvaNetHandler()
{
	return UavaNetHandler::StaticClass()->GetDefaultObject<UavaNetHandler>()->GetAvaNetHandler();
}

UavaNetRequest* GetAvaNetRequest()
{
	//static UavaNetRequest *_pNet = NULL;

	//if (!_pNet)
	//{
	//	_pNet = ConstructObject<UavaNetRequest>(UavaNetRequest::StaticClass());
	//	check(_pNet);
	//}

	//return _pNet;
	return UavaNetRequest::StaticClass()->GetDefaultObject<UavaNetRequest>();
}



FInternetIpAddr GetIpAddrFromStr(const TCHAR *addr)
{
	FInternetIpAddr connAddr;
	if (addr && appStrlen(addr) > 0)
	{
		FString Wk, Ip, Port;
		UBOOL bIsValid;
		Wk = addr;
		if ( Wk.Split(CFG_ADDRPORT_SEP, &Ip, &Port) )
		{
			connAddr.SetIp(*Ip, bIsValid);
			if (Port.Len() > 0)
				connAddr.SetPort(appAtoi(*Port));
		}
		else
		{
			connAddr.SetIp(*Wk, bIsValid);
		}
	}

	return connAddr;
}

FString SelectRandomAddress(const TCHAR *AddrList)
{
	if (AddrList && appStrlen(AddrList) > 0)
	{
		FString Wk = AddrList;
		TArray<FString> AddrArray;
		Wk.ParseIntoArrayWS(&AddrArray, CFG_ADDRLIST_SEP);
		INT AddrNum = FString::CullArray(&AddrArray);
		if (AddrNum > 0)
		{
			return AddrArray(appRand() % AddrNum);
		}
	}

	return TEXT("");
}

INT MakeAddressList(const TCHAR *AddrListStr, TArray<FString>& AddrArray)
{
	if (AddrListStr == NULL || appStrlen(AddrListStr) == 0)
		return 0;

	AddrArray.Empty();

	FString Wk = AddrListStr;
	Wk.ParseIntoArrayWS(&AddrArray, CFG_ADDRLIST_SEP);
	return FString::CullArray(&AddrArray);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void FPendingMsg::Dump()
{
	MSG_HEADER *pHeader = (MSG_HEADER*)(((_LPMSGBUF)pMsg)->GetData());
	_DUMP(TEXT("Category = %d, ID = %d, CheckTime = %f"), pHeader->msg_cat, pHeader->msg_id, CheckTime);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// CavaNetClient


DOUBLE CavaNetClient::TimeOutSec = 10.0;
DOUBLE CavaNetClient::ConnectionTimeOutSec = 60.0;


void CavaNetClient::CreateNew()
{
	// memory allocator
	//CMemAlloc::Init(&_Locals.MemAlloc);
	CMsgBuf::pAlloc = &_Locals.MemAlloc;
	CbroMemAlloc::Init(&_Locals.BroMemAlloc);
	IUnNetErrorHandler::pHandler = &_Locals.UNErrorHandler;

	if (!GavaEventHandler)
	{
		GavaEventHandler = new CavaNetEventHandler;

		GCallbackEvent->Register(CALLBACK_ReadyToPlay, GavaEventHandler);
	}

	if (!GavaNetClient)
	{
		GavaNetClient = new CavaNetClient;

		GavaNetClient->SetEventHandler(GavaEventHandler);

		if (!GIsEditor && GIsGame)
			GavaNetClient->InitializeSettings();

		debugf(TEXT("avaNetClient created."));

		if (!GIsEditor && GIsGame)
		{
			GavaNetClient->Start();

			// {{ 20070410 dEAthcURe|HP RTT tester	
#ifdef EnableRttTest
			GavaNetClient->p2pNetTester.init(&GavaNetClient->p2pNtImpl, &GavaNetClient->chsImpl);//&chsNtImpl);
#endif
			// }} 20070410 dEAthcURe|HP RTT tester
		}
	}
}

void CavaNetClient::Delete()
{
	if (GavaNetClient)
	{
		if (!GIsEditor && GIsGame)
		{
			// {{ 20070410 dEAthcURe|HP RTT tester
#ifdef EnableRttTest
			//GavaNetClient->chsImpl.StopTest();
			//GavaNetClient->chsImpl.Deactivate();
			debugf(TEXT("[dEAthcURe] terminating net tester thread..."));
			GavaNetClient->p2pNetTester.setActive(false, true, 10000); // GavaNetClient->p2pNetTester.setActive(false, false);
#endif
			// }} 20070410 dEAthcURe|HP RTT tester

			GavaNetClient->FinalizeSettings();
		}

		delete GavaNetClient;
		GavaNetClient = NULL;

		debugf(TEXT("avaNetClient deleted."));
	}

	if (GavaEventHandler)
	{
		GCallbackEvent->Unregister(CALLBACK_ReadyToPlay, GavaEventHandler);

		delete GavaEventHandler;
		GavaEventHandler = NULL;
	}
}


CavaNetClient::CavaNetClient() :
	timeHostBegin(0.0f), // [+] 20070531 dEAthcURe|HM
	EventHandler(NULL), Output(NULL), CurrConn(NULL), ExitCode(EXIT_UNKNOWN), PerformanceCounter(NULL),
	ClientKey(RXGATE_INVALID_CLIENT_KEY), ClientIP(0), SessionKeyChannel(RXGATE_INVALID_SESSION_KEY), SessionKeyGuild(RXGATE_INVALID_SESSION_KEY),
	CheckDelta(0.0), EmergencyCheckTime(0.0), ConnectionTimeOutDue(0.0), bForceConnect(FALSE), TimeToDieTick(0), TimePlayQuota(0),
	TimeToDieCount(-1), TimeGracefulQuit(0), TimeKeepAliveCheck(0.0), bStarted(FALSE), bP2pSessionDisconnected(FALSE), bInvalidPing(FALSE)
#ifdef EnableRttTest
	/*, RttTestState(0)*/, chsImpl(&p2pNetTester)
#endif
{
	appMemzero(&CurrentClientAddress, sizeof(RxGate::RXNERVE_ADDRESS));
	appMemcpy(CurrentClientAddress.address, "AVAC", 4);

	StateController = new CavaNetStateController();

#ifdef EnableP2pConn
	pp2pConn = 0x0; //new p2pConn_t(); // 20060928 dEathcURe
	pP2PHandler = new CavaP2PHandler();
	UTcpNetDriver::SetP2PHandler(pP2PHandler);
#endif

	PerformanceCounter = new hwPerformanceCounterClient_t();

}

CavaNetClient::~CavaNetClient()
{
	if (ExitCode != EXIT_GAME_END && ExitCode != EXIT_SERVER_SIDE_EXIT)
		CloseConnection(EXIT_FORCED_EXIT);

	delete StateController;

#ifdef EnableP2pConn
	UTcpNetDriver::SetP2PHandler(NULL);
	delete pP2PHandler;
	delete pp2pConn; // 20060928 dEathcURe
#endif

	delete PerformanceCounter;
}


UBOOL ParseCmdParamEx(const TCHAR *Stream, const TCHAR *Param, TCHAR *Value)
{
	const TCHAR* Start = Stream;
	if( *Stream )
	{
		while( (Start=appStrfind(Start+1,Param)) != NULL )
		{
			if( Start>Stream && (Start[-1]=='-' || Start[-1]=='/') )
			{
				*Value = 0;

				const TCHAR* Val = Start + appStrlen(Param);
				if ( Val == NULL || *Val == 0 || appIsWhitespace(*Val) )
                    return TRUE;
				if (*Val == TEXT('='))
					++Val;

				const TCHAR* End = Val;
				while ( *End != 0 && !appIsWhitespace(*End) )
					++End;
				if (End - Val == 0)
					return TRUE;

				appStrncpy(Value, Val, End - Val + 1);
				return TRUE;
			}
		}
	}
	return FALSE;
}


void CavaNetClient::InitializeSettings()
{
	_LOG(TEXT("Initializing and loading avaNet settings"));


	//if ( !_WordCensor().Init(TEXT("w1.txt"), TEXT("w2.txt"), TRUE) )
	//{
	//	_LOG(TEXT("Error! Failed to load censored word lists."));
	//}

	//if ( !_ItemDesc().Init(TEXT("ItemDesc.ini"), TEXT("SlotDesc.ini"), TRUE) )
	//{
	//	_LOG(TEXT("Error! Failed to load ItemDesc.ini or SlotDesc.ini"));
	//}

	// TimeOutSec
	//FString TimeOut;
	//if ( !GConfig->GetString(CFG_SECTION, CFG_TIMEOUTSEC, TimeOut, GNetIni) )
	//{
	//	_LOG(TEXT("TimeOutSec configuration not found; selecting default configuration."));
	//	TimeOut = TEXT("10.0");
	//}
	//TimeOutSec = appAtod(*TimeOut);
	//if (TimeOutSec <= 0.0)
	//	TimeOutSec = 30.0;
	TimeOutSec = 10.0;
	ConnectionTimeOutSec = 60.0;

	// ServerList
	FString ServerList;
	if ( !GConfig->GetString(CFG_SECTION, CFG_SERVERLIST, ServerList, GNetIni) )
	{
		_LOG(TEXT("ServerList configuration not found; selecting default configuration."));
		ServerList = TEXT("10.20.15.43");
	}
	Settings.Set(CFG_SERVERLIST, *ServerList);

	FString P2PServerList;
	if ( !GConfig->GetString(CFG_SECTION, CFG_P2PSERVERLIST, P2PServerList, GNetIni) )
	{
		_LOG(TEXT("P2pServerList configuration not found; selecting default configuration."));
		P2PServerList = TEXT("10.20.15.43:6799");
	}
	Settings.Set(CFG_P2PSERVERLIST, *P2PServerList);

	FString RtttServerList;
	if ( !GConfig->GetString(CFG_SECTION, CFG_RTTTSERVERLIST, RtttServerList, GNetIni) )
	{
		_LOG(TEXT("RtttServerList configuration not found; setting same as P2PServerList."));
		RtttServerList = P2PServerList;
	}
	Settings.Set(CFG_RTTTSERVERLIST, *RtttServerList);

#ifdef EnableP2pConn
	FString P2PPort;
	if ( !GConfig->GetString(CFG_SECTION, CFG_P2PLOCALPORT, P2PPort, GNetIni) )
	{
		_LOG(TEXT("P2PLocalPort configuration not found; selecting default configuration."));
		P2PLocalPort = 27932;
	}
	else
	{
		P2PLocalPort = appAtoi(*P2PPort);
	}
#endif

	// NewChannel
	FString LastChannel;
	if ( GConfig->GetString(CFG_SECTION, CFG_LASTCHANNEL, LastChannel, GNetIni) && LastChannel != TEXT("") )
	{
		Settings.Set(CFG_LASTCHANNEL, *LastChannel);
	}
	//else
	//{
	//	FChannelInfo *Info = _StateController->ChannelList.RandomSelect();
	//	FString NewChannel = (Info ? appItoa(Info->Idx) : TEXT(""));
	//	Settings.Set(CFG_LASTCHANNEL, TEXT(""));
	//}

	//FString PlayersPerChannel;
	//if ( GConfig->GetString(CFG_SECTION, CFG_PLAYERS_PER_CHANNEL, PlayersPerChannel, GNetIni) && PlayersPerChannel != TEXT("") )
	//{
	//}
	//else
	//{
	//	PlayersPerChannel = appItoa(DEFAULT_PLAYERS_PER_CHANNEL);
	//}
	//Settings.Set(CFG_PLAYERS_PER_CHANNEL, *PlayersPerChannel);

	// Misc
	Settings.Set(CFG_USERPASSWORD, TEXT(""));
	Settings.Set(CFG_USERID, TEXT(""));

#ifdef EnableRttTest
	FString RttRecheck;
	if ( GConfig->GetString(CFG_SECTION, CFG_RTTRECHECKTHRESHOLD, RttRecheck, GNetIni) && RttRecheck != TEXT("") )
	{
		//Settings.Set(CFG_RTTRECHECK, *RttRecheck);
		chsImpl.RttRecheckThreshold = appAtoi(*RttRecheck);
	}
	//else
	//{
	//	Settings.Set(CFG_RTTRECHECK, TEXT("100"));
	//}

	if ( GConfig->GetString(CFG_SECTION, CFG_RTTRECHECKTIME, RttRecheck, GNetIni) && RttRecheck != TEXT("") )
	{
		chsImpl.RttRecheckTime = appAtoi(*RttRecheck);
	}
#endif

	UBOOL VersionFileValid = TRUE;

	if ( Parse(appCmdLine(), TEXT("-serveraddr"), ServerList) )
	{
		_LOG(TEXT("Reading serveraddr from command line; %s"), *ServerList);
		Settings.Set(CFG_SERVERLIST, *ServerList);
	}
	else
	{
		TCHAR buf[MAX_PATH];
		DWORD nRead = ::GetPrivateProfileString(TEXT("SERVER"), TEXT("IP1"), TEXT(""), buf, MAX_PATH, TEXT("..\\version.ini"));
		if (nRead > 0 && appStrlen(buf) > 0)
		{
			_LOG(TEXT("Reading serveraddr from version.ini; %s"), buf);
			Settings.Set(CFG_SERVERLIST, buf);
		}
		else
		{
			VersionFileValid = FALSE;
		}
	}

	if ( Parse(appCmdLine(), TEXT("-p2pserveraddr"), P2PServerList) )
	{
		_LOG(TEXT("Reading p2pserveraddr from command line; %s"), *P2PServerList);
		Settings.Set(CFG_P2PSERVERLIST, *P2PServerList);
	}
	else
	{
		TCHAR buf[MAX_PATH];
		DWORD nRead = ::GetPrivateProfileString(TEXT("SERVER"), TEXT("IP2"), TEXT(""), buf, MAX_PATH, TEXT("..\\version.ini"));
		if (nRead > 0 && appStrlen(buf) > 0)
		{
			_LOG(TEXT("Reading p2pserveraddr from version.ini; %s"), buf);
			Settings.Set(CFG_P2PSERVERLIST, buf);
		}
		else
		{
			VersionFileValid = FALSE;
		}
	}

	if ( Parse(appCmdLine(), TEXT("-rtttserveraddr"), RtttServerList) )
	{
		_LOG(TEXT("Reading rtttserveraddr from command line; %s"), *RtttServerList);
		Settings.Set(CFG_RTTTSERVERLIST, *RtttServerList);
	}
	else
	{
		TCHAR buf[MAX_PATH];
		DWORD nRead = ::GetPrivateProfileString(TEXT("SERVER"), TEXT("IP3"), TEXT(""), buf, MAX_PATH, TEXT("..\\version.ini"));
		if (nRead > 0 && appStrlen(buf) > 0)
		{
			_LOG(TEXT("Reading rtttserveraddr from version.ini; %s"), buf);
			Settings.Set(CFG_RTTTSERVERLIST, buf);
		}
		else
		{
			VersionFileValid = FALSE;
		}
	}

	// {{ 20071018 dEAthcURe|HM version.ini에서 FHM 제어
	#ifdef EnableHmFastLoading
	{	
		// {{ 20071123 dEAthcURe|HM version.ini 무시
		//TCHAR strChannels[256] = TEXT("");

		//DWORD nCharRead = GetPrivateProfileStringW(
		//	TEXT("HM"),
		//	TEXT("FastLoadingChannels"),
		//	TEXT(""),
		//	strChannels,
		//	256,
		//	TEXT("..\\version.ini"));

		//if(nCharRead>0 && appStrlen(strChannels)>0) {
		//	g_hostMigration.bEnableFastLoadingFromIni = true;
		//	TArray<FString> arrChannels;
		//	arrChannels.Empty();
		//	FString wk = strChannels;
		//	wk.ParseIntoArrayWS(&arrChannels, TEXT(","));
		//	FString::CullArray(&arrChannels);

		//	g_hostMigration.clearChannelToLoadFast();
		//	for(int lpp=0;lpp<arrChannels.Num();lpp++) {
		//		g_hostMigration.setChannelToLoadFast(appAtoi(*arrChannels(lpp)));
		//		//debugf(TEXT("Channel %d registered for HM Fast loading...%s"), appAtoi(*arrChannels(lpp)), *arrChannels(lpp)); // dd
		//	}
		//	debugf(TEXT("[dEAthcURe|HM] fast loading enabled from Ini for %s"), strChannels);
		//}
		// }} 20071123 dEAthcURe|HM version.ini 무시
	}
	#endif
	// }} 20071018 dEAthcURe|HM version.ini에서 FHM 제어

//#if !FINAL_RELEASE
//	if (!VersionFileValid)
//	{
//		appMsgf(AMT_OK, TEXT("Version file does not exists!!"));
//		exit(1);
//	}
//#endif

	FString ChannelID;
	if ( Parse(appCmdLine(), TEXT("-channel"), ChannelID) )
	{
		Settings.Set(CFG_LASTCHANNEL, *ChannelID);
	}

	_StateController->Init();

	FString Key;

	// read key
	Parse(appCmdLine(),TEXT("-key"), Key);

#if !FINAL_RELEASE

	if (Key.Len() == 0)
	{
		if ( Parse(appCmdLine(),TEXT("-usn"), Key) )
		{
			const TCHAR *ServerKey = TEXT("PmangAVAShot");

			FString USN;
			FString ID;

			if ( Key.Split(TEXT("|"), &USN, &ID) )
			{
				FString PcBang = ParseParam(appCmdLine(), TEXT("pcbang")) ? TEXT("12345") : TEXT("_");
				FString AVAPcBang = ParseParam(appCmdLine(), TEXT("avapcbang")) ? *appItoa(_PCB_AVA) : TEXT("_");
				INT Age = ParseParam(appCmdLine(), TEXT("teen")) ? 17 : 20;

				time_t _t;
				time(&_t);

				FString StrToHash = FString::Printf(TEXT("%s|%s|%d|%s|%d|%s|%s"), *USN, *ID, (DWORD)_t, *PcBang, Age, *AVAPcBang, ServerKey);

				// MD5 hash
				ANSICHAR* AnsiChallenge = TCHAR_TO_ANSI( *StrToHash );
				BYTE Digest[16];
				FMD5Context Context;
				appMD5Init( &Context );
				appMD5Update( &Context, (unsigned char*)AnsiChallenge, StrToHash.Len() );
				appMD5Final( Digest, &Context );

				FString HashStr;
				for( INT i = 0; i < 16; i++ )
					HashStr += FString::Printf(TEXT("%02x"), Digest[i]);

				_LOG(TEXT("HashStr = %s"), *HashStr);

				Key = FString::Printf(TEXT("%s|%s|%d|%s|%d|%s|%s 01401900"), *USN, *ID, (DWORD)_t, *PcBang, Age, *AVAPcBang, *HashStr);
			}
		}
	}

#endif

	if ( Key.Len() > 0 )
	{
		// parse key
		_LOG(TEXT("Key = %s"), *Key);

		// defence code for the incompleted key string
		INT Pos = Key.InStr(TEXT("</"));
		if (Pos >= 0)
			Key = Key.Left(Pos);

		FString HashIndex;
		FString StatIndex;
		if ( Key.Split(TEXT(" "), &HashIndex, &StatIndex) )
		{
			if (StatIndex.Len() >= 8)
			{
				INT Region, AgeCode, Gender, Birth;
				appSSCANF(*StatIndex, TEXT("%02d%1d%1d%4d"), &Region, &AgeCode, &Gender, &Birth);
				_StateController->PlayerInfo.Region = Region;
				_StateController->PlayerInfo.AgeCode = AgeCode;
				_StateController->PlayerInfo.Gender = Gender;
				_StateController->PlayerInfo.Birth = Birth;

				_LOG(TEXT("StatIndex info; Region = %d, AgeCode = %d, Gender = %d, Birth = %d"), Region, AgeCode, Gender, Birth);
			}

			TArray<FString> KeyArray;
			INT Num = HashIndex.ParseIntoArray(&KeyArray, TEXT("|"), TRUE);

			if (Num == 7)
			{
				Settings.Set(CFG_KEYSTRING, *Key);

				Settings.Set(CFG_USERSN, *KeyArray(0));
				Settings.Set(CFG_USERID, *KeyArray(1));

				_StateController->PlayerInfo.Age = appAtoi(*KeyArray(4));
				_StateController->PlayerInfo.PcBangFlag = (KeyArray(3) != TEXT("_") && KeyArray(3).Len() > 0);
				_StateController->PlayerInfo.PcBangServiceType = appAtoi(*KeyArray(5));

				_LOG(TEXT("Adult Version = %s, PCBang Flag = %s, PCBang Service Type = %d"),
							_StateController->PlayerInfo.Age >= 18 ? TEXT("yes") : TEXT("no"),
							_StateController->PlayerInfo.PcBangFlag > 0 ? TEXT("on") : TEXT("off"),
							_StateController->PlayerInfo.PcBangServiceType);
			}
		}
	}

	PerformanceCounter->inspectHw();

	// {{ 20070508 dEAthcURe|TEST|HW CPU, GPU
	_LOG(TEXT("CPU: %s nCore=%d, %dMHz, %s identifier:%s"),
				*PerformanceCounter->myHwInfo.cpuId, PerformanceCounter->myHwInfo.nCpuCore,
				PerformanceCounter->myHwInfo.cpuCoreClock, *PerformanceCounter->myHwInfo.cpuVendorId,
				*PerformanceCounter->myHwInfo.cpuIdentifier);
	_LOG(TEXT("GPU: %s key=%s vendorId=0x%x, deviceId=0x%x"),
				*PerformanceCounter->myHwInfo.gpuId, *PerformanceCounter->myHwInfo.gpuKey,
				PerformanceCounter->myHwInfo.gpuVendorId, PerformanceCounter->myHwInfo.gpuDeviceId);
	// }} 20070508 dEAthcURe|TEST|HW CPU, GPU

	PerformanceCounter->inspectMACAddress();
	FString Mac = FString::Printf(TEXT("Network Adapter: %02x"), PerformanceCounter->myHwInfo.adapterAddress[0]);
	for (INT i = 1; i < MAX_ADAPTER_ADDRESS_LENGTH; ++i)
		Mac += FString::Printf(TEXT(":%02x"), PerformanceCounter->myHwInfo.adapterAddress[i]);
	_LOG(*Mac);

	_WebInClient().Init();
}


// 서버 접속 시작
void CavaNetClient::Start()
{
	if (!GIsEditor && GIsGame)
	{

#ifdef EnableP2pConn
		FString *P2PServerList = Settings.Find(CFG_P2PSERVERLIST);
		if (P2PServerList)
		{
			if (MakeAddressList(**P2PServerList, P2PServers) == 0)
			{
				return;
			}
			_LOG(TEXT("P2P servers"));
			for (INT i = 0; i < P2PServers.Num(); ++i)
			{
				_LOG(TEXT("%s"), *P2PServers(i));
			}
		}

		FString *RtttServerList = Settings.Find(CFG_RTTTSERVERLIST);
		if (RtttServerList)
		{
			if (MakeAddressList(**RtttServerList, RtttServers) == 0)
			{
				return;
			}
			_LOG(TEXT("RTTT servers"));
			for (INT i = 0; i < RtttServers.Num(); ++i)
			{
				_LOG(TEXT("%s"), *RtttServers(i));
			}
		}

		SelectP2PServer(0);

		//_ahead_setDefaultPort(27888);
#endif

		FString *Key = Settings.Find(CFG_KEYSTRING);
		if (Key)
		{
			if (_StateController->LastConnectResult == TEXT(""))
			{
				bStarted = TRUE;
				StateController->LastConnectResult = TEXT("connecting");
				if ( !AutoConnect() )
				{
					// reconnect ?
					if (PendingConnections.Num() == 0)
					{
						StateController->LastConnectResult = TEXT("failed");
						_LOG(TEXT("No more address to connect. Start() failed."));
					}
				}
			}
		}
		else
		{
			// key not found

			// 게임 종료
			//GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_Connect, TEXT("need key"), TEXT(""), 0, 0);
			StateController->LastConnectResult = TEXT("no key");
		}

		if (bStarted)
		{
			// 1주일 플레이 제한
			TimePlayQuota = appSeconds() + 604800;
		}
	}
}

void CavaNetClient::FinalizeSettings()
{
	//FString *ServerList = Settings.Find(CFG_SERVERLIST);
	//if (ServerList)
	//{
	//	GConfig->SetString(CFG_SECTION, CFG_SERVERLIST, **ServerList, GNetIni);
	//}

	//FString *UdpServerList = Settings.Find(CFG_P2PSERVERLIST);
	//if (UdpServerList)
	//{
	//	GConfig->SetString(CFG_SECTION, CFG_P2PSERVERLIST, **UdpServerList, GNetIni);
	//}

	//FString *LastChannel = Settings.Find(CFG_LASTCHANNEL);
	//if (LastChannel && *LastChannel != TEXT(""))
	//{
	//	GConfig->SetString(CFG_SECTION, CFG_LASTCHANNEL, **LastChannel, GNetIni);
	//}

	//FString *UserID = Settings.Find(CFG_USERID);
	//if (UserID && *UserID != TEXT(""))
	//{
	//	GConfig->SetString(CFG_SECTION, CFG_USERID, **UserID, GNetIni);
	//}

	_StateController->Final();
}


void CavaNetClient::Tick(FLOAT DeltaTime)
{
	if (!bStarted)
		return;

	if (CurrConn)
		CurrConn->Tick();

	// process timed out message once per a second
	CheckDelta += DeltaTime;
	if (CheckDelta > 1.0)
	{
		CheckDelta = 0.0;

		// 1초에 한번씩
		ProcKeepAliveCheck();

		ProcTimeToDie();

		ProcPendingConnections();
		ProcConnectionTimeOutDue();
		ProcMTFlags();
		ProcEmergencyState();

		ProcTimeOutPendingMsgs();

		GetAvaNetHandler()->CheckLoadingTimeOut();

		// Scene ( 일반/팝업 )관리를 위해 avaNetHandler::eventTick을 추가
		GetAvaNetHandler()->eventTick( CheckDelta );

		_StateController->Tick();

		_WebInClient().ProcAns();
	}
}

UBOOL CavaNetClient::CreateRxGateSession(RxGate::RXNERVE_ADDRESS &addr)
{
	_LOG(TEXT("Creating RxGateSession to %c%c%c%c%c%c%c%c(%02x%02x%02x%02x%02x%02x%02x%02x)"),
			addr.address[0], addr.address[1], addr.address[2], addr.address[3], addr.address[4], addr.address[5], addr.address[6], addr.address[7],
			addr.address[0], addr.address[1], addr.address[2], addr.address[3], addr.address[4], addr.address[5], addr.address[6], addr.address[7]);

	UBOOL Result = TRUE;
	ScopedMsgBufPtr msgbuf = CreateMsgBufN(1024);
	check(msgbuf);
	if ( RxGateTranslator::MsgCreateSessionReq(msgbuf, &addr) )
	{
		check(GavaNetClient->CurrentConnection());
		GavaNetClient->CurrentConnection()->Send(msgbuf);
	}
	else
	{
		_LOG(TEXT("Error creating CreateSessionReq message."));
		if (GavaNetClient->GetEventHandler())
			GavaNetClient->GetEventHandler()->Error(GavaNetClient->CurrentConnection(), EavaE_Session_FailedToCreate);

		Result = FALSE;
	}

	return Result;
}

void CavaNetClient::StartConnectionTimeOutCheck()
{
	ConnectionTimeOutDue = appSeconds() + ConnectionTimeOutSec;
	_LOG(TEXT("StartConnectionTimeOutCheck() : ConnectionTimeOutDue = %.2f"), ConnectionTimeOutDue);
}

void CavaNetClient::EndConnectionTimeOutCheck()
{
	ConnectionTimeOutDue = 0.0;
	_LOG(TEXT("EndConnectionTimeOutCheck() : ConnectionTimeOutDue = 0.0"));
}


void CavaNetClient::ProcConnectionTimeOutDue()
{
	// 최초 접속 시퀀스의 타임 아웃 체크; TCP connection 이후 부터 CLIENT::CONNECT_ANS를 받을 때 까지...
	if (ConnectionTimeOutDue > 0.0 && ConnectionTimeOutDue < appSeconds())
	{
		_LOG(TEXT("CreaetSessionReq or [CLIENT::CONNECT_REQ] Timed out."));
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_Connect, TEXT("time out"), TEXT(""), 0, 0);

		EndConnectionTimeOutCheck();
	}
}

void CavaNetClient::ProcMTFlags()
{
	// main thread가 아닌 다른 thread에서 main thread로 던져주는 notification을 처리하는 루틴
#ifdef EnableRttTest

	chsImpl.ProcTestResults();

	if (bInvalidPing)
	{
		GetAvaNetRequest()->SetMyRttValue(-2);
		//if (_StateController->RoomInfo.IsValid() && !_StateController->AmIHost() /*&& !_StateController->RoomInfo.PlayerList.IsEmpty(_StateController->RoomInfo.HostIdx)*/)
		//{
		//	FRoomPlayerInfo *MyInfo = _StateController->GetMyRoomPlayerInfo();
		//	if (MyInfo)
		//	{
		//		MyInfo->RoomPlayerInfo.rttScore = -2;
		//		MyInfo->RttValue = -2;
		//	}
		//	PM::ROOM::RTT_UPDATE_NTF::Send(-2);
		//	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_RttRating, TEXT("end"), TEXT(""), 0, 0);

		//	_StateController->LogChatConsole(*FString::Printf(
		//		*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_RttInvalidPing"), TEXT("AVANET")),
		//		_StateController->RoomInfo.PlayerList.PlayerList[_StateController->RoomInfo.HostIdx].RoomPlayerInfo.nickname),
		//		EChat_ReadyRoom);
		//}
		bInvalidPing = FALSE;
	}
#endif

	if (bP2pSessionDisconnected)
	{
		_LOG(TEXT("p2p connection failed"));
		GetAvaNetRequest()->LeaveRoom(ELR_P2PConnectionFailed);
		bP2pSessionDisconnected = FALSE;
	}
}

void CavaNetClient::ProcEmergencyState()
{
	// 서버가 비정상 상태일 때, 다시 복구하는 시퀀스
	if (EmergencyCheckTime > 0.0 && appSeconds() > EmergencyCheckTime)
	{
		// 내 클랜 홈 채널과 연결이 끊긴 상태라면, 다시 연결할 수 있는지 서버에 확인해본다.
		if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD && _StateController->GuildInfo.IsRegularGuild() &&
			!_StateController->GuildInfo.IsChannelConnected() && CurrentGuildAddress.address64 == 0)
		{
			PM::GUILD::GET_CHANNEL_ADDR_REQ::Send();
		}

		// TODO CMS 연결 복구 시퀀스

		EmergencyCheckTime = appSeconds() + EMERGENCY_CHECK_PERIOD;
	}
}

void CavaNetClient::ProcKeepAliveCheck()
{
	if (!CurrConn || CurrConn->ConnState != CS_Connected || !_StateController->IsStatePlaying())
		return;

	DOUBLE Now = appSeconds();

	if (Now > TimeKeepAliveCheck)
	{
		ScopedMsgBufPtr msgbuf = CreateMsgBufN(1024);
		check(msgbuf);
		if ( RxGateTranslator::MsgKeepAliveCheck(msgbuf) )
		{
			CurrConn->Send(msgbuf, TRUE);
			_LOG(TEXT("[MsgKeepAliveCheck] sent."));
		}
		TimeKeepAliveCheck = Now + 50 + appRand() % 20; // 50~70초 사이에 keep-alive 메시지를 보냄
	}
}

void CavaNetClient::ProcTimeToDie()
{
	DOUBLE Now = appSeconds();

	// 1주일 플레이 제한 처리
	if (TimePlayQuota > 0 && TimePlayQuota <= Now)
	{
		TimePlayQuota = 0;
		CloseConnection(EXIT_FORCED_EXIT);
		return;
	}

	// 게임 종료 처리 루틴
	if (TimeGracefulQuit > 0 && TimeGracefulQuit > Now)
	{
		_LOG(TEXT("Time to quit gracefully."));
		TimeGracefulQuit = 0;
		GEngine->Exec(TEXT("QUIT"));
	}

	// 20초 카운트 다운 처리
	if (TimeToDieTick > 0 && TimeToDieCount > -1 && TimeToDieTick <= Now)
	{
		if (TimeToDieCount == 0)
		{
			TimeToDieTick = 0;
			TimeToDieCount = -1;
			GEngine->Exec(TEXT("QUIT"));
		}
		else
		{
			_StateController->RTNotice = FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Client_ServerConnectionLost"), TEXT("AVANET")), TimeToDieCount);
			GetAvaNetHandler()->ProcMessage(EMsg_Admin, EMsg_Admin_Notice, *_StateController->RTNotice, TEXT(""), 0, 0);

			--TimeToDieCount;
			TimeToDieTick = Now + 1;
		}
	}
}


void CavaNetClient::ProcPendingConnections()
{
	// 서버 TCP 연결을 성공할 때까지 반복하는 루틴
	if (CurrConn)
		return;

	if (PendingConnections.Num() > 0)
	{
		// 연결 목록에서 하나를 뽑아와서 연결 시도

		_LOG(TEXT("Pending connections = %d"), PendingConnections.Num());

		INT ConnIdx = appRand() % PendingConnections.Num();
		FString ConnStr = PendingConnections(ConnIdx);
		FInternetIpAddr ConnAddr = GetIpAddrFromStr(*ConnStr);
		DWORD Addr;
		INT Port;

		PendingConnections.Remove(ConnIdx);

		ConnAddr.GetIp(Addr);
		ConnAddr.GetPort(Port);
		if (Addr == 0)
			return;
		if (Port == 0)
			ConnAddr.SetPort(RXGATE_PORT);

		_LOG(TEXT("Connecting another address(%s)"), *ConnStr);

		UBOOL Result = OpenConnection(ConnAddr);
		if (Result)
			PendingConnections.Empty();
	}
	else
	{
		// 더이상 연결할 곳이 없음

		if (_StateController->LastConnectResult == TEXT("connecting"))
		{
			_StateController->SetLastConnectResult(TEXT("failed"));
			_LOG(TEXT("No more address to connect. ProcPendingConnections() failed."));
		}
	}
}

void CavaNetClient::ProcTimeOutPendingMsgs()
{
	// 서버로 보낸 REQ 메시지들의 타임 아웃 체크
	DOUBLE Now = appSeconds();
	for ( TDynamicMap<WORD, FPendingMsg>::TIterator It(PendingMsgs); It; ++It )
	{
		FPendingMsg &Msg = It.Value();
		if (Msg.CheckTime < Now)
		{
			_LOG(TEXT("Message timed out; CheckTime = %.5f, Now = %.5f"), Msg.CheckTime, Now);
			if (GavaEventHandler)
				GavaEventHandler->MsgTimeOut(CurrConn, ((_LPMSGBUF)Msg.pMsg)->GetData(), ((_LPMSGBUF)Msg.pMsg)->GetLength());

			((_LPMSGBUF)(Msg.pMsg))->Delete();
			It.RemoveCurrent();
		}
	}
}


VARARG_BODY(void, CavaNetClient::Logf, const TCHAR*, VARARG_EXTRA(const TCHAR *Header))
{
	INT		BufferSize	= 1024;
	TCHAR*	Buffer		= NULL;
	INT		Result		= -1;

	while(Result == -1)
	{
		Buffer = (TCHAR*) appSystemRealloc( Buffer, BufferSize * sizeof(TCHAR) );
		GET_VARARGS_RESULT(Buffer,BufferSize-1,Fmt,Fmt,Result);
		BufferSize *= 2;
	};
	Buffer[Result] = 0;

	FString ts;
	time_t _tt;
	time(&_tt);
	tm *_t = localtime(&_tt);
	if (_t)
	{
		ts = FString::Printf(TEXT("%02d:%02d:%02d"), _t->tm_hour, _t->tm_min, _t->tm_sec);
	}

	GLog->Logf(NAME_DevNet, TEXT("[%s]%s %s"), *ts, Header, Buffer);

	appSystemFree( Buffer );
}

VARARG_BODY(void, CavaNetClient::LogConsole, const TCHAR*, VARARG_EXTRA(UBOOL bPrefix))
{
	// 콘솔창에 직접 로그 남김
	if (!Output)
		return;

	INT		BufferSize	= 1024;
	TCHAR*	Buffer		= NULL;
	INT		Result		= -1;

	while(Result == -1)
	{
		Buffer = (TCHAR*) appSystemRealloc( Buffer, BufferSize * sizeof(TCHAR) );
		GET_VARARGS_RESULT(Buffer,BufferSize-1,Fmt,Fmt,Result);
		BufferSize *= 2;
	};
	Buffer[Result] = 0;

	if (bPrefix)
		Output->Logf(TEXT("avaNet: %s"), Buffer);
	else
		Output->Log(Buffer);

	appSystemFree( Buffer );
}

// 현재 설정을 가지고 서버 연결
UBOOL CavaNetClient::AutoConnect()
{
	FString *ServerAddress = Settings.Find(CFG_NEWSERVERADDRESS);
	//if (!ServerAddress)
	//	return FALSE;

	//FInternetIpAddr ConnAddr = GetIpAddrFromStr(**ServerAddress);

	PendingConnections.Empty();
	if (ServerAddress)
	{
		PendingConnections.Push(*ServerAddress);
	}
	else
	{
		FString *ServerList = Settings.Find(CFG_SERVERLIST);
		if (!ServerList)
			return FALSE;
		if (MakeAddressList(**ServerList, PendingConnections) == 0)
			return FALSE;

		_LOG(TEXT("Server list"));
		for (INT i = 0; i < PendingConnections.Num(); ++i)
		{
			_LOG(TEXT("%s"), *PendingConnections(i));
		}
	}

	INT ConnIdx = appRand() % PendingConnections.Num();
	FString ConnStr = PendingConnections(ConnIdx);
	FInternetIpAddr ConnAddr = GetIpAddrFromStr(*ConnStr);
	DWORD Addr;
	INT Port;

	PendingConnections.Remove(ConnIdx);

	ConnAddr.GetIp(Addr);
	ConnAddr.GetPort(Port);
	if (Addr == 0)
		return FALSE;
	if (Port == 0)
		ConnAddr.SetPort(RXGATE_PORT);

	//bPendingConnection = FALSE;

	UBOOL Result = OpenConnection(ConnAddr);
	if (Result)
		PendingConnections.Empty();

	return Result;
}


UBOOL CavaNetClient::OpenConnection(FInternetIpAddr &addr)
{
	if (CurrConn)
	{
		_LOG(TEXT("Current connection exists. Closing it."));
		CloseConnection(EXIT_FORCED_EXIT);
		CurrConn = NULL;
		//bPendingConnection = TRUE;
		//return TRUE;
		return FALSE;
	}

	_LOG(TEXT("Connecting %s"), *addr.ToString(TRUE));

	CavaConnection *conn = new CavaConnection();
	check(conn);

	conn->Initialize();
	conn->SetNetClient(this);

	GavaNetClient->CLState = CLS_Connecting;
	if ( !conn->Connect(addr) )
	{
		_LOG(TEXT("Failed to connect"));
		//CurrConn = NULL;
		delete conn;
		return FALSE;
	}

	CurrConn = conn;
	StateController->GoToState(_AN_CONNECTING);
	if (StateController->LastConnectResult != TEXT("connecting"))
		StateController->LastConnectResult = TEXT("");

	return TRUE;
}

//CavaConnection* CavaNetClient::NewConnection(const TCHAR *addr)
UBOOL CavaNetClient::OpenConnection(const TCHAR *addr)
{
	FInternetIpAddr connAddr = GetIpAddrFromStr(addr);

	DWORD Addr;
	connAddr.GetIp(Addr);

	if (Addr == 0)
		return FALSE;
	else
		return OpenConnection(connAddr);
}

void CavaNetClient::CloseConnection(DWORD InExitCode)
{
	_LOG(TEXT("ExitCode = %d"), InExitCode);

	if (CurrConn)
	{
		// 접속 종료 로그
		ExitCode = InExitCode;

		if (ExitCode != EXIT_SERVER_SIDE_EXIT)
		{
			ScopedMsgBufPtr msgbuf = CreateMsgBufN(1024);
			check(msgbuf);
			if ( RxGateTranslator::MsgGameCloseNtf(msgbuf, ExitCode) )
			{
				_LOG(TEXT("Close log sent"));

				CurrConn->Send(msgbuf, TRUE);
			}
			else
			{
				_LOG(TEXT("Failed to send close log"));
			}
		}

		CurrConn->Disconnect();
	}
}

void CavaNetClient::DestroyConnection()
{
	if (CurrConn)
	{
		if (CurrConn->ConnState == CS_Connecting || CurrConn->ConnState == CS_Connected)
		{
			CloseConnection(EXIT_FORCED_EXIT);
		}
		else
		{
			delete CurrConn;
			CurrConn = NULL;
		}
	}
}

// 게임 종료
void CavaNetClient::Quit(UBOOL bGraceful)
{
	// unset location info to the web
	_WebInClient().ChannelUnset();

	UBOOL bQuitImmediatly = FALSE;

	if (!bGraceful || !CurrConn || TimeGracefulQuit > 0)
		bQuitImmediatly = TRUE;

	if (!bQuitImmediatly)
	{
		ScopedMsgBufPtr msgbuf = CreateMsgBufN(1024);
		if ( RxGateTranslator::MsgGameCloseNtf(msgbuf, EXIT_GAME_END) )
		{
			_LOG(TEXT("Quit gracefully on request."));

			CurrConn->Send(msgbuf);
			TimeGracefulQuit = appSeconds() + 5;
			return;
		}
		else
		{
			_LOG(TEXT("Failed to quit gracefully!"));
		}
	}

	_LOG(TEXT("Quit immediatly on request."));
	GEngine->Exec(TEXT("QUIT"));
}

UBOOL CavaNetClient::ParseAdminCommand(const TCHAR *Cmd, FOutputDevice& Ar)
{
	if (Cmd[0] == TEXT('!') && Cmd[1] == TEXT('@'))
	{
		TCHAR *Str = (TCHAR*)Cmd + 2;
		FString Msg(Str);
		if (Msg.Trim().Len() == 0)
		{
			Ar.Log(FString::Printf(TEXT("(Admin) Sending empty notice to all channels")));
			PM::ADMIN::NOTICE_NTF::Send(ID_INVALID_CHANNEL, TEXT(""));
			return TRUE;
		}

		TID_CHANNEL idChannel = ID_INVALID_CHANNEL;
		FString S = ParseToken((const TCHAR*&)Str, FALSE);

		if (appIsDigit(S[0]))
		{
			idChannel = (TID_CHANNEL)appAtoi(*S);
		}
		else
		{
			Str = (TCHAR*)Cmd + 2;
		}

		while (appIsWhitespace(*Str))
			++Str;
		if (*Str == 0)
		{
			Ar.Log(FString::Printf(TEXT("(Admin) Sending empty notice to channel %d"), idChannel));
			PM::ADMIN::NOTICE_NTF::Send(idChannel, TEXT(""));
			return TRUE;
		}

		if (idChannel == ID_INVALID_CHANNEL)
			Ar.Log(FString::Printf(TEXT("(Admin) Sending notice to all channels; %s"), Str));
		else
			Ar.Log(FString::Printf(TEXT("(Admin) Sending notice to channel %d; %s"), idChannel, Str));

		PM::ADMIN::NOTICE_NTF::Send(idChannel, Str);

		return TRUE;
	}
	else if (Cmd[0] == TEXT('%'))
	{
		TCHAR *Str = (TCHAR*)Cmd + 1;

		if ( ParseCommand((const TCHAR**)&Str, TEXT("KICK")) )
		{
			FString Who = ParseToken((const TCHAR*&)Str, FALSE);

			while (Who.Len() > 0)
			{
				if (Who[0] == TEXT('#'))
				{
					// USN
					if (_StateController->PlayerInfo.IsValid() && _StateController->PlayerInfo.PlayerInfo.idAccount == appAtoi(*Who + 1))
					{
						_CNLOG(TEXT("(Admin) You cannot kick yourself"));
					}
					else
					{
						_CNLOG(TEXT("(Admin) Kicking ass of the player with USN '%s' out of the server"), *Who + 1);
						PM::ADMIN::KICK_REQ::Send(0, appAtoi(*Who + 1), TEXT(""));
					}
					
				}
				else if (Who[0] == TEXT('@'))
				{
					_CNLOG(TEXT("(Admin) Kicking ass of the player with ID '%s' out of the server"), *Who + 1);
					// ID
					PM::ADMIN::KICK_REQ::Send(1, 0, *Who + 1);
				}
				else
				{
						// nickname
					if (_StateController->PlayerInfo.IsValid() && Who == _StateController->PlayerInfo.PlayerInfo.nickname)
					{
						_CNLOG(TEXT("(Admin) You cannot kick yourself"));
					}
					else
					{
						_CNLOG(TEXT("(Admin) Kicking ass of '%s' out of the server"), *Who);
						PM::ADMIN::KICK_REQ::Send(2, 0, *Who);
					}
				}

				Who = ParseToken((const TCHAR*&)Str, FALSE);
			}
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("CHATOFF")) )
		{
			FString Who = ParseToken((const TCHAR*&)Str, FALSE);

			while (Who.Len() > 0)
			{
				if (_StateController->PlayerInfo.IsValid() && Who == _StateController->PlayerInfo.PlayerInfo.nickname)
				{
					_CNLOG(TEXT("(Admin) You cannot block yourself from chatting"));
				}
				else
				{
					_CNLOG(TEXT("(Admin) Blocking chatting of the player '%s'"), *Who);
					PM::ADMIN::CHATOFF_REQ::Send(Who);
					Who = ParseToken((const TCHAR*&)Str, FALSE);
				}
			}
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("W")) )
		{
			FString Nickname = ParseToken((const TCHAR*&)Str, FALSE);
			while (*Str && appIsWhitespace(*Str))
			{
				Str++;
			}
			if (*Str == 0)
				return TRUE;

			if (Nickname.Len() > 0)
			{
				PM::ADMIN::WHISPER_NTF::Send(ID_INVALID_ACCOUNT, Nickname, Str);
				_CNLOG(TEXT("(Admin) Message sent to player '%s'"), *Nickname);
			}
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("CHG")) )
		{
			INT RoomID = 0;
			FString RoomName;

			if (_StateController->GetNetState() == _AN_LOBBY)
			{
				FString StrID = ParseToken((const TCHAR*&)Str, FALSE);
				RoomName = ParseToken((const TCHAR*&)Str, FALSE);

				if (StrID.Len() == 0)
					return TRUE;
				RoomID = appAtoi(*StrID);
			}
			else if (_StateController->GetNetState() == _AN_ROOM || _StateController->GetNetState() == _AN_INGAME &&
					_StateController->RoomInfo.IsValid())
			{
				RoomID = _StateController->RoomInfo.RoomInfo.idRoom;
				RoomName = Str;
			}

			if (RoomID == 0 || RoomName.Len() == 0)
				return TRUE;

			_CNLOG(TEXT("(Admin) Changing the name of room %d to '%s'"), RoomID, *RoomName);
			PM::ADMIN::CHANGE_ROOMNAME_REQ::Send(RoomID, RoomName);
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("CHATSAVE")) )
		{
			if (_StateController->ChatSave())
				Ar.Log(TEXT("(Admin) Chatting log saved."));
			else
				Ar.Log(TEXT("(Admin) failed to save chatting log."));
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("STEALTH")) )
		{
			if (_StateController->GetNetState() != _AN_CHANNELLIST)
			{
				_CNLOG(TEXT("(Admin) You are not in the channel list scene"));
				return TRUE;
			}
			FString Opt = ParseToken((const TCHAR*&)Str, FALSE);
			_StateController->StealthMode = (Opt == TEXT("on") || Opt == TEXT("1"));

			Ar.Log( FString::Printf(TEXT("(Admin) Setting stealth mode %s."), (_StateController->StealthMode ? TEXT("on") : TEXT("off"))) );
			PM::ADMIN::SET_VISIBILITY_REQ::Send(!_StateController->StealthMode);
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("MAINNOTICE")) )
		{
			while (*Str && appIsWhitespace(*Str))
			{
				Str++;
			}
			if (*Str == 0)
				return TRUE;

			FString Msg;
			if ( (Str[0] == TEXT('.') && (Str[1] == TEXT('\\') || (Str[1] == TEXT('.') && Str[2] == TEXT('\\')))) ||
				(Str[1] == TEXT(':') && Str[2] == TEXT('\\')) )
			{
				FILE *fp = _tfopen(Str, TEXT("r"));
				if (!fp)
				{
					Ar.Log( FString::Printf(TEXT("(Admin) Error opening main notice message file.")) );
					return TRUE;
				}

				Ar.Log( FString::Printf(TEXT("(Admin) Reading main notice message from %s"), Str) );

				ANSICHAR Line[SIZE_NOTICE_MSG+1];
				while (fgets(Line, SIZE_NOTICE_MSG + 1, fp))
				{
					Msg += ANSI_TO_TCHAR(Line);
				}

				fclose(fp);
			}
			else
			{
				Msg = Str;
			}

			Ar.Log( FString::Printf(TEXT("(Admin) Setting main notice message to '%s'"), *Msg) );
			PM::ADMIN::SET_MAINNOTICE_REQ::Send(Msg);
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("ROOMKICK")) )
		{
			if (!_StateController->RoomInfo.IsValid())
			{
				_CNLOG(TEXT("(Admin) You are not in the room"));
				return TRUE;
			}

			FString Who = ParseToken((const TCHAR*&)Str, FALSE);

			while (Who.Len() > 0)
			{
				if (_StateController->PlayerInfo.IsValid() && Who == _StateController->PlayerInfo.PlayerInfo.nickname)
				{
					_CNLOG(TEXT("(Admin) You cannot kick yourself from the room"));
				}
				else
				{
					FRoomPlayerInfo *Info = _StateController->RoomInfo.PlayerList.Find(Who);
					if (Info)
					{
						_CNLOG(TEXT("(Admin) Kicking '%s' out of the room"), *Who);
						PM::ROOM::KICK_PLAYER_NTF::Send(Info->RoomPlayerInfo.idSlot, Info->RoomPlayerInfo.idAccount, KR_BAN);
					}
					else
					{
						_CNLOG(TEXT("(Admin) '%s' is not found."));
					}
				}

				Who = ParseToken((const TCHAR*&)Str, FALSE);
			}
		}
		else if ( ParseCommand((const TCHAR**)&Str, TEXT("CHLIST")) )
		{
			for (INT i = 0; i < _StateController->ChannelList.ChannelList.Num(); ++i)
				Ar.Log( FString::Printf(TEXT("[%d] %s (%d)"), _StateController->ChannelList.ChannelList(i).idChannel,
															*_StateController->ChannelList.ChannelList(i).ChannelName,
															_StateController->ChannelList.ChannelList(i).Count) );
		}

		return TRUE;
	}

	return FALSE;
}

UBOOL CavaNetClient::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (!Output)
		Output = &Ar;

	const TCHAR* Str = Cmd;

	if (_StateController->AmIAdmin())
	{
		if ( ParseAdminCommand(Str, Ar) )
			return TRUE;
	}

	if ( ParseCommand(&Str, TEXT("NET")) )
	{
		StateController->ParseConsoleCommand(Str);
		return TRUE;
	}

	return FALSE;
}

#ifdef EnableP2pConn

// {{ 20070413 dEAthcURe|HP nonblock p2p client
void CavaNetClient::onP2pNotification(DWORD nParam1, DWORD nParam2)
{
	// {{ 20080213 dEAthcURe
	if(_StateController->AmIHost()) {
		switch(nParam1) {
			case P2P_SESSION_DISCONNECTED: case P2P_SESSION_DIRECT_ESTABLISHED: case P2P_SESSION_HOLE_PUNCHING_ESTABLISHED: case P2P_SESSION_RELAY_ESTABLISHED: { //log("Session established\n");								
				// {{ 20080213 dEAthcURe|SUL
				#ifdef EnableSecureUdpLayer
				if(pp2pConn && GWorld && GWorld->NetDriver && GWorld->NetDriver->pSecureUdpLayer) {
					SOCKADDR_IN* pAddr = 0x0;
					STRUCT_PEER_INFO* pi =pp2pConn->CP2PClientEx::GetPeerInfo(nParam2);
					if(pi) {
						if (pi->m_bInSameNAT) pAddr = (SOCKADDR_IN*)&pi->m_addrLocal;
						else pAddr = (SOCKADDR_IN*)&pi->m_addrPublic;
					}
					GWorld->NetDriver->pSecureUdpLayer->initPeer(*pAddr);
				}
				#endif
				// }} 20080213 dEAthcURe|SUL
			} break;
		}
		return;
	}
	// }} 20080213 dEAthcURe

	switch(nParam1) {	
		case P2P_SVR_CONNECT: { //log("P2P_SVR_CONNECT\n");
			if(pp2pConn) {
				FURL OpenURL;
				OpenURL.AddOption(TEXT("p2p_wait_resolveaddr"));

				pp2pConn->RegisterPeer(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);
				pp2pConn->RequestPeerAddr(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);
				GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
			}
		} break;
		case P2P_SVR_DISCONNECT: {
			debugf(TEXT("[CavaNetClient::onP2pNotification] P2P_SVR_DISCONNECT"));			
			FURL OpenURL;
			OpenURL.AddOption(TEXT("p2p_svr_connect_failed"));
			GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);

			_StateController->SetCurrentHostUnreachable();
			bP2pSessionDisconnected = TRUE;
			//GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("p2p connection failed"), TEXT(""), 0, 0);
			//GetAvaNetRequest()->LeaveRoom();				

			//debugf(TEXT("          <---------- P2P_SVR_DISCONNECT|traveling to %s"), *ConnectURL.String());
			//GEngine->SetClientTravel(*(ConnectURL.String()), TRAVEL_Absolute);
		} break;
		case P2P_SVR_ADDR_OK: {
			if(pp2pConn) {
				STRUCT_PEER_INFO* pi = pp2pConn->GetPeerInfo(GavaNetClient->StateController->PlayerInfo.PlayerInfo.idAccount);
				if(pi) {
					memcpy(&addrLocal, &pi->m_addrLocal, sizeof(SOCKADDR));		
					memcpy(&addrPublic, &pi->m_addrPublic, sizeof(SOCKADDR));		
				}

				if(pp2pConn->setHostAddress((SOCKADDR*)&HostAddrLocal, (SOCKADDR*)&HostAddrPublic)) {
					TCHAR localstr[256], publicstr[256];
					mbstowcs(localstr, inet_ntoa(((SOCKADDR_IN*)&HostAddrLocal)->sin_addr), 255);
					mbstowcs(publicstr, inet_ntoa(((SOCKADDR_IN*)&HostAddrPublic)->sin_addr), 255);
					debugf(TEXT("          <---------- [CavaNetClient::onP2pNotification] setHostAddress %s:%d %s:%d"), 
						localstr, ((SOCKADDR_IN*)&HostAddrLocal)->sin_port, publicstr, ((SOCKADDR_IN*)&HostAddrPublic)->sin_port);
				}
				else {
					debugf(TEXT("          <---------- [CavaNetClient::onP2pNotification] setHostAddress failed"));
				}

				int hostIdx = GavaNetClient->StateController->RoomInfo.HostIdx;
				int idAccHost = GavaNetClient->StateController->RoomInfo.PlayerList.PlayerList[hostIdx].PlayerInfo.idAccount;

				SOCKADDR_IN addr;
				if(pp2pConn->setHost(idAccHost, (sockaddr*)&addr)) {
					if(addr.sin_addr.S_un.S_addr) {
						ConnectURL.Host = FString::Printf( TEXT("%d.%d.%d.%d"), addr.sin_addr.S_un.S_un_b.s_b1,addr.sin_addr.S_un.S_un_b.s_b2,addr.sin_addr.S_un.S_un_b.s_b3,addr.sin_addr.S_un.S_un_b.s_b4);
						ConnectURL.Port = ntohs(addr.sin_port);	

						debugf(TEXT("          <---------- [CavaNetClient::onP2pNotification] getHostAddress id=%d %s %d"), idAccHost, *ConnectURL.Host, ConnectURL.Port);
					}
					else debugf(TEXT("          <---------- [CavaNetClient::onP2pNotification] invalid host addr is acquired. unchanged."));
				}
				else debugf(TEXT("          <---------- [CavaNetClient::onP2pNotification] getHostAddress id=%d failed"), idAccHost);
				
				// {{ [!] 20070224 
				UDP_HOST_INFO addrInfo;	

				addrInfo.intAddr.ipAddress = ((SOCKADDR_IN*)&addrLocal)->sin_addr.S_un.S_addr;
				addrInfo.intAddr.port = ((SOCKADDR_IN*)&addrLocal)->sin_port;

				addrInfo.extAddr.ipAddress = ((SOCKADDR_IN*)&addrPublic)->sin_addr.S_un.S_addr;
				addrInfo.extAddr.port = ((SOCKADDR_IN*)&addrPublic)->sin_port;		
				
				PM::GAME::JOIN_NTF::Send(addrInfo);	

				if(pp2pConn->RequestConnectPeer(idAccHost)) {
					debugf(TEXT("[CavaNetClient::onP2pNotification] RequestConnectPeer(%d) succeeded"), idAccHost);
				}
				else {
					debugf(TEXT("[CavaNetClient::onP2pNotification] RequestConnectPeer(%d) failed"), idAccHost);
				}

				FURL OpenURL;
				OpenURL.AddOption(TEXT("p2p_wait_peer_connect"));
				GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);
			}
		} break;
		case P2P_SESSION_DIRECT_ESTABLISHED: case P2P_SESSION_HOLE_PUNCHING_ESTABLISHED: case P2P_SESSION_RELAY_ESTABLISHED: { //log("Session established\n");								
			debugf(TEXT("[CavaNetClient::onP2pNotification] P2P_SESSION_DIRECT_ESTABLISHED/P2P_SESSION_HOLE_PUNCHING_ESTABLISHED/P2P_SESSION_RELAY_ESTABLISHED"));
			// {{ update addr
			if(pp2pConn) {
				SOCKADDR_IN* pAddr = 0x0;
				STRUCT_PEER_INFO* pi =pp2pConn->CP2PClientEx::GetPeerInfo(nParam2);
				if(pi) {
					if (pi->m_bInSameNAT) pAddr = (SOCKADDR_IN*)&pi->m_addrLocal;
					else pAddr = (SOCKADDR_IN*)&pi->m_addrPublic;
				}

				if(pAddr) {
					ConnectURL.Host = FString::Printf( TEXT("%d.%d.%d.%d"), 
						pAddr->sin_addr.S_un.S_un_b.s_b1, pAddr->sin_addr.S_un.S_un_b.s_b2, pAddr->sin_addr.S_un.S_un_b.s_b3, pAddr->sin_addr.S_un.S_un_b.s_b4);
					ConnectURL.Port = ntohs(pAddr->sin_port);	
				}
			}
			// }} update addr
			debugf(TEXT("--to %s"), *ConnectURL.String());
			GEngine->SetClientTravel(*(ConnectURL.String()), TRAVEL_Absolute);
		} break;

		case P2P_SESSION_DISCONNECTED: {
			debugf(TEXT("[CavaNetClient::onP2pNotification] P2P_SESSION_DISCONNECTED"));
			
			FURL OpenURL;
			OpenURL.AddOption(TEXT("p2p_peer_disconnected"));
			GEngine->SetClientTravel(*(OpenURL.String()), TRAVEL_Absolute);

			_StateController->SetCurrentHostUnreachable();
			bP2pSessionDisconnected = TRUE;
			//GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_LoadingProgress, TEXT("p2p connection failed"), TEXT(""), 0, 0);
			//GetAvaNetRequest()->LeaveRoom();			

			/* {{ [-] 20070531 dEAthcURe|P2p session이 끊기면 에러창
			debugf(TEXT("          <---------- P2P_SESSION_DISCONNECTED|traveling to %s"), *ConnectURL.String());
			GEngine->SetClientTravel(*(ConnectURL.String()), TRAVEL_Absolute);
			*/
		} break;
		default:
			debugf(TEXT("[CavaNetClient::onP2pNotification] %d %d"), nParam1, nParam2);
	}
}
// }} 20070413 dEAthcURe|HP nonblock p2p client

// 접속할 p2p 서버를 선택함 (RTT 서버도 같이 선택)
void CavaNetClient::SelectP2PServer(INT Index)
{
	char szip[16];
	int nPort = 0;
	if (P2PServers.Num() > 0)
	{
		FString NewP2PServer = P2PServers(Index % P2PServers.Num());
		FString Ip, Port;
		if ( NewP2PServer.Split(CFG_ADDRPORT_SEP, &Ip, &Port) )
		{
			strncpy(szip, TCHAR_TO_ANSI(*Ip), 15);
			szip[15] = 0;
			nPort = appAtoi(*Port);
		}
		else
		{
			strncpy(szip, TCHAR_TO_ANSI(*NewP2PServer), 15);
			szip[15] = 0;
		}
	}

	if (strlen(szip) == 0)
	{
		strcpy(szip, "203.215.208.50");
	}
	if (nPort == 0)
	{
		nPort = 28020;
	}

#ifdef EnableP2pConn
	if (pp2pConn)
		pp2pConn->setP2pServer(szip, nPort, true);
	else
		p2pConn_t::setP2pServerOnStartup(szip, nPort);
#endif

	_LOG(TEXT("Setting P2P server to %s:%d"), ANSI_TO_TCHAR(szip), nPort);

	if (RtttServers.Num() > 0)
	{
		FString NewP2PServer = RtttServers(Index % RtttServers.Num());
		FString Ip, Port;
		if ( NewP2PServer.Split(CFG_ADDRPORT_SEP, &Ip, &Port) )
		{
			strncpy(szip, TCHAR_TO_ANSI(*Ip), 15);
			szip[15] = 0;
			nPort = appAtoi(*Port);
		}
		else
		{
			strncpy(szip, TCHAR_TO_ANSI(*NewP2PServer), 15);
			szip[15] = 0;
		}
	}

	if (strlen(szip) == 0)
	{
		strcpy(szip, "203.215.208.51");
	}
	if (nPort == 0)
	{
		nPort = 28030;
	}

#ifdef EnableRttTest
	strcpy(p2pNetTester.p2psIpAddr, szip);
	p2pNetTester.p2psPort = nPort;
#endif

	_LOG(TEXT("Setting RTTT server to %s:%d"), ANSI_TO_TCHAR(szip), nPort);
}
#endif


void CavaNetClient::RtttInit()
{
#ifdef EnableRttTest
	if (GIsGame && !GIsEditor)
	{
		// Start rtt test module
		//GavaNetClient->chsImpl.Connected();
	}
#endif
}

void CavaNetClient::RtttSetActive(DWORD idAccount)
{
#ifdef EnableRttTest
	if (GIsGame && !GIsEditor) {
		p2pNetTester.idAccount = idAccount;
		_LOG(TEXT("[dEAthcURe] activating net tester thread... idAccount = %d"), idAccount);
		p2pNetTester.setActive(idAccount != ID_INVALID_ACCOUNT); // 20070517 dEAthcURe|RTT
	}
#endif
}


void _DumpMemory(BYTE *buf, INT len)
{
#if !FINAL_RELEASE
	if (len <= 0 || buf == NULL)
		return;

	FString DumpBuf;
	for (INT i = 0; i < len; ++i)
	{
		DumpBuf += FString::Printf(TEXT(" %02x"), buf[i]);
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
#endif
}


// {{ 20070426 dEAthcURe|HM
#ifdef EnableHostMigration
void BeNewHost(void)
{
	#ifdef EnableHmFastLoading
	GIsHostMigrating = true; // 20070806 dEAthcURe|HM		
	#endif

	_StateController->GetOpenURL(g_hostMigration.furl);
	debugf(TEXT("[dEAthcURe] BeNewHost traverse %s"), *g_hostMigration.furl.String());
	g_hostMigration.gotoState(hmsNewHost);
}
#endif
// {{ 20070426 dEAthcURe|HM
