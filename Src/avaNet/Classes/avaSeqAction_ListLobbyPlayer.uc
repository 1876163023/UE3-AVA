class avaSeqAction_ListLobbyPlayer extends avaSeqAction;



var() string UserID;
var() string UserPassword;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().ListLobbyPlayer();
}


defaultproperties
{
	ObjName="(Lobby) List Player"

    VariableLinks.Empty
}

