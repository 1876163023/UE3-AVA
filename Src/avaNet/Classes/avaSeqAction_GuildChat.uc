class avaSeqAction_GuildChat extends avaSeqAction;


var() string Message;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().GuildChat(Message);
}


defaultproperties
{
	ObjName="(Guild) Chat"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Msg",PropertyName=Message))

	ObjClassVersion=1
}

