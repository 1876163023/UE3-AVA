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
	// Host ���� ����

	struct IPADDR_INFO
	{
		// TCP/IP network byte order (which is big endian) ���� �����Ѵ�.
		union{
			unsigned long ipAddress; // IP �ּ�. _addr.s_addr�� ����
			unsigned char ip4b[4];
		};
		unsigned short port; // Port ��ȣ

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

		long limit; // ������ or ��ȿ�Ⱓ. (��¥ or Ƚ��, ������ ITEM_DESC�� ����)

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
		long limit; // ��ȿ�Ⱓ. (��¥ or Ƚ��, ������ ITEM_DESC�� ����)
		TID_ITEM id; // ������ ������ȣ
		BYTE slot; // ������̳�?
		BYTE dummy;

		//TSN_ITEM dummy2;
		TSN_ITEM item_sn; // ������ �������� SN

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
		TID_ITEM	id;				// ������ ������ȣ
		BYTE		dummy[2];

		long		limit;			// ��ȿ�Ⱓ. (��¥ or Ƚ��, ������ ITEM_DESC�� ����)

		TSN_ITEM	item_sn;		// ������ �������� SN

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
		ITEM_INFO weaponInven[MAX_INVENTORY_SIZE]; // �κ��丮
		ITEM_INFO equipInven[MAX_INVENTORY_SIZE]; // �κ��丮

		TSN_ITEM weaponSet[MAX_WEAPONSET_SIZE]; // ���� �� ������ ����.
		TSN_ITEM equipSet[MAX_EQUIPSET_SIZE]; // ���� ������ ITEM_INFO�� index �� ����

		// Ŀ���� ����
		CUSTOM_ITEM_INFO customWeapon[MAX_CUSTOM_INVENTORY_SIZE]; // Ŀ���� ������ ����Ʈ

		EFFECT_ITEM_INFO effectInven[MAX_EFFECT_INVENTORY_SIZE];
		TSN_ITEM effectSet[MAX_EFFECTSET_SIZE]; // ���� ������ ITEM_INFO�� index �� ����

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
	BYTE category : 8; // �ش� �������� ���� ���Ա�.
	BYTE idx : 4; // idx = 0 �̸� �ش� ���� ��� ���Կ� ���� �ִ�.
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

		TID_ITEM defaultItem; // �⺻ ���� ������ ��ȣ.
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

		BYTE useLimitLevel; // ��޺� �������
		BYTE gaugeType;		// 1 = ���� 2 = ������ 3 = ��¥����; _ITEM_GAUGE_TYPE_

		// ������ ������
		DWORD Durability_Game_Drop;
		DWORD Durability_Time_Drop;

		// ���� ������
		TMONEY maintenancePrice; // �ִ� ������

		// �Ⱓ�� ������
		BYTE dateLimit;		// ��� �ϼ�

		BYTE priceType;		// �� �������� �ɽ� ����������; _ITEM_PRICE_TYPE_
		TMONEY price;		// ����

		// ���
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
		BYTE statLog;		// 0 �̸� statistics log�� ��� ����� ������ ����
		BYTE isDefaultItem;

		TMONEY bonusMoney;	// �춧 ���ʽ��� �޴� ���ӸӴ�

		int slotType;		// _EQUIP_POSITION_

		// ����
		BYTE bRisConvertible;
		TID_ITEM RisConvertibleID;
		TMONEY RisConvertiblePrice;

		// Ŀ����
		CUSTOM_SLOT_TYPE customType; // ���������� Ŀ���� ����
		TID_ITEM defaultCustomItem/*[_CSI_MAX]*/; // �⺻���� �����Ŵ� Ŀ���� ������ ������ Mount

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
		TID_ITEM item_id;	// �����Ҽ� �ִ� ������..
		CUSTOM_SLOT_IDX customType; // CUSTOM_SLOT_IDX �����Ҽ� �ִ� ��ġ.

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
		TMONEY bonusMoney;	// �춧 ���ʽ��� �޴� ���ӸӴ�

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

		TMONEY bonusMoney;	// �춧 ���ʽ��� �޴� ���ӸӴ�

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
		// ��ų ����

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
			//TODO PlayRound �� TakenDamage�� �̻��ϴ�.. üũ����...
			WORD playTime; // �÷����� �ð� (��)
			WORD sprintTime; // ������Ʈ�� �ð� (��)
			WORD takenDamage; // ���� ������
			short killCount; // ų Ƚ��

			BYTE playRound; // �÷����� ����/���� ��
			BYTE headshotCount; // �ֹ���� �� ��弦 Ƚ��
			BYTE headshotKillCount; // �ֹ��� ��弦���� �޼��� kill Ƚ��
			BYTE dummy;

			BYTE weaponKillCount[4];

			WORD weaponDamage[4];
		};

		TID_ACCOUNT idAccount;

		BYTE gameWin; // �̱� ���� �� (1 or 0)
		BYTE roundWin; // �̱� ���� ��
		BYTE roundDefeat; // �� ���� ��
		BYTE disconnectCount; // ��Ŀ��Ʈ�� Ƚ�� (1 or 0)

		BYTE deathCount; // ���� Ƚ��
		BYTE helmetDropCount;

		BYTE bulletMultiKillCount; // źȯ���� ��Ƽų �޼��� Ƚ��
		BYTE grenadeMultiKillCount; // ����ź���� ��Ƽų �޼��� Ƚ��

		BYTE roundTopKillCount; // �� ���忡�� ų �� 1�� �޼�
		BYTE roundTopHeadshotKillCount; // �� ���忡�� ��弦 ų �� 1�� �޼�
		BYTE topLevelKillCount; // ��������� ����� ���� ���� ���� ų ��
		BYTE higherLevelKillCount; // ������ ����� ���� ���� ų ��

		BYTE noDamageWinCount; // �������� ���� ���� �ʰ� ���� �¸��� Ƚ��

		BYTE topScoreCount; // �� �� �������� 1���� Ƚ�� (1 or 0)
		BYTE noDeathRoundCount; // �ѹ��� ų ������ ���� ���� ��

		BYTE teamKillCount; // ��ų Ƚ��

		DWORD weaponFireCount[4]; // 0 = pistol, 1 = smg, 2 = rifle, 3 = sniper
		DWORD weaponHitCount[4];
		DWORD weaponHeadshotCount[4];

		SCORE_INFO score; // ���ھ�
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
		// ��ų ����

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
			DWORD playRound; // �÷����� ����/���� ��
			DWORD headshotCount; // �ֹ���� �� ��弦 Ƚ��

			DWORD headshotKillCount; // �ֹ��� ��弦���� �޼��� kill Ƚ��
			DWORD playTime; // �÷����� �ð� (��)

			DWORD sprintTime; // ������Ʈ�� �ð� (��)
			DWORD takenDamage; // ���� ������

			LONG killCount; // ų Ƚ��

			LONG weaponKillCount[4];
			DWORD weaponDamage[4];
		};

		DWORD gameWin; // �̱� ���� ��
		DWORD gameDefeat; // �� ���� ��

		DWORD roundWin; // �̱� ���� ��
		DWORD roundDefeat; // �� ���� ��

		DWORD disconnectCount; // ��Ŀ��Ʈ�� Ƚ��
		DWORD deathCount; // ���� Ƚ��

		SCORE_INFO score; // ���ھ�

		CLASS_SCORE_INFO classScoreInfo[_CLASS_MAX];

		// ���� ����

		WORD straightWinCount; // ���� Ƚ��
		WORD guildStraightWinCount; // Ŭ���� ���� Ƚ��

		DWORD guildWinCount; // Ŭ���� �¸� Ƚ��

		// ��Ÿ
		DWORD teamKillCount; // ��ų Ƚ��

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
		BYTE biaXPFlag;		// 0 = ���� ����ġ�� ���� �ʾ���, 1 = ���� ����ġ�� �޾���, 2 = ���� ����ġ�� ä���� �� ����ġ�� �޾���

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
		TID_CHANNEL idChannel;	// ä�� ������ȣ.
		WORD cnt_player;		// ������ ��
		CHANNEL_INFO() : idChannel(ID_INVALID_CHANNEL), cnt_player(0) {}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Room information
	// �� ����
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

	// �� ����
	struct ROOM_STATE
	{
		BYTE playing;
		BYTE numCurr;
		BYTE currRound;
		BYTE bMatch;

		ROOM_STATE() : playing(RIP_NONE),numCurr(0),currRound(0), bMatch(0) {}
	};

	// �� ����
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
		WCHAR strGuild[SIZE_GUILD_ID+1]; //��ũ������ �����ϴ� ��� �ε���

		WCHAR name[SIZE_GUILD_NAME + 1];

		TID_GUILDMARK idGuildMark;

		BYTE level;
		BYTE cntMember;

		BYTE maxGroups;
		BYTE dummy1;

		DWORD xp;

		TID_ACCOUNT idMaster;

		TPOINT income; // ��忡 ���� ����

		WCHAR motd[SIZE_GUILD_MOTD + 1]; // �����Ҷ� ä��â�� ������ �޼���

		WCHAR regdate[SIZE_GUILD_DATE + 1];

		GUILD_GROUP_INFO groups[10];// �ұ׷� ����

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

	// �÷��̾ ������ �ִ� �������
	struct PLAYER_GUILD_INFO
	{
		TID_GUILD idGuild;

		WCHAR strGuildID[SIZE_GUILD_ID + 1];
		WCHAR guildName[SIZE_GUILD_NAME + 1];

		TID_GUILDMARK idGuildMark;

		BYTE guildLevel;
		BYTE idGroup; /// �Ҹ��� ��ȣ..
		BYTE idRank; // ���.
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
	// Player�� �⺻ ����
	struct PLAYER_INFO
	{
		TID_ACCOUNT idAccount;

		WCHAR nickname[SIZE_NICKNAME+1]; // ĳ���� �̸� - �ѱ� 6�� / ���� 12�� �̳�// (�ּ� �ѱ� 2�� / ���� 4�� �̻�),(){}[]<>|�� Ư������ ���

		BYTE faceType; // ��
		BYTE level; // ���

		BYTE currentClass; // ���������� �÷����ߴ� Ŭ����..
		BYTE lastTeam; // ���������� �÷����ߴ� ��..
		WORD straightWin; // ���� ���� ȸ��

		BYTE dummy;

		BYTE channelMaskList[MAX_CHANNEL_MASK];

		BYTE xpProgress;
		DWORD xp; // ����ġ
		TPOINT supplyPoint; // ����

		//////////////////////////////////////////
		// �������..
		PLAYER_GUILD_INFO guildInfo;

		//////////////////////////////////////////
		// ITEM ����..
		PLAYER_ITEM_INFO itemInfo;

		//////////////////////////////////////////
		// Skill ����.
		PLAYER_SKILL_INFO skillInfo;

		//////////////////////////////////////////
		// Result ����.
		PLAYER_SCORE_INFO scoreInfo;

		//////////////////////////////////////////
		// Medal ����.
		PLAYER_AWARD_INFO awardInfo;


		//////////////////////////////////////////
		// ��Ÿ ĳ�� �� ����ġ..
		//TCASH cash; // �����ϰ� �ִ� ĳ��
		TMONEY money; // ���ӸӴ�

		UDP_HOST_INFO rttTestAddr;

		WCHAR configstr[SIZE_CONFIGSTR1+1]; // DB�� �����ϴ� ���� ���� ���ڿ�
		WCHAR configstr2[SIZE_CONFIGSTR2+1]; // DB�� �����ϴ� ���� ���� ���ڿ�

		DWORD	biaXP;	// ���� ����ġ...

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

	// �κ񿡼� ����ϰ� �� Player�� ����
	struct PLAYER_DISP_INFO
	{
		TID_ACCOUNT idAccount;

		WCHAR nickname[SIZE_NICKNAME+1]; // ĳ���� �̸� - �ѱ� 6�� / ���� 12�� �̳�// (�ּ� �ѱ� 2�� / ���� 4�� �̻�),(){}[]<>|�� Ư������ ���
		WCHAR guildName[SIZE_GUILD_NAME+1];

		TID_GUILDMARK idGuildMark;

		//BYTE faceType; // ��
		BYTE level;// ���
		//BYTE currentClass; // ���������� �÷����ߴ� Ŭ����..
		BYTE dummy;

		TID_ROOM idRoom;	// ��ġ ����

		DWORD gameWin; // �̱� ���� �� (1 or 0)
		DWORD gameDefeat; // �� ���� �� (1 or 0)
		DWORD disconnectCount; // ��Ŀ��Ʈ�� Ƚ�� (1 or 0)
		LONG killCount; // Kill Ƚ��
		DWORD deathCount; // ���� Ƚ��
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

		BYTE faceType; // ��
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
		BYTE idGroup;	// �Ҹ��� ��ȣ.
		BYTE idRank;	// ���.
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
		TID_MAP idMap; // �÷����� ����
		BYTE playRound; // �� �÷��� ����
		BYTE winRounds[2]; // �¸� ���� (EU, NRF)

		DWORD startTime; // ���� ���� �ð�
		DWORD playTime; // �÷��� �ð�
	};

	struct STAT_GAME_SCORE_LOG
	{
		short killCount; // ų ��
		WORD suicideCount; // �ڻ� ��
		WORD headshotKillCount; // ��弦 ų ��
		BYTE dummy[2];

		PLAYER_SCORE_INFO::SCORE_INFO score; // ����

		DWORD friendlyFireCount; // Friendly Fire ��
		WORD friendlyKillCount; // Friendly Fire�� ų �� ��

		WORD spawnCount[3]; // ���� �� (����Ʈ��, �����ø�, ��������)
	};

	struct STAT_ROUND_PLAY_LOG
	{
		BYTE winTeam; // �¸� �� (EU, NRF)
		BYTE winType; // ���尡 ���� ��� (Annihilation/Mission)
		BYTE playerCount[2]; // ���尡 ���� ������ �� ������ �÷��̾� ��; 0 = EU, 1 = NRF
		DWORD startTime; // ���� ���� �ð�
		DWORD roundTime; // �÷��� Ÿ��
	};

	struct STAT_WEAPON_LOG
	{
		TID_ITEM idWeapon; // ���� ���̵�

		BYTE round;
		BYTE dummy;

		DWORD fireCount; // �߻� ȸ��
		WORD usedCount; // ��� ȸ��/�� (�����ϰų�, �ݰų�)
		WORD hitCount_Head; // �Ӹ� �κ� ���� ȸ��
		WORD hitCount_Body; // ���� �κ� ���� ȸ��
		WORD hitCount_Stomach; // '�� �κ� ���� ȸ��
		WORD hitCount_LeftArm; // ���� �κ� ���� ȸ��
		WORD hitCount_RightArm; // ������ �κ� ���� ȸ��
		WORD hitCount_LeftLeg; // ���� �ٸ� �κ� ���� ȸ��
		WORD hitCount_RightLeg; // ������ �ٸ� �κ� ���� ȸ��

		FLOAT hitDistance; // ��� ��Ÿ�
		WORD hitDamage; // ��� ������

		short killCount[3]; // ������ ��� ȸ��
		WORD headshotKillCount; // ��弦���� ��� ȸ��
		WORD multiKillCount; // ��Ƽų ȸ��
	};

	struct STAT_KILL_LOG
	{
		DWORD killTime; // ���� �ð�
		TID_ITEM idWeapon; // ���� ����
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
		//RXNERVE_ADDRESS addr; // ���� �ּ�..

		MATCH_ROOM_INFO(): ROOM_INFO(), match_sn(SN_INVALID_MATCHROOM)
		{
			//_ClearAddress(addr);
		}
	};

}

//#pragma pack(pop)

