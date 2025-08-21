class avaSeqAction_RoomKickPlayer extends avaSeqAction;



var() string Nickname;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().RoomKickPlayer(Nickname);
}


defaultproperties
{
	ObjName="(Room) Kick Player"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))
}

