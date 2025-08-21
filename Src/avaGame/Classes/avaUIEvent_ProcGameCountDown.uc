class avaUIEvent_ProcGameCountDown extends UIEvent;


defaultproperties
{
	ObjName="Proc Game CountDown"
	ObjCategory="ProcGame"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Response",bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="OnlyCount",bWriteable=true))

	ObjClassVersion=1
}