/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetShopItems.uc

	Description: Supplys list of shop items to the UIDataStore_AvaNet.

***/
class UIDataProvider_AvaNetShopItems extends UIDataProvider
	native(inherit)
	implements(UIListElementCellProvider)
	transient;



cpptext
{
/* === IUIListElement interface === */

	/**
	 * Retrieves the list of tags that can be bound to individual cells in a single list element.
	 *
	 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
	 */
	virtual void GetElementCellTags( TMap<FName,FString>& out_CellTags );

	/**
	 * Retrieves the field type for the specified cell.
	 *
	 * @param	CellTag				the tag for the element cell to get the field type for
	 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
	 *
	 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
	 */
	virtual UBOOL GetCellFieldType( const FName& CellTag, BYTE& out_CellFieldType )
	{
		//@fixme joeg - implement this
		out_CellFieldType = DATATYPE_Property;
		return TRUE;
	}

	/**
	 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
	 *
	 * @param	CellTag			the tag for the element cell to resolve the value for
	 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
	 *							do not provide unique UIListElement objects for each element.
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
	 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
	 *							to a data collection.
	 */
	virtual UBOOL GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );

/* === UIDataProvider interface === */

	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
	{
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeapon")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponP1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponP2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponP3")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponP4")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponR1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponR2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponR3")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponR4")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponS1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponS2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponS3")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopWeaponS4")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquip")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipH1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipH11")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipH12")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipH2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipH3")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipC1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipC2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipA1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipA2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipB1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipB3")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipW1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipW2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipW3")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipT1")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipT2")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipE")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipG")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipK")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipBT")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipBD")), DATATYPE_Collection );

		// OB ½ºÆå ( ¾ó±¼, Çï¸ä, ¹æÅºº¹, A,B,C,D,E )
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipFace")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipHelmet")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipArmor")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipSlotA")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipSlotB")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipSlotC")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipSlotD")), DATATYPE_Collection );
		new(out_Fields) FUIDataProviderField( FName(TEXT("ShopEquipSlotE")), DATATYPE_Collection );
	}

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );
}

