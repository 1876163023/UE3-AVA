class avaSeqAction_GetSelectedPlayerInfo extends avaSeqAction;


var() string		NickName;
var() string		GuildName;
var() int			Level;
var() int			WinCount;
var() int			DefeatCount;
var() int			DisconnectCount;
var() int			KillCount;
var() int			DeathCount;

event Activated()
{
	if( InputLinks[0].bHasImpulse )
	{
		if( class'avaNetHandler'.static.GetAvaNetHandler().GetSelectedLobbyPlayerInfo(NickName, GuildName, Level, WinCount, DefeatCount, DisconnectCount, KillCount, DeathCount))
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if ( InputLinks[1].bHasImpulse )
	{
		if( class'avaNetHandler'.static.GetAvaNetHandler().GetSelectedFriendPlayerInfo(NickName, GuildName, Level, WinCount, DefeatCount, DisconnectCount, KillCount, DeathCount))
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;

	}
	else if ( InputLinks[2].bHasImpulse )
	{
		if( class'avaNetHandler'.static.GetAvaNetHandler().GetSelectedBlockedPlayerInfo(NickName, GuildName, Level, WinCount, DefeatCount, DisconnectCount, KillCount, DeathCount))
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;

	}
	else if ( InputLinks[3].bHasImpulse )
	{
		if( class'avaNetHandler'.static.GetAvaNetHandler().GetSelectedGuildPlayerInfo(NickName, GuildName, Level, WinCount, DefeatCount, DisconnectCount, KillCount, DeathCount))
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}	
}


defaultproperties
{
	ObjName="Get Selected PlayerInfo"

	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Lobby")
	InputLinks(1)=(LinkDesc="Friend")
	InputLinks(2)=(LinkDesc="Block")
	InputLinks(3)=(LinkDesc="Guild")

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nick",PropertyName=NickName,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Guild",PropertyName=GuildName,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Lvl",PropertyName=Level,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="nWin",PropertyName=WinCount,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="nDef",PropertyName=DefeatCount,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="nDis",PropertyName=DisconnectCount,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="nKill",PropertyName=KillCount,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="nDeath",PropertyName=DeathCount,bWriteable=true))

	ObjClassVersion=1
}