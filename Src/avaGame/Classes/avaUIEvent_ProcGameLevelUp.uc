/*
	이병 진급 이벤트.

	2007/12/13	고광록
*/
class avaUIEvent_ProcGameLevelUp extends UIEvent;

defaultproperties
{
	ObjName="Proc Game Level Up"
	ObjCategory="ProcGame"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Response",bWriteable=true))

	ObjClassVersion=1
}