/**
 * This is used to light components / actors during the game.  Doing something like:
 * LightEnvironment=FooLightEnvironment
 *
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class DynamicLightEnvironmentComponent extends LightEnvironmentComponent
	native;

/** The current state of the light environment. */
var private native transient const pointer State{class FDynamicLightEnvironmentState};

/** The number of seconds between light environment updates for actors which aren't visible. */
var() float InvisibleUpdateTime;

/** Minimum amount of time that needs to pass between full environment updates. */
var() float MinTimeBetweenFullUpdates;

/** The number of visibility samples to use within the primitive's bounding volume. */
var() int NumVolumeVisibilitySamples;

/** The color of the ambient shadow. */
var() LinearColor AmbientShadowColor;

/** The direction of the ambient shadow source. */
var() vector AmbientShadowSourceDirection;

/** The distance to create the light from the owner's origin, in radius units. */
var() float LightDistance;

/** The distance for the shadow to project beyond the owner's origin, in radius units. */
var() float ShadowDistance;

/** Whether the light environment should cast shadows */
var() bool bCastShadows;

/** Whether the light environment should be dynamically updated. */
var() bool bDynamic;

/** Ava specific */
var() bool bUpdateLastLighting;
var	transient vector	LastDirection;
var transient color	LastColor;
var	transient float	LastBrightness;
var transient vector	LastPosition;

var transient float	LastUpperSkyBrightness;
var transient color	LastUpperSkyColor;
var transient float	LastLowerSkyBrightness;
var transient color	LastLowerSkyColor;

cpptext
{
	// UObject interface.
	virtual void FinishDestroy();
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	virtual void Serialize(FArchive& Ar);

	// UActorComponent interface.
	virtual void Tick(FLOAT DeltaTime);
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void Detach();

	// ULightEnvironmentComponent interface.
	virtual void UpdateLight(const ULightComponent* Light);
}

defaultproperties
{
	InvisibleUpdateTime=4.0
	MinTimeBetweenFullUpdates=0.3
	NumVolumeVisibilitySamples=1
	AmbientShadowColor=(R=0.75,G=0.75,B=0.75)
	AmbientShadowSourceDirection=(X=0,Y=0,Z=1)
	LightDistance=1.0
	ShadowDistance=1.0
	TickGroup=TG_PostAsyncWork
	bCastShadows=TRUE
	bDynamic=TRUE
}
