/*=============================================================================
	UnSkeletalMeshCollision.cpp: Skeletal mesh collision code
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"

#if WITH_NOVODEX
#include "UnNovodexSupport.h"
#endif // WITH_NOVODEX

UBOOL USkeletalMeshComponent::LineCheck(
                FCheckResult &Result,
                const FVector& End,
                const FVector& Start,
                const FVector& Extent,
				DWORD TraceFlags)
{
	UBOOL Retval = FALSE;

	// Can only do line-tests against skeletal meshes if we have a physics asset for it.
	if( PhysicsAsset != NULL )
	{
		Retval = PhysicsAsset->LineCheck( Result, this, Start, End, Extent );
	
		// If we hit and it's an extent trace (eg player movement) - pull back the hit location in the same way as static meshes.
		if(!Retval && !Extent.IsZero())
		{
			Result.Time = Clamp(Result.Time - Clamp(0.1f,0.1f / (End - Start).Size(),4.0f / (End - Start).Size()),0.0f,1.0f);
			Result.Location = Start + (End - Start) * Result.Time;
		}
	}
	else
	{
		Retval = TRUE;
	}

#if WITH_NOVODEX && !NX_DISABLE_CLOTH
	if(ClothSim && bEnableClothSimulation && SkeletalMesh && SkeletalMesh->bEnableClothLineChecks)
	{
		FCheckResult TempResult;

		if(!ClothLineCheck(this, TempResult, End, Start, Extent, TraceFlags))
		{//hit
			if( Retval || (TempResult.Time < Result.Time) )
			{
				Result = TempResult;
				Retval = FALSE;
			}
		}
	}
#endif

	return Retval;
}


UBOOL USkeletalMeshComponent::PointCheck(FCheckResult& Result, const FVector& Location, const FVector& Extent, DWORD TraceFlags)
{
	UBOOL bHit = FALSE;

	if(PhysicsAsset)
	{
		bHit = !PhysicsAsset->PointCheck( Result, this, Location, Extent );
	}

#if WITH_NOVODEX && !NX_DISABLE_CLOTH
	if(!bHit)
	{
		if(ClothSim && bEnableClothSimulation && SkeletalMesh && SkeletalMesh->bEnableClothLineChecks)
		{
			bHit = !ClothPointCheck(Result, this, Location, Extent);
		}
	}
#endif

	return !bHit;
}

