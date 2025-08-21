/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: MsgDefRoom.h

	Description: Message definition about room.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("ROOM")
#define __MC MC_ROOM

namespace PM
{
	namespace ROOM
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,
			ID_CREATE_REQ,
			ID_CREATE_ANS,
			ID_JOIN_REQ,
			ID_JOIN_ANS,
			ID_JOIN_NTF,
			ID_INFO_REQ,
			ID_INFO_ANS,			
			ID_LEAVE_REQ,
			ID_LEAVE_ANS,
			ID_LEAVE_NTF,
			//ID_PLAYER_LIST_NTF,
			ID_CHAT_NTF,
			ID_CHANGE_HOST_NTF,
			ID_KICK_PLAYER_NTF,
			ID_CHANGE_SETTING_REQ,
			ID_CHANGE_SETTING_ANS,
			ID_CHANGE_SETTING_NTF,
			ID_CHANGE_STATE_NTF,
			ID_READY_NTF,
			ID_CHANGE_SLOT_NTF,
			ID_CHANGE_CLASS_NTF,
			ID_CHANGE_WEAPON_NTF,
			//ID_DESTROY_NTF,

			ID_PLAYER_INFO_REQ,
			ID_PLAYER_INFO_ANS,

			ID_RTT_RATING_NTF,		// deprecated
			ID_RTT_UPDATE_NTF,
			
			ID_CLAIM_HOST_NTF,		// deprecated

			ID_SET_HOSTADDR_NTF,

			ID_SWAP_TEAM_NTF,

			ID_RTTT_START_REQ,
			ID_RTTT_START_ANS,
			ID_RTTT_START_NTF,

			ID_QUICK_JOIN_REQ,
			ID_QUICK_JOIN_ANS,

			ID_REPOSITION_NTF,
			ID_HOST_RATING_NTF,

			ID_ITEM_REPAIR_NTF,

			ID_CLAN_INFO_NTF,

			ID_UPDATE_PCBANG_NTF,
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)


		namespace CREATE_REQ
		{
			enum _ID { ID = MSGDEF_ID(CREATE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_REQ);
				
				//TID_ACCOUNT		svr_acc_id;

				WCHAR roomName[SIZE_ROOM_NAME+1];
				WCHAR password[SIZE_ROOM_PWD+1];
				ROOM_SETTING setting;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CREATE_ANS
		{
			enum _ID { ID = MSGDEF_ID(CREATE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_ANS);

				//TID_ACCOUNT		svr_acc_id;
				BYTE result;

				TID_ROOM	idRoom;				// 새로 생성된 룸의 ID
				BYTE idSlot;					// 방장의 슬롯 번호
			};

			MSGDEF_TYPE TMSG;
		};

		namespace JOIN_REQ
		{
			enum _ID { ID = MSGDEF_ID(JOIN_REQ) };

			struct DEF
			{
				MSGDEF_NAME(JOIN_REQ);

				//TID_ACCOUNT		svr_acc_id;
				TID_ROOM	idRoom;
				WCHAR password[SIZE_ROOM_PWD+1];
			};

			MSGDEF_TYPE TMSG;
		};

		namespace JOIN_ANS
		{
			enum _ID { ID = MSGDEF_ID(JOIN_ANS) };

			struct DEF
			{
				MSGDEF_NAME(JOIN_ANS);

				//TID_ACCOUNT		svr_acc_id;
				BYTE result;

				TID_ROOM	idRoom;			// 조인한 룸의 ID

				ROOM_SETTING setting;
				ROOM_STATE state;
				//ROOM_PLAYER_INFO playerInfo[MAX_ALL_PLAYER_PER_ROOM];
				PM::_BUFFER playerList;			// array of ROOM_PLAYER_INFO

				BYTE idHost;
				
			};

			MSGDEF_TYPE TMSG;
		};

		namespace JOIN_NTF
		{
			enum _ID { ID = MSGDEF_ID(JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(JOIN_NTF);

				//TID_ACCOUNT		svr_acc_id;

				ROOM_PLAYER_INFO playerInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace INFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(INFO_REQ);				
			};

			MSGDEF_TYPE TMSG;
		};

		namespace INFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(INFO_ANS);

				TID_ROOM	idRoom;			// 조인한 룸의 ID

				ROOM_SETTING setting;
				ROOM_STATE state;
				//ROOM_PLAYER_INFO playerInfo[MAX_ALL_PLAYER_PER_ROOM];
				PM::_BUFFER playerList;			// array of ROOM_PLAYER_INFO

				BYTE idHost;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANGE_STATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_STATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_STATE_NTF);

				ROOM_STATE state;				
			};

			MSGDEF_TYPE TMSG;
		};

		namespace LEAVE_REQ
		{
			enum _ID { ID = MSGDEF_ID(LEAVE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(LEAVE_REQ);

				BYTE reason;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace LEAVE_ANS
		{
			enum _ID { ID = MSGDEF_ID(LEAVE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(LEAVE_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace LEAVE_NTF
		{
			enum _ID { ID = MSGDEF_ID(LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LEAVE_NTF);

				//TID_ACCOUNT		svr_acc_id;

				BYTE idSlot;
				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		};
/*
		namespace PLAYER_LIST_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_LIST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LIST_NTF)

				BYTE count;					// 플레이어 목록 개수
				PM::_BUFFER playerInfo;		// 플레이어 정보
			};

			MSGDEF_TYPE TMSG;
		};
*/
		namespace CHAT_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHAT_NTF);

				//TID_ACCOUNT		svr_acc_id;

				TID_ACCOUNT idAccount;
				PM::_STRING chatmsg;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANGE_HOST_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_HOST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_HOST_NTF);

				BYTE idHost;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace KICK_PLAYER_NTF
		{
			enum _ID { ID = MSGDEF_ID(KICK_PLAYER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(KICK_PLAYER_NTF);

				TID_ACCOUNT idAccount;
				TID_ACCOUNT idKicker;
				BYTE idSlot;
				BYTE reason;		// KICK_REASON
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANGE_SETTING_REQ
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_SETTING_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_SETTING_REQ);

				ROOM_SETTING setting;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANGE_SETTING_ANS
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_SETTING_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_SETTING_ANS);

				//TID_ACCOUNT		svr_acc_id;

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANGE_SETTING_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_SETTING_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_SETTING_NTF);

				//TID_ACCOUNT		svr_acc_id;

				ROOM_SETTING setting;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace READY_NTF
		{
			enum _ID { ID = MSGDEF_ID(READY_NTF) };

			struct DEF
			{
				MSGDEF_NAME(READY_NTF);

				//TID_ACCOUNT		svr_acc_id;

				BYTE idSlot;
				BYTE bReady;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANGE_SLOT_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_SLOT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_SLOT_NTF);

				BYTE idSlot;
				BYTE idTeam;
				BYTE newSlot;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANGE_CLASS_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_CLASS_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_CLASS_NTF);

				BYTE idSlot;
				BYTE idClass;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANGE_WEAPON_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_WEAPON_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_WEAPON_NTF);

				BYTE			idSlot;
				TID_EQUIP_SLOT	equipSlot;	// 이 군장 슬롯에 장착!
				TID_ITEM		equipWeapon;
				TID_ITEM		customItem[_CSI_MAX];
				WORD			limitPerc;
			};

			MSGDEF_TYPE TMSG;
		}

		//namespace DESTROY_NTF
		//{
		//	enum _ID { ID = MSGDEF_ID(DESTROY_NTF) };

		//	struct DEF
		//	{
		//		MSGDEF_NAME(DESTROY_NTF);

		//		
		//		DWORD		room_id;
		//	};

		//	MSGDEF_TYPE TMSG;
		//};

		namespace PLAYER_INFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_REQ);

				TID_ACCOUNT			idAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_INFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_ANS);

				BYTE				result;
				PLAYER_DISP_INFO	playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace RTT_RATING_NTF
		{
			enum _ID { ID = MSGDEF_ID(RTT_RATING_NTF) };

			struct DEF
			{
				MSGDEF_NAME(RTT_RATING_NTF);

				BYTE				idSlot;
				BYTE				rttRating;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace RTT_UPDATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(RTT_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(RTT_UPDATE_NTF);

				float rttScore;
				BYTE idSlot;
			};

			MSGDEF_TYPE TMSG;
		};


		namespace CLAIM_HOST_NTF
		{
			enum _ID { ID = MSGDEF_ID(CLAIM_HOST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CLAIM_HOST_NTF);				
			};

			MSGDEF_TYPE TMSG;
		};


		namespace SET_HOSTADDR_NTF
		{
			enum _ID { ID = MSGDEF_ID(SET_HOSTADDR_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SET_HOSTADDR_NTF);

				UDP_HOST_INFO hostInfo;
			};

			MSGDEF_TYPE TMSG;
		};


		namespace SWAP_TEAM_NTF
		{
			enum _ID { ID = MSGDEF_ID(SWAP_TEAM_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SWAP_TEAM_NTF);

				BYTE reason;
			};

			MSGDEF_TYPE TMSG;
		};


		namespace RTTT_START_REQ
		{
			enum _ID { ID = MSGDEF_ID(RTTT_START_REQ) };

			struct DEF
			{
				MSGDEF_NAME(RTTT_START_REQ)

				TID_ACCOUNT idAccount;
				UDP_HOST_INFO hostInfo;
			};

			MSGDEF_TYPE TMSG;
		};



		namespace RTTT_START_ANS
		{
			enum _ID { ID = MSGDEF_ID(RTTT_START_ANS) };

			struct DEF
			{
				MSGDEF_NAME(RTTT_START_ANS)

				UDP_HOST_INFO hostInfo;
				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		};



		namespace RTTT_START_NTF
		{
			enum _ID { ID = MSGDEF_ID(RTTT_START_NTF) };

			struct DEF
			{
				MSGDEF_NAME(RTTT_START_NTF)

				TID_ACCOUNT idAccount;
				UDP_HOST_INFO hostInfo;
			};

			MSGDEF_TYPE TMSG;
		};


		namespace QUICK_JOIN_REQ
		{
			enum _ID { ID = MSGDEF_ID(QUICK_JOIN_REQ) };

			struct DEF
			{
				MSGDEF_NAME(QUICK_JOIN_REQ);

				TID_MAP		idMap;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace QUICK_JOIN_ANS
		{
			enum _ID { ID = MSGDEF_ID(QUICK_JOIN_ANS) };

			struct DEF
			{
				MSGDEF_NAME(QUICK_JOIN_ANS);

				//TID_ACCOUNT		svr_acc_id;
				BYTE result;

				TID_ROOM	idRoom;			// 조인한 룸의 ID

				ROOM_SETTING setting;
				ROOM_STATE state;
				//ROOM_PLAYER_INFO playerInfo[MAX_ALL_PLAYER_PER_ROOM];
				PM::_BUFFER playerList;			// array of ROOM_PLAYER_INFO

				BYTE idHost;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace REPOSITION_NTF
		{
			enum _ID { ID = MSGDEF_ID(REPOSITION_NTF) };

			struct DEF
			{
				MSGDEF_NAME(REPOSITION_NTF);

				TID_ACCOUNT reposition[MAX_PLAYER_PER_ROOM];
				BYTE bKeepReady;
				BYTE reason;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace HOST_RATING_NTF
		{
			enum _ID { ID = MSGDEF_ID(HOST_RATING_NTF) };

			struct DEF
			{
				MSGDEF_NAME(HOST_RATING_NTF);

				BYTE idSlot;
				WORD rating;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ITEM_REPAIR_NTF
		{
			enum _ID { ID = MSGDEF_ID(ITEM_REPAIR_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_REPAIR_NTF);

				BYTE idSlot;
				BYTE equipSlot;	// weapon slot
				TID_ITEM idItem;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CLAN_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(CLAN_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CLAN_INFO_NTF);

				ROOM_CLAN_INFO clanInfo[2];
			};

			MSGDEF_TYPE TMSG;
		};

		namespace UPDATE_PCBANG_NTF
		{
			enum _ID { ID = MSGDEF_ID(UPDATE_PCBANG_NTF) };

			struct DEF
			{
				MSGDEF_NAME(UPDATE_PCBANG_NTF);

				TID_ACCOUNT	idAccount;		// PC방 기간이 만료된 사용자의 USN.
			};

			MSGDEF_TYPE TMSG;
		};

#pragma pack(pop)

	};	// namespace ROOM
};	// namespace PM


#undef __CAT
#undef __MC