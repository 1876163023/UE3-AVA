#pragma once

#include "def.h"
using namespace Def;

class CItemDesc;

const int MAX_EFFECT_SIZE = 10;

class CInventory
{	
public:
	CInventory(/*TID_ACCOUNT acc_id,PLAYER_ITEM_INFO* itemInfo,TPOINT* point,TCASH* cash*/);
	virtual ~CInventory();

	BOOL Init(CItemDesc* itemDesc, PLAYER_ITEM_INFO* itemInfo);

	BOOL AddInven(ITEM_INFO* item);
	BOOL DelInven(TSN_ITEM item_sn,TID_EQUIP_SLOT& equipedSlot,BOOL bForce=FALSE);


	BOOL AddEquipInven(ITEM_INFO* item);
	BOOL DelEquipInven(TSN_ITEM item_sn,TID_EQUIP_SLOT& equipedSlot,BOOL bForce=FALSE);

	BOOL AddWeaponInven(ITEM_INFO* item);
	BOOL DelWeaponInven(TSN_ITEM item_sn,TID_EQUIP_SLOT& equipedSlot,BOOL bForce=FALSE);

	BOOL AddCustomInven(CUSTOM_ITEM_INFO* item);
	BOOL DelCustomInven(TSN_ITEM item_sn,CUSTOM_SLOT_IDX slot);

	BOOL AddEffectInven(EFFECT_ITEM_INFO* item);
	BOOL DelEffectInven(TSN_ITEM item_sn,TID_EQUIP_SLOT& equipedSlot,BOOL bForce=FALSE);

	BOOL EquipSet(TID_EQUIP_SLOT equipSlot,TSN_ITEM item_sn,TSN_ITEM* changedItem=NULL);
	BOOL WeaponSet(TID_EQUIP_SLOT equipSlot,TSN_ITEM item_sn,TSN_ITEM* changedItem=NULL);
	BOOL CustomSet(TSN_ITEM item_sn,CUSTOM_SLOT_IDX slot,TID_ITEM custom_id,int limit=ITEM_LIMIT_INITED);
	BOOL EffectSet(TID_EQUIP_SLOT equipSlot,TSN_ITEM item_sn,TSN_ITEM* changedItem=NULL);

	ITEM_INFO* GetEquipInven(TSN_ITEM item_sn);
	ITEM_INFO* GetWeaponInven(TSN_ITEM item_sn);
	EFFECT_ITEM_INFO* GetEffectInven(TSN_ITEM item_sn);

	ITEM_INFO* GetInvenToSN(TSN_ITEM item_sn);
	TID_ITEM	GetItemIdToSN(TSN_ITEM item_sn);
	ITEM_DESC*	GetItemDescToSN(TSN_ITEM item_sn);

	EFFECT_ITEM_INFO* GetEffectInvenToSN(TSN_ITEM item_sn);
	TID_ITEM	GetEffectItemIdToSN(TSN_ITEM item_sn);
	EFFECT_ITEM_DESC*	GetEffectItemDescToSN(TSN_ITEM item_sn);
	
	int GetWeaponInvenSize();
	int GetEquipInvenSize();
	int GetCustomInvenSize();
	int GetEffectInvenSize();

	CUSTOM_ITEM_INFO* GetCustomInven(TSN_ITEM item_sn,TID_ITEM custom_id);
	CUSTOM_ITEM_INFO* GetCustomInvenToSlot(TSN_ITEM item_sn,CUSTOM_SLOT_IDX slot);
	CUSTOM_SLOT_TYPE  GetExistCustomItem(TSN_ITEM item_sn);

	// 젤 처음 나오는 아이탬을 리턴한다.
	ITEM_INFO* GetFirstItem(TID_ITEM item_id);
	EFFECT_ITEM_INFO* GetFirstEffectItem(TID_ITEM item_id);

	ITEM_INFO* GetEquipSet(TID_EQUIP_SLOT slot);
	ITEM_INFO* GetWeaponSet(TID_EQUIP_SLOT slot);
	EFFECT_ITEM_INFO* GetEffectSet(TID_EQUIP_SLOT slot);

	BOOL IsExistEquipSet(TID_EQUIP_SLOT slot);
	BOOL IsExistWeaponSet(TID_EQUIP_SLOT slot);
	BOOL IsExistEffectSet(TID_EQUIP_SLOT slot);

	TID_EQUIP_SLOT GetEquipSlot(TSN_ITEM sn);
	TID_EQUIP_SLOT GetWeaponSlot(TSN_ITEM sn);
	TID_EQUIP_SLOT GetEffectSlot(TSN_ITEM sn);

	TMONEY GetItemRepairMoney(TSN_ITEM sn);			// 실제 아이탬이 수리돼는 비용..

	BOOL ItemRepair(TSN_ITEM sn);					// 아이탬 수리
	BOOL ItemConvertRis(TSN_ITEM sn,TSN_ITEM new_sn);

	BOOL ItemBuy(ITEM_INFO* item);
	BOOL EffectItemBuy(EFFECT_ITEM_INFO* item);


	BOOL IsRefund(TSN_ITEM sn);
	BOOL IsRefund(TSN_ITEM sn,CUSTOM_SLOT_IDX slot);

	TMONEY GetItemRefundMoney(TSN_ITEM sn);

	TMONEY GetCustomItemRefundMoney(TSN_ITEM item_sn,CUSTOM_SLOT_IDX slot);

	void UpdateItemGauge(void *pInfo, WORD pointManPlayTime,WORD rifleManPlayTime,WORD sniperPlayTime,void (*pfnItemDestroyed)(void *pInfo, _ITEM_UPDATE_TYPE_ flag,void* result,TID_EQUIP_SLOT slot),BOOL isClanChannel=FALSE);	

	BOOL RebuildEffect();
	int GetUsedEffect(ITEM_EFFECT_TYPE effectType,TID_ITEM* item_id = NULL);
	EFFECT_INFO* GetEquipSlotEffect(TID_EQUIP_SLOT slot);
	EFFECT_INFO* GetWeaponSlotEffect(TID_EQUIP_SLOT slot);
	EFFECT_INFO* GetEffectSlotEffect(TID_EQUIP_SLOT slot);	
	
	EFFECT_INFO* GetItemEffect(TSN_ITEM sn);
	EFFECT_INFO* GetEffectItemEffect(TSN_ITEM sn);

    int GetCustomItemTag(TID_ITEM item_id);

private:
	PLAYER_ITEM_INFO*	itemInfo;
	CItemDesc* itemDesc;
public:
	int effect[MAX_EFFECT_SIZE];
};