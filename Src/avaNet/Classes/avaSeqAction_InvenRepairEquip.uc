class avaSeqAction_InvenRepairEquip extends avaSeqAction;

var() int InvenSlot;

event Activated()
{
	class'avaNetRequest'.static.GetAvaNetRequest().InvenRepairEquip(InvenSlot);
}


defaultproperties
{
	ObjName="(Inventory) Repair Equip"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
}