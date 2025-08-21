/*=============================================================================
	StaticLightingPrivate.h: Private static lighting system definitions.
	Copyright © 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Don't compile the static lighting system on consoles.
#if !CONSOLE

/** The distance to pull back the shadow visibility traces to avoid the surface shadowing itself. */
#define SHADOW_VISIBILITY_DISTANCE_BIAS	4.0f

/** The maximum number of shadow samples per triangle. */
#define MAX_SHADOW_SAMPLES_PER_TRIANGLE	32

/** The number of hardware threads to not use for building static lighting. */
#define NUM_STATIC_LIGHTING_UNUSED_THREADS 0

/** A line segment. */
class FShadowRay
{
public:

	FVector Start;
	FVector End;
	FVector Direction;
	FVector OneOverDirection;
	FLOAT Length;

	const FStaticLightingMapping* const Mapping;
	ULightComponent* const Light;

	/** Initialization constructor. */
	FShadowRay(const FVector& InStart,const FVector& InEnd,const FStaticLightingMapping* InMapping,ULightComponent* InLight):
		Start(InStart),
		End(InEnd),
		Direction(InEnd - InStart),
		Mapping(InMapping),
		Light(InLight)
	{
		OneOverDirection.X = Square(Direction.X) > DELTA ? 1.0f / Direction.X : 0.0f;
		OneOverDirection.Y = Square(Direction.Y) > DELTA ? 1.0f / Direction.Y : 0.0f;
		OneOverDirection.Z = Square(Direction.Z) > DELTA ? 1.0f / Direction.Z : 0.0f;
		Length = 1.0f;
	}
};

/** A mesh and its bounding box. */
struct FMeshAndBounds
{
	const FStaticLightingMesh* Mesh;
	FBox BoundingBox;

	/** Initialization constructor. */
	FMeshAndBounds(const FStaticLightingMesh* InMesh):
		Mesh(InMesh)
	{
		if(Mesh)
		{
			BoundingBox = Mesh->BoundingBox;
		}
	}

	/**
	 * Checks a line segment for intersection with this mesh.
	 * @param ShadowRay - The line segment to check.
	 * @return TRUE if the line segment intersects the mesh.
	 */
	UBOOL DoesShadowRayIntersect(const FShadowRay& ShadowRay) const
	{
		return	Mesh &&
				FLineBoxIntersection(BoundingBox,ShadowRay.Start,ShadowRay.End,ShadowRay.Direction,ShadowRay.OneOverDirection) &&
				Mesh->ShouldCastShadow(ShadowRay.Light,ShadowRay.Mapping) &&
				Mesh->DoesLineSegmentIntersect(ShadowRay.Start,ShadowRay.End);
	}
};

/** Information which is cached while processing a group of coherent rays. */
class FCoherentRayCache
{
public:

	/** Initialization constructor. */
	FCoherentRayCache():
		LastHitMesh(NULL)
	{}

	/** The mesh that was last hit by a ray in this thread. */
	FMeshAndBounds LastHitMesh;
};

/** The static lighting mesh. */
class FStaticLightingAggregateMesh
{
public:

	/**
	 * Merges a mesh into the shadow mesh.
	 * @param Mesh - The mesh the triangle comes from.
	 */
	void AddMesh(const FStaticLightingMesh* Mesh);

	/** Prepares the mesh for raytracing. */
	void PrepareForRaytracing();

	/**
	 * Checks a line segment for intersection with the shadow mesh.
	 * @param ShadowRay - The line segment to check for intersection.
	 * @param CoherentRayCache - The calling thread's collision cache.
	 * @return TRUE if the line segment intersects the mesh.
	 */
	UBOOL DoesShadowRayIntersect(const FShadowRay& ShadowRay,FCoherentRayCache& CoherentRayCache) const;

private:

	/** The context of an octree node, derived from the traversal of the tree. */
	class FOctreeNodeContext
	{
	public:

		/** The bounding box of the octree node. */
		FBox BoundingBox;

		/** The center of the octree node. */
		FVector Center;

		/** The number of steps up the tree to the root node. */
		INT Depth;

		/** Root node initialization constructor. */
		FOctreeNodeContext():
			BoundingBox(FVector(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX),FVector(+HALF_WORLD_MAX,+HALF_WORLD_MAX,+HALF_WORLD_MAX)),
			Center(0,0,0),
			Depth(0)
		{}

		/** Child node initialization constructor. */
		FOctreeNodeContext(const FOctreeNodeContext& ParentContext,FLOAT ChildX,FLOAT ChildY,FLOAT ChildZ):
			Depth(ParentContext.Depth + 1)
		{
			const FVector Extent = ParentContext.BoundingBox.Max - ParentContext.Center;
			BoundingBox.Min = ParentContext.BoundingBox.Min + FVector(ChildX * Extent.X,ChildY * Extent.Y,ChildZ * Extent.Z);
			BoundingBox.Max = BoundingBox.Min + Extent;
		}
	};

	/** An octree node. */
	class FOctreeNode : public FRefCountedObject
	{
	public:

		/** Initialization constructor. */
		FOctreeNode():
			bIsLeaf(TRUE)
		{}

		/**
		 * Adds a mesh to this subtree of the octree.
		 * @param Context - The context of this node.
		 * @param Mesh - The mesh to add.
		 */
		void AddMesh(const FOctreeNodeContext& Context,const FStaticLightingMesh* Mesh);

		/**
		 * Checks a line segment for intersection with the meshes in this subtree of the octree.
		 * @param Context - The context of this node.
		 * @param ShadowRay - The line segment to check.
		 * @param CoherentRayCache - The calling thread's collision cache.
		 * @return TRUE if the line segment intersects a mesh in the subtree.
		 */
		UBOOL DoesShadowRayIntersect(const FOctreeNodeContext& Context,const FShadowRay& ShadowRay,FCoherentRayCache& CoherentRayCache) const;

	private:

		/** The meshes in this node. */
		TArray<FMeshAndBounds> Meshes;

		/** The children of the node. */
		TRefCountPtr<FOctreeNode> Children[2][2][2];

		/** TRUE if the meshes should be added directly to the node, rather than subdividing when possible. */
		BITFIELD bIsLeaf : 1;
	};

	/** The root node of the octree used to cull ray-mesh intersections. */
	FOctreeNode RootNode;

	friend class FStaticLightingAggregateMeshDataProvider;

	/** The world-space kd-tree which is used by the simple meshes in the world. */
	TkDOPTree<const FStaticLightingAggregateMeshDataProvider> kDopTree;

	/** The triangles used to build the kd-tree, valid until PrepareForRaytracing is called. */
	TArray<FkDOPBuildCollisionTriangle> kDOPTriangles;

	/** The meshes used to build the kd-tree. */
	TArray<const FStaticLightingMesh*> kDopMeshes;

	/** The vertices used by the kd-tree. */
	TArray<FVector> Vertices;
};

/** The state of the static lighting system. */
class FStaticLightingSystem
{
public:

	/**
	 * Initializes this static lighting system, and builds static lighting based on the provided options.
	 * @param InOptions - The static lighting build options.
	 */
	FStaticLightingSystem(const FLightingBuildOptions& InOptions);

	/**
	 * Calculates shadowing for a given mapping surface point and light.
	 * @param Mapping - The mapping the point comes from.
	 * @param WorldSurfacePoint - The point to check shadowing at.
	 * @param Light - The light to check shadowing from.
	 * @param CoherentRayCache - The calling thread's collision cache.
	 * @return TRUE if the surface point is shadowed from the light.
	 */
	UBOOL CalculatePointShadowing(const FStaticLightingMapping* Mapping,const FVector& WorldSurfacePoint,ULightComponent* Light,FCoherentRayCache& CoherentRayCache) const;

	/**
	 * Calculates the lighting contribution of a light to a mapping vertex.
	 * @param Mapping - The mapping the vertex comes from.
	 * @param Vertex - The vertex to calculate the lighting contribution at.
	 * @param Light - The light to calculate the lighting contribution from.
	 * @return The incident lighting on the vertex.
	 */
	FLightSample CalculatePointLighting(const FStaticLightingMapping* Mapping,const FStaticLightingVertex& Vertex,ULightComponent* Light) const;

private:

	/** A thread which processes static lighting mappings. */
	class FStaticLightingThreadRunnable : public FRunnable
	{
	public:

		FRunnableThread* Thread;

		/** Initialization constructor. */
		FStaticLightingThreadRunnable(FStaticLightingSystem* InSystem):
			System(InSystem)
		{}

		// FRunnable interface.
		virtual UBOOL Init(void) { return TRUE; }
		virtual void Exit(void) {}
		virtual void Stop(void) {}
		virtual DWORD Run(void)
		{
			System->ThreadLoop(FALSE);
			return 0;
		}

	private:
		FStaticLightingSystem* System;
	};

	/** The static lighting data for a vertex mapping. */
	struct FVertexMappingStaticLightingData
	{
		FStaticLightingVertexMapping* Mapping;
		FLightMapData1D* LightMapData;
		TMap<ULightComponent*,FShadowMapData1D*> ShadowMaps;
	};

	/** The static lighting data for a texture mapping. */
	struct FTextureMappingStaticLightingData
	{
		FStaticLightingTextureMapping* Mapping;
		FLightMapData2D* LightMapData;
		TMap<ULightComponent*,FShadowMapData2D*> ShadowMaps;
	};

	/** Encapsulates a list of mappings which static lighting has been computed for, but not yet applied. */
	template<typename StaticLightingDataType>
	class TCompleteStaticLightingList
	{
	public:

		/** Initialization constructor. */
		TCompleteStaticLightingList():
			FirstElement(NULL)
		{}

		/** Adds an element to the list. */
		void AddElement(TList<StaticLightingDataType>* Element)
		{
			// Link the element at the beginning of the list.
			TList<StaticLightingDataType>* LocalFirstElement;
			do 
			{
				LocalFirstElement = FirstElement;
				Element->Next = LocalFirstElement;
			}
			while(appInterlockedCompareExchangePointer((void**)&FirstElement,Element,LocalFirstElement) != LocalFirstElement);
		}

		/** Applies the static lighting to the mappings in the list, and clears the list. */
		void ApplyAndClear();

	private:

		TList<StaticLightingDataType>* FirstElement;
	};

	/** The lights in the world which the system is building. */
	TArray<ULightComponent*> Lights;

	/** The options the system is building lighting with. */
	const FLightingBuildOptions Options;

	/** TRUE if the static lighting build has been canceled.  Written by the main thread, read by all static lighting threads. */
	UBOOL bBuildCanceled;

	/** The aggregate mesh used for raytracing. */
	FStaticLightingAggregateMesh AggregateMesh;

	/** All meshes in the system. */
	TArray< TRefCountPtr<FStaticLightingMesh> > Meshes;

	/** All mappings in the system. */
	TArray< TRefCountPtr<FStaticLightingMapping> > Mappings;

	/** The next index into Mappings which processing hasn't started for yet. */
	FThreadSafeCounter NextMappingToProcess;

	/** A list of the vertex mappings which static lighting has been computed for, but not yet applied.  This is accessed by multiple threads and should be written to using interlocked functions. */
	TCompleteStaticLightingList<FVertexMappingStaticLightingData> CompleteVertexMappingList;

	/** A list of the texture mappings which static lighting has been computed for, but not yet applied.  This is accessed by multiple threads and should be written to using interlocked functions. */
	TCompleteStaticLightingList<FTextureMappingStaticLightingData> CompleteTextureMappingList;

	/** The threads spawned by the static lighting system. */
	TIndirectArray<FStaticLightingThreadRunnable> Threads;

	/**
	 * Queries a primitive for its static lighting info, and adds it to the system.
	 * @param Primitive - The primitive to query.
	 * @param bBuildLightingForPrimitive - TRUE if lighting needs to be built for this primitive.
	 */
	void AddPrimitiveStaticLightingInfo(UPrimitiveComponent* Primitive,UBOOL bBuildLightingForPrimitive);

	/**
	 * Builds lighting for a vertex mapping.
	 * @param VertexMapping - The mapping to build lighting for.
	 */
	void ProcessVertexMapping(FStaticLightingVertexMapping* VertexMapping);

	/**
	 * Builds lighting for a texture mapping.
	 * @param TextureMapping - The mapping to build lighting for.
	 */
	void ProcessTextureMapping(FStaticLightingTextureMapping* TextureMapping);

	/**
	 * The processing loop for a static lighting thread.
	 * @param bIsMainThread - TRUE if this is running in the main thread.
	 */
	void ThreadLoop(UBOOL bIsMainThread);
};

/**
 * Given a mapping, sorts its relevant lights into shadow-mapped and light-mapped lights.
 * @param Mapping - The mapping to sort the lights for.
 * @param OutLightMappedLights - Upon return, the lights which may be added to the light-map.
 * @param OutShadowMappedLights - Upon return the lights which need to have their own shadow-map.
 */
extern void SortMappingLightsByLightingType(const FStaticLightingMapping* Mapping,TArray<ULightComponent*>& OutLightMappedLights,TArray<ULightComponent*>& OutShadowMappedLights);

/**
 * Checks if a light is behind a triangle.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Light - The light to classify.
 * @return TRUE if the light is behind the triangle.
 */
extern UBOOL IsLightBehindSurface(const FVector& TrianglePoint,const FVector& TriangleNormal,const ULightComponent* Lights);

/**
 * Culls lights that are behind a triangle.
 * @param TrianglePoint - Any point on the triangle.
 * @param TriangleNormal - The (not necessarily normalized) triangle surface normal.
 * @param Lights - The lights to cull.
 * @return A map from Lights index to a boolean which is TRUE if the light is in front of the triangle.
 */
extern FBitArray CullBackfacingLights(const FVector& TrianglePoint,const FVector& TriangleNormal,const TArray<ULightComponent*>& Lights);

#endif
