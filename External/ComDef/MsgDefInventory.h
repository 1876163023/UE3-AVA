/***

	Copyright (c) Redduck Inc. All rights reserved.

	Project: ComDef

	Name: MsgDefInventory.h

	Description: Message definition about session control.

***/

#pragma once

#include "_MsgBase.h"


#define __CAT _T("INVENTORY")
#define __MC MC_INVENTORY

namespace PM
{
	namespace INVENTORY
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Message IDs
		enum _ID
		{
			ID_NONE = 0,
						
			//////////////////////////////
			// �κ� ����..
			ID_ENTER_NTF,				/// �κ��丮�� ����.
			ID_LEAVE_NTF,

			ID_EQUIPSET_REQ,			/// ���� ������ �����ϱ�
			ID_EQUIPSET_ANS,			
			ID_WEAPONSET_REQ,			/// ���� ������ �����ϱ�
			ID_WEAPONSET_ANS,			

			ID_CUSTOMSET_REQ,			/// Ŀ���� ������ ����
			ID_CUSTOMSET_ANS,

			ID_ITEM_BUY_REQ,			/// �޿��� �������� ����
			ID_ITEM_BUY_ANS,			

			ID_ITEM_REFUND_REQ,			/// �������� �ȱ�
			ID_ITEM_REFUND_ANS,			

			ID_ITEM_GIFT_REQ,			/// �޿��� �������� �����ϱ�..
			ID_ITEM_GIFT_ANS,

			ID_REPAIR_REQ,				/// ������ ���� ��û
			ID_REPAIR_ANS,

			ID_ITEM_DELETE_NTF,			/// �������� ������ ������ ���� ����
			ID_EQUIPSET_NTF,			/// �������� ������ ������ ���� ����
			ID_WEAPONSET_NTF,	

			ID_CHANGE_CLASS_NTF,

			ID_CONVERT_RIS_REQ,
			ID_CONVERT_RIS_ANS,

			ID_UPDATE_GAUGE_NTF,

			ID_CASHITEM_BUY_REQ,			// ĳ�� ������
			ID_CASHITEM_BUY_ANS,
			ID_EFFSET_REQ,
			ID_EFFSET_ANS,
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Message definitions


#pragma pack(push)
#pragma pack(1)

		namespace ENTER_NTF
		{
			enum _ID { ID = MSGDEF_ID(ENTER_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ENTER_NTF);		
				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace LEAVE_NTF
		{
			enum _ID { ID = MSGDEF_ID(LEAVE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(LEAVE_NTF);		
				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace EQUIPSET_REQ
		{
			enum _ID { ID = MSGDEF_ID(EQUIPSET_REQ) };

			struct DEF
			{
				MSGDEF_NAME(EQUIPSET_REQ);

				TID_EQUIP_SLOT	equipSlot;	// �� ���� ���Կ� ����!
				TSN_ITEM		item_sn;			// �� ���Կ� �ִ� ��������..
				
			};

			MSGDEF_TYPE TMSG;
		}
		namespace EQUIPSET_ANS
		{
			enum _ID { ID = MSGDEF_ID(EQUIPSET_ANS) };

			struct DEF
			{
				MSGDEF_NAME(EQUIPSET_ANS);
				BYTE	result;				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace WEAPONSET_REQ
		{
			enum _ID { ID = MSGDEF_ID(WEAPONSET_REQ) };

			struct DEF
			{
				MSGDEF_NAME(WEAPONSET_REQ);

				TID_EQUIP_SLOT	equipSlot;	// �� ���� ���Կ� ����!
				TSN_ITEM		item_sn;	// �� ���Կ� �ִ� ��������..
			};

			MSGDEF_TYPE TMSG;
		}
		namespace WEAPONSET_ANS
		{
			enum _ID { ID = MSGDEF_ID(WEAPONSET_ANS) };

			struct DEF
			{
				MSGDEF_NAME(WEAPONSET_ANS);
				BYTE	result;				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CUSTOMSET_REQ
		{
			enum _ID { ID = MSGDEF_ID(CUSTOMSET_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CUSTOMSET_REQ);

				TSN_ITEM		item_sn;		// �� ���Կ� �ִ� ��������..				
				BYTE			slot;			// ������ ������..
				TID_ITEM		customItem_id;	// ������ Ŀ���� �������� ���̵�.. INVALID �� ���� ����..				
			};

			MSGDEF_TYPE TMSG;
		}
		namespace CUSTOMSET_ANS
		{
			enum _ID { ID = MSGDEF_ID(CUSTOMSET_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CUSTOMSET_ANS);
				BYTE	result;				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_BUY_REQ
		{
			enum _ID { ID = MSGDEF_ID(ITEM_BUY_REQ) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_BUY_REQ);

				TID_ITEM	idItem;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_BUY_ANS
		{
			enum _ID { ID = MSGDEF_ID(ITEM_BUY_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_BUY_ANS);					

				//TID_ACCOUNT		svr_acc_id;
				BYTE result;
				TSN_ITEM		item_sn;	// �� ���Կ� �ִ� ��������..
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_REFUND_REQ
		{
			enum _ID { ID = MSGDEF_ID(ITEM_REFUND_REQ) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_REFUND_REQ);

				//TID_ACCOUNT		svr_acc_id;

				ITEM_INFO item;		// �� ������

				//TPOINT	point;	 // ��...��...
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_REFUND_ANS
		{
			enum _ID { ID = MSGDEF_ID(ITEM_REFUND_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_REFUND_ANS);

				//TID_ACCOUNT		svr_acc_id;

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_GIFT_REQ
		{
			enum _ID { ID = MSGDEF_ID(ITEM_GIFT_REQ) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_GIFT_REQ);

				//TID_ACCOUNT		svr_acc_id;

				ITEM_INFO item;		// �� ������

				//TPOINT	point;	 // ��...��...

				TID_ACCOUNT idAccount; // ������ ��.
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_GIFT_ANS
		{
			enum _ID { ID = MSGDEF_ID(ITEM_GIFT_ANS) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_GIFT_ANS);					

				//TID_ACCOUNT		svr_acc_id;

				BYTE result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace REPAIR_REQ
		{
			enum _ID { ID = MSGDEF_ID(REPAIR_REQ) };

			struct DEF
			{
				MSGDEF_NAME(REPAIR_REQ);

				TSN_ITEM		item_sn;		// �� ���Կ� �ִ� ��������..	
			};

			MSGDEF_TYPE TMSG;
		}
		namespace REPAIR_ANS
		{
			enum _ID { ID = MSGDEF_ID(REPAIR_ANS) };

			struct DEF
			{
				MSGDEF_NAME(REPAIR_ANS);
				BYTE	result;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace ITEM_DELETE_NTF
		{
			enum _ID { ID = MSGDEF_ID(ITEM_DELETE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(ITEM_DELETE_NTF);
				
				BYTE		itemType;	// _ITEM_FLAG_
				TSN_ITEM	item_sn;
				BYTE		slot;		// if custom item .. slot
			};

			MSGDEF_TYPE TMSG;
		}
		namespace EQUIPSET_NTF
		{
			enum _ID { ID = MSGDEF_ID(EQUIPSET_NTF) };

			struct DEF
			{
				MSGDEF_NAME(EQUIPSET_NTF);
				
				TID_EQUIP_SLOT	slot;	
				TSN_ITEM		item_sn;
				
			};

			MSGDEF_TYPE TMSG;
		}
		namespace WEAPONSET_NTF
		{
			enum _ID { ID = MSGDEF_ID(WEAPONSET_NTF) };

			struct DEF
			{
				MSGDEF_NAME(WEAPONSET_NTF);
				
				TID_EQUIP_SLOT	slot;	
				TSN_ITEM		item_sn;
				
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CHANGE_CLASS_NTF
		{
			enum _ID { ID = MSGDEF_ID(CHANGE_CLASS_NTF) };

			struct DEF
			{
				MSGDEF_NAME(CHANGE_CLASS_NTF);

				BYTE idClass;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CONVERT_RIS_REQ
		{
			enum _ID { ID = MSGDEF_ID(CONVERT_RIS_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CONVERT_RIS_REQ);

				TSN_ITEM item_sn;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CONVERT_RIS_ANS
		{
			enum _ID { ID = MSGDEF_ID(CONVERT_RIS_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CONVERT_RIS_ANS);

				BYTE result;
				TSN_ITEM item_sn;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace UPDATE_GAUGE_NTF
		{
			enum _ID { ID = MSGDEF_ID(UPDATE_GAUGE_NTF) };

			struct DEF
			{
				MSGDEF_NAME(UPDATE_GAUGE_NTF);

				BYTE itemType;	// _ITEM_FLAG_
				TSN_ITEM item_sn;
				DWORD gauge;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CASHITEM_BUY_REQ
		{
			enum _ID { ID = MSGDEF_ID(CASHITEM_BUY_REQ) };

			struct DEF
			{
				MSGDEF_NAME(CASHITEM_BUY_REQ);

				TID_ITEM idItem;
				TSN_ITEM item_sn;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace CASHITEM_BUY_ANS
		{
			enum _ID { ID = MSGDEF_ID(CASHITEM_BUY_ANS) };

			struct DEF
			{
				MSGDEF_NAME(CASHITEM_BUY_ANS);

				BYTE result;
				BYTE			itemType;	// _ITEM_FLAG_
				TSN_ITEM		item_sn;
				TMONEY			updatedMoney;
				DWORD			dateLimit;
			};

			MSGDEF_TYPE TMSG;
		}

		namespace EFFSET_REQ
		{
			enum _ID { ID = MSGDEF_ID(EFFSET_REQ) };

			struct DEF
			{
				MSGDEF_NAME(EFFSET_REQ);

				TID_EQUIP_SLOT	equipSlot;	// �� ���Կ� ����!
				TSN_ITEM		item_sn;	// �� ���Կ� �ִ� ��������..
			};

			MSGDEF_TYPE TMSG;
		}

		namespace EFFSET_ANS
		{
			enum _ID { ID = MSGDEF_ID(EFFSET_ANS) };

			struct DEF
			{
				MSGDEF_NAME(EFFSET_ANS);
				BYTE	result;				
			};

			MSGDEF_TYPE TMSG;
		}


#pragma pack(pop)


	};	// namespace INVENTORY
};	// namespace PM

#undef __CAT
#undef __MC
