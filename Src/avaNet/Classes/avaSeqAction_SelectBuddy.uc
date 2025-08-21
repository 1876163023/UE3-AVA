class avaSeqAction_SelectBuddy extends avaSeqAction;


var() int ListIndex;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().SelectBuddy(ListIndex);
}


defaultproperties
{
	ObjName="(Buddy) Select Buddy"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Player List Index",PropertyName=ListIndex))
}

