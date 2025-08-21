class avaSeqCond_InvenDoIHaveCustomInSlot extends avaSeqCondition;


var() int InvenSlot;
var() int SlotIndex;


event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().DoIHaveCustomItemInSlot(InvenSlot, SlotIndex) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Inventory) Do I Have Custom in Slot"

	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No");

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Inven Slot",PropertyName=InvenSlot))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Slot Index",PropertyName=SlotIndex))
}

