#include "PrecompiledHeaders.h"
#include "avaGame.h"

IMPLEMENT_CLASS( AavaGameReplicationInfo );
IMPLEMENT_CLASS( UavaDecalLifetimeRound );
IMPLEMENT_CLASS( UavaDecalLifetimeDataRound );
IMPLEMENT_CLASS( UavaDecalLifetimeRoundAnother );
IMPLEMENT_CLASS( UavaDecalLifetimeDataRoundAnother );

float AavaGameReplicationInfo::GetAverageFPS()
{
#if STATS
	return 1.0 / GFPSCounter.GetAverage();
#else
	return 0.0f;
#endif
}

void UavaDecalLifetimeRound::AddDecal(UDecalComponent* InDecalComponent)
{
	if (!GWorld) return;
	if (!GWorld->GetWorldInfo()) return;

	if ( ManagedDecals.Num() >= Max( 1, MaxDecal ) )
	{
		EliminateDecal( ManagedDecals( 0 ) );
	}

	Super::AddDecal( InDecalComponent );
}

void UavaDecalLifetimeRound::Tick(FLOAT)
{
	if (!GWorld) return;
	if (!GWorld->GetWorldInfo()) return;
	if (!GWorld->GetWorldInfo()->GRI) return;

	INT CurrentRound = Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI )->CurrentRound;

	// Accumulate a list of decals to kill.
	TArray<UDecalComponent*> KilledDecals;

	for ( INT DecalIndex = ManagedDecals.Num() - 1 ; DecalIndex >= 0 ; --DecalIndex )
	{
		UDecalComponent*		Decal = ManagedDecals( DecalIndex );
		UavaDecalLifetimeDataRound*	LifetimeData = Cast<UavaDecalLifetimeDataRound>( Decal->LifetimeData );		

		// Kill off the decal if it older than its lifespan.
		const UBOOL bDecalAlive = LifetimeData->Round == CurrentRound;
		if ( !bDecalAlive )
		{
			ManagedDecals.Remove( DecalIndex );
			KilledDecals.AddItem( Decal );
		}
		else
		{
			return;
		}
	}

	// Eliminate decals marked for killing.
	for ( INT DecalIndex = 0 ; DecalIndex < KilledDecals.Num() ; ++DecalIndex )
	{
		EliminateDecal( KilledDecals( DecalIndex ) );
	}
}	
