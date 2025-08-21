/*=============================================================================
	SceneRendering.cpp: Scene rendering.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "EnvCubePrivate.h"
#include "EngineParticleClasses.h"
#include "BSPDynamicBatch.h"

INT GCurrentFrame = 0;

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/
#if !FINAL_RELEASE
extern UBOOL GUpdatingCullDistances;
UBOOL GBakingEnvCubes = FALSE;
UBOOL GIsGame_RenderThread = FALSE;
FOcclusionQueryPool GOcclusionQueryPool;

template<typename DrawingPolicyType>
UBOOL TStaticMeshDrawList<DrawingPolicyType>::DrawAndCheckVisibility(FCommandContextRHI* Context, const FSceneView* View, const FBitArray& StaticMeshVisibilityMap, FBitArray& OutStaticMeshVisibilityMap) const
{
	TArray<FOcclusionQueryRHIRef> OcclusionQueries;

	GOcclusionQueryPool.Reset();

	UBOOL bDirty = FALSE;
	for(INT PolicyIndex = 0;PolicyIndex < OrderedDrawingPolicies.Num();PolicyIndex++)
	{
		const FDrawingPolicyLink* DrawingPolicyLink = OrderedDrawingPolicies(PolicyIndex);
		UBOOL bDrawnShared = FALSE;

		for(typename TSparseArray<FElement>::TConstIterator ElementIt(DrawingPolicyLink->Elements); ElementIt; ++ElementIt)
		{
			const FElement& Element = *ElementIt;

			// Check if the element is visible.
			if(StaticMeshVisibilityMap(Element.MeshId))
			{
				FOcclusionQueryRHIRef Query;
				
				// 아직 visible하다고 판명되지 않은 것에 대해서만 query!
				if (!OutStaticMeshVisibilityMap(Element.MeshId))				
				{
					Query = GOcclusionQueryPool.Allocate();
				}
				
				OcclusionQueries.AddItem(Query);

				if (IsValidRef(Query))
				{
					RHIBeginOcclusionQuery(Context, Query);
				}				

				SubmitDrawCall(Context, View, Element, DrawingPolicyLink, bDrawnShared);
				bDirty = TRUE;

				if (IsValidRef(Query))
				{
					RHIEndOcclusionQuery(Context, Query);
				}
			}
		}
	}

	INT QueryIndex = 0;
	for(INT PolicyIndex = 0;PolicyIndex < OrderedDrawingPolicies.Num();PolicyIndex++)
	{
		const FDrawingPolicyLink* DrawingPolicyLink = OrderedDrawingPolicies(PolicyIndex);
		UBOOL bDrawnShared = FALSE;

		for(typename TSparseArray<FElement>::TConstIterator ElementIt(DrawingPolicyLink->Elements); ElementIt; ++ElementIt)
		{
			const FElement& Element = *ElementIt;

			// Check if the element is visible.
			if(StaticMeshVisibilityMap(Element.MeshId))
			{
				FOcclusionQueryRHIRef Query = OcclusionQueries(QueryIndex++);

				if (IsValidRef(Query))
				{
					DWORD OutNumPixels;

					if (RHIGetOcclusionQueryResult(Query,OutNumPixels,TRUE) && OutNumPixels)
					{
						OutStaticMeshVisibilityMap(Element.MeshId) = TRUE;										
					}
				}				
			}
		}
	}
	return bDirty;
}

#endif

UBOOL GForceClearBackbuffer = FALSE;

extern FParticleDataManager	GParticleDataManager;

UBOOL GExpectingDepthBuffer = FALSE;

/**
	This debug variable is toggled by the 'toggleocclusion' console command.
	Turning it on will still issue all queries but ignore the results, which
	makes it possible to analyze those queries in NvPerfHUD. It will also
	stabilize the succession of draw calls in a paused game.
*/
UBOOL GIgnoreAllOcclusionQueries = FALSE;

/**
This debug variable is set by the 'FullMotionBlur [N]' console command.
Setting N to -1 or leaving it blank will make the Motion Blur effect use the
default setting (whatever the game chooses).
N = 0 forces FullMotionBlur off.
N = 1 forces FullMotionBlur on.
*/
INT GMotionBlurFullMotionBlur = -1;


/*-----------------------------------------------------------------------------
	FViewInfo
-----------------------------------------------------------------------------*/

/** 
 * Initialization constructor. Passes all parameters to FSceneView constructor
 */
FViewInfo::FViewInfo(
	const FSceneViewFamily* InFamily,
	FSceneViewStateInterface* InState,
	FSynchronizedActorVisibilityHistory* InActorVisibilityHistory,
	const AActor* InViewActor,
	const UPostProcessChain* InPostProcessChain,
	const FPostProcessSettings* InPostProcessSettings,
	FViewElementDrawer* InDrawer,
	FLOAT InX,
	FLOAT InY,
	FLOAT InSizeX,
	FLOAT InSizeY,
	const FMatrix& InViewMatrix,
	const FMatrix& InProjectionMatrix,
	const FMatrix& InForegroundProjectMatrix,
	FLOAT InForegroundFOV,
	const FLinearColor& InBackgroundColor,
	const FLinearColor& InOverlayColor,
	const FLinearColor& InColorScale,
	const TArray<FPrimitiveSceneInfo*>& InHiddenPrimitives,
	FLOAT InLODDistanceFactor
	)
	:	FSceneView(
			InFamily,
			InState,
			InActorVisibilityHistory,
			InViewActor,
			InPostProcessChain,
			InPostProcessSettings,
			InDrawer,
			InX,
			InY,
			InSizeX,
			InSizeY,
			InViewMatrix,
			InProjectionMatrix,
			InForegroundProjectMatrix,
			InForegroundFOV,
			InBackgroundColor,
			InOverlayColor,
			InColorScale,
			InHiddenPrimitives,
			InLODDistanceFactor
			)
	,	bRequiresVelocities( FALSE )
	,	bRequiresDepth( FALSE )
	,	bIgnoreExistingQueries( FALSE )
	,	bDisableQuerySubmissions( FALSE )
	,	bUseLDRSceneColor( FALSE )
	,	NumVisibleStaticMeshElements(0)
	,	NumVisibleDynamicPrimitives(0)
	,	NumVisibleSeeThroughStaticMeshElements(0)
	,	NumVisibleSeeThroughDynamicPrimitives(0)
	//<@ ava specific ; 
	,	bHasOpaqueViewMeshElements( 0 )
	//>@ ava
	,	IndividualOcclusionQueries((FSceneViewState*)InState,1)
	,	IndividualCasterOcclusionQueries((FSceneViewState*)InState,1)	// ava specific ; 2008. 1. 18 changmin ; add cascaded shadow
#if FINAL_RELEASE	
	,	GroupedOcclusionQueries((FSceneViewState*)InState, FOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)
	,	GroupedCasterOcclusionQueries((FSceneViewState*)InState, AvaCasterOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)	// ava specific ; 2008. 1. 18
#else
	,	GroupedOcclusionQueries((FSceneViewState*)InState, GUpdatingCullDistances ? 1 : FOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)
	,	GroupedCasterOcclusionQueries((FSceneViewState*)InState, AvaCasterOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)	// ava specific ; 2008. 1. 18
#endif

{
	PrevViewProjMatrix.SetIdentity();

	// Clear fog constants.
	for(INT LayerIndex = 0; LayerIndex < ARRAY_COUNT(FogDistanceScale); LayerIndex++)
	{
		FogMinHeight[LayerIndex] = FogMaxHeight[LayerIndex] = FogDistanceScale[LayerIndex] = 0.0f;
		FogStartDistance[LayerIndex] = 0.f;
		FogInScattering[LayerIndex] = FLinearColor::Black;
		FogExtinctionDistance[LayerIndex] = FLT_MAX;
	}
}

/** 
 * Initialization constructor. 
 * @param InView - copy to init with
 */
FViewInfo::FViewInfo(const FSceneView* InView)
	:	FSceneView(*InView)
	,	bHasTranslucentViewMeshElements( 0 )
	,	bHasDistortionViewMeshElements( 0 )
	,	bRequiresVelocities( FALSE )
	,	bRequiresDepth( FALSE )
	,	bIgnoreExistingQueries( FALSE )
	,	bDisableQuerySubmissions( FALSE )
	,	NumVisibleStaticMeshElements(0)
	,	NumVisibleDynamicPrimitives(0)
	,	NumVisibleSeeThroughStaticMeshElements(0)
	,	NumVisibleSeeThroughDynamicPrimitives(0)
	//<@ ava specific ; 
	,	bHasOpaqueViewMeshElements( 0 )
	//>@ ava	
	,	IndividualOcclusionQueries((FSceneViewState*)InView->State,1)
	,	IndividualCasterOcclusionQueries((FSceneViewState*)InView->State,1)	// ava specific ; 2008. 1. 18 changmin ; add cascaded shadow
#if FINAL_RELEASE	
	,	GroupedOcclusionQueries((FSceneViewState*)InView->State, FOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)
	,	GroupedCasterOcclusionQueries((FSceneViewState*)InView->State, AvaCasterOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)	// ava specific ; 2008. 1. 18
#else
	,	GroupedOcclusionQueries((FSceneViewState*)InView->State, GUpdatingCullDistances ? 1 : FOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)
	,	GroupedCasterOcclusionQueries((FSceneViewState*)InView->State, AvaCasterOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)	// ava specific ; 2008. 1. 18
#endif

{
	PrevViewProjMatrix.SetIdentity();

	// Clear fog constants.
	for(INT LayerIndex = 0; LayerIndex < ARRAY_COUNT(FogDistanceScale); LayerIndex++)
	{
		FogMinHeight[LayerIndex] = FogMaxHeight[LayerIndex] = FogDistanceScale[LayerIndex] = 0.0f;
		FogStartDistance[LayerIndex] = 0.f;
		FogInScattering[LayerIndex] = FLinearColor::Black;
		FogExtinctionDistance[LayerIndex] = FLT_MAX;
	}	

	if( PostProcessChain )
	{
		// create render proxies for any post process effects in this view
		for( INT EffectIdx=0; EffectIdx < PostProcessChain->Effects.Num(); EffectIdx++ )
		{
			UPostProcessEffect* Effect = PostProcessChain->Effects(EffectIdx);
			// only add a render proxy if the effect is enabled
			if( Effect->IsShown(InView) )
			{
				FPostProcessSceneProxy* PostProcessSceneProxy = Effect->CreateSceneProxy(
					PostProcessSettings && Effect->bUseWorldSettings ? PostProcessSettings : NULL
					);
				if( PostProcessSceneProxy )
				{
					PostProcessSceneProxies.AddRawItem(PostProcessSceneProxy);
					bRequiresVelocities = bRequiresVelocities || PostProcessSceneProxy->RequiresVelocities( MotionBlurParameters );
					bRequiresDepth = bRequiresDepth || PostProcessSceneProxy->RequiresDepth();
				}
			}
		}

		// Mark the final post-processing effect so that we can render it directly to the view render target.
		UINT DPGIndex = SDPG_PostProcess;
		INT FinalIdx = -1;
		for( INT ProxyIdx=0; ProxyIdx < PostProcessSceneProxies.Num(); ProxyIdx++ )
		{
			if( PostProcessSceneProxies(ProxyIdx).GetDepthPriorityGroup() == DPGIndex )
			{
				FinalIdx = ProxyIdx;
				PostProcessSceneProxies(FinalIdx).TerminatesPostProcessChain( FALSE );
			}
		}
		if (FinalIdx != -1)
		{
			PostProcessSceneProxies(FinalIdx).TerminatesPostProcessChain( TRUE );
		}
	}
}

/** 
 * Destructor. 
 */
FViewInfo::~FViewInfo()
{
	for(INT ResourceIndex = 0;ResourceIndex < DynamicResources.Num();ResourceIndex++)
	{
		DynamicResources(ResourceIndex)->ReleasePrimitiveResource();
	}
}

/** 
 * Initializes the dynamic resources used by this view's elements. 
 */
void FViewInfo::InitDynamicResources()
{
	for(INT ResourceIndex = 0;ResourceIndex < DynamicResources.Num();ResourceIndex++)
	{
		DynamicResources(ResourceIndex)->InitPrimitiveResource();
	}
}



/*-----------------------------------------------------------------------------
FSceneRenderer
-----------------------------------------------------------------------------*/

FSceneRenderer::FSceneRenderer(const FSceneViewFamily* InViewFamily,FHitProxyConsumer* HitProxyConsumer,const FMatrix& InCanvasTransform)
:	Scene(InViewFamily->Scene ? (FScene*)InViewFamily->Scene : NULL)
,	ViewFamily(*InViewFamily)
,	CanvasTransform(InCanvasTransform)
,	PreviousFrameTime(0)
{
	// Collect dynamic shadow stats if the view family has the appropriate container object.
	if( InViewFamily->DynamicShadowStats )
	{
		bShouldGatherDynamicShadowStats = TRUE;
	}
	else
	{
		bShouldGatherDynamicShadowStats = FALSE;
	}

	// Copy the individual views.
	Views.Empty(InViewFamily->Views.Num());
	for(INT ViewIndex = 0;ViewIndex < InViewFamily->Views.Num();ViewIndex++)
	{
		// Construct a FViewInfo with the FSceneView properties.
		FViewInfo* ViewInfo = new(Views) FViewInfo(InViewFamily->Views(ViewIndex));
		ViewFamily.Views(ViewIndex) = ViewInfo;
		ViewInfo->Family = &ViewFamily;

		// Batch the view's elements for later rendering.
		if(ViewInfo->Drawer)
		{
			FViewElementPDI ViewElementPDI(ViewInfo,HitProxyConsumer);
			ViewInfo->Drawer->Draw(ViewInfo,&ViewElementPDI);
		}
	}
	
	if(HitProxyConsumer)
	{
		// Set the hit proxies show flag.
		ViewFamily.ShowFlags |= SHOW_HitProxies;
	}

	// Calculate the screen extents of the view family.
	UBOOL bInitializedExtents = FALSE;
	FLOAT MinFamilyX = 0;
	FLOAT MinFamilyY = 0;
	FLOAT MaxFamilyX = 0;
	FLOAT MaxFamilyY = 0;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FSceneView* View = &Views(ViewIndex);
		if(!bInitializedExtents)
		{
			MinFamilyX = View->X;
			MinFamilyY = View->Y;
			MaxFamilyX = View->X + View->SizeX;
			MaxFamilyY = View->Y + View->SizeY;
			bInitializedExtents = TRUE;
		}
		else
		{
			MinFamilyX = Min(MinFamilyX,View->X);
			MinFamilyY = Min(MinFamilyY,View->Y);
			MaxFamilyX = Max(MaxFamilyX,View->X + View->SizeX);
			MaxFamilyY = Max(MaxFamilyY,View->Y + View->SizeY);
		}
	}
	FamilySizeX = appTrunc(MaxFamilyX - MinFamilyX);
	FamilySizeY = appTrunc(MaxFamilyY - MinFamilyY);

	// Allocate the render target space to 
	check(bInitializedExtents);
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		View.RenderTargetX = appTrunc(View.X - MinFamilyX);
		View.RenderTargetY = appTrunc(View.Y - MinFamilyY);
		View.RenderTargetSizeX = Min<INT>(appTrunc(View.SizeX),ViewFamily.RenderTarget->GetSizeX());
		View.RenderTargetSizeY = Min<INT>(appTrunc(View.SizeY),ViewFamily.RenderTarget->GetSizeY());

		// Set the vector used by shaders to convert projection-space coordinates to texture space.
		View.ScreenPositionScaleBias =
			FVector4(
			View.SizeX / GSceneRenderTargets.GetBufferSizeX() / +2.0f,
			View.SizeY / GSceneRenderTargets.GetBufferSizeY() / -2.0f,
			(View.SizeY / 2.0f + GPixelCenterOffset + View.RenderTargetY) / GSceneRenderTargets.GetBufferSizeY(),
			(View.SizeX / 2.0f + GPixelCenterOffset + View.RenderTargetX) / GSceneRenderTargets.GetBufferSizeX()
			);
	}

	//// clear the motion blur information for the scene
	//ViewFamily.Scene->ClearMotionBlurInfo();

#if !FINAL_RELEASE
	// copy off the requests
	// (I apologize for the const_cast, but didn't seem worth refactoring just for the freezerendering command)
	bHasRequestedToggleFreeze = const_cast<FRenderTarget*>(InViewFamily->RenderTarget)->HasToggleFreezeCommand();
#endif
}

/**
 * Helper structure for sorted shadow stats.
 */
struct FSortedShadowStats
{
	/** Light/ primitive interaction.												*/
	FLightPrimitiveInteraction* Interaction;
	/** Shadow stat.																*/
	FCombinedShadowStats		ShadowStat;
};

/** Comparison operator used by sort. Sorts by light and then by pass number.	*/
IMPLEMENT_COMPARE_CONSTREF( FSortedShadowStats, SceneRendering,	{ if( B.Interaction->GetLightId() == A.Interaction->GetLightId() ) { return A.ShadowStat.ShadowPassNumber - B.ShadowStat.ShadowPassNumber; } else { return A.Interaction->GetLightId() - B.Interaction->GetLightId(); } } )

/**
 * Destructor, stringifying stats if stats gathering was enabled. 
 */
FSceneRenderer::~FSceneRenderer()
{
#if STATS
	// Stringify stats.
	if( bShouldGatherDynamicShadowStats )
	{
		check(ViewFamily.DynamicShadowStats);

		// Move from TMap to TArray and sort shadow stats.
		TArray<FSortedShadowStats> SortedShadowStats;
		for( TMap<FLightPrimitiveInteraction*,FCombinedShadowStats>::TIterator It(InteractionToDynamicShadowStatsMap); It; ++It )
		{
			FSortedShadowStats SortedStat;
			SortedStat.Interaction	= It.Key();
			SortedStat.ShadowStat	= It.Value();
			SortedShadowStats.AddItem( SortedStat );
		}

		// Only sort if there is something to sort.
		if( SortedShadowStats.Num() )
		{
			Sort<USE_COMPARE_CONSTREF(FSortedShadowStats,SceneRendering)>( SortedShadowStats.GetTypedData(), SortedShadowStats.Num() );
		}

		const ULightComponent* PreviousLightComponent = NULL;

		// Iterate over sorted list and stringify entries.
		for( INT StatIndex=0; StatIndex<SortedShadowStats.Num(); StatIndex++ )
		{
			const FSortedShadowStats&	SortedStat		= SortedShadowStats(StatIndex);
			FLightPrimitiveInteraction* Interaction		= SortedStat.Interaction;
			const FCombinedShadowStats&	ShadowStat		= SortedStat.ShadowStat;
			const ULightComponent*		LightComponent	= Interaction->GetLight()->LightComponent;
			
			// Only emit light row if the light has changed.
			if( PreviousLightComponent != LightComponent )
			{
				PreviousLightComponent = LightComponent;
				FDynamicShadowStatsRow StatsRow;
				
				// Figure out light name and add it.
				FString	LightName;
				if( LightComponent->GetOwner() )
				{
					LightName = LightComponent->GetOwner()->GetName();
				}
				else
				{
					LightName = FString(TEXT("Ownerless ")) + LightComponent->GetClass()->GetName();
				}
				new(StatsRow.Columns) FString(*LightName);
				
				// Add radius information for pointlight derived classes.
				FString Radius;
				if( LightComponent->IsA(UPointLightComponent::StaticClass()) )
				{
					Radius = FString::Printf(TEXT("Radius: %i"),appTrunc(((UPointLightComponent*)LightComponent)->Radius));
				}
				new(StatsRow.Columns) FString(*Radius);

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Class name.
				new(StatsRow.Columns) FString(LightComponent->GetClass()->GetName());

				// Fully qualified name.
				new(StatsRow.Columns) FString(*LightComponent->GetPathName().Replace(PLAYWORLD_PACKAGE_PREFIX,TEXT("")));

				// Add row.
				INT Index = ViewFamily.DynamicShadowStats->AddZeroed();
				(*ViewFamily.DynamicShadowStats)(Index) = StatsRow;
			}

			// Subjects
			for( INT SubjectIndex=0; SubjectIndex<ShadowStat.SubjectPrimitives.Num(); SubjectIndex++ )
			{
				const FPrimitiveSceneInfo*	PrimitiveSceneInfo	= ShadowStat.SubjectPrimitives(SubjectIndex);
				UPrimitiveComponent*		PrimitiveComponent	= PrimitiveSceneInfo->Component;
				FDynamicShadowStatsRow		StatsRow;

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Figure out actor name and add it.
				FString	PrimitiveName;
				if( PrimitiveComponent->GetOwner() )
				{
					PrimitiveName = PrimitiveComponent->GetOwner()->GetName();
				}
				else if( PrimitiveComponent->IsA(UModelComponent::StaticClass()) )
				{
					PrimitiveName = FString(TEXT("BSP"));
				}
				else
				{
					PrimitiveName = FString(TEXT("Ownerless"));
				}
				new(StatsRow.Columns) FString(*PrimitiveName);

				// Shadow type.
				FString ShadowType;
				if( ShadowStat.ShadowResolution != INDEX_NONE )
				{
					ShadowType = FString::Printf(TEXT("Projected (Res: %i)"), ShadowStat.ShadowResolution);
				}
				else
				{
					ShadowType = TEXT("Volume");
				}
				new(StatsRow.Columns) FString(*ShadowType);

				// Shadow pass number.
				FString ShadowPassNumber = TEXT("");
				if( ShadowStat.ShadowResolution != INDEX_NONE )
				{
					if( ShadowStat.ShadowPassNumber != INDEX_NONE )
					{
						ShadowPassNumber = FString::Printf(TEXT("%i"),ShadowStat.ShadowPassNumber);
					}
					else
					{
						ShadowPassNumber = TEXT("N/A");
					}
				}
				new(StatsRow.Columns) FString(*ShadowPassNumber);

				// Class name.
				new(StatsRow.Columns) FString(PrimitiveComponent->GetClass()->GetName());

				// Fully qualified name.
				new(StatsRow.Columns) FString(*PrimitiveComponent->GetPathName().Replace(PLAYWORLD_PACKAGE_PREFIX,TEXT("")));

				// Resource name
				UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>( PrimitiveComponent );
				USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>( PrimitiveComponent );

				if (StaticMeshComponent)
				{
					new(StatsRow.Columns) FString( *StaticMeshComponent->StaticMesh->GetFullName() );
				}				
				else if (SkeletalMeshComponent)
				{
					new(StatsRow.Columns) FString( *SkeletalMeshComponent->SkeletalMesh->GetFullName() );
				}
				else
				{
					new(StatsRow.Columns) FString( TEXT("???") );
				}

				// Add row.
				INT Index = ViewFamily.DynamicShadowStats->AddZeroed();
				(*ViewFamily.DynamicShadowStats)(Index) = StatsRow;
			}

			// PreShadow
			for( INT PreShadowIndex=0; PreShadowIndex<ShadowStat.PreShadowPrimitives.Num(); PreShadowIndex++ )
			{
				const FPrimitiveSceneInfo*	PrimitiveSceneInfo	= ShadowStat.PreShadowPrimitives(PreShadowIndex);
				UPrimitiveComponent*		PrimitiveComponent	= PrimitiveSceneInfo->Component;
				FDynamicShadowStatsRow		StatsRow;

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Figure out actor name and add it.
				FString	PrimitiveName;
				if( PrimitiveComponent->GetOwner() )
				{
					PrimitiveName = PrimitiveComponent->GetOwner()->GetName();
				}
				else if( PrimitiveComponent->IsA(UModelComponent::StaticClass()) )
				{
					PrimitiveName = FString(TEXT("BSP"));
				}
				else
				{
					PrimitiveName = FString(TEXT("Ownerless"));
				}
				new(StatsRow.Columns) FString(*PrimitiveName);

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Class name.
				new(StatsRow.Columns) FString(PrimitiveComponent->GetClass()->GetName());

				// Fully qualified name.
				new(StatsRow.Columns) FString(*PrimitiveComponent->GetPathName().Replace(PLAYWORLD_PACKAGE_PREFIX,TEXT("")));

				// Resource name
				UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>( PrimitiveComponent );
				USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>( PrimitiveComponent );

				if (StaticMeshComponent)
				{
					new(StatsRow.Columns) FString( *StaticMeshComponent->StaticMesh->GetFullName() );
				}				
				else if (SkeletalMeshComponent)
				{
					new(StatsRow.Columns) FString( *SkeletalMeshComponent->SkeletalMesh->GetFullName() );
				}
				else
				{
					new(StatsRow.Columns) FString( TEXT("???") );
				}

				// Add row.
				INT Index = ViewFamily.DynamicShadowStats->AddZeroed();
				(*ViewFamily.DynamicShadowStats)(Index) = StatsRow;
			}
		}
	}
#endif
}

/**
* Helper for InitViews to detect large camera movement.
*/
static bool IsLargeCameraMovement(FSceneView& View, const FMatrix& PrevViewMatrix, const FVector& PrevViewOrigin, FLOAT CameraRotationThreshold, FLOAT CameraTranslationThreshold)
{
	/** Disable motion blur if the camera has changed too much this frame. */
	FLOAT RotationThreshold = appCos( CameraRotationThreshold*PI/180.0f );
	FLOAT AngleAxis0 = View.ViewMatrix.GetAxis(0) | PrevViewMatrix.GetAxis(0);
	FLOAT AngleAxis1 = View.ViewMatrix.GetAxis(1) | PrevViewMatrix.GetAxis(1);
	FLOAT AngleAxis2 = View.ViewMatrix.GetAxis(2) | PrevViewMatrix.GetAxis(2);
	FVector Distance = FVector(View.ViewOrigin) - PrevViewOrigin;
	return 
		AngleAxis0 < RotationThreshold ||
		AngleAxis1 < RotationThreshold ||
		AngleAxis2 < RotationThreshold ||
		Distance.Size() > CameraTranslationThreshold;
}

/**
 * Initialize scene's views.
 * Check visibility, sort translucent items, etc.
 */
void FSceneRenderer::InitViews()
{
	SCOPE_CYCLE_COUNTER(STAT_InitViewsTime);

	// Setup motion blur parameters (also check for camera movement thresholds)
	PreviousFrameTime = ViewFamily.CurrentRealTime;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		FSceneViewState* ViewState = (FSceneViewState*) View.State;
		static UBOOL bEnableTimeScale = TRUE;

		// We can't use LatentOcclusionQueries when doing TiledScreenshot because in that case
		// 1-frame lag = 1-tile lag = clipped geometry, so we turn off occlusion queries
		// Occlusion culling is also disabled for hit proxy rendering.
		extern UBOOL GIsTiledScreenshot;
		const UBOOL bIsHitTesting = (ViewFamily.ShowFlags & SHOW_HitProxies) != 0;		

		if (GIsTiledScreenshot || GIgnoreAllOcclusionQueries || bIsHitTesting)
		{
			View.bDisableQuerySubmissions = TRUE;
			View.bIgnoreExistingQueries = TRUE;
		}

		if ( ViewState )
		{
			// determine if we are initializing or we should reset the persistent state
			FLOAT DeltaTime = View.Family->CurrentRealTime - ViewState->LastRenderTime;
			UBOOL bFirstFrameOrTimeWasReset = DeltaTime < -0.0001f || ViewState->LastRenderTime < 0.0001f;

			// detect conditions where we should reset occlusion queries
			if (bFirstFrameOrTimeWasReset || 
				ViewState->LastRenderTime + GEngine->PrimitiveProbablyVisibleTime < View.Family->CurrentRealTime ||
				IsLargeCameraMovement(View, 
				ViewState->PrevViewMatrixForOcclusionQuery, 
				ViewState->PrevViewOriginForOcclusionQuery, 
				GEngine->CameraRotationThreshold, GEngine->CameraTranslationThreshold))
			{
				View.bIgnoreExistingQueries = TRUE;
			}
			ViewState->PrevViewMatrixForOcclusionQuery = View.ViewMatrix;
			ViewState->PrevViewOriginForOcclusionQuery = View.ViewOrigin;


			// detect conditions where we should reset motion blur 
			if (View.bRequiresVelocities)
			{
				if (bFirstFrameOrTimeWasReset || 
					IsLargeCameraMovement(View, 
					ViewState->PrevViewMatrix, ViewState->PrevViewOrigin, 
					View.MotionBlurParameters.RotationThreshold, 
					View.MotionBlurParameters.TranslationThreshold))
				{
					ViewState->PrevProjMatrix				= View.ProjectionMatrix;
					ViewState->PendingPrevProjMatrix		= View.ProjectionMatrix;
					ViewState->PrevViewMatrix				= View.ViewMatrix;
					ViewState->PendingPrevViewMatrix		= View.ViewMatrix;
					ViewState->PrevViewOrigin				= View.ViewOrigin;
					ViewState->PendingPrevViewOrigin		= View.ViewOrigin;
					ViewState->MotionBlurTimeScale			= 1.0f;

					// PT: If the motion blur shader is the last shader in the post-processing chain then it is the one that is
					//     adjusting for the viewport offset.  So it is always required and we can't just disable the work the
					//     shader does.  The correct fix would be to disable the effect when we don't need it and to properly mark
					//     the uber-postprocessing effect as the last effect in the chain.

					//View.bRequiresVelocities				= FALSE;

				}
				else if ( DeltaTime > 0.0001f )
				{
					// New frame detected.
					ViewState->PrevViewMatrix				= ViewState->PendingPrevViewMatrix;
					ViewState->PrevViewOrigin				= ViewState->PendingPrevViewOrigin;
					ViewState->PrevProjMatrix				= ViewState->PendingPrevProjMatrix;
					ViewState->PendingPrevProjMatrix		= View.ProjectionMatrix;
					ViewState->PendingPrevViewMatrix		= View.ViewMatrix;
					ViewState->PendingPrevViewOrigin		= View.ViewOrigin;
					ViewState->MotionBlurTimeScale			= bEnableTimeScale ? (1.0f / (DeltaTime * 30.0f)) : 1.0f;
				}

				View.PrevViewProjMatrix = ViewState->PrevViewMatrix * ViewState->PrevProjMatrix;
			}

			PreviousFrameTime = ViewState->LastRenderTime;
			ViewState->LastRenderTime = View.Family->CurrentRealTime;
		}

		/** Each view starts out rendering to the HDR scene color */
		View.bUseLDRSceneColor = FALSE;
	}

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		FSceneViewState* const ViewState = (FSceneViewState*)View.State;

		if(ViewState)
		{
			// Reset the view's occlusion query pool.
			ViewState->OcclusionQueryPool.Reset();
		}

		// Initialize the view elements' dynamic resources.
		View.InitDynamicResources();				

		// Allocate the view's visibility maps.
		View.PrimitiveVisibilityMap = FBitArray(TRUE,Scene->Primitives.GetMaxIndex());		
		View.StaticMeshVisibilityMap = FBitArray(FALSE,Scene->StaticMeshes.GetMaxIndex());
		View.SeeThroughStaticMeshVisibilityMap = FBitArray(FALSE,Scene->StaticMeshes.GetMaxIndex()); 
		View.StaticMeshOccluderMap = FBitArray(FALSE,Scene->StaticMeshes.GetMaxIndex());
		View.StaticMeshVelocityMap = FBitArray(FALSE,Scene->StaticMeshes.GetMaxIndex());

#if !FINAL_RELEASE
		if (GUpdatingCullDistances)
		{
			View.FineStaticMeshVisibilityMap = FBitArray(FALSE,Scene->StaticMeshes.GetMaxIndex());
		}
#endif

		View.VisibleLightInfos.Empty(Scene->Lights.GetMaxIndex());
		for(INT LightIndex = 0;LightIndex < Scene->Lights.GetMaxIndex();LightIndex++)
		{
			new(View.VisibleLightInfos) FVisibleLightInfo();
		}

		View.PrimitiveViewRelevanceMap.Empty(Scene->Primitives.GetMaxIndex());
		View.PrimitiveViewRelevanceMap.AddZeroed(Scene->Primitives.GetMaxIndex());

		//<@ ava specific ; 2008. 1. 22 changmin
		// add cascaded shadow
		extern UBOOL GUseCascadedShadow;
		if( (ViewFamily.ShowFlags & SHOW_DynamicShadows)
			&& GSystemSettings->bAllowDynamicShadows
			&& GUseCascadedShadow
			&& Scene
			&& Scene->bHasSunLightShadow )
		{
			for( INT SplitIndex = 0; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex )
			{
				View.Ava_ReceiverBounds[SplitIndex].Init();
			}
			
		}
		//>@ ava

		/// Hidden prim쓰지도 않음!AVA
#if 0
		// Mark the primitives in the view's HiddenPrimitives array as not visible.
		for(INT PrimitiveIndex = 0;PrimitiveIndex < View.HiddenPrimitives.Num();PrimitiveIndex++)
		{
			View.PrimitiveVisibilityMap(View.HiddenPrimitives(PrimitiveIndex)->Id) = FALSE;
		}
#endif
	}

	//<@ ava specific ; 2006. 11. 28 changmin
	// pvs rendering
	UBOOL	bUsePVSInfo = FALSE;
	UModel	*StructuralModel = NULL;
	if( Scene && Scene->World && Scene->World->PersistentLevel )
	{
		StructuralModel = Scene->World->PersistentLevel->StructuralModel;
		bUsePVSInfo = StructuralModel && ( StructuralModel->LeafBytes != 0);
	}

#if !FINAL_RELEASE
	if (GUpdatingCullDistances)
	{
		bUsePVSInfo = FALSE;
	}	
#endif
	//>@ ava

	//<@ ava specific ; 2006. 12. 19 changmin
	if( bUsePVSInfo )
	{
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			FViewInfo& View = Views(ViewIndex);
			FSceneViewState *ViewState = (FSceneViewState*) View.State;
			if( !ViewState )
			{
				continue;
			}
			INT CurrentLeaf = INDEX_NONE;
			if( StructuralModel->Nodes.Num() )
			{
				CurrentLeaf = StructuralModel->GetCurrentLeaf( View.ViewOrigin );
			}

			if( CurrentLeaf == INDEX_NONE )
			{
				ViewState->Vis = NULL;
			}
			else
			{
				ViewState->Vis =&( StructuralModel->VisBytes( CurrentLeaf * StructuralModel->LeafBytes ) );
			}
			ViewState->CurrentLeaf = CurrentLeaf;
			if( GEngine->LockPvs == FALSE )
			{
				ViewState->LockedLeaf	= ViewState->CurrentLeaf;
				ViewState->LockedVis	= ViewState->Vis;
			}
		}
	}
	//>@ ava speicifc

	//<@ ava specific ; 2008. 1. 22 changmin
	FMatrix WorldToSun;
	extern UBOOL GUseCascadedShadow;
	if( (ViewFamily.ShowFlags & SHOW_DynamicShadows)
		&& GSystemSettings->bAllowDynamicShadows
		&& GUseCascadedShadow
		&& Scene
		&& Scene->bHasSunLightShadow )
	{
		FVector XAxis, YAxis;
		const FVector FaceDirection = FVector( 1.0f, 0.0f, 0.0f );
		FaceDirection.FindBestAxisVectors( XAxis, YAxis );
		const FBasisVectorMatrix BasisMatrix = FBasisVectorMatrix( -XAxis, YAxis, FaceDirection.SafeNormal(), FVector(0,0,0));
		WorldToSun = FInverseRotationMatrix(
						FVector(Scene->SunLight->WorldToLight.M[0][2],
								Scene->SunLight->WorldToLight.M[1][2],
								Scene->SunLight->WorldToLight.M[2][2]).SafeNormal().Rotation()) * BasisMatrix;
	}
	//>@ ava

	
	// Cull the scene's primitives against the view frustum.
	INT NumOccludedPrimitives = 0;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{		
		FViewInfo& View = Views(ViewIndex);
		FSceneViewState* ViewState = (FSceneViewState*)View.State;

		GCurrentFrame++;

		for(TSparseArray<FPrimitiveSceneInfoCompact>::TConstIterator PrimitiveIt(Scene->Primitives);PrimitiveIt;++PrimitiveIt)
		{
			const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo = *PrimitiveIt;							
		
#if !FINAL_RELEASE
			if( ViewState )
			{
				//// For visibility child views, check if the primitive was visible in the parent view.
				//const FSceneViewState* const ViewParent = (FSceneViewState*)ViewState->GetViewParent();
				//if(ViewParent && !ViewParent->ParentPrimitives.Contains(PrimitiveSceneInfo->Component) )
				//{
				//	continue;
				//}

				// For views with frozen visibility, check if the primitive is in the frozen visibility set.
				if(ViewState->bIsFrozen && !ViewState->FrozenPrimitives.Contains(CompactPrimitiveSceneInfo.Component) )
				{
					continue;
				}
			}
#endif

			if(!View.PrimitiveVisibilityMap.AccessCorrespondingBit(PrimitiveIt))
			{
				// Skip primitives which were marked as not visible because they were in the view's HiddenPrimitives array.
				continue;
			}

			// apply pvs info
			extern UBOOL GIsMinimapshot;
			if( bUsePVSInfo && GIsGame_RenderThread && ViewState && !GIsMinimapshot 
#if !FINAL_RELEASE
			&& !GBakingEnvCubes				
#endif
			)
			{
				BYTE* Vis = NULL;
				const INT CurrentLeaf = ViewState->CurrentLeaf;
				Vis = ViewState->Vis;
				if( CurrentLeaf != INDEX_NONE )
				{
					UBOOL bVisible = FALSE;
					// check model component visibility
					if( CompactPrimitiveSceneInfo.LeafCount < 0 )
					{
						const INT ClusterNumber = CompactPrimitiveSceneInfo.FirstLeafIndex;
						if( ClusterNumber != INDEX_NONE )
						{
							if (Vis[ClusterNumber>>3] & (1<<(ClusterNumber&7))) 
							{
								bVisible = TRUE;
							}
						}
						else
						{
							bVisible = TRUE;						
						}
					}
					else
					{
						// check static mesh component visibility
						if( CompactPrimitiveSceneInfo.LeafCount > 0 )
						{
							for( INT LeafIndex = 0; LeafIndex < CompactPrimitiveSceneInfo.LeafCount; ++LeafIndex )
							{
								// 여기서 clusternumber는 항상 valid한 값입니다.
								const INT ClusterNumber = StructuralModel->StaticMeshLeaves(CompactPrimitiveSceneInfo.FirstLeafIndex + LeafIndex );
								if( Vis[ClusterNumber>>3] & (1<<(ClusterNumber&7)) )
								{
									bVisible = TRUE;
									break;
								}
							}
						}
						else
						{
							// pvs 정보가 없는 staticmesh component ex) owner가 staticmesh actor가 아닌 것들
							bVisible = TRUE;
						}
					}

					// pvs culling
					if( !bVisible )
					{
						// 이것을 해주어야 occlusion query test에서 빠집니다....
						View.PrimitiveVisibilityMap.AccessCorrespondingBit(PrimitiveIt)		= FALSE;
						continue; // goto next view~
					}
				}
			}
			//>@ ava			

			// Check the primitive's bounding box against the view frustum.
			UBOOL bPrimitiveIsVisible = FALSE;
			if(View.ViewFrustum.IntersectBox(CompactPrimitiveSceneInfo.Bounds.Origin,CompactPrimitiveSceneInfo.Bounds.BoxExtent))
			{
				// Prefetch the full primitive scene info.
				PREFETCH(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
				PREFETCH(CompactPrimitiveSceneInfo.Proxy);

				// Cull distance 등으로 early culled out
				if (!CompactPrimitiveSceneInfo.Proxy->IsShown(&View)) // 불필요한 query를 issue하지 않기 위해)
				{
					continue;
				}

				// Check whether the primitive is occluded this frame.
				extern UBOOL GIsMinimapshot;

				const UBOOL bIsOccluded =
					!(ViewFamily.ShowFlags & SHOW_Wireframe) &&
					ViewState &&
					!GIsMinimapshot &&	// ava specific ; 2007. 5. 8
					ViewState->UpdatePrimitiveOcclusion(CompactPrimitiveSceneInfo,View,ViewFamily.CurrentRealTime)				

#if !FINAL_RELEASE
					&& !GBakingEnvCubes				
					&& !GUpdatingCullDistances
#endif
					;
				
				const UBOOL bSeeThrough = (ViewFamily.ShowFlags & SHOW_SeeThrough);
				
				if(bIsOccluded)
				{
					if (bSeeThrough && !(ViewFamily.ShowFlags & SHOW_Wireframe) && ViewState)
					{
						// Compute the primitive's view relevance.
						FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveIt.GetIndex());
						ViewRelevance = CompactPrimitiveSceneInfo.Proxy->GetViewRelevance(&View);

						if (ViewRelevance.bSeeThroughRelevance)
						{
							if(ViewRelevance.bStaticRelevance)
							{
								// Compute the distance to the primitive.
								FLOAT DistanceSquared = 0.0f;
								if(View.ViewOrigin.W > 0.0f)
								{
									DistanceSquared = (CompactPrimitiveSceneInfo.Bounds.Origin - View.ViewOrigin).SizeSquared();
								}

								// Mark the primitive's static meshes as visible.						
								for(INT MeshIndex = 0;MeshIndex < CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes.Num();MeshIndex++)
								{							
									const FStaticMesh& StaticMesh = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes(MeshIndex);
									if(DistanceSquared >= StaticMesh.MinDrawDistanceSquared && DistanceSquared < StaticMesh.MaxDrawDistanceSquared)
									{
										// Mark static mesh as visible for rendering
										View.SeeThroughStaticMeshVisibilityMap(StaticMesh.Id) = TRUE;

										View.NumVisibleSeeThroughStaticMeshElements++;
									}
								}
							}

							if(ViewRelevance.bDynamicRelevance)
							{
								// Keep track of visible dynamic primitives.
								View.SeeThroughVisibleDynamicPrimitives.AddItem(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);

								View.NumVisibleSeeThroughDynamicPrimitives++;

								// process view for this primitive proxy
								CompactPrimitiveSceneInfo.Proxy->PreRenderView(&View, GetGlobalSceneRenderState()->FrameNumber);
							}
						}
					}
				}
				else
				{
					// Compute the primitive's view relevance.
					FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveIt.GetIndex());
					ViewRelevance = CompactPrimitiveSceneInfo.Proxy->GetViewRelevance(&View);

					if(ViewRelevance.bStaticRelevance)
					{
						// Compute the distance to the primitive.
						FLOAT DistanceSquared = 0.0f;
						if(View.ViewOrigin.W > 0.0f)
						{
							DistanceSquared = (CompactPrimitiveSceneInfo.Bounds.Origin - View.ViewOrigin).SizeSquared();
						}

						// Mark the primitive's static meshes as visible.						
						for(INT MeshIndex = 0;MeshIndex < CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes.Num();MeshIndex++)
						{							
							const FStaticMesh& StaticMesh = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes(MeshIndex);
							if(DistanceSquared >= StaticMesh.MinDrawDistanceSquared && DistanceSquared < StaticMesh.MaxDrawDistanceSquared)
							{
								// Mark static mesh as visible for rendering
								View.StaticMeshVisibilityMap(StaticMesh.Id) = TRUE;
								View.NumVisibleStaticMeshElements++;

								if (ViewRelevance.bSeeThroughRelevance && bSeeThrough)
								{
									View.SeeThroughStaticMeshVisibilityMap(StaticMesh.Id) = TRUE;
									View.NumVisibleSeeThroughStaticMeshElements++;
								}									
							}
						}
					}

					if(ViewRelevance.bDynamicRelevance)
					{
						// Keep track of visible dynamic primitives.
						View.VisibleDynamicPrimitives.AddItem(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
						View.NumVisibleDynamicPrimitives++;

						if (ViewRelevance.bSeeThroughRelevance && bSeeThrough)
						{
							View.SeeThroughVisibleDynamicPrimitives.AddItem(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
							View.NumVisibleSeeThroughDynamicPrimitives++;
						}
						// process view for this primitive proxy
						CompactPrimitiveSceneInfo.Proxy->PreRenderView(&View, GetGlobalSceneRenderState()->FrameNumber);							
					}

					if ( ViewRelevance.bDecalRelevance )
					{
						// Add to the set of decal primitives.
						View.DecalPrimSet[CompactPrimitiveSceneInfo.Proxy->GetDepthPriorityGroup(&View)].AddScenePrimitive(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
					}

					if( ViewRelevance.bTranslucentRelevance	)
					{
						// Add to set of dynamic translucent primitives
						View.TranslucentPrimSet[CompactPrimitiveSceneInfo.Proxy->GetDepthPriorityGroup(&View)].AddScenePrimitive(CompactPrimitiveSceneInfo.PrimitiveSceneInfo,View,ViewRelevance.bUsesSceneColor);

						if( ViewRelevance.bDistortionRelevance )
						{
							// Add to set of dynamic distortion primitives
							View.DistortionPrimSet[CompactPrimitiveSceneInfo.Proxy->GetDepthPriorityGroup(&View)].AddScenePrimitive(CompactPrimitiveSceneInfo.PrimitiveSceneInfo,View);
						}
					}

					if( ViewRelevance.IsRelevant() )
					{
						// This primitive is in the view frustum, view relevant, and unoccluded; it's visible.
						bPrimitiveIsVisible = TRUE;

						//<@ ava specific ; 2008. 1. 22 changmin
						// compute cascaded receiver bounds here
						if( (ViewFamily.ShowFlags & SHOW_DynamicShadows)
						&& GSystemSettings->bAllowDynamicShadows
						&& GUseCascadedShadow
						&& Scene
						&& Scene->bHasSunLightShadow )
						{
							if( CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Proxy->GetDepthPriorityGroup(&View) == SDPG_World
								&& CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Proxy->bCastSunShadow
								&& (!CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->HiddenGame || CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->bCastHiddenShadow))
							{
								const UBOOL AddToReceiverBounds = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->IsA(UModelComponent::StaticClass())
									|| CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component->IsA(UStaticMeshComponent::StaticClass());

								if( AddToReceiverBounds )
								{
									for( INT SplitIndex = 0; SplitIndex < FCascadedShadowInfo::NumShadows; ++SplitIndex )
									{
										if( View.Ava_SplitedViewFrustum[SplitIndex].IntersectBox(CompactPrimitiveSceneInfo.Bounds.Origin, CompactPrimitiveSceneInfo.Bounds.BoxExtent) )
										{
											FBox WorldBoundingBox = CompactPrimitiveSceneInfo.Bounds.GetBox();
											FBox SunBoundingBox = WorldBoundingBox.TransformBy( WorldToSun );
											View.Ava_ReceiverBounds[SplitIndex] += SunBoundingBox;
										}
									}
								}
							}
						}
						//>@ ava

						// Add to the scene's list of primitive which are visible and that have lit decals.
						const UBOOL bHasLitDecals = CompactPrimitiveSceneInfo.Proxy->HasLitDecals(&View);
						if ( bHasLitDecals )
						{
							View.VisibleLitDecalPrimitives.AddItem( CompactPrimitiveSceneInfo.PrimitiveSceneInfo );
						}

						// Iterate over the lights affecting the primitive.
						for(const FLightPrimitiveInteraction* Interaction = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->LightList;
							Interaction;
							Interaction = Interaction->GetNextLight()
							)
						{
							// The light doesn't need to be rendered if it only affects light-maps or if it is a skylight.
							const UBOOL bRenderLight =
								(ViewRelevance.bForceDirectionalLightsDynamic && Interaction->GetLight()->LightType == LightType_Directional) || 
								(!Interaction->IsLightMapped() && Interaction->GetLight()->LightType != LightType_Sky);

							if ( bRenderLight )
							{
								FVisibleLightInfo& VisibleLightInfo = View.VisibleLightInfos(Interaction->GetLightId());
								for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
								{
									if(ViewRelevance.GetDPG(DPGIndex))
									{
										if ( bRenderLight )
										{
											// Increment the count of visible primitives affected by the light.
											VisibleLightInfo.DPGInfo[DPGIndex].NumVisibleLitPrimitives++;

											//if( ViewRelevance.bDynamicRelevance )
											//{
											//	// Add dynamic primitives to the light's list of visible dynamic affected primitives.
											//	VisibleLightInfo.DPGInfo[DPGIndex].VisibleDynamicLitPrimitives.AddItem(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
											//}

											if ( bHasLitDecals )
											{
												// Add to the light's list of The primitives which are visible, affected by this light and receiving lit decals.
												VisibleLightInfo.DPGInfo[DPGIndex].VisibleLitDecalPrimitives.AddItem(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
												CompactPrimitiveSceneInfo.Proxy->InitLitDecalFlags(DPGIndex);
											}
										}									
									}
								}
							}
						}
					}
				}					
			}						

			// Update the primitive's visibility state.
			View.PrimitiveVisibilityMap.AccessCorrespondingBit(PrimitiveIt) = bPrimitiveIsVisible;

#if STATS
			if (!bPrimitiveIsVisible)
			{
				NumOccludedPrimitives++;			
			}
#endif
		}
	}
	INC_DWORD_STAT_BY(STAT_OccludedPrimitives,NumOccludedPrimitives);

	// Cull the scene's lights which use modulated shadows against the view frustum.
	for(TSparseArray<FLightSceneInfo*>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
	{
		FLightSceneInfo* LightInfo = *LightIt;
		// only check lights with modulated shadows since the shadow is rendered even if NumVisibleLitPrimitives==0
		if( LightInfo->LightShadowMode == LightShadow_Modulate )
		{
			// directional lights aren't frustum culled and are assumed to be visible
			if( LightInfo->LightType == LightType_Directional )
			{
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{		
					FViewInfo& View = Views(ViewIndex);
					FVisibleLightInfo& VisibleLightInfo = View.VisibleLightInfos(LightInfo->Id);
					for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
					{
						VisibleLightInfo.DPGInfo[DPGIndex].bVisibleInFrustum = TRUE;
					}
				}
			}
			// only spot/point lights can be frustum culled
			else if( LightInfo->LightType == LightType_Point || LightInfo->LightType == LightType_Spot )
			{
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{		
					FViewInfo& View = Views(ViewIndex);
					if( View.ViewFrustum.IntersectSphere(LightInfo->GetOrigin(),LightInfo->GetRadius()) )
					{
						FVisibleLightInfo& VisibleLightInfo = View.VisibleLightInfos(LightInfo->Id);
						for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
						{
							VisibleLightInfo.DPGInfo[DPGIndex].bVisibleInFrustum = TRUE;
						}
					}
				}
			}			
		}
	}

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{		
		FViewInfo& View = Views(ViewIndex);

		// sort the translucent primitives
		for( INT DPGIndex=0; DPGIndex < SDPG_MAX_SceneRender; DPGIndex++ )
		{
			View.TranslucentPrimSet[DPGIndex].SortPrimitives();
		}
	}

	if( (ViewFamily.ShowFlags & SHOW_DynamicShadows) && GSystemSettings->bAllowDynamicShadows )
	{
		// Setup dynamic shadows.
		InitDynamicShadows();				
	}

	// Initialize the fog constants.
	InitFogConstants();	
}

/** 
* @return text description for each DPG 
*/
TCHAR* GetSceneDPGName( ESceneDepthPriorityGroup DPG )
{
	switch(DPG)
	{
	case SDPG_UnrealEdBackground:
		return TEXT("UnrealEd Background");
	case SDPG_World:
		return TEXT("World");
	case SDPG_Foreground:
		return TEXT("Foreground");
	//<@ava specific
	// 2006/8/21. changmin.
	case SDPG_ForeForeground:
		return TEXT("ForeForeground");
	//>@ava
	case SDPG_UnrealEdForeground:
		return TEXT("UnrealEd Foreground");
	case SDPG_PostProcess:
		return TEXT("PostProcess");
	case SDPG_SkyLayer0 :
		return TEXT("SkyLayer0");
	case SDPG_SkyLayer1 :
		return TEXT("SkyLayer1");
	case SDPG_SkyLayer2 :
		return TEXT("SkyLayer2");
	case SDPG_SkyLayer3 :
		return TEXT("SkyLayer3");
	default:
		return TEXT("Unknown");
	};
}

enum TweakMatricesOp
{	
	TMO_Sky,
	TMO_Foreground,
	TMO_ForeForeground,
	TMO_Restore,
};

void TweakMatrices( FSceneRenderer* Renderer, TweakMatricesOp Op )
{
	extern UBOOL GIsTiledScreenshot;

	for(INT ViewIndex = 0;ViewIndex < Renderer->Views.Num();ViewIndex++)
	{
		FViewInfo& View = Renderer->Views(ViewIndex);				

		if (Op == TMO_Restore)
		{
			View.ViewMatrix = View.SavedViewMatrix;
			View.ProjectionMatrix = View.SavedProjectionMatrix;
		}
		else if (Op == TMO_Sky)
		{
			///@ ava Specific ; sky mesh
			if ( GIsGame_RenderThread )
			{
				View.ViewMatrix = View.SavedViewMatrix;

				View.ViewMatrix.M[3][0] = 0;
				View.ViewMatrix.M[3][1] = 0;
				View.ViewMatrix.M[3][2] = 0;

				if (GIsTiledScreenshot)
				{
					extern FMatrix GOriginalProjectionMatrixForTiledShot;
					View.ProjectionMatrix = GOriginalProjectionMatrixForTiledShot;		
				}
				else
				{
					View.ProjectionMatrix = View.SavedProjectionMatrix;								
				}				
			}
		}
		else if (Op == TMO_Foreground)
		{
			if( GIsGame_RenderThread )
			{
				View.ViewMatrix = View.SavedViewMatrix;
				View.ProjectionMatrix = View.ForegroundProjectionMatrix;
			}
		}
		else if (Op == TMO_ForeForeground)
		{
			View.ViewMatrix = FMatrix::Identity;
			View.ProjectionMatrix = View.ForegroundProjectionMatrix;
		}

		View.CalculateDerivedMatrices();					
		
		if (Op == TMO_Sky && GIsTiledScreenshot)
		{
			extern FMatrix GTiledShotTweakMatrix;

			View.ViewProjectionMatrix = View.ViewProjectionMatrix * GTiledShotTweakMatrix;			
		}
	}	
}

UBOOL GSceneDepthDirty = FALSE;

void MarkDepthDirty()
{
	GSceneDepthDirty = TRUE;
}

/** returns TRUE if color buffer was also resolved */
UBOOL ResolveDepth()
{
	if (GSceneDepthDirty)
	{
		/// Depth resolved :)
		GSceneDepthDirty = FALSE;

		if (GSupportsDepthTextures)
		{
			GSceneRenderTargets.ResolveSceneDepthTexture();

			return FALSE;
		}
		else
		{	
			GSceneRenderTargets.FinishRenderingSceneColor(TRUE);
			
			return TRUE;
		}				
	}	
	else
		return FALSE;
}
/** 
* Renders the view family. 
*/
void FSceneRenderer::Render()
{
#if !FINAL_RELEASE
	extern INT GCullDistanceRequestCounter;

	if (!GUpdatingCullDistances && GCullDistanceRequestCounter > 0)
	{
		if (--GCullDistanceRequestCounter == 0)
		{
			GUpdatingCullDistances = TRUE;
		}
	}

	GIsGame_RenderThread = !GIsEditor || GIsPlayInEditorWorld_RenderThread;
#endif

	if (GIsLowEndHW)
	{
		//GSceneRenderTargets.OverrideSceneColorSurface( ViewFamily.RenderTarget->RenderTargetSurfaceRHI );
	}

	if( GIsLowEndHW )
	{
		RHISetSRGBWriteEnable( NULL, TRUE );
	}
	
	// Allocate the maximum scene render target space for the current view family.
	GSceneRenderTargets.Allocate( ViewFamily.RenderTarget->GetSizeX(), ViewFamily.RenderTarget->GetSizeY() );

	// Find the visible primitives.
	InitViews();
	
	// Whether to clear the scene color buffer before rendering color for the first DPG. When using tiling, this 
	// gets cleared later automatically.
	UBOOL bRequiresClear = !GUseTilingCode;

#if PS3
	// On PS3 we're not clearing the scene color at all, since it's going to be completely filled with game graphics anyway.
	bRequiresClear = FALSE;
#endif

	//<@ ava specific ; 2006. 12. 01 changmin
	// bsp rendering
	GBSPDynamicBatchCount++;

	void InitDynamicMeshForNewFrame();
	InitDynamicMeshForNewFrame();
	//>@ ava

	for(UINT InternalDPGIndex = 0;InternalDPGIndex < SDPG_MAX_SceneRender;InternalDPGIndex++)
	{
		UINT DPGIndex;
		
		///@ava specific :) 
		/// Swizzle DPG Index
		switch (InternalDPGIndex)
		{
		case 0 : DPGIndex = SDPG_UnrealEdBackground; break;

		case 1 : DPGIndex = SDPG_World; break;

		case 2 : continue;
		case 3 : continue;
		case 4 : continue;
		case 5 : continue;

		case 6 : DPGIndex = SDPG_Foreground; break;
		case 7 : DPGIndex = SDPG_ForeForeground; break;
		case 8 : DPGIndex = SDPG_UnrealEdForeground; break;
		case 9 : DPGIndex = SDPG_PostProcess; break;
		default : DPGIndex = SDPG_UnrealEdBackground; break;
		};

		//>@ ava specific ; 2006.8.21 changmin.
		// 기획 변경으로 ForeForeground 는 사용하지 않는다. 사용하려면, 아래의 코드를 주석처리하면 된다.
		// Post process가 두번 작동;;;
		if( DPGIndex == SDPG_ForeForeground || DPGIndex == SDPG_PostProcess )
		{
			continue;
		}
		//<@ ava

		if( ViewFamily.bDisableForegroundDPGs && (DPGIndex == SDPG_Foreground || DPGIndex == SDPG_ForeForeground))
		{
			continue;
		}

		//<@ ava specific ; 2007. 2. 13 changmin
		if( DPGIndex == SDPG_PostProcess )
		{
			continue;
		}
		extern UBOOL	GIsMinimapshot;
		if( GIsMinimapshot && DPGIndex != SDPG_World )
		{
			continue;
		}
		if( GIsMinimapshot)
		{
			bRequiresClear = TRUE;
		}
		//>@ ava

		// WARNING: if any Xbox360 rendering uses SDPG_UnrealEdBackground, it will not show up with tiling enabled
		// unless you go out of your way to Resolve the scene color and Restore it at the beginning of the tiling pass.
		// SDPG_World and SDPG_Foreground work just fine, however.
		const UBOOL bWorldDpg = (DPGIndex == SDPG_World);		
		
		// Skip Editor-only DPGs for game rendering.
		if( GIsGame_RenderThread && (DPGIndex == SDPG_UnrealEdBackground || DPGIndex == SDPG_UnrealEdForeground) )
		{
			continue;
		}		

		///@ ava Specific ; foreground FOV
		if (DPGIndex == SDPG_Foreground && !(ViewFamily.ShowFlags & (SHOW_Bounds || SHOW_ShadowFrustums)))
		{		
			TweakMatrices( this, TMO_Foreground );			
		}

		//<@ ava specific; foreforeground ; 2006.8.22 changmin
		// forefore ground 의 개체들은 view space 개체들이다.
		if( DPGIndex == SDPG_ForeForeground )
		{
			TweakMatrices( this, TMO_ForeForeground );						
		}
		//>@ ava

		SCOPED_DRAW_EVENT(EventDPG)(DEC_SCENE_ITEMS,TEXT("DPG %s"),GetSceneDPGName((ESceneDepthPriorityGroup)DPGIndex));

		// force using occ queries for wireframe if rendering is frozen in the first view
		check(Views.Num());
		UBOOL bIsOcclusionAllowed = (DPGIndex == SDPG_World) && !GIgnoreAllOcclusionQueries && !ViewFamily.bDisableOcclusionQuery;		
		const UBOOL bIsOcclusionTesting = bIsOcclusionAllowed && 
			(
				!(ViewFamily.ShowFlags & SHOW_Wireframe)
#if !FINAL_RELEASE
				|| ((FSceneViewState*)Views(0).State)->bIsFrozen
#endif
			); 

		if( GUseTilingCode && bWorldDpg )
		{
			RHIMSAAInitPrepass();
		}

		// Draw the scene pre-pass
		UBOOL bDirtyPrePass = RenderPrePass(DPGIndex,bIsOcclusionTesting);

		// Render the velocities of movable objects for the motion blur effect.
		if( !GUseTilingCode && bWorldDpg )
		{
			RenderVelocities(DPGIndex);
		}

		if( GUseTilingCode && bWorldDpg )
		{
			RHIMSAABeginRendering();
		}
		else
		{
			// Begin drawing the scene color.
			GSceneRenderTargets.BeginRenderingSceneColor();
		}		

		UBOOL bEmissiveDirtiedColor = FALSE;

		// Clear scene color buffer if necessary.
		if( bRequiresClear )
		{
			SCOPED_DRAW_EVENT( EventView )( DEC_SCENE_ITEMS, TEXT( "ClearView" ) );

			// Clear the entire viewport to make sure no post process filters in any data from an invalid region
			RHISetViewport( GlobalContext, 0, 0, 0.0f, ViewFamily.RenderTarget->GetSizeX(), ViewFamily.RenderTarget->GetSizeY(), 1.0f );

			if (!(ViewFamily.ShowFlags & SHOW_Lighting))
			{
				//<@ ava specific ; 2006. 9. 21 changmin : color 를 선택적으로 clear하기 위해 이것이 필요 없습니다.
				RHIClear( GlobalContext, TRUE, FLinearColor::Black, FALSE, 0, FALSE, 0 );			
				//>@
			}			

			// Clear the viewports to their background color
			if( GIsEditor || GForceClearBackbuffer )
			{
				//<@ ava specific ; 2006. 9. 21 changmin : editor는 frame buffer를 0으로 지웁니다.
				RHIClear( GlobalContext, TRUE, FLinearColor::Black, FALSE, 0, FALSE, 0 );

				// Clear each viewport to its own background color
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{
					SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("ClearView%d"),ViewIndex);

					FViewInfo& View = Views(ViewIndex);					

					/// Subview?
					if (View.bTransparentBackground)
					{
						void RHICopyRenderTargetIntoSurface(
							FSurfaceRHIParamRef SourceSurface, 
							FSurfaceRHIParamRef TargetSurface, 
							FTextureRHIParamRef LDRTexture,
							FSurfaceRHIParamRef LDRSurface, 
							INT SourceX, INT SourceY, INT SizeX, INT SizeY, INT TargetX, INT TargetY );

						RHICopyRenderTargetIntoSurface(
							ViewFamily.RenderTarget->RenderTargetSurfaceRHI,
							GSceneRenderTargets.GetSceneColorSurface(),
							GSceneRenderTargets.GetSceneColorLDRTexture(),
							GSceneRenderTargets.GetSceneColorLDRSurface(),
							View.X,View.Y,View.RenderTargetSizeX,View.RenderTargetSizeY,View.RenderTargetX,View.RenderTargetY);												

						GSceneRenderTargets.BeginRenderingSceneColor();

						// Set the device viewport for the view.
						RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
					}
					else
					{
						// Clear the scene color surface when rendering the first DPG.
						RHIClear(GlobalContext,TRUE,View.BackgroundColor,FALSE,0,FALSE,0);
					}					
				}
			}
			else			
			{
				// Clear each viewport to its own background color
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{
					SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("ClearView%d"),ViewIndex);

					FViewInfo& View = Views(ViewIndex);				

					/// Subview?
					if (View.bTransparentBackground)
					{
						void RHICopyRenderTargetIntoSurface(
							FSurfaceRHIParamRef SourceSurface, 
							FSurfaceRHIParamRef TargetSurface, 
							FTextureRHIParamRef LDRTexture,
							FSurfaceRHIParamRef LDRSurface, 
							INT SourceX, INT SourceY, INT SizeX, INT SizeY, INT TargetX, INT TargetY );

						RHICopyRenderTargetIntoSurface(
							ViewFamily.RenderTarget->RenderTargetSurfaceRHI,
							GSceneRenderTargets.GetSceneColorSurface(),
							GSceneRenderTargets.GetSceneColorLDRTexture(),
							GSceneRenderTargets.GetSceneColorLDRSurface(),
							View.X,View.Y,View.RenderTargetSizeX,View.RenderTargetSizeY,View.RenderTargetX,View.RenderTargetY);												

						GSceneRenderTargets.BeginRenderingSceneColor();
					}					
				}

				// Clear the depths to max depth so that depth bias blend materials always show up
				ClearSceneColorDepth();
			}
			// Only clear once.
			bRequiresClear = FALSE;
		} 		

		// draw bsp here
		if (!GIsEditor)
		{
			extern UBOOL GDrawBSP;

			GDrawBSP = TRUE;
			bEmissiveDirtiedColor |= RenderEmissive(DPGIndex);			
			GDrawBSP = FALSE;			
		}		

		if ( ViewFamily.ShowFlags & SHOW_TextureDensity )
		{
			// Draw the emissive pass for all visible primitives.
			bEmissiveDirtiedColor |= RenderTextureDensities(DPGIndex);
		}
		//<@ ava specific ; 2007. 1. 15 changmin
		else if( ViewFamily.ShowFlags & SHOW_ViewSpaceNormal )
		{
			bEmissiveDirtiedColor |= RenderViewSpaceNormals(DPGIndex);
		}
		//>@ ava
		else if( ViewFamily.ShowFlags & SHOW_BumpLighting )
		{
			bEmissiveDirtiedColor |= RenderBumpLighting(DPGIndex);
		}
		else
		{
			// Draw the emissive pass for all visible primitives.
			bEmissiveDirtiedColor |= RenderEmissive(DPGIndex);			
		}

		UBOOL bSceneColorDirty = FALSE;

		if( GUseTilingCode && bWorldDpg )
		{
			RHISetBlendState(GlobalContext, TStaticBlendState<>::GetRHI());
			RHIMSAAEndRendering(GSceneRenderTargets.GetSceneDepthTexture(), GSceneRenderTargets.GetSceneColorTexture());
		}		

		if(GUseTilingCode && bWorldDpg)
		{
			// Need to set the render targets before we try resolving, so call in twice
			GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
			//@todo sz - try to restore depths when restoring scene color (only when MB is disabled)
			RHIMSAARestoreDepth(GSceneRenderTargets.GetSceneDepthTexture());
			RenderVelocities(DPGIndex);
			GSceneRenderTargets.BeginRenderingSceneColor(TRUE);
		}		

		for (INT Phase=0; Phase<2; ++Phase)
		{
			extern UBOOL GDepthDrawingLightOnly;

			GDepthDrawingLightOnly = (Phase == 0);

			/// WorldDPG에서는 무조건 depth가 invalidate된다 :)
			if(Phase == 0 && bWorldDpg)
			{
				MarkDepthDirty();
			}
			
			if(ViewFamily.ShowFlags & SHOW_Lighting
			|| ViewFamily.ShowFlags & SHOW_BumpLighting )	// 2007. 10. 19 changmin : add viewmode bump lighting
			{			
				/// World에서만 그림자를 그릴테니 :)
				if (!GIsLowEndHW && bWorldDpg && (ViewFamily.ShowFlags & SHOW_DynamicShadows) && PreRenderLights(DPGIndex))
				{
					if (ResolveDepth())
						bSceneColorDirty = FALSE;
				}

				// Render the scene lighting.
				bSceneColorDirty |= RenderLights(DPGIndex);

				//<@ ava specific ; 2007. 5. 11
				if( Phase == 0 )
				{
					// 다음에 그릴 light가 있는지 확인한다.
					// light가 있으면 resolve한다.
					// Draw each light.
					for(TSparseArray<FLightSceneInfo*>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
					{
						const FLightSceneInfo* LightSceneInfo = *LightIt;
						const INT LightId = LightIt.GetIndex();

						if ( LightSceneInfo->bIsDepthDrawingLight)
							continue;

						// 다음 phase light가 있네... resolve~
						GSceneRenderTargets.FinishRenderingSceneColor(TRUE);
						GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
						break;
					}
				}

				if( Phase == 1  )
				{
					//<@ ava specific ; 2007. 10. 1 changmin
					// add cascaded shadows
					extern UBOOL GUseCascadedShadow;
					if( bWorldDpg && GUseCascadedShadow && Scene->bHasSunLightShadow )
					{
						MarkDepthDirty();
						ResolveDepth();
						bSceneColorDirty = FALSE;
						bSceneColorDirty |= Ava_RenderLightsUseCascadedShadow(DPGIndex);
					}

					if( DPGIndex == SDPG_Foreground && GUseCascadedShadow && Scene->bHasSunLightShadow )
					{
						bSceneColorDirty |= Ava_RenderLightsUseCascadedShadow(DPGIndex);
					}
					//>@ ava
				}
				//>@ ava

				/// Mod shadow는 마지막에 처리 :)				
				if (!GIsLowEndHW && Phase == 1)
				{
					/// World에서만 그림자를 그릴테니 :)
					if (bWorldDpg && (ViewFamily.ShowFlags & SHOW_DynamicShadows) && PreRenderModulatedShadows(DPGIndex))
					{
						if (ResolveDepth())
							bSceneColorDirty = FALSE;
					}

					// Render the modulated shadows.
					bSceneColorDirty |= RenderModulatedShadows(DPGIndex);					
				}			
			}
		}

		//<@ ava specific ; 2007. 11. 20 changmin
		extern UBOOL GUseCascadedShadow;
		if( GUseCascadedShadow && Scene->bHasSunLightShadow )
		{
			AVA_RenderLitDecals(DPGIndex);
		}
		//>@ ava

		// If any opaque elements were drawn, render pre-fog decals for opaque receivers.
		if( (bEmissiveDirtiedColor || bSceneColorDirty) && 
			(ViewFamily.ShowFlags & SHOW_Decals) )
		{
			GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
			const UBOOL bDecalsRendered = RenderDecals( DPGIndex, FALSE, TRUE );
			GSceneRenderTargets.FinishRenderingSceneColor(FALSE);

			bSceneColorDirty |= bDecalsRendered;
			if (bDecalsRendered && !(GSupportsFPBlending || GIsLowEndHW ))
			{
				//update scene color texture with the last decal rendered
				GSceneRenderTargets.ResolveSceneColor();
				bSceneColorDirty = FALSE;
			}
		}		

		if((ViewFamily.ShowFlags & SHOW_Fog) && GSystemSettings->bAllowFog)
		{
			// Render the scene fog.
			bSceneColorDirty |= RenderFog(DPGIndex);
		}

		if( ViewFamily.ShowFlags & SHOW_UnlitTranslucency )
		{
			// Distortion pass
			bSceneColorDirty |= RenderDistortion(DPGIndex);
		}

		if (bWorldDpg)
		{
			TweakMatrices( this, TMO_Sky );
			bSceneColorDirty |= RenderEmissive( SDPG_SkyLayer0 );
			bSceneColorDirty |= RenderEmissive( SDPG_SkyLayer1 );
			bSceneColorDirty |= RenderEmissive( SDPG_SkyLayer2 );
			bSceneColorDirty |= RenderEmissive( SDPG_SkyLayer3 );
			TweakMatrices( this, TMO_Restore );
		}

		if(bSceneColorDirty)
		{
			// Save the color buffer if any uncommitted changes have occurred
			GSceneRenderTargets.ResolveSceneColor();
			bSceneColorDirty = FALSE;
		}

		if( ViewFamily.ShowFlags & SHOW_UnlitTranslucency )
		{
			SCOPE_CYCLE_COUNTER(STAT_TranslucencyDrawTime);

			UBOOL bTranslucencyDirtiedColor = FALSE;

			if (bWorldDpg)
			{
				TweakMatrices( this, TMO_Sky );
				bTranslucencyDirtiedColor |= RenderTranslucency( SDPG_SkyLayer3 );
				bTranslucencyDirtiedColor |= RenderTranslucency( SDPG_SkyLayer2 );
				bTranslucencyDirtiedColor |= RenderTranslucency( SDPG_SkyLayer1 );
				bTranslucencyDirtiedColor |= RenderTranslucency( SDPG_SkyLayer0 );
				TweakMatrices( this, TMO_Restore );
			}

			// Translucent pass.
			bTranslucencyDirtiedColor |= RenderTranslucency( DPGIndex );			

			// If any translucent elements were drawn, render post-fog decals for translucent receivers.
			if ( bTranslucencyDirtiedColor)
			{
				if (ViewFamily.ShowFlags & SHOW_Decals)
				{
					RenderDecals( DPGIndex, TRUE, FALSE );	
				}

				if (GSupportsFPBlending || GIsLowEndHW )
				{
					// Finish rendering scene color after rendering translucency for this DPG.
					GSceneRenderTargets.FinishRenderingSceneColor( TRUE );
				}
				else
				{
					// Finish rendering the LDR translucency
					GSceneRenderTargets.FinishRenderingSceneColorLDR(FALSE); 

					//blend the LDR translucency onto scene color
					CombineLDRTranslucency(GlobalContext);
				}
			}			
		}	
	
		{
			const UBOOL bDirty = RenderSeeThrough( DPGIndex );

			if (bDirty)
			{
				GSceneRenderTargets.ResolveSceneColor();				
			}			
		}

#if PS3 // GEMINI_TODO: remove
		extern FLOAT GRHIDebug[20];
		if (GRHIDebug[5])
		{
			DumpRenderTarget( GSceneRenderTargets.GetSceneColorSurface() );
//			DumpRenderTarget( GSceneRenderTargets.GetSceneDepthSurface() );
//			DumpRenderTarget( GSceneRenderTargets.GetSceneColorSurface(), 1 );
//			DumpRenderTarget( GSceneRenderTargets.GetSceneDepthSurface(), 1 );
		}
		if (GRHIDebug[6])
		{
			DumpRenderTarget( GSceneRenderTargets.GetLightAttenuationSurface() );
		}
#endif		
		// post process effects pass for scene DPGs
		RenderPostProcessEffects(DPGIndex);
		
		///@ ava Specific ; foreground FOV
		if (DPGIndex == SDPG_Foreground && !(ViewFamily.ShowFlags & (SHOW_Bounds || SHOW_ShadowFrustums)) || ( DPGIndex == SDPG_ForeForeground ))		
		{
			TweakMatrices( this, TMO_Restore );
		}
		//@> ava
	}

	if( GIsLowEndHW )
	{
		RHISetSRGBWriteEnable( NULL, FALSE );
	}


#if PS3 // GEMINI_TODO: remove
	extern FLOAT GRHIDebug[20];
	GRHIDebug[5] = 0;
	GRHIDebug[6] = 0;
#endif

	extern UBOOL GIsTiledScreenshot;
	if (!GIsTiledScreenshot)
	{
		// measure scene luminance
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{	
			MeasureLuminance(&Views(ViewIndex));	
		}
	}
	

	// post process effects pass for post process DPGs
	RenderPostProcessEffects(SDPG_PostProcess);


	//<@ ava specific ; 2007. 4. 18 changmin
	extern UBOOL GDumpHdrScene;
	if( GDumpHdrScene )
	{
		TArray<FLOAT> HdrScene;
		const UINT SizeX = ViewFamily.RenderTarget->GetSizeX();
		const UINT SizeY = ViewFamily.RenderTarget->GetSizeY();
		HdrScene.AddZeroed( SizeX * SizeY * 3 );
		FSurfaceRHIRef& HdrSurface = const_cast<FSurfaceRHIRef&>( GSceneRenderTargets.GetSceneColorSurface() );
		RHIReadHdrSurfaceData( HdrSurface, 0, 0, SizeX-1, SizeY-1, HdrScene );

		FILE *f;
		f = fopen( "SceneDump.float", "wb" );
		if( f )
		{
			fwrite( &HdrScene(0), sizeof( FLOAT ), SizeX * SizeY * 3, f );
			fclose( f );
		}

		GDumpHdrScene = FALSE;
	}
	//>@ ava

	/// For now it's disabled :)
	//if (!GIsLowEndHW)
	// Finish rendering for each view.
	{
		SCOPED_DRAW_EVENT(EventFinish)(DEC_SCENE_ITEMS,TEXT("FinishRendering"));
		
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{	
			SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

			// If the bUseLDRSceneColor flag is set then that means the final post-processing shader has already renderered to
			// the view's render target and that one of the post-processing shaders has performed the gamma correction.
			if (!Views(ViewIndex).bUseLDRSceneColor)
			{
				PostProcessView(&Views(ViewIndex));
			}

#if !FINAL_RELEASE
			// display a message saying we're frozen
			FSceneViewState* ViewState = (FSceneViewState*)Views(ViewIndex).State;
			if (ViewState && ViewState->bIsFrozen)
			{
				// this is a helper class for FCanvas to be able to get screen size
				class FRenderTargetFreeze : public FRenderTarget
				{
				public:
					UINT SizeX, SizeY;
					FRenderTargetFreeze(UINT InSizeX, UINT InSizeY)
						: SizeX(InSizeX), SizeY(InSizeY)
					{}
					UINT GetSizeX() const
					{
						return SizeX;
					};
					UINT GetSizeY() const
					{
						return SizeY;
					};
				} TempRenderTarget(Views(ViewIndex).RenderTargetSizeX, Views(ViewIndex).RenderTargetSizeY);

				// create a temporary FCanvas object with the temp render target
				// so it can get the screen size
				FCanvas Canvas(&TempRenderTarget, NULL);
				DrawString(&Canvas, 10, 130, TEXT("Rendering frozen..."), GEngine->GetSmallFont(), FLinearColor(0.5f,0.5f,0.5f,0.8f));
			}
#endif
		}
	}
	
#if XBOX
	// clear portions of the screen that we never rendered to
	{
		SCOPED_DRAW_EVENT(EventFinishClears)(DEC_SCENE_ITEMS,TEXT("FinishClears"));

		// Set the view family's render target/viewport.
		RHISetRenderTarget(GlobalContext,ViewFamily.RenderTarget->RenderTargetSurfaceRHI,FSurfaceRHIRef());
        // find largest rectangle bounded by all renderedviews
		UINT MinX=ViewFamily.RenderTarget->GetSizeX(),	MinY=ViewFamily.RenderTarget->GetSizeY(), MaxX=0, MaxY=0;
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{	
			const FViewInfo& View = Views(ViewIndex);
			MinX = Min<UINT>(appTrunc(View.X),MinX);
			MinY = Min<UINT>(appTrunc(View.Y),MinY);
			MaxX = Max<UINT>(appTrunc(View.X + View.SizeX),MaxX);
			MaxY = Max<UINT>(appTrunc(View.Y + View.SizeY),MaxY);
		}
		// clear left
		if( MinX > 0 )
		{
            RHISetViewport(GlobalContext,0,0,0.0f,MinX,ViewFamily.RenderTarget->GetSizeY(),1.0f);
			RHIClear(GlobalContext,TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
		}
		// clear right
		if( MaxX < ViewFamily.RenderTarget->GetSizeX() )
		{
			RHISetViewport(GlobalContext,MaxX,0,0.0f,ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),1.0f);
			RHIClear(GlobalContext,TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
		}
		// clear top
		if( MinY > 0 )
		{
			RHISetViewport(GlobalContext,0,0,0.0f,ViewFamily.RenderTarget->GetSizeX(),MinY,1.0f);
			RHIClear(GlobalContext,TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
		}
		// clear bottom
		if( MaxY < ViewFamily.RenderTarget->GetSizeY() )
		{
			RHISetViewport(GlobalContext,0,MaxY,0.0f,ViewFamily.RenderTarget->GetSizeX(),ViewFamily.RenderTarget->GetSizeY(),1.0f);
			RHIClear(GlobalContext,TRUE,FLinearColor::Black,FALSE,0.0f,FALSE,0);
		}
	}
#endif

	if (GIsLowEndHW)
	{
		//GSceneRenderTargets.OverrideSceneColorSurface( FSurfaceRHIRef() );
	}

	// Set the view family's destination render target as the render target.
	RHISetRenderTarget(GlobalContext,ViewFamily.RenderTarget->RenderTargetSurfaceRHI, FSurfaceRHIRef());

	// Save the actor and primitive visibility states for the game thread.
	SaveVisibilityState();

	// Save the post-occlusion visibility stats for the frame and freezing info
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);
		INC_DWORD_STAT_BY(STAT_VisibleStaticMeshElements,View.NumVisibleStaticMeshElements);
		INC_DWORD_STAT_BY(STAT_VisibleDynamicPrimitives,View.NumVisibleDynamicPrimitives);

#if !FINAL_RELEASE
		// update freezing info
		FSceneViewState* ViewState = (FSceneViewState*)View.State;
		if (ViewState)
		{
			// if we're finished freezing, now we are frozen
			if (ViewState->bIsFreezing)
			{
				ViewState->bIsFreezing = FALSE;
				ViewState->bIsFrozen = TRUE;
			}

			// handle freeze toggle request
			if (bHasRequestedToggleFreeze)
			{
				// do we want to start freezing?
				if (!ViewState->bIsFrozen)
				{
					ViewState->bIsFrozen = FALSE;
					ViewState->bIsFreezing = TRUE;
					ViewState->FrozenPrimitives.Empty();
				}
				// or stop?
				else
				{
					ViewState->bIsFrozen = FALSE;
					ViewState->bIsFreezing = FALSE;
					ViewState->FrozenPrimitives.Empty();
				}
			}
		}
#endif
	}

#if !FINAL_RELEASE
	// clear the commands
	bHasRequestedToggleFreeze = FALSE;
#endif
}

/** Renders only the final post processing for the view */
void FSceneRenderer::RenderPostProcessOnly() 
{
	// post process effects passes
	for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
	{
		RenderPostProcessEffects(DPGIndex);
	}	
	RenderPostProcessEffects(SDPG_PostProcess);

	// Finish rendering for each view.
	{
		SCOPED_DRAW_EVENT(EventFinish)(DEC_SCENE_ITEMS,TEXT("FinishRendering"));
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)		
		{	
			SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);
			FinishRenderViewTarget(&Views(ViewIndex));
		}
	}
}

/** Renders the scene's prepass and occlusion queries */
UBOOL FSceneRenderer::RenderPrePass(UINT DPGIndex,UBOOL bIsOcclusionTesting)
{
	SCOPED_DRAW_EVENT(EventPrePass)(DEC_SCENE_ITEMS,TEXT("PrePass"));

	const UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	
	UBOOL bDirty=0;

	if( !GUseTilingCode || !bWorldDpg )
	{
		GSceneRenderTargets.BeginRenderingPrePass( FALSE );
	}	

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);
		const FSceneViewState* ViewState = (FSceneViewState*)View.State;

		// Set the device viewport for the view.
		RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
		RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

		if( GUseTilingCode && bWorldDpg )
		{
			RHIMSAAFixViewport();
		}

		extern UBOOL GIsTiledScreenshot;

		//@GEMINI_TODO: the Editor currently relies on the prepass clearing the depth.
		if( ( GIsEditor && !GIsPlayInEditorWorld_RenderThread ) || bIsOcclusionTesting || (DPGIndex == SDPG_World) || (DPGIndex == SDPG_Foreground) || GIsTiledScreenshot)// || (DPGIndex == SDPG_ForeForeground))
		{
			// Clear the depth buffer as required
			RHIClear(GlobalContext,FALSE,FLinearColor::Black,TRUE,1.0f,TRUE,0);
		}

		// Opaque blending, depth tests and writes.
		RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());
		RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());

		// Draw a depth pass to avoid overdraw in the other passes.
		if(bIsOcclusionTesting)
		{
			// Write the depths of primitives which were unoccluded the preview frame.
			{
#if !FINAL_RELEASE
#define FORCE_RENDER_DEPTH (GUpdatingCullDistances)
#else
#define FORCE_RENDER_DEPTH 0
#endif
				SCOPE_CYCLE_COUNTER(STAT_DepthDrawTime);

				// Build a map from static mesh ID to whether it should be used as an occluder.
				// This TConstSubsetIterator only iterates over the static-meshes which have their bit set in the static-mesh visibility map.
				for(TSparseArray<FStaticMesh*>::TConstSubsetIterator StaticMeshIt(Scene->StaticMeshes,View.StaticMeshVisibilityMap);
					StaticMeshIt;
					++StaticMeshIt
					)
				{
					const FStaticMesh* StaticMesh = *StaticMeshIt;
					if(StaticMesh->PrimitiveSceneInfo->bUseAsOccluder || FORCE_RENDER_DEPTH)
					{
						View.StaticMeshOccluderMap.AccessCorrespondingBit(StaticMeshIt) = TRUE;
					}
				}

				// @AVA --> 저 뒤로 옮깁니다
				// Draw the depth pass for the view.
				/*
				bDirty |= Scene->DPGs[DPGIndex].DepthDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshOccluderMap);
				*/
				
				// color write를 꺼야 합니다~ double speed z~ : 2007. 2. 25 changmin
				// 아래의 BeginOcclusionTests 안에서.. 한번 더 끄고, 나중에 켭니다..
				RHISetColorWriteEnable(GlobalContext,FALSE);

				// Draw the dynamic occluder primitives using a depth drawing policy.
				//<@ ava specific ; 2006. 12. 01 changmin				
				// render using dynamic bsp batcher
				BspRendering_StartBatch(BSP_OVERRIDEMATERIAL);
				//>@ ava

				extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
				extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

				ParticleRendering_StartBatch(FALSE);

				TDynamicPrimitiveDrawer<FDepthDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FDepthDrawingPolicyFactory::ContextType(),TRUE);
				for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
				{
					const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
					const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
					const UPrimitiveComponent* Component = PrimitiveSceneInfo->Component;

					const FMatrix* CheckMatrix = NULL;
					const FMotionBlurInfo* MBInfo;
					if (PrimitiveSceneInfo->Scene->GetPrimitiveMotionBlurInfo(PrimitiveSceneInfo, MBInfo) == TRUE)
					{
						CheckMatrix = &(MBInfo->PreviousLocalToWorld);
					}

					// Don't draw dynamic primitives with depth-only if we're going to render their velocities anyway.
					const UBOOL bHasVelocity = View.bRequiresVelocities
						&& !PrimitiveSceneInfo->bStaticShadowing
						&& (Abs(PrimitiveSceneInfo->Component->MotionBlurScale - 1.0f) > 0.0001f 
						|| (CheckMatrix && (!Component->LocalToWorld.Equals(*CheckMatrix, 0.0001f)))
						);

					if(	!bHasVelocity &&
						PrimitiveViewRelevance.GetDPG(DPGIndex) && 
						(FORCE_RENDER_DEPTH ||
						PrimitiveSceneInfo->bUseAsOccluder
						)
						)						
					{
						Drawer.SetPrimitive(PrimitiveSceneInfo);
						PrimitiveSceneInfo->Proxy->DrawDynamicElements(
							&Drawer,
							&View,
							DPGIndex
							);
					}
				}
				//<@ ava specific ; 2006. 12. 01 changmin
				BspRendering_EndBatch(&Drawer);
				//>@ ava

				ParticleRendering_EndBatch(&Drawer, &View, DPGIndex);

				bDirty |= Drawer.IsDirty();
			}

			// Draw the depth pass for the view.
			bDirty |= Scene->DPGs[DPGIndex].PositionOnlyDepthDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshOccluderMap);
			bDirty |= Scene->DPGs[DPGIndex].DepthDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshOccluderMap);
			
			RHISetColorWriteEnable(GlobalContext,TRUE);

			// Perform occlusion queries for the world DPG.
			{
				SCOPED_DRAW_EVENT(EventBeginOcclude)(DEC_SCENE_ITEMS,TEXT("BeginOcclusionTests View%d"),ViewIndex);
				BeginOcclusionTests(View);
			}
		}
	}
	if( !GUseTilingCode || !bWorldDpg )
	{
		GSceneRenderTargets.FinishRenderingPrePass( FALSE );
	}

	return bDirty;
}

UBOOL GDrawBSP;

/** Renders the scene's emissive pass */
UBOOL FSceneRenderer::RenderEmissive(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventEmissive)(DEC_SCENE_ITEMS,TEXT("Emissive"));

	UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	UBOOL bDirty=0;

	// Opaque blending, depth tests and writes.
	RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());
	//<@ ava specific ; 2006. 9. 21 changmin	

	if (DPGIndex >= SDPG_SkyLayer0 && DPGIndex <= SDPG_SkyLayer3 && GIsGame_RenderThread)
	{
		RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
	}
	else
	{
		RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());			
	}
	//>@ ava

	// Draw the scene's emissive and light-map color.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		EnvCube_ResetCache();

		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);
		SCOPE_CYCLE_COUNTER(STAT_EmissiveDrawTime);

		FViewInfo& View = Views(ViewIndex);

		// It's unused ; 2007. 10. 18 changmin
		//extern UBOOL GExpectingDepthBuffer;
		//GExpectingDepthBuffer = View.bRequiresDepth || (ViewFamily.ShowFlags & SHOW_DynamicShadows) || (ViewFamily.ShowFlags & SHOW_Fog) && Scene->Fogs.Num() > 0;

		// Set the device viewport for the view.
		RHISetViewport(GlobalContext,View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
		RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

		if (GUseTilingCode && bWorldDpg)
		{
			RHIMSAAFixViewport();
		}

		const UBOOL bUtilizingOnePassLighting = (View.Family->ShowFlags & SHOW_Lighting);//( GWorld && GWorld->bDrawEmissiveDynamicPrimitives == FALSE ) && (View.Family->ShowFlags & SHOW_Lighting);

		if (!GDrawBSP || GIsEditor)
		{		

#if !FINAL_RELEASE
			// Draw the scene's emissive draw lists.
			if (!(View.Family->ShowFlags & SHOW_Lighting))
			{
				if (GUpdatingCullDistances)
				{
					bDirty |= Scene->DPGs[DPGIndex].UnlitDrawList.DrawAndCheckVisibility(GlobalContext,&View,View.StaticMeshVisibilityMap,View.FineStaticMeshVisibilityMap);
				}
				else
				{
					bDirty |= Scene->DPGs[DPGIndex].UnlitDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshVisibilityMap);
				}
			}			
			else
#endif
			{
				bDirty |= Scene->DPGs[DPGIndex].EmissiveNoLightMapDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].EmissiveVertexLightMapDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].EmissiveLightMapTextureDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshVisibilityMap);			

#if !FINAL_RELEASE
				// 혹시 모르니까 Emissive pass 다 그린 담에 한 번 더 그리도록 변경
				if (GUpdatingCullDistances)
				{
					bDirty |= Scene->DPGs[DPGIndex].EmissiveNoLightMapDrawList.DrawAndCheckVisibility(GlobalContext,&View,View.StaticMeshVisibilityMap,View.FineStaticMeshVisibilityMap);
					bDirty |= Scene->DPGs[DPGIndex].EmissiveVertexLightMapDrawList.DrawAndCheckVisibility(GlobalContext,&View,View.StaticMeshVisibilityMap,View.FineStaticMeshVisibilityMap);
					bDirty |= Scene->DPGs[DPGIndex].EmissiveLightMapTextureDrawList.DrawAndCheckVisibility(GlobalContext,&View,View.StaticMeshVisibilityMap,View.FineStaticMeshVisibilityMap);
				}
#endif
			}
		}

#if !FINAL_RELEASE
		TArray<FOcclusionQueryRHIRef> Queries;
		if (GUpdatingCullDistances)
		{
			View.FineDynamicMeshVisibilityMap = FBitArray(FALSE,View.NumVisibleDynamicPrimitives);
			GOcclusionQueryPool.Reset();
		}
#endif


		//<@ ava specific ; 2006. 12. 01 changmin
		// bsp rendering
		BspRendering_StartBatch(BSP_EMISSIVE);

		extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
		extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

		ParticleRendering_StartBatch(FALSE);
		//>@ava

		// Draw the dynamic non-occluded primitives using an emissive drawing policy.
		TDynamicPrimitiveDrawer<FEmissiveDrawingPolicyFactory> Drawer(GlobalContext,&View,DPGIndex,FEmissiveDrawingPolicyFactory::ContextType(),TRUE);
		for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);

			//<@ ava specific ; 2006 / 9 /  5 changmin
			// Emissive pass 에 dynamic primitive를 그리지 않아도, modulate shadow 가 적용되는 dynamic pritmive의 render 결과는 변하지 않는다.
			// 여기서 제외하면 얼마나 빨라질까... 하는 테스트 중
			const UBOOL bOnePassPrimitive = bUtilizingOnePassLighting && PrimitiveSceneInfo->LightEnvironmentSceneInfo && (GIsGame_RenderThread || PrimitiveSceneInfo->LightEnvironmentSceneInfo->Component->bHasLighting);
			if( bOnePassPrimitive )
			{
				continue;
			}
			//>@ ava

			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
			const UBOOL bRelevantDPG = PrimitiveViewRelevance.GetDPG(DPGIndex) != 0;

			// Only draw the primitive if it's visible and relevant in the current DPG
			if(bVisible && bRelevantDPG)
			{
				if (!GIsEditor)
				{
					UBOOL bIsBSP = PrimitiveSceneInfo->Component->IsA( UModelComponent::StaticClass() );

					if (GDrawBSP ^ bIsBSP) continue;
				}

#if !FINAL_RELEASE
				FOcclusionQueryRHIRef Query;
				
				if (GUpdatingCullDistances && !View.FineDynamicMeshVisibilityMap(PrimitiveIndex))
				{
					Query = GOcclusionQueryPool.Allocate();
					RHIBeginOcclusionQuery(GlobalContext,Query);
				}

				Queries.AddItem(Query);
#endif

				Drawer.SetPrimitive(PrimitiveSceneInfo);
				PrimitiveSceneInfo->Proxy->DrawDynamicElements(
					&Drawer,
					&View,
					DPGIndex
					);

#if !FINAL_RELEASE
				if (GIsEditor && GUpdatingCullDistances)
				{
					UBOOL bIsBSP = PrimitiveSceneInfo->Component->IsA( UModelComponent::StaticClass() );

					if (bIsBSP)
					{
						BspRendering_EndBatch(&Drawer);
						BspRendering_StartBatch(BSP_EMISSIVE);
					}
				}

				if (GUpdatingCullDistances && IsValidRef(Query))
				{
					RHIEndOcclusionQuery(GlobalContext,Query);
				}
#endif
			}
		}

		//<@ ava specific ; 2006. 12. 01. changmin
		BspRendering_EndBatch(&Drawer);

		ParticleRendering_EndBatch(&Drawer, &View, DPGIndex);
		//>@ ava		

#if !FINAL_RELEASE
		if (GUpdatingCullDistances)
		{						
			INT QueryIndex=0;
			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);

				//<@ ava specific ; 2006 / 9 /  5 changmin
				// Emissive pass 에 dynamic primitive를 그리지 않아도, modulate shadow 가 적용되는 dynamic pritmive의 render 결과는 변하지 않는다.
				// 여기서 제외하면 얼마나 빨라질까... 하는 테스트 중
				const UBOOL bOnePassPrimitive = bUtilizingOnePassLighting && PrimitiveSceneInfo->LightEnvironmentSceneInfo && (GIsGame_RenderThread || PrimitiveSceneInfo->LightEnvironmentSceneInfo->Component->bHasLighting);
				if( bOnePassPrimitive )
				{
					continue;
				}
				//>@ ava

				const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

				const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
				const UBOOL bRelevantDPG = PrimitiveViewRelevance.GetDPG(DPGIndex) != 0;

				// Only draw the primitive if it's visible and relevant in the current DPG
				if(bVisible && bRelevantDPG)
				{
					if (!GIsEditor)
					{
						UBOOL bIsBSP = PrimitiveSceneInfo->Component->IsA( UModelComponent::StaticClass() );

						if (GDrawBSP ^ bIsBSP) continue;
					}

					FOcclusionQueryRHIRef Query = Queries(QueryIndex++);

					DWORD OutNumPixels;

					if (IsValidRef( Query ) && RHIGetOcclusionQueryResult(Query,OutNumPixels,TRUE))
					{
						if (OutNumPixels>0)
						{
							View.FineDynamicMeshVisibilityMap(PrimitiveIndex) = TRUE;
						}						
					}
				}
			}
		}
#endif

#if !FINAL_RELEASE
		if (GUpdatingCullDistances && GIsPlayInEditorWorld_RenderThread && bWorldDpg)
		{
			extern void UpdateCullDistance( const UPrimitiveComponent* Primitive, const FVector4& ViewOrigin );					

			for (INT MeshIndex=0; MeshIndex<Scene->StaticMeshes.Num(); ++MeshIndex)
			{
				if (View.FineStaticMeshVisibilityMap(MeshIndex))
				{
					UpdateCullDistance( Scene->StaticMeshes(MeshIndex)->PrimitiveSceneInfo->Component, View.ViewOrigin );
				}
			}

			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
			{
				if (View.FineDynamicMeshVisibilityMap(PrimitiveIndex))
				{
					const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);

					UpdateCullDistance( PrimitiveSceneInfo->Component, View.ViewOrigin );
				}
			}
		}		
#endif

		extern UBOOL GUseCascadedShadow;
		if( !(GUseCascadedShadow && Scene->bHasSunLightShadow) )	// cascaded lighting을 안하면 decal을 이전처럼...
		{
			// Draw decals for non-occluded primitives using an emissive drawing policy.

			// TRUE if rendering decals modified the depth state.
			UBOOL bDecalDepthStateIsSet = FALSE;

			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleLitDecalPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleLitDecalPrimitives(PrimitiveIndex);
				const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

				const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
				const UBOOL bRelevantDPG = PrimitiveViewRelevance.GetDPG(DPGIndex) != 0;

				// Only draw decals if the primitive if it's visible and relevant in the current DPG
				if(bVisible && bRelevantDPG)
				{
					if (!GIsEditor)
					{
						UBOOL bIsBSP = PrimitiveSceneInfo->Component->IsA( UModelComponent::StaticClass() );

						if (GDrawBSP ^ bIsBSP) continue;
					}

					// Set the depth state for decals.
					if ( !bDecalDepthStateIsSet )
					{
						RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
						bDecalDepthStateIsSet = TRUE;
					}

					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawLitDecalElements(
						GlobalContext,
						&Drawer,
						&View,
						DPGIndex,
						FALSE
						);
				}
			}

			// Restore depth state if decals were rendered.
			if ( bDecalDepthStateIsSet )
			{
				RHISetDepthState(GlobalContext,TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
			}
		}


		bDirty |= Drawer.IsDirty(); 


		if (!GDrawBSP || GIsEditor)
		{

		

		// Draw the emissive pass for the view's batched mesh elements.
		bDirty |= DrawViewElements<FEmissiveDrawingPolicyFactory>(GlobalContext,&View,FEmissiveDrawingPolicyFactory::ContextType(),DPGIndex,TRUE);

		// Draw the view's batched simple elements(lines, sprites, etc).
		bDirty |= View.BatchedViewElements[DPGIndex].Draw(GlobalContext,View.ViewProjectionMatrix,appTrunc(View.SizeX),appTrunc(View.SizeY),FALSE);

#if XBOX
		if( DPGIndex == SDPG_World )
		{
			// Hack to render foreground shadows onto the world when using modulated shadows.
			// We're rendering a depth-only pass for dynamic foreground prims. Then, during the shadow projection
			// pass, we have both world and resolved foreground depth values so that shadows will be projected on the
			// foreground prims as well as the world.

			SCOPED_DRAW_EVENT(EventForegroundHack)(DEC_SCENE_ITEMS,TEXT("Foreground Depths"));

			RHISetColorWriteEnable(GlobalContext,FALSE);

			bDirty |= Scene->DPGs[SDPG_Foreground].EmissiveNoLightMapDrawList.DrawVisible(GlobalContext,&View,View.StaticMeshVisibilityMap);

			// using FEmissiveDrawingPolicy here instead of FDepthOnlyDrawingPolicy since masked materials are not handled by the depth only policy
			TDynamicPrimitiveDrawer<FEmissiveDrawingPolicyFactory> ForegroundDrawer(GlobalContext,&View,SDPG_Foreground,FEmissiveDrawingPolicyFactory::ContextType(),TRUE);
			for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
			{
				const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
				const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);
				const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);

				if(	bVisible && 
					PrimitiveViewRelevance.GetDPG(SDPG_Foreground) )
				{
					ForegroundDrawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicElements(
						&ForegroundDrawer,
						&View,
						SDPG_Foreground
						);
				}
			}

			RHISetColorWriteEnable(GlobalContext,TRUE);

			bDirty |= ForegroundDrawer.IsDirty();
		}
#endif //XBOX
		}

	}	

	return bDirty;
}

/** 
* Renders the post process effects for a view. 
* @param DPGIndex - current depth priority group (DPG)
*/
void FSceneRenderer::RenderPostProcessEffects(UINT DPGIndex)
{
	// For now it's disabled :)
	//if (GIsLowEndHW) 
	//	return;

	SCOPED_DRAW_EVENT(EventPP)(DEC_SCENE_ITEMS,TEXT("PostProcessEffects"));

	UBOOL bSetAllocations = FALSE;

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_DRAW_EVENT(EventView)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);
		RHISetViewParameters( GlobalContext, &View, View.ViewProjectionMatrix, View.ViewOrigin );

		// render any custom post process effects
		for( INT EffectIdx=0; EffectIdx < View.PostProcessSceneProxies.Num(); EffectIdx++ )
		{
			FPostProcessSceneProxy* PPEffectProxy = &View.PostProcessSceneProxies(EffectIdx);
			if( PPEffectProxy && 
				PPEffectProxy->GetDepthPriorityGroup() == DPGIndex )
			{
				if (!bSetAllocations)
				{
					// allocate more GPRs for pixel shaders
					RHISetShaderRegisterAllocation(32, 96);
					bSetAllocations = TRUE;
				}

				// render the effect
				PPEffectProxy->Render(GlobalContext, DPGIndex,View,CanvasTransform);
			}
		}
	}

	if (bSetAllocations)
	{
		// restore default GPR allocation
		RHISetShaderRegisterAllocation(64, 64);
	}
}

/**
* Clears the scene color depth (stored in alpha channel) to max depth
* This is needed for depth bias blend materials to show up correctly
*/
void FSceneRenderer::ClearSceneColorDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("Clear Scene Color Depth"));

	const FLinearColor ClearDepthColor(0,0,0,65530);
	
	FBatchedElements BatchedElements;
	INT V00 = BatchedElements.AddVertex(FVector4(-1,-1,0,1),FVector2D(0,0),ClearDepthColor,FHitProxyId());
	INT V10 = BatchedElements.AddVertex(FVector4(1,-1,0,1),FVector2D(1,0),ClearDepthColor,FHitProxyId());
	INT V01 = BatchedElements.AddVertex(FVector4(-1,1,0,1),FVector2D(0,1),ClearDepthColor,FHitProxyId());
	INT V11 = BatchedElements.AddVertex(FVector4(1,1,0,1),FVector2D(1,1),ClearDepthColor,FHitProxyId());

	// No alpha blending, no depth tests or writes, no backface culling.
	RHISetBlendState(GlobalContext,TStaticBlendState<>::GetRHI());
	RHISetDepthState(GlobalContext,TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(GlobalContext,TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	RHISetColorWriteMask(GlobalContext,CW_ALPHA);

	// Draw a quad using the generated vertices.
	BatchedElements.AddTriangle(V00,V10,V11,GWhiteTexture,BLEND_Opaque);
	BatchedElements.AddTriangle(V00,V11,V01,GWhiteTexture,BLEND_Opaque);
	BatchedElements.Draw(
		GlobalContext,
		FMatrix::Identity,
		ViewFamily.RenderTarget->GetSizeX(),
		ViewFamily.RenderTarget->GetSizeY(),
		FALSE
		);

	RHISetColorWriteMask(GlobalContext,CW_RGBA);
}

/** 
* Renders the scene to capture target textures 
*/
void FSceneRenderer::RenderSceneCaptures()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("Scene Captures"));

	// disable tiling for rendering captures
	UBOOL SavedUseTilingCode = GUseTilingCode;
	GUseTilingCode = FALSE;	
	
	for( TSparseArray<FCaptureSceneInfo*>::TConstIterator CaptureIt(Scene->SceneCaptures); CaptureIt; ++CaptureIt )
	{
		FCaptureSceneInfo* CaptureInfo = *CaptureIt;
        CaptureInfo->CaptureScene(this);
	}

	// restore tiling setting
	GUseTilingCode = SavedUseTilingCode;
}

/** Updates the game-thread actor and primitive visibility states. */
void FSceneRenderer::SaveVisibilityState()
{
	class FActorVisibilitySet : public FActorVisibilityHistoryInterface
	{
	public:

		/**
		* Adds an actor to the visibility set.  Ensures that duplicates are not added.
		* @param VisibleActor - The actor which is visible.
		*/
		void AddActor(const AActor* VisibleActor)
		{
			if(!VisibleActors.Find(VisibleActor))
			{
				VisibleActors.Set(VisibleActor,TRUE);
			}
		}

		// FActorVisibilityHistoryInterface
		virtual UBOOL GetActorVisibility(const AActor* TestActor) const
		{
			return VisibleActors.Find(TestActor) != NULL;
		}

	private:

		// The set of visible actors.
		TMap<const AActor*,UBOOL> VisibleActors;
	};

	// Update LastRenderTime for the primitives which were visible.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);

		// Allocate an actor visibility set for the actor.
		FActorVisibilitySet* ActorVisibilitySet = new FActorVisibilitySet;

		// check if we are freezing the frame
		FSceneViewState* ViewState = (FSceneViewState*)View.State;
#if !FINAL_RELEASE
		UBOOL bIsFreezing = ViewState && ViewState->bIsFreezing;
#endif

		// Iterate over the visible primitives.
		for(TSparseArray<FPrimitiveSceneInfoCompact>::TConstSubsetIterator PrimitiveIt(Scene->Primitives,View.PrimitiveVisibilityMap);
			PrimitiveIt;
			++PrimitiveIt
			)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = PrimitiveIt->PrimitiveSceneInfo;

			// When using latent occlusion culling, this check will indicate the primitive is occluded unless it has been unoccluded the past two frames.
			// This ignores the first frame a primitive becomes visible without occlusion information and waits until the occlusion query results are available
			// the next frame before signaling to the game systems that the primitive is visible.			
			const UBOOL bWasOccluded = ViewState && ViewState->WasPrimitivePreviouslyOccluded(PrimitiveIt->Component,ViewFamily.CurrentRealTime);
			if(!bWasOccluded)
			{
				PrimitiveIt->Component->LastRenderTime = ViewFamily.CurrentWorldTime;
				if(PrimitiveSceneInfo->Owner)
				{
					ActorVisibilitySet->AddActor(PrimitiveSceneInfo->Owner);
					PrimitiveSceneInfo->Owner->LastRenderTime = ViewFamily.CurrentWorldTime;
				}
				if(PrimitiveSceneInfo->LightEnvironmentSceneInfo)
				{
					PrimitiveSceneInfo->LightEnvironmentSceneInfo->Component->LastRenderTime = ViewFamily.CurrentWorldTime;
				}

#if !FINAL_RELEASE
				// if we are freezing the scene, then remember the primitive was rendered
				if (bIsFreezing)
				{
					ViewState->FrozenPrimitives.Add(PrimitiveIt->Component);
				}
#endif
			}
		}
		
		// Update the view's actor visibility history with the new visibility set.
		if(View.ActorVisibilityHistory)
		{
			View.ActorVisibilityHistory->SetStates(ActorVisibilitySet);
		}
		else
		{
			delete ActorVisibilitySet;
		}
	}
}

/** 
* Global state shared by all FSceneRender instances 
* @return global state
*/
FGlobalSceneRenderState* FSceneRenderer::GetGlobalSceneRenderState()
{
	static FGlobalSceneRenderState GlobalSceneRenderState;
	return &GlobalSceneRenderState;
}


/*-----------------------------------------------------------------------------
BeginRenderingViewFamily
-----------------------------------------------------------------------------*/

/**
 * Helper function performing actual work in render thread.
 *
 * @param SceneRenderer	Scene renderer to use for rendering.
 */
static void RenderViewFamily_RenderThread( FSceneRenderer* SceneRenderer )
{
	{
		SCOPE_CYCLE_COUNTER(STAT_TotalSceneRenderingTime);

		SceneRenderer->GlobalContext = RHIGetGlobalContext();

		// keep track of global frame number
		SceneRenderer->GetGlobalSceneRenderState()->FrameNumber++;

		// Commit the scene's pending light attachments.
		SceneRenderer->Scene->CommitPendingLightAttachments();

		if(SceneRenderer->ViewFamily.ShowFlags & SHOW_HitProxies)
		{
			// Render the scene's hit proxies.
			SceneRenderer->RenderHitProxies();
		}
		else
		{
			if(SceneRenderer->ViewFamily.ShowFlags & SHOW_SceneCaptureUpdates)
			{
				// Render the scene for each capture
				SceneRenderer->RenderSceneCaptures();
			}

			// Render the scene.
			SceneRenderer->Render();
		}

		// Delete the scene renderer.
		delete SceneRenderer;
	}

#if STATS && CONSOLE
	/** Update STATS with the total GPU time taken to render the last frame. */
	SET_CYCLE_COUNTER(STAT_TotalGPUFrameTime, RHIGetGPUFrameCycles(), 1);
#endif
}

void BeginRenderingViewFamily(FCanvas* Canvas,const FSceneViewFamily* ViewFamily)
{
	// Enforce the editor only show flags restrictions.
	check(GIsEditor || !(ViewFamily->ShowFlags & SHOW_EditorOnly_Mask));

	// OnPreRender 등의 outside tick에서도 particle 관련 operation이 있을 수 있어서 옮김
	GParticleDataManager.UpdateDynamicData();

	// Flush the canvas first.
	Canvas->Flush();

	if( ViewFamily->Scene )
	{
		// Set the world's "needs full lighting rebuild" flag if the scene has any uncached static lighting interactions.
		FScene* const Scene = (FScene*)ViewFamily->Scene;
		UWorld* const World = Scene->GetWorld();
		if(World)
		{
			World->GetWorldInfo()->SetMapNeedsLightingFullyRebuilt(Scene->NumUncachedStaticLightingInteractions > 0);
		}

		/* EnvCube list update */
		EnvCube_Update();	

#if GEMINI_TODO
		// We need to pass the scene's hit proxies through to the hit proxy consumer!
		// Otherwise its possible the hit proxy consumer will cache a hit proxy ID it doesn't have a reference for.
		// Note that the effects of not doing this correctly are minor: clicking on a primitive that moved without invalidating the viewport's
		// cached hit proxies won't work.  Is this worth the pain?
#endif

		// Construct the scene renderer.  This copies the view family attributes into its own structures.
		FSceneRenderer* SceneRenderer = ::new FSceneRenderer(ViewFamily,Canvas->GetHitProxyConsumer(),Canvas->GetFullTransform());

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			FDrawSceneCommand,
			FSceneRenderer*,SceneRenderer,SceneRenderer,
		{
			RenderViewFamily_RenderThread(SceneRenderer);
		});
	}
	else
	{
		// Construct the scene renderer.  This copies the view family attributes into its own structures.
		FSceneRenderer* SceneRenderer = ::new FSceneRenderer(ViewFamily,Canvas->GetHitProxyConsumer(),Canvas->GetFullTransform());

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			FDrawSceneCommandPP,
			FSceneRenderer*,SceneRenderer,SceneRenderer,
		{
			SceneRenderer->GlobalContext = RHIGetGlobalContext();
			SceneRenderer->RenderPostProcessOnly();
			delete SceneRenderer;
		});
	}

	// We need to flush rendering commands if stats gathering is enabled to ensure that the stats are valid/ captured
	// before this function returns.
	if( ViewFamily->DynamicShadowStats )
	{
		FlushRenderingCommands();
	}
}

/*-----------------------------------------------------------------------------
Stat declarations.
-----------------------------------------------------------------------------*/

DECLARE_STATS_GROUP(TEXT("SceneRendering"),STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Occlusion query time"),STAT_OcclusionQueryTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Occlusion stall time"),STAT_OcclusionStallTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Occlusion update time"),STAT_OcclusionUpdateTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("InitViews time"),STAT_InitViewsTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Dynamic shadow setup time"),STAT_DynamicShadowSetupTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Total CPU rendering time"),STAT_TotalSceneRenderingTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Total GPU rendering time"),STAT_TotalGPUFrameTime,STATGROUP_SceneRendering);

DECLARE_CYCLE_STAT(TEXT("Depth DrawShared time"),STAT_DepthDrawSharedTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Emissive DrawShared time"),STAT_EmissiveDrawSharedTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Lighting DrawShared time"),STAT_LightingDrawSharedTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Shadow DrawShared time"),STAT_ShadowDrawSharedTime,STATGROUP_SceneRendering);

DECLARE_CYCLE_STAT(TEXT("Depth drawing time"),STAT_DepthDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Emissive drawing time"),STAT_EmissiveDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Shadow volume drawing time"),STAT_ShadowVolumeDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Light function drawing time"),STAT_LightFunctionDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Lighting drawing time"),STAT_LightingDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Lighting dynamic drawing time"),STAT_LightingDynamicDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Shadow drawing time"),STAT_ShadowDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Translucency drawing time"),STAT_TranslucencyDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("BSP add time"),STAT_BSPAddTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("BSP flushing time"),STAT_BSPFlush,STATGROUP_SceneRendering);

DECLARE_DWORD_COUNTER_STAT(TEXT("Occluded primitives"),STAT_OccludedPrimitives,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Occlusion queries"),STAT_OcclusionQueries,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Projected shadows"),STAT_ProjectedShadows,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible static mesh elements"),STAT_VisibleStaticMeshElements,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible dynamic primitives"),STAT_VisibleDynamicPrimitives,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Rejected visibility"),STAT_RejectedVisibility,STATGROUP_SceneRendering);

//<@ ava sepcific ; 2008. 1. 11 changmin
// add cascaded shadow
DECLARE_CYCLE_STAT(TEXT("ccs creation time"),STAT_AVA_CascadedShadow_Create, STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("ccs bsp depth drawing time"),STAT_AVA_CascadedShadow_BspRenderDepth, STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("ccs other drawing time"),STAT_AVA_CascadedShadow_OtherRenderDepth, STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("ccs projection time"),STAT_AVA_CascadedShadow_RenderProjection, STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("ccs lighting time"),STAT_AVA_CascadedShadow_Lighting, STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("ccs occlusion query time"), STAT_AVA_CascadedShadow_OcclusionQueryTime, STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("ccs occlusion update time"), STAT_AVA_CascadedShadow_OcclusionUpdateTime, STATGROUP_SceneRendering);

DECLARE_DWORD_COUNTER_STAT(TEXT("ccs casters"), STAT_AVA_CascadedShadow_CasterCount, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("ccs occlusion queries"),STAT_AVA_CascadedShadow_OcclusionQueries,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("ccs occluded shadow"), STAT_AVA_CascadedShadow_OccludedShadow, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster near bsp count"), STAT_AVA_CascadedShadow_NearBspCasters, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster far bsp count"), STAT_AVA_CascadedShadow_FarBspCasters, STATGROUP_SceneRendering);

DECLARE_DWORD_COUNTER_STAT(TEXT("caster0 count"), STAT_AVA_CascadedShadow_CasterCount0, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster1 count"), STAT_AVA_CascadedShadow_CasterCount1, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster2 count"), STAT_AVA_CascadedShadow_CasterCount2, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster3 count"), STAT_AVA_CascadedShadow_CasterCount3, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster4 count"), STAT_AVA_CascadedShadow_CasterCount4, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster5 count"), STAT_AVA_CascadedShadow_CasterCount5, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster6 count"), STAT_AVA_CascadedShadow_CasterCount6, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("caster7 count"), STAT_AVA_CascadedShadow_CasterCount7, STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("ccs all caster count"), STAT_AVA_CascadedShadow_CasterCountSum, STATGROUP_SceneRendering);
//>@ ava
