/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefFriend.h

Description: Message definition about client.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("BUDDYODL")
#define __MC MC_BUDDYODL

namespace PM
{
	namespace BUDDYODL
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE					= 0,
			ID_BUDDY_ADD_REQ		=100,
			ID_BUDDY_ADD_ANS		= 101,
			ID_BUDDY_DELETE_NTF		= 102,
			ID_BUDDY_INVITE_NTF		= 103, //DAT_MsgGameInviteNtf's ID
			ID_CLANINVITEREQ		= 104, //DAT_MsgClanInviteReq's ID
			ID_CLANINVITEANS		= 105, //DAT_MsgClanInviteAns's ID
			ID_CLANJOINNTF		= 106, //DAT_MsgClanJoinNtf's ID
			ID_PRIVATE_CHAT		= 107, //DAT_MsgPrivateChatData's ID
			ID_LOCATIONINFO_REQ	= 108, //DAT_MsgLocationInfoReq's ID
			ID_LOCATIONINFO_ANS	= 109, //DAT_MsgLocationInfoAns's ID
			ID_BUDDYINFO_REQ		= 110, //DAT_MsgBuddyInfoReq's ID
			ID_BUDDYINFO_ANS		= 111, //DAT_MsgBuddyInfoAns's ID
			ID_BUDDYINFO_END_NTF	= 112, //DAT_MsgBuddyInfoEndNtf's ID
			ID_BUDDY_ALARM_NTF		= 113, //DAT_MsgBuddyAlarmNtf's ID
			ID_AVA_BUDDY_INFO_REQ	= 114, //DAT_MsgPairBuddyinfoReq's ID
			ID_AVA_BUDDY_INFO_ANS	= 115, //DAT_MsgPairBuddyinfoAns's ID
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)		

		namespace BUDDY_ADD_REQ
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_ADD_REQ) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_ADD_REQ);

			};

			MSGDEF_TYPE TMSG;
		};

		namespace BUDDY_ADD_ANS
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_ADD_ANS) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_ADD_ANS);

				BYTE				count;							// 마지막 패킷은 RC_OK	
				PM::_BUFFER			buddystate;
			};

			MSGDEF_TYPE TMSG;
		};

		//====================================
		namespace BUDDY_DELETE_NTF
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_DELETE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_DELETE_NTF);
			};

			MSGDEF_TYPE TMSG;
		};

		//====================================
		namespace BUDDY_INVITE_NTF
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_INVITE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_INVITE_NTF);
			};

			MSGDEF_TYPE TMSG;
		};


		//====================================
		namespace PRIVATE_CHAT
		{
			enum _ID { ID = MSGDEF_ID(PRIVATE_CHAT) };

			struct DEF
			{
				MSGDEF_NAME(PRIVATE_CHAT);
			};

			MSGDEF_TYPE TMSG;
		};
		//====================================
		namespace LOCATIONINFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(LOCATIONINFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(LOCATIONINFO_REQ);
			};

			MSGDEF_TYPE TMSG;
		};
		//====================================
		namespace LOCATIONINFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(LOCATIONINFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(LOCATIONINFO_ANS);
			};

			MSGDEF_TYPE TMSG;
		};
		//====================================
		namespace BUDDYINFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(BUDDYINFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(BUDDYINFO_REQ);
			};

			MSGDEF_TYPE TMSG;
		};
		//====================================
		namespace BUDDYINFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(BUDDYINFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(BUDDYINFO_ANS);
			};

			MSGDEF_TYPE TMSG;
		};


		//====================================
		namespace BUDDYINFO_END_NTF
		{
			enum _ID { ID = MSGDEF_ID(BUDDYINFO_END_NTF) };

			struct DEF
			{
				MSGDEF_NAME(BUDDYINFO_END_NTF);
			};

			MSGDEF_TYPE TMSG;
		};

		//====================================
		namespace BUDDY_ALARM_NTF
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_ALARM_NTF) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_ALARM_NTF);
			};

			MSGDEF_TYPE TMSG;
		};

		//====================================
		namespace AVA_BUDDY_INFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(AVA_BUDDY_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(AVA_BUDDY_INFO_REQ);
			};

			MSGDEF_TYPE TMSG;
		};

		namespace AVA_BUDDY_INFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(AVA_BUDDY_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(AVA_BUDDY_INFO_ANS);
			};

			MSGDEF_TYPE TMSG;
		};

#pragma pack(pop)
	};	// namespace FRIEND
};	// namespace PM


#undef __CAT
#undef __MC
