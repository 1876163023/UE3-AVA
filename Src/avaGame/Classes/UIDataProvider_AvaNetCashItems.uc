/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetCashItems.uc

	Description: ����������� ��Ʈ ������ ���ι��̴�

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
	 * �Ķ���͵��� ��� ������Ʈ ���Ŀ� �ʿ��� ó���� �ִٸ� �߰�
	 * ��) ���� �κ��丮�� ����Ʈ �ε����� �Ķ���ͷ� �޾��� -(PostUpdateParamters)-> ItemID�� ����Ѵ�
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

var input int			SelectedInvenItemListIndex;	//! ������ �κ��丮 ������.

/** Request Trigger */
var input bool			bUseCashItemTrigger;
var input bool			bBuyCashItemTrigger;
var input bool			bGiveCashItemTrigger;
var input bool			bTakeCashItemTrigger;
var input bool			bChargeCashTrigger;			//! ĳ�� ����.
var input bool			bGetCashTrigger;			//! web���� cash������ ���´�.

defaultproperties
{
	WriteAccessType=ACCESS_WriteAll

	/*! �Լ��� ���ó�� ����.
		---------------------------
		CashItems (���� Ŭ����)
			CashItemInventory
			CashItemShop
			CashItemPopup
		---------------------------

		// F7�� ������ �����Ϳ��� ������ Field�� ����ϴ� �Լ�.
		::GetSupportedDataFields

		-- Collection ó��.(����Ʈ ������)
			// ���� Field���� �ش��ϴ� Cell���� Index�� ����ִ� �Լ�.
			::GetListElements
			
			// �� Cell�� Column�� �̸��� ����ϴ� �Լ�.
			::GetElementCellTags
			
			// CellTags���� ��ϵ� �� Tag�� �ش��ϴ� Value�� ��� �Լ�.
			::GetCellFieldValue

		-- Property ó��.(�Ϲ� ������)
		// DataField���� �ش��ϴ� Value���� ����ִ� �Լ�.
		::GetField

		-- Kismet���� bool�� Trigger������ �ٲٸ� ȣ��Ǵ� EventHandler�Լ�.(OnTrigger ??)
		// �̺�Ʈ �Լ���� ��������.
		::PostUpdateParamters
	*/

	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemInventory',ProviderTag=Inventory))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemShop',ProviderTag=Shop))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemPopup',ProviderTag=Popup))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetCashItemDurability',ProviderTag=Durability))
}