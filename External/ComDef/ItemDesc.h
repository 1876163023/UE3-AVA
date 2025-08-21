#pragma once

#include "Def.h"
using namespace Def;

#pragma warning(disable:4702)

/////////////////////////////////////////////////////////////////
// Item description data container

class IItemDescData
{
public:
	virtual BOOL Init() = 0;

	virtual ITEM_DESC* GetItem(TID_ITEM) = 0;
	virtual ITEM_DESC* GetItemByIndex(int) = 0;
	virtual CUSTOM_ITEM_DESC* GetCustomItem(TID_ITEM) = 0;
	virtual CUSTOM_ITEM_DESC* GetCustomItemByIndex(int) = 0;
	virtual EFFECT_ITEM_DESC* GetEffectItem(TID_ITEM) = 0;
	virtual EFFECT_ITEM_DESC* GetEffectItemByIndex(int) = 0;

	virtual BOOL FaceExists(TID_ITEM) = 0;
	
	virtual SLOT_DESC* GetWeaponSlot(TID_EQUIP_SLOT) = 0;
	virtual SLOT_DESC* GetWeaponSlotByType(int) = 0;
	virtual SLOT_DESC* GetEquipSlot(TID_EQUIP_SLOT) = 0;
	virtual SLOT_DESC* GetEquipSlotByType(int) = 0;
	virtual SLOT_DESC* GetEffectSlot(TID_EQUIP_SLOT) = 0;
	virtual SLOT_DESC* GetEffectSlotByType(int) = 0;	
};


/////////////////////////////////////////////////////////////////
// Item description logic

class CItemDesc
{
public:
	//CItemDesc(BOOL bAutoInit = TRUE, BOOL bEncrypted = FALSE);
	CItemDesc(IItemDescData *pData = NULL, BOOL bAutoInit = TRUE);
	virtual ~CItemDesc();

	//BOOL Init(TCHAR* pathItemDesc=NULL, TCHAR* pathSlotDesc=NULL, BOOL bEncrypted = FALSE);
	void SetDescData(IItemDescData *pData = NULL);
	IItemDescData* GetDescData() const { return pDescData; }

	BOOL Init();

	ITEM_DESC* GetItem(TID_ITEM item_id) { return pDescData->GetItem(item_id); }
	ITEM_DESC* GetItemByIndex(int index) { return pDescData->GetItemByIndex(index); }
	CUSTOM_ITEM_DESC* GetCustomItem(TID_ITEM item_id) { return pDescData->GetCustomItem(item_id); }
	CUSTOM_ITEM_DESC* GetCustomItemByIndex(int index) { return pDescData->GetCustomItemByIndex(index); }
	EFFECT_ITEM_DESC* GetEffectItem(TID_ITEM item_id) { return pDescData->GetEffectItem(item_id); }
	EFFECT_ITEM_DESC* GetEffectItemByIndex(int index) { return pDescData->GetEffectItemByIndex(index); }

	BOOL FaceExists(TID_ITEM face_id) { return pDescData->FaceExists(face_id); }

	TID_ITEM GetDefaultItem(BYTE flag,TID_EQUIP_SLOT slot);
	TID_ITEM GetDefaultCustomItem(TID_ITEM item_id/*,CUSTOM_SLOT_IDX slot*/);

	SLOT_DESC* GetWeaponSlot(TID_EQUIP_SLOT slot) { return pDescData->GetWeaponSlot(slot); }
	SLOT_DESC* GetWeaponSlotByType(int slot_type) { return pDescData->GetWeaponSlotByType(slot_type); }
	SLOT_DESC* GetEquipSlot(TID_EQUIP_SLOT slot) { return pDescData->GetEquipSlot(slot); }
	SLOT_DESC* GetEquipSlotByType(int slot_type) { return pDescData->GetEquipSlotByType(slot_type); }
	SLOT_DESC* GetEffectSlot(TID_EQUIP_SLOT slot) { return pDescData->GetEffectSlot(slot); }
	SLOT_DESC* GetEffectSlotByType(int slot_type) { return pDescData->GetEffectSlotByType(slot_type); }

	_PLAYER_CLASS GetSlotClassType(TID_EQUIP_SLOT slot);

	BOOL IsDestroyable(TID_ITEM item_id);

	BOOL IsAvailableSlot(TID_ITEM item_id,TID_EQUIP_SLOT slot);
	BOOL IsAvailableCustomSlot(TID_ITEM item_id,TID_ITEM custom_id,CUSTOM_SLOT_IDX slot);
	
	BOOL IsRefund(TID_ITEM item_id);
	BOOL IsCustomEnable(TID_ITEM item_id);

	BOOL IsInit() const { return bInit; }

	BOOL IsDateLimitItem(TID_ITEM item_id);	

private:
	IItemDescData *pDescData;
	BOOL bInit;
};


#pragma warning(default:4702)

