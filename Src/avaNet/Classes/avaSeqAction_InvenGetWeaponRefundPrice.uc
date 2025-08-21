class avaSeqAction_InvenGetWeaponRefundPrice extends avaSeqAction;

var() int InvenSlot;
var int Money;
var int Cash;

event Activated()
{
	if ( class'avaNetRequest'.static.GetAvaNetRequest().InvenGetWeaponRefundPrice(InvenSlot, Money, Cash) )
	{
		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="(Inventory) Get Weapon Refund Price"

	InputLinks(0)=(LinkDesc="In")

	OutputLinks(0)=(LinkDesc="Succeed")
	OutputLinks(1)=(LinkDesc="Fail")

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Money",bWriteable=true,PropertyName=Money)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Cash",bWriteable=true,PropertyName=Cash)
}