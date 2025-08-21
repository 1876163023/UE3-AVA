class avaSeqCond_CheckRefund extends avaSeqCondition;

var() int InvenSlot;
var() int CustomSlotIndex;

event Activated()
{
	Local bool bResult;

	if( InputLinks[0].bHasImpulse )
	{
		bResult = class'avaNetHandler'.static.GetAvaNetHandler().CheckWeaponRefundCond(InvenSlot);
	}
	else if ( InputLinks[1].bHasImpulse )
	{
		bResult = class'avaNetHandler'.static.GetAvaNetHandler().CheckEquipRefundCond(InvenSlot);
	}
	else if ( InputLinks[2].bHasImpulse )
	{
		bResult = class'avaNetHandler'.static.GetAvaNetHandler().CheckCustomRefundCond(InvenSlot, CustomSlotIndex);
	}

	OutputLinks[ bResult ? 0 : 1 ].bHasImpulse = true;
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Inventory) Check RefundCond"
	bAutoActivateOutputLinks=false;

	InputLinks(0)=(LinkDesc="Weapon")
	InputLinks(1)=(LinkDesc="Equip")
	InputLinks(2)=(LinkDesc="Custom")

	OutputLinks(0)=(LinkDesc="Valid")
	OutputLinks(1)=(LinkDesc="Invalid")

	VariableLinks.Empty

	VariableLinks.Add((ExpectedType=class'SeqVar_Int', LinkDesc="Inven Slot", PropertyName=InvenSlot))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int', LinkDesc="Custom SlotIndex", PropertyName=CustomSlotIndex, bHidden=true))
}