class avaSeqAction_InvenRepair extends avaSeqAction;


var() int InvenSlot;


event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenRepairWeapon(InvenSlot);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenRepairEquip(InvenSlot);
	}
}


defaultproperties
{
	ObjName="(Inventory) Repair"

	InputLinks(0)=(LinkDesc="Weapon")
	InputLinks(1)=(LinkDesc="Equip")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inventory Slot",PropertyName=InvenSlot)

	ObjClassVersion=1
}

