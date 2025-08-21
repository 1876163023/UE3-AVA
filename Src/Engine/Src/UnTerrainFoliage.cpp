/*=============================================================================
	UnTerrainFoliage.cpp: Terrain foliage code.
	Copyright 1998-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "UnTerrainRender.h"

FTerrainFoliageIndexBuffer::FTerrainFoliageIndexBuffer(const FComponentFoliageInstanceList& InInstanceList):
	InstanceList(InInstanceList),
	MeshIndices(InInstanceList.Mesh->StaticMesh->LODModels(0).IndexBuffer.Indices),
	NumVerticesPerInstance(InInstanceList.Mesh->StaticMesh->LODModels(0).NumVertices)
{}

void FTerrainFoliageIndexBuffer::InitRHI()
{
	// Determine the type of index to use, and the total size of the buffer.
	const UINT Stride = (InstanceList.Instances.Num() * NumVerticesPerInstance > 65535) ? sizeof(DWORD) : sizeof(WORD);
	const UINT Size = Stride * InstanceList.Instances.Num() * MeshIndices.Num();

	if(Size > 0)
	{
		// Create and lock the index buffer.
		IndexBufferRHI = RHICreateIndexBuffer(Stride,Size,NULL,FALSE);
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI,0,Size);

		if (Stride == sizeof(DWORD))
		{
			DWORD* DestIndex = (DWORD*)Buffer;
			for(INT InstanceIndex = 0;InstanceIndex < InstanceList.Instances.Num();InstanceIndex++)
			{
				const DWORD BaseVertexIndex = InstanceIndex * NumVerticesPerInstance;
				const WORD* SourceIndex = &MeshIndices(0);
				for(INT Index = 0;Index < MeshIndices.Num();Index++)
				{
					*DestIndex++ = BaseVertexIndex + *SourceIndex++;
				}
			}
		}
		else
		{
			WORD* DestIndex = (WORD*)Buffer;
			for(INT InstanceIndex = 0;InstanceIndex < InstanceList.Instances.Num();InstanceIndex++)
			{
				const WORD BaseVertexIndex = InstanceIndex * NumVerticesPerInstance;
				const WORD* SourceIndex = &MeshIndices(0);
				for(INT Index = 0;Index < MeshIndices.Num();Index++)
				{
					*DestIndex++ = BaseVertexIndex + *SourceIndex++;
				}
			}
		}

		// Unlock the index buffer.
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
}

FTerrainFoliageVertexBuffer::FTerrainFoliageVertexBuffer(const FComponentFoliageInstanceList& InInstanceList):
	InstanceList(InInstanceList),
	Mesh(InInstanceList.Mesh)
{}

void FTerrainFoliageVertexBuffer::InitRHI()
{
	const UINT NumVerticesPerInstance = Mesh->StaticMesh->LODModels(0).NumVertices;
	const UINT Size = sizeof(FTerrainFoliageInstanceVertex) * InstanceList.Instances.Num() * NumVerticesPerInstance;
	if(Size > 0)
	{
		// Create the RHI vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(Size,NULL,TRUE);
	}
}

void FTerrainFoliageVertexBuffer::WriteVertices(const TArray<INT>& VisibleFoliageInstances,const FVector& ViewOrigin)
{
	SCOPE_CYCLE_COUNTER(STAT_TerrainFoliageTime);

	const UINT NumVerticesPerInstance = Mesh->StaticMesh->LODModels(0).NumVertices;

	// Compute the size of the visible vertices.
	const UINT VisibleSize = sizeof(FTerrainFoliageInstanceVertex) * VisibleFoliageInstances.Num() * NumVerticesPerInstance;

	if(VisibleSize > 0)
	{
		check(IsValidRef(VertexBufferRHI));

		// Allocate a temporary buffer large enough to hold the vertices of a single foliage mesh instance.
		FTerrainFoliageInstanceVertex* TempVertices = new FTerrainFoliageInstanceVertex[NumVerticesPerInstance];

		// Lock the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI,0,VisibleSize);
		FTerrainFoliageInstanceVertex* DestVertex = (FTerrainFoliageInstanceVertex*)VertexBufferData;

		// Cache constants used to determine transition scale.
		const FLOAT MinTransitionRadiusSquared = Square(Mesh->MinTransitionRadius);
		const FLOAT InvTransitionSize = 1.0f / Max(Mesh->MaxDrawRadius - Mesh->MinTransitionRadius,0.0f);

		// Generate the instance vertices.
		for(INT VisibleInstanceIndex = 0;VisibleInstanceIndex < VisibleFoliageInstances.Num();VisibleInstanceIndex++)
		{
			const INT InstanceIndex = VisibleFoliageInstances(VisibleInstanceIndex);
			const FTerrainFoliageInstance& Instance = InstanceList.Instances(InstanceIndex);

			const FLOAT DistanceSquared = (ViewOrigin - Instance.Location).SizeSquared();
			if(DistanceSquared < MinTransitionRadiusSquared)
			{
				// For fully scaled foliage, simply copy the vertices.
				appMemcpy(DestVertex,&InstanceList.Vertices(InstanceIndex * NumVerticesPerInstance),sizeof(FTerrainFoliageInstanceVertex) * NumVerticesPerInstance);
			}
			else
			{
				// For foliage which is undergoing a scaling transition, modify the vertices and write them to the buffer.
				const FLOAT TransitionScale = (1.0f - (appSqrt(DistanceSquared) - Mesh->MinTransitionRadius) * InvTransitionSize);

				for(UINT VertexIndex = 0;VertexIndex < NumVerticesPerInstance;VertexIndex++)
				{
					TempVertices[VertexIndex] = InstanceList.Vertices(InstanceIndex * NumVerticesPerInstance + VertexIndex);

					// Apply the transition scale.
					const FVector RelativePosition = TempVertices[VertexIndex].Position - Instance.Location;
					TempVertices[VertexIndex].Position = Instance.Location + RelativePosition * TransitionScale;
				}

				// Write the vertex to the buffer.
				// todosz - hack required due to alignment restrictions 
				// when doing a struct assignment within a locked vertex buffer
				appMemcpy( DestVertex, TempVertices, sizeof(FTerrainFoliageInstanceVertex) * NumVerticesPerInstance );
			}

			DestVertex += NumVerticesPerInstance;
		}

		// Free the temporary vertex buffer.
		delete TempVertices;

		// Unlock the vertex buffer.
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FComponentFoliageInstanceList::Create(const FTerrainFoliageMesh* Mesh,const UTerrainComponent* Component,const FTerrainWeightedMaterial& WeightedMaterial)
{
	ATerrain* const Terrain = Component->GetTerrain();
	const FLOAT MinTransitionRadiusSquared = Square(Mesh->MinTransitionRadius);
	const FLOAT InvTransitionSize = 1.0f / Max(Mesh->MaxDrawRadius - Mesh->MinTransitionRadius,0.0f);
	const FVector2D ShadowMapCoordinateOffset(0.5f / (Component->TrueSectionSizeX * Terrain->StaticLightingResolution + 1),0.5f / (Component->TrueSectionSizeY * Terrain->StaticLightingResolution + 1));

	// Shortcuts to static mesh data.
	const FStaticMeshRenderData& FoliageMeshRenderData = Mesh->StaticMesh->LODModels(0);

	FRandomStream RandomStream(Mesh->Seed ^ Component->SectionBaseX ^ Component->SectionBaseY);

	for(INT PatchY = 0;PatchY < Component->TrueSectionSizeY;PatchY++)
	{
		for(INT PatchX = 0;PatchX < Component->TrueSectionSizeX;PatchX++)
		{
			if (Terrain->IsTerrainQuadVisible(Component->SectionBaseX + PatchX,Component->SectionBaseY + PatchY) == FALSE)
			{
				// Don't populate foliage on holes...
				continue;
			}

			const FTerrainPatch Patch = Terrain->GetPatch(Component->SectionBaseX + PatchX,Component->SectionBaseY + PatchY);

			// Compute the vertices and material weights for this terrain quad.
			FVector QuadVertices[2][2];
			FLOAT Weights[2][2];
			for(INT VertexY = 0;VertexY < 2;VertexY++)
			{
				for(INT VertexX = 0;VertexX < 2;VertexX++)
				{
					QuadVertices[VertexX][VertexY] = Terrain->GetCollisionVertex(Patch,Component->SectionBaseX + PatchX,Component->SectionBaseY + PatchY,VertexX,VertexY,1);
					Weights[VertexX][VertexY] = WeightedMaterial.Weight(Component->SectionBaseX + PatchX + VertexX,Component->SectionBaseY + PatchY + VertexY) / 255.0f;
				}
			}

			for(INT InstanceIndex = 0;InstanceIndex < Mesh->Density;InstanceIndex++)
			{
				const FLOAT X = RandomStream.GetFraction();
				const FLOAT Y = RandomStream.GetFraction();
				const FLOAT ScaleX = RandomStream.GetFraction();
				const FLOAT ScaleY = RandomStream.GetFraction();
				const FLOAT ScaleZ = RandomStream.GetFraction();
				const FLOAT Yaw = RandomStream.GetFraction();
				const FLOAT Random = RandomStream.GetFraction();

				if(Random <= QuadLerp(Weights[0][0],Weights[1][0],Weights[0][1],Weights[1][1],X,Y))
				{
					FTerrainFoliageInstance* FoliageInstance = new(Instances) FTerrainFoliageInstance;

					// Calculate the instance's location in local space.
					const FVector LocalLocation = QuadLerp(QuadVertices[0][0],QuadVertices[1][0],QuadVertices[0][1],QuadVertices[1][1],X,Y) -
													FVector(Component->SectionBaseX,Component->SectionBaseY,0);
					FoliageInstance->Location = Component->LocalToWorld.TransformFVector(LocalLocation);

					// Calculate the instance's static lighting texture coordinate.
					const FVector2D ShadowMapCoordinate = ShadowMapCoordinateOffset + FVector2D(
						(PatchX + X) * Terrain->StaticLightingResolution / (Component->TrueSectionSizeX * Terrain->StaticLightingResolution + 1),
						(PatchY + Y) * Terrain->StaticLightingResolution / (Component->TrueSectionSizeX * Terrain->StaticLightingResolution + 1)
						);

					// Calculate the transform from instance local space to world space.
					const FRotator Rotation(0,appTrunc(Yaw * 65535),0);
					const FMatrix NormalInstanceToWorld = FRotationMatrix(Rotation);
					const FMatrix InstanceToWorld =
						FScaleMatrix(Lerp(FVector(1,1,1) * Mesh->MinScale,FVector(1,1,1) * Mesh->MaxScale,FVector(ScaleX,ScaleY,ScaleZ))) *
						NormalInstanceToWorld *
						FTranslationMatrix(FoliageInstance->Location);

					// Generate the foliage vertices.
					for(UINT VertexIndex = 0;VertexIndex < FoliageMeshRenderData.NumVertices;VertexIndex++)
					{
						const FVector& SrcPosition = FoliageMeshRenderData.PositionVertexBuffer.VertexPosition(VertexIndex);
						const FPackedNormal& SrcTangentX = FoliageMeshRenderData.VertexBuffer.VertexTangentX(VertexIndex);
						const FPackedNormal& SrcTangentY = FoliageMeshRenderData.VertexBuffer.VertexTangentY(VertexIndex);
						const FPackedNormal& SrcTangentZ = FoliageMeshRenderData.VertexBuffer.VertexTangentZ(VertexIndex);

						FTerrainFoliageInstanceVertex* DestVertex = new(Vertices) FTerrainFoliageInstanceVertex;

						DestVertex->Position = InstanceToWorld.TransformFVector(SrcPosition);
						DestVertex->TexCoord = FoliageMeshRenderData.VertexBuffer.VertexUV(VertexIndex,0);
						DestVertex->ShadowMapCoord = ShadowMapCoordinate;

						DestVertex->TangentX = NormalInstanceToWorld.TransformNormal(SrcTangentX);
						DestVertex->TangentY = NormalInstanceToWorld.TransformNormal(SrcTangentY);
						DestVertex->TangentZ = NormalInstanceToWorld.TransformNormal(SrcTangentZ);
					}
				}
			}
		}
	}

	Instances.Shrink();
	Vertices.Shrink();

	// Initialize the vertex and index buffers.
	VertexBuffer.Init();
	IndexBuffer.Init();

	// Initialize the vertex factory.
	FLocalVertexFactory::DataType FoliageVertexFactoryData;
	FoliageVertexFactoryData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FTerrainFoliageInstanceVertex,Position,VET_Float3);
	FoliageVertexFactoryData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FTerrainFoliageInstanceVertex,TangentX,VET_PackedNormal);
	FoliageVertexFactoryData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FTerrainFoliageInstanceVertex,TangentY,VET_PackedNormal);
	FoliageVertexFactoryData.TangentBasisComponents[2] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FTerrainFoliageInstanceVertex,TangentZ,VET_PackedNormal);
	FoliageVertexFactoryData.TextureCoordinates.AddItem(STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FTerrainFoliageInstanceVertex,TexCoord,VET_Float2));
	FoliageVertexFactoryData.ShadowMapCoordinateComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&VertexBuffer,FTerrainFoliageInstanceVertex,ShadowMapCoord,VET_Float2);

	FoliageVertexFactory.SetData(FoliageVertexFactoryData);
	FoliageVertexFactory.Init();
}

void FTerrainComponentSceneProxy::DrawFoliage(FPrimitiveDrawInterface* PDI,const FSceneView* View)
{
	ATerrain* const Terrain = ComponentOwner->GetTerrain();
	const FVector& ViewOrigin = View->ViewOrigin;
	const FBoxSphereBounds& Bounds = PrimitiveSceneInfo->Bounds;

	SCOPE_CYCLE_COUNTER(STAT_FoliageRenderTime);

	// Create foliage instance batches.
	if(View->ProjectionMatrix.M[3][3] < 1.0f)
	{
		const FLOAT Distance = (Bounds.Origin - ViewOrigin).Size() - Bounds.SphereRadius;

		// Clean out cached foliage instances which are no longer needed for this component.
		for(ComponentFoliageInstanceListsType::TIterator It(ComponentFoliageInstanceLists);It;It.Next())
		{
			if(Distance >= (*It).Mesh->MaxDrawRadius)
			{
				It.RemoveCurrent();
			}
		}

		// Build batches of foliage mesh instances which are within their MaxDrawRadius.
		for(INT MaterialIndex = 0;MaterialIndex < Terrain->WeightedMaterials.Num();MaterialIndex++)
		{
			UTerrainMaterial*	TerrainMaterial = Terrain->WeightedMaterials(MaterialIndex).Material;
			for(INT MeshIndex = 0;MeshIndex < TerrainMaterial->FoliageMeshes.Num();MeshIndex++)
			{
				const FTerrainFoliageMesh* Mesh = &TerrainMaterial->FoliageMeshes(MeshIndex);

				if(Distance < Mesh->MaxDrawRadius && Mesh->StaticMesh)
				{
					const FStaticMeshRenderData& FoliageMeshRenderData = Mesh->StaticMesh->LODModels(0);
					FComponentFoliageInstanceList* InstanceList = ComponentFoliageInstanceLists.Find(Mesh);
					if (!InstanceList)
					{
						InstanceList = ComponentFoliageInstanceLists.Add(Mesh);
						InstanceList->Create(Mesh,ComponentOwner,Terrain->WeightedMaterials(MaterialIndex));
					}

					// Find the material to use for the instance.
					const UMaterialInstance* Material;
					if(Mesh->Material)
					{
						Material = Mesh->Material;
					}
					else if(Mesh->StaticMesh->LODModels(0).Elements.Num() && Mesh->StaticMesh->LODModels(0).Elements(0).Material)
					{
						Material = Mesh->StaticMesh->LODModels(0).Elements(0).Material;
					}
					else
					{
						Material = GEngine->DefaultMaterial;
					}

					// Find the foliage instances visible in this view.
					TArray<INT> VisibleFoliageInstances;
					VisibleFoliageInstances.Empty(InstanceList->Instances.Num());
					const FLOAT DrawRadiusSquared = Square(Mesh->MaxDrawRadius);
					const FLOAT MeshRadius = Mesh->StaticMesh->Bounds.SphereRadius * Mesh->MaxScale;
					for(INT InstanceIndex = 0;InstanceIndex < InstanceList->Instances.Num();InstanceIndex++)
					{
						const FTerrainFoliageInstance& Instance = InstanceList->Instances(InstanceIndex);
						if(	(Instance.Location - View->ViewOrigin).SizeSquared() < DrawRadiusSquared &&
							View->ViewFrustum.IntersectSphere(Instance.Location,MeshRadius))
						{
							VisibleFoliageInstances.AddItem(InstanceIndex);
						}
					}

					INC_DWORD_STAT_BY(STAT_TerrainFoliageInstances,VisibleFoliageInstances.Num());

					if(VisibleFoliageInstances.Num() && FoliageMeshRenderData.NumVertices && FoliageMeshRenderData.IndexBuffer.Indices.Num())
					{
						InstanceList->VertexBuffer.WriteVertices(VisibleFoliageInstances,ViewOrigin);

						FMeshElement MeshElement;
						MeshElement.IndexBuffer = &InstanceList->IndexBuffer;
						MeshElement.VertexFactory = &InstanceList->FoliageVertexFactory;
						MeshElement.MaterialInstance = Material->GetInstanceInterface(bSelected);
						MeshElement.LCI = CurrentMaterialInfo->ComponentLightInfo;
						MeshElement.LocalToWorld = FMatrix::Identity;
						MeshElement.WorldToLocal = FMatrix::Identity;						
						MeshElement.FirstIndex = 0;
						MeshElement.NumPrimitives = VisibleFoliageInstances.Num() * Mesh->StaticMesh->LODModels(0).IndexBuffer.Indices.Num() / 3;
						MeshElement.MinVertexIndex = 0;
						MeshElement.MaxVertexIndex = VisibleFoliageInstances.Num() * Mesh->StaticMesh->LODModels(0).NumVertices - 1;
						MeshElement.Type = PT_TriangleList;
						MeshElement.DepthPriorityGroup = GetDepthPriorityGroup(View);
						DrawRichMesh(PDI,MeshElement,FLinearColor(0,1,0),LevelColor,PropertyColor,PrimitiveSceneInfo,bSelected);
					}
				}
			}
		}
	}
}
