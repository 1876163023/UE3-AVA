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

			
			ID_JOIN_CLAN_NTF,					// MsgGW -> GCS : Clan ����
			ID_GM_JOIN_CLAN_NTF,				// MsgGW -> GCS : GM�� ���� Clan ����
			ID_SECEDE_CLAN_NTF,					// MsgGW -> GCS : Clan Ż��
			ID_KICKOUT_CLANMEMBER_NTF,			// MsgGW -> GCS : Clan ���� Ż��
			ID_GM_KICKOUT_CLANMEMBER_NTF,		// MsgGW -> GCS : GM�� ���� Clan ���� Ż��
			ID_ENTRUST_MASTER_NTF,				// MsgGW -> GCS : ������ ����
			ID_GM_ENTRUST_MASTER_NTF,			// MsgGW -> GCS : GM�� ���� ������ ����
			ID_SET_GRADE_CLANMEMBER_NTF,		// MsgGW -> GCS : Clan�� �������
			ID_GM_SET_GRADE_CLANMEMBER_NTF,		// MsgGW -> GCS : GM�� ���� Clan�� �������
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
				TID_ACCOUNT	dwFrom_USN; // Ŭ�������� ó���� ȸ���� USN
				TID_ACCOUNT	dwTo_USN;	// Ŭ���������Ϸ��� ȸ���� USN 	
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
				TID_ACCOUNT	dwGM_USN; // Ŭ�������� ó���� GM�� USN 
				TID_ACCOUNT	dwTo_USN; // Ŭ���������Ϸ��� ȸ���� USN 
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
				TID_ACCOUNT	dwUSN;			// Ŭ������ Ż���� ȸ���� USN
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
				TID_ACCOUNT	dwFrom_USN;	 // Ŭ������ �����Ų User�� USN 
				TID_ACCOUNT	dwTo_USN;	 // Ŭ������ ������� User�� USN 
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
				TID_ACCOUNT	dwGM_USN;		// Ŭ������ �����Ų GM�� USN 
				TID_ACCOUNT	dwTo_USN;		// Ŭ������ ������� User�� USN 
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
				TID_ACCOUNT	dwFrom_USN;		// ������ ������ USN
				TID_ACCOUNT	dwTo_USN;		// ������ ����� USN
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
				TID_ACCOUNT	dwGM_USN;	// ������ ������ GM�� USN
				TID_ACCOUNT	dwTo_USN;	// ������ ����� USN
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
				TID_ACCOUNT	dwFromGrade;  // ������ ��� 
				TID_ACCOUNT	dwToGrade;    // ����� ��� 
				TID_ACCOUNT	dwFrom_USN;	  // ����� ������ ������� USN 
				TID_ACCOUNT	dwTo_USN;	  // ����� ����� ������� USN 
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
				TID_ACCOUNT	dwFromGrade;// ������ ��� 
				TID_ACCOUNT	dwToGrade;	// ����� ��� 
				TID_ACCOUNT	dwGM_USN;	// ����� ������ GM�� USN
				TID_ACCOUNT	dwTo_USN;	// ����� ����� ������� USN 
			};

			MSGDEF_TYPE TMSG;
		}

#pragma pack(pop)
	}	// namespace WEB
}	// namespace PM


#undef __CAT
#undef __MC
