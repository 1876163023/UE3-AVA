/**
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class SoundCue extends Object
	hidecategories(object)
	native;

struct native export SoundNodeEditorData
{
	var	native const int NodePosX;
	var native const int NodePosY;
};

/** Sound group this sound cue belongs to */
var							Name									SoundGroup;
var							SoundNode								FirstNode;
var		const native		Map{USoundNode*,FSoundNodeEditorData}	EditorData;
var		transient			float									MaxAudibleDistance;
var()						float									VolumeMultiplier;
var()						float									PitchMultiplier;
var							float									Duration;

/** Reference to FaceFX AnimSet package the animation is in */
var()						FaceFXAnimSet							FaceFXAnimSetRef;
/** Name of the FaceFX Group the animation is in */
var()						string									FaceFXGroupName;
/** Name of the FaceFX Animation */
var()						string									FaceFXAnimName;

/** Maximum number of times this cue can be played concurrently. */
var()						int										MaxConcurrentPlayCount;
/** Number of times this cue is currently being played. */
var	const transient duplicatetransient int							CurrentPlayCount;

cpptext
{
	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/**
	 * Returns a description of this object that can be used as extra information in list views.
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT Index );

	/**
	 * @return		Sum of the size of waves referenced by this cue.
	 */
	virtual INT GetResourceSize();

	// USoundCue interface
	UBOOL IsAudible( const FVector &SourceLocation, const FVector &ListenerLocation, AActor* SourceActor, INT& bIsOccluded, UBOOL bCheckOcclusion );
	UBOOL IsAudibleSimple( FVector* Location );

	// Tool drawing
	void DrawCue(FCanvas* Canvas, TArray<USoundNode*>& SelectedNodes);
}

native function float GetCueDuration();

defaultproperties
{
	VolumeMultiplier=0.75
	PitchMultiplier=1
	MaxConcurrentPlayCount=0
}
