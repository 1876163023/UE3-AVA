class avaUIEvent_ProcBuddyAddReqAns extends UIEvent;


defaultproperties
{
	ObjName="Proc Buddy AddReqAns"
	ObjCategory="ProcBuddy"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="BuddyName",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Accept",bWriteable=true))

	ObjClassVersion=1
}