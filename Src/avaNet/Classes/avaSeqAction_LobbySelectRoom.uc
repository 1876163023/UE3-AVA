class avaSeqAction_LobbySelectRoom extends avaSeqAction;


var() int ListIndex;



event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().LobbySelectRoom(ListIndex);
}


defaultproperties
{
	ObjName="(Lobby) Select Room"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Room List Index",PropertyName=ListIndex))
}

