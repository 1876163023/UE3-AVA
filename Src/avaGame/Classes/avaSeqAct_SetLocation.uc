class avaSeqAct_SetLocation extends SequenceAction;

event activated()
{
	Local SeqVar_Object ObjVar;
	Local SeqVar_Vector VecVar;
	Local vector NewLocation;
	
	// assume that the Value of "Location " is unique or none.
	foreach LinkedVariables(class'SeqVar_Vector', VecVar, "Location")
	{
		NewLocation = VecVar.VectValue;
	}

	if( VSize(NewLocation) == 0.0 )
		return;

	foreach LinkedVariables(class'SeqVar_Object', ObjVar, "TargetActor")
	{
		if( Actor(ObjVar.GetObjectValue()) != None )
		{
				Actor(ObjVar.GetObjectValue()).SetLocation(NewLocation);
		}
	}
}


defaultproperties
{
	bCallHandler=false
	ObjName="SetLocation"
	ObjCategory="Actor"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="Location",bWriteable=false,MaxVars=1))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="TargetActor",bWriteable=true))
}