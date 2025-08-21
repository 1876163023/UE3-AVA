/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetReadyRoom.uc

	Description: 대기방 관련 루트 프로바이더

***/
class UIDataProvider_AvaNetReadyRoom extends avaUIParamDataProvider
	native(inherit)
	transient;

cpptext
{

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
	virtual void PostUpdateParamters( UBOOL bParmChanged ) {}

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
	 * TargetWidget을 업데이트 (활성/비활성화, 숨김/보임, 체크/체크해제 등등)
	 *
	 * @param	bVisible - 이 데이터 프로바이더를 사용하는 Owner가 보이거나 숨겨져야함을 가리킨다.
	 *
	 * @return
	 */
	void UpdateWidget( const FString& FieldName, UUIObject* TargetWidget );
}

defaultproperties
{
	//WriteAccessType=ACCESS_WriteAll
	InternalProviderData.Add((ProviderClass=class'UIDataProvider_AvaNetReadyRoomPlayers',ProviderTag=Players))
}