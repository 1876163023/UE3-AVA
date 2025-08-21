class avaSeqAction_SelectBlock extends avaSeqAction;


var() int ListIndex;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().SelectBlock(ListIndex);
}


defaultproperties
{
	ObjName="(Block) Select Player"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Player List Index",PropertyName=ListIndex))
}

