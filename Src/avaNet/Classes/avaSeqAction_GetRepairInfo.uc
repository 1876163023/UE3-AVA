class avaSeqAction_GetRepairInfo extends avaSeqAction;

var() int InvenSlot;
var() int RepairPrice;
vaR() bool bAfford;

event Activated()
{
	Local bool bResult;
	Local byte byteAfford;

	if (InputLinks[0].bHasImpulse)
	{
		bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetWeaponRepairInfo(InvenSlot, RepairPrice, byteAfford);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetEquipRepairInfo(InvenSlot, RepairPrice, byteAfford);
	}

	bAfford = bool(byteAfford);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjName="(Inventory) GetRepairInfo"

	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Weapon")
	InputLinks(1)=(LinkDesc="Equip")

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="RepairPrice",PropertyName=RepairPrice)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Bool',LinkDesc="bAfford",PropertyName=bAfford)

	ObjClassVersion=1
}