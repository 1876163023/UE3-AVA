/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_AttachToEvent extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
};

defaultproperties
{
	ObjName="Attach To Event"
	ObjCategory="Event"

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Attachee")
	EventLinks(0)=(LinkDesc="Event")
}
