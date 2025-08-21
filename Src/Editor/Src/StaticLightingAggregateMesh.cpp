/*=============================================================================
	StaticLightingAggregateMesh.cpp: Static lighting aggregate mesh implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "StaticLightingPrivate.h"

#if !CONSOLE

// Definitions.
#define TRIANGLE_AREA_THRESHOLD				DELTA
#define MAX_OCTREE_DEPTH					12
#define MAX_OCTREE_NODE_MESHES				4
#define MAX_TRIANGLES_PER_AGGREGATED_MESH	200

void FStaticLightingAggregateMesh::AddMesh(const FStaticLightingMesh* Mesh)
{
	// Only use shadow casting meshes.
	if(Mesh->bCastShadow)
	{
		// Add meshes with many triangles to the octree, add the triangles of the lower polygon meshes to the world-space kd-tree.
		// Also add meshes which don't uniformly cast shadows, as the kd-tree only supports uniform shadow casting.
		if(Mesh->NumTriangles > MAX_TRIANGLES_PER_AGGREGATED_MESH || !Mesh->IsUniformShadowCaster())
		{
			RootNode.AddMesh(FOctreeNodeContext(),Mesh);
		}
		else
		{
			const INT BaseVertexIndex = Vertices.Num();
			Vertices.Add(Mesh->NumVertices);

			for(INT TriangleIndex = 0;TriangleIndex < Mesh->NumTriangles;TriangleIndex++)
			{
				// Read the triangle from the mesh.
				FStaticLightingVertex V0;
				FStaticLightingVertex V1;
				FStaticLightingVertex V2;
				Mesh->GetTriangle(TriangleIndex,V0,V1,V2);

				INT I0 = 0;
				INT I1 = 0;
				INT I2 = 0;
				Mesh->GetTriangleIndices(TriangleIndex,I0,I1,I2);

				Vertices(BaseVertexIndex + I0) = V0.WorldPosition;
				Vertices(BaseVertexIndex + I1) = V1.WorldPosition;
				Vertices(BaseVertexIndex + I2) = V2.WorldPosition;

				// Compute the triangle's normal.
				const FVector TriangleNormal = (V2.WorldPosition - V0.WorldPosition) ^ (V1.WorldPosition - V0.WorldPosition);

				// Compute the triangle area.
				const FLOAT TriangleArea = TriangleNormal.Size() * 0.5f;

				// Ignore zero area triangles.
				if(TriangleArea > TRIANGLE_AREA_THRESHOLD)
				{
					new(kDOPTriangles) FkDOPBuildCollisionTriangle(
						BaseVertexIndex + I0,BaseVertexIndex + I1,BaseVertexIndex + I2,
						kDopMeshes.AddItem(Mesh), // Use the triangle's material index as an index into kDOPMeshes.
						V0.WorldPosition,V1.WorldPosition,V2.WorldPosition
						);
				}
			}
		}
	}
}

void FStaticLightingAggregateMesh::PrepareForRaytracing()
{
	// Log information about the aggregate mesh.
	debugf(TEXT("Static lighting kd-tree: %u vertices, %u triangles"),Vertices.Num(),kDOPTriangles.Num());

	// Build the kd-tree for simple meshes.
	kDopTree.Build(kDOPTriangles);
	kDOPTriangles.Empty();
}

class FStaticLightingAggregateMeshDataProvider
{
public:

	/** Initialization constructor. */
	FStaticLightingAggregateMeshDataProvider(const FStaticLightingAggregateMesh* InMesh,const FShadowRay& InShadowRay):
		Mesh(InMesh),
		ShadowRay(InShadowRay)
	{}

	// kDOP data provider interface.

	FORCEINLINE const FVector& GetVertex(DWORD Index) const
	{
		return Mesh->Vertices(Index);
	}

	FORCEINLINE UMaterialInstance* GetMaterial(DWORD MaterialIndex) const
	{
		return NULL;
	}

	FORCEINLINE const TkDOPTree<const FStaticLightingAggregateMeshDataProvider>& GetkDOPTree(void) const
	{
		return Mesh->kDopTree;
	}

	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return FMatrix::Identity;
	}

	FORCEINLINE const FMatrix& GetWorldToLocal(void) const
	{
		return FMatrix::Identity;
	}

	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return FMatrix::Identity;
	}

	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return 1.0f;
	}

private:

	const FStaticLightingAggregateMesh* Mesh;
	const FShadowRay& ShadowRay;
};

UBOOL FStaticLightingAggregateMesh::DoesShadowRayIntersect(const FShadowRay& ShadowRay,FCoherentRayCache& CoherentRayCache) const
{
	// Check the mesh which was last hit by a ray in this thread.
	if(	CoherentRayCache.LastHitMesh.DoesShadowRayIntersect(ShadowRay))
	{
		return TRUE;
	}

	// Reset the last hit mesh.
	CoherentRayCache.LastHitMesh = NULL;

	// Check the kd-tree containing low polygon meshes first.
	FCheckResult Result(1.0f);
	FStaticLightingAggregateMeshDataProvider kDOPDataProvider(this,ShadowRay);
	TkDOPLineCollisionCheck<const FStaticLightingAggregateMeshDataProvider> kDOPCheck(ShadowRay.Start,ShadowRay.Start + ShadowRay.Direction * ShadowRay.Length,TRACE_StopAtAnyHit,kDOPDataProvider,&Result);
	if(kDopTree.LineCheck(kDOPCheck))
	{
		return TRUE;
	}

	// Check against all other meshes.
	return RootNode.DoesShadowRayIntersect(FOctreeNodeContext(),ShadowRay,CoherentRayCache);
}

void FStaticLightingAggregateMesh::FOctreeNode::AddMesh(const FOctreeNodeContext& Context,const FStaticLightingMesh* Mesh)
{
	UBOOL bAddMeshToThisNode = TRUE;
	UBOOL bIntersectsChildX[2] = { TRUE, TRUE };
	UBOOL bIntersectsChildY[2] = { TRUE, TRUE };
	UBOOL bIntersectsChildZ[2] = { TRUE, TRUE };

	// If this isn't a leaf, find the children which the mesh intersects.
	if(!bIsLeaf)
	{
		// Cull nodes on the opposite side of the X-axis division.
		if(Mesh->BoundingBox.Min.X > Context.Center.X)
		{
			bIntersectsChildX[0] = FALSE;
		}
		else if(Mesh->BoundingBox.Max.X < Context.Center.X)
		{
			bIntersectsChildX[1] = FALSE;
		}

		// Cull nodes on the opposite side of the Y-axis division.
		if(Mesh->BoundingBox.Min.Y > Context.Center.Y)
		{
			bIntersectsChildY[0] = FALSE;
		}
		else if(Mesh->BoundingBox.Max.Y < Context.Center.Y)
		{
			bIntersectsChildY[1] = FALSE;
		}

		// Cull nodes on the opposite side of the Z-axis division.
		if(Mesh->BoundingBox.Min.Z > Context.Center.Z)
		{
			bIntersectsChildZ[0] = FALSE;
		}
		else if(Mesh->BoundingBox.Max.Z < Context.Center.Z)
		{
			bIntersectsChildZ[1] = FALSE;
		}

		// Count the number of children the mesh intersects.
		INT NumChildrenIntersected = 0;
		for(INT ChildX = 0;ChildX < 2;ChildX++)
		{
			for(INT ChildY = 0;ChildY < 2;ChildY++)
			{
				for(INT ChildZ = 0;ChildZ < 2;ChildZ++)
				{
					if(bIntersectsChildX[ChildX] && bIntersectsChildY[ChildY] && bIntersectsChildZ[ChildZ])
					{
						NumChildrenIntersected++;
					}
				}
			}
		}

		// If the mesh intersects more than a single child, add it directly to this node.
		bAddMeshToThisNode = NumChildrenIntersected > 1;
	}

	if(bAddMeshToThisNode)
	{
		// Add the mesh to this node.
		new(Meshes) FMeshAndBounds(Mesh);

		// If the node is a leaf, and has too many meshes in it, split it into children.
		if(bIsLeaf && Meshes.Num() > MAX_OCTREE_NODE_MESHES && Context.Depth < MAX_OCTREE_DEPTH)
		{
			// Allow meshes to be added to children of this node.
			bIsLeaf = FALSE;

			// Make a copy of the node's mesh list, and clear it.
			TArray<FMeshAndBounds> SplitMeshes = Meshes;
			Meshes.Empty();

			// Re-add all of the node's meshes, potentially creating children of this node for them.
			for(INT MeshIndex = 0;MeshIndex < SplitMeshes.Num();MeshIndex++)
			{
				AddMesh(Context,SplitMeshes(MeshIndex).Mesh);
			}
		}
	}
	else
	{
		// Add the mesh to any child nodes which is intersects.
		for(INT ChildX = 0;ChildX < 2;ChildX++)
		{
			for(INT ChildY = 0;ChildY < 2;ChildY++)
			{
				for(INT ChildZ = 0;ChildZ < 2;ChildZ++)
				{
					// Check that the mesh intersects this child.
					if(bIntersectsChildX[ChildX] && bIntersectsChildY[ChildY] && bIntersectsChildZ[ChildZ])
					{
						// If the child node hasn't been created yet, create it.
						if(!Children[ChildX][ChildY][ChildZ])
						{
							Children[ChildX][ChildY][ChildZ] = new FOctreeNode();
						}

						// Add the mesh to the child node.
						Children[ChildX][ChildY][ChildZ]->AddMesh(FOctreeNodeContext(Context,ChildX,ChildY,ChildZ),Mesh);
					}
				}
			}
		}
	}
}

UBOOL FStaticLightingAggregateMesh::FOctreeNode::DoesShadowRayIntersect(const FOctreeNodeContext& Context,const FShadowRay& ShadowRay,FCoherentRayCache& CoherentRayCache) const
{
	// If this node has children, check the line segment for intersection with them.
	if(!bIsLeaf)
	{
		for(INT ChildX = 0;ChildX < 2;ChildX++)
		{
			for(INT ChildY = 0;ChildY < 2;ChildY++)
			{
				for(INT ChildZ = 0;ChildZ < 2;ChildZ++)
				{
					if(Children[ChildX][ChildY][ChildZ])
					{
						FOctreeNodeContext ChildContext(Context,ChildX,ChildY,ChildZ);
						if(	FLineBoxIntersection(Context.BoundingBox,ShadowRay.Start,ShadowRay.End,ShadowRay.Direction,ShadowRay.OneOverDirection) &&
							Children[ChildX][ChildY][ChildZ]->DoesShadowRayIntersect(Context,ShadowRay,CoherentRayCache)
							)
						{
							return TRUE;
						}
					}
				}
			}
		}
	}

	// Check the line segment for intersection with the meshes in this node.
	for(INT MeshIndex = 0;MeshIndex < Meshes.Num();MeshIndex++)
	{
		const FMeshAndBounds& MeshAndBounds = Meshes(MeshIndex);
		if(MeshAndBounds.DoesShadowRayIntersect(ShadowRay))
		{
			// Update the thread's last hit mesh.
			CoherentRayCache.LastHitMesh = MeshAndBounds.Mesh;

			return TRUE;
		}
	}

	return FALSE;
}

#endif
