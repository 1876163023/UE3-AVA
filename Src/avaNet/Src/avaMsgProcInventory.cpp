/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaMsgProcInventory.cpp

	Description: Implementation of message processors

***/
#include "avaNet.h"

#define ASSERT check

#include "ComDef/Def.h"

using namespace Def;

#include "ComDef/MsgDef.h"
#include "ComDef/MsgDefInventory.h"

#include "avaNetClient.h"
#include "avaMsgProc.h"
#include "avaMsgSend.h"
#include "avaNetStateController.h"
#include "avaStaticData.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Inventory

void PM::INVENTORY::EQUIPSET_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(EQUIPSET_REQ, pmsgbuf);

	if (!_StateController->PlayerInfo.IsValid())
	{
		pmsgbuf->Delete();
		return;
	}

	PM::INVENTORY::EQUIPSET_REQ::TMSG pmsg(pmsgbuf);
	TMSG msg(pData);

	BREAK_SECTION_BEGIN()
	{
		if (msg.Data().result == RC_OK)
		{
			CInventory &Inven = _StateController->PlayerInfo.Inven;

			if (pmsg.Data().equipSlot >= 0 && pmsg.Data().equipSlot < MAX_EQUIPSET_SIZE)
			{
				TSN_ITEM ChangedSN = pmsg.Data().item_sn;
				Inven.EquipSet(pmsg.Data().equipSlot, pmsg.Data().item_sn, &ChangedSN);
				//_StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[pmsg.Data().equipSlot] = pmsg.Data().item_sn;

				if (pmsg.Data().item_sn == SN_INVALID_ITEM && ChangedSN != SN_INVALID_ITEM)
				{
					ITEM_DESC *Desc = _ItemDesc().GetItem( Inven.GetItemIdToSN(ChangedSN) );
					if (Desc)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("set default"), Desc->GetName(), 0, 0);
						break;
					}
				}

				// empty invalidated grenade slots
				Inven.RebuildEffect();
				_StateController->PlayerInfo.CheckGrenadeSlots();

				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("ok"), TEXT(""), 0, 0);
				break;
			}
		}
		else if (msg.Data().result == RC_ITEM_NOT_USE_LEVEL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("low level"), TEXT(""), 0, 0);
			break;;
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EquipSet, TEXT("failed"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::WEAPONSET_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(WEAPONSET_REQ, pmsgbuf);

	if (!_StateController->PlayerInfo.IsValid())
	{
		pmsgbuf->Delete();
		return;
	}

	PM::INVENTORY::WEAPONSET_REQ::TMSG pmsg(pmsgbuf);
	TMSG msg(pData);

	BREAK_SECTION_BEGIN()
	{
		if (msg.Data().result == RC_OK)
		{
			CInventory &Inven = _StateController->PlayerInfo.Inven;

			INT EquipSlot = pmsg.Data().equipSlot;
			TSN_ITEM ItemSN = pmsg.Data().item_sn;
			if (EquipSlot >= 0 && EquipSlot < MAX_WEAPONSET_SIZE)
			{
				SLOT_DESC *pSlot = _ItemDesc().GetWeaponSlot(EquipSlot);
				if (!pSlot)
				{
					_LOG(TEXT("Error setting weapon; invalid slot"));
					break;
				}

				TSN_ITEM ChangedSN = ItemSN;
				Inven.WeaponSet(EquipSlot, ItemSN, &ChangedSN);
				//_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[EquipSlot] = ItemSN;

				if (_StateController->RoomInfo.IsValid())
				{
					FRoomPlayerInfo *pInfo = _StateController->GetMyRoomPlayerInfo();
					if (pInfo)
					{
						if (ChangedSN == SN_INVALID_ITEM)
						{
							//pInfo->PlayerInfo.weaponItem[EquipSlot] = ID_INVALID_ITEM;
							if (pSlot->slotType & _EP_WEAP_PRIMARY)
							{
								_LOG(TEXT("Error setting weapon; primary weapon cannot be unset."));
								break;
							}
						}
						else
						{
							if (pSlot->slotType & _EP_WEAP_PRIMARY)
							{
								INT ClassID = ((pSlot->slotType & _EP_P1) ? _CLASS_POINTMAN : (pSlot->slotType & _EP_R1) ?
												_CLASS_RIFLEMAN : (pSlot->slotType & _EP_S1) ? _CLASS_SNIPER : _CLASS_NONE);
								if (ClassID == _CLASS_NONE)
								{
									_LOG(TEXT("Error setting weapon; invalid class"));
									break;
								}

								CUSTOM_ITEM_INFO *Cust = NULL;
								for (INT csi = 0; csi < _CSI_MAX; ++csi)
								{
									Cust = Inven.GetCustomInvenToSlot(ChangedSN, (CUSTOM_SLOT_IDX)csi);
									if (Cust)
										pInfo->RoomPlayerInfo.customItem[ClassID][csi] = Cust->id;
									else
										pInfo->RoomPlayerInfo.customItem[ClassID][csi] = ID_INVALID_ITEM;
								}
							}
						}
					}
				}

				if (ItemSN == SN_INVALID_ITEM && ChangedSN != SN_INVALID_ITEM)
				{
					ITEM_DESC *Desc = _ItemDesc().GetItem( Inven.GetItemIdToSN(ChangedSN) );
					if (Desc)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("set default"), Desc->GetName(), 0, 0);
						break;
					}
				}
			}

			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("ok"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result == RC_ITEM_NOT_USE_LEVEL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("low level"), TEXT(""), 0, 0);
			break;;
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_WeaponSet, TEXT("failed"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::CUSTOMSET_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(CUSTOMSET_REQ, pmsgbuf);

	if (!_StateController->PlayerInfo.IsValid())
		return;

	PM::INVENTORY::CUSTOMSET_REQ::TMSG pmsg(pmsgbuf);
	TMSG msg(pData);

	BREAK_SECTION_BEGIN()
	{
		if (msg.Data().result == RC_ITEM_NOT_USE_LEVEL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("low level"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result == RC_INVENTORY_FULL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("full"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result == RC_ITEM_EXIST_PREV_BUYITEM)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("processing"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result != RC_OK)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
			break;
		}

		CInventory &Inven = _StateController->PlayerInfo.Inven;

		TMONEY RefundMoney = 0;
		CUSTOM_ITEM_INFO *pOldItem = Inven.GetCustomInvenToSlot(pmsg.Data().item_sn, (CUSTOM_SLOT_IDX)pmsg.Data().slot);
		if (pOldItem)
		{
			CUSTOM_ITEM_DESC *pOldDesc = _ItemDesc().GetCustomItem(pOldItem->id);
			if (pOldDesc->priceType == _IPT_MONEY)
				RefundMoney = Inven.GetCustomItemRefundMoney(pOldItem->item_sn, (CUSTOM_SLOT_IDX)pmsg.Data().slot);
			_LOG(TEXT("'%s' was set on the slot[%d]; refund price = %d"), pOldDesc->GetName(), pmsg.Data().slot, RefundMoney);
		}
		else
		{
			_LOG(TEXT("The slot[%d] was empty."), pmsg.Data().slot);
		}

		if (pmsg.Data().customItem_id != ID_INVALID_ITEM)
		{
			CUSTOM_ITEM_DESC *ItemDesc = _ItemDesc().GetCustomItem(pmsg.Data().customItem_id);
			if (!ItemDesc)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("invalid item"), TEXT(""), 0, 0);
				break;
			}

			if (!Inven.CustomSet(pmsg.Data().item_sn, (CUSTOM_SLOT_IDX)pmsg.Data().slot, pmsg.Data().customItem_id))
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
				break;
			}

			// refund old item
			_StateController->PlayerInfo.PlayerInfo.money += RefundMoney;
			if (ItemDesc->priceType == _IPT_MONEY)
			{
				_StateController->PlayerInfo.PlayerInfo.money -= ItemDesc->price;
				if (_StateController->PlayerInfo.PlayerInfo.money < 0)
					_StateController->PlayerInfo.PlayerInfo.money = 0;
			}

			_LOG(TEXT("New custom item is '%s'; price = %d; my money = %d"), ItemDesc->GetName(), ItemDesc->price, _StateController->PlayerInfo.PlayerInfo.money);
		}
		else
		{
			if (!Inven.CustomSet(pmsg.Data().item_sn, (CUSTOM_SLOT_IDX)pmsg.Data().slot, ID_INVALID_ITEM))
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("failed"), TEXT(""), 0, 0);
				break;
			}

			// refund old item
			_StateController->PlayerInfo.PlayerInfo.money += RefundMoney;

			_LOG(TEXT("The slot[%d] is unset; my money = %d"), pmsg.Data().slot, _StateController->PlayerInfo.PlayerInfo.money);

			if ((CUSTOM_SLOT_IDX)pmsg.Data().slot == _CSI_MOUNT)
			{
				CUSTOM_ITEM_INFO *pNewItem = Inven.GetCustomInvenToSlot(pmsg.Data().item_sn, (CUSTOM_SLOT_IDX)pmsg.Data().slot);
				if (pNewItem)
				{
					CUSTOM_ITEM_DESC *pNewDesc = _ItemDesc().GetCustomItem(pNewItem->id);
					if (pNewDesc)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("set default"), pNewDesc->GetName(), 0, 0);
						break;
					}
				}
			}
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_CustomSet, TEXT("ok"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::ITEM_BUY_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(ITEM_BUY_REQ, pmsgbuf);

	BREAK_SECTION_BEGIN()
	{
		if (!_StateController->PlayerInfo.IsValid())
		{
			break;
		}

		PM::INVENTORY::ITEM_BUY_REQ::TMSG pmsg(pmsgbuf);
		TMSG msg(pData);

		if (msg.Data().result == RC_ITEM_NOT_USE_LEVEL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("low level"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result == RC_INVENTORY_FULL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("full"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result == RC_ITEM_EXIST_PREV_BUYITEM)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("processing"), TEXT(""), 0, 0);
			break;
		}
		else if (msg.Data().result != RC_OK)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("failed"), TEXT(""), 0, 0);
			break;
		}

		ITEM_DESC *ItemDesc = _ItemDesc().GetItem(pmsg.Data().idItem);
		if (ItemDesc)
		{
			// money
			if (ItemDesc->priceType == _IPT_MONEY)
			{
				_StateController->PlayerInfo.PlayerInfo.money -= ItemDesc->price;
				if (_StateController->PlayerInfo.PlayerInfo.money < 0)
					_StateController->PlayerInfo.PlayerInfo.money = 0;
			}

			CInventory &Inven = _StateController->PlayerInfo.Inven;

			ITEM_INFO Item;
			Item.id = pmsg.Data().idItem;
			Item.sn = msg.Data().item_sn;
			Item.limit = ITEM_LIMIT_INITED;

			if (Inven.ItemBuy(&Item))
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("ok"), TEXT(""), 0, 0);
				pmsgbuf->Delete();
				return;
			}
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Buy, TEXT("invalid"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::ITEM_GIFT_ANS::Proc(_LPMSGBUF pData)
{
	CHECK_AND_DELETE_PENDING_MSG(ITEM_GIFT_REQ);

	//TMSG msg(pData);
	//DEF &def = msg.Data();
}

//void PM::INVENTORY::CUSTOM_BUY_ANS::Proc(_LPMSGBUF pData)
//{
//	CHECK_AND_DELETE_PENDING_MSG(CUSTOM_BUY_REQ);
//
//	//TMSG msg(pData);
//	//DEF &def = msg.Data();
//}

void PM::INVENTORY::REPAIR_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(REPAIR_REQ, pmsgbuf);

	BREAK_SECTION_BEGIN()
	{
		if (!_StateController->PlayerInfo.IsValid())
			break;

		FPlayerInfo &Player = _StateController->PlayerInfo;
		CInventory &Inven = Player.Inven;

		PM::INVENTORY::REPAIR_REQ::TMSG pmsg(pmsgbuf);
		TMSG msg(pData);

		ITEM_INFO *pInfo = Inven.GetInvenToSN(pmsg.Data().item_sn);
		if (pInfo)
		{
			if (msg.Data().result == RC_ITEM_FULL_GAUGE)
			{
				pInfo->limit = ITEM_LIMIT_INITED;
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("already repaired"), TEXT(""), 0, 0);
				break;
			}

			if (msg.Data().result != RC_OK)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("failed"), TEXT(""), 0, 0);
				break;
			}

			TMONEY Cost = Inven.GetItemRepairMoney(pmsg.Data().item_sn);
			if (Cost == 0)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("already repaired"), TEXT(""), 0, 0);
				break;
			}
			if (Player.PlayerInfo.money < Cost)
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("no money"), TEXT(""), 0, 0);
				break;
			}

			if (Inven.ItemRepair(pmsg.Data().item_sn))
			{
				Player.PlayerInfo.money -= Cost;

				// update RoomPlayerInfo if it is valid
				FRoomPlayerInfo *RoomPlayer = _StateController->GetMyRoomPlayerInfo();
				if (RoomPlayer)
				{
					for (INT i = 0; i < MAX_WEAPONSET_SIZE; ++i)
					{
						if (Player.PlayerInfo.itemInfo.weaponSet[i] == pInfo->sn)
						{
							RoomPlayer->RoomPlayerInfo.weaponItem[i].limitPerc = 100;
							break;
						}
					}
				}

				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("ok"), TEXT(""), 0, 0);
				break;
			}
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Repair, TEXT("failed"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::CONVERT_RIS_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(CONVERT_RIS_REQ, pmsgbuf);

	BREAK_SECTION_BEGIN()
	{
		if (!_StateController->PlayerInfo.IsValid())
			break;

		PM::INVENTORY::CONVERT_RIS_REQ::TMSG pmsg(pmsgbuf);
		TMSG msg(pData);

		if (msg.Data().result != RC_OK)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("failed"), TEXT(""), 0, 0);
			break;
		}

		CInventory &Inven = _StateController->PlayerInfo.Inven;

		ITEM_DESC *Desc = _ItemDesc().GetItem(Inven.GetItemIdToSN(pmsg.Data().item_sn));
		if (!Desc || !Desc->bRisConvertible)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("invalid slot"), TEXT(""), 0, 0);
			break;
		}

		if (Desc->RisConvertiblePrice > _StateController->PlayerInfo.PlayerInfo.money)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("no money"), TEXT(""), 0, 0);
			break;
		}

		if ( Inven.ItemConvertRis(pmsg.Data().item_sn, msg.Data().item_sn) )
		{
			_StateController->PlayerInfo.PlayerInfo.money -= Desc->RisConvertiblePrice;

			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("ok"), TEXT(""), 0, 0);
		}
		else
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_ConvertRIS, TEXT("failed"), TEXT(""), 0, 0);
		}
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::ITEM_REFUND_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(ITEM_REFUND_REQ, pmsgbuf);

	BREAK_SECTION_BEGIN()
	{
		if (!_StateController->PlayerInfo.IsValid())
			break;

		PM::INVENTORY::ITEM_REFUND_REQ::TMSG pmsg(pmsgbuf);
		TMSG msg(pData);

		if (msg.Data().result != RC_OK)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("failed"), TEXT(""), 0, 0);
			break;
		}

		CInventory &Inven = _StateController->PlayerInfo.Inven;
		if (!Inven.IsRefund(pmsg.Data().item.sn))
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("cannot sell"), TEXT(""), 0, 0);
			break;
		}

		TMONEY Money = Inven.GetItemRefundMoney(pmsg.Data().item.sn);
		_StateController->PlayerInfo.PlayerInfo.money += Money;
		if (IsWeaponItem(pmsg.Data().item.id))
		{
			TID_EQUIP_SLOT Slot = ID_INVALID_EQUIP_SLOT;
			if ( !Inven.DelWeaponInven(pmsg.Data().item.sn, Slot) )
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("cannot remove"), TEXT(""), 0, 0);
				break;
			}

			if (Slot != ID_INVALID_EQUIP_SLOT)
			{
				ITEM_INFO *Info = Inven.GetWeaponSet(Slot);
				if (Info)
				{
					ITEM_DESC *Desc = _ItemDesc().GetItem(Info->id);
					if (Desc /*&& Desc->isDefaultItem*/)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("set default"), TEXT("weapon"), 0, 0);
						break;
					}
				}
			}
		}
		else if (IsEquipItem(pmsg.Data().item.id))
		{
			TID_EQUIP_SLOT Slot = ID_INVALID_EQUIP_SLOT;
			if ( !Inven.DelEquipInven(pmsg.Data().item.sn, Slot) )
			{
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("cannot remove"), TEXT(""), 0, 0);
				break;
			}

			// empty invalidated grenade slots
			Inven.RebuildEffect();
			_StateController->PlayerInfo.CheckGrenadeSlots();

			if (Slot != ID_INVALID_EQUIP_SLOT)
			{
				ITEM_INFO *Info = Inven.GetEquipSet(Slot);
				if (Info)
				{
					ITEM_DESC *Desc = _ItemDesc().GetItem(Info->id);
					if (Desc /*&& Desc->isDefaultItem*/)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("set default"), TEXT("equip"), 0, 0);
						break;
					}
				}
			}
		}
		else
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("invalid item"), TEXT(""), 0, 0);
			break;
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_Sell, TEXT("ok"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}

void PM::INVENTORY::ITEM_DELETE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	TSN_ITEM ItemSN = def.item_sn;
	CInventory &Inven = _StateController->PlayerInfo.Inven;

	switch (def.itemType)
	{
	case _IF_WEAPON:
	case _IF_EQUIP:
		{
			TID_ITEM idItem = Inven.GetItemIdToSN(ItemSN);
			ITEM_DESC *Desc = Inven.GetItemDescToSN(ItemSN);
			if (idItem != ID_INVALID_ITEM)
			{
				TID_EQUIP_SLOT Slot;
				BOOL Result = ( def.itemType == _IF_WEAPON ? Inven.DelWeaponInven(ItemSN, Slot, TRUE) : Inven.DelEquipInven(ItemSN, Slot, TRUE) );
				if ( Result )
				{
					if (Desc)
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_DeleteItem, Desc->GetName(), TEXT(""), idItem, 0);
					_LOG(TEXT("Item deleted (%s); slot = %d, id = %d, sn = %I64d"),
									(Desc ? Desc->GetName() : TEXT("Unknown")), Slot, idItem, ItemSN);
				}
				else
				{
					_LOG(TEXT("Error! failed to delete the item. iditem = %d"), idItem);
				}
			}
		}
		break;

	case _IF_CUSTOM:
		{
			CUSTOM_SLOT_IDX SlotIdx = (CUSTOM_SLOT_IDX)def.slot;
			CUSTOM_ITEM_INFO *Item = Inven.GetCustomInvenToSlot(ItemSN, SlotIdx);
			ITEM_DESC *Desc = _ItemDesc().GetItem(Item->id);
			ITEM_DESC *Weapon = _StateController->PlayerInfo.IsValid() ?
									_ItemDesc().GetItem( Inven.GetItemIdToSN(Item->item_sn) ) :
									NULL;
			if ( Item && Inven.DelCustomInven(ItemSN, SlotIdx) )
			{
				if (Desc && Weapon)
					GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_DeleteCustom, Desc->GetName(), Weapon->GetName(), Item->id, Weapon->id );

				_LOG(TEXT("Custom item deleted (%s @ %s); weapon id = %d, item sn = %I64d, custom slot = %d, custom item id = %d"),
							(Weapon ? Weapon->GetName() : TEXT("Unknown")), (Desc ? Desc->GetName() : TEXT("Unknown")), Weapon->id, Item->item_sn, Item->slot, Item->id);
			}
		}
		break;

	case _IF_EFFECT:
		{
			TID_ITEM idItem = Inven.GetEffectItemIdToSN(ItemSN);
			EFFECT_ITEM_DESC *Desc = Inven.GetEffectItemDescToSN(ItemSN);
			if (idItem != ID_INVALID_ITEM)
			{
				TID_EQUIP_SLOT Slot;
				BOOL Result = Inven.DelEffectInven(ItemSN, Slot, TRUE);
				if ( Result )
				{
					if (Desc)
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_DeleteEffect, Desc->GetName(), TEXT(""), idItem, 0);
					_LOG(TEXT("Effect item deleted (%s); slot = %d, id = %d, sn = %I64d"),
									(Desc ? Desc->GetName() : TEXT("Unknown")), Slot, idItem, ItemSN);
				}
			}
		}
		break;
	}
}

void PM::INVENTORY::UPDATE_GAUGE_NTF::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	TMSG msg(pData);
	DEF &def = msg.Data();

	CInventory &Inven = _StateController->PlayerInfo.Inven;

	switch (def.itemType)
	{
	case _IF_WEAPON:
		{
			ITEM_INFO *Item = Inven.GetWeaponInven(def.item_sn);
			if (Item)
			{
				Item->limit = def.gauge;
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_UpdateGauge, TEXT(""), TEXT(""), 0, 0);
			}
		}
		break;

	case _IF_EQUIP:
		{
			ITEM_INFO *Item = Inven.GetEquipInven(def.item_sn);
			if (Item)
			{
				Item->limit = def.gauge;
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_UpdateGauge, TEXT(""), TEXT(""), 0, 0);
			}
		}
		break;

	case _IF_EFFECT:
		{
			EFFECT_ITEM_INFO *Item = Inven.GetEffectInven(def.item_sn);
			if (Item)
			{
				Item->limit = def.gauge;
				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_UpdateGauge, TEXT(""), TEXT(""), 0, 0);
			}
		}
		break;
	}
}

void PM::INVENTORY::CASHITEM_BUY_ANS::Proc(_LPMSGBUF pData)
{
	if (!_StateController->PlayerInfo.IsValid())
		return;

	if (!_StateController->BuyingItemInfo.IsProcessing())
	{
		_LOG(TEXT("No item was requested!"));
		return;
	}
	{
		TID_ITEM idItem = _StateController->BuyingItemInfo.idItem;
		_StateController->BuyingItemInfo.Clear();

		TMSG msg(pData);
		DEF &def = msg.Data();

		if (msg.Data().result == RC_INVENTORY_FULL)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("full"), TEXT(""), 0, 0);
			return;
		}
		else if (msg.Data().result != RC_OK)
		{
			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("failed"), TEXT(""), 0, 0);
			return;
		}

		if (def.updatedMoney > 0)
		{
			_StateController->PlayerInfo.PlayerInfo.money = def.updatedMoney;
		}

		CInventory &Inven = _StateController->PlayerInfo.Inven;

		if (IsEffectItem(idItem))
		{
			EFFECT_ITEM_DESC *ItemDesc = _ItemDesc().GetEffectItem(idItem);
			if (ItemDesc)
			{
				//_StateController->PlayerInfo.Cash -= ItemDesc->buyCashPrice;
				//if (_StateController->PlayerInfo.Cash < 0)
				//	_StateController->PlayerInfo.Cash = 0;

				EFFECT_ITEM_INFO *Item = Inven.GetEffectInven(def.item_sn);
				if (Item)
				{
					Item->limit = (ItemDesc->gaugeType == _IGT_DATE ? def.dateLimit : ITEM_LIMIT_INITED);
				}
				else
				{
					EFFECT_ITEM_INFO Item;
					Item.id = idItem;
					Item.item_sn = def.item_sn;
					Item.limit = (ItemDesc->gaugeType == _IGT_DATE ? def.dateLimit : ITEM_LIMIT_INITED);

					if (!Inven.EffectItemBuy(&Item))
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("failed"), TEXT(""), 0, 0);
						return;
					}
				}

				SLOT_DESC *SlotDesc = _ItemDesc().GetEffectSlotByType(ItemDesc->slotType);
				if (!SlotDesc)
				{
					GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("invalid"), TEXT(""), 0, 0);
					return;
				}

				for (INT i = 0; i < MAX_EFFECT_INVENTORY_SIZE; ++i)
				{
					if (_StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[i].item_sn == def.item_sn)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("ok"), TEXT("effect"), SlotDesc->index, i);
						return;
					}
				}

				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("failed"), TEXT(""), 0, 0);
				return;
			}
		}
		else if (IsEquipItem(idItem) || IsWeaponItem(idItem))
		{
			ITEM_DESC *ItemDesc = _ItemDesc().GetItem(idItem);
			if (ItemDesc)
			{
				//_StateController->PlayerInfo.Cash -= ItemDesc->buyCashPrice;
				//if (_StateController->PlayerInfo.Cash < 0)
				//	_StateController->PlayerInfo.Cash = 0;

				ITEM_INFO *Item = (IsEquipItem(idItem) ? Inven.GetEquipInven(def.item_sn) : Inven.GetWeaponInven(def.item_sn));
				if (Item)
				{
					Item->limit = (ItemDesc->gaugeType == _IGT_DATE ? def.dateLimit : ITEM_LIMIT_INITED);
				}
				else
				{
					ITEM_INFO Item;
					Item.id = idItem;
					Item.sn = def.item_sn;
					Item.limit = (ItemDesc->gaugeType == _IGT_DATE ? def.dateLimit : ITEM_LIMIT_INITED);

					if (!Inven.ItemBuy(&Item))
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("failed"), TEXT(""), 0, 0);
						return;
					}
				}

				if (IsEquipItem(idItem))
				{
					SLOT_DESC *SlotDesc = _ItemDesc().GetEquipSlotByType(ItemDesc->slotType);
					if (!SlotDesc)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("invalid"), TEXT(""), 0, 0);
						return;
					}

					for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
					{
						if (_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[i].sn == def.item_sn)
						{
							GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("ok"), TEXT("equip"), SlotDesc->index, i);
							return;
						}
					}
				}
				else
				{
					SLOT_DESC *SlotDesc = _ItemDesc().GetWeaponSlotByType(ItemDesc->slotType);
					if (!SlotDesc)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("invalid"), TEXT(""), 0, 0);
						return;
					}

					for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
					{
						if (_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].sn == def.item_sn)
						{
							GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("ok"), TEXT("weapon"), SlotDesc->index, i);
							return;
						}
					}
				}

				GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("failed"), TEXT(""), 0, 0);
				return;
			}
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffBuy, TEXT("invalid"), TEXT(""), 0, 0);
	}
}

void PM::INVENTORY::EFFSET_ANS::Proc(_LPMSGBUF pData)
{
	_LPMSGBUF pmsgbuf;
	CHECK_PENDING_MSG(EFFSET_REQ, pmsgbuf);

	if (!_StateController->PlayerInfo.IsValid())
	{
		pmsgbuf->Delete();
		return;
	}

	PM::INVENTORY::EFFSET_REQ::TMSG pmsg(pmsgbuf);
	TMSG msg(pData);

	BREAK_SECTION_BEGIN()
	{
		_LOG(TEXT("1"));
		if (msg.Data().result == RC_OK)
		{
			CInventory &Inven = _StateController->PlayerInfo.Inven;

			INT EquipSlot = pmsg.Data().equipSlot;
			TSN_ITEM ItemSN = pmsg.Data().item_sn;
			_LOG(TEXT("EquipSlot = %d, SN = %I64d"), EquipSlot, ItemSN);
			if (EquipSlot >= 0 && EquipSlot < MAX_EFFECTSET_SIZE)
			{
				_LOG(TEXT("2"));
				SLOT_DESC *pSlot = _ItemDesc().GetEffectSlot(EquipSlot);
				if (!pSlot)
				{
					_LOG(TEXT("Error setting effect item; invalid slot"));
					break;
				}

				_LOG(TEXT("3"));
				TSN_ITEM ChangedSN = ItemSN;
				BOOL Result = Inven.EffectSet(EquipSlot, ItemSN, &ChangedSN);

				_LOG(TEXT("%s"), Result ? TEXT("succ") : TEXT("fail"));

				if (ItemSN == SN_INVALID_ITEM && ChangedSN != SN_INVALID_ITEM)
				{
					EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem( Inven.GetEffectItemIdToSN(ChangedSN) );
					if (Desc)
					{
						GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("set default"), Desc->GetName(), 0, 0);
						break;
					}
				}
			}

			GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("ok"), TEXT(""), 0, 0);
			break;
		}

		GetAvaNetHandler()->ProcMessage(EMsg_Inventory, EMsg_Inventory_EffSet, TEXT("failed"), TEXT(""), 0, 0);
	}
	BREAK_SECTION_END()

	pmsgbuf->Delete();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

void PM::INVENTORY::Proc(_LPMSGBUF pData)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)pData->GetData();

	switch (pHeader->msg_id)
	{
		CASE_MSG_PROC(EQUIPSET_ANS)
		CASE_MSG_PROC(WEAPONSET_ANS)
		CASE_MSG_PROC(CUSTOMSET_ANS)
		CASE_MSG_PROC(ITEM_BUY_ANS)
		CASE_MSG_PROC(ITEM_GIFT_ANS)
		CASE_MSG_PROC(REPAIR_ANS)
		CASE_MSG_PROC(CONVERT_RIS_ANS)
		CASE_MSG_PROC(ITEM_REFUND_ANS)
		CASE_MSG_PROC(ITEM_DELETE_NTF)
		CASE_MSG_PROC(CASHITEM_BUY_ANS)
		CASE_MSG_PROC(EFFSET_ANS)

	default:
		_LOG(TEXT("Invalid INVENTORY message received. ID = %d"), pHeader->msg_id);
	}
}

void PM::INVENTORY::ProcTimeOut(const BYTE *Buffer, INT BufferLen)
{
	MSG_HEADER *pHeader = (MSG_HEADER*)Buffer;

	switch (pHeader->msg_id)
	{
		CASE_MSG_TIMEOUT_PROC(Inventory, EquipSet, EQUIPSET_REQ)
		CASE_MSG_TIMEOUT_PROC(Inventory, WeaponSet, WEAPONSET_REQ)
		CASE_MSG_TIMEOUT_PROC(Inventory, Buy, ITEM_BUY_REQ)
		CASE_MSG_TIMEOUT_PROC(Inventory, Repair, REPAIR_REQ)
		CASE_MSG_TIMEOUT_PROC(Inventory, ConvertRIS, CONVERT_RIS_REQ)
		CASE_MSG_TIMEOUT_PROC(Inventory, Sell, ITEM_REFUND_REQ)
		CASE_MSG_TIMEOUT_PROC(Inventory, EffSet, EFFSET_REQ)
	default:
		_LOG(TEXT("Some INVENTORY message timed out. ID = %d"), pHeader->msg_id);
	}
}
