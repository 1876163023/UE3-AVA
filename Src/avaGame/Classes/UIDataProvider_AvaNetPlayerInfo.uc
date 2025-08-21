/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataProvider_AvaNetPlayerInfo.uc

	Description: 자기자신, 친구, 차단, 대기자, 클랜원등의 정보를 나타냄

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
	 * 파라메터들을 모두 업데이트 한후에 필요한 처리가 있다면 추가
	 * 예) 무기 인벤토리의 리스트 인덱스를 파라메터로 받았음 -(PostUpdateParamters)-> ItemID를 계산한다
	 */
	virtual void PostUpdateParamters( UBOOL bParmChanged );

	/**
	 * TargetWidget을 업데이트 (활성/비활성화, 숨김/보임, 체크/체크해제 등등)
	 */
	void UpdateWidget( const FString& FieldName, UUIObject* TargetWidget );
}

var()	input	int		ListIndex;			// 선택한 리스트의 인덱스
var()	input	bool	bWaiting;			// 대기자 리스트를 선택했음
var()	input	bool	bBuddy;				// 친구 리스트를 선택했음
var()	input	bool	bBlock;				// 차단 리스트를 선택했음
var()	input	bool	bClan;				// 클랜원 리스트를 선택했음

var() EPlayerInfoCategory PlayerInfoCat;

var	int		AccountID;			// 내부에서 사용할 AccountID, ListIndex가 들어오면 계산해서 저장해둔다

/** 플레이어 정보를 미리 계산해두어 활성화/비활성화, 보임/안보임 처리( UpdateWidget )에 사용한다 */
var	bool	bMyPlayer;
var	bool	bOtherPlayer;		// !bBuddyPlayer && !bBlockPlayer && !bClanPlayer
var	bool	bBuddyPlayer;		
var	bool	bBlockPlayer;
var	bool	bMyClanPlayer;
var	bool	bOtherClanPlayer;

defaultproperties
{
}