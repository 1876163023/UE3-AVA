class avaSeqAction_InvenRepairWeapon extends avaSeqAction;

var() int InvenSlot;

event Activated()
{
	class'avaNetRequest'.static.GetAvaNetRequest().InvenRepairWeapon(InvenSlot);
}


defaultproperties
{
	ObjName="(Inventory) RepairWeapon"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
}