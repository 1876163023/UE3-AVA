/*=============================================================================
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNDECALRENDERDATA_H__
#define __UNDECALRENDERDATA_H__

#include "UnSkeletalMesh.h"		// For FRigidSkinVertex and FSoftSkinVertex

// Forward declarations.
class FDecalRenderData;

/**
 * Class for representing the state of a decal, to be used in the rendering thread by
 * receiving primitives for intersecting with their geometry.
 */
class FDecalState
{
public:
	const UDecalComponent*	DecalComponent;
	UMaterialInstance*		DecalMaterial;

	FVector OrientationVector;
	FVector HitLocation;
	FVector HitNormal;
	FVector HitTangent;
	FVector HitBinormal;

	FVector FrustumVerts[8];

	FLOAT OffsetX;
	FLOAT OffsetY;

	FLOAT Thickness;
	FLOAT Width;
	FLOAT Height;

	FLOAT DepthBias;
	FLOAT SlopeScaleDepthBias;
	INT SortOrder;

	TArray<FPlane> Planes;
	/** Matrix that transforms world-space positions to decal texture coordinates. */
	FMatrix WorldTexCoordMtx;
	FMatrix DecalFrame;
	FMatrix WorldToDecal;

	/** Name of the bone that was hit. */
	FName HitBone;
	/** Index of the hit bone. */
	INT HitBoneIndex;
	/** Index of the hit BSP node. */
	INT HitNodeIndex;
	/** If not -1, specifies the level into the world's level array of the BSP node that was hit. */
	INT HitLevelIndex;

	TArray<AActor*> Filter;
	BYTE FilterMode;

	BYTE DepthPriorityGroup;
	BITFIELD bNoClip:1;
	BITFIELD bProjectOnBackfaces:1;
	BITFIELD bProjectOnBSP:1;
	BITFIELD bProjectOnStaticMeshes:1;
	BITFIELD bProjectOnSkeletalMeshes:1;
	BITFIELD bProjectOnTerrain:1;
	BITFIELD bHasUnlitTranslucency:1;
	BITFIELD bHasUnlitDistortion:1;
	/** TRUE if the decal uses a lit material. */
	BITFIELD bIsLit:1;

	/**
	 * computes an axis-aligned bounding box of the decal frustum vertices projected to the screen.
	 *
	 * @param		SceneView		Scene projection
	 * @param		OutMin			[out] Min vertex of the screen AABB.
	 * @param		OutMax			[out] Max vertex of the screen AABB.
	 * @return						FALSE if the AABB has zero area, TRUE otherwise.
	 */
	UBOOL QuadToClippedScreenSpaceAABB(const class FSceneView* SceneView, FVector2D& OutMin, FVector2D& OutMax) const;
};

class FDecalInteraction
{
public:
	UDecalComponent*	Decal;
	FDecalRenderData*	RenderData;
	FDecalState			DecalState;

	FDecalInteraction(UDecalComponent* InDecal, FDecalRenderData* InRenderData)
		:	Decal( InDecal )
		,	RenderData( InRenderData )
	{}

	FDecalInteraction()
		:	Decal( NULL )
		,	RenderData( NULL )
	{}
};

struct FDecalVertex
{
	FVector			Position;
	FPackedNormal	TangentX;
	FPackedNormal	TangentY;
	FPackedNormal	TangentZ;

	/** Decal mesh texture coordinates. */
	FVector2D		UV;	

	/** Decal vertex light map coordinated.  Added in engine version VER_DECAL_ADDED_DECAL_VERTEX_LIGHTMAP_COORD. */
	FVector2D		LightMapCoordinate;

	/** Transforms receiver tangent basis into decal tangent basis for normal map lookup. */
	FVector2D		NormalTransform0;
	FVector2D		NormalTransform1;

	FDecalVertex() {}	
	FDecalVertex(const FVector& InPosition,
		const FPackedNormal& InTangentX,
		const FPackedNormal& InTangentY,
		const FPackedNormal& InTangentZ,
		const FVector2D& InUV,		
		const FVector2D& InNormalTransform0,
		const FVector2D& InNormalTransform1)
		:	Position( InPosition )
		,	TangentX( InTangentX )
		,	TangentY( InTangentY )
		,	TangentZ( InTangentZ )
		,	UV( InUV )		
		,	NormalTransform0( InNormalTransform0 )
		,	NormalTransform1( InNormalTransform1 )
	{}
	FDecalVertex(const FVector& InPosition,
		const FPackedNormal& InTangentX,
		const FPackedNormal& InTangentY,
		const FPackedNormal& InTangentZ,
		const FVector2D& InUV,
		const FVector2D& InNormalTransform0,
		const FVector2D& InNormalTransform1,
		const FVector2D& InLightmapUV )
		:	Position( InPosition )
		,	TangentX( InTangentX )
		,	TangentY( InTangentY )
		,	TangentZ( InTangentZ )
		,	UV( InUV )
		,	NormalTransform0( InNormalTransform0 )
		,	NormalTransform1( InNormalTransform1 )
		,	LightMapCoordinate( InLightmapUV )
	{}
};

/**
 * FDecalVertex serializer.
 */
FArchive& operator<<(FArchive& Ar, FDecalVertex& DecalVertex);

///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Vertex>
class FDecalVertexBuffer : public FVertexBuffer
{
public:
	typedef Vertex	VertexType;

	/**
	* @param	InDecalRenderData	Pointer to parent structure.
	*/
	FDecalVertexBuffer(FDecalRenderData* InDecalRenderData)
		:	DecalRenderData( InDecalRenderData )
	{}

	// FRenderResource interface.
	virtual void InitRHI();

	virtual FString GetFriendlyName()
	{
		return FString( TEXT("Decal vertex buffer") );
	}

protected:
	/** Pointer to parent structure, used in GetData() and associated functions. */
	FDecalRenderData*	DecalRenderData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FDecalRenderData
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FDecalRenderData 
{
public:
	typedef FDecalVertex			VertexType;
	typedef TArray<VertexType>		VertexArray;

	FDecalRenderData(FLightCacheInterface* InLCI=NULL, UBOOL bInUsesVertexResources=TRUE, UBOOL bInUsesIndexResources=TRUE)
		:	DecalVertexBuffer( this )
		,	NumTriangles( 0 )
		,	Data( 0 )
		,	LCI( InLCI )
		,	bUsesVertexResources( bInUsesVertexResources )
		,	bUsesIndexResources( bInUsesIndexResources )
	{}

	/**
	 * Prepares resources for deletion.
	 * This is only called by the rendering thread.
	 */
	~FDecalRenderData();

	/**
	 * Initializes resources.
	 * This is only called by the game thread, when a receiver is attached to a decal.
	 */
	void InitResources();

	/**
	 * Add a vertex to the decal vertex buffer.
	 *
	 * @param	Vertex		The vertex to add.
	 */
	FORCEINLINE void AddVertex(const VertexType& Vertex)
	{
		Vertices.AddItem( Vertex );
	}

	/**
	 * Add an index to the decal index buffer.
	 *
	 * @param	Index		The index to add.
	 */
	FORCEINLINE void AddIndex(WORD Index)
	{
		IndexBuffer.Indices.AddItem( Index );
	}

	/**
	 * Returns the number of vertices.
	 */
	FORCEINLINE INT GetNumVertices() const
	{
		return Vertices.Num();
	}

	/**
	 * Returns the number of indices.
	 */
	FORCEINLINE INT GetNumIndices() const
	{
		return IndexBuffer.Indices.Num();
	}	

	/**
	 * FDecalRenderData serializer.
	 */
	friend FArchive& operator<<(FArchive& Ar, FDecalRenderData& DecalRenderData);

	/** Source data. */
	VertexArray						Vertices;

	/** Vertex buffer. */
	FDecalVertexBuffer<VertexType>	DecalVertexBuffer;

	/** Decal vertex factory. */
	FLocalVertexFactory				VertexFactory;	

	/** New index buffer used in Gemini. */
	FRawIndexBuffer					IndexBuffer;

	/** Number of triangles represented by this vertex set. */
	UINT							NumTriangles;

	/** Members available for decal receiver-specific use. */
	INT								Data;

	/** The static lighting cache for the mesh the decal is attached to. */
	FLightCacheInterface*			LCI;

	/**
	 * Vertex lightmap for the decal.  Used by e.g. decals on vertex lightmapped static meshes,
	 * where the mesh'light map samples are resampled according to the decal vertex indices.
	 * If NULL and the decal material is lit, the receiver's light map will be used.
	 */
	FLightMapRef					LightMap1D;

	/** 'Rigid' vertices for decals on CPU-skinned meshes. */
	TArray<FRigidSkinVertex>		RigidVertices;
	/** 'Soft' vertices for decals on CPU-skinned meshes. */
	TArray<FSoftSkinVertex>			SoftVertices;

	/** Used to indicate whether the FDecalRenderData's vertex buffer/factory resources are used. */
	BITFIELD						bUsesVertexResources:1;
	/** Used to indicate whether the FDecalRenderData's index buffer resources should are used. */
	BITFIELD						bUsesIndexResources:1;

private:
	/**
	 * Prepares resources for deletion.
	 * This is only called by the rendering thread.
	 */
	void ReleaseResources();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FStaticReceiverData
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FStaticReceiverData
{
public:
	/** Vertex array type. */
	typedef FDecalRenderData::VertexArray	VertexArray;

	/** The receiving component. */
	UPrimitiveComponent*	Component;
	/** Source vertex data. */
	VertexArray				Vertices;
	/** Index buffer. */
	TArray<WORD>			Indices;
	/** Number of decal triangles. */
	UINT					NumTriangles;
	/** Lightmap. */
	FLightMapRef			LightMap1D;

	FStaticReceiverData()
		: NumTriangles( 0 )
	{}

	/**
	 * FStaticReceiverData serializer.
	 */
	friend FArchive& operator<<(FArchive& Ar, FStaticReceiverData& Tgt);
};

/** Copies data from an FStaticReceiverData instance to an FDecalRenderData instance. */
FORCEINLINE void CopyStaticReceiverDataToDecalRenderData(FDecalRenderData& Dst, const FStaticReceiverData& Src)
{
	Dst.Vertices				= Src.Vertices;
	Dst.IndexBuffer.Indices		= Src.Indices;
	Dst.NumTriangles			= Src.NumTriangles;
	Dst.LightMap1D				= Src.LightMap1D;
}

/** Copies data from an FDecalRenderData instance to an FStaticReceiverData instance. */
FORCEINLINE void CopyDecalRenderDataToStaticReceiverData(FStaticReceiverData& Dst, const FDecalRenderData& Src)
{
	Dst.Vertices		= Src.Vertices;
	Dst.Indices			= Src.IndexBuffer.Indices;
	Dst.NumTriangles	= Src.NumTriangles;
	Dst.LightMap1D		= Src.LightMap1D;
}

/**
 * Helper classes for UPrimitiveComponent::GenerateDecalRenderData implementations.
 */
class FDecalLocalSpaceInfo
{
public:
	const FDecalState& Decal;
	const FMatrix& ReceiverWorldToLocal;
	FConvexVolume Convex;
	FVector LocalLookVector;
	FVector LocalLocation;
	FVector LocalTangent;
	FVector LocalBinormal;

	FDecalLocalSpaceInfo(const FDecalState& InDecal, const FMatrix& InReceiverWorldToLocal)
		:	Decal( InDecal )
		,	ReceiverWorldToLocal( InReceiverWorldToLocal )
	{
		for ( INT PlaneIndex = 0 ; PlaneIndex < Decal.Planes.Num() ; ++PlaneIndex )
		{
			Convex.Planes.AddItem( Decal.Planes(PlaneIndex).TransformBy( ReceiverWorldToLocal ) );
		}

		LocalLookVector = ReceiverWorldToLocal.TransformNormal( Decal.OrientationVector ).SafeNormal();
		LocalLocation = ReceiverWorldToLocal.TransformFVector( Decal.HitLocation );
		LocalTangent = ReceiverWorldToLocal.TransformNormal( Decal.HitTangent ).SafeNormal();
		LocalBinormal = ReceiverWorldToLocal.TransformNormal( Decal.HitBinormal ).SafeNormal();
	}

	/**
	 * Computes decal texture coordinates from the the specified world-space position.
	 *
	 * @param		InPos			World-space position.
	 * @param		OutTexCoords	[out] Decal texture coordinates.
	 */
	void ComputeTextureCoordinates(const FVector& InPos, FVector2D& OutTexCoords) const
	{
		const FVector OutPos = Decal.WorldTexCoordMtx.TransformFVector(InPos-Decal.HitLocation);
		OutTexCoords.X = (-OutPos.X+0.5f+Decal.OffsetX);
		OutTexCoords.Y = (-OutPos.Y+0.5f+Decal.OffsetY);
	}
};

/**
 * CPU Clipper for decal polygons.
 */
class FDecalPoly
{
public:
	FVector			  FaceNormal;
	TArray<FVector>	  Vertices;
	TArray<INT>		  Indices;

	//<@ ava specific ; 2006. 10. 13 changmin
	TArray<FVector2D>	LightmapCoords;
	//>@ ava

	//<@ ava specific ; 2006. 10. 16 changmin
	TArray<FQuantizedLightSample> VertexLights;
	//>@ ava

	void Init()
	{
		Vertices.Reset();
		Indices.Reset();

		//<@ ava specific ; 2006. 10. 13 changmin
		LightmapCoords.Reset();
		//>@ ava

		//<@ ava specific ; 2006. 10. 16 changmin
		VertexLights.Reset();
		//>@ ava
	}

	// Returns non-zero if a problem occurs.
	INT CalcNormal()
	{
		checkSlow( Vertices.Num() == 3 );
		FaceNormal = (Vertices(1) - Vertices(0)) ^ (Vertices(2) - Vertices(0));

		static const FLOAT DECAL_THRESH_ZERO_NORM_SQUARED = 0.0001f;
		if( FaceNormal.SizeSquared() < DECAL_THRESH_ZERO_NORM_SQUARED )
		{
			return 1;
		}
		FaceNormal.Normalize();
		return 0;
	}

	UBOOL ClipAgainstConvex(const FConvexVolume& Convex)
	{
		for( INT PlaneIndex = 0 ; PlaneIndex < Convex.Planes.Num() ; ++PlaneIndex )
		{
			const FPlane&	Plane = Convex.Planes(PlaneIndex);
			if( !Split(-FVector(Plane),Plane * Plane.W) )
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	// Split a poly and keep only the front half.
	// @return		Number of vertices, 0 if clipped away.
	INT Split( const FVector &Normal, const FVector &Base );

	/**
	* Split with plane quickly for in-game geometry operations.
	* Results are always valid. May return sliver polys.
	*/
	INT SplitWithPlaneFast(const FPlane& Plane, FDecalPoly* FrontPoly) const;

	UBOOL PartOfPolyInside(const FConvexVolume& Convex) const;
};


/*
 * FRenderResource interface for FDecalVertexBuffer.
 * Has to be defined after FDecalRenderData has been properly declared.
 */
template<typename Vertex>
void FDecalVertexBuffer<Vertex>::InitRHI()
{
	const INT NumVerts = DecalRenderData->Vertices.Num();
	if ( NumVerts > 0 )
	{
		const UINT Size = NumVerts*sizeof(VertexType);
		VertexBufferRHI = RHICreateVertexBuffer( Size, NULL, FALSE );
		void* DestBuf = RHILockVertexBuffer( VertexBufferRHI, 0, Size );
		appMemcpy( DestBuf, DecalRenderData->Vertices.GetData(), Size );
		RHIUnlockVertexBuffer( VertexBufferRHI );
	}
}


#endif // __UNDECALRENDERDATA_H__
