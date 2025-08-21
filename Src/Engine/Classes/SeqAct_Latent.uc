/**
 * This is a basic action that supports latent execution on actors.  It will
 * remain active by default until all targeted actors have either finished the
 * latent behavior, or have been destroyed.
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_Latent extends SequenceAction
	abstract
	native(Sequence);

cpptext
{
	virtual void PreActorHandle(AActor *inActor);
	virtual UBOOL UpdateOp(FLOAT deltaTime);
	virtual void Activated();
	virtual void DeActivated();
};

/** List of all actors currently performing this op */
var array<Actor> LatentActors;

/** Indicates whether or not this latent action has been aborted */
var bool bAborted;

/**
 * Allows an actor to abort this current latent action, forcing
 * the Aborted output link to be activated instead of the default
 * one on normal completion.
 *
 * @param	latentActor - actor aborting the latent action
 */
native function AbortFor(Actor latentActor);

defaultproperties
{
	ObjName="Undefined Latent"
	ObjColor=(R=128,G=128,B=0,A=255)
	OutputLinks(0)=(LinkDesc="Finished")
	OutputLinks(1)=(LinkDesc="Aborted")

	bLatentExecution=TRUE
}
