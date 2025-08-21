class avaSeqAction_BlockDelete extends avaSeqAction;

var() string Nickname;


event Activated()
{
	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().BlockDelete(Nickname);
}


defaultproperties
{
	ObjName="(Block) Delete"

	InputLinks(0)=(LinkDesc="In")

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))

	ObjClassVersion=1
}

