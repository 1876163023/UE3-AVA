/**
 * SeqAct_CommitMapChange
 *
 * Kismet action commiting pending map change
 *
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CommitMapChange extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
};

defaultproperties
{
	ObjName="Commit Map Change"

	ObjCategory="Level"
	VariableLinks.Empty
	InputLinks(0)=(LinkDesc="Commit")
}
