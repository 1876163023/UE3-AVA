//=============================================================================
//  avaSeqAct_BuildBox
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/05/12 by OZ
//		avaVolume_BuildBox Á¦¾î¿ë Sequence Action.
//
//=============================================================================
class avaSeqAct_BuildBox extends SequenceAction;

var avaVolume_BuildBox	BuildVolume;

event Activated()
{
	if (InputLinks[0].bHasImpulse)			// Enable Volume
	{
		if ( BuildVolume != None )	BuildVolume.EnableBuildBox( true );
	}
	else if (InputLinks[1].bHasImpulse)		// Disable Volume
	{
		if ( BuildVolume != None )	BuildVolume.EnableBuildBox( false );
	}
}

defaultproperties
{
	ObjCategory="Objective"
	ObjName="BuildBox"
	bCallHandler=false

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="BuildVolume",PropertyName=BuildVolume,MinVars=0, MaxVars=1 )
}