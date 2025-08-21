class avaSeqAction_InvenSetWeapon extends avaSeqAction;


var() int EquipSlot;
var() int InvenSlot;


event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenSetWeapon(EquipSlot, InvenSlot);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenUnsetWeapon(EquipSlot, InvenSlot);
	}
}


defaultproperties
{
	ObjName="(Inventory) Set Weapon"

	InputLinks(0)=(LinkDesc="Set")
	InputLinks(1)=(LinkDesc="Unset")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Equip Slot",PropertyName=EquipSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inventory Slot",PropertyName=InvenSlot)

	ObjClassVersion=1
}

