class avaSeqAction_GetLastWhisperedPlayerName extends avaSeqAction;

var() string LastPlayerName;

event Activated()
{
	LastPlayerName = class'avaGame.avaNetHandler'.static.GetAvaNetHandler().GetLastWhisperedPlayerName();
}


defaultproperties
{
	ObjName="Get Name (LastWhisperedPlayer)"

    VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="NickName",bWriteable=true,PropertyName=LastPlayerName)
}

