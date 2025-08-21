class avaUIEvent_ProcBuddyStateChanged extends UIEvent;


defaultproperties
{
	ObjName="Proc Buddy StateChanged"
	ObjCategory="ProcBuddy"
	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="BuddyName",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="bLogIn",bWriteable=true))

	ObjClassVersion=1
}