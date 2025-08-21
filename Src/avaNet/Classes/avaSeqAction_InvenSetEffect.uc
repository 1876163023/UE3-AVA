class avaSeqAction_InvenSetEffect extends avaSeqAction;


var() int EquipSlot;
var() int InvenSlot;


event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenSetEffect(EquipSlot, InvenSlot);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenUnsetEffect(EquipSlot, InvenSlot);
	}
}


defaultproperties
{
	ObjName="(Inventory) Set Effect Item"

	InputLinks(0)=(LinkDesc="Set")
	InputLinks(1)=(LinkDesc="Unset")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Equip Slot",PropertyName=EquipSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inventory Slot",PropertyName=InvenSlot)

	ObjClassVersion=1
}

