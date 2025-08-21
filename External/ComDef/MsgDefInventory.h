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
			// 인벤 관련..
			ID_ENTER_NTF,				/// 인벤토리로 입장.
			ID_LEAVE_NTF,

			ID_EQUIPSET_REQ,			/// 군장 아이탬 장착하기
			ID_EQUIPSET_ANS,			
			ID_WEAPONSET_REQ,			/// 무기 아이탬 장착하기
			ID_WEAPONSET_ANS,			

			ID_CUSTOMSET_REQ,			/// 커스텀 아이템 장착
			ID_CUSTOMSET_ANS,

			ID_ITEM_BUY_REQ,			/// 샾에서 아이탬을 구입
			ID_ITEM_BUY_ANS,			

			ID_ITEM_REFUND_REQ,			/// 아이탬을 팔기
			ID_ITEM_REFUND_ANS,			

			ID_ITEM_GIFT_REQ,			/// 샾에서 아이탬을 선물하기..
			ID_ITEM_GIFT_ANS,

			ID_REPAIR_REQ,				/// 아이탬 수리 요청
			ID_REPAIR_ANS,

			ID_ITEM_DELETE_NTF,			/// 서버에서 보내는 아이탬 삭제 통지
			ID_EQUIPSET_NTF,			/// 서버에서 보내는 아이탬 장착 통지
			ID_WEAPONSET_NTF,	

			ID_CHANGE_CLASS_NTF,

			ID_CONVERT_RIS_REQ,
			ID_CONVERT_RIS_ANS,

			ID_UPDATE_GAUGE_NTF,

			ID_CASHITEM_BUY_REQ,			// 캐쉬 아이템
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

				TID_EQUIP_SLOT	equipSlot;	// 이 군장 슬롯에 장착!
				TSN_ITEM		item_sn;			// 이 슬롯에 있는 아이탬을..
				
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

				TID_EQUIP_SLOT	equipSlot;	// 이 군장 슬롯에 장착!
				TSN_ITEM		item_sn;	// 이 슬롯에 있는 아이탬을..
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

				TSN_ITEM		item_sn;		// 이 슬롯에 있는 아이탬을..				
				BYTE			slot;			// 장착할 포지션..
				TID_ITEM		customItem_id;	// 장착할 커스텀 아이탬의 아이디.. INVALID 면 장착 해지..				
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
				TSN_ITEM		item_sn;	// 이 슬롯에 있는 아이탬을..
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

				ITEM_INFO item;		// 팔 아이탬

				//TPOINT	point;	 // 돈...흠...
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

				ITEM_INFO item;		// 팔 아이탬

				//TPOINT	point;	 // 돈...흠...

				TID_ACCOUNT idAccount; // 선물할 넘.
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

				TSN_ITEM		item_sn;		// 이 슬롯에 있는 아이탬을..	
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

				TID_EQUIP_SLOT	equipSlot;	// 이 슬롯에 장착!
				TSN_ITEM		item_sn;	// 이 슬롯에 있는 아이탬을..
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
