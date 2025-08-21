/*
	Level(World)의 Kismet으로 Event를 발생시키는 액션.

	2007/01/30	고광록
*/
class avaUIAction_ActivateLevelEventObject extends UIAction_ActivateLevelEvent
	native;

cpptext
{
	void Activated();
}

DefaultProperties
{
	ObjName="ava Activate Level Event Object"
	ObjCategory="Level"

	EventName=DefaultEvent

	// 문자열을 링크할 수 있게 해준다.
	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Object",MaxVars=1,bWriteable=true))

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Failed")
	OutputLinks(1)=(LinkDesc="Success")
}
