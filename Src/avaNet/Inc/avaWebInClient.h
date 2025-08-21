/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaWebInClient.h

	Description: Declaration of Web In Client wrapper class.

***/
#pragma once


#define _WEB_IN_CLIENT


#ifdef _WEB_IN_CLIENT

#include "ComDef/Def.h"


class CavaWebInClient
{
public:
	CavaWebInClient();
	virtual ~CavaWebInClient();

	UBOOL Init();
	HWND GetWICHandle()
	{
		return hwndWIC;
	}
	void SetReceiveHandle(HWND h);

	UBOOL OpenChargeWindow();
	UBOOL GetCash();
	UBOOL SendGift(Def::TID_ITEM idItem, DWORD SalesID, Def::TID_ACCOUNT idAccountTo);
	UBOOL OpenGiftWindow(DWORD SalesID);
	UBOOL Buy(Def::TID_ITEM idItem, DWORD SalesID);

	UBOOL GuildJoin(const FString &idGuild, Def::TID_ACCOUNT idAccountFrom, Def::TID_ACCOUNT idAccountTo);
	UBOOL GuildKick(const FString &idGuild, Def::TID_ACCOUNT idAccountFrom, Def::TID_ACCOUNT idAccountTo);
	UBOOL GuildLeave(const FString &idGuild, Def::TID_ACCOUNT idAccountFrom);

	UBOOL ChannelSet(const FString &Loc);
	UBOOL ChannelUnset();

	UBOOL GetTimeStamp();

	void ProcAns();
	void ProcMessage(const FString &Param);

	void ProcOpenChargeWindow();
	void ProcGetCash(const FString &Param);
	void ProcSendGift(const FString &Param);
	void ProcOpenGiftWindow();
	void ProcBuy(const FString &Param);

	void ProcGuild(const FString &Param);

	void ProcChannelSet(const FString &Param);
	void ProcChannelUnset(const FString &Param);

	void ProcTimeStamp(const FString &Param);

	INT GetNow();

private:
	void CreateFingerprint(ANSICHAR *Buf, INT TimeStamp);
	INT CreateTimeStamp();

private:
	enum _WIC_Commands
	{
		WIC_CMD_UNKNOWN = 0,
		WIC_CMD_OPEN_CHARGE_WINDOW,
		WIC_CMD_GET_CASH,
		WIC_CMD_SEND_GIFT,
		WIC_CMD_OPEN_GIFT_WINDOW,
		WIC_CMD_BUY,
		WIC_CMD_GUILD_JOIN,
		WIC_CMD_GUILD_KICK,
		WIC_CMD_GUILD_LEAVE,
		WIC_CMD_CHANNEL_SET,
		WIC_CMD_CHANNEL_UNSET,
		WIC_CMD_GET_TIMESTAMP,
	};

	HWND hwndWIC, hwndRecv;
	HANDLE hAnsEvent;
	INT CurrentCmd;

	UBOOL bChannelSet;

	Def::TID_ACCOUNT TempAccount;

	INT BaseServerTime;
	//INT BaseClientTime;
	DOUBLE BaseClientTime;
};


inline CavaWebInClient& _WebInClient()
{
	static CavaWebInClient _wic;
	return _wic;
}

#endif
