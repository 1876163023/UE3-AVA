class avaSeqAction_InvenSetCustom extends avaSeqAction;

var() int InvenSlot;
var() int CustomSlot;

event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenSetCustom(InvenSlot, CustomSlot);
	}
	else if (InputLinks[1].bHasImpulse)
	{
		class'avaNet.avaNetRequest'.static.GetAvaNetRequest().InvenUnsetCustom(InvenSlot, CustomSlot);
	}
}

defaultproperties
{
	ObjName="(Inventory) Set Custom"

	InputLinks(0)=(LinkDesc="Set")
	InputLinks(1)=(LinkDesc="Unset")

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inventory Slot",PropertyName=InvenSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Custom Slot",PropertyName=CustomSlot)

	ObjClassVersion=1
}