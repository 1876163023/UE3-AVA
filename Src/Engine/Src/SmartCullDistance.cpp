/*=============================================================================
SceneRendering.cpp: Scene rendering.
Copyright 2006-2007, Redduck Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "EngineParticleClasses.h"
#include "UnTerrain.h"

#if !FINAL_RELEASE
UBOOL GUpdatingCullDistances = FALSE;
INT GCullDistanceUpdateCount, GCullDistanceUpdateCount2;

static TMap<FString, FSmartCullDistance> GPendingCullDistanceData;
static TArray<FBox> GLastUpdatedCullDistanceBounds;
static FLOAT GLastUpdateCullDistanceTime;

UBOOL ShouldUpdateCullDistance( const UPrimitiveComponent* Component )
{	
	if (!Component || Component->HiddenGame) return FALSE;

	if (Component->GetOwner() && !Component->GetOwner()->bStatic) return FALSE;

	if (Component->DepthPriorityGroup != SDPG_World) return FALSE;	

	return Component->IsA( UModelComponent::StaticClass() ) || 
		Component->IsA( UTerrainComponent::StaticClass() ) || 
		Component->IsA( UStaticMeshComponent::StaticClass() ) || 
		Component->IsA( USkeletalMeshComponent::StaticClass() ) || 
		Component->IsA( UParticleSystemComponent::StaticClass() );		
}

FString MakePersistentComponentName( UPrimitiveComponent* Component )
{
	AActor* Actor = Component->GetOwner();
	if (!Actor)
	{
		UModelComponent* ModelComponent = Cast<UModelComponent>( Component );
		check(ModelComponent);

		ULevel* Level = Cast<ULevel>( ModelComponent->GetOuter() );
		check(Level);

		for (INT i=0; i<Level->ModelComponents.Num(); ++i)
		{
			if (Level->ModelComponents(i) == ModelComponent)
			{
				return FString::Printf( TEXT("%s.%s.ModelComponents(%d)"), *Level->GetOuter()->GetName(), *Level->GetName(), i );
			}
		}		

		check(FALSE);
	}	

	ATerrain* Terrain = Cast<ATerrain>(Actor);

	if (Terrain != NULL)
	{
		for (INT i=0; i<Terrain->TerrainComponents.Num(); ++i)
		{
			if (Terrain->TerrainComponents(i) == Component)
			{
				return FString::Printf( TEXT("%s.%s.TerrainComponents(%d)"), *Actor->GetOuter()->GetName(), *Actor->GetName(), i );
			}
		}
	}
	else
	{
		for (INT i=0; i<Actor->Components.Num(); ++i)
		{
			if (Actor->Components(i) == Component)
			{
				return FString::Printf( TEXT("%s.%s.Components(%d)"), *Actor->GetOuter()->GetName(), *Actor->GetName(), i );
			}
		}
	}	

	return FString::Printf( TEXT("%s.%s.%s"), *Actor->GetOuter()->GetName(), *Actor->GetName(), *Component->GetName() );
}

void UpdateCullDistance( const UPrimitiveComponent* Primitive, const FVector4& ViewOrigin )
{
	check( IsInRenderingThread() );	

	/// Static한 actor에 있는 component만 대상이다!
	if (ShouldUpdateCullDistance( Primitive ))
	{
		FVector OrgDelta = Primitive->SceneInfo->Bounds.Origin - ViewOrigin;	

		UBOOL bRegistered = FALSE;

		for (INT i=0; i<9; ++i)
		{
			FLOAT Distance = 0.0f;
			FVector Delta = OrgDelta;	

			INT x = i/3 - 1;
			INT y = i%3 - 1;

			Delta.X += 16.0f * x;
			Delta.Y += 16.0f * y;

			if(ViewOrigin.W > 0.0f)
			{
				Distance = FVector2D(Delta.X, Delta.Y).Size();
			}

#define MARGIN 128.0f

			const INT Index = FSmartCullDistance::FindIndex(Delta.X,Delta.Y);
			if (Primitive->CullDistanceEx.Value[Index] < Distance + MARGIN)
			{
				extern INT GCullDistanceUpdateCount;
				GCullDistanceUpdateCount++;

				if (Primitive->CullDistanceEx.Value[Index] < Distance)
				{
					extern INT GCullDistanceUpdateCount2;
					GCullDistanceUpdateCount2++;
				}				

				Primitive->SceneInfo->UpdateCullDistance( Index, Distance + MARGIN );

				if (!bRegistered)
				{
					bRegistered = TRUE;

					FString PersistenComponentName = MakePersistentComponentName( const_cast<UPrimitiveComponent*>(Primitive) );
					GPendingCullDistanceData.Set( *PersistenComponentName, Primitive->CullDistanceEx );

					if ( GLastUpdateCullDistanceTime < GWorld->GetTimeSeconds())
					{
						GLastUpdatedCullDistanceBounds.Empty();
					}

					GLastUpdateCullDistanceTime = GWorld->GetTimeSeconds();

					GLastUpdatedCullDistanceBounds.AddItem( Primitive->SceneInfo->Bounds.GetBox() );					
				}
			}

			for (INT i=0; i<ARRAY_COUNT(Primitive->CullDistanceEx.Value); ++i)
			{
				if (Primitive->CullDistanceEx.Value[i] == 0)
				{
					Primitive->SceneInfo->UpdateCullDistance( i, 1.0f );					
				}
			}
		}
	}
}

void DrawCullDistanceDebugInfo()
{
	for (INT i=0; i<GLastUpdatedCullDistanceBounds.Num(); ++i)
	{
		DrawWireBox( GWorld->LineBatcher, GLastUpdatedCullDistanceBounds(i), FColor( 0xff00ffff ), SDPG_World );		
	}
}

class CullDistance_PrimitiveComponentIterator
{
public :
	virtual void Process( UPrimitiveComponent* ) = NULL;

	void Run()
	{
		for( INT i=0; i<GWorld->Levels.Num(); ++i)
		{
			ULevel* Level = GWorld->Levels(i);

			for (INT j=0; j<Level->ModelComponents.Num(); ++j)
			{
				UPrimitiveComponent* Component = Level->ModelComponents(j);

				Process( Level->ModelComponents(j) );
			}				

			Level->MarkPackageDirty();
		}

		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;

			if (Actor->bStatic)
			{				
				ATerrain* Terrain = Cast<ATerrain>( Actor );

				if (Terrain != NULL)
				{
					for (INT i=0; i<Terrain->TerrainComponents.Num(); ++i)
					{
						UPrimitiveComponent* Component = Cast<UPrimitiveComponent>( Terrain->TerrainComponents(i) );

						if (!ShouldUpdateCullDistance( Component )) continue;

						Process( Component );
					}
				}
				else
				{								
					for (INT i=0; i<Actor->Components.Num(); ++i)
					{
						UPrimitiveComponent* Component = Cast<UPrimitiveComponent>( Actor->Components(i) );

						if (!ShouldUpdateCullDistance( Component )) continue;

						Process( Component );
					}
				}
			}				
		}			
	}
};

#if !FINAL_RELEASE
INT GCullDistanceRequestCounter = 0;

void BeginUpdatingCullDistances()
{	
	GCullDistanceUpdateCount = 0;
	GCullDistanceUpdateCount2 = 0;

	ENQUEUE_UNIQUE_RENDER_COMMAND( ToggleUpdateCullDistances, 
	{
		GCullDistanceRequestCounter = 30; // activate updating cull distance 30 frames later		
	}
	);
}

void EndUpdatingCullDistances()
{
	ENQUEUE_UNIQUE_RENDER_COMMAND( ToggleUpdateCullDistances, 
	{
		GCullDistanceRequestCounter = 0;
		GUpdatingCullDistances = 0; // activate updating cull distance 30 frames later		
	}
	);
}

UBOOL HasRequestedUpdatingCullDistances()
{
	FlushRenderingCommands();

	return GUpdatingCullDistances || GCullDistanceRequestCounter > 0;
}
#endif

UBOOL ExecSmartCullDistanceCommands( const TCHAR* Cmd, FOutputDevice& Ar )
{
#if !FINAL_RELEASE
	if( ParseCommand(&Cmd,TEXT("UPDATECULLDISTANCES")))
	{
		extern INT GPrecomputingFlags;
		if (GPrecomputingFlags == 0)
		{
			Ar.Log( TEXT("Cull distance should be updated within PIE-precompute mode") );
			return 1;
		}	

		if (HasRequestedUpdatingCullDistances())
		{
			EndUpdatingCullDistances();
		}
		else
		{
			BeginUpdatingCullDistances();
		}

		

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("APPLYCULLDISTANCES")))
	{
		class UpdateCullDistance_Iterator : public CullDistance_PrimitiveComponentIterator
		{
		public :
			virtual void Process( UPrimitiveComponent* Component )
			{
				FString Name = MakePersistentComponentName( Component );

				const FSmartCullDistance* Value = GPendingCullDistanceData.Find( Name );					

				if (Value != NULL)
				{
					Component->CullDistanceEx = *Value;
				}				

				GWorld->Scene->UpdatePrimitiveTransform( Component );						

				Component->PostEditChange( NULL );

				Component->GetOutermost()->MarkPackageDirty();					
			}
		} Instance;			

		Instance.Run();						

		GPendingCullDistanceData.Empty();

		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("RESETCULLDISTANCES")))
	{
		GPendingCullDistanceData.Empty();

		class UpdateCullDistance_Iterator : public CullDistance_PrimitiveComponentIterator
		{
		public :
			TMap<FString, FLOAT> Data;

			virtual void Process( UPrimitiveComponent* Component )
			{
				for (INT i=0; i<8; ++i) 
					Component->CullDistanceEx.Value[i] = 0;
			}
		} Instance;			

		Instance.Run();		

		return TRUE;
	}
	else
		return FALSE;
#else
	return FALSE;
#endif
}

#endif