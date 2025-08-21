/*=============================================================================
	SceneRendering.h: Scene rendering definitions.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** An association between a hit proxy and a mesh. */
class FHitProxyMeshPair : public FMeshElement
{
public:
	FHitProxyId HitProxyId;

	/** Initialization constructor. */
	FHitProxyMeshPair(const FMeshElement& InMesh,FHitProxyId InHitProxyId):
		FMeshElement(InMesh),
		HitProxyId(InHitProxyId)
	{}
};

/** Information about a visible light. */
class FVisibleLightInfo
{
public:

	/** Information about a visible light in a specific DPG */
	class FDPGInfo
	{
	public:

		/** The dynamic primitives which are both visible and affected by this light. */
		TArray<FPrimitiveSceneInfo*> VisibleDynamicLitPrimitives;

		/** The primitives which are visible, affected by this light and receiving lit decals. */
		TArray<FPrimitiveSceneInfo*> VisibleLitDecalPrimitives;

		/** The total number of visible primitives which are affected by this light. */
		INT NumVisibleLitPrimitives;

		/** TRUE if this light passed the view frustum check. Only used for lights w/ modulated shadows */
		UBOOL bVisibleInFrustum;

		FDPGInfo()
		:	NumVisibleLitPrimitives(0)
		,	bVisibleInFrustum(FALSE)
		{}
	};

	/** Information about the light in each DPGs. */
	FDPGInfo DPGInfo[SDPG_MAX_SceneRender];
};

/** 
* Set of sorted translucent scene prims  
*/
class FTranslucentPrimSet
{
public:

	/** 
	* Iterate over the sorted list of prims and draw them
	* @param Context - command context
	* @param View - current view used to draw items
	* @param DPGIndex - current DPG used to draw items
	* @param bPreFog - TRUE if the draw call is occurring before fog has been rendered.
	* @return TRUE if anything was drawn
	*/
	UBOOL Draw(FCommandContextRHI* Context,const class FViewInfo* View,UINT DPGIndex,UBOOL bPreFog);

	/**
	* Add a new primitive to the list of sorted prims
	* @param PrimitiveSceneInfo - primitive info to add. Origin of bounds is used for sort.
	* @param ViewInfo - used to transform bounds to view space
	* @param bUsesSceneColor - primitive samples from scene color
	*/
	void AddScenePrimitive(FPrimitiveSceneInfo* PrimitivieSceneInfo,const FViewInfo& ViewInfo, UBOOL bUsesSceneColor=FALSE);

	/**
	* Sort any primitives that were added to the set back-to-front
	*/
	void SortPrimitives();

	/** 
	* @return number of prims to render
	*/
	INT NumPrims() const
	{
		return SortedPrims.Num() + SortedSceneColorPrims.Num();
	}

private:
	/** contains a scene prim and its sort key */
	struct FSortedPrim
	{
		FSortedPrim(FPrimitiveSceneInfo* InPrimitiveSceneInfo,FLOAT InSortKey)
			:	PrimitiveSceneInfo(InPrimitiveSceneInfo)
			,	SortKey(InSortKey)
		{

		}

		FPrimitiveSceneInfo* PrimitiveSceneInfo;
		FLOAT SortKey;
	};
	/** list of sorted translucent primitives */
	TArray<FSortedPrim> SortedPrims;
	/** list of sorted translucent primitives that use the scene color. These are drawn after all other translucent prims */
	TArray<FSortedPrim> SortedSceneColorPrims;
	/** sortkey compare class */
	IMPLEMENT_COMPARE_CONSTREF( FSortedPrim,TranslucentRender,{ return (A.SortKey <= B.SortKey) ? 1 : -1; } )
};

/** 
 * Set of primitives with decal scene relevance.
 */
class FDecalPrimSet
{
public:
	/** 
	 * Iterates over the sorted list of sorted decal prims and draws them.
	 *
	 * @param	Context							Command context.
	 * @param	View							The Current view used to draw items.
	 * @param	DPGIndex						The current DPG used to draw items.
	 * @param	bTranslucentReceiverPass		TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	 * @param	bPreFog							TRUE if the draw call is occurring before fog has been rendered.
	 * @return									TRUE if anything was drawn, FALSE otherwise.
	 */
	UBOOL Draw(FCommandContextRHI* Context,const class FViewInfo* View,UINT DPGIndex,UBOOL bTranslucentReceiverPass, UBOOL bPreFog);

	/**
	* Adds a new primitive to the list of sorted prims
	* @param	PrimitiveSceneInfo		The primitive info to add.
	*/
	void AddScenePrimitive(const FPrimitiveSceneInfo* PrimitivieSceneInfo)
	{
		Prims.AddItem( PrimitivieSceneInfo );
	}

	/**
	 * @param		PrimitiveIndex		The index of the primitive to return.
	 * @return							The primitive at the specified index.
	 */
	const FPrimitiveSceneInfo* GetScenePrimitive(INT PrimitiveIndex) const
	{
		return Prims( PrimitiveIndex );
	}

	/** 
	 * @return		The number of primitives in this set.
	 */
	INT NumPrims() const
	{
		return Prims.Num();
	}

private:
	/** List of primitives with decal relevance. */
	TArray<const FPrimitiveSceneInfo*> Prims;
};

/** MotionBlur parameters */
struct FMotionBlurParameters
{
	FMotionBlurParameters()
		:	VelocityScale( 1.0f )
		,	MaxVelocity( 1.0f )
		,	bFullMotionBlur( TRUE )
		,	RotationThreshold( 45.0f )
		,	TranslationThreshold( 10000.0f )
	{
	}
	FLOAT VelocityScale;
	FLOAT MaxVelocity;
	UBOOL bFullMotionBlur;
	FLOAT RotationThreshold;
	FLOAT TranslationThreshold;
};

/**
* Combines consecutive primitives which use the same occlusion query into a single DrawIndexedPrimitive call.
*/
class FOcclusionQueryBatcher
{
public:

	/** The maximum number of consecutive previously occluded primitives which will be combined into a single occlusion query. */
	enum { OccludedPrimitiveQueryBatchSize = 8 };

	/** Initialization constructor. */
	FOcclusionQueryBatcher(class FSceneViewState* ViewState,UINT InMaxBatchedPrimitives);

	/** Destructor. */
	~FOcclusionQueryBatcher();

	/** Renders the current batch and resets the batch state. */
	void Flush(FCommandContextRHI* Context);

	/**
	* Batches a primitive's occlusion query for rendering.
	* @param Bounds - The primitive's bounds.
	*/
	INT BatchPrimitive(const FBoxSphereBounds& Bounds);
	UBOOL BatchChildPrimitive(INT ParentIndex, const FBoxSphereBounds& Bounds);

	/** A batched primitive. */
	struct FPrimitive
	{
		FVector Origin;
		FVector Extent;
		FOcclusionQueryRHIRef OcclusionQuery;
		
		INT FirstChild;

		union
		{
			INT NumChildren;
			INT NextChild;
		};
	};

	/** The pending primitives. */
	TArray<FPrimitive> Primitives;
	TArray<FPrimitive> ChildPrimitives;

private:

	/** The pending batches. */
	TArray<FOcclusionQueryRHIRef> BatchOcclusionQueries;	

	/** The batch new primitives are being added to. */
	FOcclusionQueryRHIParamRef CurrentBatchOcclusionQuery;

	/** The maximum number of primitives in a batch. */
	const UINT MaxBatchedPrimitives;

	/** The number of primitives in the current batch. */
	UINT NumBatchedPrimitives;

	/** The pool to allocate occlusion queries from. */
	class FOcclusionQueryPool* OcclusionQueryPool;
};

//<@ ava specific ; 2008. 1. 18 changmin
// add cascaded shadow
class AvaCasterOcclusionQueryBatcher
{
public:

	/** The maximum number of consecutive previously occluded primitives which will be combined into a single occlusion query. */
	enum { OccludedPrimitiveQueryBatchSize = 128 };

	/** Initialization constructor. */
	AvaCasterOcclusionQueryBatcher(class FSceneViewState* ViewState,UINT InMaxBatchedPrimitives);

	/** Destructor. */
	~AvaCasterOcclusionQueryBatcher();

	/** Renders the current batch and resets the batch state. */
	void Flush(FCommandContextRHI* Context);

	/**
	* Batches a primitive's occlusion query for rendering.
	* @param Bounds - The primitive's bounds.
	*/
	INT BatchPrimitive(const FBoxSphereBounds& Bounds, const FLightSceneInfo* LightSceneInfo, const FPrimitiveSceneInfo* PrimitiveSceneInfo );
	UBOOL BatchChildPrimitive(INT ParentIndex, const FBoxSphereBounds& Bounds, const FLightSceneInfo* LightSceneInfo, const FPrimitiveSceneInfo* PrimitiveSceneInfo);

	/** A batched primitive. */
	struct FPrimitive
	{
		FBox BoundingBox;
		const FLightSceneInfo* LightSceneInfo;
		const FPrimitiveSceneInfo* PrimitiveSceneInfo;
		FOcclusionQueryRHIRef OcclusionQuery;
		INT FirstChild;
		union
		{
			INT NumChildren;
			INT NextChild;
		};
	};

	/** The pending primitives. */
	TArray<FPrimitive> Primitives;
	TArray<FPrimitive> ChildPrimitives;

private:

	/** The pending batches. */
	TArray<FOcclusionQueryRHIRef> BatchOcclusionQueries;	

	/** The batch new primitives are being added to. */
	FOcclusionQueryRHIParamRef CurrentBatchOcclusionQuery;

	/** The maximum number of primitives in a batch. */
	const UINT MaxBatchedPrimitives;

	/** The number of primitives in the current batch. */
	UINT NumBatchedPrimitives;

	/** The pool to allocate occlusion queries from. */
	class FOcclusionQueryPool* OcclusionQueryPool;
};


//>@ ava


/** A FSceneView with additional state used by the scene renderer. */
class FViewInfo : public FSceneView
{
public:

	/** A map from projected shadow ID to a boolean visibility value. */
	FBitArray ShadowVisibilityMap;

	/** A map from primitive ID to a boolean visibility value. */
	FBitArray PrimitiveVisibilityMap;

	/** A map from primitive ID to the primitive's view relevance. */
	TArray<FPrimitiveViewRelevance> PrimitiveViewRelevanceMap;

	/** A map from static mesh ID to a boolean visibility value. */
	FBitArray StaticMeshVisibilityMap;

	FBitArray SeeThroughStaticMeshVisibilityMap;

#if !FINAL_RELEASE
	/** Cull distance update를 위해 정확한 visibility를 체크하기 위해 추가함 */
	FBitArray FineStaticMeshVisibilityMap;

	FBitArray FineDynamicMeshVisibilityMap;
#endif

	/** A map from static mesh ID to a boolean occluder value. */
	FBitArray StaticMeshOccluderMap;

	/** A map from static mesh ID to a boolean visibility value for velocity rendering. */
	FBitArray StaticMeshVelocityMap;

	/** The dynamic primitives visible in this view. */
	TArray<const FPrimitiveSceneInfo*> VisibleDynamicPrimitives;

	/** The dynamic primitives visible in this view. */
	TArray<const FPrimitiveSceneInfo*> SeeThroughVisibleDynamicPrimitives;

	/** The primitives visible in this view that have lit decals. */
	TArray<const FPrimitiveSceneInfo*> VisibleLitDecalPrimitives;	

	/** Set of decal primitives for this view - one for each DPG */
	FDecalPrimSet DecalPrimSet[SDPG_MAX_SceneRender];

	/** Set of translucent prims for this view - one for each DPG */
	FTranslucentPrimSet TranslucentPrimSet[SDPG_MAX_SceneRender];

	/** Set of distortion prims for this view - one for each DPG */
	FDistortionPrimSet DistortionPrimSet[SDPG_MAX_SceneRender];
	
	/** A map from light ID to a boolean visibility value. */
	TArray<FVisibleLightInfo> VisibleLightInfos;

	/** The view's batched elements, sorted by DPG. */
	FBatchedElements BatchedViewElements[SDPG_MAX_SceneRender];

	/** The view's mesh elements, sorted by DPG. */
	TIndirectArray<FHitProxyMeshPair> ViewMeshElements[SDPG_MAX_SceneRender];

	/** TRUE if the DPG has at least one mesh in ViewMeshElements[DPGIndex] with a translucent material. */
	BITFIELD bHasTranslucentViewMeshElements : SDPG_MAX_SceneRender;

	/** TRUE if the DPG has at least one mesh in ViewMeshElements[DPGIndex] with a distortion material. */
	BITFIELD bHasDistortionViewMeshElements : SDPG_MAX_SceneRender;

	//<@ ava specific : 2006.8.21 changmin.
	/** TRUE if the DPG has at least one mesh in ViewMeshElements[DPGIndex] with a opaque material. */
	BITFIELD bHasOpaqueViewMeshElements : SDPG_MAX_SceneRender;
	//>@ ava

	/** The dynamic resources used by the view elements. */
	TArray<FDynamicPrimitiveResource*> DynamicResources;

	/** fog params for 4 layers of height fog */
	FLOAT FogMinHeight[4];
	FLOAT FogMaxHeight[4];
	FLOAT FogDistanceScale[4];
	FLOAT FogExtinctionDistance[4];
	FLinearColor FogInScattering[4];
	FLOAT FogStartDistance[4];

	/** Whether FSceneRenderer needs to output velocities during pre-pass. */
	UBOOL					bRequiresVelocities;

	/** Whether FSceneRenderer needs to output depth buffer during emissive/point and sky light. */
	UBOOL					bRequiresDepth;

	/** Whether we should ignore queries from last frame (useful to ignoring occlusions on the first frame after a large camera movement). */
	UBOOL					bIgnoreExistingQueries;

	/** Whether we should submit new queries this frame. (used to disable occlusion queries completely. */
	UBOOL					bDisableQuerySubmissions;

	/** Whether the scene color is contained in the LDR surface */
	UBOOL					bUseLDRSceneColor;

	FMatrix					PrevViewProjMatrix;
	FMotionBlurParameters	MotionBlurParameters;

    /** Post process render proxies */
    TIndirectArray<FPostProcessSceneProxy> PostProcessSceneProxies;

	/** An intermediate number of visible static meshes.  Doesn't account for occlusion until after FinishOcclusionQueries is called. */
	INT NumVisibleStaticMeshElements;

	/** An intermediate number of visible dynamic primitives.  Doesn't account for occlusion until after FinishOcclusionQueries is called. */
	INT NumVisibleDynamicPrimitives;

	INT NumVisibleSeeThroughStaticMeshElements;
	INT NumVisibleSeeThroughDynamicPrimitives;

	FOcclusionQueryBatcher IndividualOcclusionQueries;
	FOcclusionQueryBatcher GroupedOcclusionQueries;

	//<@ ava specific ; 2008. 1.18 changmin
	// add cascaded shadow
	AvaCasterOcclusionQueryBatcher IndividualCasterOcclusionQueries;
	AvaCasterOcclusionQueryBatcher GroupedCasterOcclusionQueries;
	//>@ ava

	/** 
	* Initialization constructor. Passes all parameters to FSceneView constructor
	*/
	FViewInfo(
		const FSceneViewFamily* InFamily,
		FSceneViewStateInterface* InState,
		FSynchronizedActorVisibilityHistory* InHistory,
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
		FLOAT InLODDistanceFactor = 1.0f
		);

	/** 
	* Initialization constructor. 
	* @param InView - copy to init with
	*/
	explicit FViewInfo(const FSceneView* InView);

	/** 
	* Destructor. 
	*/
	~FViewInfo();

	/** 
	* Initializes the dynamic resources used by this view's elements. 
	*/
	void InitDynamicResources();
};


/**
 * Used to hold combined stats for a shadow. In the case of shadow volumes the ShadowResolution remains
 * at INDEX_NONE and the Subjects array has a single entry. In the case of projected shadows the shadows
 * for the preshadow and subject are combined in this stat and so are primitives with a shadow parent.
 */
struct FCombinedShadowStats
{
	/** Array of shadow subjects. The first one is the shadow parent in the case of multiple entries.	*/
	TArray<const FPrimitiveSceneInfo*>	SubjectPrimitives;
	/** Array of preshadow primitives in the case of projected shadows.									*/
	TArray<const FPrimitiveSceneInfo*>	PreShadowPrimitives;
	/** Shadow resolution in the case of projected shadows, INDEX_NONE for shadow volumes.				*/
	INT									ShadowResolution;
	/** Shadow pass number in the case of projected shadows, INDEX_NONE for shadow volumes.				*/
	INT									ShadowPassNumber;

	/**
	 * Default constructor, initializing ShadowResolution for shadow volume case. 
	 */
	FCombinedShadowStats()
	:	ShadowResolution(INDEX_NONE)
	,	ShadowPassNumber(INDEX_NONE)
	{}
};

/**
* Global render state 
*/
class FGlobalSceneRenderState
{
public:
	FGlobalSceneRenderState() : FrameNumber(0) {}
	/** Incremented once per frame before the first scene is being rendered */
	UINT FrameNumber;
};

/**
 * Used as the scope for scene rendering functions.
 * It is initialized in the game thread by FSceneViewFamily::BeginRender, and then passed to the rendering thread.
 * The rendering thread calls Render(), and deletes the scene renderer when it returns.
 */
class FSceneRenderer
{
public:

	/** The scene being rendered. */
	FScene* Scene;

	/** The view family being rendered.  This references the Views array. */
	FSceneViewFamily ViewFamily;

	/** The views being rendered. */
	TArray<FViewInfo> Views;

	/** The visible projected shadows. */
	TIndirectArray<FProjectedShadowInfo> ProjectedShadows;

	//<@ ava specific ; 2007. 10. 1 changmin
	// add cascaded shadows
	TIndirectArray<FCascadedShadowInfo> CascadedShadows;
	//>@ ava

	/** The canvas transform used to render the scene. */
	FMatrix CanvasTransform;

	/** The width in screen pixels of the view family being rendered. */
	UINT FamilySizeX;

	/** The height in screen pixels of the view family being rendered. */
	UINT FamilySizeY;

	/** If a freeze request has been made */
	UBOOL bHasRequestedToggleFreeze;

	/** The global RHI context */
	FCommandContextRHI* GlobalContext;

	/** Initialization constructor. */
	FSceneRenderer(const FSceneViewFamily* InViewFamily,FHitProxyConsumer* HitProxyConsumer,const FMatrix& InCanvasTransform);

	/** Destructor, stringifying stats if stats gathering was enabled. */
	~FSceneRenderer();

	/** Renders the view family. */
	void Render();

	/** Renders only the final post processing for the view */
	void RenderPostProcessOnly();

	/** Render the view family's hit proxies. */
	void RenderHitProxies();

	/** Renders the scene to capture target textures */
	void RenderSceneCaptures();

	/** 
	* Global state shared by all FSceneRender instances 
	* @return global state
	*/
	FGlobalSceneRenderState* GetGlobalSceneRenderState();

private:
	/** Map from light primitive interaction to its combined shadow stats. Only used during stats gathering. */
	TMap<FLightPrimitiveInteraction*,FCombinedShadowStats> InteractionToDynamicShadowStatsMap;

	/** Whether we should be gathering dynamic shadow stats this frame. */
	UBOOL bShouldGatherDynamicShadowStats;

	/** The world time of the previous frame. */
	FLOAT PreviousFrameTime;

	/**
	 * Creates a projected shadow for light-primitive interaction.
	 * @param Interaction - The interaction to create a shadow for.
	 * @param OutPreShadows - If the primitive has a preshadow, CreateProjectedShadow adds it to OutPreShadows.
	 */
	void CreateProjectedShadow(const FLightPrimitiveInteraction* Interaction,TArray<FProjectedShadowInfo*>& OutPreShadows);

	//<@ ava specific ; 2007. 9. 20 changmin
	// add cascaded shadow
	void Ava_CreateCascadedShadows( const FLightSceneInfo *LightSceneInfo );

	UBOOL Ava_RenderLightsUseCascadedShadow(UINT DPGIndex);

	UBOOL Ava_RenderCascadedShadows( const FLightSceneInfo *LightSceneInfo, UBOOL bRenderDepth );
	//>@ ava


	/** Finds the visible dynamic shadows for each view. */
	void InitDynamicShadows();

	/** Determines which primitives are visible for each view. */
	void InitViews();

	/** Initialized the fog constants for each view. */
	void InitFogConstants();

	/**
	 * Renders the scene's prepass and occlusion queries.
	 * If motion blur is enabled, it will also render velocities for dynamic primitives to the velocity buffer and
	 * flag those pixels in the stencil buffer.
	 */
	UBOOL RenderPrePass(UINT DPGIndex,UBOOL bIsOcclusionTesting);

	/** Renders the scene's emissive pass */
	UBOOL RenderEmissive(UINT DPGIndex);

	UBOOL RenderSeeThrough(UINT DPGIndex);

	/** Renders the scene's fogging. */
	UBOOL RenderFog(UINT DPGIndex);

	/** bound shader state for full-screen fog pass */
	static FGlobalBoundShaderStateRHIRef FogBoundShaderState;

	/** bound shader state for combining LDR translucency with scene color */
	static FGlobalBoundShaderStateRHIRef LDRCombineBoundShaderState;

	/** Renders the scene's lighting. */
	UBOOL RenderLights(UINT DPGIndex);

	/** Renders the scene's lighting. */
	UBOOL PreRenderLights(UINT DPGIndex);

	/** Renders the scene's distortion */
	UBOOL RenderDistortion(UINT DPGIndex);

	//<@ ava specific ; 2007. 11. 20 changmin
	UBOOL AVA_RenderLitDecals(UINT DPGIndex);
	//>@ ava
	
	/** 
	 * Renders the scene's translucency.
	 *
	 * @param	DPGIndex	Current DPG used to draw items.
	 * @return				TRUE if anything was drawn.
	 */
	UBOOL RenderTranslucency(UINT DPGIndex);

	/** 
	* Combines the recently rendered LDR translucency with scene color.
	*
	* @param	Context	 The RHI command context the primitives are being rendered to. 
	*/
	void CombineLDRTranslucency(FCommandContextRHI* Context);

	/** 
	 * Renders the scene's decals.
	 *
	 * @param	DPGIndex					Current DPG used to draw items.
	 * @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	 * @param	bPreFog						TRUE if the draw call is occurring before fog has been rendered.
	 * @return								TRUE if anything was drawn.
	 */
	UBOOL RenderDecals(UINT DPGIndex, UBOOL bTranslucentReceiverPass, UBOOL bPreFog);

	/** Renders the velocities of movable objects for the motion blur effect. */
	void RenderVelocities(UINT DPGIndex);

	/** Renders world-space texture density instead of the normal color. */
	UBOOL RenderTextureDensities(UINT DPGIndex);

	//<@ ava specific ; 2007. 1. 15 changmin
	UBOOL RenderViewSpaceNormals(UINT DPGIndex);
	//>@ ava

	//<@ ava specific ; 2007. 10. 16 changmin
	UBOOL RenderBumpLighting(UINT DPGIndex);
	//>@ ava

	/** Begins the occlusion tests for a view. */
	void BeginOcclusionTests(FViewInfo& View);
	/** bound shader state for occlusion test prims */
	static FBoundShaderStateRHIRef OcclusionTestBoundShaderState;

	/** Renders the post process effects for a view. */
	void RenderPostProcessEffects(UINT DPGIndex);

	/**
	 * Post processes a view, writing the contents to ViewFamily.RenderTarget.
	 * @param View - The view to process.
	*/
	void PostProcessView(const FViewInfo* View);

	/**
	 * Finish rendering a view, writing the contents to ViewFamily.RenderTarget.
	 * @param View - The view to process.
	*/
	void FinishRenderViewTarget(const FViewInfo* View);

	/**
	 * Measure luminance of view :)
	 * @param View - The view to process.
	 */
	void MeasureLuminance(const FViewInfo* View);

	/** bound shader state for full-screen gamma correction pass */
	static FGlobalBoundShaderStateRHIRef PostProcessBoundShaderState;
	static FGlobalBoundShaderStateRHIRef AutomaticExposureBoundShaderState;
	static FGlobalBoundShaderStateRHIRef EncodeSceneColorBoundShaderState;
	static FGlobalBoundShaderStateRHIRef ColorCorrectionBoundShaderState[4];
	static FGlobalBoundShaderStateRHIRef ColorCorrectionSampleBoundShaderState;

	/**
	  * Used by RenderLights to figure out if projected shadows need to be rendered to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything needs to be rendered
	  */
	UBOOL CheckForProjectedShadows( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render projected shadows to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderProjectedShadows( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to figure out if shadow volumes need to be rendered to the stencil buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything needs to be rendered
	  */
	UBOOL CheckForShadowVolumes( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render shadow volumes to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @param LightIndex The light's index into FScene::Lights
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderShadowVolumes( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	* Used by RenderLights to figure out if shadow volumes need to be rendered to the attenuation buffer.
	*
	* @param LightSceneInfo Represents the current light
	* @return TRUE if anything needs to be rendered
	*/
	UBOOL CheckForShadowVolumeAttenuation( const FLightSceneInfo* LightSceneInfo );

	/**
	* Attenuate the shadowed area of a shadow volume. For use with modulated shadows
	* @param LightSceneInfo - Represents the current light
	* @return TRUE if anything got rendered
	*/
	UBOOL AttenuateShadowVolumes( const FLightSceneInfo* LightSceneInfo );

	/**
	  * Used by RenderLights to figure out if light functions need to be rendered to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @return TRUE if anything got rendered
	  */
	UBOOL CheckForLightFunction( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render a light function to the attenuation buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @param LightIndex The light's index into FScene::Lights
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderLightFunction( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	  * Used by RenderLights to render a light to the scene color buffer.
	  *
	  * @param LightSceneInfo Represents the current light
	  * @param LightIndex The light's index into FScene::Lights
	  * @return TRUE if anything got rendered
	  */
	UBOOL RenderLight( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex );

	/**
	 * Renders all the modulated shadows to the scene color buffer.
	 * @param	DPGIndex					Current DPG used to draw items.
	 * @return TRUE if anything got rendered
	 */
	UBOOL RenderModulatedShadows(UINT DPGIndex);

	/* AVA specific */
	UBOOL PreRenderModulatedShadows(UINT DPGIndex);

	/**
	* Clears the scene color depth (stored in alpha channel) to max depth
	* This is needed for depth bias blend materials to show up correctly
	*/
	void ClearSceneColorDepth();

	/** Saves the actor and primitive visibility states for the game thread. */
	void SaveVisibilityState();
};

/**
 * An implementation of the dynamic primitive definition interface to draw the elements passed to it on a given RHI command interface.
 */
template<typename DrawingPolicyFactoryType>
class TDynamicPrimitiveDrawer : public FPrimitiveDrawInterface
{
public:

	TDynamicPrimitiveDrawer(
		FCommandContextRHI* InContext,
		const FViewInfo* InView,
		UINT InDPGIndex,
		const typename DrawingPolicyFactoryType::ContextType& InDrawingContext,
		UBOOL InPreFog
		):
		FPrimitiveDrawInterface(InView),
		View(InView),
		Context(InContext),
		DPGIndex(InDPGIndex),
		DrawingContext(InDrawingContext),
		bPreFog(InPreFog),
		bDirty(FALSE)
	{}

	~TDynamicPrimitiveDrawer()
	{
		if(View)
		{
			// Draw the batched elements.
			BatchedElements.Draw(
				Context,
				View->ViewProjectionMatrix,
				appTrunc(View->SizeX),
				appTrunc(View->SizeY),
				(View->Family->ShowFlags & SHOW_HitProxies) != 0
				);
		}

		// Cleanup the dynamic resources.
		for(INT ResourceIndex = 0;ResourceIndex < DynamicResources.Num();ResourceIndex++)
		{
			//release the resources before deleting, they will delete themselves
			DynamicResources(ResourceIndex)->ReleasePrimitiveResource();
		}
	}

	void SetPrimitive(const FPrimitiveSceneInfo* NewPrimitiveSceneInfo)
	{
		PrimitiveSceneInfo = NewPrimitiveSceneInfo;
		HitProxyId = PrimitiveSceneInfo->DefaultDynamicHitProxyId;
	}

	// FPrimitiveDrawInterface interface.

	virtual UBOOL IsHitTesting()
	{
		return FALSE;
	}

	virtual void SetHitProxy(HHitProxy* HitProxy)
	{
		if(HitProxy)
		{
			// Only allow hit proxies from CreateHitProxies.
			checkMsg(PrimitiveSceneInfo->HitProxies.FindItemIndex(HitProxy) != INDEX_NONE,"Hit proxy used in DrawDynamicElements which wasn't created in CreateHitProxies");
			HitProxyId = HitProxy->Id;
		}
		else
		{
			HitProxyId = FHitProxyId();
		}
	}

	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource)
	{
		// Add the dynamic resource to the list of resources to cleanup on destruction.
		DynamicResources.AddItem(DynamicResource);

		// Initialize the dynamic resource immediately.
		DynamicResource->InitPrimitiveResource();
	}

	virtual UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance) const
	{
		return DrawingPolicyFactoryType::IsMaterialIgnored(MaterialInstance);
	}

	virtual void DrawMesh(const FMeshElement& Mesh)
	{
		if( Mesh.DepthPriorityGroup == DPGIndex )
		{
			const UBOOL bIsTwoSided = Mesh.MaterialInstance->GetMaterial()->IsTwoSided();
			const UBOOL bIsNonDirectionalLighting = Mesh.MaterialInstance->GetMaterial()->GetLightingModel() == MLM_NonDirectional;
			const UBOOL bNeedsBackfacePass = bIsTwoSided && !bIsNonDirectionalLighting;
			for(INT bBackFace = 0;bBackFace < (bNeedsBackfacePass ? 2 : 1);bBackFace++)
			{
				bDirty |= DrawingPolicyFactoryType::DrawDynamicMesh(
					Context,
					View,
					DrawingContext,
					Mesh,
					bBackFace,
					bPreFog,
					PrimitiveSceneInfo,
					HitProxyId
					);
			}
		}
	}

	virtual void DrawSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Sprite,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		)
	{
		if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
		{
			BatchedElements.AddSprite(Position,SizeX,SizeY,Sprite,Color,HitProxyId);
			bDirty = TRUE;
		}
	}

	virtual void DrawLine(
		const FVector& Start,
		const FVector& End,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		)
	{
		if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
		{
			BatchedElements.AddLine(Start,End,Color,HitProxyId);
			bDirty = TRUE;
		}
	}

	virtual void DrawPoint(
		const FVector& Position,
		const FLinearColor& Color,
		FLOAT PointSize,
		BYTE DepthPriorityGroup
		)
	{
		if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
		{
			BatchedElements.AddPoint(Position,PointSize,Color,HitProxyId);
			bDirty = TRUE;
		}
	}

	//<@ ava specific ; 2008. 2. 27 changmin
	virtual INT Ava_AddVertex( const FVector& Position, const FVector2D& UV, const FLinearColor& Color, BYTE DepthPriorityGroup )
	{
		INT VertexIndex = INDEX_NONE;
		if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
		{
			VertexIndex = BatchedElements.AddVertex( FVector4( Position, 1.0f ), UV, Color, HitProxyId );
			bDirty = TRUE;
		}
		return VertexIndex;
	}

	virtual void Ava_AddTriangle( INT V0, INT V1, INT V2, const FTexture* Texture, EBlendMode BlendMode, BYTE DepthPriorityGroup )
	{
		if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
		{
			BatchedElements.AddTriangle( V0, V1, V2, Texture, BlendMode );
			bDirty = TRUE;
		}
	}
	//>@ ava

	// Accessors.
	/**
	 * @return		TRUE if fog has not yet been rendered.
	 */
	UBOOL IsPreFog() const
	{
		return bPreFog;
	}

	/**
	 * @return		TRUE if any elements have been rendered by this drawer.
	 */
	UBOOL IsDirty() const
	{
		return bDirty;
	}

private:
	/** The view which is being rendered. */
	const FViewInfo* const View;

	/** The RHI command context the primitives are being rendered to. */
	FCommandContextRHI* Context;

	/** The DPG which is being rendered. */
	const UINT DPGIndex;

	/** The drawing context passed to the drawing policy for the mesh elements rendered by this drawer. */
	typename DrawingPolicyFactoryType::ContextType DrawingContext;

	/** The primitive being rendered. */
	const FPrimitiveSceneInfo* PrimitiveSceneInfo;

	/** The current hit proxy ID being rendered. */
	FHitProxyId HitProxyId;

	/** The batched simple elements. */
	FBatchedElements BatchedElements;

	/** The dynamic resources which have been registered with this drawer. */
	TArray<FDynamicPrimitiveResource*> DynamicResources;

	/** TRUE if fog has not yet been rendered. */
	BITFIELD bPreFog : 1;

	/** Tracks whether any elements have been rendered by this drawer. */
	BITFIELD bDirty : 1;
};

/**
 * Draws a view's elements with the specified drawing policy factory type.
 * @param Context - The RHI command interface to execute the draw commands on.
 * @param View - The view to draw the meshes for.
 * @param DrawingContext - The drawing policy type specific context for the drawing.
 * @param DPGIndex - The depth priority group to draw the elements from.
 * @param bPreFog - TRUE if the draw call is occurring before fog has been rendered.
 */
template<class DrawingPolicyFactoryType>
UBOOL DrawViewElements(
	FCommandContextRHI* Context,
	const FViewInfo* View,
	const typename DrawingPolicyFactoryType::ContextType& DrawingContext,
	UINT DPGIndex,
	UBOOL bPreFog
	)
{
	// Draw the view's mesh elements.
	for(INT MeshIndex = 0;MeshIndex < View->ViewMeshElements[DPGIndex].Num();MeshIndex++)
	{
		const FHitProxyMeshPair& Mesh = View->ViewMeshElements[DPGIndex](MeshIndex);
		const UBOOL bIsTwoSided = Mesh.MaterialInstance->GetMaterial()->IsTwoSided();
		const UBOOL bIsNonDirectionalLighting = Mesh.MaterialInstance->GetMaterial()->GetLightingModel() == MLM_NonDirectional;
		const UBOOL bNeedsBackfacePass = bIsTwoSided && !bIsNonDirectionalLighting;
		for(INT bBackFace = 0;bBackFace < (bNeedsBackfacePass ? 2 : 1);bBackFace++)
		{
			DrawingPolicyFactoryType::DrawDynamicMesh(
				Context,
				View,
				DrawingContext,
				Mesh,
				bBackFace,
				bPreFog,
				NULL,
				Mesh.HitProxyId
				);
		}
	}

	return View->ViewMeshElements[DPGIndex].Num() != 0;
}

/**
 * Draws a given set of dynamic primitives to a RHI command interface, using the specified drawing policy type.
 * @param Context - The RHI command interface to execute the draw commands on.
 * @param View - The view to draw the meshes for.
 * @param DrawingContext - The drawing policy type specific context for the drawing.
 * @param DPGIndex - The depth priority group to draw the elements from.
 * @param bPreFog - TRUE if the draw call is occurring before fog has been rendered.
 */
template<class DrawingPolicyFactoryType>
UBOOL DrawDynamicPrimitiveSet(
	FCommandContextRHI* Context,
	const FViewInfo* View,
	const typename DrawingPolicyFactoryType::ContextType& DrawingContext,
	UINT DPGIndex,
	UBOOL bPreFog
	)
{
	// Draw the view's elements.
	UBOOL bDrewViewElements = DrawViewElements<DrawingPolicyFactoryType>(Context,View,DrawingContext,DPGIndex,bPreFog);

	extern void ParticleRendering_StartBatch( UBOOL bTranslucentPass );
	extern void ParticleRendering_EndBatch( FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex );

	ParticleRendering_StartBatch(FALSE);

	// Draw the elements of each dynamic primitive.
	TDynamicPrimitiveDrawer<DrawingPolicyFactoryType> Drawer(Context,View,DPGIndex,DrawingContext,bPreFog);
	for(INT PrimitiveIndex = 0;PrimitiveIndex < View->VisibleDynamicPrimitives.Num();PrimitiveIndex++)
	{
		const FPrimitiveSceneInfo* PrimitiveSceneInfo = View->VisibleDynamicPrimitives(PrimitiveIndex);

		if(!View->PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
		{
			continue;
		}

		if(!View->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id).GetDPG(DPGIndex))
		{
			continue;
		}

		Drawer.SetPrimitive(PrimitiveSceneInfo);
		PrimitiveSceneInfo->Proxy->DrawDynamicElements(
			&Drawer,
			View,
			DPGIndex
			);
	}

	ParticleRendering_EndBatch(&Drawer, View, DPGIndex);

	return bDrewViewElements || Drawer.IsDirty();
} 

/** A primitive draw interface which adds the drawn elements to the view's batched elements. */
class FViewElementPDI : public FPrimitiveDrawInterface
{
public:

	FViewElementPDI(FViewInfo* InViewInfo,FHitProxyConsumer* InHitProxyConsumer):
		FPrimitiveDrawInterface(InViewInfo),
		ViewInfo(InViewInfo),
		HitProxyConsumer(InHitProxyConsumer)
	{}

	virtual UBOOL IsHitTesting()
	{
		return HitProxyConsumer != NULL;
	}
	virtual void SetHitProxy(HHitProxy* HitProxy)
	{
		// Change the current hit proxy.
		CurrentHitProxy = HitProxy;

		if(HitProxyConsumer && HitProxy)
		{
			// Notify the hit proxy consumer of the new hit proxy.
			HitProxyConsumer->AddHitProxy(HitProxy);
		}
	}

	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource)
	{
		ViewInfo->DynamicResources.AddItem(DynamicResource);
	}

	virtual void DrawSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Sprite,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		)
	{
		ViewInfo->BatchedViewElements[DepthPriorityGroup].AddSprite(
			Position,
			SizeX,
			SizeY,
			Sprite,
			Color,
			CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
			);
	}

	virtual void DrawLine(
		const FVector& Start,
		const FVector& End,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		)
	{
		ViewInfo->BatchedViewElements[DepthPriorityGroup].AddLine(Start,End,Color,CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId());
	}

	virtual void DrawPoint(
		const FVector& Position,
		const FLinearColor& Color,
		FLOAT PointSize,
		BYTE DepthPriorityGroup
		)
	{
		ViewInfo->BatchedViewElements[DepthPriorityGroup].AddPoint(
			Position,
			PointSize,
			Color,
			CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
			);
	}

	virtual void DrawMesh(const FMeshElement& Mesh)
	{
		const UINT DepthPriorityGroup = Mesh.DepthPriorityGroup < SDPG_MAX_SceneRender ? Mesh.DepthPriorityGroup : SDPG_World;

		// Keep track of view mesh elements whether that have transluceny or distortion.
		ViewInfo->bHasTranslucentViewMeshElements |= (Mesh.IsTranslucent() << DepthPriorityGroup);
		ViewInfo->bHasDistortionViewMeshElements |= (Mesh.IsDistortion() << DepthPriorityGroup);

		//<@ ava specific : 2006.8.18 changmin
		ViewInfo->bHasOpaqueViewMeshElements |= ( (!Mesh.IsTranslucent()) << DepthPriorityGroup );
		//>@

		new(ViewInfo->ViewMeshElements[DepthPriorityGroup]) FHitProxyMeshPair(
			Mesh,
			CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
			);
	}

	//<@ ava  specific ; 2008. 2.27 changmin
	virtual INT Ava_AddVertex( const FVector& Position, const FVector2D& UV, const FLinearColor& Color, BYTE DepthPriorityGroup )
	{
		return INDEX_NONE;
	}
	virtual void Ava_AddTriangle( INT V0, INT V1, INT V2, const FTexture* Texture, EBlendMode BlendMode, BYTE DepthPriorityGroup)
	{

	}
	//>@ ava

private:
	FViewInfo* ViewInfo;
	TRefCountPtr<HHitProxy> CurrentHitProxy;
	FHitProxyConsumer* HitProxyConsumer;
};
