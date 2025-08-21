/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefAdmin.h

Description: Message definition about administration.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("ADMIN")
#define __MC MC_ADMIN

namespace PM
{
	namespace ADMIN
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,

			ID_SET_VISIBILITY_REQ,
			ID_SET_VISIBILITY_ANS,
			ID_NOTICE_NTF,
			ID_KICK_REQ,
			ID_KICK_ANS,
			ID_KICK_NTF,
			ID_CHATOFF_REQ,
			ID_CHATOFF_ANS,
			ID_CHATOFF_NTF,
			ID_CHANGE_ROOMNAME_REQ,
			ID_CHANGE_ROOMNAME_ANS,
			ID_CHANGE_ROOMNAME_NTF,
			ID_SET_TICKER_REQ,
			ID_SET_TICKER_ANS,
			ID_SET_MAINNOTICE_REQ,
			ID_SET_MAINNOTICE_ANS,
			ID_WHISPER_NTF,
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)

		namespace NOTICE_NTF
		{
			enum _ID { ID = MSGDEF_ID(NOTICE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(NOTICE_NTF);

				TID_CHANNEL idChannel;
				PM::_STRING tmsg;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_VISIBILITY_REQ
		{
			enum _ID { ID = MSGDEF_ID(SET_VISIBILITY_REQ) };

			struct DEF
			{
				MSGDEF_NAME(SET_VISIBILITY_REQ);

				BYTE bVisible;	
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_VISIBILITY_ANS
		{
			enum _ID { ID = MSGDEF_ID(SET_VISIBILITY_ANS) };

			struct DEF
			{
				MSGDEF_NAME(SET_VISIBILITY_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace KICK_REQ
		{
			enum _ID { ID = MSGDEF_ID(KICK_REQ) };

			struct DEF
			{
				MSGDEF_NAME(KICK_REQ);

				BYTE type;	// 0 = USN, 1 = ID, 2 = nickname
				TSN_USER user_sn;
				WCHAR name[SIZE_NICKNAME+1];
			};

			MSGDEF_TYPE TMSG;
		}


		namespace KICK_ANS
		{
			enum _ID { ID = MSGDEF_ID(KICK_ANS) };

			struct DEF
			{
				MSGDEF_NAME(KICK_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace KICK_NTF
		{
			enum _ID { ID = MSGDEF_ID(KICK_NTF) };

			struct DEF
			{
				MSGDEF_NAME(KICK_NTF);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace CHATOFF_REQ
		{
			enum _ID { ID = MSGDEF_ID(CHATOFF_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHATOFF_REQ);

				WCHAR nickname[SIZE_NICKNAME+1];
			};

			MSGDEF_TYPE TMSG;
		}


		namespace CHATOFF_ANS
		{
			enum _ID { ID = MSGDEF_ID(CHATOFF_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHATOFF_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace CHATOFF_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHATOFF_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHATOFF_NTF);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace CHANGE_ROOMNAME_REQ
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_ROOMNAME_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_ROOMNAME_REQ);

				TID_ROOM idRoom;
				WCHAR roomname[SIZE_ROOM_NAME+1];
			};

			MSGDEF_TYPE TMSG;
		}


		namespace CHANGE_ROOMNAME_ANS
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_ROOMNAME_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_ROOMNAME_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace CHANGE_ROOMNAME_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_ROOMNAME_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_ROOMNAME_NTF);

				TID_ROOM idRoom;
				WCHAR roomname[SIZE_ROOM_NAME+1];
			};

			MSGDEF_TYPE TMSG;
		}
		namespace SET_TICKER_REQ
		{
			enum _ID { ID = MSGDEF_ID(SET_TICKER_REQ) };

			struct DEF
			{
				MSGDEF_NAME(SET_TICKER_REQ);

				WCHAR ticker[MAX_TICKER_STR+1];
			};

			MSGDEF_TYPE TMSG;
		}


		namespace SET_TICKER_ANS
		{
			enum _ID { ID = MSGDEF_ID(SET_TICKER_ANS) };

			struct DEF
			{
				MSGDEF_NAME(SET_TICKER_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace SET_MAINNOTICE_REQ
		{
			enum _ID { ID = MSGDEF_ID(SET_MAINNOTICE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(SET_MAINNOTICE_REQ);

				WCHAR notice[SIZE_NOTICE_MSG+1];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_MAINNOTICE_ANS
		{
			enum _ID { ID = MSGDEF_ID(SET_MAINNOTICE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(SET_MAINNOTICE_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace WHISPER_NTF
		{
			enum _ID { ID = MSGDEF_ID(WHISPER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(WHISPER_NTF);

				TID_ACCOUNT		idAccount;			/// 보낼때는 To 받을때는 From.
				WCHAR			nickname[SIZE_NICKNAME+1];	// 닉네임
				PM::_STRING		chatmsg;			///	대화내용
			};

			MSGDEF_TYPE TMSG;
		};		

#pragma pack(pop)
	}	// namespace ADMIN
}	// namespace PM


#undef __CAT
#undef __MC
