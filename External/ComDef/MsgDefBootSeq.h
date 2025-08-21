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

			ID_SVR_START_NTF,			/// ������ �����Ҷ� �����ִ� ����
			ID_SVR_STOP_NTF,			/// ������ �����Ҷ� �����ִ� ����
			ID_SVR_INFO_NTF,			/// �ٸ� ������ �����ߴٴ� �޼����� �ް� �ڽ��� ������ �����ٶ�

			ID_SVR_ACTIVE_NTF,			/// ������ ���񽺸� �����ߴٴ� �뺸
			ID_SVR_INACTIVE_NTF,		/// ������ ���񽺸� �ߴ��ߴٴ� �뺸
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

				_ServerType		serverType;		/// ���� Ÿ��
				TID_SERVER		idServer;		/// ���� ���̵�	

				IPADDR_INFO		hostInfo;		/// Ŭ���̾�Ʈ���� ���������� �޴� �������� �������� ä������.. 
			};

			MSGDEF_TYPE TMSG;
		};	

		namespace SVR_STOP_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_START_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_START_NTF)

				_ServerType		serverType;		/// ���� Ÿ��
				TID_SERVER		idServer;		/// ���� ���̵�	
			};

			MSGDEF_TYPE TMSG;
		};	

		namespace SVR_INFO_NTF
		{
			enum _ID { ID = MSGDEF_ID(SVR_INFO_NTF) };

			struct DEF
			{
				MSGDEF_NAME(SVR_INFO_NTF)

				_ServerType		serverType;		/// ���� Ÿ��
				TID_SERVER		idServer;		/// ���� ���̵�	

				IPADDR_INFO		hostInfo;		/// Ŭ���̾�Ʈ���� ���������� �޴� �������� �������� ä������.. 
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
