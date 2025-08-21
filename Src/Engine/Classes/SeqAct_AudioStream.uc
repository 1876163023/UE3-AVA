/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_AudioStream extends SequenceAction native(Sequence);	

enum AudioStreamOp
{
	ASO_Play,		
	ASO_Stop,	
};

var() AudioStreamOp Operation;
var() int Stream;

cpptext
{
	void Activated();
}

event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Audio Stream"
	ObjCategory="Misc"
	ObjClassVersion=2	

	Operation = ASO_Play
}
