//=============================================================================
// Copyright 2004-2005 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================
#include "PrecompiledHeaders.h"
#include "avaGame.h"
//#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "UnPath.h"

IMPLEMENT_CLASS(AavaTeamPlayerStart);
IMPLEMENT_CLASS(AavaTeamPlayerStartManager);
IMPLEMENT_CLASS(AavaDefensePoint);

void AavaPickupFactory::PostEditMove(UBOOL bFinished)
{
	if ( bFinished )
	{
		// align pickupbase mesh to floor
		if ( BaseMesh )
		{
			FCheckResult Hit(1.f);
			FLOAT CollisionHeight, CollisionRadius;
			GetBoundingCylinder(CollisionRadius, CollisionHeight);
			GWorld->SingleLineCheck( Hit, this, Location - FVector(0.f,0.f,1.5f*CollisionHeight), Location, TRACE_World, GetCylinderExtent() );
			if ( Hit.Time < 1.f )
			{				
				Rotation = FindSlopeRotation(Hit.Normal, Rotation);
				FVector DefaultTranslation = Cast<AavaPickupFactory>(GetClass()->GetDefaultActor())->BaseMesh->Translation;
				BaseMesh->Translation = DefaultTranslation - FVector(CollisionRadius * (1.f - Hit.Normal.Z*Hit.Normal.Z));
				BaseMesh->BeginDeferredReattach();
			}
		}
	}

	Super::PostEditMove( bFinished );
}

void AavaPickupFactory::Spawned()
{
	Super::Spawned();

	if ( !GWorld->HasBegunPlay() )
		PostEditMove();
}

void AavaTeamPlayerStart::PostEditChange(UProperty* PropertyThatChanged)
{
	UTexture2D* NewSprite = NULL;

	// managerGroup 이 0 이 아닌 경우에는 회색으로 찍어 준다.
	// 사실 managerGroup 에 해당하는 녀석을 찾아서 그 녀석의 sprite 색깔을
	// 따라 가는 것이 제일 좋은데 
	if ( ManagerGroup != 0 && TeamSprites.Num() >= 2 )
	{
		NewSprite = TeamSprites(2);
	}
	else if (TeamNumber < TeamSprites.Num())
	{
		NewSprite = TeamSprites(TeamNumber);
	}
	else
	{
		// get sprite from defaults
		AavaTeamPlayerStart* Default = GetClass()->GetDefaultObject<AavaTeamPlayerStart>();
		for (INT i = 0; i < Default->Components.Num() && NewSprite == NULL; i++)
		{
			USpriteComponent* SpriteComponent = Cast<USpriteComponent>(Default->Components(i));
			if (SpriteComponent != NULL)
			{
				NewSprite = SpriteComponent->Sprite;
			}
		}
	}

	if (NewSprite != NULL)
	{
		// set the new sprite as the current one
		USpriteComponent* SpriteComponent = NULL;
		for (INT i = 0; i < Components.Num() && SpriteComponent == NULL; i++)
		{
			SpriteComponent = Cast<USpriteComponent>(Components(i));
		}
		if (SpriteComponent != NULL)
		{
			SpriteComponent->Sprite = NewSprite;
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AavaTeamPlayerStart::Spawned()
{
	Super::Spawned();

	PostEditChange(NULL);
}

void AavaTeamPlayerStartManager::PostEditChange(UProperty* PropertyThatChanged)
{
	UTexture2D* NewSprite = NULL;
	if (InitialTeamNumber < TeamSprites.Num())
	{
		NewSprite = TeamSprites(InitialTeamNumber);
	}
	else
	{
		// get sprite from defaults
		AavaTeamPlayerStartManager* Default = GetClass()->GetDefaultObject<AavaTeamPlayerStartManager>();
		for (INT i = 0; i < Default->Components.Num() && NewSprite == NULL; i++)
		{
			USpriteComponent* SpriteComponent = Cast<USpriteComponent>(Default->Components(i));
			if (SpriteComponent != NULL)
			{
				NewSprite = SpriteComponent->Sprite;
			}
		}
	}

	if (NewSprite != NULL)
	{
		// set the new sprite as the current one
		USpriteComponent* SpriteComponent = NULL;
		for (INT i = 0; i < Components.Num() && SpriteComponent == NULL; i++)
		{
			SpriteComponent = Cast<USpriteComponent>(Components(i));
		}
		if (SpriteComponent != NULL)
		{
			SpriteComponent->Sprite = NewSprite;
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AavaTeamPlayerStartManager::Spawned()
{
	Super::Spawned();

	PostEditChange(NULL);
}


void AavaDefensePoint::PostEditChange(UProperty* PropertyThatChanged)
{
	UTexture2D* NewSprite = NULL;
	if (DefendedObjective != NULL && DefendedObjective->DefenderTeamIndex < TeamSprites.Num())
	{
		NewSprite = TeamSprites(DefendedObjective->DefenderTeamIndex);
	}
	else
	{
		// get sprite from defaults
		AavaDefensePoint* Default = GetArchetype<AavaDefensePoint>();
		for (INT i = 0; i < Default->Components.Num() && NewSprite == NULL; i++)
		{
			USpriteComponent* SpriteComponent = Cast<USpriteComponent>(Default->Components(i));
			if (SpriteComponent != NULL)
			{
				NewSprite = SpriteComponent->Sprite;
			}
		}
	}

	if (NewSprite != NULL)
	{
		// set the new sprite as the current one
		USpriteComponent* SpriteComponent = NULL;
		for (INT i = 0; i < Components.Num() && SpriteComponent == NULL; i++)
		{
			SpriteComponent = Cast<USpriteComponent>(Components(i));
		}
		if (SpriteComponent != NULL)
		{
			SpriteComponent->Sprite = NewSprite;
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AavaDefensePoint::Spawned()
{
	Super::Spawned();

	// by default, all defense points have their own group
	DefenseGroup = GetFName();

	PostEditChange(NULL);
}

UBOOL AavaPawn::SuggestJumpVelocity(FVector& JumpVelocity, FVector End, FVector Start)
{
	return true;
}

ETestMoveResult AavaPawn::FindJumpUp(FVector Direction, FVector &CurrentPosition)
{
	return TESTMOVE_Stopped;	
}

/* TryJumpUp()
Check if could jump up over obstruction
*/
UBOOL AavaPawn::TryJumpUp(FVector Dir, FVector Destination, DWORD TraceFlags, UBOOL bNoVisibility)
{
	FVector Out = 14.f * Dir;
	FCheckResult Hit(1.f);
	FVector Up = FVector(0.f,0.f,MaxJumpHeight);

	if ( bNoVisibility )
	{
		// do quick trace check first
		FVector Start = Location + FVector(0.f, 0.f, CylinderComponent->CollisionHeight);
		FVector End = Start + Up;
		GWorld->SingleLineCheck(Hit, this, End, Start, TRACE_World);
		UBOOL bLowCeiling = Hit.Time < 1.f;
		if ( bLowCeiling )
		{
			End = Hit.Location;
		}
		GWorld->SingleLineCheck(Hit, this, Destination, End, TraceFlags);
		if ( (Hit.Time < 1.f) && (Hit.Actor != Controller->MoveTarget) )
		{
			return false;			
		}
	}

	GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TRACE_World, GetCylinderExtent());
	FLOAT FirstHit = Hit.Time;
	if ( FirstHit > 0.5f )
	{
		GWorld->SingleLineCheck(Hit, this, Location + Up * Hit.Time + Out, Location + Up * Hit.Time, TraceFlags, GetCylinderExtent());
		
		return (Hit.Time == 1.f);
	}
	return false;
}

INT AavaPawn::calcMoveFlags()
{
	return ( bCanWalk * R_WALK + bCanFly * R_FLY + bCanSwim * R_SWIM + bJumpCapable * R_JUMP ); 
}


static AavaGameObjective* TestObjective = NULL;

IMPLEMENT_COMPARE_POINTER(ANavigationPoint, avaPathing, { return appTrunc(appSqrt((TestObjective->Location - A->Location).SizeSquared() - (TestObjective->Location - B->Location).SizeSquared())); })

void AavaGameObjective::AddForcedSpecs(AScout* Scout)
{
	// put the five closest visible NavigationPoints in the ShootSpots array

	// create list of all non-blocking non-flying source NavigationPoints
	TArray<ANavigationPoint*> NavList;
	for (ANavigationPoint* N = GWorld->GetFirstNavigationPoint(); N != NULL; N = N->nextNavigationPoint)
	{
		if (N != this && !N->bBlockActors && !N->bDestinationOnly && !N->bFlyingPreferred)
		{
			NavList.AddItem(N);
		}
	}

	// sort by distance
	TestObjective = this;
	Sort<USE_COMPARE_POINTER(ANavigationPoint,avaPathing)>(NavList.GetTypedData(), NavList.Num());
	TestObjective = NULL;

	// put the first five that succeed a visibility trace into the ShootSpots array
	ShootSpots.Empty();
	FCheckResult Hit(1.0f);
	FVector TargetLoc = GetTargetLocation();
	for (INT i = 0; i < NavList.Num(); i++)
	{
		if (GWorld->SingleLineCheck(Hit, Scout, TargetLoc, NavList(i)->Location, TRACE_World | TRACE_StopAtAnyHit))
		{
			ShootSpots.AddItem(NavList(i));
			if (ShootSpots.Num() >= 5)
			{
				break;
			}
		}
	}

	// if bAllowOnlyShootable, we don't need to be reachable if we found any ShootSpots
	if (bAllowOnlyShootable && ShootSpots.Num() > 0)
	{
		bMustBeReachable = FALSE;
	}
	else
	{
		bMustBeReachable = GetArchetype<ANavigationPoint>()->bMustBeReachable;
	}
}