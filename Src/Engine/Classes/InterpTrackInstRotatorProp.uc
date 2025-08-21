/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class InterpTrackInstRotatorProp extends InterpTrackInst
	native(Interpolation);


cpptext
{
	virtual void SaveActorState(UInterpTrack* Track);
	virtual void RestoreActorState(UInterpTrack* Track);

	virtual void InitTrackInst(UInterpTrack* Track);
}

/** Pointer to vector property in TrackObject. */
var	pointer		RotatorProp; 

/** Saved value for restoring state when exiting Matinee. */
var	rotator		ResetRotator;
