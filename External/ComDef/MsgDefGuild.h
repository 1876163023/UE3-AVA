/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefGuild.h

Description: Message definition about Clan.

***/

#pragma once

#include "_MsgBase.h"

#define __CAT _T("GUILD")
#define __MC MC_GUILD

namespace PM
{
	namespace GUILD
	{
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,

			ID_CHANNEL_CREATE_REQ, // CHS -> CLS 클랜채널 생성 요청
			ID_CHANNEL_CREATE_ANS, // CLS -> CHS 클랜생성 응답
			ID_CHANNEL_CREATE_NTF, // CLS -> CHS 클랜채널 생성 통지

			ID_CHANNEL_LIST_REQ, // CHS -> CLS 클랜채널 리스트를 요청
			ID_CHANNEL_LIST_ANS, // CLS -> CHS 클랜채널 리스트 응답

			ID_JOIN_NTF, // CLS -> CLI
			ID_CHS_JOIN_NTF, // CLS -> CHS
			ID_LEAVE_NTF, // 클랜 탈퇴
			ID_KICK_NTF, // 클랜 강퇴

			ID_CHANNEL_LEAVE_REQ,
			ID_CHANNEL_LEAVE_ANS,
			ID_CHANNEL_LEAVE_NTF,

			ID_LOBBY_JOIN_REQ,
			ID_LOBBY_JOIN_ANS,
			ID_LOBBY_JOIN_NTF,

			ID_LOBBY_LEAVE_REQ,
			ID_LOBBY_LEAVE_ANS,
			ID_LOBBY_LEAVE_NTF,

			ID_LOBBY_PLAYERLIST_ANS,

			ID_CREATE_ROOM_NTF,

			ID_LOBBY_CHAT_NTF,
			ID_MOTD_NTF,
			ID_NOTICE_NTF,
			ID_CHAT_NTF,
			ID_MEMBER_LIST_NTF,
			ID_LOGIN_NTF, // CLS -> CLI 로그인 통지
			ID_LOGOUT_NTF, // CLS -> CLI 로그아웃 통지
			ID_INFO_NTF, // CLS -> CLI 클랜채널 조인시 클라이언트로 보내주는 정보.
			ID_WHISPER_NTF,

			ID_PLAYER_INFO_REQ,
			ID_PLAYER_INFO_ANS,

			ID_GET_CHANNEL_ADDR_REQ,
			ID_GET_CHANNEL_ADDR_ANS,

			ID_ENTRUST_MASTER_NTF, // 마스터 위임
			ID_GRANTGRADE_NTF, // 권한 부여
			ID_ROOM_UPDATE_STATE_NTF,

			ID_CHS_MOVE_NTF, // 채널이동(CHS -> CLS)

			ID_PLAYER_DIS_INFO_REQ, // CLS -> CHS 현재 접속하는 기본정보 요청
			ID_PLAYER_DIS_INFO_ANS, // CHS -> CLS

			ID_CHANNEL_INFO_NTF,// CHS -> CLI 접속할 클랜채널 정보 통지
			// CHS -> CLS 클랜 생성 요청
			ID_CHANNEL_DESTORY_NTF, // CLS -> CHS 클랜채널삭제 통지

			ID_CREATE_REQ, // CLI -> CHS 클랜 생성 요청
			ID_CREATE_ANS, // CHS -> CLI 클랜 생성 응답

			ID_LEVEL_UP_NTF,

			ID_SET_MOTD_REQ, // CLI -> CLS 공지사항 수정 요청
			ID_SET_MOTD_ANS, // CLS -> CLI 응답

			ID_ROOM_LIST_ANS, // CLS -> CLI
			ID_ROOM_LIST_NTF,

			ID_SVR_SCORE_UPDATE_NTF, // 클랜전 업데이트 통지..
			ID_SCORE_UPDATE_NTF, // 클랜전 업데이트 통지..

			ID_NICKNAME_UPDATE_NTF,
			ID_CLANNAME_UPDATE_NTF,

			ID_PLAYER_LOCATION_REQ,
			ID_PLAYER_LOCATION_ANS,
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions
#pragma pack(push)
#pragma pack(1)

#ifdef _AVA_SERVER

		namespace CHANNEL_DESTORY_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANNEL_DESTORY_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_DESTORY_NTF);

				TID_GUILD idGuild;
				WCHAR strGuildID[SIZE_GUILD_ID+1];
			};

			MSGDEF_TYPE TMSG;
		}
#endif

		namespace CHANNEL_INFO_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_INFO_NTF);

				TID_GUILD idGuild;
				_INT64 addr;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNEL_CREATE_REQ
		{
			enum _ID{ ID = MSGDEF_ID( CHANNEL_CREATE_REQ ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_CREATE_REQ);

				TID_GUILD idGuild;
				TID_ACCOUNT idAccount; // 생성을 요청한 클라이언트의 ID
				WCHAR strGuildID[SIZE_GUILD_ID+1];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANNEL_CREATE_ANS
		{
			enum _ID{ ID = MSGDEF_ID( CHANNEL_CREATE_ANS ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_CREATE_ANS);

				BYTE result;
				TID_ACCOUNT idAccount; // 만든놈..
				TID_GUILD idGuild; // 생성된 길드.
				TID_SERVER idServer; // 만든 서버 아이디..
				_INT64 addr; // GateListener Address..
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANNEL_CREATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID( CHANNEL_CREATE_NTF ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_CREATE_NTF);

				TID_GUILD idGuild;
				TID_SERVER idServer;
				WCHAR strGuildID[SIZE_GUILD_ID+1];
				_INT64 addr; // GateListener Address..
			};

			MSGDEF_TYPE TMSG;
		}

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
				TID_SERVER idServer;
				PM::_BUFFER channelInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace LOBBY_PLAYERLIST_ANS
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_PLAYERLIST_ANS) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_PLAYERLIST_ANS);

				BYTE result; // 마지막 패킷은 RC_OK
				PM::_BUFFER playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANNEL_LEAVE_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LEAVE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LEAVE_REQ);

				TID_GUILD idGuild;
				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANNEL_LEAVE_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LEAVE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LEAVE_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_JOIN_REQ
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_JOIN_REQ) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_JOIN_REQ);

				TID_GUILD idGuild;
				TID_ACCOUNT idAccount;
				_INT64 addr;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_JOIN_ANS
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_JOIN_ANS) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_JOIN_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_JOIN_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_JOIN_NTF);

				LOBBY_PLAYER_INFO playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_LEAVE_REQ
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_LEAVE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_LEAVE_REQ);

				TID_GUILD idGuild;
				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_LEAVE_ANS
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_LEAVE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_LEAVE_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_LEAVE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_LEAVE_NTF);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOBBY_CHAT_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_CHAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_CHAT_NTF);

				TID_GUILD idGuild;
				TID_ACCOUNT idAccount; // 보낼때는 To 받을때는 From.
				WCHAR nickname[SIZE_NICKNAME+1]; // 닉네임
				PM::_STRING chatmsg; // 대화내용
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ROOM_LIST_ANS
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_ANS);

				BYTE result; // RC_OK 성공. 나머지는 서버에서 연결을 끊을것임.
				PM::_BUFFER roomInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_LIST_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_NTF);

				BYTE result; // RC_OK 성공. 나머지는 서버에서 연결을 끊을것임.
				PM::_BUFFER roomInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CREATE_ROOM_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CREATE_ROOM_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_ROOM_NTF);

				ROOM_INFO roomInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace MOTD_NTF
		{
			enum _ID{ ID = MSGDEF_ID(MOTD_NTF) };

			struct DEF
			{
				MSGDEF_NAME(MOTD_NTF);

				PM::_STRING motd;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace NOTICE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(NOTICE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(NOTICE_NTF);

				TID_ACCOUNT idAccount;
				PM::_STRING chatmsg;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHAT_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHAT_NTF);

				TID_ACCOUNT idAccount;
				PM::_STRING chatmsg;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace JOIN_NTF
		{
			enum _ID{ ID = MSGDEF_ID(JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(JOIN_NTF);

				GUILD_PLAYER_INFO playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHS_JOIN_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHS_JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHS_JOIN_NTF);

				TID_ACCOUNT idAccount;
				PLAYER_GUILD_INFO playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LEAVE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LEAVE_NTF);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace KICK_NTF
		{
			enum _ID{ ID = MSGDEF_ID(KICK_NTF) };

			struct DEF
			{
				MSGDEF_NAME(KICK_NTF);

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ENTRUST_MASTER_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ENTRUST_MASTER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ENTRUST_MASTER_NTF);

				TID_GUILD idGuild;
				TID_ACCOUNT idFromAccount;
				TID_ACCOUNT idToAccount;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GRANTGRADE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(GRANTGRADE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(GRANTGRADE_NTF);

				TID_GUILD idGuild;
				TID_ACCOUNT idFromAccount;
				TID_ACCOUNT idToAccount;
				BYTE byRank;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOGIN_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOGIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOGIN_NTF);

				TID_ACCOUNT idAccount; // 클랜가입을 처리한 회원의 USN
				TID_CHANNEL idChannel;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LOGOUT_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOGOUT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOGOUT_NTF);

				TID_ACCOUNT idAccount; // 클랜가입을 처리한 회원의 USN
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHS_MOVE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHS_MOVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHS_MOVE_NTF);

				TID_GUILD idGuild;
				TID_ACCOUNT idAccount; // 클랜가입을 처리한 회원의 USN
				BYTE serverType; // ST_CHS , ST_GCS ....
				TID_CHANNEL idChannel;
				_INT64 addr; // 해당 CHS 주소
			};

			MSGDEF_TYPE TMSG;
		}

		namespace INFO_NTF
		{
			enum _ID{ ID = MSGDEF_ID(INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(INFO_NTF);

				GUILD_INFO info;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace WHISPER_NTF
		{
			enum _ID{ ID = MSGDEF_ID(WHISPER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(WHISPER_NTF);

				TID_ACCOUNT idAccount; // 보낼때는 To 받을때는 From.
				WCHAR nickname[SIZE_NICKNAME+1]; // 닉네임
				PM::_STRING chatmsg; // 대화내용
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_INFO_REQ
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_REQ);

				TID_ACCOUNT idAccount;
				TID_GUILD idGuild;
			};

			MSGDEF_TYPE TMSG;
		}

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

		namespace PLAYER_DIS_INFO_REQ
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_DIS_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_DIS_INFO_REQ);

				TID_ACCOUNT idAccount;
				TID_GUILD idGuild;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_DIS_INFO_ANS
		{
			enum _ID{ ID = MSGDEF_ID(PLAYER_DIS_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_DIS_INFO_ANS);

				BYTE result;
				TID_ACCOUNT idAccount;
				TID_GUILD idGuild;
				TID_CHANNEL	idChannel;
				PLAYER_DISP_INFO playerInfo;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GET_CHANNEL_ADDR_REQ
		{
			enum _ID{ ID = MSGDEF_ID(GET_CHANNEL_ADDR_REQ) };

			struct DEF
			{
				MSGDEF_NAME(GET_CHANNEL_ADDR_REQ);
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GET_CHANNEL_ADDR_ANS
		{
			enum _ID{ ID = MSGDEF_ID(GET_CHANNEL_ADDR_ANS) };

			struct DEF
			{
				MSGDEF_NAME(GET_CHANNEL_ADDR_ANS);

				_INT64 addr;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CREATE_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CREATE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_REQ);

				WCHAR name[SIZE_GUILD_NAME+1];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CREATE_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CREATE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_ANS);

				BYTE result;
				TID_GUILD idGuild;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LEVEL_UP_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LEVEL_UP_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LEVEL_UP_NTF);

				BYTE level;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_MOTD_REQ
		{
			enum _ID{ ID = MSGDEF_ID(SET_MOTD_REQ) };

			struct DEF
			{
				MSGDEF_NAME(SET_MOTD_REQ);

				PM::_STRING motd;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_MOTD_ANS
		{
			enum _ID{ ID = MSGDEF_ID(SET_MOTD_ANS) };

			struct DEF
			{
				MSGDEF_NAME(SET_MOTD_ANS);

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace MEMBER_LIST_NTF
		{
			enum _ID{ ID = MSGDEF_ID(MEMBER_LIST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(MEMBER_LIST_NTF);

				_BUFFER members; // list of GUILD_PLAYER_INFO
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SVR_SCORE_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(SVR_SCORE_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_SCORE_UPDATE_NTF);

				TID_GUILD idGuild; // 이긴 길드 ID
				TID_GUILD destGuildID; // 상대편 길드 ID..
				WCHAR destGuildName[SIZE_GUILD_NAME+1];
				TID_MAP idMap; // 플레이한 맵..
				BYTE teamCode; // 자기가 플레이한 팀..
				BYTE winFlag; // 0 = lose, 1 = win..
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SCORE_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(SCORE_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SCORE_UPDATE_NTF);

				DWORD totalWinCnt;
				DWORD totalLoseCnt;
				TID_GUILD destGuildID; // 상대편 길드 ID..
				WCHAR destGuildName[SIZE_GUILD_NAME+1];
				TID_MAP idMap; // 플레이한 맵..
				BYTE teamCode; // 자기가 플레이한 팀..
				BYTE winFlag; // 0 = lose, 1 = win..
			};

			MSGDEF_TYPE TMSG;
		}

		namespace NICKNAME_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(NICKNAME_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(NICKNAME_UPDATE_NTF);

				TID_ACCOUNT idAccount;
				WCHAR nickname[SIZE_NICKNAME+1];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CLANNAME_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CLANNAME_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CLANNAME_UPDATE_NTF);

				WCHAR guildName[SIZE_GUILD_NAME+1];
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
			};

			MSGDEF_TYPE TMSG;
		}


#pragma pack(pop)
	}; // namespace GUILD
}; // namespace PM

#undef __CAT
#undef __MC
