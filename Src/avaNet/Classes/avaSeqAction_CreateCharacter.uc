class avaSeqAction_CreateCharacter extends avaSeqAction;



var() int FaceIndex;
var() string Nickname;

event Activated()
{
	// ������ 1 ~ 5������ ���� ����Ѵ�.
	local int ServerFaceIndex;
	
	ServerFaceIndex = FaceIndex + 1;

	`log("avaSeqAction_CreateCharacter - Nickname:"@Nickname @"FaceIndex:" @ServerFaceIndex);

	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().CreateCharacter(Nickname, ServerFaceIndex);
}


defaultproperties
{
	ObjName="Create Character"

    VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Nickname",PropertyName=Nickname))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Face Index",PropertyName=FaceIndex))
}

