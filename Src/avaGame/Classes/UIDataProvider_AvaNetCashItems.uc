/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetCashItems.uc

	Description: 유료아이템의 루트 데이터 프로바이더

***/
class UIDataProvider_AvaNetCashItems extends avaUIParamDataProvider
	native(inherit)
	implements(UIListElementProvider)
	transient;

cpptext
{
/* === IUIListElement interface === */

	/**
	 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
	 *
	 * @return	the list of tags supported by this element provider which correspond to list element data.
	 */
	virtual TArray<FName> GetElementProviderTags() { TArray<FName> OutProviderTags; return OutProviderTags; }

	virtual INT GetElementCount( FName FieldName ) { return 1; }

	virtual UBOOL GetListElements( FName FieldName, TArray<INT>& out_Elements );

	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellSchemaProvider( FName FieldName );

	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellValueProvider( FName FieldName, INT ListIndex ) { return GetElementCellSchemaProvider(FieldName); }


/* === UIDataProvider interface === */

	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields );

	/**
	 * 파라메터들을 모두 업데이트 한후에 필요한 처리가 있다면 추가
	 * 예) 무기 인벤토리의 리스트 인덱스를 파라메터로 받았음 -(PostUpdateParamters)-> ItemID를 계산한다
	 */
	virtual void PostUpdateParamters( UBOOL bParmChanged );

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );

	/**
	 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	FieldValue		the value to store for the property specified.
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex=INDEX_NONE );
}

var input int			SelectedInvenItemListIndex;	//! 선택한 인벤토리 아이템.

/** Request Trigger */
var input bool			bUseCashItemTrigger;
var input bool			bBuyCashItemTrigger;
var input bool			bGiveCashItemTrigger;
var input bool			bTakeCashItemTrigger;
var input bool			bChargeCashTrigger;			//! 캐쉬 충전.
var input bool			bGetCashTrigger;			//! web에서 cash정볼를 얻어온다.

defaultproperties
{
	WriteAccessType=ACCESS_WriteAll

	/*! 함수의 사용처와 설명.
		---------------------------
		CashItems (지금 클래스)
			CashItemInventory
			CashItemShop
			CashItemPopup
		---------------------------

		// F7을 누르면 에디터에서 나오는 Field를 등록하는 함수.
		::GetSupportedDataFields

		-- Collection 처리.(리스트 윈도우)
			// 위에 Field값에 해당하는 Cell들의 Index를 얻어주는 함수.
			::GetListElements
			
			// 각 Cell의 Column의 이름을 등록하는 함수.
			::GetElementCellTags
			
			// CellTags에서 등록된 각 Tag에 해당하는 Value를 얻는 함수.
			::GetCellFieldValue

		-- Property 처리.(일반 윈도우)
		// DataField값에 해당하는 Value값을 얻어주는 함수.
		::GetField

		-- Kismet에서 bool형 Trigger변수를 바꾸면 호출되는 EventHandler함수.(OnTrigger ??)
		// 이벤트 함수라고 생각하자.
		::PostUpdateParamters
	*/

	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemInventory',ProviderTag=Inventory))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemShop',ProviderTag=Shop))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemPopup',ProviderTag=Popup))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemDurability',ProviderTag=Durability))
}