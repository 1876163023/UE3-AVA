/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetOptionSettings.uc

	Description: 비디오, 오디오, 조작, 게임 옵션설정창의 루트 프로바이더

***/
class UIDataProvider_AvaNetOptionSettings extends avaUIParamDataProvider
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

defaultproperties
{
	WriteAccessType=ACCESS_WriteAll

	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetOptionSettingVideo',ProviderTag=Video))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetOptionSettingAudio',ProviderTag=Audio))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetOptionSettingGame',ProviderTag=Game))
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetOptionSettingMouse',ProviderTag=Mouse))
}