/*=============================================================================
	UnLight.cpp: Bsp light mesh illumination builder code
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "LightingBuildOptions.h"
#include "StaticLightingPrivate.h"

// Don't compile the static lighting system on consoles.
#if !CONSOLE

FStaticLightingSystem::FStaticLightingSystem(const FLightingBuildOptions& InOptions):
	Options(InOptions),
	bBuildCanceled(FALSE)
{
	DOUBLE StartTime = appSeconds();

	// Prepare lights for rebuild.
	for(TObjectIterator<ULightComponent> LightIt;LightIt;++LightIt)
	{
		ULightComponent* const Light = *LightIt;
		const UBOOL bLightIsInWorld = Light->GetOwner() && GWorld->ContainsActor(Light->GetOwner());
		if(bLightIsInWorld)
		{
			// Make sure the light GUIDs and volumes are up-to-date.
			Light->ValidateLightGUIDs();
			Light->UpdateVolumes();

			// Add the light to the system's list of lights in the world.
			Lights.AddItem(Light);
		}
	}

	// Gather static lighting info from actor components.
	for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* const Level = GWorld->Levels(LevelIndex);
		const UBOOL bBuildLightingForLevel = Options.ShouldBuildLightingForLevel( Level );

		// Gather static lighting info from BSP.
		const UBOOL bBuildBSPLighting = 
			bBuildLightingForLevel &&
			Options.bBuildBSP &&
			!Options.bOnlyBuildSelectedActors;
		for( INT ComponentIndex=0; ComponentIndex<Level->ModelComponents.Num(); ComponentIndex++ )
		{
			UModelComponent* ModelComponent = Level->ModelComponents(ComponentIndex);
			if( ModelComponent )
			{
				AddPrimitiveStaticLightingInfo(ModelComponent,bBuildBSPLighting);
			}
		}

		// Gather static lighting info from actors.
		for(INT ActorIndex = 0;ActorIndex < Level->Actors.Num();ActorIndex++)
		{
			AActor* Actor = Level->Actors(ActorIndex);
			if(Actor)
			{
				const UBOOL bBuildActorLighting =
					bBuildLightingForLevel &&
					Options.bBuildActors &&
					(!Options.bOnlyBuildSelectedActors || Actor->IsSelected());

				// Gather static lighting info from each of the actor's components.
				for(INT ComponentIndex = 0;ComponentIndex < Actor->AllComponents.Num();ComponentIndex++)
				{
					UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Actor->AllComponents(ComponentIndex));
					if(Primitive)
					{
						AddPrimitiveStaticLightingInfo(Primitive,bBuildActorLighting);
					}
				}
			}
		}
	}

	// Prepare the aggregate mesh for raytracing.
	AggregateMesh.PrepareForRaytracing();

	// Stop the rendering thread while lighting is being built, since it busy-waits for the game thread.
	StopRenderingThread();

	// Spawn the static lighting threads.
	const UINT NumStaticLightingThreads = Max<UINT>(0,GNumHardwareThreads - NUM_STATIC_LIGHTING_UNUSED_THREADS);
	for(UINT ThreadIndex = 1;ThreadIndex < NumStaticLightingThreads;ThreadIndex++)
	{
		FStaticLightingThreadRunnable* ThreadRunnable = new(Threads) FStaticLightingThreadRunnable(this);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TEXT("StaticLightingThread"), 0, 0, 0, TPri_Normal);
	}

	// Begin the static lighting progress bar.
	GWarn->BeginSlowTask(TEXT("Building static lighting"),TRUE);

	// Start the static lighting thread loop on the main thread, too.
	// Once it returns, all static lighting mappings have begun processing.
	ThreadLoop(TRUE);

	// Wait for the static lighting threads to finish, and destroy them.
	for(INT ThreadIndex = 0;ThreadIndex < Threads.Num();ThreadIndex++)
	{
		Threads(ThreadIndex).Thread->WaitForCompletion();
		GThreadFactory->Destroy(Threads(ThreadIndex).Thread);
	}
	Threads.Empty();

	// Apply the last of the completed mappings.
	CompleteVertexMappingList.ApplyAndClear();
	CompleteTextureMappingList.ApplyAndClear();

	// End the static lighting progress bar.
	GWarn->EndSlowTask();

	// Restart the rendering thread after the lighting has been built.
	if(GUseThreadedRendering)
	{
		StartRenderingThread();
	}

	// Flush pending shadow-map and light-map encoding.
	DOUBLE EncodingStartTime = appSeconds();
	UShadowMap2D::FinishEncoding();
	FLightMap2D::FinishEncoding();

	// Ensure all primitives which were marked dirty by the lighting build are updated.
	DOUBLE UpdateComponentsStartTime = appSeconds();
	GWorld->UpdateComponents(FALSE);

	// Clear the world's lighting needs rebuild flag.
	if(!bBuildCanceled)
	{
		GWorld->GetWorldInfo()->SetMapNeedsLightingFullyRebuilt( FALSE );
	}

	// Clean up old shadow-map and light-map data.
	UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

	// Log execution time.
	warnf( NAME_Log, TEXT("Illumination: %7.2f seconds (%7.2f seconds encoding lightmaps)"), appSeconds() - StartTime, UpdateComponentsStartTime - EncodingStartTime );
	warnf( NAME_Log, TEXT("%7.2f thread-seconds"),(appSeconds() - StartTime) * NumStaticLightingThreads);
}

template<typename StaticLightingDataType>
void FStaticLightingSystem::TCompleteStaticLightingList<StaticLightingDataType>::ApplyAndClear()
{
	while(FirstElement)
	{
		// Atomically read the complete list and clear the shared head pointer.
		TList<StaticLightingDataType>* LocalFirstElement;
		do { LocalFirstElement = FirstElement; }
		while(appInterlockedCompareExchangePointer((void**)&FirstElement,NULL,LocalFirstElement) != LocalFirstElement);

		// Traverse the local list.
		TList<StaticLightingDataType>* Element = LocalFirstElement;
		while(Element)
		{
			// Flag the lights in the mapping's light-map as having been used in a light-map.
			if(Element->Element.LightMapData)
			{
				for(INT LightIndex = 0;LightIndex < Element->Element.LightMapData->Lights.Num();LightIndex++)
				{
					Element->Element.LightMapData->Lights(LightIndex)->bHasLightEverBeenBuiltIntoLightMap = TRUE;
				}
			}

			// Apply the mapping's new static lighting.
			Element->Element.Mapping->Apply(Element->Element.LightMapData,Element->Element.ShadowMaps);

			// Delete this link and advance to the next.
			TList<StaticLightingDataType>* NextElement = Element->Next;
			delete Element;
			Element = NextElement;
		};
	};
}

void FStaticLightingSystem::AddPrimitiveStaticLightingInfo(UPrimitiveComponent* Primitive,UBOOL bBuildLightingForComponent)
{
	// Find the lights relevant to the primitive.
	TArray<ULightComponent*> PrimitiveRelevantLights;
	for(INT LightIndex = 0;LightIndex < Lights.Num();LightIndex++)
	{
		ULightComponent* Light = Lights(LightIndex);
		if(	!Light->IsA(USkyLightComponent::StaticClass()) &&
			Light->AffectsPrimitive(Primitive))
		{
			PrimitiveRelevantLights.AddItem(Light);
		}
	}

	// Query the component for its static lighting info.
	FStaticLightingPrimitiveInfo PrimitiveInfo;
	Primitive->GetStaticLightingInfo(PrimitiveInfo,PrimitiveRelevantLights,Options);

	// Add the component's shadow casting meshes to the system.
	for(INT MeshIndex = 0;MeshIndex < PrimitiveInfo.Meshes.Num();MeshIndex++)
	{
		FStaticLightingMesh* Mesh = PrimitiveInfo.Meshes(MeshIndex);
		Meshes.AddItem(Mesh);
		AggregateMesh.AddMesh(Mesh);
	}

	// If lighting is being built for this component, add its mappings to the system.
	if(bBuildLightingForComponent)
	{
		for(INT MappingIndex = 0;MappingIndex < PrimitiveInfo.Mappings.Num();MappingIndex++)
		{
			Mappings.AddItem(PrimitiveInfo.Mappings(MappingIndex));
		}
	}
}

UBOOL FStaticLightingSystem::CalculatePointShadowing(const FStaticLightingMapping* Mapping,const FVector& WorldSurfacePoint,ULightComponent* Light,FCoherentRayCache& CoherentRayCache) const
{
	// Treat points which the light doesn't affect as shadowed to avoid the costly ray check.
	if(!Light->AffectsBounds(FBoxSphereBounds(WorldSurfacePoint,FVector(0,0,0),0)))
	{
		return TRUE;

	}

	// Check for visibility between the point and the light.
	UBOOL bIsShadowed = FALSE;
	if(Light->CastShadows && Light->CastStaticShadows)
	{
		const FVector4 LightPosition = Light->GetPosition();

		// Construct a line segment between the light and the surface point.
		const FVector LightVector = (FVector)LightPosition - WorldSurfacePoint * LightPosition.W;
		const FShadowRay ShadowRay(
			WorldSurfacePoint + LightVector.SafeNormal() * SHADOW_VISIBILITY_DISTANCE_BIAS,
			WorldSurfacePoint + LightVector,
			Mapping,
			Light
			);

		// Check the line segment for intersection with the static lighting meshes.
		bIsShadowed = AggregateMesh.DoesShadowRayIntersect(ShadowRay,CoherentRayCache);
	}

	return bIsShadowed;
}

FLightSample FStaticLightingSystem::CalculatePointLighting(const FStaticLightingMapping* Mapping,const FStaticLightingVertex& Vertex,ULightComponent* Light) const
{
	// Calculate the direction from the vertex to the light.
	const FVector4 LightPosition = Light->GetPosition();
	const FVector WorldLightVector = (FVector)LightPosition - Vertex.WorldPosition * LightPosition.W;

	// Transform the light vector to tangent space.
	const FVector TangentLightVector = 
		FVector(
			WorldLightVector | Vertex.WorldTangentX,
			WorldLightVector | Vertex.WorldTangentY,
			WorldLightVector | Vertex.WorldTangentZ
			).SafeNormal();
	// Compute the incident lighting of the light on the vertex.
	return FLightSample(Light->GetDirectIntensity(Vertex.WorldPosition),TangentLightVector);
}

void FStaticLightingSystem::ThreadLoop(UBOOL bIsMainThread)
{
	UBOOL bIsDone = FALSE;
	while(!bIsDone && !bBuildCanceled)
	{
		// Atomically read and increment the next mapping index to process.
		INT MappingIndex = NextMappingToProcess.Increment() - 1;

		if(MappingIndex < Mappings.Num())
		{
			// If this is the main thread, update progress and apply completed static lighting.
			if(bIsMainThread)
			{
				// Update the progress bar.
				GWarn->StatusUpdatef(MappingIndex,Mappings.Num(),TEXT("Building static lighting"));

				// Apply completed static lighting.
				CompleteVertexMappingList.ApplyAndClear();
				CompleteTextureMappingList.ApplyAndClear();

				// Check the for build cancellation.
				if(GEditor->GetMapBuildCancelled())
				{
					bBuildCanceled = TRUE;
				}
			}

			// Build the mapping's static lighting.
			if(Mappings(MappingIndex)->GetVertexMapping())
			{
				ProcessVertexMapping(Mappings(MappingIndex)->GetVertexMapping());
			}
			else if(Mappings(MappingIndex)->GetTextureMapping())
			{
				ProcessTextureMapping(Mappings(MappingIndex)->GetTextureMapping());
			}
		}
		else
		{
			// Processing has begun for all mappings.
			bIsDone = TRUE;
		}
	}
}

void SortMappingLightsByLightingType(const FStaticLightingMapping* Mapping,TArray<ULightComponent*>& OutLightMappedLights,TArray<ULightComponent*>& OutShadowMappedLights)
{
	for(INT LightIndex = 0;LightIndex < Mapping->Mesh->RelevantLights.Num();LightIndex++)
	{
		ULightComponent* Light = Mapping->Mesh->RelevantLights(LightIndex);

		const UBOOL bUseStaticLighting = Light->UseStaticLighting(Mapping->bForceDirectLightMap);
		if(bUseStaticLighting)
		{
			OutLightMappedLights.AddItem(Light);
		}
		else
		{
			OutShadowMappedLights.AddItem(Light);
		}
	}
}

UBOOL IsLightBehindSurface(const FVector& TrianglePoint,const FVector& TriangleNormal,const ULightComponent* Light)
{
	// Calculate the direction from the triangle to the light.
	const FVector4 LightPosition = Light->GetPosition();
	const FVector WorldLightVector = (FVector)LightPosition - TrianglePoint * LightPosition.W;

	// Check if the light is in front of the triangle.
	const FLOAT Dot = WorldLightVector | TriangleNormal;
	return Dot < 0.0f;
}

FBitArray CullBackfacingLights(const FVector& TrianglePoint,const FVector& TriangleNormal,const TArray<ULightComponent*>& Lights)
{
	FBitArray Result(FALSE,Lights.Num());
	for(INT LightIndex = 0;LightIndex < Lights.Num();LightIndex++)
	{
		Result(LightIndex) = !IsLightBehindSurface(TrianglePoint,TriangleNormal,Lights(LightIndex));
	}
	return Result;
}

struct FRadiosityLightingSystem
{
	const FLightingBuildOptions Options;

	FRadiosityLightingSystem( const FLightingBuildOptions& InOptions )
		: Options(InOptions)
	{
		DOUBLE	StartTime		= appSeconds();
		DOUBLE	BSPTime			= 0;
		DOUBLE	ActorTime		= 0;
		DOUBLE	EncodingTime	= 0;

		//!{ 2006-04-03	허 창 민

		// Stop the rendering thread while lighting is being built, since it busy-waits for the game thread.
		StopRenderingThread();

		AvaRadiosityAdapter RadiosityAdapter;

		if( Options.bUseRadiosityLighting || Options.bUseRayTraceLighting )
		{
			bool bStart = RadiosityAdapter.StartUp();
			bool bRender = false;

			if( bStart )
			{
				AvaTextureData DefaultTextureData;
				DefaultTextureData.SourceMaterial_ = NULL;
				DefaultTextureData.SizeX_ = 100;
				DefaultTextureData.SizeY_ = 100;
				DefaultTextureData.Reflectivity_ = FVector( 0.9f, 0.9f, 0.9f );
				DefaultTextureData.ReflectivityScale_ = 1.0f;
				DefaultTextureData.Brightness_ = FVector( 0, 0, 0 );
				RadiosityAdapter.TextureDatas_.AddItem( DefaultTextureData );

				RadiosityAdapter.GatherWorldInfo(Options);
				bRender = RadiosityAdapter.Render(Options);

				if( !bRender )
				{
					warnf( TEXT("-------- Build Radiosity Lighting is Failed -----------") );
					RadiosityAdapter.ShutDown();
					return;
				}
			}
			else
			{
				warnf( TEXT("-------- Start Up Radiosity Lighting is Failed -----------") );
				return;
			}
		}

		//!} 2006-04-03	허 창 민		

		// Make sure all light components have GUIDs and their volumes are up-to-date.
		for( TObjectIterator<ULightComponent> It; It; ++It )
		{
			ULightComponent* LightComponent	= *It;
			LightComponent->ValidateLightGUIDs();
			LightComponent->UpdateVolumes();
		}

		UBOOL	bAbortLightLevel	= FALSE;
		ULevel*	CachedCurrentLevel	= GWorld->CurrentLevel;

		// Iterate over all levels and perform lighting rebuild duties on them.
		for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
		{
			ULevel* Level = GWorld->Levels(LevelIndex);

			if( Options.ShouldBuildLightingForLevel( Level ) )
			{	
				GWorld->CurrentLevel = Level;

				{
					struct FAvaLightmapPacker
					{
						ULevel* Level;
						DOUBLE& EncodingTime;

						FAvaLightmapPacker( AvaRadiosityAdapter& Adapter, ULevel* InLevel, DOUBLE& InEncodingTime ) 
							: Level(InLevel), EncodingTime(InEncodingTime)
						{
							extern void AvaBeginPackingLightmap( AvaRadiosityAdapter& RadiosityAdapter );

							AvaBeginPackingLightmap( Adapter );
						}

						~FAvaLightmapPacker()
						{	
							extern void AvaEndPackingLightmap();

							// Encode all allocated light-maps and shadow-maps.
							DOUBLE EncodingStartTime = appSeconds();

							AvaEndPackingLightmap();
							
							FLightMap2D::FinishEncoding();
							UShadowMap2D::FinishEncoding();

							DOUBLE DeltaTime = appSeconds() - EncodingStartTime;
							warnf( NAME_Log, TEXT("Encoding lightmaps for %30s took %7.2f seconds."), *Level->GetOutermost()->GetName(), DeltaTime );
							EncodingTime += DeltaTime;
						}
					} ScopedPacker( RadiosityAdapter, Level, EncodingTime );				
					

					// Illuminate BSP.
					if( Options.bBuildBSP && !Options.bOnlyBuildSelectedActors )
					{
						DOUBLE	BSPStartTime				= appSeconds();
						INT		ModelComponentCount			= 0;
						INT		TotalModelComponentCount	= Level->ModelComponents.Num();

						GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("IlluminatingBSP")), TRUE );
						GWarn->StatusUpdatef(0,TotalModelComponentCount,*LocalizeUnrealEd(TEXT("IlluminatingBSPF")),0,TotalModelComponentCount);

						Level->InvalidateModelSurface();					

						for( INT ComponentIndex=0; ComponentIndex<Level->ModelComponents.Num(); ComponentIndex++ )
						{
							// Break out of this build function early if the map build was canceled.
							if( GEditor->GetMapBuildCancelled() )
							{
								bAbortLightLevel = TRUE;
								break;
							}

							// Cache lighting for model component.
							UModelComponent* ModelComponent = Level->ModelComponents(ComponentIndex);
							if( ModelComponent )
							{							
								ModelComponent->CacheRadiosityLighting( Options, RadiosityAdapter );							

								ModelComponent->MarkPackageDirty();
							}

							GWarn->StatusUpdatef(ModelComponentCount++,TotalModelComponentCount,*LocalizeUnrealEd(TEXT("IlluminatingBSPF")),ModelComponentCount,TotalModelComponentCount);
						}					

						Level->CommitModelSurfaces();

						GWarn->EndSlowTask();

						DOUBLE DeltaTime = appSeconds() - BSPStartTime;
						warnf( NAME_Log, TEXT("Lighting BSP for       %30s took %7.2f seconds."), *Level->GetOutermost()->GetName(), DeltaTime );
						BSPTime += DeltaTime;
					}

					if( bAbortLightLevel )
					{
						break;
					}

					// Illuminate actors.
					if( Options.bBuildActors && !bAbortLightLevel )
					{
						DOUBLE	ActorStartTime		= appSeconds();
						INT		ProgressDenominator = Level->Actors.Num();

						GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("IlluminatingActors")), TRUE );
						GWarn->StatusUpdatef(0, ProgressDenominator, *LocalizeUnrealEd(TEXT("IlluminatingActorsF")), 0, ProgressDenominator);					

						for( INT ActorIndex=0; ActorIndex<Level->Actors.Num(); ActorIndex++ )
						{
							// Break out of this build function early if the map build was cancelled.
							if( GEditor->GetMapBuildCancelled() )
							{
								bAbortLightLevel = TRUE;
								break;
							}

							AActor* Actor = Level->Actors(ActorIndex);
							// Only build selected actors if wanted.			
							if( Actor && (!Options.bOnlyBuildSelectedActors || Actor->IsSelected())	)
							{							
								Actor->CacheRadiosityLighting( Options, RadiosityAdapter );							

								Actor->MarkPackageDirty();
							}

							GWarn->StatusUpdatef(ActorIndex,ProgressDenominator,*LocalizeUnrealEd(TEXT("IlluminatingActorsF")),ActorIndex,ProgressDenominator);
						}										

						GWarn->EndSlowTask();

						DOUBLE DeltaTime = appSeconds() - ActorStartTime;
						warnf( NAME_Log, TEXT("Lighting Actors for    %30s took %7.2f seconds."), *Level->GetOutermost()->GetName(), DeltaTime );
						ActorTime += DeltaTime;
					}

					if( bAbortLightLevel )
					{
						break;
					}
				}				

				// Update the BSP light-map coordinates for this (current) level.
				GWorld->InvalidateModelSurface( TRUE );
			}	
		}

		//!{ 2006-04-10	허 창 민
		if( Options.bUseRadiosityLighting || Options.bUseRayTraceLighting )
		{
			RadiosityAdapter.ShutDown();

			// for debugging
			//GWorld->SampleDrawer->Samples.Empty();
			//RadiosityAdapter.GetRadiositySamples( &GWorld->SampleDrawer->Samples );
		}
		//!} 2006-04-10	허 창 민		
		//<@ ava specific ; 2006. 12. 05 changmin
		// bsp lightmap coordinate에 lightmap texture coordinate bias/scale을 적용합니다.
		if( !bAbortLightLevel )
		{
			for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); ++LevelIndex )
			{
				ULevel *Level = GWorld->Levels(LevelIndex);
				if( Options.ShouldBuildLightingForLevel( Level ) )
				{
					GWorld->CurrentLevel = Level;
					if( Options.bBuildBSP && !Options.bOnlyBuildSelectedActors )
					{
						for( INT ComponentIndex = 0; ComponentIndex < Level->ModelComponents.Num(); ++ComponentIndex )
						{
							UModelComponent *ModelComponent = Level->ModelComponents(ComponentIndex);
							if(ModelComponent)
							{
								ModelComponent->UpdateLightmapTextureCoordinate();
							}
						}

						for( INT ComponentIndex = 0; ComponentIndex < Level->ModelComponents.Num(); ++ComponentIndex )
						{
							UModelComponent *ModelComponent = Level->ModelComponents(ComponentIndex);
							if(ModelComponent)
							{	
								ModelComponent->ResetLightmapTextureCoordinateScaleAndBias();
							}
						}
					}
				}
			}
		}
		//>@ avature coordinate bias/scale을 적용합니다.
	
		// Update components.
		GWorld->UpdateComponents( FALSE );

		// Restore current level clobbered during lighting rebuild.
		GWorld->CurrentLevel = CachedCurrentLevel;

		// Lighting build has been aborted - clean up light/ shadow map data and fix up BSP.
		if( bAbortLightLevel == TRUE )
		{
			// Finish lightmap encoding, required to free memory.
			DOUBLE EncodingStartTime = appSeconds();
			FLightMap2D::FinishEncoding();
			UShadowMap2D::FinishEncoding();
			EncodingTime += appSeconds() - EncodingStartTime;

			// Invalidate lighting for all actors in levels we canceled building.
			for( FActorIterator It; It; ++It )
			{
				AActor* Actor = *It;
				if( Options.ShouldBuildLightingForLevel( Actor->GetLevel() ) )
				{
					Actor->InvalidateLightingCache();
				}
			}

			// Update the BSP light-map coordinates.
			GWorld->InvalidateModelSurface( Options.bOnlyBuildCurrentLevel );

			// Update components.
			GWorld->UpdateComponents( Options.bOnlyBuildCurrentLevel );
		}
		else
		{
			GWorld->GetWorldInfo()->SetMapNeedsLightingFullyRebuilt( FALSE );
		}

		// Restart the rendering thread after the lighting has been built.
		if(GUseThreadedRendering)
		{
			StartRenderingThread();
		}

		warnf( NAME_Log, TEXT("Illumination: %7.2f seconds (%7.2f seconds BSP, %7.2f seconds Actors, %7.2f seconds encoding lightmaps)"), appSeconds() - StartTime, BSPTime, ActorTime, EncodingTime );

		// Clean up old shadow map data.
		UObject::CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
	}
};

#endif

/**
 * Builds lighting information depending on passed in options.
 *
 * @param	Options		Options determining on what and how lighting is built
 */
void UEditorEngine::BuildLighting(const FLightingBuildOptions& Options)
{
#if !CONSOLE
	if (Options.bUseRadiosityLighting || Options.bUseRayTraceLighting )
	{
		FRadiosityLightingSystem System(Options);
	}
	else
	{	
		// Invoke the static lighting system.
		FStaticLightingSystem System(Options);
	}
#endif
}
