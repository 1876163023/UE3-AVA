/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetReadyRoom.uc

	Description: ���� ���� ��Ʈ ���ι��̴�

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
	 * �Ķ���͵��� ��� ������Ʈ ���Ŀ� �ʿ��� ó���� �ִٸ� �߰�
	 * ��) ���� �κ��丮�� ����Ʈ �ε����� �Ķ���ͷ� �޾��� -(PostUpdateParamters)-> ItemID�� ����Ѵ�
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
	 * TargetWidget�� ������Ʈ (Ȱ��/��Ȱ��ȭ, ����/����, üũ/üũ���� ���)
	 *
	 * @param	bVisible - �� ������ ���ι��̴��� ����ϴ� Owner�� ���̰ų� ������������ ����Ų��.
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