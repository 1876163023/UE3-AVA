/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefWeb.h

Description: Message definition between NW MsgGateway <-> AVA Servers.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("WEB")
#define __MC MC_WEB

namespace PM
{
	namespace WEB
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,

			ID_GUILD_JOIN_NTF,
			ID_WEB_REQ,
			ID_WEB_ANS,

			
			ID_JOIN_CLAN_NTF,					// MsgGW -> GCS : Clan 가입
			ID_GM_JOIN_CLAN_NTF,				// MsgGW -> GCS : GM에 의해 Clan 가입
			ID_SECEDE_CLAN_NTF,					// MsgGW -> GCS : Clan 탈퇴
			ID_KICKOUT_CLANMEMBER_NTF,			// MsgGW -> GCS : Clan 강제 탈퇴
			ID_GM_KICKOUT_CLANMEMBER_NTF,		// MsgGW -> GCS : GM에 의해 Clan 강제 탈퇴
			ID_ENTRUST_MASTER_NTF,				// MsgGW -> GCS : 마스터 위임
			ID_GM_ENTRUST_MASTER_NTF,			// MsgGW -> GCS : GM에 의해 마스터 위임
			ID_SET_GRADE_CLANMEMBER_NTF,		// MsgGW -> GCS : Clan원 등급조정
			ID_GM_SET_GRADE_CLANMEMBER_NTF,		// MsgGW -> GCS : GM에 의해 Clan원 등급조정
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)

		namespace JOIN_CLAN_NTF
		{
			enum _ID { ID = MSGDEF_ID(JOIN_CLAN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(JOIN_CLAN_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwFrom_USN; // 클랜가입을 처리한 회원의 USN
				TID_ACCOUNT	dwTo_USN;	// 클랜에가입하려는 회원의 USN 	
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GM_JOIN_CLAN_NTF
		{
			enum _ID { ID = MSGDEF_ID(GM_JOIN_CLAN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(GM_JOIN_CLAN_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwGM_USN; // 클랜가입을 처리한 GM의 USN 
				TID_ACCOUNT	dwTo_USN; // 클랜에가입하려는 회원의 USN 
			};

			MSGDEF_TYPE TMSG;

		}

		namespace SECEDE_CLAN_NTF
		{
			enum _ID { ID = MSGDEF_ID(SECEDE_CLAN_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SECEDE_CLAN_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwUSN;			// 클랜에서 탈퇴한 회원의 USN
			};

			MSGDEF_TYPE TMSG;

		}

		namespace KICKOUT_CLANMEMBER_NTF
		{
			enum _ID { ID = MSGDEF_ID(KICKOUT_CLANMEMBER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(KICKOUT_CLANMEMBER_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwFrom_USN;	 // 클랜에서 강퇴시킨 User의 USN 
				TID_ACCOUNT	dwTo_USN;	 // 클랜에서 강퇴당한 User의 USN 
			};

			MSGDEF_TYPE TMSG;

		}

		namespace GM_KICKOUT_CLANMEMBER_NTF
		{
			enum _ID { ID = MSGDEF_ID(GM_KICKOUT_CLANMEMBER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(GM_KICKOUT_CLAN_NTF);

				BYTE result; 

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwGM_USN;		// 클랜에서 강퇴시킨 GM의 USN 
				TID_ACCOUNT	dwTo_USN;		// 클랜에서 강퇴당한 User의 USN 
			};

			MSGDEF_TYPE TMSG;

		}

		namespace ENTRUST_MASTER_NTF
		{
			enum _ID { ID = MSGDEF_ID(ENTRUST_MASTER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ENTRUST_MASTER_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwFrom_USN;		// 마스터 위임자 USN
				TID_ACCOUNT	dwTo_USN;		// 마스터 계승자 USN
			};

			MSGDEF_TYPE TMSG;

		}

		namespace GM_ENTRUST_MASTER_NTF
		{
			enum _ID { ID = MSGDEF_ID(GM_ENTRUST_MASTER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(GM_ENTRUST_MASTER_NTF);

				BYTE	result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwGM_USN;	// 마스터 위위한 GM의 USN
				TID_ACCOUNT	dwTo_USN;	// 마스터 계승자 USN
			};

			MSGDEF_TYPE TMSG;

		}

		namespace SET_GRADE_CLANMEMBER_NTF
		{
			enum _ID { ID = MSGDEF_ID(SET_GRADE_CLANMEMBER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SET_GRADE_CLANMEMBER_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwFromGrade;  // 변경전 등급 
				TID_ACCOUNT	dwToGrade;    // 변경된 등급 
				TID_ACCOUNT	dwFrom_USN;	  // 등급을 변경한 사용자의 USN 
				TID_ACCOUNT	dwTo_USN;	  // 등급이 변경된 사용자의 USN 
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GM_SET_GRADE_CLANMEMBER_NTF
		{
			enum _ID { ID = MSGDEF_ID(GM_SET_GRADE_CLANMEMBER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(GM_SET_GRADE_CLANMEMBER_NTF);

				BYTE result;

				TID_GUILD	idGuild;	// Clan Channel Serial Number
				TID_ACCOUNT	dwFromGrade;// 변경전 등급 
				TID_ACCOUNT	dwToGrade;	// 변경된 등급 
				TID_ACCOUNT	dwGM_USN;	// 등급을 변경한 GM의 USN
				TID_ACCOUNT	dwTo_USN;	// 등급이 변경된 사용자의 USN 
			};

			MSGDEF_TYPE TMSG;
		}

#pragma pack(pop)
	}	// namespace WEB
}	// namespace PM


#undef __CAT
#undef __MC
