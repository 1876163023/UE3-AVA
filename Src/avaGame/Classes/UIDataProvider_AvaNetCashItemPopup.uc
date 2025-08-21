/**
 * @ingroup _AVAGame
 *
 * ĳ�������� �˾� ���� ��� ( ������ϴ� ĳ�� ������ ������, �� ����, ... )
 *
 *
 * @date 2007-11-12
 *
 * @author YTS
 *
 * @todo	ĳ�������� ������, �̸�, �󼼳���, ����
 *			������ ���� ��û, ĳ������ ��û, ������ ������ ����
 *
 */
class UIDataProvider_AvaNetCashItemPopup extends avaUIParamDataProvider
	native(inherit)
	transient;

cpptext
{
	/**
	 * �Ķ���͵��� ��� ������Ʈ ���Ŀ� �ʿ��� ó���� �ִٸ� �߰�
	 * ��) ���� �κ��丮�� ����Ʈ �ε����� �Ķ���ͷ� �޾��� -(PostUpdateParamters)-> ItemID�� ����Ѵ�
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

/** ���õ� ������ �ε��� */
var input int		SelectedItemListIndex;				//! ���õ� ���� ������.

/*! @brief ���õ� ������ ����Ʈ �ε���.
	@note
		Shop���� ���õ� ���� ����� ��쿡 ������ ������ ���� 
		�ش��ϴ� Money���� ����ϱ� ���ؼ� �ʿ��ϴ�.
*/
var input int		DurabilityListIndex;

var input string	Result;								//! Effect Buy Event�����(���� "ok"�� ��쿡�� �Ʒ� 3���� ���� ��ȿ�ϴ�)
var input string	ItemType;							//! effect, equip, weapon
var input int		EquipSlot;							//! ��񽽷�.
var input int		InvenSlot;							//! �κ��丮 ����.(ListIndex)

/** �̺�Ʈ Ʈ���� */
var input bool		bBuyCashItemRequestTrigger;			// ������ ������ ���� ��û�� �����ϴ�
var input bool		bGiftCashItemRequestTrigger;		// ������ ������ ���� ��û�� �����ϴ�.
var input bool		bChargeCashRequestTrigger;			// ������ �ϱ����� Web���� â�� ���ϴ�
var input bool		bUseRequestTrigger;					// ������ �������� �ٷ� ����մϴ�.
var input bool		bEquipRequestTrigger;				// ������ ���Ӿ������� �ٷ� �����մϴ�

defaultproperties
{
	WriteAccessType=ACCESS_WriteAll
	SelectedItemListIndex=INDEX_NONE
}