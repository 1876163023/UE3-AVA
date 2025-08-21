class avaSeqAction_GetCustomItemID extends avaSeqAction;

var() int InvenSlot;
var() int CustomSlotIndex;
var() int ItemID;

event Activated()
{
	Local bool bResult;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetCustomItemID( InvenSlot, CustomSlotIndex, ItemID);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjName="Get CustomItemID"

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Custom Slot",PropertyName=CustomSlotIndex)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Item ID",bWriteable=true,PropertyName=ItemID)

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed");
}