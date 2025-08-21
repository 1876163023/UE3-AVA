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

			ID_CHECK_NICK_REQ,			/// 새로 등록한 사용자일 경우 닉네임을 체크할때
			ID_CHECK_NICK_ANS,

			ID_PLAYER_INFO_NTF,			/// 플레이어 정보.

			// TEST
			ID_CREATE_ACCOUNT_REQ,
			ID_CREATE_ACCOUNT_ANS,

			//
			ID_PLAYER_INFO_REQ,			/// 플레이어 정보.
			ID_PLAYER_INFO_ANS,

			ID_HWINFO_NTF,

			ID_MAIN_NOTICE_NTF,			// 대문에서 보여줄 공지

			ID_KICK_NTF,

			ID_DISCONNECT_NTF,
			ID_SET_CONFIG_NTF,
				
			//GUILD
			ID_GUILD_CONNECT_REQ,
			ID_GUILD_CONNECT_ANS,			

			ID_GUILD_PLAYER_INFO_REQ,		/// 플레이어 정보.
			ID_GUILD_PLAYER_INFO_ANS ,	/// GCS --> CHS 플레이어정보 이동

			ID_GUILD_PLAYER_UPDATE_REQ,		/// 플레이어 정보.
			ID_GUILD_PLAYER_UPDATE_ANS ,	/// GCS --> CHS 플레이어정보 이동

			ID_PLAYER_ITEM_INFO_NTF,			/// 플레이어 정보.
			ID_PLAYER_AWARD_INFO_NTF,			/// 플레이어 정보.
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

		// 맨 처음 접속하거나.. 재접속할때 쓰이는 패킷.
		namespace CONNECT_REQ
		{
			enum _ID { ID = MSGDEF_ID(CONNECT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CONNECT_REQ);

				WORD			versionProtocol;		// 프로토콜 버젼 - Comdef/Version.h 에 VERSION 참조.
				WORD			versionClient;			// 클라이언트 버전
				
				BYTE			reconnect;				// 처음 접속할때는 0,
														// 같은 서버내 세션 이동은.. 1,
														// CHS -> CHS접속은.. 2,
								
				_INT64			addr;					// CHS 이동시 플레어어 정보를 얻어오기 위한 이전 CHS의 Connectionless Address..

				PM::_STRING		key;					// 웹 서버가 클라이언트에게 전달해주는 키
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CONNECT_ANS
		{
			enum _ID { ID = MSGDEF_ID(CONNECT_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CONNECT_ANS);

				BYTE			result;					// RC_OK 성공.
				TID_ACCOUNT		idAccount;
														// RC_LOGIN_NO_ACCOUNT 인증은 성공했으나 계정정보가 없어서 새로생성해야함.
														// 나머지는 다 실패. 서버에서 연결을 끊음!

				BYTE			gmLevel;

				_INT64			addr;					// 새로 접속한 서버의 Connectionless Address
			};

			MSGDEF_TYPE TMSG;
		}

		// 맨 처음 접속하거나.. 재접속할때 쓰이는 패킷.
		namespace GUILD_CONNECT_REQ
		{
			enum _ID { ID = MSGDEF_ID(GUILD_CONNECT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_CONNECT_REQ);

				WORD			versionProtocol;		// 프로토콜 버젼 - Comdef/Version.h 에 VERSION 참조.
				WORD			versionClient;			// 클라이언트 버전

				TID_ACCOUNT		idAccount;
				TID_GUILD		idGuild;
				BYTE			reconnect;				// 처음 접속할때는 0,

				_INT64			addr;					// CHS 이동시 플레어어 정보를 얻어오기 위한 이전 CHS의 Connectionless Address..

				PM::_STRING		key;					// 웹 서버가 클라이언트에게 전달해주는 키
			};

			MSGDEF_TYPE TMSG;
		}

		namespace GUILD_CONNECT_ANS
		{
			enum _ID { ID = MSGDEF_ID(GUILD_CONNECT_ANS) };

			struct DEF
			{
				MSGDEF_NAME(GUILD_CONNECT_ANS);

				BYTE			result;				// RC_OK 성공.
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

		// 캐릭터를 새로 생성할때 쓰이는 패킷.
		namespace CHECK_NICK_REQ
		{
			enum _ID { ID = MSGDEF_ID(CHECK_NICK_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CHECK_NICK_REQ);

				BYTE	idFace;
				WCHAR	nickname[SIZE_NICKNAME+1];	// 닉네임				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHECK_NICK_ANS
		{
			enum _ID { ID = MSGDEF_ID(CHECK_NICK_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CHECK_NICK_ANS);

				BYTE			result;				/// 닉네임 체크결과	RC_OK 는 성공. 나머지는 실패.			
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

				PLAYER_INFO			playerInfo;			/// 플레이어의 정보				
				
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

				PLAYER_INFO			playerInfo;			/// 플레이어의 정보				

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
                
				PLAYER_INFO			playerInfo;			/// 플레이어의 정보				

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
				BYTE				result;				// RC_OK 성공.				
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

				WCHAR		nickname[SIZE_NICKNAME+1];		// 캐릭터 이름

				BYTE		faceType;						// 얼굴
				BYTE		level;							// 계급

				BYTE		currentClass;					// 마지막으로 플레이했던 클래스
				BYTE		lastTeam;						// 마지막으로 플레이했던 팀
				WORD		straightWin;					// 현재 연승 회수		

				BYTE		xpProgress;
				DWORD		xp;								// 경험치		
				TPOINT		supplyPoint;					// 보급

				// Skill 정보.
				PLAYER_SKILL_INFO	skillInfo;

				//////////////////////////////////////////
				// Result 정보.
				PLAYER_SCORE_INFO	scoreInfo;

				//////////////////////////////////////////
				// 기타 캐쉬 및 경험치..
				//TCASH		cash;							// 보유하고 있는 캐쉬		
				TMONEY		money;							// 게임머니

				WCHAR		configstr[SIZE_CONFIGSTR1+1];	// DB에 저장하는 개인 설정 문자열
				WCHAR		configstr2[SIZE_CONFIGSTR2+1];	// DB에 저장하는 개인 설정 문자열

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
				PM::_BUFFER		customWeapon;	// 커스텀 아이템 리스트
				PM::_BUFFER		effectInven;	// 부스트 아이템 리스트
				
				TSN_ITEM		weaponSet[MAX_WEAPONSET_SIZE];	// 병과 별 셋팅한 무기.	
				TSN_ITEM		equipSet[MAX_EQUIPSET_SIZE];	// 장착 아이탬 ITEM_INFO의 index 를 저장				
				TSN_ITEM		effectSet[MAX_EFFECTSET_SIZE]; // 장착 아이탬 ITEM_INFO의 index 를 저장
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

		// 캐릭터를 새로 생성할때 쓰이는 패킷.
		/*
		namespace CREATE_ACCOUNT_REQ
		{
			enum _ID { ID = MSGDEF_ID(CREATE_ACCOUNT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CREATE_ACCOUNT_REQ);

				WCHAR			user_id[SIZE_USER_ID+1];		// 유저
				WCHAR			password[SIZE_USER_PWD+1];	// 암호
				WCHAR			username[SIZE_USERNAME+1];	// 실명
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
