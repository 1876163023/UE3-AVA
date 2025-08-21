class avaSeqAction_LeaveChannel extends avaSeqAction;




event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().LeaveChannel();
}


defaultproperties
{
	ObjName="(Lobby) Leave"

    VariableLinks.Empty
}

