/*=============================================================================
	UnScene.h: Scene manager definitions.
	Copyright ?2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes the draw mesh macros
#include "UnSceneUtils.h"
#include "EngineSceneClasses.h"

/**
 * Information about a light which affects a mesh element.
 * Mirrored in PrimitiveComponent.uc
 */
struct FMeshElementLightInfo
{
	/** The light which affects the mesh element. */
	class ULightComponent* Light;

	/** An optional vertex buffer containing static shadowing information. */
	FVertexBuffer* ShadowVertexBuffer;

	/** An optional texture containing static shadowing information. */
	UShadowMap2D* ShadowMap;

	/**
	 * Minimal initialization constructor.
	 */
	FMeshElementLightInfo(ULightComponent* InLight,FVertexBuffer* InShadowVertexBuffer,UShadowMap2D* InShadowMap):
		Light(InLight),
		ShadowVertexBuffer(InShadowVertexBuffer),
		ShadowMap(InShadowMap)
	{}
};

/**
 * The type of a mesh element.
 */
enum EMeshElementType
{
	MET_TriList = 0,
	MET_WireframeTriList = 1,
	MET_LineList = 2,
	MET_PointList = 3,
	MET_TriStrip = 4,
	MET_Max = 5,
	MET_NumBits = 3
};

//
//	FRawTriangleVertex
//

struct FRawTriangleVertex
{
	FVector		Position,
				TangentX,
				TangentY,
				TangentZ;
	FVector2D	TexCoord;

	// Constructors.

	FRawTriangleVertex() {}
	FRawTriangleVertex( const FVector& InPosition ):
		Position(InPosition),
		TangentX(FVector(1,0,0)),
		TangentY(FVector(0,1,0)),
		TangentZ(FVector(0,0,1)),
		TexCoord(FVector2D(0,0))
	{}
	FRawTriangleVertex(const FVector& InPosition,const FVector& InTangentX,const FVector& InTangentY,const FVector& InTangentZ,const FVector2D& InTexCoord):
		Position(InPosition),
		TangentX(InTangentX),
		TangentY(InTangentY),
		TangentZ(InTangentZ),
		TexCoord(InTexCoord)
	{}
};

/**
 * An interface for rendering a batch of arbitrary, code generated triangles with the same material in a non-optimal but easy way.
 */
struct FTriangleRenderInterface
{
	// DrawTriangle - Enqueues a triangle for rendering.

	virtual void DrawTriangle(const FRawTriangleVertex& V0,const FRawTriangleVertex& V1,const FRawTriangleVertex& V2) = 0;

	// DrawQuad - Enqueues a triangulated quad for rendering.

	void DrawQuad(const FRawTriangleVertex& V0,const FRawTriangleVertex& V1,const FRawTriangleVertex& V2,const FRawTriangleVertex& V3)
	{
		DrawTriangle(V0,V1,V2);
		DrawTriangle(V0,V2,V3);
	}

	// Finish - Cleans up the interface.  Interface pointer may not be used after calling Finish.

	virtual void Finish() = 0;
};

/**
 * The type of a shadow volume.
 */
enum EShadowVolumeType
{
	SVT_ZFail = 0,
	SVT_ZPass = 1,
	SVT_NumBits = 1
};

/**
 * An interface to the platform-specific shadow-volume rendering code.
 */
struct FShadowVolumeRenderInterface
{
	// DrawShadowVolume - Draws a shadow volume.

	virtual void DrawShadowVolume(
		ULightComponent* Light,
		FVertexFactory* VertexFactory,
		FIndexBuffer* IndexBuffer,
		const FMatrix* LocalToWorld,
		const FMatrix* WorldToLocal,
		UINT FirstIndex,
		UINT NumPrimitives,
		UINT MinVertexIndex,
		UINT MaxVertexIndex,
		EShadowVolumeType Type,
		UBOOL ReverseCulling,
		ESceneDepthPriorityGroup DepthPriority
		)
	{}
};

/**
 * An interface to the platform-specific rendering code.
 */
struct FPrimitiveRenderInterface
{
	// GetViewport - Returns the viewport that's being rendered to.

	virtual FChildViewport* GetViewport() = 0;

	// IsHitTesting - Returns whether this PRI is rendering hit proxies.

	virtual UBOOL IsHitTesting() = 0;

	// SetHitProxy - Sets the active hit proxy.

	virtual void SetHitProxy(HHitProxy* HitProxy) {}

	/** Sets the PreDepthSortKey for rendering unlit translucent mesh elements. */
	virtual void SetUnlitTranslucencyPreDepthSortKey(INT KeyValue) {}

	// DrawMesh - Draws a lit triangle mesh.

	virtual void DrawMesh(
		FVertexFactory* VertexFactory,
		FIndexBuffer* IndexBuffer,
		FMaterialInstance* MaterialInstance,
		const FMatrix* LocalToWorld,
		const FMatrix* WorldToLocal,
		const TArray<FMeshElementLightInfo>* Lights,
		ULightMap* LightMap,
		class AActor* AmbientCubeActor,
		UINT FirstIndex,
		UINT NumPrimitives,
		UINT MinVertexIndex,
		UINT MaxVertexIndex,
		UBOOL ReverseCulling,
		UBOOL CastShadow,
		EMeshElementType Type,
		ESceneDepthPriorityGroup DepthPriority,		
		FMatrix* PreviousLocalToWorld = NULL,
		FLOAT LightVisibility = 1
		)
	{}

	// DrawSprite - Draws an unlit sprite.

	virtual void DrawSprite(const FVector& Position,FLOAT SizeX,FLOAT SizeY,FTextureBase* Sprite,FColor Color,ESceneDepthPriorityGroup DepthPriority = SDPG_World) {}

	// DrawLine - Draws an unlit line segment.

	virtual void DrawLine(const FVector& Start,const FVector& End,FColor Color,ESceneDepthPriorityGroup DepthPriority = SDPG_World) {}

	// DrawPoint - Draws an unlit point.

	virtual void DrawPoint(const FVector& Position,FColor Color,FLOAT PointSize,ESceneDepthPriorityGroup DepthPriority = SDPG_World) {}

	// Solid shape drawing utility functions. Not really designed for speed - more for debugging.
	// These utilities functions are implemented in UnScene.cpp using GetTRI.

	virtual FTriangleRenderInterface* GetTRI(const FMatrix& LocalToWorld,FMaterialInstance* MaterialInstance,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawBox(const FMatrix& BoxToWorld,const FVector& Radii,FMaterialInstance* MaterialInstance,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawSphere(const FVector& Center,const FVector& Radii,INT NumSides,INT NumRings,FMaterialInstance* MaterialInstance,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawCone(const FMatrix& ConeToWorld, FLOAT Angle1, FLOAT Angle2, INT NumSides, UBOOL bDrawSideLines, const FColor& SideLineColor, FMaterialInstance* MaterialInstance, ESceneDepthPriorityGroup DepthPriority = SDPG_World);

	// Line drawing utility functions.

	void DrawWireBox(const FBox& Box,FColor Color,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawCircle(const FVector& Base,const FVector& X,const FVector& Y,FColor Color,FLOAT Radius,INT NumSides,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawWireCylinder(const FVector& Base,const FVector& X,const FVector& Y,const FVector& Z,FColor Color,FLOAT Radius,FLOAT HalfHeight,INT NumSides,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawDirectionalArrow(const FMatrix& ArrowToWorld,FColor InColor,FLOAT Length,FLOAT ArrowSize = 1.f,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawWireStar(const FVector& Position, FLOAT Size, FColor Color,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawDashedLine(const FVector& Start, const FVector& End, FColor Color, FLOAT DashSize,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
	void DrawWireDiamond(const FMatrix& DiamondMatrix, FLOAT Size, const FColor& InColor,ESceneDepthPriorityGroup DepthPriority = SDPG_World); 

	void DrawFrustumWireframe(const FMatrix& WorldToFrustum,FColor Color,ESceneDepthPriorityGroup DepthPriority = SDPG_World);
};

/**
 * Encapsulates the inside and/or outside state of an intersection test.
 */
struct FOutcode
{
private:

	BITFIELD	Inside : 1,
				Outside : 1;

public:

	// Constructor.

	FOutcode():
		Inside(0), Outside(0)
	{}
	FOutcode(UBOOL InInside,UBOOL InOutside):
		Inside(InInside), Outside(InOutside)
	{}

	// Accessors.

	FORCEINLINE void SetInside(UBOOL NewInside) { Inside = NewInside; }
	FORCEINLINE void SetOutside(UBOOL NewOutside) { Outside = NewOutside; }
	FORCEINLINE UBOOL GetInside() const { return Inside; }
	FORCEINLINE UBOOL GetOutside() const { return Outside; }
};

//
//	FConvexVolume
//

struct FConvexVolume
{
public:

	TArray<FPlane>	Planes;
	/** This is the set of planes pre-permuted to SSE/Altivec form */
	TArray<FPlane> PermutedPlanes;

	FConvexVolume()
	{
		INT N = 5;
	}

	/**
	 * Builds the set of planes used to clip against. Also, puts the planes
	 * into a form more readily used by SSE/Altivec so 4 planes can be
	 * clipped against at once.
	 */
	FConvexVolume(const TArray<FPlane>& InPlanes) :
		Planes( InPlanes )
	{
		Init();
	}

	/**
	 * Builds the permuted planes for SSE/Altivec fast clipping
	 */
	void Init(void)
	{
		INT NumToAdd = Planes.Num() / 4;
		// Presize the array
		PermutedPlanes.Empty(NumToAdd);
		// For each set of four planes
		for (INT Count = 0; Count < NumToAdd; Count += 4)
		{
			// Add them in SSE ready form
			new(PermutedPlanes)FPlane(Planes(Count + 0).X,Planes(Count + 1).X,Planes(Count + 2).X,Planes(Count + 3).X);
			new(PermutedPlanes)FPlane(Planes(Count + 0).Y,Planes(Count + 1).Y,Planes(Count + 2).Y,Planes(Count + 3).Y);
			new(PermutedPlanes)FPlane(Planes(Count + 0).Z,Planes(Count + 1).Z,Planes(Count + 2).Z,Planes(Count + 3).Z);
			new(PermutedPlanes)FPlane(Planes(Count + 0).W,Planes(Count + 1).W,Planes(Count + 2).W,Planes(Count + 3).W);
		}
	}

	/**
	 * Clips a polygon to the volume.
	 *
	 * @param	Polygon - The polygon to be clipped.  If the true is returned, contains the
	 *			clipped polygon.
	 *
	 * @return	Returns false if the polygon is entirely outside the volume and true otherwise.
	 */
	UBOOL ClipPolygon(FPoly& Polygon) const;

	// Intersection tests.

	FOutcode GetBoxIntersectionOutcode(const FVector& Origin,const FVector& Extent) const;

	UBOOL IntersectBox(const FVector& Origin,const FVector& Extent) const;
	UBOOL IntersectSphere(const FVector& Origin,const FLOAT Radius) const;

	/**
	 * Creates a convex volume bounding the view frustum for a view-projection matrix.
	 *
	 * @param	ViewProjectionMatrix - The view-projection matrix which defines the view frustum.
	 * @param	UseNearPlane - True if the convex volume should be bounded by the view frustum's near clipping plane.
	 * @param	OutFrustumBounds - The FConvexVolume which contains the view frustum bounds on return.
	 */
    static void GetViewFrustumBounds(const FMatrix& ViewProjectionMatrix,UBOOL UseNearPlane,FConvexVolume& OutFrustumBounds);	

	/**
	 * Serializor
	 *
	 * @param	Ar				Archive to serialize data to
	 * @param	ConvexVolume	Convex volumes to serialize to archive
	 *
	 * @return passed in archive
	 */
	friend FArchive& operator<<(FArchive& Ar,FConvexVolume& ConvexVolume);
};

/**
 * Helper structure to gather information about light/ primitive/ preshadow primitive interaction.
 */
struct FDynamicShadowStats
{
	/** Shadow parent */
	UPrimitiveComponent*			ShadowParent;
	/** Array of primitives casting the dynamic shadow */
	TArray<UPrimitiveComponent*>	ShadowPrimitives;	
	/** Array of prmitives that are in the	dynamic primitives preshadow */
	TArray<UPrimitiveComponent*>	PreShadowPrimitives;
	/** Whether the dynamic shadow is using a shadow volume */
	UBOOL							bIsUsingShadowVolume;

	/**
	 * Serializor
	 *
	 * @param	Ar			Archive to serialize data to
	 * @param	ShadowStats	Shadow stats to serialize to archive
	 * @return passed in archive
	 */
	friend FArchive& operator<<(FArchive& Ar,FDynamicShadowStats& ShadowStats);

	/**
	 * Equals operator.
	 *
	 * @param Other	Other object to compare against
	 * @return TRUE if equal, FALSE otherwise
	 */
	UBOOL operator == ( const FDynamicShadowStats& Other ) const
	{
		UBOOL bIsEqual	=		ShadowParent			== Other.ShadowParent 
							&&	ShadowPrimitives		== Other.ShadowPrimitives
							&&	PreShadowPrimitives		== Other.PreShadowPrimitives
							&&	bIsUsingShadowVolume	== Other.bIsUsingShadowVolume;

		return bIsEqual;
	}
};

/**
 * Stringified version of dynamic shadow stats.
 */
struct FDynamicShadowStatsStringified
{
	/**
	 * Default constructor.
	 */
	FDynamicShadowStatsStringified()
	{}

	/**
	 * Constructor initializing all member variables.
	 *
	 * @param InOwner		name of the component's owner
	 * @param InComponent	name of the component
	 */
	FDynamicShadowStatsStringified( const TCHAR* InLight, const TCHAR* InShadowParent, const TCHAR* InSubject, const TCHAR* InPreShadow, const TCHAR* InComponent, const TCHAR* InFullActorName )
	:	Light( InLight )
	,	ShadowParent( InShadowParent )
	,	Subject( InSubject )
	,	PreShadow( InPreShadow )
	,	Component( InComponent )
	,	FullActorName( InFullActorName )
	{}

	/** Name of light component */
	FString	Light;
	/** Name of shadow parent */
	FString ShadowParent;
	/** Name of shadow subject */
	FString	Subject;
	/** Name of preshadow primitive */
	FString PreShadow;
	/** Name of component */
	FString Component;
	/** Full name of associated actor. */
	FString FullActorName;
};


//
//	FScene
//

struct FScene
{
	TArray<class USkyLightComponent*>				SkyLights;
	TArray<class UHeightFogComponent*>				Fogs;
	TArray<class UWindPointSourceComponent*>		WindPointSources;
	TArray<class UWindDirectionalSourceComponent*>	WindDirectionalSources;
	/** probes used to capture this scene to texture */
	TArray<class FSceneCaptureProbe*>				SceneCaptureProbes;
	/** Whether the code should gather a snapshot of dynamic shadow stats */
	UBOOL											bGatherDynamicShadowStatsSnapshot;
	/** Multi map from light component to dynamic shadow stats helper structure */
	TMultiMap<ULightComponent*,FDynamicShadowStats>	LightToDynamicShadowStatsMap;
	/** Stringified version of dynamic shadow stats */
	TArray<FDynamicShadowStatsStringified>			DynamicShadowStatsStringified;

	// AddPrimitive
	
	virtual void AddPrimitive(UPrimitiveComponent* Primitive) = 0;

	// RemovePrimitive

	virtual void RemovePrimitive(UPrimitiveComponent* Primitive) = 0;

	// AddLight

	virtual void AddLight(ULightComponent* Light);

	// RemoveLight

	virtual void RemoveLight(ULightComponent* Light);

	// GetVisibleComponents

	virtual void GetVisibleComponents(const FSceneContext& Context,const FPlane& ViewOrigin,const FConvexVolume& Frustum,TArray<UPrimitiveComponent*>& OutPrimitives,TArray<ULightComponent*>& OutLights) = 0;

	// GetRelevantLights

	virtual void GetRelevantLights(UPrimitiveComponent* Primitive,TArray<ULightComponent*>& OutLights) = 0;

	// Serialize

	void Serialize(FArchive& Ar);

	// scene captures
	virtual void AddSceneCaptureProbe( FSceneCaptureProbe* Probe ) {}
	virtual void RemoveSceneCaptureProbe( FSceneCaptureProbe* Probe ) {}
	virtual void UpdateSceneCaptures( FRenderInterface* RI, struct FSceneView* ClientView ) {}

	/**
	 * Updates stringified dynamic shadow stats from LightToDynamicShadowStatsMap.
	 */
	void StringifyDynamicShadowStats();

	/**
	 * Returns whether audio playback is allowed for this scene.
	 *
	 * @return TRUE
	 */
	virtual UBOOL AllowAudioPlayback();
};

/**
 * A cachable resource for cross-frame scene view state: pupil dilation, etc.
 */
struct FSceneViewState : FResource
{
	DECLARE_RESOURCE_TYPE(FSceneViewState,FResource);

	// Constructor.

	FSceneViewState()
	{}
};

//
//	EShowFlags
//

typedef QWORD EShowFlags;

#define	SHOW_Editor					DECLARE_UINT64(0x0000000100000000)
#define	SHOW_Game					DECLARE_UINT64(0x0000000200000000)
#define	SHOW_Collision				DECLARE_UINT64(0x0000000400000000)
#define	SHOW_Grid					DECLARE_UINT64(0x0000000800000000)
#define	SHOW_Selection				DECLARE_UINT64(0x0000001000000000)
#define	SHOW_Bounds					DECLARE_UINT64(0x0000002000000000)
#define	SHOW_StaticMeshes			DECLARE_UINT64(0x0000004000000000)
#define	SHOW_Terrain				DECLARE_UINT64(0x0000008000000000)
#define	SHOW_BSP					DECLARE_UINT64(0x0000010000000000)
#define	SHOW_SkeletalMeshes 		DECLARE_UINT64(0x0000020000000000)
#define	SHOW_Constraints			DECLARE_UINT64(0x0000040000000000)
#define	SHOW_Fog					DECLARE_UINT64(0x0000080000000000)
#define	SHOW_Foliage				DECLARE_UINT64(0x0000100000000000)
#define	SHOW_Orthographic			DECLARE_UINT64(0x0000200000000000)
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
#define	SHOW_MissingCollisionModel	DECLARE_UINT64(0x0000000000000001)	// Show any static meshes with collision on but without a collision model in bright colour.
#define	SHOW_Decals					DECLARE_UINT64(0x0000000000000002)	// Draw decals.
#define	SHOW_DecalInfo				DECLARE_UINT64(0x0000000000000004)	// Draw decal dev info (frustums, tangent axes, etc).
#define	SHOW_LightRadius			DECLARE_UINT64(0x0000000000000008)	// Draw point light radii.
#define	SHOW_AudioRadius			DECLARE_UINT64(0x0000000000000010)	// Draw sound actor radii.
#define	SHOW_DynamicShadows			DECLARE_UINT64(0x0000000000000020)	// Draw dynamic shadows.
#define SHOW_PostProcess			DECLARE_UINT64(0x0000000000000040)  // Draw post process effects
#define SHOW_PerformanceColoration	DECLARE_UINT64(0x0000000000000080)  // Render meshes based on relative performance based on per instance flags
#define SHOW_SceneCaptureUpdates	DECLARE_UINT64(0x0000000000000100)	// Update scene capture probes
#define SHOW_Sprites				DECLARE_UINT64(0x0000000000000200)	// Draw sprite components
#define SHOW_LevelColoration		DECLARE_UINT64(0x0000000000000400)  // Render meshes with colors based on what the levle they belong to.
#define SHOW_Ambientcubes			DECLARE_UINT64(0x0000000000000800)  
#define	SHOW_DefaultGame	(SHOW_Game | SHOW_StaticMeshes | SHOW_SkeletalMeshes | SHOW_Terrain | SHOW_Foliage | SHOW_BSP | SHOW_Fog | SHOW_Particles | SHOW_Decals | SHOW_DynamicShadows | SHOW_SceneCaptureUpdates | SHOW_Sprites | SHOW_PostProcess | SHOW_UnlitTranslucency )
#define	SHOW_DefaultEditor	(SHOW_Editor | SHOW_StaticMeshes | SHOW_SkeletalMeshes | SHOW_Terrain | SHOW_Foliage | SHOW_BSP | SHOW_Fog | SHOW_Portals | SHOW_Grid | SHOW_Selection | SHOW_Volumes | SHOW_NavigationNodes | SHOW_Particles | SHOW_BuilderBrush | SHOW_Decals | SHOW_DecalInfo | SHOW_LightRadius | SHOW_AudioRadius | SHOW_DynamicShadows | SHOW_SceneCaptureUpdates | SHOW_Sprites | SHOW_PostProcess | SHOW_ModeWidgets | SHOW_UnlitTranslucency )

//!{ 2006-06-07	Çã Ã¢ ¹Î
#define SHOW_Samples				DECLARE_UINT64(0x0000000000000800)  // show radiosity samples
//!} 2006-06-07	Çã Ã¢ ¹Î

//
//	ESceneViewMode
//

enum ESceneViewMode
{
	SVM_None			= 0x00,
	SVM_Wireframe		= 0x01,
	SVM_BrushWireframe	= 0x02,	// Wireframe which shows CSG brushes and not BSP.
	SVM_Unlit			= 0x04,
	SVM_Lit				= 0x08,
	SVM_LightingOnly	= 0x10,
	SVM_LightComplexity	= 0x20,

	SVM_WireframeMask = SVM_Wireframe | SVM_BrushWireframe
};

//
//	FSceneView
//

struct FSceneView
{
	FMatrix				ViewMatrix,
						ProjectionMatrix;

	//{{ Foreground Fov Áö¿ø
	//!{ 2006.3.6	Çã Ã¢ ¹Î
	FMatrix				ForegroundProjectionMatrix;
	FMatrix				OtherProjectionMatrix;
	FLOAT				ForegroundFOV;
	//!} 2006.3.6	Çã Ã¢ ¹Î
	//}}

	FLOAT				NearClipPlane;
	/** field of view matching projection matrix */
	FLOAT				FOV;
	/** FOV based multiplier for cull distance on objects */
	FLOAT FOVBias;

	EShowFlags			ShowFlags;
	DWORD				ViewMode;
	FColor				BackgroundColor,
						OverlayColor;

	FSceneViewState*	State;
	UBOOL				AutomaticBrightness;
	FLOAT				BrightnessAdjustSpeed,
						ManualBrightnessScale;

	/* Color scale multiplier used during post processing */
	FPlane				ColorScale;
	/** The brightness multiplier for the light bloom */
	FLOAT				BloomAlpha;
	/** The the bloom width factor */
	FLOAT				BloomAttenuation;

	/** The actor which is being viewed from. */
	AActor*				ViewActor;

	/** Current LOD level for the view */
	INT					LODLevel;

	// Constructor.

#ifdef WITH_DPVS
	DPVS::Camera*		DPVS_Camera;	
#endif

	FSceneView();

	/**
	 * Returns whether lighting is visible in this view.
	 */
	UBOOL IsLit() const
	{
		switch(ViewMode)
		{
		case SVM_Lit:
		case SVM_LightingOnly:
			return 1;
		default:
			return 0;
		};
	}

	// Project/Deproject - Transforms points from world space to post-perspective screen space.

	FPlane Project(const FVector& V) const;
	FVector Deproject(const FPlane& P) const;
};

/**
 * Information about a scene currently being rendered, passed to each primitive in the scene.
 */
struct FSceneContext
{
	FSceneView*	View;
	FScene*		Scene;
	INT			X,
				Y;
	UINT		SizeX,
				SizeY;

	FMatrix ViewProjectionMatrix;
	FMatrix InvViewProjectionMatrix;

	//{{ Foreground Fov Áö¿ø
	//!{ 2006.3.6	Çã Ã¢ ¹Î
	FMatrix ForegroundViewProjectionMatrix;
	FMatrix InvForegoundViewProjectionMatrix;

	FMatrix OtherViewProjectionMatrix;		// Other = World / Background DPG
	FMatrix InvOtherViewProjectionMatrix;	
	//!} 2006.3.6	Çã Ã¢ ¹Î
	//}}

	FConvexVolume ViewFrustum;
	FPlane ViewOrigin;

	/** optional render target surface to render the scene to */
	FRenderTarget*	RenderTarget;

	// Constructor.
	FSceneContext(){}

	FSceneContext(FSceneView* InView,FScene* InScene,INT InX,INT InY,UINT InSizeX,UINT InSizeY,FRenderTarget* InRenderTarget=NULL):
		View(InView),
		Scene(InScene),
		X(InX),
		Y(InY),
		SizeX(InSizeX),
		SizeY(InSizeY),
		RenderTarget(InRenderTarget)
	{
		//{{ Foreground Fov Áö¿ø
		//!{ 2006.3.6	Çã Ã¢ ¹Î
		OtherViewProjectionMatrix = View->ViewMatrix * View->ProjectionMatrix;
		InvOtherViewProjectionMatrix = OtherViewProjectionMatrix.Inverse();

		ForegroundViewProjectionMatrix = View->ViewMatrix * View->ForegroundProjectionMatrix;
		InvForegoundViewProjectionMatrix = ForegroundViewProjectionMatrix.Inverse();

		ViewProjectionMatrix = OtherViewProjectionMatrix;
		InvViewProjectionMatrix = InvOtherViewProjectionMatrix;
		//!} 2006.3.6	Çã Ã¢ ¹Î
		//}}

		FConvexVolume::GetViewFrustumBounds(ViewProjectionMatrix,0,ViewFrustum);

		// Calculate the view origin from the view/projection matrices.
		if(View->ProjectionMatrix.M[3][3] < 1.0f)
		{
			ViewOrigin = FPlane(View->ViewMatrix.Inverse().GetOrigin(),1);
		}
		else
		{
			ViewOrigin = FPlane(View->ViewMatrix.Inverse().TransformNormal(FVector(0,0,-1)).SafeNormal(),0);
		}
	}

	//{{ Foreground Fov Áö¿ø
	//!{ 2006.3.6	Çã Ã¢ ¹Î
	void StartForegroundDPG()
	{
		if( GIsGame || GIsPlayInEditorWorld )
		{
			ViewProjectionMatrix = ForegroundViewProjectionMatrix;
			InvViewProjectionMatrix = InvForegoundViewProjectionMatrix;

			View->ProjectionMatrix = View->ForegroundProjectionMatrix;
		}
	}
	void EndForegroundDPG()
	{
		if( GIsGame || GIsPlayInEditorWorld )
		{
			ViewProjectionMatrix = OtherViewProjectionMatrix;
			InvViewProjectionMatrix = InvOtherViewProjectionMatrix;

			View->ProjectionMatrix = View->OtherProjectionMatrix;
		}
	}
	//!} 2006.3.6	Çã Ã¢ ¹Î
	//}}

	void FixAspectRatio(FLOAT InAspectRatio);

	/**
	 * Transforms a point into pixel coordinates.
	 *
	 * @return		FLASE if the resulting W is negative, meaning the point is behind the view.
	 */
	UBOOL Project(const FVector& V,INT& InX, INT& InY) const;

	/**
	 * Given a UMaterialInstance applied to a mesh, returns the FMaterialInstance that should be used for rendering in this view.
	 * @param	Material		The UMaterialInstance applied to the mesh.  This may be NULL, in which case GEngine->DefaultMaterial is used.
	 * @param	WireColor		The color to use for wireframe rendering.
	 * @param	Selected		True if the mesh is selected.
	 * @param	Lights			Array of FMeshElementLightInfos, used for lighting complexity coloration.
	 * @param	LevelColor		The color to use for level membership rendering.
	 */
	FMaterialInstance* GetRenderMaterial(FMaterialInstance* Material,const FColor& WireColor,UBOOL Selected,TArray<FMeshElementLightInfo>* Lights,const FColor* LevelColor) const;
	FMaterialInstance* GetRenderMaterial(UMaterialInstance* Material,const FColor& WireColor,UBOOL Selected,TArray<FMeshElementLightInfo>* Lights,const FColor* LevelColor) const;
};

/**
 * A scene containing common thumbnail code.
 */
struct FPreviewScene: FScene
{
	TArray<class UActorComponent*>		Components;
	TArray<class UPrimitiveComponent*>	Primitives;
	TArray<class ULightComponent*>		Lights;

	class UDirectionalLightComponent*		DirectionalLightComponent;

	// Constructor/destructor.

	FPreviewScene(FRotator LightRotation = FRotator(-8192,8192,0),FLOAT SkyBrightness = 0.25f,FLOAT LightBrightness = 1.0f);
	virtual ~FPreviewScene();

	// FScene interface.

	virtual void AddPrimitive(UPrimitiveComponent* Primitive);
	virtual void RemovePrimitive(UPrimitiveComponent* Primitive);
	virtual void AddLight(ULightComponent* Light);
	virtual void RemoveLight(ULightComponent* Light);
	virtual void GetVisibleComponents(const FSceneContext& Context,const FPlane& ViewOrigin,const FConvexVolume& Frustum,TArray<UPrimitiveComponent*>& OutPrimitives,TArray<ULightComponent*>& OutLights);
	virtual void GetRelevantLights(UPrimitiveComponent* Primitive,TArray<ULightComponent*>& OutLights);

	// FPreviewScene interface

	FRotator GetLightDirection();
	void SetLightDirection(const FRotator& InLightDir);
		

	// Serializer.

	friend FArchive& operator<<(FArchive& Ar,FPreviewScene& P)
	{
		return Ar << P.Components;
	}
};

/**
 * A material instance that overrides the texture of GEngine->EmissiveTexturedMaterial.
 */
class FTextureMaterialInstance : public FMaterialInstance
{
public:

	FTextureMaterialInstance(FTextureBase* InTexture);

	// FMaterial interface.
	virtual UBOOL GetTextureValue(const FName& ParameterName,FTextureBase** OutValue) const;

private:

	FTextureBase* Texture;
};

/** Keeps track of screen space mesh extents and checking for overlaps */
class FScreenPrimExtents
{
public:
	/** screen space extents for a set of primitives */
	TArray<FBox> PrimitiveExtents;
	/** accumulated extents */
	FBox TotalExtents;

	/** 
	* Constructor, default
	*/
	FScreenPrimExtents()
	{
		TotalExtents.Init();
	}

	/** 
	* Reset the list of extents
	*/
	void Init();

	/** 
	* Add extents for a new primitive 
	* @param	Primitive - prim with world bounding box extents
	* @param	WorldToScreen - screen space transfrom matrix
	*/
	void AddPrimitiveExtents( UPrimitiveComponent* Primitive, const FMatrix& WorldToScreen );
};
