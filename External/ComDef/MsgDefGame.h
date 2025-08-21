/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: MsgDefGame.h

	Description: Message definition about game.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("GAME")
#define __MC MC_GAME

namespace PM
{
	namespace GAME
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,
			ID_START_NTF,
			ID_READY_NTF,
			ID_JOIN_NTF,
			ID_END_NTF,
			ID_UPDATE_STATE_NTF,
			ID_UPDATE_SCORE_NTF,
			ID_PING_REQ,
			ID_PING_ANS,
			ID_LEAVE_NTF,

			ID_HOST_BAN_NTF,

			ID_START_COUNT_NTF,
			ID_CANCEL_COUNT_NTF,

			ID_REPORT_STAT_NTF,
			ID_LOADING_PROGRESS_NTF,

			ID_SKILL_UPDATE_NTF,
			ID_RESULT_UPDATE_NTF,
			ID_AWARD_UPDATE_NTF,
			ID_ITEM_UPDATE_NTF,

			ID_RESULT_NTF,		//  all member result
			ID_REPORT_VOTE_NTF,
			ID_CHAT_NTF,
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)


		namespace START_NTF
		{
			enum _ID { ID = MSGDEF_ID(START_NTF) };

			struct DEF
			{
				MSGDEF_NAME(START_NTF)

				BYTE bStart;			// 0이면.. 대기.. 1이면 스타트!!!!
				// resolved from UDP server
				UDP_HOST_INFO hostInfo;
			};

			MSGDEF_TYPE TMSG;
		};




		namespace END_NTF
		{
			enum _ID { ID = MSGDEF_ID(END_NTF) };

			struct DEF
			{
				MSGDEF_NAME(END_NTF)

				WORD avgHostPing;
			};

			MSGDEF_TYPE TMSG;
		};



		namespace JOIN_NTF
		{
			enum _ID { ID = MSGDEF_ID(JOIN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(JOIN_NTF)

				TID_ACCOUNT idAccount;
				UDP_HOST_INFO hostInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace LEAVE_NTF
		{
			enum _ID { ID = MSGDEF_ID(LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LEAVE_NTF)

				TID_ACCOUNT idAccount;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace READY_NTF
		{
			enum _ID { ID = MSGDEF_ID(READY_NTF) };

			struct DEF
			{
				MSGDEF_NAME(READY_NTF)
			};

			MSGDEF_TYPE TMSG;
		};

		namespace UPDATE_STATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(UPDATE_STATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(UPDATE_STATE_NTF)

				BYTE roundCount;
			};

			MSGDEF_TYPE TMSG;
		};


		namespace UPDATE_SCORE_NTF
		{
			enum _ID { ID = MSGDEF_ID(UPDATE_SCORE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(UPDATE_SCORE_NTF)

				_BUFFER playerScore;
				BYTE bGameEnd;
				BYTE teamWinCount[2];
			};

			MSGDEF_TYPE TMSG;
		};





		namespace PING_REQ
		{
			enum _ID { ID = MSGDEF_ID(PING_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PING_REQ)

				unsigned long timestamp;
			};

			MSGDEF_TYPE TMSG;
		}




		namespace PING_ANS
		{
			enum _ID { ID = MSGDEF_ID(PING_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PING_ANS)

				unsigned long timestamp;
			};

			MSGDEF_TYPE TMSG;
		}


		namespace HOST_BAN_NTF
		{
			enum _ID { ID = MSGDEF_ID(HOST_BAN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(HOST_BAN_NTF)

				TID_ACCOUNT idHostAccount;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace START_COUNT_NTF
		{
			enum _ID { ID = MSGDEF_ID(START_COUNT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(START_COUNT_NTF)
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CANCEL_COUNT_NTF
		{
			enum _ID { ID = MSGDEF_ID(CANCEL_COUNT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CANCEL_COUNT_NTF)
			};

			MSGDEF_TYPE TMSG;
		};


		namespace REPORT_STAT_NTF
		{
			enum _ID { ID = MSGDEF_ID(REPORT_STAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(REPORT_STAT_NTF)

				DWORD logFilter;
				STAT_GAME_SCORE_LOG gameScoreLogs[2];
				PM::_BUFFER roundPlayLogs;
				PM::_BUFFER roundWeaponLogs;
				PM::_BUFFER killLogs;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace LOADING_PROGRESS_NTF
		{
			enum _ID { ID = MSGDEF_ID(LOADING_PROGRESS_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LOADING_PROGRESS_NTF)
				
				BYTE	idSlot;
				BYTE	progress;	// 0~100 까지의 진행률
				BYTE	dummy[2];
				INT		step;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace SKILL_UPDATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(SKILL_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SKILL_UPDATE_NTF);

				PLAYER_SKILL_INFO	skillInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace RESULT_UPDATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(RESULT_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(RESULT_UPDATE_NTF);

				WORD	xp;
				BYTE	xpProgress;
				BYTE	level;
				DWORD	supplyPoint;
				WORD	money;
				int		supplyCnt;

				// bonuses
				BYTE	biaXPFlag;
				WORD	biaXP;
				WORD	boostXP;
				WORD	boostSupply;
				WORD	boostMoney;
				WORD	pcBangXP;
				WORD	pcBangMoney;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace AWARD_UPDATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(AWARD_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(AWARD_UPDATE_NTF);

				TID_AWARD award_id;				
			};

			MSGDEF_TYPE TMSG;
		};

		namespace ITEM_UPDATE_NTF
		{
			enum _ID { ID = MSGDEF_ID(ITEM_UPDATE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_UPDATE_NTF);

				_BUFFER		itemInfo;		// ITEM_BASE_INFO list
				BYTE		updateType;		// 0 = supply , 1 = udate level
			};

			MSGDEF_TYPE TMSG;
		};

		namespace RESULT_NTF
		{
			enum _ID { ID = MSGDEF_ID(RESULT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(RESULT_NTF);

				ROOM_RESULT_INFO resultInfo[MAX_PLAYER_PER_ROOM];
				BYTE teamWinCount[2];
			};

			MSGDEF_TYPE TMSG;
		};
		
		namespace REPORT_VOTE_NTF
		{
			enum _ID { ID = MSGDEF_ID(REPORT_VOTE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(REPORT_VOTE_NTF);

				DWORD command;
				TID_ACCOUNT idCaller;
				TID_ACCOUNT idVoter[MAX_PLAYER_PER_TEAM-1];
				DWORD param1;
				DWORD param2;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace CHAT_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHAT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHAT_NTF);

				TID_ACCOUNT idAccount;
				PM::_STRING chatmsg;

				BYTE		team;
			};

			MSGDEF_TYPE TMSG;
		};




#pragma pack(pop)

	};	// namespace GAME
};	// namespace PM


#undef __CAT
#undef __MC
