/*=============================================================================
	UnLevel.cpp: Level-related functions
	Copyright 1997-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"
#include "EngineSequenceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "UnOctree.h"
#include "UnTerrain.h"
#include "EnvCubePrivate.h"

IMPLEMENT_CLASS(ULineBatchComponent);

/*-----------------------------------------------------------------------------
	ULevelBase implementation.
-----------------------------------------------------------------------------*/

ULevelBase::ULevelBase( const FURL& InURL )
: Actors( this )
, URL( InURL )

{}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void ULevelBase::StaticConstructor()
{
	UClass* Class = GetClass();
	Class->EmitObjectArrayReference( STRUCT_OFFSET( ULevelBase, Actors ) );
}

void ULevelBase::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	Ar << Actors;
	Ar << URL;
}
IMPLEMENT_CLASS(ULevelBase);


/*-----------------------------------------------------------------------------
	ULevel implementation.
-----------------------------------------------------------------------------*/

//@deprecated with VER_SPLIT_SOUND_FROM_TEXTURE_STREAMING
struct FStreamableResourceInstanceDeprecated
{
	FSphere BoundingSphere;
	FLOAT TexelFactor;
	friend FArchive& operator<<( FArchive& Ar, FStreamableResourceInstanceDeprecated& ResourceInstance )
	{
		Ar << ResourceInstance.BoundingSphere;
		Ar << ResourceInstance.TexelFactor;
		return Ar;
	}
};
//@deprecated with VER_SPLIT_SOUND_FROM_TEXTURE_STREAMING
struct FStreamableResourceInfoDeprecated
{
	UObject* Resource;
	TArray<FStreamableResourceInstanceDeprecated> ResourceInstances;
	friend FArchive& operator<<( FArchive& Ar, FStreamableResourceInfoDeprecated& ResourceInfo )
	{
		Ar << ResourceInfo.Resource;
		Ar << ResourceInfo.ResourceInstances;
		return Ar;
	}
};
//@deprecated with VER_RENDERING_REFACTOR
struct FStreamableSoundInstanceDeprecated
{
	FSphere BoundingSphere;
	friend FArchive& operator<<( FArchive& Ar, FStreamableSoundInstanceDeprecated& SoundInstance )
	{
		Ar << SoundInstance.BoundingSphere;
		return Ar;
	}
};
//@deprecated with VER_RENDERING_REFACTOR
struct FStreamableSoundInfoDeprecated
{
	USoundNodeWave*	SoundNodeWave;
	TArray<FStreamableSoundInstanceDeprecated> SoundInstances;
	friend FArchive& operator<<( FArchive& Ar, FStreamableSoundInfoDeprecated& SoundInfo )
	{
		Ar << SoundInfo.SoundNodeWave;
		Ar << SoundInfo.SoundInstances;
		return Ar;
	}
};
//@deprecated with VER_RENDERING_REFACTOR
struct FStreamableTextureInfoDeprecated
{
	UTexture*							Texture;
	TArray<FStreamableTextureInstance>	TextureInstances;
	friend FArchive& operator<<( FArchive& Ar, FStreamableTextureInfoDeprecated& TextureInfo )
	{
		Ar << TextureInfo.Texture;
		Ar << TextureInfo.TextureInstances;
		return Ar;
	}
};


IMPLEMENT_CLASS(ULevel);

ULevel::ULevel( const FURL& InURL )
:	ULevelBase( InURL )
{
}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void ULevel::StaticConstructor()
{
	UClass* Class = GetClass();
	Class->EmitObjectReference( STRUCT_OFFSET( ULevel, Model ) );
	Class->EmitObjectArrayReference( STRUCT_OFFSET( ULevel, ModelComponents ) );
	Class->EmitObjectArrayReference( STRUCT_OFFSET( ULevel, GameSequences ) );
	Class->EmitObjectReference( STRUCT_OFFSET( ULevel, NavListStart ) );
	Class->EmitObjectReference( STRUCT_OFFSET( ULevel, NavListEnd ) );
	Class->EmitObjectReference( STRUCT_OFFSET( ULevel, CoverListStart ) );
	Class->EmitObjectReference( STRUCT_OFFSET( ULevel, CoverListEnd ) );
	Class->EmitObjectArrayReference( STRUCT_OFFSET( ULevel, CrossLevelActors ) );

	//<@ ava specific ; 2007. 2. 1 changmin
	Class->EmitObjectReference( STRUCT_OFFSET( ULevel, StructuralModel) );
	//>@ ava
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void ULevel::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );	

	for (INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; ++PlatformIndex)
	{
		for( TMap<UTexture2D*,TArray<FStreamableTextureInstance> >::TIterator It(TextureToInstancesMapPerPlatform[PlatformIndex]); It; ++It )
		{
			UTexture2D* Texture2D = It.Key();
			AddReferencedObject( ObjectArray, Texture2D );
		}
	}	
}

void ULevel::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << Model;

	//<@ ava specific ; 2007. 2. 1 changmin
	if( Ar.LicenseeVer() >= VER_AVA_ADD_STRUCTURAL_BSP_TO_LEVEL )
	{
		Ar << StructuralModel;
	}
	//>@ ava

	Ar << ModelComponents;

	Ar << GameSequences;

	if( !Ar.IsTransacting() )
	{
		if( Ar.Ver() < VER_SPLIT_SOUND_FROM_TEXTURE_STREAMING )
		{
			TArray<FStreamableResourceInfoDeprecated> ResourceInfos;
			Ar << ResourceInfos;
		}
		else
		{
			if( Ar.Ver() < VER_RENDERING_REFACTOR )
			{
				TArray<FStreamableTextureInfoDeprecated> TextureInfos;
				Ar << TextureInfos;
			}
			else
			{
				if (Ar.LicenseeVer() < VER_AVA_DONOTLOAD_NONRELEVANT_TEXTURES)
				{
					Ar << TextureToInstancesMap;

					for (INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; ++PlatformIndex)
					{
						TextureToInstancesMapPerPlatform[PlatformIndex] = TextureToInstancesMap;
					}

					TextureToInstancesMap.Empty();
				}
				else
				{
					for (INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; ++PlatformIndex)
					{
						const UBOOL bSkipReference = !GIsEditor && GIsGame && PlatformIndex != GRHIShaderPlatform;

						if (bSkipReference)
						{
							Ar.SetSkipObjectRef( TRUE );
						}

						Ar << TextureToInstancesMapPerPlatform[PlatformIndex];

						if (bSkipReference)
						{
							Ar.SetSkipObjectRef( FALSE );
						}
					}
				}
			}

			if( Ar.Ver() < VER_RENDERING_REFACTOR )
			{
				TArray<FStreamableSoundInfoDeprecated> SoundInfos;
				Ar << SoundInfos;
			}
		}
    
	    if( Ar.Ver() >= VER_PRECOOK_PHYS_BSP_TERRAIN )
	    {
		    CachedPhysBSPData.BulkSerialize(Ar);
	    }
    
	    if( Ar.Ver() >= VER_PRECOOK_PHYS_STATICMESH_CACHE )
	    {
		    Ar << CachedPhysSMDataMap;
		    Ar << CachedPhysSMDataStore;
	    }
	    
	    if( Ar.Ver() >= VER_PRECOOK_PERTRI_PHYS_STATICMESH )
	    {
		    Ar << CachedPhysPerTriSMDataMap;
		    Ar << CachedPhysPerTriSMDataStore;
	    }
    
	    if( Ar.Ver() >= VER_SAVE_PRECOOK_PHYS_VERSION )
	    {
		    Ar << CachedPhysBSPDataVersion;
		    Ar << CachedPhysSMDataVersion;
	    }

		if( Ar.Ver() >= VER_LEVEL_FORCE_STREAM_TEXTURES )
		{
			if (Ar.LicenseeVer() < VER_AVA_DONOTLOAD_NONRELEVANT_TEXTURES)
			{
				Ar << ForceStreamTextures;

				for (INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; ++PlatformIndex)
				{
					ForceStreamTexturesPerPlatform[PlatformIndex] = ForceStreamTextures;
				}

				ForceStreamTextures.Empty();
			}			
			else
			{
				for (INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; ++PlatformIndex)
				{
					const UBOOL bSkipReference = !GIsEditor && GIsGame && PlatformIndex != GRHIShaderPlatform;

					if (bSkipReference)
					{
						Ar.SetSkipObjectRef( TRUE );
					}

					Ar << ForceStreamTexturesPerPlatform[PlatformIndex];

					if (bSkipReference)
					{
						Ar.SetSkipObjectRef( FALSE );
					}
				}
			}
		}
	}

	// Mark archive and package as containing a map if we're serializing to disk.
	if( !HasAnyFlags( RF_ClassDefaultObject ) && Ar.IsPersistent() )
	{
		Ar.ThisContainsMap();
		GetOutermost()->ThisContainsMap();
	}

	if ( Ar.Ver() >= VER_PERLEVEL_NAVLIST )
	{
		// serialize the nav list
		Ar << NavListStart;
		Ar << NavListEnd;
		// and cover
		Ar << CoverListStart;
		Ar << CoverListEnd;
		// serialize the list of cross level actors
		Ar << CrossLevelActors;
	}
}


/**
 * Sorts the actor list by net relevancy and static behaviour. First all net relevant static
 * actors, then all remaining static actors and then the rest. This is done to allow the dynamic
 * and net relevant actor iterators to skip large amounts of actors.
 */
void ULevel::SortActorList()
{		
	INT				StartIndex	= 0;
	TArray<AActor*> NewActors;

	// The world info and default brush have fixed actor indices.
	NewActors.AddItem(Actors(StartIndex++));
	NewActors.AddItem(Actors(StartIndex++));

	// Static not net relevant actors.
	for( INT ActorIndex=StartIndex; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && !Actor->bDeleteMe && Actor->bStatic && !Actor->bAlwaysRelevant )
		{
			NewActors.AddItem( Actor );
		}
	}
	iFirstNetRelevantActor=NewActors.Num();

	// Static net relevant actors.
	for( INT ActorIndex=StartIndex; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);		
		if( Actor && !Actor->bDeleteMe && Actor->bStatic && Actor->bAlwaysRelevant )
		{
			NewActors.AddItem( Actor );
		}
	}
	iFirstDynamicActor=NewActors.Num();

	// Remaining (dynamic, potentially net relevant actors)
	for( INT ActorIndex=StartIndex; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);			
		if( Actor && !Actor->bDeleteMe && !Actor->bStatic )
		{
			NewActors.AddItem( Actor );
		}
	}

	// Replace with sorted list.
	Actors = NewActors;

	// Don't use sorted optimization outside of gameplay so we can safely shuffle around actors e.g. in the Editor
	// without there being a chance to break code using dynamic/ net relevant actor iterators.
	if( !GIsGame )
	{
		iFirstNetRelevantActor	= 0;
		iFirstDynamicActor		= 0;
	}
}

/**
 * Makes sure that all light components have valid GUIDs associated.
 */
void ULevel::ValidateLightGUIDs()
{
	for( TObjectIterator<ULightComponent> It; It; ++It )
	{
		ULightComponent*	LightComponent	= *It;
		UBOOL				IsInLevel		= LightComponent->IsIn( this );

		if( IsInLevel )
		{
			LightComponent->ValidateLightGUIDs();
		}
	}
}

/**
 * Presave function, gets called once before the level gets serialized (multiple times) for saving.
 * Used to rebuild streaming data on save.
 */
void ULevel::PreSave()
{
	Super::PreSave();

	if( !IsTemplate() )
	{
		UPackage* Package = CastChecked<UPackage>(GetOutermost());

		ValidateLightGUIDs();

		// Don't rebuild streaming data if we're saving out a cooked package as the raw data required has already been stripped.
		if( !(Package->PackageFlags & PKG_Cooked) )
		{
			BuildStreamingData();
		}

		// Build bsp-trimesh data for physics engine
		BuildPhysBSPData();

		// clean up the nav list
		GWorld->RemoveLevelNavList(this);

		// clear out any crosslevel references
		for (INT ActorIdx = 0; ActorIdx < Actors.Num(); ActorIdx++)
		{
			AActor *Actor = Actors(ActorIdx);
			if (Actor != NULL)
			{
				if (Actor->Base != NULL && Actor->GetOutermost() != Actor->Base->GetOutermost())
				{
					Actor->SetBase(NULL);
				}
				ANavigationPoint *Nav = Cast<ANavigationPoint>(Actor);
				if (Nav != NULL)
				{
					for (INT PathIdx = 0; PathIdx < Nav->PathList.Num(); PathIdx++)
					{
						UReachSpec *Spec = Nav->PathList(PathIdx);
						if (Spec == NULL ||
							Spec->Start == NULL ||
							(*Spec->End == NULL && !Spec->End.Guid.IsValid()) ||
							Spec->Start != Nav)
						{
							Nav->PathList.Remove(PathIdx--,1);
							continue;
						}
						if (*Spec->End != NULL && Spec->Start->GetOutermost() != Spec->End->GetOutermost())
						{
							Nav->bHasCrossLevelPaths = TRUE;
							Spec->End.Guid = Spec->End->NavGuid;
						}
					}
				}
			}
		}
		// build the list of cross level actors
		CrossLevelActors.Empty();
		for (INT ActorIdx = 0; ActorIdx < Actors.Num(); ActorIdx++)
		{
			AActor *Actor = Actors(ActorIdx);
			if (Actor != NULL && !Actor->IsPendingKill())
			{
				TArray<FNavReference*> NavRefs;
				Actor->GetNavReferences(NavRefs,TRUE);
				Actor->GetNavReferences(NavRefs,FALSE);
				if (NavRefs.Num() > 0)
				{
					// and null the cross level references
					UBOOL bHasCrossLevelRef = FALSE;
					for (INT Idx = 0; Idx < NavRefs.Num(); Idx++)
					{
						if (NavRefs(Idx)->Nav == NULL || Cast<ULevel>(NavRefs(Idx)->Nav->GetOuter()) != this)
						{
							bHasCrossLevelRef = TRUE;
							NavRefs(Idx)->Nav = NULL;
						}
					}
					if (bHasCrossLevelRef)
					{
						CrossLevelActors.AddItem(Actor);
					}
				}
			}
		}
	}
}

/**
 * Removes existing line batch components from actors and associates streaming data with level.
 */
void ULevel::PostLoad()
{
	Super::PostLoad();

	//@AVA ; SM2에선 lightmap을 postload에서 바로 Updateresource하지 않는다.
	if (!GIsEditor && GIsGame && IsSM2Platform(GRHIShaderPlatform))
	{
		TArray<UTexture2D*> LightmapTexturesToBeInitialized;

		for( INT ModelComponentIndex=0; ModelComponentIndex<ModelComponents.Num(); ++ModelComponentIndex)
		{
			UModelComponent* ModelComponent = ModelComponents(ModelComponentIndex);		

			for (INT i=0; i<ModelComponent->GetElements().Num(); ++i)
			{
				const FModelElement& Element = ModelComponent->GetElements()(i);

				FLightMap2D* Lightmap = (FLightMap2D*)((FLightMap*)Element.LightMap);

				if (!Lightmap) continue;		

				for (INT j=0; j<NUM_AVA_LIGHTMAPS; ++j)
				{
					const UTexture2D* Texture = Lightmap->GetTexture(j);

					if (Texture)
						LightmapTexturesToBeInitialized.AddUniqueItem((UTexture2D*)Texture);				
				}		
			}
		}

		for (INT i=0; i<LightmapTexturesToBeInitialized.Num(); ++i)
		{
			LightmapTexturesToBeInitialized(i)->UpdateResource();
		}
	}

	//@todo: investigate removal of code cleaning up existing LineBatchComponents.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if(Actor)
		{
			for( INT ComponentIndex=0; ComponentIndex<Actor->Components.Num(); ComponentIndex++ )
			{
				UActorComponent* Component = Actor->Components(ComponentIndex);
				if( Component && Component->IsA(ULineBatchComponent::StaticClass()) )
				{
					check(!Component->IsAttached());
					Actor->Components.Remove(ComponentIndex--);
				}

 
 				//!{ 2006-06-07	허 창 민
 				if( Component && Component->IsA(USampleComponent::StaticClass()) )
 				{
 					check(!Component->IsAttached());
 					Actor->Components.Remove(ComponentIndex--);
 				}
 				//!} 2006-06-07	허 창 민
 
 
 			}
 		}
 	} 
}

/**
 * Clears all components of actors associated with this level (aka in Actors array) and 
 * also the BSP model components.
 */
void ULevel::ClearComponents()
{
	bAreComponentsCurrentlyAttached = FALSE;

	// Remove the model components from the scene.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->ConditionalDetach();
		}
	}

	// Remove the actors' components from the scene.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor )
		{
			Actor->ClearComponents();
		}
	}
}

/**
 * A TMap key type used to sort BSP nodes by locality and zone.
 */
struct FModelComponentKey
{
	UINT	ZoneIndex;
	UINT	X;
	UINT	Y;
	UINT	Z;
	DWORD	MaskedPolyFlags;

	//<@ ava specific ; 2006. 11. 17 changmin
	INT		ClusterNumber;
	//>@ ava

	friend UBOOL operator==(const FModelComponentKey& A,const FModelComponentKey& B)
	{
		return	A.ZoneIndex == B.ZoneIndex 
			&&	A.X == B.X 
			&&	A.Y == B.Y 
			&&	A.Z == B.Z 
			&&	A.MaskedPolyFlags == B.MaskedPolyFlags
			//<@ ava specific ; 2006. 11. 17 changmin
			&&	A.ClusterNumber == B.ClusterNumber;
			//>@ ava
	}

	friend DWORD GetTypeHash(const FModelComponentKey& Key)
	{
		return appMemCrc(&Key,sizeof(Key),0);
	}
};

/**
 * Updates all components of actors associated with this level (aka in Actors array) and 
 * creates the BSP model components.
 */
void ULevel::UpdateComponents()
{
	// Update all components in one swoop.
	IncrementalUpdateComponents( 0 );
}

/**
 * Incrementally updates all components of actors associated with this level.
 *
 * @param NumComponentsToUpdate	Number of components to update in this run, 0 for all
 */
void ULevel::IncrementalUpdateComponents( INT NumComponentsToUpdate )
{
	// A value of 0 means that we want to update all components.
	if( NumComponentsToUpdate == 0 )
	{
		NumComponentsToUpdate = Actors.Num();
	}
	// Only the game can use incremental update functionality.
	else
	{
		checkMsg(!GIsEditor && GIsGame,TEXT("Cannot call IncrementalUpdateComponents with non 0 argument in the Editor/ commandlets."));
	}

	// Do BSP on the first pass.
	if( CurrentActorIndexForUpdateComponents == 0 )
	{
		// Create/update the level's BSP model components.
		if(!ModelComponents.Num())
		{
			// Update the model vertices and edges.
			Model->UpdateVertices();
			Model->BuildShadowData();
			Model->InvalidSurfaces = 0;
    
		    // Clear the model index buffers.
		    Model->MaterialIndexBuffers.Empty();

			//<@ ava specific ; 2006. 9. 27 changmin
			Model->ComponentIndexBuffers.Empty();
			//>@ ava

			TMap< FModelComponentKey, TArray<WORD> > ModelComponentMap;

			// Sort the nodes by zone, grid cell and masked poly flags.
			for(INT NodeIndex = 0;NodeIndex < Model->Nodes.Num();NodeIndex++)
			{
				FBspNode& Node = Model->Nodes(NodeIndex);
				FBspSurf& Surf = Model->Surfs(Node.iSurf);

				if(Node.NumVertices > 0)
				{
					for(INT BackFace = 0;BackFace < ((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
					{
						// Calculate the bounding box of this node.
						FBox NodeBounds(0);
						for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
						{
							NodeBounds += Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
						}

						// Create a sort key for this node using the grid cell containing the center of the node's bounding box.

#define MODEL_GRID_SIZE_XY	(2048.0f)
#define MODEL_GRID_SIZE_Z	(4096.0f)
						FModelComponentKey Key;
						//<@ ava specific ; 2007. 1. 29 changmin
						if( GWorld->PersistentLevel
						//&& GWorld->PersistentLevel->StructuralModel->LeafBytes == 0
						&& GWorld->PersistentLevel->StructuralModel->LeafInfos.Num() == 0 )	// 2008. 2. 28 changmin
						{
							// 이 case가 epic's original version입니다.
							Key.ZoneIndex		= Model->NumZones ? Node.iZone[1 - BackFace] : INDEX_NONE;
							Key.X				= appFloor(NodeBounds.GetCenter().X / MODEL_GRID_SIZE_XY);
							Key.Y				= appFloor(NodeBounds.GetCenter().Y / MODEL_GRID_SIZE_XY);
							Key.Z				= appFloor(NodeBounds.GetCenter().Z / MODEL_GRID_SIZE_Z);
							Key.MaskedPolyFlags	= Surf.PolyFlags & PF_ModelComponentMask;
						}
						else
						{
							Key.ZoneIndex		= Model->NumZones ? Node.iZone[1 - BackFace] : INDEX_NONE;
							Key.X				= 0;
							Key.Y				= 0;
							Key.Z				= 0;
							Key.ClusterNumber	= Node.iCluster;
							Key.MaskedPolyFlags	= Surf.PolyFlags & PF_ModelComponentMask;
						}
						//>@ ava

						// Find an existing node list for the grid cell.
						TArray<WORD>* ComponentNodes = ModelComponentMap.Find(Key);
						if(!ComponentNodes)
						{
							// This is the first node we found in this grid cell, create a new node list for the grid cell.
							ComponentNodes = &ModelComponentMap.Set(Key,TArray<WORD>());
						}

						// Add the node to the grid cell's node list.
						ComponentNodes->AddUniqueItem(NodeIndex);
					}
				}
			}

			// Create a UModelComponent for each grid cell's node list.
			for(TMap< FModelComponentKey, TArray<WORD> >::TConstIterator It(ModelComponentMap);It;++It)
			{
				const FModelComponentKey& Key = It.Key();
				for(INT NodeIndex = 0;NodeIndex < It.Value().Num();NodeIndex++)
				{
					Model->Nodes(It.Value()(NodeIndex)).ComponentIndex = ModelComponents.Num();
					Model->Nodes(It.Value()(NodeIndex)).ComponentNodeIndex = NodeIndex;
				}
				//<@ ava specific ; 2006. 11. 17 changmin
				//ModelComponents.AddItem(new(this) UModelComponent(Model,Key.ZoneIndex,ModelComponents.Num(),Key.MaskedPolyFlags,It.Value()));
				ModelComponents.AddItem(new(this) UModelComponent(Model,Key.ZoneIndex,ModelComponents.Num(),Key.MaskedPolyFlags,It.Value(), Key.ClusterNumber));
				//>@ ava
			}
		}
		else
		{
			for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
			{
				if(ModelComponents(ComponentIndex))
				{
					ModelComponents(ComponentIndex)->ConditionalDetach();
				}
			}
		}

		// Update model components.
		for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
		{
			if(ModelComponents(ComponentIndex))
			{
				ModelComponents(ComponentIndex)->ConditionalAttach(GWorld->Scene,NULL,FMatrix::Identity);
			}
		}

		// Initialize the model's index buffers.
		for(TDynamicMap<UMaterialInstance*,FRawIndexBuffer32>::TIterator IndexBufferIt(Model->MaterialIndexBuffers);IndexBufferIt;++IndexBufferIt)
		{
			BeginInitResource(&IndexBufferIt.Value());
		}

		//<@ ava specific ; 2006. 9. 27 changmin
		for(TDynamicMap<WORD, FRawIndexBuffer32>::TIterator IndexBufferIt(Model->ComponentIndexBuffers); IndexBufferIt; ++IndexBufferIt )
		{
			BeginInitResource(&IndexBufferIt.Value());
		}
		//>@ ava
	}

	// Update the actor components.
	NumComponentsToUpdate = Min( NumComponentsToUpdate, Actors.Num() - CurrentActorIndexForUpdateComponents );
	for( INT i=0; i<NumComponentsToUpdate; i++ )
	{
		AActor* Actor = Actors(CurrentActorIndexForUpdateComponents++);
		if( Actor )
		{
			Actor->ClearComponents();
			Actor->ConditionalUpdateComponents();
		}
	}

	// See whether we are done.
	if( CurrentActorIndexForUpdateComponents == Actors.Num() )
	{
		CurrentActorIndexForUpdateComponents	= 0;
		bAreComponentsCurrentlyAttached			= TRUE;
	}
	// Only the game can use incremental update functionality.
	else
	{
		check(!GIsEditor && GIsGame);
	}
}

void ULevel::PreEditUndo()
{
	Super::PreEditUndo();

	// Detach existing model components.  These are left in the array, so they are saved for undoing the undo.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->ConditionalDetach();
		}
	}

	// Wait for the components to be detached.
	FlushRenderingCommands();
}

/**
 * Invalidates the cached data used to render the level's UModel.
 */
void ULevel::InvalidateModelGeometry()
{
	// Save the level/model state for transactions.
	Model->Modify();
	Modify();

	// Begin releasing the model's resources.
	Model->BeginReleaseResources();

	// Remove existing model components.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->Modify();
			ModelComponents(ComponentIndex)->ConditionalDetach();
		}
	}
	ModelComponents.Empty();
}

/**
 * Discards the cached data used to render the level's UModel.  Assumes that the
 * faces and vertex positions haven't changed, only the applied materials.
 */
void ULevel::InvalidateModelSurface()
{
	Model->InvalidSurfaces = TRUE;
}

void ULevel::CommitModelSurfaces()
{
	if(Model->InvalidSurfaces)
	{
		// Begin releasing the model's resources.
		Model->BeginReleaseResources();

		// Wait for the model's resources to be released.
		FlushRenderingCommands();

		// Clear the model index buffers.
		Model->MaterialIndexBuffers.Empty();

		//<@ ava specific ; 2006. 9. 27 changmin.
		Model->ComponentIndexBuffers.Empty();
		//>@ ava

		// Update the model vertices.
		Model->UpdateVertices();

		// Update the model components.
		for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
		{
			if(ModelComponents(ComponentIndex))
			{
				ModelComponents(ComponentIndex)->CommitSurfaces();
			}
		}		

		Model->InvalidSurfaces = 0;
		
		// Initialize the model's index buffers.
		for(TDynamicMap<UMaterialInstance*,FRawIndexBuffer32>::TIterator IndexBufferIt(Model->MaterialIndexBuffers);IndexBufferIt;++IndexBufferIt)
		{
			BeginInitResource(&IndexBufferIt.Value());
		}

		//<@ ava specific ; 2006. 9. 27 changmin
		for(TDynamicMap<WORD, FRawIndexBuffer32>::TIterator IndexBufferIt(Model->ComponentIndexBuffers); IndexBufferIt;++IndexBufferIt)
		{
			BeginInitResource(&IndexBufferIt.Value());
		}
		//>@ ava
	}
}

/**
 * Static helper function to add a material's textures to a level's StaticStreamableTextureInfos array.
 * 
 * @param Level						Level whose StaticStreamableTextureInfos array is being used
 * @param MaterialInstance			Material instance to query for streamable textures
 * @param BoundingSphere			BoundingSphere to add
 * @param TexelFactor				TexelFactor to add
 * @param bOnlyStreamWorldTextures	Only consider textures in the world group if TRUE
 */
static void AddStreamingMaterial( ULevel* Level, UMaterialInstance* MaterialInstance, const FSphere& BoundingSphere, FLOAT TexelFactor, UBOOL bOnlyStreamWorldTextures, UTexture2D* TargetTexture )
{
	// Nothing to do if we don't have a material instance/ material.
	if( MaterialInstance == NULL )
	{
		return;
	}

	// Get the textures which are used by the material instance.
	TArray<UTexture*> Textures;
	MaterialInstance->GetTexturesNeededForPlatform(Textures);

	// Create a streamable texture instance for each texture used by the material instance.
	for(INT TextureIndex = 0;TextureIndex < Textures.Num();TextureIndex++)
	{
		// ... now see whether it's a non- NULL 2D texture.
		UTexture2D* Texture2D = Cast<UTexture2D>(Textures(TextureIndex));
		if ( Texture2D )
		{
			if (TargetTexture && (TargetTexture != Texture2D))
			{
				continue;
			}
			UBOOL bIsLightOrShadowMap	= Texture2D->IsA(UShadowMapTexture2D::StaticClass()) || Texture2D->IsA(ULightMapTexture2D::StaticClass());
			UBOOL bShouldStreamTexture	=	!bIsLightOrShadowMap 
										&&  (!bOnlyStreamWorldTextures 
											|| (Texture2D->LODGroup == TEXTUREGROUP_World)
											|| (Texture2D->LODGroup == TEXTUREGROUP_WorldNormalMap));

			if( bShouldStreamTexture )
			{
				// Texture instance information.
				FStreamableTextureInstance TextureInstance;
				TextureInstance.BoundingSphere	= BoundingSphere;
				TextureInstance.TexelFactor		= TexelFactor;

				// See whether there already is an instance in the level.
				TArray<FStreamableTextureInstance>* TextureInstances = Level->TextureToInstancesMapPerPlatform[GRHIShaderPlatform].Find( Texture2D );

				// We have existing instances.
				if( TextureInstances )
				{
					// Add to the array.
					TextureInstances->AddItem( TextureInstance );
				}
				// This is the first instance.
				else
				{
					// Create array with current instance as the only entry.
					TArray<FStreamableTextureInstance> NewTextureInstances;
					NewTextureInstances.AddItem( TextureInstance );
					// And set it .
					Level->TextureToInstancesMapPerPlatform[GRHIShaderPlatform].Set( Texture2D, NewTextureInstances );
				}
			}
		}
	}
}

/**
 * Rebuilds static streaming data.	
 */
void ULevel::BuildStreamingData(UTexture2D* TargetTexture)
{
#if !CONSOLE
	DOUBLE StartTime = appSeconds();	

	FlushRenderingCommands();

	EShaderPlatform OldPlatform = GRHIShaderPlatform;	

	for (INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; ++PlatformIndex)
	{
		// Start from scratch.
		TextureToInstancesMapPerPlatform[PlatformIndex].Empty();
		ForceStreamTexturesPerPlatform[PlatformIndex].Empty();

		// Game이 아니면 타 플랫폼에 대한 처리 skip :)
		if (!GIsEditor && GIsGame)
		{
			if (PlatformIndex != GRHIShaderPlatform)
				continue;			
		}
		else
		{
			GRHIShaderPlatform = (EShaderPlatform)PlatformIndex;	
		}

		GShaderCompilePlatform = GRHIShaderPlatform;
	
		// Static meshes...
		for( TObjectIterator<UStaticMeshComponent> It; It; ++It )
		{
			UStaticMeshComponent*	StaticMeshComponent	= *It;
			AActor*					Owner				= StaticMeshComponent->GetOwner();
			if( Owner )
			{
				UBOOL bIsStatic					= Owner->bStatic;
				UBOOL bIsLevelPlacedKActor		= Owner->bNoDelete && Owner->IsA(AKActor::StaticClass());
				UBOOL bIsInLevel				= StaticMeshComponent->IsIn( this );
				UBOOL bUseAllComponents			= Owner->bConsiderAllStaticMeshComponentsForStreaming;
				UBOOL bOnlyStreamWorldTextures	= !bIsStatic && !bIsLevelPlacedKActor && !bUseAllComponents;
				if( bIsInLevel && StaticMeshComponent->StaticMesh )
				{
					if(!StaticMeshComponent->bIgnoreInstanceForTextureStreaming)
					{
						StaticMeshComponent->UpdateBounds();

						//TODO: Handle multiple LODs here
						for( INT ElementIndex = 0; ElementIndex < StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
						{
							UMaterialInstance*	MaterialInstance	= StaticMeshComponent->GetMaterial(ElementIndex);
							FLOAT				TexelFactor			= StaticMeshComponent->StaticMesh->GetStreamingTextureFactor(0) 
								* StaticMeshComponent->GetOwner()->DrawScale 
								* StaticMeshComponent->GetOwner()->DrawScale3D.GetAbsMax()
								* StaticMeshComponent->Scale
								* StaticMeshComponent->Scale3D.GetAbsMax();
							FSphere				BoundingSphere		= FSphere( StaticMeshComponent->Bounds.Origin, StaticMeshComponent->Bounds.SphereRadius );

							AddStreamingMaterial( this, MaterialInstance, BoundingSphere, TexelFactor, bOnlyStreamWorldTextures, TargetTexture );
						}
					}

					// Look for static mesh components with the bForceMipStreaming flag set.
					if(StaticMeshComponent->bForceMipStreaming)
					{
						for( INT ElementIndex = 0; ElementIndex < StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
						{
							UMaterialInstance*	MaterialInstance = StaticMeshComponent->GetMaterial(ElementIndex);
							if(MaterialInstance)
							{
								TArray<UTexture*> Textures;
								MaterialInstance->GetTexturesNeededForPlatform(Textures);
								for(INT TexIndex = 0; TexIndex < Textures.Num(); TexIndex++ )
								{
									UTexture2D* Tex = Cast<UTexture2D>( Textures(TexIndex) );
									if(Tex)
									{
										ForceStreamTexturesPerPlatform[GRHIShaderPlatform].Set( Tex, TRUE );
									}
								}
							}
						}
					}
				}
			}
		}


		// Skeletal meshes...
		for( TObjectIterator<USkeletalMeshComponent> It; It; ++It )
		{
			USkeletalMeshComponent*	SkeletalMeshComponent	= *It;
			AActor*					Owner					= SkeletalMeshComponent->GetOwner();
			if( Owner )
			{
				UBOOL bIsInLevel = SkeletalMeshComponent->IsIn( this );
				if( bIsInLevel && SkeletalMeshComponent->SkeletalMesh )
				{
					// Look for skeletal mesh components with the bForceMipStreaming flag set.
					if(SkeletalMeshComponent->bForceMipStreaming)
					{
						INT NumMaterials = Max(SkeletalMeshComponent->SkeletalMesh->Materials.Num(), SkeletalMeshComponent->Materials.Num());
						for( INT MatIndex = 0; MatIndex < NumMaterials; MatIndex++ )
						{
							UMaterialInstance*	MaterialInstance = SkeletalMeshComponent->GetMaterial(MatIndex);
							if(MaterialInstance)
							{
								TArray<UTexture*> Textures;
								MaterialInstance->GetTexturesNeededForPlatform(Textures);
								for(INT TexIndex = 0; TexIndex < Textures.Num(); TexIndex++ )
								{
									UTexture2D* Tex = Cast<UTexture2D>( Textures(TexIndex) );
									if(Tex)
									{
										ForceStreamTexturesPerPlatform[GRHIShaderPlatform].Set( Tex, TRUE );
									}
								}
							}
						}
					}
				}
			}
		}

		// BSP...
		for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
		{
			ABrush* BrushActor	= Cast<ABrush>(Actors(ActorIndex));
			UModel* Brush		= BrushActor && !BrushActor->bDeleteMe ? BrushActor->Brush : NULL;

			if( Brush && Brush->Polys )
			{
				TArray<FVector> Points;
				for( INT PolyIndex=0; PolyIndex<Brush->Polys->Element.Num(); PolyIndex++ )
				{			
					for( INT VertexIndex=0; VertexIndex<Brush->Polys->Element(PolyIndex).Vertices.Num(); VertexIndex++ )
					{
						Points.AddItem(Brush->Polys->Element(PolyIndex).Vertices(VertexIndex));
					}
				}

				//@todo streaming: should be optimized to use per surface specific texel factor.

				FBoxSphereBounds	Bounds = FBoxSphereBounds( &Points(0), Points.Num() ).TransformBy(BrushActor->LocalToWorld());
				FSphere				BoundingSphere( Bounds.Origin, Bounds.SphereRadius );
				FLOAT				TexelFactor	= Brush->GetStreamingTextureFactor();
				TArray<UMaterialInstance*>	Materials;

				for( INT PolyIndex=0; PolyIndex<Brush->Polys->Element.Num(); PolyIndex++ )
				{
					FPoly&		Poly	 = Brush->Polys->Element(PolyIndex);
					if( Poly.Material ) 
					{
						Materials.AddUniqueItem( Poly.Material );
					}
				}

				for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
				{
					AddStreamingMaterial( this, Materials(MaterialIndex), BoundingSphere, TexelFactor, FALSE, TargetTexture );
				}
			}
		}

		// Terrain...
		for( TObjectIterator<UTerrainComponent> It; It; ++It )
		{
			UTerrainComponent*	TerrainComponent	= *It;
			UBOOL				IsInLevel			= TerrainComponent->IsIn( this );

			if( IsInLevel )
			{
				TerrainComponent->UpdateBounds();

				FSphere				BoundingSphere		= FSphere( TerrainComponent->Bounds.Origin, TerrainComponent->Bounds.SphereRadius );
				ATerrain*			Terrain				= TerrainComponent->GetTerrain();

				for( INT MaterialIndex=0; MaterialIndex<Terrain->WeightedMaterials.Num(); MaterialIndex++ )
				{
					UMaterialInstance*	Material	= NULL;
					FLOAT		TexelFactor	= 0;

					for( INT BatchIndex=0; BatchIndex<TerrainComponent->BatchMaterials.Num(); BatchIndex++ )
					{
						if( TerrainComponent->BatchMaterials(BatchIndex).Get(MaterialIndex) )
						{
							UTerrainMaterial* TerrainMaterial = Terrain->WeightedMaterials(MaterialIndex).Material;
							if( TerrainMaterial && TerrainMaterial->Material )
							{
								Material	= TerrainMaterial->Material;
								TexelFactor = TerrainMaterial->MappingScale * Terrain->DrawScale * Terrain->DrawScale3D.GetAbsMax();
								break;
							}
						}
					}

					if( Material )
					{
						AddStreamingMaterial( this, Material, BoundingSphere, TexelFactor, FALSE, TargetTexture );
					}
				}
			}
		}
	}

	if (GIsEditor || !GIsGame)
	{
		GRHIShaderPlatform = OldPlatform;
	}	

	GShaderCompilePlatform = GRHIShaderPlatform;

	debugf(TEXT("ULevel::BuildStreamingData took %.3f seconds."), appSeconds() - StartTime);
#else
	appErrorf(TEXT("ULevel::BuildStreamingData should not be called on a console"));
#endif
}

/**
 *	Retrieves the array of streamable texture isntances.
 *
 */
TArray<FStreamableTextureInstance>* ULevel::GetStreamableTextureInstances(UTexture2D*& TargetTexture)
{
	typedef TArray<FStreamableTextureInstance>	STIA_Type;
	for (TMap<UTexture2D*,STIA_Type>::TIterator It(TextureToInstancesMapPerPlatform[GRHIShaderPlatform]); It; ++It)
	{
		TArray<FStreamableTextureInstance>& TSIA = It.Value();
		TargetTexture = It.Key();
		return &TSIA;
	}		

	return NULL;
}

/**
 * Returns the default brush for this level.
 *
 * @return		The default brush for this level.
 */
ABrush* ULevel::GetBrush() const
{
	checkMsg( Actors.Num() >= 2, *GetName() );
	ABrush* DefaultBrush = Cast<ABrush>( Actors(1) );
	checkMsg( DefaultBrush != NULL, *GetName() );
	checkMsg( DefaultBrush->BrushComponent, *GetName() );
	checkMsg( DefaultBrush->Brush != NULL, *GetName() );
	return DefaultBrush;
}

/**
 * Returns the world info for this level.
 *
 * @return		The AWorldInfo for this level.
 */
AWorldInfo* ULevel::GetWorldInfo() const
{
	check( Actors.Num() >= 2 );
	AWorldInfo* WorldInfo = Cast<AWorldInfo>( Actors(0) );
	check( WorldInfo != NULL );
	return WorldInfo;
}

/**
 * Returns the sequence located at the index specified.
 *
 * @return	a pointer to the USequence object located at the specified element of the GameSequences array.  Returns
 *			NULL if the index is not a valid index for the GameSequences array.
 */
USequence* ULevel::GetGameSequence() const
{
	USequence* Result = NULL;

	if( GameSequences.Num() )
	{
		Result = GameSequences(0);
	}

	return Result;
}

/**
 * Initializes all actors after loading completed.
 *
 * @param bForDynamicActorsOnly If TRUE, this function will only act on non static actors
 */
void ULevel::InitializeActors(UBOOL bForDynamicActorsOnly)
{
	UBOOL			bIsServer				= GWorld->IsServer();
	APhysicsVolume*	DefaultPhysicsVolume	= GWorld->GetDefaultPhysicsVolume();

	// Kill non relevant client actors, initialize render time, set initial physic volume, initialize script execution and rigid body physics.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			// Kill off actors that aren't interesting to the client.
			if( !bIsServer && !Actor->bScriptInitialized )
			{
				if (Actor->bStatic || Actor->bNoDelete)
				{
					if (!Actor->bExchangedRoles)
					{
						Exchange( Actor->Role, Actor->RemoteRole );
						Actor->bExchangedRoles = TRUE;
					}
				}
				else
				{
					GWorld->DestroyActor( Actor );
				}
			}

			if( !Actor->IsPendingKill() )
			{
				Actor->LastRenderTime	= 0.f;
				Actor->PhysicsVolume	= DefaultPhysicsVolume;
				Actor->Touching.Empty();
				// don't reinitialize actors that have already been initialized (happens for actors that persist through a seamless level change)
				if (!Actor->bScriptInitialized || Actor->GetStateFrame() == NULL)
				{
					Actor->InitExecution();
				}
			}
		}
	}
}

/**
 * Routes pre and post begin play to actors and also sets volumes.
 *
 * @param bForDynamicActorsOnly If TRUE, this function will only act on non static actors
 *
 * @todo seamless worlds: this doesn't correctly handle volumes in the multi- level case
 */
void ULevel::RouteBeginPlay(UBOOL bForDynamicActorsOnly)
{
	// this needs to only be done once, so when we do this again for reseting
	// dynamic actors, we can't do it again
	if (!bForDynamicActorsOnly)
	{
		GWorld->AddLevelNavList( this, TRUE );
	}

	// Send PreBeginPlay, set zones and collect volumes.
	TArray<AVolume*> LevelVolumes;		
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			if( !Actor->bScriptInitialized && (!Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic) )
			{
				Actor->PreBeginPlay();
			}
			AVolume* Volume = Actor->GetAVolume();
			if( Volume )
			{
				LevelVolumes.AddItem(Volume);
			}
		}
	}

	// Send set volumes, beginplay on components, and postbeginplay.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			if( !Actor->bScriptInitialized )
			{
				Actor->SetVolumes( LevelVolumes );
			}

			if( !Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic )
			{
#ifdef DEBUG_CHECKCOLLISIONCOMPONENTS
				INT NumCollisionComponents = 0;
#endif
				// Call BeginPlay on Components.
				for(INT ComponentIndex = 0;ComponentIndex < Actor->Components.Num();ComponentIndex++)
				{
					UActorComponent* ActorComponent = Actor->Components(ComponentIndex);
					if( ActorComponent && ActorComponent->IsAttached() )
					{
						ActorComponent->ConditionalBeginPlay();
#ifdef DEBUG_CHECKCOLLISIONCOMPONENTS
						UPrimitiveComponent *C = Cast<UPrimitiveComponent>(ActorComponent);
						if ( C && C->ShouldCollide() )
						{
							NumCollisionComponents++;
							if( NumCollisionComponents > 1 )
							{
								debugf(TEXT("additional collision component %s owned by %s"), *C->GetName(), *GetName());
							}
						}
#endif
					}
				}
			}
			if( !Actor->bScriptInitialized )
			{
				if( !Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic )
				{
					Actor->PostBeginPlay();
				}
				// Set script initialized if we skip routing begin play as some code relies on it.
				else
				{
					Actor->bScriptInitialized = TRUE;
				}
			}
		}
	}
}

// {{ 20070103 dEAthcURe|HM
#ifdef EnableHostMigration
void ULevel::HmRouteBeginPlay(UBOOL bForDynamicActorsOnly)
{
	// this needs to only be done once, so when we do this again for reseting
	// dynamic actors, we can't do it again
	/* // disable 
	if (!bForDynamicActorsOnly)
	{
		GWorld->AddLevelNavList( this, TRUE );
	}
	*/

	// Send PreBeginPlay, set zones and collect volumes.
	TArray<AVolume*> LevelVolumes;		
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);

		// {{ 20070111 dEAthcURe
		/*
		if(Actor && Actor->bDisabledForHostMigration) {
			continue;
		}
		*/
		// }} 20070111 dEAthcURe

		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			//debugf(TEXT("[dEAthcURe] ULevel::HmRouteBeginPlay before PreBeginPlay actor %s bScriptInitialized=%d bStatic=%d bRouteBeginPlayEvenIfStatic=%d"), *Actor->GetName(), Actor->bScriptInitialized, Actor->bStatic, Actor->bRouteBeginPlayEvenIfStatic);
			if( !Actor->bScriptInitialized && (!Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic) )
			{
				Actor->PreBeginPlay();
			}
			AVolume* Volume = Actor->GetAVolume();
			if( Volume )
			{
				LevelVolumes.AddItem(Volume);
			}
		}
	}

	// Send set volumes, beginplay on components, and postbeginplay.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);

		// {{ 20070111 dEAthcURe
		/*
		if(Actor && Actor->bDisabledForHostMigration) {
			continue;
		}
		*/
		// }} 20070111 dEAthcURe

		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			if( !Actor->bScriptInitialized )
			{
				Actor->SetVolumes( LevelVolumes );
			}

			if( !Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic )
			{
#ifdef DEBUG_CHECKCOLLISIONCOMPONENTS
				INT NumCollisionComponents = 0;
#endif
				// Call BeginPlay on Components.
				for(INT ComponentIndex = 0;ComponentIndex < Actor->Components.Num();ComponentIndex++)
				{
					UActorComponent* ActorComponent = Actor->Components(ComponentIndex);
					if( ActorComponent && ActorComponent->IsAttached() )
					{
						ActorComponent->ConditionalBeginPlay();
#ifdef DEBUG_CHECKCOLLISIONCOMPONENTS
						UPrimitiveComponent *C = Cast<UPrimitiveComponent>(ActorComponent);
						if ( C && C->ShouldCollide() )
						{
							NumCollisionComponents++;
							if( NumCollisionComponents > 1 )
							{
								debugf(TEXT("additional collision component %s owned by %s"), *C->GetName(), *GetName());
							}
						}
#endif
					}
				}
			}
			//debugf(TEXT("[dEAthcURe] ULevel::HmRouteBeginPlay before PostBeginPlay actor %s bScriptInitialized=%d bStatic=%d bRouteBeginPlayEvenIfStatic=%d"), *Actor->GetName(), Actor->bScriptInitialized, Actor->bStatic, Actor->bRouteBeginPlayEvenIfStatic);
			if( !Actor->bScriptInitialized )
			{
				if( !Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic )
				{
					Actor->PostBeginPlay();
				}
				// Set script initialized if we skip routing begin play as some code relies on it.
				else
				{
					Actor->bScriptInitialized = TRUE;
				}
			}
		}
	}
}
#endif
// }} 20070103 dEAthcURe|HM

UBOOL ULevel::HasAnyActorsOfType(UClass *SearchType)
{
	// just search the actors array
	for (INT Idx = 0; Idx < Actors.Num(); Idx++)
	{
		AActor *Actor = Actors(Idx);
		// if valid, not pending kill, and
		// of the correct type
		if (Actor != NULL &&
			!Actor->IsPendingKill() &&
			Actor->IsA(SearchType))
		{
			return TRUE;
		}
	}
	return FALSE;
}

UBOOL ULevel::HasPathNodes()
{
	// if this is the editor
	if (GIsEditor)
	{
		// check the actor list, as paths may not be rebuilt
		return HasAnyActorsOfType(ANavigationPoint::StaticClass());
	}
	else
	{
		// otherwise check the nav list pointers
		return (NavListStart != NULL && NavListEnd != NULL);
	}
}

//test
//debug
//pathdebug
#if 0 && !PS3 && !FINAL_RELEASE
#define CHECKNAVLIST(b, x, n) \
		if( !GIsEditor && ##b ) \
		{ \
			debugf(*##x); \
			for (ANavigationPoint *T = GWorld->GetFirstNavigationPoint(); T != NULL; T = T->nextNavigationPoint) \
			{ \
				T->ClearForPathFinding(); \
			} \
			UWorld::VerifyNavList(*##x, ##n); \
		}
#else
#define CHECKNAVLIST(b, x, n)
#endif

void ULevel::AddToNavList( ANavigationPoint *Nav, UBOOL bDebugNavList )
{
	if (Nav != NULL)
	{
		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT("ADD %s to nav list %s"), *Nav->GetFullName(), *GetFullName()), Nav );

		UBOOL bNewList = FALSE;

		// if the list is currently invalid,
		if (NavListStart == NULL)
		{
			// set the new nav as the start/end of the list
			NavListStart = Nav;
			NavListEnd = Nav;
			Nav->nextNavigationPoint = NULL;
			bNewList = TRUE;
		}
		else
		{
			// otherwise insert the nav at the start
			Nav->nextNavigationPoint = NavListStart;
			NavListStart = Nav;
		}
		// add to the cover list as well
		ACoverLink *Link = Cast<ACoverLink>(Nav);
		if (Link != NULL)
		{
			if (CoverListStart == NULL)
			{
				CoverListStart = Link;
				CoverListEnd = Link;
				Link->NextCoverLink = NULL;
			}
			else
			{
				Link->NextCoverLink = CoverListStart;
				CoverListStart = Link;
			}
		}

		if (bNewList && GIsGame)
		{
			GWorld->AddLevelNavList(this,bDebugNavList);
			debugf(TEXT(">>>  ADDED %s to world nav list because of %s"), *GetFullName(), *Nav->GetFullName());
		}

		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT(">>> ADDED %s to nav list"), *Nav->GetFullName()), Nav );
	}
}

void ULevel::RemoveFromNavList( ANavigationPoint *Nav, UBOOL bDebugNavList )
{
	if( GIsEditor && !GIsGame )
	{
		// skip if in the editor since this shouldn't be reliably used (build paths only)
		return;
	}

	if (Nav != NULL)
	{
		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT("REMOVE %s from nav list"), *Nav->GetFullName()), Nav );

		AWorldInfo *Info = GWorld->GetWorldInfo();

		// navigation point
		{
			// this is the nav that was pointing to this nav in the linked list
			ANavigationPoint *PrevNav = NULL;

			// remove from the world list
			// first check to see if this is the head of the world nav list
			if (Info->NavigationPointList == Nav)
			{
				// adjust to the next
				Info->NavigationPointList = Nav->nextNavigationPoint;
			}
			else
			{
				// otherwise hunt through the list for it
				for (ANavigationPoint *ChkNav = Info->NavigationPointList; ChkNav != NULL; ChkNav = ChkNav->nextNavigationPoint)
				{
					if (ChkNav->nextNavigationPoint == Nav)
					{
						// remove from the list
						PrevNav = ChkNav;
						ChkNav->nextNavigationPoint = Nav->nextNavigationPoint;
						break;
					}
				}
			}

			// check to see if it was the head of the level list
			if (Nav == NavListStart)
			{
				NavListStart = Nav->nextNavigationPoint;
			}

			// check to see if it was the end of the level list
			if (Nav == NavListEnd)
			{
				// if the previous nav is in this level
				if (PrevNav != NULL &&
					PrevNav->GetLevel() == this)
				{
					// then set the end to that
					NavListEnd = PrevNav;
				}
				// otherwise null the end
				else
				{
					NavListEnd = NULL;
				}
			}
		}

		// update the cover list as well (MIRROR NavList* update!)
		ACoverLink *Link = Cast<ACoverLink>(Nav);
		if (Link != NULL)
		{
			// this is the nav that was pointing to this nav in the linked list
			ACoverLink *PrevLink = NULL;

			// remove from the world list
			// first check to see if this is the head of the world nav list
			if (Info->CoverList == Link)
			{
				// adjust to the next
				Info->CoverList = Link->NextCoverLink;
			}
			else
			{
				// otherwise hunt through the list for it
				for (ACoverLink *ChkLink = Info->CoverList; ChkLink != NULL; ChkLink = ChkLink->NextCoverLink)
				{
					if (ChkLink->NextCoverLink == Link)
					{
						// remove from the list
						PrevLink = ChkLink;
						ChkLink->NextCoverLink = Link->NextCoverLink;
						break;
					}
				}
			}

			// check to see if it was the head of the level list
			if (Link == CoverListStart)
			{
				CoverListStart = Link->NextCoverLink;
			}

			// check to see if it was the end of the level list
			if (Link == CoverListEnd)
			{
				// if the previous nav is in this level
				if (PrevLink != NULL &&
					PrevLink->GetLevel() == this)
				{
					// then set the end to that
					CoverListEnd = PrevLink;
				}
				// otherwise null the end
				else
				{
					CoverListEnd = NULL;
				}
			}
		}

		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT(">>> REMOVED %s from nav list"), *Nav->GetFullName()), Nav );
	}
}

void ULevel::ResetNavList()
{
	NavListStart = NULL;
	NavListEnd = NULL;
	CoverListStart = NULL;
	CoverListEnd = NULL;
}

// {{ 20061026 dEAthcURe|HM
#ifdef EnableHostMigration
AActor* ULevel::findActor(const FString& name)
{
	for(int idxActor=0;idxActor<Actors.Num();idxActor++) {
		AActor* pActor = Actors(idxActor);		

		if(pActor) {
			if(pActor->GetName() == name) {
				return pActor;
			}			
		}						
	}
	return 0x0;
}
AActor* ULevel::findActorByClassName(const FString& name, int& idx)
{
	for(;idx<Actors.Num();idx++) {
		AActor* pActor = Actors(idx);		

		if(pActor) {
			if(pActor->GetClass()->GetName() == name) {
				++idx; // 다음 검색을 위해 inc
				return pActor;
			}			
		}						
	}
	return 0x0;
}
AActor* ULevel::findActor(UClass* pBase)
{
	for(int idxActor=0;idxActor<Actors.Num();idxActor++) {
		AActor* pActor = Actors(idxActor);		

		if(pActor) {
			if(pActor->IsA(pBase)) {
				return pActor;
			}			
		}						
	}
	return 0x0;
}
#endif
// }} 20061026 dEAthcURe|HM

/*-----------------------------------------------------------------------------
	ULineBatchComponent implementation.
-----------------------------------------------------------------------------*/

/** Represents a LineBatchComponent to the scene manager. */
class FLineBatcherSceneProxy : public FPrimitiveSceneProxy
{
 public:
	FLineBatcherSceneProxy(const ULineBatchComponent* InComponent):
		FPrimitiveSceneProxy(InComponent), Lines(InComponent->BatchedLines), LineBatchComponent( InComponent )
	{
		ViewRelevance.bDynamicRelevance = TRUE;
		for(INT LineIndex = 0;LineIndex < Lines.Num();LineIndex++)
		{
			const ULineBatchComponent::FLine& Line = Lines(LineIndex);
			ViewRelevance.SetDPG(Line.DepthPriority,TRUE);
		}
	}

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT InDepthPriorityGroup)
	{
		for (INT i = 0; i < Lines.Num(); i++)
		{
			PDI->DrawLine(Lines(i).Start, Lines(i).End, Lines(i).Color, Lines(i).DepthPriority);
		}
	}

	UBOOL IsCacheable() const
	{
		return TRUE;
	}

	UBOOL IsStillValid( const UPrimitiveComponent* InComponent ) const
	{
		return TRUE;
	}

	void Touch( const FPrimitiveSceneProxyCacheInfo& PRI )
	{
		FPrimitiveSceneProxy::Touch( PRI );

		for(INT LineIndex = 0;LineIndex < Lines.Num();LineIndex++)
		{
			const ULineBatchComponent::FLine& Line = Lines(LineIndex);
			ViewRelevance.SetDPG(Line.DepthPriority,FALSE);
		}

		Lines.Empty();

		Lines = LineBatchComponent->BatchedLines;

		for(INT LineIndex = 0;LineIndex < Lines.Num();LineIndex++)
		{
			const ULineBatchComponent::FLine& Line = Lines(LineIndex);
			ViewRelevance.SetDPG(Line.DepthPriority,TRUE);
		}
	}

	/**
	 *  Returns a struct that describes to the renderer when to draw this proxy.
	 *	@param		Scene view to use to determine our relevence.
	 *  @return		View relevance struct
	 */
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		return ViewRelevance;
	}
	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + Lines.GetAllocatedSize() ); }

 private:
	 TArray<ULineBatchComponent::FLine> Lines;
	 FPrimitiveViewRelevance ViewRelevance;
	 const ULineBatchComponent* LineBatchComponent;
};

void ULineBatchComponent::DrawLine(const FVector& Start,const FVector& End,const FLinearColor& Color,BYTE DepthPriority)
{
	new(BatchedLines) FLine(Start,End,Color,DefaultLifeTime,DepthPriority);
	BeginDeferredReattach();
}

void ULineBatchComponent::DrawLine(const FVector& Start,const FVector& End,FColor Color,FLOAT LifeTime,BYTE DepthPriority)
{
	new(BatchedLines) FLine(Start,End,Color,LifeTime,DepthPriority);
	BeginDeferredReattach();
}

/** Provide many lines to draw - faster than calling DrawLine many times. */
void ULineBatchComponent::DrawLines(const TArray<FLine>& InLines)
{
	BatchedLines.Append(InLines);
	BeginDeferredReattach();
}

void ULineBatchComponent::Tick(FLOAT DeltaTime)
{
	UBOOL bNeedsUpdate = FALSE;
	// Update the life time of batched lines, removing the lines which have expired.
	for(INT LineIndex = 0;LineIndex < BatchedLines.Num();LineIndex++)
	{
		FLine& Line = BatchedLines(LineIndex);
		if(Line.RemainingLifeTime > 0.0f)
		{
			Line.RemainingLifeTime -= DeltaTime;
			if(Line.RemainingLifeTime <= 0.0f)
			{
				// The line has expired, remove it.
				BatchedLines.Remove(LineIndex--);
				bNeedsUpdate = TRUE;
			}
		}
	}
	if (bNeedsUpdate)
	{
		BeginDeferredReattach();
	}
}

/**
 * Creates a new scene proxy for the line batcher component.
 * @return	Pointer to the FLineBatcherSceneProxy
 */
FPrimitiveSceneProxy* ULineBatchComponent::CreateSceneProxy()
{
	return new FLineBatcherSceneProxy(this);
}

