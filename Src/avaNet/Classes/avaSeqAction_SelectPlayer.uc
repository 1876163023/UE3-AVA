class avaSeqAction_SelectPlayer extends avaSeqAction;


var() int ListIndex;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().SelectPlayer(ListIndex);
}


defaultproperties
{
	ObjName="(Lobby/Room) Select Player"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Player List Index",PropertyName=ListIndex))
}

