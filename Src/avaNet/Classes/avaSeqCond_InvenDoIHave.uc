class avaSeqCond_InvenDoIHave extends avaSeqCondition;


var() int ListIndex;



event Activated()
{
	if (InputLinks[0].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().DoIHaveItem(ListIndex) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if (InputLinks[1].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().DoIHaveCustomItem(ListIndex) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Inventory) Do I Have"

	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Weapon/Equip")
	InputLinks(1)=(LinkDesc="Custom Item");

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No");

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="List Index",PropertyName=ListIndex)
}

