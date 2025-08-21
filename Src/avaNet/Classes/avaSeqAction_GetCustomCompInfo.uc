class avaSeqAction_GetCustomCompInfo extends avaSeqAction;

var() int InvenSlot;
var() int CustomListIndex;
var() int AlterSlot;
var() int AlterItemID;
var() int CompPrice;

event Activated()
{
	Local bool bResult;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetCustomCompInfo(InvenSlot, CustomListIndex, AlterSlot,AlterItemID, CompPrice);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjName="(Inventory) Get Compensation Info"

	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Custom ListIndex",PropertyName=CustomListIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Alter Slot",PropertyName=AlterSlot))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Alter ItemID",PropertyName=AlterItemID))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Comp Price",PropertyName=CompPrice))

	ObjClassVersion=2
}