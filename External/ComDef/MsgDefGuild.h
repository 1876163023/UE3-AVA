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

			ID_CHANNEL_CREATE_REQ, // CHS -> CLS Ŭ��ä�� ���� ��û
			ID_CHANNEL_CREATE_ANS, // CLS -> CHS Ŭ������ ����
			ID_CHANNEL_CREATE_NTF, // CLS -> CHS Ŭ��ä�� ���� ����

			ID_CHANNEL_LIST_REQ, // CHS -> CLS Ŭ��ä�� ����Ʈ�� ��û
			ID_CHANNEL_LIST_ANS, // CLS -> CHS Ŭ��ä�� ����Ʈ ����

			ID_JOIN_NTF, // CLS -> CLI
			ID_CHS_JOIN_NTF, // CLS -> CHS
			ID_LEAVE_NTF, // Ŭ�� Ż��
			ID_KICK_NTF, // Ŭ�� ����

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
			ID_LOGIN_NTF, // CLS -> CLI �α��� ����
			ID_LOGOUT_NTF, // CLS -> CLI �α׾ƿ� ����
			ID_INFO_NTF, // CLS -> CLI Ŭ��ä�� ���ν� Ŭ���̾�Ʈ�� �����ִ� ����.
			ID_WHISPER_NTF,

			ID_PLAYER_INFO_REQ,
			ID_PLAYER_INFO_ANS,

			ID_GET_CHANNEL_ADDR_REQ,
			ID_GET_CHANNEL_ADDR_ANS,

			ID_ENTRUST_MASTER_NTF, // ������ ����
			ID_GRANTGRADE_NTF, // ���� �ο�
			ID_ROOM_UPDATE_STATE_NTF,

			ID_CHS_MOVE_NTF, // ä���̵�(CHS -> CLS)

			ID_PLAYER_DIS_INFO_REQ, // CLS -> CHS ���� �����ϴ� �⺻���� ��û
			ID_PLAYER_DIS_INFO_ANS, // CHS -> CLS

			ID_CHANNEL_INFO_NTF,// CHS -> CLI ������ Ŭ��ä�� ���� ����
			// CHS -> CLS Ŭ�� ���� ��û
			ID_CHANNEL_DESTORY_NTF, // CLS -> CHS Ŭ��ä�λ��� ����

			ID_CREATE_REQ, // CLI -> CHS Ŭ�� ���� ��û
			ID_CREATE_ANS, // CHS -> CLI Ŭ�� ���� ����

			ID_LEVEL_UP_NTF,

			ID_SET_MOTD_REQ, // CLI -> CLS �������� ���� ��û
			ID_SET_MOTD_ANS, // CLS -> CLI ����

			ID_ROOM_LIST_ANS, // CLS -> CLI
			ID_ROOM_LIST_NTF,

			ID_SVR_SCORE_UPDATE_NTF, // Ŭ���� ������Ʈ ����..
			ID_SCORE_UPDATE_NTF, // Ŭ���� ������Ʈ ����..

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
				TID_ACCOUNT idAccount; // ������ ��û�� Ŭ���̾�Ʈ�� ID
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
				TID_ACCOUNT idAccount; // �����..
				TID_GUILD idGuild; // ������ ���.
				TID_SERVER idServer; // ���� ���� ���̵�..
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

				BYTE result; // RC_OK ����. �������� �������� ������ ��������.
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

				BYTE result; // ������ ��Ŷ�� RC_OK
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
				TID_ACCOUNT idAccount; // �������� To �������� From.
				WCHAR nickname[SIZE_NICKNAME+1]; // �г���
				PM::_STRING chatmsg; // ��ȭ����
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ROOM_LIST_ANS
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_ANS);

				BYTE result; // RC_OK ����. �������� �������� ������ ��������.
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

				BYTE result; // RC_OK ����. �������� �������� ������ ��������.
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

				TID_ACCOUNT idAccount; // Ŭ�������� ó���� ȸ���� USN
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

				TID_ACCOUNT idAccount; // Ŭ�������� ó���� ȸ���� USN
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
				TID_ACCOUNT idAccount; // Ŭ�������� ó���� ȸ���� USN
				BYTE serverType; // ST_CHS , ST_GCS ....
				TID_CHANNEL idChannel;
				_INT64 addr; // �ش� CHS �ּ�
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

				TID_ACCOUNT idAccount; // �������� To �������� From.
				WCHAR nickname[SIZE_NICKNAME+1]; // �г���
				PM::_STRING chatmsg; // ��ȭ����
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

				TID_GUILD idGuild; // �̱� ��� ID
				TID_GUILD destGuildID; // ����� ��� ID..
				WCHAR destGuildName[SIZE_GUILD_NAME+1];
				TID_MAP idMap; // �÷����� ��..
				BYTE teamCode; // �ڱⰡ �÷����� ��..
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
				TID_GUILD destGuildID; // ����� ��� ID..
				WCHAR destGuildName[SIZE_GUILD_NAME+1];
				TID_MAP idMap; // �÷����� ��..
				BYTE teamCode; // �ڱⰡ �÷����� ��..
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
