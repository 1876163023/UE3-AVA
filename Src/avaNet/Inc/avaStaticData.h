/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: avaNet

	Name: avaStaticData.h

	Description: Declaration of classes which handling static data - used by avaNet

***/
#pragma once


#include "ComDef/ItemDesc.h"
#include "ComDef/WordCensor.h"


INT LoadStringsFromFile(TArray<FString> &OutArray, TCHAR *FileName, UBOOL bEncrypted = TRUE);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemDesc

const size_t MAX_ITEM_GRAPH_LIST = 8;

template<typename T>
struct FavaItemDescT : public T
{
public:
	typedef T _BaseT;
	typedef FavaItemDescT<T> _MyT;

	FString ItemName;
	FString Description;
	WCHAR IconChar;
	SHORT GraphValue[MAX_ITEM_GRAPH_LIST];

	FavaItemDescT(TID_ITEM _id = ID_INVALID_ITEM, const _MyT *_parent = NULL, DWORD _sales_id = 0) : _BaseT(_id, _parent, _sales_id)
	{
		appMemzero(GraphValue, sizeof(GraphValue));
	}
	virtual ~FavaItemDescT() {}

private:
	virtual const WCHAR* _GetName() const
	{
		return *ItemName;
	}
	virtual const WCHAR* _GetDescription() const
	{
		return *Description;
	}
	virtual WCHAR _GetIcon() const
	{
		return IconChar;
	}
	virtual short _GetGraphValue(int idx) const
	{
		return idx >= 0 && idx < MAX_ITEM_GRAPH_LIST ? GraphValue[idx] : 0;
	}
};

typedef FavaItemDescT<ITEM_DESC> FavaItemDesc;
typedef FavaItemDescT<CUSTOM_ITEM_DESC> FavaCustomItemDesc;
typedef FavaItemDescT<EFFECT_ITEM_DESC> FavaEffectItemDesc;


class CavaItemDescData : public IItemDescData
{
public:
	CavaItemDescData();
	~CavaItemDescData();

	BOOL Init();
	void Final();

	ITEM_DESC* GetItem(TID_ITEM idItem);
	ITEM_DESC* GetItemByIndex(int index);
	CUSTOM_ITEM_DESC* GetCustomItem(TID_ITEM idItem);
	CUSTOM_ITEM_DESC* GetCustomItemByIndex(int index);
	EFFECT_ITEM_DESC* GetEffectItem(TID_ITEM idItem);
	EFFECT_ITEM_DESC* GetEffectItemByIndex(int index);

	BOOL FaceExists(TID_ITEM idFace);

	SLOT_DESC* GetWeaponSlot(TID_EQUIP_SLOT slot);
	SLOT_DESC* GetWeaponSlotByType(int slot_type);
	SLOT_DESC* GetEquipSlot(TID_EQUIP_SLOT slot);
	SLOT_DESC* GetEquipSlotByType(int slot_type);
	SLOT_DESC* GetEffectSlot(TID_EQUIP_SLOT slot);
	SLOT_DESC* GetEffectSlotByType(int slot_type);

	INT GetAvailableWeaponsBySlot(INT idSlot, TArray<INT>& ItemList);
	INT GetAvailableEquipsBySlot(INT idSlot, TArray<INT>& ItemList);
	INT GetAvailableEffectsBySlot(INT idSlot, TArray<INT>& ItemList);

	void DumpFace();
	void DumpWeapon();
	void DumpEquip();
	void DumpCustom();
	void DumpEffect();

private:
	typedef TMap<TID_ITEM, FavaItemDesc*> TMAP_ITEMLIST;
	typedef TMap<TID_ITEM, FavaCustomItemDesc*> TMAP_CUSTOMITEMLIST;
	typedef TMap<TID_ITEM, FavaEffectItemDesc*> TMAP_EFFECTITEMLIST;
	typedef TArray<TID_ITEM> TSET_FACELIST;

	TMAP_ITEMLIST itemList;
	TArray<FavaItemDesc*> itemArray;
	TMAP_CUSTOMITEMLIST customItemList;
	TArray<FavaCustomItemDesc*> customItemArray;
	TMAP_EFFECTITEMLIST effectItemList;
	TArray<FavaEffectItemDesc*> effectItemArray;
	TSET_FACELIST faceList;

	SLOT_DESC equipSlotDesc[MAX_EQUIPSET_SIZE];
	SLOT_DESC weaponSlotDesc[MAX_WEAPONSET_SIZE];
	SLOT_DESC effectSlotDesc[MAX_EFFECTSET_SIZE];

	BOOL bInit;
};



inline CItemDesc& _ItemDesc()
{
	static CavaItemDescData _itemDescData;
	static CItemDesc _itemDesc(&_itemDescData);
	return _itemDesc;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ShopDesc

typedef WORD TID_SHOPITEM;
const TID_SHOPITEM ID_INVALID_SHOPITEM = 0;

class CavaShopDesc;

struct FavaShopItem
{
public:
	friend CavaShopDesc;

	struct FavaShopOption
	{
		TID_ITEM OptionID;
		FString OptionName;
		BASE_ITEM_DESC *pItem;

		FavaShopOption() : OptionID(ID_INVALID_ITEM), pItem(NULL) {}
		FavaShopOption(TID_ITEM _id, const FString &_name) : OptionID(_id), OptionName(_name), pItem(NULL) {}
	};

	TID_SHOPITEM ShopItemID;
	TArray<FavaShopOption> Options;
	INT DisplayType;					// _ITEM_DISPLAY_TYPE_
	DWORD DisplayPriority;
	WCHAR Icon;

private:
	FString ShopItemName;
	FString Description;

public:
	FavaShopItem(TID_SHOPITEM _id = ID_INVALID_SHOPITEM) : ShopItemID(_id), DisplayType(_IDT_NONE), DisplayPriority(0), Icon(0) {}

	UBOOL IsValid() const
	{
		return ShopItemID != ID_INVALID_SHOPITEM && Options.Num() > 0 && DisplayType != _IDT_NONE;
	}

	FString GetName() const
	{
		return ShopItemName.Len() > 0 ? ShopItemName : (Options.Num() > 0 && Options(0).pItem) ? Options(0).pItem->GetName() : TEXT("");
	}
	FString GetDescription() const
	{
		return Description.Len() > 0 ? Description : (Options.Num() > 0 && Options(0).pItem) ? Options(0).pItem->GetDescription() : TEXT("");
	}
	BYTE GetItemFlag() const
	{
		return Options.Num() > 0 && Options(0).pItem ? Options(0).pItem->GetItemFlag() : _IF_NONE;
	}
	TID_ITEM GetDefaultItemID() const
	{
		return Options.Num() > 0 ? Options(0).OptionID : ID_INVALID_ITEM;
	}
	BASE_ITEM_DESC* GetDefaultItem() const
	{
		return Options.Num() > 0 ? Options(0).pItem : NULL;
	}
	TID_ITEM GetItemID(INT OptionIndex=0) const
	{
		return Options.Num() > OptionIndex ? Options(OptionIndex).OptionID : ID_INVALID_ITEM;
	}
	const FString& GetOptionName(INT OptionIndex=0) const
	{
		static FString temp(TEXT(" "));
		return Options.Num() > OptionIndex ? Options(OptionIndex).OptionName : temp;
	}
};

class CavaShopDesc
{
public:
	CavaShopDesc();
	~CavaShopDesc();

	UBOOL Init();
	void Final();

	FavaShopItem* GetItem(TID_SHOPITEM idItem);
	FavaShopItem* GetItemByIndex(INT index);
	FavaShopItem* GetCustomItem(TID_SHOPITEM idItem);
	FavaShopItem* GetCustomItemByIndex(INT index);
	FavaShopItem* GetEffectItem(TID_SHOPITEM idItem);
	FavaShopItem* GetEffectItemByIndex(INT index);

	UBOOL IsInit() const { return bInit; }

	void DumpWeapon();
	void DumpEquip();
	void DumpCustom();
	void DumpEffect();

private:
	typedef TMap<TID_SHOPITEM, FavaShopItem*> TMAP_SHOPLIST;

	TMAP_SHOPLIST shopItemList;
	TArray<FavaShopItem*> shopItemArray;
	TMAP_SHOPLIST shopCustomItemList;
	TArray<FavaShopItem*> shopCustomItemArray;
	TMAP_SHOPLIST shopEffectItemList;
	TArray<FavaShopItem*> shopEffectItemArray;

private:
	UBOOL bInit;
};

inline CavaShopDesc& _ShopDesc()
{
	static CavaShopDesc _shopDesc;
	return _shopDesc;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WordCensor

struct CavaFilterNode
{
	CavaFilterNode(TCHAR _value = TEXT(''), BOOL _end = FALSE) : value(_value), end(_end) {}
	~CavaFilterNode() {}

	typedef TMap<TCHAR, CavaFilterNode> TMAP_NODELIST;

	TCHAR value;
	BOOL end;
	TMAP_NODELIST mapNext;
};


class CavaWordFilter : public IWordFilter
{
public:
	CavaWordFilter(TCHAR *path);
	~CavaWordFilter();

	BOOL Init();
	void SetIgnoreCh(TCHAR ch);
	void ReplaceIgnoreCh(TCHAR *str);

	void SetCensorWord(TCHAR *str);
	BOOL IsIncludeCurse(TCHAR *str);
	BOOL ReplaceCensorWord(TCHAR *str);

private:
	typedef TMap<TCHAR, TCHAR> TSET_CHARLIST;

	FString FilePath;
	CavaFilterNode root;
	TSET_CHARLIST setIgnoreCh;
};


inline CWordCensor& _WordCensor()
{
	static CavaWordFilter _filterChatRoomName(TEXT("W2.TXT"));
	static CWordCensor _wordCensor(NULL, &_filterChatRoomName);
	return _wordCensor;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TickerMsg

class CavaTickerMsg
{
public:
	CavaTickerMsg();
	~CavaTickerMsg();

	UBOOL Init();
	//UBOOL Save();

	void AddPlain(TCHAR *Msg);
	void Add(TCHAR *Msg, const INT MsgType = 0);
	void Clear();
	void Dump();
	INT Num() { return MsgList.Num(); }
	UBOOL IsDirty() const { return bDirty; }
	UBOOL IsInit() const { return bInit; }

	FString operator[](INT Idx)
	{
		if (Idx >= MsgList.Num())
			return TEXT("");

		TStringList::TIterator It(MsgList.GetHead());
		for (INT i = 0; i < Idx; ++i)
		{
			++It;
		}
		return *It;
	}

private:
	typedef TDoubleLinkedList<FString> TStringList;

	UBOOL bInit;
	TStringList MsgList;
	UBOOL bDirty;
};

inline CavaTickerMsg& _TickerMsg()
{
	static CavaTickerMsg _tickerMsg;
	return _tickerMsg;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RoomName

class CavaRoomName
{
public:
	CavaRoomName();
	~CavaRoomName();

	UBOOL Init();
	FString GetRandomRoomName();

	UBOOL IsInit() const { return bInit; }

private:
	TArray<FString> RoomNameList;
	UBOOL bInit;
};

inline CavaRoomName& _RoomName()
{
	static CavaRoomName _roomName;
	return _roomName;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AwardDesc

struct FAwardInfo
{
	BYTE id;
	BYTE Grade;
	BYTE Type;
	INT CheckProperty;
	INT CheckValue;

	FString AwardName;
	FString Desc;
	FString RewardDesc;

	FAwardInfo() : id(Def::ID_INVALID_AWARD), Grade(UCHAR_MAX), Type(UCHAR_MAX), CheckProperty(0), CheckValue(-1) {}
};

class CavaAwardDesc
{
public:
	enum _CheckProperty
	{
		_Check_None,

		_Check_Level,
		_Check_XP,
		_Check_GameWin,
		_Check_GameDefeat,
		_Check_RoundWin,
		_Check_RoundDefeat,
		_Check_DisconnectCount,
		_Check_DeathCount,
		_Check_StraightWinCount,
		_Check_TeamKillCount,
		_Check_PlayTime,

		_Check_Score_Attacker,
		_Check_Score_Defender,
		_Check_Score_Leader,
		_Check_Score_Tactic,

		_Check_P_PlayRound,
		_Check_P_HeadshotCount,
		_Check_P_HeadshotKillCount,
		_Check_P_PlayTime,
		_Check_P_SprintTime,
		_Check_P_TakenDamage,
		_Check_P_KillCount,
		_Check_P_WeaponKillCount_Pistol,
		_Check_P_WeaponKillCount_Knife,
		_Check_P_WeaponKillCount_Grenade,
		_Check_P_WeaponKillCount_Primary,
		_Check_P_WeaponDamage_Pistol,
		_Check_P_WeaponDamage_Knife,
		_Check_P_WeaponDamage_Grenade,
		_Check_P_WeaponDamage_Primary,

		_Check_R_PlayRound,
		_Check_R_HeadshotCount,
		_Check_R_HeadshotKillCount,
		_Check_R_PlayTime,
		_Check_R_SprintTime,
		_Check_R_TakenDamage,
		_Check_R_KillCount,
		_Check_R_WeaponKillCount_Pistol,
		_Check_R_WeaponKillCount_Knife,
		_Check_R_WeaponKillCount_Grenade,
		_Check_R_WeaponKillCount_Primary,
		_Check_R_WeaponDamage_Pistol,
		_Check_R_WeaponDamage_Knife,
		_Check_R_WeaponDamage_Grenade,
		_Check_R_WeaponDamage_Primary,

		_Check_S_PlayRound,
		_Check_S_HeadshotCount,
		_Check_S_HeadshotKillCount,
		_Check_S_PlayTime,
		_Check_S_SprintTime,
		_Check_S_TakenDamage,
		_Check_S_KillCount,
		_Check_S_WeaponKillCount_Pistol,
		_Check_S_WeaponKillCount_Knife,
		_Check_S_WeaponKillCount_Grenade,
		_Check_S_WeaponKillCount_Primary,
		_Check_S_WeaponDamage_Pistol,
		_Check_S_WeaponDamage_Knife,
		_Check_S_WeaponDamage_Grenade,
		_Check_S_WeaponDamage_Primary,

		_Check_P_SkillMaster,
		_Check_R_SkillMaster,
		_Check_S_SkillMaster,
	};

	CavaAwardDesc();
	~CavaAwardDesc();

	UBOOL Init();

	FAwardInfo* GetAwardInfo(BYTE id);
	UBOOL IsInit() const { return bInit; }

	static INT GetCheckProperty(const TCHAR *prop);

private:
	TArray<FAwardInfo> AwardList;
	UBOOL bInit;
};

inline CavaAwardDesc& _AwardDesc()
{
	static CavaAwardDesc _awardDesc;
	return _awardDesc;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SkillDesc

struct FSkillInfo
{
	BYTE id;
	BYTE TypeID;
	FString SkillName;
	FString TypeName;
	FString CondDesc;
	FString EffectDesc;

	FSkillInfo() : id(Def::ID_INVALID_SKILL), TypeID(UCHAR_MAX) {}
};

class CavaSkillDesc
{
public:
	CavaSkillDesc();
	~CavaSkillDesc();

	UBOOL Init();

	FSkillInfo* GetSkillInfo(BYTE idClass, BYTE id);

	UBOOL IsInit() const { return bInit; }

private:
	TArray<FSkillInfo> SkillList[_CLASS_MAX];
	UBOOL bInit;
};

inline CavaSkillDesc& _SkillDesc()
{
	static CavaSkillDesc _skillDesc;
	return _skillDesc;
}
