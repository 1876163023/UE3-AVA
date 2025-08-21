/*=============================================================================
AvaConvex.cpp: StaticMesh -> Convex Polygon
Redduck inc
=============================================================================*/

#include "EnginePrivate.h"

#include "UnMeshBuild.h"
#include "UnStaticMeshLegacy.h"

#pragma optimize("", off)

/**
* This is the static mesh specific version for finding edges
*/
class FStaticMeshEdgeBuilder2 :
	public TEdgeBuilder<FLegacyStaticMeshVertex>
{
protected:
	TMultiMap<INT, FMeshEdge*>	IndexToEdgeList;

public:
	/**
	* Constructor that passes all work to the parent class
	*/
	FStaticMeshEdgeBuilder2(const TArray<WORD>& InIndices,
		const TArray<FLegacyStaticMeshVertex>& InVertices,
		TArray<FMeshEdge>& OutEdges	) :	TEdgeBuilder<FLegacyStaticMeshVertex>(InIndices,InVertices,OutEdges)
	{
	}

	/**
	* Searches the list of edges to see if this one matches an existing and
	* returns a pointer to it if it does
	*
	* @param Index1 the first index to check for
	* @param Index2 the second index to check for
	*
	* @return NULL if no edge was found, otherwise the edge that was found
	*/
	inline FMeshEdge* FindOppositeEdge(INT Index1,INT Index2)
	{
		FMeshEdge* Edge = NULL;
		TArray<FMeshEdge*> EdgeList;
		// Search the hash for a corresponding vertex
		IndexToEdgeList.MultiFind(Index2,EdgeList);
		// Now search through the array for a match or not
		for (INT EdgeIndex = 0; EdgeIndex < EdgeList.Num() && Edge == NULL;
			EdgeIndex++)
		{
			FMeshEdge* OtherEdge = EdgeList(EdgeIndex);
			// See if this edge matches the passed in edge
			if (OtherEdge != NULL && DoesEdgeMatch(Index1,Index2,OtherEdge))
			{
				// We have a match
				Edge = OtherEdge;
			}
		}
		return Edge;
	}

	/**
	* Updates an existing edge if found or adds the new edge to the list
	*
	* @param Index1 the first index in the edge
	* @param Index2 the second index in the edge
	* @param Triangle the triangle that this edge was found in
	*/
	inline void AddEdge(INT Index1,INT Index2,INT Triangle)
	{
		// If this edge matches another then just fill the other triangle
		// otherwise add it
		FMeshEdge* OtherEdge = FindOppositeEdge(Index1,Index2);
		if (OtherEdge == NULL)
		{
			// Add a new edge to the array
			INT EdgeIndex = Edges.AddZeroed();
			Edges(EdgeIndex).Vertices[0] = Index1;
			Edges(EdgeIndex).Vertices[1] = Index2;
			Edges(EdgeIndex).Faces[0] = Triangle;
			Edges(EdgeIndex).Faces[1] = -1;
			// Also add this edge to the hash for faster searches
			// NOTE: This relies on the array never being realloced!
			IndexToEdgeList.Add(Index1,&Edges(EdgeIndex));
		}
		else
		{
			OtherEdge->Faces[1] = Triangle;
		}
	}

	/**
	* Uses a hash of indices to edge lists so that it can avoid the n^2 search
	* through the full edge list
	*/
	void FindEdges(void)
	{
		// @todo Handle something other than trilists when building edges
		INT TriangleCount = Indices.Num() / 3;
		INT EdgeCount = 0;
		// Work through all triangles building the edges
		for (INT Triangle = 0; Triangle < TriangleCount; Triangle++)
		{
			// Determine the starting index
			INT TriangleIndex = Triangle * 3;
			// Get the indices for the triangle
			INT Index1 = Indices(TriangleIndex);
			INT Index2 = Indices(TriangleIndex + 1);
			INT Index3 = Indices(TriangleIndex + 2);
			// Add the first to second edge
			AddEdge(Index1,Index2,Triangle);
			// Now add the second to third
			AddEdge(Index2,Index3,Triangle);
			// Add the third to first edge
			AddEdge(Index3,Index1,Triangle);
		}
	}

	/**
	* This function determines whether a given edge matches or not for a static mesh
	*
	* @param Index1 The first index of the edge being checked
	* @param Index2 The second index of the edge
	* @param OtherEdge The edge to compare. Was found via the map
	*
	* @return TRUE if the edge is a match, FALSE otherwise
	*/
	UBOOL DoesEdgeMatch(INT Index1,INT Index2,FMeshEdge* OtherEdge)
	{
		if ( OtherEdge->Vertices[1] == Index1 && OtherEdge->Faces[1] == -1 )
		{
			return TRUE;
		}
		return FALSE;
	}
};

void ComputeCoplanerGroup( const TArray<TArray<INT> >& FaceEdgeListList,
						   const TArray<FMeshEdge>& Edges,
						   const TArray<FVector>& normals,
						   TArray<INT>* InFaceCoplanerGroup )
{
	INT CoplanerGroupId = 1;

	TArray<INT>& FaceCoplanerGroup = *InFaceCoplanerGroup;

	const INT NumTriangles = FaceEdgeListList.Num();

	for( INT TriangleIndex = 0; TriangleIndex < NumTriangles; ++TriangleIndex )
	{
		const FVector& faceNormal = normals(TriangleIndex);

		// 인접 face 에 group이 할당되었으면 그 그룹에 들어갈 수 있는지 확인할 것
		if( FaceCoplanerGroup(TriangleIndex) == 0 )
		{
			const FVector& faceNormal = normals(TriangleIndex);

			// 동일 평면의 인접 면에 group id 전파
			const TArray<INT>& FaceEdgeList = FaceEdgeListList(TriangleIndex);

			for( INT EdgeIndex = 0; EdgeIndex < FaceEdgeList.Num(); ++EdgeIndex )
			{
				const FMeshEdge& Edge = Edges( FaceEdgeList(EdgeIndex) );

				const INT otherIndex = ( Edge.Faces[0] == TriangleIndex ) ? Edge.Faces[1] : Edge.Faces[0];

				if( otherIndex != INDEX_NONE && FaceCoplanerGroup(otherIndex) != 0 )
				{
					const FVector& otherNormal = normals( otherIndex );
					float dotValue = faceNormal | otherNormal;
					if( dotValue > 0.999f )
					{
						FaceCoplanerGroup(TriangleIndex) = FaceCoplanerGroup(otherIndex);
					}
				}
			}
		}

		// 자신의 group index가 없다면, 새로운 그룹을 할당. 아니면 지정된 그룹을 사용
		const INT CurrentCoplanerGroupId = FaceCoplanerGroup(TriangleIndex) == 0 ? CoplanerGroupId++ : FaceCoplanerGroup(TriangleIndex);

		FaceCoplanerGroup(TriangleIndex) = CurrentCoplanerGroupId;

		// 동일 평면의 인접 면에 group id 전파
		const TArray<INT>& FaceEdgeList = FaceEdgeListList(TriangleIndex);

		for( INT EdgeIndex = 0; EdgeIndex < FaceEdgeList.Num(); ++EdgeIndex )
		{
			const FMeshEdge& Edge = Edges( FaceEdgeList(EdgeIndex) );

			const INT otherIndex = ( Edge.Faces[0] == TriangleIndex ) ? Edge.Faces[1] : Edge.Faces[0];

			if( otherIndex != INDEX_NONE  )
			{
				if( FaceCoplanerGroup(otherIndex) == 0 )
				{
					const FVector& otherNormal = normals(otherIndex);
					float dotValue = faceNormal | otherNormal;
					if( dotValue > 0.999f)
					{
						FaceCoplanerGroup(otherIndex) = CurrentCoplanerGroupId;
					}
				}
				else
				{
					// skip same group
					if( FaceCoplanerGroup(otherIndex) == CurrentCoplanerGroupId )
					{
						continue;
					}

					// coplaner group 사이에도 merging 이 가능해야 한다.
					const FVector& otherNormal = normals(otherIndex);
					float dotValue = faceNormal | otherNormal;
					if( dotValue > 0.999f)
					{
						INT idForChagne = FaceCoplanerGroup(otherIndex);
						for( INT groupIndex = 0; groupIndex < FaceCoplanerGroup.Num(); ++groupIndex)
						{
							if( FaceCoplanerGroup(groupIndex) == idForChagne )
							{
								FaceCoplanerGroup(groupIndex) = CurrentCoplanerGroupId;
							}
						}
					}
				}
			}
		}
	}
}

void MergeCoplanerTrianglesToConvexPolygons( const TArray<INT>& coplanerTriList,
											 const TArray<FVector>& normals,
											 const TArray<TArray<INT> >& FaceEdgeListList,
											 const TArray<FMeshEdge>& Edges,
											 const TArray<FLegacyStaticMeshVertex>& VertexBuffer,
											 const TArray<WORD>& IndexBuffer,
											 const TArray<FColor>& CoplanerGroupColors,
											 TArray<TArray<INT> >* result_convexes,
											 TArray<TArray<INT> >* result_convexes_to_facelist
											 )
{
	// merge co-planer triangles to convex polygon
	
	if( coplanerTriList.Num() == 0 )
	{
		return;
	}

	const FVector& planeNormal = normals(coplanerTriList(0));

	INT convexGroupId = 0;
	TArray<INT> convexGroupIds;
	for( INT i = 0; i < coplanerTriList.Num(); ++i )
	{
		convexGroupIds.AddItem( INDEX_NONE );
	}

	TArray<TArray<INT> > convexes;
	convexes.AddZeroed( coplanerTriList.Num() );

	// generate convexes
	for( INT coplanerTriIndex = 0; coplanerTriIndex < coplanerTriList.Num(); ++coplanerTriIndex )
	{
		const INT triIndex = coplanerTriList(coplanerTriIndex);

		// 새로운 convex의 base가 된다.
		if( convexGroupIds(coplanerTriIndex) == INDEX_NONE )
		{
			// 인접한 tri 중 convex group 에 들어간 것이 있는지 알아본다.
			const TArray<INT>& FaceEdgeList = FaceEdgeListList(triIndex);

			for( INT faceEdgeIndex = 0; faceEdgeIndex < FaceEdgeList.Num(); ++faceEdgeIndex )
			{
				INT edgeIndex = FaceEdgeList(faceEdgeIndex);
				const FMeshEdge& edge = Edges(edgeIndex);

				UBOOL bReverseEdge = (edge.Faces[1] == triIndex);
				INT otherTriIndex = bReverseEdge ? edge.Faces[0] : edge.Faces[1];

				if( otherTriIndex == INDEX_NONE )
				{
					continue;
				}

				// coplaner?
				const INT otherCoplanerTriIndex = coplanerTriList.FindItemIndex(otherTriIndex);

				if( otherCoplanerTriIndex == INDEX_NONE )
				{
					continue;
				}

				const INT otherConvexGroupId = convexGroupIds(otherCoplanerTriIndex);

				if( otherConvexGroupId != INDEX_NONE )
				{
					// try to merge
					const INT startVertexIndex = bReverseEdge ? edge.Vertices[1] : edge.Vertices[0];

					INT addedVertexIndex = INDEX_NONE;

					// 추가되는 vertex는 자신의 vertex 이다.
					for( INT i = 0; i < 3; ++i)
					{
						INT vertexIndex = IndexBuffer( triIndex * 3 + i );
						if( vertexIndex != edge.Vertices[0] && vertexIndex != edge.Vertices[1] )
						{
							addedVertexIndex = vertexIndex;
						}
					}

					check( addedVertexIndex != INDEX_NONE );

					const FVector& addedVertex = VertexBuffer(addedVertexIndex).Position;

					FVector AddedSide[2];
					if( !bReverseEdge )
					{
						AddedSide[0] = addedVertex - VertexBuffer(edge.Vertices[1]).Position;
						AddedSide[1] = VertexBuffer(edge.Vertices[0]).Position - addedVertex;
					}
					else
					{
						AddedSide[0] = addedVertex - VertexBuffer(edge.Vertices[0]).Position;
						AddedSide[1] = VertexBuffer(edge.Vertices[1]).Position - addedVertex;
					}

					// plane normal 은 out 방향입니다.
					FVector SplitPlane[2];
					SplitPlane[0] = AddedSide[0].SafeNormal() ^ planeNormal;
					SplitPlane[1] = AddedSide[1].SafeNormal() ^ planeNormal;


					// check convexity
					const TArray<INT>& convex = convexes(otherConvexGroupId);
					UBOOL isConvex = TRUE;
					for( INT point= 0; point < convex.Num(); ++point)
					{
						const FVector& testPoint = VertexBuffer(convex(point)).Position;
						FLOAT dist0 = FPointPlaneDist( testPoint, addedVertex, SplitPlane[0] );
						FLOAT dist1 = FPointPlaneDist( testPoint, addedVertex, SplitPlane[1] );

						// 
						if( dist0 > THRESH_SPLIT_POLY_WITH_PLANE || dist1 > THRESH_SPLIT_POLY_WITH_PLANE )
						{
							isConvex = FALSE;
							break;
						}
					}

					if( isConvex )
					{
						TArray<INT> mergedConvex;
						const INT windingStart = convex.FindItemIndex(startVertexIndex);
						for( INT wp = 0; wp < convex.Num(); ++wp )
						{
							mergedConvex.AddItem( convex( ( windingStart + wp ) % convex.Num() ) );
						}
						mergedConvex.AddItem( addedVertexIndex );

						/////////////// debuging merge algorithm code ////////////////////////
						//debugf( TEXT("merge %d neighbor convex group"), otherConvexGroupId );

						//FString convexString = FString::Printf( TEXT("face %d -> %d ("), triIndex, otherTriIndex );
						//
						//for( int i = 0; i < mergedConvex.Num(); ++i)
						//{
						//	convexString += FString::Printf(TEXT("%d "), mergedConvex(i) );
						//}
						//convexString += FString(TEXT(")"));
						//debugf( *convexString );


						// copy result
						convexes(otherConvexGroupId) = mergedConvex;

						convexGroupIds(coplanerTriIndex) = otherConvexGroupId;

						break;
					}
				}
			}

			// 인접 tri의 convex group에도 들어가지 못한다면...
			if( convexGroupIds(coplanerTriIndex) == INDEX_NONE )
			{
				convexGroupIds(coplanerTriIndex) = convexGroupId;
				for( INT i = 0; i < 3; ++i )
				{
					convexes(convexGroupId).AddItem( IndexBuffer( triIndex * 3 + i ) );
				}

				++convexGroupId;
			}
		}

		const INT curConvexId = convexGroupIds(coplanerTriIndex);

		const TArray<INT>& FaceEdgeList = FaceEdgeListList(triIndex);
		for( INT faceEdgeIndex = 0; faceEdgeIndex < FaceEdgeList.Num(); ++faceEdgeIndex )
		{
			INT edgeIndex = FaceEdgeList(faceEdgeIndex);
			const FMeshEdge& edge = Edges(edgeIndex);

			UBOOL bReverseEdge = (edge.Faces[1] == triIndex);
			INT otherTriIndex = bReverseEdge ? edge.Faces[0] : edge.Faces[1];

			if( otherTriIndex == INDEX_NONE )
			{
				continue;
			}
			// coplaner?
			const INT otherCoplanerTriIndex = coplanerTriList.FindItemIndex(otherTriIndex);
			if( otherCoplanerTriIndex == INDEX_NONE )
			{
				continue;
			}

			// convex group 이 지정되었으면 pass
			if( convexGroupIds(otherCoplanerTriIndex) != INDEX_NONE )
			{
				continue;
			}


			/////////////// try to merge
			const INT startVertexIndex = bReverseEdge ? edge.Vertices[0] : edge.Vertices[1];

			INT addedVertexIndex = INDEX_NONE;
			for( INT i = 0; i < 3; ++i)
			{
				INT vertexIndex = IndexBuffer(otherTriIndex*3+i);
				if( vertexIndex != edge.Vertices[0] && vertexIndex != edge.Vertices[1] )
				{
					addedVertexIndex = vertexIndex;
				}
			}

			check( addedVertexIndex != INDEX_NONE );

			const FVector& addedVertex = VertexBuffer(addedVertexIndex).Position;

			FVector AddedSide[2];
			if( bReverseEdge )
			{
				AddedSide[0] = addedVertex - VertexBuffer(edge.Vertices[1]).Position;
				AddedSide[1] = VertexBuffer(edge.Vertices[0]).Position - addedVertex;
			}
			else
			{
				AddedSide[0] = addedVertex - VertexBuffer(edge.Vertices[0]).Position;
				AddedSide[1] = VertexBuffer(edge.Vertices[1]).Position - addedVertex;
			}

			// edge winding 이 cw.
			FVector SplitPlane[2];
			SplitPlane[0] = AddedSide[0].SafeNormal() ^ planeNormal ;
			SplitPlane[1] = AddedSide[1].SafeNormal() ^ planeNormal ;


			// check convexity
			const TArray<INT>& convex = convexes(curConvexId);
			UBOOL isConvex = TRUE;
			for( INT point= 0; point < convex.Num(); ++point)
			{
				const FVector& testPoint = VertexBuffer(convex(point)).Position;
				FLOAT dist0 = FPointPlaneDist( testPoint, addedVertex, SplitPlane[0] );
				FLOAT dist1 = FPointPlaneDist( testPoint, addedVertex, SplitPlane[1] );

				// 
				if( dist0 > THRESH_SPLIT_POLY_WITH_PLANE || dist1 > THRESH_SPLIT_POLY_WITH_PLANE )
				{
					isConvex = FALSE;
					break;
				}
			}

			if( isConvex )
			{
				TArray<INT> mergedConvex;
				const INT windingStart = convex.FindItemIndex(startVertexIndex);
				for( INT wp = 0; wp < convex.Num(); ++wp )
				{
					mergedConvex.AddItem( convex( ( windingStart + wp ) % convex.Num() ) );
				}
				mergedConvex.AddItem( addedVertexIndex );

				////////////////// debugging merge algorithm code /////////////////////////
				//debugf( TEXT("merge %d convex group"), curConvexId );
				//FString convexString = FString::Printf(TEXT("\t face %d -> %d ("), otherTriIndex, triIndex );
				//for( int i = 0; i < mergedConvex.Num(); ++i)
				//{
				//	convexString += FString::Printf(TEXT("%d "), mergedConvex(i) );
				//}

				//convexString += FString( TEXT(")") );
				//debugf( *convexString );

				// copy result
				convexes(curConvexId) = mergedConvex;

				convexGroupIds(otherCoplanerTriIndex) = curConvexId;
			}

		}
	} // end of generating covexes

	// generate convex face list
	TArray< TArray<INT> > convexFaceList;
	convexFaceList.AddZeroed( convexGroupId );
	for( INT coplanerIndex = 0; coplanerIndex < convexGroupIds.Num(); ++coplanerIndex )
	{
		INT id = convexGroupIds(coplanerIndex);
		check( id != -1 );
		convexFaceList(id).AddItem(coplanerTriList(coplanerIndex));
	}

	// merging between convexes

	// super convex
	TArray< TArray<INT> >& super_convexes = *result_convexes;
	TArray< TArray<INT> > subject_convexes;

	TArray< TArray<INT> >& super_convex_to_face_list = *result_convexes_to_facelist;
	TArray< TArray<INT> > subject_convex_to_face_list;

	TArray<INT> super_convex_ids;

	subject_convexes = convexes;
	subject_convex_to_face_list = convexFaceList;

	UBOOL is_merge_occur = FALSE;

	INT debug_run_count = 0;

	while ( 1 )
	{
		is_merge_occur = FALSE;

		// initialize super convex id
		super_convex_ids.Empty();

		for( INT i = 0; i < subject_convexes.Num(); ++i)
		{
			super_convex_ids.AddItem( INDEX_NONE );
		}

		INT super_convex_id = 0;

		// merge subject convexes to super convexes
		for( INT convexIndex = 0; convexIndex < subject_convexes.Num(); ++convexIndex )
		{
			// empty convex도 건너뛴다( 위에서 원래 사이즈보다 많게 만들기 때문이다)
			if( subject_convexes(convexIndex).Num() == 0 )
			{
				continue;
			}

			// super convex에 포함되었으면, 더이상 고려치 않는다.
			if( super_convex_ids(convexIndex) != INDEX_NONE )
			{
				continue;
			}

			// id 할당.. zero base
			const INT cur_super_convex_id = super_convex_id++;

			// super_convex의 base convex 추가
			super_convexes.AddItem( subject_convexes(convexIndex) );

			// super_convex를 구성하는 face 추가
			super_convex_to_face_list.AddItem( subject_convex_to_face_list(convexIndex) );

			TArray<INT>& cur_super_convex = super_convexes( cur_super_convex_id );
			TArray<INT>& cur_face_list = super_convex_to_face_list( cur_super_convex_id );

			super_convex_ids(convexIndex) = cur_super_convex_id;

			for( INT otherIndex =0; otherIndex < subject_convexes.Num(); ++otherIndex )
			{
				// pass self
				if ( convexIndex == otherIndex )
				{
					continue;
				}

				//
				if( super_convex_ids(otherIndex) != INDEX_NONE )
				{
					continue;
				}

				const TArray<INT>& otherConvex = subject_convexes(otherIndex);

				TArray<INT> sharedVertexes;
				INT edge_start_wp_index = INDEX_NONE;
				for( INT wp = 0; wp < cur_super_convex.Num(); ++wp )
				{
					const INT next_wp = ( wp + 1 ) % cur_super_convex.Num();
					const INT prev_wp = ( wp - 1 ) < 0 ? ( wp - 1 ) + cur_super_convex.Num() : wp - 1;

					const UBOOL match_wp = otherConvex.ContainsItem( cur_super_convex(wp) );
					const UBOOL match_next_wp = otherConvex.ContainsItem( cur_super_convex(next_wp) );
					const UBOOL match_prev_wp = otherConvex.ContainsItem( cur_super_convex(prev_wp) );

					if( match_wp && match_next_wp )
					{
						sharedVertexes.AddUniqueItem( cur_super_convex(wp) );
						sharedVertexes.AddUniqueItem( cur_super_convex(next_wp) );
					}

					if( match_wp && !match_prev_wp )
					{
						edge_start_wp_index = wp;
					}
				}

				// there is no shared edges
				if( sharedVertexes.Num() < 2 )
				{
					continue;
				}

				// 같은 winding이 하나 이상 있을 때...... 이 조건에 걸린다....
				// 같은 winding이 안생겨야 하는데.. static mesh modeling 시 생길수도 있나보다...
				// 그러므로... 이 조건을 좀 완화해서.. 건너뛰도록 해보자..
				//check( edge_start_wp_index != INDEX_NONE );
				if( edge_start_wp_index == INDEX_NONE )
					continue;

				// 순서에 맞게 vertex 를 정렬
				TArray<INT> sharedEdge;
				for( INT wp = 0; wp < cur_super_convex.Num(); ++wp )
				{
					const int cur_wp = ( edge_start_wp_index + wp ) % cur_super_convex.Num();
					const UBOOL is_edge = sharedVertexes.ContainsItem( cur_super_convex( cur_wp ) );
					if( is_edge )
					{
						sharedEdge.AddItem( cur_super_convex( cur_wp ) );
					}
				}

				// we find shared edge.. try to merge..

				TArray<INT> mergedGeometry;

				// master winding
				// end -> start ( num = numvertex - shared vertices + 2 ) 2 mean edge vertex
				INT start_wp_index = cur_super_convex.FindItemIndex(sharedEdge.Last());
				INT wp_num = cur_super_convex.Num() - sharedEdge.Num() + 2;	
				for( INT wp = 0; wp < wp_num; ++wp)
				{
					INT wp_index = ( start_wp_index + wp ) % cur_super_convex.Num();
					mergedGeometry.AddItem( cur_super_convex( wp_index ) );
				}

				// slave winding
				// start + 1 -> end -1 ( num = num vertices - shared vertices )
				start_wp_index = otherConvex.FindItemIndex( sharedEdge(0) ) + 1;
				wp_num = otherConvex.Num() - sharedEdge.Num();

				INT other_edge_base = mergedGeometry.Num();

				for( INT wp = 0; wp < wp_num; ++wp )
				{
					INT wp_index = ( start_wp_index + wp ) % otherConvex.Num();
					mergedGeometry.AddItem( otherConvex( wp_index ) );
				}

				// check convexity
				// for each new added other convex edges..
				UBOOL is_convex = TRUE;
				for( INT edge = other_edge_base - 1; edge < mergedGeometry.Num(); ++edge )
				{
					INT v1_index = mergedGeometry( edge );
					INT v2_index = mergedGeometry( (edge + 1) % mergedGeometry.Num() );

					// edge winding 이 cw.
					// plane normal 은 out 방향입니다.
					const FVector side = (VertexBuffer(v2_index).Position - VertexBuffer(v1_index).Position).SafeNormal();
					const FVector split_plane = side ^ planeNormal ;
					const FVector& plane_vertex = VertexBuffer(v2_index).Position;

					for( INT point= 0; point < mergedGeometry.Num(); ++point)
					{
						const FVector& testPoint = VertexBuffer(mergedGeometry(point)).Position;
						FLOAT dist = FPointPlaneDist( testPoint, plane_vertex, split_plane );
						// 
						if( dist > THRESH_SPLIT_POLY_WITH_PLANE )
						{
							is_convex = FALSE;
							break;
						}
					}
				}

				// add to super convex list
				if( is_convex )
				{
					is_merge_occur = TRUE;

					//@DEBUG : 'merge convex'
					//FString run_count = FString::Printf(TEXT("pass(%d) : "), debug_run_count);
					//FString master_convex_string = FString::Printf(TEXT("master(%d) : ("), cur_super_convex.Num());
					//for( INT cp = 0; cp < cur_super_convex.Num(); ++cp )
					//{
					//	master_convex_string += FString::Printf(TEXT("%d "), cur_super_convex(cp) );
					//}
					//master_convex_string += FString::Printf(TEXT(") + "));

					//FString slave_convex_string = FString::Printf(TEXT("slave(%d) : ("), otherConvex.Num());
					//for( INT cp = 0; cp < otherConvex.Num(); ++cp )
					//{
					//	slave_convex_string += FString::Printf(TEXT("%d "), otherConvex(cp) );
					//}
					//slave_convex_string += FString::Printf(TEXT(") = "));

					//FString merge_convex_string = FString::Printf(TEXT("merge(%d) : ("), mergedGeometry.Num());
					//for( INT cp = 0; cp < mergedGeometry.Num(); ++cp )
					//{
					//	merge_convex_string += FString::Printf(TEXT("%d "), mergedGeometry(cp) );
					//}
					//merge_convex_string += FString::Printf(TEXT(")"));

					//FString shared_edge_string = FString::Printf(TEXT("<shread(%d) : ("), sharedEdge.Num() );
					//for( INT cp = 0; cp < sharedEdge.Num(); ++cp )
					//{
					//	shared_edge_string += FString::Printf(TEXT("%d "), sharedEdge(cp) );
					//}
					//shared_edge_string += FString::Printf(TEXT(")>"));

					//debugf( *(run_count + master_convex_string + slave_convex_string + merge_convex_string + shared_edge_string ));

					// copy result
					cur_super_convex = mergedGeometry;

					cur_face_list.Append( subject_convex_to_face_list(otherIndex) );

					// assign super convex id to other subject convex
					super_convex_ids(otherIndex) = cur_super_convex_id;

					// 다른 것과 합쳐진 convex를 삭제한다.
					TArray<INT>& mergedConvex = subject_convexes(otherIndex);
					mergedConvex.Empty();
				}
			}
		}

		//@Debug 용
		debug_run_count++;
		
		// no merge occur, stop algorithm
		if( !is_merge_occur )
		//if( subject_convexes.Num() == super_convexes.Num() )
		{
			break;
		}
		else
		{
			// prepare to next turn
			subject_convexes = super_convexes;
			subject_convex_to_face_list = super_convex_to_face_list;

			super_convexes.Empty();
			super_convex_to_face_list.Empty();
		}
	}

	// optimize convexes
	for( INT convex_index = 0; convex_index < super_convexes.Num(); ++convex_index )
	{
		TArray<INT>& convex = super_convexes(convex_index);

		//@DEBUG : 'optimize convex'
		//FString before_convex_string(TEXT("before convex : "));
		//for( INT cp = 0; cp < convex.Num(); ++cp )
		//{
		//	before_convex_string += FString::Printf(TEXT("%d "), convex(cp) );
		//}
		//before_convex_string += FString::Printf(TEXT(")"));

		FMemMark mem_mark(GEngineMem);
		FVector* side_plane_normal = new(GEngineMem) FVector[convex.Num()];
		FVector side;

		// remove identical vertex
		UBOOL degenerate_convex = FALSE;
		for( INT cp = 0; cp < convex.Num(); ++cp )
		{
			const INT cp_prev = ( cp - 1 + convex.Num() ) % convex.Num();
			const FVector& prev_pos = VertexBuffer( convex( cp_prev ) ).Position;
			const FVector& pos = VertexBuffer( convex( cp ) ).Position;

			side = pos - prev_pos;
			side_plane_normal[cp] = side ^ planeNormal;

			if( !side_plane_normal[cp].Normalize() )
			{
				// eliminate these nearly identical points
				convex.Remove(cp, 1);
				cp--;

				//check( convex.Num() > 2 );
				if( convex.Num() < 3 )
				{
					super_convexes.Remove(convex_index, 1);
					super_convex_to_face_list.Remove(convex_index,1);
					--convex_index;

					degenerate_convex = TRUE;

					break;
				}
			}
		}

		if( !degenerate_convex )
		{
			// remove colinear vertex
			for( INT cp = 0; cp < convex.Num(); ++cp )
			{
				const INT cp_next = (cp+1) % convex.Num();
				if ( ( side_plane_normal[cp] | side_plane_normal[cp_next] ) >= 0.99f )
				//if( FPointsAreNear( side_plane_normal[cp], side_plane_normal[cp_next], FLOAT_NORMAL_THRESH) )
				{
					appMemcpy( &side_plane_normal[cp], &side_plane_normal[cp+1], (convex.Num() - (cp+1)) * sizeof(FVector) );
					convex.Remove(cp, 1);
					cp--;

					//check( convex.Num() > 2 );
					// degenerate convex
					if( convex.Num() < 3 )
					{
						super_convexes.Remove(convex_index, 1);
						super_convex_to_face_list.Remove(convex_index,1);
						--convex_index;

						degenerate_convex = TRUE;
						break;
					}
				}
			}
		}

		mem_mark.Pop();

		//@DEBUG : 'optimize convex'
		//if( !degenerate_convex )
		//{
		//	FString after_convex_string(TEXT("after convex : "));
		//	for( INT cp = 0; cp < convex.Num(); ++cp )
		//	{
		//		after_convex_string += FString::Printf(TEXT("%d "), convex(cp) );
		//	}
		//	after_convex_string += FString::Printf(TEXT(")"));

		//	// print result
		//	debugf( *before_convex_string );
		//	debugf( *after_convex_string );
		//}
		
	}
}

//
// IndexBuffer	: Export Element Index Buffer
// VertexBuffer	: Static Mesh Vertex Buffer
//

void TrianglesToConvexPolygons(	const TArray<WORD>& IndexBuffer,
								const TArray<FLegacyStaticMeshVertex>& VertexBuffer,
								const TArray<FVector>& TriangleNormals,
								TArray<AvaConvexPolygon>* ConvexPolygons,
								TArray<INT>* ConvexPolygonIndexBuffer,
								TArray<FVector>* ConvexPlanes,
								TArray<FVector>* ConvexTangentXs,
								TArray<FVector>* ConvexTangentYs,
								TArray<INT>* ConvexToTris )
{
	const INT NumTriangles = IndexBuffer.Num() / 3;

	// store face normal list
	const TArray<FVector>& normals = TriangleNormals;
	//for( INT triIndex = 0; triIndex < NumTriangles; ++triIndex )
	//{
	//	FVector normal = (FVector)VertexBuffer(IndexBuffer( triIndex * 3 + 0 )).TangentZ;
	//	normals.AddItem( normal.SafeNormal() );
	//}

	// build edge list
	TArray<FMeshEdge> Edges;
	FStaticMeshEdgeBuilder2( IndexBuffer, VertexBuffer, Edges ).FindEdges();

	// build face edge list
	TArray< TArray<INT> > FaceEdgeListList;
	FaceEdgeListList.AddZeroed( NumTriangles );
	for( INT EdgeIndex = 0; EdgeIndex < Edges.Num(); ++EdgeIndex )
	{
		const FMeshEdge& Edge = Edges(EdgeIndex);
		if( Edge.Faces[0] != INDEX_NONE )
		{
			FaceEdgeListList( Edge.Faces[0] ).AddUniqueItem(EdgeIndex);
		}

		if( Edge.Faces[1] != INDEX_NONE )
		{
			FaceEdgeListList( Edge.Faces[1] ).AddUniqueItem(EdgeIndex);
		}
	}

	// find adjacent coplaner faces
	TArray<INT> FaceCoplanerGroup;
	FaceCoplanerGroup.AddZeroed( NumTriangles );
	ComputeCoplanerGroup( FaceEdgeListList, Edges, normals, &FaceCoplanerGroup );

	// co-planer group to facelist 
	TArray< TArray<INT> > GroupToFaceList;
	GroupToFaceList.AddZeroed( NumTriangles + 1 );
	for( INT faceIndex = 0; faceIndex < FaceCoplanerGroup.Num(); ++faceIndex )
	{
		const INT groupId = FaceCoplanerGroup(faceIndex);
		check( groupId != 0 );
		GroupToFaceList(groupId).AddItem( faceIndex );
	}

	// make different colors : 인접색이 다르도록 배치한다.
	TArray<FColor> CoplanerGroupColors;
	FColor CandidateColor = FColor::MakeRandomColor();
	CoplanerGroupColors.AddItem( CandidateColor );

	for( INT groupIndex = 1; groupIndex < GroupToFaceList.Num(); ++groupIndex)
	{
		CandidateColor = FColor::MakeRandomColor();

		while ( CoplanerGroupColors(groupIndex-1) == CandidateColor )
		{
			CandidateColor = FColor::MakeRandomColor();
		}

		CoplanerGroupColors.AddItem( CandidateColor );
	}

	// merge co-planer triangles to convex polygons
	for( INT coplanerIndex = 0; coplanerIndex < GroupToFaceList.Num(); ++coplanerIndex )
	{
		// super convex
		TArray<TArray<INT> > convexes;
		TArray<TArray<INT> > convex_to_face_list;

		TArray<INT>& coplanerTriList = GroupToFaceList( coplanerIndex );

		MergeCoplanerTrianglesToConvexPolygons( coplanerTriList, normals, FaceEdgeListList, Edges, VertexBuffer, IndexBuffer, CoplanerGroupColors,
			&convexes, &convex_to_face_list);

		if( convexes.Num() )
		{
			const FVector& planeNormal = normals( coplanerTriList(0) );

			// pack normal
			const INT plane_index = ConvexPlanes->AddUniqueItem( planeNormal );

			// pack index buffer and create polygon info
			for( INT convexIndex = 0; convexIndex < convexes.Num(); ++convexIndex )
			{
				const TArray<INT>& convex = convexes(convexIndex);
				const TArray<INT>& facelist = convex_to_face_list(convexIndex);

				if( convex.Num() == 0 )
				{
					continue;
				}

				// convex는 같은 tangent space를 가집니다. 따라서 모든 점에서의 tangent space가 같습니다.
				// pack tangent space
				const FLegacyStaticMeshVertex& Vertex = VertexBuffer(convex(0));
				const FVector& TangentX = (FVector) Vertex.TangentX;
				const FVector& TangentY = (FVector) Vertex.TangentY;

				FVector ConvexTangentX = TangentX - planeNormal * (TangentX | planeNormal);
				FVector ConvexTangentY = TangentY - planeNormal * (TangentY | planeNormal);

				const INT tan_x_index = ConvexTangentXs->AddUniqueItem( ConvexTangentX );
				const INT tan_y_index = ConvexTangentYs->AddUniqueItem( ConvexTangentY );

                // generate convex poly
				AvaConvexPolygon poly;

				poly.IndexStart_	= ConvexPolygonIndexBuffer->Num();
				poly.NumIndex_		= convex.Num();

				poly.TriStart_		= ConvexToTris->Num();
				poly.NumTris_		= facelist.Num();

				poly.PlainIndex_	= plane_index;
				poly.TangentXIndex_	= tan_x_index;
				poly.TangentYIndex_ = tan_y_index;

				poly.Color_			= CoplanerGroupColors( coplanerIndex );

				ConvexPolygons->AddItem( poly );

				// pack convex index buffer
				for( INT convex_point = 0; convex_point < convex.Num(); ++convex_point )
				{
					ConvexPolygonIndexBuffer->AddItem( convex( convex_point ) );
				}

				// pack element tri index
				for( INT convex_tri = 0; convex_tri < facelist.Num(); ++convex_tri)
				{
					ConvexToTris->AddItem( facelist(convex_tri) );
				}

				

				
			}
		}		
	}
}

#pragma optimize("", on)