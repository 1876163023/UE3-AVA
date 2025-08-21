/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class AnimNotify_Scripted extends AnimNotify
	native(Anim)
	abstract;

event Notify( Actor Owner, AnimNodeSequence AnimSeqInstigator );

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class UAnimNodeSequence* NodeSeq );
}
