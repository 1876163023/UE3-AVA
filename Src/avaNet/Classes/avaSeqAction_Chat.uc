class avaSeqAction_Chat extends avaSeqAction;



var() string ChatMsg;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().Chat(ChatMsg);
}


defaultproperties
{
	ObjName="Chat"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Message",PropertyName=ChatMsg))

	ObjClassVersion=2
}
