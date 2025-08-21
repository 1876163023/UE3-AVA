/***

Copyright (c) Redduck Inc. All rights reserved.

Project: ComDef

Name: MsgDefClient.h

Description: Message definition about client.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("CLIENT")
#define __MC MC_CLIENT

namespace PM
{
	namespace CLIENT
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,

			ID_CONNECT_REQ,
			ID_CONNECT_ANS,

			ID_CHECK_NICK_REQ,			/// ���� ����� ������� ��� �г����� üũ�Ҷ�
			ID_CHECK_NICK_ANS,

			ID_PLAYER_INFO_NTF,			/// �÷��̾� ����.

			// TEST
			ID_CREATE_ACCOUNT_REQ,
			ID_CREATE_ACCOUNT_ANS,

			//
			ID_PLAYER_INFO_REQ,			/// �÷��̾� ����.
			ID_PLAYER_INFO_ANS,

			ID_HWINFO_NTF,

			ID_MAIN_NOTICE_NTF,			// �빮���� ������ ����

			ID_KICK_NTF,

			ID_DISCONNECT_NTF,
			ID_SET_CONFIG_NTF,
				
			//GUILD
			ID_GUILD_CONNECT_REQ,
			ID_GUILD_CONNECT_ANS,			

			ID_GUILD_PLAYER_INFO_REQ,		/// �÷��̾� ����.
			ID_GUILD_PLAYER_INFO_ANS ,	/// GCS --> CHS �÷��̾����� �̵�

			ID_GUILD_PLAYER_UPDATE_REQ,		/// �÷��̾� ����.
			ID_GUILD_PLAYER_UPDATE_ANS ,	/// GCS --> CHS �÷��̾����� �̵�

			ID_PLAYER_ITEM_INFO_NTF,			/// �÷��̾� ����.
			ID_PLAYER_AWARD_INFO_NTF,			/// �÷��̾� ����.
			ID_PLAYER_GUILD_INFO_NTF,
			ID_PLAYER_GUILD_ADDR_NTF,

			ID_SET_RTTTEST_ADDR_NTF,

			ID_PLAYER_DATA_RECOVERY_NTF,

			ID_PLAYER_HOST_RATING_NTF,

			ID_UPDATE_PCBANG_NTF,
		};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)

		// �� ó�� �����ϰų�.. �������Ҷ� ���̴� ��Ŷ.
		namespace CONNECT_REQ
		{
			enum _ID { ID = MSGDEF_ID(CONNECT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CONNECT_REQ);

				WORD			versionProtocol;		// �������� ���� - Comdef/Version.h �� VERSION ����.
				WORD			versionClient;			// Ŭ���̾�Ʈ ����
				
				BYTE			reconnect;				// ó�� �����Ҷ��� 0,
														// ���� ������ ���� �̵���.. 1,
														// CHS -> CHS������.. 2,
								
				_INT64			addr;					// CHS �̵��� �÷���� ������ ������ ���� ���� CHS�� Connectionless Address..

				PM::_STRING		key;					// �� ������ Ŭ���̾�Ʈ���� �������ִ� Ű
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CONNECT_ANS
		{
			enum _ID { ID = MSGDEF_ID(CONNECT_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CONNECT_ANS);

				BYTE			result;					// RC_OK ����.
				TID_ACCOUNT		idAccount;
														// RC_LOGIN_NO_ACCOUNT ������ ���������� ���������� ��� ���λ����ؾ���.
														// �������� �� ����. �������� ������ ����!

				BYTE			gmLevel;

				_INT64			addr;					// ���� ������ ������ Connectionless Address
			};

			MSGDEF_TYPE TMSG;
		}

		// �� ó�� �����ϰų�.. �������Ҷ� ���̴� ��Ŷ.
		namespace GUILD_CONNECT_REQ
		{
			enum _ID { ID = MSGDEF_ID(GUILD_CONNECT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_CONNECT_REQ);

				WORD			versionProtocol;		// �������� ���� - Comdef/Version.h �� VERSION ����.
				WORD			versionClient;			// Ŭ���̾�Ʈ ����

				TID_ACCOUNT		idAccount;
				TID_GUILD		idGuild;
				BYTE			reconnect;				// ó�� �����Ҷ��� 0,

				_INT64			addr;					// CHS �̵��� �÷���� ������ ������ ���� ���� CHS�� Connectionless Address..

				PM::_STRING		key;					// �� ������ Ŭ���̾�Ʈ���� �������ִ� Ű
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GUILD_CONNECT_ANS
		{
			enum _ID { ID = MSGDEF_ID(GUILD_CONNECT_ANS) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_CONNECT_ANS);

				BYTE			result;				// RC_OK ����.
			};

			MSGDEF_TYPE TMSG;
		}

		
		namespace DISCONNECT_NTF
		{
			enum _ID { ID = MSGDEF_ID(DISCONNECT_NTF) };

			struct DEF
			{
				MSGDEF_NAME(DISCONNECT_NTF);

				TID_ACCOUNT		idAccount;
				TID_GUILD		idGuild;
			};

			MSGDEF_TYPE TMSG;
		}		

		// ĳ���͸� ���� �����Ҷ� ���̴� ��Ŷ.
		namespace CHECK_NICK_REQ
		{
			enum _ID { ID = MSGDEF_ID(CHECK_NICK_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHECK_NICK_REQ);

				BYTE	idFace;
				WCHAR	nickname[SIZE_NICKNAME+1];	// �г���				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHECK_NICK_ANS
		{
			enum _ID { ID = MSGDEF_ID(CHECK_NICK_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHECK_NICK_ANS);

				BYTE			result;				/// �г��� üũ���	RC_OK �� ����. �������� ����.			
				TID_ACCOUNT		idAccount;				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_INFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_REQ);

				TID_ACCOUNT			idAccount;				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_INFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_ANS);

				TID_ACCOUNT			idAccount;
                BYTE				result;

				PLAYER_INFO			playerInfo;			/// �÷��̾��� ����				
				
				_INT64				log_sn;
				_INT64				login_time;	

				long				denyChatTime;

				BYTE				gmLevel;
				BOOL				bVisible;

				WORD				hwRating;
				WORD				nb;

				TID_ACCOUNT			biaUSN;
				DWORD				pcbangServiceType;

				DWORD				supplyCnt;
			};

			MSGDEF_TYPE TMSG;
		}

        namespace GUILD_PLAYER_INFO_REQ
		{
			enum _ID { ID = MSGDEF_ID(GUILD_PLAYER_INFO_REQ) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_PLAYER_INFO_REQ);

				TID_ACCOUNT			idAccount;		
				TID_GUILD			idGuild;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GUILD_PLAYER_INFO_ANS
		{
			enum _ID { ID = MSGDEF_ID(GUILD_PLAYER_INFO_ANS) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_PLAYER_INFO_ANS);

				TID_ACCOUNT			idAccount;
				TID_GUILD			idGuild;
				BYTE				result;

				PLAYER_INFO			playerInfo;			/// �÷��̾��� ����				

				_INT64				log_sn;
				_INT64				login_time;	

				long				denyChatTime;

				BYTE				gmLevel;
				BOOL				bVisible;

				WORD				hwRating;
				WORD				nb;

				TID_ACCOUNT			biaUSN;
				DWORD				pcbangServiceType;

				DWORD				supplyCnt;

			};

			MSGDEF_TYPE TMSG;
		}

		namespace GUILD_PLAYER_UPDATE_REQ
		{
			enum _ID { ID = MSGDEF_ID(GUILD_PLAYER_UPDATE_REQ) };
 
			struct DEF
			{
				MSGDEF_NAME(GUILD_PLAYER_UPDATE_REQ);

				TID_ACCOUNT			idAccount;
				TID_GUILD			idGuild;
                
				PLAYER_INFO			playerInfo;			/// �÷��̾��� ����				

				long				denyChatTime;
				WORD				hwRating;
				WORD				nb;

				TID_ACCOUNT			biaUSN;

				DWORD				supplyCnt;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GUILD_PLAYER_UPDATE_ANS
		{
			enum _ID { ID = MSGDEF_ID(GUILD_PLAYER_UPDATE_ANS) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_PLAYER_UPDATE_ANS);

				TID_ACCOUNT			idAccount;
				TID_GUILD			idGuild;
				BYTE				result;				// RC_OK ����.				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_INFO_NTF);

				TID_ACCOUNT	idAccount;		

				WCHAR		nickname[SIZE_NICKNAME+1];		// ĳ���� �̸�

				BYTE		faceType;						// ��
				BYTE		level;							// ���

				BYTE		currentClass;					// ���������� �÷����ߴ� Ŭ����
				BYTE		lastTeam;						// ���������� �÷����ߴ� ��
				WORD		straightWin;					// ���� ���� ȸ��		

				BYTE		xpProgress;
				DWORD		xp;								// ����ġ		
				TPOINT		supplyPoint;					// ����

				// Skill ����.
				PLAYER_SKILL_INFO	skillInfo;

				//////////////////////////////////////////
				// Result ����.
				PLAYER_SCORE_INFO	scoreInfo;

				//////////////////////////////////////////
				// ��Ÿ ĳ�� �� ����ġ..
				//TCASH		cash;							// �����ϰ� �ִ� ĳ��		
				TMONEY		money;							// ���ӸӴ�

				WCHAR		configstr[SIZE_CONFIGSTR1+1];	// DB�� �����ϴ� ���� ���� ���ڿ�
				WCHAR		configstr2[SIZE_CONFIGSTR2+1];	// DB�� �����ϴ� ���� ���� ���ڿ�

				DWORD		biaXP;

				BYTE		channelMaskList[MAX_CHANNEL_MASK];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_ITEM_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_ITEM_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_ITEM_INFO_NTF);

				PM::_BUFFER		weaponInven;
				PM::_BUFFER		equipInven;
				PM::_BUFFER		customWeapon;	// Ŀ���� ������ ����Ʈ
				PM::_BUFFER		effectInven;	// �ν�Ʈ ������ ����Ʈ
				
				TSN_ITEM		weaponSet[MAX_WEAPONSET_SIZE];	// ���� �� ������ ����.	
				TSN_ITEM		equipSet[MAX_EQUIPSET_SIZE];	// ���� ������ ITEM_INFO�� index �� ����				
				TSN_ITEM		effectSet[MAX_EFFECTSET_SIZE]; // ���� ������ ITEM_INFO�� index �� ����
			};

			MSGDEF_TYPE TMSG;
		}

		namespace PLAYER_AWARD_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_AWARD_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_AWARD_INFO_NTF);

				PLAYER_AWARD_INFO awardInfo;
			};

			MSGDEF_TYPE TMSG;
		}		

		namespace PLAYER_GUILD_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_GUILD_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_GUILD_INFO_NTF);

				PLAYER_GUILD_INFO	guildInfo;
				_INT64				addr;
			};

			MSGDEF_TYPE TMSG;
		}		

		namespace PLAYER_GUILD_ADDR_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_GUILD_ADDR_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_GUILD_ADDR_NTF);

				_INT64				addr;
			};

			MSGDEF_TYPE TMSG;
		}		

		// ĳ���͸� ���� �����Ҷ� ���̴� ��Ŷ.
		/*
		namespace CREATE_ACCOUNT_REQ
		{
			enum _ID { ID = MSGDEF_ID(CREATE_ACCOUNT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_ACCOUNT_REQ);

				WCHAR			user_id[SIZE_USER_ID+1];		// ����
				WCHAR			password[SIZE_USER_PWD+1];	// ��ȣ
				WCHAR			username[SIZE_USERNAME+1];	// �Ǹ�
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CREATE_ACCOUNT_ANS
		{
			enum _ID { ID = MSGDEF_ID(CREATE_ACCOUNT_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_ACCOUNT_ANS);

				BYTE				result;
				//TID_ACCOUNT			idAccount;
			};

			MSGDEF_TYPE TMSG;
		};
		*/


		namespace HWINFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(HWINFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(HWINFO_NTF);

				WCHAR cpu_id[SIZE_HWID+1];
				WCHAR gpu_id[SIZE_HWID+1];
				DWORD mem_size;
				BYTE adapterAddress[SIZE_ADAPTER_ADDRESS];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace MAIN_NOTICE_NTF
		{
			enum _ID { ID = MSGDEF_ID(MAIN_NOTICE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(MAIN_NOTICE_NTF);

				WCHAR msg[SIZE_NOTICE_MSG+1];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace KICK_NTF
		{
			enum _ID { ID = MSGDEF_ID(KICK_NTF) };

			struct DEF
			{
				MSGDEF_NAME(KICK_NTF);

				BYTE flag;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_CONFIG_NTF
		{
			enum _ID { ID = MSGDEF_ID(SET_CONFIG_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SET_CONFIG_NTF);

				WCHAR configstr[SIZE_CONFIGSTR1+1];
				WCHAR configstr2[SIZE_CONFIGSTR2+1];
			};

			MSGDEF_TYPE TMSG;
		}

		namespace SET_RTTTEST_ADDR_NTF
		{
			enum _ID { ID = MSGDEF_ID(SET_RTTTEST_ADDR_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SET_RTTTEST_ADDR_NTF);

				UDP_HOST_INFO addrInfo;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_DATA_RECOVERY_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_DATA_RECOVERY_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_DATA_RECOVERY_NTF);
			};

			MSGDEF_TYPE TMSG;
		};

		namespace PLAYER_HOST_RATING_NTF
		{
			enum _ID { ID = MSGDEF_ID(PLAYER_HOST_RATING_NTF) };

			struct DEF
			{
				MSGDEF_NAME(PLAYER_HOST_RATING_NTF);

				WORD rating;
			};

			MSGDEF_TYPE TMSG;
		};

		namespace UPDATE_PCBANG_NTF
		{
			enum _ID { ID = MSGDEF_ID(UPDATE_PCBANG_NTF) };

			struct DEF
			{
				MSGDEF_NAME(UPDATE_PCBANG_NTF);

				TID_ACCOUNT		idAccount;				
				DWORD			serviceType;
			};

			MSGDEF_TYPE TMSG;
		}

#pragma pack(pop)
	}	// namespace CLIENT
}	// namespace PM


#undef __CAT
#undef __MC
