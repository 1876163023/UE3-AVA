/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SetMaterial extends SequenceAction
	native(Sequence);

/** Material to apply to target when action is activated. */
var()	MaterialInstance	NewMaterial;

/** Index in the Materials array to replace with NewMaterial when this action is activated. */
var()	INT					MaterialIndex;

cpptext
{
	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

defaultproperties
{
	ObjName="Set Material"
	ObjCategory="Actor"
}