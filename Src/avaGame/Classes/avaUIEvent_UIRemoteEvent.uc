class avaUIEvent_UIRemoteEvent extends UIEvent
	native;

var() Name EventName;

defaultproperties
{
	ObjName="UI Remote Event"

	EventName=DefaultEvent
	bPlayerOnly=FALSE

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Str",bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Int",bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Bool",bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Float",bWriteable=true,bHidden=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="Obj",bWriteable=true,bHidden=true))
}