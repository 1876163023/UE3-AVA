class avaSeqAct_SetRotation extends SequenceAction;


event activated()
{
	Local SeqVar_Object ObjVar;
	Local SeqVar_Vector VecVar;
	Local Rotator NewRotator;
	Local vector NewRotation;

	foreach LinkedVariables(class'SeqVar_Vector', VecVar, "RotationVector")
		NewRotation = VecVar.VectValue;

	if( VSize(NewRotation) == 0.0 )
		return;

	//NewRotator = Rotator(NewRotation);
	NewRotator.Pitch	=	NewRotation.x*65535/360;
	NewRotator.Yaw		=	NewRotation.y*65535/360;
	NewRotator.Roll		=	NewRotation.z*65535/360;

	foreach LinkedVariables(class'SeqVar_Object', ObjVar, "TargetActor")
		if( Actor(ObjVar.GetObjectValue()) != None )
			Actor(ObjVar.GetObjectValue()).SetRotation(NewRotator);
}


defaultproperties
{
	bCallHandler=false
	ObjName="SetRotation"
	ObjCategory="Actor"

	VariableLinks.Empty
	VariableLinks.Add((ExpectedType=class'SeqVar_Vector',LinkDesc="RotationVector",MaxVars=1,bWriteable=false))
	VariableLinks.Add((ExpectedType=class'SeqVar_Object',LinkDesc="TargetActor",bWriteable=true))
}
