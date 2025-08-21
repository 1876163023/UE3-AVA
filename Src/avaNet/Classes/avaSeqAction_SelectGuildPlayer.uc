class avaSeqAction_SelectGuildPlayer extends avaSeqAction;


var() int ListIndex;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().SelectGuildPlayer(ListIndex);
}


defaultproperties
{
	ObjName="(Guild) Select Player"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Player List Index",PropertyName=ListIndex))
}

