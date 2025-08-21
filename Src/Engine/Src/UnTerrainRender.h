/*=============================================================================
	UnTerrainRender.h: Definitions and inline code for rendering TerrainComponet
	Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef TERRAIN_RENDER_HEADER
#define TERRAIN_RENDER_HEADER

#include "ScenePrivate.h"

// Forward declarations.
class FDecalState;
class FDynamicTerrainData;
class FTerrainComponentSceneProxy;
struct FTerrainObject;
class UTerrainComponent;

//
//	FTerrainVertexBuffer
//
struct FTerrainVertexBuffer: FVertexBuffer
{
	/**
	 * Constructor.
	 * @param InMeshRenderData pointer to parent structure
	 */
	FTerrainVertexBuffer(FTerrainObject* InTerrainObject, UTerrainComponent* InComponent, INT InMaxTessellation, UBOOL bInIsDynamic = FALSE) :
		  bIsDynamic(bInIsDynamic)
		, TerrainObject(InTerrainObject)
		, Component(InComponent)
		, MaxTessellation(InMaxTessellation)
		, MaxVertexCount(0)
		, CurrentTessellation(-1)
		, VertexCount(0)
		, bRepackRequired(bInIsDynamic)
		, bIsCollisionLevel(FALSE)
	{
	}

	// FRenderResource interface.
	virtual void InitRHI();

	/** 
	 * Initialize the dynamic RHI for this rendering resource 
	 */
	virtual void InitDynamicRHI();

	/** 
	 * Release the dynamic RHI for this rendering resource 
	 */
	virtual void ReleaseDynamicRHI();

	virtual FString GetFriendlyName() const
	{
		return TEXT("Terrain component vertices");
	}

	inline INT GetMaxTessellation()		{	return MaxTessellation;		}
	inline INT GetMaxVertexCount()		{	return MaxVertexCount;		}
	inline INT GetCurrentTessellation()	{	return CurrentTessellation;	}
	inline INT GetVertexCount()			{	return VertexCount;			}
	inline UBOOL GetRepackRequired()	{	return bRepackRequired;		}
	inline void ClearRepackRequired()	{	bRepackRequired = FALSE;	}

	virtual void SetCurrentTessellation(INT InCurrentTessellation)
	{
		CurrentTessellation = Clamp<INT>(InCurrentTessellation, 0, MaxTessellation);
	}

	virtual UBOOL FillData(INT TessellationLevel);

	void SetIsCollisionLevel(UBOOL bIsCollision)
	{
		bIsCollisionLevel = bIsCollision;
	}

private:
	/** Flag indicating it is dynamic						*/
	UBOOL				bIsDynamic;
	/** The owner terrain object							*/
	FTerrainObject*		TerrainObject;
	/** The 'owner' component								*/
	UTerrainComponent*	Component;
	/** The maximum tessellation to create vertices for		*/
	INT					MaxTessellation;
	/** The maximum number of vertices in the buffer		*/
	INT					MaxVertexCount;
	/** The maximum tessellation to create vertices for		*/
	INT					CurrentTessellation;
	/** The number of vertices in the buffer				*/
	INT					VertexCount;
	/** A repack is required								*/
	UBOOL				bRepackRequired;
	/** Flag indicating it is for rendering the collision	*/
	UBOOL				bIsCollisionLevel;
};


//
//	FTerrainFullVertexBuffer
//
struct FTerrainFullVertexBuffer : FTerrainVertexBuffer
{
	/**
	 * Constructor.
	 * @param InMeshRenderData pointer to parent structure
	 */
	FTerrainFullVertexBuffer(FTerrainObject* TerrainObject, UTerrainComponent* InComponent, INT InMaxTessellation) :
		  FTerrainVertexBuffer(TerrainObject, InComponent, InMaxTessellation, FALSE)
	{
	}

	// FRenderResource interface.
	virtual void InitRHI()
	{
		FTerrainVertexBuffer::InitRHI();
	}

	/** 
	 * Initialize the dynamic RHI for this rendering resource 
	 */
	virtual void InitDynamicRHI()
	{
		// Do NOTHING for the FullVertexBuffer
	}

	/** 
	 * Release the dynamic RHI for this rendering resource 
	 */
	virtual void ReleaseDynamicRHI()
	{
		// Do NOTHING for the FullVertexBuffer
	}

	virtual FString GetFriendlyName() const
	{
		return TEXT("Terrain FULL component vertices");
	}

	virtual void SetCurrentTessellation(INT InCurrentTessellation)
	{
		// Do NOTHING for the FullVertexBuffer
	}

	virtual UBOOL FillData(INT TessellationLevel)
	{
		return FTerrainVertexBuffer::FillData(TessellationLevel);
	}
};

//
//	FTerrainIndexBuffer declaration
//
struct FTerrainIndexBuffer;

//
//	FTerrainTessellationIndexBuffer declaration
//
struct FTerrainTessellationIndexBuffer;

//
//	FTerrainObject
//
struct FTerrainObject : public FDeferredCleanupInterface
{
public:
	FTerrainObject(UTerrainComponent* InTerrainComponent, INT InMaxTessellation) :
		  bIsInitialized(FALSE)
		, bIsDeadInGameThread(FALSE)
		, bIsShowingCollision(FALSE)
	    , TerrainComponent(InTerrainComponent)
		, TessellationLevels(NULL)
		, VertexFactory(NULL)
		, DecalVertexFactory(NULL)
		, VertexBuffer(NULL)
		, FullVertexBuffer(NULL)
		, FullIndexBuffer(NULL)
		, CollisionVertexFactory(NULL)
		, CollisionVertexBuffer(NULL)
		, CollisionSmoothIndexBuffer(NULL)
	{
		check(TerrainComponent);
		if (TerrainComponent->GetTerrain())
		{
			bIsShowingCollision = TerrainComponent->GetTerrain()->bShowingCollision;
		}
		Init();
//		InitResources();
	}
	
	virtual ~FTerrainObject();

	void Init();

	virtual void InitResources();
	virtual void ReleaseResources();
	virtual void Update();
	virtual const FVertexFactory* GetVertexFactory() const;

	//@todo.SAS. Remove this! START
	UBOOL GetIsDeadInGameThread()
	{
		return bIsDeadInGameThread;
	}
	void SetIsDeadInGameThread(UBOOL bInIsDeadInGameThread)
	{
		bIsDeadInGameThread = bInIsDeadInGameThread;
	}
	//@todo.SAS. Remove this! STOP

#if 1	//@todo. Remove these as we depend on the component anyway!
	inline INT		GetComponentSectionSizeX()		{	return ComponentSectionSizeX;		}
	inline INT		GetComponentSectionSizeY()		{	return ComponentSectionSizeY;		}
	inline INT		GetComponentSectionBaseX()		{	return ComponentSectionBaseX;		}
	inline INT		GetComponentSectionBaseY()		{	return ComponentSectionBaseY;		}
	inline INT		GetComponentTrueSectionSizeX()	{	return ComponentTrueSectionSizeX;	}
	inline INT		GetComponentTrueSectionSizeY()	{	return ComponentTrueSectionSizeY;	}
#endif	//#if 1	//@todo. Remove these as we depend on the component anyway!
	inline INT		GetNumVerticesX()				{	return NumVerticesX;				}
	inline INT		GetNumVerticesY()				{	return NumVerticesY;				}
	inline INT		GetMaxTessellationLevel()		{	return MaxTessellationLevel;		}
	inline FLOAT	GetTerrainHeightScale()			{	return TerrainHeightScale;			}
	inline FLOAT	GetTessellationDistanceScale()	{	return TessellationDistanceScale;	}
	inline BYTE		GetTessellationLevel(INT Index)	{	return TessellationLevels[Index];	}
	inline INT		GetLightMapResolution()			{	return LightMapResolution;			}
	inline FLOAT	GetShadowCoordinateScaleX()		{	return ShadowCoordinateScale.X;		}
	inline FLOAT	GetShadowCoordinateScaleY()		{	return ShadowCoordinateScale.Y;		}
	inline FLOAT	GetShadowCoordinateBiasX()		{	return ShadowCoordinateBias.X;		}
	inline FLOAT	GetShadowCoordinateBiasY()		{	return ShadowCoordinateBias.Y;		}

	inline void		SetShadowCoordinateScale(const FVector2D& InShadowCoordinateScale)
	{
		ShadowCoordinateScale = InShadowCoordinateScale;
	}
	inline void		SetShadowCoordinateBias(const FVector2D& InShadowCoordinateBias)
	{
		ShadowCoordinateBias = InShadowCoordinateBias;
	}

	UBOOL UpdateResources(INT TessellationLevel, UBOOL bRepackRequired);

	// FDeferredCleanupInterface
	virtual void FinishCleanup()
	{
		delete this;
	}

	// allow access to mesh component
	friend class FDynamicTerrainData;
	friend class FTerrainComponentSceneProxy;
	friend struct FTerrainIndexBuffer;
	friend struct FTerrainTessellationIndexBuffer;
	friend struct FTerrainDetessellationIndexBuffer;

protected:
	/** Set to TRUE in InitResources() and FALSE in ReleaseResources()	*/
	UBOOL					bIsInitialized;
	/** Debugging flag...												*/
	UBOOL					bIsDeadInGameThread;
	/** Showing collision flag...										*/
	UBOOL					bIsShowingCollision;
	/** The owner component												*/
	UTerrainComponent*		TerrainComponent;

	/** The component section size and base (may not need these...)		*/
#if 1	//@todo. Remove these as we depend on the component anyway!
	INT						ComponentSectionSizeX;
	INT						ComponentSectionSizeY;
	INT						ComponentSectionBaseX;
	INT						ComponentSectionBaseY;
	INT						ComponentTrueSectionSizeX;
	INT						ComponentTrueSectionSizeY;
#endif	//#if 1	//@todo. Remove these as we depend on the component anyway!
	INT						NumVerticesX;
	INT						NumVerticesY;
	/** The maximum tessellation level of the terrain					*/
	INT						MaxTessellationLevel;
	/** The minimum tessellation level of the terrain					*/
	INT						MinTessellationLevel;
	/** The editor-desired tessellation level to display at				*/
	INT						EditorTessellationLevel;
	FLOAT					TerrainHeightScale;
	FLOAT					TessellationDistanceScale;
	INT						LightMapResolution;
	FVector2D				ShadowCoordinateScale;
	FVector2D				ShadowCoordinateBias;

	/** The TessellationLevels arrays (per-batch)						*/
	BYTE*								TessellationLevels;

	/** The vertex factory												*/
	FTerrainVertexFactory*				VertexFactory;
	/** The decal vertex factory										*/
	FTerrainDecalVertexFactory*			DecalVertexFactory;
	/** The vertex buffer containing the vertices for the component		*/
	FTerrainVertexBuffer*				VertexBuffer;
	/** The index buffers for each batch material						*/
	FTerrainTessellationIndexBuffer*	SmoothIndexBuffer;
	/** The material resources for each batch							*/
	TArray<FMaterialInstance*>			BatchMaterialResources;

	/** For rendering at full-patch (lowest tessellation)				*/
	FTerrainVertexFactory				FullVertexFactory;
	FTerrainDecalVertexFactory			FullDecalVertexFactory;
	FTerrainFullVertexBuffer*			FullVertexBuffer;
	FTerrainIndexBuffer*				FullIndexBuffer;
	FMaterialInstance*					FullMaterialResource;

	// For rendering the collision...
	/** The vertex factory												*/
	FTerrainVertexFactory*				CollisionVertexFactory;
	/** The vertex buffer containing the vertices for the component		*/
	FTerrainVertexBuffer*				CollisionVertexBuffer;
	/** The index buffers for each batch material						*/
	FTerrainTessellationIndexBuffer*	CollisionSmoothIndexBuffer;
};

//
//	FTerrainIndexBuffer
//
struct FTerrainIndexBuffer: FIndexBuffer
{
	FTerrainObject* TerrainObject;
	INT	SectionSizeX;
	INT	SectionSizeY;
	INT NumVisibleTriangles;

	// Constructor.
	FTerrainIndexBuffer(FTerrainObject* InTerrainObject) :
		  TerrainObject(InTerrainObject)
		, SectionSizeX(InTerrainObject->GetComponentSectionSizeX())
		, SectionSizeY(InTerrainObject->GetComponentSectionSizeY())
		, NumVisibleTriangles(INDEX_NONE)
	{
	}

	// FRenderResource interface.
	virtual void InitRHI();

	virtual FString GetFriendlyName() const
	{
		return TEXT("Terrain component indices (full batch)");
	}
};

/** An instance of a foliage mesh. */
struct FTerrainFoliageInstance
{
	FVector Location;
};

/** A vertex of a foliage instance. */
struct FTerrainFoliageInstanceVertex
{
	FVector Position;
	FVector2D TexCoord;
	FVector2D ShadowMapCoord;
	FPackedNormal TangentX;
	FPackedNormal TangentY;
	FPackedNormal TangentZ;
};

/** An index buffer containing the triangles of a foliage instance list. */
class FTerrainFoliageIndexBuffer : public FIndexBuffer
{
public:

	/** Initialization constructor. */
	FTerrainFoliageIndexBuffer(const class FComponentFoliageInstanceList& InInstanceList);

	  // FRenderResource interface.
	  virtual void InitRHI();
private:

	const FComponentFoliageInstanceList& InstanceList;
	const TResourceArray<WORD,TRUE,INDEXBUFFER_ALIGNMENT>& MeshIndices;

	const UINT NumVerticesPerInstance;
};

/** A vertex buffer containing the vertices of a foliage instance list. */
class FTerrainFoliageVertexBuffer: public FVertexBuffer
{
public:

	/** Initialization constructor. */
	FTerrainFoliageVertexBuffer(const class FComponentFoliageInstanceList& InInstanceList);

	// FRenderResource interface.
	virtual void InitRHI();
	virtual FString GetFriendlyName() const { return TEXT("Terrain foliage vertices"); }

	/**
	 * Writes the vertices for a specific view point to the vertex buffer.
	 * @param VisibleFoliageInstances - The indices of the foliage instances which are visible in the view.
	 * @param ViewOrigin - The origin of the view.
	 */
	void WriteVertices(const TArray<INT>& VisibleFoliageInstances,const FVector& ViewOrigin);

private:

	const FComponentFoliageInstanceList& InstanceList;
	const FTerrainFoliageMesh* const Mesh;
};

/** Information about the component's instances of a particular foliage mesh. */
class FComponentFoliageInstanceList
{
public:

	/** The mesh that is being instanced. */
	const FTerrainFoliageMesh* Mesh;

	/** All the component's instances of the mesh. */
	TArray<FTerrainFoliageInstance>	Instances;

	/** The vertices for all the component's instances of the mesh. */
	TArray<FTerrainFoliageInstanceVertex> Vertices;

	/** The vertex buffer used to hold the component's foliage instance vertices. */
	FTerrainFoliageVertexBuffer VertexBuffer;

	/** The index buffer used to hold the component's foliage instance indices. */
	FTerrainFoliageIndexBuffer IndexBuffer;

	/** The vertex factory used to render the component's foliage instances. */
	FLocalVertexFactory FoliageVertexFactory;

	/** Initialization constructor. */
	FComponentFoliageInstanceList(const FTerrainFoliageMesh* InMesh):
		Mesh(InMesh),
		VertexBuffer(*this),
		IndexBuffer(*this)
	{}

	/** Destructor. */
	~FComponentFoliageInstanceList()
	{
		VertexBuffer.Release();
		IndexBuffer.Release();
		FoliageVertexFactory.Release();
	}

	/**
	* Creates the foliage instances for this mesh+component pair.  Fills in the Instances and Vertices arrays.
	* @param Mesh - The mesh being instances.
	* @param Component - The terrain component which the mesh is being instanced on.
	* @param WeightedMaterial - The terrain weightmap which controls this foliage mesh's density.
	*/
	void Create(const FTerrainFoliageMesh* Mesh,const UTerrainComponent* Component,const FTerrainWeightedMaterial& WeightedMaterial);

	/** Implements the THashSet key interface for THashSet<FComponentFoliageInstances>. */
	struct KeyFuncs
	{
		typedef const FTerrainFoliageMesh* KeyType;
		typedef const FTerrainFoliageMesh* KeyInitType;
		typedef const FTerrainFoliageMesh* ElementInitType;

		static KeyInitType GetSetKey(ElementInitType Element)
		{
			return Element;
		}

		static KeyInitType GetSetKey(const FComponentFoliageInstanceList& Element)
		{
			return Element.Mesh;
		}

		static UBOOL Matches(KeyInitType A,KeyInitType B)
		{
			return A == B;
		}

		static DWORD GetTypeHash(KeyInitType Key)
		{
			return PointerHash(Key);
	}
};
};

/** Type info for FComponentFoliageInstanceList. */
template<> class TTypeInfo<FComponentFoliageInstanceList>
{
public:
	typedef const FTerrainFoliageMesh* ConstInitType;
	enum { NeedsConstructor = 1	};
	enum { NeedsDestructor = 1	};
};

//
//	FTerrainComponentSceneProxy
//
class FTerrainComponentSceneProxy : public FPrimitiveSceneProxy
{
private:
	class FTerrainComponentInfo : public FLightCacheInterface
	{
	public:

		/** Initialization constructor. */
		FTerrainComponentInfo(const UTerrainComponent& Component)
		{
			// Build the static light interaction map.
			for (INT LightIndex = 0; LightIndex < Component.IrrelevantLights.Num(); LightIndex++)
			{
				StaticLightInteractionMap.Set(Component.IrrelevantLights(LightIndex), FLightInteraction::Irrelevant());
			}
			
			LightMap = Component.LightMap;
			if (LightMap)
			{
				for (INT LightIndex = 0; LightIndex < LightMap->LightGuids.Num(); LightIndex++)
				{
					StaticLightInteractionMap.Set(LightMap->LightGuids(LightIndex), FLightInteraction::LightMap());
				}
			}

			for (INT LightIndex = 0; LightIndex < Component.ShadowMaps.Num(); LightIndex++)
			{
				UShadowMap2D* ShadowMap = Component.ShadowMaps(LightIndex);
				if (ShadowMap && ShadowMap->IsValid())
				{
					StaticLightInteractionMap.Set(
						ShadowMap->GetLightGuid(),
						FLightInteraction::ShadowMap2D(
							ShadowMap->GetTexture(),
							ShadowMap->GetCoordinateScale(),
							ShadowMap->GetCoordinateBias()
							)
						);

					Component.TerrainObject->SetShadowCoordinateBias(ShadowMap->GetCoordinateBias());
					Component.TerrainObject->SetShadowCoordinateScale(ShadowMap->GetCoordinateScale());
				}
			}
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
		{
			// Check for a static light interaction.
			const FLightInteraction* Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightmapGuid);
			if (!Interaction)
			{
				Interaction = StaticLightInteractionMap.Find(LightSceneInfo->LightGuid);
			}
			return Interaction ? *Interaction : FLightInteraction::Uncached();
		}

		virtual const FLightMap* GetLightMap() const
		{
			return LightMap;
		}

	private:
		/** A map from persistent light IDs to information about the light's interaction with the model element. */
		TMap<FGuid,FLightInteraction> StaticLightInteractionMap;

		/** The light-map used by the element. */
		const FLightMap* LightMap;
	};

	/** */
	struct FTerrainBatchInfo
	{
		FTerrainBatchInfo(UTerrainComponent* Component, INT BatchIndex);
		~FTerrainBatchInfo();

        FMaterialInstance* MaterialInstance;
		UBOOL bIsTerrainMaterialResourceInstance;
		TArray<UTexture2D*> WeightMaps;
	};

	struct FTerrainMaterialInfo
	{
		FTerrainMaterialInfo(UTerrainComponent* Component);
		~FTerrainMaterialInfo();
		
		TArray<FTerrainBatchInfo*> BatchInfoArray;
		FTerrainComponentInfo* ComponentLightInfo;
	};

public:
	/** Initialization constructor. */
	FTerrainComponentSceneProxy(UTerrainComponent* Component);
	~FTerrainComponentSceneProxy();

	/** Cacheable ; ava optimiziation */
	UBOOL IsCacheable() const;
	UBOOL IsStillValid( const UPrimitiveComponent* InComponent ) const;

	// FPrimitiveSceneProxy interface.
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex);	

	/**
	 * Draws the primitive's decal elements.  This is called from the rendering thread for each frame of each view.
	 * The dynamic elements will only be rendered if GetViewRelevance declares decal relevance.
	 * Called in the rendering thread.
	 *
	 * @param	Context							The RHI command context to which the primitives are being rendered.
	 * @param	OpaquePDI						The interface which receives the opaque primitive elements.
	 * @param	TranslucentPDI					The interface which receives the translucent primitive elements.
	 * @param	View							The view which is being rendered.
	 * @param	DepthPriorityGroup				The DPG which is being rendered.
	 * @param	bTranslucentReceiverPass		TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	 * @param	bEmissivePass					TRUE if the draw call is occurring after the emissive pass, before lighting.
	 */
	virtual void DrawDecalElements(FCommandContextRHI* Context, FPrimitiveDrawInterface* OpaquePDI, FPrimitiveDrawInterface* TranslucentPDI, const FSceneView* View, UINT DepthPriorityGroup, UBOOL bTranslucentReceiverPass);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		const EShowFlags ShowFlags = View->Family->ShowFlags;
		if (TerrainObject != NULL)
		{
			if (IsShown(View) && (ShowFlags & SHOW_Terrain))
			{
				Result.bDynamicRelevance = TRUE;
				Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
				Result.bDecalRelevance = HasRelevantDecals(View);

				if (TerrainObject->bIsShowingCollision == TRUE)
				{
					Result.bTranslucentRelevance = TRUE;
				}

				//@todo.SAS. Kick off the terrain tessellation in a separate here
			}
			if (IsShadowCast(View) && (ShowFlags & SHOW_Terrain))
			{
				Result.bShadowRelevance = TRUE;
			}
		}
		return Result;
	}

	/**
	 *	Determines the relevance of this primitive's elements to the given light.
	 *	@param	LightSceneInfo			The light to determine relevance for
	 *	@param	bDynamic (output)		The light is dynamic for this primitive
	 *	@param	bRelevant (output)		The light is relevant for this primitive
	 *	@param	bLightMapped (output)	The light is light mapped for this primitive
	 */
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
	{
		// Attach the light to the primitive's static meshes.
		bDynamic = TRUE;
		bRelevant = FALSE;
		bLightMapped = TRUE;

		if (CurrentMaterialInfo)
		{
			if (CurrentMaterialInfo->ComponentLightInfo)
			{
				ELightInteractionType InteractionType = CurrentMaterialInfo->ComponentLightInfo->GetInteraction(LightSceneInfo).GetType();
				if(InteractionType != LIT_CachedIrrelevant)
				{
					bRelevant = TRUE;
				}
				if(InteractionType != LIT_CachedLightMap && InteractionType != LIT_CachedIrrelevant)
				{
					bLightMapped = FALSE;
				}
				if(InteractionType != LIT_Uncached)
				{
					bDynamic = FALSE;
				}
			}
		}
		else
		{
			bRelevant = TRUE;
			bLightMapped = FALSE;
		}
	}

	/**
	 *	Called when the rendering thread adds the proxy to the scene.
	 *	This function allows for generating renderer-side resources.
	 */
	virtual UBOOL CreateRenderThreadResources();

	/**
	 *	Called when the rendering thread removes the dynamic data from the scene.
	 */
	virtual UBOOL ReleaseRenderThreadResources();

	void UpdateData(UTerrainComponent* Component);
	void UpdateData_RenderThread(FTerrainMaterialInfo* NewMaterialInfo);

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }

protected:
	AActor* GetOwner();

private:
	AActor* Owner;
	UTerrainComponent* ComponentOwner;

	FTerrainObject* TerrainObject;

	UBOOL bSelected;

	FColor LevelColor;
	FColor PropertyColor;	

	BITFIELD bCastShadow : 1;

	FColoredMaterialInstance SelectedWireframeMaterialInstance;
	FColoredMaterialInstance DeselectedWireframeMaterialInstance;

	FTerrainMaterialInfo*	CurrentMaterialInfo;

	/** Cache of MaxTessellationLevel, as computed in DrawDynamicElements. */
	UINT MaxTessellation;

	/** Array of meshes, one for each batch material.  Populated by DrawDynamicElements. */
	TArray<FMeshElement> Meshes;

	/** A hashed set container for the component's foliage instances. */
	typedef THashSet<FComponentFoliageInstanceList,FComponentFoliageInstanceList::KeyFuncs> ComponentFoliageInstanceListsType;

	/** The component's foliage instances. */
	ComponentFoliageInstanceListsType ComponentFoliageInstanceLists;

	/** Draws the component's foliage. */
	void DrawFoliage(FPrimitiveDrawInterface* PDI,const FSceneView* View);
};

#endif	//#ifndef TERRAIN_RENDER_HEADER
