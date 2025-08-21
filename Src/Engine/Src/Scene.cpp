/*=============================================================================
	Scene.cpp: Scene manager implementation.
	Copyright 2003-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "AvaColorConversion.h"

#define TRACK_GAME_THREAD_RENDER_MALLOCS		0

#if TRACK_GAME_THREAD_RENDER_MALLOCS
DECLARE_DWORD_COUNTER_STAT( TEXT( "Mallocs during proxy creation" ), STAT_GameToRendererMalloc, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Frees during proxy creation" ), STAT_GameToRendererFree, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Net memory during proxy creation" ), STAT_GameToRendererNet, STATGROUP_Memory );
#endif

DECLARE_DWORD_COUNTER_STAT( TEXT( "Proxy Total" ), STAT_GameToRendererMallocTotal, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Proxy FParticleSystemSceneProxy" ), STAT_GameToRendererMallocPSSP, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Proxy FSkeletalMeshSceneProxy" ), STAT_GameToRendererMallocSkMSP, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Proxy FStaticMeshSceneProxy" ), STAT_GameToRendererMallocStMSP, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Proxy FLensFlareSceneProxy" ), STAT_GameToRendererMallocLFSP, STATGROUP_Memory );
DECLARE_DWORD_COUNTER_STAT( TEXT( "Proxy Other" ), STAT_GameToRendererMallocOther, STATGROUP_Memory );

IMPLEMENT_CLASS(UScene);

void CopyExposureData( FSceneViewStateInterface* InDest, FSceneViewStateInterface* InSrc )
{
	FSceneViewState* Dest = (FSceneViewState*)InDest;
	FSceneViewState* Src  = (FSceneViewState*)InSrc;

	Dest->ExposureData.Copy( Src->ExposureData );
}

FExposureData::FExposureData()
{
	bUseDynamicTonemapping = TRUE;

	ManualBrightnessScale = 1.0f;
	LastRenderTime = 0.0f;
	
	bDisplayLuminanceHistogram = FALSE;

	ExposureCenterRegionX = 0.75f;
	ExposureCenterRegionY = 0.80f;

	GConfig->GetBool(TEXT("Engine.Exposure"), TEXT("bDisplayLuminanceHistogram"), bDisplayLuminanceHistogram, GEngineIni );		
}

void FExposureData::Copy( const FExposureData& Src )
{
	ManualBrightnessScale = Src.ManualBrightnessScale;
	LastRenderTime = Src.LastRenderTime;	
	bUseDynamicTonemapping = Src.bUseDynamicTonemapping;		

	LuminanceHistogram.Copy( Src.LuminanceHistogram );
}

void FExposureData::Measure( FCommandContextRHI* Context, const FViewInfo* View )
{	
	if (View->Family->ShowFlags & SHOW_Lighting || View->Family->ShowFlags & SHOW_BumpLighting )
	{
		if (!(View->Family->ShowFlags & SHOW_ViewSpaceNormal) && !View->bTransparentBackground && bUseDynamicTonemapping)		
		{
			LuminanceHistogram.Measure( Context, View, ExposureCenterRegionX, ExposureCenterRegionY );
		}	
	}
}

void FExposureData::Update( FCommandContextRHI* Context, const FViewInfo* View )
{	
	if (View->Family->ShowFlags & SHOW_Lighting || View->Family->ShowFlags & SHOW_BumpLighting )
	{
		if (!(View->Family->ShowFlags & SHOW_ViewSpaceNormal) && !View->bTransparentBackground && bUseDynamicTonemapping)		
		{
			FLOAT BloomStart = 0.5f;

			if (View->PostProcessSettings)
			{
				BloomStart = View->PostProcessSettings->Bloom_Threshold;
			}
			
			LuminanceHistogram.Update( View->Family->CurrentRealTime, View->ColorScale.R, BloomStart, View->Family->ShowFlags & SHOW_BumpLighting );		
		}	
	}
}

void FExposureData::DisplayDebugInfo( FCanvas* Canvas, INT& Y )
{
	if( GIsGame_RenderThread && bDisplayLuminanceHistogram || !GIsGame_RenderThread && GWorld && GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->bDisplayTonemapInfo)
	{
		LuminanceHistogram.DisplayHistogram( Canvas, Y );
		DrawString( Canvas, 10, Y, *FString::Printf( TEXT("Tonemap scale     : %.4f(%.4f, %.4f)"), LuminanceHistogram.ScaleFactor, LuminanceHistogram.MinScale, LuminanceHistogram.MaxScale ), GEngine->TinyFont, FColor(255,255,255) );
		Y += 20;
		DrawString( Canvas, 10, Y, *FString::Printf( TEXT("Current Key Value : %.4f"), LuminanceHistogram.KeyValue ), GEngine->TinyFont, FColor(255, 255, 255) );
		Y += 20;
	}
}

FExposureData::~FExposureData()
{	
}


FSynchronizedActorVisibilityHistory::FSynchronizedActorVisibilityHistory():
	States(NULL),
	CriticalSection(NULL)
{
}

FSynchronizedActorVisibilityHistory::~FSynchronizedActorVisibilityHistory()
{
	if(CriticalSection)
	{
		GSynchronizeFactory->Destroy(CriticalSection);
		CriticalSection = NULL;
	}
	delete States;
}

void FSynchronizedActorVisibilityHistory::Init()
{
	CriticalSection = GSynchronizeFactory->CreateCriticalSection();
}

UBOOL FSynchronizedActorVisibilityHistory::GetActorVisibility(const AActor* Actor) const
{
	FScopeLock Lock(CriticalSection);
	return States ? States->GetActorVisibility(Actor) : NULL;
}

void FSynchronizedActorVisibilityHistory::SetStates(FActorVisibilityHistoryInterface* NewStates)
{
	FScopeLock Lock(CriticalSection);
	delete States;
	States = NewStates;
}



/**
 * Utility function to create the inverse depth projection transform to be used
 * by the shader system.
 * @param ProjectionMatrix - used to extract the scene depth ratios
 * @param InvertZ - projection calc is affected by inverted device Z
 * @return vector containting the ratios needed to convert from device Z to world Z
 */
FVector4 CreateInvDeviceZToWorldZTransform(FMatrix const & ProjectionMatrix, UBOOL InvertZ)
{
	// The depth projection comes from the the following projection matrix:
	//
	// | 1  0  0  0 |
	// | 0  1  0  0 |
	// | 0  0  A  1 |
	// | 0  0  B  0 |
	//
	// Z' = (Z * A + B) / Z
	// Z' = A + B / Z
	//
	// So to get Z from Z' is just:
	// Z = B / (Z' - A)
	//
	// The old shader code uses:
	// Z = C1 / (1 - (Z' / C2))	--- Where C1 = -B / A, C2 = A
	//
	// The new shader code uses:
	// Z = 1 / (Z' * C1 - C2)   --- Where C1 = 1/B, C2 = A/B
	//

	FLOAT DepthMul = ProjectionMatrix.M[2][2];
	FLOAT DepthAdd = ProjectionMatrix.M[3][2];

	// Adjust the values for an inverted Z buffer

	if (InvertZ)
	{
		DepthMul = 1.0f - DepthMul;
		DepthAdd = - DepthAdd;
	}

	return FVector4(
		- DepthAdd / DepthMul,		// Old projection method
		DepthMul,
		1.f / DepthAdd,				// New projection method
		DepthMul / DepthAdd
		);
}

FSceneView::FSceneView(
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
	const FMatrix& InForegroundProjectionMatrix,	
	FLOAT InForegroundFOV,
	const FLinearColor& InBackgroundColor,
	const FLinearColor& InOverlayColor,
	const FLinearColor& InColorScale,
	const TArray<FPrimitiveSceneInfo*>& InHiddenPrimitives,
	UBOOL InTransparentBackground /* = FALSE */,
	FLOAT InLODDistanceFactor,
	FLOAT InExposureCenterRegionX,
	FLOAT InExposureCenterRegionY,
	FLOAT InOpacity
	):
	Family(InFamily),
	State(InState),
	ActorVisibilityHistory(InActorVisibilityHistory),
	ViewActor(InViewActor),
	PostProcessChain(InPostProcessChain),
	PostProcessSettings(InPostProcessSettings),
	Drawer(InDrawer),
	X(InX),
	Y(InY),
	SizeX(InSizeX),
	SizeY(InSizeY),
	ViewMatrix(InViewMatrix),
	ProjectionMatrix(InProjectionMatrix),
	BackgroundColor(InBackgroundColor),
	OverlayColor(InOverlayColor),
	ColorScale(InColorScale),
	HiddenPrimitives(InHiddenPrimitives),
	ForegroundProjectionMatrix(InForegroundProjectionMatrix),
	ForegroundFOV(InForegroundFOV),
	bTransparentBackground(InTransparentBackground),
	LODDistanceFactor(InLODDistanceFactor),
	ExposureCenterRegionX(InExposureCenterRegionX),
	ExposureCenterRegionY(InExposureCenterRegionY),
	Opacity( InOpacity ),
	SeeThroughGroupMask( 0 ),
	SeeThroughSilhouetteColor( 1, 1, 1, 1 )
{
	check(SizeX > 0.0f);
	check(SizeY > 0.0f);
	
	SavedViewMatrix = ViewMatrix;
	SavedProjectionMatrix = ProjectionMatrix;	

	CalculateDerivedMatrices();	

	// Derive the view frustum from the view projection matrix.
	ViewFrustum = GetViewFrustumBounds(ViewProjectionMatrix,FALSE);

	// Derive the view's near clipping distance and plane.
	bHasNearClippingPlane = ViewProjectionMatrix.GetFrustumNearPlane(NearClippingPlane);
	NearClippingDistance = Abs(ProjectionMatrix.M[2][2]) > DELTA ? -ProjectionMatrix.M[3][2] / ProjectionMatrix.M[2][2] : 0.0f;

	

	// Setup transformation constants to be used by the graphics hardware to transform device normalized depth samples
	// into world oriented z.
	InvDeviceZToWorldZTransform = CreateInvDeviceZToWorldZTransform(ProjectionMatrix, GUsesInvertedZ);

	// Init to invalid values
	RenderTargetX = -1;
	RenderTargetY = -1;
	RenderTargetSizeX = -1;
	RenderTargetSizeY = -1;
	ScreenPositionScaleBias = FVector4(-1);

	if (State)
	{
		FSceneViewState* ViewState = (FSceneViewState*)State;	
	
		if ( GWorld && PostProcessSettings )
		{
			FColorCorrectionResource* ResourceToApply = NULL;
			UBOOL bSucceeded = FALSE;
			INT RequestedColorCorrection = PostProcessSettings->ColorCorrection_Index;
			if ( RequestedColorCorrection >= 0 && RequestedColorCorrection < GWorld->GetWorldInfo()->ColorCorrections.Num() )
			{
				FColorCorrectionData& ColorCorrection = GWorld->GetWorldInfo()->ColorCorrections(PostProcessSettings->ColorCorrection_Index);

				if (!ColorCorrection.Resource)
				{
					ColorCorrection.UpdateResource();
				}

				ResourceToApply = ColorCorrection.Resource;
			}

			ViewState->SetColorCorrection( GWorld->GetDeltaSeconds(), ResourceToApply, PostProcessSettings->ColorCorrection_InterpolationDuration );
		}
		else
		{
			ViewState->SetColorCorrection( GWorld->GetDeltaSeconds(), NULL, 0.0f );
		}
	}
}

void FSceneView::CalculateDerivedMatrices()
{
	// Compute the view projection matrx and its inverse.
	ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
	// For precision reasons the inverse is calculated independently.
	InvViewProjectionMatrix = ProjectionMatrix.Inverse() * ViewMatrix.Inverse();

	// Calculate the view origin from the view/projection matrices.
	if(ProjectionMatrix.M[3][3] < 1.0f)
	{
		ViewOrigin = FVector4(ViewMatrix.Inverse().GetOrigin(),1);
	}
	else
	{
		ViewOrigin = FVector4(ViewMatrix.Inverse().TransformNormal(FVector(0,0,-1)).SafeNormal(),0);
	}
}

//<@ 2007. 9. 10 changmin
// add cascaded shadow map
void FSceneView::Ava_SplitViewFrustum()
{
	//     0      1     2      3
	// Near - 100 - 200 - 800 - end
	// Near - 100 - 200 - 400 - 800 - 1600 - 3200 - 6400 - 12800 ( 256m )
	// 100 uu  = 2m ( 1uu = 2cm )
	//<@ ava specific ; 2007. 11. 6 changmin
	// add cascaded shadow
	extern FLOAT GSliceValues[8];
	Ava_SliceValues[0] = GSliceValues[0];
	Ava_SliceValues[1] = GSliceValues[1];
	Ava_SliceValues[2] = GSliceValues[2];
	Ava_SliceValues[3] = GSliceValues[3];
	Ava_SliceValues[4] = GSliceValues[4];
	Ava_SliceValues[5] = GSliceValues[5];
	Ava_SliceValues[6] = GSliceValues[6];
	Ava_SliceValues[7] = GSliceValues[7];

	for( INT TestValue = 0; TestValue < 8; ++TestValue )
	{
		GSliceValues[TestValue] = Ava_SliceValues[TestValue] = (FLOAT)appFloor(NearClippingDistance * appPow(25600.0f/NearClippingDistance, ((FLOAT)(TestValue+1)/8.0f)));
	}

	//>@ ava

	FLOAT NearSliceDistance	= NearClippingDistance;
	FLOAT FarSliceDistance	= Ava_SliceValues[0];
	FLOAT ScaleOffset = 0.0f;
	INT VertexIndex = 0;
	for( INT SliceIndex = 0; SliceIndex < FCascadedShadowInfo::NumShadows; ++SliceIndex )
	{
		FVector FrustumVertex;
		Ava_SplitBounds[SliceIndex].Init();

		FMatrix SplitedProjectionMatrix;
		SplitedProjectionMatrix = ProjectionMatrix;

		SplitedProjectionMatrix.M[2][2] = (FarSliceDistance+ScaleOffset) / ( FarSliceDistance - NearSliceDistance + 2*ScaleOffset);
		SplitedProjectionMatrix.M[3][2] = - (NearSliceDistance-ScaleOffset)  * ( (FarSliceDistance +ScaleOffset)/ (FarSliceDistance - NearSliceDistance + 2 * ScaleOffset) );

		const FMatrix SplitedViewProjectionMatrix = ViewMatrix * SplitedProjectionMatrix;
		const FMatrix InvMatrix = SplitedViewProjectionMatrix.Inverse();

		Ava_SplitedViewFrustum[SliceIndex] = GetViewFrustumBounds( SplitedViewProjectionMatrix, TRUE );

		for(UINT Z = 0;Z < 2;Z++)
		{
			for(UINT Y = 0;Y < 2;Y++)
			{
				for(UINT X = 0;X < 2;X++)
				{
					FVector4 UnprojectedVertex = InvMatrix.TransformFVector4(
						FVector4(
						(X ? -1.0f : 1.0f),
						(Y ? -1.0f : 1.0f),
						(Z ?  0.0f : 1.0f),
						1.0f
						)
						);
					FrustumVertex = UnprojectedVertex / UnprojectedVertex.W;
					Ava_SplitedFrustumVertices[VertexIndex++] = FrustumVertex;
					Ava_SplitBounds[SliceIndex] += FrustumVertex;
				}
			}
		}

		NearSliceDistance = FarSliceDistance;
		FarSliceDistance = Ava_SliceValues[SliceIndex + 1];

		ScaleOffset = 10.0f;
	}
}
//>@ changmin

FVector4 FSceneView::WorldToScreen(const FVector& WorldPoint) const
{
	return ViewProjectionMatrix.TransformFVector4(FVector4(WorldPoint,1));
}

FVector FSceneView::ScreenToWorld(const FVector4& ScreenPoint) const
{
	return InvViewProjectionMatrix.TransformFVector4(ScreenPoint);
}

UBOOL FSceneView::ScreenToPixel(const FVector4& ScreenPoint,FVector2D& OutPixelLocation) const
{
	if(ScreenPoint.W > 0.0f)
	{
		FLOAT InvW = 1.0f / ScreenPoint.W;
		OutPixelLocation = FVector2D(
			X + (0.5f + ScreenPoint.X * 0.5f * InvW) * SizeX,
			Y + (0.5f - ScreenPoint.Y * 0.5f * InvW) * SizeY
			);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

FVector4 FSceneView::PixelToScreen(FLOAT InX,FLOAT InY,FLOAT Z) const
{
	return FVector4(
		-1.0f + InX / SizeX * +2.0f,
		+1.0f + InY / SizeY * -2.0f,
		Z,
		1
		);
}

/** Transforms a point from the view's world-space into pixel coordinates relative to the view's X,Y. */
UBOOL FSceneView::WorldToPixel(const FVector& WorldPoint,FVector2D& OutPixelLocation) const
{
	const FVector4 ScreenPoint = WorldToScreen(WorldPoint);
	return ScreenToPixel(ScreenPoint, OutPixelLocation);
}

/**  
 * Transforms a point from the view's world-space into the view's screen-space.  
 * Divides the resulting X, Y, Z by W before returning. 
 */
FPlane FSceneView::Project(const FVector& WorldPoint) const
{
	FPlane Result = WorldToScreen(WorldPoint);
	const FLOAT RHW = 1.0f / Result.W;

	return FPlane(Result.X * RHW,Result.Y * RHW,Result.Z * RHW,Result.W);
}

/** 
 * Transforms a point from the view's screen-space into world coordinates
 * multiplies X, Y, Z by W before transforming. 
 */
FVector FSceneView::Deproject(const FPlane& ScreenPoint) const
{
	return (ViewMatrix * ProjectionMatrix).Inverse().TransformFVector4(FPlane(ScreenPoint.X * ScreenPoint.W,ScreenPoint.Y * ScreenPoint.W,ScreenPoint.Z * ScreenPoint.W,ScreenPoint.W));
}

FSceneViewFamily::FSceneViewFamily(const FRenderTarget* InRenderTarget,FSceneInterface* InScene,EShowFlags InShowFlags,FLOAT InCurrentWorldTime,FLOAT InCurrentRealTime,FDynamicShadowStats* InDynamicShadowStats,UBOOL InbClear,UBOOL InDisableOcclusionQuery,UBOOL InEncodingOffline, UBOOL InDisableForegroundDPGs):
	RenderTarget(InRenderTarget),
	Scene(InScene),
	ShowFlags(InShowFlags),
	CurrentWorldTime(InCurrentWorldTime),
	CurrentRealTime(InCurrentRealTime),
	DynamicShadowStats(InDynamicShadowStats),
	bClear(InbClear),
	bDisableOcclusionQuery(InDisableOcclusionQuery),
	bEncodingOffline(InEncodingOffline),
	bDisableForegroundDPGs(InDisableForegroundDPGs)
{	
}

FSceneViewFamilyContext::~FSceneViewFamilyContext()
{
	// Cleanup the views allocated for this view family.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		delete Views(ViewIndex);
	}
}

void FScene::AddPrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	SCOPE_CYCLE_COUNTER(STAT_AddScenePrimitiveRenderThreadTime);

	// Allocate an entry in the primitives array for the primitive, and initialize its compact scene info.
	PrimitiveSceneInfo->Id = Primitives.Add().Index;
	FPrimitiveSceneInfoCompact& PrimitiveSceneInfoCompact = Primitives(PrimitiveSceneInfo->Id);
	PrimitiveSceneInfoCompact.Init(PrimitiveSceneInfo);

	PrimitiveSceneInfo->EnvCube = NULL;

	// Add the primitive to its shadow parent's linked list of children.
	PrimitiveSceneInfo->LinkShadowParent();

	// Add the primitive to its shadow parent's linked list of children.
	PrimitiveSceneInfo->LinkOcclusionGroup();

	// Create any RenderThreadResources required.
	PrimitiveSceneInfo->Proxy->CreateRenderThreadResources();

	// Add the primitive to the scene.
	PrimitiveSceneInfo->AddToScene();
	
	if(PrimitiveSceneInfo->LightEnvironmentSceneInfo)
	{
		// Add this primitive to the light environment's primitive list.
		PrimitiveSceneInfo->LightEnvironmentSceneInfo->AttachedPrimitives.AddItem(PrimitiveSceneInfo);
	}
}

void FScene::AddPrimitive(UPrimitiveComponent* Primitive)
{
	SCOPE_CYCLE_COUNTER(STAT_AddScenePrimitiveGameThreadTime);
	checkf(!Primitive->HasAnyFlags(RF_Unreachable), TEXT("%s"), *Primitive->GetFullName());

#if TRACK_GAME_THREAD_RENDER_MALLOCS
	GMalloc->BeginTrackingThread();
#endif

	// Save the world transform for next time the primitive is added to the scene
	FLOAT DeltaTime = GWorld->GetTimeSeconds() - Primitive->LastSubmitTime;
	if ( DeltaTime < -0.0001f || Primitive->LastSubmitTime < 0.0001f )
	{
		// Time was reset?		
		Primitive->LastSubmitTime = GWorld->GetTimeSeconds();
	}
	else if ( DeltaTime > 0.0001f )
	{
		// First call for the new frame?		
		Primitive->LastSubmitTime = GWorld->GetTimeSeconds();
	}	

	// Create the primitive's scene proxy.
	FPrimitiveSceneProxy* Proxy = Primitive->CreateSceneProxy();
	if(!Proxy)
	{
#if TRACK_GAME_THREAD_RENDER_MALLOCS
		GMalloc->EndTrackingThread();
#endif
		// Primitives which don't have a proxy are irrelevant to the scene manager.
		return;
	}

	// Cache the primitive's initial transform.
	Proxy->SetTransform(Primitive->LocalToWorld,Primitive->LocalToWorldDeterminant);

	// Create the primitive render info.
	FPrimitiveSceneInfo* PrimitiveSceneInfo = new FPrimitiveSceneInfo(Primitive,Proxy,this);
	Primitive->SceneInfo = PrimitiveSceneInfo;

	INC_DWORD_STAT_BY( Proxy->GetMemoryStatType(), Proxy->GetMemoryFootprint() + PrimitiveSceneInfo->GetMemoryFootprint() );
	INC_DWORD_STAT_BY( STAT_GameToRendererMallocTotal, Proxy->GetMemoryFootprint() + PrimitiveSceneInfo->GetMemoryFootprint() );

	// Send a command to the rendering thread to add the primitive to the scene.
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FAddPrimitiveCommand,
		FScene*,Scene,this,
		FPrimitiveSceneInfo*,PrimitiveSceneInfo,PrimitiveSceneInfo,
		{
			Scene->AddPrimitiveSceneInfo_RenderThread(PrimitiveSceneInfo);
		});

#if TRACK_GAME_THREAD_RENDER_MALLOCS
	GMalloc->EndTrackingThread();
#endif
}

/** The rendering thread side of UpdatePrimitiveTransform. */
class FUpdatePrimitiveTransformCommand
{
public:

	/** Initialization constructor. */
	FUpdatePrimitiveTransformCommand(UPrimitiveComponent* Primitive):
		PrimitiveSceneInfo(Primitive->SceneInfo),
		LocalToWorld(Primitive->LocalToWorld),		
		Bounds(Primitive->Bounds),
		LocalToWorldDeterminant(Primitive->LocalToWorldDeterminant)
	{}

	/** Called in the rendering thread to apply the updated transform. */
	void Apply()
	{
		// Reset EnvCube :)
		PrimitiveSceneInfo->EnvCube = NULL;

		// Update the primitive's motion blur information.
		PrimitiveSceneInfo->Scene->AddPrimitiveMotionBlur(PrimitiveSceneInfo, FALSE);		

		// Update the primitive's transform.
		PrimitiveSceneInfo->Proxy->SetTransform(LocalToWorld,LocalToWorldDeterminant);

		// Update the primitive's bounds.
		PrimitiveSceneInfo->Bounds = Bounds;

		// If the primitive has static mesh elements, it should have returned TRUE from ShouldRecreateProxyOnUpdateTransform!
		check(!PrimitiveSceneInfo->StaticMeshes.Num());

		// Update the primitive's compact scene info.
		FPrimitiveSceneInfoCompact& PrimitiveSceneInfoCompact = PrimitiveSceneInfo->Scene->Primitives(PrimitiveSceneInfo->Id);
		PrimitiveSceneInfoCompact.Init(PrimitiveSceneInfo);

		// Re-add the primitive to the scene with the new transform.
		PrimitiveSceneInfo->RemoveFromScene();
		PrimitiveSceneInfo->AddToScene();
	}

private:
	
	FPrimitiveSceneInfo* PrimitiveSceneInfo;
	FMatrix LocalToWorld;	
	FBoxSphereBounds Bounds;
	FLOAT LocalToWorldDeterminant;
};

void FScene::UpdatePrimitiveTransform(UPrimitiveComponent* Primitive)
{
	// Save the world transform for next time the primitive is added to the scene
	FLOAT DeltaTime = GWorld->GetTimeSeconds() - Primitive->LastSubmitTime;
	if ( DeltaTime < -0.0001f || Primitive->LastSubmitTime < 0.0001f )
	{
		// Time was reset?		
		Primitive->LastSubmitTime = GWorld->GetTimeSeconds();
	}
	else if ( DeltaTime > 0.0001f )
	{
		// First call for the new frame?		
		Primitive->LastSubmitTime = GWorld->GetTimeSeconds();
	}	

	if(Primitive->SceneInfo)
	{
		// Check if the primitive needs to recreate its proxy for the transform update.
		if(Primitive->ShouldRecreateProxyOnUpdateTransform())
		{
			// Re-add the primitive from scratch to recreate the primitive's proxy.
			RemovePrimitive(Primitive);
			AddPrimitive(Primitive);
		}
		else
		{
			// Send a message to rendering thread to update the proxy's cached transforms.
			FUpdatePrimitiveTransformCommand Command(Primitive);
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				UpdateTransformCommand,
				FUpdatePrimitiveTransformCommand,Command,Command,
				{
					Command.Apply();
				});
		}
	}
	else
	{
		// If the primitive doesn't have a scene info object yet, it must be added from scratch.
		AddPrimitive(Primitive);
	}
}

void FScene::RemovePrimitiveSceneInfo_RenderThread(FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	PrimitiveSceneInfo->EnvCube = NULL;

	// Update the primitive's motion blur information.
	AddPrimitiveMotionBlur(PrimitiveSceneInfo, TRUE);		

	if(PrimitiveSceneInfo->LightEnvironmentSceneInfo)
	{
		// Remove the primitive from its light environment's primitive list.
		PrimitiveSceneInfo->LightEnvironmentSceneInfo->AttachedPrimitives.RemoveItem(PrimitiveSceneInfo);
	}

	Primitives.Remove(PrimitiveSceneInfo->Id);

	// Unlink the primitive from its shadow parent.
	PrimitiveSceneInfo->UnlinkOcclusionGroup();

	// Unlink the primitive from its shadow parent.
	PrimitiveSceneInfo->UnlinkShadowParent();

	// Remove the primitive from the scene.
	PrimitiveSceneInfo->RemoveFromScene();

	// Delete the primitive scene proxy.
	delete PrimitiveSceneInfo->Proxy;
	PrimitiveSceneInfo->Proxy = NULL;
}

void FScene::RemovePrimitive(UPrimitiveComponent* Primitive)
{
	SCOPE_CYCLE_COUNTER(STAT_RemoveScenePrimitiveTime);

	FPrimitiveSceneInfo* PrimitiveSceneInfo = Primitive->SceneInfo;

	if(PrimitiveSceneInfo)
	{
		// Disassociate the primitive's render info.
		Primitive->SceneInfo = NULL;

		// Send a command to the rendering thread to remove the primitive from the scene.
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FRemovePrimitiveCommand,
			FScene*,Scene,this,
			FPrimitiveSceneInfo*,PrimitiveSceneInfo,PrimitiveSceneInfo,
			{
				Scene->RemovePrimitiveSceneInfo_RenderThread(PrimitiveSceneInfo);
			});

		// Begin the deferred cleanup of the primitive scene info.
		BeginCleanup(PrimitiveSceneInfo);
	}
}

void FScene::CommitPendingLightAttachments()
{
	SCOPE_CYCLE_COUNTER(STAT_AddSceneLightTime);

	for(TMap<FLightSceneInfo*,UBOOL>::TConstIterator LightIt(PendingLightAttachments);LightIt;++LightIt)
	{
		FLightSceneInfo* LightSceneInfo = LightIt.Key();

		//<@ ava specific ; 2007. 12. 28 changmin
		// add cascade shadow
		if( LightSceneInfo->bUseCascadedShadowmap && LightSceneInfo->LightType == LightType_Directional )
		{
			bHasSunLightShadow = TRUE;
			SunLight = LightSceneInfo;

			//<@ 2008. 1. 23 changmin
			// caching caster frustums
			extern UBOOL GUseCascadedShadow;
			void CreateShadowFrustum( const FLightSceneInfo *LightSceneInfo,  const FBox& Bounds, FConvexVolume* ShadowFrustum, FMatrix* CasterMatrix );
			if( GUseCascadedShadow )
			{
				FConvexVolume			ShadowVolume;
				AvaShadowVolumeVertices ShadowVolumeVertices;
				for(TSparseArray<FPrimitiveSceneInfoCompact>::TIterator PrimitiveIt(Primitives);PrimitiveIt;++PrimitiveIt)
				{
					FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo = *PrimitiveIt;
					FPrimitiveSceneProxy* Proxy = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Proxy;
					UPrimitiveComponent* Component = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Component;

					if( Proxy->bCastSunShadow
					&& (!Component->HiddenGame || Component->bCastHiddenShadow)
					&& Component->HasStaticShadowing()
					&& CompactPrimitiveSceneInfo.PrimitiveSceneInfo->ShadowVolumeId == INDEX_NONE )
					{
						FMatrix ShadowMatrix;
						CreateShadowFrustum( LightSceneInfo, CompactPrimitiveSceneInfo.Bounds.GetBox(), &ShadowVolume, &ShadowMatrix );
						FMatrix InvShadowMatrix = ShadowMatrix.Inverse();
						// Generate vertices for the shadow's frustum.
						for(UINT Z = 0;Z < 2;Z++)
						{
							for(UINT Y = 0;Y < 2;Y++)
							{
								for(UINT X = 0;X < 2;X++)
								{
									const FVector4 UnprojectedVertex = InvShadowMatrix.TransformFVector4(
										FVector4(
										(X ? -1.0f : 1.0f),
										(Y ? -1.0f : 1.0f),
										(Z ?  1.0f : 0.0f),
										1.0f
										)
										);
									const FVector ProjectedVertex = UnprojectedVertex / UnprojectedVertex.W;
									ShadowVolumeVertices.Vertices[GetCubeVertexIndex(X,Y,Z)] = ProjectedVertex;
								}
							}
						}
						INT Id = StaticSunShadowVolumes.AddItem(ShadowVolume);
						INT VertexId = StaticSunShadowVertices.AddItem(ShadowVolumeVertices);
						CompactPrimitiveSceneInfo.PrimitiveSceneInfo->ShadowVolumeId = Id;
						CompactPrimitiveSceneInfo.PrimitiveSceneInfo->ShadowVolumeVerticesId = VertexId;

					}
				}

			}
			//>@ ava
		}
		//>@ ava

		// Add the light to the scene's light list.
		LightSceneInfo->Id = Lights.AddItem(LightSceneInfo);

		// Find the light environments that are affected by the light.
		for(TSparseArray<FLightEnvironmentSceneInfo*>::TConstIterator LightEnvironmentIt(LightEnvironments);LightEnvironmentIt;++LightEnvironmentIt)
		{
			FLightEnvironmentSceneInfo* LightEnvironmentSceneInfo = *LightEnvironmentIt;
			if(LightSceneInfo->AffectsLightEnvironment(LightEnvironmentSceneInfo))
			{
				// Link the light to this light environment.
				LightSceneInfo->LightEnvironments.AddItem(LightEnvironmentSceneInfo);
				LightEnvironmentSceneInfo->AttachedLights.AddItem(LightSceneInfo);

				// Find primitives in this light environment which are affected by the light.
				for(INT PrimitiveIndex = 0;PrimitiveIndex < LightEnvironmentSceneInfo->AttachedPrimitives.Num();PrimitiveIndex++)
				{
					FPrimitiveSceneInfo* PrimitiveSceneInfo = LightEnvironmentSceneInfo->AttachedPrimitives(PrimitiveIndex);
					if(LightSceneInfo->AffectsPrimitive(FPrimitiveSceneInfoCompact(PrimitiveSceneInfo)))
					{
						// Attach the light to this primitive. (this updates the light environment's interaction list)
						FLightPrimitiveInteraction::Create(LightSceneInfo,PrimitiveSceneInfo);
					}
				}
			}
		}

		if(LightSceneInfo->bAffectsDefaultLightEnvironment)
		{
			// Look up the light group for the light's lighting channels.
			TArray<FLightSceneInfo*>* LightingChannelLightGroup = LightingChannelLightGroupMap.Find(LightSceneInfo->LightingChannels);
			if(!LightingChannelLightGroup)
			{
				// If there is no existing light group for the lighting channels, create one.
				LightingChannelLightGroup = &LightingChannelLightGroupMap.Set(LightSceneInfo->LightingChannels,TArray<FLightSceneInfo*>());
			}

			// Add the light to the light group.
			LightingChannelLightGroup->AddItem(LightSceneInfo);

			// For lights which aren't in a light environment, check for interactions with all primitives.
			for(TSparseArray<FPrimitiveSceneInfoCompact>::TConstIterator PrimitiveIt(Primitives);PrimitiveIt;++PrimitiveIt)
			{
				FPrimitiveSceneInfo* PrimitiveSceneInfo = PrimitiveIt->PrimitiveSceneInfo;
				if(LightSceneInfo->AffectsPrimitive(*PrimitiveIt))
				{
					FLightPrimitiveInteraction::Create(LightSceneInfo,PrimitiveSceneInfo);
				}
			}
		}
	}

	// Reset the pending light attachment list.
	PendingLightAttachments.Empty();
}

void FScene::AddLightSceneInfo_RenderThread(FLightSceneInfo* LightSceneInfo)
{
	// Simply add the light to the pending light attachment list until the next time the scene is rendered.
	// This is necessary to ensure that all light environments involving the light have been updated.
	PendingLightAttachments.Set(LightSceneInfo,TRUE);
}

void FScene::AddLight(ULightComponent* Light)
{
	// Whether the light has entirely precomputed lighting and therefore doesn't need to be added to the world.
	UBOOL bHasEntirelyPrecomputedLighting = FALSE;	
	if( Light->UseDirectLightMap && Light->HasStaticLighting() && !Light->IsA(USkyLightComponent::StaticClass()) )
	{
		if( !Light->bCanAffectDynamicPrimitivesOutsideDynamicChannel )
		{
			// The only way for this light to affect dynamic primitives is if it is in the dynamic channel.
			bHasEntirelyPrecomputedLighting = !Light->LightingChannels.Dynamic;
		}
		else
		{
			// Create a lighting channel that has all channels except BSP, Static and CompositeDynamic set.
			FLightingChannelContainer PotentiallyNonStatic;
			PotentiallyNonStatic.SetAllChannels();
			PotentiallyNonStatic.BSP				= FALSE;
			PotentiallyNonStatic.Static				= FALSE;
			PotentiallyNonStatic.CompositeDynamic	= FALSE;

			// A light with a light channel not overlapping with the "potentially non static" channel is only
			// affecting static objects
			bHasEntirelyPrecomputedLighting = !Light->LightingChannels.OverlapsWith( PotentiallyNonStatic );
		}
	}


	UBOOL bShouldAddLight = TRUE;
	// Game-only performance optimizations/ scalability options.
	if( GIsGame )
	{
		// Don't add lights only affecting static objects to the world during gameplay as all the lighting
		// and shadowing is precalculated and we don't need to consider them.
		if( bHasEntirelyPrecomputedLighting )
		{
			bShouldAddLight = FALSE;
		}

		// If dynamic lights are globally disabled, only add the ones affecting light environments.
		if( !GSystemSettings->bAllowDynamicLights && Light->bAffectsDefaultLightEnvironment )
		{
			bShouldAddLight = FALSE;
		}		
	}

	if( bShouldAddLight )
	{
		// Create the light scene info.
		FLightSceneInfo* LightSceneInfo = Light->CreateSceneInfo();
		Light->SceneInfo = LightSceneInfo;
		check(LightSceneInfo);

		// Send a command to the rendering thread to add the light to the scene.
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FAddLightCommand,
			FScene*,Scene,this,
			FLightSceneInfo*,LightSceneInfo,LightSceneInfo,
		{
			Scene->AddLightSceneInfo_RenderThread(LightSceneInfo);
		});
	}
}

void FScene::UpdateLightTransform(ULightComponent* Light)
{
	// Simply remove the light and re-add it to the scene to update the cached transform.
	RemoveLight(Light);
	if(Light->bEnabled)
	{
		AddLight(Light);
	}
}

void FScene::AddLightEnvironment(const ULightEnvironmentComponent* LightEnvironment)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		AddLightEnvironment,
		FScene*,Scene,this,
		FLightEnvironmentSceneInfo*,LightEnvironmentSceneInfo,LightEnvironment->SceneInfo,
	{
		LightEnvironmentSceneInfo->Id = Scene->LightEnvironments.AddItem(LightEnvironmentSceneInfo);
	});
}

void FScene::RemoveLightEnvironment(const ULightEnvironmentComponent* LightEnvironment)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		RemoveLightEnvironment,
		FScene*,Scene,this,
		FLightEnvironmentSceneInfo*,LightEnvironmentSceneInfo,LightEnvironment->SceneInfo,
	{
		Scene->LightEnvironments.Remove(LightEnvironmentSceneInfo->Id);
		LightEnvironmentSceneInfo->Id = INDEX_NONE;
	});
}

void FScene::RemoveLightSceneInfo_RenderThread(FLightSceneInfo* LightSceneInfo)
{
	// Find and remove the light from the pending light attachment list.	
	const UBOOL bLightAttachmentWasPending = PendingLightAttachments.HasKey(LightSceneInfo);	
	if( bLightAttachmentWasPending )
	{
		PendingLightAttachments.Remove(LightSceneInfo);
	}
	// If the light's attachment was still pending, no further work
	else
	{
		//<@ ava specific ; 2007. 12. 28 changmin
		// add cascaded shadow
		if( LightSceneInfo->bUseCascadedShadowmap && LightSceneInfo->LightType == LightType_Directional )
		{
			bHasSunLightShadow = FALSE;
			SunLight = NULL;
		}
		//>@ ava
		// Remove the light from the light environments it affects.
		for(INT EnvironmentIndex = 0;EnvironmentIndex < LightSceneInfo->LightEnvironments.Num();EnvironmentIndex++)
		{
			FLightEnvironmentSceneInfo* LightEnvironmentSceneInfo = LightSceneInfo->LightEnvironments(EnvironmentIndex);

			// Delink the light from the light environment.
			LightEnvironmentSceneInfo->AttachedLights.RemoveItem(LightSceneInfo);
		}

		if(LightSceneInfo->bAffectsDefaultLightEnvironment)
		{
			// Look up the light group for the light's lighting channels.
			TArray<FLightSceneInfo*>* LightingChannelLightGroup = LightingChannelLightGroupMap.Find(LightSceneInfo->LightingChannels);
			check(LightingChannelLightGroup);

			// Remove the light from the light group.
			LightingChannelLightGroup->RemoveItem(LightSceneInfo);

			// If the lighting channel light group is empty, free it.
			if(!LightingChannelLightGroup->Num())
			{
				LightingChannelLightGroupMap.Remove(LightSceneInfo->LightingChannels);
			}
		}

		Lights.Remove(LightSceneInfo->Id);
		LightSceneInfo->Detach();
	}

	delete LightSceneInfo;
}

void FScene::RemoveLight(ULightComponent* Light)
{
	SCOPE_CYCLE_COUNTER(STAT_RemoveSceneLightTime);

	FLightSceneInfo* LightSceneInfo = Light->SceneInfo;

	if(LightSceneInfo)
	{
		// Disassociate the primitive's render info.
		Light->SceneInfo = NULL;

		// Send a command to the rendering thread to remove the light from the scene.
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FRemoveLightCommand,
			FScene*,Scene,this,
			FLightSceneInfo*,LightSceneInfo,LightSceneInfo,
			{
				Scene->RemoveLightSceneInfo_RenderThread(LightSceneInfo);
			});
	}
}

void FScene::AddHeightFog(UHeightFogComponent* FogComponent)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FAddFogCommand,
		FScene*,Scene,this,
		FHeightFogSceneInfo,HeightFogSceneInfo,FHeightFogSceneInfo(FogComponent),
		{
			// Create a FHeightFogSceneInfo for the component in the scene's fog array.
			new(Scene->Fogs) FHeightFogSceneInfo(HeightFogSceneInfo);

			// Sort the scene's fog array by height.
			Sort<USE_COMPARE_CONSTREF(FHeightFogSceneInfo,SceneCore)>(&Scene->Fogs(0),Scene->Fogs.Num());
		});
}

void FScene::RemoveHeightFog(UHeightFogComponent* FogComponent)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FAddFogCommand,
		FScene*,Scene,this,
		UHeightFogComponent*,FogComponent,FogComponent,
		{
			// Remove the given component's FHeightFogSceneInfo from the scene's fog array.
			for(INT FogIndex = 0;FogIndex < Scene->Fogs.Num();FogIndex++)
			{
				if(Scene->Fogs(FogIndex).Component == FogComponent)
				{
					Scene->Fogs.Remove(FogIndex);
					break;
				}
			}
		});
}

void FScene::AddWindSource(UWindDirectionalSourceComponent* WindComponent)
{
	FWindSourceSceneProxy* SceneProxy = WindComponent->CreateSceneProxy();
	WindComponent->SceneProxy = SceneProxy;

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FAddWindSourceCommand,
		FScene*,Scene,this,
		FWindSourceSceneProxy*,SceneProxy,SceneProxy,
	{
		Scene->WindSources.AddItem(SceneProxy);
	});
}

void FScene::RemoveWindSource(UWindDirectionalSourceComponent* WindComponent)
{
	FWindSourceSceneProxy* SceneProxy = WindComponent->SceneProxy;
	WindComponent->SceneProxy = NULL;

	if(SceneProxy)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FRemoveWindSourceCommand,
			FScene*,Scene,this,
			FWindSourceSceneProxy*,SceneProxy,SceneProxy,
		{
			Scene->WindSources.RemoveItem(SceneProxy);

			delete SceneProxy;
		});
	}
}

const TArray<FWindSourceSceneProxy*>& FScene::GetWindSources_RenderThread() const
{
	check(IsInRenderingThread());
	return WindSources;
}

/**
 * Retrieves the lights interacting with the passed in primitive and adds them to the out array.
 *
 * Render thread version of function.
 * 
 * @param	Primitive				Primitive to retrieve interacting lights for
 * @param	RelevantLights	[out]	Array of lights interacting with primitive
 */
void FScene::GetRelevantLights_RenderThread( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const
{
	check( Primitive );
	check( RelevantLights );
	if( Primitive->SceneInfo )
	{
		for( const FLightPrimitiveInteraction* Interaction=Primitive->SceneInfo->LightList; Interaction; Interaction=Interaction->GetNextLight() )
		{
			RelevantLights->AddItem( Interaction->GetLight()->LightComponent );
		}
	}
}

/**
 * Retrieves the lights interacting with the passed in primitive and adds them to the out array.
 *
 * @param	Primitive				Primitive to retrieve interacting lights for
 * @param	RelevantLights	[out]	Array of lights interacting with primitive
 */
void FScene::GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const
{
	if( Primitive && RelevantLights )
	{
		// Add interacting lights to the array.
		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
			FGetRelevantLightsCommand,
			const FScene*,Scene,this,
			UPrimitiveComponent*,Primitive,Primitive,
			TArray<const ULightComponent*>*,RelevantLights,RelevantLights,
			{
				Scene->GetRelevantLights_RenderThread( Primitive, RelevantLights );
			});

		// We need to block the main thread as the rendering thread needs to finish modifying the array before we can continue.
		FlushRenderingCommands();
	}
}

/**
 * Create the scene capture info for a capture component and add it to the scene
 * @param CaptureComponent - component to add to the scene 
 */
void FScene::AddSceneCapture(USceneCaptureComponent* CaptureComponent)
{
	check(CaptureComponent);

	// create a new scene capture probe
	FSceneCaptureProbe* SceneProbe = CaptureComponent->CreateSceneCaptureProbe();
	// add it to the scene
	if( SceneProbe )
	{
		// Create a new capture info for the component
		FCaptureSceneInfo* CaptureInfo = new FCaptureSceneInfo(CaptureComponent,SceneProbe);

		// Send a command to the rendering thread to add the capture to the scene.
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FAddSceneCaptureCommand,
			FScene*,Scene,this,
			FCaptureSceneInfo*,CaptureInfo,CaptureInfo,
		{
			// add the newly created capture info to the list of captures in the scene
			CaptureInfo->AddToScene(Scene);
		});
	}
}

/**
 * Remove the scene capture info for a capture component from the scene
 * @param CaptureComponent - component to remove from the scene 
 */
void FScene::RemoveSceneCapture(USceneCaptureComponent* CaptureComponent)
{
	check(CaptureComponent);

	FCaptureSceneInfo* CaptureInfo = CaptureComponent->CaptureInfo;
    if( CaptureInfo )
	{
		// Disassociate the component's capture info.
		CaptureComponent->CaptureInfo = NULL;

		// Send a command to the rendering thread to remove the capture from the scene.
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FRemoveCaptureCommand,
			FScene*,Scene,this,
			FCaptureSceneInfo*,CaptureInfo,CaptureInfo,
		{
			// render thread should be done w/ it at this point. safe to destroy 
			delete CaptureInfo;
		});
	}
}

/** 
 *	Indicates if sounds in this should be allowed to play. 
 *	By default - sound is only allowed in the GWorld scene.
 */
UBOOL FScene::AllowAudioPlayback()
{
	// Update wave instances if component is attached to current world. This means that only PIE can emit sound if a PIE session is active
	// and prevents ambient sounds from being played more than once as they will be part of the original world and the PIE world. The lesser
	// of two evils...

	return ((GWorld->Scene == this) || bAlwaysAllowAudioPlayback);
}

/**
 * @return		TRUE if hit proxies should be rendered in this scene.
 */
UBOOL FScene::RequiresHitProxies() const
{
	return (GIsEditor && bRequiresHitProxies);
}

/** 
*	Set the primitives motion blur info
* 
*	@param PrimitiveSceneInfo	The primitive to add
*/
void FScene::AddPrimitiveMotionBlur(FPrimitiveSceneInfo* PrimitiveSceneInfo, UBOOL bRemoving)
{
	check(PrimitiveSceneInfo);
	if (PrimitiveSceneInfo->Proxy && PrimitiveSceneInfo->Component && (PrimitiveSceneInfo->Proxy->IsMovable() == TRUE))
	{
		// Check for validity...
		INT MBIndex = PrimitiveSceneInfo->Component->MotionBlurInfoIndex;
		if (MBIndex != -1)
		{
			if (MBIndex >= MotionBlurInfoArray.Num())
			{
				// The primitive was detached and not rendered, but hung onto by the game thread.
				// So reset the MBInfoIndex.
				PrimitiveSceneInfo->Component->MotionBlurInfoIndex = -1;
			}
			else
			{
				FMotionBlurInfo& MBInfo = MotionBlurInfoArray(MBIndex);
				if (MBInfo.Component != PrimitiveSceneInfo->Component)
				{
					// A different component is in this slot...
					// Make same assumption as above.
					PrimitiveSceneInfo->Component->MotionBlurInfoIndex = -1;
				}
			}
		}

		if (PrimitiveSceneInfo->Component->MotionBlurInfoIndex == -1)
		{
			// First time add
			INT Index = MotionBlurInfoArray.AddItem(FMotionBlurInfo());
			FMotionBlurInfo& MBInfo = MotionBlurInfoArray(Index);

			MBInfo.PrimitiveSceneInfo = PrimitiveSceneInfo;
			MBInfo.Component = PrimitiveSceneInfo->Component;
			if (bRemoving)
			{
				MBInfo.PreviousLocalToWorld = PrimitiveSceneInfo->Component->LocalToWorld;
			}
			else
			{
				MBInfo.PreviousLocalToWorld = PrimitiveSceneInfo->Proxy->GetLocalToWorld();
			}
			PrimitiveSceneInfo->Component->MotionBlurInfoIndex = Index;
		}
		else
		{
			// Update the PrimitiveSceneInfo pointer
			check(PrimitiveSceneInfo->Component->MotionBlurInfoIndex < MotionBlurInfoArray.Num());
			FMotionBlurInfo& MBInfo = MotionBlurInfoArray(PrimitiveSceneInfo->Component->MotionBlurInfoIndex);
			if (bRemoving)
			{
				// When removing, set the PSI to null to indicate the primtive component pointer is invalid
				MBInfo.PrimitiveSceneInfo = NULL;
			}
			else
			{
				// Update the PSI pointer
				MBInfo.PrimitiveSceneInfo = PrimitiveSceneInfo;
			}
		}
	}
}

/**
*	Clear out the motion blur info
*/
void FScene::ClearMotionBlurInfo()
{
	for (INT MBIndex = 0; MBIndex < MotionBlurInfoArray.Num(); MBIndex++)
	{
		FMotionBlurInfo& MBInfo = MotionBlurInfoArray(MBIndex);

		if (MBInfo.PrimitiveSceneInfo)
		{
			// The primitive component has not been released... clear it's MBInfoIndex
			MBInfo.Component->MotionBlurInfoIndex = -1;
		}
	}

	MotionBlurInfoArray.Empty();
}

/** 
*	Get the primitives motion blur info
* 
*	@param	PrimitiveSceneInfo	The primitive to retrieve the motion blur info for
*	@param	MotionBlurInfo		The pointer to set to the motion blur info for the primitive (OUTPUT)
*
*	@return	UBOOL				TRUE if the primitive info was found and set
*/
UBOOL FScene::GetPrimitiveMotionBlurInfo(const FPrimitiveSceneInfo* PrimitiveSceneInfo, const FMotionBlurInfo*& MBInfo)
{
	if (PrimitiveSceneInfo && PrimitiveSceneInfo->Component)
	{
		if ((PrimitiveSceneInfo->Component->MotionBlurInfoIndex != -1) &&
			(PrimitiveSceneInfo->Component->MotionBlurInfoIndex < MotionBlurInfoArray.Num()))
		{
			MBInfo = &(MotionBlurInfoArray(PrimitiveSceneInfo->Component->MotionBlurInfoIndex));
			return TRUE;
		}
	}
	return FALSE;
}

void FScene::Release()
{
	// Send a command to the rendering thread to release the scene.
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		FReleaseCommand,
		FScene*,Scene,this,
		{
			delete Scene;
		});
}

void FSceneViewState::SetColorCorrection( FLOAT ElapsedTime, FColorCorrectionResource* Resource, FLOAT TransitionTime )
{
	UBOOL bFound = FALSE;
	
	for (INT i=ColorCorrectionTickets.Num()-1; i>=0; --i)
	{
		FColorCorrectionTicket& Ticket = ColorCorrectionTickets(i);

		if (Ticket.Resource == Resource && TransitionTime > 0)
		{
			bFound = TRUE;
			Ticket.WeightGradient = Max( Ticket.WeightGradient, ( 1.0f - Ticket.CurrentWeight ) / TransitionTime );
		}

		Ticket.CurrentWeight += Ticket.WeightGradient * ElapsedTime;

		if (Ticket.CurrentWeight >= 1.0f)
		{
			Ticket.WeightGradient = 0.0f;
		}
		else if (Ticket.CurrentWeight < 0.0f)
		{
			ColorCorrectionTickets.Remove(i);
		}
	}

	if (Resource && !bFound)
	{
		// 기존에 하나도 없었거나, immediate 요청인 경우
		if (ColorCorrectionTickets.Num() == 0 || TransitionTime <= 0.0f)
		{
			// immediate를 위해서
			ColorCorrectionTickets.Empty();

			FColorCorrectionTicket* Ticket = new (ColorCorrectionTickets) FColorCorrectionTicket;
			Ticket->CurrentWeight = 1.0f;
			Ticket->Resource = Resource;
			Ticket->WeightGradient = 0.0f;
		}
		else
		{
			FColorCorrectionTicket* Ticket = new (ColorCorrectionTickets) FColorCorrectionTicket;
			Ticket->CurrentWeight = 0.0f;
			Ticket->Resource = Resource;
			Ticket->WeightGradient = 1.0f / TransitionTime;
		}
	}

	INT Location = -1;

	// 나머지 ticket에 대해 fade out 처리를 한다
	for (INT i=0; i<ColorCorrectionTickets.Num(); ++i)
	{
		FColorCorrectionTicket& Ticket = ColorCorrectionTickets(i);

		// 지금 설정된 것만 pass
		if (Ticket.Resource != Resource)
		{
			Ticket.WeightGradient = Min( Ticket.WeightGradient, -Ticket.CurrentWeight / TransitionTime );
		}			
		else
		{
			Location = i;
		}
	}		

	if (Location >= 0 && Location != ColorCorrectionTickets.Num() - 1)
	{
		ColorCorrectionTickets.SwapItems( Location, ColorCorrectionTickets.Num() - 1 );
	}	
}

/**
* Dummy NULL scene interface used by dedicated servers.
*/
class FNULLSceneInterface : public FSceneInterface
{
public:
	FNULLSceneInterface( UWorld* InWorld )
		:	World( InWorld )
	{}

	virtual void AddPrimitive(UPrimitiveComponent* Primitive){}
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive){}

	/** Updates the transform of a primitive which has already been added to the scene. */
	virtual void UpdatePrimitiveTransform(UPrimitiveComponent* Primitive){}

	virtual void AddLight(ULightComponent* Light){}
	virtual void RemoveLight(ULightComponent* Light){}

	/** Updates the transform of a light which has already been added to the scene. */
	virtual void UpdateLightTransform(ULightComponent* Light){}

	virtual void AddLightEnvironment(const ULightEnvironmentComponent* LightEnvironment) {}
	virtual void RemoveLightEnvironment(const ULightEnvironmentComponent* LightEnvironment) {}

	virtual void AddSceneCapture(USceneCaptureComponent* CaptureComponent){}
	virtual void RemoveSceneCapture(USceneCaptureComponent* CaptureComponent){}

	virtual void AddHeightFog(class UHeightFogComponent* FogComponent){}
	virtual void RemoveHeightFog(class UHeightFogComponent* FogComponent){}

	virtual void AddFogVolume(const UPrimitiveComponent* VolumeComponent){};
	virtual void AddFogVolume(class FFogVolumeDensitySceneInfo* FogVolumeSceneInfo, const UPrimitiveComponent* VolumeComponent){};
	virtual void RemoveFogVolume(const UPrimitiveComponent* VolumeComponent){};

	virtual void AddWindSource(class UWindDirectionalSourceComponent* WindComponent) {}
	virtual void RemoveWindSource(class UWindDirectionalSourceComponent* WindComponent) {}
	virtual const TArray<class FWindSourceSceneProxy*>& GetWindSources_RenderThread() const
	{
		static TArray<class FWindSourceSceneProxy*> NullWindSources;
		return NullWindSources;
	}

	virtual void Release(){}

	/**
	* Retrieves the lights interacting with the passed in primitive and adds them to the out array.
	*
	* @param	Primitive				Primitive to retrieve interacting lights for
	* @param	RelevantLights	[out]	Array of lights interacting with primitive
	*/
	virtual void GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const {}

	/** Indicates if sounds in this should be allowed to play. */
	virtual UBOOL AllowAudioPlayback() 
	{
		return FALSE;
	}

	/**
	* @return		TRUE if hit proxies should be rendered in this scene.
	*/
	virtual UBOOL RequiresHitProxies() const 
	{
		return FALSE;
	}

	// Accessors.
	virtual class UWorld* GetWorld() const
	{
		return World;
	}

	/**
	* Return the scene to be used for rendering
	*/
	virtual FSceneInterface* GetRenderScene()
	{
		return this;
	}

private:
	UWorld* World;
};


/**
 * Allocates a new instance of the private FScene implementation for the given world.
 * @param World - An optional world to associate with the scene.
 * @param bAlwaysAllowAudioPlayback - Indicate that audio in this scene should always be played, even if its not the GWorld.
 * @param bInRequiresHitProxies- Indicates that hit proxies should be rendered in the scene.
 */
FSceneInterface* AllocateScene(UWorld* World, UBOOL bInAlwaysAllowAudioPlayback, UBOOL bInRequiresHitProxies)
{
	FSceneInterface* SceneInterface = NULL;

	// Create a full fledged scene if we have something to render.
	if( GIsClient )
	{
		FScene* Scene = new FScene;
		Scene->World = World;
		Scene->bAlwaysAllowAudioPlayback = bInAlwaysAllowAudioPlayback;
		Scene->bRequiresHitProxies = bInRequiresHitProxies;
		SceneInterface = Scene;
	}
	// And fall back to a dummy/ NULL implementation for commandlets and dedicated server.
	else
	{
		SceneInterface = new FNULLSceneInterface( World );
	}

	return SceneInterface;
}

FSceneViewStateInterface* AllocateViewState()
{
	return new FSceneViewState();
}

FPrimitiveSceneProxy* Scene_GetProxyFromInfo(FPrimitiveSceneInfo* Info)
{
	return Info ? Info->Proxy : NULL;
}

/*-----------------------------------------------------------------------------
Stat declarations.
-----------------------------------------------------------------------------*/

DECLARE_STATS_GROUP(TEXT("SceneUpdate"),STATGROUP_SceneUpdate);

DECLARE_CYCLE_STAT(TEXT("AddPrimitive (RT) time"),STAT_AddScenePrimitiveRenderThreadTime,STATGROUP_SceneUpdate);
DECLARE_CYCLE_STAT(TEXT("AddPrimitive (GT) time"),STAT_AddScenePrimitiveGameThreadTime,STATGROUP_SceneUpdate);

DECLARE_CYCLE_STAT(TEXT("AddLight time"),STAT_AddSceneLightTime,STATGROUP_SceneUpdate);
DECLARE_CYCLE_STAT(TEXT("TouchPrimitive time"),STAT_TouchScenePrimitiveTime,STATGROUP_SceneUpdate);
DECLARE_CYCLE_STAT(TEXT("RemovePrimitive time"),STAT_RemoveScenePrimitiveTime,STATGROUP_SceneUpdate);
DECLARE_CYCLE_STAT(TEXT("RemoveLight time"),STAT_RemoveSceneLightTime,STATGROUP_SceneUpdate);