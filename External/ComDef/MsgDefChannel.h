/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefChannel.h

Description: Message definition about Channel.

***/

#pragma once

#include "_MsgBase.h"
//#include "Item.h"

#define __CAT _T("CHANNEL")
#define __MC MC_CHANNEL

namespace PM
{
	namespace CHANNEL
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,

			ID_CHANNEL_UPDATE_NTF, /// CHS 끼리 채널 정보가 업데이트 될때 마다..

			ID_CHANNEL_LIST_REQ, /// 여러개 채널일때 채널 정보 전송.
			ID_CHANNEL_LIST_ANS, /// 여러개 채널일때 채널 정보 전송.

			ID_CHANNEL_JOIN_REQ, /// 역시 채널이 여러개 일때 특정 채널에 접속할때
			ID_CHANNEL_JOIN_ANS,

			ID_CHANNEL_LEAVE_REQ, /// 유저가 채널을 벋어남.
			ID_CHANNEL_LEAVE_ANS, /// 유저가 채널을 벋어남.

			ID_PLAYER_LIST_REQ, /// 대기실의 유저 리스트.
			ID_PLAYER_LIST_ANS,
			ID_PLAYER_LIST_NTF,
			ID_PLAYER_INFO_REQ,
			ID_PLAYER_INFO_ANS,

			ID_ROOM_LIST_REQ,
			ID_ROOM_LIST_ANS,
			ID_ROOM_LIST_NTF, /// 채널에 속한 방목록.
			ID_ROOM_LIST_VIEW_NTF, /// 클라이언트에서 보는 방 번호들을 서버로 전송.
			ID_ROOM_LIST_VIEW_ADD_NTF, /// 방이 새로생겼거나. 없어졌을때. 추가로 추가될 방..
			ID_ROOM_INFO_REQ,
			ID_ROOM_INFO_ANS,

			ID_SET_FILTER_REQ,
			ID_SET_FILTER_ANS,

			ID_LOBBY_JOIN_NTF, /// 대기실의 유저가 들어올때 서버에서 보내주는 통지.
			ID_LOBBY_LEAVE_NTF, /// 대기실의 유저가 나갈때 서버에서 보내주는 통지.

			ID_LOBBY_CHAT_NTF, /// 대기실에서의 채팅 메세지

			ID_ROOM_UPDATE_STATE_NTF, /// 방 정보가 업데이트 되었을때.
			ID_ROOM_UPDATE_SETTING_NTF, /// 방 정보가 업데이트 되었을때.
			ID_ROOM_CREATE_NTF, /// 방 생성되었을때 서버에서 보내주는 통지.
			ID_ROOM_DELETE_NTF, /// 방 삭제되었을때 서버에서 보내주는 통지.

			ID_PLAYER_LIST_DIFF_NTF,
			ID_ROOM_LIST_DIFF_NTF, /// 채널에 속한 방목록.

			ID_WHISPER_NTF,

			ID_CHANNELLIST_UPDATE_NTF,

			ID_CHANNEL_DESC_NTF,

			ID_PLAYER_LOCATION_REQ,
			ID_PLAYER_LOCATION_ANS,

			ID_FOLLOW_PLAYER_REQ,
			ID_FOLLOW_PLAYER_ANS,
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)

		// CHS 끼리 채널 정보 업데이트..
		namespace CHANNEL_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_UPDATE_NTF)

				// 채널 정보..
				TID_SERVER idServer;
				TID_CHANNEL idChannel;
				int numPlayer;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNELLIST_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHANNELLIST_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNELLIST_UPDATE_NTF)

				// 채널 정보..
				TID_SERVER idServer;
				CHANNEL_INFO channelInfo[MAX_CHANNEL_PER_SERVER];
			};

			MSGDEF_TYPE TMSG;
		};

		// CHS에 접속한수 CL이 서버로 채널 목록 및 상태를 요청하는 패킷.
		namespace CHANNEL_LIST_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LIST_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LIST_REQ);
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNEL_LIST_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LIST_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LIST_ANS);

				BYTE result; // RC_OK 성공. 나머지는 서버에서 연결을 끊을것임.

				PM::_BUFFER channelInfo;

				//WORD count; /// 채널 개수
				//CHANNEL_INFO channelInfo[MAX_CHANNEL_PER_GAME]; /// 채널 정보
				TID_SERVER idServer; // 해당 채널 서버의 ID
			};

			MSGDEF_TYPE TMSG;
		};

		// CL이 특정 채널로 조인할때 쓰이는 메세지.
		namespace CHANNEL_JOIN_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_JOIN_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_JOIN_REQ);

				TID_CHANNEL idChannel; /// 조인하려는 체널의 번호
				BYTE bFollowing;		// 따라가기 하고 있는 중
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNEL_JOIN_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_JOIN_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_JOIN_ANS);

				BYTE result; /// 결과 RC_OK 는성공. RC_CHANNEL_PLAYER_FULL 는 채널이 꽈악 참. 나머지 실패.
				_INT64 addr; /// 새로 접속할 서버의 Address;
			};

			MSGDEF_TYPE TMSG;
		};

		// CL이 특정 채널을 떠날때 쓰이는 메세지
		namespace CHANNEL_LEAVE_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LEAVE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LEAVE_REQ);
			};

			MSGDEF_TYPE TMSG;
		}

		// CL이 특정 채널을 떠날때 쓰이는 메세지
		namespace CHANNEL_LEAVE_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LEAVE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LEAVE_ANS);

				BYTE result; /// 결과 RC_OK 는성공. RC_CHANNEL_PLAYER_FULL 는 채널이 꽈악 참. 나머지 실패.
			};

			MSGDEF_TYPE TMSG;
		}

		// 룸 리스트.. 요청
		namespace ROOM_LIST_REQ
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_REQ) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_REQ);

				//BYTE direction; // refresh,get = 0, prev = 1 , next = 2 // 룸 페이징 할때만 쓰임.
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_LIST_ANS
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_ANS);

				BYTE result;

				USHORT count; // 룸 정보의 수. (단, 0 <= count <= NUM_ROOMS_PER_PAGE)

				DWORD diffIndex;
			};

			MSGDEF_TYPE TMSG;
		};

		// 룸 정보..
		namespace ROOM_LIST_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_NTF);

				BYTE result; // 마지막 패킷은 RC_OK

				PM::_BUFFER roomInfo;

				//DWORD diffIndex;

				//USHORT count; // 룸 정보의 수. (단, 0 <= count <= NUM_ROOMS_PER_PAGE)
				//ROOM_INFO roomInfo[MAX_ROOM_PER_PAGE]; // 한 페이지에 해당하는 룸 리스트
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_LIST_VIEW_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_VIEW_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_VIEW_NTF);

				//TID_ACCOUNT svr_acc_id;

				BYTE count; // 룸 정보의 수. (단, 0 <= count <= NUM_ROOMS_PER_PAGE)
				TID_ROOM roomView[MAX_ROOM_PER_PAGE]; // 한 페이지에 해당하는 룸 리스트
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_LIST_VIEW_ADD_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_VIEW_ADD_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_VIEW_ADD_NTF);

				TID_ROOM room_id; // 뷰에 추가될 방 번호..!!!
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_INFO_REQ
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_INFO_REQ);

				TID_ROOM room_id;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_INFO_ANS
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_INFO_ANS);

				BYTE result;
				TID_ROOM room_id;
				BYTE roundCount;
				WCHAR playerList[MAX_PLAYER_PER_ROOM][SIZE_NICKNAME+1];
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_LIST_REQ
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_LIST_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LIST_REQ);
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_LIST_ANS
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_LIST_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LIST_ANS);

				BYTE result;
				USHORT count;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_LIST_NTF
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_LIST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LIST_NTF);

				BYTE result; // 마지막 패킷은 RC_OK

				PM::_BUFFER playerInfo;

				//USHORT count; /// 유저개수
				//LOBBY_PLAYER_INFO playerInfo[MAX_PLAYER_PER_PAGE]; /// 대기실의 유저정보
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_INFO_REQ
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_REQ);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_INFO_ANS
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_ANS);

				BYTE result;
				PLAYER_DISP_INFO playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_LOCATION_REQ
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_LOCATION_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LOCATION_REQ);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_LOCATION_ANS
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_LOCATION_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LOCATION_ANS);

				BYTE result;
				TID_ACCOUNT idAccount;
				TID_CHANNEL idChannel;
				TID_ROOM idRoom;		// 상대방이 채널 목록을 보고 있으면 USHRT_MAX
			};

			MSGDEF_TYPE TMSG;
		}

		namespace FOLLOW_PLAYER_REQ
		{
			enum _ID{ ID = MSGDEF_ID(FOLLOW_PLAYER_REQ) };

			struct DEF
			{
				MSGDEF_NAME(FOLLOW_PLAYER_REQ);

				TID_ACCOUNT idTarget;
				TID_ACCOUNT idSender;
				TID_GUILD idGuild;
				BYTE level;
				BYTE pcBangFlag;
				float sdRatio;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace FOLLOW_PLAYER_ANS
		{
			enum _ID{ ID = MSGDEF_ID(FOLLOW_PLAYER_ANS) };

			struct DEF
			{
				MSGDEF_NAME(FOLLOW_PLAYER_ANS);

				BYTE result;		// 상대방이 채널 목록을 보고 있으면 RC_FAIL
				TID_ACCOUNT idAccount;
				TID_CHANNEL idChannel;
				TID_ROOM idRoom;
				BYTE bPassword;
				WCHAR roomName[SIZE_ROOM_NAME+1];
			};

			MSGDEF_TYPE TMSG;
		}

		// 방정보가 업데이트 돼었을때..
		namespace ROOM_UPDATE_STATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_UPDATE_STATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_UPDATE_STATE_NTF);

				//TID_ACCOUNT svr_acc_id;
				TID_ROOM idRoom;
				ROOM_STATE state; /// 업데이트 돼는 방의 정보
			};

			MSGDEF_TYPE TMSG;
		};

		// 방정보가 업데이트 돼었을때..
		namespace ROOM_UPDATE_SETTING_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_UPDATE_SETTING_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_UPDATE_SETTING_NTF);

				//TID_ACCOUNT svr_acc_id;
				TID_ROOM idRoom;
				ROOM_SETTING setting; /// 업데이트 돼는 방의 정보
			};

			MSGDEF_TYPE TMSG;
		};

		// 새로운 방 생성.
		namespace ROOM_CREATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_CREATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_CREATE_NTF);

				ROOM_INFO roomInfo; /// 새로 생성돼는 방의 정보
			};

			MSGDEF_TYPE TMSG;
		};

		// 방삭제..
		namespace ROOM_DELETE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_DELETE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_DELETE_NTF);

				TID_ROOM idRoom; /// 삭제되는 방의 정보
			};

			MSGDEF_TYPE TMSG;
		};

		// 내가 로비에 있을때.. 서버로 부터 같은 로비에 다른 대기자가 들어왔을때.
		namespace LOBBY_JOIN_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_JOIN_NTF);

				LOBBY_PLAYER_INFO playerInfo; /// 대기실에 들어온 유저정보..
			};

			MSGDEF_TYPE TMSG;
		};

		// 로비에서 누군가가 나갔을때..
		namespace LOBBY_LEAVE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_LEAVE_NTF);

				TID_ACCOUNT idAccount; /// 대기실에서 나간 유저
			};

			MSGDEF_TYPE TMSG;
		};

		// 로비내에서 채팅.
		namespace LOBBY_CHAT_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_CHAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_CHAT_NTF);

				TID_ACCOUNT idAccount;/// 보낼때는 To 받을때는 From.
				//WCHAR nickname[SIZE_NICKNAME+1]; // 닉네임
				PM::_STRING chatmsg; /// 대화내용
			};

			MSGDEF_TYPE TMSG;
		};

		// 방목록요청시.. 전체보기냐.. 대기자만 보기냐..
		namespace SET_FILTER_REQ
		{
			enum _ID{ ID = MSGDEF_ID(SET_FILTER_REQ) };

			struct DEF
			{
				MSGDEF_NAME(SET_FILTER_REQ);

				BYTE filter;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace SET_FILTER_ANS
		{
			enum _ID{ ID = MSGDEF_ID(SET_FILTER_ANS) };

			struct DEF
			{
				MSGDEF_NAME(SET_FILTER_ANS);

				BYTE result;
				USHORT count;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_LIST_DIFF_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_DIFF_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_DIFF_NTF);

				DWORD diffIndex;
				PM::_BUFFER diff;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_LIST_DIFF_NTF
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_LIST_DIFF_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_LIST_DIFF_NTF);

				PM::_BUFFER diffIn;
				PM::_BUFFER diffOut;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace WHISPER_NTF
		{
			enum _ID{ ID = MSGDEF_ID(WHISPER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(WHISPER_NTF);

				TID_ACCOUNT idAccount; /// 보낼때는 To 받을때는 From.
				WCHAR nickname[SIZE_NICKNAME+1]; // 닉네임
				PM::_STRING chatmsg; /// 대화내용
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNEL_DESC_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_DESC_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_DESC_NTF);

				PM::_BUFFER channelDesc;	// CHANNEL_DESC 리스트
				PM::_STRING nameList;		// 채널의 이름 리스트; channelDesc와 1대1 매치. "\n"로 구분함. 빈 이름의 경우 기본 이름을 사용함.
				TID_SERVER idServer;		// 해당 채널 서버의 ID
			};

			MSGDEF_TYPE TMSG;
		};



#pragma pack(pop)
	}; // namespace CHS
}; // namespace PM


#undef __CAT
#undef __MC
