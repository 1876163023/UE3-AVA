class avaSeqAction_GetItemId extends avaSeqAction;

enum EAVANETListIndexType
{
	AVANET_LISTINDEX_WeaponInven,
	AVANET_LISTINDEX_EquipInven,
	AVANET_LISTINDEX_ItemList,
	AVANET_LISTINDEX_CustomItemList,
};

var() int ListIndex;
var() int ItemId;
var() EAVANETListIndexType ListIndexType;

event Activated()
{
	Local bool bResult;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetItemId( ListIndexType, ListIndex, ItemId);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjName="Get ItemId"

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Item ID",bWriteable=true,PropertyName=ItemId)

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed");
}