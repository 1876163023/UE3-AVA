class avaSeqAction_InvenConvertRIS extends avaSeqAction;

var() int InvenSlot;

event Activated()
{
	class'avaNetRequest'.static.GetAvaNetRequest().InvenConvertRIS(InvenSlot);
}


defaultproperties
{
	ObjName="(Inventory) Convert RIS"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
}