/**
 *	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 *	Advanced version of SkeletalMeshActor which uses an AnimTree instead of having a single AnimNodeSequence defined in its defaultproperties
 */

class SkeletalMeshActorMAT extends SkeletalMeshActor
	native(Anim)
	placeable;

cpptext
{
	virtual void GetAnimControlSlotDesc(TArray<struct FAnimSlotDesc>& OutSlotDescs);
	virtual void PreviewBeginAnimControl(TArray<class UAnimSet*>& InAnimSets);
	virtual void PreviewSetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition);
	virtual void PreviewSetAnimWeights(TArray<FAnimSlotInfo>& SlotInfos);
	virtual void PreviewFinishAnimControl();
}

/** Array of Slots */
var transient Array<AnimNodeSlot>	SlotNodes;

/** Start AnimControl. Add required AnimSets. */
native function MAT_BeginAnimControl(Array<AnimSet> InAnimSets);

/** Update AnimTree from track info */
native function MAT_SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies);

/** Update AnimTree from track weights */
native function MAT_SetAnimWeights(Array<AnimSlotInfo> SlotInfos);

/** End AnimControl. Release required AnimSets */
native function MAT_FinishAnimControl();


/** Called when we start an AnimControl track operating on this Actor. Supplied is the set of AnimSets we are going to want to play from. */
simulated event BeginAnimControl(Array<AnimSet> InAnimSets)
{
	MAT_BeginAnimControl(InAnimSets);
}

/** Called each from while the Matinee action is running, with the desired sequence name and position we want to be at. */
simulated event SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies)
{
	MAT_SetAnimPosition(SlotName, ChannelIndex, InAnimSeqName, InPosition, bFireNotifies);
}

/** Called each from while the Matinee action is running, to set the animation weights for the actor. */
simulated event SetAnimWeights(Array<AnimSlotInfo> SlotInfos)
{
	MAT_SetAnimWeights(SlotInfos);
}

/** Called when we are done with the AnimControl track. */
simulated event FinishAnimControl()
{
	MAT_FinishAnimControl();
}


defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		Animations=None
	End Object
}