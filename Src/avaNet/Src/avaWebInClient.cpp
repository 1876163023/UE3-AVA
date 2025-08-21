#include "avaNet.h"
#include "avaWebInClient.h"
#include "WinDrv.h"

#include "avaNetStateController.h"
#include "avaCommunicator.h"
#include "avaMsgSend.h"


#ifdef _WEB_IN_CLIENT


using namespace Def;


#define WIC_ID_GET_MSG_HWND				"2D67AF8D-88E6-4407-83C7-155ACF425B82"

#define WIC_ID_OPEN_CHARGE_WINDOW		TEXT("8F7D5D1D-E648-4b56-9161-C5FF277B9C4D")
#define WIC_ID_OPEN_CHARGE_WINDOW_ANS	TEXT("2CC3BBF4-F86E-4bc4-8FB2-F817FF7F69CF")

#define WIC_ID_GET_CASH					TEXT("52401F44-E3B9-4bc9-A58F-89A477DF3726")
#define WIC_ID_GET_CASH_ANS				TEXT("4E7915B7-3786-4312-9CB7-BB0C8E346746")

#define WIC_ID_SEND_GIFT				"D065B59C-67EF-4aa9-B822-6A9142299318=%d|%d"
#define WIC_ID_SEND_GIFT_ANS			TEXT("A7828094-4A4A-4ee2-85F6-DF3EE2042423")
#define WIC_ID_OPEN_GIFT_WINDOW			"c0685ede-0eba-42f6-85af-9d798528e839=%d"
#define WIC_ID_OPEN_GIFT_WINDOW_ANS		TEXT("7b5ed705-4019-4ff9-88fd-f809051496c7")

#define WIC_ID_BUY						"6487A280-3B1C-4e8f-9DA5-FDADD9978041=%d"
#define WIC_ID_BUY_ANS					TEXT("A79D448A-BD05-455d-9B3A-9E83F4826360")

#define WIC_ID_DATA						TEXT("FD5F9651-6429-40a6-A829-D58938D4DD89")
#define WIC_ID_DATA_ANS					TEXT("D9F76F10-6963-4115-A690-18795F581EEA")

#define WIC_ID_CUSTOM					"729ED77C-ABD6-4847-A911-E3A12AE6C846"
#define WIC_ID_CUSTOM_ANS				TEXT("C1CD4B5B-4EBA-4e45-84AD-F4CF15E72F60")

//guid=actcode|timestamp|fingerprint|guildid|from_usn|to_usn
#define WIC_ID_GUILD_JOIN				"729ED77C-ABD6-4847-A911-E3A12AE6C846=102|%d|%s|%s|%d|%d"
//guid=actcode|timestamp|fingerprint|guildid|from_usn|to_usn
#define WIC_ID_GUILD_KICK				"729ED77C-ABD6-4847-A911-E3A12AE6C846=103|%d|%s|%s|%d|%d"
//guid=actcode|timestamp|fingerprint|guildid|from_usn
#define WIC_ID_GUILD_LEAVE				"729ED77C-ABD6-4847-A911-E3A12AE6C846=104|%d|%s|%s|%d"

#define WIC_ID_GET_TIMESTAMP			"729ED77C-ABD6-4847-A911-E3A12AE6C846=106"

#define WIC_ID_DISCONNECTED				"18E2135C-7BA8-4e48-8D98-3B7B885AE14D"

#define WIC_ID_CHANNEL					"FD5F9651-6429-40a6-A829-D58938D4DD89"
#define WIC_ID_CHANNEL_SET				"FD5F9651-6429-40a6-A829-D58938D4DD89=ON|320|%s"
#define WIC_ID_CHANNEL_UNSET			"FD5F9651-6429-40a6-A829-D58938D4DD89=DEL|320"
#define WIC_ID_CHANNEL_ANS				TEXT("D9F76F10-6963-4115-A690-18795F581EEA")

#define WIC_WND_HANDLE					TEXT("D7CD782E-A5B6-471d-9564-72F87BA18BDF")


CavaWebInClient::CavaWebInClient() : hwndWIC(NULL), hAnsEvent(NULL), hwndRecv(NULL), CurrentCmd(WIC_CMD_UNKNOWN), bChannelSet(FALSE),
									TempAccount(ID_INVALID_ACCOUNT), BaseServerTime(0), BaseClientTime(0.0)
{
}

CavaWebInClient::~CavaWebInClient()
{
}


void CavaWebInClient::CreateFingerprint(ANSICHAR *Buf, INT TimeStamp)
{
	check(Buf);

	// MD5 hash
	ANSICHAR AnsiChallenge[MAX_PATH];
	sprintf(AnsiChallenge, "PmangAVAShot320ava%d", TimeStamp);
	BYTE Digest[16];
	FMD5Context Context;
	appMD5Init( &Context );
	appMD5Update( &Context, (unsigned char*)AnsiChallenge, strlen(AnsiChallenge) );
	appMD5Final( Digest, &Context );

	ANSICHAR Temp[3];
	for( INT i = 0; i < 16; ++i )
	{
		sprintf(Temp, "%02x", Digest[i]);
		appMemcpy(Buf + i * 2, Temp, 2);
	}

	_LOG(TEXT("Challenge string = %s"), ANSI_TO_TCHAR(AnsiChallenge));
}

INT CavaWebInClient::CreateTimeStamp()
{
	//__time32_t t;
	//_time32(&t);
	//INT CurrTime = (INT)t;
	DOUBLE CurrTime = appSeconds();

	if (CurrTime <= BaseClientTime)
	{
		return BaseServerTime;
	}

	return (INT)(CurrTime - BaseClientTime) + BaseServerTime;
}

UBOOL CavaWebInClient::Init()
{
	if (GIsEditor || !GIsGame)
		return TRUE;

	hwndWIC = ::FindWindow(NULL, WIC_WND_HANDLE);
	if (!hwndWIC)
	{
		_LOG(TEXT("Failed to find WebInClient module; %s"), WIC_WND_HANDLE);
		return FALSE;
	}

	_LOG(TEXT("WebInClient initialized."));

	return TRUE;
}

void CavaWebInClient::SetReceiveHandle(HWND h)
{
	if (GIsEditor || !GIsGame)
		return;

	hwndRecv = h;

	if (hwndRecv)
	{
		COPYDATASTRUCT stData;
		stData.dwData = 0;
		stData.cbData = 37;
		stData.lpData = WIC_ID_GET_MSG_HWND;

		if ( (HWND)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData)) == hwndWIC )
		{
			_LOG(TEXT("Succeeded."));
		}
		else
		{
			_LOG(TEXT("Failed."));
		}

		GetTimeStamp();
	}
	else
	{
		_LOG(TEXT("Set receive handle to NULL"));
	}
}


UBOOL CavaWebInClient::OpenChargeWindow()
{
	if (!hwndWIC)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Charge, TEXT("wic inactive"), TEXT(""), 0, 0);
		return FALSE;
	}
	if (hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Charge, TEXT("wic working"), TEXT(""), 0, 0);
		return FALSE;
	}

	HANDLE h = ::OpenEvent(EVENT_ALL_ACCESS | EVENT_MODIFY_STATE, FALSE, WIC_ID_OPEN_CHARGE_WINDOW);
	if (!h)
	{
		_LOG(TEXT("Failed to open event"));
		return FALSE;
	}

	CurrentCmd = WIC_CMD_OPEN_CHARGE_WINDOW;

	::SetEvent(h);
	::CloseHandle(h);
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_OPEN_CHARGE_WINDOW_ANS);

	_LOG(TEXT("[OPEN_CHARGE_WINDOW] sent."));

	return TRUE;
}

UBOOL CavaWebInClient::GetCash()
{
	if (!hwndWIC)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_GetCash, TEXT("wic inactive"), TEXT(""), 0, 0);
		return FALSE;
	}
	if (hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_GetCash, TEXT("wic working"), TEXT(""), 0, 0);
		return FALSE;
	}

	HANDLE h = ::OpenEvent(EVENT_ALL_ACCESS | EVENT_MODIFY_STATE, FALSE, WIC_ID_GET_CASH);
	if (!h)
	{
		_LOG(TEXT("Failed to open event"));
		return FALSE;
	}

	CurrentCmd = WIC_CMD_GET_CASH;

	::SetEvent(h);
	::CloseHandle(h);
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_GET_CASH_ANS);

	_LOG(TEXT("[GET_CASH] sent."));

	return TRUE;
}

UBOOL CavaWebInClient::SendGift(Def::TID_ITEM idItem, DWORD SalesID, TID_ACCOUNT idAccountTo)
{
	if (!hwndWIC)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_SendGift, TEXT("wic inactive"), TEXT(""), 0, 0);
		return FALSE;
	}
	if (hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_SendGift, TEXT("wic working"), TEXT(""), 0, 0);
		return FALSE;
	}

	if (_StateController->BuyingItemInfo.IsProcessing())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_SendGift, TEXT("processing"), TEXT(""), 0, 0);
		return FALSE;
	}

	CurrentCmd = WIC_CMD_SEND_GIFT;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	sprintf(szTemp, WIC_ID_SEND_GIFT, SalesID, idAccountTo);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_SEND_GIFT_ANS);

	_LOG(TEXT("[SEND_GIFT] sent."));

	_StateController->BuyingItemInfo.SetProcess(idItem, idAccountTo);

	return TRUE;
}

UBOOL CavaWebInClient::OpenGiftWindow(DWORD SalesID)
{
	if (!hwndWIC)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Charge, TEXT("wic inactive"), TEXT(""), 0, 0);
		return FALSE;
	}
	if (hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Charge, TEXT("wic working"), TEXT(""), 0, 0);
		return FALSE;
	}

	CurrentCmd = WIC_CMD_OPEN_GIFT_WINDOW;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	sprintf(szTemp, WIC_ID_OPEN_GIFT_WINDOW, SalesID);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_OPEN_GIFT_WINDOW_ANS);

	_LOG(TEXT("[OPEN_GIFT_WINDOW] sent."));

	return TRUE;
}

UBOOL CavaWebInClient::Buy(Def::TID_ITEM idItem, DWORD SalesID)
{
	if (!hwndWIC)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("wic inactive"), TEXT(""), 0, 0);
		return FALSE;
	}
	if (hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("wic working"), TEXT(""), 0, 0);
		return FALSE;
	}

	if (_StateController->BuyingItemInfo.IsProcessing())
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("processing"), TEXT(""), 0, 0);
		return FALSE;
	}

	CurrentCmd = WIC_CMD_BUY;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	sprintf(szTemp, WIC_ID_BUY, SalesID);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_BUY_ANS);

	_LOG(TEXT("[BUY] sent."));

	_StateController->BuyingItemInfo.SetProcess(idItem);

	return TRUE;
}

UBOOL CavaWebInClient::GuildJoin( const FString &idGuild, Def::TID_ACCOUNT idAccountFrom, Def::TID_ACCOUNT idAccountTo )
{
	if (!hwndWIC || hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
		return FALSE;

	CurrentCmd = WIC_CMD_GUILD_JOIN;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	INT TimeStamp = CreateTimeStamp();;
	ANSICHAR Fingerprint[33];
	Fingerprint[32] = 0;
	CreateFingerprint(Fingerprint, TimeStamp);

	sprintf(szTemp, WIC_ID_GUILD_JOIN, TimeStamp, Fingerprint, TCHAR_TO_ANSI(*idGuild), idAccountFrom, idAccountTo);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	//_LOG(TEXT("%s == %s"), *idGuild, ANSI_TO_TCHAR(TCHAR_TO_ANSI(*idGuild)));
	_LOG(TEXT("TimeStamp = %d; String = %s"), TimeStamp, ANSI_TO_TCHAR(szTemp));

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_CUSTOM_ANS);

	_LOG(TEXT("[GUILD_JOIN] sent."));

	TempAccount = idAccountTo;

	return TRUE;
}

UBOOL CavaWebInClient::GuildKick( const FString &idGuild, Def::TID_ACCOUNT idAccountFrom, Def::TID_ACCOUNT idAccountTo )
{
	if (!hwndWIC || hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
		return FALSE;

	//if (!CheckRecvHwnd())
	//	return FALSE;

	CurrentCmd = WIC_CMD_GUILD_KICK;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	INT TimeStamp = CreateTimeStamp();;
	ANSICHAR Fingerprint[33];
	Fingerprint[32] = 0;
	CreateFingerprint(Fingerprint, TimeStamp);

	sprintf(szTemp, WIC_ID_GUILD_KICK, TimeStamp, Fingerprint, TCHAR_TO_ANSI(*idGuild), idAccountFrom, idAccountTo);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	//_LOG(TEXT("%s == %s"), *idGuild, ANSI_TO_TCHAR(TCHAR_TO_ANSI(*idGuild)));
	_LOG(TEXT("TimeStamp = %d; String = %s"), TimeStamp, ANSI_TO_TCHAR(szTemp));

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_CUSTOM_ANS);

	_LOG(TEXT("[GUILD_KICK] sent."));

	TempAccount = idAccountTo;

	return TRUE;
}

UBOOL CavaWebInClient::GuildLeave( const FString &idGuild, Def::TID_ACCOUNT idAccountFrom )
{
	if (!hwndWIC || hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
		return FALSE;

	CurrentCmd = WIC_CMD_GUILD_LEAVE;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	INT TimeStamp = CreateTimeStamp();;
	ANSICHAR Fingerprint[33];
	Fingerprint[32] = 0;
	CreateFingerprint(Fingerprint, TimeStamp);

	sprintf(szTemp, WIC_ID_GUILD_LEAVE, TimeStamp, Fingerprint, TCHAR_TO_ANSI(*idGuild), idAccountFrom);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	//_LOG(TEXT("%s == %s"), *idGuild, ANSI_TO_TCHAR(TCHAR_TO_ANSI(*idGuild)));
	_LOG(TEXT("TimeStamp = %d; String = %s"), TimeStamp, ANSI_TO_TCHAR(szTemp));

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_CUSTOM_ANS);

	_LOG(TEXT("[GUILD_LEAVE] sent."));

	return TRUE;
}

UBOOL CavaWebInClient::ChannelSet(const FString &Loc)
{
	if (!hwndWIC || hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
		return FALSE;

	CurrentCmd = WIC_CMD_CHANNEL_SET;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	INT TimeStamp = CreateTimeStamp();;
	ANSICHAR Fingerprint[33];
	Fingerprint[32] = 0;
	CreateFingerprint(Fingerprint, TimeStamp);

	sprintf(szTemp, WIC_ID_CHANNEL_SET, TCHAR_TO_ANSI(*Loc));
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_CHANNEL_ANS);

	_LOG(TEXT("[CHANNEL_SET] sent. (%s)"), *Loc);

	bChannelSet = TRUE;

	return TRUE;
}

UBOOL CavaWebInClient::ChannelUnset()
{
	if (!hwndWIC || hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
		return FALSE;
	if (!bChannelSet)
		return FALSE;

	CurrentCmd = WIC_CMD_CHANNEL_UNSET;

	COPYDATASTRUCT stData;
	ANSICHAR szTemp[MAX_PATH];

	INT TimeStamp = CreateTimeStamp();;
	ANSICHAR Fingerprint[33];
	Fingerprint[32] = 0;
	CreateFingerprint(Fingerprint, TimeStamp);

	sprintf(szTemp, WIC_ID_CHANNEL_UNSET);
	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(szTemp) + 1;
	stData.lpData = (PVOID)szTemp;

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_CHANNEL_ANS);

	_LOG(TEXT("[CHANNEL_UNSET] sent."));
	bChannelSet = FALSE;

	return TRUE;
}

UBOOL CavaWebInClient::GetTimeStamp()
{
	if (!hwndWIC || hAnsEvent || CurrentCmd != WIC_CMD_UNKNOWN)
		return FALSE;

	CurrentCmd = WIC_CMD_GET_TIMESTAMP;

	COPYDATASTRUCT stData;

	stData.dwData= 0;
	stData.cbData= (DWORD)strlen(WIC_ID_GET_TIMESTAMP) + 1;
	stData.lpData = (PVOID)WIC_ID_GET_TIMESTAMP;

	HANDLE h = (HANDLE)::SendMessage(hwndWIC, WM_COPYDATA, (WPARAM)hwndRecv, (LPARAM)(&stData));
	if (h != hwndWIC)
	{
		_LOG(TEXT("Failed to send message to WebInClient module."));
		return FALSE;
	}
	hAnsEvent = ::CreateEvent(NULL, FALSE, FALSE, WIC_ID_CUSTOM_ANS);

	_LOG(TEXT("[GET_TIMESTAMP] sent."));

	return TRUE;
}

void CavaWebInClient::ProcAns()
{
	if (hAnsEvent && ::WaitForSingleObject(hAnsEvent, 0) == WAIT_OBJECT_0)
	{
		hAnsEvent = NULL;

		switch (CurrentCmd)
		{
		case WIC_CMD_OPEN_CHARGE_WINDOW:
			ProcOpenChargeWindow();
			break;
		case WIC_CMD_GET_CASH:
			//ProcGetCash(TEXT(""));
			break;
		case WIC_CMD_SEND_GIFT:
			//ProcSendGift(TEXT(""));
			break;
		case WIC_CMD_OPEN_GIFT_WINDOW:
			ProcOpenGiftWindow();
			break;
		case WIC_CMD_BUY:
			//ProcBuy(TEXT(""));
			break;
		case WIC_CMD_GUILD_JOIN:
			_LOG(TEXT("[GUILD_JOIN] terminated"));
			break;
		case WIC_CMD_GUILD_KICK:
			_LOG(TEXT("[GUILD_KICK] terminated"));
			break;
		case WIC_CMD_GUILD_LEAVE:
			_LOG(TEXT("[GUILD_LEAVE] terminated"));
			break;
		case WIC_CMD_CHANNEL_SET:
			_LOG(TEXT("[CHANNEL_SET] terminated"));
			break;
		case WIC_CMD_CHANNEL_UNSET:
			_LOG(TEXT("[CHANNEL_UNSET] terminated"));
			break;
		case WIC_CMD_GET_TIMESTAMP:
			_LOG(TEXT("[GET_TIMESTAMP] terminated"));
			break;
		default:
			_LOG(TEXT("Unknown command's answer received."));
			break;
		}

		CurrentCmd = WIC_CMD_UNKNOWN;
	}
}

void CavaWebInClient::ProcMessage(const FString &Param)
{
	_LOG(TEXT("Param = %s"), *Param);

	switch (CurrentCmd)
	{
	case WIC_CMD_GET_CASH:
		ProcGetCash(Param);
		break;
	case WIC_CMD_SEND_GIFT:
		ProcSendGift(Param);
		break;
	case WIC_CMD_BUY:
		ProcBuy(Param);
		break;
	case WIC_CMD_GUILD_JOIN:
	case WIC_CMD_GUILD_KICK:
	case WIC_CMD_GUILD_LEAVE:
		ProcGuild(Param);
		break;
	case WIC_CMD_CHANNEL_SET:
		ProcChannelSet(Param);
		break;
	case WIC_CMD_CHANNEL_UNSET:
		ProcChannelUnset(Param);
		break;
	case WIC_CMD_GET_TIMESTAMP:
		ProcTimeStamp(Param);
		break;
	default:
		break;
	}

	//CurrentCmd = WIC_CMD_UNKNOWN;
}

void CavaWebInClient::ProcOpenChargeWindow()
{
	_LOG(TEXT("[OPEN_CHARGE_WINDOW] terminated."));
	GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Charge, TEXT("ok"), TEXT(""), 0, 0);

	//GetCash();
}

void CavaWebInClient::ProcGetCash(const FString &Param)
{
	if (Param.Len() > 0)
	{
		DWORD Cash;
		appSSCANF(*Param, TEXT("CASH=%d"), &Cash);

		_StateController->PlayerInfo.Cash = Cash;

		_LOG(TEXT("Player's cash is %d"), Cash);

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_GetCash, TEXT("ok"), TEXT(""), Cash, 0);
		_LOG(TEXT("[GET_CASH] terminated."));

		return;
	}

	_LOG(TEXT("Failed to get cash from web."));
	GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_GetCash, TEXT("failed"), TEXT(""), 0, 0);
}

void CavaWebInClient::ProcSendGift(const FString &Param)
{
	if (!_StateController->BuyingItemInfo.IsProcessing())
	{
		_LOG(TEXT("[SEND_GIFT] terminated but no item was requested!"));
		return;
	}

	TID_ITEM idItem = _StateController->BuyingItemInfo.idItem;
	TID_ACCOUNT idAccount = _StateController->BuyingItemInfo.idAccount;
	_StateController->BuyingItemInfo.Clear();

	{
		TArray<FString> Results;
		if ( Param.ParseIntoArray(&Results, TEXT("|"), FALSE) != 3)
		{
			_LOG(TEXT("Insufficient result parameters."));
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_SendGift, TEXT("bad web result"), TEXT(""), 0, 0);
			return;
		}

		INT ErrCode = 0;
		DWORD Cash = 0;
		FString CashStr;
		FString RetMsg;
		FString Key, Value;

		for (INT i = 0; i < Results.Num(); ++i)
		{
			if ( !Results(i).Split(TEXT("="), &Key, &Value) )
			{
				_LOG(TEXT("Failed to get result code -> %s"), *Results(i));
				continue;
			}

			if (Key == TEXT("RESULT"))
			{
				ErrCode = appAtoi(*Value);
			}
			else if (Key == TEXT("CASH"))
			{
				CashStr = Value;
				_StateController->PlayerInfo.Cash = Cash;
			}
			else if (Key == TEXT("RETMSG"))
			{
				RetMsg = Value;
			}
		}

		if (CashStr.Len() > 0)
		{
			Cash = appAtoi(*CashStr);
			_StateController->PlayerInfo.Cash = Cash;
		}
		else
			Cash = _StateController->PlayerInfo.Cash;

		if (ErrCode == 1)
		{
			// succeed
			_LOG(TEXT("sent the gift; idItem = %d, idAccount = %d, Cash = %d"), idItem, idAccount, Cash);
		}
		else
		{
			// fail
			_LOG(TEXT("Failed to send gift; Result = %d, Message = %s"), ErrCode, *RetMsg);
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_SendGift, TEXT("web failed"), *RetMsg, 0, 0);
		}

		_LOG(TEXT("[SEND_GIFT] terminated."));
		return;
	}
}

void CavaWebInClient::ProcOpenGiftWindow()
{
	_LOG(TEXT("[OPEN_GIFT_WINDOW] terminated."));
	GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_SendGift, TEXT("ok"), TEXT(""), 0, 0);
}

void CavaWebInClient::ProcBuy(const FString &Param)
{
	if (!_StateController->BuyingItemInfo.IsProcessing())
	{
		_LOG(TEXT("[BUY] terminated but no item was requested!"));
		return;
	}

	TID_ITEM idItem = _StateController->BuyingItemInfo.idItem;

	{
		TArray<FString> Results;
		if ( Param.ParseIntoArray(&Results, TEXT("|"), FALSE) == 0)
		{
			_LOG(TEXT("Insufficient result parameters."));
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("bad web result"), TEXT(""), 0, 0);
			return;
		}

		INT ErrCode = 0;
		TCASH Cash = 0;
		FString CashStr;
		FString RetMsg;
		FString Key, Value;

		for (INT i = 0; i < Results.Num(); ++i)
		{
			if ( !Results(i).Split(TEXT("="), &Key, &Value) )
			{
				_LOG(TEXT("Failed to get result code -> %s"), *Results(i));
				continue;
			}

			if (Key == TEXT("RESULT"))
			{
				ErrCode = appAtoi(*Value);
			}
			else if (Key == TEXT("CASH"))
			{
				CashStr = Value;
			}
			else if (Key == TEXT("RETMSG"))
			{
				RetMsg = Value;
			}
		}

		if (CashStr.Len() > 0)
		{
			Cash = appAtoi(*CashStr);
			_StateController->PlayerInfo.Cash = Cash;
		}
		else
			Cash = _StateController->PlayerInfo.Cash;

		if (ErrCode == 1)
		{
			// succeed
			_LOG(TEXT("Bought the item; idItem = %d, Cash = %d, ItemSN = %s"), idItem, Cash, *RetMsg);
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("web ok"), TEXT(""), Cash, 0);

			// RetMsg == ItemSN
			PM::INVENTORY::CASHITEM_BUY_REQ::Send(idItem, _tstoi64(*RetMsg));
		}
		else
		{
			// fail
			_LOG(TEXT("Failed to buy the item; Result = %d, Message = %s"), ErrCode, *RetMsg);
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, (ErrCode == -9 ? TEXT("no cash") : TEXT("web failed")), *RetMsg, 0, 0);

			_StateController->BuyingItemInfo.Clear();
		}

		_LOG(TEXT("[BUY] terminated."));

		return;
	}
}

void CavaWebInClient::ProcGuild(const FString &Param)
{
	FString Key, Value;
	if ( !Param.Split(TEXT(":="), &Key, &Value) )
	{
		_LOG(TEXT("Failed to parse result message."));
		return;
	}

	if (Key == TEXT("SUCCESS") && Value == TEXT("OK"))
	{
		switch (CurrentCmd)
		{
		case WIC_CMD_GUILD_JOIN:
			if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD && TempAccount != ID_INVALID_ACCOUNT)
				_Communicator().JoinGuild(TempAccount, &GavaNetClient->CurrentGuildAddress, _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild);
			TempAccount = ID_INVALID_ACCOUNT;
			break;
		case WIC_CMD_GUILD_KICK:
			if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD && TempAccount != ID_INVALID_ACCOUNT)
				PM::GUILD::KICK_NTF::Send(TempAccount);
			TempAccount = ID_INVALID_ACCOUNT;
			break;
		case WIC_CMD_GUILD_LEAVE:
			if (_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD)
			{
				PM::GUILD::LEAVE_NTF::Send();

				_StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild = ID_INVALID_GUILD;
				appStrcpy(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName, TEXT(""));
				_StateController->GuildInfo.Clear();

				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_Left"), TEXT("AVANET")), EChat_GuildSystem);

				GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
				GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_PlayerInfo, TEXT(""), TEXT(""), 0, 0);
			}
			break;
		}
	}
	else if (Key == TEXT("ERROR"))
	{
		switch ( appAtoi(*Value) )
		{
		case 1001:
			_LOG(TEXT("Error! Invalid timestamp."));
			break;
		case 1002:
			_LOG(TEXT("Error! Invalid fingerprint."));
			break;
		case 1003:
			_LOG(TEXT("Error! Invalid action code."));
			break;
		case 1004:
			_LOG(TEXT("Error! Invalid action parameters."));
			break;
		default:
			_LOG(TEXT("Error! Unknown error code."));
			break;
		}

		_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_RequestFailed"), TEXT("AVANET")), EChat_PlayerSystem);
	}
	else if (Key == TEXT("FAIL"))
	{
		switch (CurrentCmd)
		{
		case WIC_CMD_GUILD_JOIN:
			if (Value == TEXT("ERR_JOIN_DUPLE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_AlreadyHasClan"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_JOIN_GAME_RUN"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_InvalidPlayer"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_INSERT"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_ADMIT_DUPLE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_ClanAlreadyJoined"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_ADMIT_UNREGISTER"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_UnregisteredPlayer"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_STATUS"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_UPDATE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_COUNT"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_MODIFY"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_GUILD_LEVEL"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_MEMBER_OVER"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_MemberFull"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			break;
		case WIC_CMD_GUILD_KICK:
		case WIC_CMD_GUILD_LEAVE:
			if (Value == TEXT("ERR_DB_MEMBER_DELETE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_MEMBER_MASTER_OUT"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_ClanMasterCannotLeave"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_DELETE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_ADMIN_DELETE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_LADDER_DELETE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_STATUS"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_STATUS_UPDATE"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_COUNT"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			else if (Value == TEXT("ERR_DB_MEMBER_MODIFY"))
			{
				_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_WiC_DBFailed"), TEXT("AVANET")), EChat_PlayerSystem);
			}
			break;
		}
	}

	//_LOG(TEXT("WebInClient: GUILD_JOIN terminated."));
}

void CavaWebInClient::ProcChannelSet(const FString &Param)
{
	_LOG(TEXT("[CHANNEL_SET] terminated."));
}

void CavaWebInClient::ProcChannelUnset(const FString &Param)
{
	_LOG(TEXT("[CHANNEL_UNSET] terminated."));
}

void CavaWebInClient::ProcTimeStamp(const FString &Param)
{
	_LOG(TEXT("[GET_TIMESTAMP] terminated."));

	FString Key, Value;
	if ( !Param.Split(TEXT(":="), &Key, &Value) )
	{
		_LOG(TEXT("Failed to parse result message."));
		return;
	}

	if (Key == TEXT("SUCCESS"))
	{
		BaseServerTime = appAtoi(*Value);
		//__time32_t t;
		//_time32(&t);
		//BaseClientTime = (INT)t;
		BaseClientTime = appSeconds();
		_LOG(TEXT("BaseServerTime = %d, BaseClientTime = %.2f"), BaseServerTime, BaseClientTime);
	}
	else
	{
		_LOG(TEXT("Failed to get timestamp."));
	}
}

INT CavaWebInClient::GetNow()
{
	if (BaseServerTime == 0 || BaseClientTime == 0.0)
		return 0;

	return CreateTimeStamp();
}

#endif
