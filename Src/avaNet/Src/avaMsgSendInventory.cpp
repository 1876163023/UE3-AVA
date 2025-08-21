/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgSend.cpp

	Description: Implementation of message senders

***/
#include "avaNet.h"

using namespace Def;


#define ASSERT check

#include "avaMsgSend.h"
#include "avaConnection.h"
#include "avaNetStateController.h"

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefClient.h"
#include "ComDef/MsgDefChannel.h"
#include "ComDef/MsgDefRoom.h"
#include "ComDef/MsgDefGame.h"
#include "ComDef/MsgDefInventory.h"
#include "ComDef/MsgDefAdmin.h"
#include "ComDef/MsgDefGuild.h"
#include "RxGateTranslator/RxGateTranslator.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Inventory

void PM::INVENTORY::ENTER_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_LOBBY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_AUTO(msg);
}

void PM::INVENTORY::LEAVE_NTF::Send()
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	SEND_MSG_AUTO(msg);
}

void PM::INVENTORY::EQUIPSET_REQ::Send(TID_EQUIP_SLOT EquipSlot, TSN_ITEM ItemSN)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().equipSlot = EquipSlot;
	msg.Data().item_sn = ItemSN;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::WEAPONSET_REQ::Send(TID_EQUIP_SLOT EquipSlot, TSN_ITEM ItemSN)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_ROOM && _StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}
	if (_StateController->GetNetState() == _AN_ROOM && _StateController->AmISpectator())
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().equipSlot = EquipSlot;
	msg.Data().item_sn = ItemSN;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::CUSTOMSET_REQ::Send(TSN_ITEM ItemSN, TID_ITEM idCustomPart, BYTE Pos)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().item_sn = ItemSN;
	msg.Data().customItem_id = idCustomPart;
	msg.Data().slot = Pos;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::ITEM_BUY_REQ::Send(TID_ITEM idItem)
{
	CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idItem = idItem;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::ITEM_GIFT_REQ::Send(TID_ITEM idItem, DWORD Expire, TID_ACCOUNT idAccount)
{
	CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.item.id = idItem;
	def.item.limit = Expire;
	def.idAccount = idAccount;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::REPAIR_REQ::Send(TSN_ITEM ItemSN)
{
	CHECK_ANY_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY && 
		_StateController->GetNetState() != _AN_ROOM)		// Room에서도 가능하도록 추가.(2007/03/08 NGMI[000469])
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().item_sn = ItemSN;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::CHANGE_CLASS_NTF::Send(BYTE idClass)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	if (_StateController->PlayerInfo.PlayerInfo.currentClass == idClass)
		return;

	TMSG msg;
	DEF &def = msg.Data();

	def.idClass = idClass;

	SEND_MSG_AUTO(msg);
}

void PM::INVENTORY::CONVERT_RIS_REQ::Send(TSN_ITEM ItemSN)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().item_sn = ItemSN;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::ITEM_REFUND_REQ::Send(ITEM_INFO &Item)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().item = Item;

	SEND_MSG_AUTO_P(msg);
}

void PM::INVENTORY::CASHITEM_BUY_REQ::Send(TID_ITEM idItem, TSN_ITEM ItemSN)
{
	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;
	DEF &def = msg.Data();

	def.idItem = idItem;
	def.item_sn = ItemSN;

	SEND_MSG_AUTO(msg);
}

void PM::INVENTORY::EFFSET_REQ::Send(TID_EQUIP_SLOT EquipSlot, TSN_ITEM ItemSN)
{
	CHECK_PENDING_SEND_MSG();

	CHECK_NO_CONNECTION()

	if (_StateController->GetNetState() != _AN_INVENTORY)
	{
		PROC_MSG_SEND_ERROR(TEXT("invalid state"));
		return;
	}

	TMSG msg;

	msg.Data().equipSlot = EquipSlot;
	msg.Data().item_sn = ItemSN;

	SEND_MSG_AUTO_P(msg);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcMsgSendErrorInventory(WORD MsgID, const FString& Err)
{
	BYTE id = (MsgID & 0xff);

	using namespace PM::INVENTORY;

	switch (id)
	{
		CASE_MSG_SEND_ERROR(Inventory, Enter, ENTER_NTF, Err)
		CASE_MSG_SEND_ERROR(Inventory, Leave, LEAVE_NTF, Err)
		CASE_MSG_SEND_ERROR(Inventory, EquipSet, EQUIPSET_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, WeaponSet, WEAPONSET_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, CustomSet, CUSTOMSET_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, Buy, ITEM_BUY_REQ, Err)
		//CASE_MSG_SEND_ERROR(Inventory, ITEM_GIFT_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, Repair, REPAIR_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, ChangeClass, CHANGE_CLASS_NTF, Err)
		CASE_MSG_SEND_ERROR(Inventory, ConvertRIS, CONVERT_RIS_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, Sell, ITEM_REFUND_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, EffBuy, CASHITEM_BUY_REQ, Err)
		CASE_MSG_SEND_ERROR(Inventory, EffSet, EFFSET_REQ, Err)
	default:
		_LOG(TEXT("Failed to send some INVENTORY message. ID = %d"), id);
	}
}
