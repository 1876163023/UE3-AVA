/*=============================================================================
UnStaticMeshLight.cpp: Static mesh lighting code.
Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRaster.h"

/**
 * Creates a static lighting vertex to represent the given static mesh vertex.
 * @param VertexBuffer - The static mesh's vertex buffer.
 * @param VertexIndex - The index of the static mesh vertex to access.
 * @param OutVertex - Upon return, contains a static lighting vertex representing the specified static mesh vertex.
 */
static void GetStaticLightingVertex(
	const FPositionVertexBuffer& PositionVertexBuffer,
	const FStaticMeshVertexBuffer& VertexBuffer,
	UINT VertexIndex,
	const FMatrix& LocalToWorld,
	const FMatrix& LocalToWorldInverseTranspose,
	FStaticLightingVertex& OutVertex
	)
{
	OutVertex.WorldPosition = LocalToWorld.TransformFVector(PositionVertexBuffer.VertexPosition(VertexIndex));
	OutVertex.WorldTangentX = LocalToWorld.TransformNormal(VertexBuffer.VertexTangentX(VertexIndex)).SafeNormal();
	OutVertex.WorldTangentY = LocalToWorld.TransformNormal(VertexBuffer.VertexTangentY(VertexIndex)).SafeNormal();
	OutVertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(VertexBuffer.VertexTangentZ(VertexIndex)).SafeNormal();

	checkSlow(VertexBuffer.GetNumTexCoords() <= ARRAY_COUNT(OutVertex.TextureCoordinates));
	for(UINT TextureCoordinateIndex = 0;TextureCoordinateIndex < VertexBuffer.GetNumTexCoords();TextureCoordinateIndex++)
	{
		OutVertex.TextureCoordinates[TextureCoordinateIndex] = VertexBuffer.VertexUV(VertexIndex,TextureCoordinateIndex);
	}
}


/** Represents the triangles of one LOD of a static mesh primitive to the static lighting system. */
class FStaticMeshStaticLightingMesh : public FStaticLightingMesh
{
public:

	/** The meshes representing other LODs of this primitive. */
	TArray<FStaticLightingMesh*> OtherLODs;

	/** Initialization constructor. */
	FStaticMeshStaticLightingMesh(const UStaticMeshComponent* InPrimitive,INT InLODIndex,const TArray<ULightComponent*>& InRelevantLights):
		FStaticLightingMesh(
			InPrimitive->StaticMesh->LODModels(InLODIndex).GetTriangleCount(),
			InPrimitive->StaticMesh->LODModels(InLODIndex).NumVertices,
			InPrimitive->CastShadow,
			InRelevantLights,
			InPrimitive->Bounds.GetBox()
			),
		StaticMesh(InPrimitive->StaticMesh),
		Primitive(InPrimitive),
		LODIndex(InLODIndex),
		LocalToWorldInverseTranspose(InPrimitive->LocalToWorld.Inverse().Transpose()),
		bReverseWinding(InPrimitive->LocalToWorldDeterminant < 0.0f)
	{
	}

	// FStaticLightingMesh interface.

	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const
	{
		const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

		// Lookup the triangle's vertex indices.
		const WORD I0 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + 0);
		const WORD I1 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 2 : 1));
		const WORD I2 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 1 : 2));

		// Translate the triangle's static mesh vertices to static lighting vertices.
		GetStaticLightingVertex(LODRenderData.PositionVertexBuffer,LODRenderData.VertexBuffer,I0,Primitive->LocalToWorld,LocalToWorldInverseTranspose,OutV0);
		GetStaticLightingVertex(LODRenderData.PositionVertexBuffer,LODRenderData.VertexBuffer,I1,Primitive->LocalToWorld,LocalToWorldInverseTranspose,OutV1);
		GetStaticLightingVertex(LODRenderData.PositionVertexBuffer,LODRenderData.VertexBuffer,I2,Primitive->LocalToWorld,LocalToWorldInverseTranspose,OutV2);
	}

	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

		// Lookup the triangle's vertex indices.
		OutI0 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + 0);
		OutI1 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 2 : 1));
		OutI2 = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + (bReverseWinding ? 1 : 2));
	}

	virtual UBOOL ShouldCastShadow(ULightComponent* Light,const FStaticLightingMapping* Receiver) const
	{
		// If the receiver is the same primitive but a different LOD, don't cast shadows on it.
		if(OtherLODs.ContainsItem(Receiver->Mesh))
		{
			return FALSE;
		}
		else
		{
			return FStaticLightingMesh::ShouldCastShadow(Light,Receiver);
		}
	}

	virtual UBOOL IsUniformShadowCaster() const
	{
		// If this mesh is one of multiple LODs, it won't uniformly shadow all of them.
		return OtherLODs.Num() == 0 && FStaticLightingMesh::IsUniformShadowCaster();
	}

	virtual UBOOL DoesLineSegmentIntersect(const FVector& Start,const FVector& End) const
	{
		// Create the object that knows how to extract information from the component/mesh
		FStaticMeshCollisionDataProvider Provider(Primitive);

		// Create the check structure with all the local space fun
		FCheckResult Result(1.0f);
		TkDOPLineCollisionCheck<FStaticMeshCollisionDataProvider> kDOPCheck(Start,End,TRACE_StopAtAnyHit,Provider,&Result);

		// Do the line check
		return StaticMesh->kDOPTree.LineCheck(kDOPCheck);
	}

private:

	/** The static mesh this mesh represents. */
	const UStaticMesh* StaticMesh;
	
	/** The primitive this mesh represents. */
	const UStaticMeshComponent* const Primitive;

	/** The LOD this mesh represents. */
	const INT LODIndex;

	/** The inverse transpose of the primitive's local to world transform. */
	const FMatrix LocalToWorldInverseTranspose;

	/** TRUE if the primitive has a transform which reverses the winding of its triangles. */
	const BITFIELD bReverseWinding : 1;
};

/** Represents a static mesh primitive with texture mapped static lighting. */
class FStaticMeshStaticLightingTextureMapping : public FStaticLightingTextureMapping
{
public:

	/** Initialization constructor. */
	FStaticMeshStaticLightingTextureMapping(UStaticMeshComponent* InPrimitive,INT InLODIndex,FStaticLightingMesh* InMesh,INT InSizeX,INT InSizeY,INT InTextureCoordinateIndex,UBOOL bPerformFullQualityRebuild):
		FStaticLightingTextureMapping(
			InMesh,
			InPrimitive,
			bPerformFullQualityRebuild ? InSizeX : InSizeX / 2,
			bPerformFullQualityRebuild ? InSizeY : InSizeY / 2,
			InTextureCoordinateIndex,
			InPrimitive->bForceDirectLightMap
			),
		Primitive(InPrimitive),
		LODIndex(InLODIndex)
	{}

	// FStaticLightingTextureMapping interface
	virtual void Apply(FLightMapData2D* LightMapData,const TMap<ULightComponent*,FShadowMapData2D*>& ShadowMapData)
	{
		// Determine the material to use for grouping the light-maps and shadow-maps.
		UMaterialInstance* const Material = Primitive->GetNumElements() == 1 ? Primitive->GetMaterial(0) : NULL;

		// Ensure LODData has enough entries in it.
		if(LODIndex >= Primitive->LODData.Num())
		{
			Primitive->LODData.AddZeroed(LODIndex + 1 - Primitive->LODData.Num());
		}
		FStaticMeshComponentLODInfo& ComponentLODInfo = Primitive->LODData(LODIndex);

		// Create a light-map for the primitive.
		ComponentLODInfo.LightMap = FLightMap2D::AllocateLightMap(
			Primitive,
			LightMapData,
			LightMapData,	// compile 되도록 수정, 사용하는 않는 코드 영역임.
			FLightMap2D::NeedsBumpedLightmap(Material),
			Primitive->Bounds,
			FALSE
			);
		delete LightMapData;

		// Create the shadow-maps for the primitive.
		ComponentLODInfo.ShadowMaps.Empty(ShadowMapData.Num());
		for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
		{
			ComponentLODInfo.ShadowMaps.AddItem(
				new(Owner) UShadowMap2D(
					*ShadowMapDataIt.Value(),
					ShadowMapDataIt.Key()->LightGuid,
					Material,
					Primitive->Bounds
					)
				);
			delete ShadowMapDataIt.Value();
		}

		// Build the list of statically irrelevant lights.
		// TODO: This should be stored per LOD.
		Primitive->IrrelevantLights.Empty();
		for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
		{
			const ULightComponent* Light = Mesh->RelevantLights(LightIndex);

			// Check if the light is stored in the light-map.
			const UBOOL bIsInLightMap = ComponentLODInfo.LightMap && ComponentLODInfo.LightMap->LightGuids.ContainsItem(Light->LightmapGuid);

			// Check if the light is stored in the shadow-map.
			UBOOL bIsInShadowMap = FALSE;
			for(INT LightIndex = 0;LightIndex < ComponentLODInfo.ShadowMaps.Num();LightIndex++)
			{
				if(ComponentLODInfo.ShadowMaps(LightIndex)->GetLightGuid() == Light->LightGuid)
				{
					bIsInShadowMap = TRUE;
					break;
				}
			}

			// Add the light to the statically irrelevant light list if it is in the potentially relevant light list, but didn't contribute to the light-map or a shadow-map.
			if(!bIsInLightMap && !bIsInShadowMap)
			{	
				Primitive->IrrelevantLights.AddUniqueItem(Light->LightGuid);
			}
		}

		// Mark the primitive's package as dirty.
		Primitive->MarkPackageDirty();
	}

private:

	/** The primitive this mapping represents. */
	UStaticMeshComponent* const Primitive;

	/** The LOD this mapping represents. */
	const INT LODIndex;
};

/** Represents a static mesh primitive with vertex mapped static lighting. */
class FStaticMeshStaticLightingVertexMapping : public FStaticLightingVertexMapping
{
public:

	/** Initialization constructor. */
	FStaticMeshStaticLightingVertexMapping(UStaticMeshComponent* InPrimitive,INT InLODIndex,FStaticLightingMesh* InMesh,UBOOL bPerformFullQualityBuild):
		FStaticLightingVertexMapping(
			InMesh,
			InPrimitive,
			InPrimitive->bForceDirectLightMap,
			1.0f / Square((FLOAT)InPrimitive->SubDivisionStepSize),
			!(bPerformFullQualityBuild && InPrimitive->bUseSubDivisions)
			),
		Primitive(InPrimitive),
		LODIndex(InLODIndex)
	{}

	// FStaticLightingTextureMapping interface
	virtual void Apply(FLightMapData1D* LightMapData,const TMap<ULightComponent*,FShadowMapData1D*>& ShadowMapData)
	{
		// Ensure LODData has enough entries in it.
		if(LODIndex >= Primitive->LODData.Num())
		{
			Primitive->LODData.AddZeroed(LODIndex + 1 - Primitive->LODData.Num());
		}
		FStaticMeshComponentLODInfo& ComponentLODInfo = Primitive->LODData(LODIndex);

		// Create a light-map for the primitive.
		ComponentLODInfo.LightMap = new FLightMap1D(Primitive,*LightMapData);
		delete LightMapData;

		// Create the shadow-maps for the primitive.
		ComponentLODInfo.ShadowMaps.Empty(ShadowMapData.Num());
		for(TMap<ULightComponent*,FShadowMapData1D*>::TConstIterator ShadowMapDataIt(ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
		{
			ComponentLODInfo.ShadowVertexBuffers.AddItem(new(Owner) UShadowMap1D(ShadowMapDataIt.Key()->LightGuid,*ShadowMapDataIt.Value()));
			delete ShadowMapDataIt.Value();
		}

		// Build the list of statically irrelevant lights.
		// TODO: This should be stored per LOD.
		Primitive->IrrelevantLights.Empty();
		for(INT LightIndex = 0;LightIndex < Mesh->RelevantLights.Num();LightIndex++)
		{
			const ULightComponent* Light = Mesh->RelevantLights(LightIndex);

			// Check if the light is stored in the light-map.
			const UBOOL bIsInLightMap = ComponentLODInfo.LightMap && ComponentLODInfo.LightMap->LightGuids.ContainsItem(Light->LightmapGuid);

			// Check if the light is stored in the shadow-map.
			UBOOL bIsInShadowMap = FALSE;
			for(INT LightIndex = 0;LightIndex < ComponentLODInfo.ShadowVertexBuffers.Num();LightIndex++)
			{
				if(ComponentLODInfo.ShadowVertexBuffers(LightIndex)->GetLightGuid() == Light->LightGuid)
				{
					bIsInShadowMap = TRUE;
					break;
				}
			}

			// Add the light to the statically irrelevant light list if it is in the potentially relevant light list, but didn't contribute to the light-map or a shadow-map.
			if(!bIsInLightMap && !bIsInShadowMap)
			{	
				Primitive->IrrelevantLights.AddUniqueItem(Light->LightGuid);
			}
		}
		
		// Mark the primitive's package as dirty.
		Primitive->MarkPackageDirty();
	}

private:

	/** The primitive this mapping represents. */
	UStaticMeshComponent* const Primitive;

	/** The LOD this mapping represents. */
	const INT LODIndex;
};

void UStaticMeshComponent::GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options)
{
	// Determine whether this static mesh has static shadowing while it is still attached to its owner actor.
	const UBOOL bHasStaticShadowing = HasStaticShadowing();

	if( StaticMesh && bHasStaticShadowing )
	{
		// Process each LOD separately.
		TArray<FStaticMeshStaticLightingMesh*> StaticLightingMeshes;
		for(INT LODIndex = 0;LODIndex < StaticMesh->LODModels.Num();LODIndex++)
		{
			const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

			// Figure out whether we are storing the lighting/ shadowing information in a texture or vertex buffer.
			UBOOL	bUseTextureMap;
			INT		LightMapWidth	= 0;
			INT		LightMapHeight	= 0;
			GetLightMapResolution( LightMapWidth, LightMapHeight );

			if( (LightMapWidth > 0) && (LightMapHeight > 0) 
			&&	StaticMesh->LightMapCoordinateIndex >= 0 
			&&	(UINT)StaticMesh->LightMapCoordinateIndex < LODRenderData.VertexBuffer.GetNumTexCoords()
			)
			{
				bUseTextureMap = TRUE;
			}
			else
			{
				bUseTextureMap = FALSE;
			}

			// Create a static lighting mesh for the LOD.
			FStaticMeshStaticLightingMesh* StaticLightingMesh = new FStaticMeshStaticLightingMesh(this,LODIndex,InRelevantLights);
			OutPrimitiveInfo.Meshes.AddItem(StaticLightingMesh);
			StaticLightingMeshes.AddItem(StaticLightingMesh);

			if(bUseTextureMap)
			{
				// Create a static lighting texture mapping for the LOD.
				OutPrimitiveInfo.Mappings.AddItem(new FStaticMeshStaticLightingTextureMapping(this,LODIndex,StaticLightingMesh,LightMapWidth,LightMapHeight,StaticMesh->LightMapCoordinateIndex,Options.bPerformFullQualityBuild));
			}
			else
			{
				// Create a static lighting vertex mapping for the LOD.
				OutPrimitiveInfo.Mappings.AddItem(new FStaticMeshStaticLightingVertexMapping(this,LODIndex,StaticLightingMesh,Options.bPerformFullQualityBuild));
			}
		}

		// Give each LOD's static lighting mesh a list of the other LODs of this primitive, so they can disallow shadow casting between LODs.
		for(INT MeshIndex = 0;MeshIndex < StaticLightingMeshes.Num();MeshIndex++)
		{
			for(INT OtherMeshIndex = 0;OtherMeshIndex < StaticLightingMeshes.Num();OtherMeshIndex++)
			{
				if(MeshIndex != OtherMeshIndex)
				{
					StaticLightingMeshes(MeshIndex)->OtherLODs.AddItem(StaticLightingMeshes(OtherMeshIndex));
				}
			}
		}
	}
}

void UStaticMeshComponent::InvalidateLightingCache()
{
	// Save the static mesh state for transactions.
	Modify();

	// Mark lighting as requiring a rebuilt.
	MarkLightingRequiringRebuild();

	// Detach the component from the scene for the duration of this function.
	FComponentReattachContext ReattachContext(this);

	// Discard all cached lighting.
	IrrelevantLights.Empty();
	for(INT i=0;i<LODData.Num();i++)
	{
		LODData(i).ShadowMaps.Empty();
		LODData(i).ShadowVertexBuffers.Empty();
		LODData(i).LightMap = NULL;
	}
}


