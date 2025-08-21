/**
 * Activated when an actor touches another actor.  Will be called on both actors, first on the actor that
 * was originally touched, then on the actor that did the touching
 *
 * Originator: the actor that owns this event
 * Instigator: the actor that was touched
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvent_Touch extends SequenceEvent
	native(Sequence);

cpptext
{
	virtual UBOOL CheckTouchActivate(AActor *inOriginator, AActor *inInstigator, UBOOL bTest = FALSE);
	virtual UBOOL CheckUnTouchActivate(AActor *inOriginator, AActor *inInstigator, UBOOL bTest = FALSE);

private:
	// hide the default implementation to force use of CheckTouchActivate/CheckUnTouchActivate
	virtual UBOOL CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest=FALSE, TArray<INT>* ActivateIndices = NULL, UBOOL bPushTop = FALSE);
};

//==========================
// Base variables

/** List of class types that are considered valid for this event */
var() array<class<Actor> >		ClassProximityTypes;

/** Force the player to be overlapping at the time of activation? */
var() bool bForceOverlapping;

/**
 * Use Instigator, not actual Actor.
 * For projectiles, it returns the instigator.
 */
var() bool bUseInstigator;

/** List of all actors that have activated this touch event, so that untouch may be properly fired. */
var array<Actor> TouchedList;

native noexport function bool CheckTouchActivate(Actor InOriginator, Actor InInstigator, optional bool bTest);

event Toggled()
{
	local int Idx;
	// if now enabled
	if (bEnabled)
	{
		// check activation for everything currently touching the originator
		if (Originator != None)
		{
			for (Idx = 0; Idx < Originator.Touching.Length; Idx++)
			{
				CheckTouchActivate(Originator,Originator.Touching[Idx]);
			}
		}
	}
	else
	{
		// otherwise clear the touched list
		TouchedList.Length = 0;
	}
}

defaultproperties
{
	ObjName="Touch"
	ObjCategory="Physics"
	ClassProximityTypes(0)=class'Pawn'
	OutputLinks(0)=(LinkDesc="Touched")
	OutputLinks(1)=(LinkDesc="UnTouched")

	// default to overlap check, as this is generally the expected behavior
	bForceOverlapping=TRUE

	// set a default retrigger delay since touches are fairly frequent
	ReTriggerDelay=0.1f
}
