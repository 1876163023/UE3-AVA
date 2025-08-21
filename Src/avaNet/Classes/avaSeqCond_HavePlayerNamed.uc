class avaSeqCond_HavePlayerNamed extends avaSeqCondition;

var() string PlayerName;
var() int FindIndex;

event Activated()
{
	FindIndex = INDEX_NONE;
	if (InputLinks[0].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().HaveFriendPlayerNamed(PlayerName, FindIndex) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if (InputLinks[1].bHasImpulse)
	{
		if ( class'avaNetHandler'.static.GetAvaNetHandler().HaveBlockedPlayerNamed(PlayerName, FindIndex) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;
	}
	else if ( InputLinks[2].bHasImpulse )
	{
			if ( class'avaNetHandler'.static.GetAvaNetHandler().HaveClanMemberNamed(PlayerName, FindIndex) )
			OutputLinks[0].bHasImpulse = true;
		else
			OutputLinks[1].bHasImpulse = true;

	}
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Lobby) Have Player Named"

	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="Friend")
	InputLinks(1)=(LinkDesc="Blocked")
	InputLinks(2)=(LinkDesc="Clan")

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No");

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Name",PropertyName=PlayerName)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="ListIndex",PropertyName=FindIndex,bWriteable=true)

	ObjClassVersion = 3
}

