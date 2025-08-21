/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: Def.h

Description: Master definition header included by all AVA server and client projects.

***/

#pragma once

//#pragma pack(push)
//#pragma pack(1)


#if _MSC_VER >= 1400
// VisualStudio 2005

	// in stdlib.h
	/* _countof helper */
	#if !defined(_countof)
		#if !defined(__cplusplus)
			#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
		#else
			extern "C++"
			{
				template <typename _CountofType, size_t _SizeOfArray>
				char (*__countof_helper(UNALIGNED _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
				#define _countof(_Array) sizeof(*__countof_helper(_Array))
			}
		#endif
	#endif

	#define _ava_wcscpy(_t, _s) wcscpy_s(_t, _countof(_t), _s)
	#define _ava_sprintf(_t, _n, _s, ...) sprintf_s(_t, _n, _s, __VA_ARGS__)
	#define _ava_swprintf(_t, _n, _s, ...) swprintf_s(_t, _n, _s, __VA_ARGS__)

#else
// VisualStudio 2003

#pragma warning(disable:4002)	// Warning: too many actual parameters for macro 'ident'
#pragma warning(disable:4003)	// Warning: not enough actual parameters for macro 'ident'
template<typename T> inline const T& VARG(const T &t) { return t; }
inline const TCHAR* VARG() { return _T(""); }

#define _ava_wcscpy wcscpy
#define _ava_sprintf(_t, _n, _s, _a, _b, _c, _d, _e, _f, _g, _h) sprintf(_t, _s, VARG(_a), VARG(_b), VARG(_c), VARG(_d), VARG(_e), VARG(_f), VARG(_g), VARG(_h))
#define _ava_swprintf(_t, _n, _s, _a, _b, _c, _d, _e, _f, _g, _h) swprintf(_t, _s, VARG(_a), VARG(_b), VARG(_c), VARG(_d), VARG(_e), VARG(_f), VARG(_g), VARG(_h))

#endif

#include "DefType.h"

namespace Def
{
	//////////////////////////////////////////////////////////////////////////////////////
	// Host 관련 정보

	struct IPADDR_INFO
	{
		// TCP/IP network byte order (which is big endian) 으로 저장한다.
		union{
			unsigned long ipAddress; // IP 주소. _addr.s_addr와 동일
			unsigned char ip4b[4];
		};
		unsigned short port; // Port 번호

		BYTE dummy1[2];

		IPADDR_INFO(void): ipAddress(0), port(0) {}

		const IPADDR_INFO& operator=(const IPADDR_INFO& _hostInfo)
		{
			this->ipAddress = _hostInfo.ipAddress;
			this->port = _hostInfo.port;
			return *this;
		}
		void GetAddress(CHAR* szIP, size_t bufSize = 16)
		{
			if(szIP)
				_ava_sprintf(szIP,bufSize,"%d.%d.%d.%d",ip4b[0],ip4b[1],ip4b[2],ip4b[3]);
		}
		void GetAddress(WCHAR* szIP, size_t bufSize = 16)
		{
			if(szIP)
				_ava_swprintf(szIP,bufSize,L"%d.%d.%d.%d",(WORD)ip4b[0],(WORD)ip4b[1],(WORD)ip4b[2],(WORD)ip4b[3]);
		}

		bool IsValid() const
		{
			return ipAddress > 0 && port > 0;
		}
	};

	struct UDP_HOST_INFO
	{
		IPADDR_INFO intAddr;
		IPADDR_INFO extAddr;
	};


	//////////////////////////////////////////////////////////////////////////////////////
	// Item information
	union ITEM_ID
	{
		WORD id;
#pragma warning( disable : 4201)
		struct
		{
			BYTE idx;
			BYTE category; // ITEM_CATEGORY
		};
#pragma warning( default : 4201)
		ITEM_ID() : id(ID_INVALID_ITEM) {}
	};

	struct ITEM_BASE_INFO
	{
		TID_ITEM id;
		BYTE dummy[2];

		TSN_ITEM sn;
		ITEM_BASE_INFO(): id(ID_INVALID_ITEM),sn(SN_INVALID_ITEM){}
	};

	struct ITEM_INFO
	{
		TID_ITEM id;
		BYTE tag;
		BYTE dummy;

		long limit; // 내구성 or 유효기간. (날짜 or 횟수, 구분은 ITEM_DESC에 정의)

		TSN_ITEM sn;

		ITEM_INFO(): id(ID_INVALID_ITEM),tag(0),/*invenIdx(ID_INVALID_INVEN_SLOT),*/limit(0),sn(SN_INVALID_ITEM){}

		void Clear()
		{
			id = ID_INVALID_ITEM;
			tag = 0;
			limit = 0;
			//invenIdx = ID_INVALID_INVEN_SLOT;
			sn = SN_INVALID_ITEM;
		}
		BOOL IsEmpty()
		{
			if((sn == SN_INVALID_ITEM) || (id == ID_INVALID_ITEM))
				return TRUE;
			return FALSE;
		}
	};

	struct CUSTOM_ITEM_INFO
	{
		long limit; // 유효기간. (날짜 or 횟수, 구분은 ITEM_DESC에 정의)
		TID_ITEM id; // 아이탬 고유번호
		BYTE slot; // 사용중이냐?
		BYTE dummy;

		//TSN_ITEM dummy2;
		TSN_ITEM item_sn; // 장착한 아이탬의 SN

		CUSTOM_ITEM_INFO(): id(ID_INVALID_ITEM),slot(_CSI_NONE),item_sn(SN_INVALID_ITEM),/*bUsed(0),*/limit(0){}

		void Clear()
		{
			id = ID_INVALID_ITEM;
			slot = _CSI_NONE;
			item_sn = SN_INVALID_ITEM;
			//bUsed = 0;
			limit = 0;
		}
		BOOL IsEmpty()
		{
			return (item_sn == SN_INVALID_ITEM);
		}
	};

	struct EFFECT_ITEM_INFO
	{
		TID_ITEM	id;				// 아이탬 고유번호
		BYTE		dummy[2];

		long		limit;			// 유효기간. (날짜 or 횟수, 구분은 ITEM_DESC에 정의)

		TSN_ITEM	item_sn;		// 장착한 아이탬의 SN

		EFFECT_ITEM_INFO(): id(ID_INVALID_ITEM),item_sn(SN_INVALID_ITEM),limit(0){}

		void Clear()
		{
			id = ID_INVALID_ITEM;			
			item_sn = SN_INVALID_ITEM;			
			limit = 0;
		}
		BOOL IsEmpty()
		{
			if((item_sn == SN_INVALID_ITEM) || (id == ID_INVALID_ITEM))
				return TRUE;
			return FALSE;
		}
	};

	/*
	struct SLOT_INFO
	{
	//BYTE slotIndex;
	BYTE invenIndex;

	SLOT_INFO() : slotIndex(ID_INVALID_INVEN_SLOT),invenIndex(ID_INVALID_INVEN_SLOT) {}
	};

	union ITEM_SLOT
	{
	DWORD slot;
	struct
	{
	TID_ITEM item_id;
	BYTE pos;
	BYTE type;
	};

	ITEM_SLOT() : slot(ID_INVALID_ITEM_SLOT) {}
	};*/

	/*struct ITEM_SLOT
	{
	BYTE slot;

	ITEM_SLOT() : slot(ID_INVALID_INVEN_SLOT) {}
	};*/


	struct ROOM_ITEM_INFO
	{
		TID_ITEM id;
		WORD limitPerc;

		ROOM_ITEM_INFO(): id(ID_INVALID_ITEM), limitPerc(100) {}

		void Clear()
		{
			id = ID_INVALID_ITEM;
			limitPerc = 100;
		}
		BOOL IsEmpty()
		{
			return (id == ID_INVALID_ITEM);
		}
		const ROOM_ITEM_INFO& operator=(const ITEM_INFO &item)
		{
			id = item.id;
			limitPerc = (WORD)(100 * item.limit / ITEM_LIMIT_INITED);

			return *this;
		}
	};



	struct PLAYER_ITEM_INFO
	{
		ITEM_INFO weaponInven[MAX_INVENTORY_SIZE]; // 인벤토리
		ITEM_INFO equipInven[MAX_INVENTORY_SIZE]; // 인벤토리

		TSN_ITEM weaponSet[MAX_WEAPONSET_SIZE]; // 병과 별 셋팅한 무기.
		TSN_ITEM equipSet[MAX_EQUIPSET_SIZE]; // 장착 아이탬 ITEM_INFO의 index 를 저장

		// 커스텀 정보
		CUSTOM_ITEM_INFO customWeapon[MAX_CUSTOM_INVENTORY_SIZE]; // 커스텀 아이템 리스트

		EFFECT_ITEM_INFO effectInven[MAX_EFFECT_INVENTORY_SIZE];
		TSN_ITEM effectSet[MAX_EFFECTSET_SIZE]; // 장착 아이탬 ITEM_INFO의 index 를 저장

		PLAYER_ITEM_INFO()
		{
			memset(weaponSet,0x00,sizeof(TSN_ITEM) * MAX_WEAPONSET_SIZE);
			memset(equipSet,0x00,sizeof(TSN_ITEM) * MAX_EQUIPSET_SIZE);
			memset(effectSet,0x00,sizeof(TSN_ITEM) * MAX_EFFECTSET_SIZE);
		}
	};




	/*
	union SLOT_TYPE
	{
	WORD type;
	#pragma warning( disable : 4201)
	struct
	{
	BYTE flag : 4; // _ITEM_FLAG_
	BYTE category : 8; // 해당 아이탬이 들어가는 슬롯군.
	BYTE idx : 4; // idx = 0 이면 해당 군의 모든 슬롯에 들어갈수 있다.
	};
	#pragma warning( default : 4201)
	SLOT_TYPE() : type(0) {}
	SLOT_TYPE(BYTE flag,BYTE category,BYTE idx) : flag(flag),category(category),idx(idx) {}
	};
	*/


	union CUSTOM_SLOT_TYPE
	{
		BYTE type;
#pragma warning( disable : 4201)
		struct
		{
			BYTE front :1;
			BYTE mount : 1;
			BYTE barrel : 1;
			BYTE trigger : 1;
			BYTE grip : 1;
			BYTE stock: 1;
			BYTE dummy1 : 2;
		};
#pragma warning( default : 4201)
		CUSTOM_SLOT_TYPE() : type(0) {}
		BOOL IsCustomEnable()
		{
			return (type != 0);
		}
	};

	struct SLOT_DESC
	{
		int slotType; // _EQUIP_POSITION_

		TID_ITEM defaultItem; // 기본 장착 아이템 번호.
		TID_EQUIP_SLOT index;//
	};

	struct EFFECT_INFO
	{
		int effectType;
		int effectValue;

		EFFECT_INFO() : effectType(_IET_NONE),effectValue(0) {}
	};

	struct BASE_ITEM_DESC
	{
		TID_ITEM id;
		DWORD sales_id;

		TID_ITEM idParent;

		TID_ITEM code;

		BYTE useLimitLevel; // 계급별 사용제한
		BYTE gaugeType;		// 1 = 정비도 2 = 내구도 3 = 날짜제한; _ITEM_GAUGE_TYPE_

		// 내구도 아이템
		DWORD Durability_Game_Drop;
		DWORD Durability_Time_Drop;

		// 정비도 아이템
		TMONEY maintenancePrice; // 최대 수리비

		// 기간제 아이템
		BYTE dateLimit;		// 사용 일수

		BYTE priceType;		// 이 아이탬이 케쉬 아이탬인지; _ITEM_PRICE_TYPE_
		TMONEY price;		// 가격

		// 상속
		BASE_ITEM_DESC *parentItem;

		BASE_ITEM_DESC(TID_ITEM _id = ID_INVALID_ITEM, const BASE_ITEM_DESC *_parent = NULL, DWORD _sales_id = 0) :
			id(_id),
			sales_id(_sales_id),
			idParent(ID_INVALID_ITEM),
			code(ID_INVALID_ITEM),
			useLimitLevel(0),
			gaugeType(0),
			priceType(0),
			price(0),
			Durability_Game_Drop(0),
			Durability_Time_Drop(0),
			maintenancePrice(0),
			dateLimit(0)
		{
			if (_sales_id == 0)
				sales_id = id;
			if (_parent)
				InheritFrom(*_parent);
			else
				parentItem = NULL;
		}
		virtual ~BASE_ITEM_DESC() {}

		virtual BOOL InheritFrom(const BASE_ITEM_DESC &parent)
		{
			idParent = parent.id;
			code = parent.code;
			useLimitLevel = parent.useLimitLevel;
			gaugeType = parent.gaugeType;
			priceType = parent.priceType;
			price = parent.price;
			Durability_Game_Drop = parent.Durability_Game_Drop;
			Durability_Time_Drop = parent.Durability_Time_Drop;
			maintenancePrice = parent.maintenancePrice;
			dateLimit = parent.dateLimit;
			parentItem = const_cast<BASE_ITEM_DESC*>(&parent);

			return TRUE;
		}

		int GetItemFlag() const
		{
			if (id == ID_INVALID_ITEM)
				return _IF_NONE;
			BYTE ic = HIBYTE(id);
			switch (ic)
			{
			case _IC_FACE:
				return _IF_NONE;
			case _IC_PISTOL:
			case _IC_SMG:
			case _IC_AR:
			case _IC_SR:
			case _IC_KNIFE:
			case _IC_GRENADE:
			case _IC_HEAVY:
				return _IF_WEAPON;
			case _IC_HELMET:
			case _IC_ARMOR:
			case _IC_HEAD:
			case _IC_TORSO:
			case _IC_LEG:
				return _IF_EQUIP;
			case _IC_EFFECT:
				return _IF_EFFECT;
			default:
				if (ic >= _IC_CUSTOM && ic <_IC_COLLECTING)
					return _IF_CUSTOM;
				break;
			}
			return _IF_NONE;
		}

		virtual BOOL IsValid() const
		{
			return id != ID_INVALID_ITEM && code != ID_INVALID_ITEM;
		}

		const WCHAR* GetName() const
		{
			static const WCHAR *_defName = L"";
			const WCHAR *_name = _GetName();
			if (!_name || wcslen(_name) == 0)
			{
				if (parentItem && parentItem->IsValid())
					return parentItem->GetName();
				else
					return _defName;
			}
			return _name;
		}
		const WCHAR* GetDescription() const
		{
			static const WCHAR *_defDesc = L"";
			const WCHAR *_desc = _GetDescription();
			if (!_desc || wcslen(_desc) == 0)
			{
				if (parentItem && parentItem->IsValid())
					return parentItem->GetDescription();
				else
					return _defDesc;
			}
			return _desc;
		}
		WCHAR GetIcon() const
		{
			return _GetIcon();
		}
		short GetGraphValue(int idx) const
		{
			return _GetGraphValue(idx);
		}

	private:
		virtual const WCHAR* _GetName() const { return NULL; }
		virtual const WCHAR* _GetDescription() const { return NULL; }
		virtual WCHAR _GetIcon() const { return 0; }
		virtual short _GetGraphValue(int idx) const { return 0; }
	};

	struct ITEM_DESC : public BASE_ITEM_DESC
	{
		BYTE destroyable;
		BYTE statLog;		// 0 이면 statistics log에 사용 기록을 남기지 않음
		BYTE isDefaultItem;

		TMONEY bonusMoney;	// 살때 보너스로 받는 게임머니

		int slotType;		// _EQUIP_POSITION_

		// 개조
		BYTE bRisConvertible;
		TID_ITEM RisConvertibleID;
		TMONEY RisConvertiblePrice;

		// 커스텀
		CUSTOM_SLOT_TYPE customType; // 장착가능한 커스텀 슬롯
		TID_ITEM defaultCustomItem/*[_CSI_MAX]*/; // 기본으로 장착돼는 커스텀 아이탬 무조건 Mount

		// Effect..
		EFFECT_INFO effectInfo;

		ITEM_DESC(TID_ITEM _id = ID_INVALID_ITEM, const ITEM_DESC *_parent = NULL, DWORD _sales_id = 0) :
			BASE_ITEM_DESC(_id, _parent, _sales_id),
			destroyable(0),
			statLog(0),
			isDefaultItem(0),
			bonusMoney(0),
			slotType(_EP_NONE),
			bRisConvertible(0),
			RisConvertibleID(ID_INVALID_ITEM),
			RisConvertiblePrice(0),
			defaultCustomItem(ID_INVALID_ITEM)
			{
			}
		virtual ~ITEM_DESC() {}

		virtual BOOL InheritFrom(const BASE_ITEM_DESC &parent)
		{
			if (!IsWeaponItem(parent.id) && !IsEquipItem(parent.id))
				return FALSE;

			if ( !BASE_ITEM_DESC::InheritFrom(parent) )
				return FALSE;

			const ITEM_DESC &_desc = static_cast<const ITEM_DESC&>(parent);
			destroyable = _desc.destroyable;
			statLog = _desc.statLog;
			isDefaultItem = _desc.isDefaultItem;
			bonusMoney = _desc.bonusMoney;
			slotType = _desc.slotType;
			bRisConvertible = _desc.bRisConvertible;
			RisConvertibleID = _desc.RisConvertibleID;
			RisConvertiblePrice = _desc.RisConvertiblePrice;
			defaultCustomItem = _desc.defaultCustomItem;

			return TRUE;
		}
	};

	struct CUSTOM_ITEM_DESC : public BASE_ITEM_DESC
	{
		TID_ITEM item_id;	// 장착할수 있는 아이탬..
		CUSTOM_SLOT_IDX customType; // CUSTOM_SLOT_IDX 장착할수 있는 위치.

		BYTE isDefaultItem;

		CUSTOM_ITEM_DESC(TID_ITEM _id = ID_INVALID_ITEM, const CUSTOM_ITEM_DESC *_parent = NULL, DWORD _sales_id = 0) :
			BASE_ITEM_DESC(_id, _parent, _sales_id)
		{
		}
		virtual ~CUSTOM_ITEM_DESC() {}

		virtual BOOL InheritFrom(const BASE_ITEM_DESC &parent)
		{
			if (!IsCustomItem(parent.id))
				return FALSE;

			if ( !BASE_ITEM_DESC::InheritFrom(parent) )
				return FALSE;

			const CUSTOM_ITEM_DESC &_desc = static_cast<const CUSTOM_ITEM_DESC&>(parent);
			item_id = _desc.item_id;
			isDefaultItem = _desc.isDefaultItem;
			customType = _desc.customType;

			return TRUE;
		}

		virtual BOOL IsValid() const
		{
			return BASE_ITEM_DESC::IsValid() && item_id != ID_INVALID_ITEM && customType != _CSI_NONE;
		}
	};

	struct EFFECT_ITEM_DESC : public BASE_ITEM_DESC
	{
		TMONEY bonusMoney;	// 살때 보너스로 받는 게임머니

		int slotType;		// _EQUIP_POSITION_

		// Effect..
		EFFECT_INFO effectInfo;

		EFFECT_ITEM_DESC(TID_ITEM _id = ID_INVALID_ITEM, const EFFECT_ITEM_DESC *_parent = NULL, DWORD _sales_id = 0) :
			BASE_ITEM_DESC(_id, _parent, _sales_id),
			bonusMoney(0)
		{
		}
		virtual ~EFFECT_ITEM_DESC() {}

		virtual BOOL InheritFrom(const BASE_ITEM_DESC &parent)
		{
			if (!IsEffectItem(parent.id))
				return FALSE;

			if ( !BASE_ITEM_DESC::InheritFrom(parent) )
				return FALSE;

			const EFFECT_ITEM_DESC &_desc = static_cast<const EFFECT_ITEM_DESC&>(parent);
			bonusMoney = _desc.bonusMoney;
			slotType = _desc.slotType;

			return TRUE;
		}
	};

	struct PACKAGE_ITEM_DESC : public BASE_ITEM_DESC
	{
		enum _Const
		{
			MAX_ITEMLIST_SIZE = 6
		};

		TMONEY bonusMoney;	// 살때 보너스로 받는 게임머니

		TID_ITEM itemList[MAX_ITEMLIST_SIZE];
		BYTE itemCount;

		PACKAGE_ITEM_DESC(TID_ITEM _id = ID_INVALID_ITEM, const ITEM_DESC *_parent = NULL, DWORD _sales_id = 0) :
			BASE_ITEM_DESC(_id, _parent, _sales_id),
			bonusMoney(0)
		{
			memset(itemList, 0, sizeof(itemList));
		}
		virtual ~PACKAGE_ITEM_DESC() {}

		virtual BOOL InheritFrom(const BASE_ITEM_DESC &parent)
		{
			//if (!IsCustomItem(parent.id))
			//	return FALSE;

			if ( !BASE_ITEM_DESC::InheritFrom(parent) )
				return FALSE;

			const PACKAGE_ITEM_DESC &_desc = static_cast<const PACKAGE_ITEM_DESC&>(parent);
			bonusMoney = _desc.bonusMoney;
			memcpy(itemList, _desc.itemList, sizeof(itemList));
			itemCount = _desc.itemCount;

			return TRUE;
		}

		virtual BOOL IsValid() const
		{
			return BASE_ITEM_DESC::IsValid() && itemCount > 0;
		}
	};


	struct PLAYER_SKILL_INFO
	{
		//__int64 skill; // 8Byte Bit Field..
		WORD skill[_CLASS_MAX];
		PLAYER_SKILL_INFO()
		{
			for (int i = 0; i < _CLASS_MAX; ++i)
				skill[i] = 0;
		}
	};

	struct PLAYER_AWARD_INFO
	{
		WORD info[MAX_AWARD_PER_PLAYER];

		PLAYER_AWARD_INFO()
		{
			memset(&info,0x00,sizeof(PLAYER_AWARD_INFO));
		}
	};


	//////////////////////////////////////////////////////////////////////////////////////
	// Player information
	struct PLAYER_RESULT_INFO
	{
		// 스킬 관련

		struct SCORE_INFO
		{
			WORD attacker;
			WORD defender;

			WORD leader;
			WORD tactic;

			WORD Sum() { return attacker + defender + leader + tactic; }
		};

		struct CLASS_RESULT_INFO
		{
			//TODO PlayRound 와 TakenDamage가 이상하다.. 체크하자...
			WORD playTime; // 플레이한 시간 (초)
			WORD sprintTime; // 스프린트한 시간 (초)
			WORD takenDamage; // 받은 데미지
			short killCount; // 킬 횟수

			BYTE playRound; // 플레이한 라운드/스폰 수
			BYTE headshotCount; // 주무기로 준 헤드샷 횟수
			BYTE headshotKillCount; // 주무기 헤드샷으로 달성한 kill 횟수
			BYTE dummy;

			BYTE weaponKillCount[4];

			WORD weaponDamage[4];
		};

		TID_ACCOUNT idAccount;

		BYTE gameWin; // 이긴 게임 수 (1 or 0)
		BYTE roundWin; // 이긴 라운드 수
		BYTE roundDefeat; // 진 라운드 수
		BYTE disconnectCount; // 디스커넥트한 횟수 (1 or 0)

		BYTE deathCount; // 죽은 횟수
		BYTE helmetDropCount;

		BYTE bulletMultiKillCount; // 탄환으로 멀티킬 달성한 횟수
		BYTE grenadeMultiKillCount; // 수류탄으로 멀티킬 달성한 횟수

		BYTE roundTopKillCount; // 한 라운드에서 킬 수 1위 달성
		BYTE roundTopHeadshotKillCount; // 한 라운드에서 헤드샷 킬 수 1위 달성
		BYTE topLevelKillCount; // 상대팀에서 계급이 제일 높은 유저 킬 수
		BYTE higherLevelKillCount; // 나보다 계급이 높은 유저 킬 수

		BYTE noDamageWinCount; // 데미지를 전혀 입지 않고 라운드 승리한 횟수

		BYTE topScoreCount; // 팀 내 점수에서 1등한 횟수 (1 or 0)
		BYTE noDeathRoundCount; // 한번도 킬 당하지 않은 라운드 수

		BYTE teamKillCount; // 팀킬 횟수

		DWORD weaponFireCount[4]; // 0 = pistol, 1 = smg, 2 = rifle, 3 = sniper
		DWORD weaponHitCount[4];
		DWORD weaponHeadshotCount[4];

		SCORE_INFO score; // 스코어
		CLASS_RESULT_INFO classResultInfo[_CLASS_MAX];

		BYTE currentClass;
		BYTE bLeader;	
		BYTE dummy[2];
				
		PLAYER_RESULT_INFO()
		{
			memset(this,0x00,sizeof(PLAYER_RESULT_INFO));
		}

		LONG GetKillCount() const
		{
			LONG _count = 0;
			for (int _i = 0; _i < _CLASS_MAX; ++_i)
				_count += classResultInfo[_i].killCount;
			return _count;
		}
	};

	struct PLAYER_SCORE_INFO
	{
		// 스킬 관련

		struct SCORE_INFO
		{
			DWORD attacker;
			DWORD defender;

			DWORD leader;
			DWORD tactic;

			DWORD Sum() { return attacker + defender + leader + tactic; }
		};

		struct CLASS_SCORE_INFO
		{
			DWORD playRound; // 플레이한 라운드/스폰 수
			DWORD headshotCount; // 주무기로 준 헤드샷 횟수

			DWORD headshotKillCount; // 주무기 헤드샷으로 달성한 kill 횟수
			DWORD playTime; // 플레이한 시간 (초)

			DWORD sprintTime; // 스프린트한 시간 (초)
			DWORD takenDamage; // 받은 데미지

			LONG killCount; // 킬 횟수

			LONG weaponKillCount[4];
			DWORD weaponDamage[4];
		};

		DWORD gameWin; // 이긴 게임 수
		DWORD gameDefeat; // 진 게임 수

		DWORD roundWin; // 이긴 라운드 수
		DWORD roundDefeat; // 진 라운드 수

		DWORD disconnectCount; // 디스커넥트한 횟수
		DWORD deathCount; // 죽은 횟수

		SCORE_INFO score; // 스코어

		CLASS_SCORE_INFO classScoreInfo[_CLASS_MAX];

		// 훈장 관련

		WORD straightWinCount; // 연승 횟수
		WORD guildStraightWinCount; // 클랜전 연승 횟수

		DWORD guildWinCount; // 클랜전 승리 횟수

		// 기타
		DWORD teamKillCount; // 팀킬 횟수

		PLAYER_SCORE_INFO()
		{
			memset(this,0x00,sizeof(PLAYER_SCORE_INFO));
		}

		LONG GetKillCount() const
		{
			LONG _count = 0;
			for (int _i = 0; _i < _CLASS_MAX; ++_i)
				_count += classScoreInfo[_i].killCount;
			return _count;
		}

		float GetSDRatio()
		{
			return ((float)(score.Sum() + GetKillCount())) / (deathCount > 0 ? (float)deathCount : 1.0f);
		}

		void AddResult(PLAYER_RESULT_INFO &result)
		{
			if (result.disconnectCount > 0)
			{
				disconnectCount += result.disconnectCount;
			}
			else
			{
				if (result.gameWin == 0)
					++gameDefeat;
				else if (result.gameWin == 1)
					++gameWin;
				//else if (result.gameWin == 255)
			}

			roundWin += result.roundWin;
			roundDefeat += result.roundDefeat;
			deathCount += result.deathCount;
			score.attacker += result.score.attacker;
			score.defender += result.score.defender;
			score.leader += result.score.leader;
			score.tactic += result.score.tactic;
			for (int i = 0; i < _CLASS_MAX; ++i)
			{
				classScoreInfo[i].playRound += result.classResultInfo[i].playRound;
				classScoreInfo[i].headshotCount += result.classResultInfo[i].headshotCount;
				classScoreInfo[i].headshotKillCount += result.classResultInfo[i].headshotKillCount;
				classScoreInfo[i].playTime += result.classResultInfo[i].playTime;
				classScoreInfo[i].sprintTime += result.classResultInfo[i].sprintTime;
				classScoreInfo[i].takenDamage += result.classResultInfo[i].takenDamage;
				classScoreInfo[i].killCount += result.classResultInfo[i].killCount;
				for (int j = 0; j < 4; ++j)
				{
					classScoreInfo[i].weaponKillCount[j] += result.classResultInfo[i].weaponKillCount[j];
					classScoreInfo[i].weaponDamage[j] += result.classResultInfo[i].weaponDamage[j];
				}
			}
		}
	};

	struct ROOM_RESULT_INFO
	{
		WORD score;
		WORD death;
		DWORD xp;
		DWORD supplyPoint;
		BYTE level;
		BYTE bLeader;
		BYTE currentClass;
		BYTE biaXPFlag;		// 0 = 전우 경험치를 받지 않았음, 1 = 전우 경험치를 받았음, 2 = 전우 경험치를 채워서 실 경험치를 받았음

		TID_ITEM equipItem[MAX_EQUIPSET_SIZE];
		ROOM_ITEM_INFO weaponItem[MAX_WEAPONSET_SIZE];
		TID_ITEM customItem[_CLASS_MAX][_CSI_MAX];

		PLAYER_SKILL_INFO skillInfo;

		TID_ITEM effectList[MAX_EFFECTSET_SIZE];

		ROOM_RESULT_INFO() : score(0), death(0), xp(0), supplyPoint(0), level(0), bLeader(0), currentClass((BYTE)_CLASS_NONE), biaXPFlag(0)
		{
			memset(equipItem,0x00,sizeof(equipItem));
			memset(weaponItem,0x00,sizeof(weaponItem));
			memset(customItem,0x00,sizeof(customItem));

			memset(effectList,0x00,sizeof(effectList));
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	//// Channel

	struct CHANNEL_DESC
	{
		TID_CHANNEL idChannel;
		BYTE flag;
		BYTE idx;
		WORD maxPlayers;
		BYTE mask;
		BYTE dummy;

		CHANNEL_DESC() : idChannel(Def::ID_INVALID_CHANNEL), flag(0), idx(0), maxPlayers(0), mask(UCHAR_MAX)
		{
		}
	};

	struct CHANNEL_INFO
	{
		TID_CHANNEL idChannel;	// 채널 고유번호.
		WORD cnt_player;		// 접속자 수
		CHANNEL_INFO() : idChannel(ID_INVALID_CHANNEL), cnt_player(0) {}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Room information
	// 방 설정
	struct ROOM_SETTING
	{
		TID_MAP idMap;
		
		BYTE tkLevel;
		BYTE limitLevel;
		BYTE roundToWin;
		
		BYTE numMax;
		BYTE autoBalance;
		BYTE allowSpectator;
		BYTE allowInterrupt;

		BYTE allowBackView;
		BYTE allowGhostChat;
		BYTE autoSwapTeam;
		BYTE mapOption;

		ROOM_SETTING(): idMap(ID_INVALID_MAP),tkLevel(0),limitLevel(0),roundToWin(0),numMax(0),autoBalance(0),
			allowSpectator(0),allowInterrupt(0),allowBackView(0),allowGhostChat(0),autoSwapTeam(0), mapOption(0) {}
	};

	// 방 상태
	struct ROOM_STATE
	{
		BYTE playing;
		BYTE numCurr;
		BYTE currRound;
		BYTE bMatch;

		ROOM_STATE() : playing(RIP_NONE),numCurr(0),currRound(0), bMatch(0) {}
	};

	// 방 정보
	struct ROOM_INFO
	{
		TID_ROOM idRoom;
		BYTE bPassword;
		BYTE dummy;

		WCHAR roomName[SIZE_ROOM_NAME+1];
		WCHAR hostName[SIZE_NICKNAME+1];

		ROOM_SETTING setting;

		ROOM_STATE state;

		TID_GUILD idGuild[2];

		ROOM_INFO()
		{
			Clear();
		}

		void Clear()
		{
			memset(this, 0x00, sizeof(ROOM_INFO));
			//idRoom = ID_INVALID_ROOM;
			//setting.numMax = 0;
			//state.numCurr = 0;
			//state.playing = 0;
			//state.currRound = 0;

			//bPassword = FALSE;

			//memset(roomName,0x00,sizeof(WCHAR)*(SIZE_ROOM_NAME+1));
			//memset(hostName,0x00,sizeof(WCHAR)*(SIZE_NICKNAME+1));
		}
	};

	struct ROOM_CLAN_INFO
	{
		TID_GUILD idGuild;
		WCHAR guildName[SIZE_GUILD_NAME+1];

		ROOM_CLAN_INFO() : idGuild(ID_INVALID_GUILD)
		{
			memset(guildName,0x00,sizeof(WCHAR)*(SIZE_GUILD_NAME+1));
		}		
	};


	//////////////////////////////////////////////////////////////////////////////////////
	// Guild information

	struct GUILD_GROUP_INFO
	{
		WCHAR groupName[SIZE_GUILD_GROUP_NAME + 1];
		BYTE idGroup;
		BYTE cntMember;

		DWORD xp;

		TID_ACCOUNT members[SIZE_GUILD_MEMBERS_PER_GROUP];
	};

	//struct GUILD_RANK_INFO
	//{
	// //WCHAR rankName[SIZE_GUILD_RANK_NAME + 1];

	// struct _AUTHORITY
	// {
	// int invite : 1;
	// int promote : 1;
	// int demote : 1;

	// int setAuthority : 1;
	// int editInfo : 1;

	// int setGroupName : 1;
	// int editGroupMember : 1;
	// int editOwnGroupMember : 1;

	// int depositInventory : 1;
	// int withdrawInventory : 1;

	// int buyShop : 1;

	// int giveGift : 1;
	// int distributeIncome : 1;
	// } authority;
	// //BYTE dummy1[3];
	//};

	struct GUILD_INFO
	{
		static _GUILD_PRIV privInfo[SIZE_GUILD_RANK];
		static void InitPrivInfo();

		TID_GUILD idGuild;
		WCHAR strGuild[SIZE_GUILD_ID+1]; //스크링으로 저장하는 길드 인덱스

		WCHAR name[SIZE_GUILD_NAME + 1];

		TID_GUILDMARK idGuildMark;

		BYTE level;
		BYTE cntMember;

		BYTE maxGroups;
		BYTE dummy1;

		DWORD xp;

		TID_ACCOUNT idMaster;

		TPOINT income; // 길드에 대한 월급

		WCHAR motd[SIZE_GUILD_MOTD + 1]; // 접속할때 채팅창에 나오는 메세지

		WCHAR regdate[SIZE_GUILD_DATE + 1];

		GUILD_GROUP_INFO groups[10];// 소그룹 정보

		DWORD		totalWinCnt;
		DWORD		totalLoseCnt;

		TMONEY		clanMoney;

		GUILD_INFO() : idGuild(ID_INVALID_GUILD),idGuildMark(ID_INVALID_GUILDMARK),level(0),cntMember(0),maxGroups(0),xp(0),idMaster(ID_INVALID_ACCOUNT),income(0),totalWinCnt(0),totalLoseCnt(0),clanMoney(0)
		{
			memset(strGuild,0x00,sizeof(WCHAR)*(SIZE_GUILD_ID + 1));
			memset(name,0x00,sizeof(WCHAR)*(SIZE_GUILD_NAME + 1));
			memset(motd,0x00,sizeof(WCHAR)*(SIZE_GUILD_MOTD + 1));
			memset(regdate,0x00,sizeof(WCHAR)*(SIZE_GUILD_DATE + 1));
		}
	};

	// 플레이어가 가지고 있는 길드정보
	struct PLAYER_GUILD_INFO
	{
		TID_GUILD idGuild;

		WCHAR strGuildID[SIZE_GUILD_ID + 1];
		WCHAR guildName[SIZE_GUILD_NAME + 1];

		TID_GUILDMARK idGuildMark;

		BYTE guildLevel;
		BYTE idGroup; /// 소모임 번호..
		BYTE idRank; // 등급.
		BYTE dummy;

		PLAYER_GUILD_INFO(): idGuild(ID_INVALID_GUILD),idGuildMark(ID_INVALID_GUILDMARK),guildLevel(0),idGroup(ID_INVALID_GUILD_GROUP),idRank(0)
		{
			memset(strGuildID,0x00,sizeof(WCHAR)*(SIZE_GUILD_ID + 1));
			memset(guildName,0x00,sizeof(WCHAR)*(SIZE_GUILD_NAME + 1));
		}

		BOOL HasPriv(_GUILD_PRIV priv) const
		{
			return (idRank >= 0 && idRank < SIZE_GUILD_RANK) ? GUILD_INFO::privInfo[idRank] & priv : FALSE;
		}
	};


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Player의 기본 정보
	struct PLAYER_INFO
	{
		TID_ACCOUNT idAccount;

		WCHAR nickname[SIZE_NICKNAME+1]; // 캐릭터 이름 - 한글 6자 / 영문 12자 이내// (최소 한글 2자 / 영문 4자 이상),(){}[]<>|의 특수문자 허용

		BYTE faceType; // 얼굴
		BYTE level; // 계급

		BYTE currentClass; // 마지막으로 플레이했던 클레스..
		BYTE lastTeam; // 마지막으로 플레이했던 팀..
		WORD straightWin; // 현재 연승 회수

		BYTE dummy;

		BYTE channelMaskList[MAX_CHANNEL_MASK];

		BYTE xpProgress;
		DWORD xp; // 경험치
		TPOINT supplyPoint; // 보급

		//////////////////////////////////////////
		// 길드정보..
		PLAYER_GUILD_INFO guildInfo;

		//////////////////////////////////////////
		// ITEM 정보..
		PLAYER_ITEM_INFO itemInfo;

		//////////////////////////////////////////
		// Skill 정보.
		PLAYER_SKILL_INFO skillInfo;

		//////////////////////////////////////////
		// Result 정보.
		PLAYER_SCORE_INFO scoreInfo;

		//////////////////////////////////////////
		// Medal 정보.
		PLAYER_AWARD_INFO awardInfo;


		//////////////////////////////////////////
		// 기타 캐쉬 및 경험치..
		//TCASH cash; // 보유하고 있는 캐쉬
		TMONEY money; // 게임머니

		UDP_HOST_INFO rttTestAddr;

		WCHAR configstr[SIZE_CONFIGSTR1+1]; // DB에 저장하는 개인 설정 문자열
		WCHAR configstr2[SIZE_CONFIGSTR2+1]; // DB에 저장하는 개인 설정 문자열

		DWORD	biaXP;	// 전우 경험치...

		PLAYER_INFO()
		{
			idAccount = ID_INVALID_ACCOUNT;
			memset(nickname,0x00,sizeof(nickname));
			faceType = 0;
			level = 0;
			xp = 0;
			xpProgress = 0;
			money = 0;
			currentClass = _CLASS_POINTMAN;
			//memset(skill,0x00,sizeof(skill));
			//memset(medal,0x00,sizeof(medal));
			//cash = 0;
			supplyPoint = 0;
			straightWin = 0;
			lastTeam = 0;
			memset(configstr,0x00,sizeof(configstr));
			memset(configstr2,0x00,sizeof(configstr2));

			memset(channelMaskList,0x00,sizeof(channelMaskList));
		}
	};

	struct LOBBY_PLAYER_INFO
	{
		TID_ACCOUNT idAccount;
		WCHAR nickname[SIZE_NICKNAME+1];
		BYTE level;

		BYTE dummy;

		LOBBY_PLAYER_INFO(): idAccount(ID_INVALID_ACCOUNT),level(0)
		{
			memset(nickname,0x00,sizeof(nickname));
		}

		const LOBBY_PLAYER_INFO& operator=(const PLAYER_INFO &ci)
		{
			idAccount = ci.idAccount;
			_ava_wcscpy(nickname, ci.nickname);
			level = ci.level;

			return *this;
		}
	};

	// 로비에서 사용하게 될 Player의 정보
	struct PLAYER_DISP_INFO
	{
		TID_ACCOUNT idAccount;

		WCHAR nickname[SIZE_NICKNAME+1]; // 캐릭터 이름 - 한글 6자 / 영문 12자 이내// (최소 한글 2자 / 영문 4자 이상),(){}[]<>|의 특수문자 허용
		WCHAR guildName[SIZE_GUILD_NAME+1];

		TID_GUILDMARK idGuildMark;

		//BYTE faceType; // 얼굴
		BYTE level;// 계급
		//BYTE currentClass; // 마지막으로 플레이했던 클레스..
		BYTE dummy;

		TID_ROOM idRoom;	// 위치 정보

		DWORD gameWin; // 이긴 게임 수 (1 or 0)
		DWORD gameDefeat; // 진 게임 수 (1 or 0)
		DWORD disconnectCount; // 디스커넥트한 횟수 (1 or 0)
		LONG killCount; // Kill 횟수
		DWORD deathCount; // 죽은 횟수
		PLAYER_SCORE_INFO::SCORE_INFO scoreInfo;

		//TID_ITEM equipItem[MAX_EQUIPSET_SIZE];
		//TID_ITEM weaponItem[MAX_WEAPONSET_SIZE];

		PLAYER_DISP_INFO()// : idAccount(ID_INVALID_ACCOUNT),faceType(0),level(0),currentClass(_CLASS_RIFLEMAN),gameWin(0),gameDefeat(0),disconnectCount(0),killCount(0),deathCount(0)
		{
			memset(this,0x00,sizeof(PLAYER_DISP_INFO));
			//currentClass = _CLASS_RIFLEMAN;
		}

		const PLAYER_DISP_INFO& operator=(const PLAYER_INFO &ci)
		{
			idAccount = ci.idAccount;
			_ava_wcscpy(nickname, ci.nickname);
			//faceType = ci.faceType;
			level = ci.level;
			//currentClass = ci.currentClass;

			_ava_wcscpy(guildName, ci.guildInfo.guildName);

			idGuildMark = ci.guildInfo.idGuildMark;

			gameWin = ci.scoreInfo.gameWin;
			gameDefeat = ci.scoreInfo.gameDefeat;
			disconnectCount = ci.scoreInfo.disconnectCount;
			killCount = ci.scoreInfo.GetKillCount();
			deathCount = ci.scoreInfo.deathCount;
			scoreInfo = ci.scoreInfo.score;

			//for(int _i=0;_i<MAX_EQUIPSET_SIZE;++_i)
			//{
			// if(ci.itemInfo.equipSet[_i] != SN_INVALID_ITEM)
			// {
			// for(int _j=0;_j<MAX_INVENTORY_SIZE;++_j)
			// {
			// if(ci.itemInfo.equipInven[_j].sn == ci.itemInfo.equipSet[_i])
			// {
			// equipItem[_i] = ci.itemInfo.equipInven[_j].id;
			// break;
			// }
			// }
			// }
			// else
			// equipItem[_i] = ID_INVALID_ITEM;
			//}
			//for(int _i=0;_i<MAX_WEAPONSET_SIZE;++_i)
			//{
			// if(ci.itemInfo.weaponSet[_i] != SN_INVALID_ITEM)
			// {
			// for(int _j=0;_j<MAX_INVENTORY_SIZE;++_j)
			// {
			// if(ci.itemInfo.weaponInven[_j].sn == ci.itemInfo.weaponSet[_i])
			// {
			// weaponItem[_i] = ci.itemInfo.weaponInven[_j].id;
			// break;
			// }
			// }
			// }
			// else
			// weaponItem[_i] = ID_INVALID_ITEM;
			//}
			return *this;
		}

		float GetSDRatio()
		{
			return ((float)(scoreInfo.Sum() + killCount)) / (deathCount > 0 ? (float)deathCount : 1.0f);
		}
	};

	struct ROOM_PLAYER_INFO
	{
		TID_ACCOUNT idAccount;

		WCHAR nickname[SIZE_NICKNAME+1];
		WCHAR guildName[SIZE_GUILD_NAME+1];

		TID_GUILDMARK	idGuildMark;

		BYTE faceType; // 얼굴
		BYTE level;
		BYTE idSlot;
		BYTE bReady;

		BYTE currentClass;
		BYTE pcBang;
		WORD hostRating;

		DWORD leaderScore;
		float rttScore;

		PLAYER_SKILL_INFO skillInfo;

		TID_ITEM equipItem[MAX_EQUIPSET_SIZE];
		ROOM_ITEM_INFO weaponItem[MAX_WEAPONSET_SIZE];
		TID_ITEM customItem[_CLASS_MAX][_CSI_MAX];

		ROOM_PLAYER_INFO(): idAccount(ID_INVALID_ACCOUNT),level(0),idSlot(ID_INVALID_ROOM_SLOT),idGuildMark(ID_INVALID_GUILDMARK),
			bReady(_READY_NONE),faceType(0),currentClass(0),hostRating(0),pcBang(0),rttScore(0.0),leaderScore(0)
		{
			memset(nickname,0x00,sizeof(nickname));
			memset(guildName,0x00,sizeof(guildName));
			memset(equipItem,0x00,sizeof(equipItem));
			memset(weaponItem,0x00,sizeof(weaponItem));
			memset(customItem,0x00,sizeof(customItem));
		}

		BYTE GetTeamID() const
		{
			return (BYTE)(idSlot / MAX_PLAYER_PER_TEAM);
		}

		const ROOM_PLAYER_INFO& operator=(const PLAYER_INFO &ci)
		{
			idAccount = ci.idAccount;
			_ava_wcscpy(nickname, ci.nickname);
			_ava_wcscpy(guildName, ci.guildInfo.guildName);
			idGuildMark = ci.guildInfo.idGuildMark;
			level = ci.level;
			faceType = ci.faceType;
			currentClass = ci.currentClass;
			leaderScore = ci.scoreInfo.score.leader;

			skillInfo = ci.skillInfo;

			for(int _i=0;_i<MAX_EQUIPSET_SIZE;++_i)
			{
				equipItem[_i] = ID_INVALID_ITEM;

				if(ci.itemInfo.equipSet[_i] != SN_INVALID_ITEM)
				{
					for(int _j=0;_j<MAX_INVENTORY_SIZE;++_j)
					{
						if(ci.itemInfo.equipInven[_j].sn == ci.itemInfo.equipSet[_i])
						{
							equipItem[_i] = ci.itemInfo.equipInven[_j].id;
							break;
						}
					}
				}				
			}

			for(int _x=0;_x<_CSI_MAX; ++_x)
			{
				customItem[_CLASS_POINTMAN][_x] = ID_INVALID_ITEM;
				customItem[_CLASS_RIFLEMAN][_x] = ID_INVALID_ITEM;
				customItem[_CLASS_SNIPER][_x] = ID_INVALID_ITEM;
			}

			for(int _i=0;_i<MAX_WEAPONSET_SIZE;++_i)
			{
				weaponItem[_i].Clear();				

				if(ci.itemInfo.weaponSet[_i] != SN_INVALID_ITEM)
				{
					for(int _j=0;_j<MAX_INVENTORY_SIZE;++_j)
					{
						if(ci.itemInfo.weaponInven[_j].sn == ci.itemInfo.weaponSet[_i])
						{
							weaponItem[_i] = ci.itemInfo.weaponInven[_j];

							//custom
							if(_i == _CLASS_POINTMAN || _i == _CLASS_RIFLEMAN || _i == _CLASS_SNIPER)
							{
								for(int _k=0;_k<MAX_CUSTOM_INVENTORY_SIZE;++_k)
								{
									if(ci.itemInfo.weaponInven[_j].sn == ci.itemInfo.customWeapon[_k].item_sn)
									{
										customItem[_i][ci.itemInfo.customWeapon[_k].slot] = ci.itemInfo.customWeapon[_k].id;
									}
								}
							}
							break;
						}
					}
				}				
			}

			return *this;
		}
	};

	struct GUILD_PLAYER_INFO
	{
		TID_ACCOUNT idAccount;

		WCHAR nickname[SIZE_NICKNAME+1];

		BYTE level;
		BYTE idGroup;	// 소모임 번호.
		BYTE idRank;	// 등급.
		BYTE dummy;
		TID_CHANNEL idChannel;

		GUILD_PLAYER_INFO(): idAccount(ID_INVALID_ACCOUNT), level(0), idGroup(ID_INVALID_GUILD_GROUP), idRank(0), idChannel(ID_INVALID_CHANNEL)
		{
			memset(nickname,0x00,sizeof(nickname));
		}

		const GUILD_PLAYER_INFO& operator=(const PLAYER_INFO &ci)
		{
			idAccount = ci.idAccount;
			_ava_wcscpy(nickname, ci.nickname);
			level = ci.level;
			idGroup = ci.guildInfo.idGroup;
			idRank = ci.guildInfo.idRank;

			return *this;
		}

		BOOL HasPriv(_GUILD_PRIV priv) const
		{
			return (idRank >= 0 && idRank < SIZE_GUILD_RANK) ? GUILD_INFO::privInfo[idRank] & priv : FALSE;
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Game statistics data
	struct STAT_GAME_PLAY_LOG
	{
		TID_MAP idMap; // 플레이한 레벨
		BYTE playRound; // 총 플레이 라운드
		BYTE winRounds[2]; // 승리 라운드 (EU, NRF)

		DWORD startTime; // 게임 시작 시간
		DWORD playTime; // 플레이 시간
	};

	struct STAT_GAME_SCORE_LOG
	{
		short killCount; // 킬 수
		WORD suicideCount; // 자살 수
		WORD headshotKillCount; // 헤드샷 킬 수
		BYTE dummy[2];

		PLAYER_SCORE_INFO::SCORE_INFO score; // 점수

		DWORD friendlyFireCount; // Friendly Fire 수
		WORD friendlyKillCount; // Friendly Fire로 킬 한 수

		WORD spawnCount[3]; // 스폰 수 (포인트맨, 라이플맨, 스나이퍼)
	};

	struct STAT_ROUND_PLAY_LOG
	{
		BYTE winTeam; // 승리 팀 (EU, NRF)
		BYTE winType; // 라운드가 끝난 방식 (Annihilation/Mission)
		BYTE playerCount[2]; // 라운드가 끝난 시점에 각 진영별 플레이어 수; 0 = EU, 1 = NRF
		DWORD startTime; // 라운드 시작 시간
		DWORD roundTime; // 플레이 타임
	};

	struct STAT_WEAPON_LOG
	{
		TID_ITEM idWeapon; // 무기 아이디

		BYTE round;
		BYTE dummy;

		DWORD fireCount; // 발사 회수
		WORD usedCount; // 사용 회수/빈도 (스폰하거나, 줍거나)
		WORD hitCount_Head; // 머리 부분 명중 회수
		WORD hitCount_Body; // 몸통 부분 명중 회수
		WORD hitCount_Stomach; // '배 부분 명중 회수
		WORD hitCount_LeftArm; // 왼팔 부분 명중 회수
		WORD hitCount_RightArm; // 오른팔 부분 명중 회수
		WORD hitCount_LeftLeg; // 왼쪽 다리 부분 명중 회수
		WORD hitCount_RightLeg; // 오른쪽 다리 부분 명중 회수

		FLOAT hitDistance; // 평균 사거리
		WORD hitDamage; // 평균 데미지

		short killCount[3]; // 병과별 사살 회수
		WORD headshotKillCount; // 헤드샷으로 사살 회수
		WORD multiKillCount; // 멀티킬 회수
	};

	struct STAT_KILL_LOG
	{
		DWORD killTime; // 죽인 시간
		TID_ITEM idWeapon; // 죽인 무기
		BYTE dummy[2];
		FLOAT killerLocX;
		FLOAT killerLocY;
		FLOAT killerLocZ;
		FLOAT victimLocX;
		FLOAT victimLocY;
		FLOAT victimLocZ;
	};



	//////////////////////////////////////////////////////////////////////////////////////
	// misc

	struct MATCH_ROOM_INFO: ROOM_INFO
	{
		TSN_MATCHROOM match_sn;
		//RXNERVE_ADDRESS addr; // 서버 주소..

		MATCH_ROOM_INFO(): ROOM_INFO(), match_sn(SN_INVALID_MATCHROOM)
		{
			//_ClearAddress(addr);
		}
	};

}

//#pragma pack(pop)

