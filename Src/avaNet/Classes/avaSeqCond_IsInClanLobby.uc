class avaSeqCond_IsInClanLobby extends avaSeqCondition;

event Activated()
{
	if ( class'avaNetHandler'.static.GetAvaNetHandler().IsInClanLobby() )
	{
		OutputLinks[0].bHasImpulse = true;
	}
	else
	{
		OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Lobby)Is In ClanLobby"

	OutputLinks(0)=(LinkDesc="True")
	OutputLinks(1)=(LinkDesc="False")

	VariableLinks.Empty
}