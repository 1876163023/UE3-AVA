//=============================================================================
// Volume:  a bounding volume
// touch() and untouch() notifications to the volume as actors enter or leave it
// enteredvolume() and leftvolume() notifications when center of actor enters the volume
// pawns with bIsPlayer==true  cause playerenteredvolume notifications instead of actorenteredvolume()
// Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Volume extends Brush
	native
	nativereplication;

var() Actor AssociatedActor;			// this actor gets touch() and untouch notifications as the volume is entered or left
var() int LocationPriority;
var() localized string LocationName;
var() int LocationNameIndex;

/** Should pawns be forced to walk when inside this volume? */
var() bool bForcePawnWalk;
/** Should process all actors within this volume */
var() bool bProcessAllActors;

//! Toggle On이 될 경우 탱크가 날아다니는 것을 막기 위해 추가.(2007/07/24 고광록)
var() bool bIgnoreBlockRigidBody;

cpptext
{
	INT Encompasses(FVector point);
	void SetVolumes();
	virtual void SetVolumes(const TArray<class AVolume*>& Volumes);
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
	virtual UBOOL IsAVolume() const {return TRUE;}
	virtual AVolume* GetAVolume() { return this; }
	virtual INT* GetOptimizedRepList(BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
}

native noexport function bool Encompasses(Actor Other); // returns true if center of actor is within volume

event PostBeginPlay()
{
	Super.PostBeginPlay();

	if ( AssociatedActor != None )
	{
		GotoState('AssociatedTouch');
		InitialState = GetStateName();
	}

	if ( bIgnoreBlockRigidBody )
		CollisionComponent.SetBlockRigidBody( FALSE );
}

simulated function string GetLocationStringFor(PlayerReplicationInfo PRI)
{
	return LocationName;
}

simulated function int GetLocationNameIndex()
{
	return LocationNameIndex;
}

/**
 * list important Volume variables on canvas.  HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	super.DisplayDebug(HUD, out_YL, out_YPos);

	HUD.Canvas.DrawText("AssociatedActor "$AssociatedActor, false);
	out_YPos += out_YL;
	HUD.Canvas.SetPos(4, out_YPos);
}

State AssociatedTouch
{
	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
		AssociatedActor.Touch(Other, OtherComp, HitLocation, HitNormal);
	}

	event untouch( Actor Other )
	{
		AssociatedActor.untouch(Other);
	}

	event BeginState(Name PreviousStateName)
	{
		local Actor A;

		ForEach TouchingActors(class'Actor', A)
			Touch(A, None, A.Location, Vect(0,0,1) );
	}
}

/**	Handling Toggle event from Kismet. */
simulated function OnToggle(SeqAct_Toggle Action)
{
	// Turn ON
	if (Action.InputLinks[0].bHasImpulse)
	{
		if(!bCollideActors)
		{
			SetCollision(true, bBlockActors);
		}

		if ( bIgnoreBlockRigidBody )
			CollisionComponent.SetBlockRigidBody( FALSE );
		else
			CollisionComponent.SetBlockRigidBody( TRUE );
	}
	// Turn OFF
	else if (Action.InputLinks[1].bHasImpulse)
	{
		if(bCollideActors)
		{
			SetCollision(false, bBlockActors);
		}
		CollisionComponent.SetBlockRigidBody( FALSE );
	}
	// Toggle
	else if (Action.InputLinks[2].bHasImpulse)
	{
		SetCollision(!bCollideActors, bBlockActors);

		if ( bIgnoreBlockRigidBody )
			CollisionComponent.SetBlockRigidBody( FALSE );
		else
			CollisionComponent.SetBlockRigidBody( !CollisionComponent.BlockRigidBody );
	}

	ForceNetRelevant();

	SetForcedInitialReplicatedProperty(Property'Engine.Actor.bCollideActors', (bCollideActors == default.bCollideActors));
}

simulated event CollisionChanged()
{
	if ( bIgnoreBlockRigidBody )
		CollisionComponent.SetBlockRigidBody( FALSE );
	else
		// rigid body collision should match Unreal collision
		CollisionComponent.SetBlockRigidBody(bCollideActors);
}

event ProcessActorSetVolume( Actor Other );

cpptext
{
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
	virtual void PostEditImport();
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=true
		bAcceptsLights=true
		LightingChannels=(Dynamic=TRUE,bInitialized=TRUE)
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=true
		BlockRigidBody=false
		AlwaysLoadOnClient=True
		AlwaysLoadOnServer=True
	End Object

	bCollideActors					=	True
	bSkipActorPropertyReplication	=	true
	LocationNameIndex				=	-1
}
