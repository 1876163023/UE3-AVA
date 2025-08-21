/*=============================================================================
	Scene.h: Scene manager definitions.
	Copyright 2003-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes the draw mesh macros
#include "UnSceneUtils.h"

// enums are already declared in Engine.h
#define NO_ENUMS 1
#include "EngineSceneClasses.h"
#undef NO_ENUMS

// Forward declarations.
class FSceneViewFamily;
class FSceneInterface;
class FLightSceneInfo;
class ULightComponent;
class USceneCaptureComponent;
class FCaptureSceneInfo;
class UPostProcessChain;

/// added by deif
#if !(CONSOLE || FINAL_RELEASE)
extern UBOOL GIsPlayInEditorWorld_RenderThread;
extern UBOOL GIsGame_RenderThread;
#else
#define GIsPlayInEditorWorld_RenderThread 0
#define GIsGame_RenderThread 1
#endif

//
//	EShowFlags
//

typedef QWORD EShowFlags;

#define	SHOW_MissingCollisionModel	DECLARE_UINT64(0x0000000000000001)	// Show any static meshes with collision on but without a collision model in bright colour.
#define	SHOW_Decals					DECLARE_UINT64(0x0000000000000002)	// Draw decals.
#define	SHOW_DecalInfo				DECLARE_UINT64(0x0000000000000004)	// Draw decal dev info (frustums, tangent axes, etc).
#define	SHOW_LightRadius			DECLARE_UINT64(0x0000000000000008)	// Draw point light radii.
#define	SHOW_AudioRadius			DECLARE_UINT64(0x0000000000000010)	// Draw sound actor radii.
#define	SHOW_DynamicShadows			DECLARE_UINT64(0x0000000000000020)	// Draw dynamic shadows.
#define SHOW_PostProcess			DECLARE_UINT64(0x0000000000000040)  // Draw post process effects
#define SHOW_BSPSplit				DECLARE_UINT64(0x0000000000000080)	// Colors BSP based on model component association.
#define SHOW_SceneCaptureUpdates	DECLARE_UINT64(0x0000000000000100)	// Update scene capture probes
#define SHOW_Sprites				DECLARE_UINT64(0x0000000000000200)	// Draw sprite components
#define SHOW_LevelColoration		DECLARE_UINT64(0x0000000000000400)  // Render objects with colors based on what the level they belong to.

#define SHOW_Wireframe				DECLARE_UINT64(0x0000000000000800)
#define SHOW_Lighting				DECLARE_UINT64(0x0000000000001000)
#define SHOW_Materials				DECLARE_UINT64(0x0000000000002000)
#define SHOW_LightComplexity		DECLARE_UINT64(0x0000000000004000)
#define SHOW_Brushes				DECLARE_UINT64(0x0000000000008000)

#define	SHOW_CollisionNonZeroExtent	DECLARE_UINT64(0x0000000000010000)
#define	SHOW_CollisionZeroExtent	DECLARE_UINT64(0x0000000000020000)
#define	SHOW_CollisionRigidBody		DECLARE_UINT64(0x0000000000040000)
#define SHOW_Collision_Any			(SHOW_CollisionNonZeroExtent | SHOW_CollisionZeroExtent | SHOW_CollisionRigidBody)

#define SHOW_PropertyColoration		DECLARE_UINT64(0x0000000000080000)  // Render objects with colors based on the property values.
#define SHOW_StreamingBounds		DECLARE_UINT64(0x0000000000100000)	// Render streaming bounding volumes for the currently selected texture

#define SHOW_TextureDensity			DECLARE_UINT64(0x0000000000200000)	// Colored according to world-space texture density.

#define SHOW_TerrainCollision		DECLARE_UINT64(0x0000000000400000)	// Renders the terrain collision mesh overlay
#define SHOW_LensFlares				DECLARE_UINT64(0x0000000000800000)	// Renders LensFlares

//<@ ava specific ; 
#define SHOW_SolidBspOnly			DECLARE_UINT64(0x0000000001000000)
#define SHOW_ViewSpaceNormal		DECLARE_UINT64(0X0000000002000000)
#define SHOW_BumpLighting			DECLARE_UINT64(0X0000000004000000)
#define SHOW_LeafColoration			DECLARE_UINT64(0x0000000010000000)
#define SHOW_Ambientcubes			DECLARE_UINT64(0x0000000020000000)  
//#define SHOW_Samples				DECLARE_UINT64(0x0000000040000000)  // show radiosity samples
#define SHOW_SeeThrough				DECLARE_UINT64(0x0000000040000000)  
#define SHOW_RadiosityGeometry		DECLARE_UINT64(0x0000000080000000)  // show radiosity geometry
//>@ ava

#define	SHOW_Editor					DECLARE_UINT64(0x0000000100000000)
#define	SHOW_Game					DECLARE_UINT64(0x0000000200000000)
#define	SHOW_Collision				DECLARE_UINT64(0x0000000400000000)
#define	SHOW_Grid					DECLARE_UINT64(0x0000000800000000)
#define	SHOW_Selection				DECLARE_UINT64(0x0000001000000000)
#define	SHOW_Bounds					DECLARE_UINT64(0x0000002000000000)
#define	SHOW_StaticMeshes			DECLARE_UINT64(0x0000004000000000)
#define	SHOW_Terrain				DECLARE_UINT64(0x0000008000000000)
#define	SHOW_BSP					DECLARE_UINT64(0x0000010000000000)	// Draws BSP brushes.
#define	SHOW_SkeletalMeshes 		DECLARE_UINT64(0x0000020000000000)
#define	SHOW_Constraints			DECLARE_UINT64(0x0000040000000000)
#define	SHOW_Fog					DECLARE_UINT64(0x0000080000000000)
#define	SHOW_Foliage				DECLARE_UINT64(0x0000100000000000)
#define	SHOW_BSPTriangles			DECLARE_UINT64(0x0000200000000000)  // Draws BSP triangles.
#define	SHOW_Paths					DECLARE_UINT64(0x0000400000000000)
#define	SHOW_MeshEdges				DECLARE_UINT64(0x0000800000000000)	// In the filled view modes, render mesh edges as well as the filled surfaces.
#define	SHOW_LargeVertices			DECLARE_UINT64(0x0001000000000000)	// Displays large clickable icons on static mesh vertices
#define SHOW_UnlitTranslucency		DECLARE_UINT64(0x0002000000000000)	// Render unlit translucency
#define	SHOW_Portals				DECLARE_UINT64(0x0004000000000000)
#define	SHOW_HitProxies				DECLARE_UINT64(0x0008000000000000)	// Draws each hit proxy in the scene with a different color.
#define	SHOW_ShadowFrustums			DECLARE_UINT64(0x0010000000000000)	// Draws un-occluded shadow frustums as 
#define	SHOW_ModeWidgets			DECLARE_UINT64(0x0020000000000000)	// Draws mode specific widgets and controls in the viewports (should only be set on viewport clients that are editing the level itself)
#define	SHOW_KismetRefs				DECLARE_UINT64(0x0040000000000000)	// Draws green boxes around actors in level which are referenced by Kismet. Only works in editor.
#define	SHOW_Volumes				DECLARE_UINT64(0x0080000000000000)	// Draws Volumes
#define	SHOW_CamFrustums			DECLARE_UINT64(0x0100000000000000)	// Draws camera frustums
#define	SHOW_NavigationNodes		DECLARE_UINT64(0x0200000000000000)	// Draws actors associated with path noding
#define	SHOW_Particles				DECLARE_UINT64(0x0400000000000000)	// Draws particles
#define SHOW_LightInfluences		DECLARE_UINT64(0x0800000000000000)	// Visualize light influences
#define	SHOW_BuilderBrush			DECLARE_UINT64(0x1000000000000000)	// Draws the builder brush wireframe
#define	SHOW_TerrainPatches			DECLARE_UINT64(0x2000000000000000)	// Draws an outline around each terrain patch
#define	SHOW_Cover					DECLARE_UINT64(0x4000000000000000)	// Complex cover rendering
#define	SHOW_ActorTags				DECLARE_UINT64(0x8000000000000000)	// Draw an Actors Tag next to it in the viewport. Only works in the editor.


#define	SHOW_DefaultGame	(SHOW_Game | SHOW_StaticMeshes | SHOW_SkeletalMeshes | SHOW_Terrain | SHOW_Foliage | SHOW_BSPTriangles | SHOW_BSP | SHOW_Fog | SHOW_Particles | SHOW_Decals | SHOW_DynamicShadows | SHOW_SceneCaptureUpdates | SHOW_Sprites | SHOW_PostProcess | SHOW_UnlitTranslucency | SHOW_ViewMode_Lit | SHOW_LensFlares)
#define	SHOW_DefaultEditor	(SHOW_Editor | SHOW_StaticMeshes | SHOW_SkeletalMeshes | SHOW_Terrain | SHOW_Foliage | SHOW_BSP | SHOW_Fog | SHOW_Portals | SHOW_Grid | SHOW_Selection | SHOW_Volumes | SHOW_NavigationNodes | SHOW_Particles | SHOW_BuilderBrush | SHOW_Decals | SHOW_DecalInfo | SHOW_LightRadius | SHOW_AudioRadius | SHOW_DynamicShadows | SHOW_SceneCaptureUpdates | SHOW_Sprites | SHOW_PostProcess | SHOW_ModeWidgets | SHOW_UnlitTranslucency | SHOW_Constraints | SHOW_LensFlares)

// The show flags which only editor views can use.
#define SHOW_EditorOnly_Mask	(SHOW_Editor | SHOW_LightRadius | SHOW_AudioRadius | SHOW_LargeVertices | SHOW_HitProxies | SHOW_NavigationNodes | SHOW_LightInfluences | SHOW_TerrainPatches | SHOW_Cover | SHOW_ActorTags | SHOW_PropertyColoration | SHOW_RadiosityGeometry )

/**
 * View modes, predefined configurations of a subset of the show flags.
 */

//<@ ava specific ; 2007. 1. 15 changmin : SHOW_ViewMode_Mask에 SHOW_ViewSpaceNormal 추가 >@
//<@ ava specific ; 2007. 10. 16 changmin : SHOW_BumpLighting 추가

// The flags to clear before setting any of the viewmode show flags.
#define SHOW_ViewMode_Mask (SHOW_Wireframe | SHOW_BSPTriangles | SHOW_Lighting | SHOW_Materials | SHOW_LightComplexity | SHOW_Brushes | SHOW_TextureDensity | SHOW_PostProcess | SHOW_ViewSpaceNormal | SHOW_BumpLighting )

// Wireframe w/ BSP
#define SHOW_ViewMode_Wireframe (SHOW_Wireframe | SHOW_BSPTriangles)

// Wireframe w/ brushes
#define SHOW_ViewMode_BrushWireframe (SHOW_Wireframe | SHOW_Brushes)

// Unlit
#define SHOW_ViewMode_Unlit (SHOW_BSPTriangles | SHOW_Materials | SHOW_PostProcess)

// Lit
#define SHOW_ViewMode_Lit (SHOW_BSPTriangles | SHOW_Materials | SHOW_Lighting | SHOW_PostProcess)

// Lit wo/ materials
#define SHOW_ViewMode_LightingOnly (SHOW_BSPTriangles | SHOW_Lighting | SHOW_PostProcess)

// Colored according to light count.
#define SHOW_ViewMode_LightComplexity (SHOW_BSPTriangles | SHOW_LightComplexity)

// Colored according to world-space texture density.
#define SHOW_ViewMode_TextureDensity (SHOW_BSPTriangles | SHOW_Materials | SHOW_TextureDensity )

//<@ ava specific ; 2007. 1. 15 changmin
#define SHOW_ViewMode_ViewSpaceNormal (SHOW_BSPTriangles | SHOW_Materials | SHOW_ViewSpaceNormal )
//>@ ava

//<@ ava specific ;2007. 10. 16 changmin
#define SHOW_ViewMode_BumpLighting (SHOW_BSPTriangles | SHOW_Materials | SHOW_BumpLighting )
//>@ ava

/**
 * The scene manager's persistent view state.
 */
class FSceneViewStateInterface
{
public:
	/** Called in the game thread to destroy the view state. */
	virtual void Destroy() = 0;

protected:
	// Don't allow direct deletion of the view state, Destroy should be called instead.
	virtual ~FSceneViewStateInterface() {}
};

/**
 * Allocates a new instance of the private scene manager implementation of FSceneViewStateInterface
 */
extern FSceneViewStateInterface* AllocateViewState();

/**
 * The types of interactions between a light and a primitive.
 */
enum ELightInteractionType
{
	LIT_CachedIrrelevant,
	LIT_CachedLightMap,
	LIT_CachedShadowMap1D,
	LIT_CachedShadowMap2D,
	LIT_Uncached
};

/**
 * Information about an interaction between a light and a mesh.
 */
class FLightInteraction
{
public:

	// Factory functions.
	static FLightInteraction Uncached() { return FLightInteraction(LIT_Uncached,NULL,FVector2D(),FVector2D()); }
	static FLightInteraction LightMap() { return FLightInteraction(LIT_CachedLightMap,NULL,FVector2D(),FVector2D()); }
	static FLightInteraction Irrelevant() { return FLightInteraction(LIT_CachedIrrelevant,NULL,FVector2D(),FVector2D()); }
	static FLightInteraction ShadowMap1D(const FVertexBuffer* ShadowVertexBuffer)
	{
		return FLightInteraction(LIT_CachedShadowMap1D,ShadowVertexBuffer,FVector2D(),FVector2D());
	}
	static FLightInteraction ShadowMap2D(
		const UTexture2D* ShadowTexture,
		const FVector2D& InShadowCoordinateScale,
		const FVector2D& InShadowCoordinateBias
		)
	{
		return FLightInteraction(LIT_CachedShadowMap2D,ShadowTexture,InShadowCoordinateScale,InShadowCoordinateBias);
	}

	// Accessors.
	ELightInteractionType GetType() const { return Type; }
	const FVertexBuffer* GetShadowVertexBuffer() const
	{
		check(Type == LIT_CachedShadowMap1D);
		return (FVertexBuffer*)ShadowResource;
	}
	const UTexture2D* GetShadowTexture() const
	{
		check(Type == LIT_CachedShadowMap2D);
		return (UTexture2D*)ShadowResource;
	}
	const FVector2D& GetShadowCoordinateScale() const
	{
		check(Type == LIT_CachedShadowMap2D);
		return ShadowCoordinateScale;
	}
	const FVector2D& GetShadowCoordinateBias() const
	{
		check(Type == LIT_CachedShadowMap2D);
		return ShadowCoordinateBias;
	}

private:

	/**
	 * Minimal initialization constructor.
	 */
	FLightInteraction(
		ELightInteractionType InType,
		const void* InShadowResource,
		const FVector2D& InShadowCoordinateScale,
		const FVector2D& InShadowCoordinateBias
		):
		Type(InType),
		ShadowResource(InShadowResource),
		ShadowCoordinateScale(InShadowCoordinateScale),
		ShadowCoordinateBias(InShadowCoordinateBias)
	{}

	ELightInteractionType Type;
	const void* ShadowResource;
	FVector2D ShadowCoordinateScale;
	FVector2D ShadowCoordinateBias;
};

/**
 * An interface to cached lighting for a specific mesh.
 */
class FLightCacheInterface
{
public:
	FLightCacheInterface() : Next(NULL)
	{}
	//<@ ava specific ; 2006. 11. 30 changmin
	FLightCacheInterface *Next;
	//>@ ava
	virtual FLightInteraction GetInteraction(const class FLightSceneInfo* LightSceneInfo) const = 0;
	virtual const class FLightMap* GetLightMap() const = 0;
};

/**
 * A mesh element definition.
 */
struct FMeshElement
{
	const FIndexBuffer* IndexBuffer;
	const FVertexFactory* VertexFactory;
	/** 
	 *	DynamicVertexData - pointer to user memory containing the vertex data.
	 *	Used for rendering dynamic data directly.
	 */
	const void* DynamicVertexData;
	INT DynamicVertexStride;
	/** 
	 *	DynamicIndexData - pointer to user memory containing the index data.
	 *	Used for rendering dynamic data directly.
	 */
	const void* DynamicIndexData;
	INT DynamicIndexStride;
	const FMaterialInstance* MaterialInstance;
	const FLightCacheInterface* LCI;
	FMatrix LocalToWorld;
	FMatrix WorldToLocal;	
	UINT FirstIndex;
	UINT NumPrimitives;
	INT BaseVertexIndex;	//<@ ava specific; 2007. 2. 26 changmin //>@ ava
	UINT MinVertexIndex;
	UINT MaxVertexIndex;
	FLOAT DepthBias;
	FLOAT SlopeScaleDepthBias;
	BITFIELD UseDynamicData : 1;
	BITFIELD ReverseCulling : 1;
	BITFIELD CastShadow : 1;
	BITFIELD bWireframe : 1;
	BITFIELD Type : PT_NumBits;
	BITFIELD ParticleType : PET_NumBits;
	BITFIELD DepthPriorityGroup : UCONST_SDPG_NumBits;

	//<@ ava specific : 2006. 9. 27 changmin
	BITFIELD bDepthMesh : 1;
	BITFIELD bRenderMesh : 1;
	BITFIELD bDrawnShared : 1;

	FBox BoundingBox;
	//>@ ava 



	FORCEINLINE UBOOL IsTranslucent() const
	{
        return MaterialInstance && IsTranslucentBlendMode(MaterialInstance->GetMaterial()->GetBlendMode());
	}

	FORCEINLINE UBOOL IsDistortion() const
	{
		return MaterialInstance && MaterialInstance->GetMaterial()->IsDistorted();
	}

	/** 
	* @return vertex stride specified for the mesh. 0 if not dynamic
	*/
	FORCEINLINE DWORD GetDynamicVertexStride() const
	{
		if (UseDynamicData && DynamicVertexData)
		{
			return DynamicVertexStride;
		}
		else
		{
			return 0;
		}
	}

	/** Default constructor. */
	FMeshElement():
		IndexBuffer(NULL),
		VertexFactory(NULL),
		DynamicVertexData(NULL),
		DynamicIndexData(NULL),
		MaterialInstance(NULL),
		LCI(NULL),
		DepthBias(0.f),
		SlopeScaleDepthBias(0.f),
		UseDynamicData(FALSE),
		ReverseCulling(FALSE),
		CastShadow(TRUE),
		bWireframe(FALSE),
		Type(PT_TriangleList),
		ParticleType(PET_None),
		//<@ ava specific ; 2007. 2. 26 changmin
		bDepthMesh(TRUE),
		bRenderMesh(TRUE),
		bDrawnShared(FALSE),
		BaseVertexIndex(0)	// -1 는 vertex base를 사용하지 않음을 뜻합니다.
		//>@ ava
	{}
};

/**
 * An interface implemented by dynamic resources which need to be initialized and cleaned up by the rendering thread.
 */
class FDynamicPrimitiveResource
{
public:

	virtual void InitPrimitiveResource() = 0;
	virtual void ReleasePrimitiveResource() = 0;
};

/**
 * The base interface used to query a primitive for its dynamic elements.
 */
class FPrimitiveDrawInterface
{
public:

	const FSceneView* const View;

	/** Initialization constructor. */
	FPrimitiveDrawInterface(const FSceneView* InView):
		View(InView)
	{}

	virtual UBOOL IsHitTesting() = 0;
	virtual void SetHitProxy(HHitProxy* HitProxy) = 0;

	virtual void RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource) = 0;

	virtual void DrawSprite(
		const FVector& Position,
		FLOAT SizeX,
		FLOAT SizeY,
		const FTexture* Sprite,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		) = 0;

	virtual void DrawLine(
		const FVector& Start,
		const FVector& End,
		const FLinearColor& Color,
		BYTE DepthPriorityGroup
		) = 0;

	virtual void DrawPoint(
		const FVector& Position,
		const FLinearColor& Color,
		FLOAT PointSize,
		BYTE DepthPriorityGroup
		) = 0;

	//<@ ava specific ; 2008. 2. 27 changmin
	virtual INT Ava_AddVertex( const FVector& Position, const FVector2D& UV, const FLinearColor& Color, BYTE DepthPriorityGroup ) = 0;
	virtual void Ava_AddTriangle( INT V0, INT V1, INT V2, const FTexture* Texture, EBlendMode BlendMode, BYTE DepthPriorityGroup) = 0;
	//>@ ava

	/**
	* Determines whether a particular material will be ignored in this context.
	* @param MaterialInstance - The render proxy of the material to check.
	* @return TRUE if meshes using the material will be ignored in this context.
	*/
	virtual UBOOL IsMaterialIgnored(const FMaterialInstance* MaterialInstance) const
	{
		return FALSE;
	}

	/**
	 * Draw a mesh element.
	 * This should only be called through the DrawMesh function.
	 */
	virtual void DrawMesh(const FMeshElement& Mesh) = 0;
};

/**
 * An interface to a scene interaction.
 */
class FViewElementDrawer
{
public:

	/**
	 * Draws the interaction using the given draw interface.
	 */
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI) {}
};

/**
 * An interface used to query a primitive for its static elements.
 */
class FStaticPrimitiveDrawInterface
{
public:
	virtual void SetHitProxy(HHitProxy* HitProxy) = 0;
	virtual void DrawMesh(
		const FMeshElement& Mesh,
		FLOAT MinDrawDistance,
		FLOAT MaxDrawDistance
		) = 0;
};

/**
 * An interface used by a primitive to render its shadow volumes.
 */
class FShadowVolumeDrawInterface
{
public:
	/**
	  * Called by FPrimitiveSceneProxy to render shadow volumes to the stencil buffer.
	  *
	  * @param IndexBuffer Shadow volume index buffer, indexing into the original vertex buffer
	  * @param VertexFactory Vertex factory that represents the shadow volume vertex format
	  * @param LocalToWorld World matrix for the primitive
	  * @param FirstIndex First index to use in the index buffer
	  * @param NumPrimitives Number of triangles in the triangle list
	  * @param MinVertexIndex Lowest index used in the index buffer
	  * @param MaxVertexIndex Highest index used in the index buffer
	  */
	virtual void DrawShadowVolume(
		FIndexBufferRHIParamRef IndexBuffer,
		const class FLocalShadowVertexFactory& VertexFactory,
		const FMatrix& LocalToWorld,
		UINT FirstIndex,
		UINT NumPrimitives,
		UINT MinVertexIndex,
		UINT MaxVertexIndex
		) = 0;
};

/**
 * The different types of relevance a primitive scene proxy can declare towards a particular scene view.
 */
struct FPrimitiveViewRelevance
{
	/** The primitive's static elements are rendered for the view. */
	BITFIELD bStaticRelevance : 1; 
	/** The primitive's dynamic elements are rendered for the view. */
	BITFIELD bDynamicRelevance : 1;
	/** The primitive is casting a shadow. */
	BITFIELD bShadowRelevance : 1;
	/** The primitive's decal elements are rendered for the view. */
	BITFIELD bDecalRelevance : 1;

	/** The primitive has one or more UnrealEd background DPG elements. */
	BITFIELD bUnrealEdBackgroundDPG : 1;
	/** The primitive has one or more world DPG elements. */
	BITFIELD bWorldDPG : 1;
	/** The primitive has one or more foreground DPG elements. */
	BITFIELD bForegroundDPG : 1;
	/** The primitive has one or more UnrealEd DPG elements. */
	BITFIELD bUnrealEdForegroundDPG : 1;

	/** The primitive has one or more translucent elements. */
	BITFIELD bTranslucentRelevance : 1;
	/** The primitive has one or more distortion elements. */
	BITFIELD bDistortionRelevance : 1;

	BITFIELD bSeeThroughRelevance : 1;

	/** The primitive has one or more elements which use a material that samples scene color */
	BITFIELD bUsesSceneColor : 1;
	/** Forces all directional lights to be dynamic. */
	BITFIELD bForceDirectionalLightsDynamic : 1;

	//<@ ava specific ; 2006.8.21 changmin.
	BITFIELD bForeForegroundDPG : 1;
	BITFIELD bSky0DPG : 1;
	BITFIELD bSky1DPG : 1;
	BITFIELD bSky2DPG : 1;
	BITFIELD bSky3DPG : 1;
	//>@ ava

	/** Initialization constructor. */
	FPrimitiveViewRelevance():
		bStaticRelevance(FALSE),
		bDynamicRelevance(FALSE),
		bShadowRelevance(FALSE),
		bDecalRelevance(FALSE),
		bUnrealEdBackgroundDPG(FALSE),
		bWorldDPG(FALSE),
		bForegroundDPG(FALSE),
		bSky0DPG(FALSE),
		bSky1DPG(FALSE),
		bSky2DPG(FALSE),
		bSky3DPG(FALSE),
		bUnrealEdForegroundDPG(FALSE),
		bTranslucentRelevance(FALSE),
		bDistortionRelevance(FALSE),
		bSeeThroughRelevance(FALSE),
		bUsesSceneColor(FALSE),
		bForceDirectionalLightsDynamic(FALSE),
		bForeForegroundDPG(FALSE)
	{}

	/** Logical OR operator.  Sets any relevance bits which are present in either FPrimitiveViewRelevance. */
	FPrimitiveViewRelevance& operator|=(const FPrimitiveViewRelevance& B)
	{
		bStaticRelevance |= B.bStaticRelevance != 0;
		bDynamicRelevance |= B.bDynamicRelevance != 0;
		bShadowRelevance |= B.bShadowRelevance != 0;
		bDecalRelevance |= B.bDecalRelevance != 0;
		bUnrealEdBackgroundDPG |= B.bUnrealEdBackgroundDPG != 0;
		bWorldDPG |= B.bWorldDPG != 0;
		bForegroundDPG |= B.bForegroundDPG != 0;
		bUnrealEdForegroundDPG |= B.bUnrealEdForegroundDPG != 0;
		bTranslucentRelevance |= B.bTranslucentRelevance != 0;
		bDistortionRelevance |= B.bDistortionRelevance != 0;
		bSeeThroughRelevance |= B.bSeeThroughRelevance != 0;
		bUsesSceneColor |= B.bUsesSceneColor != 0;
		//<@ ava specific ; 2006.8.21 changmin
		bForeForegroundDPG |= B.bForeForegroundDPG != 0;
		bSky0DPG |= B.bSky0DPG != 0;
		bSky1DPG |= B.bSky1DPG != 0;
		bSky2DPG |= B.bSky2DPG != 0;
		bSky3DPG |= B.bSky3DPG != 0;
		//>@ ava
		return *this;
	}

	/** Binary logical OR operator. */
	friend FPrimitiveViewRelevance operator|(const FPrimitiveViewRelevance& A,const FPrimitiveViewRelevance& B)
	{
		FPrimitiveViewRelevance Result(A);
		Result |= B;
		return Result;
	}

	/** Sets the flag corresponding to the given DPG. */
	void SetDPG(UINT DPGIndex,UBOOL bValue)
	{
		switch(DPGIndex)
		{
		case SDPG_UnrealEdBackground:	bUnrealEdBackgroundDPG = bValue; break;
		case SDPG_World:				bWorldDPG = bValue; break;
		case SDPG_Foreground:			bForegroundDPG = bValue; break;
		case SDPG_UnrealEdForeground:	bUnrealEdForegroundDPG = bValue; break;
		//<@ ava specific; 2006.8.21 changmin.
		case SDPG_ForeForeground:	bForeForegroundDPG = bValue; break;
		case SDPG_SkyLayer0:		bSky0DPG = bValue; break;
		case SDPG_SkyLayer1:		bSky1DPG = bValue; break;
		case SDPG_SkyLayer2:		bSky2DPG = bValue; break;
		case SDPG_SkyLayer3:		bSky3DPG = bValue; break;
		//>@ ava
		}
	}

	/** Gets the flag corresponding to the given DPG. */
	UBOOL GetDPG(UINT DPGIndex) const
	{
		switch(DPGIndex)
		{
		case SDPG_UnrealEdBackground:	return bUnrealEdBackgroundDPG;
		case SDPG_World:				return bWorldDPG;
		case SDPG_Foreground:			return bForegroundDPG;
		case SDPG_UnrealEdForeground:	return bUnrealEdForegroundDPG;
		default:						return FALSE;

		//@< ava specific ; 2006.8.21 changmin
		case SDPG_ForeForeground: return bForeForegroundDPG;
		case SDPG_SkyLayer0:		return bSky0DPG;
		case SDPG_SkyLayer1:		return bSky1DPG;
		case SDPG_SkyLayer2:		return bSky2DPG;
		case SDPG_SkyLayer3:		return bSky3DPG;
		//@> ava

		
		}
	}

	/** @return True if the primitive is relevant to the view. */
	UBOOL IsRelevant() const
	{
		return (bStaticRelevance | bDynamicRelevance) != 0;
	}
};

/**
*	Helper structure for storing motion blur information for a primitive
*/
struct FMotionBlurInfo
{
	/** The component this info represents.			*/
	UPrimitiveComponent* Component;
	/** The primitive scene info for the component.	*/
	FPrimitiveSceneInfo* PrimitiveSceneInfo;
	/** The previous LocalToWorld of the component.	*/
	FMatrix	PreviousLocalToWorld;
};

/**
 * An interface to the private scene manager implementation of a scene.  Use AllocateScene to create.
 * The scene
 */
class FSceneInterface
{
public:

	virtual void AddPrimitive(UPrimitiveComponent* Primitive) = 0;
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive) = 0;

	/** Updates the transform of a primitive which has already been added to the scene. */
	virtual void UpdatePrimitiveTransform(UPrimitiveComponent* Primitive) = 0;

	virtual void AddLight(ULightComponent* Light) = 0;
	virtual void RemoveLight(ULightComponent* Light) = 0;
	
	/** Updates the transform of a light which has already been added to the scene. */
	virtual void UpdateLightTransform(ULightComponent* Light) = 0;

	/**
	* Adds a light environment to the scene.
	* @param LightEnvironment - The light environment to add.
	*/
	virtual void AddLightEnvironment(const class ULightEnvironmentComponent* LightEnvironment) = 0;
	/**
	* Removes a light environment from the scene.
	* @param LightEnvironment - The light environment to add.
	*/
	virtual void RemoveLightEnvironment(const class ULightEnvironmentComponent* LightEnvironment) = 0;

	virtual void AddSceneCapture(USceneCaptureComponent* CaptureComponent) = 0;
	virtual void RemoveSceneCapture(USceneCaptureComponent* CaptureComponent) = 0;

	virtual void AddHeightFog(class UHeightFogComponent* FogComponent) = 0;
	virtual void RemoveHeightFog(class UHeightFogComponent* FogComponent) = 0;
	/**
	* Adds a wind source component to the scene.
	* @param WindComponent - The component to add.
	*/
	virtual void AddWindSource(class UWindDirectionalSourceComponent* WindComponent) = 0;
	/**
	* Removes a wind source component from the scene.
	* @param WindComponent - The component to remove.
	*/
	virtual void RemoveWindSource(class UWindDirectionalSourceComponent* WindComponent) = 0;
	/**
	* Accesses the wind source list.  Must be called in the rendering thread.
	* @return The list of wind sources in the scene.
	*/
	virtual const TArray<class FWindSourceSceneProxy*>& GetWindSources_RenderThread() const = 0;
	/**
	* Release this scene and remove it from the rendering thread
	*/
	virtual void Release() = 0;

	/**
	 * Retrieves the lights interacting with the passed in primitive and adds them to the out array.
	 *
	 * @param	Primitive				Primitive to retrieve interacting lights for
	 * @param	RelevantLights	[out]	Array of lights interacting with primitive
	 */
	virtual void GetRelevantLights( UPrimitiveComponent* Primitive, TArray<const ULightComponent*>* RelevantLights ) const = 0;
	
	/** Indicates if sounds in this should be allowed to play. */
	virtual UBOOL AllowAudioPlayback() = 0;

	/**
	 * @return		TRUE if hit proxies should be rendered in this scene.
	 */
	virtual UBOOL RequiresHitProxies() const = 0;

	// Accessors.
	virtual class UWorld* GetWorld() const = 0;

	/** 
	*	Set the primitives motion blur info
	* 
	*	@param	PrimitiveSceneInfo	The primitive to add
	*	@param	bRemoving			TRUE if the primitive is being removed
	*/
	virtual void AddPrimitiveMotionBlur(FPrimitiveSceneInfo* PrimitiveSceneInfo, UBOOL bRemoving) {};

	/**
	*	Clear out the motion blur info
	*/
	virtual void ClearMotionBlurInfo() {};

	/** 
	*	Get the primitives motion blur info
	* 
	*	@param	PrimitiveSceneInfo	The primitive to retrieve the motion blur info for
	*	@param	MotionBlurInfo		The pointer to set to the motion blur info for the primitive (OUTPUT)
	*
	*	@return	UBOOL				TRUE if the primitive info was found and set
	*/
	virtual UBOOL GetPrimitiveMotionBlurInfo(const FPrimitiveSceneInfo* PrimitiveSceneInfo, const FMotionBlurInfo*& MBInfo)
	{
		return FALSE;
	};


protected:
	virtual ~FSceneInterface() {}
};

/**
 * Allocates a new instance of the private FScene implementation for the given world.
 * @param World - An optional world to associate with the scene.
 * @param bAlwaysAllowAudioPlayback - Indicate that audio in this scene should always be played, even if its not the GWorld.
 * @param bInRequiresHitProxies- Indicates that hit proxies should be rendered in the scene.
 */
extern FSceneInterface* AllocateScene(UWorld* World, UBOOL bAlwaysAllowAudioPlayback, UBOOL bInRequiresHitProxies);

/**
 * An interface to an actor visibility state for a single frame of a view.
 */
class FActorVisibilityHistoryInterface
{
public:

	/** Virtual destructor. */
	virtual ~FActorVisibilityHistoryInterface() {}

	/**
	 * Tests whether an actor was visible for the scene view.
	 * @param Actor - The actor to test.
	 * @return True if the actor was visible.
	 */
	virtual UBOOL GetActorVisibility(const AActor* Actor) const = 0;
};

/**
 * Encapsulates the most recent actor visibility states for a particular view.
 * Provides thread-safe access to the actor visibility states while a scene is being rendered which updates the states.
 */
class FSynchronizedActorVisibilityHistory
{
public:

	/** Initialization constructor. */
	FSynchronizedActorVisibilityHistory();

	/** Destructor. */
	~FSynchronizedActorVisibilityHistory();

	/** Initializes the history.  Must be called before calling any of the other members. */
	void Init();

	/**
	 * Tests whether an actor was visible for the scene view.
	 * @param Actor - The actor to test.
	 * @return True if the actor was visible.
	 */
	UBOOL GetActorVisibility(const AActor* Actor) const;

	/**
	 * Updates the actor visibility states for a new frame.
	 * @param NewStates - The new actor visibility states.  Ownership is transferred to the callee.
	 */
	void SetStates(FActorVisibilityHistoryInterface* NewStates);

private:

	/** The most recent actor visibility states. */
	FActorVisibilityHistoryInterface* States;

	/** The critical section used to synchronize access to the actor visibility states. */
	FCriticalSection* CriticalSection;
};

/**
 * A projection from scene space into a 2D screen region.
 */
class FSceneView
{
public:
	const FSceneViewFamily* Family;
	FSceneViewStateInterface* State;

	/** A pointer to the history to write the view's actor visibility states to.  May be NULL. */
	FSynchronizedActorVisibilityHistory* ActorVisibilityHistory;

	/** The actor which is being viewed from. */
	const AActor* ViewActor;

	/** Chain of post process effects for this view */
	const UPostProcessChain* PostProcessChain;

	/** The view-specific post process settings. */
	const struct FPostProcessSettings* PostProcessSettings;

	/** An interaction which draws the view's interaction elements. */
	FViewElementDrawer* Drawer;

	FLOAT X;
	FLOAT Y;
	FLOAT SizeX;
	FLOAT SizeY;

	/** Automatic exposure control */
	FLOAT ExposureCenterRegionX;
	FLOAT ExposureCenterRegionY;

	/** The X coordinate of the view in the scene render targets. */
	INT RenderTargetX;

	/** The Y coordinate of the view in the scene render targets. */
	INT RenderTargetY;

	/** The width of the view in the scene render targets. */
	INT RenderTargetSizeX;

	/** The height of the view in the scene render targets. */
	INT RenderTargetSizeY;

	/** opacity */
	FLOAT Opacity;

	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
	//{{ Foreground Fov 지원
	//!{ 2006.3.6	허 창 민
	FMatrix				ForegroundProjectionMatrix;	
	FLOAT				ForegroundFOV;
	//!} 2006.3.6	허 창 민
	//}}
	FMatrix SavedViewMatrix, SavedProjectionMatrix;

	FLinearColor BackgroundColor;
	FLinearColor OverlayColor;
	
	DWORD		SeeThroughGroupMask;

	TArray<FLinearColor>		SeeThroughGroupColors;
	FLinearColor				SeeThroughSilhouetteColor;

	/** Color scale multiplier used during post processing */
	FLinearColor ColorScale;

	/** The primitives which are hidden for this view. */
	TArray<FPrimitiveSceneInfo*> HiddenPrimitives;	

	// Derived members.
	FMatrix ViewProjectionMatrix;
	FMatrix InvViewProjectionMatrix;

	void CalculateDerivedMatrices();

	//<@ ava specific ; 2007. 9. 10 changmin
	// add Cascaded Shadow map : NumShadow를 지정할 수가 없네...
	void Ava_SplitViewFrustum();
	FLOAT	Ava_SliceValues[8];
	FBox	Ava_SplitBounds[8];
	FVector Ava_SplitedFrustumVertices[8*8];	// 8 vertices per split
	FConvexVolume Ava_SplitedViewFrustum[8];
	FBox	Ava_ReceiverBounds[8];
	//>@ ava

	FVector4 ViewOrigin;
	FConvexVolume ViewFrustum;
	UBOOL bHasNearClippingPlane;
	FPlane NearClippingPlane;
	FLOAT NearClippingDistance;
	UBOOL bTransparentBackground;

	/* Vector used by shaders to convert depth buffer samples into z coordinates in world space */
	FVector4 InvDeviceZToWorldZTransform;

	/* Vector used by shaders to convert projection-space coordinates to texture space. */
	FVector4 ScreenPositionScaleBias;

	/** FOV based multiplier for cull distance on objects */
	FLOAT LODDistanceFactor;

	/** Initialization constructor. */
	FSceneView(
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
		UBOOL InTransparentBackground = FALSE,
		FLOAT InLODDistanceFactor = 1.0f,
		FLOAT InExposureCenterRegionX = 0.75f,
		FLOAT InExposureCenterRegionY = 0.80f,
		FLOAT InOpacity = 1.0f
		);

	/** Transforms a point from world-space to the view's screen-space. */
	FVector4 WorldToScreen(const FVector& WorldPoint) const;

	/** Transforms a point from the view's screen-space to world-space. */
	FVector ScreenToWorld(const FVector4& ScreenPoint) const;

	/** Transforms a point from the view's screen-space into pixel coordinates relative to the view's X,Y. */
	UBOOL ScreenToPixel(const FVector4& ScreenPoint,FVector2D& OutPixelLocation) const;

	/** Transforms a point from pixel coordinates into the view's screen-space. */
	FVector4 PixelToScreen(FLOAT X,FLOAT Y,FLOAT Z) const;

	/** Transforms a point from the view's world-space into pixel coordinates relative to the view's X,Y. */
	UBOOL WorldToPixel(const FVector& WorldPoint,FVector2D& OutPixelLocation) const;

	/** 
	 * Transforms a point from the view's world-space into the view's screen-space. 
	 * Divides the resulting X, Y, Z by W before returning. 
	 */
	FPlane Project(const FVector& WorldPoint) const;

	/** 
	 * Transforms a point from the view's screen-space into world coordinates
	 * multiplies X, Y, Z by W before transforming. 
	 */
	FVector Deproject(const FPlane& ScreenPoint) const;	
};

/**
 * Helper structure for dynamic shadow stats containing stringified information in
 * column form.
 */
struct FDynamicShadowStatsRow
{
	/** Columns of this row. */
	TArray<FString> Columns;
};
typedef TArray<FDynamicShadowStatsRow> FDynamicShadowStats;

/**
 * A set of views into a scene which only have different view transforms and owner actors.
 */
class FSceneViewFamily
{
public:

	/** The views which make up the family. */
	TArray<const FSceneView*> Views;

	/** The render target which the views are being rendered to. */
	const FRenderTarget* RenderTarget;

	/** The scene being viewed. */
	FSceneInterface* Scene;

	/** The show flags for the views. */
	EShowFlags ShowFlags;

	/** The current world time. */
	FLOAT CurrentWorldTime;

	/** The current real time. */
	FLOAT CurrentRealTime;

	/** Pointer to dynamic shadow stats. If non- NULL stats will be written to it. */
	FDynamicShadowStats* DynamicShadowStats;

	/** Used to defer the back buffer clearing to just before the back buffer is drawn to */
	UBOOL bClear;

	/** AVA **/
	UBOOL bDisableOcclusionQuery;
	UBOOL bEncodingOffline;
	UBOOL bDisableForegroundDPGs;

	/** Initialization constructor. */
	FSceneViewFamily(const FRenderTarget* InRenderTarget,FSceneInterface* InScene,EShowFlags InShowFlags,FLOAT InCurrentWorldTime,FLOAT InCurrentRealTime,FDynamicShadowStats* InDynamicShadowStats,UBOOL bClear=FALSE,UBOOL InDisableOcclusionQuery=FALSE,UBOOL InEncodingOffline=FALSE, UBOOL InDisableForegroundDPGs=FALSE);
};

/**
 * Call from the game thread to send a message to the rendering thread to being rendering this view family.
 */
extern void BeginRenderingViewFamily(FCanvas* Canvas,const FSceneViewFamily* ViewFamily);

#if !FINAL_RELEASE
/**
* Sends a freeze rendering message over the render queue
*/
extern void FreezeRendering();

/**
* Sends a freeze rendering message over the render queue
*/
extern void UnfreezeRendering();

#endif

/**
 * A view family which deletes its views when it goes out of scope.
 */
class FSceneViewFamilyContext : public FSceneViewFamily
{
public:
	//<@ ava specific ; 2006 / 9 / 5 changmin
	// bDrawEmissiveDynamicPrimitives 추가
	/** Initialization constructor. */
	FSceneViewFamilyContext(const FRenderTarget* InRenderTarget,FSceneInterface* InScene,EShowFlags InShowFlags,FLOAT InCurrentWorldTime,FLOAT InCurrentRealTime,FDynamicShadowStats* InDynamicShadowStats,UBOOL bClear=FALSE,UBOOL InDisableOcclusionQuery=FALSE,UBOOL InEncodingOffline=FALSE, UBOOL InDisableForegroundDPGS=FALSE):
		FSceneViewFamily(InRenderTarget,InScene,InShowFlags,InCurrentWorldTime,InCurrentRealTime,InDynamicShadowStats,bClear,InDisableOcclusionQuery,InEncodingOffline,InDisableForegroundDPGS)
	{}
	//>@ ava
	/** Destructor. */
	~FSceneViewFamilyContext();
};

// Static-function 
class FPrimitiveSceneProxy;
FPrimitiveSceneProxy* Scene_GetProxyFromInfo(FPrimitiveSceneInfo* Info);
