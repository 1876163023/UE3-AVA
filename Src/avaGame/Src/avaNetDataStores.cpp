/*=============================================================================
	avaNetDataStores.cpp: UI data store class implementations
=============================================================================*/

#include "avaGame.h"

// widgets and supporting UI classes
#include "UnUIMarkupResolver.h"
#include "AvaSortTemplate.h"

//#include "avaNet.h"
#include "avaNetClient.h"
#include "avaNetStateController.h"
#include "avaStaticData.h"
#include "avaCommunicator.h"
#include "ComDef/Inventory.h"

#include "AvaNetAccessor.h"
#include "avaNetClasses.h"
#include "avaTransactions.h"

IMPLEMENT_CLASS(UUIDataStore_AvaNet);

/************************************************************************/
/* Icon, Text MarkupString		                                       */
/************************************************************************/
inline static INT GetInvenIndexFromID( INT InvenID )
{
	INT InvenIndex = INDEX_NONE;

	// EquipInven
	if( InvenID >= 10000 )
		InvenIndex = InvenID % 10000;

	// WeaponInven
	else if ( (0 <= InvenID && InvenID < 100 * Def::MAX_WEAPONSET_SIZE)
		&& (InvenID %= 100) < Def::MAX_INVENTORY_SIZE )
	{
		InvenIndex = InvenID;
	}

	return InvenIndex;
}

inline static FString GetPCBangIconString()
{
	return Localize(TEXT("UIGeneral"), TEXT("Icon_PCBang"), TEXT("avaNet"));
}

inline static FString GetBIAIconString()
{
	return Localize(TEXT("UIResultScene"), TEXT("Icon_BIAPoint"), TEXT("avaNet"));
}

inline static FString GetMoneyMarkString()
{
	return Localize(TEXT("UIGeneral"), TEXT("Text_MoneyMark"), TEXT("avaNet"));
}

inline static FString GetCashString()
{
	return Localize(TEXT("UIGeneral"), TEXT("Text_Cash"), TEXT("avaNet"));
}

inline static FString GetLevelIconString( BYTE Level )
{
	//	return Localize(TEXT("PlayerLevelName"), *FString::Printf(TEXT("Icon_Level[%d]"),Level), TEXT("avaNet"));
	return FString::Printf( TEXT("<Strings:avaNet.PlayerLevelName.Icon_Level[%d]>"), Level);
}

inline static FString GetLevelNameString( BYTE Level )
{
	//	return Localize(TEXT("PlayerLevelName"), *FString::Printf(TEXT("Name_Level[%d]"),Level), TEXT("avaNet"));
	return FString::Printf( TEXT("<Strings:avaNet.PlayerLevelName.Name_Level[%d]>"), Level);
}

inline static FString GetDateLimitString(BYTE day, INT GaugeType=_IGT_DATE)
{
	return GaugeType == _IGT_DATE ? appItoa(day) + Localize(TEXT("UIGeneral"), TEXT("Text_Day"), TEXT("avaNet")) : TEXT(" ");
}

inline static FString GetFmtMoneyString( INT Money )
{
	static const int RestUnitCount = 3;
	FString FmtMoneyStr = appItoa(Money);
	INT RestCount = FmtMoneyStr.Len() > 0 ? Money > 0 ? (FmtMoneyStr.Len() - 1) / RestUnitCount : (FmtMoneyStr.Len() - 2) / RestUnitCount : 0;
	for( INT i = 0 ; i < RestCount ; i++)
	{
		INT SplitRightCount = (i + 1)*RestUnitCount + i;
		FmtMoneyStr = FmtMoneyStr.Left( FmtMoneyStr.Len() - SplitRightCount) + TEXT(",") + FmtMoneyStr.Right( SplitRightCount );
	}

	return FmtMoneyStr;
}

inline static FString GetCashMoneyString(INT money)
{
	return GetFmtMoneyString(money) + Localize( TEXT("UIGeneral"), TEXT("Text_Won"), TEXT("avaNet"));
}

inline static FString GetBonusMoneyString(INT money, UBOOL bMoneyIcon=FALSE)
{
	FString moneyMark = bMoneyIcon ? Localize( TEXT("UIGeneral"), TEXT("Icon_MoneyMark"), TEXT("avaNet")) : TEXT("");

//	debugf(TEXT("GetBonusMoneyString, %s, %d"), *GetFmtMoneyString(money), bMoneyIcon);

	return money ? moneyMark + GetFmtMoneyString(money) : TEXT(" ");
}

inline static FString GetItemSlotTypeString(TID_ITEM id, INT slotType)
{
	static const TCHAR *EquipTypes[] =
	{
		TEXT(""),
		TEXT("H1"), 
		TEXT("H11"), 
		TEXT("H12"), 
		TEXT("H2"), 
		TEXT("H3"), 
		TEXT("C1"), 
		TEXT("C2"), 
		TEXT("A1"), 
		TEXT("A2"), 
		TEXT("B1"), 
		TEXT("B3"), 
		TEXT("W1"), 
		TEXT("W2"), 
		TEXT("W3"), 
		TEXT("T1"), 
		TEXT("T2"), 
		TEXT("E"), 
		TEXT("G"), 
		TEXT("K"), 
		TEXT("BT"), 
		TEXT("BD"), 
		TEXT("FACE"), 
	};

	static const TCHAR *WeaponTypes[] =
	{
		TEXT(""),
		TEXT("R1"), 
		TEXT("R2"), 
		TEXT("R3"), 
		TEXT("R4"), 
		TEXT("P1"), 
		TEXT("P2"), 
		TEXT("P3"), 
		TEXT("P4"), 
		TEXT("S1"), 
		TEXT("S2"), 
		TEXT("S3"), 
		TEXT("S4"), 
	};

	static const TCHAR *EffectTypes[] =
	{
		TEXT(""),
		TEXT("EE"),
		TEXT("ES"),
		TEXT("EM"),
	};

	INT slotIndex = 0;
	for(INT i = 1; i < 0x80000000; i <<= 1, slotIndex++)
	{
		if ( slotType & i )
			break;
	}

	FString Result;
	const TCHAR *pKeyStr;

	if ( IsEquipItem(id) )
	{
		Result = Localize(TEXT("ItemSlotName"), *FString::Printf(TEXT("EquipTypes[%s]"), EquipTypes[slotIndex]), TEXT("avaNet"));
		pKeyStr = EquipTypes[slotIndex];
	}
	else if ( IsWeaponItem(id) )
	{
		Result = Localize(TEXT("ItemSlotName"), *FString::Printf(TEXT("WeaponTypes[%s]"), WeaponTypes[slotIndex]), TEXT("avaNet"));
		pKeyStr = WeaponTypes[slotIndex];
	}
	else
	{
		Result = Localize(TEXT("ItemSlotName"), *FString::Printf(TEXT("EffectTypes[%s]"), EffectTypes[slotIndex]), TEXT("avaNet"));
		pKeyStr = EffectTypes[slotIndex];
	}

//	debugf(TEXT("GetItemSlotTypeString.Result(%s) %s[%d]"), *Result, pKeyStr, slotIndex);

	return Result;
}

// Gauge 를 만드는 동작으로 GetSupplyGauge, GetXPGauge, GetInvenGauge 에서 쓰임
inline static FString GetGaugeIconString( FString& LeftGaugeStr, FString& RightGaugeStr, FLOAT Ratio)
{
	static const FString WidthStr = TEXT("XL=");
	static const FString TexUStr = TEXT("U=");
	INT XL = -1, U = -1, Offset = 0;
	if( Parse( *LeftGaugeStr, *WidthStr, XL) && XL > 0)
	{
		INT NewXL = XL * Ratio;
		Offset = NewXL;
		LeftGaugeStr = NewXL == 0 ? FString(TEXT("")) : LeftGaugeStr.Replace( *(WidthStr + appItoa(XL)), *(WidthStr + appItoa(NewXL)) );
	}
	if( Parse( *RightGaugeStr, *WidthStr, XL) && XL > 0 
		&& Parse( *RightGaugeStr, *TexUStr, U) && U >= 0)
	{
		INT NewXL = XL - Offset;
		RightGaugeStr = Offset == 0 ? RightGaugeStr : RightGaugeStr.Replace( *(TexUStr + appItoa(U)), *(TexUStr + appItoa(U + Offset)) );
		RightGaugeStr = NewXL == 0 ? FString(TEXT("")) : RightGaugeStr.Replace( *(WidthStr + appItoa(XL)), *(WidthStr + appItoa(NewXL)) );
	}

	return LeftGaugeStr + RightGaugeStr;
}

/*! @brief GaugeType에 따라서 제한 기간, 시간, 정비도를 표시한다.
	@param GaugeType
		(deftype.h)
		enum _ITEM_GAUGE_TYPE_
		{
			_IGT_NONE = 0,			// 사고파는 아이탬이 아니다.
			_IGT_MAINTENANCE = 1,	// 정비도
			_IGT_DURABILITY = 2,	// 내구도
			_IGT_DATE = 3,			// 날짜..
		};
	@param Limit
		GuageType에 따라서 정비도, 내구도, 날짜가 된다.
	@param Ratio
		날짜인 경우 전체 Limit값을 알 수 없음으로 직접 넣어 줄 수 있도록 추가함.
*/
static FString GetItemGaugeIconString( BYTE GaugeType, INT Limit, FLOAT Ratio=1.0f)
{
	FString GaugeLocalString[2];
	FString GaugeResultString;

	if ( GaugeType == _IGT_DURABILITY )
	{
		GaugeLocalString[0] = Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_Durability[0]"), TEXT("avaNet"));
		GaugeLocalString[1] = Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_Durability[1]"), TEXT("avaNet"));

		Limit = Clamp<INT>(Limit, 0, ITEM_LIMIT_INITED);
		Ratio = Limit / (FLOAT)ITEM_LIMIT_INITED;
		GaugeResultString = GetGaugeIconString( GaugeLocalString[0], GaugeLocalString[1],  Ratio );
	}
	else if ( GaugeType == _IGT_MAINTENANCE )
	{
		Limit = Clamp<INT>(Limit, 0, ITEM_LIMIT_INITED);
		Ratio = Limit / (FLOAT)ITEM_LIMIT_INITED;
		GaugeResultString = FString::Printf(*Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_Maintenance"), TEXT("avaNet")), Ratio * 100);
	}
	else if ( GaugeType == _IGT_DATE )
	{
		time_t worldTime = static_cast<time_t>(_StateController->GetNow());
		time_t itemTime  = static_cast<time_t>(Limit);
		time_t remainTime = itemTime - worldTime;

		// 이미 사용기간이 지난 경우.
		if ( remainTime <= 0 )
			GaugeResultString = appItoa(0) + Localize(TEXT("UIGeneral"), TEXT("Text_Hour"), TEXT("avaNet"));
		else
		{
			static const time_t OneMin = 60;
			static const time_t OneHour = 60 * OneMin;
			static const time_t OneDay = 24 * OneHour;

			if ( remainTime >= OneDay )
				GaugeResultString = appItoa(remainTime / OneDay) + Localize(TEXT("UIGeneral"), TEXT("Text_Day"), TEXT("avaNet"));
			else
			{
				// 빨간색으로 표시한다.
				Ratio = (float)remainTime / OneDay;
				GaugeResultString = appItoa(remainTime / OneHour) + Localize(TEXT("UIGeneral"), TEXT("Text_Hour"), TEXT("avaNet"));
			}
		}
	}
	else if ( GaugeType == _IGT_NONE )
	{
		GaugeResultString = Localize(TEXT("UIInventoryScene"), TEXT("Text_GaugeType_None"), TEXT("avaNet"));
	}
	else
	{
		GaugeResultString = TEXT("Unknown GaugeType");
	}

	if( GaugeType == _IGT_DURABILITY || GaugeType == _IGT_MAINTENANCE || GaugeType == _IGT_DATE )
	{
		FColor CandidateColor[2] = { FLinearColor::White, FLinearColor::White };
		FString NewColorString[2];
		FLOAT RatioInRange = 1.f;
		if ( Ratio < 0.01f )
		{
			NewColorString[0] = TEXT("Broken");
			NewColorString[1] = TEXT("Broken");
		}
		else if ( Ratio < (1.f/3.f) )
		{
			NewColorString[0] = TEXT("Broken");
			NewColorString[1] = TEXT("Bad");
			RatioInRange = Ratio * 3.f;
		}
		else if ( Ratio < (2.f/3.f) )
		{
			NewColorString[0] = TEXT("Bad");
			NewColorString[1] = TEXT("Normal");
			RatioInRange = (Ratio - (1.f/3.f)) * 3.f;
		}
		else
		{
			NewColorString[0] = TEXT("Normal");
			NewColorString[1] = TEXT("Good");
			RatioInRange = (Ratio - (2.f/3.f)) * 3.f;
		}
		NewColorString[0] = Localize(TEXT("UIInventoryScene"), *FString::Printf(TEXT("IconMod_Maintenance[%s]"), *NewColorString[0]), TEXT("avaNet"));
		NewColorString[1] = Localize(TEXT("UIInventoryScene"), *FString::Printf(TEXT("IconMod_Maintenance[%s]"), *NewColorString[1]), TEXT("avaNet"));

		Parse( *NewColorString[0], TEXT("R="), CandidateColor[0].R);
		Parse( *NewColorString[0], TEXT("G="), CandidateColor[0].G);
		Parse( *NewColorString[0], TEXT("B="), CandidateColor[0].B);
		Parse( *NewColorString[1], TEXT("R="), CandidateColor[1].R);
		Parse( *NewColorString[1], TEXT("G="), CandidateColor[1].G);
		Parse( *NewColorString[1], TEXT("B="), CandidateColor[1].B);

		FColor NewColor = CandidateColor[1];//(FLinearColor(CandidateColor[0]) + FLinearColor(CandidateColor[1])) * 0.5f;
		GaugeResultString = FString::Printf(TEXT("<Color:R=%d,G=%d,B=%d>%s<Color:/>"), NewColor.R, NewColor.G, NewColor.B, *GaugeResultString);
	}

//	debugf(TEXT("GetItemGaugeIconString( %d, %d ) - GaugeResultString[%s]"), GaugeType, Limit, *GaugeResultString);

	return GaugeResultString;
}

inline static FString GetItemGaugePercentString( BYTE GaugeType, INT Limit )
{
	FString ResultString;
	FLOAT Percent = (Limit / (FLOAT)ITEM_LIMIT_INITED) * 100.f;

	switch( GaugeType )
	{
	case _IGT_DURABILITY:	
	case _IGT_MAINTENANCE:	ResultString = FString::Printf(TEXT("%.1f%%"),Percent);	break;
	case _IGT_DATE:			ResultString = appItoa(Limit) + Localize(TEXT("UIGeneral"), TEXT("Text_Day"), TEXT("avaNet"));	break;
	case _IGT_NONE:			ResultString = Localize(TEXT("UIInventoryScene"), TEXT("Text_GaugeType_None"), TEXT("avaNet"));	break;
	default:				ResultString = TEXT("Unknown GaugeType");	break;
	}

	return ResultString;
}

inline static FString GetSupplyGaugeIconString( INT Limit )
{
	Limit = Clamp<INT>(Limit, 0, SUPPLYPOINT_MAX);

	FString GaugeLocalString[2];
	FString GaugeResultString;
	GaugeLocalString[0] = Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_SupplyGauge[0]"), TEXT("avaNet"));
	GaugeLocalString[1] = Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_SupplyGauge[1]"), TEXT("avaNet"));

	FLOAT Ratio = Limit / (FLOAT)SUPPLYPOINT_MAX;
	GaugeResultString = GetGaugeIconString( GaugeLocalString[0], GaugeLocalString[1], Ratio );
	return GaugeResultString;
}

inline static FString GetXPGaugeString( BYTE XpProgress )
{
	FLOAT Ratio = Clamp<BYTE>( XpProgress, 0, 100 ) / 100.f;

	FString GaugeLocalString[2];
	FString GaugeResultString;
	GaugeLocalString[0] = Localize(TEXT("PlayerStat"), TEXT("Icon_Common_XPGauge[0]"), TEXT("avaNet"));
	GaugeLocalString[1] = Localize(TEXT("PlayerStat"), TEXT("Icon_Common_XPGauge[1]"), TEXT("avaNet"));

	GaugeResultString = GetGaugeIconString( GaugeLocalString[0], GaugeLocalString[1], Ratio );
	return GaugeResultString;
}

extern FString GetAwardGaugeIconString( INT AwardProgress )
{
	FLOAT Ratio = (FLOAT)Clamp<BYTE>(AwardProgress, 0, 100) / 100.0f;

	// 달성했다면 글자로 출력한다.
	if ( Ratio >= 1.0 )
		return Localize(TEXT("PlayerStat"),TEXT("Text_List_AwardAchievement"), TEXT("avaNet"));

	FString GaugeLocalString[2];
	FString GaugeResultString;
	GaugeLocalString[0] = Localize(TEXT("PlayerStat"), TEXT("Icon_Common_AwardGauge[0]"), TEXT("avaNet"));
	GaugeLocalString[1] = Localize(TEXT("PlayerStat"), TEXT("Icon_Common_AwardGauge[1]"), TEXT("avaNet"));

	GaugeResultString = GetGaugeIconString( GaugeLocalString[0], GaugeLocalString[1], Ratio );
	return GaugeResultString;
}

inline static FString GetXPComboString( INT XP, INT XpIncrPercent )
{
	static const FString IncrIcon = TEXT("<Strings:avaNet.UIGeneral.Icon_Increase_SmallSize>");

	FString ResultStr = FString::Printf(TEXT("%s%d"), XP > 0 ? TEXT("+") : TEXT("") , XP );
	ResultStr += XpIncrPercent > 0 ? FString::Printf(TEXT("(%s%d%%)"), *IncrIcon, XpIncrPercent) : FString(TEXT(""));

	return ResultStr;
}

inline static FString GetSupplyComboString( INT Supply, INT SupplyIncrPercent )
{
	static const FString IncrIcon = TEXT("<Strings:avaNet.UIGeneral.Icon_Increase_SmallSize>");

	FString ResultStr = FString::Printf(TEXT("%s%d"), Supply > 0 ? TEXT("+") : TEXT(""), Supply );
	ResultStr += SupplyIncrPercent > 0 ? FString::Printf(TEXT("(%s%d%%)"), *IncrIcon, SupplyIncrPercent) : FString(TEXT(""));

	return ResultStr;
}

inline static FString GetSlotImageString( CUSTOM_SLOT_TYPE AvailCustomSlots, CUSTOM_SLOT_TYPE* EquippedCustomSlots = NULL )
{
	FString SlotImageStr;
	const FString ColorStr = Localize(TEXT("UIInventoryScene"),TEXT("IconMod_SlotNotEquipped"), TEXT("avaNet"));
	static const TCHAR* SlotImagePrefix = TEXT("Icon_WeaponCustom_%s[%s]");
	static const TCHAR* SlotStatus[] = { TEXT("Active"), TEXT("Enabled"), TEXT("Disabled") };
	static const TCHAR* SlotPos[] = { TEXT("Front"), TEXT("Mount"), TEXT("Barrel"), TEXT("Trigger"), TEXT("Grip"), TEXT("Stock") };

	SlotImageStr += AvailCustomSlots.front ? (EquippedCustomSlots && EquippedCustomSlots->front) ? 
		Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[0],SlotStatus[0] ) ,TEXT("avaNet")) : 
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[0],SlotStatus[1] ) ,TEXT("avaNet")) :
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[0],SlotStatus[2] ) ,TEXT("avaNet"));
	SlotImageStr += AvailCustomSlots.mount ? (EquippedCustomSlots && EquippedCustomSlots->mount) ?
		Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[1],SlotStatus[0] ) ,TEXT("avaNet")) : 
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[1],SlotStatus[1] ) ,TEXT("avaNet")) :
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[1],SlotStatus[2] ) ,TEXT("avaNet"));
	SlotImageStr += AvailCustomSlots.barrel ? (EquippedCustomSlots && EquippedCustomSlots->barrel) ?
		Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[2],SlotStatus[0] ) ,TEXT("avaNet")) : 
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[2],SlotStatus[1] ) ,TEXT("avaNet")) :
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[2],SlotStatus[2] ) ,TEXT("avaNet"));
	SlotImageStr += AvailCustomSlots.trigger ? (EquippedCustomSlots && EquippedCustomSlots->trigger) ?
		Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[3],SlotStatus[0] ) ,TEXT("avaNet")) : 
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[3],SlotStatus[1] ) ,TEXT("avaNet")) :
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[3],SlotStatus[2] ) ,TEXT("avaNet"));
	SlotImageStr += AvailCustomSlots.grip ? (EquippedCustomSlots && EquippedCustomSlots->grip) ?
		Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[4],SlotStatus[0] ) ,TEXT("avaNet")) : 
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[4],SlotStatus[1] ) ,TEXT("avaNet")) :
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[4],SlotStatus[2] ) ,TEXT("avaNet"));
	SlotImageStr += AvailCustomSlots.stock ? (EquippedCustomSlots && EquippedCustomSlots->stock) ?
		Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[5],SlotStatus[0] ) ,TEXT("avaNet")) : 
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[5],SlotStatus[1] ) ,TEXT("avaNet")) :
	Localize(TEXT("UIInventoryScene"), *FString::Printf(SlotImagePrefix, SlotPos[5],SlotStatus[2] ) ,TEXT("avaNet"));

	return SlotImageStr;
}

inline static FString GetSlotImageString( CUSTOM_SLOT_TYPE AvailCustomSlots, ITEM_INFO* ItemInfo)
{
	CUSTOM_SLOT_TYPE EquippedCustomSlots;
	if( ItemInfo != NULL && _StateController->PlayerInfo.IsValid() )
	{
		CInventory& Inven = _StateController->PlayerInfo.Inven;
		EquippedCustomSlots.trigger = Inven.GetCustomInvenToSlot( ItemInfo->sn, _CSI_TRIGGER ) != NULL ? 1 : 0;
		EquippedCustomSlots.barrel = Inven.GetCustomInvenToSlot( ItemInfo->sn, _CSI_BARREL ) != NULL ? 1 : 0;
		EquippedCustomSlots.mount = Inven.GetCustomInvenToSlot( ItemInfo->sn, _CSI_MOUNT ) != NULL ? 1 : 0;
		EquippedCustomSlots.stock = Inven.GetCustomInvenToSlot( ItemInfo->sn, _CSI_STOCK ) != NULL ? 1 : 0;
		EquippedCustomSlots.grip = Inven.GetCustomInvenToSlot( ItemInfo->sn, _CSI_GRIP ) != NULL ? 1 : 0;
		EquippedCustomSlots.front = Inven.GetCustomInvenToSlot( ItemInfo->sn, _CSI_FRONT ) != NULL ? 1 : 0;
	}

	return GetSlotImageString( AvailCustomSlots, &EquippedCustomSlots );
}

inline static FString GetIconCodeString( FString IconCode, FString SectionName = "")
{
	FString FontPrefix;
	if( SectionName.Trim().Len() > 0 )
		FontPrefix = Localize(TEXT("UIInventoryScene"),*SectionName,TEXT("avaNet"));

	return FontPrefix.Trim().Len() > 0 ? FontPrefix + IconCode + TEXT("<Fonts:/>") : IconCode;
}

inline static FString GetIconCodeString( const ITEM_DESC* ItemDesc, const CUSTOM_ITEM_DESC* cItemDesc = NULL )
{
	UBOOL bEquip = ItemDesc ? (((ITEM_ID*)(&ItemDesc->id))->category >= _IC_HELMET && ((ITEM_ID*)(&ItemDesc->id))->category <= _IC_LEG) : FALSE;
	FString FontPrefix = Localize(TEXT("UIInventoryScene"), bEquip ? TEXT("Icon_Common_EquipIconFont") : TEXT("Icon_Common_WeaponIconFont") ,TEXT("avaNet"));
	TCHAR szIconCode[] = { TEXT('\0'), TEXT('\0') };
	szIconCode[0] = ItemDesc ? ((ITEM_DESC*)ItemDesc)->GetIcon() :
		cItemDesc ? ((ITEM_DESC*)cItemDesc)->GetIcon() : TEXT('a');

	return FontPrefix.Trim().Len() > 0 ? FontPrefix + szIconCode + TEXT("<Fonts:/>") : szIconCode;
}

inline static FString GetIconCodeString( const EFFECT_ITEM_DESC *pItemDesc )
{
	FString FontPrefix = Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_EffectIconFont") ,TEXT("avaNet"));
	TCHAR szIconCode[] = TEXT("a\0");

	if ( pItemDesc )
		szIconCode[0] = ((EFFECT_ITEM_DESC*)pItemDesc)->GetIcon();

	return FontPrefix.Trim().Len() > 0 ? FontPrefix + szIconCode + TEXT("<Fonts:/>") : szIconCode;
}

//! avaNetHandler.GetItemDesc함수에서 Kismet으로 전달하기 위해서 사용하는 함수. 
FString GetIconCodeStringExt(const ITEM_DESC* ItemDesc, const CUSTOM_ITEM_DESC* cItemDesc = NULL)
{
	return GetIconCodeString( ItemDesc, cItemDesc);
}

/* 친구 상관관계 아이콘 **/
inline static FString GetBuddySideIconString( WORD CorrelationType  )
{
	static const FString InvRelStr = TEXT("Inv. Rel");
	FString BuddyTypeStr =  ( CorrelationType == BT_BUDDY_ONESIDE) ? TEXT("UniDir") :
		( CorrelationType == BT_BUDDY_BOTH) ? TEXT("BiDir") :
		( CorrelationType == BT_BUDDY_OTHER) ? TEXT("Other") :
		( CorrelationType == BT_BUDDY_BIA) ? TEXT("BIA") :			// 전우도 양방향 친구임
		( CorrelationType == BT_BLOCK) ? TEXT("Block") : InvRelStr;
	return BuddyTypeStr != InvRelStr ? FString::Printf( TEXT("<Strings:avaNet.UILobbyScene.Icon_Correlation_%s>"), *BuddyTypeStr) : InvRelStr;
}

inline static FString GetOnlineStatusIconString( UBOOL bIsOnline, WORD CorrelationType = BT_BUDDY_OTHER)
{
	return FString::Printf(TEXT("<Strings:avaNet.UILobbyScene.Icon_%sOnlineStatus_%s>"), CorrelationType == BT_BUDDY_BIA ? TEXT("BIA_") : TEXT(""),bIsOnline ? TEXT("Online") : TEXT("Offline"));
}

FString GetSupportItemIcon( BYTE ItemType, INT EffectValue )
{
	FString ItemTypeStr;
	switch (ItemType)
	{
	case _IET_NONE:			ItemTypeStr = TEXT("");	break;
	case _IET_GR:			ItemTypeStr = TEXT("PCBang");	break;	/* PC방 플래그로 대체*/
	case _IET_EXP_BOOST:	ItemTypeStr = TEXT("XP");	break;
	case _IET_SP_BOOST:		ItemTypeStr = TEXT("Supply");	break;
	case _IET_MONEY_BOOST:	ItemTypeStr = TEXT("Money");	break;
	case _IET_ACCESSORY:	ItemTypeStr = TEXT("Accessory");	break;
	default: debugf(TEXT("check GetSupportItemIcon.ItemType(%d)"), ItemType); check(FALSE);	break;
	}

	return Localize(TEXT("UIResultScene"), *FString::Printf(TEXT("Icon_SupportItem[%s]"), *ItemTypeStr), TEXT("avaNet") );
}

inline static FString GetLastResultMsgTypeString( BYTE MsgType )
{
	FString MsgTypeStr;

	switch( MsgType )
	{
	case LastResultMsgType_PCBangXP:		MsgTypeStr = TEXT("PCBangXP");	break;
	case LastResultMsgType_PCBangMoney:		MsgTypeStr = TEXT("PCBangMoney");	break;
	case LastResultMsgType_PCBangClanPoint:	MsgTypeStr = TEXT("PCBangClanPoint");	break;
	case LastResultMsgType_SupplyMoney:		MsgTypeStr = TEXT("SupplyMoney");	break;
	case LastResultMsgType_SupplyItem:		MsgTypeStr = TEXT("SupplyItem");	break;
	case LastResultMsgType_Level:			MsgTypeStr = TEXT("Level");	break;
	case LastResultMsgType_XP:				MsgTypeStr = TEXT("XP");	break;
	case LastResultMsgType_Supply:			MsgTypeStr = TEXT("Supply");	break;
	case LastResultMsgType_Money:			MsgTypeStr = TEXT("Money");	break;
	case LastResultMsgType_MoneyBoost:		MsgTypeStr = TEXT("MoneyBoost");	break;
	case LastResultMsgType_Brother:			MsgTypeStr = TEXT("Brother");	break;
	case LastResultMsgType_EventXP:			MsgTypeStr = TEXT("EventXP");	break;
	case LastResultMsgType_EventCoin:		MsgTypeStr = TEXT("EventCoin");	break;
	default:
		// (MsgType < LastResultMsgType_MAX)에서 default로 왔다면 추가된 값을 빼먹어서 이다.
		check( MsgType >= LastResultMsgType_MAX );
		MsgTypeStr = TEXT("Unknown");
		break;
	}

	return MsgTypeStr;
}

inline static FString GetLastResultMsgIcon( BYTE MsgType )
{
	FString MsgTypeStr = GetLastResultMsgTypeString( MsgType );
	FString Section = FString::Printf(TEXT("Icon_LastResultMsg[%s]"), *MsgTypeStr);
	return Localize(TEXT("UIResultScene"), *Section, TEXT("avaNet"));
}

inline static FString GetLastResultMsgName( BYTE MsgType )
{
	FString MsgTypeStr = GetLastResultMsgTypeString( MsgType );
	FString Section = FString::Printf(TEXT("Text_LastResultMsg[%s]"), *MsgTypeStr);
	return Localize(TEXT("UIResultScene"), *Section, TEXT("avaNet"));
}

inline static FString GetLastResultMsgDesc( BYTE MsgType, INT Limit = 0 )
{
	const static FString NewLineStr = TEXT("\\n");

	FString MsgTypeStr = GetLastResultMsgTypeString( MsgType );
	FString Section = FString::Printf(TEXT("Text_LastResultMsg_Desc[%s]"), *MsgTypeStr);
	FString ResultStr = Localize(TEXT("UIResultScene"), *Section, TEXT("avaNet"));

	FLOAT Ratio = (FLOAT)Limit / (FLOAT)ITEM_LIMIT_INITED;

	FString GaugeIconStr;
	FString GaugeStr;
	FString GaugePercentStr;
	switch( MsgType )
	{
	case LastResultMsgType_PCBangXP:	break;
	case LastResultMsgType_PCBangMoney:	break;
	case LastResultMsgType_PCBangClanPoint:	break;
	case LastResultMsgType_SupplyMoney:	break;
	case LastResultMsgType_SupplyItem:	break;
	case LastResultMsgType_Level:	break;
	case LastResultMsgType_Money:	break;
	case LastResultMsgType_XP:		
	case LastResultMsgType_Supply:	
	case LastResultMsgType_MoneyBoost:
		GaugeStr = GetItemGaugeIconString( _IGT_DURABILITY, Limit );
		GaugePercentStr = GetItemGaugePercentString(_IGT_DURABILITY, Limit );
		break;
	case LastResultMsgType_Brother:	break;
	case LastResultMsgType_EventXP:	break;
	case LastResultMsgType_EventCoin:	break;
	default: check(FALSE);	break;
	}

	if( GaugeStr.Len() > 0 && GaugePercentStr.Len() > 0 )
		GaugeIconStr = GaugeStr + (Limit == 0 ? Localize(TEXT("UIResultScene"), TEXT("Text_SupportItemRunOut"), TEXT("avaNet")) : GaugePercentStr);

	FString ColorModStr;
	if( Ratio > 0.1f)
		ColorModStr = TEXT("Text_Mod_LastResultItemUsages[]");
	else if ( Ratio > 0.f )
		ColorModStr = TEXT("Text_Mod_LastResultItemUsages[Bad]");
	else
		ColorModStr = TEXT("Text_Mod_LastResultItemUsages[Destroyed]");
	ColorModStr = Localize(TEXT("UIResultScene"), *ColorModStr, TEXT("avaNet"));

	GaugeIconStr = ColorModStr + GaugeIconStr + (ColorModStr.InStr(TEXT("<color:"),FALSE,TRUE) != INDEX_NONE ? TEXT("<color:/>") : TEXT("") );

	if( GaugeIconStr.Len() > 0 )
		ResultStr += FString(TEXT("\\n")) + GaugeIconStr;

//	debugf(TEXT("GetLastResultMsgDesc - Ratio(%f) Limit(%d) ResultStr(%s)"), Ratio, Limit, *ResultStr);

	return ResultStr;
}

//! make ClanMark markup string from ClanMark id.
inline static FString GetClanMarkString(INT ClanMarkID, UBOOL bSmall=true)
{
	extern UavaNetHandler *GavaNetHandler;

	return GavaNetHandler->GetClanMarkPkgNameFromID(ClanMarkID, bSmall, true);
}

//! gaugeType이 정비도라서 기간 제한없이 영구적인 아이템에 표시될 Icon의 markup string.
inline static FString GetNoLimitIconString(BYTE GaugeType)
{
	return ( GaugeType == _IGT_MAINTENANCE ) ? Localize( TEXT("UIGeneral"), TEXT("Icon_NoLimit"), TEXT("avaNet")) : TEXT(" ");
}

/** 승리조건 스트링을 얻어온다 (점수 : 30, 섬멸 :100, 5판 3선승 등등) */
inline static FString GetWinConditionString( const FString& WinCondTypeStr, INT ListIndex = INDEX_NONE )
{
	TArray<INT> Parameters;
	Parameters.AddItem(ListIndex);

	FString KeyStr = FString(TEXT("Text_WinCondType_")) + WinCondTypeStr;

	// 특이한 승리조건에 대해 먼저 처리해준다. 필요한 숫자(몇승, 몇점 등등)가 있다면 준비
	static TCHAR *EscortType[] = { TEXT("Normal"), TEXT("StopWatch") };
	FString WinCondTypeL = WinCondTypeStr.ToLower();
	if( WinCondTypeL.InStr(TEXT("round")) != INDEX_NONE )
		Parameters.AddItem(ListIndex * 2 - 1);
	else if( WinCondTypeL.InStr(TEXT("escort")) != INDEX_NONE && (0 <= ListIndex && ListIndex < ARRAY_COUNT(EscortType)) )
		KeyStr = KeyStr + TEXT("[") + EscortType[ListIndex] + TEXT("]");

	// 실제 현지화 문자열을 받아온다음, 예를 들어 '(섬멸 : %d)' 와 같은 형태
	FString ResultStr = Localize(TEXT("UIMiscScene"), *KeyStr, TEXT("avaNet"));

	// %d를 매꿔넣는다
	static const FString FormatIntStr = TEXT("%d");
	INT FindIndex = INDEX_NONE;
	while( (FindIndex = ResultStr.InStr(FormatIntStr)) != INDEX_NONE && Parameters.Num() > 0 )
	{
		ResultStr = ResultStr.Left(FindIndex) + appItoa(Parameters.Pop()) + ResultStr.Mid(FindIndex + FormatIntStr.Len());
	}

	return ResultStr;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetMyPlayer
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetMyPlayer);

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_AvaNetMyPlayer::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	FName Field = *FieldName;

	if (FieldName == TEXT("USN"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.idAccount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if (FieldName == TEXT("Nickname"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.nickname;
		}
		else
		{
			OutFieldValue.StringValue = TEXT("----");
		}
	}
	else if (FieldName == TEXT("GuildName"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			if( FString(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName).Trim().Len() != 0)
				OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName;
			else
				OutFieldValue.StringValue = Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if (FieldName == TEXT("GuildMark"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = GetClanMarkString( _StateController->PlayerInfo.GetClanMarkID() );
		}
		else
		{
			OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? GetClanMarkString(0) : TEXT("");
		}

		// "Inv." ???
		return TRUE;
	}
	else if (FieldName == TEXT("GuildLevel"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildLevel);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if (FieldName == TEXT("Level"))
	{
		if( _StateController->PlayerInfo.IsValid() )
			OutFieldValue.StringValue = GetLevelNameString(_StateController->PlayerInfo.PlayerInfo.level);
		else
			OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? *GetLevelNameString(0) : TEXT("----");
	}
	else if (FieldName == TEXT("LevelIcon"))
	{
		if( _StateController->PlayerInfo.IsValid() )
			OutFieldValue.StringValue = GetLevelIconString( _StateController->PlayerInfo.PlayerInfo.level);
		else
			OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? *GetLevelIconString(0) : TEXT("-- ");
	}

	else if (FieldName == TEXT("XP"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.xp);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if ( FieldName == TEXT("XpProgress"))
	{
		if ( _StateController->PlayerInfo.IsValid() )
			OutFieldValue.StringValue = GetXPGaugeString( _StateController->PlayerInfo.PlayerInfo.xpProgress);
		else
			OutFieldValue.StringValue = GetXPGaugeString( 0 );
	}
	else if (FieldName == TEXT("SupplyPoint"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = GetSupplyGaugeIconString( _StateController->PlayerInfo.PlayerInfo.supplyPoint );
		}
		else
		{
			OutFieldValue.StringValue = GetSupplyGaugeIconString( 0 );
		}
	}
	else if (FieldName == TEXT("Cash"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = GetFmtMoneyString( GetMyPlayerCash() );
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if (FieldName == TEXT("Money"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = GetFmtMoneyString( GetMyPlayerMoney() );
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if (FieldName == TEXT("PCBangImage"))
	{
		if ( GetAvaNetHandler()->IsInPcBang() )
		{
			// 프리미엄 PC방.
			OutFieldValue.StringValue = Localize(TEXT("UIGeneral"),TEXT("Icon_PCBangImage"),TEXT("avaNet"));
		}
		else
		{
			OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? Localize(TEXT("UIGeneral"),TEXT("Icon_PCBangImage"),TEXT("avaNet")) : TEXT(" ");
		}
	}
	else if (FieldName == TEXT("LastClass"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_POINTMAN ? TEXT("P") :
				_StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_RIFLEMAN ? TEXT("R") : TEXT("S");
		}
		else
		{
			OutFieldValue.StringValue = TEXT("P");
		}
	}
	else if (FieldName == TEXT("LastWeaponIcon"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			TSN_ITEM ItemSN = SN_INVALID_ITEM;
			INT SlotType = (_StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_POINTMAN ? _EP_P1 :
				_StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_RIFLEMAN ? _EP_R1 : _EP_S1);
			SLOT_DESC *pSlotDesc = _ItemDesc().GetWeaponSlotByType(SlotType);
			if (pSlotDesc)
				ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[pSlotDesc->index];

			if (ItemSN != SN_INVALID_ITEM)
			{
				TID_ITEM ItemID = ID_INVALID_ITEM;
				for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
				{
					if (ItemSN == _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].sn)
					{
						ItemID = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].id;
						break;
					}
				}

				ITEM_DESC *pItem = _ItemDesc().GetItem(ItemID);

				OutFieldValue.StringValue = (pItem ? FString::Printf(TEXT("%c"), pItem->GetIcon()) : TEXT("a"));
			}
			else
			{
				OutFieldValue.StringValue = TEXT("x");
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("x");
		}
	}
	else if (FieldName == TEXT("LastWeaponName"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			TSN_ITEM ItemSN = SN_INVALID_ITEM;
			INT SlotType = (_StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_POINTMAN ? _EP_P1 :
				_StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_RIFLEMAN ? _EP_R1 : _EP_S1);
			SLOT_DESC *pSlotDesc = _ItemDesc().GetWeaponSlotByType(SlotType);
			if (pSlotDesc)
				ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[pSlotDesc->index];

			if (ItemSN != SN_INVALID_ITEM)
			{
				ITEM_DESC *ItemDesc = NULL;
				for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
				{
					if (ItemSN == _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].sn)
					{
						ItemDesc = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].id);
						break;
					}
				}

				OutFieldValue.StringValue = (ItemDesc ? ItemDesc->GetName() : TEXT("????"));
			}
			else
			{
				OutFieldValue.StringValue = TEXT("????");
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("Weapon");
		}
	}
	else if (FieldName == TEXT("ScoreAttacker"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.score.attacker);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("ScoreDefender"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.score.defender);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("ScoreLeader"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.score.leader);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("ScoreTactic"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.score.tactic);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("GameWinCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.gameWin);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("-");
		}
	}
	else if (FieldName == TEXT("GameDefeatCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.gameDefeat);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("-");
		}
	}
	else if (FieldName == TEXT("GameWinRatio"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			FLOAT b = (_StateController->PlayerInfo.PlayerInfo.scoreInfo.gameDefeat > 0 ? _StateController->PlayerInfo.PlayerInfo.scoreInfo.gameDefeat : 1.0);
			OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->PlayerInfo.PlayerInfo.scoreInfo.gameWin / b);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0.0");
		}
	}
	else if (FieldName == TEXT("RoundWinCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.roundWin);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RoundDefeatCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.roundDefeat);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RoundWinRatio"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			FLOAT b = (_StateController->PlayerInfo.PlayerInfo.scoreInfo.roundDefeat > 0 ? _StateController->PlayerInfo.PlayerInfo.scoreInfo.roundDefeat : 1.0);
			OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->PlayerInfo.PlayerInfo.scoreInfo.roundWin / b);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0.0");
		}
	}
	else if (FieldName == TEXT("DisconnectCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.disconnectCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("KillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD KillCount = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].killCount +
				_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].killCount +
				_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].killCount;
			OutFieldValue.StringValue = appItoa(KillCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("DeathCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.deathCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	//else if (FieldName == TEXT("KillDeathRatio"))
	//{
	//	if (_StateController->PlayerInfo.IsValid())
	//	{
	//		DWORD KillCount = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].killCount +
	//						_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].killCount +
	//						_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].killCount;
	//		FLOAT b = (_StateController->PlayerInfo.PlayerInfo.scoreInfo.deathCount > 0 ? _StateController->PlayerInfo.PlayerInfo.scoreInfo.deathCount : 1.0);
	//		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)KillCount / b);
	//	}
	//	else
	//	{
	//		OutFieldValue.StringValue = TEXT("0.0");
	//	}
	//}
	else if (FieldName == TEXT("ScoreDeathRatio"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), _StateController->PlayerInfo.PlayerInfo.scoreInfo.GetSDRatio());
		}
		else
		{
			OutFieldValue.StringValue = TEXT("--");
		}
	}
	else if (FieldName == TEXT("GameStraightWinCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.straightWinCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("TeamKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.teamKillCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("BIAProgress"))
	{
		// @ TODO : 게임중일때 값 넣기
		INT Limit = (GIsEditor && !GIsGame) ? ITEM_LIMIT_INITED/2 : _StateController->PlayerInfo.PlayerInfo.biaXP * ( ITEM_LIMIT_INITED / (FLOAT)BIA_EXP_MAX );
		OutFieldValue.StringValue = GetItemGaugeIconString( _IGT_DURABILITY, Limit );
	}
	else if (FieldName == TEXT("BIARatio"))
	{
		INT Limit = (GIsEditor && !GIsGame) ? ITEM_LIMIT_INITED/2 : _StateController->PlayerInfo.PlayerInfo.biaXP * ( ITEM_LIMIT_INITED / (FLOAT)BIA_EXP_MAX );
		FLOAT Ratio = (Limit/(FLOAT)ITEM_LIMIT_INITED);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.1f%%"), Ratio * 100.f);
	}
	else if (FieldName == TEXT("PointManPlayRound"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].playRound);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManHeadShotCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].headshotCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManHeadShotKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].headshotKillCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManPlayTime"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].playTime);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManPlayTimeFmt"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD PlayTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].playTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), PlayTime / 3600, (PlayTime % 3600) / 60, (PlayTime % 3600) % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("00:00:00");
		}
	}
	else if (FieldName == TEXT("PointManSprintTime"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD SprintTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].sprintTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), SprintTime / 3600, (SprintTime % 3600 ) / 60, SprintTime % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManTakenDamage"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].takenDamage);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].killCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponKillCount1"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[3]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponDamage1"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[3]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponKillCount2"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[1]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponDamage2"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[1]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponKillCount3"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[0]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponDamage3"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[0]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponKillCount4"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[2]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManWeaponDamage4"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[2]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("PointManKillCountPerMinute"))
	{
		if( _StateController->PlayerInfo.IsValid() )
		{
			DWORD KillCount = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].killCount;
			DWORD PlayTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].playTime;
			FLOAT KillCountPerMinute = (PlayTime != 0) ? KillCount / (PlayTime / 60.f) : 0.f ;
			OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), KillCountPerMinute);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0.0");
		}
	}
	else if (FieldName == TEXT("RifleManPlayRound"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].playRound);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManHeadShotCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].headshotCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManHeadShotKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].headshotKillCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManPlayTime"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].playTime);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManPlayTimeFmt"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD PlayTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].playTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), PlayTime / 3600, (PlayTime % 3600) / 60, (PlayTime % 3600) % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("00:00:00");
		}
	}
	else if (FieldName == TEXT("RifleManSprintTime"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD SprintTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].sprintTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), SprintTime / 3600 , (SprintTime % 3600) /60, SprintTime % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManTakenDamage"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].takenDamage);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].killCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount1"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[3]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponDamage1"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[3]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount2"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[1]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponDamage2"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[1]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount3"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[0]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponDamage3"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[0]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount4"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[2]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManWeaponDamage4"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[2]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("RifleManKillCountPerMinute"))
	{
		if( _StateController->PlayerInfo.IsValid() )
		{
			DWORD KillCount = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].killCount;
			DWORD PlayTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].playTime;
			FLOAT KillCountPerMinute = (PlayTime != 0) ? KillCount / (PlayTime / 60.f) : 0.f ;
			OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), KillCountPerMinute);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0.0");
		}
	}
	else if (FieldName == TEXT("SniperPlayRound"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].playRound);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperHeadShotCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].headshotCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperHeadShotKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].headshotKillCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperPlayTime"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].playTime);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperPlayTimeFmt"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD PlayTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].playTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), PlayTime / 3600, (PlayTime % 3600) / 60, (PlayTime % 3600) % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("00:00:00");
		}
	}
	else if (FieldName == TEXT("SniperSprintTime"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			DWORD SprintTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].sprintTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"),SprintTime / 3600,(SprintTime % 3600) / 60, SprintTime % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperTakenDamage"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].takenDamage);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperKillCount"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].killCount);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponKillCount1"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[3]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponDamage1"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[3]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponKillCount2"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[1]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponDamage2"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[1]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponKillCount3"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[0]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponDamage3"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[0]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponKillCount4"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[2]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperWeaponDamage4"))
	{
		if (_StateController->PlayerInfo.IsValid())
		{
			OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[2]);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0");
		}
	}
	else if (FieldName == TEXT("SniperKillCountPerMinute"))
	{
		if( _StateController->PlayerInfo.IsValid() )
		{
			DWORD KillCount = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].killCount;
			DWORD PlayTime = _StateController->PlayerInfo.PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].playTime;
			FLOAT KillCountPerMinute = (PlayTime != 0) ? KillCount / (PlayTime / 60.f) : 0.f ;
			OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), KillCountPerMinute);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("0.0");
		}
	}
	else if (Field == TEXT("Handler_ClassPointman_Checkbox"))
	{
		OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_POINTMAN ? GTrue : GFalse;
	}
	else if (Field == TEXT("Handler_ClassRifleman_Checkbox"))
	{
		OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_RIFLEMAN	? GTrue : GFalse;
	}
	else if (Field == TEXT("Handler_ClassSniper_Checkbox"))
	{
		OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.currentClass == _CLASS_SNIPER ? GTrue : GFalse;
	}

	if( OutFieldValue.StringValue.Len() == 0 )
		OutFieldValue.StringValue = TEXT("Inv.");

	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetMyPlayer::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("USN")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Nickname")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildLevel")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMark")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Level")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LevelIcon")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("XP")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("XpProgress")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyPoint")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Cash")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Money")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("PCBangImage")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("LastClass")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LastWeaponIcon")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LastWeaponName")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreAttacker")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreDefender")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreLeader")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreTactic")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameWinCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameDefeatCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameWinRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoundWinCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoundDefeatCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoundWinRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DisconnectCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DeathCount")));
	//new(out_Fields) FUIDataProviderField(FName(TEXT("KillDeathRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreDeathRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameStraightWinCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("TeamKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BIAProgress")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BIARatio")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManPlayRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManHeadShotCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManHeadShotKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManPlayTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManPlayTimeFmt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManSprintTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManTakenDamage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage4")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManPlayRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManHeadShotCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManHeadShotKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManPlayTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManPlayTimeFmt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManSprintTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManTakenDamage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage4")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperPlayRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperHeadShotCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperHeadShotKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperPlayTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperPlayTimeFmt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperSprintTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperTakenDamage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage4")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("Handler_ClassPointman_Checkbox")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Handler_ClassRifleman_Checkbox")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Handler_ClassSniper_Checkbox")));
}

void UUIDataProvider_AvaNetMyPlayer::UpdateWidget( const FString& FieldName, UUIObject* TargetWidget )
{
	FName Field = *FieldName;
	if( _StateController->IsStateInRoom() )
	{
		if( Field == TEXT("Handler_ClassPointman_Checkbox") ||
			Field == TEXT("Handler_ClassRifleman_Checkbox") ||
			Field == TEXT("Handler_ClassSniper_Checkbox"))
		{
			UBOOL bEnabled = TargetWidget->IsEnabled();
			UavaNetHandler* NetHandler = GetAvaNetHandler();
			
			// 관전자는 병과변경 불가, 게임시작 카운트중에 변경 불가, 방장이 아닌대기자는 준비상태에서 병과변경 불가
			bEnabled = !(NetHandler->AmISpectator() || NetHandler->IsCountingDown() || ( !NetHandler->AmIHost() && NetHandler->AmIReady()));
			if( bEnabled != TargetWidget->IsEnabled() )
				TargetWidget->SetEnabled( bEnabled );
		}
	}
}

/* ==========================================================================================================
UUIDataProvider_AvaNetSelectedRoom
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetSelectedRoom);



/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetSelectedRoom::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
#define _SELECT_CURRENT_MAP()																						\
	if (_StateController->IsStateInRoom())																		\
	{																											\
	MapInfo = _StateController->GetCurrentMap();															\
}																											\
		else if (_StateController->GetNetState() == _AN_LOBBY)														\
	{																											\
	if (_StateController->ChannelInfo.IsValid())															\
	{																										\
	FRoomDispInfo *Room = _StateController->RoomList.GetSelected();										\
	if (Room)																							\
	MapInfo = _StateController->MapList.Find(Room->RoomInfo.setting.idMap);							\
}																										\
}


#define _SELECT_CURRENT_ROOM()											\
	if (_StateController->IsStateInRoom())							\
	{																\
	if (_StateController->RoomInfo.IsValid())					\
	Room = &(_StateController->RoomInfo);					\
}																\
		else if (_StateController->GetNetState() == _AN_LOBBY)			\
	{																\
	if (_StateController->ChannelInfo.IsValid())				\
	{															\
	Room = _StateController->RoomList.GetSelected();		\
}															\
}

	if (FieldName == TEXT("RoomID"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room)
				OutFieldValue.StringValue = appItoa(Room->RoomInfo.idRoom);
			else
				OutFieldValue.StringValue = TEXT("-");
	}
	else if (FieldName == TEXT("RoomName"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room)
				OutFieldValue.StringValue = Room->RoomInfo.roomName;
			else
				OutFieldValue.StringValue = TEXT("----");
	}
	if (FieldName == TEXT("HostName"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room)
				OutFieldValue.StringValue = Room->RoomInfo.hostName;
			else
				OutFieldValue.StringValue = TEXT(" ");
	}
	else if (FieldName == TEXT("MapName"))
	{
		FMapInfo *MapInfo = NULL;

		_SELECT_CURRENT_MAP()

			if (MapInfo)
				OutFieldValue.StringValue = MapInfo->MapName;
			else
				OutFieldValue.StringValue = TEXT(" ");
	}
	else if (appStristr( *FieldName, TEXT("MapImage")) == *FieldName )
	{
		FMapInfo *MapInfo = NULL;

		_SELECT_CURRENT_MAP()

			INT MapListIndex = INDEX_NONE;
		if( Parse( *FieldName, TEXT("MapListIndex="), MapListIndex ) && _StateController->MapList.MapList.IsValidIndex(MapListIndex) )
			MapInfo = &_StateController->MapList.MapList(MapListIndex);

		if (MapInfo)
		{
			OutFieldValue.StringValue = FString(TEXT("<Images:")) + MapInfo->ImagePathName + FString(TEXT(";XL=186 YL=68>"));
		}
		else
		{
			OutFieldValue.StringValue = TEXT(" ");
		}
	}
	else if (FieldName == TEXT("MapDescription"))
	{
		FMapInfo *MapInfo = NULL;

		_SELECT_CURRENT_MAP()

			if (MapInfo)
				OutFieldValue.StringValue = MapInfo->Description;
			else
				OutFieldValue.StringValue = TEXT("Unknown Mission");
	}
	else if (FieldName == TEXT("MapListIndex"))
	{
		TID_MAP MapID;
		INT MapListIndex = -1;
		if( _StateController->IsStateInRoom())
		{
			if (_StateController->RoomInfo.IsValid())
			{
				MapID = _StateController->RoomInfo.RoomInfo.setting.idMap;
				for( INT i = 0 ; i < _StateController->MapList.MapList.Num() ; i++)
				{
					if( _StateController->MapList.MapList(i).idMap == MapID )
					{
						MapListIndex = i;
						break;
					}
				}
				OutFieldValue.StringValue = MapListIndex >= 0 ? appItoa(MapListIndex) : FString(TEXT("Invalid idMap ?"));
			}
			else
				OutFieldValue.StringValue = TEXT("Invalid RoomInfo ?");
		}
		else if( _StateController->GetNetState() == _AN_LOBBY)
		{
			if (_StateController->ChannelInfo.IsValid())
			{
				FRoomDispInfo *Room = _StateController->RoomList.GetSelected();
				if (Room)
				{
					MapID = Room->RoomInfo.setting.idMap;
					for( INT i = 0 ; i < _StateController->MapList.MapList.Num() ; i++)
					{
						if( _StateController->MapList.MapList(i).idMap == MapID )
						{
							MapListIndex = i;
							break;
						}
					}
					OutFieldValue.StringValue = MapListIndex >= 0 ? appItoa(MapListIndex) : FString(TEXT("Invalid idMap ?"));
				}
				else
					OutFieldValue.StringValue = TEXT("Room is not selected");
			}
			else
				OutFieldValue.StringValue = TEXT("invalid channelinfo ?");
		}
		else
		{
			OutFieldValue.StringValue = TEXT("Invalid NetState ?");
		}
	}
	else if (FieldName == TEXT("SettingSpectator"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if ( Room != NULL && Room->RoomInfo.setting.allowSpectator)
				OutFieldValue.StringValue = GTrue;
			else
				OutFieldValue.StringValue = GFalse;
	}
	else if (FieldName == TEXT("SettingInterruptible"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room != NULL && Room->RoomInfo.setting.allowInterrupt)
				OutFieldValue.StringValue = GTrue;
			else
				OutFieldValue.StringValue = GFalse;
	}
	else if (FieldName == TEXT("SettingAutoBalance"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room != NULL && Room->RoomInfo.setting.autoBalance)
				OutFieldValue.StringValue = GTrue;
			else
				OutFieldValue.StringValue = GFalse;
	}
	else if (FieldName == TEXT("SettingTeamKill"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			BYTE tkLevel = Room != NULL ? Room->RoomInfo.setting.tkLevel : 0;
		OutFieldValue.StringValue = tkLevel == 0 ? TEXT("팀킬 없음") : tkLevel == 1 ? TEXT("수류탄 허용") : TEXT("모두 허용");
	}
	else if ( FieldName == TEXT("SettingBackView") )
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			BYTE BackView = Room ? Room->RoomInfo.setting.allowBackView : FALSE;
		OutFieldValue.StringValue = BackView ? GTrue : GFalse;
	}
	else if ( FieldName == TEXT("SettingGhostChat") )
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM();

		BYTE GhostChat = Room ? Room->RoomInfo.setting.allowGhostChat : FALSE;
		OutFieldValue.StringValue = GhostChat ? GTrue : GFalse;
	}
	else if (FieldName == TEXT("WinCondition"))
	{
		FRoomDispInfo *Room = NULL;
		FMapInfo* MapInfo = NULL;

		_SELECT_CURRENT_ROOM()
			_SELECT_CURRENT_MAP()

			if (Room && MapInfo)
			{
				BYTE RoundToWin = Room->RoomInfo.setting.roundToWin;
				OutFieldValue.StringValue = GetWinConditionString( MapInfo->WinCondType, RoundToWin );
			}
			else
				OutFieldValue.StringValue = TEXT("Invalid Roominfo ?");
	}
	else if (FieldName == TEXT("MaxPlayers"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room)
				OutFieldValue.StringValue = appItoa(Room->RoomInfo.setting.numMax);
			else
				OutFieldValue.StringValue = TEXT("0");
	}
	else if (FieldName == TEXT("CurrentPlayers"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room)
				OutFieldValue.StringValue = appItoa(Room->RoomInfo.state.numCurr);
			else
				OutFieldValue.StringValue = TEXT("0");
	}
	else if (FieldName == TEXT("CurrentRound"))
	{
		FRoomDispInfo *Room = NULL;

		_SELECT_CURRENT_ROOM()

			if (Room && Room->RoomInfo.state.playing == Def::RIP_PLAYING)
				OutFieldValue.StringValue = appItoa(Room->RoomInfo.state.currRound);
			else
				OutFieldValue.StringValue = TEXT("0");
	}
	else if (FieldName == TEXT("SpectatorSlot1") || FieldName == TEXT("SpectatorSlot2") || FieldName == TEXT("SpectatorSlot3") || FieldName == TEXT("SpectatorSlot4"))
	{
		FRoomInfo *Room = NULL;
		if (_StateController->IsStateInRoom())
		{
			if (_StateController->RoomInfo.IsValid())
				Room = &(_StateController->RoomInfo);
		}

		//_SELECT_CURRENT_ROOM()

		if (Room)
		{
			INT Idx = (FieldName == TEXT("SpectatorSlot1") ? Def::MAX_PLAYER_PER_ROOM :
				FieldName == TEXT("SpectatorSlot2") ? Def::MAX_PLAYER_PER_ROOM + 1 :
				FieldName == TEXT("SpectatorSlot3") ? Def::MAX_PLAYER_PER_ROOM + 2 : Def::MAX_PLAYER_PER_ROOM + 3);

		if ( !Room->PlayerList.IsEmpty(Idx) )
		{
			OutFieldValue.StringValue = Room->PlayerList.PlayerList[Idx].PlayerInfo.nickname;
		}
		}
	}
	else if ( FieldName == TEXT("GuildNameLeft") )
	{
		FRoomInfo *Room = NULL;
		if (_StateController->IsStateInRoom())
		{
			if (_StateController->RoomInfo.IsValid())
				Room = &(_StateController->RoomInfo);
		}

		//_SELECT_CURRENT_ROOM();
		if ( Room && _StateController->ChannelInfo.IsFriendlyGuildChannel() )
		{
			INT Index = Room->PlayerList.FindFirstValidSlot(RT_EU);
			if ( Index != -1 )
				OutFieldValue.StringValue = Room->PlayerList.PlayerList[Index].RoomPlayerInfo.guildName;
//			debugf(TEXT("GuildNameLeft [%d] %s"), Index, *OutFieldValue.StringValue);
		}

		if ( OutFieldValue.StringValue.Len() == 0 )
			OutFieldValue.StringValue = TEXT(" ");
	}
	else if ( FieldName == TEXT("GuildNameRight") )
	{
		FRoomInfo *Room = NULL;
		if (_StateController->IsStateInRoom())
		{
			if (_StateController->RoomInfo.IsValid())
				Room = &(_StateController->RoomInfo);
		}

		//_SELECT_CURRENT_ROOM();
		if ( Room && _StateController->ChannelInfo.IsFriendlyGuildChannel() )
		{
			INT Index = Room->PlayerList.FindFirstValidSlot(RT_NRF);
			if ( Index != -1 )
				OutFieldValue.StringValue = Room->PlayerList.PlayerList[Index].RoomPlayerInfo.guildName;
//			debugf(TEXT("GuildNameRight [%d] %s"), Index, *OutFieldValue.StringValue);
		}

		if ( OutFieldValue.StringValue.Len() == 0 )
			OutFieldValue.StringValue = TEXT(" ");
	}
	else if ( FieldName == TEXT("GuildMarkLeft") )
	{
		FRoomInfo *Room = NULL;
		if (_StateController->IsStateInRoom() && _StateController->ChannelInfo.IsFriendlyGuildChannel())
		{
			if (_StateController->RoomInfo.IsValid())
			{
				Room = &(_StateController->RoomInfo);

				//! EU-ClanMark.
				INT Index = Room->PlayerList.FindFirstValidSlot(RT_EU);
				if ( Index != -1 )
					OutFieldValue.StringValue = GetClanMarkString(Room->PlayerList.PlayerList[Index].GetClanMarkID(), false);

				// 슬롯에 Player가 없다면 클랜마크를 출력하지 않는다.(Index == -1)
			}
		}

		if( GIsEditor && !GIsGame )
			OutFieldValue.StringValue = GetClanMarkString(0, false);
	}
	else if ( FieldName == TEXT("GuildMarkRight") )
	{
		FRoomInfo *Room = NULL;
		if (_StateController->IsStateInRoom() && _StateController->ChannelInfo.IsFriendlyGuildChannel())
		{
			if (_StateController->RoomInfo.IsValid())
			{
				Room = &(_StateController->RoomInfo);

				//! NRF-ClanMark.
				INT Index = Room->PlayerList.FindFirstValidSlot(RT_NRF);
				if ( Index != -1 )
					OutFieldValue.StringValue = GetClanMarkString(Room->PlayerList.PlayerList[Index].GetClanMarkID(), false);

				// 슬롯에 Player가 없다면 클랜마크를 출력하지 않는다.(Index == -1)
			}
		}

		if( GIsEditor && !GIsGame )
			OutFieldValue.StringValue = GetClanMarkString(0, false);
	}

	if( OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = TEXT(" ");
	}

	return TRUE;


#undef _SELECT_CURRENT_MAP
#undef _SELECT_CURRENT_ROOM
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetSelectedRoom::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoomID")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoomName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("HostName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapImage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapDescription")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapListIndex")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SettingSpectator")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SettingInterruptible")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SettingAutoBalance")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SettingTeamKill")));
	//new(out_Fields) FUIDataProviderField(FName(TEXT("SettingAllowTacticalSkill")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SettingBackView")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SettingGhostChat")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WinCondition")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MaxPlayers")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentPlayers")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SpectatorSlot1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SpectatorSlot2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SpectatorSlot3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SpectatorSlot4")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildNameLeft")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildNameRight")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMarkLeft")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMarkRight")));
}

/* ==========================================================================================================
UUIDataProvider_AvaNetChannels
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetChannels);


void UUIDataProvider_AvaNetChannels::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	//OutCellTags.Set(FName(TEXT("Index")), TEXT("Index"));
	//OutCellTags.Set(FName(TEXT("ChannelID"), TEXT("Index"));
	OutCellTags.Set(FName(TEXT("ChannelName")), *Localize(TEXT("Channel"), TEXT("Text_List_ChannelName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("PlayerCount")), *Localize(TEXT("Channel"), TEXT("Text_List_PlayerCount"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("GroupNum")), TEXT("GroupNum")); // 그룹번호 예를들어 '0'은 일반채널 등등. 키즈멧 내부에서 사용
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetChannels::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if( GIsEditor && !GIsGame )
	{
		OutFieldValue.StringValue = FString(CellTag.GetName()).Left(5) + appItoa(ListIndex);
	}
	else
	{
		// "내 길드채널에 대한 처리"
		if( IsClanHome( ListIndex ) )
		{
			if ( CellTag == FName(TEXT("ChannelName")) )
			{
				OutFieldValue.StringValue = Localize(TEXT("Channel"), TEXT("Text_MyClanChannel"), TEXT("avaNet"));
			}
			else if (CellTag == FName(TEXT("PlayerCount")))
			{
				// "내 길드채널"은 몇명이 플레이중인지 알 필요없음
				OutFieldValue.StringValue = TEXT(" ");
			}
			else if ( CellTag == FName(TEXT("GroupNum")) )
			{
				OutFieldValue.StringValue = TEXT(" ");
			}
		}
		else if ( _StateController->ChannelList.ChannelList.IsValidIndex(ListIndex))
		{
			FChannelInfo &Info = _StateController->ChannelList.ChannelList(ListIndex);
			//if (CellTag == FName(TEXT("Index")))
			//{
			//	OutFieldValue.StringValue = FString::Printf(TEXT("%02d"), ListIndex + 1);
			//}
			//else if (CellTag == FName(TEXT("ChannelID")))
			//{
			//	OutFieldValue.StringValue = appItoa(_StateController->ChannelList.ChannelList(ListIndex).Idx);
			//}

			static const TCHAR* StatusStr[] = { TEXT("Normal"), TEXT("Congestion"), TEXT("Full") };
			const FLOAT Ratio = Info.Count/(FLOAT)(Info.MaxPlayers != 0 ? Info.MaxPlayers : 1);

			if (CellTag == FName(TEXT("ChannelName")))
			{
//				debugf(TEXT("### ListIndex(%d), ChannelName(%s)"), ListIndex, *Info.ChannelName);

				FString SectionName = FString::Printf(TEXT("Text_Mod_ChannelName[%s]"), StatusStr[ Ratio < 0.9f ? Ratio < 0.7f ? 0 : 1 : 2 ]);
				OutFieldValue.StringValue = Localize(TEXT("Channel"), *SectionName, TEXT("avaNet")) + Info.ChannelName;
			}
			else if (CellTag == FName(TEXT("PlayerCount")))
			{
				if (Info.IsValid())
				{
					const INT MaxPlayersLen = Max<INT>(appItoa(Info.MaxPlayers).Len(),1);
					FString FmtStr = FString(TEXT("[%0")) + appItoa(Info.Count >= Info.MaxPlayers ? MaxPlayersLen : MaxPlayersLen - 1) + TEXT("d/%d]");
					FString SectionName = FString::Printf(TEXT("Text_Mod_PlayerCount[%s]"), StatusStr[ Ratio < 0.9f ? Ratio < 0.7f ? 0 : 1 : 2 ]);
					OutFieldValue.StringValue = Localize(TEXT("Channel"), *SectionName, TEXT("avaNet")) + FString::Printf(*FmtStr, Info.Count, Info.MaxPlayers);
				}
				else
					OutFieldValue.StringValue = TEXT("[N/A]");
			}
			else if ( CellTag == FName(TEXT("GroupNum")) )
			{
				OutFieldValue.StringValue = appItoa(Info.Flag);
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("[N/A]");
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetChannels::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

UBOOL UUIDataProvider_AvaNetChannels::SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	UBOOL bResult = FALSE;

	// "내클랜홈은 항상 맨위에 위치"
	UBOOL bExistClanHome = FALSE;
	INT ClanHomeCount = 0;
	for( INT ListItemIndex = 0 ; ListItemIndex < ListItems.Num() ; ListItemIndex++ )
	{
		if( IsClanHome( ListItems(ListItemIndex).DataSource.DataSourceIndex ) )
		{
			appMemswap( ListItems.GetTypedData() + ClanHomeCount, ListItems.GetTypedData() + ListItemIndex, sizeof(ListItems(0)) );
			bExistClanHome = TRUE;
			ClanHomeCount++;
			break;
		}
	}

	// 그룹별로 정렬을 하도록 한다.
	// 그룹이 변하는 지점의 ListItemIndex를 기록해둔다
	INT ChannelFlag = INDEX_NONE;
	TArray<INT> GroupFirstIndices;
	for( INT ListItemIndex = ClanHomeCount ; ListItemIndex < ListItems.Num() ; ListItemIndex++ )
	{
		FChannelInfo* ChannelInfo = GetChannelInfoByIndex( ListItems(ListItemIndex).DataSource.DataSourceIndex );
		if( ChannelInfo == NULL )
			continue;

		if( ChannelFlag != ChannelInfo->Flag )
		{
			GroupFirstIndices.AddItem( ListItemIndex );
			ChannelFlag = ChannelInfo->Flag;
		}
	}

	// 그룹이 변하는 지점을 기준으로 정렬한다
	if( PrimaryCellTag == TEXT("ChannelName") )
	{
		for( INT GroupIndex = 0 ; GroupIndex < GroupFirstIndices.Num() ; GroupIndex++ )
		{
			INT GroupFirstIndex = GroupFirstIndices(GroupIndex);
			INT GroupLastIndex = GroupFirstIndices.IsValidIndex(GroupIndex + 1) ? GroupFirstIndices(GroupIndex + 1) : ListItems.Num();
			ava::sort( ListItems.GetTypedData() + GroupFirstIndex, ListItems.GetTypedData() + GroupLastIndex,
				UIListItemSort(SortParameters, PartialStringSort(FALSE, TRUE, TRUE),PartialIntegerSort()) );
		}
		bResult = TRUE;
	}
	else if( PrimaryCellTag == TEXT("PlayerCount") )
	{
		for( INT GroupIndex = 0 ; GroupIndex < GroupFirstIndices.Num() ; GroupIndex++ )
		{
			INT GroupFirstIndex = GroupFirstIndices(GroupIndex);
			INT GroupLastIndex = GroupFirstIndices.IsValidIndex(GroupIndex + 1) ? GroupFirstIndices(GroupIndex + 1) : ListItems.Num();
			ava::sort( ListItems.GetTypedData() + GroupFirstIndex, ListItems.GetTypedData() + GroupLastIndex,
				UIListItemSort(SortParameters, PartialIntegerSort(), bind2Sort( PartialStringSort(FALSE, TRUE, TRUE), PartialIntegerSort() )));
		}
		bResult = TRUE;
	}

	return bResult;
}


/* ==========================================================================================================
UUIDataProvider_AvaNetLobbyRooms
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLobbyRooms);


void UUIDataProvider_AvaNetLobbyRooms::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Available")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_Available"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Level")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelLimit"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("RoomID")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_RoomID"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Locked")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_Locked"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("RoomName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_RoomName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("MissionType")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_MissionType"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("HostName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_HostName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("MapName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_MapName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("PlayerCount")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_PlayerCount"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("RoomState")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_RoomState"), TEXT("avaNet")));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetLobbyRooms::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	FString StringPrefix;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("Available")))
			{
				OutFieldValue.StringValue = (ListIndex % 2 == 0 ? TEXT("OK") : TEXT(" "));
			}
			else if (CellTag == FName(TEXT("Level")))
			{
				//OutFieldValue.StringValue = appItoa(_StateController->ChannelList.ChannelList(ListIndex).Idx);
				OutFieldValue.StringValue = appItoa(0);
			}
			else if (CellTag == FName(TEXT("RoomID")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("%3d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("Locked")))
			{
				OutFieldValue.StringValue = (ListIndex % 3 == 0 ? TEXT("(*)") : TEXT(" "));
			}
			else if (CellTag == FName(TEXT("RoomName")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("테스트 %d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("MissionType")))
			{
				OutFieldValue.StringValue = Localize(TEXT("UILobbyScene"), TEXT("Text_List_MissionType"), TEXT("avaNet"));
			}
			else if (CellTag == FName(TEXT("HostName")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("방장 %d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("MapName")))
			{
				OutFieldValue.StringValue = TEXT("HammerBlow");
			}
			else if (CellTag == FName(TEXT("PlayerCount")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("%2d/%d"), ListIndex % Def::MAX_PLAYER_PER_ROOM, Def::MAX_PLAYER_PER_ROOM);
			}
			else if (CellTag == FName(TEXT("RoomState")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("%s"), ListIndex % 4 == 0 ? TEXT("진행") : TEXT("대기"));
			}
		}
	}
	else
	{
		FRoomDispInfo* RoomInfo = GetRoomInfoByIndex(ListIndex);
		if ( RoomInfo != NULL )
		{
			Def::ROOM_INFO &Info = RoomInfo->RoomInfo;
			UBOOL bAcceptable = IsAcceptableRoom( ListIndex );
			StringPrefix = bAcceptable ? FString(TEXT("")) : Localize(TEXT("UILobbyScene"), TEXT("TextMod_RoomIsFull"), TEXT("avaNet"));
			OutFieldValue.StringValue = StringPrefix;
			if (CellTag == FName(TEXT("Available")))
			{
				OutFieldValue.StringValue += (Info.state.numCurr < Info.setting.numMax ? TEXT("OK") : TEXT(" "));
			}
			else if (CellTag == FName(TEXT("Level")))
			{
				//OutFieldValue.StringValue = appItoa(_StateController->ChannelList.ChannelList(ListIndex).Idx);
				//OutFieldValue.StringValue = appItoa(0);
				OutFieldValue.StringValue += TEXT(" ");
			}
			else if (CellTag == FName(TEXT("RoomID")))
			{
				OutFieldValue.StringValue += FString::Printf(TEXT("%3d"), _StateController->RoomList.RoomList(ListIndex).RoomInfo.idRoom);
			}
			else if (CellTag == FName(TEXT("Locked")))
			{
				OutFieldValue.StringValue += (_StateController->RoomList.RoomList(ListIndex).RoomInfo.bPassword > 0 ? Localize(TEXT("UILobbyScene"), TEXT("Icon_ListElem_Locked"), TEXT("avaNet")) : FString(TEXT(" ")));
			}
			else if (CellTag == FName(TEXT("RoomName")))
			{
				OutFieldValue.StringValue += _StateController->RoomList.RoomList(ListIndex).RoomInfo.roomName;
			}
			else if (CellTag == FName(TEXT("MissionType")))
			{
				FMapInfo *Info = _StateController->MapList.Find(_StateController->RoomList.RoomList(ListIndex).RoomInfo.setting.idMap);
				OutFieldValue.StringValue += Info ? Localize(TEXT("UIRoomSettingScene"), *FString::Printf(TEXT("Text_MapTypeName[%s]"), *Info->Description), TEXT("avaNet")) : FString(TEXT(" "));
			}
			else if (CellTag == FName(TEXT("HostName")))
			{
				OutFieldValue.StringValue += _StateController->RoomList.RoomList(ListIndex).RoomInfo.hostName;
			}
			else if (CellTag == FName(TEXT("MapName")))
			{
				FMapInfo *Info = _StateController->MapList.Find(_StateController->RoomList.RoomList(ListIndex).RoomInfo.setting.idMap);
				if (!Info)
					Info = _StateController->MapList.Find(1);	// fake info

				if (Info)
					OutFieldValue.StringValue += Info->MapName;
				else
					OutFieldValue.StringValue += TEXT("room map info ?");
			}
			else if (CellTag == FName(TEXT("PlayerCount")))
			{
				Def::ROOM_INFO &Info = _StateController->RoomList.RoomList(ListIndex).RoomInfo;
				OutFieldValue.StringValue += FString::Printf(TEXT("%2d/%d"), Info.state.numCurr, Info.setting.numMax);
			}
			else if (CellTag == FName(TEXT("RoomState")))
			{
				OutFieldValue.StringValue += FString::Printf(TEXT("%s"), _StateController->RoomList.RoomList(ListIndex).RoomInfo.state.playing == Def::RIP_PLAYING ? TEXT("진행") : TEXT("대기"));
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (/*OutFieldValue.StringValue.Len() == 0*/ OutFieldValue.StringValue.Len() == StringPrefix.Len())
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetLobbyRooms::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

UBOOL UUIDataProvider_AvaNetLobbyRooms::SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	if (PrimaryCellTag == FName(TEXT("Available")) || PrimaryCellTag == FName(TEXT("HostName")) || 
		PrimaryCellTag == FName(TEXT("RoomName")) || PrimaryCellTag == FName(TEXT("MapName")) || 
		PrimaryCellTag == FName(TEXT("RoomState")) || PrimaryCellTag == FName(TEXT("MissionType")) || 
		PrimaryCellTag == FName(TEXT("Locked")))
	{  
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(), 
			UIListItemSort(SortParameters, NormalStringSort(), NormalIntegerSort()) );
		return TRUE;
	}
	else if (PrimaryCellTag == FName(TEXT("Level")) ||
		PrimaryCellTag == FName(TEXT("RoomID")) ||
		PrimaryCellTag == FName(TEXT("PlayerCount")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(), 
			UIListItemSortBase<NormalIntegerSort>(SortParameters, NormalIntegerSort()) );
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================================================
UUIDataProvider_AvaNetLobbyPlayers
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLobbyPlayers);


void UUIDataProvider_AvaNetLobbyPlayers::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Level")),*Localize(TEXT("UILobbyScene"), TEXT("Text_List_Level"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Nickname")),*Localize(TEXT("UILobbyScene"), TEXT("Text_List_NickName"), TEXT("avaNet")));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetLobbyPlayers::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("Level")))
			{
				OutFieldValue.StringValue = appItoa(ListIndex % 10 + 1);
			}
			else if (CellTag == FName(TEXT("Nickname")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("플레이어%2d"), ListIndex);
			}
		}
	}
	else
	{
		if ( _StateController->LobbyPlayerList.PlayerList.IsValidIndex(ListIndex) )
		{
			if (CellTag == FName(TEXT("Level")))
			{
				BYTE Lev = _StateController->LobbyPlayerList.PlayerList(ListIndex).LobbyPlayerInfo.level;
				OutFieldValue.StringValue = GetLevelIconString(Lev);
			}
			else if (CellTag == FName(TEXT("Nickname")))
			{
				OutFieldValue.StringValue = _StateController->LobbyPlayerList.PlayerList(ListIndex).LobbyPlayerInfo.nickname;
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetLobbyPlayers::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

UBOOL UUIDataProvider_AvaNetLobbyPlayers::SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	if( PrimaryCellTag == FName(TEXT("Level")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<ParseArgumentSort<INT>, NormalStringSort>
			( SortParameters, ParseArgumentSort<INT>(TEXT("Icon_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE ) );
		return TRUE;
	}
	else if ( PrimaryCellTag == FName(TEXT("NickName")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<NormalStringSort>(SortParameters,NormalStringSort()) );
		return TRUE;
	}

	return FALSE;
}


/* ==========================================================================================================
UUIDataProvider_AvaNetLobbyFriendPlayers
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLobbyFriendPlayers);


void UUIDataProvider_AvaNetLobbyFriendPlayers::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Correlation")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_Correlation"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelCombo")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelCombo"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelIcon")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelIcon"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NickName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_NickName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NickNameCombo")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_NickName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("GuildName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_GuildName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NetLocation")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_NetLocation"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("OnlineStatus")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_OnlineStatus"), TEXT("avaNet")) );
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetLobbyFriendPlayers::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if ( CellTag ==  FName(TEXT("Correlation")) )
		{
			OutFieldValue.StringValue = GetBuddySideIconString( BT_BUDDY_BOTH );
		}
		else if ( CellTag ==  FName(TEXT("LevelCombo")) )
		{
			OutFieldValue.StringValue = GetLevelIconString( ListIndex % 50 ) + GetLevelNameString( ListIndex % 50 );
		}
		else if ( CellTag ==  FName(TEXT("LevelIcon")) )
		{
			OutFieldValue.StringValue = GetLevelIconString( ListIndex % 50 );
		}
		else if ( CellTag ==  FName(TEXT("LevelName")) )
		{
			OutFieldValue.StringValue = GetLevelNameString( ListIndex % 50 );
		}
		else if ( CellTag ==  FName(TEXT("NickName")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("Test"), TEXT("Text_NickName"), TEXT("avaNet"));
		}
		else if ( CellTag ==  FName(TEXT("NickNameCombo")) )
		{
			FString BIAModStr = (ListIndex % 2) == 0 ? *Localize(TEXT("LobbyScene"), TEXT("Text_Mod_BIAPlayer"), TEXT("avaNet")) : TEXT("");
			OutFieldValue.StringValue = GetBuddySideIconString(BT_BUDDY_BOTH) + BIAModStr + Localize(TEXT("Test"), TEXT("Text_NickName"), TEXT("avaNet"));
		}
		else if ( CellTag ==  FName(TEXT("GuildName")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("Test"), TEXT("Text_ClanName"), TEXT("avaNet"));
		}
		else if ( CellTag ==  FName(TEXT("NetLocation")) )
		{
			OutFieldValue.StringValue = CellTag.GetName();
		}
		else if ( CellTag ==  FName(TEXT("OnlineStatus")) )
		{
			OutFieldValue.StringValue = GetOnlineStatusIconString( (ListIndex % 2) == 0, ((ListIndex >> 1) % 2) == 0 ? BT_BUDDY_BIA : BT_BUDDY_OTHER );
		}
	}
	else
	{
		if ( _Communicator().BuddyList.BuddyList.IsValidIndex( ListIndex ) )
		{
			FBuddyInfo& BuddyInfo = _Communicator().BuddyList(ListIndex);
			if ( CellTag ==  FName(TEXT("Correlation")) )
			{
				OutFieldValue.StringValue = GetBuddySideIconString( BuddyInfo.BuddyType );
			}
			else if ( CellTag ==  FName(TEXT("LevelCombo")) )
			{
				OutFieldValue.StringValue = GetLevelIconString( BuddyInfo.Level ) + GetLevelNameString( BuddyInfo.Level);
			}
			else if ( CellTag ==  FName(TEXT("LevelIcon")) )
			{
				OutFieldValue.StringValue = GetLevelIconString( BuddyInfo.Level );
			}
			else if ( CellTag ==  FName(TEXT("LevelName")) )
			{
				OutFieldValue.StringValue = GetLevelNameString( BuddyInfo.Level );
			}
			else if ( CellTag ==  FName(TEXT("NickName")) )
			{
				OutFieldValue.StringValue = BuddyInfo.Nickname;
			}
			else if ( CellTag ==  FName(TEXT("NickNameCombo")) )
			{
				FString BIAModStr = BuddyInfo.IsBuddyBIA() ? *Localize(TEXT("LobbyScene"), TEXT("Text_Mod_BIAPlayer"), TEXT("avaNet")) : TEXT("");
				OutFieldValue.StringValue = GetBuddySideIconString(BuddyInfo.BuddyType) + BIAModStr + BuddyInfo.Nickname;
			}
			else if ( CellTag ==  FName(TEXT("GuildName")) )
			{
				OutFieldValue.StringValue = BuddyInfo.GuildName.Trim().Len() != 0 ? BuddyInfo.GuildName : Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
			}
			else if ( CellTag ==  FName(TEXT("NetLocation")) )
			{
				OutFieldValue.StringValue = BuddyInfo.GetLocation();
			}
			else if ( CellTag ==  FName(TEXT("OnlineStatus")) )
			{
				OutFieldValue.StringValue = GetOnlineStatusIconString( BuddyInfo.IsOnline(), BuddyInfo.BuddyType );
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("Inv.");
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("N/A");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetLobbyFriendPlayers::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

UBOOL UUIDataProvider_AvaNetLobbyFriendPlayers::SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	UBOOL bResult = FALSE;
	// (전우/ 전우가 아닌 플레이어)별로 따로 그려줘야한다.
	FBuddyInfo* BuddyBIA = _Communicator().GetBIA();

	TArray<INT> GroupFirstIndices;
	if( _Communicator().BuddyList.BuddyList.Num() > 0 )
	{
		UBOOL CurrBIA = (&_Communicator().BlockList(0)) == BuddyBIA;
		GroupFirstIndices.AddItem(0);
		for( INT ListItemIndex = 1 ; ListItemIndex < ListItems.Num() ; ListItemIndex++ )
		{
			INT ListIndex = ListItems(ListItemIndex).DataSource.DataSourceIndex;
			if( ! _Communicator().BuddyList.BuddyList.IsValidIndex(ListIndex) )
				continue;

			UBOOL NextBIA = &_Communicator().BuddyList(ListIndex) != BuddyBIA;
			if( CurrBIA != NextBIA  )
			{
				GroupFirstIndices.AddItem( ListItemIndex );
				CurrBIA = NextBIA;
			}
		}
	}

	for( INT GroupIndex = 0 ; GroupIndex < GroupFirstIndices.Num() ; GroupIndex++ )
	{
		INT GroupFirstIndex = GroupFirstIndices(GroupIndex);
		INT GroupLastIndex = GroupFirstIndices.IsValidIndex(GroupIndex + 1) ? GroupFirstIndices(GroupIndex + 1) : ListItems.Num();

		FUIListItem* FirstItem = ListItems.GetTypedData() + GroupFirstIndex;
		FUIListItem* LastItem = ListItems.GetTypedData() + GroupLastIndex;

		if ( PrimaryCellTag ==  FName(TEXT("Correlation")) )
		{
			ava::sort( FirstItem, LastItem, 
				UIListItemSort(SortParameters, ParseArgumentSort<FString>(TEXT("Correlation_"), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
			bResult = TRUE;
		}
		else if ( PrimaryCellTag ==  FName(TEXT("LevelIcon")) )
		{
			ava::sort( FirstItem, LastItem, 
				UIListItemSort( SortParameters, ParseArgumentSort<INT>(TEXT("Icon_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
			bResult = TRUE;
		}
		else if ( PrimaryCellTag ==  FName(TEXT("LevelName")) || PrimaryCellTag ==  FName(TEXT("LevelCombo")) )
		{
			ava::sort( FirstItem, LastItem, 
				UIListItemSort(SortParameters, ParseArgumentSort<INT>(TEXT("Name_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE ) );
			bResult = TRUE;
		}
		else if ( PrimaryCellTag ==  FName(TEXT("NickName")) || PrimaryCellTag ==  FName(TEXT("NickNameCombo")) ||
			PrimaryCellTag ==  FName(TEXT("GuildName")) || PrimaryCellTag ==  FName(TEXT("NetLocation")) )
		{
			ava::sort( FirstItem, LastItem,
				UIListItemSortBase<NormalStringSort>(SortParameters, NormalStringSort()));
			bResult = TRUE;
		}
		else if ( PrimaryCellTag ==  FName(TEXT("OnlineStatus")) )
		{
			ava::sort( FirstItem, LastItem, 
				UIListItemSort(SortParameters, ParseArgumentSort<FString>(TEXT("OnlineStatus_"), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
			bResult = TRUE;
		}
	}

	return bResult;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetRoomSettings
========================================================================================================== */

void UUIDataProvider_AvaNetRoomSettings::LoadRoomSettings()
{
	if( _StateController->IsStateInRoom() )
	{
		FRoomDispInfo* RoomInfo = GetSelectedRoomInfo();
		FMapInfo* MapInfo = GetSelectedMapInfo();
		if( RoomInfo && MapInfo )
		{
			SelectedMissionType = MapInfo->MissionType;
			FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting(SelectedMissionType);
			check(RoomSetting);

			TArray<FMapInfo> MapList;
			GetMapList( MapList );

			RoomName = RoomInfo->RoomInfo.roomName;
			RoomSetting->idMap = MapInfo->idMap;
			RoomSetting->tkLevel = RoomInfo->RoomInfo.setting.tkLevel;
			RoomSetting->autoBalance = RoomInfo->RoomInfo.setting.autoBalance;
			RoomSetting->allowSpectator = RoomInfo->RoomInfo.setting.allowSpectator;
			RoomSetting->allowInterrupt = RoomInfo->RoomInfo.setting.allowInterrupt;
			RoomSetting->allowBackView = RoomInfo->RoomInfo.setting.allowBackView;
			RoomSetting->allowGameGhostChat = RoomInfo->RoomInfo.setting.allowGhostChat;
			RoomSetting->autoSwapTeam = RoomInfo->RoomInfo.setting.autoSwapTeam;
			RoomSetting->roundToWin = RoomInfo->RoomInfo.setting.roundToWin;
			RoomSetting->mapOption = RoomInfo->RoomInfo.setting.mapOption;
			//RoomSetting->MaxPlayer = RoomInfo->RoomInfo.setting.numMax;

			// 방인원 변경은 방생성이후에는 불가능하므로 모든 방설정값을 현재 방인원값으로 맞춘다
			for( INT MissionType = 0 ; MissionType < NMT_MAX ; MissionType++ )
			{
				FavaRoomSetting* EachRoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting(MissionType);
				if( EachRoomSetting )
					EachRoomSetting->MaxPlayer = RoomInfo->RoomInfo.setting.numMax;
			}
		}
	}
	else
	{
		GetAvaNetRequest()->ResetRoomSettingsAsDefault();

		SelectedMissionType = NMT_Mission;
		RoomName = GetAvaNetRequest()->GetRandomRoomName();
		RoomPassword = TEXT("");
	}
}

void UUIDataProvider_AvaNetRoomSettings::SendChangedRequest()
{
	FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting( SelectedMissionType );

	if( RoomSetting )
	{
		if( _StateController->GetNetState() == _AN_ROOM )
			GetAvaNetRequest()->RoomSetting( *RoomSetting );
		else
			GetAvaNetRequest()->CreateRoom( RoomName, RoomPassword, *RoomSetting );
	}
	else
	{
		warnf(TEXT("Setting Not Found"));
	}
}

IMPLEMENT_CLASS(UUIDataProvider_AvaNetRoomSettings);

/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetRoomSettings::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	// 방생성/ 방설정
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoomName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoomPassword")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapImage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("tkLevel")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("levelLimit")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("autoBalance")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowSpectator")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowInterrupt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowBackView")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowGhostChat")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("roundToWin")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("maxRoomPlayer")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("automaticTeamSwap")));

	// 대기방 디스플레이
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoomName_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoomPassword_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapName_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MapImage_Diabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("tkLevel_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("levelLimit_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("autoBalance_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowSpectator_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowInterrupt_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowBackView_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("allowGhostChat_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("roundToWin_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("maxRoomPlayer_Disabled")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("automaticTeamSwap_Disabled")));

	// 서로 다른 리스트들 RoomSettings에서 묶어서 처리합니다.
	new(out_Fields) FUIDataProviderField(FName(TEXT("AvailMaps")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("AvailWinConds")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("AvailMaxPlayers")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("AvailTeamKillLevels")),DATATYPE_Collection);

	// Enable/Disable/Visible/Hidden Handler
	new(out_Fields) FUIDataProviderField(FName(TEXT("Handler_MissionType_Mission")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Handler_MissionType_Warfare")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Handler_MissionType_MilitaryDrill")));
}

/**
* Retrieves the list elements associated with the data tag specified.
*
* @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
*							from GetElementProviderTags.
* @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
*
* @return	TRUE if this data store contains a list element data provider matching the tag specified.
*/
UBOOL UUIDataProvider_AvaNetRoomSettings::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );
	
	if( FieldName == TEXT("AvailWinConds") )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT i = 0 ; i < 5 ; i++ )
				out_Elements.AddItem(i);
		}
		else
		{
			FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting( SelectedMissionType );
			FMapInfo* MapInfo = NULL;
			if( RoomSetting && (MapInfo = _StateController->MapList.Find(RoomSetting->idMap)) != NULL )
			{
				for( INT i = 0 ; i < MapInfo->WinCondList.Num() ; i++)
				{
					out_Elements.AddItem(MapInfo->WinCondList(i));
				}
			}
		}
	}
	else if ( FieldName == TEXT("AvailMaxPlayers") )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT i = 0 ; i < 5 ; i++ )
				out_Elements.AddItem(i);
		}
		else
		{
			if( GetCurrentChannelGroup() == EChannelFlag_Practice )
			{
				out_Elements.AddItem( 6 );
			}
			else
			{
				FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting( SelectedMissionType );
				FMapInfo* MapInfo = NULL;
				if( RoomSetting && (MapInfo = _StateController->MapList.Find(RoomSetting->idMap)) != NULL )
				{
					for( INT i = 0 ; i < MapInfo->MaxPlayerList.Num() ; i++)
					{
						out_Elements.AddItem(MapInfo->MaxPlayerList(i));
					}
				}
			}
		}
	}
	else if ( FieldName == TEXT("AvailTeamKillLevels") )
	{
		for( INT i = 0 ; i < 3 ; i++ )
			out_Elements.AddItem(i);
	}
	else if ( FieldName == TEXT("AvailMaps") )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT Index = 0 ; Index < 10 ;Index++ )
				out_Elements.AddItem(Index);
		}
		else
		{
			TArray<FMapInfo> MapList;
			GetMapList( MapList );
			for (INT Index = 0; Index < MapList.Num() ; ++Index)
			{
				FMapInfo& MapInfo = MapList(Index);
				UBOOL bHidden = MapInfo.bHidden;
				bHidden = bHidden || MapInfo.ExclChannelGroups.FindItemIndex( GetCurrentChannelGroup() ) != INDEX_NONE;
				bHidden = bHidden || (SelectedMissionType != INDEX_NONE && SelectedMissionType != MapInfo.MissionType);

				if( !bHidden )
					out_Elements.AddItem(Index);
			}
		}
	}

	return TRUE;
}

/**
* Retrieves the list of tags that can be bound to individual cells in a single list element.
*
* @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
*/
void UUIDataProvider_AvaNetRoomSettings::GetElementCellTags( TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(FName(TEXT("WinCond")), TEXT("WinCond"));
	out_CellTags.Set(FName(TEXT("MaxPlayers")), TEXT("MaxPlayers"));
	out_CellTags.Set(FName(TEXT("TeamKillLevel")), TEXT("TeamKillLevel"));
	out_CellTags.Set(FName(TEXT("MapName")), TEXT("MapName"));
	out_CellTags.Set(FName(TEXT("MapDesc")), TEXT("MapDesc"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	out_FieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetRoomSettings::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex /*=INDEX_NONE*/ )
{
	if( CellTag == TEXT("WinCond") )
	{
		if( GIsEditor && !GIsGame )
		{
			out_FieldValue.StringValue = FString::Printf(TEXT("WinCond %d"),ListIndex);
		}
		else
		{
			FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting( SelectedMissionType );
			FMapInfo* MapInfo = NULL;
			if( RoomSetting && (MapInfo = _StateController->MapList.Find(RoomSetting->idMap)) != NULL )
			{
				out_FieldValue.StringValue = GetWinConditionString( MapInfo->WinCondType, ListIndex );
			}
			else
				out_FieldValue.StringValue = TEXT("mapinfo ?");
		}
	}
	else if ( CellTag == TEXT("MaxPlayers") )
	{
		if( GIsEditor && !GIsGame )
		{
			out_FieldValue.StringValue = FString::Printf(TEXT("MaxPlayers %d"),ListIndex);
		}
		else
		{
			out_FieldValue.StringValue = appItoa( ListIndex ) + Localize(TEXT("UIGeneral"),TEXT("Text_Persons"),TEXT("avaNet"));
		}
	}
	else if ( CellTag == TEXT("TeamKillLevel") )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"), *FString::Printf(TEXT("Text_TeamKillLevel[%d]"),ListIndex), TEXT("avaNet"));
	}
	else if ( CellTag == TEXT("MapName") )
	{
		FMapInfo* MapInfo = GetMapInfo( ListIndex );
		out_FieldValue.StringValue = MapInfo ? MapInfo->MapName : TEXT("MapName");
	}
	else if ( CellTag == TEXT("MapDesc") )
	{
		FMapInfo* MapInfo = GetMapInfo( ListIndex );
		if( MapInfo )
			out_FieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"),*FString::Printf(TEXT("Text_MapTypeName[%s]"),*MapInfo->Description),TEXT("avaNet"));
		else
			out_FieldValue.StringValue = TEXT("[MapDesc]");
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}


/**
* 파라메터들을 모두 업데이트 한후에 필요한 처리가 있다면 추가
* 예) 무기 인벤토리의 리스트 인덱스를 파라메터로 받았음 -(PostUpdateParamters)-> ItemID를 계산한다
*/
void UUIDataProvider_AvaNetRoomSettings::PostUpdateParamters( UBOOL bParmChanged )
{
	if( bParmChanged )
	{
		UavaNetRequest* NetRequest = GetAvaNetRequest();
		
		if ( bLoadRoomSetting )
		{
			LoadRoomSettings();
			bLoadRoomSetting = FALSE;
		}

		if ( bSettingChangeRequest )
		{
			SendChangedRequest();
			bSettingChangeRequest = FALSE;
		}

		PreviousMissionType = SelectedMissionType;
	}
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetRoomSettings::GetField(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	FName InFieldName = *FieldName;
	UBOOL bAllowEmpty = FALSE;

	// 현재 미션타입에 맞는 저장된 방 설정 정보를 가져온다
	FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting( SelectedMissionType );
	if( InFieldName == TEXT("RoomName") || InFieldName == TEXT("RoomName_Disabled") )
	{
		OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? FString(TEXT("roomname")) : RoomName;
	}
	else if( InFieldName == TEXT("RoomPassword") || InFieldName == TEXT("RoomPassword_Disabled") )
	{
		bAllowEmpty = TRUE;
		OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? FString(TEXT("password")) : RoomPassword;
	}
	else if( InFieldName == TEXT("MapName") || InFieldName == TEXT("MapName_Disabled") )
	{
		FMapInfo* MapInfo = _StateController->MapList.Find( RoomSetting ? RoomSetting->idMap : 0 );
		OutFieldValue.StringValue = MapInfo ? MapInfo->MapName : TEXT("MapName");
	}
	else if( InFieldName == TEXT("MapImage") || InFieldName == TEXT("MapImage_Disabled") )
	{
		FMapInfo* MapInfo = _StateController->MapList.Find( RoomSetting ? RoomSetting->idMap : 0 );
		OutFieldValue.StringValue = MapInfo ? FString(TEXT("<Images:")) + MapInfo->ImagePathName + FString(TEXT(";XL=186 YL=68>")) : TEXT("map image");
	}
	else if( InFieldName == TEXT("tkLevel") || InFieldName == TEXT("tkLevel_Disabled") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"), *FString::Printf(TEXT("Text_TeamKillLevel[%d]"), RoomSetting ? RoomSetting->tkLevel : 0),TEXT("avaNet"));
	}
	else if( InFieldName == TEXT("levelLimit") || InFieldName == TEXT("levelLimit_Disabled"))
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"), TEXT("Text_LevelLimit_Data[0]"), TEXT("avaNet"));
	}
	else if ( InFieldName == TEXT("autoBalance") || InFieldName == TEXT("autoBalance_Disabled") )
	{
		OutFieldValue.StringValue = RoomSetting && RoomSetting->autoBalance ? GTrue : GFalse;
	}
	else if ( InFieldName == TEXT("allowSpectator") || InFieldName == TEXT("allowSpectator_Disabled"))
	{
		OutFieldValue.StringValue = RoomSetting && RoomSetting->allowSpectator ? GTrue : GFalse;
	}
	else if ( InFieldName == TEXT("allowInterrupt") || InFieldName == TEXT("allowInterrupt_Disabled"))
	{
		OutFieldValue.StringValue = RoomSetting && RoomSetting->allowInterrupt ? GTrue : GFalse;
	}
	else if ( InFieldName == TEXT("allowBackView") || InFieldName == TEXT("allowBackView_Disabled"))
	{
		OutFieldValue.StringValue = RoomSetting && RoomSetting->allowBackView ? GTrue : GFalse;
	}
	else if ( InFieldName == TEXT("allowGhostChat") || InFieldName == TEXT("allowGhostChat_Disabled"))
	{
		OutFieldValue.StringValue = RoomSetting && RoomSetting->allowGameGhostChat ? GTrue : GFalse;
	}
	else if ( InFieldName == TEXT("roundToWin") || InFieldName == TEXT("roundToWin_Disabled"))
	{
		FMapInfo* MapInfo = _StateController->MapList.Find( RoomSetting ? RoomSetting->idMap : 0 );
		OutFieldValue.StringValue = MapInfo ? GetWinConditionString(MapInfo->WinCondType, RoomSetting ? RoomSetting->roundToWin : 0) : GNone;
	}
	else if ( InFieldName == TEXT("maxRoomPlayer") || InFieldName == TEXT("maxRoomPlayer_Disabled"))
	{
		OutFieldValue.StringValue = appItoa( RoomSetting ? RoomSetting->MaxPlayer : 10 ) + Localize(TEXT("UIGeneral"), TEXT("Text_Persons"), TEXT("avaNet"));
	}
	else if ( InFieldName == TEXT("automaticTeamSwap") || InFieldName == TEXT("automaticTeamSwap_Disabled"))
	{
		OutFieldValue.StringValue = RoomSetting && RoomSetting->autoSwapTeam ? GTrue : GFalse;
	}
	else if ( InFieldName == TEXT("Handler_MissionType_Mission"))
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"),TEXT("Text_MissionType[Mission]"),TEXT("avaNet"));
	}
	else if ( InFieldName == TEXT("Handler_MissionType_Warfare"))
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"),TEXT("Text_MissionType[Warfare]"),TEXT("avaNet"));
	}
	else if ( InFieldName == TEXT("Handler_MissionType_MilitaryDrill"))
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"),TEXT("Text_MissionType[MilitaryDrill]"),TEXT("avaNet"));
	}

	if( !bAllowEmpty && OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = GNone;
	}
	return TRUE;
}

UBOOL UUIDataProvider_AvaNetRoomSettings::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex /*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	FName InFieldName = *FieldName;

	FString StringValue = FieldValue.StringValue;
	FLOAT FloatValue = FieldValue.RangeValue.GetCurrentValue();
	INT IntValue = appTrunc(FloatValue);
	UBOOL bValue = IntValue ? TRUE : FALSE;

	FavaRoomSetting* RoomSetting = GetAvaNetRequest()->GetCurrentRoomSetting(SelectedMissionType);

	if( RoomSetting )
	{
		if( InFieldName == TEXT("RoomName") )
		{
			RoomName = StringValue;
			bResult = TRUE;
		}
		else if( InFieldName == TEXT("RoomPassword") )
		{
			RoomPassword = StringValue;
			bResult = TRUE;
		}
		else if( InFieldName == TEXT("MapName") )
		{
			INT MapIndex;
			for( MapIndex = _StateController->MapList.MapList.Num() - 1 ; MapIndex >= 0  ; MapIndex-- )
				if( _StateController->MapList.MapList(MapIndex).idMap == RoomSetting->idMap )
					break;

			INT NewMapIndex = IntValue;
			if( MapIndex >= 0 && MapIndex != NewMapIndex )
			{
				RoomSetting->idMap = _StateController->MapList.MapList(NewMapIndex).idMap;
				FMapInfo* MapInfo = _StateController->MapList.Find(RoomSetting->idMap);
				if( MapInfo )
				{
					RoomSetting->roundToWin = MapInfo->DefaultWinCond;

					// ReadyRoom(대기방/친선클랜 대기방)에서는 인원수를 변경하지 않는다.
					if ( !_StateController->IsStateInRoom() )
						RoomSetting->MaxPlayer = MapInfo->DefaultMaxPlayer;

					bResult = TRUE;
				}
			}
		}
		else if( InFieldName == TEXT("MapImage") )
		{
			bResult = TRUE;
		}
		else if( InFieldName == TEXT("tkLevel") )
		{
			if( RoomSetting->tkLevel != IntValue )
			{
				RoomSetting->tkLevel = IntValue;
				bResult = TRUE;
			}
		}
		else if( InFieldName == TEXT("levelLimit") )
		{
			bResult = TRUE;
		}
		else if ( InFieldName == TEXT("autoBalance") )
		{
			if( RoomSetting->autoBalance != bValue )
			{
				RoomSetting->autoBalance = IntValue;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("allowSpectator") )
		{
			if( RoomSetting->allowSpectator != bValue )
			{
				RoomSetting->allowSpectator = bValue;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("allowInterrupt") )
		{
			if( RoomSetting->allowInterrupt != bValue )
			{
				RoomSetting->allowInterrupt = bValue;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("allowBackView") )
		{
			if( RoomSetting->allowBackView != bValue )
			{
				RoomSetting->allowBackView = bValue;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("allowGhostChat") )
		{
			if( RoomSetting->allowGameGhostChat != bValue )
			{
				RoomSetting->allowGameGhostChat = bValue;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("roundToWin") )
		{
			if( RoomSetting->roundToWin != IntValue )
			{
				RoomSetting->roundToWin = IntValue;
				RoomSetting->mapOption = RoomSetting->roundToWin > 0 ? 1 : 0;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("maxRoomPlayer") )
		{
			if( RoomSetting->MaxPlayer != IntValue )
			{
				// ReadyRoom에서는 인원수를 변경하지 않는다.
				if ( !IsStateInRoom() ) // ROOM[2]
					RoomSetting->MaxPlayer = IntValue;
				bResult = TRUE;
			}
		}
		else if ( InFieldName == TEXT("automaticTeamSwap") )
		{
			if( RoomSetting->autoSwapTeam	 != bValue )
			{
				RoomSetting->autoSwapTeam = bValue;
				bResult = TRUE;
			}
		}
	}

	// 설정이 바뀔때마다 서버에 방설정 갱신을 요청
	//if( bResult && IsStateInRoom() )
	//{
	//	bSettingChangeRequestTrigger = TRUE;
	//	PostUpdateParamters( TRUE );
	//}

	return bResult;
}

void UUIDataProvider_AvaNetRoomSettings::UpdateWidget( const FString& FieldName, UUIObject* TargetWidget )
{
	check(TargetWidget);
	FName Field = *FieldName;

	UBOOL bEnabled = TargetWidget->IsEnabled();
	UBOOL bVisible = TargetWidget->IsVisible();

	FavaNetChannelSettingInfo& ChannelSettings = _StateController->GetChannelSettingInfo();
	BYTE CurrentChannel = GetCurrentChannelGroup();

	// 미션타입버튼의 활성/비활성, 체크여부를 결정
	if(	Field == TEXT("Handler_MissionType_Mission") ||
		Field == TEXT("Handler_MissionType_Warfare") ||
		Field == TEXT("Handler_MissionType_MilitaryDrill"))
	{
		if( _StateController->GetNetState() == _AN_ROOM )
		{
			FMapInfo* MapInfo = GetSelectedMapInfo();

			if( MapInfo )
			{
				UBOOL bIsChecked = FALSE;
				if( MapInfo->MissionType == NMT_Mission && Field == TEXT("Handler_MissionType_Mission") ||
					MapInfo->MissionType == NMT_Warfare && Field == TEXT("Handler_MissionType_Warfare") ||
					MapInfo->MissionType == NMT_MilitaryDrill && Field == TEXT("Handler_MissionType_MilitaryDrill") )
					bIsChecked = TRUE;

				UavaUICheckLabelButton *CLButton = Cast<UavaUICheckLabelButton>(TargetWidget);
				if( CLButton != NULL )
					CLButton->SetValue(bIsChecked);
			}
		}
		else
		{
			UBOOL bIsChecked = FALSE;
			if( ChannelSettings.MissionTypeSpecial )
			{
				if( Field == TEXT("Handler_MissionType_Mission") )
					bIsChecked = TRUE;
			}
			else if( ChannelSettings.MissionTypeWarfare )
			{
				if( Field == TEXT("Handler_MissionType_Warfare") )
					bIsChecked = TRUE;
			}
			else if( ChannelSettings.MissionTypeTraining )
			{
				if( Field == TEXT("Handler_MissionType_MilitaryDrill"))
					bIsChecked = TRUE;
			}

			UavaUICheckLabelButton *CLButton = Cast<UavaUICheckLabelButton>(TargetWidget);
			if( CLButton != NULL )
			{
				// CheckLabelButton : TRUE 이벤트를 확실히 발생하도록 한번 FALSE로 바꿔줌
				if( CLButton->bIsChecked )
					CLButton->SetValue( FALSE );
				CLButton->SetValue( bIsChecked );
			}
		}
	}

	//// 군사훈련버튼은 클랜홈/친선클랜/연습채널/방송채널에서 볼 수 없다.
	//if( Field == TEXT("Handler_MissionType_MilitaryDrill") )
	//{
	//	if( CurrentChannel == EChannelFlag_Clan ||
	//		CurrentChannel == EChannelFlag_MyClan ||
	//		CurrentChannel == EChannelFlag_Practice ||
	//		CurrentChannel == EChannelFlag_Match)
	//	{
	//		bVisible = FALSE;
	//	}
	//}

	//// 전면전은 연습채널에서 볼 수 없다.
	//if( Field == TEXT("Handler_MissionType_Warfare") )
	//{
	//	if( CurrentChannel == EChannelFlag_Practice )
	//	{
	//		bVisible = FALSE;
	//	}
	//}

	//// 특수전은 방송채널에서 볼 수 없다.
	//if ( Field == TEXT("Handler_MissionType_Mission") )
	//{
	//	if( CurrentChannel == EChannelFlag_Match )
	//	{
	//		bVisible = FALSE;
	//	}
	//}

	//if( Field == TEXT("autoBalance") )
	//{
	//	bEnabled = SelectedMissionType == NMT_MilitaryDrill ? FALSE : TRUE;
	//	if(CurrentChannel == EChannelFlag_AutoBalance ||		// 자동팀균형 채널에서 '자동팀균형'은 변경 불가
	//		CurrentChannel == EChannelFlag_Clan)	// 클랜홈에서는 자동팀균형 불가
	//	{
	//		bEnabled = FALSE;
	//	}
	//}
	//// 방설정변경시 최대인원 변경불가
	//// 연습채널에서 방최대인원 변경 불가
	//if( Field == TEXT("maxRoomPlayer") && 
	//	(IsStateInRoom() || CurrentChannel == EChannelFlag_Practice) )
	//{
	//	bEnabled = FALSE;
	//}

	if( Field == TEXT("Handler_MissionType_Mission") )
	{
		bVisible = ChannelSettings.MissionTypeSpecial;
	}
	else if( Field == TEXT("Handler_MissionType_Warfare") )
	{
		bVisible = ChannelSettings.MissionTypeWarfare;
	}
	else if( Field == TEXT("Handler_MissionType_MilitaryDrill") )
	{
		bVisible = ChannelSettings.MissionTypeTraining;
	}
	else if( Field == TEXT("tkLevel"))
	{
		bEnabled = ChannelSettings.EnableTKLevel;
	}
	else if( Field == TEXT("autoBalance") )
	{
		bEnabled = SelectedMissionType != NMT_MilitaryDrill && ChannelSettings.EnableAutoBalance;
	}
	else if( Field == TEXT("allowSpectator"))
	{
		bEnabled = ChannelSettings.EnableSpectator;
	}
	else if( Field == TEXT("allowInterrupt"))
	{
		bEnabled = ChannelSettings.EnableInterrupt;
	}
	else if( Field == TEXT("allowBackView"))
	{
		bEnabled = ChannelSettings.EnableBackView;
	}
	else if( Field == TEXT("allowGhostChat"))
	{
		bEnabled = ChannelSettings.EnableGhostChat;
	}
	else if( Field == TEXT("maxRoomPlayer") )
	{
		bEnabled = ChannelSettings.EnableMaxPlayers && !IsStateInRoom();
	}
	else if( Field == TEXT("automaticTeamSwap"))
	{
		bEnabled = ChannelSettings.EnableAutoSwapTeam;
	}

	// 방이름/비밀번호는 대기방에서 변경 불가
	if( IsStateInRoom() &&
		(Field == TEXT("RoomPassword") || Field == TEXT("RoomName")) )
	{
		bEnabled = FALSE;
	}

	// 대기방/친선클랜대기방에서 읽기전용으로만 쓰일 필드에 대해서는 항상 비활성화해준다
	if( FieldName.InStr(TEXT("_Disabled")) != INDEX_NONE )
	{
		bEnabled = FALSE;
	}

	if( TargetWidget->IsEnabled() != bEnabled )
	{
		UUIComboBox* ComboBox = Cast<UUIComboBox>(TargetWidget->GetOwner());
		if( ComboBox )
			ComboBox->SetEnabled(bEnabled);
		else
			TargetWidget->SetEnabled(bEnabled);
	}

	if( TargetWidget->IsVisible() != bVisible)
	{
		UUIComboBox* ComboBox = Cast<UUIComboBox>(TargetWidget->GetOwner());
		if( ComboBox )
			ComboBox->eventSetVisibility(bVisible);
		else
			TargetWidget->eventSetVisibility(bVisible);
	}
}

/* ==========================================================================================================
UUIDataProvider_AvaNetLobbyBlockedPlayers
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLobbyBlockedPlayers);


void UUIDataProvider_AvaNetLobbyBlockedPlayers::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Correlation")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_Correlation"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelCombo")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelCombo"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelIcon")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelIcon"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NickName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_NickName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("GuildName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_GuildName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NetLocation")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_NetLocation"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("OnlineStatus")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_OnlineStatus"), TEXT("avaNet")) );
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetLobbyBlockedPlayers::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		OutFieldValue.StringValue = FString(CellTag.GetName()).Left(3) + appItoa(ListIndex);
	}
	else
	{
		if ( _Communicator().BlockList.BuddyList.IsValidIndex( ListIndex ) )
		{
			FBuddyInfo& BuddyInfo = _Communicator().BlockList(ListIndex);
			if ( CellTag ==  FName(TEXT("Correlation")) )
			{
				OutFieldValue.StringValue = ( BuddyInfo.BuddyType == BT_BUDDY_ONESIDE) ? Localize(TEXT("UILobbyScene"),TEXT("Icon_Correlation_UniDir"),TEXT("avaNet")) :
					( BuddyInfo.BuddyType == BT_BUDDY_BOTH) ? Localize(TEXT("UILobbyScene"),TEXT("Icon_Correlation_BiDir"),TEXT("avaNet")) :
					( BuddyInfo.BuddyType == BT_BUDDY_OTHER) ? Localize(TEXT("UILobbyScene"),TEXT("Icon_Correlation_Other"),TEXT("avaNet")) :
					( BuddyInfo.BuddyType == BT_BLOCK) ? Localize(TEXT("UILobbyScene"),TEXT("Icon_Correlation_Block"),TEXT("avaNet")) :
					( BuddyInfo.BuddyType == BT_BUDDY_BIA) ? Localize(TEXT("UILobbyScene"),TEXT("Icon_Correlation_BIA"),TEXT("avaNet")) : FString(TEXT("Inv. Rel"));
			}
			else if ( CellTag ==  FName(TEXT("LevelCombo")) )
			{
				OutFieldValue.StringValue = GetLevelIconString( BuddyInfo.Level ) + GetLevelNameString( BuddyInfo.Level);
			}
			else if ( CellTag ==  FName(TEXT("LevelIcon")) )
			{
				OutFieldValue.StringValue = GetLevelIconString( BuddyInfo.Level );
			}
			else if ( CellTag ==  FName(TEXT("LevelName")) )
			{
				OutFieldValue.StringValue = GetLevelNameString( BuddyInfo.Level );
			}
			else if ( CellTag ==  FName(TEXT("NickName")) )
			{
				OutFieldValue.StringValue = BuddyInfo.Nickname;
			}
			else if ( CellTag ==  FName(TEXT("GuildName")) )
			{
				OutFieldValue.StringValue = BuddyInfo.GuildName.Trim().Len() != 0 ? BuddyInfo.GuildName : Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
			}
			else if ( CellTag ==  FName(TEXT("NetLocation")) )
			{
				OutFieldValue.StringValue = BuddyInfo.GetLocation();
			}
			else if ( CellTag ==  FName(TEXT("OnlineStatus")) )
			{
				OutFieldValue.StringValue = GetOnlineStatusIconString( BuddyInfo.IsOnline(), BuddyInfo.BuddyType );
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("Inv.");
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("N/A");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetLobbyBlockedPlayers::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

UBOOL UUIDataProvider_AvaNetLobbyBlockedPlayers::SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	if ( PrimaryCellTag ==  FName(TEXT("Correlation")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(), 
			UIListItemSortBase<ParseArgumentSort<FString>, NormalStringSort>
			( SortParameters, ParseArgumentSort<FString>(TEXT("Correlation_"), TRUE, TRUE), NormalStringSort(), FALSE,TRUE) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("LevelIcon")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<ParseArgumentSort<INT>, NormalStringSort>
			( SortParameters, ParseArgumentSort<INT>(TEXT("Icon_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("LevelName")) || PrimaryCellTag ==  FName(TEXT("LevelCombo")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<ParseArgumentSort<INT>, NormalStringSort>
			( SortParameters, ParseArgumentSort<INT>(TEXT("Name_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("NickName")) || PrimaryCellTag ==  FName(TEXT("NickNameCombo")) ||
		PrimaryCellTag ==  FName(TEXT("GuildName")) || PrimaryCellTag ==  FName(TEXT("NetLocation")) )
	{
		ava::sort(ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(), 
			UIListItemSortBase<NormalStringSort>( SortParameters, NormalStringSort()) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("OnlineStatus")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(), 
			UIListItemSortBase<ParseArgumentSort<FString>,NormalStringSort>
			( SortParameters, ParseArgumentSort<FString>(TEXT("OnlineStatus_"), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================================================
UUIDataProvider_AvaNetRoomPlayerField
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetRoomPlayerField);

/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetRoomPlayerField::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("NickName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LevelIcon")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LevelName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("OnlineStatus")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameWinCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameLoseCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameDefeatCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DisconnectCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("KillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DeathCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScorePerDeath")));
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetRoomPlayerField::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if( GIsEditor && !GIsGame )
	{
		if ( FieldName.ToLower().InStr(TEXT("count")) >= 0 )
		{
			OutFieldValue.StringValue = appItoa(0);
		}
		else if ( FieldName.ToLower().InStr(TEXT("per")) >= 0 )
		{
			OutFieldValue.StringValue = TEXT("0.0");
		}
		else
			OutFieldValue.StringValue = FieldName;
	}
	else
	{
		BYTE RoomSlotIdx = INDEX_NONE;
		Parse( *FieldName, TEXT("RoomSlotIdx="), RoomSlotIdx );
		FPlayerDispInfo* PlayerDispInfo = GetRoomPlayerDispInfo( RoomSlotIdx );

		if( PlayerDispInfo != NULL && PlayerDispInfo->IsFullInfo() )
		{
			Def::PLAYER_DISP_INFO &PlayerInfo = PlayerDispInfo->PlayerInfo;

			FName InFieldName;
			{
				INT InStrRes = FieldName.InStr(TEXT("?"));
				FString TruncFieldName = InStrRes != INDEX_NONE ? FieldName.Left( InStrRes ) : FieldName;
				InFieldName = FName( *TruncFieldName );
			}
			const TCHAR* szFieldName = InFieldName.GetName();
			if ( InFieldName == TEXT("NickName"))
			{
				OutFieldValue.StringValue = PlayerInfo.nickname;
			}
			else if ( InFieldName == TEXT("LevelIcon"))
			{
				OutFieldValue.StringValue = GetLevelIconString( PlayerInfo.level );
			}
			else if ( InFieldName == TEXT("LevelName"))
			{
				OutFieldValue.StringValue = GetLevelNameString( PlayerInfo.level );
			}
			else if ( InFieldName == TEXT("GuildName"))
			{
				OutFieldValue.StringValue = PlayerInfo.guildName;
			}
			else if ( InFieldName == TEXT("GameWinCount"))
			{
				OutFieldValue.StringValue = appItoa(PlayerInfo.gameWin);
			}
			else if ( InFieldName == TEXT("GameLoseCount"))
			{
				OutFieldValue.StringValue = appItoa(PlayerInfo.gameDefeat);
			}
			else if ( InFieldName == TEXT("GameDefeatCount"))
			{
				OutFieldValue.StringValue = appItoa(PlayerInfo.gameDefeat);
			}
			else if ( InFieldName == TEXT("DisconnectCount"))
			{
				OutFieldValue.StringValue = appItoa(PlayerInfo.disconnectCount);
			}
			else if ( InFieldName == TEXT("KillCount"))
			{
				OutFieldValue.StringValue = appItoa(PlayerInfo.killCount);
			}
			else if ( InFieldName == TEXT("DeathCount"))
			{
				OutFieldValue.StringValue = appItoa(PlayerInfo.deathCount);
			}
			else if ( InFieldName == TEXT("ScorePerDeath"))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"),PlayerInfo.GetSDRatio());
			}
		}
		else if( PlayerDispInfo != NULL && !PlayerDispInfo->IsFullInfo() )
		{
			OutFieldValue.StringValue = TEXT("-");
		}
		else
		{
			OutFieldValue.StringValue = RoomSlotIdx == INDEX_NONE ? TEXT("'RoomSlotIdx' Parameter not found") : TEXT("Inv. playerinfo");
		}
	}

	if( OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/* ==========================================================================================================
UUIDataProvider_AvaNetChatMsgs
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetChatMsgs);


void UUIDataProvider_AvaNetChatMsgs::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("MsgText")),TEXT("MsgText"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetChatMsgs::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("MsgText")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("채팅 메시지 %02d"), ListIndex);
			}
		}
	}
	else
	{
		if (ListIndex >= 0 && ListIndex < _StateController->ChatMsgList.ChatList.Num())
		{
			if (CellTag == FName(TEXT("MsgText")))
			{
				OutFieldValue.StringValue = _StateController->ChatMsgList[_StateController->ChatMsgList.ChatList.Num() - ListIndex - 1];
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetChatMsgs::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}



/* ==========================================================================================================
UUIDataProvider_AvaNetPlayerWeapon
========================================================================================================== */

#define _GEN_SLOTNAME(X) if (SlotType & _EP_##X) return TEXT(#X)

inline FString GetEquipSlotName(INT SlotType)
{
	_GEN_SLOTNAME(H1);
	_GEN_SLOTNAME(H11);
	_GEN_SLOTNAME(H12);
	_GEN_SLOTNAME(H2);
	_GEN_SLOTNAME(H3);
	_GEN_SLOTNAME(C1);
	_GEN_SLOTNAME(C2);
	_GEN_SLOTNAME(A1);
	_GEN_SLOTNAME(A2);
	_GEN_SLOTNAME(B1);
	_GEN_SLOTNAME(B3);
	_GEN_SLOTNAME(W1);
	_GEN_SLOTNAME(W2);
	_GEN_SLOTNAME(W3);
	_GEN_SLOTNAME(T1);
	_GEN_SLOTNAME(T2);
	_GEN_SLOTNAME(E);
	_GEN_SLOTNAME(G);
	_GEN_SLOTNAME(K);
	_GEN_SLOTNAME(BT);
	_GEN_SLOTNAME(BD);

	return TEXT("??");
}

inline FString GetWeaponSlotName(INT SlotType)
{
	_GEN_SLOTNAME(R1);
	_GEN_SLOTNAME(R2);
	_GEN_SLOTNAME(R3);
	_GEN_SLOTNAME(R4);
	_GEN_SLOTNAME(P1);
	_GEN_SLOTNAME(P2);
	_GEN_SLOTNAME(P3);
	_GEN_SLOTNAME(P4);
	_GEN_SLOTNAME(S1);
	_GEN_SLOTNAME(S2);
	_GEN_SLOTNAME(S3);
	_GEN_SLOTNAME(S4);

	return TEXT("??");
}

#undef _GEN_SLOTNAME



IMPLEMENT_CLASS(UUIDataProvider_AvaNetPlayerWeapon);



/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetPlayerWeapon::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if (GIsEditor && !GIsGame)
	{
		if (FieldName == TEXT("SlotName"))
		{
			SLOT_DESC *pSlot = _ItemDesc().GetWeaponSlot(SlotID);
			OutFieldValue.StringValue = (pSlot ? GetWeaponSlotName(pSlot->slotType) : TEXT("??"));
		}
		else if (FieldName == TEXT("ItemID"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("10%02d"), SlotID);
		}
		else if (FieldName == TEXT("ItemName"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Weapon10%02d"), SlotID);
		}
		else if (FieldName == TEXT("ItemDesc"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Weapon10%02d description"), SlotID);
		}
		else if (FieldName == TEXT("IconCode"))
		{
			OutFieldValue.StringValue = TEXT("x");
		}
		else if (FieldName == TEXT("UseLimit"))
		{
			OutFieldValue.StringValue = TEXT("ALL");
		}
	}
	else
	{
		ITEM_DESC *pItem = NULL;
		if (_StateController->PlayerInfo.IsValid())
		{
			TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[SlotID];
			if (ItemSN != SN_INVALID_ITEM)
			{
				for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
				{
					if (ItemSN == _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].sn)
					{
						pItem = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].id);
						break;
					}
				}
			}
		}

		if (FieldName == TEXT("SlotName"))
		{
			SLOT_DESC *pSlot = _ItemDesc().GetWeaponSlot(SlotID);
			OutFieldValue.StringValue = (pSlot ? GetWeaponSlotName(pSlot->slotType) : TEXT("??"));
		}
		else if (FieldName == TEXT("ItemID"))
		{
			OutFieldValue.StringValue = (pItem ? appItoa(pItem->id) : TEXT(" "));
		}
		else if (FieldName == TEXT("ItemName"))
		{
			OutFieldValue.StringValue = (pItem ? pItem->GetName() : TEXT(" "));
		}
		else if (FieldName == TEXT("ItemDesc"))
		{
			OutFieldValue.StringValue = (pItem ? pItem->GetDescription() : TEXT(" "));
		}
		else if (FieldName == TEXT("IconCode"))
		{
			OutFieldValue.StringValue = (pItem ? FString::Printf(TEXT("%c"), pItem->GetIcon()) : TEXT("x"));
		}
		else if (FieldName == TEXT("UseLimit"))
		{
			if (pItem)
			{
				//OutFieldValue.StringValue = (pItem->useLimitClass == 0 ? TEXT("") : pItem->useLimitClass == 1 ? TEXT("P") : pItem->useLimitClass == 2 ? TEXT("R") : TEXT("S"));
				//if (pItem->useLimitLevel > 0)
				//	OutFieldValue.StringValue += appItoa(pItem->useLimitLevel);
				//if (OutFieldValue.StringValue == TEXT(""))
				//	OutFieldValue.StringValue = TEXT("ALL");
				BYTE LevelLimit = pItem->useLimitLevel;
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Prefix"), TEXT("avaNet")) + GetLevelIconString(LevelLimit) + Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Postfix"), TEXT("avaNet"));
			}
			else
			{
				OutFieldValue.StringValue = TEXT("?");
			}
		}
	}

	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetPlayerWeapon::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("SlotName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemID")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemDesc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("IconCode")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("UseLimit")));
}

/* ==========================================================================================================
UUIDataProvider_AvaNetFilteredWeapons
========================================================================================================== */
// 보조 함수

template<typename T>
FORCEINLINE INT GetRangeIndexFromArray( TArray<T> RangeArray, T Value )
{
	INT ResultIndex = INDEX_NONE;
	for( INT i = 0 ; i < RangeArray.Num() ; i++ )
	{
		if( RangeArray(i) >= Value )
		{
			ResultIndex = i;
			break;
		}
	}

	return ResultIndex;
}

INT GetWeaponInvenIndexFromSN( PLAYER_ITEM_INFO& PlayerItemInfo,TSN_ITEM& itemSN, ITEM_INFO*& itemInfo, ITEM_DESC*& itemDesc)
{
	itemInfo = NULL;
	itemDesc = NULL;
	INT ResultIndex = INDEX_NONE;
	for( INT i = 0 ; i < MAX_INVENTORY_SIZE ; i++ )
	{
		if( PlayerItemInfo.weaponInven[i].sn == itemSN )
		{
			itemInfo = &PlayerItemInfo.weaponInven[i];
			itemDesc = itemInfo ? _ItemDesc().GetItem(itemInfo->id) : NULL;
			ResultIndex = i;
			break;
		}
	}
	return ResultIndex;
}

INT GetEquipInvenIndexFromSN( PLAYER_ITEM_INFO& PlayerItemInfo,TSN_ITEM& itemSN, ITEM_INFO*& itemInfo, ITEM_DESC*& itemDesc )
{
	itemInfo = NULL;
	itemDesc = NULL;
	INT ResultIndex = INDEX_NONE;
	for( INT i = 0 ; i < MAX_INVENTORY_SIZE ; i++ )
	{
		if( PlayerItemInfo.equipInven[i].sn == itemSN )
		{
			itemInfo = &PlayerItemInfo.equipInven[i];
			itemDesc = itemInfo ? _ItemDesc().GetItem(itemInfo->id) : NULL;
			ResultIndex = i;
			break;
		}
	}
	return ResultIndex;
}

void GetOutElementsFromFilter( EFilteredWeapons_DisplayFilter DisplayFilter, TArray<INT>& OutElements, TArray<FLOAT>& RatioBounds )
{
	ITEM_INFO* itemInfo = NULL;
	ITEM_INFO* oldItemInfo = NULL;
	ITEM_DESC* itemDesc = NULL;
	ITEM_DESC* oldItemDesc = NULL;

	if( DisplayFilter == WEAPONDISPFILTER_NONE)
	{
		// nothing to do
	}
	else if( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_POINTMAN ||
		DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_RIFLEMAN ||
		DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_SNIPER )
	{
		TSN_ITEM itemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[DisplayFilter - WEAPONDISPFILTER_EQUIPPED_PRIWEAP_POINTMAN];
		if( itemSN != SN_INVALID_ITEM )
		{
			INT InvenIndex = GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.PlayerInfo.itemInfo, itemSN, itemInfo, itemDesc );
			INT OldInvenIndex = GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.OldItemInfo, itemSN, oldItemInfo, oldItemDesc );
			//check( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) );
			//check( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			UBOOL bValidCond = ( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) ) &&
				( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			if( bValidCond &&
				itemDesc->gaugeType != _IGT_NONE &&	// 게이지가 내구도/정비도/기간제 일때
				OldInvenIndex != INDEX_NONE )		// 그리고 원래부터 가지고 있던 아이템일때
			{
				// 게이지 단계가 서로 다른 단계에 있을때
				if( GetRangeIndexFromArray( RatioBounds, itemInfo->limit / (FLOAT)ITEM_LIMIT_INITED ) != GetRangeIndexFromArray( RatioBounds, oldItemInfo->limit / (FLOAT)ITEM_LIMIT_INITED )  )
					OutElements.AddUniqueItem( InvenIndex );
			}
		}
	}
	else if( DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_POINTMAN ||
		DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_RIFLEMAN || 
		DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_SNIPER)
	{
		TSN_ITEM itemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[DisplayFilter - WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_POINTMAN];

		if( itemSN != SN_INVALID_ITEM)
		{
			INT InvenIndex = GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.PlayerInfo.itemInfo, itemSN, itemInfo, itemDesc );
			INT OldInvenIndex = GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.OldItemInfo, itemSN, oldItemInfo, oldItemDesc );
			//check( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) );
			//check( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			UBOOL bValidCond = ( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) ) &&
				( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL));

			if( bValidCond && OldInvenIndex != INDEX_NONE )
			{
				CInventory& Inven = _StateController->PlayerInfo.Inven;
				CInventory OldInven;
				OldInven.Init( &_ItemDesc(), &_StateController->PlayerInfo.OldItemInfo );

				for( INT i = 0 ; i < _CSI_MAX ; i++ )
				{
					CUSTOM_ITEM_INFO* cItemInfo = Inven.GetCustomInvenToSlot( itemInfo->sn, (Def::CUSTOM_SLOT_IDX)i);
					CUSTOM_ITEM_INFO* cOldItemInfo = OldInven.GetCustomInvenToSlot( oldItemInfo->sn, (Def::CUSTOM_SLOT_IDX)i );

					CUSTOM_ITEM_DESC* cItemDesc = cItemInfo ? _ItemDesc().GetCustomItem( cItemInfo->id ) : NULL;
					CUSTOM_ITEM_DESC* cOldItemDesc = cOldItemInfo ? _ItemDesc().GetCustomItem( cOldItemInfo->id ) : NULL;

					if( (cItemInfo != NULL && cItemDesc != NULL) && cOldItemInfo != NULL &&
						cItemInfo->id == cOldItemInfo->id && cItemDesc->gaugeType != _IGT_NONE &&	// 같은 종류의 아이템일때
						GetRangeIndexFromArray( RatioBounds, cItemInfo->limit / (FLOAT)ITEM_LIMIT_INITED ) != GetRangeIndexFromArray(RatioBounds, cOldItemInfo->limit / (FLOAT)ITEM_LIMIT_INITED))
						// 게이지 단계가 서로 다를경우에만
					{
						// 커스텀 파츠의 경우에만 예외적으로 슬롯인덱스를 넣어준다.
						OutElements.AddItem(i);
					}
				}
			}			
		}
	}
	else if ( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_WEAP_EXCEPT_PRIWEAP )
	{
		TArray<TSN_ITEM> ItemSNStored;
		for( INT i = 3 ; i < MAX_WEAPONSET_SIZE ; i++)
		{
			TSN_ITEM itemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[i];
			if( itemSN == SN_INVALID_ITEM )
				continue;

			INT InvenIndex = GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.PlayerInfo.itemInfo, itemSN, itemInfo, itemDesc );
			INT OldInvenIndex = GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.OldItemInfo, itemSN, oldItemInfo, oldItemDesc );
			//check( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) );
			//check( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			UBOOL bValidCond = ( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) ) &&
				( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			if( bValidCond &&
				OldInvenIndex != INDEX_NONE && 
				!(itemDesc->slotType & _EP_WEAP_PRIMARY) &&
				itemDesc->gaugeType != _IGT_NONE && 
				GetRangeIndexFromArray(RatioBounds, itemInfo->limit / (FLOAT)ITEM_LIMIT_INITED) != GetRangeIndexFromArray(RatioBounds, oldItemInfo->limit / (FLOAT)ITEM_LIMIT_INITED) )
			{
				if( ItemSNStored.FindItemIndex( itemSN ) == INDEX_NONE)
				{
					OutElements.AddItem( InvenIndex );
					ItemSNStored.AddItem(itemSN);
				}
			}
		}
	}
	else if ( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_EQUIPS )
	{
		TArray<TSN_ITEM> ItemSNStored;
		for( INT i = 0 ; i < MAX_EQUIPSET_SIZE ; i++ )
		{
			TSN_ITEM itemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[i];
			if( itemSN == SN_INVALID_ITEM )
				continue;

			INT InvenIndex = GetEquipInvenIndexFromSN( _StateController->PlayerInfo.PlayerInfo.itemInfo, itemSN, itemInfo, itemDesc );
			INT OldInvenIndex = GetEquipInvenIndexFromSN( _StateController->PlayerInfo.OldItemInfo, itemSN, oldItemInfo, oldItemDesc );
			//check( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) );
			//check( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			UBOOL bValidCond = ( InvenIndex != INDEX_NONE && (itemInfo != NULL && itemDesc != NULL) ) &&
				( OldInvenIndex == INDEX_NONE || (oldItemInfo != NULL && oldItemDesc != NULL) );

			if( bValidCond &&
				OldInvenIndex != INDEX_NONE &&
				itemDesc->gaugeType != _IGT_NONE && 
				GetRangeIndexFromArray( RatioBounds, itemInfo->limit / (FLOAT) ITEM_LIMIT_INITED ) != GetRangeIndexFromArray( RatioBounds, oldItemInfo->limit / (FLOAT)ITEM_LIMIT_INITED ) )				
			{
				if( ItemSNStored.FindItemIndex( itemSN ) == INDEX_NONE )
				{
					ItemSNStored.AddItem(itemSN);
					OutElements	.AddItem(InvenIndex);
				}
			}
		}
	}
}


IMPLEMENT_CLASS(UUIDataProvider_AvaNetFilteredWeapons);


void UUIDataProvider_AvaNetFilteredWeapons::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("CustomSlotImage")),TEXT("CustomSlotImage"));
	OutCellTags.Set(FName(TEXT("WeaponName")),TEXT("WeaponName"));
	OutCellTags.Set(FName(TEXT("WeaponIcon")),TEXT("WeaponIcon"));
	OutCellTags.Set(FName(TEXT("GaugeIcon")),TEXT("GaugeIcon"));
	OutCellTags.Set(FName(TEXT("Maintenance")),TEXT("Maintenance"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetFilteredWeapons::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if( CellTag == FName(TEXT("CustomSlotImage")) )
		{
			OutFieldValue.StringValue = TEXT("SlotImg");
		}
		else if( CellTag == FName(TEXT("WeaponName")) )
		{
			OutFieldValue.StringValue = TEXT("WeapName");
		}
		else if ( CellTag == FName(TEXT("WeaponIcon")) )
		{
			OutFieldValue.StringValue = GetIconCodeString(TEXT("a"),TEXT("Icon_Common_WeaponIconFont"));
		}
		else if ( CellTag == FName(TEXT("GaugeIcon")) )
		{
			OutFieldValue.StringValue = GetItemGaugeIconString( _IGT_MAINTENANCE, ITEM_LIMIT_INITED / 2 );
		}
		else if ( CellTag == FName(TEXT("Maintenance")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_WeaponStatusMsg[Normal]"), TEXT("avaNet"));
		}
	}
	else
	{
		CUSTOM_ITEM_INFO* cItemInfo = NULL;
		ITEM_INFO* itemInfo = NULL;

		if( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_POINTMAN ||
			DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_RIFLEMAN ||
			DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_SNIPER)
		{
			itemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ListIndex];
		}
		else if( DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_POINTMAN ||
			DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_RIFLEMAN ||
			DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_SNIPER)
		{
			CInventory& Inven = _StateController->PlayerInfo.Inven;
			TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[DisplayFilter - WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_POINTMAN];
			ITEM_INFO* WeaponItemInfo = NULL;
			ITEM_DESC* WeaponItemDesc = NULL;
			GetWeaponInvenIndexFromSN( _StateController->PlayerInfo.PlayerInfo.itemInfo, ItemSN, WeaponItemInfo, WeaponItemDesc );
			cItemInfo = WeaponItemInfo ? Inven.GetCustomInvenToSlot( WeaponItemInfo->sn, (Def::CUSTOM_SLOT_IDX)ListIndex ) : NULL;
		}
		else if ( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_WEAP_EXCEPT_PRIWEAP )
		{
			itemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ListIndex];
		}
		else if ( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_EQUIPS )
		{
			itemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex];
		}

		CUSTOM_ITEM_DESC* cItemDesc = cItemInfo ? _ItemDesc().GetCustomItem( cItemInfo->id ) : NULL;
		ITEM_DESC* itemDesc = itemInfo ? _ItemDesc().GetItem( itemInfo->id ) : NULL;

		if( CellTag == FName(TEXT("CustomSlotImage")) )
		{
			if( cItemInfo && cItemDesc )
			{
				if( cItemDesc->customType == _CSI_FRONT )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Front"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_MOUNT )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Mount"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_BARREL )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Barrel"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_TRIGGER )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Trigger"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_GRIP)					
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Grip"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_STOCK )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Stock"),TEXT("avaNet"));
				else
					OutFieldValue.StringValue = FString(TEXT("Invalid Slot"));

			}
			else
			{
				OutFieldValue.StringValue = TEXT("Inv. cItemDesc");
			}
		}
		else if( CellTag == FName(TEXT("WeaponName")) )
		{
			OutFieldValue.StringValue = itemDesc ? itemDesc->GetName() : 
				cItemDesc ? cItemDesc->GetName() : TEXT(" ");
		}
		else if ( CellTag == FName(TEXT("WeaponIcon")) )
		{
			OutFieldValue.StringValue = GetIconCodeString( itemDesc, cItemDesc);
		}
		else if ( CellTag == FName(TEXT("GaugeIcon")) )
		{
			BYTE GaugeType = itemDesc ? itemDesc->gaugeType : cItemDesc ? cItemDesc->gaugeType : NULL;
			INT Limit = itemDesc ? itemInfo->limit : cItemInfo ? cItemInfo->limit : NULL;
			OutFieldValue.StringValue = GetItemGaugeIconString( GaugeType, Limit );
		}
		else if ( CellTag == FName(TEXT("Maintenance")) )
		{
			if( (itemDesc && itemInfo) && itemDesc->slotType & _EP_WEAP_PRIMARY )
			{
				FLOAT Ratio = itemInfo->limit / (FLOAT)ITEM_LIMIT_INITED;
				if ( Ratio <= 0.f )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_WeaponStatusMsg[OutOfOrder]"), TEXT("avaNet"));
				else if( Ratio < 0.2f )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_WeaponStatusMsg[Bad]"), TEXT("avaNet"));
				else if ( Ratio < 0.8f )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_WeaponStatusMsg[Normal]"), TEXT("avaNet"));
				else
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_WeaponStatusMsg[Good]"), TEXT("avaNet"));
			}
			else
			{
				OutFieldValue.StringValue = TEXT(" ");
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("N/A");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetFilteredWeapons::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/* ==========================================================================================================
UUIDataProvider_AvaNetPlayerEquip
========================================================================================================== */


IMPLEMENT_CLASS(UUIDataProvider_AvaNetPlayerEquip);



/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetPlayerEquip::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if (GIsEditor && !GIsGame)
	{
		if (FieldName == TEXT("SlotName"))
		{
			SLOT_DESC *pSlot = _ItemDesc().GetEquipSlot(SlotID);
			OutFieldValue.StringValue = (pSlot ? GetEquipSlotName(pSlot->slotType) : TEXT("??"));
		}
		else if (FieldName == TEXT("ItemID"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("10%02d"), SlotID);
		}
		else if (FieldName == TEXT("ItemName"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Item10%02d"), SlotID);
		}
		else if (FieldName == TEXT("ItemDesc"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Item10%02d description"), SlotID);
		}
		else if (FieldName == TEXT("IconCode"))
		{
			OutFieldValue.StringValue = TEXT("x");
		}
		else if (FieldName == TEXT("UseLimit"))
		{
			OutFieldValue.StringValue = TEXT("ALL");
		}
	}
	else
	{
		ITEM_DESC *pItem = NULL;
		if (_StateController->PlayerInfo.IsValid())
		{
			TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[SlotID];
			if (ItemSN != SN_INVALID_ITEM)
			{
				for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
				{
					if (ItemSN == _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[i].sn)
					{
						pItem = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[i].id);
						break;
					}
				}
			}
		}

		if (FieldName == TEXT("SlotName"))
		{
			SLOT_DESC *pSlot = _ItemDesc().GetEquipSlot(SlotID);
			OutFieldValue.StringValue = (pSlot ? GetEquipSlotName(pSlot->slotType) : TEXT("??"));
		}
		else if (FieldName == TEXT("ItemID"))
		{
			OutFieldValue.StringValue = (pItem ? appItoa(pItem->id) : TEXT(" "));
		}
		else if (FieldName == TEXT("ItemName"))
		{
			OutFieldValue.StringValue = (pItem ? pItem->GetName() : TEXT(" "));
		}
		else if (FieldName == TEXT("ItemDesc"))
		{
			OutFieldValue.StringValue = (pItem ? pItem->GetDescription() : TEXT(" "));
		}
		else if (FieldName == TEXT("IconCode"))
		{
			OutFieldValue.StringValue = (pItem ? FString::Printf(TEXT("%c"), pItem->GetIcon()) : TEXT("a"));
		}
		else if (FieldName == TEXT("UseLimit"))
		{
			if (pItem)
			{
				//OutFieldValue.StringValue = (pItem->useLimitClass == 0 ? TEXT("") : pItem->useLimitClass == 1 ? TEXT("P") : pItem->useLimitClass == 2 ? TEXT("R") : TEXT("S"));
				//if (pItem->useLimitLevel > 0)
				//	OutFieldValue.StringValue += appItoa(pItem->useLimitLevel);
				//if (OutFieldValue.StringValue == TEXT(""))
				//	OutFieldValue.StringValue = TEXT("ALL");
				BYTE LevelLimit = pItem->useLimitLevel;
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Prefix"), TEXT("avaNet")) + GetLevelIconString(LevelLimit) + Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Postfix"), TEXT("avaNet"));
			}
			else
			{
				OutFieldValue.StringValue = TEXT("?");
			}
		}
	}

	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetPlayerEquip::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("SlotName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemID")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemDesc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("IconCode")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("UseLimit")));
}


/* ==========================================================================================================
UUIDataProvider_AvaNetPlayerCustoms
========================================================================================================== */


IMPLEMENT_CLASS(UUIDataProvider_AvaNetPlayerCustoms);



/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetPlayerCustoms::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if (FieldName.Len() <= 2)
		return FALSE;
	if ( GetInvenIndexFromID(InvenID) >= 10000 )
		return FALSE;

	CUSTOM_SLOT_IDX CustomSlot = _CSI_NONE;
	FString FN = FieldName.Right(FieldName.Len() - 2);

	if (appStrncmp(*FieldName, TEXT("F_"), 2) == 0)
	{
		CustomSlot = _CSI_FRONT;
	}
	else if (appStrncmp(*FieldName, TEXT("M_"), 2) == 0)
	{
		CustomSlot = _CSI_MOUNT;
	}
	else if (appStrncmp(*FieldName, TEXT("B_"), 2) == 0)
	{
		CustomSlot = _CSI_BARREL;
	}
	else if (appStrncmp(*FieldName, TEXT("T_"), 2) == 0)
	{
		CustomSlot = _CSI_TRIGGER;
	}
	else if (appStrncmp(*FieldName, TEXT("G_"), 2) == 0)
	{
		CustomSlot = _CSI_GRIP;
	}
	else if (appStrncmp(*FieldName, TEXT("S_"), 2) == 0)
	{
		CustomSlot = _CSI_STOCK;
	}

	if (CustomSlot == _CSI_NONE)
		return FALSE;

	if (GIsEditor && !GIsGame)
	{

		if (FN == TEXT("ItemID"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("10%02d"), GetInvenIndexFromID(InvenID));
		}
		else if (FN == TEXT("ItemName"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Custom10%02d"), GetInvenIndexFromID(InvenID));
		}
		else if (FN == TEXT("ItemDesc"))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Custom10%02d description"), GetInvenIndexFromID(InvenID));
		}
		else if (FN == TEXT("IconCode"))
		{
			OutFieldValue.StringValue = TEXT("x");
		}
		else if (FN == TEXT("Durability"))
		{
			//OutFieldValue.StringValue = appItoa(ITEM_LIMIT_INITED);
			OutFieldValue.StringValue = GetItemGaugeIconString( _IGT_DURABILITY, ITEM_LIMIT_INITED / 2 );
		}
	}
	else
	{
		CUSTOM_ITEM_DESC *pDesc = NULL;
		CUSTOM_ITEM_INFO *pItem = NULL;
		if (_StateController->PlayerInfo.IsValid())
		{
			CInventory Inven;
			Inven.Init(&_ItemDesc(), &_StateController->PlayerInfo.PlayerInfo.itemInfo);

			ITEM_INFO &item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(InvenID)];
			pItem = Inven.GetCustomInvenToSlot(item.sn, CustomSlot);
			if ( pItem != NULL && !pItem->IsEmpty())
				pDesc = _ItemDesc().GetCustomItem(pItem->id);
		}

		if (FN == TEXT("ItemID"))
		{
			OutFieldValue.StringValue = (pDesc ? appItoa(pDesc->id) : TEXT(" "));
		}
		else if (FN == TEXT("ItemName"))
		{
			OutFieldValue.StringValue = (pDesc ? pDesc->GetName() : TEXT(" "));
		}
		else if (FN == TEXT("ItemDesc"))
		{
			OutFieldValue.StringValue = (pDesc ? pDesc->GetDescription() : TEXT(" "));
		}
		else if (FN == TEXT("IconCode"))
		{
			OutFieldValue.StringValue = (pDesc ? FString::Printf(TEXT("%c"), pDesc->GetIcon()) : TEXT("a"));
		}
		else if (FN == TEXT("Durability"))
		{
			OutFieldValue.StringValue = (pItem && pDesc ? GetItemGaugeIconString( pDesc->gaugeType, pItem->limit ) : TEXT(" "));
		}
	}

	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetPlayerCustoms::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemID")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemDesc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("IconCode")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Durability")));
}


/* ==========================================================================================================
UUIDataProvider_AvaNetInventory
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetInventory);


void UUIDataProvider_AvaNetInventory::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("SlotID")),TEXT("SlotID"));
	OutCellTags.Set(FName(TEXT("ItemID")),TEXT("ItemID"));
	OutCellTags.Set(FName(TEXT("ItemName")),TEXT("ItemName"));
	OutCellTags.Set(FName(TEXT("ItemNameTag")),TEXT("ItemNameTag"));
	OutCellTags.Set(FName(TEXT("ItemDesc")),TEXT("ItemDesc"));
	OutCellTags.Set(FName(TEXT("Durability")),TEXT("Durability"));
	OutCellTags.Set(FName(TEXT("UseLimit")),TEXT("UseLimit"));
	OutCellTags.Set(FName(TEXT("IconCode")),TEXT("IconCode"));
	OutCellTags.Set(FName(TEXT("EquipFlag")),TEXT("EquipFlag"));			// 키즈멧 내부에서 사용
	OutCellTags.Set(FName(TEXT("EquipStatus")),TEXT("EquipStatus"));		// 화면표시용
	OutCellTags.Set(FName(TEXT("AvailSlotImage")),TEXT("AvailSlotImage"));
	OutCellTags.Set(FName(TEXT("ItemSN")),TEXT("ItemSN"));
	OutCellTags.Set(FName(TEXT("Tag")),TEXT("Tag"));
	OutCellTags.Set(FName(TEXT("NoLimitIcon")),TEXT("NoLimitIcon"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetInventory::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("SlotID")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("%02d"), GetInvenIndexFromID(ListIndex));
			}
			else if (CellTag == FName(TEXT("ItemID")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("10%02d"), GetInvenIndexFromID(ListIndex));
			}
			else if (CellTag == FName(TEXT("ItemName")))
			{
				if (ListIndex >= 10000)
					OutFieldValue.StringValue = FString::Printf(TEXT("Equip%02d"), GetInvenIndexFromID(ListIndex));
				else
					OutFieldValue.StringValue = FString::Printf(TEXT("Weapon%02d"), GetInvenIndexFromID(ListIndex));
			}
			else if (CellTag == FName(TEXT("ItemNameTag")))
			{
				if (ListIndex >= 10000)
					OutFieldValue.StringValue = FString::Printf(TEXT("Equip%02d(0)"), GetInvenIndexFromID(ListIndex));
				else
					OutFieldValue.StringValue = FString::Printf(TEXT("Weapon%02d(0)"), GetInvenIndexFromID(ListIndex));
			}
			else if (CellTag == FName(TEXT("ItemDesc")))
			{
				if (ListIndex >= 10000)
					OutFieldValue.StringValue = FString::Printf(TEXT("Equip%02d description"), GetInvenIndexFromID(ListIndex));
				else
					OutFieldValue.StringValue = FString::Printf(TEXT("Weapon%02d description"), GetInvenIndexFromID(ListIndex));
			}
			else if (CellTag == FName(TEXT("Durability")))
			{
				OutFieldValue.StringValue = GetItemGaugeIconString( _IGT_DURABILITY, ITEM_LIMIT_INITED / 2 );
			}
			else if (CellTag == FName(TEXT("IconCode")))
			{
				OutFieldValue.StringValue = GetIconCodeString(TEXT("a"),TEXT("Icon_Common_WeaponIconFont"));
			}
			else if (CellTag == FName(TEXT("UseLimit")))
			{
				OutFieldValue.StringValue = TEXT("ALL");
			}
			else if (CellTag == FName(TEXT("EquipFlag")))
			{
				OutFieldValue.StringValue = TEXT("E");
			}
			else if (CellTag == FName(TEXT("EquipStatus")))
			{
				FString EquipIconStr = Localize(TEXT("UIInventoryScene"), TEXT("Icon_EquipFlag_Item"),TEXT("avaNet"));
				OutFieldValue.StringValue = EquipIconStr.Len() > 0 ? EquipIconStr : Localize(TEXT("UIInventoryScene"), TEXT("Text_EquipFlag_Item"),TEXT("avaNet"));
			}
			else if (CellTag == FName(TEXT("AvailSlotImage")))
			{
				CUSTOM_SLOT_TYPE AvailCustomSlots;
				OutFieldValue.StringValue = GetSlotImageString( AvailCustomSlots);
			}
			else if (CellTag == FName(TEXT("ItemSN")))
			{
				OutFieldValue.StringValue = TEXT("SN");
			}
			else if (CellTag == FName(TEXT("Tag")))
			{
				OutFieldValue.StringValue = TEXT("0");
			}
			else if( CellTag == TEXT("NoLimitIcon") )
			{
				OutFieldValue.StringValue = GetNoLimitIconString(_IGT_MAINTENANCE);
			}
		}
	}
	else
	{


		if (ListIndex >= 0 && _StateController->PlayerInfo.IsValid())
		{
			// 필요한 정보 초기화 (ITEM_DESC, ITEM_INFO)
			ITEM_INFO& rItemInfo = ListIndex >= 10000 ? _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex - 10000] : 
				_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ListIndex % 100];

			ITEM_DESC *pItem = NULL;
			if (ListIndex >= 10000)
				pItem = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex-10000].id);
			else
				pItem = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(ListIndex)].id);


			// 각 셀태그별 Text, Image를 걸어줌
			if (CellTag == FName(TEXT("SlotID")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("%02d"), ListIndex >= 10000 ? ListIndex - 10000 : ListIndex % 100);
			}
			else if (CellTag == FName(TEXT("ItemID")))
			{
				//if (ListIndex >= 10000)
				//	OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex-10000].id);
				//else
				//	OutFieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ListIndex % 100].id);
				OutFieldValue.StringValue = appItoa(rItemInfo.id);
			}
			else if (CellTag == FName(TEXT("ItemName")) || CellTag == FName(TEXT("ItemNameTag")))
			{
				OutFieldValue.StringValue = (pItem ? pItem->GetName() : TEXT("????"));
				if( CellTag == FName(TEXT("ItemNameTag")) )
					OutFieldValue.StringValue += rItemInfo.tag > 0 ? appItoa(rItemInfo.tag) : TEXT("") ;
			}
			else if (CellTag == FName(TEXT("ItemDesc")))
			{
				OutFieldValue.StringValue = (pItem ? pItem->GetDescription() : TEXT("????"));
			}
			else if (CellTag == FName(TEXT("Durability")))
			{
				if( pItem && 0 <= rItemInfo.limit )//&& rItemInfo.limit <= ITEM_LIMIT_INITED)
				{
					OutFieldValue.StringValue = GetItemGaugeIconString( pItem->gaugeType, rItemInfo.limit );
				}
				else
				{
					OutFieldValue.StringValue = FString(TEXT("Iteminfo?")) ;
				}
			}
			else if (CellTag == FName(TEXT("IconCode")))
			{
				//				FString WeaponIconCode = (pItem ? FString::Printf(TEXT("%c"), pItem->GetIcon()) : TEXT("a"));
				OutFieldValue.StringValue = GetIconCodeString( pItem );
			}
			else if (CellTag == FName(TEXT("UseLimit")))
			{
				//ITEM_DESC *pItem = NULL;
				//if (ListIndex >= 10000)
				//	pItem = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex-10000].id);
				//else
				//	pItem = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[ListIndex].id);

				if (pItem)
				{
					//OutFieldValue.StringValue = (pItem->useLimitClass == 0 ? TEXT("") : pItem->useLimitClass == 1 ? TEXT("P") : pItem->useLimitClass == 2 ? TEXT("R") : TEXT("S"));
					//if (pItem->useLimitLevel > 0)
					//	OutFieldValue.StringValue += appItoa(pItem->useLimitLevel);
					//if (OutFieldValue.StringValue == TEXT(""))
					//	OutFieldValue.StringValue = TEXT("ALL");
					BYTE LevelLimit = pItem->useLimitLevel;
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Prefix"), TEXT("avaNet")) + GetLevelIconString(LevelLimit) + Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Postfix"), TEXT("avaNet"));
				}
				else
				{
					OutFieldValue.StringValue = TEXT("?");
				}
			}
			else if (CellTag == FName(TEXT("EquipStatus")) || CellTag == FName(TEXT("EquipFlag")))
			{
				OutFieldValue.StringValue = TEXT(" ");
				UBOOL bIsWeapon = ListIndex < 10000;
				UBOOL bIsFlag = CellTag == FName(TEXT("EquipFlag"));

				if ( ! bIsWeapon )
				{
					TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[ListIndex-10000].sn;
					for (INT i = 0; i < MAX_EQUIPSET_SIZE; ++i)
					{
						if (_StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[i] == ItemSN)
						{
							if( bIsFlag )
								OutFieldValue.StringValue = TEXT("E");
							else
							{
								FString EquipIconStr = Localize(TEXT("UIInventoryScene"), TEXT("Icon_EquipFlag_Item"),TEXT("avaNet"));
								OutFieldValue.StringValue = EquipIconStr.Len() > 0 ? EquipIconStr : Localize(TEXT("UIInventoryScene"), TEXT("Text_EquipFlag_Item"),TEXT("avaNet"));
							}
							break;
						}
					}
				}
				else
				{
					TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(ListIndex)].sn;
					INT EquipSlot = ListIndex / 100;

					if (EquipSlot >= 0 && EquipSlot < MAX_WEAPONSET_SIZE &&
						_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[EquipSlot] == ItemSN)
					{
						if( bIsFlag )
							OutFieldValue.StringValue = TEXT("E");
						else
						{
							FString EquipIconStr = Localize(TEXT("UIInventoryScene"), TEXT("Text_EquipFlag_Weapon"), TEXT("avaNet"));
							OutFieldValue.StringValue = EquipIconStr.Len() > 0 ? EquipIconStr : Localize(TEXT("UIInventoryScene"), TEXT("Text_EquipFlag_Weapon"), TEXT("avaNet"));
						}
					}
				}
			}
			else if (CellTag == FName(TEXT("AvailSlotImage")))
			{	
				if( pItem )
				{
					OutFieldValue.StringValue = (pItem->slotType & _EP_WEAP_PRIMARY) ? GetSlotImageString( pItem->customType, &rItemInfo ) : TEXT(" ");
				}
				else
					OutFieldValue.StringValue = TEXT("Invalid ItemDesc?");
			}
			else if ( CellTag == FName(TEXT("ItemSN")) )
			{
				// Item Serial Number를 얻어오는 부분 추가.(2007/02/27 고광록)
				OutFieldValue.StringValue = FString::Printf(TEXT("%I64u"), rItemInfo.sn);
			}
			else if (CellTag == FName(TEXT("Tag")))
			{
				OutFieldValue.StringValue = rItemInfo.tag > 0 ? appItoa(rItemInfo.tag) : TEXT("");
			}
			else if( CellTag == TEXT("NoLimitIcon") )
			{
				OutFieldValue.StringValue = pItem ? GetNoLimitIconString(pItem->gaugeType) : TEXT(" ");
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetInventory::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}


/* ==========================================================================================================
UUIDataProvider_AvaNetShopItems
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetShopItems);


void UUIDataProvider_AvaNetShopItems::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("ItemID")),TEXT("ItemID"));
	OutCellTags.Set(FName(TEXT("ItemName")),TEXT("ItemName"));
	OutCellTags.Set(FName(TEXT("ItemDesc")),TEXT("ItemDesc"));
	OutCellTags.Set(FName(TEXT("UseLimit")),TEXT("UseLimit"));
	OutCellTags.Set(FName(TEXT("IconCode")),TEXT("IconCode"));
	OutCellTags.Set(FName(TEXT("AvailSlotImages")), TEXT("AvailSlotImages"));
	OutCellTags.Set(FName(TEXT("Price")),TEXT("Price"));
	OutCellTags.Set(FName(TEXT("NoLimitIcon")),TEXT("NoLimitIcon"));
	OutCellTags.Set(FName(TEXT("Recommendation")),TEXT("Recommendation"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetShopItems::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("ItemID")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("10%02d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("ItemName")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("Item%02d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("ItemDesc")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("Item%02d description"), ListIndex);
			}
			else if (CellTag == FName(TEXT("UseLimit")))
			{
				OutFieldValue.StringValue = TEXT("ALL");
			}
			else if (CellTag == FName(TEXT("IconCode")))
			{
				OutFieldValue.StringValue = GetIconCodeString(TEXT("a"), TEXT("Icon_Common_WeaponIconFont"));
			}
			else if (CellTag == FName(TEXT("AvailSlotImages")))
			{
				CUSTOM_SLOT_TYPE AvailSlots;
				OutFieldValue.StringValue = GetSlotImageString(AvailSlots);
			}
			else if (CellTag == FName(TEXT("Price")))
			{
				OutFieldValue.StringValue = TEXT("$1000");
			}
			else if ( CellTag == FName(TEXT("NoLimitIcon")) )
			{
				OutFieldValue.StringValue = GetNoLimitIconString(_IGT_MAINTENANCE);
			}
			else if ( CellTag == FName(TEXT("Recommendation")) )
			{
				OutFieldValue.StringValue = ListIndex % 2 == 0 ? Localize( TEXT("UIGeneral"), TEXT("Icon_Recommendation"), TEXT("avaNet")) : TEXT(" ");
			}
		}
	}
	else
	{
		if (ListIndex >= 0)
		{
			FavaShopItem *pItem = _ShopDesc().GetItemByIndex(ListIndex);
			ITEM_DESC *pItemDesc = pItem ? _ItemDesc().GetItem(pItem->GetDefaultItemID()) : NULL;

			check(pItemDesc);

			if (CellTag == FName(TEXT("ItemID")))
			{
				OutFieldValue.StringValue = (pItemDesc ? appItoa(pItemDesc->id) : TEXT("??"));
			}
			else if (CellTag == FName(TEXT("ItemName")))
			{
				OutFieldValue.StringValue = (pItemDesc ? pItemDesc->GetName() : TEXT("????"));
			}
			else if (CellTag == FName(TEXT("ItemName")))
			{
				OutFieldValue.StringValue = (pItemDesc ? pItemDesc->GetDescription() : TEXT("????"));
			}
			else if (CellTag == FName(TEXT("ItemDesc")))
			{
				OutFieldValue.StringValue = (pItemDesc ? pItemDesc->GetDescription() : TEXT("????"));
			}
			else if (CellTag == FName(TEXT("UseLimit")))
			{
				if (pItemDesc)
				{
					//OutFieldValue.StringValue = (pItemDesc->useLimitClass == 0 ? TEXT("") : pItemDesc->useLimitClass == 1 ? TEXT("P") : pItemDesc->useLimitClass == 2 ? TEXT("R") : TEXT("S"));
					//if (pItemDesc->useLimitLevel > 0)
					//	OutFieldValue.StringValue += appItoa(pItemDesc->useLimitLevel);
					//if (OutFieldValue.StringValue == TEXT(""))
					//	OutFieldValue.StringValue = TEXT("ALL");
					BYTE LevelLimit = pItemDesc->useLimitLevel;
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Prefix"), TEXT("avaNet")) + GetLevelIconString(LevelLimit) + Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Postfix"), TEXT("avaNet"));
				}
				else
				{
					OutFieldValue.StringValue = TEXT("?");
				}
			}
			else if (CellTag == FName(TEXT("IconCode")))
			{
				//UBOOL bEquip = pItemDesc ? (((ITEM_ID*)(&pItemDesc->id))->category >= _IC_HELMET && ((ITEM_ID*)(&pItemDesc->id))->category <= _IC_LEG) : FALSE;
				//FString ItemIconCode = (pItemDesc ? FString::Printf(TEXT("%c"), pItemDesc->GetIcon()) : TEXT("a"));
				OutFieldValue.StringValue =  GetIconCodeString( pItemDesc );
			}
			else if (CellTag == FName(TEXT("AvailSlotImages")))
			{

				if( pItemDesc )
					OutFieldValue.StringValue = pItemDesc->slotType & _EP_WEAP_PRIMARY ? GetSlotImageString( pItemDesc->customType ) : TEXT(" ");
				else
					OutFieldValue.StringValue = TEXT("no ItemDesc");
			}
			else if (CellTag == FName(TEXT("Price")))
			{
				//pItemDesc->priceType
				//FString MoneyMarkStr = Localize(TEXT("UIGeneral"), TEXT("Text_MoneyMark"), TEXT("avaNet"));
				//OutFieldValue.StringValue = FString::Printf(TEXT("%s%d"), MoneyMarkStr.Len() > 0 ? *MoneyMarkStr : TEXT("$") ,pItemDesc ? pItemDesc->buyMoneyPrice : 0);
				OutFieldValue.StringValue = Localize(TEXT("UIGeneral"), TEXT("Text_MoneyMark"), TEXT("avaNet")) + GetFmtMoneyString(pItemDesc ? pItemDesc->price : 0);
			}
			else if ( CellTag == FName(TEXT("NoLimitIcon")) )
			{
				OutFieldValue.StringValue = GetNoLimitIconString(pItemDesc->gaugeType);
			}
			else if ( CellTag == FName(TEXT("Recommendation")) )
			{
				OutFieldValue.StringValue = pItemDesc->GetGraphValue(7) ? Localize( TEXT("UIGeneral"), TEXT("Icon_Recommendation"), TEXT("avaNet")) : TEXT(" ");
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetShopItems::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}


/* ==========================================================================================================
UUIDataProvider_AvaNetShopCustomItems
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetShopCustomItems);


void UUIDataProvider_AvaNetShopCustomItems::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	//BYTE		useLimitLevel;		// 계급별 사용제한
	//BYTE		gaugeType;			// 1 = 정비도 2 = 내구도 3 = 날짜제한.
	//BYTE		priceType;			// 이 아이탬이 케쉬 아이탬인지..머니 아이탬인지.. 0 이면 캐쉬.1이면 머니		
	OutCellTags.Set(FName(TEXT("ItemID")),TEXT("ItemID"));
	OutCellTags.Set(FName(TEXT("SlotImage")),TEXT("SlotImage"));
	OutCellTags.Set(FName(TEXT("ItemName")),TEXT("ItemName"));
	OutCellTags.Set(FName(TEXT("ItemDesc")),TEXT("ItemDesc"));
	OutCellTags.Set(FName(TEXT("IconCode")),TEXT("IconCode"));
	OutCellTags.Set(FName(TEXT("Durability")),TEXT("Durability"));
	OutCellTags.Set(FName(TEXT("UseLimit")),TEXT("UseLimit"));
	OutCellTags.Set(FName(TEXT("Price")),TEXT("Price"));
	OutCellTags.Set(FName(TEXT("NoLimitIcon")),TEXT("NoLimitIcon"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetShopCustomItems::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("ItemID")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("10%02d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("SlotImage")))
			{
				CUSTOM_SLOT_TYPE AvailCustomSlots;
				OutFieldValue.StringValue = GetSlotImageString( AvailCustomSlots );
			}
			else if (CellTag == FName(TEXT("ItemName")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("Item%02d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("ItemDesc")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("Item%02d description"), ListIndex);
			}
			else if (CellTag == FName(TEXT("IconCode")))
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("x"), ListIndex);
			}
			else if (CellTag == FName(TEXT("Durability")))
			{
				OutFieldValue.StringValue = GetItemGaugeIconString( _IGT_DURABILITY, ITEM_LIMIT_INITED / 2 );
			}
			else if (CellTag == FName(TEXT("UseLimit")))
			{
				OutFieldValue.StringValue = TEXT("ALL");
			}
			else if (CellTag == FName(TEXT("Price")))
			{
				OutFieldValue.StringValue = TEXT("$1000");
			}
			else if (CellTag == FName(TEXT("NoLimitIcon")))
			{
				OutFieldValue.StringValue = GetNoLimitIconString(_IGT_MAINTENANCE);
			}
		}
	}
	else
	{
		if (ListIndex >= 0)
		{
			FavaShopItem *pItem = _ShopDesc().GetCustomItemByIndex(ListIndex);
			CUSTOM_ITEM_DESC *pItemDesc = (pItem != NULL) ? _ItemDesc().GetCustomItem(pItem->GetDefaultItemID()) : NULL;
			CUSTOM_ITEM_INFO *cItemInfo = NULL;
			if (_StateController != NULL && pItemDesc != NULL)
			{
				cItemInfo = _StateController->PlayerInfo.Inven.GetCustomInven( *((INT64*)&ItemSN), pItemDesc->id);
			}

			FString ImagePathName(TEXT("avaUISkinEx.ava_CHANNEL_Image"));

			if (CellTag == FName(TEXT("ItemID")))
			{
				OutFieldValue.StringValue = (pItemDesc ? appItoa(pItemDesc->id) : TEXT("ID?"));
			}
			else if (CellTag == FName(TEXT("SlotImage")))
			{
				if( pItemDesc != NULL)
				{
					if( pItemDesc->customType == _CSI_FRONT )
						OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Front"),TEXT("avaNet"));
					else if ( pItemDesc->customType == _CSI_MOUNT )
						OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Mount"),TEXT("avaNet"));
					else if ( pItemDesc->customType == _CSI_BARREL )
						OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Barrel"),TEXT("avaNet"));
					else if ( pItemDesc->customType == _CSI_TRIGGER )
						OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Trigger"),TEXT("avaNet"));
					else if ( pItemDesc->customType == _CSI_GRIP)					
						OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Grip"),TEXT("avaNet"));
					else if ( pItemDesc->customType == _CSI_STOCK )
						OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Stock"),TEXT("avaNet"));
					else
						OutFieldValue.StringValue = FString(TEXT("Invalid Slot"));
				}
				else
					OutFieldValue.StringValue = FString(TEXT("ItemInfo?"));
			}
			else if (CellTag == FName(TEXT("ItemName")))
			{
				OutFieldValue.StringValue = (pItemDesc ? pItemDesc->GetName() : TEXT("NAME?"));
			}
			else if (CellTag == FName(TEXT("ItemDesc")))
			{
				OutFieldValue.StringValue = (pItemDesc ? pItemDesc->GetDescription() : TEXT("DESC?"));
			}
			else if (CellTag == FName(TEXT("IconCode")))
			{
				//FString CustomIconCode = (pItemDesc ? FString::Printf(TEXT("%c"), pItemDesc->GetIcon()) : TEXT("a"));
				OutFieldValue.StringValue = GetIconCodeString( NULL, pItemDesc );
			}
			else if (CellTag == FName(TEXT("Durability")))
			{
				if( cItemInfo && pItemDesc )
				{
					OutFieldValue.StringValue =  GetItemGaugeIconString( pItemDesc->gaugeType, cItemInfo->limit );
				}
				else
				{
					OutFieldValue.StringValue = FString(TEXT("CustomIteminfo?")) ;
				}
			}
			else if (CellTag == FName(TEXT("UseLimit")))
			{
				if (pItemDesc)
				{
					//OutFieldValue.StringValue = (pItemDesc->useLimitClass == 0 ? TEXT("") : pItemDesc->useLimitClass == 1 ? TEXT("P") : pItemDesc->useLimitClass == 2 ? TEXT("R") : TEXT("S"));
					//if (pItemDesc->useLimitLevel > 0)
					//	OutFieldValue.StringValue += appItoa(pItemDesc->useLimitLevel);
					//if (OutFieldValue.StringValue == TEXT(""))
					//	OutFieldValue.StringValue = TEXT("ALL");
					BYTE LevelLimit = pItemDesc->useLimitLevel;
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Prefix"), TEXT("avaNet")) + GetLevelIconString(LevelLimit) + Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Postfix"), TEXT("avaNet"));
				}
				else
				{
					OutFieldValue.StringValue = TEXT("?");
				}
			}
			else if (CellTag == FName(TEXT("Price")))
			{
				//pItemDesc->priceType
				//				OutFieldValue.StringValue = FString::Printf(TEXT("$%d"), pItemDesc ? pItemDesc->buyMoneyPrice : 0);
				OutFieldValue.StringValue = Localize(TEXT("UIGeneral"), TEXT("Text_MoneyMark"), TEXT("avaNet")) + GetFmtMoneyString(pItemDesc ? pItemDesc->price : 0);
			}
			else if (CellTag == FName(TEXT("NoLimitIcon")))
			{
				OutFieldValue.StringValue = GetNoLimitIconString(pItemDesc->gaugeType);
			}
		}
		else
		{
			OutFieldValue.StringValue = FString(TEXT(" "));
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetShopCustomItems::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/* ==========================================================================================================
UUIDataProvider_AvaNetItemDesc
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetItemDesc);


void UUIDataProvider_AvaNetItemDesc::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("GraphItemName")),TEXT("GraphItemName"));
	OutCellTags.Set(FName(TEXT("GraphValue")),TEXT("GraphValue"));
	OutCellTags.Set(FName(TEXT("GraphValueDiff")),TEXT("GraphValueDiff"));
	OutCellTags.Set(FName(TEXT("Diff")), TEXT("Diff"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetItemDesc::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if( GIsEditor && !GIsGame)
	{
		//OutFieldValue.StringValue = TEXT("usages : <avaNet:ItemGraphList? WeaponItemID=(Index from weaponlist) CustomItemID=(index from customitemlist)");
		if( CellTag == FName(TEXT("GraphItemName")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_ItemGraph[0]"), TEXT("avaNet"));
		}
		else if ( CellTag == FName(TEXT("GraphValue")) )
		{
			OutFieldValue.StringValue = FString::Printf(*Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_ItemGraph_Set"), TEXT("avaNet")), 100);
		}
	}
	else
	{
		if( bHideItemDesc )
		{
			OutFieldValue.StringValue = TEXT(" ");
		}
		else if( CellTag == FName(TEXT("GraphItemName")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), *FString::Printf(TEXT("Text_ItemGraph[%d]"), ListIndex), TEXT("avaNet"));
		}
		else if ( CellTag == FName(TEXT("GraphValue")) || CellTag == FName(TEXT("GraphValueDiff")) || CellTag == FName(TEXT("Diff")))
		{
			if( _StateController != NULL && _StateController->PlayerInfo.IsValid() &&
				0 <= ListIndex && ListIndex < MAX_ITEM_GRAPH_LIST )
			{
				ITEM_INFO ItemInfo;
				ITEM_DESC* pItem = NULL;
				SHORT WeaponValue = 0;
				SHORT CustomItemValue = 0;
				SHORT Difference = 0;
				Def::CUSTOM_SLOT_IDX CustomItemSlot = _CSI_NONE;

				if( 0 <= InvenID && InvenID < 10000 && _StateController != NULL && _StateController->PlayerInfo.IsValid())
				{
					ItemInfo = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(InvenID)];
					pItem = _ItemDesc().GetItem(ItemInfo.id);
				}
				if( WeaponItemID >= 0 )
				{
					pItem = _ItemDesc().GetItem( WeaponItemID);
				}
				if( CustomItemID >= 0 )
				{

					CUSTOM_ITEM_DESC* cItem = _ItemDesc().GetCustomItem(CustomItemID);
					if( cItem != NULL )
					{
						CustomItemValue = cItem->GetGraphValue(ListIndex);
						CustomItemSlot = cItem->customType;
					}
				}

				if( pItem != NULL )
				{
					WeaponValue = pItem->GetGraphValue(ListIndex);
					Difference = CustomItemValue;

					CInventory& Inven = _StateController->PlayerInfo.Inven;
					CUSTOM_ITEM_INFO* cItemInfo = Inven.GetCustomInvenToSlot(ItemInfo.sn, CustomItemSlot);
					if( cItemInfo )
					{
						CUSTOM_ITEM_DESC* cItemDesc = _ItemDesc().GetCustomItem(cItemInfo->id);
						if( cItemDesc )
						{
							WeaponValue += cItemDesc->GetGraphValue(ListIndex);
							Difference = CustomItemValue - cItemDesc->GetGraphValue(ListIndex);
						}
					}
				}

				FString CellTagLower = FString(CellTag.GetName()).ToLower();
				if( CellTagLower.InStr( TEXT("graphvalue") ) != INDEX_NONE )
				{
					// 장탄수는 게이지로 보여주지 않는다
					if( ListIndex == 6)
					{
						//FString DiffStr = Difference > 0 ? FString::Printf(TEXT("(+%d)"),Difference) : 
						//	Difference < 0 ? FString::Printf(TEXT("(%d)"),Difference) : FString("");
						//OutFieldValue.StringValue = FString::Printf(TEXT("%d%s"), WeaponValue,*DiffStr);
						OutFieldValue.StringValue = appItoa(WeaponValue);
					}
					else
					{
						FString ImageStr, ReplaceStr;
						SHORT AppliedValue = Difference < 0 ? WeaponValue + Difference : WeaponValue;
						SHORT LeftValue = (100 - Clamp((AppliedValue + Abs(Difference)),0,100));
						SHORT AccumValue = 0;
						//			debugf(TEXT("Applied = %d, Diff = %d, Left = %d"),AppliedValue, Difference, LeftValue);
						if( AppliedValue > 0)
						{
							ImageStr = Localize(TEXT("UIInventoryScene"), TEXT("Icon_Common_ItemGraph_Set"), TEXT("avaNet"));
							OutFieldValue.StringValue += FString::Printf( *ImageStr, AppliedValue);
							AccumValue += AppliedValue;
						}
						if( Abs(Difference) != 0 )
						{
							FString SectionStr = Difference > 0 ? FString(TEXT("Icon_Common_ItemGraph_DiffPos")) : FString(TEXT("Icon_Common_ItemGraph_DiffNeg"));
							ImageStr = Localize( TEXT("UIInventoryScene"), *SectionStr , TEXT("avaNet") );
							if( Parse(*ImageStr, TEXT("U="),ReplaceStr) )
								ImageStr = ImageStr.Replace(*ReplaceStr, *appItoa(appAtoi(*ReplaceStr) + AccumValue));
							OutFieldValue.StringValue += FString::Printf( *ImageStr, Abs(Difference));
							AccumValue += Abs(Difference);
						}
						if( LeftValue > 0 )
						{
							ImageStr = Localize( TEXT("UIInventoryScene"), TEXT("Icon_Common_ItemGraph_UnSet"), TEXT("avaNet"));
							if( Parse(*ImageStr, TEXT("U="),ReplaceStr) )
								ImageStr = ImageStr.Replace(*ReplaceStr, *appItoa(appAtoi(*ReplaceStr) + AccumValue));
							OutFieldValue.StringValue += FString::Printf( *ImageStr , LeftValue);
							AccumValue += LeftValue;
						}
					}
				}

				if( CellTagLower.InStr(TEXT("diff")) != INDEX_NONE )
				{
					OutFieldValue.StringValue += FString::Printf(TEXT(" %s%d"), Difference > 0 ? TEXT("+") : Difference < 0 ? TEXT("-") : TEXT(""), Difference );
				}
			}
			// CellTag 'GraphValue' 끝;
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetItemDesc::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}


/* ==========================================================================================================
UUIDataProvider_AvaNetItemDescField
========================================================================================================== */


IMPLEMENT_CLASS(UUIDataProvider_AvaNetItemDescField);

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetItemDescField::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if( GIsEditor && !GIsGame )
	{

	}
	else
	{
		TID_ITEM ItemID = INDEX_NONE;
		INT InvenID = INDEX_NONE;
		INT CustomSlotIndex= _CSI_NONE;
		ITEM_DESC* itemDesc = NULL;
		ITEM_INFO* itemInfo = NULL;
		CUSTOM_ITEM_INFO* cItemInfo = NULL;
		CUSTOM_ITEM_DESC* cItemDesc = NULL;
		if( Parse( *FieldName, TEXT("ItemID="), ItemID) )
		{
			itemDesc = _ItemDesc().GetItem(ItemID);
			cItemDesc = _ItemDesc().GetCustomItem(ItemID);
		}
		else if ( Parse( *FieldName, TEXT("InvenID="), InvenID) )
		{
			INT InvenIndex = GetInvenIndexFromID( InvenID );
			if( InvenIndex != INDEX_NONE)
			{
				itemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenIndex];
				itemDesc = _ItemDesc().GetItem( itemInfo->id );
			}

			if( itemInfo != NULL && Parse( *FieldName, TEXT("CustomSlotIndex="), CustomSlotIndex ) )
			{
				CInventory Inven;
				Inven.Init( &_ItemDesc(), &_StateController->PlayerInfo.PlayerInfo.itemInfo );
				cItemInfo = Inven.GetCustomInvenToSlot(itemInfo->sn, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex);
				cItemDesc = cItemInfo ? _ItemDesc().GetCustomItem( cItemInfo->id ) : NULL;
				itemDesc = NULL;

				if( cItemDesc == NULL )
				{
					OutFieldValue.StringValue == TEXT(" ");
					return TRUE;
				}
			}
		}

		if( appStristr( *FieldName, TEXT("ItemName")) == *FieldName )
		{
			OutFieldValue.StringValue = itemDesc ? itemDesc->GetName() :
				cItemDesc ? cItemDesc->GetName() : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("IconCode")) == *FieldName )
		{
			TCHAR szIconCode[] = { TEXT('\0') , TEXT('\0') };
			szIconCode[0] = itemDesc ? itemDesc->GetIcon() :
				cItemDesc ? cItemDesc->GetIcon() : TEXT('\0');
			OutFieldValue.StringValue = appStrlen(szIconCode) > 0 ? FString(szIconCode) : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("LiteralDescription")) == *FieldName )
		{
			OutFieldValue.StringValue = itemDesc ? itemDesc->GetDescription() :
				cItemDesc ? cItemDesc->GetDescription() : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("MoneyPrice")) == *FieldName )
		{
			OutFieldValue.StringValue = itemDesc ? (itemDesc->priceType == _IPT_MONEY ? appItoa(itemDesc->price) : TEXT("invalid property")) :
				cItemDesc ? (cItemDesc->priceType == _IPT_MONEY ? appItoa(cItemDesc->price) : TEXT("invalid property")) : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("CashPrice")) == *FieldName)
		{
			OutFieldValue.StringValue = itemDesc ? (itemDesc->priceType == _IPT_CASH ? appItoa(itemDesc->price) : TEXT("invalid property")) :
				cItemDesc ? (cItemDesc->priceType == _IPT_CASH ? appItoa(cItemDesc->price) : TEXT("invalid property")) : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("RepairPrice")) ==  *FieldName )
		{
			OutFieldValue.StringValue = itemDesc ? appItoa(itemDesc->maintenancePrice) :
				cItemDesc ? TEXT("invalid property") : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("RefundPrice")) ==  *FieldName )
		{
			OutFieldValue.StringValue = itemDesc ? appItoa(itemDesc->price / 5) :
				cItemDesc ? TEXT("invalid property") : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("RebuildPrice")) ==  *FieldName )
		{
			OutFieldValue.StringValue = itemDesc ? appItoa(itemDesc->RisConvertiblePrice) :
				cItemDesc ? TEXT("invalid property") : TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("CustomType")) ==  *FieldName )
		{
			if( itemDesc )
			{
				//CUSTOM_SLOT_TYPE CSType = itemDesc->customType;
				//OutFieldValue.StringValue += CSType.front ? Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Front"),TEXT("avaNet")) : Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));
				//OutFieldValue.StringValue += CSType.mount ? Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Mount"),TEXT("avaNet")) : Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));
				//OutFieldValue.StringValue += CSType.barrel ? Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Barrel"),TEXT("avaNet")) : Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));
				//OutFieldValue.StringValue += CSType.trigger ? Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Trigger"),TEXT("avaNet")) : Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));
				//OutFieldValue.StringValue += CSType.grip ? Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Grip"),TEXT("avaNet")) : Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));
				//OutFieldValue.StringValue += CSType.stock ? Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Stock"),TEXT("avaNet")) : Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));						
				OutFieldValue.StringValue = GetSlotImageString( itemDesc->customType );
			}
			else if ( cItemDesc )
			{
				if( cItemDesc->customType == _CSI_FRONT )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Front"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_MOUNT )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Mount"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_BARREL )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Barrel"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_TRIGGER )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Trigger"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_GRIP)					
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Grip"),TEXT("avaNet"));
				else if ( cItemDesc->customType == _CSI_STOCK )
					OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Stock"),TEXT("avaNet"));
				else
					OutFieldValue.StringValue = FString(TEXT("Invalid Slot"));
			}
			else
				OutFieldValue.StringValue = TEXT("item desc ?");
		}
		else if( appStristr( *FieldName, TEXT("GraphValue")) ==  *FieldName )
		{
			SHORT GraphValue;
			OutFieldValue.StringValue = TEXT(" ");
			for( INT i = 0 ; i < MAX_ITEM_GRAPH_LIST ; i++ )
			{
				if( (itemDesc != NULL && (GraphValue = itemDesc->GetGraphValue(i)) != 0) ||
					(cItemDesc != NULL && (GraphValue = cItemDesc->GetGraphValue(i)) != 0) )
				{
					OutFieldValue.StringValue += Localize(TEXT("UIInventoryScene"), *FString::Printf(TEXT("Text_ItemGraph[%d]"), i), TEXT("avaNet")) 
						+ (GraphValue > 0 ? FString(TEXT("+")) : FString(TEXT("")))
						+ appItoa(GraphValue) + FString(TEXT(" "));
				}
			}
		}
		else
		{
			OutFieldValue.StringValue = FString(TEXT("unexpected field name ")) + FieldName;
		}
	}

	if( OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetItemDescField::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("ItemName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("IconCode")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LiteralDescription")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MoneyPrice")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("CashPrice")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RepairPrice")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RefundPrice")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RebuildPrice")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("CustomType")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GraphValue")));
}

/* ==========================================================================================================
UUIDataProvider_AvaNetItemDescList
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetItemDescList);


void UUIDataProvider_AvaNetItemDescList::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("ItemName")),TEXT("ItemName"));
	OutCellTags.Set(FName(TEXT("Durability")),TEXT("Durability"));
	OutCellTags.Set(FName(TEXT("BrandNew")),TEXT("BrandNew"));
	OutCellTags.Set(FName(TEXT("IconCode")),TEXT("IconCode"));
	OutCellTags.Set(FName(TEXT("LevelLimit")),TEXT("LevelLimit"));
	OutCellTags.Set(FName(TEXT("AvailSlot")),TEXT("AvailSlot"));
	OutCellTags.Set(FName(TEXT("TypedPrice")),TEXT("TypedPrice"));
	OutCellTags.Set(FName(TEXT("LiteralDesc")), TEXT("LiteralDesc"));
	OutCellTags.Set(FName(TEXT("CustomSlotImage")), TEXT("CustomSlotImage"));
	OutCellTags.Set(FName(TEXT("NoLimitIcon")), TEXT("NoLimitIcon"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetItemDescList::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	INT ItemID = ListIndex;

	if( GIsEditor && !GIsGame)
	{
		if ( CellTag == FName(TEXT("ItemName")) )
			OutFieldValue.StringValue = TEXT("ItemName");
		else if ( CellTag == FName(TEXT("Durability")) )
			OutFieldValue.StringValue = GetItemGaugeIconString( _IGT_DURABILITY, ITEM_LIMIT_INITED / 2 );
		else if ( CellTag == FName(TEXT("BrandNew")) )
			OutFieldValue.StringValue = TEXT("New?");
		else if ( CellTag == FName(TEXT("IconCode")) )
			OutFieldValue.StringValue = TEXT("a");
		else if ( CellTag == FName(TEXT("LevelLimit")) )
			OutFieldValue.StringValue = TEXT("Lvl");
		else if ( CellTag == FName(TEXT("AvailSlot")) )
			OutFieldValue.StringValue = TEXT("AvailSlot");
		else if ( CellTag == FName(TEXT("TypedPrice")) )
			OutFieldValue.StringValue = TEXT("TypedPrice");
		else if ( CellTag == FName(TEXT("LiteralDesc")) )
			OutFieldValue.StringValue = TEXT("LiteralDesc");
		else if ( CellTag == FName(TEXT("CustomSlotImage")) )
		{
			CUSTOM_SLOT_TYPE AvailCustomSlots;
			OutFieldValue.StringValue = GetSlotImageString( AvailCustomSlots );
		}
	}
	else if( ItemID < 0 )
	{
		OutFieldValue.StringValue = TEXT("Invalid ItemID");
	}
	else if( bHideItemDesc )
	{
		OutFieldValue.StringValue = TEXT(" ");
	}
	else
	{
		ITEM_INFO* ItemInfo = NULL;
		CUSTOM_ITEM_INFO* cItemInfo = NULL;
		ITEM_DESC* ItemDesc = NULL;
		CUSTOM_ITEM_DESC* cItemDesc = NULL;
		UBOOL bCustomItemCheck = FALSE;

		if( 0 <= InvenIndex && InvenIndex < MAX_INVENTORY_SIZE &&
			_StateController != NULL && _StateController->PlayerInfo.IsValid() )
		{
			ItemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenIndex];
		}
		else if ( 0 <= EquipInvenIndex && EquipInvenIndex < MAX_INVENTORY_SIZE 
			&& _StateController != NULL && _StateController->PlayerInfo.IsValid() )
		{
			ItemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[EquipInvenIndex];
		}

		if( ItemInfo != NULL && CustomSlotIndex >= 0 
			&& _StateController != NULL && _StateController->PlayerInfo.IsValid())
		{
			CInventory Inven;
			Inven.Init(&_ItemDesc(), &_StateController->PlayerInfo.PlayerInfo.itemInfo);
			cItemInfo = Inven.GetCustomInvenToSlot(ItemInfo->sn, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex );
			bCustomItemCheck = TRUE;
		}

		cItemDesc = _ItemDesc().GetCustomItem( cItemInfo ? cItemInfo->id : ItemID );
		ItemDesc = bCustomItemCheck == FALSE  ? _ItemDesc().GetItem( ItemID ) : NULL;

		// 커스텀 파츠에 대한 정보를 찾으려 하였으나 정보가 없다
		if( bCustomItemCheck && cItemDesc == NULL )
		{
			OutFieldValue.StringValue = TEXT(" ");
			return FALSE;
		}
		// 커스텀 파츠에 대한 정보를 찾았으므로 ItemInfo와 ItemDesc는 필요없음
		if( bCustomItemCheck )
		{
			ItemInfo = NULL;
			ItemDesc = NULL;
		}

		if ( CellTag == FName(TEXT("ItemName")) )
		{
			OutFieldValue.StringValue = ItemDesc ? ItemDesc->GetName() : 
				cItemDesc ? cItemDesc->GetName() : TEXT("Invalid Item");
		}
		else if ( CellTag == FName(TEXT("Durability")) )
		{
			if( (ItemInfo != NULL && ItemDesc != NULL) || 
				(cItemInfo != NULL && cItemDesc != NULL) )
			{
				OutFieldValue.StringValue = GetItemGaugeIconString( ItemDesc ? ItemDesc->gaugeType : cItemDesc ? cItemDesc->gaugeType : _IGT_NONE, 
					ItemInfo ? ItemInfo->limit : cItemInfo ? cItemInfo->limit : 0 );
			}
			else
			{
				OutFieldValue.StringValue = TEXT("item inapplicable");
			}
		}
		else if ( CellTag == FName(TEXT("BrandNew")) )
		{
			OutFieldValue.StringValue = TEXT("New? (not impl yet)");
		}
		else if ( CellTag == FName(TEXT("IconCode")) )
		{
			//FString ItemIconCode = ItemDesc ? FString::Printf(TEXT("%c"), ItemDesc->GetIcon()) :
			//						cItemDesc ? FString::Printf(TEXT("%c"), cItemDesc->GetIcon()) : TEXT("a");
			OutFieldValue.StringValue = GetIconCodeString( ItemDesc, cItemDesc );
		}
		else if ( CellTag == FName(TEXT("LevelLimit")) )
		{
			BYTE LevelLimit = ItemDesc ? ItemDesc->useLimitLevel : cItemDesc ? cItemDesc->useLimitLevel : 0;
			OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Prefix"), TEXT("avaNet")) + GetLevelIconString(LevelLimit) + Localize(TEXT("UIInventoryScene"), TEXT("Text_UseLevelLimit_Postfix"), TEXT("avaNet"));
		}
		else if ( CellTag == FName(TEXT("AvailSlot")) )
		{
			if( ItemDesc )
			{
				if( (ItemDesc->slotType & _EP_WEAP_PRIMARY) )
				{
					OutFieldValue.StringValue = ItemInfo ? GetSlotImageString( ItemDesc->customType, ItemInfo ) : GetSlotImageString( ItemDesc->customType );

				}
				else
				{
					OutFieldValue.StringValue = TEXT(" ");
				}

			}
			else
			{
				OutFieldValue.StringValue = TEXT("Invalid ItemDesc?");
			}
		}
		else if ( CellTag == FName(TEXT("TypedPrice")) )
		{
			if( (ItemDesc != NULL && ItemDesc->priceType == _IPT_CASH) 
				|| (cItemDesc != NULL && cItemDesc->priceType == _IPT_CASH) )
			{
				Def::TCASH Cash = ItemDesc ? ItemDesc->price : cItemDesc ? cItemDesc->price : 0;
				OutFieldValue.StringValue += FString::Printf(TEXT("[%s]"), *GetCashString()) + appItoa(Cash);
			}
			else if ( (ItemDesc != NULL && ItemDesc->priceType == _IPT_MONEY) ||
				(cItemDesc != NULL && cItemDesc->priceType == _IPT_MONEY) )
			{
				Def::TMONEY Money = ItemDesc ? ItemDesc->price : cItemDesc ? cItemDesc->price : 0;
				OutFieldValue.StringValue += Localize(TEXT("UIGeneral"), TEXT("Text_GameMoney"), TEXT("avaNet")) + GetFmtMoneyString(Money);
			}
			else if (  (ItemDesc != NULL && ItemDesc->priceType == _IPT_NONE) ||
				(cItemDesc != NULL && cItemDesc->priceType == _IPT_NONE))
			{
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_PriceType_None"), TEXT("avaNet"));
			}
			else
				OutFieldValue.StringValue = TEXT("Invalid Item");
		}
		else if ( CellTag == FName(TEXT("LiteralDesc")) )
		{
			OutFieldValue.StringValue = ItemDesc ? ItemDesc->GetDescription() :
				cItemDesc ? cItemDesc->GetDescription() : TEXT("Invalid Item");
		}
		else if ( CellTag == FName(TEXT("CustomSlotImage")) )
		{
			Def::CUSTOM_SLOT_IDX CustomType = cItemDesc ? cItemDesc->customType : _CSI_NONE;

			if( CustomType == _CSI_FRONT )
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Front"),TEXT("avaNet"));
			else if ( CustomType == _CSI_MOUNT )
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Mount"),TEXT("avaNet"));
			else if ( CustomType == _CSI_BARREL )
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Barrel"),TEXT("avaNet"));
			else if ( CustomType == _CSI_TRIGGER )
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Trigger"),TEXT("avaNet"));
			else if ( CustomType == _CSI_GRIP)					
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Grip"),TEXT("avaNet"));
			else if ( CustomType == _CSI_STOCK )
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Stock"),TEXT("avaNet"));
			else if ( CustomType == _CSI_NONE )
				OutFieldValue.StringValue = Localize(TEXT("UIInventoryScene"),TEXT("Icon_WeaponCustom_Blank"),TEXT("avaNet"));
			else
				OutFieldValue.StringValue = FString(TEXT("Invalid Slot"));
		}
		else if ( CellTag == FName(TEXT("NoLimitIcon")) )
		{
			OutFieldValue.StringValue = GetNoLimitIconString( ItemDesc ? ItemDesc->gaugeType : (cItemDesc ? cItemDesc->gaugeType : 0) );
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("N/A");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetItemDescList::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/* ==========================================================================================================
UUIDataProvider_AvaNetMisc
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetMisc);


void UUIDataProvider_AvaNetMisc::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("ChannelGroup")),TEXT("ChannelGroup"));
	OutCellTags.Set(FName(TEXT("ChatType")), TEXT("ChatType"));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetMisc::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if( GIsEditor && !GIsGame)
	{
		if( CellTag != FName(TEXT("ChannelGroup")))
			OutFieldValue.StringValue = FString(CellTag.GetName()) + appItoa(ListIndex);
		else
			OutFieldValue.StringValue = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelGroup[%d]"),ListIndex), TEXT("avaNet"));
	}
	else
	{
		if ( CellTag == FName(TEXT("ChannelGroup")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelGroup[%d]"),ListIndex), TEXT("avaNet"));
		}
		else if ( CellTag == FName(TEXT("ChatType")) )
		{
			OutFieldValue.StringValue = Localize(TEXT("UILobbyScene"), *FString::Printf(TEXT("Text_Label_ChatType[%d]"),ListIndex), TEXT("avaNet"));
		}
		else
			OutFieldValue.StringValue = TEXT("invalid celltag"); 

	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetMisc::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}


/* ==========================================================================================================
UUIDataProvider_AvaNetItemDescField
========================================================================================================== */


IMPLEMENT_CLASS(UUIDataProvider_AvaNetMiscField);

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetMiscField::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if( GIsEditor && !GIsGame )
	{
		OutFieldValue.StringValue = FieldName;
	}
	else
	{
		if( appStristr(*FieldName,TEXT("DefaultMaxPlayer")) == *FieldName )
		{
			INT MapListIndex = INDEX_NONE;
			if( Parse(*FieldName, TEXT("MapListIndex="),MapListIndex) && 
				_StateController && _StateController->MapList.MapList.IsValidIndex(MapListIndex) )
			{
				FMapInfo& MapInfo = _StateController->MapList.MapList(MapListIndex);
				OutFieldValue.StringValue = appItoa(MapInfo.DefaultMaxPlayer);
			}
		}
		else if ( appStristr(*FieldName,TEXT("DefaultWinCondition")) == *FieldName )
		{
			INT MapListIndex = INDEX_NONE;
			if( Parse(*FieldName, TEXT("MapListIndex="),MapListIndex) && 
				_StateController && _StateController->MapList.MapList.IsValidIndex(MapListIndex) )
			{
				FMapInfo& MapInfo = _StateController->MapList.MapList(MapListIndex);
				OutFieldValue.StringValue = appItoa(MapInfo.DefaultWinCond);
			}
		}
	}

	if( OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetMiscField::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("DefaultMaxPlayer")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DefaultWinCondition")));
}

UBOOL UUIDataProvider_AvaNetMiscField::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	return bResult;
}


/* ==========================================================================================================
UUIDataProvider_AvaNetLastResult
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLastResult);

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetLastResult::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL GIsEditorNotGame = GIsEditor && !GIsGame;

	if (FieldName == TEXT("TeamScoreEU"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.TeamScore[0]);
	}
	else if (FieldName == TEXT("TeamScoreNRF"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.TeamScore[1]);
	}
	else if (FieldName == TEXT("GuildNameLeft"))
	{
		OutFieldValue.StringValue = TEXT("<avaNet:SelectedRoom.GuildNameLeft>");
		//OutFieldValue.StringValue = Localize(TEXT("UIResultScene"), TEXT("Text_Test_ClanNameLeft"), TEXT("avaNet"));
	}
	else if (FieldName == TEXT("GuildMarkLeft"))
	{
		OutFieldValue.StringValue = TEXT("<avaNet:SelectedRoom.GuildMarkLeft>");
	}
	else if (FieldName == TEXT("GuildNameRight"))
	{
		OutFieldValue.StringValue = TEXT("<avaNet:SelectedRoom.GuildNameRight>");
		//OutFieldValue.StringValue = Localize(TEXT("UIResultScene"), TEXT("Text_Test_ClanNameRight"), TEXT("avaNet"));
	}
	else if (FieldName == TEXT("GuildMarkRight"))
	{
		OutFieldValue.StringValue = TEXT("<avaNet:SelectedRoom.GuildMarkRight>");
	}
	else if (FieldName == TEXT("Level"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), _StateController->LastResultInfo.Level);
	}
	else if (FieldName == TEXT("LevelName"))
	{
		OutFieldValue.StringValue = GetLevelNameString(_StateController->LastResultInfo.Level);
	}
	else if (FieldName == TEXT("XP"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), _StateController->LastResultInfo.xp);
	}
	else if (FieldName == TEXT("XPCombo"))
	{
		INT CurrentXP = GIsEditorNotGame ? 150 : _StateController->LastResultInfo.xp;
		INT BonusXPPerc = GIsEditorNotGame ? 20 : _StateController->PlayerInfo.GetBonusXPPerc() + _StateController->PlayerInfo.GetPcBangBonusXPPerc();
		OutFieldValue.StringValue = GetXPComboString( CurrentXP ,BonusXPPerc );
	}
	else if ( FieldName == TEXT("XPDiffAbs") )
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_XP );
		INT XPDiffAbs = MsgInfo ? MsgInfo->Variation : 0;
		OutFieldValue.StringValue = FString::Printf(TEXT("+%d"),XPDiffAbs);
	}
	else if( FieldName == TEXT("XPDiffPerc") )
	{
		INT XPDiffPercent = GIsEditorNotGame ? 20 : _StateController->PlayerInfo.GetBonusXPPerc();
		OutFieldValue.StringValue = FString::Printf(TEXT("+%d%%"),XPDiffPercent);
	}
	else if (FieldName == TEXT("SupplyPoint"))
	{
		OutFieldValue.StringValue = GetSupplyGaugeIconString( _StateController->LastResultInfo.SupplyPoint );
	}
	else if (FieldName == TEXT("SupplyPointText"))
	{
		OutFieldValue.StringValue = appItoa( _StateController->LastResultInfo.SupplyPoint);
	}
	else if (FieldName == TEXT("SupplyPointTextCombo"))
	{
		INT CurrentSupply = GIsEditorNotGame ? 10000 : _StateController->LastResultInfo.SupplyPoint;
		INT BonusSupplyPerc = GIsEditorNotGame ? 15 : _StateController->PlayerInfo.GetBonusSupplyPerc();
		OutFieldValue.StringValue = GetSupplyComboString( CurrentSupply, BonusSupplyPerc );
	}
	else if ( FieldName == TEXT("SupplyDiffAbs") )
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_Supply );
		INT SupplyDiffAbs = MsgInfo ? MsgInfo->Variation : 0;
		OutFieldValue.StringValue = FString::Printf(TEXT("+%d"),SupplyDiffAbs);
	}
	else if( FieldName == TEXT("SupplyDiffPerc") )
	{
		INT SupplyDiffPercent = GIsEditorNotGame ? 15 : _StateController->PlayerInfo.GetBonusSupplyPerc();
		OutFieldValue.StringValue = FString::Printf(TEXT("+%d%%"),SupplyDiffPercent);
	}
	else if (FieldName == TEXT("SupplyCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), _StateController->LastResultInfo.SupplyCount);
	}
	else if (FieldName == TEXT("NextItemSupplyCount"))
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_SupplyMoney );
		INT NextItemSupplyCount = MsgInfo ? MsgInfo->Variation : 0;

		OutFieldValue.StringValue = appItoa(NextItemSupplyCount);
	}
	else if (FieldName == TEXT("PCBangXPDiffPerc"))
	{
		INT PCBangDiffPercent = GIsEditorNotGame ? 15 : _StateController->PlayerInfo.GetPcBangBonusXPPerc();
		OutFieldValue.StringValue = FString::Printf(TEXT("+%d%%"),PCBangDiffPercent);
	}
	else if (FieldName == TEXT("PCBangXPDiffAbs"))
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_PCBangXP );
		INT PCBangDiffAbs = MsgInfo ? MsgInfo->Variation : 0;

		OutFieldValue.StringValue = FString::Printf(TEXT("+%d"), PCBangDiffAbs);
	}
	else if (FieldName == TEXT("PCBangMoneyDiffAbs"))
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_PCBangMoney );
		INT PCBangDiffAbs = MsgInfo ? MsgInfo->Variation : 0;

		OutFieldValue.StringValue = FString::Printf(TEXT("+%s%d"), *GetMoneyMarkString(), PCBangDiffAbs);
	}
	else if (FieldName == TEXT("Money"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("+ %s%d"), *GetMoneyMarkString() ,_StateController->LastResultInfo.Money);
	}
	else if (FieldName == TEXT("AwardMoneyDiffAbs"))
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_Money );
		INT AwardMoneyDiffAbs = MsgInfo ? MsgInfo->Variation : 0;
		OutFieldValue.StringValue = FString::Printf(TEXT("+%s%d"), *GetMoneyMarkString(), AwardMoneyDiffAbs);
	}
	else if (FieldName == TEXT("BoostMoneyDiffAbs"))
	{
		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_MoneyBoost );
		INT BoostMoneyDiffAbs = MsgInfo ? MsgInfo->Variation : 0;
		OutFieldValue.StringValue = FString::Printf(TEXT("+%s%d"), *GetMoneyMarkString(), BoostMoneyDiffAbs);
	}
	else if (FieldName == TEXT("MoneyDiffAbs" ) )
	{
		INT MoneyDiffAbs = _StateController->LastResultInfo.Money;
		OutFieldValue.StringValue = FString::Printf(TEXT("+%s%d"), *GetMoneyMarkString(),MoneyDiffAbs);
	}
	else if (FieldName == TEXT("ScoreAttacker"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.score.attacker);
	}
	else if (FieldName == TEXT("ScoreDefender"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.score.defender);
	}
	else if (FieldName == TEXT("ScoreLeader"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.score.leader);
	}
	else if (FieldName == TEXT("ScoreTactic"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.score.tactic);
	}
	else if (FieldName == TEXT("ScoreSum"))
	{
		INT Score = _StateController->LastResultInfo.PlayerResultInfo.score.Sum() +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].killCount +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].killCount +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].killCount;
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), Score);
	}
	else if (FieldName == TEXT("GameWinOrDefeat"))
	{
		if (_StateController->AmISpectator())
			OutFieldValue.StringValue = TEXT("Victory");
		else
			OutFieldValue.StringValue = (_StateController->LastResultInfo.PlayerResultInfo.gameWin > 0 ? TEXT("Victory") : TEXT("Defeat"));
	}
	else if (FieldName == TEXT("RoundWinCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.roundWin);
	}
	else if (FieldName == TEXT("RoundDefeatCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.roundDefeat);
	}
	else if (FieldName == TEXT("WinRatio"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.roundDefeat > 0 ? _StateController->LastResultInfo.PlayerResultInfo.roundDefeat : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.roundWin / b);
	}
	else if (FieldName == TEXT("DisconnectCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.disconnectCount);
	}
	else if (FieldName == TEXT("DisconnectPerGame"))
	{
		DWORD GameCount = _StateController->LastResultInfo.PlayerResultInfo.roundWin + _StateController->LastResultInfo.PlayerResultInfo.roundDefeat;
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), GameCount > 0 ? _StateController->LastResultInfo.PlayerResultInfo.disconnectCount / (FLOAT)GameCount : 0.f);
	}
	else if (FieldName == TEXT("KillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].killCount +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].killCount +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].killCount);
	}
	else if (FieldName == TEXT("DeathCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.deathCount);
	}
	//else if (FieldName == TEXT("KillDeathRatio"))
	//{
	//	INT Kill = _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].killCount +
	//			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].killCount +
	//			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].killCount;
	//	FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.deathCount > 0 ? _StateController->LastResultInfo.PlayerResultInfo.deathCount : 1.0);
	//	OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)Kill / b);
	//}
	else if (FieldName == TEXT("ScoreDeathRatio"))
	{
		INT Score = _StateController->LastResultInfo.PlayerResultInfo.score.Sum() +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].killCount +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].killCount +
			_StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].killCount;
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.deathCount > 0 ? _StateController->LastResultInfo.PlayerResultInfo.deathCount : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)Score / b);
	}
	else if (FieldName == TEXT("BIADiffPerc"))
	{
		// @ TODO : 전우 포인트 증감값 넣기
		INT DiffPerc = (GIsEditor && !GIsGame) ? 5 : _StateController->PlayerInfo.GetBonusBIAPerc();
		OutFieldValue.StringValue = FString::Printf(TEXT("+%d"), DiffPerc);
	}
	else if (FieldName == TEXT("BIADiffAbs"))
	{
		// 현재 바뀐값과 증감 비율을 가지고 역으로 계산하지 않기로 했음
		//INT BIADiffPercent = GIsEditorNotGame ? 15 : _StateController->PlayerInfo.GetBonusBIAPerc();
		//INT BIADiffAbs = (BIADiffPercent/(BIADiffPercent + 100.f)) * _StateController->PlayerInfo.PlayerInfo.biaXP;

		FLastResultInfo::FLastResultMsgInfo* MsgInfo = _StateController->LastResultInfo.GetLastResultMsgInfo( LastResultMsgType_Brother );
		INT BIADiffAbs = MsgInfo ? MsgInfo->Variation : 0;

		OutFieldValue.StringValue = FString::Printf(TEXT("+%d"), BIADiffAbs);
	}
	else if (FieldName == TEXT("BIAIcon"))
	{
		UBOOL bBIARefresh = (GIsEditor && !GIsGame) ? TRUE : (_StateController->LastResultInfo.BIAFlag == 2);
		OutFieldValue.StringValue = bBIARefresh ? *Localize(TEXT("UIResultScene"),TEXT("Icon_BIAPoint"),TEXT("avaNet")) : TEXT(" ");
	}
	else if (FieldName == TEXT("HelmetDropCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.helmetDropCount);
	}
	else if (FieldName == TEXT("BulletMultiKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.bulletMultiKillCount);
	}
	else if (FieldName == TEXT("GrenadeMultiKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.grenadeMultiKillCount);
	}
	else if (FieldName == TEXT("TopKillRounds"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.roundTopKillCount);
	}
	else if (FieldName == TEXT("TopHeadshotKillRounds"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.roundTopHeadshotKillCount);
	}
	else if (FieldName == TEXT("EliteKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.topLevelKillCount);
	}
	else if (FieldName == TEXT("VeteranKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.higherLevelKillCount);
	}
	else if (FieldName == TEXT("NoDamageWinRounds"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.noDamageWinCount);
	}
	else if (FieldName == TEXT("IsTopScored"))
	{
		OutFieldValue.StringValue = (_StateController->LastResultInfo.PlayerResultInfo.topScoreCount ? TEXT("Yes") : TEXT("No"));
	}
	else if (FieldName == TEXT("NoDeathRounds"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.noDamageWinCount);
	}
	else if (FieldName == TEXT("TeamKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.teamKillCount);
	}
	else if (FieldName == TEXT("WeaponFireCountPistol"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[0]);
	}
	else if (FieldName == TEXT("WeaponFireCountSMG"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[1]);
	}
	else if (FieldName == TEXT("WeaponFireCountAR"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[2]);
	}
	else if (FieldName == TEXT("WeaponFireCountSR"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[3]);
	}
	else if (FieldName == TEXT("WeaponHitCountPistol"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[0]);
	}
	else if (FieldName == TEXT("WeaponHitCountSMG"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[1]);
	}
	else if (FieldName == TEXT("WeaponHitCountAR"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[2]);
	}
	else if (FieldName == TEXT("WeaponHitCountSR"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[3]);
	}
	else if (FieldName == TEXT("WeaponHitRatioPistol"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[0] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[0] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[0] / b);
	}
	else if (FieldName == TEXT("WeaponHitRatioSMG"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[1] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[1] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[1] / b);
	}
	else if (FieldName == TEXT("WeaponHitRatioAR"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[2] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[2] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[2] / b);
	}
	else if (FieldName == TEXT("WeaponHitRatioSR"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[3] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[3] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHitCount[3] / b);
	}
	else if (FieldName == TEXT("WeaponHeadshotCountPistol"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[0]);
	}
	else if (FieldName == TEXT("WeaponHeadshotCountSMG"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[1]);
	}
	else if (FieldName == TEXT("WeaponHeadshotCountAR"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[2]);
	}
	else if (FieldName == TEXT("WeaponHeadshotCountSR"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[3]);
	}
	else if (FieldName == TEXT("WeaponHeadshotRatioPistol"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[0] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[0] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[0] / b);
	}
	else if (FieldName == TEXT("WeaponHeadshotRatioSMG"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[1] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[1] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[1] / b);
	}
	else if (FieldName == TEXT("WeaponHeadshotRatioAR"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[2] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[2] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[2] / b);
	}
	else if (FieldName == TEXT("WeaponHeadshotRatioSR"))
	{
		FLOAT b = (_StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[3] > 0 ? _StateController->LastResultInfo.PlayerResultInfo.weaponFireCount[3] : 1.0);
		OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), (FLOAT)_StateController->LastResultInfo.PlayerResultInfo.weaponHeadshotCount[3] / b);
	}
	else if (FieldName == TEXT("PointManPlayRound"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].playRound);
	}
	else if (FieldName == TEXT("PointManHeadshotCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].headshotCount);
	}
	else if (FieldName == TEXT("PointManHeadshotKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].headshotKillCount);
	}
	else if (FieldName == TEXT("PointManPlayTime"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].playTime);
	}
	else if (FieldName == TEXT("PointManPlayTimeFmt"))
	{
		if (_StateController->LastResultInfo.IsValid())
		{
			DWORD PlayTime = _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].playTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), PlayTime / 3600, (PlayTime % 3600) / 60, (PlayTime % 3600) % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("00:00:00");
		}
	}
	else if (FieldName == TEXT("PointManSprintTime"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].sprintTime / 60, _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].sprintTime % 60);
	}
	else if (FieldName == TEXT("PointManTakenDamage"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].takenDamage);
	}
	else if (FieldName == TEXT("PointManKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].killCount);
	}
	else if (FieldName == TEXT("PointManWeaponKillCount1"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponKillCount[0]);
	}
	else if (FieldName == TEXT("PointManWeaponDamage1"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponDamage[0]);
	}
	else if (FieldName == TEXT("PointManWeaponKillCount2"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponKillCount[1]);
	}
	else if (FieldName == TEXT("PointManWeaponDamage2"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponDamage[1]);
	}
	else if (FieldName == TEXT("PointManWeaponKillCount3"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponKillCount[2]);
	}
	else if (FieldName == TEXT("PointManWeaponDamage3"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponDamage[2]);
	}
	else if (FieldName == TEXT("PointManWeaponKillCount4"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponKillCount[3]);
	}
	else if (FieldName == TEXT("PointManWeaponDamage4"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[0].weaponDamage[3]);
	}
	else if (FieldName == TEXT("RifleManPlayRound"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].playRound);
	}
	else if (FieldName == TEXT("RifleManHeadshotCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].headshotCount);
	}
	else if (FieldName == TEXT("RifleManHeadshotKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].headshotKillCount);
	}
	else if (FieldName == TEXT("RifleManPlayTime"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].playTime);
	}
	else if (FieldName == TEXT("RifleManPlayTimeFmt"))
	{
		if (_StateController->LastResultInfo.IsValid())
		{
			DWORD PlayTime = _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].playTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), PlayTime / 3600, (PlayTime % 3600) / 60, (PlayTime % 3600) % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("00:00:00");
		}
	}
	else if (FieldName == TEXT("RifleManSprintTime"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].sprintTime /60, _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].sprintTime % 60);
	}
	else if (FieldName == TEXT("RifleManTakenDamage"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].takenDamage);
	}
	else if (FieldName == TEXT("RifleManKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].killCount);
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount1"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponKillCount[0]);
	}
	else if (FieldName == TEXT("RifleManWeaponDamage1"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponDamage[0]);
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount2"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponKillCount[1]);
	}
	else if (FieldName == TEXT("RifleManWeaponDamage2"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponDamage[1]);
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount3"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponKillCount[2]);
	}
	else if (FieldName == TEXT("RifleManWeaponDamage3"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponDamage[2]);
	}
	else if (FieldName == TEXT("RifleManWeaponKillCount4"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponKillCount[3]);
	}
	else if (FieldName == TEXT("RifleManWeaponDamage4"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[1].weaponDamage[3]);
	}
	else if (FieldName == TEXT("SniperPlayRound"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].playRound);
	}
	else if (FieldName == TEXT("SniperHeadshotCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].headshotCount);
	}
	else if (FieldName == TEXT("SniperHeadshotKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].headshotKillCount);
	}
	else if (FieldName == TEXT("SniperPlayTime"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].playTime);
	}
	else if (FieldName == TEXT("SniperPlayTimeFmt"))
	{
		if (_StateController->LastResultInfo.IsValid())
		{
			DWORD PlayTime = _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].playTime;
			OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d:%02d"), PlayTime / 3600, (PlayTime % 3600) / 60, (PlayTime % 3600) % 60);
		}
		else
		{
			OutFieldValue.StringValue = TEXT("00:00:00");
		}
	}
	else if (FieldName == TEXT("SniperSprintTime"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%02d:%02d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].sprintTime/ 60 , _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].sprintTime % 60);
	}
	else if (FieldName == TEXT("SniperTakenDamage"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].takenDamage);
	}
	else if (FieldName == TEXT("SniperKillCount"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].killCount);
	}
	else if (FieldName == TEXT("SniperWeaponKillCount1"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponKillCount[0]);
	}
	else if (FieldName == TEXT("SniperWeaponDamage1"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponDamage[0]);
	}
	else if (FieldName == TEXT("SniperWeaponKillCount2"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponKillCount[1]);
	}
	else if (FieldName == TEXT("SniperWeaponDamage2"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponDamage[1]);
	}
	else if (FieldName == TEXT("SniperWeaponKillCount3"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponKillCount[2]);
	}
	else if (FieldName == TEXT("SniperWeaponDamage3"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponDamage[2]);
	}
	else if (FieldName == TEXT("SniperWeaponKillCount4"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponKillCount[3]);
	}
	else if (FieldName == TEXT("SniperWeaponDamage4"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"), _StateController->LastResultInfo.PlayerResultInfo.classResultInfo[2].weaponDamage[3]);
	}

	return TRUE;
}

/* ==========================================================================================================
UUIDataProvider_AvaNetGuildInfoField
========================================================================================================== */


IMPLEMENT_CLASS(UUIDataProvider_AvaNetGuildInfoField);

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetGuildInfoField::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	FString FieldNameLower = FieldName.ToLower();
	if( GIsEditor && !GIsGame )
	{
		if ( FieldNameLower == TEXT("guildwincount") )
		{
			OutFieldValue.StringValue = FString::FormatAsNumber( 99999 );
		}
		else if ( FieldNameLower == TEXT("guilddefeatcount") )
		{
			OutFieldValue.StringValue = FString::FormatAsNumber( 99999 );
		}
		else if ( FieldNameLower == TEXT("guildmarklarge") )
		{
			OutFieldValue.StringValue = GetClanMarkString(0, false);//Localize(TEXT("UILobbyScene"), TEXT("Icon_Test_ClanImageL"), TEXT("avaNet"));
		}
		else if ( FieldNameLower == TEXT("guildmarksmall"))
		{
			OutFieldValue.StringValue = GetClanMarkString(0);//Localize(TEXT("UILobbyScene"), TEXT("Icon_Test_ClanImageS"), TEXT("avaNet"));
		}
		else
		{
			OutFieldValue.StringValue = FieldName;
		}
	}
	else
	{
		if( _StateController->GuildInfo.IsValid() )
		{
			if( FieldNameLower == TEXT("guildname") )
			{
				OutFieldValue.StringValue = _StateController->GuildInfo.GuildInfo.name;
			}
			else if ( FieldNameLower == TEXT("guildlevel") )
			{
				OutFieldValue.StringValue = appItoa(_StateController->GuildInfo.GuildInfo.level);
			}
			else if ( FieldNameLower == TEXT("guildmembercount") )
			{
				OutFieldValue.StringValue = appItoa(_StateController->GuildInfo.GuildInfo.cntMember);
			}
			else if ( FieldNameLower == TEXT("guildxp") )
			{
				OutFieldValue.StringValue = appItoa(_StateController->GuildInfo.GuildInfo.xp);
			}
			else if ( FieldNameLower == TEXT("guildmotd") )
			{
				OutFieldValue.StringValue = appStrlen(_StateController->GuildInfo.GuildInfo.motd) > 0 ? _StateController->GuildInfo.GuildInfo.motd : TEXT(" ");
			}
			else if ( FieldNameLower == TEXT("guildmaster") )
			{
				OutFieldValue.StringValue = *_StateController->GuildInfo.GetMasterName();
			}
			else if ( FieldNameLower == TEXT("guildregdate") )
			{
				FString RegDate = _StateController->GuildInfo.GuildInfo.regdate;
				INT Pos = RegDate.InStr(TEXT(" "));
				if (Pos >= 0)
					OutFieldValue.StringValue = RegDate.Left(Pos + 1);
				else
					OutFieldValue.StringValue = TEXT(" ");
			}
			else if ( FieldNameLower == TEXT("guildwincount") )
			{
				if( _StateController->GuildInfo.IsValid() )
					OutFieldValue.StringValue = FString::FormatAsNumber( _StateController->GuildInfo.GuildInfo.totalWinCnt );
				else
					OutFieldValue.StringValue = TEXT(" ");
			}
			else if ( FieldNameLower == TEXT("guilddefeatcount") )
			{
				if( _StateController->GuildInfo.IsValid() )
					OutFieldValue.StringValue = FString::FormatAsNumber( _StateController->GuildInfo.GuildInfo.totalLoseCnt );
				else
					OutFieldValue.StringValue = TEXT(" ");
			}
			else if ( FieldNameLower == TEXT("guildmarklarge") )
			{
				OutFieldValue.StringValue = GetClanMarkString(_StateController->GuildInfo.GetClanMarkID(), false);

				// 클랜로비에서 없는 경우는 없다.
				if ( OutFieldValue.StringValue.Len() == 0 )
					OutFieldValue.StringValue = GetClanMarkString(0, false);//Localize(TEXT("UILobbyScene"), TEXT("Text_Test_ClanImageL"), TEXT("avaNet"));
			}
			else if ( FieldNameLower == TEXT("guildmarksmall"))
			{
				OutFieldValue.StringValue = GetClanMarkString(_StateController->GuildInfo.GetClanMarkID());

				// 클랜로비에서 없는 경우는 없다.
				if ( OutFieldValue.StringValue.Len() == 0 )
					OutFieldValue.StringValue = GetClanMarkString(0);//Localize(TEXT("UILobbyScene"), TEXT("Text_Test_ClanImageS"), TEXT("avaNet"));
			}
			else
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("inv field %s"), *FieldName);;
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("Inv. guild info");
		}
	}

	if( OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetGuildInfoField::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildLevel")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMemberCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildXP")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMOTD")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMaster")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildRegDate")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildWinCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildDefeatCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMarkLarge")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMarkSmall")));
}

/* ==========================================================================================================
UUIDataProvider_AvaNetGuildMembers
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetGuildMembers);


void UUIDataProvider_AvaNetGuildMembers::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("LevelCombo")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelCombo"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelIcon")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelIcon"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("LevelName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_LevelName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NickName")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_NickName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("GuildSubGroup")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_GuildSubGroup"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("GuildRank")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_GuildRank"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("OnlineStatus")), *Localize(TEXT("UILobbyScene"), TEXT("Text_List_OnlineStatus"), TEXT("avaNet")) );
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetGuildMembers::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		OutFieldValue.StringValue = FString(CellTag.GetName()).Left(3) + appItoa(ListIndex);
	}
	else
	{
		if ( _StateController->GuildInfo.IsValid() && _StateController->GuildInfo.PlayerList.PlayerList.IsValidIndex(ListIndex) )
		{
			const FGuildPlayerInfo& GuildPlayerInfoOuter = _StateController->GuildInfo.PlayerList.PlayerList(ListIndex);
			const Def::GUILD_PLAYER_INFO& GuildPlayerInfo = GuildPlayerInfoOuter.GuildPlayerInfo;

			if ( CellTag ==  FName(TEXT("LevelCombo")) )
			{
				OutFieldValue.StringValue = GetLevelIconString( GuildPlayerInfo.level ) + GetLevelNameString( GuildPlayerInfo.level );
			}
			else if ( CellTag ==  FName(TEXT("LevelIcon")) )
			{
				OutFieldValue.StringValue = GetLevelIconString( GuildPlayerInfo.level );
			}
			else if ( CellTag ==  FName(TEXT("LevelName")) )
			{
				OutFieldValue.StringValue = GetLevelNameString( GuildPlayerInfo.level );
			}
			else if ( CellTag ==  FName(TEXT("NickName")) )
			{
				OutFieldValue.StringValue = GuildPlayerInfo.nickname;
			}
			else if ( CellTag ==  FName(TEXT("GuildSubGroup")) )
			{
				OutFieldValue.StringValue = appItoa(GuildPlayerInfo.idGroup);
			}
			else if ( CellTag ==  FName(TEXT("GuildRank")) )
			{
				OutFieldValue.StringValue = appItoa(GuildPlayerInfo.idRank);
			}
			else if ( CellTag ==  FName(TEXT("OnlineStatus")) )
			{
				OutFieldValue.StringValue = GetOnlineStatusIconString(GuildPlayerInfoOuter.IsOnline());
			}
		}
		else
		{
			OutFieldValue.StringValue = TEXT("Inv. GuildInfo");
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("N/A");
	}
	return TRUE;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataProvider_AvaNetGuildMembers::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

UBOOL UUIDataProvider_AvaNetGuildMembers::SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	if ( PrimaryCellTag ==  FName(TEXT("LevelIcon")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(), 
			UIListItemSortBase<ParseArgumentSort<INT>,NormalStringSort>
			( SortParameters, ParseArgumentSort<INT>(TEXT("Icon_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("LevelName")) || PrimaryCellTag ==  FName(TEXT("LevelCombo")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<ParseArgumentSort<INT>,NormalStringSort>
			( SortParameters, ParseArgumentSort<INT>(TEXT("Name_Level["), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("NickName")) || PrimaryCellTag ==  FName(TEXT("GuildSubGroup")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<NormalStringSort>(SortParameters, NormalStringSort()) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("GuildRank")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<NormalIntegerSort>(SortParameters,NormalIntegerSort()) );
		return TRUE;
	}
	else if ( PrimaryCellTag ==  FName(TEXT("OnlineStatus")) )
	{
		ava::sort( ListItems.GetTypedData(), ListItems.GetTypedData() + ListItems.Num(),
			UIListItemSortBase<ParseArgumentSort<FString>, NormalStringSort>
			( SortParameters, ParseArgumentSort<FString>(TEXT("OnlineStatus_"), TRUE, TRUE), NormalStringSort(), FALSE, TRUE) );
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================================================
UUIDataProvider_AvaNetLastResult
========================================================================================================== */

/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataProvider_AvaNetLastResult::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("TeamScoreEU")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("TeamScoreNRF")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildNameLeft")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMarkLeft")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildNameRight")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GuildMarkRight")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("Level")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("LevelName")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("XP")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("XPCombo")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("XPDiffPerc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("XPDiffAbs")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyPoint")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyPointText")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyPointTextCombo")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyDiffPerc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyDiffAbs")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SupplyCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("NextItemSupplyCount")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("PCBangXPDiffPerc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PCBangXPDiffAbs")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PCBangMoneyDiffAbs")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("AwardMoneyDiffAbs")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BoostMoneyDiffAbs")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("MoneyDiffAbs")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("Money")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreAttacker")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreDefender")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreLeader")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreTactic")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreSum")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GameWinOrDefeat")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoundWinCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoundDefeatCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RoundWinRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DisconnectCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DIsconnectPerGame")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("KillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("DeathCount")));
	//new(out_Fields) FUIDataProviderField(FName(TEXT("KillDeathRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ScoreDeathRatio")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BIADiffPerc")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BIADiffAbs")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BIAIcon")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("HelmetDropCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("BulletMultiKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("GrenadeMultiKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("TopKillRounds")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("TopHeadshotKillRounds")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("EliteKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("VeteranKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("NoDamageWinRounds")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("IsTopScored")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("NoDeathRounds")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("TeamKillCount")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponFireCountPistol")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponFireCountSMG")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponFireCountAR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponFireCountSR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitCountPistol")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitCountSMG")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitCountAR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitCountSR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitRatioPistol")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitRatioSMG")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitRatioAR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHitRatioSR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotCountPistol")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotCountSMG")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotCountAR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotCountSR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotRatioPistol")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotRatioSMG")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotRatioAR")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("WeaponHeadshotRatioSR")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManPlayRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManHeadShotCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManHeadShotKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManPlayTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManPlayTimeFmt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManSprintTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManTakenDamage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponKillCount4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManWeaponDamage4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("PointManKillCountPerMinute")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManPlayRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManHeadShotCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManHeadShotKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManPlayTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManPlayTimeFmt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManSprintTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManTakenDamage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponKillCount4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManWeaponDamage4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("RifleManKillCountPerMinute")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperPlayRound")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperHeadShotCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperHeadShotKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperPlayTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperPlayTimeFmt")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperSprintTime")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperTakenDamage")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperKillCount")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage1")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage2")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage3")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponKillCount4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperWeaponDamage4")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("SniperKillCountPerMinute")));
}

void UUIDataProvider_AvaNetLastResult::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Level")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Level"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("ClanName")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_ClanName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("ClanMark")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_ClanMark"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("NickName")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_NickName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("NickNameCombo")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_NickName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Score")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Score"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Death")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Death"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("EXP")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_EXP"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("EXPCombo")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_EXP"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Supply")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Supply"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("SupplyCombo")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Supply"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("IsLevelUp")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_IsLevelUp"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("IsLeader")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_IsLeader"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("PcBang")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_PcBang"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("UpdateMsg")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_UpdateMsg"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("SupportItems")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_SupportItems"), TEXT("avaNet")));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetLastResult::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (CellTag == FName(TEXT("Level")))
		{
			OutFieldValue.StringValue = TEXT("1");
		}
		else if ( CellTag == FName(TEXT("ClanName")) )
		{
			OutFieldValue.StringValue = TEXT("--");
		}
		else if ( CellTag == FName(TEXT("ClanMark")) )
		{
			OutFieldValue.StringValue = GetClanMarkString(0);
		}
		else if (CellTag == FName(TEXT("NickName")) || CellTag == FName(TEXT("NickNameCombo")))
		{
			FString PlayerNameMod = ( ListIndex == 1) ? Localize(TEXT("UIResultScene"), TEXT("TextMod_ListElem_NickNameItsMe"), TEXT("avaNet")) : FString(TEXT(""));
			FString BIAModStr = (ListIndex == 2) ? Localize(TEXT("UILobbyScene"), TEXT("Text_Mod_BIAPlayer"), TEXT("avaNet")) : FString(TEXT(""));
			OutFieldValue.StringValue =  PlayerNameMod + BIAModStr +  FString::Printf(TEXT("Player%02d"), ListIndex);
			if( CellTag == TEXT("NickNameCombo") )
			{
				FString SquadLeaderIcon = ( ListIndex % 12 == 0 ) ? Localize(TEXT("UIResultScene"), TEXT("Icon_ListElem_SquadLeader"), TEXT("avaNet")) : FString(TEXT(""));
				OutFieldValue.StringValue = SquadLeaderIcon + OutFieldValue.StringValue;
			}
		}
		else if (CellTag == FName(TEXT("Score")))
		{
			OutFieldValue.StringValue = TEXT("15");
		}
		else if (CellTag == FName(TEXT("Death")))
		{
			OutFieldValue.StringValue = TEXT("10");
		}
		else if (CellTag == FName(TEXT("EXP")))
		{
			OutFieldValue.StringValue = TEXT("+500");
		}
		else if (CellTag == FName(TEXT("EXPCombo")))
		{
			OutFieldValue.StringValue = GetXPComboString( 500,40 );
		}
		else if (CellTag == FName(TEXT("Supply")))
		{
			OutFieldValue.StringValue = TEXT("+300");
		}
		else if (CellTag == FName(TEXT("SupplyCombo")))
		{
			OutFieldValue.StringValue = GetSupplyComboString( 10000, 20 );
		}
		else if (CellTag == FName(TEXT("IsLevelUp")))
		{
			OutFieldValue.StringValue = TEXT("+");
		}
		else if (CellTag == FName(TEXT("IsLeader")))
		{
			OutFieldValue.StringValue = TEXT("Leader");
		}
		else if (CellTag == FName(TEXT("PcBang")))
		{
			OutFieldValue.StringValue = TEXT("PC");
		}
		else if (CellTag == FName(TEXT("UpdateMsg")))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Update message #%d"), ListIndex);
		}
		else if (CellTag == FName(TEXT("SupportItems")) )
		{
			OutFieldValue.StringValue += GetPCBangIconString();
			OutFieldValue.StringValue += GetSupportItemIcon( Def::_IET_EXP_BOOST, 0);
			OutFieldValue.StringValue += GetSupportItemIcon( Def::_IET_SP_BOOST, 0);
			OutFieldValue.StringValue += GetSupportItemIcon( Def::_IET_MONEY_BOOST, 0);
		}
	}
	else if ( _StateController->LastResultInfo.IsValid() && _StateController->LastResultInfo.RoomResultInfo.IsValidIndex(ListIndex) )
	{
		FLastResultInfo::FPlayerResultInfo *Info = &_StateController->LastResultInfo.RoomResultInfo(ListIndex);
		if (CellTag == FName(TEXT("Level")))
		{
			if (Info && Info->Level >= 0)
				OutFieldValue.StringValue = GetLevelIconString(Info->Level);
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		else if ( CellTag == FName(TEXT("ClanName")) )
		{
			for(int i = 0; i < Def::MAX_ALL_PLAYER_PER_ROOM; i++)
			{
				if ( Info->idSlot == _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.idSlot )
					OutFieldValue.StringValue = _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.guildName;
			}
			return TRUE;
		}
		else if ( CellTag == FName(TEXT("ClanMark")) )
		{
			for(int i = 0; i < Def::MAX_ALL_PLAYER_PER_ROOM; i++)
			{
				if ( Info->idSlot == _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.idSlot )
					OutFieldValue.StringValue = GetClanMarkString( _StateController->RoomInfo.PlayerList.PlayerList[i].GetClanMarkID() );
			}
			return TRUE;
		}
		else if (CellTag == FName(TEXT("NickName")) || CellTag == FName(TEXT("NickNameCombo")))
		{
			FString PlayerName = _StateController->PlayerInfo.PlayerInfo.nickname;
			UBOOL ItsMe = PlayerName.Trim().TrimTrailing() == Info->Nickname.Trim().TrimTrailing();
			
			FBuddyInfo* BIAInfo = _Communicator().GetBIA();
			UBOOL ItsMyBIA = !ItsMe && BIAInfo && Info && BIAInfo->Nickname.Trim().TrimTrailing() == Info->Nickname.Trim().TrimTrailing();

			FString PlayerNameMod = ItsMe ? Localize(TEXT("UIResultScene"), TEXT("TextMod_ListElem_NickNameItsMe"), TEXT("avaNet")) :
									ItsMyBIA ? Localize(TEXT("UILobbyScene"), TEXT("Text_Mod_BIAPlayer"), TEXT("avaNet")) : FString(TEXT(""));
			OutFieldValue.StringValue = Info ? PlayerNameMod + Info->Nickname : TEXT(" ");

			if( CellTag == TEXT("NickNameCombo") )
			{
				FString SquadLeaderIcon = ( Info && Info->bLeader ) ? Localize(TEXT("UIResultScene"), TEXT("Icon_ListElem_SquadLeader"), TEXT("avaNet")) : FString(TEXT(""));
				OutFieldValue.StringValue = SquadLeaderIcon + OutFieldValue.StringValue;
			}
		}
		else if (CellTag == FName(TEXT("Score")))
		{
			if (Info)
				OutFieldValue.StringValue = appItoa(Info->Score);
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("Death")))
		{
			if (Info && Info->Death >= 0)
				OutFieldValue.StringValue = appItoa(Info->Death);
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("EXP")))
		{
			if (Info && Info->xp >= 0)
				OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), Info->xp);
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("ExpCombo")))
		{
			OutFieldValue.StringValue =	OutFieldValue.StringValue = GetXPComboString( 500,40 );
			if( Info )
				OutFieldValue.StringValue = GetXPComboString( Info->xp , Info->GetBonusXPPerc() + Info->GetPcBangBonusXPPerc() );
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("Supply")))
		{
			if (Info && Info->SupplyPoint >= 0)
				OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), Info->SupplyPoint);
			else
				OutFieldValue.StringValue = appItoa(0);
		}
		else if (CellTag == FName(TEXT("SupplyCombo")))
		{
			if( Info )
				OutFieldValue.StringValue = GetSupplyComboString( Info->SupplyPoint, Info->GetBonusSupplyPerc() );
			else
				OutFieldValue.StringValue = appItoa(0);
		}
		else if (CellTag == FName(TEXT("IsLevelUp")))
		{
			if (Info)
			{
				OutFieldValue.StringValue = Info->Level > Info->LastLevel ? TEXT("+") : TEXT(" ");
			}
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		else if (CellTag == FName(TEXT("IsLeader")))
		{
			if (Info)
			{
				OutFieldValue.StringValue = Info->bLeader ? TEXT("Leader") : TEXT(" ");
			}
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		else if (CellTag == FName(TEXT("PcBang")))
		{
			if (Info)
			{
				OutFieldValue.StringValue = Info->bPcBang ? TEXT("PC") : TEXT(" ");
			}
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		//else if (CellTag == FName(TEXT("UpdateMsg")))
		//{
		//	if (ListIndex >= 0 && ListIndex < _StateController->LastResultInfo.UpdateMsgList.Num())
		//		OutFieldValue.StringValue = _StateController->LastResultInfo.UpdateMsgList(ListIndex);
		//	else
		//		OutFieldValue.StringValue = TEXT(" ");
		//}
		else if ( CellTag == FName(TEXT("SupportItems")) )
		{
			if (Info)
			{
				OutFieldValue.StringValue += Info->bPcBang ? *GetPCBangIconString() : TEXT(" ");
				if ( CanApplyLastResult() )
				{
					for( INT i = 0 ; i < Info->EffectList.Num() ; i++)
					{
						EFFECT_ITEM_DESC *pDesc = _ItemDesc().GetEffectItem(Info->EffectList(i));
						if (pDesc)
							OutFieldValue.StringValue += GetSupportItemIcon( pDesc->effectInfo.effectType, 0);
					}

					if( Info->biaXPFlag > _BIAXP_NONE )
						OutFieldValue.StringValue += GetBIAIconString();
				}
			}

			if (OutFieldValue.StringValue.Len() == 0)
				OutFieldValue.StringValue = TEXT(" ");
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

/* ==========================================================================================================
UIDataProvider_AvaNetLastResultMsgs
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLastResultMsgs);

void UUIDataProvider_AvaNetLastResultMsgs::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Icon")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_LastResultMsg_Icon"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Name")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_LastResultMsg_Name"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Desc")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_LastResultMsg_Desc"), TEXT("avaNet")));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetLastResultMsgs::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (CellTag == FName(TEXT("Icon")))
		{
			OutFieldValue.StringValue = GetLastResultMsgIcon(ListIndex);
		}
		else if (CellTag == FName(TEXT("Name")))
		{
			OutFieldValue.StringValue = GetLastResultMsgName(ListIndex);
		}
		else if (CellTag == FName(TEXT("Desc")))
		{
			OutFieldValue.StringValue = GetLastResultMsgDesc(ListIndex, ITEM_LIMIT_INITED / 2);
		}
	}
	else
	{
		if (CellTag == FName(TEXT("Icon")))
		{
			OutFieldValue.StringValue = GetLastResultMsgIcon(ListIndex);
		}
		else if (CellTag == FName(TEXT("Name")))
		{
			OutFieldValue.StringValue = GetLastResultMsgName(ListIndex);
		}
		else if (CellTag == FName(TEXT("Desc")))
		{
			BYTE MsgType = ListIndex;
			BYTE EffectType = _IET_NONE;
			INT Limit = 0;
			TID_EQUIP_SLOT Slot = ID_INVALID_EQUIP_SLOT;

			EFFECT_ITEM_INFO* EffectItemInfo = NULL;
			EFFECT_ITEM_DESC* EffectItemDesc = NULL;
//			GetEffectItemByType( _StateController->PlayerInfo.PlayerInfo.itemInfo, EffectType, &EffectItemInfo, &EffectItemDesc );

			switch (MsgType)
			{
				case LastResultMsgType_XP:	Slot = 0; break;
				case LastResultMsgType_Supply:		Slot = 1; break;
				case LastResultMsgType_MoneyBoost:	Slot = 2; break;
			}

			if ( Slot != ID_INVALID_EQUIP_SLOT )
			{
				EffectItemInfo = GavaNetClient->StateController->PlayerInfo.Inven.GetEffectSet(Slot);
				if( EffectItemInfo != NULL )
					Limit = EffectItemInfo->limit;
				else
					Limit = 0;
			}

			OutFieldValue.StringValue = GetLastResultMsgDesc(ListIndex, Limit);
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

UBOOL UUIDataProvider_AvaNetLastResultMsgs::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/* ==========================================================================================================
UIDataProvider_AvaNetEffectItems
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetEffectItems);

void UUIDataProvider_AvaNetEffectItems::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( FName(TEXT("EffectItems")), DATATYPE_Collection );
}


void UUIDataProvider_AvaNetEffectItems::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Icon")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_LastResultMsg_Icon"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Desc")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_LastResultMsg_Desc"), TEXT("avaNet")));
}

/**
* Resolves the value of the cell specified by CellTag and stores it in the output parameter.
*
* @param	CellTag			the tag for the element cell to resolve the value for
* @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
*							do not provide unique UIListElement objects for each element.
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
*							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
*							to a data collection.
*/
UBOOL UUIDataProvider_AvaNetEffectItems::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	UBOOL bResult = TRUE;
	EFFECT_ITEM_DESC* EffectItemDesc = _ItemDesc().GetEffectItem(ListIndex);
	UBOOL IsPCBangFlag = (ListIndex == _IET_GR);

	if (GIsEditor && !GIsGame)
	{
		if( IsPCBangFlag )
		{
			if( CellTag == TEXT("Icon") )
			{
				OutFieldValue.StringValue = GetSupportItemIcon( _IET_GR, 0 );
			}
			else if ( CellTag == TEXT("Desc") )
			{
				OutFieldValue.StringValue = Localize(TEXT("UIResultScene"), TEXT("Text_SupportItemDesc[PCBang]"), TEXT("avaNet"));
			}
		}
		else
		{
			if (CellTag == FName(TEXT("Icon")))
			{
				if ( EffectItemDesc )
					OutFieldValue.StringValue = GetSupportItemIcon( EffectItemDesc->effectInfo.effectType, 0);
				else
					OutFieldValue.StringValue = GetSupportItemIcon( _IET_MONEY_BOOST, 0);
			}
			else if (CellTag == FName(TEXT("Desc")))
			{
				FString EffectDesc;
				if ( EffectItemDesc )
					EffectDesc = FString(EffectItemDesc->GetName()) + TEXT("\\n") + EffectItemDesc->GetDescription();
				else
					EffectDesc = FString::Printf(TEXT("Item %d\\nType Description Here"), ListIndex );

				OutFieldValue.StringValue = EffectDesc;
			}
		}
	}
	else
	{
		if( EffectItemDesc )
		{
			if (CellTag == FName(TEXT("Icon")))
			{
				BYTE EffectType = EffectItemDesc->effectInfo.effectType;
				OutFieldValue.StringValue = GetSupportItemIcon( EffectType , 0);
			}
			else if (CellTag == FName(TEXT("Desc")))
			{
				OutFieldValue.StringValue = FString(EffectItemDesc->GetName()) + TEXT("\\n") + EffectItemDesc->GetDescription();
			}
		}
		else if ( IsPCBangFlag )
		{
			if (CellTag == FName(TEXT("Icon")))
			{
				BYTE EffectType = _IET_GR;
				OutFieldValue.StringValue = GetSupportItemIcon( EffectType , 0);
			}
			else if (CellTag == FName(TEXT("Desc")))
			{
				OutFieldValue.StringValue = Localize(TEXT("UIResultScene"), TEXT("Text_SupportItemDesc[PCBang]"), TEXT("avaNet"));
			}
		}
		else
		{
			bResult = FALSE;
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return bResult;
}

UBOOL UUIDataProvider_AvaNetEffectItems::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetCashItems
========================================================================================================== */

IMPLEMENT_CLASS( UUIDataProvider_AvaNetCashItems );

/* === IUIListElement interface === */

UBOOL UUIDataProvider_AvaNetCashItems::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	return FALSE;
}

TScriptInterface<class IUIListElementCellProvider> UUIDataProvider_AvaNetCashItems::GetElementCellSchemaProvider( FName FieldName )
{
	return TScriptInterface<class IUIListElementCellProvider>();
}

/* === UIDataProvider interface === */

void UUIDataProvider_AvaNetCashItems::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	GetInternalDataProviderSupportedDataFields( out_Fields );
}

void UUIDataProvider_AvaNetCashItems::PostUpdateParamters( UBOOL bParmChanged )
{
	if( bParmChanged )
	{
		// 아이템 사용.
		if ( bUseCashItemTrigger )
		{
			// 아이템의 SN을 얻어서.
			if ( SelectedInvenItemListIndex >= 0 )
			{
				EFFECT_ITEM_DESC*	pItem = NULL;
				TSN_ITEM			item_sn;
				TID_ITEM			id;
				UBOOL				bUsedItem = false;
				INT					slot = -1;

				id = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[SelectedInvenItemListIndex].id;
				if ( id != ID_INVALID_ITEM )
					pItem = _ItemDesc().GetEffectItem(id);
				if ( pItem )
					slot = pItem->effectInfo.effectType - _IET_EXP_BOOST;

				item_sn = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[SelectedInvenItemListIndex].item_sn;

				// 장착된(사용중인) 아이템인지 알아낸다.
				for (INT i = 0; i < MAX_EFFECTSET_SIZE; i++)
					if ( item_sn == _StateController->PlayerInfo.PlayerInfo.itemInfo.effectSet[i] )
					{
						bUsedItem = true;
						break;
					}

//				debugf(TEXT("slot[%d], bUsedItem[%d], SelectedInvenItemListIndex[%d], pItem->effectInfo.effectType[%d]"), 
//					slot, bUsedItem, SelectedInvenItemListIndex, pItem->effectInfo.effectType);

				if ( bUsedItem )
					GetAvaNetRequest()->InvenUnsetEffect(slot, SelectedInvenItemListIndex);
				else
					GetAvaNetRequest()->InvenSetEffect(slot, SelectedInvenItemListIndex);
			}

			bUseCashItemTrigger = false;
		}

		// 캐쉬 충전.
		if ( bChargeCashTrigger )
		{
			extern UavaNetHandler *GavaNetHandler;
			GavaNetHandler->WICOpenChargeWindow();

//			debugf(TEXT("WICOpenChargeWindow"));

			bChargeCashTrigger = false;
		}

		if ( bGetCashTrigger )
		{
			extern UavaNetHandler *GavaNetHandler;
			GavaNetHandler->WICGetCash();

//			debugf(TEXT("WICGetCash"));

			bGetCashTrigger = false;
		}
	}
}

UBOOL UUIDataProvider_AvaNetCashItems::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	return TRUE;
}

UBOOL UUIDataProvider_AvaNetCashItems::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	return TRUE;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetCashItems - UUIDataProvider_AvaNetCashItemInventory
========================================================================================================== */

IMPLEMENT_CLASS( UUIDataProvider_AvaNetCashItemInventory );

/* === IUIListElement interface === */

UBOOL UUIDataProvider_AvaNetCashItemInventory::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );

	// 선택된 리스트에서 해당하는 조건에 맞춰서 out_Elements를 채워준다.

	if ( GIsEditor && !GIsGame )
	{
		if( FieldName == TEXT("Inventory") )
		{
			// 현재 선택된 버튼의 종류에 따라서 해당 아이템을 보여준다.
			for( INT i = 0 ; i < 10; i++ )
				out_Elements.AddItem(i);
		}
		else if ( FieldName == TEXT("ActiveItems") || 
			FieldName == TEXT("ActiveXpItem") || 
			FieldName == TEXT("ActiveSupplyItem") || 
			FieldName == TEXT("ActiveMoneyItem") )
		{
			INT slot = 0;

			if ( FieldName == TEXT("ActiveXpItem") )
				slot = 0;
			else if ( FieldName == TEXT("ActiveSupplyItem") )
				slot = 1;
			else if ( FieldName == TEXT("ActiveMoneyItem") )
				slot = 2;

			out_Elements.AddItem(slot);
		}
		else if( FieldName == TEXT("SelectedInvenItem") )
		{
			out_Elements.AddItem(0);
		}
	}
	else
	{
		static const INT EffectTypeUI2NET[] =
		{
			_IET_NONE,
			_IET_NONE,
			_IET_EXP_BOOST,
			_IET_SP_BOOST,
			_IET_MONEY_BOOST,
		};

		if( FieldName == TEXT("Inventory") )
		{
			EFFECT_ITEM_DESC*	pItem = NULL;
			TID_ITEM			id;

			// 인벤토리를 순회하면서 해당하는 아이템만 보여준다.
			for( INT i = 0; i < MAX_EFFECT_INVENTORY_SIZE; i++)
			{
				id = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[i].id;
				// 더이상 인벤토리에 아이템이 없다면 빠져나온다.
				if ( id == ID_INVALID_ITEM )
					continue;

//				debugf(TEXT("CashInven[%d] = id[%d]"), i, _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[i].id);

				pItem = _ItemDesc().GetEffectItem(id);
				if ( pItem == NULL )
				{
					warnf(TEXT("EffectItem-ID[%d] doesn't has the description.(invalid id)"), id);
					continue;
				}

				if ( CurrentEffectType == CASHITEMEFFECT_Xp ||
					 CurrentEffectType == CASHITEMEFFECT_Supply ||
					 CurrentEffectType == CASHITEMEFFECT_Money )
				{
					if ( pItem->effectInfo.effectType == EffectTypeUI2NET[CurrentEffectType] )
						out_Elements.AddItem(i);
				}
				else if ( CurrentEffectType == CASHITEMEFFECT_All )
					out_Elements.AddItem(i);
				else if ( CurrentEffectType == CASHITEMEFFECT_HotAndNew )
					out_Elements.AddItem(i);
				else
					out_Elements.AddItem(i);
			}
		}
		else if ( FieldName == TEXT("ActiveItems") || 
				  FieldName == TEXT("ActiveXpItem") || 
				  FieldName == TEXT("ActiveSupplyItem") || 
				  FieldName == TEXT("ActiveMoneyItem") )
		{
			INT slot = 0;

			if ( FieldName == TEXT("ActiveXpItem") )
				slot = 0;
			else if ( FieldName == TEXT("ActiveSupplyItem") )
				slot = 1;
			else if ( FieldName == TEXT("ActiveMoneyItem") )
				slot = 2;

			// slot에 해당하는 아이템 고유번호를 얻는다.
			TSN_ITEM item_sn = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectSet[slot];

			for( INT i = 0; i < MAX_EFFECT_INVENTORY_SIZE; i++)
			{
				if ( _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[i].id == ID_INVALID_ITEM )
					continue;

				// 해당 슬롯의 고유번호와 같은 아이템을 인벤토리에서 찾아서 해당 index를 넣어준다.
				if ( _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[i].item_sn == item_sn )
				{
					// Xp, Supply, Money에 따라서 사용하는 아이템 인덱스를 넣어준다.
					out_Elements.AddItem(i);
					break;
				}
			}
		}
		else if( FieldName == TEXT("SelectedInvenItem") )
		{
			// 인벤토리에서 선택된 아이템 인덱스를 넣어준다.
			if ( SelectedItemListIndex >= 0 )
				out_Elements.AddItem(SelectedItemListIndex);
		}
	}

	return TRUE;
}

//! UI Editor에서 보여지는 DataFields.(F7를 누르면 나오는 에디터)
void UUIDataProvider_AvaNetCashItemInventory::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( TEXT("Inventory"), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( TEXT("ActiveItems"), DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField( TEXT("ActiveXpItem"), DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField( TEXT("ActiveSupplyItem"), DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField( TEXT("ActiveMoneyItem"), DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField( TEXT("SelectedInvenItem"), DATATYPE_Collection);

	new(out_Fields) FUIDataProviderField( TEXT("SelectedItemPreview"), DATATYPE_Property );
	new(out_Fields) FUIDataProviderField( TEXT("SelectedItemButton"), DATATYPE_Property );
}

void UUIDataProvider_AvaNetCashItemInventory::GetElementCellTags( TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(FName(TEXT("Type")), TEXT("Type"));
	out_CellTags.Set(FName(TEXT("Name")), TEXT("Name"));
	out_CellTags.Set(FName(TEXT("Durability")), TEXT("Durability"));
	out_CellTags.Set(FName(TEXT("Icon")), TEXT("Icon"));
	out_CellTags.Set(FName(TEXT("UseIcon")), TEXT("UseIcon"));
	out_CellTags.Set(FName(TEXT("UseFlag")), TEXT("UseFlag"));
}

UBOOL UUIDataProvider_AvaNetCashItemInventory::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	// FName의 operator==()를 이용해서 빠른 비교를 위해서 미리 선언...
	static const FName sType( TEXT("Type") );
	static const FName sName( TEXT("Name") );
	static const FName sDurability( TEXT("Durability") );
	static const FName sIcon( TEXT("Icon") );
	static const FName sUseIcon( TEXT("UseIcon") );
	static const FName sUseFlag( TEXT("UseFlag") );

	if( GIsEditor && !GIsGame )
	{
		if( CellTag == TEXT("Type") )
		{
			TCHAR *TestStr[] = { TEXT("XP"), TEXT("Supply"), TEXT("Money") };
			out_FieldValue.StringValue = Localize(TEXT("UIInventoryScene"), 
												  *FString::Printf(TEXT("Text_CashItem_Category[%s]"), TestStr[ListIndex % 3]), 
												  TEXT("avaNet"));
		}
		else if( CellTag == TEXT("Name") )
		{
			out_FieldValue.StringValue = FString::Printf(TEXT("Item%02d"), ListIndex);
		}
		else if( CellTag == TEXT("Durability") )
		{
			INT TempIndex = 1 + ListIndex % 4;
			switch(TempIndex)
			{
				case 1: out_FieldValue.StringValue = GetItemGaugeIconString( _IGT_MAINTENANCE, ITEM_LIMIT_INITED / 2 ); break;
				case 2: out_FieldValue.StringValue = GetItemGaugeIconString( _IGT_DURABILITY, ITEM_LIMIT_INITED / 2 ); break;
				case 3: out_FieldValue.StringValue = GetItemGaugeIconString( _IGT_DATE, 7 * 3600 ); break;
				case 4: out_FieldValue.StringValue = GetItemGaugeIconString( _IGT_DATE, 7 * 24 * 3600 ); break;
			}
		}
		else if( CellTag == TEXT("Icon") )
		{
			TCHAR TempIconCode[] = { TEXT('s') + ListIndex % 4, TEXT('\0') };
			out_FieldValue.StringValue = GetIconCodeString(TempIconCode,TEXT("Icon_Common_EffectIconFont"));
		}
		else if( CellTag == TEXT("UseIcon") )
		{
			out_FieldValue.StringValue = ListIndex & 1 ? Localize(TEXT("UIInventoryScene"), TEXT("Icon_UseCashItem"),TEXT("avaNet")) : TEXT(" ");
		}
		else if( CellTag == TEXT("UseFlag") )
		{
			out_FieldValue.StringValue = TEXT("UseFlag");
		}
	}
	else
	{
		EFFECT_ITEM_DESC*	pItem = NULL;
		EFFECT_ITEM_INFO*	pItemInfo = NULL;
		TID_ITEM			id;
		TSN_ITEM			item_sn;
		UBOOL				bUsedItem = false;

		if ( (id = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[ListIndex].id) != ID_INVALID_ITEM )
			pItem = _ItemDesc().GetEffectItem(id);

		if ( pItem )
		{
			if( CellTag == TEXT("Type") )
			{
				out_FieldValue.StringValue = GetItemSlotTypeString(pItem->id, pItem->slotType);
			}
			else if( CellTag == TEXT("Name") )
			{
				out_FieldValue.StringValue = pItem->GetName();
			}
			else if( CellTag == TEXT("Durability") )
			{
				pItemInfo = &_StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[ListIndex];

				out_FieldValue.StringValue = GetItemGaugeIconString( pItem->gaugeType, pItemInfo->limit );
			}
			else if( CellTag == TEXT("Icon") )
			{
				out_FieldValue.StringValue = GetIconCodeString(pItem);
			}
			else if( CellTag == sUseIcon || CellTag == sUseFlag )
			{
				// 아이템의 SN을 얻어서.
				item_sn = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[ListIndex].item_sn;

				// 장착된(사용중인) 아이템인지 알아낸다.
				for (INT i = 0; i < MAX_EFFECTSET_SIZE; i++)
					if ( item_sn == _StateController->PlayerInfo.PlayerInfo.itemInfo.effectSet[i] )
					{
						bUsedItem = true;
						break;
					}

				if ( CellTag == sUseIcon )
					out_FieldValue.StringValue = bUsedItem ? Localize(TEXT("UIInventoryScene"), TEXT("Icon_UseCashItem"),TEXT("avaNet")) : TEXT(" ");
				else
					out_FieldValue.StringValue = bUsedItem ? TEXT("Used") : TEXT("Unused");
			}
		}
	}

	if( out_FieldValue.StringValue.Len() == 0 )
	{
		out_FieldValue.StringValue = GNone;
	}

	return TRUE;
}

UBOOL UUIDataProvider_AvaNetCashItemInventory::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( GIsEditor && !GIsGame )
	{
		if( FieldName == TEXT("SelectedItemPreview") )
			out_FieldValue.StringValue = TEXT("inventory item preview");
		else if ( FieldName == TEXT("SelectedItemButton") )
			out_FieldValue.StringValue = Localize( TEXT("UIInventoryScene"), TEXT("Text_Button_Use"), TEXT("avaNet") );
	}
	else
	{
		EFFECT_ITEM_DESC*	pItem = NULL;
		TID_ITEM			id;
		TSN_ITEM			item_sn;

		if( FieldName == TEXT("SelectedItemPreview") )
		{
			if ( SelectedItemListIndex >= 0 )
			{
				if ( (id = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[SelectedItemListIndex].id) != ID_INVALID_ITEM )
					pItem = _ItemDesc().GetEffectItem(id);
			}

			if ( pItem )
				out_FieldValue.StringValue = pItem->GetDescription();
			else
				out_FieldValue.StringValue = TEXT(" ");
		}
		else if ( FieldName == TEXT("SelectedItemButton") )
		{
			UBOOL	bUsedItem = false;
			TCHAR*	ButtonStrings[] = { TEXT("Text_Button_Use"), TEXT("Text_Button_Release") };

			if ( SelectedItemListIndex >= 0 )
			{
				// 아이템의 SN을 얻어서.
				item_sn = _StateController->PlayerInfo.PlayerInfo.itemInfo.effectInven[SelectedItemListIndex].item_sn;

				// 장착된(사용중인) 아이템인지 알아낸다.
				for (INT i = 0; i < MAX_EFFECTSET_SIZE; i++)
					if ( item_sn == _StateController->PlayerInfo.PlayerInfo.itemInfo.effectSet[i] )
					{
						bUsedItem = true;
						break;
					}
			}

			// 사용중이면 '해제'버튼을...(거꾸로 표시)
			out_FieldValue.StringValue = Localize( TEXT("UIInventoryScene"), ButtonStrings[bUsedItem], TEXT("avaNet") );
		}
	}

	return true;
}


/* ==========================================================================================================
	UUIDataProvider_AvaNetCashItems - UUIDataProvider_AvaNetCashItemShop
========================================================================================================== */

IMPLEMENT_CLASS( UUIDataProvider_AvaNetCashItemShop );

/* === IUIListElement interface === */

//! 임시로 _ItemDesc()에서 ListIndex로 얻는 함수가 만들어 지기 전까지 사용할 배열.
static const INT InternalCashItems[] = 
{
	11551, 
	11561, 
	11571, 
	11591, 
	11601, 
	11611, 
};

enum EGlobalIndexType
{
	GIT_EFFECT,
	GIT_ITEM,
	GIT_NONE,

	GIT_BASEMASK	= 0x4000,
	GIF_LOCALMASK	= GIT_BASEMASK - 1,
};

//! ListIndex를 Type에 맞춰서 GlobalIndex로 변환시켜 준다.
inline INT ListIndexToGlobalIndex(int index, int type)
{
	return index | (GIT_BASEMASK << type);
}

//! GlobalIndex를 ListIndex로 변환시켜 준다.
inline INT GlobalIndexToListIndex(int index)
{
	return index & GIF_LOCALMASK;
}

//! GlobalIndex의 type을 얻어온다.
inline EGlobalIndexType GlobalIndexType(int index)
{
	int type = 0;

#if !FINAL_RELEASE
	check( index != 0 );
#endif

	for (DWORD typemask = GIT_BASEMASK; typemask < 0x100000000; typemask<<=1, type++ )
	{
		if ( typemask & index )
			return (EGlobalIndexType)type;
	}

	return GIT_NONE;
}

UBOOL UUIDataProvider_AvaNetCashItemShop::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );

	if ( GIsEditor && !GIsGame )
	{
		if( FieldName == TEXT("Shop") )
		{
			for( INT i = 0 ; i < 10; i++ )
				out_Elements.AddItem(i);
		}
		else if ( FieldName == TEXT("SelectedShopItem") )
			out_Elements.AddItem(0);
	}
	else
	{
		if( FieldName == TEXT("Shop") )
		{
			static const INT EffectTypeUI2NET[] =
			{
				_IET_NONE,
				_IET_NONE,
				_IET_EXP_BOOST,
				_IET_SP_BOOST,
				_IET_MONEY_BOOST,
			};

			FavaShopItem *pItem = NULL;
			EFFECT_ITEM_DESC *pEffectItemDesc = NULL;

			for( INT i = 0 ; (pItem = _ShopDesc().GetEffectItemByIndex(i)) != NULL ; i++ )
			{
				if ( CurrentEffectType == CASHITEMEFFECT_HotAndNew )
				{
					// Hot&New가 아닌 목록 제외.
					if ( pItem->DisplayType != _IDT_HOT && pItem->DisplayType != _IDT_NEW )
						continue;
				}
				else
				{
					// 보이지 않는 목록 제외.
					if ( pItem->DisplayType == _IDT_NONE )
						continue;
				}

				pEffectItemDesc = _ItemDesc().GetEffectItem(pItem->GetDefaultItemID());

				if ( CurrentEffectType == CASHITEMEFFECT_Xp ||
					CurrentEffectType == CASHITEMEFFECT_Supply ||
					CurrentEffectType == CASHITEMEFFECT_Money )
				{
					if ( pEffectItemDesc->effectInfo.effectType == EffectTypeUI2NET[CurrentEffectType] )
						out_Elements.AddItem( ListIndexToGlobalIndex(i, GIT_EFFECT) );
				}
				else if ( CurrentEffectType == CASHITEMEFFECT_All )
					out_Elements.AddItem( ListIndexToGlobalIndex(i, GIT_EFFECT) );
				else if ( CurrentEffectType == CASHITEMEFFECT_HotAndNew )
					out_Elements.AddItem( ListIndexToGlobalIndex(i, GIT_EFFECT) );
				else
					out_Elements.AddItem( ListIndexToGlobalIndex(i, GIT_EFFECT) );
			}

			// 다른 캐쉬 아이템도 찾아서 추가한다.
			if ( CurrentEffectType == CASHITEMEFFECT_All || CurrentEffectType == CASHITEMEFFECT_HotAndNew )
			{
				ITEM_DESC *pItemDesc = NULL;

				for( INT i = 0 ; (pItem = _ShopDesc().GetItemByIndex(i)) != NULL ; i++ )
				{
					//	debugf(TEXT("Item: Index[%d] Name[%s] show(%d)"), i, pItem->GetName(), pItem->shopDisplay);

					if ( CurrentEffectType == CASHITEMEFFECT_All )
					{
						// 보이지 않는 목록 제외.
						if ( pItem->DisplayType == _IDT_NONE )
							continue;
					}
					else if ( CurrentEffectType == CASHITEMEFFECT_HotAndNew )
					{
						// Hot&New가 아닌 목록 제외.
						if ( pItem->DisplayType != _IDT_HOT && pItem->DisplayType != _IDT_NEW )
							continue;
					}

					pItemDesc = _ItemDesc().GetItem(pItem->GetDefaultItemID());

					// 표시되지 말아야 하거나, 캐쉬 아이템이 아닌 경우에는 무시한다.
					if ( pItemDesc->priceType != _IPT_CASH )
						continue;

					out_Elements.AddItem( ListIndexToGlobalIndex(i, GIT_ITEM) );
				}
			}
		}
		else if ( FieldName == TEXT("SelectedShopItem") )
		{
			// 실제로는 GlobalIndex이다.
			if ( SelectedItemListIndex >= 0 )
				out_Elements.AddItem(SelectedItemListIndex);
		}
	}

	return TRUE;
}

void UUIDataProvider_AvaNetCashItemShop::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( TEXT("Shop"), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( TEXT("SelectedShopItem"), DATATYPE_Collection );

	new(out_Fields) FUIDataProviderField( TEXT("SelectedItemPreview"), DATATYPE_Property );

	new(out_Fields) FUIDataProviderField( TEXT("ButtonText_Buy"), DATATYPE_Property);
	new(out_Fields) FUIDataProviderField( TEXT("DurabilityOptions"), DATATYPE_Property);

	//! kismet에서 ComboBox에 들어가는 옵션의 개수를 얻기 위해서 추가됨.
	new(out_Fields) FUIDataProviderField( TEXT("OptionCount"), DATATYPE_Property);
}

void UUIDataProvider_AvaNetCashItemShop::GetElementCellTags( TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(FName(TEXT("Name")), TEXT("Name"));
	out_CellTags.Set(FName(TEXT("Icon")), TEXT("Icon"));
	out_CellTags.Set(FName(TEXT("Cost")), TEXT("Cost"));
	out_CellTags.Set(FName(TEXT("Type")), TEXT("Type"));
	out_CellTags.Set(FName(TEXT("DateLimit")), TEXT("DateLimit"));
	out_CellTags.Set(FName(TEXT("BonusMoney")), TEXT("BonusMoney"));
}

UBOOL UUIDataProvider_AvaNetCashItemShop::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	if ( GIsEditor && !GIsGame )
	{
		if( CellTag == TEXT("Name") )
		{
			out_FieldValue.StringValue = FString::Printf(TEXT("Item%02d"), ListIndex);
		}
		else if( CellTag == TEXT("Icon") )
		{
			TCHAR TempIconCode[] = { TEXT('s') + ListIndex % 4, TEXT('\0') };
			out_FieldValue.StringValue = GetIconCodeString(TempIconCode,TEXT("Icon_Common_EffectIconFont"));
		}
		else if( CellTag == TEXT("Cost") )
		{
			out_FieldValue.StringValue = TEXT("Cost");
		}
		else if( CellTag == TEXT("Type") )
		{
			TCHAR *TestStr[] = { TEXT("XP"), TEXT("Supply"), TEXT("Money") };
			out_FieldValue.StringValue = Localize(TEXT("UIInventoryScene"), *FString::Printf(TEXT("Text_CashItem_Category[%s]"), TestStr[ListIndex % 3]), TEXT("avaNet"));
		}
		else if( CellTag == TEXT("DateLimit") )
		{
			out_FieldValue.StringValue = GetDateLimitString(7);
		}
		else if( CellTag == TEXT("BonusMoney") )
		{
			out_FieldValue.StringValue = GetBonusMoneyString(1000, TRUE);
		}
	}
	else
	{
		UBOOL bSelected = (SelectedItemListIndex == ListIndex);

//		debugf(TEXT("SelectedItemListIndex(%d), ListIndex(%d)"), SelectedItemListIndex, ListIndex);

		// 인덱스의 종류를 얻어온다.
		EGlobalIndexType eType = GlobalIndexType(ListIndex);

		// 실제 타입의 LocalIndex(??)로 변환한다.
		INT LocalIndex = GlobalIndexToListIndex(ListIndex);

		FavaShopItem *pItem = NULL;
		FavaShopItem::FavaShopOption *pOption = NULL;

		if ( eType == GIT_EFFECT )
		{
			EFFECT_ITEM_DESC *pItemDesc = NULL;

			if ( LocalIndex >= 0 )
			{
				pItem = _ShopDesc().GetEffectItemByIndex( LocalIndex );
				if ( pItem )
					pItemDesc = _ItemDesc().GetEffectItem(pItem->GetItemID(bSelected ? DurabilityListIndex : 0));
			}

			if ( pItemDesc )
			{
				if( CellTag == TEXT("Name") )
				{
					out_FieldValue.StringValue = pItemDesc->GetName();
				}
				else if( CellTag == TEXT("Icon") )
				{
					out_FieldValue.StringValue = GetIconCodeString(pItemDesc);
				}
				else if( CellTag == TEXT("Cost") )
				{
					out_FieldValue.StringValue = GetCashMoneyString((INT)pItemDesc->price);
				}
				else if( CellTag == TEXT("Type") )
				{
					out_FieldValue.StringValue = GetItemSlotTypeString(pItemDesc->id, pItemDesc->slotType);
				}
				else if( CellTag == TEXT("DateLimit") )
				{
					out_FieldValue.StringValue = bSelected && (pItem->Options.Num() > 1) ? TEXT(" ") : pItem->GetOptionName();
					//out_FieldValue.StringValue = bSelected ? TEXT(" ") : GetDateLimitString(pItemDesc->dateLimit, pItemDesc->gaugeType);
				}
				else if( CellTag == TEXT("BonusMoney") )
				{
					out_FieldValue.StringValue = GetBonusMoneyString(0, TRUE);
				}
			}
		}
		else if ( eType == GIT_ITEM )
		{
			ITEM_DESC *pItemDesc = NULL;

			if ( LocalIndex >= 0 )
			{
				pItem = _ShopDesc().GetItemByIndex( LocalIndex );
				if ( pItem )
					pItemDesc = _ItemDesc().GetItem(pItem->GetItemID(bSelected ? DurabilityListIndex : 0));
			}

			if ( pItemDesc )
			{
				if( CellTag == TEXT("Name") )
				{
					out_FieldValue.StringValue = pItemDesc->GetName();
				}
				else if( CellTag == TEXT("Icon") )
				{
					out_FieldValue.StringValue = GetIconCodeString(pItemDesc);
				}
				else if( CellTag == TEXT("Cost") )
				{
					out_FieldValue.StringValue = GetCashMoneyString((INT)pItemDesc->price);
				}
				else if( CellTag == TEXT("Type") )
				{
					out_FieldValue.StringValue = GetItemSlotTypeString(pItemDesc->id, pItemDesc->slotType);
				}
				else if( CellTag == TEXT("DateLimit") )
				{
					out_FieldValue.StringValue = bSelected && (pItem->Options.Num() > 1) ? TEXT(" ") : pItem->GetOptionName();
					//out_FieldValue.StringValue = bSelected ? TEXT(" ") : GetDateLimitString(pItemDesc->dateLimit, pItemDesc->gaugeType);
				}
				else if( CellTag == TEXT("BonusMoney") )
				{
					out_FieldValue.StringValue = GetBonusMoneyString((INT)pItemDesc->bonusMoney, TRUE);
				}
			}
		}

		if ( out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("??");
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = GNone;

	return TRUE;
}

UBOOL UUIDataProvider_AvaNetCashItemShop::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	FName KeyName = *FieldName;

	if( KeyName == TEXT("SelectedItemPreview") )
	{
		if ( GIsEditor && !GIsGame )
		{
			out_FieldValue.StringValue = TEXT("Shop Item Preview");
		}
		else
		{
			// 인덱스의 종류를 얻어온다.
			EGlobalIndexType eType = GlobalIndexType(SelectedItemListIndex);

			// 실제 타입의 LocalIndex(??)로 변환한다.
			INT LocalIndex = GlobalIndexToListIndex(SelectedItemListIndex);

			FavaShopItem *pItem = NULL;

			if ( eType == GIT_EFFECT )
			{
				if ( LocalIndex >= 0 )
					pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
			}
			else if ( eType == GIT_ITEM )
			{
				if ( LocalIndex >= 0 )
					pItem = _ShopDesc().GetItemByIndex(LocalIndex);
			}

			if ( pItem )
				out_FieldValue.StringValue = pItem->GetDescription();

			if ( out_FieldValue.StringValue.Len() == 0 )
				out_FieldValue.StringValue = TEXT(" ");
		}

		return TRUE;
	}
	else if ( KeyName == TEXT("ButtonText_Buy") )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Button_Buy"), TEXT("avaNet"));
		return TRUE;
	}
	else if ( KeyName == TEXT("DurabilityOptions") )
	{
		if ( GIsEditor && !GIsGame )
		{
			out_FieldValue.StringValue = TEXT("Shop Item Durability Options");
		}
		else
		{
			// 인덱스의 종류를 얻어온다.
			EGlobalIndexType eType = GlobalIndexType(SelectedItemListIndex);

			// 실제 타입의 LocalIndex(??)로 변환한다.
			INT LocalIndex = GlobalIndexToListIndex(SelectedItemListIndex);

			FavaShopItem *pItem = NULL;

			if ( eType == GIT_EFFECT )
			{
				if ( LocalIndex >= 0 )
					pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
			}
			else if ( eType == GIT_ITEM )
			{
				if ( LocalIndex >= 0 )
					pItem = _ShopDesc().GetItemByIndex(LocalIndex);
			}

			if ( pItem )
			{
				FString Won = Localize(TEXT("UIGeneral"), TEXT("Text_Won"), TEXT("avaNet"));
				FString Euro = Localize(TEXT("UIGeneral"), TEXT("Text_GameMoney"), TEXT("avaNet"));
				TMONEY	bonusMoney = 0;

				for (INT i = 0; i < pItem->Options.Num(); i++)
				{
					out_FieldValue.StringValue += pItem->Options(i).OptionName;
					out_FieldValue.StringValue += FString::Printf(TEXT(" : %s"), *GetFmtMoneyString(pItem->Options(i).pItem->price)) + Won;

					if ( eType == GIT_EFFECT )
						bonusMoney = ((EFFECT_ITEM_DESC*)pItem->Options(i).pItem)->bonusMoney;
					else if ( eType == GIT_ITEM )
						bonusMoney = ((ITEM_DESC*)pItem->Options(i).pItem)->bonusMoney;

					if ( bonusMoney > 0 && bonusMoney < 0x7FFFFFFF )
						out_FieldValue.StringValue += FString::Printf(TEXT("(+%s %s)"), *Euro, *GetFmtMoneyString(bonusMoney));

					out_FieldValue.StringValue += TEXT("\n");
				}
			}

			if ( out_FieldValue.StringValue.Len() == 0 )
				out_FieldValue.StringValue = TEXT(" ");
		}
		return TRUE;
	}
	else if ( KeyName == TEXT("OptionCount") )
	{
		// 인덱스의 종류를 얻어온다.
		EGlobalIndexType eType = GlobalIndexType(SelectedItemListIndex);

		// 실제 타입의 LocalIndex(??)로 변환한다.
		INT LocalIndex = GlobalIndexToListIndex(SelectedItemListIndex);

		FavaShopItem *pItem = NULL;

		if ( eType == GIT_EFFECT )
		{
			if ( LocalIndex >= 0 )
				pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
		}
		else if ( eType == GIT_ITEM )
		{
			if ( LocalIndex >= 0 )
				pItem = _ShopDesc().GetItemByIndex(LocalIndex);
		}

		if ( pItem )
			out_FieldValue.StringValue = FString::Printf(TEXT("%d"), pItem->Options.Num());

		if ( out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("0");

		return TRUE;
	}

//	debugf(TEXT("### CashItemShop::GetCellFieldValue(%s, %d)"), *FieldName, SelectedItemListIndex);

	// 선택된 셀의 값을 얻어준다.
	return GetCellFieldValue(KeyName, SelectedItemListIndex, out_FieldValue, ArrayIndex);
}

/* ==========================================================================================================
UUIDataProvider_AvaNetCashItems - UUIDataProvider_AvaNetCashItemDurability
========================================================================================================== */

IMPLEMENT_CLASS( UUIDataProvider_AvaNetCashItemDurability );

void UUIDataProvider_AvaNetCashItemDurability::PostUpdateParamters( UBOOL bParmChanged )
{
	if ( bParmChanged )
	{
	}
}

/* === IUIListElement interface === */

void UUIDataProvider_AvaNetCashItemDurability::GetElementCellTags( TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(FName(TEXT("Durability")), *Localize(TEXT("UIGeneral"), TEXT("Text_Durability"), TEXT("avaNet")) );
}

UBOOL UUIDataProvider_AvaNetCashItemDurability::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	INT Days[] = { 7, 15, 30 };
	FString Day = Localize(TEXT("UIGeneral"), TEXT("Text_Day"), TEXT("avaNet"));

	if ( GIsEditor && !GIsGame )
	{
		if( CellTag == TEXT("Durability") )
		{
			out_FieldValue.StringValue = FString::Printf(TEXT("%d%s"), Days[ListIndex], *Day);
			//debugf(TEXT("CashItemDurability: Cell(Durability) ListIndex(%d), Day(%d), %s"), ListIndex, Days[ListIndex], *out_FieldValue.StringValue);
		}
	}
	else
	{
		if ( SelectedShopItemListIndex >= 0 )
		{
			// 인덱스의 종류를 얻어온다.
			EGlobalIndexType eType = GlobalIndexType(SelectedShopItemListIndex);

			// 실제 타입의 LocalIndex(??)로 변환한다.
			INT LocalIndex = GlobalIndexToListIndex(SelectedShopItemListIndex);

			FavaShopItem *pItem = NULL;

			if ( eType == GIT_EFFECT )
				pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
			else if ( eType == GIT_ITEM )
				pItem = _ShopDesc().GetItemByIndex(LocalIndex);

			if ( pItem )
			{
				if( CellTag == TEXT("Durability") )
				{
					out_FieldValue.StringValue = pItem->Options(ListIndex).OptionName;
				}
			}
		}

		if ( out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("??");
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = GNone;

	return TRUE;
}

/* === UIDataProvider interface === */

void UUIDataProvider_AvaNetCashItemDurability::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( TEXT("SelectedItem") );
	new(out_Fields) FUIDataProviderField( TEXT("ShopItemDurability"), DATATYPE_Collection );
}

UBOOL UUIDataProvider_AvaNetCashItemDurability::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	FName fieldName = *FieldName;

	INT Days[] = { 7, 15, 30 };
	FString Day = Localize(TEXT("UIGeneral"), TEXT("Text_Day"), TEXT("avaNet"));

	if ( GIsEditor && !GIsGame )
	{
		if ( fieldName == TEXT("SelectedItem") )
		{
			out_FieldValue.StringValue = FString::Printf(TEXT("%d%s"), Days[0], *Day);
			return TRUE;
		}		
	}
	else
	{
		if ( fieldName == TEXT("SelectedItem") )
		{
			// 여기서 DurabilityListIndex값이 현재 선택된 SelectedShopItemListIndex에서
			// 최대, 최소값을 얻어와서 짤라줘야 한다.
			if ( SelectedShopItemListIndex >= 0 )
			{
				// 인덱스의 종류를 얻어온다.
				EGlobalIndexType eType = GlobalIndexType(SelectedShopItemListIndex);

				// 실제 타입의 LocalIndex(??)로 변환한다.
				INT LocalIndex = GlobalIndexToListIndex(SelectedShopItemListIndex);

				FavaShopItem *pItem = NULL;

				if ( eType == GIT_EFFECT )
					pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
				else if ( eType == GIT_ITEM )
					pItem = _ShopDesc().GetItemByIndex(LocalIndex);

				if ( pItem )
					out_FieldValue.StringValue = pItem->Options(DurabilityListIndex).OptionName;
			}

//			out_FieldValue.StringValue = FString::Printf(TEXT("%d%s"), Days[DurabilityListIndex], *Day);
			return TRUE;
		}
	}

	return TRUE;
//	return GetCellFieldValue(*FieldName, INDEX_NONE, out_FieldValue, ArrayIndex);
}

/* === IUIListElement interface === */

UBOOL UUIDataProvider_AvaNetCashItemDurability::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	if ( GIsEditor && !GIsGame )
	{
		for(int i = 0; i < 3; i++)
			out_Elements.AddItem(i);
	}
	else
	{
		// SelectedShopItemListIndex에서 내구도의 개수를 얻어와서
		// DurabilityListIndex값을 재설정 해줘야 한다.

		if ( SelectedShopItemListIndex >= 0 )
		{
			// 인덱스의 종류를 얻어온다.
			EGlobalIndexType eType = GlobalIndexType(SelectedShopItemListIndex);

			// 실제 타입의 LocalIndex(??)로 변환한다.
			INT LocalIndex = GlobalIndexToListIndex(SelectedShopItemListIndex);

			FavaShopItem *pItem = NULL;

			if ( eType == GIT_EFFECT )
			{
				pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
				if ( pItem )
				{
					for(int i = 0; i < pItem->Options.Num(); i++)
						out_Elements.AddItem(i);
				}
			}
			else if ( eType == GIT_ITEM )
			{
				pItem = _ShopDesc().GetItemByIndex(LocalIndex);
				if ( pItem )
				{
					for(int i = 0; i < pItem->Options.Num(); i++)
						out_Elements.AddItem(i);
				}
			}
		}
	}

	return TRUE;
}


/* ==========================================================================================================
UUIDataProvider_AvaNetCashItems - UUIDataProvider_AvaNetCashItemPopup
========================================================================================================== */

IMPLEMENT_CLASS( UUIDataProvider_AvaNetCashItemPopup );

void UUIDataProvider_AvaNetCashItemPopup::PostUpdateParamters( UBOOL bParmChanged )
{
	if( bParmChanged )
	{
		// 캐쉬 아이템 구매.
		if( bBuyCashItemRequestTrigger )
		{
			// 상점에서 선택된 ListIndex를 이용해서 해당 아이템 ID를 얻어서 구매를 요청한다.
			if ( SelectedItemListIndex >= 0 )
			{
				// 인덱스의 종류를 얻어온다.
				EGlobalIndexType eType = GlobalIndexType(SelectedItemListIndex);

				// 실제 타입의 LocalIndex(??)로 변환한다.
				INT LocalIndex = GlobalIndexToListIndex(SelectedItemListIndex);

				FavaShopItem *pItem = NULL;

				if ( eType == GIT_EFFECT )
					pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
				else if ( eType == GIT_ITEM )
					pItem = _ShopDesc().GetItemByIndex(LocalIndex);

				if ( pItem )
				{
					//debugf(TEXT("CashItem - DurIndex(%d), ItemID(%d)"), DurabilityListIndex, pItem->GetItemID(DurabilityListIndex));
					GetAvaNetHandler()->WICBuyItem(pItem->GetItemID(DurabilityListIndex));
				}
			}

			bBuyCashItemRequestTrigger = FALSE;
		}

		// 선물하기.
		if ( bGiftCashItemRequestTrigger )
		{

			bGiftCashItemRequestTrigger = FALSE;
		}

		if( bChargeCashRequestTrigger )
		{

			bChargeCashRequestTrigger = FALSE;
		}

		if( bUseRequestTrigger )
		{

			bUseRequestTrigger = FALSE;
		}

		// 구매한 아이템을 바로 장착하기.
		if( bEquipRequestTrigger )
		{
			if ( ItemType == "effect" )
			{
				GetAvaNetRequest()->InvenSetEffect(EquipSlot, InvenSlot);
			}
			else if ( ItemType == "equip" )
			{
				GetAvaNetRequest()->InvenSetEquip(EquipSlot, InvenSlot);
			}
			else if ( ItemType == "weapon" )
			{
				GetAvaNetRequest()->InvenSetWeapon(EquipSlot, InvenSlot);
			}
			else
			{
				// 알 수 없는 종류.
				warnf(TEXT("CashItemPopup.bEquipRequestTrigger - Unknown ItemType (%s)"), *ItemType);
			}

			bEquipRequestTrigger = FALSE;
		}
	}
}


/* === IUIListElement interface === */

void UUIDataProvider_AvaNetCashItemPopup::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( TEXT("ItemName"), DATATYPE_Property );
	new(out_Fields) FUIDataProviderField( TEXT("ItemIcon"), DATATYPE_Property );
	new(out_Fields) FUIDataProviderField( TEXT("ItemDesc"), DATATYPE_Property );
	new(out_Fields) FUIDataProviderField( TEXT("ItemPrice"), DATATYPE_Property );
	new(out_Fields) FUIDataProviderField( TEXT("ItemType"), DATATYPE_Property );
	new(out_Fields) FUIDataProviderField( TEXT("NotEnoughCash"), DATATYPE_Property );	// 캐쉬아이템을 모자른 캐쉬
	new(out_Fields) FUIDataProviderField( TEXT("NextCash"), DATATYPE_Property );		// 캐쉬아이템을 사고남은 캐쉬
	new(out_Fields) FUIDataProviderField( TEXT("PrevCash"), DATATYPE_Property );		// 캐쉬아이템을 사기전 캐쉬
	new(out_Fields) FUIDataProviderField( TEXT("BonusMoney"), DATATYPE_Property );		// 아이템으로 받을 보너스 게임머니
	new(out_Fields) FUIDataProviderField( TEXT("PrevMoney"), DATATYPE_Property );		// 아이템으로 보너스 게임머니를 받기전 가지고있던 게임머니
	new(out_Fields) FUIDataProviderField( TEXT("DateLimit"), DATATYPE_Property );		// 날짜 제한.
}

UBOOL UUIDataProvider_AvaNetCashItemPopup::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	const INT SamplePrice = 1000;
	const INT SampleMoney = 10000;

	if ( GIsEditor && !GIsGame )
	{
		if( FieldName == TEXT("ItemName") )
		{
			out_FieldValue.StringValue = TEXT("ItemName");
		}
		else if( FieldName == TEXT("ItemIcon") )
		{
			out_FieldValue.StringValue = GetIconCodeString(TEXT("s"),TEXT("Icon_Common_EffectIconFont"));
		}
		else if( FieldName == TEXT("ItemDesc") )
		{
			out_FieldValue.StringValue = TEXT("ItemDesc");
		}
		else if( FieldName == TEXT("ItemPrice") )
		{
			out_FieldValue.StringValue = appItoa(SamplePrice);
		}
		else if( FieldName == TEXT("ItemType") )
		{
			out_FieldValue.StringValue = Localize(TEXT("UIInventoryScene"), TEXT("Text_Label_BuyCashItem_Type[Durability]"), TEXT("avaNet"));
		}
		else if( FieldName == TEXT("NotEnoughCash") )
		{
			out_FieldValue.StringValue = appItoa( Max(GetMyPlayerCash() - SamplePrice, 0) );
		}
		else if( FieldName == TEXT("NextCash") )
		{
			out_FieldValue.StringValue = appItoa(GetMyPlayerCash() - SamplePrice);
		}
		else if( FieldName == TEXT("PrevCash") )
		{
			out_FieldValue.StringValue = appItoa(GetMyPlayerCash() + SamplePrice);
		}
		else if( FieldName == TEXT("BonusMoney") )
		{
			out_FieldValue.StringValue = GetBonusMoneyString(SamplePrice);
		}
		else if( FieldName == TEXT("PrevMoney") )
		{
			out_FieldValue.StringValue = appItoa(GetMyPlayerMoney() - SampleMoney);
		}
		else if( FieldName == TEXT("DateLimit") )
		{
			out_FieldValue.StringValue = GetDateLimitString(7);
		}
	}
	else if (SelectedItemListIndex >= 0)
	{
		// 인덱스의 종류를 얻어온다.
		EGlobalIndexType eType = GlobalIndexType(SelectedItemListIndex);

		// 실제 타입의 LocalIndex(??)로 변환한다.
		INT LocalIndex = GlobalIndexToListIndex(SelectedItemListIndex);

		if ( eType == GIT_EFFECT )
		{
			FavaShopItem*		pItem = NULL;
			EFFECT_ITEM_DESC*	pItemDesc = NULL;

			// 상점에서 선택한 인덱스로 아이템의 정보를 얻어온다.
			if ( LocalIndex >= 0 )
			{
				pItem = _ShopDesc().GetEffectItemByIndex(LocalIndex);
				if ( pItem )
					pItemDesc = _ItemDesc().GetEffectItem(pItem->GetItemID(DurabilityListIndex));
			}

			if ( pItemDesc )
			{
				if( FieldName == TEXT("ItemName") )
				{
					out_FieldValue.StringValue = pItemDesc->GetName();
				}
				else if( FieldName == TEXT("ItemIcon") )
				{
					out_FieldValue.StringValue = GetIconCodeString(pItemDesc);
				}
				else if( FieldName == TEXT("ItemDesc") )
				{
					out_FieldValue.StringValue = pItemDesc->GetDescription();
				}
				else if( FieldName == TEXT("ItemPrice") )
				{
					out_FieldValue.StringValue = appItoa(pItemDesc->price);
				}
				else if( FieldName == TEXT("ItemType") )
				{
					out_FieldValue.StringValue = GetItemSlotTypeString(pItemDesc->id, pItemDesc->slotType);
				}
				else if( FieldName == TEXT("NotEnoughCash") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString( Min(GetMyPlayerCash() - (INT)pItemDesc->price, 0) );
				}
				else if( FieldName == TEXT("NextCash") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString( Max(GetMyPlayerCash() - (INT)pItemDesc->price, 0) );
				}
				else if( FieldName == TEXT("PrevCash") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString(GetMyPlayerCash());
				}
				else if( FieldName == TEXT("BonusMoney") )
				{
					out_FieldValue.StringValue = GetBonusMoneyString(0);
				}
				else if( FieldName == TEXT("PrevMoney") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString(GetMyPlayerMoney() - 0);
				}
				else if( FieldName == TEXT("DateLimit") )
				{
					out_FieldValue.StringValue = pItem->GetOptionName(DurabilityListIndex);
					//out_FieldValue.StringValue = GetDateLimitString(pItemDesc->dateLimit, pItemDesc->gaugeType);
				}
			}
		}
		else if ( eType == GIT_ITEM )
		{
			FavaShopItem*	pItem = NULL;
			ITEM_DESC*		pItemDesc = NULL;

			// 상점에서 선택한 인덱스로 아이템의 정보를 얻어온다.
			if ( LocalIndex >= 0 )
			{
				pItem = _ShopDesc().GetItemByIndex(LocalIndex);
				if ( pItem )
					pItemDesc = _ItemDesc().GetItem(pItem->GetItemID(DurabilityListIndex));
			}

			if ( pItemDesc )
			{
				if( FieldName == TEXT("ItemName") )
				{
					out_FieldValue.StringValue = pItemDesc->GetName();
				}
				else if( FieldName == TEXT("ItemIcon") )
				{
					out_FieldValue.StringValue = GetIconCodeString(pItemDesc);
				}
				else if( FieldName == TEXT("ItemDesc") )
				{
					out_FieldValue.StringValue = pItemDesc->GetDescription();
				}
				else if( FieldName == TEXT("ItemPrice") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString(pItemDesc->price);
				}
				else if( FieldName == TEXT("ItemType") )
				{
					out_FieldValue.StringValue = GetItemSlotTypeString(pItemDesc->id, pItemDesc->slotType);
				}
				else if( FieldName == TEXT("NotEnoughCash") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString( Min(GetMyPlayerCash() - (INT)pItemDesc->price, 0) );
				}
				else if( FieldName == TEXT("NextCash") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString( Max(GetMyPlayerCash() - (INT)pItemDesc->price, 0) );
				}
				else if( FieldName == TEXT("PrevCash") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString(GetMyPlayerCash());
				}
				else if( FieldName == TEXT("BonusMoney") )
				{
					out_FieldValue.StringValue = GetBonusMoneyString((INT)pItemDesc->bonusMoney);
				}
				else if( FieldName == TEXT("PrevMoney") )
				{
					out_FieldValue.StringValue = GetFmtMoneyString(GetMyPlayerMoney() - pItemDesc->bonusMoney);
				}
				else if( FieldName == TEXT("DateLimit") )
				{
					out_FieldValue.StringValue = pItem->GetOptionName(DurabilityListIndex);
					//out_FieldValue.StringValue = GetDateLimitString(pItemDesc->dateLimit, pItemDesc->gaugeType);
				}
			}
		}
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetLastGameResult
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetLastGameResult)

void UUIDataProvider_AvaNetLastGameResult::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( TEXT("LastGamePlayersEU"), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( TEXT("LastGamePlayersNRF"), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( TEXT("LastGamePlayers"), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( TEXT("LastGamePlayersMD"), DATATYPE_Collection );
}

void UUIDataProvider_AvaNetLastGameResult::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Rank")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Rank"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Level")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Level"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("ClanName")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_ClanName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("ClanMark")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_ClanMark"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("NickName")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_NickName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("NickNameCombo")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_NickName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Score")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Score"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Death")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Death"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("EXP")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_EXP"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("EXPCombo")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_EXP"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("Supply")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Supply"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("SupplyCombo")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_Supply"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("IsLevelUp")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_IsLevelUp"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("IsLeader")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_IsLeader"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("PcBang")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_PcBang"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("UpdateMsg")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_UpdateMsg"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("SupportItems")), *Localize(TEXT("UIResultScene"), TEXT("Text_List_SupportItems"), TEXT("avaNet")));
}

UBOOL UUIDataProvider_AvaNetLastGameResult::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (CellTag	== FName(TEXT("Rank")))
		{
			OutFieldValue.StringValue = appItoa(ListIndex+1);
		}
		if (CellTag == FName(TEXT("Level")))
		{
			OutFieldValue.StringValue = TEXT("1");
		}
		else if ( CellTag == FName(TEXT("ClanName")) )
		{
			OutFieldValue.StringValue = TEXT("--");
		}
		else if ( CellTag == FName(TEXT("ClanMark")) )
		{
			OutFieldValue.StringValue = GetClanMarkString(0);
		}
		else if (CellTag == FName(TEXT("NickName")) || CellTag == FName(TEXT("NickNameCombo")))
		{
			FString PlayerNameMod = ( ListIndex == 1) ? Localize(TEXT("UIResultScene"), TEXT("TextMod_ListElem_NickNameItsMe"), TEXT("avaNet")) : FString(TEXT(""));
			FString BIAModStr = (ListIndex == 2) ? Localize(TEXT("UILobbyScene"), TEXT("Text_Mod_BIAPlayer"), TEXT("avaNet")) : FString(TEXT(""));
			OutFieldValue.StringValue =  PlayerNameMod + BIAModStr +  FString::Printf(TEXT("Player%02d"), ListIndex);
			if( CellTag == TEXT("NickNameCombo") )
			{
				FString SquadLeaderIcon = ( ListIndex % 12 == 0 ) ? Localize(TEXT("UIResultScene"), TEXT("Icon_ListElem_SquadLeader"), TEXT("avaNet")) : FString(TEXT(""));
				OutFieldValue.StringValue = SquadLeaderIcon + OutFieldValue.StringValue;
			}
		}
		else if (CellTag == FName(TEXT("Score")))
		{
			OutFieldValue.StringValue = TEXT("15");
		}
		else if (CellTag == FName(TEXT("Death")))
		{
			OutFieldValue.StringValue = TEXT("10");
		}
		else if (CellTag == FName(TEXT("EXP")))
		{
			OutFieldValue.StringValue = TEXT("+500");
		}
		else if (CellTag == FName(TEXT("EXPCombo")))
		{
			OutFieldValue.StringValue = GetXPComboString( 500,40 );
		}
		else if (CellTag == FName(TEXT("Supply")))
		{
			OutFieldValue.StringValue = TEXT("+300");
		}
		else if (CellTag == FName(TEXT("SupplyCombo")))
		{
			OutFieldValue.StringValue = GetSupplyComboString( 10000, 20 );
		}
		else if (CellTag == FName(TEXT("IsLevelUp")))
		{
			OutFieldValue.StringValue = TEXT("+");
		}
		else if (CellTag == FName(TEXT("IsLeader")))
		{
			OutFieldValue.StringValue = TEXT("Leader");
		}
		else if (CellTag == FName(TEXT("PcBang")))
		{
			OutFieldValue.StringValue = TEXT("PC");
		}
		else if (CellTag == FName(TEXT("UpdateMsg")))
		{
			OutFieldValue.StringValue = FString::Printf(TEXT("Update message #%d"), ListIndex);
		}
		else if (CellTag == FName(TEXT("SupportItems")) )
		{
			OutFieldValue.StringValue += GetPCBangIconString();
			OutFieldValue.StringValue += GetSupportItemIcon( Def::_IET_EXP_BOOST, 0);
			OutFieldValue.StringValue += GetSupportItemIcon( Def::_IET_SP_BOOST, 0);
			OutFieldValue.StringValue += GetSupportItemIcon( Def::_IET_MONEY_BOOST, 0);
		}
	}
	else if ( _StateController->LastResultInfo.IsValid() && _StateController->LastResultInfo.RoomResultInfo.IsValidIndex(ListIndex) )
	{
		FLastResultInfo::FPlayerResultInfo *Info = &_StateController->LastResultInfo.RoomResultInfo(ListIndex);
		if ( CellTag == TEXT("Rank") )
		{
			OutFieldValue.StringValue = appItoa(ListIndex + 1);
		}
		if (CellTag == FName(TEXT("Level")))
		{
			if (Info && Info->Level >= 0)
				OutFieldValue.StringValue = GetLevelIconString(Info->Level);
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		else if ( CellTag == FName(TEXT("ClanName")) )
		{
			for(int i = 0; i < Def::MAX_ALL_PLAYER_PER_ROOM; i++)
			{
				if ( Info->idSlot == _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.idSlot )
					OutFieldValue.StringValue = _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.guildName;
			}
			return TRUE;
		}
		else if ( CellTag == FName(TEXT("ClanMark")) )
		{
			for(int i = 0; i < Def::MAX_ALL_PLAYER_PER_ROOM; i++)
			{
				if ( Info->idSlot == _StateController->RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.idSlot )
					OutFieldValue.StringValue = GetClanMarkString( _StateController->RoomInfo.PlayerList.PlayerList[i].GetClanMarkID() );
			}
			return TRUE;
		}
		else if (CellTag == FName(TEXT("NickName")) || CellTag == FName(TEXT("NickNameCombo")))
		{
			FString PlayerName = _StateController->PlayerInfo.PlayerInfo.nickname;
			UBOOL ItsMe = PlayerName.Trim().TrimTrailing() == Info->Nickname.Trim().TrimTrailing();

			FBuddyInfo* BIAInfo = _Communicator().GetBIA();
			UBOOL ItsMyBIA = !ItsMe && BIAInfo && Info && BIAInfo->Nickname.Trim().TrimTrailing() == Info->Nickname.Trim().TrimTrailing();

			FString PlayerNameMod = ItsMe ? Localize(TEXT("UIResultScene"), TEXT("TextMod_ListElem_NickNameItsMe"), TEXT("avaNet")) :
				ItsMyBIA ? Localize(TEXT("UILobbyScene"), TEXT("Text_Mod_BIAPlayer"), TEXT("avaNet")) : FString(TEXT(""));
			OutFieldValue.StringValue = Info ? PlayerNameMod + Info->Nickname : TEXT(" ");

			if( CellTag == TEXT("NickNameCombo") )
			{
				FString SquadLeaderIcon = ( Info && Info->bLeader ) ? Localize(TEXT("UIResultScene"), TEXT("Icon_ListElem_SquadLeader"), TEXT("avaNet")) : FString(TEXT(""));
				OutFieldValue.StringValue = SquadLeaderIcon + OutFieldValue.StringValue;
			}
		}
		else if (CellTag == FName(TEXT("Score")))
		{
			if (Info)
				OutFieldValue.StringValue = appItoa(Info->Score);
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("Death")))
		{
			if (Info && Info->Death >= 0)
				OutFieldValue.StringValue = appItoa(Info->Death);
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("EXP")))
		{
			if (Info && Info->xp >= 0)
				OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), Info->xp);
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("ExpCombo")))
		{
			OutFieldValue.StringValue =	OutFieldValue.StringValue = GetXPComboString( 500,40 );
			if( Info )
				OutFieldValue.StringValue = GetXPComboString( Info->xp , Info->GetBonusXPPerc() + Info->GetPcBangBonusXPPerc() );
			else
				OutFieldValue.StringValue = TEXT("0");
		}
		else if (CellTag == FName(TEXT("Supply")))
		{
			if (Info && Info->SupplyPoint >= 0)
				OutFieldValue.StringValue = FString::Printf(TEXT("%+d"), Info->SupplyPoint);
			else
				OutFieldValue.StringValue = appItoa(0);
		}
		else if (CellTag == FName(TEXT("SupplyCombo")))
		{
			if( Info )
				OutFieldValue.StringValue = GetSupplyComboString( Info->SupplyPoint, Info->GetBonusSupplyPerc() );
			else
				OutFieldValue.StringValue = appItoa(0);
		}
		else if (CellTag == FName(TEXT("IsLevelUp")))
		{
			if (Info)
			{
				OutFieldValue.StringValue = Info->Level > Info->LastLevel ? TEXT("+") : TEXT(" ");
			}
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		else if (CellTag == FName(TEXT("IsLeader")))
		{
			if (Info)
			{
				OutFieldValue.StringValue = Info->bLeader ? TEXT("Leader") : TEXT(" ");
			}
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		else if (CellTag == FName(TEXT("PcBang")))
		{
			if (Info)
			{
				OutFieldValue.StringValue = Info->bPcBang ? TEXT("PC") : TEXT(" ");
			}
			else
				OutFieldValue.StringValue = TEXT(" ");
		}
		//else if (CellTag == FName(TEXT("UpdateMsg")))
		//{
		//	if (ListIndex >= 0 && ListIndex < _StateController->LastResultInfo.UpdateMsgList.Num())
		//		OutFieldValue.StringValue = _StateController->LastResultInfo.UpdateMsgList(ListIndex);
		//	else
		//		OutFieldValue.StringValue = TEXT(" ");
		//}
		else if ( CellTag == FName(TEXT("SupportItems")) )
		{
			if (Info)
			{
				OutFieldValue.StringValue += Info->bPcBang ? *GetPCBangIconString() : TEXT(" ");
				if ( CanApplyLastResult() )
				{
					for( INT i = 0 ; i < Info->EffectList.Num() ; i++)
					{
						EFFECT_ITEM_DESC *pDesc = _ItemDesc().GetEffectItem(Info->EffectList(i));
						if (pDesc)
							OutFieldValue.StringValue += GetSupportItemIcon( pDesc->effectInfo.effectType, 0);
					}

					if( Info->biaXPFlag > _BIAXP_NONE )
						OutFieldValue.StringValue += GetBIAIconString();
				}
			}

			if (OutFieldValue.StringValue.Len() == 0)
				OutFieldValue.StringValue = TEXT(" ");
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}

UBOOL UUIDataProvider_AvaNetLastGameResult::GetListElements( FName FieldName, TArray<INT>& OutElements )
{
	UBOOL bResult = FALSE;

	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );

	if ( FieldName == TEXT("LastGamePlayersEU") ||
		FieldName == TEXT("LastGamePlayersNRF") ||
		FieldName == TEXT("LastGamePlayers") ||
		FieldName == TEXT("LastGamePlayersMD"))
	{
		INT StartIndex = (FieldName == TEXT("LastGamePlayersNRF")) ? MAX_PLAYER_PER_ROOM/ 2 : 0;
		INT EndIndex = (FieldName == TEXT("LastGamePlayersEU")) ? MAX_PLAYER_PER_ROOM/2 : MAX_PLAYER_PER_ROOM;

		// 개인전(MilitaryDrill)의 경우 최대 인원수는 16명
		//if( FieldName == TEXT("LastGamePlayersMD") )
		//	EndIndex = Min(16, EndIndex);
		// 그러나 EU,NRF 슬롯을 따로 쓰기 때문에 5:5라면 (0,1,2,3,4) (12,13,14,15,16) 이렇게 인덱스를 쓸 수 있다.

		if( GIsEditor && !GIsGame )
		{
			for( INT Index = StartIndex ; Index < EndIndex ; Index++ )
				OutElements.AddItem(Index);

		}
		else
		{
			for (INT Index = 0; Index < _StateController->LastResultInfo.RoomResultInfo.Num(); Index++)
			{
				if (_StateController->LastResultInfo.RoomResultInfo(Index).IsValid() &&
					StartIndex <= _StateController->LastResultInfo.RoomResultInfo(Index).idSlot &&
					_StateController->LastResultInfo.RoomResultInfo(Index).idSlot < EndIndex)
				{
					OutElements.AddItem(Index);
				}
			}
		}
		bResult = TRUE;
	}

	return bResult;
}

UBOOL UUIDataProvider_AvaNetLastGameResult::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	return FALSE;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetReadyRoom
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetReadyRoom)

void UUIDataProvider_AvaNetReadyRoom::GetSupportedDataFields( TArray<struct FUIDataProviderField>& OutFields )
{
	new (OutFields) FUIDataProviderField(TEXT("SettingSceneTitle"));
	// Widget Handler ( manipulate state whether enable or disable, visible or not )
	new (OutFields) FUIDataProviderField(TEXT("Handler_TeamChangeEU"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_TeamChangeNRF"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_TeamChangeSpectator"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_ReadyAction"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_StartAction"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_KickAction"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_InviteAction"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_OpenInventory"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_ResetRoomSetting"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_ClanSwapTeam"),DATATYPE_Property);

	GetInternalDataProviderSupportedDataFields( OutFields );
}

UBOOL UUIDataProvider_AvaNetReadyRoom::GetField( const FString& FieldName, struct FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = TRUE;
	FName Field = *FieldName;
	UavaNetHandler* NetHandler = GetAvaNetHandler();

	if( Field == TEXT("SettingSceneTitle") )
	{
		FString KeyStr = _StateController->IsStateInRoom() ? TEXT("Text_ChangeTitle") : TEXT("Text_MakeTitle");
		OutFieldValue.StringValue = Localize(TEXT("UIRoomSettingScene"),*KeyStr,TEXT("avaNet"));
	}
	else if( Field == TEXT("Handler_TeamChangeEU") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_TeamChangeEU"), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_TeamChangeNRF") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_TeamChangeNRF"), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_TeamChangeSpectator") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_Spectate"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_ReadyAction") )
	{
		FString KeyStr = NetHandler->AmIReady() ? TEXT("Text_Button_Cancel_Ready") : TEXT("Text_Button_Ready");
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),*KeyStr,TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_StartAction") )
	{
		FString KeyStr = NetHandler->IsCountingDown() ? TEXT("Text_Button_Cancel_Start") : TEXT("Text_Button_Start");
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),*KeyStr,TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_KickAction") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_Kick"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_InviteAction") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_Invite"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_OpenInventory") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_SelectWeapon"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_ResetRoomSetting") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_Settings"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_ClanSwapTeam") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"),TEXT("Text_Button_SwapTeam"),TEXT("avaNet"));
		bResult = TRUE;
	}

	if( bResult && OutFieldValue.StringValue.Len() == 0 )
		OutFieldValue.StringValue = GNone;

	return bResult;
}

void UUIDataProvider_AvaNetReadyRoom::UpdateWidget( const FString& FieldName, UUIObject* TargetWidget )
{
	check(TargetWidget);
	FName Field = *FieldName;

	if( _StateController->IsStateInRoom() )
	{
		UavaNetHandler* NetHandler = GetAvaNetHandler();
		UavaNetRequest* NetRequest = GetAvaNetRequest();
		UBOOL bRequestSceneScene = FALSE;
		UBOOL bEnable = TargetWidget->IsEnabled();
		UBOOL bVisible = !TargetWidget->IsHidden();

		if( Field == TEXT("Handler_TeamChangeEU") ||
			Field == TEXT("Handler_TeamChangeNRF"))
		{
			FMapInfo* MapInfo = GetSelectedMapInfo();
			if( MapInfo )
			{
				UBOOL bHost = NetHandler->AmIHost();
				UBOOL bReady = NetHandler->AmIReady();
				UBOOL bCount = NetHandler->IsCountingDown();
				// 방장일 경우 카운트다운 중에는 팀을 옮길수 없다
				// 방장이 아닐경우 (준비중이고 카운트다운 중일 때)를 제외하면 옮겨다닐 수 있다
				if( bHost )
					bEnable = !bCount;
				else
					bEnable = !(bCount || bReady);

				bEnable = bEnable && _StateController->GetChannelSetting( EChannelSetting_RoomChangeTeam );
				bEnable = bEnable && MapInfo->MissionType != NMT_MilitaryDrill;
				// 그러나, 개인전(군사훈련)맵에서는 항상 비활성
				//bEnable = MapInfo->MissionType == NMT_MilitaryDrill ? FALSE : bEnable;
			}
		}
		else if ( Field == TEXT("Handler_TeamChangeSpectator") )
		{
			bEnable =  NetHandler->IsSpectatorAllowed(); 
			bVisible = !(GetCurrentChannelGroup() == EChannelFlag_Clan && !bEnable);	// 친선클랜전에서 관전불허시 안보이도록함
		}
		else if ( Field == TEXT("Handler_ReadyAction") )
		{
			bVisible = !NetHandler->AmIHost() && !NetHandler->AmISpectator() && NetRequest->GetCurrentRoomState() == RIP_WAIT;
			bEnable = !NetHandler->IsCountingDown();
			bRequestSceneScene = TRUE;
		}
		else if ( Field == TEXT("Handler_StartAction") )
		{
			bVisible = NetHandler->AmIHost() || NetRequest->GetCurrentRoomState() != RIP_WAIT;
			bRequestSceneScene = TRUE;
		}
		else if ( Field == TEXT("Handler_KickAction") )
		{
			bVisible = NetHandler->AmIHost();
			bEnable = !NetHandler->IsCountingDown();
			bRequestSceneScene = TRUE;
		}
		else if( Field == TEXT("Handler_InviteAction") )
		{
			bVisible = FALSE;
			bEnable = FALSE;
			bRequestSceneScene = TRUE;
		}
		else if( Field == TEXT("Handler_OpenInventory") )
		{
			bEnable = (!NetHandler->AmISpectator() && !NetHandler->IsCountingDown()) && (NetHandler->AmIHost() || !NetHandler->AmIReady());
		}
		else if( Field == TEXT("Handler_ResetRoomSetting") )
		{
			bVisible = NetHandler->AmIHost();
			bEnable = !NetHandler->IsCountingDown();
		}
		else if( Field == TEXT("Handler_ClanSwapTeam") )
		{
			bVisible = NetHandler->AmIHost();
			bEnable = !NetHandler->IsCountingDown();
		}

		if( TargetWidget->IsEnabled() != bEnable )
			TargetWidget->SetEnabled( bEnable );

		if( TargetWidget->IsHidden() == bVisible )
			TargetWidget->eventSetVisibility( bVisible );

		if( bRequestSceneScene )
			TargetWidget->RequestSceneUpdate(FALSE, TRUE);
	}
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetReadyRoom - Players
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetReadyRoomPlayers)

void UUIDataProvider_AvaNetReadyRoomPlayers::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( FName(TEXT("PlayerList")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("PlayerListEU")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("PlayerListNRF")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("PlayerListSpectator")), DATATYPE_Collection );
}

UBOOL UUIDataProvider_AvaNetReadyRoomPlayers::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	return FALSE;
}

UBOOL UUIDataProvider_AvaNetReadyRoomPlayers::GetListElements( FName FieldName, TArray<INT>& OutElements )
{
	UBOOL bResult = FALSE;

	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );

	const INT EachPlayerCount = _StateController->IsStateInRoom() && _StateController->RoomInfo.IsValid() ? 
								_StateController->RoomInfo.RoomInfo.setting.numMax / 2 : MAX_PLAYER_PER_ROOM / 2;

	if( FieldName == TEXT("PlayerList") )
	{
		for( INT Index = 0 ; Index < 0 + EachPlayerCount ; Index++ )
			if( !_StateController->RoomInfo.PlayerList.IsEmpty(Index) )
				OutElements.AddItem(Index);

		for( UINT Index = MAX_PLAYER_PER_ROOM/2 ; Index < MAX_PLAYER_PER_ROOM/2 + EachPlayerCount ; Index++ )
			if( !_StateController->RoomInfo.PlayerList.IsEmpty(Index) )
				OutElements.AddItem(Index);
		bResult = TRUE;
	}
	else if( FieldName == TEXT("PlayerListEU") )
	{
		for( INT Index = 0 ; Index < 0 + EachPlayerCount ; Index++ )
			OutElements.AddItem(Index);
		bResult = TRUE;
	}
	else if( FieldName == TEXT("PlayerListNRF") )
	{
		for( UINT Index = MAX_PLAYER_PER_ROOM/2 ; Index < MAX_PLAYER_PER_ROOM/2 + EachPlayerCount ; Index++ )
			OutElements.AddItem(Index);
		bResult = TRUE;
	}
	else if( FieldName == TEXT("PlayerListSpectator") )
	{
		if( (GIsEditor && !GIsGame) || _StateController->RoomInfo.RoomInfo.setting.allowSpectator > 0 )
		{
			for( INT Index = MAX_PLAYER_PER_ROOM ; Index < MAX_ALL_PLAYER_PER_ROOM ; Index++ )
				OutElements.AddItem(Index);
		}
		bResult = TRUE;
	}

	return bResult;
}

void UUIDataProvider_AvaNetReadyRoomPlayers::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("Level")), *Localize(TEXT("UIRoomScene"), TEXT("Text_List_Level"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("ClanMark")), *Localize(TEXT("UIRoomScene"), TEXT("Text_List_Clan"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("Nickname")), *Localize(TEXT("UIRoomScene"), TEXT("Text_List_NickName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("NicknameCombo")), *Localize(TEXT("UIRoomScene"), TEXT("Text_List_NickName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("Class")), *Localize(TEXT("UIRoomScene"), TEXT("Text_List_Class"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("Ready")),*Localize(TEXT("UIRoomScene"), TEXT("Text_List_Ready"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("PcBang")),*Localize(TEXT("UIRoomScene"), TEXT("Text_List_PcBang"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("RTT")),*Localize(TEXT("UIRoomScene"), TEXT("Text_List_RTT"), TEXT("avaNet")));
}

UBOOL UUIDataProvider_AvaNetReadyRoomPlayers::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	OutFieldValue.PropertyTag = CellTag;
	//@todo joeg - set correct PropertyType here
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (GIsEditor && !GIsGame)
	{
		if (ListIndex >= 0)
		{
			if (CellTag == FName(TEXT("Level")))
			{
				OutFieldValue.StringValue = appItoa(ListIndex % 10 + 1);
			}
			else if (CellTag == FName(TEXT("ClanMark")) )
			{
				OutFieldValue.StringValue = GetClanMarkString(0);
			}
			else if (CellTag == FName(TEXT("Nickname")) )
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("플레이어%02d"), ListIndex);
			}
			else if ( CellTag == FName(TEXT("NicknameCombo")) )
			{
				OutFieldValue.StringValue = FString::Printf(TEXT("플레이어%02d"), ListIndex);
			}
			else if (CellTag == FName(TEXT("Class")))
			{
				OutFieldValue.StringValue = (ListIndex % 3 == 0 ? TEXT("R") : ListIndex % 3 == 1 ? TEXT("P") : TEXT("S"));
			}
			else if (CellTag == FName(TEXT("Ready")))
			{
				OutFieldValue.StringValue = (ListIndex % 3 == 0 ? TEXT(" ") : ListIndex % 3 == 1 ? TEXT("Loading") : TEXT("Ready"));
			}
			else if ( CellTag == FName(TEXT("PcBang")))
			{
				OutFieldValue.StringValue = (ListIndex % 2 == 0 ? TEXT("PC") : TEXT(" "));
			}
			else if ( CellTag == FName(TEXT("RTT")))
			{
				OutFieldValue.StringValue = TEXT("0ms");
			}
		}
	}
	else
	{
		if ( ListIndex >= 0 && ListIndex < Def::MAX_ALL_PLAYER_PER_ROOM )
		{
			FRoomInfo *Room = NULL;

			if (_StateController->IsStateInRoom())
			{
				if (_StateController->RoomInfo.IsValid())
					Room = &(_StateController->RoomInfo);
			}

			if (CellTag == FName(TEXT("Level")))
			{
				if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						BYTE Lev = Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.level;
						OutFieldValue.StringValue = GetLevelIconString(Lev);
					}
				}
				else
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
			}
			else if (CellTag == FName(TEXT("ClanMark")) )
			{
				if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						OutFieldValue.StringValue = GetClanMarkString( Room->PlayerList.PlayerList[ListIndex].GetClanMarkID() );
					}
				}
				else
				{
					OutFieldValue.StringValue = TEXT(" ");
				}

				// Nothing...
				return TRUE;
			}
			else if ( CellTag == FName(TEXT("Nickname")))
			{
				if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						OutFieldValue.StringValue += Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.nickname;
					}
				}
				else
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
			}
			else if (CellTag == FName(TEXT("NicknameCombo")))
			{
				if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						FString NickName = Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.nickname;

						if( Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.pcBang )
							OutFieldValue.StringValue += GetPCBangIconString();
						if( NickName.Trim().TrimTrailing() == FString(_StateController->PlayerInfo.PlayerInfo.nickname).Trim().TrimTrailing() )
							OutFieldValue.StringValue += Localize(TEXT("UIRoomScene"), TEXT("TextMod_ListElem_NickNameItsMe"), TEXT("avaNet"));

						OutFieldValue.StringValue += NickName;
					}
				}
				else
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
			}
			else if (CellTag == FName(TEXT("Class")))
			{
				if (_StateController->GetNetState() == _AN_LOBBY)
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
				else if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						INT c = Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.currentClass;
						FString LocalizeKey = c == Def::_CLASS_POINTMAN ? TEXT("Pointman") :
							c == Def::_CLASS_RIFLEMAN ? TEXT("Rifleman") : TEXT("Sniper");
						OutFieldValue.StringValue = FString::Printf(TEXT("<Strings:avaNet.UIRoomScene.Icon_Class_%s>"), *LocalizeKey);
					}
				}
			}
			else if (CellTag == FName(TEXT("Ready")))
			{
				if (_StateController->GetNetState() == _AN_LOBBY)
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
				else if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						switch (Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.bReady)
						{
						case _READY_NONE:
							OutFieldValue.StringValue = (Room->HostIdx == ListIndex ? TEXT("Host") : TEXT(" "));
							break;
						case _READY_WAIT:
							OutFieldValue.StringValue = (Room->HostIdx == ListIndex ? TEXT("Host") : TEXT("Ready"));
							break;
						case _READY_LOADING:
							OutFieldValue.StringValue = TEXT("Loading");
							break;
						case _READY_PLAYING:
							OutFieldValue.StringValue = TEXT("Playing");
							break;
						}
					}
				}
			}
			else if  ( CellTag == FName(TEXT("PcBang")) )
			{
				if (_StateController->GetNetState() == _AN_LOBBY)
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
				else if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex))
					{
						OutFieldValue.StringValue = TEXT("Inv.");
					}
					else
					{
						OutFieldValue.StringValue = (Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.pcBang > 0 ? TEXT("PC") : TEXT(" "));
					}
				}
			}
			else if  ( CellTag == FName(TEXT("RTT")) )
			{
				if (_StateController->GetNetState() == _AN_LOBBY)
				{
					OutFieldValue.StringValue = TEXT(" ");
				}
				else if (Room)
				{
					if (Room->PlayerList.IsEmpty(ListIndex) )
					{
						OutFieldValue.StringValue = TEXT(" ");
					}
					else
					{
						if( Room->HostIdx == ListIndex )
						{
							WORD HostRating = Room->PlayerList.PlayerList[ListIndex].RoomPlayerInfo.hostRating;
							if( HostRating < 100 )
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_HostRating_Never"), TEXT("avaNet"));
							else if( HostRating < 130 )
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_HostRating_Bad"), TEXT("avaNet"));
							else if( HostRating < 180 )
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_HostRating_Normal"), TEXT("avaNet"));
							else if( HostRating < 230 )
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_HostRating_Good"), TEXT("avaNet"));
							else
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_HostRating_VeryGood"), TEXT("avaNet"));
						}
						else if( 0 <= ListIndex && ListIndex < Def::MAX_ALL_PLAYER_PER_ROOM )
						{
							FLOAT RTTValue = Room->PlayerList.PlayerList[ListIndex].RttValue;
							if( RTTValue == -2.f )
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_RTT_Never"), TEXT("avaNet"));
							else if ( RTTValue == -1.f )
								OutFieldValue.StringValue = TEXT(" ");
							else if( 0.f <= RTTValue && RTTValue <= 30.f)
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_RTT_VeryGood"), TEXT("avaNet"));
							else if( 30.f < RTTValue && RTTValue <= 60.f)
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_RTT_Good"), TEXT("avaNet"));
							else if( 60.f < RTTValue && RTTValue <= 100.f)
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_RTT_Normal"), TEXT("avaNet"));
							else if( 100.f < RTTValue )
								OutFieldValue.StringValue = Localize(TEXT("UIRoomScene"), TEXT("Icon_RTT_Bad"), TEXT("avaNet"));
							else
								OutFieldValue.StringValue = TEXT("Inv.RTT");
						}
					} 
					// _STATE == "Room"
				}
				// FieldName == "RTT"
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = GNone;
	}
	return TRUE;

#undef _SELECT_CURRENT_ROOM
}

void UUIDataProvider_AvaNetReadyRoomPlayers::UpdateWidget( const FString& FieldName, UUIObject* TargetWidget )
{
	FName Field = *FieldName;
	if( Field == TEXT("PlayerListEU") ||
		Field == TEXT("PlayerListNRF"))
	{
		UavaNetHandler* NetHandler = GetAvaNetHandler();
		UBOOL bCount = NetHandler->IsCountingDown();
		UBOOL bReady = !( NetHandler->AmIHost() || NetHandler->AmISpectator() ) && NetHandler->AmIReady();
		TargetWidget->eventSetDblClickEnabled( !(bCount || bReady) );
	}
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetMainNavigation
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetMainNavigation)

void UUIDataProvider_AvaNetMainNavigation::GetSupportedDataFields( TArray<struct FUIDataProviderField>& OutFields )
{
	new (OutFields) FUIDataProviderField(TEXT("Handler_Settings"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_Leave"),DATATYPE_Property);

	GetInternalDataProviderSupportedDataFields( OutFields );
}

UBOOL UUIDataProvider_AvaNetMainNavigation::GetField( const FString& FieldName, struct FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	FName Field = *FieldName;

	if( Field == TEXT("Handler_Settings") )
	{
		OutFieldValue.StringValue = Localize(TEXT("MainNavigation"),TEXT("Text_Label_Config"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_Leave") )
	{
		FString KeyStr;
		if( GIsEditor && !GIsGame )
		{
			KeyStr = TEXT("Text_Label_Back");
		}
		else
		{
			switch( _StateController->GetNetState() )
			{
			case _AN_LOBBY:
				KeyStr = TEXT("Text_Label_LeaveToChannel");	break;
			case _AN_ROOM:
			case _AN_INVENTORY:
				KeyStr = TEXT("Text_Label_LeaveToLobby");	break;
			case _AN_CHANNELLIST:
				KeyStr = TEXT("Text_Label_LeaveGame");	break;
			default:
				KeyStr = TEXT("Text_Label_Back");	break;
			}
		}
		OutFieldValue.StringValue = Localize(TEXT("MainNavigation"),*KeyStr,TEXT("avaNet"));
		bResult = TRUE;
	}

	if( bResult && OutFieldValue.StringValue.Len() == 0 )
		OutFieldValue.StringValue = GNone;

	return bResult;
}

void UUIDataProvider_AvaNetMainNavigation::UpdateWidget( const FString& FieldName, UUIObject* TargetWidget )
{
	check(TargetWidget);
	FName Field = *FieldName;

	if( _StateController->IsStateInRoom() )
	{
		if( Field == TEXT("Handler_Settings") ||
			Field == TEXT("Handler_Leave"))
		{
			UavaNetHandler* NetHandler = GetAvaNetHandler();
			UBOOL bEnable = TargetWidget->IsEnabled();
			
			bEnable = !NetHandler->IsCountingDown();

			if( TargetWidget->IsEnabled() != bEnable )
				TargetWidget->SetEnabled(bEnable);
		}
	}
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetPlayerInfo
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetPlayerInfo)

void UUIDataProvider_AvaNetPlayerInfo::GetSupportedDataFields( TArray<struct FUIDataProviderField>& OutFields )
{
	// PlayerInfo Labels for displaying fully
	new (OutFields) FUIDataProviderField(TEXT("FullNickName"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("FullClanName"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("FullGameRecord"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("FullSDRatio"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("FullDisconnect"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("FullNetLocation"),DATATYPE_Property);

	// PlayerInfo Labels
	new (OutFields) FUIDataProviderField(TEXT("LevelIcon"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("NickName"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("ClanName"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("ClanMark"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("WinCount"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("DefeatCount"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("SDRatio"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Disconnect"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("NetLocation"),DATATYPE_Property);

	// PlayerInfo Buttons
	new (OutFields) FUIDataProviderField(TEXT("Handler_AddBuddy"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_DelBuddy"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_AddBlock"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_DelBlock"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_PrivateChat"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_InviteClan"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_KickClan"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_InviteGame"),DATATYPE_Property);
	new (OutFields) FUIDataProviderField(TEXT("Handler_FollowPlayer"),DATATYPE_Property);

	GetInternalDataProviderSupportedDataFields( OutFields );
}

UBOOL UUIDataProvider_AvaNetPlayerInfo::GetField( const FString& FieldName, struct FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;
	FName Field = *FieldName;

	if( Field == TEXT("Handler_AddBuddy") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_AddToFriendList"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_DelBuddy") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_BreakRelation"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("Handler_AddBlock") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_AddToBlackList"),TEXT("avaNet"));
		bResult	 = TRUE;
	}
	else if( Field == TEXT("Handler_DelBlock") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_RecoverRelation"),TEXT("avaNet"));
		bResult	 = TRUE;
	}
	else if( Field == TEXT("Handler_PrivateChat") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_PrivateChat"),TEXT("avaNet"));
		bResult	 = TRUE;		
	}
	else if( Field == TEXT("Handler_InviteClan") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_InviteClan"),TEXT("avaNet"));
		bResult	 = TRUE;
	}
	else if( Field == TEXT("Handler_KickClan") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_KickClan"),TEXT("avaNet"));
		bResult	 = TRUE;
	}
	else if( Field == TEXT("Handler_InviteGame") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_InviteGame"),TEXT("avaNet"));
		bResult	 = TRUE;
	}
	else if( Field == TEXT("Handler_FollowPlayer") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Button_FollowPlayer"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("FullNickName") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Label_PlayerInfo_NickName"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("FullClanName") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Label_PlayerInfo_ClanName"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("FullGameRecord") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Label_PlayerInfo_GameRecord"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("FullSDRatio") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Label_PlayerInfo_SDRatio"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("FullDisconnect") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Label_PlayerInfo_Disconnect"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( Field == TEXT("FullNetLocation") )
	{
		OutFieldValue.StringValue = Localize(TEXT("SelectedPlayerInfo"),TEXT("Text_Label_PlayerInfo_NetLocation"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else
	{
		Def::TID_ACCOUNT AccountID = static_cast<Def::TID_ACCOUNT>(this->AccountID);
		UBOOL bMyPlayer = _StateController->PlayerInfo.PlayerInfo.idAccount == AccountID;
		if( GIsEditor && !GIsGame )
		{
			if( Field == TEXT("NickName") )
				OutFieldValue.StringValue = Localize(TEXT("Test"),TEXT("Text_NickName"),TEXT("avaNet"));
			else if( Field == TEXT("ClanName") )
				OutFieldValue.StringValue = Localize(TEXT("Test"),TEXT("Text_ClanName"),TEXT("avaNet"));
			else if( Field == TEXT("ClanMark") )
				OutFieldValue.StringValue = GetClanMarkString(0);
			else if( Field == TEXT("LevelIcon") )
				OutFieldValue.StringValue = GetLevelIconString(1);
			else if( Field == TEXT("WinCount") || Field == TEXT("DefeatCount") )
				OutFieldValue.StringValue = appItoa(9999);
			else if( Field == TEXT("Disconnect") )
				OutFieldValue.StringValue = appItoa(100);
			else if( Field == TEXT("SDRatio") )
				OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"),1.f);
			else if( Field == TEXT("NetLocation") )
				OutFieldValue.StringValue = Localize(TEXT("UILobbyScene"),TEXT("Text_OnlineStatus_Offline"),TEXT("avaNet"));

			bResult = TRUE;
		}
		else if( AccountID != Def::ID_INVALID_ACCOUNT )
		{
			if ( PlayerInfoCat == PLAYERINFOCAT_Waiting )
			{
				FPlayerDispInfo *Player = GetSelectedPlayerDispInfo();
				if (Field == TEXT("NickName"))
				{
					if( bMyPlayer )
						OutFieldValue.StringValue = _StateController->PlayerInfo.PlayerInfo.nickname;
					else if (Player)
						OutFieldValue.StringValue = Player->PlayerInfo.nickname;
					else
						OutFieldValue.StringValue = TEXT("Player");
				}
				else if (Field == TEXT("ClanName"))
				{
					if( bMyPlayer )
					{
						FString GuildName = FString(_StateController->PlayerInfo.PlayerInfo.guildInfo.guildName);
						OutFieldValue.StringValue = GuildName.Trim().Len() > 0 ? GuildName : Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
					}
					else if (Player)
					{
						if (Player->IsFullInfo())
						{
							FString GuildName = FString(Player->PlayerInfo.guildName);
							OutFieldValue.StringValue = GuildName.Trim().Len() > 0 ? GuildName : Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
						}
						else
							OutFieldValue.StringValue = TEXT("--");
					}
					else
						OutFieldValue.StringValue = Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
				}
				else if (Field == TEXT("ClanMark"))
				{
					if( bMyPlayer )
					{
						FString GuildName = _StateController->PlayerInfo.PlayerInfo.guildInfo.guildName;
						OutFieldValue.StringValue = GetClanMarkString( GuildName.Trim().Len() > 0 ? _StateController->PlayerInfo.PlayerInfo.guildInfo.idGuildMark : -1);
					}
					else if (Player)
						OutFieldValue.StringValue = GetClanMarkString( Player->GetClanMarkID() );
					else
						OutFieldValue.StringValue = (GIsEditor && !GIsGame) ? GetClanMarkString(0) : TEXT("");

					// Inv. ???
					return TRUE;
				}
				else if (Field == TEXT("LevelIcon"))
				{
					if( bMyPlayer )
						OutFieldValue.StringValue = GetLevelIconString( _StateController->PlayerInfo.PlayerInfo.level);
					else
						OutFieldValue.StringValue = GetLevelIconString( Player ? Player->PlayerInfo.level : 0);
				}
				else if (Field == TEXT("WinCount"))
				{
					if ( bMyPlayer )
						OutFieldValue.StringValue = appItoa( _StateController->PlayerInfo.PlayerInfo.scoreInfo.gameWin );
					else if (Player && Player->IsFullInfo())
						OutFieldValue.StringValue = appItoa(Player->PlayerInfo.gameWin);
					else
						OutFieldValue.StringValue = TEXT("-");
				}
				else if (Field == TEXT("DefeatCount"))
				{
					if ( bMyPlayer )
						OutFieldValue.StringValue = appItoa( _StateController->PlayerInfo.PlayerInfo.scoreInfo.gameDefeat );
					else if (Player && Player->IsFullInfo())
						OutFieldValue.StringValue = appItoa(Player->PlayerInfo.gameDefeat);
					else
						OutFieldValue.StringValue = TEXT("-");
				}
				else if (Field == TEXT("Disconnect"))
				{
					if ( bMyPlayer )
						OutFieldValue.StringValue = appItoa( _StateController->PlayerInfo.PlayerInfo.scoreInfo.disconnectCount );
					else if (Player && Player->IsFullInfo())
						OutFieldValue.StringValue = appItoa(Player->PlayerInfo.disconnectCount);
					else
						OutFieldValue.StringValue = TEXT("-");
				}
				else if ( Field == TEXT("SDRatio") )
				{
					if( bMyPlayer )
						OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), _StateController->PlayerInfo.PlayerInfo.scoreInfo.GetSDRatio());
					else if (Player && Player->IsFullInfo())
						OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), Player->PlayerInfo.GetSDRatio());
					else
						OutFieldValue.StringValue = TEXT("-");
				}

				bResult = TRUE;
			}
			else if ( PlayerInfoCat == PLAYERINFOCAT_Buddy )
			{
				FBuddyInfo* pBuddyInfo = _Communicator().BuddyList.GetSelected();
				if ( pBuddyInfo != NULL )
				{
					FBuddyInfo& BuddyInfo = *pBuddyInfo;
					if ( Field == TEXT("NickName"))
					{
						OutFieldValue.StringValue = BuddyInfo.Nickname;
					}
					else if ( Field == TEXT("LevelIcon"))
					{
						OutFieldValue.StringValue = GetLevelIconString( BuddyInfo.Level );
					}
					else if ( Field == TEXT("ClanName"))
					{
						OutFieldValue.StringValue = BuddyInfo.GuildName.Trim().Len() != 0 ? BuddyInfo.GuildName : Localize(TEXT("UIGeneral"),TEXT("Text_NotInClan"),TEXT("avaNet"));
					}
					else if ( Field == TEXT("ClanMark"))
					{
						// BuddyInfo.idGuild가 비유효할때도 idGuildMark값은 유효할 수 있다.
						// 따라서 제대로된 정보(IsFullInfo)인지 검사하고 길드이름(GuildName)이 있는지 검사한다
						FString GuildName = BuddyInfo.GuildName.Trim();
						OutFieldValue.StringValue = GetClanMarkString( BuddyInfo.IsFullInfo() && GuildName.Len() > 0 ? BuddyInfo.idGuildMark : BuddyInfo.GetClanMarkID() );
						return TRUE;
					}
					else if ( Field == TEXT("WinCount"))
					{
						if (BuddyInfo.IsFullInfo())
							OutFieldValue.StringValue = appItoa(BuddyInfo.GameWin);
						else
							OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("DefeatCount"))
					{
						if (BuddyInfo.IsFullInfo())
							OutFieldValue.StringValue = appItoa(BuddyInfo.GameDefeat);
						else
							OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("Disconnect"))
					{
						if (BuddyInfo.IsFullInfo())
							OutFieldValue.StringValue = appItoa(BuddyInfo.DisconnectCount);
						else
							OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("SDRatio"))
					{
						if (BuddyInfo.IsFullInfo())
							OutFieldValue.StringValue = FString::Printf(TEXT("%.3f"), BuddyInfo.GetSDRatio());
						else
							OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("NetLocation") )
					{
						OutFieldValue.StringValue = BuddyInfo.IsOnline() ? BuddyInfo.GetLocation() : Localize(TEXT("UILobbyScene"), TEXT("Text_OnlineStatus_Offline") ,TEXT("avaNet"));
					}
				}

				bResult = TRUE;
			}
			else if ( PlayerInfoCat == PLAYERINFOCAT_Block )
			{
				FBuddyInfo* pBuddyInfo = _Communicator().BlockList.GetSelected();
				if( pBuddyInfo != NULL )
				{
					FBuddyInfo& BuddyInfo = *pBuddyInfo;

					if ( Field == TEXT("NickName"))
					{
						OutFieldValue.StringValue = BuddyInfo.Nickname;
					}
					else if ( Field == TEXT("LevelIcon"))
					{
						OutFieldValue.StringValue = GetLevelIconString( BuddyInfo.Level );
					}
					else if ( Field == TEXT("ClanName"))
					{
						OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("WinCount"))
					{
						OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("DefeatCount"))
					{
						OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("Disconnect"))
					{
						OutFieldValue.StringValue = TEXT("-");
					}
					else if ( Field == TEXT("SDRatio"))
					{
						OutFieldValue.StringValue = TEXT("--");
					}
				}

				bResult = TRUE;
			}
			else if ( PlayerInfoCat == PLAYERINFOCAT_Clan )
			{
				FGuildPlayerInfo* pGuildPlayerInfoOuter = NULL ;
				if( _StateController->GuildInfo.IsValid() 
					&& (pGuildPlayerInfoOuter = _StateController->GuildInfo.PlayerList.GetSelected()) != NULL )
				{
					const FGuildPlayerInfo& GuildPlayerInfoOuter = *pGuildPlayerInfoOuter;
					const Def::GUILD_PLAYER_INFO& GuildPlayerInfo = GuildPlayerInfoOuter.GuildPlayerInfo;

					UBOOL bFullInfo = ((FGuildPlayerInfo&)GuildPlayerInfoOuter).IsFullInfo();

					if ( Field == TEXT("NickName"))
					{
						OutFieldValue.StringValue = GuildPlayerInfo.nickname;
					}
					else if ( Field == TEXT("ClanName") )
					{
						OutFieldValue.StringValue = _StateController->GuildInfo.GuildInfo.name;
					}
					else if ( Field == TEXT("ClanMark") )
					{
						OutFieldValue.StringValue = GetClanMarkString( _StateController->PlayerInfo.GetClanMarkID() );
						return TRUE;
					}
					else if ( Field == TEXT("LevelIcon"))
					{
						OutFieldValue.StringValue = GetLevelIconString( GuildPlayerInfo.level );
					}
					else if ( Field == TEXT("WinCount"))
					{
						OutFieldValue.StringValue = bFullInfo ? appItoa( GuildPlayerInfoOuter.GameWin ) : TEXT("-");
					}
					else if ( Field == TEXT("DefeatCount"))
					{
						OutFieldValue.StringValue = bFullInfo ? appItoa( GuildPlayerInfoOuter.GameDefeat ) : TEXT("-");
					}
					else if ( Field == TEXT("Disconnect"))
					{
						OutFieldValue.StringValue = bFullInfo ? appItoa( GuildPlayerInfoOuter.DisconnectCount ) : TEXT("-");
					}
					else if ( Field == TEXT("SDRatio"))
					{
						OutFieldValue.StringValue = bFullInfo ? FString::Printf(TEXT("%.3f"), ((FGuildPlayerInfo&)GuildPlayerInfoOuter).GetSDRatio()) : TEXT("-");
					}
					else if ( Field == TEXT("NetLocation"))
					{
						OutFieldValue.StringValue = bFullInfo ? ((FGuildPlayerInfo&)GuildPlayerInfoOuter).GetLocation() : TEXT("-");
					}
				}
				bResult = TRUE;
			}
		}
	}

	if( bResult && OutFieldValue.StringValue.Len() == 0 )
		OutFieldValue.StringValue = GNone;

	return bResult;
}

void UUIDataProvider_AvaNetPlayerInfo::PostUpdateParamters( UBOOL bParmChanged )
{
	if( bWaiting )
	{
		PlayerInfoCat = PLAYERINFOCAT_Waiting;
		bWaiting = FALSE;
	}
	if( bBuddy )
	{
		PlayerInfoCat = PLAYERINFOCAT_Buddy;
		bBuddy = FALSE;
	}
	if( bBlock )
	{
		PlayerInfoCat = PLAYERINFOCAT_Block;
		bBlock = FALSE;
	}
	if( bClan )
	{
		PlayerInfoCat = PLAYERINFOCAT_Clan;
		bClan = FALSE;
	}

	INT PrevAccountID = AccountID;
	if( PlayerInfoCat == PLAYERINFOCAT_Waiting )
	{
		FPlayerDispInfo* PlayerInfo = GetSelectedPlayerDispInfo();
		AccountID = PlayerInfo ? PlayerInfo->PlayerInfo.idAccount : ID_INVALID_ACCOUNT;
	}
	else if( PlayerInfoCat == PLAYERINFOCAT_Buddy )
	{
		FBuddyInfo* BuddyInfo = _Communicator().BuddyList.GetSelected();
		AccountID = BuddyInfo ? BuddyInfo->idAccount : ID_INVALID_ACCOUNT;
	}
	else if( PlayerInfoCat == PLAYERINFOCAT_Block )
	{
		FBuddyInfo* BlockInfo = _Communicator().BlockList.GetSelected();
		AccountID = BlockInfo ? BlockInfo->idAccount : ID_INVALID_ACCOUNT;
	}
	else if( PlayerInfoCat == PLAYERINFOCAT_Clan )
	{
		FGuildPlayerInfo* ClanMemberInfo = _StateController->GuildInfo.PlayerList.GetSelected();
		AccountID = ClanMemberInfo ? ClanMemberInfo->GuildPlayerInfo.idAccount : ID_INVALID_ACCOUNT;
	}

	UBOOL AccountIDChanged = (PrevAccountID != AccountID);
	if( bParmChanged || AccountIDChanged )
	{
		if( AccountID != ID_INVALID_ACCOUNT )
		{
			bMyPlayer = _StateController->PlayerInfo.PlayerInfo.idAccount == AccountID;
			bBlockPlayer = _Communicator().BlockList.Find( AccountID ) != NULL;
			bBuddyPlayer = _Communicator().BuddyList.Find( AccountID ) != NULL;
			bMyClanPlayer = _StateController->GuildInfo.PlayerList.Find( AccountID ) != NULL;
			bOtherClanPlayer = FALSE;
			bOtherPlayer = !(bBlockPlayer || bBuddyPlayer || bMyClanPlayer || bOtherClanPlayer);
		}
	}
}

void UUIDataProvider_AvaNetPlayerInfo::UpdateWidget( const FString& FieldName, UUIObject* TargetWidget )
{
	if( GIsEditor && !GIsGame )
		return;

	check(TargetWidget);
	FName Field = *FieldName;
	UBOOL bVisible = TargetWidget->IsVisible();
	UBOOL bEnabled = TargetWidget->IsEnabled();

	if( Field == TEXT("FullNickName") )
	{
		bVisible = TRUE;
	}
	else if( Field == TEXT("FullClanName") ||
		Field == TEXT("FullGameRecord") ||
		Field == TEXT("FullSDRatio") ||
		Field == TEXT("FullDisconnect"))
	{
		bVisible = (PlayerInfoCat != PLAYERINFOCAT_Block);
	}
	else if( Field == TEXT("FullNetLocation") )
	{
		bVisible = (PlayerInfoCat != PLAYERINFOCAT_Block &&
					PlayerInfoCat != PLAYERINFOCAT_Waiting);
	}
	else if( Field == TEXT("Handler_AddBuddy") )
	{
		bVisible = !bMyPlayer && (bOtherPlayer || (!bBuddyPlayer && !bBlockPlayer));
	}
	else if( Field == TEXT("Handler_DelBuddy") )
	{
		FBuddyInfo* BIABuddy = _Communicator().GetBIA();
		UBOOL IsBIAPlayer = BIABuddy && BIABuddy->idAccount == AccountID;
		bVisible = bBuddyPlayer && !IsBIAPlayer;
	}
	else if( Field == TEXT("Handler_AddBlock") )
	{
		bVisible = !bMyPlayer && (bOtherPlayer || (!bBuddyPlayer && !bBlockPlayer) );
	}
	else if( Field == TEXT("Handler_DelBlock") )
	{
		bVisible = !bMyPlayer && bBlockPlayer;
	}
	else if( Field == TEXT("Handler_PrivateChat") )
	{
		bVisible = !bMyPlayer && !bBlockPlayer;
	}
	else if( Field == TEXT("Handler_InviteClan") )
	{
		UBOOL AmIClanMember = _StateController->GuildInfo.PlayerList.Find(_StateController->PlayerInfo.PlayerInfo.idAccount) != NULL;
		bVisible = !bMyPlayer && !bBlockPlayer && !bMyClanPlayer && AmIClanMember;
		bEnabled = GetAvaNetRequest()->GuildDoIHavePriv(PRIV_INVITE);
	}
	else if( Field == TEXT("Handler_KickClan") )
	{
		UBOOL AmIClanMember = _StateController->GuildInfo.PlayerList.Find(_StateController->PlayerInfo.PlayerInfo.idAccount) != NULL;
		bVisible = !bMyPlayer && bMyClanPlayer && AmIClanMember;
		bEnabled = GetAvaNetRequest()->GuildDoIHavePriv(PRIV_KICK);

	}
	else if( Field == TEXT("Handler_InviteGame") )
	{
		bVisible = _StateController->IsStateInRoom() && !bMyPlayer && !bBlockPlayer && (bOtherPlayer || bBuddyPlayer);
	}
	else if( Field == TEXT("Handler_FollowPlayer") )
	{
		UBOOL bMovable = _StateController->GetNetState() == _AN_LOBBY || _StateController->GetNetState() == _AN_ROOM;
		FBuddyInfo* BuddyInfo = _Communicator().BuddyList.Find(AccountID);
		UBOOL bBiBuddyOnline = BuddyInfo && (BuddyInfo->BuddyType == BT_BUDDY_BOTH || BuddyInfo->BuddyType == BT_BUDDY_BIA) && BuddyInfo->IsOnline();
		FGuildPlayerInfo* ClanPlayerInfo = _StateController->GuildInfo.PlayerList.Find(AccountID);
		UBOOL bClanPlayerOnline = ClanPlayerInfo && ClanPlayerInfo->IsOnline() && !bBlock;
		
		bVisible = bMovable && !bMyPlayer && !bBlockPlayer && (bClanPlayerOnline || bBiBuddyOnline);
	}

	if( bVisible != TargetWidget->IsVisible() )
	{
		TargetWidget->eventSetVisibility(bVisible);
	}

	if( bEnabled != TargetWidget->IsEnabled() )
	{
		TargetWidget->SetEnabled(bEnabled);
	}
}

/* ==========================================================================================================
	UUIDataStore_AvaNet
========================================================================================================== */

/**
 * Hook for performing any initialization required for this data store
 */
void UUIDataStore_AvaNet::InitializeDataStore()
{
	if (MyPlayerProvider == NULL)
	{
		MyPlayerProvider = ConstructObject<UUIDataProvider_AvaNetMyPlayer>(UUIDataProvider_AvaNetMyPlayer::StaticClass());
	}
	if (SelectedRoomProvider == NULL)
	{
		SelectedRoomProvider = ConstructObject<UUIDataProvider_AvaNetSelectedRoom>(UUIDataProvider_AvaNetSelectedRoom::StaticClass());
	}
	if (ChannelsProvider == NULL)
	{
		ChannelsProvider = ConstructObject<UUIDataProvider_AvaNetChannels>(UUIDataProvider_AvaNetChannels::StaticClass());
	}
	if (LobbyRoomsProvider == NULL)
	{
		LobbyRoomsProvider = ConstructObject<UUIDataProvider_AvaNetLobbyRooms>(UUIDataProvider_AvaNetLobbyRooms::StaticClass());
	}
	if (LobbyPlayersProvider == NULL)
	{
		LobbyPlayersProvider = ConstructObject<UUIDataProvider_AvaNetLobbyPlayers>(UUIDataProvider_AvaNetLobbyPlayers::StaticClass());
	}
	if (LobbyFriendPlayersProvider == NULL)
	{
		LobbyFriendPlayersProvider = ConstructObject<UUIDataProvider_AvaNetLobbyFriendPlayers>(UUIDataProvider_AvaNetLobbyFriendPlayers::StaticClass());
	}
	if (LobbyBlockedPlayersProvider == NULL )
	{
		LobbyBlockedPlayersProvider = ConstructObject<UUIDataProvider_AvaNetLobbyBlockedPlayers>(UUIDataProvider_AvaNetLobbyBlockedPlayers::StaticClass());
	}
	if (ChatMsgsProvider == NULL)
	{
		ChatMsgsProvider = ConstructObject<UUIDataProvider_AvaNetChatMsgs>(UUIDataProvider_AvaNetChatMsgs::StaticClass());
	}
	if (InventoryProvider == NULL)
	{
		InventoryProvider = ConstructObject<UUIDataProvider_AvaNetInventory>(UUIDataProvider_AvaNetInventory::StaticClass());
	}
	if (ShopItemsProvider == NULL)
	{
		ShopItemsProvider = ConstructObject<UUIDataProvider_AvaNetShopItems>(UUIDataProvider_AvaNetShopItems::StaticClass());
	}
	if (ShopCustomItemsProvider == NULL )
	{
		ShopCustomItemsProvider = ConstructObject<UUIDataProvider_AvaNetShopCustomItems>(UUIDataProvider_AvaNetShopCustomItems::StaticClass());
	}
	if (ItemDescProvider == NULL )
	{
		ItemDescProvider = ConstructObject<UUIDataProvider_AvaNetItemDesc>(UUIDataProvider_AvaNetItemDesc::StaticClass());
	}
	if (ItemDescFieldProvider == NULL )
	{
		ItemDescFieldProvider = ConstructObject<UUIDataProvider_AvaNetItemDescField>(UUIDataProvider_AvaNetItemDescField::StaticClass());
	}
	if (ItemDescListProvider == NULL )
	{
		ItemDescListProvider = ConstructObject<UUIDataProvider_AvaNetItemDescList>(UUIDataProvider_AvaNetItemDescList::StaticClass());
	}
	if ( MiscProvider == NULL )
	{
		MiscProvider = ConstructObject<UUIDataProvider_AvaNetMisc>(UUIDataProvider_AvaNetMisc::StaticClass());
	}
	if ( MiscFieldProvider == NULL )
	{
		MiscFieldProvider = ConstructObject<UUIDataProvider_AvaNetMiscField>(UUIDataProvider_AvaNetMiscField::StaticClass());
	}
	if (LastResultProvider == NULL)
	{
		LastResultProvider = ConstructObject<UUIDataProvider_AvaNetLastResult>(UUIDataProvider_AvaNetLastResult::StaticClass());
	}
	if ( LastResultMsgProvider == NULL )
	{
		LastResultMsgProvider = ConstructObject<UUIDataProvider_AvaNetLastResultMsgs>(UUIDataProvider_AvaNetLastResultMsgs::StaticClass());
	}
	if ( EffectItemsProvider == NULL )
	{
		EffectItemsProvider = ConstructObject<UUIDataProvider_AvaNetEffectItems>(UUIDataProvider_AvaNetEffectItems::StaticClass());
	}
	if ( GuildInfoFieldProvider == NULL )
	{
		GuildInfoFieldProvider = ConstructObject<UUIDataProvider_AvaNetGuildInfoField>(UUIDataProvider_AvaNetGuildInfoField::StaticClass());
	}
	if ( GuildMembersProvider == NULL )
	{
		GuildMembersProvider = ConstructObject<UUIDataProvider_AvaNetGuildMembers>(UUIDataProvider_AvaNetGuildMembers::StaticClass());
	}
	if ( FilteredWeaponProvider == NULL )
	{
		FilteredWeaponProvider = ConstructObject<UUIDataProvider_AvaNetFilteredWeapons>(UUIDataProvider_AvaNetFilteredWeapons::StaticClass());
	}
	if ( RoomPlayerFieldProvider == NULL )
	{
		RoomPlayerFieldProvider = ConstructObject<UUIDataProvider_AvaNetRoomPlayerField>(UUIDataProvider_AvaNetRoomPlayerField::StaticClass());
	}
	// InternalProviderData 로 옮김.
	//if ( RoomSettingsProvider == NULL )
	//{
	//	RoomSettingsProvider = ConstructObject<UUIDataProvider_AvaNetRoomSettings>(UUIDataProvider_AvaNetRoomSettings::StaticClass());
	//}

	if (PlayerWeaponProviders.Num() == 0)
	{
		for (INT i = 0; i < MAX_WEAPONSET_SIZE; ++i)
		{
			UUIDataProvider_AvaNetPlayerWeapon *pProvider = ConstructObject<UUIDataProvider_AvaNetPlayerWeapon>(UUIDataProvider_AvaNetPlayerWeapon::StaticClass());
			pProvider->SlotID = i;
			PlayerWeaponProviders.AddItem(pProvider);
		}
	}
	if (PlayerEquipProviders.Num() == 0)
	{
		for (INT i = 0; i < MAX_EQUIPSET_SIZE; ++i)
		{
			UUIDataProvider_AvaNetPlayerEquip *pProvider = ConstructObject<UUIDataProvider_AvaNetPlayerEquip>(UUIDataProvider_AvaNetPlayerEquip::StaticClass());
			pProvider->SlotID = i;
			PlayerEquipProviders.AddItem(pProvider);
		}
	}
	if (PlayerCustomProviders.Num() == 0)
	{
		for (INT i = 0; i < MAX_INVENTORY_SIZE; ++i)
		{
			UUIDataProvider_AvaNetPlayerCustoms *pProvider = ConstructObject<UUIDataProvider_AvaNetPlayerCustoms>(UUIDataProvider_AvaNetPlayerCustoms::StaticClass());
			pProvider->InvenID = i;
			PlayerCustomProviders.AddItem(pProvider);
		}
	}

	InitializeInternalDataProviders();
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	out_FieldValue	receives the resolved value for the property specified.
*							@see ParseDataStoreReference for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataStore_AvaNet::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	extern INT GAVAVersion;
	extern INT GAVABuiltFromChangeList;

	if (FieldName == TEXT("AvaVersion"))
	{
		out_FieldValue.StringValue = FString::Printf(TEXT("%d"), GAVAVersion );
	}
	else if (FieldName == TEXT("AvaChangeListVersion"))
	{
		out_FieldValue.StringValue = FString::Printf(TEXT("%d"), GAVABuiltFromChangeList );
	}
	else if (FieldName == TEXT("ChannelName"))
	{
		if (_StateController->GetNetState() == _AN_LOBBY)
		{
			out_FieldValue.StringValue = _StateController->ChannelInfo.ChannelName;
		}
		else if (_StateController->IsStateInRoom())
		{
			out_FieldValue.StringValue = _StateController->ChannelInfo.ChannelName;
		}
		else
			out_FieldValue.StringValue = TEXT("????");
	}
	else if (FieldName == TEXT("ChannelID"))
	{
		if (_StateController->ChannelInfo.IsMyClanChannel())
		{
			out_FieldValue.StringValue = TEXT("0");
		}
		else
		{
			if (_StateController->GetNetState() == _AN_LOBBY || _StateController->IsStateInRoom())
			{
				out_FieldValue.StringValue = FString::Printf(TEXT("%03d"), _StateController->ChannelInfo.idChannel);
			}
			else
				out_FieldValue.StringValue = TEXT("???");
		}
	}
	else if (FieldName == TEXT("MainNoticeMsg"))
	{
		if (GIsEditor && !GIsGame)
		{
			out_FieldValue.StringValue = TEXT("This is the main notice message\nClipMode(Wrap) test");
		}
		else
		{
			out_FieldValue.StringValue = *_StateController->MainNotice;
		}
	}
	else if (FieldName == TEXT("RTNoticeMsg"))
	{
		if (GIsEditor && !GIsGame)
		{
			out_FieldValue.StringValue = TEXT("This is the real-time notice message");
		}
		else
		{
			out_FieldValue.StringValue = *_StateController->RTNotice;
		}
	}
	else if (appStrnicmp(*FieldName, TEXT("TickerMsg"), 9) == 0)
	{
		INT MsgIdx = appAtoi(*FieldName + 9);

		if (GIsEditor && !GIsGame)
		{
			if (MsgIdx >= 0 && MsgIdx < 10)
				out_FieldValue.StringValue = *FString::Printf(_TEXT("This is the ticker message #%d"), MsgIdx);
			else
				out_FieldValue.StringValue = TEXT("This is the ticker message");
		}
		else
		{
			if (MsgIdx >= 0 && MsgIdx < 10)
				out_FieldValue.StringValue = *_TickerMsg()[MsgIdx];
			else
				out_FieldValue.StringValue = TEXT("");
		}
	}
	else
	{
		FString *Value = GavaNetClient->Settings.Find(FieldName);
		if (Value)
		{
			out_FieldValue.StringValue = *Value;
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/**
* Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	FieldValue		the value to store for the property specified.
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataStore_AvaNet::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	GavaNetClient->Settings.Set(*FieldName, *FieldValue.StringValue);
	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataStore_AvaNet::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	for (TMap<FString,FString>::TIterator It(GavaNetClient->Settings); It; ++It)
	{
		new(out_Fields) FUIDataProviderField(FName(*(It.Key())));
	}

	new(out_Fields) FUIDataProviderField(FName(TEXT("AvaVersion")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("AvaChangeListVersion")));

	new(out_Fields) FUIDataProviderField(FName(TEXT("ChannelName")));
	new(out_Fields) FUIDataProviderField(FName(TEXT("ChannelID")));

	new(out_Fields) FUIDataProviderField(TEXT("MainNoticeMsg"));
	new(out_Fields) FUIDataProviderField(TEXT("RTNoticeMsg"));

	for (INT i = 0; i < 10; ++i)
		new(out_Fields) FUIDataProviderField(*FString::Printf(TEXT("TickerMsg%d"), i));

	check(MyPlayerProvider && SelectedRoomProvider && ItemDescFieldProvider && MiscFieldProvider &&
		LastResultProvider && GuildInfoFieldProvider && RoomPlayerFieldProvider );

	new(out_Fields) FUIDataProviderField(TEXT("MyPlayer"), DATATYPE_Provider, MyPlayerProvider);
	new(out_Fields) FUIDataProviderField(TEXT("SelectedRoom"), DATATYPE_Provider, SelectedRoomProvider);
	new(out_Fields) FUIDataProviderField(TEXT("ItemDescField"), DATATYPE_Provider, ItemDescFieldProvider);
	new(out_Fields) FUIDataProviderField(TEXT("MiscField"), DATATYPE_Provider, MiscFieldProvider);
	new(out_Fields) FUIDataProviderField(TEXT("LastResult"), DATATYPE_Provider, LastResultProvider);
	new(out_Fields) FUIDataProviderField(TEXT("GuildInfoField"), DATATYPE_Provider, GuildInfoFieldProvider);
	new(out_Fields) FUIDataProviderField(TEXT("RoomPlayerField"), DATATYPE_Provider, RoomPlayerFieldProvider);

	//	InternalProviderData로 옮김
	//	new(out_Fields) FUIDataProviderField(TEXT("RoomSettings"), DATATYPE_Provider, RoomSettingsProvider);

	check(PlayerWeaponProviders.Num() > 0 && PlayerEquipProviders.Num() > 0 && PlayerCustomProviders.Num());

	new(out_Fields) FUIDataProviderField(TEXT("PlayerWeapons"), PlayerWeaponProviders);
	new(out_Fields) FUIDataProviderField(TEXT("PlayerEquips"), PlayerEquipProviders);
	new(out_Fields) FUIDataProviderField(TEXT("PlayerCustoms"), PlayerCustomProviders);

	check(ChannelsProvider && LobbyRoomsProvider && LobbyPlayersProvider && LobbyFriendPlayersProvider && LobbyBlockedPlayersProvider && GuildMembersProvider && ChatMsgsProvider && InventoryProvider &&
		ShopItemsProvider && ShopCustomItemsProvider && ItemDescProvider && ItemDescListProvider && MiscProvider && LastResultProvider && FilteredWeaponProvider && LastResultMsgProvider && EffectItemsProvider );
	// Ask the providers for their fields
	ChannelsProvider->GetSupportedDataFields(out_Fields);
	LobbyRoomsProvider->GetSupportedDataFields(out_Fields);
	LobbyPlayersProvider->GetSupportedDataFields(out_Fields);
	LobbyFriendPlayersProvider->GetSupportedDataFields(out_Fields);
	LobbyBlockedPlayersProvider->GetSupportedDataFields(out_Fields);
	ChatMsgsProvider->GetSupportedDataFields(out_Fields);
	InventoryProvider->GetSupportedDataFields(out_Fields);
	ShopItemsProvider->GetSupportedDataFields(out_Fields);
	ShopCustomItemsProvider->GetSupportedDataFields(out_Fields);
	ItemDescProvider->GetSupportedDataFields(out_Fields);
	ItemDescListProvider->GetSupportedDataFields(out_Fields);
	MiscProvider->GetSupportedDataFields(out_Fields);
	GuildMembersProvider->GetSupportedDataFields(out_Fields);
	FilteredWeaponProvider->GetSupportedDataFields(out_Fields);
	LastResultMsgProvider->GetSupportedDataFields(out_Fields);
	EffectItemsProvider->GetSupportedDataFields(out_Fields);

	// @deprecated : new(out_Fields) FUIDataProviderField(FName(TEXT("UpdateMsgs")), DATATYPE_Collection );

	// InternalProviderData에 생성된 Provider들을 GetSupportedDataFields에 걸어준다
	GetInternalDataProviderSupportedDataFields( out_Fields );
}

/**
* Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
* user an idea as to what a bound datastore will look like in-game.
*
* @param		DataTag		the tag corresponding to the data field that we want filler data for
*
* @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
*/
FString UUIDataStore_AvaNet::GenerateFillerData( const FString& DataTag )
{
	FString Filler = Super::GenerateFillerData(DataTag);
	if (Filler.Len() == 0)
	{
		FString *Value = GavaNetClient->Settings.Find(DataTag);
		if (Value)
		{
			Filler = *Value;
		}
	}
	return Filler;
}


/**
* Retrieves the list of all data tags contained by this element provider which correspond to list element data.
*
* @return	the list of tags supported by this element provider which correspond to list element data.
*/
TArray<FName> UUIDataStore_AvaNet::GetElementProviderTags(void)
{
	TArray<FName> Tags;
	Tags.AddItem(FName(TEXT("Channels")));
	Tags.AddItem(FName(TEXT("LobbyRooms")));
	Tags.AddItem(FName(TEXT("LobbyPlayers")));
	Tags.AddItem(FName(TEXT("ChatMsgs")));
	Tags.AddItem(FName(TEXT("InventoryWeapon")));
	Tags.AddItem(FName(TEXT("InventoryWeaponP1")));
	Tags.AddItem(FName(TEXT("InventoryWeaponP2")));
	Tags.AddItem(FName(TEXT("InventoryWeaponP3")));
	Tags.AddItem(FName(TEXT("InventoryWeaponP4")));
	Tags.AddItem(FName(TEXT("InventoryWeaponP5")));
	Tags.AddItem(FName(TEXT("InventoryWeaponP6")));
	Tags.AddItem(FName(TEXT("InventoryWeaponR1")));
	Tags.AddItem(FName(TEXT("InventoryWeaponR2")));
	Tags.AddItem(FName(TEXT("InventoryWeaponR3")));
	Tags.AddItem(FName(TEXT("InventoryWeaponR4")));
	Tags.AddItem(FName(TEXT("InventoryWeaponR5")));
	Tags.AddItem(FName(TEXT("InventoryWeaponR6")));
	Tags.AddItem(FName(TEXT("InventoryWeaponS1")));
	Tags.AddItem(FName(TEXT("InventoryWeaponS2")));
	Tags.AddItem(FName(TEXT("InventoryWeaponS3")));
	Tags.AddItem(FName(TEXT("InventoryWeaponS4")));
	Tags.AddItem(FName(TEXT("InventoryWeaponS5")));
	Tags.AddItem(FName(TEXT("InventoryWeaponS6")));
	Tags.AddItem(FName(TEXT("InventoryEquip")));
	Tags.AddItem(FName(TEXT("InventoryEquipH1")));
	Tags.AddItem(FName(TEXT("InventoryEquipH11")));
	Tags.AddItem(FName(TEXT("InventoryEquipH12")));
	Tags.AddItem(FName(TEXT("InventoryEquipH2")));
	Tags.AddItem(FName(TEXT("InventoryEquipH3")));
	Tags.AddItem(FName(TEXT("InventoryEquipC1")));
	Tags.AddItem(FName(TEXT("InventoryEquipC2")));
	Tags.AddItem(FName(TEXT("InventoryEquipA1")));
	Tags.AddItem(FName(TEXT("InventoryEquipA2")));
	Tags.AddItem(FName(TEXT("InventoryEquipB1")));
	Tags.AddItem(FName(TEXT("InventoryEquipB3")));
	Tags.AddItem(FName(TEXT("InventoryEquipW1")));
	Tags.AddItem(FName(TEXT("InventoryEquipW2")));
	Tags.AddItem(FName(TEXT("InventoryEquipW3")));
	Tags.AddItem(FName(TEXT("InventoryEquipT1")));
	Tags.AddItem(FName(TEXT("InventoryEquipT2")));
	Tags.AddItem(FName(TEXT("InventoryEquipE")));
	Tags.AddItem(FName(TEXT("InventoryEquipG")));
	Tags.AddItem(FName(TEXT("InventoryEquipK")));
	Tags.AddItem(FName(TEXT("InventoryEquipBT")));
	Tags.AddItem(FName(TEXT("InventoryEquipBD")));
	Tags.AddItem(FName(TEXT("ShopWeapon")));
	Tags.AddItem(FName(TEXT("ShopWeaponP1")));
	Tags.AddItem(FName(TEXT("ShopWeaponP2")));
	Tags.AddItem(FName(TEXT("ShopWeaponP3")));
	Tags.AddItem(FName(TEXT("ShopWeaponP4")));
	Tags.AddItem(FName(TEXT("ShopWeaponR1")));
	Tags.AddItem(FName(TEXT("ShopWeaponR2")));
	Tags.AddItem(FName(TEXT("ShopWeaponR3")));
	Tags.AddItem(FName(TEXT("ShopWeaponR4")));
	Tags.AddItem(FName(TEXT("ShopWeaponS1")));
	Tags.AddItem(FName(TEXT("ShopWeaponS2")));
	Tags.AddItem(FName(TEXT("ShopWeaponS3")));
	Tags.AddItem(FName(TEXT("ShopWeaponS4")));
	Tags.AddItem(FName(TEXT("ShopEquip")));
	Tags.AddItem(FName(TEXT("ShopEquipH1")));
	Tags.AddItem(FName(TEXT("ShopEquipH11")));
	Tags.AddItem(FName(TEXT("ShopEquipH12")));
	Tags.AddItem(FName(TEXT("ShopEquipH2")));
	Tags.AddItem(FName(TEXT("ShopEquipH3")));
	Tags.AddItem(FName(TEXT("ShopEquipC1")));
	Tags.AddItem(FName(TEXT("ShopEquipC2")));
	Tags.AddItem(FName(TEXT("ShopEquipA1")));
	Tags.AddItem(FName(TEXT("ShopEquipA2")));
	Tags.AddItem(FName(TEXT("ShopEquipB1")));
	Tags.AddItem(FName(TEXT("ShopEquipB3")));
	Tags.AddItem(FName(TEXT("ShopEquipW1")));
	Tags.AddItem(FName(TEXT("ShopEquipW2")));
	Tags.AddItem(FName(TEXT("ShopEquipW3")));
	Tags.AddItem(FName(TEXT("ShopEquipT1")));
	Tags.AddItem(FName(TEXT("ShopEquipT2")));
	Tags.AddItem(FName(TEXT("ShopEquipE")));
	Tags.AddItem(FName(TEXT("ShopEquipG")));
	Tags.AddItem(FName(TEXT("ShopEquipK")));
	Tags.AddItem(FName(TEXT("ShopEquipBT")));
	Tags.AddItem(FName(TEXT("ShopEquipBD")));
	return Tags;
}

/**
* Returns the number of list elements associated with the data tag specified.
*
* @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
*						from GetElementProviderTags.
*
* @return	the total number of elements that are required to fully represent the data specified.
*/
INT UUIDataStore_AvaNet::GetElementCount( FName FieldName )
{
	return 1;
}

/**
* Retrieves the list elements associated with the data tag specified.
*
* @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
*							from GetElementProviderTags.
* @param	OutElements		will be filled with the elements associated with the data specified by DataTag.
*
* @return	TRUE if this data store contains a list element data provider matching the tag specified.
*/
UBOOL UUIDataStore_AvaNet::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	UBOOL bResult = FALSE;

	if (ChannelsProvider && appStristr(FieldName.GetName(), TEXT("Channels")) == FieldName.GetName() )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT i = 0 ; i < 30 ; i++)
				OutElements.AddItem(i);
		}
		else
		{
			// Group="0,1,3" 로 값이 들어오면 0번 1번 3번 그룹을 한 리스트에 보여준다
			FString GroupStr;
			Parse( FieldName.GetName(), TEXT("Group="), GroupStr );

			TArray<INT> GroupNumList;
			TArray<FString> GroupStrList;
			GroupStr.ParseIntoArrayWS( &GroupStrList, TEXT(",") );
			for( INT i = 0 ; i < GroupStrList.Num() ; i++ )
				GroupNumList.AddItem( appAtoi(*GroupStrList(i)) );

			// 클랜채널에는 최소한 한개의 채널이 존재한다 (내 길드채널)
			if ( GroupNumList.FindItemIndex(EChannelFlag_Clan) != INDEX_NONE )
				OutElements.AddItem(_StateController->ChannelList.ChannelList.Num());

			// temporary
			if ( GroupNumList.FindItemIndex(EChannelFlag_Clan) != INDEX_NONE )
				GroupNumList.AddUniqueItem(EChannelFlag_Temp);

			for( INT GroupListIndex = 0 ; GroupListIndex < GroupNumList.Num() ; GroupListIndex++ )
			{
				INT CurrentGroup = GroupNumList(GroupListIndex);
				for (INT Index = 0; Index < _StateController->ChannelList.ChannelList.Num(); Index++)
				{
					UBOOL bUseChannelItem = ( CurrentGroup == _StateController->ChannelList.ChannelList(Index).Flag );
					if ( _StateController->ChannelList.ChannelList(Index).IsValid() && bUseChannelItem )
					{
//						debugf(TEXT("Channels Group(%d), Name(%s)"), CurrentGroup, *_StateController->ChannelList.ChannelList(Index).ChannelName);
						OutElements.AddItem(Index);
					}
				}
			}

			//for (INT Index = 0; Index < _StateController->ChannelList.ChannelList.Num(); Index++)
			//{
			//	UBOOL bUseChannelItem = (GroupIndex == INDEX_NONE) || (GroupIndex == _StateController->ChannelList.ChannelList(Index).Flag);
			//	if (!_StateController->ChannelList.ChannelList(Index).IsValid() && bUseChannelItem )
			//		OutElements.AddItem(Index);
			//}
		}

		bResult = TRUE;
	}
	else if (LobbyRoomsProvider && appStristr(FieldName.GetName(), TEXT("LobbyRooms")) == FieldName.GetName())
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 30; Index++)
			{
				OutElements.AddItem(Index);
			}
		}
		else
		{
			for (INT Index = 0; Index < _StateController->RoomList.RoomList.Num(); Index++)
			{
				OutElements.AddItem(Index);
			}
		}

		bResult = TRUE;
	}
	else if (LobbyPlayersProvider && FieldName == FName(TEXT("LobbyPlayers")))
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 30; Index++)
			{
				OutElements.AddItem(Index);
			}
		}
		else
		{
			for (INT Index = 0; Index < _StateController->LobbyPlayerList.PlayerList.Num(); Index++)
			{
				OutElements.AddItem(Index);
			}
		}

		bResult = TRUE;
	}
	else if (LobbyFriendPlayersProvider && FieldName == FName(TEXT("LobbyFriendPlayers")))
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 30; Index++)
			{
				OutElements.AddItem(Index);
			}
		}
		else
		{
			// 전우를 먼저 리스트에 보여준다
			for (INT Index = 0; Index < _Communicator().BuddyList.Num(); ++Index)
				if ( _Communicator().BuddyList(Index).IsBuddyBIA())
					OutElements.AddItem(Index);

			// 전우가 아닌 다른 플레이어를 그뒤에 보여준다
			for (INT Index = 0; Index < _Communicator().BuddyList.Num(); ++Index)
				if ( !_Communicator().BuddyList(Index).IsBuddyBIA())
					OutElements.AddItem(Index);
		}

		bResult = TRUE;
	}
	else if (LobbyBlockedPlayersProvider && FieldName == FName(TEXT("LobbyBlockedPlayers")))
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 30; Index++)
			{
				OutElements.AddItem(Index);
			}
		}
		else
		{
			for (INT Index = 0; Index < _Communicator().BlockList.Num(); Index++)
			{
				OutElements.AddItem(Index);
			}
		}

		bResult = TRUE;
	}
	else if (GuildMembersProvider && FieldName == FName(TEXT("GuildMembers")) )
	{
		if (GIsEditor && !GIsGame )
		{
			for (INT Index = 0; Index < 30; Index++)
			{
				OutElements.AddItem(Index);
			}
		}
		else
		{
			if( _StateController->GuildInfo.IsValid() )
			{
				for (INT Index = 0; Index < _StateController->GuildInfo.PlayerList.PlayerList.Num() ; Index++)
				{
					//if( FString(_StateController->PlayerInfo.PlayerInfo.nickname) != FString(_StateController->GuildInfo.PlayerList.PlayerList(Index).PlayerInfo.nickname) )
					OutElements.AddItem(Index);
				}
			}
		}

		bResult = TRUE;
	}
	else if (ChatMsgsProvider && FieldName == FName(TEXT("ChatMsgs")))
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 30; Index++)
			{
				OutElements.AddItem(Index);
			}
		}
		else
		{
			for (INT Index = 0; Index < _StateController->ChatMsgList.ChatList.Num(); Index++)
			{
				OutElements.AddItem(Index);
			}
		}

		bResult = TRUE;
	}
	else if (InventoryProvider && appStrnicmp(FieldName.GetName(), TEXT("InventoryWeapon"), 15) == 0)
	{
		if (FieldName == FName(TEXT("InventoryWeapon")))
		{
			if (GIsEditor && !GIsGame)
			{
				for (INT Index = 0; Index < 10; Index++)
				{
					OutElements.AddItem(Index);
				}
			}
			else
			{
				if (_StateController->PlayerInfo.IsValid())
				{
					for (INT Index = 0; Index < MAX_INVENTORY_SIZE; Index++)
					{
						if (!_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].IsEmpty())
							OutElements.AddItem(Index);
					}
				}
			}

			bResult = TRUE;
		}
		else
		{
			TCHAR *Code = (TCHAR*)FieldName.GetName() + 15;
			INT ClassID = -1;

			if (appStrlen(Code) == 2)
			{
				switch (appToUpper(Code[0]))
				{
				case TEXT('P'):
					ClassID = _CLASS_POINTMAN;
					break;
				case TEXT('R'):
					ClassID = _CLASS_RIFLEMAN;
					break;
				case TEXT('S'):
					ClassID = _CLASS_SNIPER;
					break;
				}

				if (ClassID >= 0 && Code[1] >= TEXT('1') && Code[1] <= TEXT('6'))
				{
					if (GIsEditor && !GIsGame)
					{
						for (INT Index = 0; Index < 10; Index++)
						{
							OutElements.AddItem(Index);
						}
					}
					else
					{
						if (_StateController->PlayerInfo.IsValid())
						{
							INT SlotCode = Code[1] - TEXT('1');
							INT EquipSlot = (SlotCode < 3 ? ClassID + SlotCode * 3 : 6 + ClassID * 3 + SlotCode);

							if (EquipSlot >= 0 && EquipSlot < MAX_WEAPONSET_SIZE)
							{
								SLOT_DESC *Slot = _ItemDesc().GetWeaponSlot(EquipSlot);
								if (Slot)
								{
									for (INT Index = 0; Index < MAX_INVENTORY_SIZE; Index++)
									{
										if (!_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].IsEmpty())
										{
											ITEM_DESC *ItemDesc = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].id);
											if (ItemDesc && (ItemDesc->slotType & Slot->slotType))
											{
												OutElements.AddItem(Index + EquipSlot * 100);
											}
										}
									}
								}
							}
						}
					}
					bResult = TRUE;
				}
			}
		}
	}
	else if (InventoryProvider && appStrnicmp(FieldName.GetName(), TEXT("InventoryEquip"),14) == 0 /*FieldName == FName(TEXT("InventoryEquip"))*/)
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 10; Index++)
			{
				OutElements.AddItem(Index + 10000);
			}

			bResult = TRUE;
		}
		else
		{
			INT SlotType = -1;
			const TCHAR *Code = FieldName.GetName() + 14;
			UBOOL bGetAllCell = FALSE;

			if ( appStricmp(Code, TEXT("")) == 0 )				bGetAllCell = TRUE;
			else if( appStricmp(Code, TEXT("H1")) == 0 )		SlotType = _EP_H1;
			else if ( appStricmp(Code, TEXT("H11")) == 0 )		SlotType = _EP_H11;
			else if ( appStricmp(Code, TEXT("H12")) == 0 )		SlotType = _EP_H12;
			else if ( appStricmp(Code, TEXT("H2")) == 0 )		SlotType = _EP_H2;
			else if ( appStricmp(Code, TEXT("H3")) == 0 )		SlotType = _EP_H3;
			else if ( appStricmp(Code, TEXT("C1")) == 0 )		SlotType = _EP_C1;
			else if ( appStricmp(Code, TEXT("C2")) == 0 )		SlotType = _EP_C2;
			else if ( appStricmp(Code, TEXT("A1")) == 0 )		SlotType = _EP_A1;
			else if ( appStricmp(Code, TEXT("A2")) == 0 )		SlotType = _EP_A2;
			else if ( appStricmp(Code, TEXT("B1")) == 0 )		SlotType = _EP_B1;
			else if ( appStricmp(Code, TEXT("B3")) == 0 )		SlotType = _EP_B3;
			else if ( appStricmp(Code, TEXT("W1")) == 0 )		SlotType = _EP_W1;
			else if ( appStricmp(Code, TEXT("W2")) == 0 )		SlotType = _EP_W2;
			else if ( appStricmp(Code, TEXT("W3")) == 0 )		SlotType = _EP_W3;
			else if ( appStricmp(Code, TEXT("T1")) == 0 )		SlotType = _EP_T1;
			else if ( appStricmp(Code, TEXT("T2")) == 0 )		SlotType = _EP_T2;
			else if ( appStricmp(Code, TEXT("E")) == 0 )		SlotType = _EP_E;
			else if ( appStricmp(Code, TEXT("G")) == 0 )		SlotType = _EP_G;
			else if ( appStricmp(Code, TEXT("K")) == 0 )		SlotType = _EP_K;
			else if ( appStricmp(Code, TEXT("BT")) == 0 )		SlotType = _EP_BT;
			else if ( appStricmp(Code, TEXT("BD")) == 0 )		SlotType = _EP_BD;

			else if ( appStricmp(Code, TEXT("Face")) == 0 )		SlotType = /*_EP_H11 | _EP_H12 | _EP_H2 |*/_EP_H12;
			else if ( appStricmp(Code, TEXT("Helmet")) == 0 )	SlotType = _EP_H1;
			else if ( appStricmp(Code, TEXT("Armor")) == 0 )	SlotType = _EP_BD;
			else if ( appStricmp(Code, TEXT("SlotA")) == 0 )	SlotType = _EP_W1;
			else if ( appStricmp(Code, TEXT("SlotB")) == 0 )	SlotType = _EP_W2;
			else if ( appStricmp(Code, TEXT("SlotC")) == 0 )	SlotType = _EP_W3;
			else if ( appStricmp(Code, TEXT("SlotD")) == 0 )	SlotType = _EP_T1;
			else if ( appStricmp(Code, TEXT("SlotE")) == 0 )	SlotType = _EP_T2;

			if (_StateController->PlayerInfo.IsValid())
			{
				for (INT Index = 0; Index < MAX_INVENTORY_SIZE; Index++)
				{
					if (!_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[Index].IsEmpty())
					{
						ITEM_DESC *ItemDesc = _ItemDesc().GetItem(_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[Index].id);

						//! 얼굴 인벤토리에 출력이 안되서 디버깅중.(2007/12/11)
//						debugf(TEXT("_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[%d] = %d, Slot(%d)==FindSlot(%d)"), Index,
//							_StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[Index].id, (ItemDesc !=NULL ? ItemDesc->slotType : 0), SlotType);

						if( bGetAllCell || (ItemDesc != NULL && ItemDesc->slotType & SlotType) )
							OutElements.AddItem(Index + 10000);
					}
				}
			}

			bResult = (bGetAllCell || SlotType > 0);
		}
	}
	else if (ShopItemsProvider && appStrnicmp(FieldName.GetName(), TEXT("ShopWeapon"), 10) == 0 /* FieldName == FName(TEXT("ShopWeapon")) */)
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 10; Index++)
			{
				OutElements.AddItem(Index);
			}

			bResult = TRUE;
		}
		else
		{
			TCHAR *Code = (TCHAR*)FieldName.GetName() + 10;
			FavaShopItem *pItem = NULL;

			if( appStrlen(Code) == 0)
			{
				ITEM_DESC *pItemDesc = NULL;
				for (INT Index = 0; (pItem = _ShopDesc().GetItemByIndex(Index)) != NULL; ++Index)
				{
					if ( pItem->DisplayType == _IDT_NONE )
						continue;

					pItemDesc = _ItemDesc().GetItem(pItem->GetDefaultItemID());

					// 게임머니를 사용해서 살 수 있는 아이템이 아니면 무시.(2007/12/11)
					if ( pItemDesc == NULL || pItemDesc->priceType != _IPT_MONEY )
						continue;

					if ( ((ITEM_ID*)(&pItemDesc->id))->category <= _IC_HEAVY )
						OutElements.AddItem(Index);
				}
			}
			else if ( appStrlen(Code) == 2 )
			{
				INT ClassMod = -1;
				INT SlotType = 0;
				INT nSlot = (Code[1] - TEXT('0'));
				if( nSlot == 5 || nSlot == 6 )
					nSlot = 4;

				switch(appToUpper(Code[0]))
				{
				case 'P':	ClassMod = 1;	break;
				case 'R':	ClassMod = 0;	break;
				case 'S':	ClassMod = 2;	break;
				}
				if( (0 <= ClassMod && ClassMod <= 2) && 1 <= nSlot && nSlot <= 4 )
				{
					if (_StateController->PlayerInfo.IsValid())
					{
						SlotType = 1 << (ClassMod * 4 + nSlot);
						ITEM_DESC *pItemDesc = NULL;
						for (INT Index = 0; (pItem = _ShopDesc().GetItemByIndex(Index)) != NULL; ++Index)
						{
							if ( pItem->DisplayType == _IDT_NONE )
								continue;

							pItemDesc = _ItemDesc().GetItem(pItem->GetDefaultItemID());

							// 게임머니를 사용해서 살 수 있는 아이템이 아니면 무시.(2007/12/11)
							if ( pItemDesc == NULL || pItemDesc->priceType != _IPT_MONEY )
							{
								//debugf(TEXT("pItemDesc->priceType != _IPT_MONEY"));
								continue;
							}

							if ( ((ITEM_ID*)(&pItemDesc->id))->category <= _IC_HEAVY && 
								 pItemDesc->slotType & SlotType )
							{
								OutElements.AddItem(Index);
							}
						}
					}
				}
			}
			bResult = TRUE;
		}
	}
	else if (ShopItemsProvider && appStrnicmp(FieldName.GetName(), TEXT("ShopEquip"), 9) == 0  /*FieldName == FName(TEXT("ShopEquip"))*/ )
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 10; Index++)
			{
				OutElements.AddItem(Index);
			}

			bResult = TRUE;
		}
		else
		{
			INT SlotType = -1;
			const TCHAR* Code = FieldName.GetName() + 9;
			UBOOL bGetAllCell = FALSE;

			if ( appStricmp(Code, TEXT("")) == 0 )				bGetAllCell = TRUE;
			else if( appStricmp(Code, TEXT("H1")) == 0 )		SlotType = _EP_H1;
			else if ( appStricmp(Code, TEXT("H11")) == 0 )		SlotType = _EP_H11;
			else if ( appStricmp(Code, TEXT("H12")) == 0 )		SlotType = _EP_H12;
			else if ( appStricmp(Code, TEXT("H2")) == 0 )		SlotType = _EP_H2;
			else if ( appStricmp(Code, TEXT("H3")) == 0 )		SlotType = _EP_H3;
			else if ( appStricmp(Code, TEXT("C1")) == 0 )		SlotType = _EP_C1;
			else if ( appStricmp(Code, TEXT("C2")) == 0 )		SlotType = _EP_C2;
			else if ( appStricmp(Code, TEXT("A1")) == 0 )		SlotType = _EP_A1;
			else if ( appStricmp(Code, TEXT("A2")) == 0 )		SlotType = _EP_A2;
			else if ( appStricmp(Code, TEXT("B1")) == 0 )		SlotType = _EP_B1;
			else if ( appStricmp(Code, TEXT("B3")) == 0 )		SlotType = _EP_B3;
			else if ( appStricmp(Code, TEXT("W1")) == 0 )		SlotType = _EP_W1;
			else if ( appStricmp(Code, TEXT("W2")) == 0 )		SlotType = _EP_W2;
			else if ( appStricmp(Code, TEXT("W3")) == 0 )		SlotType = _EP_W3;
			else if ( appStricmp(Code, TEXT("T1")) == 0 )		SlotType = _EP_T1;
			else if ( appStricmp(Code, TEXT("T2")) == 0 )		SlotType = _EP_T2;
			else if ( appStricmp(Code, TEXT("E")) == 0 )		SlotType = _EP_E;
			else if ( appStricmp(Code, TEXT("G")) == 0 )		SlotType = _EP_G;
			else if ( appStricmp(Code, TEXT("K")) == 0 )		SlotType = _EP_K;
			else if ( appStricmp(Code, TEXT("BT")) == 0 )		SlotType = _EP_BT;
			else if ( appStricmp(Code, TEXT("BD")) == 0 )		SlotType = _EP_BD;

			else if ( appStricmp(Code, TEXT("Face")) == 0 )		SlotType = /*_EP_H11 | _EP_H12 | _EP_H2 |*/ _EP_H12;
			else if ( appStricmp(Code, TEXT("Helmet")) == 0 )	SlotType = _EP_H1;
			else if ( appStricmp(Code, TEXT("Armor")) == 0 )	SlotType = _EP_BD;
			else if ( appStricmp(Code, TEXT("SlotA")) == 0 )	SlotType = _EP_W1;
			else if ( appStricmp(Code, TEXT("SlotB")) == 0 )	SlotType = _EP_W2;
			else if ( appStricmp(Code, TEXT("SlotC")) == 0 )	SlotType = _EP_W3;
			else if ( appStricmp(Code, TEXT("SlotD")) == 0 )	SlotType = _EP_T1;
			else if ( appStricmp(Code, TEXT("SlotE")) == 0 )	SlotType = _EP_T2;

			FavaShopItem *pItem = NULL;
			ITEM_DESC *pItemDesc = NULL;
			for (INT Index = 0; (pItem = _ShopDesc().GetItemByIndex(Index)) != NULL; ++Index)
			{
				if ( pItem->DisplayType == _IDT_NONE )
					continue;

				pItemDesc = _ItemDesc().GetItem(pItem->GetDefaultItemID());

				// 게임머니를 사용해서 살 수 있는 아이템이 아니면 무시.(2007/12/11)
				if ( pItemDesc == NULL || pItemDesc->priceType != _IPT_MONEY )
					continue;

				//if (it->second->slotType & _EP_CHARACTER)
				if ( ((ITEM_ID*)(&pItemDesc->id))->category >= _IC_HELMET && ((ITEM_ID*)(&pItemDesc->id))->category <= _IC_LEG)
				{
					if ( bGetAllCell || pItemDesc->slotType & SlotType )
						OutElements.AddItem(Index);
				}
			}

			bResult = (bGetAllCell || SlotType > 0);
		}
	}
	else if ( ShopCustomItemsProvider && appStrnicmp( FieldName.GetName(), TEXT("ShopCustomWeapon"), 16) == 0 )
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 10; Index++)
			{
				OutElements.AddItem(Index);
			}

			bResult = TRUE;
		}
		else
		{
			const TCHAR* szOption = FieldName.GetName() + 16;
			if( appStrnicmp(szOption, TEXT(";"),1) == 0)
			{
				const TCHAR* szNumber = szOption + 1;
				INT InvenID = appAtoi(szNumber);
				if( appStrlen(szNumber) != 0 && ( 0 <= InvenID && InvenID < 10000 ))
				{
					if( _StateController->PlayerInfo.IsValid() )
					{
						ITEM_INFO &item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(InvenID)];
						FavaShopItem *pItem = NULL;
						CUSTOM_ITEM_DESC *pItemDesc = NULL;
						for (INT Index = 0; (pItem = _ShopDesc().GetCustomItemByIndex(Index)) != NULL; ++Index)
						{
							if ( pItem->DisplayType == _IDT_NONE )
								continue;

							pItemDesc = _ItemDesc().GetCustomItem(pItem->GetDefaultItemID());

							// 게임머니를 사용해서 살 수 있는 아이템이 아니면 무시.(2007/12/11)
							if ( pItemDesc == NULL || pItemDesc->priceType != _IPT_MONEY )
								continue;

							if ( item.id == pItemDesc->item_id )
								OutElements.AddItem(Index);
						}

						bResult = TRUE;
					}
				}
			}
		}
	}
	else if ( ShopCustomItemsProvider && appStrnicmp( FieldName.GetName(), TEXT("ShopCustomEquip"), 15 ) == 0 )
	{
		if (GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 10; Index++)
			{
				OutElements.AddItem(Index);
			}
			bResult = TRUE;
		}
	}
	else if ( ShopCustomItemsProvider && appStrnicmp( FieldName.GetName(), TEXT("EquippedCustomItem"), 18) == 0 )
	{
		if( GIsEditor && !GIsGame)
		{
			for (INT Index = 0; Index < 6; Index++)
			{
				OutElements.AddItem(Index);
			}
			bResult = TRUE;
		}
		else
		{
			int InvenID;
			const TCHAR* szOption = FieldName.GetName() + 18;
			if( appStrlen(szOption) == 0)
			{
				for( INT CustomSlotIndex = 0 ; CustomSlotIndex < _CSI_MAX ; CustomSlotIndex++)
				{
					OutElements.AddItem(CustomSlotIndex);
				}
			}
			else if( Parse(szOption, TEXT("InvenID="), InvenID) )
			{
				if(( 0 <= InvenID && InvenID < 10000 ))
				{
					if( _StateController->PlayerInfo.IsValid() )
					{
						CInventory Inven;
						Inven.Init(&_ItemDesc(), &_StateController->PlayerInfo.PlayerInfo.itemInfo);
						ITEM_INFO &item = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(InvenID)];

						*(INT64*)&ShopCustomItemsProvider->ItemSN = item.sn;

						// 선택된 무기(InvenID)에 장착한 CustomParts의 id를 미리 저장해둔다.
						TArray<TID_ITEM> ItemIdList;
						for( INT CustomSlotIndex = 0 ; CustomSlotIndex < _CSI_MAX ; CustomSlotIndex++)
						{
							CUSTOM_ITEM_INFO* cItem = Inven.GetCustomInvenToSlot(item.sn, (Def::CUSTOM_SLOT_IDX)CustomSlotIndex);
							ItemIdList.AddItem( cItem ? cItem->id : -1);
						}

						// 미리 OutElements의 배열크기를 정해주고 값없음(-1)을 써준다
						OutElements.AddZeroed(ItemIdList.Num());
						for( INT i= 0 ; i < ItemIdList.Num() ; i++ )
							OutElements(i) = -1;

						// CustomItemList를 돌면서 장착한 CustomParts의 Index를 가져온다
						CUSTOM_ITEM_DESC *pItemDesc = NULL;
						for (INT Index = 0; (pItemDesc = _ItemDesc().GetCustomItemByIndex(Index)) != NULL; ++Index)
						{
							INT FindIndex = ItemIdList.FindItemIndex(pItemDesc->id);
							if( FindIndex >= 0 )
							{
								OutElements(FindIndex) = Index;
							}
						}
						bResult = TRUE;
					}
				}
			}
			/*CInventory Inven;
			Inven.Init( &_ItemDesc(), &_StateController->>PlayerInfo.PlayerInfo.itemInfo );*/
		}
	}
	else if (ItemDescProvider && appStristr( FieldName.GetName(), TEXT("ItemGraphList") ) == FieldName.GetName())
	{
		INT WeaponItemID = -1, CustomItemID = -1;
		INT InvenID = -1, CustomListIndex = -1, WeaponListIndex = -1;
		ItemDescProvider->WeaponItemID = -1;
		ItemDescProvider->CustomItemID = -1;
		ItemDescProvider->InvenID = -1;
		ItemDescProvider->bHideItemDesc = FALSE;

		ItemDescProvider->WeaponItemID = Parse(FieldName.GetName(), TEXT("WeaponItemID="), WeaponItemID) ? WeaponItemID : -1;
		ItemDescProvider->CustomItemID = Parse(FieldName.GetName(), TEXT("CustomItemID="), CustomItemID) ? CustomItemID : -1;
		FString HideItemDescStr;
		if( Parse(FieldName.GetName(), TEXT("bHideItemDesc="), HideItemDescStr) )
		{
			FString HideItemDescStrLower = HideItemDescStr.Trim().TrimTrailing().ToLower();
			ItemDescProvider->bHideItemDesc = ( HideItemDescStrLower == TEXT("1") || HideItemDescStrLower == TEXT("true") ) ? TRUE : FALSE;
		}

		if( Parse(FieldName.GetName(), TEXT("InvenID="), InvenID ) &&
			( 0 <= InvenID && InvenID < 10000 ) &&
			_StateController != NULL && _StateController->PlayerInfo.IsValid() )
		{
			ItemDescProvider->WeaponItemID = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[GetInvenIndexFromID(InvenID)].id;
			ItemDescProvider->InvenID = InvenID;
		}
		if( Parse(FieldName.GetName(), TEXT("WeaponListIndex="),CustomListIndex) )
		{
			ITEM_DESC *pItemDesc = _ItemDesc().GetItemByIndex(CustomListIndex);
			if (pItemDesc)
				ItemDescProvider->WeaponItemID = pItemDesc->id;
		}
		if( Parse(FieldName.GetName(), TEXT("CustomListIndex="),CustomListIndex) )
		{
			CUSTOM_ITEM_DESC *pItemDesc = _ItemDesc().GetCustomItemByIndex(CustomListIndex);
			if (pItemDesc)
				ItemDescProvider->CustomItemID = pItemDesc->id;
		}

		for ( INT Index = 0 ; Index < MAX_ITEM_GRAPH_LIST - 1; Index++ )
		{
			OutElements.AddItem(Index);
		}
		bResult = TRUE;
	}
	else if ( ItemDescListProvider && appStristr( FieldName.GetName(), TEXT("ItemDescList")) == FieldName.GetName() )
	{
		ItemDescListProvider->ItemId = -1;
		ItemDescListProvider->InvenIndex = -1;
		ItemDescListProvider->CustomSlotIndex = -1;
		ItemDescListProvider->EquipInvenIndex = -1;
		ItemDescListProvider->bHideItemDesc = FALSE;

		if( GIsEditor && !GIsGame )
		{
			OutElements.AddItem(0);
		}
		else
		{
			// InvenID와 ItemID가 모두 인자로 넘어올경우 InvenID에 해당하는 Item의 값을 보여준다.
			INT InvenID = -1;			// InvenID는 InvenListIndex와 같은 의미이다. ( UI의 InventoryList에서 얻어온 collection Index )
			INT EquipInvenID = -1;		// EquipInvenID는 EquipInvenListIndex와 같은 의미이다 (UI의 EquipInventoryList에서 얻어온 collection Index)
			INT InvenIndex = -1;		// PlayerInfo.ItemInfo.WeaponInven[ V ]
			INT EquipInvenIndex = -1;	// PlayerInfo.itemInfo.EquipInven[ V ]
			INT SlotID = -1;			// PlayerInfo.ItemInfo.WeaponSet[ V ]
			INT EquipSlotID = -1;		// PlayerInfo.itemInfo.EquipSet[ V ]
			INT ItemID = -1;
			INT CustomSlotIndex = -1;	// Def::CUSTOM_SLOT_INDEX의 값중 하나 (InvenIndex나 IndexID가 있어야 - 즉 무기가 있어야 적용가능)

			InvenID = Parse(FieldName.GetName(), TEXT("InvenID="), InvenID) ? InvenID : -1;
			InvenIndex = Parse(FieldName.GetName(), TEXT("InvenIndex="),InvenIndex) ? InvenIndex : -1;
			EquipInvenIndex = Parse(FieldName.GetName(), TEXT("EquipInvenID="), EquipInvenID) ? EquipInvenID : -1;
			SlotID = Parse(FieldName.GetName(), TEXT("SlotID="), SlotID) ? SlotID : -1;
			EquipSlotID = Parse(FieldName.GetName(), TEXT("EquipSlotID="), EquipSlotID) ? EquipSlotID : -1;
			ItemID = Parse(FieldName.GetName(), TEXT("ItemID="), ItemID) ? ItemID : -1;
			CustomSlotIndex = Parse(FieldName.GetName(), TEXT("CustomSlotIndex="), CustomSlotIndex) ? CustomSlotIndex : -1;
			ItemDescListProvider->ItemId = ItemID;

			FString HideItemDescStr;
			if( Parse(FieldName.GetName(), TEXT("bHideItemDesc="), HideItemDescStr) )
			{
				FString HideItemDescStrLower = HideItemDescStr.Trim().TrimTrailing().ToLower();
				ItemDescListProvider->bHideItemDesc = ( HideItemDescStrLower == TEXT("1") || HideItemDescStrLower == TEXT("true") ) ? TRUE : FALSE;
			}

			// 세가지 값에 따라 해당하는 무기정보(ITEM_INFO)를 받아온다.
			// 1. SlotID - 병과당 가지고있는 6개의 Slot으로부터 ITEM_INFO를 얻어온다
			// 2. InvenIndex - 플레이어가 가진 무기정보중에 WeaponInven 배열의 인덱스로 부터 ITEM_INFO를 얻어온다.
			// 3. InvenID - 인벤토리 리스트(<avaNet:WeaponInventory>)로부터 얻은 리스트 아이템 인덱스로부터 ITEM_INFO를 얻어온다
			if( 0 <= SlotID && SlotID < MAX_WEAPONSET_SIZE 
				&& _StateController != NULL && _StateController->PlayerInfo.IsValid() 
				&& _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[SlotID] != SN_INVALID_ITEM )
			{
				TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponSet[SlotID];
				for( INT i = 0 ; i < MAX_INVENTORY_SIZE ; i++ )
				{
					if( _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[i].sn == ItemSN )
					{
						InvenIndex = i;
						break;
					}
				}
				ItemDescListProvider->InvenIndex = InvenIndex;
				ItemDescListProvider->ItemId = (0 <= InvenIndex && InvenIndex < MAX_INVENTORY_SIZE ) ? _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenIndex].id : ItemID;
			}
			else if( 0 <= InvenIndex && InvenIndex <= MAX_INVENTORY_SIZE 
				&& _StateController != NULL && _StateController->PlayerInfo.IsValid())
			{
				ItemDescListProvider->InvenIndex = InvenIndex;
				INT NewItemID = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenIndex].id;
				ItemDescListProvider->ItemId = NewItemID >= 0 ? NewItemID : ItemID;
			}
			else if ( ( 0 <= InvenID && InvenID < 10000 ) 
				&& _StateController != NULL && _StateController->PlayerInfo.IsValid() )
			{
				INT NewItemID = _StateController->PlayerInfo.PlayerInfo.itemInfo.weaponInven[InvenIndex = GetInvenIndexFromID(InvenID)].id;
				ItemDescListProvider->ItemId = NewItemID >= 0 ? NewItemID : ItemID;
				ItemDescListProvider->InvenIndex = InvenIndex;
			}
			else if( 0 <= EquipSlotID && EquipSlotID < MAX_EQUIPSET_SIZE
				&& _StateController != NULL && _StateController->PlayerInfo.IsValid() 
				&& _StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[EquipSlotID] != SN_INVALID_ITEM )
			{
				TSN_ITEM ItemSN = _StateController->PlayerInfo.PlayerInfo.itemInfo.equipSet[EquipSlotID];
				for( INT i = 0 ; i < MAX_INVENTORY_SIZE ; i++ )
				{
					if( _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[i].sn == ItemSN )
					{
						EquipInvenIndex = i;
						break;
					}
				}
				ItemDescListProvider->EquipInvenIndex = EquipInvenIndex;
				ItemDescListProvider->ItemId = (0 <= EquipInvenIndex && EquipInvenIndex < MAX_INVENTORY_SIZE ) ? _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[EquipInvenIndex].id : ItemID;
			}
			else if ( EquipInvenID >= 0 && (EquipInvenIndex = GetInvenIndexFromID(EquipInvenID) ) != INDEX_NONE
				&& _StateController != NULL && _StateController->PlayerInfo.IsValid()
				&& EquipInvenIndex < MAX_INVENTORY_SIZE )
			{
				ItemDescListProvider->EquipInvenIndex = EquipInvenIndex;
				ItemDescListProvider->ItemId = (0 <= EquipInvenIndex && EquipInvenIndex < MAX_INVENTORY_SIZE ) ? _StateController->PlayerInfo.PlayerInfo.itemInfo.equipInven[EquipInvenIndex].id : ItemID;
			}

			if( ItemDescListProvider->InvenIndex >= 0 && 0 <= CustomSlotIndex && CustomSlotIndex < _CSI_MAX
				&& _StateController != NULL && _StateController->PlayerInfo.IsValid() )
			{
				ItemDescListProvider->CustomSlotIndex = CustomSlotIndex;
			}

			if( ItemDescListProvider->ItemId >= 0 )
				OutElements.AddItem(ItemDescListProvider->ItemId);
		}

		bResult = TRUE;
	}
	else if ( MiscProvider && FieldName == FName(TEXT("AvailChannelGroupList")) )
	{
		for( INT i = 0 ; i < EChannelFlag_MAX ; i++ )
			OutElements.AddItem(i);
	}
	else if ( MiscProvider && appStristr( FieldName.GetName(), TEXT("AvailChatTypeList") ) == FieldName.GetName() )
	{
		INT IgnoreIndex = INDEX_NONE;
		IgnoreIndex = Parse( FieldName.GetName(), TEXT("Ignore="), IgnoreIndex ) ? IgnoreIndex : INDEX_NONE;

		for( INT i = 0 ; i < 2 ; i++ )
			if( i != IgnoreIndex)
				OutElements.AddItem(i);
	}
	else if ( FilteredWeaponProvider && appStristr(FieldName.GetName(), TEXT("FilteredWeapons")) == FieldName.GetName() )
	{
		FilteredWeaponProvider->DisplayFilter = WEAPONDISPFILTER_NONE;

		if( GIsEditor && !GIsGame )
		{
			INT ItemCount = 0;
			INT DisplayFilter = INDEX_NONE;
			Parse( FieldName.GetName(), TEXT("DisplayFilter="), DisplayFilter);

			if( DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_POINTMAN ||
				DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_RIFLEMAN ||
				DisplayFilter == WEAPONDISPFILTER_EQUIPPED_PRIWEAP_SNIPER )
				ItemCount = 1;
			else if ( DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_POINTMAN ||
				DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_RIFLEMAN ||
				DisplayFilter == WEAPONDISPFILTER_CUSTOMPARTS_PRIWEAP_SNIPER )
				ItemCount = 2;
			else
				ItemCount = 5;

			for(INT i = 0 ; i < ItemCount ; i++)
				OutElements.AddItem(i);
		}
		else
		{
			/* Limit Ratio 포맷 : LimitRatio="0.3,0.5,0.8" // 0.3, 0.5, 0.8 만큼 정비(내구)도가 깎였을때 보여준다*/
			FString LimitRatioStr;
			Parse( FieldName.GetName(), TEXT("LimitRatio="), LimitRatioStr );
			TArray<FString> LimitRatioList;
			LimitRatioStr.ParseIntoArrayWS(&LimitRatioList, TEXT(","));
			TArray<FLOAT> LimitRatio;
			for( INT i = 0 ; i < LimitRatioList.Num() ; i++ )
				LimitRatio.AddUniqueItem(appAtof(*LimitRatioList(i)));

			ava::sort( &LimitRatio(0), &LimitRatio(0) + LimitRatio.Num() );

			INT DisplayFilter;
			Parse( FieldName.GetName(), TEXT("DisplayFilter="), DisplayFilter);

			if( 0 <= DisplayFilter && DisplayFilter < WEAPONDISPFILTER_NONE )
				FilteredWeaponProvider->DisplayFilter = DisplayFilter;

			EFilteredWeapons_DisplayFilter Filter = (EFilteredWeapons_DisplayFilter)FilteredWeaponProvider->DisplayFilter;
			GetOutElementsFromFilter( Filter, OutElements, LimitRatio );
		}
		bResult = TRUE;
	}
	else if ( LastResultMsgProvider && FieldName == TEXT("LastResultMsgs") )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT MsgIdx = 0 ; MsgIdx < LastResultMsgType_MAX ; MsgIdx++ )
			{
				OutElements.AddItem( MsgIdx );
			}
		}
		else
		{
			FLastResultInfo& LastResultInfo = _StateController->LastResultInfo;
			for( INT MsgIdx = 0 ; MsgIdx < LastResultInfo.LastResultMsgData.Num() ; MsgIdx++ )
			{
				BYTE MsgType = LastResultInfo.GetMsgInfo( MsgIdx ).MsgType;
				if( CanApplyLastResult( MsgType ) )
					OutElements.AddItem( MsgType );
			}
		}
	}
	else if ( EffectItemsProvider && appStristr(FieldName.GetName(),TEXT("EffectItems")) == FieldName.GetName() )
	{
		const INT EffectTypes[] = { _IET_GR, _IET_EXP_BOOST, _IET_SP_BOOST, _IET_MONEY_BOOST };
		if( GIsEditor && !GIsGame )
		{
			OutElements.AddUniqueItem( _IET_GR /* PC방 플래그로 대체 */ );
			for( INT ItemID = 0 ; ItemID < 50000 ; ItemID++ )
			{
				EFFECT_ITEM_DESC* EffectItemDesc = _ItemDesc().GetEffectItem( ItemID );
				if( EffectItemDesc != NULL )
					OutElements.AddItem( ItemID );
			}
			//for( INT TypeIdx = 0 ; TypeIdx < ARRAY_COUNT(EffectTypes) ; TypeIdx++ )
			//{
			//	OutElements.AddItem( EffectTypes[TypeIdx] );
			//}
		}
		else
		{
			INT ListIndex=INDEX_NONE;
			if( Parse( FieldName.GetName(), TEXT("ListIndex="), ListIndex) &&
				_StateController->LastResultInfo.RoomResultInfo.IsValidIndex(ListIndex))
			{
				FLastResultInfo::FPlayerResultInfo *Info = &_StateController->LastResultInfo.RoomResultInfo(ListIndex);
				if( Info != NULL )
				{
					if( Info->bPcBang )
						OutElements.AddUniqueItem( _IET_GR /* PC방 플래그로 대체 */ );

					if( CanApplyLastResult() )
					{
						for( INT i = 0 ; i < Info->EffectList.Num() ; i++)
						{
							EFFECT_ITEM_DESC *pDesc = _ItemDesc().GetEffectItem(Info->EffectList(i));
							if (pDesc)
								OutElements.AddUniqueItem( Info->EffectList(i) );
						}
					}
				}
			}
		}
	}

	return bResult;
}


/**
* Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
* Used by the UI editor to know which cells are available for binding to individual list cells.
*
* @param	FieldName		the tag of the list element data provider that we want the schema for.
*
* @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
*			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
*/
TScriptInterface<IUIListElementCellProvider> UUIDataStore_AvaNet::GetElementCellSchemaProvider(FName FieldName)
{
	if ( appStristr( FieldName.GetName(), TEXT("Channels") ) == FieldName.GetName() )
	{
		return ChannelsProvider;
	}
	else if ( appStristr( FieldName.GetName(), TEXT("LobbyRooms")) == FieldName.GetName())
	{
		return LobbyRoomsProvider;
	}
	else if (FieldName == FName(TEXT("LobbyPlayers")))
	{
		return LobbyPlayersProvider;
	}
	else if (FieldName == FName(TEXT("LobbyFriendPlayers")))
	{
		return LobbyFriendPlayersProvider;
	}
	else if (FieldName == FName(TEXT("LobbyBlockedPlayers")))
	{
		return LobbyBlockedPlayersProvider;
	}
	else if (FieldName == FName(TEXT("GuildMembers")) )
	{
		return GuildMembersProvider;
	}
	else if (FieldName == FName(TEXT("ChatMsgs")))
	{
		return ChatMsgsProvider;
	}
	else if (appStrnicmp(FieldName.GetName(), TEXT("InventoryWeapon"), 15) == 0 || 
		appStrnicmp(FieldName.GetName(), TEXT("InventoryEquip"),14) == 0 )
	{
		return InventoryProvider;
	}
	else if (appStrnicmp(FieldName.GetName(), TEXT("ShopWeapon"), 10) == 0 || 
		appStrnicmp(FieldName.GetName(), TEXT("ShopEquip"), 9) == 0 )
	{
		return ShopItemsProvider;
	}
	else if (appStrnicmp(FieldName.GetName(), TEXT("ShopCustomWeapon"), 16) == 0 || 
		appStrnicmp(FieldName.GetName(), TEXT("ShopCustomEquip"), 15) == 0 || 
		appStrnicmp(FieldName.GetName(), TEXT("EquippedCustomItem"),18) == 0 )
	{
		return ShopCustomItemsProvider;
	}
	else if (appStristr(FieldName.GetName(), TEXT("ItemGraphList")) == FieldName.GetName() )
	{
		return ItemDescProvider;
	}
	else if ( appStristr(FieldName.GetName(), TEXT("ItemDescList")) == FieldName.GetName() )
	{
		return ItemDescListProvider;
	}
	else if ( FieldName == FName(TEXT("AvailChannelGroupList")) || 
		appStristr(FieldName.GetName(), TEXT("AvailChatTypeList")) == FieldName.GetName())
	{
		return MiscProvider;
	}
	else if ( FieldName == FName(TEXT("LastResultMsgs")) )
	{
		return LastResultMsgProvider;
	}
	else if ( appStristr( FieldName.GetName(), TEXT("EffectItems") ) == FieldName.GetName() )
	{
		return EffectItemsProvider;
	}
	else if ( appStristr( FieldName.GetName(), TEXT("FilteredWeapons") ) == FieldName.GetName() )
	{
		return FilteredWeaponProvider;
	}


	return TScriptInterface<IUIListElementCellProvider>();
}

/**
* Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
* of the list element indicated by CellValueProvider.DataSourceIndex
*
* @param	FieldName		the tag of the list element data field that we want the values for
* @param	ListIndex		the list index for the element to get values for
* 
* @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
*/
TScriptInterface<IUIListElementCellProvider> UUIDataStore_AvaNet::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	return GetElementCellSchemaProvider(FieldName);
}

/**
* Resolves PropertyName into a list element provider that provides list elements for the property specified.
*
* @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
*
* @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
*			there is no list element provider associated with the specified property.
*/
TScriptInterface<class IUIListElementProvider> UUIDataStore_AvaNet::ResolveListElementProvider(const FString& PropertyName)
{
	return ResolveListElementProviderRecursion( this, PropertyName );
}

TScriptInterface<class IUIListElementProvider> UUIDataStore_AvaNet::ResolveListElementProviderRecursion( UUIDataProvider* DataProvider, const FString& PartialPropertyName )
{
	check(DataProvider);

	INT FindIndex = PartialPropertyName.InStr( TEXT(".") );
	FString FieldStr = FindIndex == INDEX_NONE ? PartialPropertyName : PartialPropertyName.Right(PartialPropertyName.Len() - FindIndex - 1);

	if ( FindIndex == INDEX_NONE )
	{
		IUIListElementProvider* ListElementInterface = (IUIListElementProvider*)DataProvider->GetInterfaceAddress( IUIListElementProvider::UClassType::StaticClass() );
		if( ! ListElementInterface )
		{
			return TScriptInterface<class IUIListElementProvider>();
		}
		else
		{
			TArray<FUIDataProviderField> OutFields;
			DataProvider->GetSupportedDataFields( OutFields );
			for( INT FieldIndex = 0 ; FieldIndex < OutFields.Num() ; FieldIndex++ )
			{
				if( OutFields(FieldIndex).FieldType == DATATYPE_Collection &&
					OutFields(FieldIndex).FieldTag == *PartialPropertyName )
				{
					TScriptInterface<class IUIListElementProvider> ScriptInterface;
					ScriptInterface.SetObject( DataProvider );
					ScriptInterface.SetInterface( ListElementInterface );
					return ScriptInterface;
				}
			}
		}
	}
	else
	{
		FString ProviderName = PartialPropertyName.Left(FindIndex);

		TArray<FUIDataProviderField> OutFields;
		DataProvider->GetSupportedDataFields( OutFields );
		for( INT FieldIndex = 0 ; FieldIndex < OutFields.Num() ; FieldIndex++ )
		{
			const FUIDataProviderField& DataField = OutFields(FieldIndex);
			TArray<UUIDataProvider*> Providers;

			if( DataField.FieldTag == *ProviderName &&
				DataField.FieldType == DATATYPE_Provider &&
				DataField.GetProviders( Providers ) )
			{
				for( INT ProviderIndex = 0 ; ProviderIndex < Providers.Num() ; ProviderIndex++ )
				{
					TScriptInterface<class IUIListElementProvider> ScriptInterface;
					ScriptInterface = ResolveListElementProviderRecursion( Providers(ProviderIndex), FieldStr );
					if( ScriptInterface )
						return ScriptInterface;
				}
			}
		}
	}

	return TScriptInterface<class IUIListElementProvider>(this);
}

/**
* Allows list element providers the chance to perform custom sorting of a collection of list elements.  Implementors should implement this
* method if they desire to perform complex sorting behavior, such as considering additional data when evaluting the order to sort the elements into.
*
* @param	CollectionDataFieldName		the name of a collection data field inside this UIListElementProvider associated with the
*										list items provided.  Guaranteed to one of the values returned from GetElementProviderTags.
* @param	ListItems					the array of list items that need sorting.
* @param	OutputType	determines the format of the result.
*						EVALPOS_None:
*							return value is formatted using this screen position's ScaleType for the specified face
*						EVALPOS_PercentageOwner:
* @param	SortParameters				the parameters to use for sorting
*										PrimaryIndex:
*											the index [into the ListItems' Cells array] for the cell which the user desires to perform primary sorting with.
*										SecondaryIndex:
*											the index [into the ListItems' Cells array] for the cell which the user desires to perform secondary sorting with.  Not guaranteed
*											to be a valid value; Comparison should be performed using the value of the field indicated by PrimarySortIndex, then when these
*											values are identical, the value of the cell field indicated by SecondarySortIndex should be used.
*
* @return	TRUE to indicate that custom sorting was performed by this UIListElementProvider.  Custom sorting is not required - if this method returns FALSE,
*			the list bound to this UIListElementProvider will perform its default sorting behavior (alphabetical sorting of the desired cell values)
*/
UBOOL UUIDataStore_AvaNet::SortListElements( FName CollectionDataFieldName, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	if( ListItems.Num() == 0 || ListItems(0).Cells.Num() == 0 )
		return FALSE;

	UUIList *OwnerList = ListItems(0).Cells(0).OwnerList;
	UUIComp_ListPresenter* ListPresenter = OwnerList ? OwnerList->CellDataComponent : NULL;
	if( ListPresenter == NULL )
		return FALSE;

	FUIElementCellSchema& Schema = ListPresenter->ElementSchema;
	TScriptInterface<class IUIListElementCellProvider> CellProvider = GetElementCellSchemaProvider( CollectionDataFieldName );

	// 해당 정렬용 프로바이더를 찾지 못한 경우
	// ResolveListElementProvider를 찾아 사용한다.
	if( ! CellProvider )
	{
		TScriptInterface<class IUIListElementProvider> ListProvider;
		UUIDataProvider* DataProvider;
		DataProvider = Cast<UUIDataProvider>(ResolveListElementProvider( FString( CollectionDataFieldName.GetName() ) ).GetObject());
		IUIListElementCellProvider* CellProviderInterface = NULL;
		if( DataProvider &&
			(CellProviderInterface = (IUIListElementCellProvider*)DataProvider->GetInterfaceAddress( IUIListElementProvider::UClassType::StaticClass() ) ) != NULL )
		{
			CellProvider.SetObject( DataProvider );
			CellProvider.SetInterface( CellProviderInterface );
		}
	}

	FUIListSortingParameters SortParms = SortParameters;

	for( INT CellIndex = 0 ; CellIndex < Schema.Cells.Num() ; CellIndex++)
	{
		FName CellTagName = Schema.Cells(CellIndex).CellDataField;
		if( CellProvider == LobbyRoomsProvider )
		{
			if( CellTagName == FName(TEXT("RoomID")))
			{
				SortParms.SecondaryIndex = CellIndex;
				break;
			}
		}
		else if ( CellProvider == ChannelsProvider )
		{
			if( CellTagName == FName(TEXT("ChannelName")) )
			{
				SortParms.SecondaryIndex = CellIndex;
				break;
			}
		}
		else if ( CellProvider == LobbyPlayersProvider  || 
			CellProvider == LobbyFriendPlayersProvider || 
			CellProvider == LobbyBlockedPlayersProvider || 
			CellProvider == GuildMembersProvider ||
			Cast<UUIDataProvider_AvaNetReadyRoomPlayers>(CellProvider.GetObject()) )
		{
			if( CellTagName == FName(TEXT("NickName")) || CellTagName == FName(TEXT("NickNameCombo")) )
			{
				SortParms.SecondaryIndex = CellIndex;
				break;
			}
		}
	}

	FName EmptyName;
	return CellProvider->SortListElements( Schema.Cells.IsValidIndex(SortParms.PrimaryIndex) ? Schema.Cells(SortParms.PrimaryIndex).CellDataField : EmptyName,
		Schema.Cells.IsValidIndex(SortParms.SecondaryIndex) ? Schema.Cells(SortParms.SecondaryIndex).CellDataField : EmptyName, ListItems, SortParms);
}
