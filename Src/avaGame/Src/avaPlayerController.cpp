//=============================================================================
// Copyright 2004-2005 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================
#include "PrecompiledHeaders.h"
#include "avaGame.h"
//#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "UnPath.h"

IMPLEMENT_CLASS(AavaPlayerController);

extern UBOOL GForceClearBackbuffer;

void AavaPlayerController::SetClearBackBufferFlag(UBOOL bFlag)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		SetClearBackBufferMode,
		BOOL,bFlag,bFlag,
	{
		GForceClearBackbuffer = bFlag;
	});
}

FString AavaPlayerController::GetMapFilename()
{
	return GWorld->URL.Map;
}

/** HearSound()
If sound is audible, calls eventClientHearSound() so local or remote player will hear it.
*/
UBOOL AavaPlayerController::CanApplyCullDistance() 
{ 
	return (!bBehindView && ViewTarget != NULL);		
}

void AavaPlayerController::HearSound(USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed)
{
	eventClientHearSound( InSoundCue, SoundPlayer, SoundLocation, bStopWhenOwnerDestroyed, FALSE );	
}

void AavaPlayerController::CheckShake(FLOAT& MaxOffset, FLOAT& Offset, FLOAT& Rate, FLOAT Time)
{
	if (Abs(Offset) >= Abs(MaxOffset))
	{
		Offset = MaxOffset;
		if ( Time > 1.f )
		{
			if (Time * Abs(MaxOffset / Rate) <= 1.0)
			{
				MaxOffset = MaxOffset * (1 / Time - 1);
				Rate = Rate / (1 / Time - 1);
			}
			else
			{
				MaxOffset *= -1;
				Rate *= -1;
			}
		}
		else
		{
			MaxOffset = 0.f;
			Offset = 0.f;
			Rate = 0.f;
		}
	}
}

void AavaPlayerController::UpdateShakeRotComponent(FLOAT& Max, INT& Current, FLOAT& Rate, FLOAT Time, FLOAT DeltaTime)
{
    Current = appRound((Current & 65535) + Rate * DeltaTime) & 65535;
    if ( Current > 32768 )
    Current -= 65536;

    FLOAT fCurrent = Current;
    CheckShake(Max, fCurrent, Rate, Time);
    Current = (INT)fCurrent;
}

void AavaPlayerController::OnPreRender()
{
	//if ( Pawn != NULL && Pawn->Weapon != NULL )
	//{
	//	(Cast<AavaWeapon>(Pawn->Weapon))->SetPositionEx( Cast<AavaPawn>(Pawn) );
	//}
}

UBOOL AavaPlayerController::Tick(FLOAT DeltaSeconds, ELevelTick TickType)
{
	if ( Pawn != NULL )
	{
		const AavaPawn* ap = Cast<AavaPawn>(Pawn);
		if ( ap != NULL )
		{
			if ( ap->NightvisionActivated == TRUE )
			{
				if ( CurrentBattery > 0 && bInfinityBattery == FALSE )
				{
					CurrentBattery -= DeltaSeconds * NVGConsumptionSpeed;
					if ( CurrentBattery <= 0 )
					{
						CurrentBattery = 0;
						// Turn off Night Vision...
						eventServerTurnOffNightvision();
					}
				}
			}
			else
			{
				if ( CurrentBattery < MaximumBattery )
				{
					CurrentBattery += DeltaSeconds * BatteryChargeSpeed;
					if ( CurrentBattery > MaximumBattery )
						CurrentBattery = MaximumBattery;
				}
			}
		}
	}

	if ( Super::Tick(DeltaSeconds,TickType) )
	{
		if( bUsePhysicsRotation )
		{
			physicsRotation(DeltaSeconds);
		}
		if ( PawnShadowMode == SHADOW_Self )
		{
			APawn* OldShadowPawn = ShadowPawn;

			if ( Pawn && Pawn->Mesh)
			{
				ShadowPawn = NULL;
				if ( bBehindView)
					ShadowPawn = Pawn;
			}
			else
			{
				ShadowPawn = NULL;
			}
			if ( ShadowPawn != OldShadowPawn )
			{
				if ( OldShadowPawn && OldShadowPawn->Mesh )
				{
					OldShadowPawn->Mesh->CastShadow = FALSE;
					OldShadowPawn->Mesh->bCastDynamicShadow = FALSE;
				}
				if ( ShadowPawn && ShadowPawn->Mesh )
				{
					ShadowPawn->Mesh->CastShadow = TRUE;
					ShadowPawn->Mesh->bCastDynamicShadow = TRUE;					
				}
				else if ( Pawn  && Pawn->Mesh )
				{
					Pawn->Mesh->CastShadow = FALSE;
					Pawn->Mesh->bCastDynamicShadow = FALSE;
				}
			}
		}
		return 1;
	}
	return 0;
}

FLOAT AavaPlayerController::GetGlobalSeconds()
{
	return (FLOAT)appSeconds();
}
