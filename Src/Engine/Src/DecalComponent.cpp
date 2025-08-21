/*=============================================================================
	DecalComponent.cpp: Decal implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "ScenePrivate.h"
#include "UnDecalRenderData.h"
#include "UnTerrain.h"
#include "UnTerrainRender.h"

IMPLEMENT_CLASS(UDecalComponent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FDecalSceneProxy
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !FINAL_RELEASE
class FDecalSceneProxy : public FPrimitiveSceneProxy
{
public:
	FDecalSceneProxy(const UDecalComponent* Component)
		:	FPrimitiveSceneProxy( Component )
		,	Owner( Component->GetOwner() )
		,	DepthPriorityGroup( Component->DepthPriorityGroup )
		,	bSelected( Component->IsOwnerSelected() )
		,	Bounds( Component->Bounds )
		,	Width( Component->Width )
		,	Height( Component->Height )
		,	Thickness( Component->Thickness )
		,	HitLocation( Component->HitLocation )
		,	HitNormal( Component->HitNormal )
		,	HitTangent( Component->HitTangent )
		,	HitBinormal( Component->HitBinormal )
	{
		Component->GenerateDecalFrustumVerts( Verts );
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		const EShowFlags ShowFlags = View->Family->ShowFlags;

		if ( ShowFlags & SHOW_Decals )
		{
			if ( ((ShowFlags & SHOW_Bounds) && (GIsGame_RenderThread || !Owner || bSelected)) ||
				((ShowFlags & SHOW_DecalInfo) && (GIsGame_RenderThread || bSelected)) )
			{
				Result.bDynamicRelevance = TRUE;
				Result.bForegroundDPG = TRUE;
				Result.SetDPG(SDPG_Foreground,TRUE);
			}
		}

		return Result;
	}

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
	{
		if ( View->Family->ShowFlags & SHOW_Decals )
		{	
			if ( (View->Family->ShowFlags & SHOW_Bounds) && ((View->Family->ShowFlags & SHOW_Game) || !Owner || bSelected) )
			{
				//if ( DPGIndex == SDPG_Foreground )
				{
					// Draw the decal's bounding box and sphere.
					DrawWireBox(PDI,Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
					DrawCircle(PDI,Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),Bounds.SphereRadius,32,SDPG_Foreground);
					DrawCircle(PDI,Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),Bounds.SphereRadius,32,SDPG_Foreground);
					DrawCircle(PDI,Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),Bounds.SphereRadius,32,SDPG_Foreground);
				}
			}
			if ( (View->Family->ShowFlags & SHOW_DecalInfo) && ((View->Family->ShowFlags & SHOW_Game) || !Owner || bSelected)  )
			{
				//if ( DPGIndex == SDPG_World )
				{
					const FColor White(255, 255, 255);
					const FColor Red(255,0,0);
					const FColor Green(0,255,0);
					const FColor Blue(0,0,255);

					// Upper box.
					PDI->DrawLine( Verts[0], Verts[1], White, SDPG_Foreground );
					PDI->DrawLine( Verts[1], Verts[2], White, SDPG_Foreground );
					PDI->DrawLine( Verts[2], Verts[3], White, SDPG_Foreground );
					PDI->DrawLine( Verts[3], Verts[0], White, SDPG_Foreground );

					// Lower box.
					PDI->DrawLine( Verts[4], Verts[5], White, SDPG_Foreground );
					PDI->DrawLine( Verts[5], Verts[6], White, SDPG_Foreground );
					PDI->DrawLine( Verts[6], Verts[7], White, SDPG_Foreground );
					PDI->DrawLine( Verts[7], Verts[4], White, SDPG_Foreground );

					// Vertical box pieces.
					PDI->DrawLine( Verts[0], Verts[4], White, SDPG_Foreground );
					PDI->DrawLine( Verts[1], Verts[5], White, SDPG_Foreground );
					PDI->DrawLine( Verts[2], Verts[6], White, SDPG_Foreground );
					PDI->DrawLine( Verts[3], Verts[7], White, SDPG_Foreground );

					// Normal, Tangent, Binormal.
					const FLOAT HalfWidth = Width/2.f;
					const FLOAT HalfHeight = Height/2.f;
					PDI->DrawLine( HitLocation, HitLocation + (HitNormal*Thickness), Red, SDPG_Foreground );
					PDI->DrawLine( HitLocation, HitLocation + (HitTangent*HalfWidth), Green, SDPG_Foreground );
					PDI->DrawLine( HitLocation, HitLocation + (HitBinormal*HalfHeight), Blue, SDPG_Foreground );
				}
			}
		}
	}

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

private:
	AActor* Owner;

	BITFIELD DepthPriorityGroup : UCONST_SDPG_NumBits;
	BITFIELD bSelected : 1;

	FBoxSphereBounds Bounds;
	FVector Verts[8];

	FLOAT Width;
	FLOAT Height;
	FLOAT Thickness;

	FVector HitLocation;
	FVector HitNormal;
	FVector HitTangent;
	FVector HitBinormal;
};
#endif // !FINAL_RELEASE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  UDecalComponent
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
* @return	TRUE if the decal is enabled as specified in GEngine, FALSE otherwise.
*/
UBOOL UDecalComponent::IsEnabled() const
{
	return (bStaticDecal && GSystemSettings->bAllowStaticDecals) || (!bStaticDecal && GSystemSettings->bAllowDynamicDecals);
}

/**
 * Builds orthogonal planes from HitLocation, HitNormal, Width, Height and Thickness.
 */
void UDecalComponent::UpdateOrthoPlanes()
{
	enum PlaneIndex
	{
		DP_LEFT,
		DP_RIGHT,
		DP_FRONT,
		DP_BACK,
		DP_NEAR,
		DP_FAR,
		DP_NUM
	};

	const FVector Position = Location;
	const FVector Normal = -Orientation.Vector();
	const FRotationMatrix NewFrame( Orientation );
	const FLOAT OffsetRad = static_cast<FLOAT>( PI*DecalRotation/180. );
	const FLOAT CosT = appCos( OffsetRad );
	const FLOAT SinT = appSin( OffsetRad );

	const FMatrix OffsetMatrix(
		FPlane(1.0f,	0.0f,	0.0f,	0.0f),
		FPlane(0.0f,	CosT,	SinT,	0.0f),
		FPlane(0.0f,	-SinT,	CosT,	0.0f),
		FPlane(0.0f,	0.0f,	0.0f,	1.0f) );

	const FMatrix OffsetFrame( OffsetMatrix*NewFrame );

	const FVector Tangent = -OffsetFrame.GetAxis(1);
	const FVector Binormal = OffsetFrame.GetAxis(2);

	// Ensure the Planes array is correctly sized.
	if ( Planes.Num() != DP_NUM )
	{
		Planes.Empty( DP_NUM );
		Planes.Add( DP_NUM );
	}

	const FLOAT TDotP = Tangent | Position;
	const FLOAT BDotP = Binormal | Position;
	const FLOAT NDotP = Normal | Position;

	Planes(DP_LEFT)	= FPlane( -Tangent, Width/2.f - TDotP );
	Planes(DP_RIGHT)= FPlane( Tangent, Width/2.f + TDotP );

	Planes(DP_FRONT)= FPlane( -Binormal, Height/2.f - BDotP );
	Planes(DP_BACK)	= FPlane( Binormal, Height/2.f + BDotP );

	Planes(DP_NEAR)	= FPlane( Normal, -NearPlane + NDotP );
	Planes(DP_FAR)	= FPlane( -Normal, FarPlane - NDotP );

	HitLocation = Position;
	HitNormal = Normal;
	HitBinormal = Binormal;
	HitTangent = Tangent;
}

/**
 * @return		TRUE if both IsEnabled() and Super::IsValidComponent() return TRUE.
 */
UBOOL UDecalComponent::IsValidComponent() const
{
	return (GEngine ? IsEnabled() : TRUE) && Super::IsValidComponent();
}

void UDecalComponent::CheckForErrors()
{
	Super::CheckForErrors();

	// Get the decal owner's name.
	FString OwnerName(GNone);
	if ( Owner )
	{
		OwnerName = Owner->GetName();
	}

	if ( !DecalMaterial )
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, Owner, *FString::Printf(TEXT("%s::%s : Decal's material is NULL"), *GetName(), *OwnerName), MCACTION_NONE, TEXT("DecalMaterialNull"));
	}
	else
	{
		// Warn about direct or indirect references to non decal materials.
		const UMaterial* ReferencedMaterial = DecalMaterial->GetMaterial();
		if( ReferencedMaterial && !ReferencedMaterial->IsA( UDecalMaterial::StaticClass() ) )
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, Owner, *FString::Printf(TEXT("%s::%s : Decal's material is not a DecalMaterial"), *GetName(), *OwnerName), MCACTION_NONE, TEXT("DecalMaterialIsAMaterial"));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Scene attachment
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Allocates a sort key to the decal if the decal is non-static.
 */
void UDecalComponent::AllocateSortKey()
{
	if ( !bStaticDecal )
	{
		static INT SDecalSortKey = 100;
		SortOrder = ++SDecalSortKey;
	}
}

void UDecalComponent::Attach()
{
	if ( !GIsUCC )
	{
		DetachFromReceivers();

		// Dynamic decals and static decals not in game attach here.
		// Static decals in game attach in UDecalComponent::BeginPlay().
		if ( !bStaticDecal || (bStaticDecal && !GIsGame) )
		{
			// All decals outside of game will fully compute receivers.
			if ( !GIsGame || StaticReceivers.Num() == 0 )
			{
				ComputeReceivers();
			}
			else
			{
				AttachToStaticReceivers();
			}
		}
	}

	Super::Attach();
}

void UDecalComponent::UpdateTransform()
{
	// Make sure the planes and hit info are up to date for bounds calculations.
	UpdateOrthoPlanes();
	Super::UpdateTransform();
}

#if STATS || LOOKING_FOR_PERF_ISSUES
namespace {

static DOUBLE GSumMsecsSpent = 0.;
static INT GNumSamples = 0;
static INT GSampleSize = 100;

class FScopedTimingBlock
{
	const UDecalComponent* Decal;
	const TCHAR* OutputString;
	DOUBLE StartTime;

public:
	FScopedTimingBlock(UDecalComponent* InDecal, const TCHAR* InString)
		:	Decal( InDecal )
		,	OutputString( InString )
	{
		StartTime = appSeconds();
	}

	~FScopedTimingBlock()
	{
		const DOUBLE MsecsSpent = (appSeconds() - StartTime)*1000.0;
		/*
		if ( !Decal->bStaticDecal )
		{
			GSumMsecsSpent += MsecsSpent;
			++GNumSamples;
			if ( GNumSamples == GSampleSize )
			{
				const DOUBLE MsecsAvg = GSumMsecsSpent / static_cast<DOUBLE>(GNumSamples);
				warnf( NAME_DevDecals, TEXT("Decal AVG: (%.3f)"), MsecsAvg );
				GSumMsecsSpent = 0.;
				GNumSamples = 0;
			}
		}
		*/

		//if ( MsecsSpent > 1.0 )
		{
			const FString DecalMaterial = Decal->DecalMaterial->GetMaterial()->GetName();
			if ( Decal->bStaticDecal && Decal->GetOwner() )
			{
				// Log the decal actor name if the decal is static (level-placed) and has an owner.
				warnf( NAME_DevDecals, TEXT("Decal %s(%i,%s) - %s(%.3f)"),
					*Decal->GetOwner()->GetName(), Decal->DecalReceivers.Num(), Decal->bNoClip ? TEXT("TRUE"):TEXT("FALSE"), OutputString, MsecsSpent );
			}
			else
			{
				// Log the decal material.
				warnf( NAME_DevDecals, TEXT("Decal %s(%i,%s) - %s(%.3f)"),
					*DecalMaterial, Decal->DecalReceivers.Num(), Decal->bNoClip ? TEXT("TRUE"):TEXT("FALSE"), OutputString, MsecsSpent );
			}
		}
	}
};
} // namespace
#endif

#define ATTACH_RECEIVER(PrimitiveComponent) \
	if ( (PrimitiveComponent)->IsAttached() && (PrimitiveComponent)->GetScene() == GetScene() ) \
	{ \
		AttachReceiver( PrimitiveComponent ); \
	}

/**
 * Does a point check against BSP and an overlap check against actors for the
 * specified bounding volume.
 */
static void QueryForOverlapping(const FBoxSphereBounds& Bounds,
								FCheckResult*& OutBSPResult,
								FCheckResult*& OutActorResult,
								UBOOL bProjectOnBSP,
								UBOOL bProjectOnNonBSP,
								UDecalComponent* Decal)
{
	// Query BSP.
	{
		//const FScopedTimingBlock TimingBlock( Decal, TEXT("  -----BSP") );
		if ( bProjectOnBSP )
		{
			FCheckResult TestHit(1.f);
			if( GWorld->BSPPointCheck( TestHit, NULL, Bounds.Origin, Bounds.BoxExtent ) == 0 )
			{
				// Hit.
				TestHit.GetNext()	= NULL;
				OutBSPResult		= new( GMem ) FCheckResult( TestHit );
			}
		}
	}

	// Query non-BSP.
	{
		//const FScopedTimingBlock TimingBlock( Decal, TEXT("  -----Actors") );
		if ( bProjectOnNonBSP )
		{
			OutActorResult = GWorld->Hash->ActorOverlapCheck( GMem, NULL, Bounds.Origin, Bounds.SphereRadius );
		}
	}

}

/**
 * Attaches to static receivers.
 */
void UDecalComponent::AttachToStaticReceivers()
{
	if ( !IsEnabled() )
	{
		return;
	}

	// Make sure the planes and hit info are up to date for bounds calculations.
	UpdateOrthoPlanes();

	if ( DecalMaterial )
	{
		// const FScopedTimingBlock TimingBlock( this, TEXT("StaticReceivers") );
		for ( INT ReceiverIndex = 0 ; ReceiverIndex < StaticReceivers.Num() ; ++ReceiverIndex )
		{
			FStaticReceiverData* StaticReceiver = StaticReceivers(ReceiverIndex);
			UPrimitiveComponent* Receiver = StaticReceiver->Component;
			if ( Receiver )
			{
				if ( Receiver->IsAttached() && Receiver->GetScene() == GetScene() )
				{
					FDecalRenderData* NewRenderData = new FDecalRenderData( NULL, TRUE, TRUE );
					CopyStaticReceiverDataToDecalRenderData( *NewRenderData, *StaticReceiver );
					NewRenderData->InitResources();

					// Add the decal attachment to the receiver.
					//debugf( NAME_DevDecals, TEXT("AttachToStaticReceivers: binding %s to %s"), *Receiver->GetName(), *GetName() );					
					Receiver->AttachDecal( this, NewRenderData, NULL );

					// Add the receiver to this decal.
					FDecalReceiver* NewDecalReceiver = new(DecalReceivers) FDecalReceiver;
					NewDecalReceiver->Component = Receiver;
					NewDecalReceiver->RenderData = NewRenderData;
				}
			}
		}
	}
}

/**
 * Updates ortho planes, computes the receiver set, and connects to a decal manager.
 */
void UDecalComponent::ComputeReceivers()
{
	if ( !IsEnabled() )
	{
		return;
	}

	// Make sure the planes and hit info are up to date for bounds calculations.
	UpdateOrthoPlanes();

	if ( DecalMaterial )
	{
		AllocateSortKey();

		if ( HitComponent )
		{
			// const FScopedTimingBlock TimingBlock( this, TEXT("HitComponent") );
			ATTACH_RECEIVER( HitComponent );
		}
		// We assume that a NULL HitComponent during gameplay means we hit the BSP.
		//@todo DB - spawned dynamic decals won't project onto anything but BSP if HitComponent==NULL
		else if ( GIsGame && !bStaticDecal )
		{
			// const FScopedTimingBlock TimingBlock( this, TEXT("BSPOnly(NULL HitComponent)") );
			//@todo decals: all this could be optimized by setting a HitLevel when decals are created.
			UpdateBounds();

			// Do nothing if the decal does not project on BSP.
			if ( bProjectOnBSP )
			{
				FCheckResult BSPTestHit(1.f);
				if( GWorld->BSPPointCheck( BSPTestHit, NULL, Bounds.Origin, Bounds.BoxExtent ) == 0 )
				{
					for( FCheckResult* HitResult=&BSPTestHit; HitResult; HitResult=HitResult->GetNext() )
					{
						ULevel* HitLevel = HitResult->Level;
						if( HitLevel )
						{
							// Iterate over the hit level's model components.
							for( INT ModelComponentIndex = 0 ; ModelComponentIndex < HitLevel->ModelComponents.Num() ; ++ModelComponentIndex )
							{
								UModelComponent* ModelComponent = HitLevel->ModelComponents(ModelComponentIndex);
								if( ModelComponent )
								{
									if (ModelComponent->Bounds.GetBox().Intersect( Bounds.GetBox() ))
									{
										ATTACH_RECEIVER( ModelComponent );
									}									
								}
							}
						}
					}
				}
			}
		}
		else if ( ReceiverImages.Num() > 0 )
		{
			// const FScopedTimingBlock TimingBlock( this, TEXT("ReceiverImages") );
			for ( INT ReceiverIndex = 0 ; ReceiverIndex < ReceiverImages.Num() ; ++ReceiverIndex )
			{
				UPrimitiveComponent* PotentialReceiver = ReceiverImages(ReceiverIndex);
				if ( PotentialReceiver )
				{
					ATTACH_RECEIVER( PotentialReceiver );
				}
			}
		}
		else
		{
#if LOOKING_FOR_PERF_ISSUES
			if( GIsGame )
			{
				debugf( NAME_PerfWarning, TEXT( "UDecalComponent::ComputeReceivers is using slow codepath at runtime for '%s' with material '%s'" ), *GetFullName(), *DecalMaterial->GetName() );
			}
			const FScopedTimingBlock TimingBlock( this, TEXT("SlowPath") );
#endif
			// If the decal is static, get its level.
			// Invariant: Static decals should always have owners!
			ULevel* DecalLevel = NULL;
			INT DecalLevelIndex = INDEX_NONE;
			if( bStaticDecal && GetOwner() )
			{
				DecalLevel = GetOwner()->GetLevel();
				if ( DecalLevel )
				{
					verify( GWorld->Levels.FindItem( DecalLevel, DecalLevelIndex ) );
				}
			}

			// Update the decal bounds and query the collision hash for potential receivers.
			UpdateBounds();

			FMemMark Mark(GMem);

			FCheckResult* BSPResult = NULL;
			FCheckResult* ActorResult = NULL;
			const UBOOL bProjectOnNonBSP = bProjectOnStaticMeshes || bProjectOnSkeletalMeshes || bProjectOnTerrain;
			QueryForOverlapping( Bounds, BSPResult, ActorResult, bProjectOnBSP, bProjectOnNonBSP, this );

			// Attach to non-BSP.  ActorResult will be NULL if bProjectOnNonBSP is FALSE.
			for( FCheckResult* HitResult = ActorResult ; HitResult ; HitResult = HitResult->GetNext() )
			{
				if ( HitResult->Component )
				{
					// If this is a static decal, make sure the receiver is in the same level as the decal.
					UBOOL bShouldAttach = TRUE;
					if ( DecalLevel )
					{
						const AActor* ReceiverOwner = HitResult->Component->GetOwner();
						if ( ReceiverOwner )
						{
							const ULevel* ReceiverLevel = ReceiverOwner->GetLevel();
							if ( DecalLevel != ReceiverLevel )
							{
								bShouldAttach = FALSE;
							}
						}
					}

					// Attach the decal if the level matched, or if the decal was spawned in game.
					if ( bShouldAttach )
					{
						ATTACH_RECEIVER( HitResult->Component );
					}
				}
			}

			// Attach to BSP.  BSPResult will be NULL if bProjectOnBSP is FALSE.
			const UBOOL bNoHitNodeIndexWasSpecified = HitNodeIndex == INDEX_NONE;
			// Attach to BSP.  BSPResult will be NULL if bProjectOnBSP is FALSE.
			for( FCheckResult* HitResult = BSPResult ; HitResult ; HitResult = HitResult->GetNext() )
			{
				if ( HitResult->LevelIndex != INDEX_NONE )
				{
					// If the decal is static, make sure the receiving BSP is in the same level as the decal.
					if ( DecalLevelIndex != INDEX_NONE && DecalLevelIndex != HitResult->LevelIndex )
					{
						continue;
					}
					// If a hit level was specified on the decal and it doesn't match this BSP result, continue.
					if ( HitLevelIndex != INDEX_NONE && HitLevelIndex != HitResult->LevelIndex )
					{
						continue;
					}

					// If no hit node index was specified, use the node index of the check result.
					if ( bNoHitNodeIndexWasSpecified )
					{
						HitNodeIndex = HitResult->Item;
					}
					// Iterate over the hit level's model components.
					const TArray<UModelComponent*>& ModelComponents = GWorld->Levels(HitResult->LevelIndex)->ModelComponents;
					for( INT ModelComponentIndex = 0 ; ModelComponentIndex < ModelComponents.Num() ; ++ModelComponentIndex )
					{
						if( ModelComponents(ModelComponentIndex) )
						{
							ATTACH_RECEIVER( ModelComponents(ModelComponentIndex) );
						}
					}
					// If no hit node index was specified, restore.
					if ( bNoHitNodeIndexWasSpecified )
					{
						HitNodeIndex = INDEX_NONE;
					}
				}
			}

			Mark.Pop();
		}

		ConnectToManager();
	}
}

void UDecalComponent::AttachReceiver(UPrimitiveComponent* Receiver)
{
	// Invariant: Receiving component is not already in the decal's receiver list
	if ( Receiver->bAcceptsDecals && (Receiver->bAcceptsDecalsDuringGameplay || !GWorld->HasBegunPlay()) )
	{
		const UBOOL bIsReceiverHidden = (Receiver->GetOwner() && Receiver->GetOwner()->bHidden) || Receiver->HiddenGame;
		if ( !bIsReceiverHidden || bProjectOnHidden )
		{
			if ( FilterComponent( Receiver ) )
			{
				FDecalState DecalState;
				CaptureDecalState( &DecalState );

				// Generate decal geometry.
				FDecalRenderData* DecalRenderData = Receiver->GenerateDecalRenderData( &DecalState );

				// Was any geometry created?
				if ( DecalRenderData )
				{
					DecalRenderData->InitResources();

					// Add the decal attachment to the receiver.
					//debugf( NAME_DevDecals, TEXT("AttachReciver: binding %s to %s"), *Receiver->GetName(), *GetName() );
					Receiver->AttachDecal( this, DecalRenderData, &DecalState );

					// Add the receiver to this decal.
					FDecalReceiver* NewDecalReceiver = new(DecalReceivers) FDecalReceiver;
					NewDecalReceiver->Component = Receiver;
					NewDecalReceiver->RenderData = DecalRenderData;
				}
			}
		}
	}
}

/**
 * Disconnects the decal from its manager and detaches receivers.
 */
void UDecalComponent::DetachFromReceivers()
{
	DisconnectFromManager();

	for ( INT ReceiverIndex = 0 ; ReceiverIndex < DecalReceivers.Num() ; ++ReceiverIndex )
	{
		FDecalReceiver& Receiver = DecalReceivers(ReceiverIndex);
		if ( Receiver.Component )
		{
			// Detach the decal from the primitive.
			Receiver.Component->DetachDecal( this );
			Receiver.Component = NULL;
		}
	}

	// Now that the receiver has been disassociated from the decal, clear its render data.
	ReleaseResources( GIsEditor );
}

void UDecalComponent::Detach()
{
	if ( !GIsUCC )
	{
		DetachFromReceivers();
	}
	Super::Detach();
}

/**
 * Enqueues decal render data deletion. Split into separate function to work around ICE with VS.NET 2003.
 *
 * @param DecalRenderData	Decal render data to enqueue deletion for
 */
static void FORCENOINLINE EnqueueDecalRenderDataDeletion( FDecalRenderData* DecalRenderData )
{
	// We have to clear the lightmap reference on the game thread because FLightMap1D::Cleanup enqueues
	// a rendering command.  Rendering commands cannot be enqueued from the rendering thread, which would
	// happen if we let FDecalRenderData's dtor clear the lightmap reference.
	DecalRenderData->LightMap1D = NULL;

	// Enqueue deletion of render data and releasing its resources.
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER( 
		DeleteDecalRenderDataCommand, 
		FDecalRenderData*,
		DecalRenderData,
		DecalRenderData,
		{
			delete DecalRenderData;
		} );
}

void UDecalComponent::ReleaseResources(UBOOL bBlockOnRelease)
{
	// Iterate over all receivers and enqueue their deletion.
	for ( INT ReceiverIndex = 0 ; ReceiverIndex < DecalReceivers.Num() ; ++ReceiverIndex )
	{
		FDecalReceiver& Receiver = DecalReceivers(ReceiverIndex);
		if ( Receiver.RenderData )
		{
			// Ensure the component has already been disassociated from the decal before clearing its render data.
			check( Receiver.Component == NULL );
			// Enqueue deletion of decal render data.
			EnqueueDecalRenderDataDeletion( Receiver.RenderData );
			// No longer safe to access, NULL out.
			Receiver.RenderData = NULL;
		}
	}

	// Empty DecalReceivers array now that resource deletion has been enqueued.
	DecalReceivers.Empty();

	// Create a fence for deletion of component in case there is a pending init e.g.
	if ( !ReleaseResourcesFence )
	{
		ReleaseResourcesFence = new FRenderCommandFence;
	}
	ReleaseResourcesFence->BeginFence();

	// Wait for fence in case we requested to block on release.
	if( bBlockOnRelease )
	{
		ReleaseResourcesFence->Wait();
	}
}

void UDecalComponent::BeginPlay()
{
	Super::BeginPlay();

	// Static decals in game attach here.
	// Dynamic decals and static decals not in game attach in UDecalComponent::Attach().
	if ( bStaticDecal && GIsGame && !GIsUCC )
	{
		if ( StaticReceivers.Num() == 0 )
		{
			ComputeReceivers();
		}
		else
		{
			AttachToStaticReceivers();
		}
	}
}

void UDecalComponent::BeginDestroy()
{
	Super::BeginDestroy();
	ReleaseResources( FALSE );
	FreeStaticReceivers();
}

/**
 * Frees any StaticReceivers data.
 */
void UDecalComponent::FreeStaticReceivers()
{
	// Free any static receiver information.
	for ( INT ReceiverIndex = 0 ; ReceiverIndex < StaticReceivers.Num() ; ++ReceiverIndex )
	{
		delete StaticReceivers(ReceiverIndex);
	}
	StaticReceivers.Empty();
}

UBOOL UDecalComponent::IsReadyForFinishDestroy()
{
	check(ReleaseResourcesFence);
	const UBOOL bDecalIsReadyForFinishDestroy = ReleaseResourcesFence->GetNumPendingFences() == 0;
	return bDecalIsReadyForFinishDestroy && Super::IsReadyForFinishDestroy();
}

void UDecalComponent::FinishDestroy()
{
	// Finish cleaning up any receiver render data.
	for ( INT ReceiverIndex = 0 ; ReceiverIndex < DecalReceivers.Num() ; ++ReceiverIndex )
	{
		FDecalReceiver& Receiver = DecalReceivers(ReceiverIndex);
		delete Receiver.RenderData;
	}
	DecalReceivers.Empty();

	// Delete any existing resource fence.
	delete ReleaseResourcesFence;
	ReleaseResourcesFence = NULL;

	Super::FinishDestroy();
}

void UDecalComponent::PreSave()
{
	Super::PreSave();

	// Mitigate receiver attachment cost for static decals by storing off receiver render data.
	// Don't save static receivers when cooking because intersection queries with potential receivers
	// may not function properly.  Instead, we require static receivers to have been computed when the
	// level was saved in the editor.
	if ( bStaticDecal && !GIsUCC )
	{
		FreeStaticReceivers();
		for ( INT ReceiverIndex = 0 ; ReceiverIndex < DecalReceivers.Num(); ++ReceiverIndex )
		{
			FDecalReceiver& DecalReceiver = DecalReceivers(ReceiverIndex);
			if ( DecalReceiver.Component && DecalReceiver.RenderData )
			{
				FStaticReceiverData* NewStaticReceiver	= new FStaticReceiverData;
				NewStaticReceiver->Component			= DecalReceiver.Component;
				CopyDecalRenderDataToStaticReceiverData( *NewStaticReceiver, *DecalReceiver.RenderData );

				StaticReceivers.AddItem( NewStaticReceiver );
			}
		}
		StaticReceivers.Shrink();
	}
}

/**
 * @return		TRUE if the application filter passes the specified component, FALSE otherwise.
 */
UBOOL UDecalComponent::FilterComponent(UPrimitiveComponent* Component) const
{
	UBOOL bResult = TRUE;

	const AActor* Owner = Component->GetOwner();
	if ( !Owner )
	{
		// Actors with no owners fail if the filter is an affect filter.
		if ( FilterMode == FM_Affect )
		{
			bResult = FALSE;
		}
	}
	else
	{
		// The actor has an owner; pass it through the filter.
		if ( FilterMode == FM_Ignore )
		{
			// Reject if the component is in the filter.
			bResult = !Filter.ContainsItem( const_cast<AActor*>(Owner) );
		}
		else if ( FilterMode == FM_Affect )
		{
			// Accept if the component is in the filter.
			bResult = Filter.ContainsItem( const_cast<AActor*>(Owner) );
		}
	}

	return bResult;
}

/**
 * Creates a proxy to represent the primitive to the scene manager in the rendering thread.
 *
 * @return The proxy object.
 */
FPrimitiveSceneProxy* UDecalComponent::CreateSceneProxy()
{
#if !FINAL_RELEASE
	return new FDecalSceneProxy( this );
#else
	return NULL;
#endif // !FINAL_RELEASE
}

/**
 * Sets the component's bounds based on the vertices of the decal frustum.
 */
void UDecalComponent::UpdateBounds()
{
	FVector Verts[8];
	GenerateDecalFrustumVerts( Verts );

	Bounds = FBoxSphereBounds( FBox( Verts, 8 ) );

	// Expand the bounds slightly to prevent false occlusion.
	static FLOAT s_fOffset	= 1.0f;
	static FLOAT s_fScale	= 1.1f;
	const FVector Value(Bounds.BoxExtent.X + s_fOffset, Bounds.BoxExtent.Y + s_fOffset, Bounds.BoxExtent.Z + s_fOffset);
	Bounds = FBoxSphereBounds(Bounds.Origin,(Bounds.BoxExtent + FVector(s_fOffset)) * s_fScale,(Bounds.SphereRadius + s_fOffset) * s_fScale);
}

/**
 * Fills in the specified vertex list with the local-space decal frustum vertices.
 */
void UDecalComponent::GenerateDecalFrustumVerts(FVector Verts[8]) const
{
	const FLOAT HalfWidth = Width/2.f;
	const FLOAT HalfHeight = Height/2.f;
	Verts[0] = Location + (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	Verts[1] = Location + (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	Verts[2] = Location - (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	Verts[3] = Location - (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	Verts[4] = Location + (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * FarPlane);
	Verts[5] = Location + (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * FarPlane);
	Verts[6] = Location - (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * FarPlane);
	Verts[7] = Location - (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * FarPlane);
}

/**
 * Fills in the specified decal state object with this decal's state.
 */
void UDecalComponent::CaptureDecalState(FDecalState* DecalState) const
{
	DecalState->DecalComponent = this;

	// Capture the decal material, or the default material if no material was specified.
	DecalState->DecalMaterial = DecalMaterial;
	if ( !DecalState->DecalMaterial )
	{
		DecalState->DecalMaterial = GEngine->DefaultMaterial;
	}

	// Determine if the decal material has unlit translucency or unlit distortion.
	DecalState->bHasUnlitTranslucency = FALSE;
	DecalState->bHasUnlitDistortion = FALSE;
	DecalState->bIsLit = FALSE;
	const UMaterial* Material = DecalState->DecalMaterial->GetMaterial();
	if ( Material )
	{
		if ( Material->LightingModel == MLM_Unlit )
		{
			DecalState->bHasUnlitTranslucency = IsTranslucentBlendMode( (EBlendMode)Material->BlendMode );
			DecalState->bHasUnlitDistortion = Material->HasDistortion();
		}
		else
		{
			DecalState->bIsLit = TRUE;
		}
	}

	DecalState->OrientationVector = Orientation.Vector();
	DecalState->HitLocation = HitLocation;
	DecalState->HitNormal = HitNormal;
	DecalState->HitTangent = HitTangent;
	DecalState->HitBinormal = HitBinormal;
	DecalState->OffsetX = OffsetX;
	DecalState->OffsetY = OffsetY;

	DecalState->Thickness = Thickness;
	DecalState->Width = Width;
	DecalState->Height = Height;

	DecalState->DepthBias = DepthBias;
	DecalState->SlopeScaleDepthBias = SlopeScaleDepthBias;
	DecalState->SortOrder = SortOrder;

	DecalState->Planes = Planes;
	DecalState->WorldTexCoordMtx = FMatrix( TileX*HitTangent/Width,
											TileY*HitBinormal/Height,
											HitNormal,
											FVector(0.f,0.f,0.f) ).Transpose();
	DecalState->DecalFrame = FMatrix( -HitNormal, HitTangent, HitBinormal, HitLocation );
	DecalState->WorldToDecal = DecalState->DecalFrame.Inverse();
	DecalState->HitBone = HitBone;
	DecalState->HitBoneIndex = INDEX_NONE;
	DecalState->HitNodeIndex = HitNodeIndex;
	DecalState->HitLevelIndex = HitLevelIndex;
	DecalState->Filter = Filter;
	DecalState->FilterMode = FilterMode;
	DecalState->DepthPriorityGroup = DepthPriorityGroup;
	DecalState->bNoClip = bNoClip;
	DecalState->bProjectOnBackfaces = bProjectOnBackfaces;
	DecalState->bProjectOnBSP = bProjectOnBSP;
	DecalState->bProjectOnStaticMeshes = bProjectOnStaticMeshes;
	DecalState->bProjectOnSkeletalMeshes = bProjectOnSkeletalMeshes;
	DecalState->bProjectOnTerrain = bProjectOnTerrain;

	// Compute frustum verts.
	const FLOAT HalfWidth = Width/2.f;
	const FLOAT HalfHeight = Height/2.f;
	DecalState->FrustumVerts[0] = HitLocation + (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	DecalState->FrustumVerts[1] = HitLocation + (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	DecalState->FrustumVerts[2] = HitLocation - (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	DecalState->FrustumVerts[3] = HitLocation - (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * NearPlane);
	DecalState->FrustumVerts[4] = HitLocation + (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * FarPlane);
	DecalState->FrustumVerts[5] = HitLocation + (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * FarPlane);
	DecalState->FrustumVerts[6] = HitLocation - (HitBinormal * HalfHeight) - (HitTangent * HalfWidth) - (HitNormal * FarPlane);
	DecalState->FrustumVerts[7] = HitLocation - (HitBinormal * HalfHeight) + (HitTangent * HalfWidth) - (HitNormal * FarPlane);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Interaction with UDecalManager
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * return	The UDecalManager object appropriate for this decal.
 */
UDecalManager* UDecalComponent::GetManager()
{
	if ( !bStaticDecal && GWorld )
	{
		return GWorld->DecalManager;
	}

	return NULL;
}

/**
 * Connects this decal to the appropriate manager.  Called by eg ComputeReceivers.
 */
void UDecalComponent::ConnectToManager()
{
	UDecalManager* Manager = GetManager();
	if ( Manager )
	{
		Manager->Connect( this );
	}
}

/**
 * Disconnects the decal from its manager.  Called by eg DetachFromReceivers.
 */
void UDecalComponent::DisconnectFromManager()
{
	UDecalManager* Manager = GetManager();
	if ( Manager )
	{
		Manager->Disconnect( this );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Serialization
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UDecalComponent::Serialize(FArchive& Ar)
{
	Super::Serialize( Ar );

	if ( Ar.Ver() >= VER_DECAL_STATIC_DECALS_SERIALIZED )
	{
		if ( Ar.IsLoading() )
		{
			/////////////////////////
			// Loading.

			// Load number of static receivers from archive.
			INT NumStaticReceivers = 0;
			Ar << NumStaticReceivers;
			// Free existing static receivers.
			FreeStaticReceivers();
			StaticReceivers.AddZeroed(NumStaticReceivers);
			for ( INT ReceiverIndex = 0 ; ReceiverIndex < NumStaticReceivers ; ++ReceiverIndex )
			{
				// Allocate new static receiver data.
				FStaticReceiverData* NewStaticReceiver = new FStaticReceiverData;
				// Fill in its members from the archive.
				Ar << *NewStaticReceiver;
				// Add it to the list of static receivers.
				StaticReceivers(ReceiverIndex) = NewStaticReceiver;
			}
		}
		else if ( Ar.IsSaving() )
		{
			/////////////////////////
			// Saving.

			// Save number of static receivers to archive.
			INT NumStaticReceivers = StaticReceivers.Num();
			Ar << NumStaticReceivers;

			// Write each receiver to the archive.
			for ( INT ReceiverIndex = 0 ; ReceiverIndex < NumStaticReceivers ; ++ReceiverIndex )
			{
				FStaticReceiverData* StaticReceiver = StaticReceivers(ReceiverIndex);
				Ar << *StaticReceiver;
			}
		}
		else if ( Ar.IsObjectReferenceCollector() )
		{
			// When collecting object references, be sure to include the components referenced via StaticReceivers.
			for ( INT ReceiverIndex = 0 ; ReceiverIndex < StaticReceivers.Num() ; ++ReceiverIndex )
			{
				FStaticReceiverData* StaticReceiver = StaticReceivers(ReceiverIndex);
				Ar << StaticReceiver->Component;
			}
		}
	}
	else
	{
		if ( Ar.Ver() >= VER_DECAL_REFACTOR )
		{
		}
		else if ( Ar.Ver() == VER_DECAL_RENDERDATA )
		{
			// Discard render data by serializing into a temp buffer.
			TArray<FDecalRenderData> TempRenderData;
			Ar << TempRenderData;
		}
		else if ( Ar.Ver() >= VER_DECAL_RENDERDATA_POINTER )
		{
			if ( Ar.IsLoading() )
			{
				INT NewNumRenderData;
				Ar << NewNumRenderData;

				// Allocate new render data objects and read their values from the archive
				for ( INT i = 0 ; i < NewNumRenderData ; ++i )
				{
					FDecalRenderData RenderData;
					Ar << RenderData;
				}
			}
		}
	}
}
