class avaSeqAction_Whisper extends avaSeqAction;


var() string Nickname;
var() string ChatMsg;

event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().Whisper(Nickname, ChatMsg);
}


defaultproperties
{
	ObjName="(Comm.) Whisper"

	InputLinks.Empty
	InputLinks(0)=(LinkDesc="In")

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Message",PropertyName=ChatMsg))

	ObjClassVersion=1
}

