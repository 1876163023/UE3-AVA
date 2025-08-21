/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: MsgDefBootSeq.h

	Description: Message definition about AVA server boot sequence.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("BOOTSEQ")
#define __MC MC_BOOTSEQ

namespace PM
{
	namespace BOOTSEQ
	{
		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,	

			ID_SVR_START_NTF,			/// 서버를 시작할때 보내주는 정보
			ID_SVR_STOP_NTF,			/// 서버를 정지할때 보내주는 정보
			ID_SVR_INFO_NTF,			/// 다른 서버가 시작했다는 메세지를 받고 자신의 정보를 보내줄때

			ID_SVR_ACTIVE_NTF,			/// 서버가 서비스를 시작했다는 통보
			ID_SVR_INACTIVE_NTF,		/// 서버가 서비스를 중단했다는 통보
			/////////////////////////////////////////////			
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions

#pragma pack(push)
#pragma pack(1)

		namespace SVR_START_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_START_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_START_NTF)

				_ServerType		serverType;		/// 서버 타입
				TID_SERVER		idServer;		/// 서버 아이디	

				IPADDR_INFO		hostInfo;		/// 클라이언트에서 직접접속을 받는 서버들은 이정보를 채워주자.. 
			};

			MSGDEF_TYPE TMSG;
		};	

		namespace SVR_STOP_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_START_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_START_NTF)

				_ServerType		serverType;		/// 서버 타입
				TID_SERVER		idServer;		/// 서버 아이디	
			};

			MSGDEF_TYPE TMSG;
		};	

		namespace SVR_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_INFO_NTF)

				_ServerType		serverType;		/// 서버 타입
				TID_SERVER		idServer;		/// 서버 아이디	

				IPADDR_INFO		hostInfo;		/// 클라이언트에서 직접접속을 받는 서버들은 이정보를 채워주자.. 
			};

			MSGDEF_TYPE TMSG;
		};

		namespace SVR_ACTIVE_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_ACTIVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_ACTIVE_NTF)

				RXNERVE_ADDRESS addr;
			};

			MSGDEF_TYPE TMSG;
		};	

		namespace SVR_INACTIVE_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_INACTIVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_INACTIVE_NTF)

				RXNERVE_ADDRESS addr;
			};

			MSGDEF_TYPE TMSG;
		};		
		
#pragma pack(pop)


	};	// namespace CHSBOOTSEQ
};	// namespace PM


#undef __CAT
#undef __MC
