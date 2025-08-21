/*=============================================================================
	UnCamera.cpp: Unreal Engine Camera Actor implementation
	Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(ACamera);
IMPLEMENT_CLASS(ACameraActor);

IMPLEMENT_CLASS(UCameraModifier);

/*------------------------------------------------------------------------------
	ACamera
------------------------------------------------------------------------------*/


/**
 * Set a new ViewTarget with optional transition time
 */
void ACamera::SetViewTarget(AActor* NewTarget, FLOAT TransitionTime)
{
	// Make sure view target is valid
	if( NewTarget == NULL )
	{
		NewTarget = PCOwner;
	}

	// Update current ViewTargets
	CheckViewTarget(ViewTarget);
	if( PendingViewTarget.Target )
	{
		CheckViewTarget(PendingViewTarget);
	}

	// if different then new one, then assign it
	if( NewTarget != ViewTarget.Target )
	{
		// if a transition time is specified, then set pending view target accordingly
		if( TransitionTime > 0 )
		{
			BlendLength		= TransitionTime;
			BlendTimeToGo	= TransitionTime;
			
			AssignViewTarget(NewTarget, PendingViewTarget);
			CheckViewTarget(PendingViewTarget);
		}
		else
		{
			// otherwise, assign new viewtarget instantly
			AssignViewTarget(NewTarget, ViewTarget);
			CheckViewTarget(ViewTarget);
			// remove old pending ViewTarget so we don't still try to switch to it
			PendingViewTarget.Target = NULL;
		}
	}
	else
	{
		PendingViewTarget.Target = NULL;
	}
}


void ACamera::AssignViewTarget(AActor* NewTarget, FTViewTarget& VT)
{
	if( !NewTarget || (NewTarget == VT.Target) )
	{
		return;
	}

	AActor* OldViewTarget	= VT.Target;
	VT.Target				= NewTarget;
	// Set aspect ratio with default.
	VT.AspectRatio			= DefaultAspectRatio;

	// Set FOV with default.
	VT.POV.FOV				= DefaultFOV;

	VT.Target->eventBecomeViewTarget(PCOwner);
	
	if( OldViewTarget )
	{
		OldViewTarget->eventEndViewTarget(PCOwner);
	}

	if( !PCOwner->LocalPlayerController() && (GWorld->GetNetMode() != NM_Client) )
	{
		PCOwner->eventClientSetViewTarget(VT.Target);
	}
}


/** 
 * Make sure ViewTarget is valid 
 */
void ACamera::CheckViewTarget(FTViewTarget& VT)	
{
	if( !VT.Target )
	{
		VT.Target = PCOwner;
	}

	// Update ViewTarget PlayerReplicationInfo (used to follow same player through pawn transitions, etc., when spectating)
	if( VT.Target == PCOwner || (VT.Target->GetAPawn() && (VT.Target == PCOwner->Pawn)) ) 
	{	
		VT.PRI = NULL;
	}
	else if( VT.Target->GetAController() )
	{
		VT.PRI = VT.Target->GetAController()->PlayerReplicationInfo;
	}
	else if( VT.Target->GetAPawn() )
	{
		VT.PRI = VT.Target->GetAPawn()->PlayerReplicationInfo;
	}
	else if( Cast<APlayerReplicationInfo>(VT.Target) )
	{
		VT.PRI = Cast<APlayerReplicationInfo>(VT.Target);
	}
	else
	{
		VT.PRI = NULL;
	}

	if( VT.PRI && !VT.PRI->bDeleteMe )
	{
		if( !VT.Target || VT.Target->bDeleteMe || !VT.Target->GetAPawn() || (VT.Target->GetAPawn()->PlayerReplicationInfo != VT.PRI) )
		{
			VT.Target = NULL;

			// not viewing pawn associated with RealViewTarget, so look for one
			// Assuming on server, so PRI Owner is valid
			if( !VT.PRI->Owner )
			{
				VT.PRI = NULL;
			}
			else
			{
				AController* PRIOwner = VT.PRI->Owner->GetAController();
				if( PRIOwner )
				{
					AActor* PRIViewTarget = PRIOwner->Pawn;
					if( PRIViewTarget && !PRIViewTarget->bDeleteMe )
					{
						AssignViewTarget(PRIViewTarget, VT);
					}
					else
					{
						VT.PRI = NULL;
					}
				}
				else
				{
					VT.PRI = NULL;
				}
			}
		}
	}

	if( !VT.Target || VT.Target->bDeleteMe )
	{
		if( PCOwner->Pawn && !PCOwner->Pawn->bDeleteMe && !PCOwner->Pawn->bPendingDelete )
		{
			AssignViewTarget(PCOwner->Pawn, VT);
		}
		else
		{
			AssignViewTarget(PCOwner, VT);
		}
	}

	// Keep PlayerController in synch
	PCOwner->ViewTarget		= VT.Target;
	PCOwner->RealViewTarget	= VT.PRI;
}


/** 
 * Returns current ViewTarget 
 */
AActor* ACamera::GetViewTarget()
{
	// if blending to another view target, return this one first
	if( PendingViewTarget.Target )
	{
		CheckViewTarget(PendingViewTarget);
		if( PendingViewTarget.Target )
		{
			return PendingViewTarget.Target;
		}
	}

	CheckViewTarget(ViewTarget);
	return ViewTarget.Target;
}


UBOOL ACamera::PlayerControlled()
{
	return (PCOwner != NULL);
}


/*------------------------------------------------------------------------------
	ACameraActor
------------------------------------------------------------------------------*/

/** 
 *	Use to assign the camera static mesh to the CameraActor used in the editor.
 *	Beacuse HiddenGame is true and CollideActors is false, the component should be NULL in-game.
 */
void ACameraActor::Spawned()
{
	Super::Spawned();

	if(MeshComp)
	{
		if( !MeshComp->StaticMesh)
		{
			UStaticMesh* CamMesh = LoadObject<UStaticMesh>(NULL, TEXT("EditorMeshes.MatineeCam_SM"), NULL, LOAD_None, NULL);
			FComponentReattachContext ReattachContext(MeshComp);
			MeshComp->StaticMesh = CamMesh;
		}
	}

	// Sync component with CameraActor frustum settings.
	UpdateDrawFrustum();
}

/** Used to synchronise the DrawFrustumComponent with the CameraActor settings. */
void ACameraActor::UpdateDrawFrustum()
{
	if(DrawFrustum)
	{
		DrawFrustum->FrustumAngle = FOVAngle;
		DrawFrustum->FrustumStartDist = 10.f;
		DrawFrustum->FrustumEndDist = 1000.f;
		DrawFrustum->FrustumAspectRatio = AspectRatio;
	}
}

/** Ensure DrawFrustumComponent is up to date. */
void ACameraActor::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	Super::UpdateComponentsInternal(bCollisionUpdate);
	UpdateDrawFrustum();
}

/** Used to push new frustum settings down into preview component when modifying camera through property window. */
void ACameraActor::PostEditChange(UProperty* PropertyThatChanged)
{
	UpdateDrawFrustum();
	Super::PostEditChange(PropertyThatChanged);
}
