#pragma once
#ifndef __AVA_NET_ACCESSOR__
#define __AVA_NET_ACCESSOR__

#include "avaNetStateController.h"
#include "avaStaticData.h"
#include "avaCommunicator.h"
#include "ComDef/Inventory.h"

//////////////////////////////////////////////////////////////////////////
/// MyPlayer
//////////////////////////////////////////////////////////////////////////
FORCEINLINE INT GetMyPlayerMoney()
{
	return _StateController->PlayerInfo.PlayerInfo.money;
}

FORCEINLINE INT GetMyPlayerCash()
{
	return _StateController->PlayerInfo.Cash;
}

//////////////////////////////////////////////////////////////////////////
/// Channel
//////////////////////////////////////////////////////////////////////////

FORCEINLINE UBOOL IsClanHome( INT ChannelListIndex )
{
	return _StateController->ChannelList.ChannelList.Num() == ChannelListIndex;
}

FORCEINLINE BYTE GetCurrentChannelGroup()
{
	return _StateController->ChannelInfo.IsValid() ? _StateController->ChannelInfo.Flag : UCHAR_MAX;
}

FORCEINLINE FChannelInfo* GetChannelInfoByIndex( INT ChannelListIndex )
{
	if( !_StateController->ChannelList.ChannelList.IsValidIndex( ChannelListIndex ) )
		return NULL;
	return &_StateController->ChannelList.ChannelList( ChannelListIndex );
}

//////////////////////////////////////////////////////////////////////////
/// Lobby, ReadyRoom
//////////////////////////////////////////////////////////////////////////

FORCEINLINE FRoomDispInfo* GetRoomInfoByIndex( INT RoomListIndex )
{
	if( !_StateController->RoomList.RoomList.IsValidIndex( RoomListIndex ) )
		return NULL;

	return &_StateController->RoomList.RoomList( RoomListIndex );
}

FORCEINLINE UBOOL IsStateInRoom() { return _StateController->IsStateInRoom(); }

FORCEINLINE FRoomDispInfo* GetSelectedRoomInfo()
{
	if( _StateController->IsStateInRoom() )
		return _StateController->RoomInfo.IsValid() ? &_StateController->RoomInfo : NULL;
	else if( _StateController->GetNetState() == _AN_LOBBY ) 
		return _StateController->RoomList.GetSelected();
	return NULL;
}

FORCEINLINE UBOOL IsAcceptableRoom( FRoomDispInfo* RoomInfo )
{
	if( RoomInfo == NULL )
		return FALSE;

	UBOOL bResult = FALSE;
	UBOOL bValidCount = RoomInfo->RoomInfo.state.numCurr < RoomInfo->RoomInfo.setting.numMax;
	if( _StateController->ChannelInfo.IsFriendlyGuildChannel() )
	{
		TID_GUILD idGuild = _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuild;
		UBOOL bValidGuild = (RoomInfo->RoomInfo.idGuild[0] == idGuild || RoomInfo->RoomInfo.idGuild[0] == ID_INVALID_GUILD ||
			RoomInfo->RoomInfo.idGuild[1] == idGuild || RoomInfo->RoomInfo.idGuild[1] == ID_INVALID_GUILD);
		bResult = bValidGuild && bValidCount;

	}
	else if ( _StateController->ChannelInfo.IsNormalChannel() )
	{
		bResult = bValidCount;
	}
	else if ( _StateController->ChannelInfo.IsMyClanChannel() )
	{
		bResult = bValidCount;
	}

	return bResult;
}

FORCEINLINE UBOOL IsAcceptableRoom( INT RoomListIndex )
{
	if( !_StateController->RoomList.RoomList.IsValidIndex(RoomListIndex) )
		return FALSE;

	return IsAcceptableRoom( &_StateController->RoomList.RoomList(RoomListIndex) );
}


FORCEINLINE FMapInfo* GetSelectedMapInfo()
{
	if (_StateController->IsStateInRoom())
	{
		return _StateController->GetCurrentMap();
	}
	else if (_StateController->GetNetState() == _AN_LOBBY && _StateController->ChannelInfo.IsValid())
	{
		FRoomDispInfo *Room = _StateController->RoomList.GetSelected();
		if (Room)
			return _StateController->MapList.Find(Room->RoomInfo.setting.idMap);
	}
	return NULL;
}

FORCEINLINE INT GetMapList( TArray<FMapInfo>& OutMapList )
{
	OutMapList = _StateController->MapList.MapList;
	return _StateController->MapList.MapList.Num();
}

FORCEINLINE FMapInfo* GetMapInfo( INT MapListIndex )
{
	return _StateController->MapList.MapList.IsValidIndex(MapListIndex) ? &_StateController->MapList.MapList(MapListIndex) : NULL;
}

//////////////////////////////////////////////////////////////////////////
/// PlayerInfo
//////////////////////////////////////////////////////////////////////////

FORCEINLINE FPlayerDispInfo* GetSelectedPlayerDispInfo()
{
	FPlayerDispInfo* Player = NULL;

	if (_StateController->GetNetState() == _AN_ROOM)
	{
		if (_StateController->RoomInfo.IsValid())
		{
			Player = _StateController->RoomInfo.PlayerList.GetSelected();
			if (!Player)
			{
				if (!_StateController->RoomInfo.PlayerList.IsEmpty(_StateController->MyRoomSlotIdx))
					Player = &(_StateController->RoomInfo.PlayerList.PlayerList[_StateController->MyRoomSlotIdx]);
			}
		}
	}
	else if (_StateController->GetNetState() == _AN_LOBBY)
	{
		if (_StateController->ChannelInfo.IsValid() || _StateController->ChannelInfo.IsMyClanChannel())
		{
			Player = _StateController->LobbyPlayerList.GetSelected();
			if (!Player)
			{
				for (INT i = 0; i < _StateController->LobbyPlayerList.PlayerList.Num(); ++i)
				{
					if (_StateController->LobbyPlayerList.PlayerList(i).PlayerInfo.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
						Player = &(_StateController->LobbyPlayerList.PlayerList(i));
				}
			}
		}
	}

	return Player;
}

FORCEINLINE FPlayerDispInfo* GetRoomPlayerDispInfo( BYTE RoomSlotIdx )
{
	FPlayerDispInfo* Player = NULL;

	if (_StateController->GetNetState() == _AN_ROOM &&
		_StateController->RoomInfo.IsValid() &&
		(0 <= RoomSlotIdx && RoomSlotIdx < Def::MAX_PLAYER_PER_ROOM))
	{
		Player = &_StateController->RoomInfo.PlayerList.PlayerList[RoomSlotIdx];
	}

	return Player;
}

//////////////////////////////////////////////////////////////////////////
/// Boost Items ( XP, Money, Supply )
//////////////////////////////////////////////////////////////////////////

FORCEINLINE UBOOL GetEffectItemByType( Def::PLAYER_ITEM_INFO& ItemInfo, BYTE EffectType, EFFECT_ITEM_INFO** EffectItemInfo, EFFECT_ITEM_DESC** EffectItemDesc = NULL )
{
	if( EffectItemDesc == NULL || EffectItemInfo == NULL )
		return FALSE;

	*EffectItemDesc = NULL;
	*EffectItemInfo = NULL;

	EFFECT_ITEM_INFO* pItem = NULL;
	EFFECT_ITEM_DESC* pDesc = NULL;
	for( INT i = 0 ; i < MAX_EFFECT_INVENTORY_SIZE ; i++ )
	{
		pItem = &ItemInfo.effectInven[i];
		pDesc = _ItemDesc().GetEffectItem(pItem->id);
		if( pItem->id != ID_INVALID_ITEM && pDesc != NULL &&
			pDesc->effectInfo.effectType == EffectType )
		{
			break;
		}
	}

	if( EffectItemInfo )
		*EffectItemInfo = pItem;
	if( EffectItemDesc )
		*EffectItemDesc = pDesc;

	return pItem || pDesc;
}

FORCEINLINE	UBOOL IsBoostItemMsgType( BYTE MsgType )
{
	return MsgType == LastResultMsgType_MoneyBoost || MsgType == LastResultMsgType_XP || MsgType == LastResultMsgType_Supply;
}


FORCEINLINE UBOOL CanApplyLastResult( INT MsgType = INDEX_NONE )
{
	if( MsgType == INDEX_NONE )
		return !_StateController->ChannelInfo.IsMyClanChannel();
	else
		return !_StateController->ChannelInfo.IsMyClanChannel() || !IsBoostItemMsgType(MsgType);
}

#endif