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

			ID_CHANNEL_UPDATE_NTF, /// CHS ���� ä�� ������ ������Ʈ �ɶ� ����..

			ID_CHANNEL_LIST_REQ, /// ������ ä���϶� ä�� ���� ����.
			ID_CHANNEL_LIST_ANS, /// ������ ä���϶� ä�� ���� ����.

			ID_CHANNEL_JOIN_REQ, /// ���� ä���� ������ �϶� Ư�� ä�ο� �����Ҷ�
			ID_CHANNEL_JOIN_ANS,

			ID_CHANNEL_LEAVE_REQ, /// ������ ä���� ���.
			ID_CHANNEL_LEAVE_ANS, /// ������ ä���� ���.

			ID_PLAYER_LIST_REQ, /// ������ ���� ����Ʈ.
			ID_PLAYER_LIST_ANS,
			ID_PLAYER_LIST_NTF,
			ID_PLAYER_INFO_REQ,
			ID_PLAYER_INFO_ANS,

			ID_ROOM_LIST_REQ,
			ID_ROOM_LIST_ANS,
			ID_ROOM_LIST_NTF, /// ä�ο� ���� ����.
			ID_ROOM_LIST_VIEW_NTF, /// Ŭ���̾�Ʈ���� ���� �� ��ȣ���� ������ ����.
			ID_ROOM_LIST_VIEW_ADD_NTF, /// ���� ���λ���ų�. ����������. �߰��� �߰��� ��..
			ID_ROOM_INFO_REQ,
			ID_ROOM_INFO_ANS,

			ID_SET_FILTER_REQ,
			ID_SET_FILTER_ANS,

			ID_LOBBY_JOIN_NTF, /// ������ ������ ���ö� �������� �����ִ� ����.
			ID_LOBBY_LEAVE_NTF, /// ������ ������ ������ �������� �����ִ� ����.

			ID_LOBBY_CHAT_NTF, /// ���ǿ����� ä�� �޼���

			ID_ROOM_UPDATE_STATE_NTF, /// �� ������ ������Ʈ �Ǿ�����.
			ID_ROOM_UPDATE_SETTING_NTF, /// �� ������ ������Ʈ �Ǿ�����.
			ID_ROOM_CREATE_NTF, /// �� �����Ǿ����� �������� �����ִ� ����.
			ID_ROOM_DELETE_NTF, /// �� �����Ǿ����� �������� �����ִ� ����.

			ID_PLAYER_LIST_DIFF_NTF,
			ID_ROOM_LIST_DIFF_NTF, /// ä�ο� ���� ����.

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

		// CHS ���� ä�� ���� ������Ʈ..
		namespace CHANNEL_UPDATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_UPDATE_NTF)

				// ä�� ����..
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

				// ä�� ����..
				TID_SERVER idServer;
				CHANNEL_INFO channelInfo[MAX_CHANNEL_PER_SERVER];
			};

			MSGDEF_TYPE TMSG;
		};

		// CHS�� �����Ѽ� CL�� ������ ä�� ��� �� ���¸� ��û�ϴ� ��Ŷ.
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

				PM::_BUFFER channelInfo;

				//WORD count; /// ä�� ����
				//CHANNEL_INFO channelInfo[MAX_CHANNEL_PER_GAME]; /// ä�� ����
				TID_SERVER idServer; // �ش� ä�� ������ ID
			};

			MSGDEF_TYPE TMSG;
		};

		// CL�� Ư�� ä�η� �����Ҷ� ���̴� �޼���.
		namespace CHANNEL_JOIN_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_JOIN_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_JOIN_REQ);

				TID_CHANNEL idChannel; /// �����Ϸ��� ü���� ��ȣ
				BYTE bFollowing;		// ���󰡱� �ϰ� �ִ� ��
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNEL_JOIN_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_JOIN_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_JOIN_ANS);

				BYTE result; /// ��� RC_OK �¼���. RC_CHANNEL_PLAYER_FULL �� ä���� �ʾ� ��. ������ ����.
				_INT64 addr; /// ���� ������ ������ Address;
			};

			MSGDEF_TYPE TMSG;
		};

		// CL�� Ư�� ä���� ������ ���̴� �޼���
		namespace CHANNEL_LEAVE_REQ
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LEAVE_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LEAVE_REQ);
			};

			MSGDEF_TYPE TMSG;
		}

		// CL�� Ư�� ä���� ������ ���̴� �޼���
		namespace CHANNEL_LEAVE_ANS
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_LEAVE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_LEAVE_ANS);

				BYTE result; /// ��� RC_OK �¼���. RC_CHANNEL_PLAYER_FULL �� ä���� �ʾ� ��. ������ ����.
			};

			MSGDEF_TYPE TMSG;
		}

		// �� ����Ʈ.. ��û
		namespace ROOM_LIST_REQ
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_REQ) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_REQ);

				//BYTE direction; // refresh,get = 0, prev = 1 , next = 2 // �� ����¡ �Ҷ��� ����.
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

				USHORT count; // �� ������ ��. (��, 0 <= count <= NUM_ROOMS_PER_PAGE)

				DWORD diffIndex;
			};

			MSGDEF_TYPE TMSG;
		};

		// �� ����..
		namespace ROOM_LIST_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_NTF);

				BYTE result; // ������ ��Ŷ�� RC_OK

				PM::_BUFFER roomInfo;

				//DWORD diffIndex;

				//USHORT count; // �� ������ ��. (��, 0 <= count <= NUM_ROOMS_PER_PAGE)
				//ROOM_INFO roomInfo[MAX_ROOM_PER_PAGE]; // �� �������� �ش��ϴ� �� ����Ʈ
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

				BYTE count; // �� ������ ��. (��, 0 <= count <= NUM_ROOMS_PER_PAGE)
				TID_ROOM roomView[MAX_ROOM_PER_PAGE]; // �� �������� �ش��ϴ� �� ����Ʈ
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ROOM_LIST_VIEW_ADD_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_LIST_VIEW_ADD_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_LIST_VIEW_ADD_NTF);

				TID_ROOM room_id; // �信 �߰��� �� ��ȣ..!!!
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

				BYTE result; // ������ ��Ŷ�� RC_OK

				PM::_BUFFER playerInfo;

				//USHORT count; /// ��������
				//LOBBY_PLAYER_INFO playerInfo[MAX_PLAYER_PER_PAGE]; /// ������ ��������
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
				TID_ROOM idRoom;		// ������ ä�� ����� ���� ������ USHRT_MAX
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

				BYTE result;		// ������ ä�� ����� ���� ������ RC_FAIL
				TID_ACCOUNT idAccount;
				TID_CHANNEL idChannel;
				TID_ROOM idRoom;
				BYTE bPassword;
				WCHAR roomName[SIZE_ROOM_NAME+1];
			};

			MSGDEF_TYPE TMSG;
		}

		// �������� ������Ʈ �ž�����..
		namespace ROOM_UPDATE_STATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_UPDATE_STATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_UPDATE_STATE_NTF);

				//TID_ACCOUNT svr_acc_id;
				TID_ROOM idRoom;
				ROOM_STATE state; /// ������Ʈ �Ŵ� ���� ����
			};

			MSGDEF_TYPE TMSG;
		};

		// �������� ������Ʈ �ž�����..
		namespace ROOM_UPDATE_SETTING_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_UPDATE_SETTING_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_UPDATE_SETTING_NTF);

				//TID_ACCOUNT svr_acc_id;
				TID_ROOM idRoom;
				ROOM_SETTING setting; /// ������Ʈ �Ŵ� ���� ����
			};

			MSGDEF_TYPE TMSG;
		};

		// ���ο� �� ����.
		namespace ROOM_CREATE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_CREATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_CREATE_NTF);

				ROOM_INFO roomInfo; /// ���� �����Ŵ� ���� ����
			};

			MSGDEF_TYPE TMSG;
		};

		// �����..
		namespace ROOM_DELETE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(ROOM_DELETE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ROOM_DELETE_NTF);

				TID_ROOM idRoom; /// �����Ǵ� ���� ����
			};

			MSGDEF_TYPE TMSG;
		};

		// ���� �κ� ������.. ������ ���� ���� �κ� �ٸ� ����ڰ� ��������.
		namespace LOBBY_JOIN_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_JOIN_NTF);

				LOBBY_PLAYER_INFO playerInfo; /// ���ǿ� ���� ��������..
			};

			MSGDEF_TYPE TMSG;
		};

		// �κ񿡼� �������� ��������..
		namespace LOBBY_LEAVE_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_LEAVE_NTF);

				TID_ACCOUNT idAccount; /// ���ǿ��� ���� ����
			};

			MSGDEF_TYPE TMSG;
		};

		// �κ񳻿��� ä��.
		namespace LOBBY_CHAT_NTF
		{
			enum _ID{ ID = MSGDEF_ID(LOBBY_CHAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOBBY_CHAT_NTF);

				TID_ACCOUNT idAccount;/// �������� To �������� From.
				//WCHAR nickname[SIZE_NICKNAME+1]; // �г���
				PM::_STRING chatmsg; /// ��ȭ����
			};

			MSGDEF_TYPE TMSG;
		};

		// ���Ͽ�û��.. ��ü�����.. ����ڸ� �����..
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

				TID_ACCOUNT idAccount; /// �������� To �������� From.
				WCHAR nickname[SIZE_NICKNAME+1]; // �г���
				PM::_STRING chatmsg; /// ��ȭ����
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHANNEL_DESC_NTF
		{
			enum _ID{ ID = MSGDEF_ID(CHANNEL_DESC_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANNEL_DESC_NTF);

				PM::_BUFFER channelDesc;	// CHANNEL_DESC ����Ʈ
				PM::_STRING nameList;		// ä���� �̸� ����Ʈ; channelDesc�� 1��1 ��ġ. "\n"�� ������. �� �̸��� ��� �⺻ �̸��� �����.
				TID_SERVER idServer;		// �ش� ä�� ������ ID
			};

			MSGDEF_TYPE TMSG;
		};



#pragma pack(pop)
	}; // namespace CHS
}; // namespace PM


#undef __CAT
#undef __MC
