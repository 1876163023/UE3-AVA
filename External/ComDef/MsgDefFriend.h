/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefFriend.h

Description: Message definition about client.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("FRIEND")
#define __MC MC_FRIEND

namespace PM
{
	namespace FRIEND
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,
			ID_BUDDY_STATE_REQ,
			ID_BUDDY_STATE_ANS,
			ID_BUDDY_STATE_UPDATE_NTF,//BDS에게 buddy state update 알림 메시지

			// BUS<-->BUS
			ID_ADD_BUDDY_REQ_NTF,
			ID_ADD_BUDDY_ANS_NTF,

			// Client <--> BUS
			//ID_BUDDY_INFO_UPDATE_NTF,	// 로그인,로그아웃,위치정보 갱신 메시지
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)		

		namespace BUDDY_STATE_REQ
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_STATE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_STATE_REQ);

				BYTE				count;							// 마지막 패킷은 RC_OK	
				PM::_BUFFER			usns;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace BUDDY_STATE_ANS
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_STATE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(BUDDY_STATE_ANS);

				BYTE				count;							// 마지막 패킷은 RC_OK	
				PM::_BUFFER			buddystate;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace BUDDY_STATE_UPDATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(BUDDY_STATE_UPDATE_NTF) };
			struct DEF
			{
				MSGDEF_NAME(BUDDY_STATE_UPDATE_NTF);
				BYTE			count;
				PM::_BUFFER		usns;
			};
			MSGDEF_TYPE	TMSG;
		};

		namespace ADD_BUDDY_REQ_NTF
		{
			enum _ID { ID = MSGDEF_ID(ADD_BUDDY_REQ_NTF)};
			struct DEF
			{
				MSGDEF_NAME(ADD_BUDDY_REQ_NTF);
				BYTE			Ftype;
				ULONG			Fromusn;// 요청을 한  사용자
				ULONG			Tousn;  // 대상이 되는 사용자
				WCHAR			Fromnick[SIZE_NICKNAME+1];
			};
			MSGDEF_TYPE	TMSG;
		};

		namespace ADD_BUDDY_ANS_NTF
		{
			enum _ID { ID = MSGDEF_ID(ADD_BUDDY_ANS_NTF)};
			struct DEF
			{
				MSGDEF_NAME(ADD_BUDDY_ANS_NTF);
				BYTE			Ftype;
				ULONG			Fromusn;// 수락을 한 사용자
				ULONG			Tousn;  // 요청을 한 사용자
				WCHAR			Fromnick[SIZE_NICKNAME+1];
			};
			MSGDEF_TYPE	TMSG;
		};
#pragma pack(pop)
	};	// namespace FRIEND
};	// namespace PM


#undef __CAT
#undef __MC
