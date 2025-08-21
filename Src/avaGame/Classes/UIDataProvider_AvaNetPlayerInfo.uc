/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetPlayerInfo.uc

	Description: �ڱ��ڽ�, ģ��, ����, �����, Ŭ�������� ������ ��Ÿ��

***/
class UIDataProvider_AvaNetPlayerInfo extends avaUIParamDataProvider
	native(inherit)
	transient;

enum EPlayerInfoCategory
{
	PLAYERINFOCAT_Waiting,
	PLAYERINFOCAT_Buddy,
	PLAYERINFOCAT_Block,
	PLAYERINFOCAT_Clan,
};

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
	 * �Ķ���͵��� ��� ������Ʈ ���Ŀ� �ʿ��� ó���� �ִٸ� �߰�
	 * ��) ���� �κ��丮�� ����Ʈ �ε����� �Ķ���ͷ� �޾��� -(PostUpdateParamters)-> ItemID�� ����Ѵ�
	 */
	virtual void PostUpdateParamters( UBOOL bParmChanged );

	/**
	 * TargetWidget�� ������Ʈ (Ȱ��/��Ȱ��ȭ, ����/����, üũ/üũ���� ���)
	 */
	void UpdateWidget( const FString& FieldName, UUIObject* TargetWidget );
}

var()	input	int		ListIndex;			// ������ ����Ʈ�� �ε���
var()	input	bool	bWaiting;			// ����� ����Ʈ�� ��������
var()	input	bool	bBuddy;				// ģ�� ����Ʈ�� ��������
var()	input	bool	bBlock;				// ���� ����Ʈ�� ��������
var()	input	bool	bClan;				// Ŭ���� ����Ʈ�� ��������

var() EPlayerInfoCategory PlayerInfoCat;

var	int		AccountID;			// ���ο��� ����� AccountID, ListIndex�� ������ ����ؼ� �����صд�

/** �÷��̾� ������ �̸� ����صξ� Ȱ��ȭ/��Ȱ��ȭ, ����/�Ⱥ��� ó��( UpdateWidget )�� ����Ѵ� */
var	bool	bMyPlayer;
var	bool	bOtherPlayer;		// !bBuddyPlayer && !bBlockPlayer && !bClanPlayer
var	bool	bBuddyPlayer;		
var	bool	bBlockPlayer;
var	bool	bMyClanPlayer;
var	bool	bOtherClanPlayer;

defaultproperties
{
}