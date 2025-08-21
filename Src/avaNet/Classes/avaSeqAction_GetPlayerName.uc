class avaSeqAction_GetPlayerName extends avaSeqAction;

var() int PlayerListIndex;
var() string PlayerName;

event Activated()
{
	if ( InputLinks[0].bHasImpulse )
	{
		PlayerName = class'avaNetHandler'.static.GetAvaNetHandler().GetMyURLString("Name");
		OutputLinks[0].bHasImpulse = true;
	}
	else if (InputLinks[1].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().GetLobbyPlayerName( PlayerListIndex, PlayerName ) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if (InputLinks[2].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().GetFriendPlayerName( PlayerListIndex, PlayerName ) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if (InputLinks[3].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().GetBlockedPlayerName( PlayerListIndex, PlayerName ) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
}


defaultproperties
{
	ObjName="GetPlayerName"

	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Self")
	InputLinks(1)=(LinkDesc="Lobby")
	InputLinks(2)=(LinkDesc="Friend")
	InputLinks(3)=(LinkDEsc="Blocked")

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="ListIndex",PropertyName=PlayerListIndex))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Name",PropertyName=PlayerName,bWriteable=true))

	ObjClassVersion=1
}