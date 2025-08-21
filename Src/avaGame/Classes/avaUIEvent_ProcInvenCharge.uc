/*
	충전 후 호출되는 이벤트.

	2007/12/10	고광록
		FakeFullScreen에서 FullScreen으로 변환하기 위해서 이벤트 필요.
*/
class avaUIEvent_ProcInvenCharge extends UIEvent;

defaultproperties
{
	ObjName="Proc Inven Charge Cash"
	ObjCategory="ProcInven"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Response",bWriteable=true))

	ObjClassVersion=1
}