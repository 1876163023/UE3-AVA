class avaSeqAction_InvenGetCustomRefundPrice extends avaSeqAction;

var() int InvenSlot;
var() int CustomSlot;
var int Money;
var int Cash;

event Activated()
{
	Local SeqVar_Int IntVar;

	foreach LinkedVariables(class'SeqVar_Int', IntVar, "Custom Slot")
	{
		CustomSlot = IntVar.IntValue;
	}
	`log("avaSeqAction_InvenGetCustomRefundPrice : InvenSlot =" @ InvenSlot @ ", CustomSlot =" @ CustomSlot);

	if ( class'avaNetRequest'.static.GetAvaNetRequest().InvenGetCustomRefundPrice(InvenSlot, CustomSlot, Money, Cash) )
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
	ObjName="(Inventory) Get Custom Refund Price"

	InputLinks(0)=(LinkDesc="In")

	OutputLinks(0)=(LinkDesc="Succeed")
	OutputLinks(1)=(LinkDesc="Fail")

	bAutoActivateOutputLinks=false

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Custom Slot",PropertyName=CustomSlot)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="Money",bWriteable=true,PropertyName=Money)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Cash",bWriteable=true,PropertyName=Cash)
}