/**
 * @ingroup _AVAGame
 *
 * 캐쉬아이템 팝업 내용 출력 ( 사려고하는 캐쉬 아이템 아이콘, 상세 설명, ... )
 *
 *
 * @date 2007-11-12
 *
 * @author YTS
 *
 * @todo	캐쉬아이템 아이콘, 이름, 상세내역, 가격
 *			서버에 구매 요청, 캐쉬충전 요청, 구매한 아이템 장착
 *
 */
class UIDataProvider_AvaNetCashItemPopup extends avaUIParamDataProvider
	native(inherit)
	transient;

cpptext
{
	/**
	 * 파라메터들을 모두 업데이트 한후에 필요한 처리가 있다면 추가
	 * 예) 무기 인벤토리의 리스트 인덱스를 파라메터로 받았음 -(PostUpdateParamters)-> ItemID를 계산한다
	 */
	virtual void PostUpdateParamters( UBOOL bParmChanged );

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
}

/** 선택된 아이템 인덱스 */
var input int		SelectedItemListIndex;				//! 선택된 상점 아이템.

/*! @brief 선택된 내구도 리스트 인덱스.
	@note
		Shop에서 선택된 값이 변경될 경우에 기존의 내구도 값에 
		해당하는 Money값을 출력하기 위해서 필요하다.
*/
var input int		DurabilityListIndex;

var input string	Result;								//! Effect Buy Event결과값(물론 "ok"인 경우에만 아래 3개의 값이 유효하다)
var input string	ItemType;							//! effect, equip, weapon
var input int		EquipSlot;							//! 장비슬롯.
var input int		InvenSlot;							//! 인벤토리 슬롯.(ListIndex)

/** 이벤트 트리거 */
var input bool		bBuyCashItemRequestTrigger;			// 서버로 아이템 구매 요청을 보냅니다
var input bool		bGiftCashItemRequestTrigger;		// 서버로 아이템 선물 요청을 보냅니다.
var input bool		bChargeCashRequestTrigger;			// 충전을 하기위해 Web충전 창을 엽니다
var input bool		bUseRequestTrigger;					// 구매한 아이템을 바로 사용합니다.
var input bool		bEquipRequestTrigger;				// 구매한 게임아이템을 바로 장착합니다

defaultproperties
{
	WriteAccessType=ACCESS_WriteAll
	SelectedItemListIndex=INDEX_NONE
}