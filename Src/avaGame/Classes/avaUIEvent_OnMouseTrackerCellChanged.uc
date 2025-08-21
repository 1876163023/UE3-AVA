class avaUIEvent_OnMouseTrackerCellChanged extends UIEvent
	native;

defaultproperties
{
	ObjName="MouseTracker CellChanged"
	ObjCategory="UI"

	VariableLinks.Add((ExpectedType=class'SeqVar_String',LinkDesc="Alias",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Elem Index",bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Cell Index",bWriteable=true))
}