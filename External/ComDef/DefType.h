/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: Def.h

	Description: Master type definition commonly used among AVA server and client projects.

***/

#pragma once

//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#include "limits.h"

namespace Def
{
	struct _INT64
	{
		BYTE parts[8];

		_INT64()
		{
			memset(parts, 0, 8);
		}
		_INT64(const _INT64 &n)
		{
			memcpy(parts, n.parts, 8);
		}
		_INT64(const INT64 n)
		{
			memcpy(parts, &n, 8);
		}
		_INT64(const UINT64 n)
		{
			memcpy(parts, &n, 8);
		}
		_INT64(const LONG n)
		{
			memcpy(parts, &n, 4);
			memset(parts + 4, 0, 4);
		}
		_INT64(const ULONG n)
		{
			memcpy(parts, &n, 4);
			memset(parts + 4, 0, 4);
		}
		_INT64(const int n)
		{
			memcpy(parts, &n, 4);
			memset(parts + 4, 0, 4);
		}

		// assignment
		_INT64& operator=(INT64 n)
		{
			memcpy(parts, &n, 8);
			return *this;
		}
		_INT64& operator=(UINT64 n)
		{
			memcpy(parts, &n, 8);
			return *this;
		}
		_INT64& operator=(LONG n)
		{
			memcpy(parts, &n, 4);
			memset(parts + 4, 0, 4);
			return *this;
		}
		_INT64& operator=(ULONG n)
		{
			memcpy(parts, &n, 4);
			memset(parts + 4, 0, 4);
			return *this;
		}

		operator INT64()
		{
			return *((INT64*)parts);
		}
		//operator UINT64()
		//{
		//	return *((UINT64*)parts);
		//}

		// comparison
		friend bool operator==(_INT64 &l, _INT64 &n);
		friend bool operator!=(_INT64 &l, _INT64 &n);
		friend bool operator==(_INT64 &l, const _INT64 &n);
		friend bool operator!=(_INT64 &l, const _INT64 &n);
		friend bool operator==(const _INT64 &l, _INT64 &n);
		friend bool operator!=(const _INT64 &l, _INT64 &n);
		friend bool operator==(const _INT64 &l, const _INT64 &n);
		friend bool operator!=(const _INT64 &l, const _INT64 &n);
		friend bool operator==(_INT64 &l, INT64 n);
		friend bool operator!=(_INT64 &l, INT64 n);
		friend bool operator==(const _INT64 &l, INT64 n);
		friend bool operator!=(const _INT64 &l, INT64 n);
		friend bool operator==(_INT64 &l, LONG n);
		friend bool operator!=(_INT64 &l, LONG n);
		friend bool operator==(const _INT64 &l, LONG n);
		friend bool operator!=(const _INT64 &l, LONG n);

		// add
		friend _INT64 operator+(_INT64 &l, _INT64 &n);
		//friend _INT64 operator+(_INT64 &l, const _INT64 &n);
		//friend _INT64 operator+(const _INT64 &l, _INT64 &n);
		//friend _INT64 operator+(const _INT64 &l, const _INT64 &n);
		//friend _INT64 operator+(_INT64 &l, LONG n);
		//friend _INT64 operator+(const _INT64 &l, LONG n);

		// increment
		_INT64& operator++()
		{
			++(*((INT64*)parts));
			return *this;
		}
		_INT64 operator++(int)
		{
			return _INT64( (*((INT64*)parts)) + 1 );
		}
	};

	inline bool operator==(_INT64 &l, _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) == 0;
	}
	inline bool operator!=(_INT64 &l, _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) != 0;
	}
	inline bool operator==(_INT64 &l, const _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) == 0;
	}
	inline bool operator!=(_INT64 &l, const _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) != 0;
	}
	inline bool operator==(const _INT64 &l, _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) == 0;
	}
	inline bool operator!=(const _INT64 &l, _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) != 0;
	}
	inline bool operator==(const _INT64 &l, const _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) == 0;
	}
	inline bool operator!=(const _INT64 &l, const _INT64 &n)
	{
		return memcmp(l.parts, n.parts, 8) != 0;
	}
	inline bool operator==(_INT64 &l, INT64 n)
	{
		return memcmp(l.parts, &n, 8) == 0;
	}
	inline bool operator!=(_INT64 &l, INT64 n)
	{
		return memcmp(l.parts, &n, 8) != 0;
	}
	inline bool operator==(const _INT64 &l, INT64 n)
	{
		return memcmp(l.parts, &n, 8) == 0;
	}
	inline bool operator!=(const _INT64 &l, INT64 n)
	{
		return memcmp(l.parts, &n, 8) != 0;
	}
	inline bool operator==(_INT64 &l, LONG n)
	{
		return memcmp(l.parts, &n, 8) == 0;
	}
	inline bool operator!=(_INT64 &l, LONG n)
	{
		return memcmp(l.parts, &n, 8) != 0;
	}
	inline bool operator==(const _INT64 &l, LONG n)
	{
		return memcmp(l.parts, &n, 8) == 0;
	}
	inline bool operator!=(const _INT64 &l, LONG n)
	{
		return memcmp(l.parts, &n, 8) != 0;
	}

	inline _INT64 operator+(_INT64 &l, _INT64 &n)
	{
		return _INT64( *((INT64*)l.parts) + *((INT64*)n.parts) );
	}
	//inline _INT64 operator+(_INT64 &l, const _INT64 &n)
	//{
	//	return _INT64( *((INT64*)l.parts) + *((INT64*)n.parts) );
	//}
	//inline _INT64 operator+(const _INT64 &l, _INT64 &n)
	//{
	//	return _INT64( *((INT64*)l.parts) + *((INT64*)n.parts) );
	//}
	//inline _INT64 operator+(const _INT64 &l, const _INT64 &n)
	//{
	//	return _INT64( *((INT64*)l.parts) + *((INT64*)n.parts) );
	//}
	//inline _INT64 operator+(_INT64 &l, LONG n)
	//{
	//	return _INT64( *((INT64*)l.parts) + (INT64)n );
	//}
	//inline _INT64 operator+(const _INT64 &l, LONG n)
	//{
	//	return _INT64( *((INT64*)l.parts) + (INT64)n );
	//}

	//////////////////////////////////////////////////////////////////////////////////////
	// Hardware info
	const	size_t	SIZE_HWID = 64;
	const	size_t	SIZE_ADAPTER_ADDRESS = 8;

	//////////////////////////////////////////////////////////////////////////////////////
	// Account
	typedef ULONG		TSN_USER;

	typedef	ULONG		TID_ACCOUNT;						// ����� ������ Primary Key�� long���̴�.
	const	TID_ACCOUNT	ID_INVALID_ACCOUNT = 0;				// ������� �ʴ� ����� ������ key ��

	const	size_t	SIZE_CONFIGSTR1 = 64;
	const	size_t	SIZE_CONFIGSTR2 = 8;

	//////////////////////////////////////////////////////////////////////////////////////
	// Player ���� ����
	const	size_t	SIZE_HASH_STR = 255;
	const	size_t	SIZE_USER_ID  = 12;
	const	size_t	SIZE_USER_PWD = 12;	
	const	size_t	SIZE_NICKNAME = 10;
	const	size_t	SIZE_USERNAME = 12;

	const	size_t	SIZE_NICKNAME_MIN = 3;
	const	size_t	FACETYPE_MAX = 5;
	const	BYTE	LEVEL_MAX = 50;
	const	int		SUPPLYPOINT_MAX = 15000;
	const	int		BIA_EXP_MAX = 500;

	enum _RECONNECT_FLAG
	{
		_RF_NORMAL = 0,
		_RF_CHANGING_SESSION,
		_RF_FORCED_CONNECT,
		_RF_FOLLOWING_PLAYER,
	};

	enum _BIAXP_FLAG
	{
		_BIAXP_NONE = 0,
		_BIAXP_RECV = 1,
		_BIAXP_FULL = 2,
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Server
	typedef	WORD		TID_SERVER;							// ���� ���̵�
	const	TID_SERVER	ID_INVALID_SERVER = 0;				// ������� �ʴ� ���� ���̵�

	//////////////////////////////////////////////////////////////////////////////////////
	// Channel
	typedef	WORD		TID_CHANNEL;						// Channel ID Type
	const	TID_CHANNEL	ID_INVALID_CHANNEL = 0;				// Channel ID �� ������� �ʴ� ��
	const	TID_CHANNEL	ID_MY_CLAN_HOME = USHRT_MAX;		// �� Ŭ�� Ȩ

	const	size_t		MAX_CHANNEL_PER_SERVER = 20;
	const	size_t		MAX_CHANNEL_PER_GAME = 10000;
	const	size_t		MAX_CHANNEL_MASK = 26;


	enum _CHANNEL_FLAG
	{
		_CF_NORMAL,
		_CF_TRAINEE,
		_CF_MATCH,
		_CF_RESERVE2,
		_CF_RESERVE3,
		_CF_NEWBIE,
		_CF_CLAN,
		_CF_PCBANG,
		_CF_EVENT,
		_CF_MYCLAN,
		_CF_PRACTICE,
		_CF_BROADCAST,
		_CF_AUTOBALANCE,
		_CF_TEMP,
		_CF_MAX,
	};

	enum _CHANNEL_MASK_LEVEL
	{
		_CML_NONE = 0,
		_CML_PLAYER,
		_CML_REFREE,
		_CML_BROADCAST,
	};

	const	float		CHANNEL_SDRATIO_THRESHOLD_NEWBIE = 1.0f;
	const	int			CHANNEL_LEVEL_THRESHOLD_TRAINEE = 8;


	//////////////////////////////////////////////////////////////////////////////////////
	// Lobby
	const	size_t		MAX_ROOM_PER_PAGE = 12;				// CL�� �� ��� �� ȭ�鿡�� �� �� �ִ� ���� ��
	const	size_t		MAX_PLAYER_PER_PAGE = 24;			// CL�� �� ��� �� ȭ�鿡�� �� �� �ִ� ���� ��

	//////////////////////////////////////////////////////////////////////////////////////
	// Room
	typedef	WORD		TID_ROOM;							// Room ID Type	
	const	TID_ROOM	ID_INVALID_ROOM = 0;				// �� ID�� �� ������� �ʴ� ���̵�
	const	BYTE		ID_INVALID_ROOM_SLOT = UCHAR_MAX;

	const	size_t		MAX_PLAYER_PER_TEAM = 12;
	const	size_t		MAX_PLAYER_PER_ROOM = MAX_PLAYER_PER_TEAM * 2;
	const	size_t		MAX_SPECTATOR_PER_ROOM = 12;
	const	size_t		MAX_ALL_PLAYER_PER_ROOM = MAX_PLAYER_PER_ROOM + MAX_SPECTATOR_PER_ROOM;

	const	size_t		SIZE_ROOM_NAME = 16;
	const	size_t		SIZE_ROOM_PWD = 8;

	enum _READY_STATE
	{
		_READY_NONE = 0,
		_READY_WAIT = 1,
		_READY_LOADING = 2,
		_READY_PLAYING = 3,
		//_READY_MAX,
	};	

	enum _ROOM_STATE
	{
		RIP_NONE = 0,		// 
		RIP_WAIT,		// ������� ��		
		RIP_PLAYING,	// �÷������� ��		

		RIP_ALLREADY,
		RIP_MATCH_WAIT,
	};

	// �� ���͸� ���
	enum _ROOM_INFO_FILTER
	{
		RIF_ALL = 0,	// �÷��� ���� ��
		RIF_WAIT,		// ������� ��		
	};

	/// �� ��Ͽ��� ������ ���� ���� 
	enum _ROOM_PAGE_DIRECTION
	{
		PAGE_NONE = 0,
		PAGE_PREV = 1,
		PAGE_NEXT = 2,
	};

	enum _ROOM_TEAM
	{
		RT_EU = 0,
		RT_NRF,
		RT_SPECTATOR,
		RT_MAX,
		RT_NONE = UCHAR_MAX,
	};

	enum _ROOM_TEAMKILL
	{
		TK_NO_DAMAGE,
		TK_NO_BULLETDAMAGE,
		TK_ALL_DAMAGE
	};
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Match Room
	const	size_t		MAX_PLAYER_PER_MATCHROOM = 12;
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Guild

	typedef	DWORD		TID_GUILD;							// Guild ID Type
	typedef int			TID_GUILDMARK;						// guild mark id

	const	TID_GUILD	ID_INVALID_GUILD = 0;
	const	TID_GUILDMARK	ID_INVALID_GUILDMARK = -1;
	const	TID_GUILDMARK	ID_NO_GUILDMARK = 0;
	const	BYTE		ID_INVALID_GUILD_GROUP = 0;
	const	size_t		SIZE_GUILD_ID = 12;
	const	size_t		SIZE_GUILD_NAME = 12;
	const	size_t		SIZE_GUILD_MOTD = 100;
	const	size_t		SIZE_GUILD_GROUP_NAME = 12;
	const	size_t		SIZE_GUILD_RANK = 5;
	const	size_t		SIZE_GUILD_RANK_NAME = 12;
	const	size_t		SIZE_GUILD_MEMBERS_PER_GROUP = 10;
	const	size_t		SIZE_GUILD_DATE = 30;      //Ŭ�� ���� ��¥, �߰� : 2007.6.12 ������

	enum _GUILD_PRIV
	{
		PRIV_INVITE					= (1 << 0),
		PRIV_KICK					= (1 << 1),
		PRIV_SET_MOTD				= (1 << 2),
		PRIV_SEND_NOTICE			= (1 << 3),
		PRIV_CREATE_ROOM			= (1 << 4),
		PRIV_JOIN_ROOM				= (1 << 5),
		PRIV_CREATE_MATCH			= (1 << 6),
		PRIV_JOIN_MATCH				= (1 << 7),
		PRIV_EDIT_GROUP				= (1 << 8),
		PRIV_INVENTORY_BROWSE		= (1 << 9),
		PRIV_INVENTORY_DEPOSIT		= (1 << 10),
		PRIV_INVENTORY_WITHDRAW		= (1 << 11),
		PRIV_SHOP_BROWSE			= (1 << 12),
		PRIV_SHOP_BUY				= (1 << 13),
		PRIV_SHOP_REFUND			= (1 << 14),

		PRIV_ADMIN					= PRIV_KICK | PRIV_SET_MOTD | PRIV_SEND_NOTICE | PRIV_EDIT_GROUP,
		PRIV_INVENTORY				= PRIV_INVENTORY_BROWSE | PRIV_INVENTORY_DEPOSIT | PRIV_INVENTORY_WITHDRAW,
		PRIV_SHOP					= PRIV_SHOP_BROWSE | PRIV_SHOP_BUY | PRIV_SHOP_REFUND,

		PRIV_NONE					= 0,
		PRIV_FULL					= 0xffffffff,
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Item
	typedef WORD		TID_ITEM;
	const TID_ITEM		ID_INVALID_ITEM = 0;

	typedef _INT64		TSN_ITEM;					// Item�� Global Unique ID Type
	const TSN_ITEM		SN_INVALID_ITEM = 0;

	const size_t MAX_INVENTORY_SIZE = 50;			// �κ��丮 ũ��
	const size_t MAX_WEAPONSET_SIZE = 18;			// ���� Equip ����..	
	const size_t MAX_EQUIPSET_SIZE = 21;			// ���� Equip ����..	
	const size_t MAX_CUSTOM_INVENTORY_SIZE = UCHAR_MAX;
    const size_t MAX_EFFECTSET_SIZE = 3;
	const size_t MAX_EFFECT_INVENTORY_SIZE = 50;

	const	long	ITEM_LIMIT_INITED = 50000;

	enum _ITEM_FLAG_
	{
		_IF_NONE = -1,
		_IF_WEAPON = 0,		// ���� ������.
		_IF_EQUIP,			// ���� ������
		_IF_CUSTOM,			// Ŀ���� ������
		_IF_EFFECT,			// BoostItem..
	};

	enum _ITEM_PRICE_TYPE_
	{
		_IPT_NONE = 0,		// ����Ĵ� �������� �ƴϴ�.
		_IPT_CASH = 1,		// ĳ��
		_IPT_MONEY = 2,		// �Ӵ�		
	};

	enum _ITEM_DISPLAY_TYPE_
	{
		_IDT_NONE = 0,
		_IDT_SHOW = 1,
		_IDT_HOT = 2,
		_IDT_NEW = 3,
	};

	enum _ITEM_GAUGE_TYPE_
	{
		_IGT_NONE = 0,		// ����Ĵ� �������� �ƴϴ�.
		_IGT_MAINTENANCE = 1,	// ����
		_IGT_DURABILITY = 2,	// ������
		_IGT_DATE = 3,			// ��¥..
	};

	enum _ITEM_CATEGORY_
	{
		_IC_FACE = 0,

		//_IC_PRIMARY_WEAPON = 1,		// 1. �ֹ���
		//_IC_SECONDARY_WEAPON,		// 2. ��������
		//_IC_MELEE_WEAPON,			// 3. ��������
		//_IC_GRENADE_WEAPON,			// 4. ��ô����
		//_IC_ABILITY_ITEM,			// 5. ��ɾ�����
		//_IC_VISUAL_ITEM,			// 6. ġ�������
		//_IC_CUSTOM_ITEM,			// 7. Ŀ���͸����������
		//_IC_COLLECTING_ITEM,		// 8. �÷��þ�����

		// ����
		_IC_PISTOL = 1,		// ����
		_IC_SMG = 3,		// SMG
		_IC_AR = 5,			// AR
		_IC_SR = 7,			// SR
		_IC_KNIFE = 9,		// Knife
		_IC_GRENADE = 11,	// ThrowWeapon
		_IC_HEAVY = 13,		// HeavyWeapon

		// ��ź��
		_IC_HELMET = 21,	// Helmet
		_IC_ARMOR = 23,		// Armor

		// ��Ÿ
		_IC_HEAD = 31,		// Head
		_IC_TORSO = 33,		// Torso
		_IC_LEG = 35,		// Leg

		_IC_EFFECT = 45,

		// Ŀ����
		_IC_CUSTOM = 51,

		// �÷���
		_IC_COLLECTING = 101,

		_IC_SKILL_POINTMAN = 200,
		_IC_SKILL_RIFLEMAN = 201,
		_IC_SKILL_SNIPER = 202,
	};

//#define IsFace(id) ( HIBYTE(id) == _IC_FACE )
	inline BOOL IsFace(TID_ITEM id)
	{
		return HIBYTE(id) == _IC_FACE;
	}

//#define IsEquipItem(id)					\
//	(	HIBYTE(id) == _IC_HELMET ||		\
//	HIBYTE(id) == _IC_ARMOR ||		\
//	HIBYTE(id) == _IC_HEAD ||		\
//	HIBYTE(id) == _IC_TORSO ||		\
//	HIBYTE(id) == _IC_LEG)
	inline BOOL IsEquipItem(TID_ITEM id)
	{
		BYTE ic = HIBYTE(id);
		return ic == _IC_HELMET ||
				ic == _IC_ARMOR ||
				ic == _IC_HEAD ||
				ic == _IC_TORSO ||
				ic == _IC_LEG;
	}

//#define IsWeaponItem(id)				\
//	(	HIBYTE(id) == _IC_PISTOL ||		\
//	HIBYTE(id) == _IC_SMG ||		\
//	HIBYTE(id) == _IC_AR ||			\
//	HIBYTE(id) == _IC_SR ||			\
//	HIBYTE(id) == _IC_KNIFE ||		\
//	HIBYTE(id) == _IC_GRENADE ||	\
//	HIBYTE(id) == _IC_HEAVY)
	inline BOOL IsWeaponItem(TID_ITEM id)
	{
		BYTE ic = HIBYTE(id);
		return ic == _IC_PISTOL ||
				ic == _IC_SMG ||
				ic == _IC_AR ||
				ic == _IC_SR ||
				ic == _IC_KNIFE ||
				ic == _IC_GRENADE ||
				ic == _IC_HEAVY;
	}

//#define IsCustomItem(id)	( HIBYTE(id) >= _IC_CUSTOM && HIBYTE(id) < _IC_COLLECTING )
	inline BOOL IsCustomItem(TID_ITEM id)
	{
		BYTE ic = HIBYTE(id);
		return ic >= _IC_CUSTOM && ic < _IC_COLLECTING;
	}

//#define IsEffectItem(id)	( HIBYTE(id) == _IC_EFFECT )
	inline BOOL IsEffectItem(TID_ITEM id)
	{
		return HIBYTE(id) == _IC_EFFECT;
	}

	enum _WEAPON_TYPE
	{
		_WEAPON_KNIFE,
		_WEAPON_PISTOL,
		_WEAPON_GRENADE,
		_WEAPON_SMG,
		_WEAPON_RIFLE,
		_WEAPON_SNIPER,
		_WEAPON_MAX,
		_WEAPON_PRIMARY = 3,
	};

	enum CUSTOM_SLOT_IDX
	{
		_CSI_NONE = UCHAR_MAX,

		_CSI_FRONT = 0,
		_CSI_MOUNT,
		_CSI_BARREL,
		_CSI_TRIGGER,
		_CSI_GRIP,
		_CSI_STOCK,
		_CSI_MAX,
	};

	enum ITEM_EFFECT_TYPE
	{
		_IET_NONE = 0,
		_IET_GR,				// ����ź ���� �߰�..

		_IET_EXP_BOOST,			// ����ġ ��.
		_IET_SP_BOOST,			// ����ġ ��.
		_IET_MONEY_BOOST,		// ���ӸӴ� ��.
		_IET_ACCESSORY,			// �Ǽ�����
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Equip
	typedef BYTE			TID_EQUIP_SLOT;
	const	TID_EQUIP_SLOT	ID_INVALID_EQUIP_SLOT = UCHAR_MAX;

	// �÷��̾ �����Ǵ� ��ġ
	enum _EQUIP_POSITION_
	{
		_EP_NONE = 0,

		// ĳ���� ����
		_EP_H1 = (1 << 1),
		_EP_H11 = (1 << 2),
		_EP_H12 = (1 << 3),
		_EP_H2 = (1 << 4),
		_EP_H3 = (1 << 5),
		_EP_C1 = (1 << 6),
		_EP_C2 = (1 << 7),
		_EP_A1 = (1 << 8),
		_EP_A2 = (1 << 9),
		_EP_B1 = (1 << 10),
		_EP_B3 = (1 << 11),
		_EP_W1 = (1 << 12),
		_EP_W2 = (1 << 13),
		_EP_W3 = (1 << 14),
		_EP_T1 = (1 << 15),
		_EP_T2 = (1 << 16),
		_EP_E = (1 << 17),
		_EP_G = (1 << 18),
		_EP_K = (1 << 19),
		_EP_BT = (1 << 20),
		_EP_BD = (1 << 21),
		_EP_FACE = (1 << 22),


		// ���� ����
		_EP_R1 = (1 << 1),
		_EP_R2 = (1 << 2),
		_EP_R3 = (1 << 3),
		_EP_R4 = (1 << 4),
		_EP_P1 = (1 << 5),
		_EP_P2 = (1 << 6),
		_EP_P3 = (1 << 7),
		_EP_P4 = (1 << 8),
		_EP_S1 = (1 << 9),
		_EP_S2 = (1 << 10),
		_EP_S3 = (1 << 11),
		_EP_S4 = (1 << 12),

		// ����
		_EP_WEAP_RIFLEMAN = (_EP_R1 | _EP_R2 | _EP_R3 | _EP_R4),	
		_EP_WEAP_POINTMAN = (_EP_P1 | _EP_P2 | _EP_P3 | _EP_P4),
		_EP_WEAP_SNIPER = (_EP_S1 | _EP_S2 | _EP_S3 | _EP_S4),
		_EP_WEAP_PRIMARY = (_EP_R1 | _EP_P1 | _EP_S1),
		_EP_WEAP_SECONDARY = (_EP_R2 | _EP_P2 | _EP_S2),
		_EP_WEAP_MELEE = (_EP_R3 | _EP_P3 | _EP_S3),
		_EP_WEAP_GRENADE = (_EP_R4 | _EP_P4 | _EP_S4),
		_EP_WEAP_CUSTOMIZABLE = (_EP_WEAP_PRIMARY),
		//_EP_WEAPON = (_EP_WEAP_RIFLEMAN | _EP_WEAP_POINTMAN | _EP_WEAP_SNIPER),
		//_EP_CHARACTER = ~_EP_WEAPON,
		_EP_C = (_EP_C1 | _EP_C2),
		_EP_A = (_EP_A1 | _EP_A2),
		_EP_W = (_EP_W1 | _EP_W2 | _EP_W3),
		_EP_T = (_EP_T1 | _EP_T2),

		// ����Ʈ ������
		_EP_EFF_EE = (1 << 1),
		_EP_EFF_ES = (1 << 2),
		_EP_EFF_EM = (1 << 3),
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Map
	typedef	BYTE		TID_MAP;							// ���� �� ���̵�
	const	TID_MAP		ID_INVALID_MAP = 0;

	enum _MAP_TYPE
	{
		_MAP_SPECIAL = 0,
		_MAP_WARFARE,
		_MAP_TRAINING,
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Etc..
	typedef DWORD	TPOINT;						// ����Ʈ
	typedef DWORD	TCASH;						// ĳ��

	typedef DWORD	TMONEY;						// ����

	const	TMONEY	DEFAULT_GAME_MONEY = 80000;
		
	// Ticker
	const size_t MAX_TICKER = 10;
	const size_t MAX_TICKER_STR = 128;

	// Notice
	const size_t SIZE_NOTICE_MSG = 256;

	// pcbang service type
	enum _PCBANG_SERVICE_TYPE
	{
		_PCB_NONE = 0,
		_PCB_AVA = 69863
	};


	//////////////////////////////////////////////////////////////////////////////////////
	// Skill
	const BYTE		ID_INVALID_SKILL = 0;
	const size_t	MAX_SKILL_PER_CLASS = 16;			//��ų ����

	#define IsSkillID(id) (	HIBYTE(id) == _IC_SKILL_POINTMAN || HIBYTE(id) == _IC_SKILL_RIFLEMAN ||  HIBYTE(id) == _IC_SKILL_SNIPER )

	enum _SKILL_POSITION
	{
		// pointman skill
		_SKILL_RUNNING_BASIC = (1 << 1),
		_SKILL_RUNNING_EXPERT = (1 << 2),
		_SKILL_RUNNING_MASTER = (1 << 3),
		_SKILL_SAFE_LANDING = (1 << 4),
		_SKILL_FAST_SILENT_MOVE = (1 << 5),
		_SKILL_FAST_SPRINT = (1 << 6),
		_SKILL_ADVANCED_KNIFE_MOVE = (1 << 7),
		_SKILL_SHARP_KNIFE = (1 << 8),
		_SKILL_SMG_QUICK_RELOAD = (1 << 9),
		_SKILL_SMG_EXTRA_ARMO = (1 << 10),
		_SKILL_BASIC_BATTLE_INSTINT = (1 << 11),
		_SKILL_ADVANCED_BATTLE_INSTINT = (1 << 12),

		// rifleman skill
		_SKILL_LOW_CLASS_DEFENCE = (1 << 1),
		_SKILL_MEDIUM_CLASS_DEFENCE = (1 << 2),
		_SKILL_HIGH_CLASS_DEFENCE = (1 << 3),
		_SKILL_HARDEN_HELMET = (1 << 4),
		_SKILL_ADVANCED_HELMET_DEFENCE = (1 << 5),
		_SKILL_POW_ROLLING = (1 << 6),
		_SKILL_POW_THROWING = (1 << 7),
		_SKILL_QUICK_THROW = (1 << 8),
		_SKILL_BASIC_AIMED_SHOT = (1 << 9),
		_SKILL_ADVANCED_AIMED_SHOT = (1 << 10),
		_SKILL_AR_EXTRA_AMMO = (1 << 11),
		_SKILL_AR_QUICK_RELOAD = (1 << 12),

		// sniper skill
		_SKILL_SNIPING_NOVICE = (1 << 1),
		_SKILL_SNIPING_EXPERT = (1 << 2),
		_SKILL_SNIPING_MASTER = (1 << 3),
		_SKILL_CROUCH_SNIPING = (1 << 4),
		_SKILL_MOVING_SNIPING = (1 << 5),
		_SKILL_PISTOL_EXTRA_AMMO = (1 << 6),
		_SKILL_PISTOL_QUICK_RELOAD = (1 << 7),
		_SKILL_SR_QUICK_EQUIP = (1 << 8),
		_SKILL_SR_EXTRA_AMMO = (1 << 9),
		_SKILL_SR_QUICK_MOVE = (1 << 10),
		_SKILL_SR_QUICK_RELOAD = (1 << 11),
	};	

	//////////////////////////////////////////////////////////////////////////////////////
	// Medal...
	typedef BYTE	TID_AWARD;						
	const TID_AWARD	ID_INVALID_AWARD = UCHAR_MAX;
	const size_t MAX_AWARD_PER_PLAYER = 128;
	const size_t MAX_COUNT_PER_AWARD = 999;
	const size_t MAX_STRAIGHT_WIN_COUNT = 999;

	//////////////////////////////////////////////////////////////////////////////////////
	// Player...
	enum _PLAYER_CLASS
	{
		_CLASS_NONE	= -1,
		_CLASS_POINTMAN = 0,
		_CLASS_RIFLEMAN = 1,
		_CLASS_SNIPER = 2,
		_CLASS_MAX,
	};

	enum _PLAYER_LEVEL
	{
		_LEV_UNKNOWN = 0,
		_LEV_TRAINEE = 1,
		_LEV_SOLDIER = 2,
		_LEV_SERGEANT = 6,
		_LEV_LIEUTENANT = 21,
		_LEV_OFFICER = 36,
		_LEV_COMMANDER = 51,
		_LEV_MAX = 70
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Result codes
	enum _RESULT_CODE_
	{
		// Common
		RC_SUCCEED = 0,						// ����..
		RC_OK = 0,							
		RC_FAIL,							// ����..

		RC_INVALID_VERSION,

		// �α��� ����..
		RC_AUTH_ALREADY_EXIST,				// �̹� �������̴�...	
		RC_AUTH_ADMIN_KICKED,					// ��ڿ��� ű���� ����

		// 
		RC_LOGIN_NO_NICK,					// ���� ����.. ó�� ����.	
		RC_NICK_ALREADY_EXIST,				// �г����� �̹� �����Ѵ�.

		RC_INVALID_NICKNAME,

		RC_SESSION_MOVE,
		RC_SERVER_MOVE,

		RC_CHANNEL_INVALID_LEVEL,			// ���� ���ѿ� �ɷ��� ���� �Ұ���
		RC_CHANNEL_PLAYER_FULL,				// �ο��� ���ѿ� �ɷ��� ���� �Ұ���
		RC_CHANNEL_ROOM_FULL,				// ä�ο� ����� �ִ� ���� ������ ��á��.

		RC_INVALID_ROOMNAME,
		RC_ROOM_PLAYER_FULL,				// �����Ϸ��� ���� �ο��� �� �� ���
		RC_ROOM_INVALID_PASSWORD,			// �� ��й�ȣ�� Ʋ�� ���
		RC_ROOM_INVALID_LEVEL,				// �濡 �����Ϸ��� �÷��̾��� ������ ���� ���� ���
		RC_ROOM_INVALID_CONDITION,			// ���ǿ� �´� ���� �� ã�� ���
		RC_ROOM_INVALID_STATE,
		RC_ROOM_INVALID_CLAN,				// �� �� �ִ� Ŭ�� ����� �ƴ� ���
		RC_ROOM_BAN_PLAYER,					// �� �� ������� ���� ���
		RC_QUICKJOIN_ROOM_NOTFOUND,			// ���� ������ ���� ã���� ���� ���

		RC_GUILD_NOT_JOIN,
		RC_GUILD_NAME_ALREADY_EXIST,
		RC_GUILD_LOW_LEVEL,
		RC_GUILD_ALREADY_EXIST,				
		RC_GUILD_NO_PRIV,					// ���� ����
		RC_GCS_NOT_FOUND,
		RC_INVALID_CLAN,					// �ش� Ŭ���� ���� ���� ������
		RC_NOT_CLANMEMBER,					// Ŭ�� ����� �ƴ�

		RC_INVENTORY_FULL,
		RC_ITEM_NOT_USE_LEVEL,

		RC_SEND_CONTINUE,
		RC_DB_ERROR,

		RC_ITEM_FULL_GAUGE,

		// Ŭ������(CLS)
		RC_CLANMEMBER_OVER,			// Ŭ������ ���� �ʰ�(100��)
		RC_CLANMEMBER_ALREADY,		// �̹� �����ϴ� Ŭ����
		RC_CLANMEMBER_UNKNOWNSTATE,	// Ŭ������ STATE�� �˼����� ���ܻ�Ȳ
		RC_CLANMEMBER_NONEXISTENT,	// �������� �ʴ� Ŭ����
		RC_CLAN_CREATE_FAIL,		// Ŭ�� ���� ����
		RC_CLANROOM_NOTHING,		// �ش� ���� ã���� �������
		RC_CLANROOM_FULL,			// Ŭ������ ���Ѽ���ŭ �����Ǿ� ���̻��� ������� �Ұ����� ���
		RC_CLAN_DBWRITE_FAIL,		// Ŭ�������� DB WRITE�� FAIL
		RC_CLAN_DBREAD_FAIL,		// Ŭ�������� Ŭ�������� DB�κ��� Read�� ����

		RC_CHANNEL_NO_PRIV,			// ä�ο� ������ ������ ����
		RC_CHANNEL_INVALID_SD,		// S/D ���ѿ� �ɷ��� ä�� ���� �Ұ���
		RC_CHANNEL_INVALID_CLAN,	// Ŭ������ ������ �� �ִ� ä��
		RC_CHANNEL_PCBANG_ONLY,		// PC�� ����ڸ� ������ �� �ִ� ä��

		RC_ITEM_EXIST_PREV_BUYITEM,	// ���� �����ƿ� ���� ó������ �ʾҽ�..
	};


	// kick reason
	enum _KICK_REASON
	{
		KR_NONE = 0,
		KR_BAN,
		KR_LOADING_TIMEOUT,
		KR_SETTING_CHANGED,
		KR_ROOM_DESTROYED,
		KR_LAG,
		KR_INVALID_LEVEL,
		KR_INVALID_CLAN,
		KR_INVALID_SD,
		KR_PCBANG_ONLY,
	};


	// game exit reason
	enum _EXIT_CODE
	{
		EXIT_GAME_END,					// ���� ����
		EXIT_GAME_GUARD_ERROR,			// ���� ���� ����
		EXIT_FORCED_EXIT,				// Ŭ���̾�Ʈ ���� ���� �� ���� ���� ����
		EXIT_INVALID_STATE,				// �ǵ����� ���� ��Ȳ���� �޽��� �޾��� ��
		EXIT_FAILED_TO_CONNECT,			// Gate ���� ����
		EXIT_FAILED_TO_CREATE_SESSION,	// ���� ���� ����
		EXIT_FAILED_TO_CHANGE_SESSION,	// ���� �̵� ����
		EXIT_SERVER_SIDE_EXIT,			// ������ ���� ����
		EXIT_UNKNOWN,
	};

	// room leaving reason
	enum _ROOM_LEAVING_REASON
	{
		ELR_NORMAL,
		ELR_IDLE,
		ELR_DIDNT_READY,
		ELR_HOST_DIDNT_START,
		ELR_PACKAGE_MISMATCH,
		ELR_PACKAGE_NOT_FOUND,
		ELR_FAILED_TO_CONNECT_HOST,
		ELR_REJECTED_BY_HOST,
		ELR_MD5_FAILED,
		ELR_P2P_CONNECTION_FAILED,
	};


	// team-swap, reposition reason
	enum _REPOSITION_REASON
	{
		RR_NONE = 0,
		RR_HOSTREQ,
		RR_AUTOBALANCE,
		RR_GAMEREQ,
		RR_AUTOSWAPTEAM,
	};

	//////////////////////////////////////////////////////////////////////////////////////
	// Vote

	enum _VOTE_COMMAND
	{
		VC_KICK,
	};

	const	TMONEY	VOTE_FEE = 0;	

	//
	//////////////////////////////////////////////////////////////////////////////////////
	// ����� ��ġ�� ���̴� Ű.
	typedef _INT64		TSN_MATCHROOM;
	typedef _INT64		TSN_ROOM;

	const TSN_MATCHROOM		SN_INVALID_MATCHROOM = 0;
	const TSN_ROOM			SN_INVALID_ROOM = 0;
	const size_t			MAX_MATCHROOM_PER_SERVER = 100;

	///////////////////////////////////////////////////////////////////////////////////////
	// CPlayer <==> Inventory Ŭ������... ����Ÿ.. ������Ʈ ����..
	enum _ITEM_UPDATE_TYPE_
	{
		_IUT_NONE_ = 0,
		_IUT_ITEM_INSERT_,
		_IUT_ITEM_DELETE_,
		_IUT_ITEM_GAUGE_UPDATE_,
		_IUT_CUSTOM_ITEM_DELETE_,
		_IUT_CUSTOM_ITEM_GAUGE_UPDATE_,
		_IUT_EFFECT_ITEM_DELETE_,
		_IUT_EFFECT_ITEM_GAUGE_UPDATE_,
	
		_IUT_ITEM_DATE_CHECK_,		
		_IUT_EFFECT_ITEM_DATE_CHECK_,
	};
};

