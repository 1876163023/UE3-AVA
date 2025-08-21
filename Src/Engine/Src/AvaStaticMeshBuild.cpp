/*=============================================================================
AvaStaticMeshBuild.cpp: Calculate Static Mesh LightMap coordinate
Redduck inc
=============================================================================*/

#include "EnginePrivate.h"

#include "UnStaticMeshLegacy.h"

#include "UnTextureLayout.h"

#include "UnMeshBuild.h"

template< class Array, class Bulk > void CopyArrayToBulk( Array& Src, Bulk& Dest )
{
	Dest.RemoveBulkData();
	Dest.Lock( LOCK_READ_WRITE );
	void* DestPointer = Dest.Realloc( Src.Num() );
	appMemcpy( DestPointer, &Src(0), Dest.GetBulkDataSize() );
	Dest.Unlock();
}

// defined in AvaConvex.cpp..
void TrianglesToConvexPolygons( const TArray<WORD>& IndexBuffer,
							    const TArray<FLegacyStaticMeshVertex>& VertexBuffer,
								const TArray<FVector>& TriangleNormals,
								TArray<AvaConvexPolygon>* ConvexPolygons,
								TArray<INT>* ConvexPolygonIndexBuffer,
								TArray<FVector>* ConvexPlanes,
								TArray<FVector>* ConvexTangentXs,
								TArray<FVector>* ConvexTangentYs,
								TArray<INT>* ConvexToTris );

#define LIGHTMAP_MAX_WIDTH	2048
#define LIGHTMAP_MAX_HEIGHT	2048

#define LIGHTMAP_TEXTURE_WIDTH	512
#define LIGHTMAP_TEXTURE_HEIGHT	512


struct FStaticMeshLightMapLayout : FTextureLayout
{
	FStaticMeshLightMapLayout( UINT sizeX, UINT sizeY ) : FTextureLayout( 1, 1, sizeX, sizeY )
	{}
};


namespace 
{
	//
	//	PointsEqual
	//
	inline UBOOL PointsEqual(const FVector& V1,const FVector& V2)
	{
		if(Abs(V1.X - V2.X) > THRESH_POINTS_ARE_SAME * 4.0f)
			return 0;

		if(Abs(V1.Y - V2.Y) > THRESH_POINTS_ARE_SAME * 4.0f)
			return 0;

		if(Abs(V1.Z - V2.Z) > THRESH_POINTS_ARE_SAME * 4.0f)
			return 0;

		return 1;
	}

	//
	//	TangentsEqual
	//
	inline UBOOL TangentsEqual(const FVector& V1,const FVector& V2)
	{
		const FLOAT DOT_THRESH = 0.707106f;		// cos 45
		if( ( V1 | V2 ) < DOT_THRESH )
			return 0;

		return 1;
	}

	//
	//	NormalsEqual
	//
	inline UBOOL NormalsEqual(const FVector& V1,const FVector& V2)
	{
		const FLOAT DOT_THRESH = 0.99999f;
		if( ( V1 | V2 ) < DOT_THRESH )
			return 0;

		return 1;
	}


	inline UBOOL SameTangentSpace( const FVector& TanX, const FVector& TanY, const FVector& TanZ,
									const FVector& CompTanX, const FVector& CompTanY, const FVector& CompTanZ )
	{
		const FLOAT DOT_THRESH = 0.707106f;	// cos 45
		if( (TanX | CompTanX) < DOT_THRESH  )
			return 0;

		if( (TanY | CompTanY) < DOT_THRESH  )
			return 0;

		if( (TanZ | CompTanZ) < DOT_THRESH  )
			return 0;

		return 1;
	}

	//
	//	UVsEqual
	//

	inline UBOOL UVsEqual(const FVector2D& UV1,const FVector2D& UV2)
	{
		if(Abs(UV1.X - UV2.X) > (1.0f / 1024.0f))
			return 0;

		if(Abs(UV1.Y - UV2.Y) > (1.0f / 1024.0f))
			return 0;

		return 1;
	}

	//
	//	FindVertexIndex
	//

	INT FindVertexIndex(FVector Position,
						FPackedNormal TangentX,
						FPackedNormal TangentY,
						FPackedNormal TangentZ,
						TArray<FLegacyStaticMeshVertex>* ConvexPolygonVertexBuffer )
	{
		// Find any identical vertices already in the vertex buffer.

		INT	VertexBufferIndex = INDEX_NONE;

		for(INT VertexIndex = 0;VertexIndex < ConvexPolygonVertexBuffer->Num();VertexIndex++)
		{
			// Compare vertex position and normal.

			FLegacyStaticMeshVertex*	CompareVertex = &(*ConvexPolygonVertexBuffer)(VertexIndex);

			if( !PointsEqual(CompareVertex->Position,Position) )
			{
				continue;
			}

			if( !SameTangentSpace(CompareVertex->TangentX, CompareVertex->TangentY, CompareVertex->TangentZ, TangentX, TangentY, TangentZ) )
			{
				continue;
			}

			// The vertex matches!
			VertexBufferIndex = VertexIndex;
			break;
		}

		// If there is no identical vertex already in the vertex buffer...

		if(VertexBufferIndex == INDEX_NONE)
		{
			// Add a new vertex to the vertex buffers.

			FLegacyStaticMeshVertex	Vertex;

			Vertex.Position = Position;
			Vertex.TangentX = TangentX;
			Vertex.TangentY = TangentY;
			Vertex.TangentZ = TangentZ;

			VertexBufferIndex = ConvexPolygonVertexBuffer->AddItem(Vertex);
		}

		return VertexBufferIndex;
	}
}

UBOOL UStaticMesh::HasRadiosityGeometry()
{
	UBOOL Result = FALSE;

	if( LODModels.Num() )
	{
		Result = LODModels(0).BulkConvexPolygons.GetElementCount() > 0;
	}

	return Result;
}

UBOOL UStaticMesh::ModifyLuxelSize( INT LuxelSizeInWorldUnit )
{
	UBOOL bUpdated = FALSE;

	for( INT iLod = 0; iLod < LODModels.Num(); ++iLod )
	{
		if( LODModels(iLod).BulkConvexPolygons.GetElementCount() )
		{
			LODModels(iLod).GenerateLightMapCoordinate( LuxelSize );

			bUpdated = TRUE;
		}
	}

	return bUpdated;
}
void UStaticMesh::GenerateLightMapCoordinate()
{
	for( INT iLod = 0; iLod < LODModels.Num(); ++iLod )
	{
		LODModels(iLod).CreateRadiosityGeometry();
		LODModels(iLod).GenerateLightMapCoordinate( LuxelSize );
	}

	LightMapCoordinateIndex = 1;
}

void UStaticMesh::RemoveRadiosityGeometry()
{
	for( INT LODIndex = 0; LODIndex < LODModels.Num(); ++LODIndex )
	{
		LODModels(LODIndex).RemoveRadiosityGeometry();
	}
	Build();
	LightMapCoordinateIndex = 0;
}

void FStaticMeshRenderData::RemoveRadiosityGeometry()
{
	if( BulkExportElements.GetElementCount() != 0 )
	{
		// 생성된 Lightmap Coordinate를 삭제한다.
		FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) RawTriangles.Lock(LOCK_READ_WRITE);
		for(INT TriangleIndex = 0;TriangleIndex < RawTriangles.GetElementCount();TriangleIndex++)
		{
			FStaticMeshTriangle* Triangle = &RawTriangleData[TriangleIndex];
			Triangle->NumUVs = 1;
		}
		RawTriangles.Unlock();

		// Radiosity Geometry를 없앤다.
		BulkExportElements.RemoveBulkData();
		BulkConvexPolygons.RemoveBulkData();
		BulkConvexPlanes.RemoveBulkData();
		BulkConvexTangentXs.RemoveBulkData();
		BulkConvexTangentYs.RemoveBulkData();
		BulkConvexToTris.RemoveBulkData();
		BulkConvexPolygonIndexBuffer.RemoveBulkData();
		BulkConvexPolygonVertexBuffer.RemoveBulkData();
	}
}

void FStaticMeshRenderData::CreateRadiosityGeometry()
{
	const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) RawTriangles.Lock(LOCK_READ_ONLY);

	INT NumDegenerate = 0;

	// create mapping element triangle index to raw triangle index
	TArray<TArray<INT> >	ElementTriangleToRawTriangle;
	for( INT ElementIndex = 0; ElementIndex < Elements.Num(); ++ElementIndex )
	{
		ElementTriangleToRawTriangle.AddItem(TArray<INT>());
	}

	// create mapping element triangle 
	TArray<TArray<FVector> > ElementTriangleNormals;
	for( INT ElementIndex = 0; ElementIndex < Elements.Num(); ++ElementIndex )
	{
		ElementTriangleNormals.AddItem(TArray<FVector>());
	}


	// Initialize material index buffers.
	TArray<TArray<WORD> >	ElementIndexBuffers;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		new(ElementIndexBuffers) TArray<WORD>();
	}

	// export related information
	TArray<AvaStaticMeshExportElement>		ExportElements;
	TArray<AvaConvexPolygon>				ConvexPolygons;
	TArray<FVector>							ConvexPlanes;				// for convex polygon's normals
	TArray<FVector>							ConvexTangentXs;			// for convex polygon's tangent X list
	TArray<FVector>							ConvexTangentYs;			// for convex polygon's tangent Y list
	TArray<INT>								ConvexToTris;				// for convex polygon to element triangle list list
	TArray<INT>								ConvexPolygonIndexBuffer;	// index buffer
	TArray<FLegacyStaticMeshVertex>			ConvexPolygonVertexBuffer;	// position and uv
	TArray<FVector2D>						ConvexPolygonUVBuffer;

	// Calculate triangle normals.
	TArray<FVector>	TriangleTangentX(RawTriangles.GetElementCount());
	TArray<FVector>	TriangleTangentY(RawTriangles.GetElementCount());
	TArray<FVector>	TriangleTangentZ(RawTriangles.GetElementCount());

	for(INT TriangleIndex = 0;TriangleIndex < RawTriangles.GetElementCount();TriangleIndex++)
	{
		const FStaticMeshTriangle*	Triangle = &RawTriangleData[TriangleIndex];
		INT						UVIndex = 0;
		FVector					TriangleNormal = FPlane(
			Triangle->Vertices[2],
			Triangle->Vertices[1],
			Triangle->Vertices[0]
			);

			FVector	P1 = Triangle->Vertices[0],	P2 = Triangle->Vertices[1],	P3 = Triangle->Vertices[2];

			FMatrix	ParameterToLocal(
				FPlane(	P2.X - P1.X,	P2.Y - P1.Y,	P2.Z - P1.Z,	0	),
				FPlane(	P3.X - P1.X,	P3.Y - P1.Y,	P3.Z - P1.Z,	0	),
				FPlane(	P1.X,			P1.Y,			P1.Z,			0	),
				FPlane(	0,				0,				0,				1	)
				);

			FVector2D	T1 = Triangle->UVs[0][UVIndex],
				T2 = Triangle->UVs[1][UVIndex],
				T3 = Triangle->UVs[2][UVIndex];

			FMatrix		ParameterToTexture(
				FPlane(	T2.X - T1.X,	T2.Y - T1.Y,	0,	0	),
				FPlane(	T3.X - T1.X,	T3.Y - T1.Y,	0,	0	),
				FPlane(	T1.X,			T1.Y,			1,	0	),
				FPlane(	0,				0,				0,	1	)
				);

			FMatrix	TextureToLocal = ParameterToTexture.Inverse() * ParameterToLocal;
			FVector	TangentX = TextureToLocal.TransformNormal(FVector(1,0,0)).SafeNormal(),
				TangentY = TextureToLocal.TransformNormal(FVector(0,1,0)).SafeNormal(),
				TangentZ;

			TangentX = TangentX - TriangleNormal * (TangentX | TriangleNormal);
			TangentY = TangentY - TriangleNormal * (TangentY | TriangleNormal);
			TangentZ = TriangleNormal;

			TriangleTangentX(TriangleIndex) = TangentX.SafeNormal();
			TriangleTangentY(TriangleIndex) = TangentY.SafeNormal();
			TriangleTangentZ(TriangleIndex) = TangentZ.SafeNormal();
	}



	for( INT TriIndex = 0; TriIndex < RawTriangles.GetElementCount(); ++TriIndex )
	{
		const FStaticMeshTriangle* Triangle = &RawTriangleData[TriIndex];

		if( PointsEqual( Triangle->Vertices[0], Triangle->Vertices[1])
			|| PointsEqual( Triangle->Vertices[0], Triangle->Vertices[2])
			|| PointsEqual( Triangle->Vertices[1], Triangle->Vertices[2])
			)
		{
			// degenerate triangles
			++NumDegenerate;
			continue;
		}

		INT VertexIndices[3];
		for( INT VertexIndex = 0; VertexIndex < 3; ++VertexIndex )
		{
			// point 위치가 같고, tangent space가 비슷하면 같은 vertex 라고 판별합니다.
			// tangent space는 mirror 된 경우는 다른 vertex 라고 판별합니다.
			// shared vertex
			VertexIndices[VertexIndex] = FindVertexIndex(
				Triangle->Vertices[VertexIndex],
				TriangleTangentX(TriIndex),
				TriangleTangentY(TriIndex),
				TriangleTangentZ(TriIndex),
				&ConvexPolygonVertexBuffer );
		}

		// Reject degenerate triangles.

		if(VertexIndices[0] == VertexIndices[1] || VertexIndices[1] == VertexIndices[2] || VertexIndices[0] == VertexIndices[2])
		{
			++NumDegenerate;
			continue;
		}

		// add index
		// raw triangle 의 winding 은 ccw 입니다.
		// 하지만, 왼손좌표계를 사용하기 때문에, winding 순서를 cw로 바꿉니다.
		ElementIndexBuffers(Triangle->MaterialIndex).AddItem( (WORD)VertexIndices[2] );
		ElementIndexBuffers(Triangle->MaterialIndex).AddItem( (WORD)VertexIndices[1] );
		ElementIndexBuffers(Triangle->MaterialIndex).AddItem( (WORD)VertexIndices[0] );

		// element triangle index -> raw tri index
		ElementTriangleToRawTriangle(Triangle->MaterialIndex).AddItem( TriIndex );

		ElementTriangleNormals(Triangle->MaterialIndex).AddItem( TriangleTangentZ(TriIndex) );
	}

	// 여기부터는 RawTriangle 이 필요 없네요...
	RawTriangles.Unlock();

	// 이제부터 winding은 cw 입니다~

	// 아래의 내용을 Element 별로 할 수 있도록 변경해야 합니다...~~~~~ 어렵네....
	// 그 후에는... 이 내용을 가지고 Export 할 수 있도록 변경해야 합니다... 할 일이 많네~~

	for( INT elementIndex = 0; elementIndex < Elements.Num(); ++elementIndex )
	{
		const TArray<WORD>& elementIndexBuffer = ElementIndexBuffers(elementIndex);

		AvaStaticMeshExportElement exportElement;
		exportElement.PolygonStart_ = ConvexPolygons.Num();

		TrianglesToConvexPolygons(	elementIndexBuffer,
			ConvexPolygonVertexBuffer,
			ElementTriangleNormals(elementIndex),
			&ConvexPolygons,
			&ConvexPolygonIndexBuffer,
			&ConvexPlanes,
			&ConvexTangentXs,
			&ConvexTangentYs,
			&ConvexToTris );

		exportElement.NumPolygons_ = ConvexPolygons.Num() - exportElement.PolygonStart_;

		// element tri index -> raw triangle index로 변환합니다...
		for( INT polyIndex = exportElement.PolygonStart_; polyIndex < ConvexPolygons.Num(); ++polyIndex )
		{
			const AvaConvexPolygon& poly = ConvexPolygons(polyIndex);
			for( INT triIndex = 0; triIndex < poly.NumTris_; ++triIndex )
			{
				ConvexToTris(poly.TriStart_ + triIndex) = (ElementTriangleToRawTriangle(elementIndex))(ConvexToTris(poly.TriStart_ + triIndex));
			}
		}

		ExportElements.AddItem( exportElement );
	}

	// export geometry bulk data를 채웁니다.
	CopyArrayToBulk( ExportElements, BulkExportElements );
	CopyArrayToBulk( ConvexPolygons, BulkConvexPolygons );
	CopyArrayToBulk( ConvexPlanes, BulkConvexPlanes );
	CopyArrayToBulk( ConvexTangentXs, BulkConvexTangentXs );
	CopyArrayToBulk( ConvexTangentYs, BulkConvexTangentYs );
	CopyArrayToBulk( ConvexToTris, BulkConvexToTris );
	CopyArrayToBulk( ConvexPolygonIndexBuffer, BulkConvexPolygonIndexBuffer );
	CopyArrayToBulk( ConvexPolygonVertexBuffer, BulkConvexPolygonVertexBuffer );
}

void FStaticMeshRenderData::GenerateLightMapCoordinate( INT LuxelInWorldUnit )
{
	// Light Map 좌표를 생성하기 위해서는 Radiosity Geometry가 있어야 합니다..
	if( BulkExportElements.GetElementCount() == 0 )
	{
		CreateRadiosityGeometry();
	}

	FStaticMeshLightMapLayout LightMapLayOut( LIGHTMAP_MAX_WIDTH, LIGHTMAP_MAX_HEIGHT );

	const UINT Padding = 2;
	const INT LightMapCoordChannel = 1;

	FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) RawTriangles.Lock(LOCK_READ_WRITE);

	// generate light map texture coorinate
	const AvaStaticMeshExportElement* RawExportElements	= (AvaStaticMeshExportElement*) BulkExportElements.Lock(LOCK_READ_ONLY);
	AvaConvexPolygon* RawPolygons						= (AvaConvexPolygon*) BulkConvexPolygons.Lock(LOCK_READ_WRITE);
	const INT* RawConvexToTris							= (INT*) BulkConvexToTris.Lock(LOCK_READ_ONLY);
	const INT* RawIndices								= (INT*) BulkConvexPolygonIndexBuffer.Lock(LOCK_READ_ONLY);
	const FLegacyStaticMeshVertex* RawVertices			= (FLegacyStaticMeshVertex*) BulkConvexPolygonVertexBuffer.Lock(LOCK_READ_ONLY);

	for( INT elementIndex = 0; elementIndex < Elements.Num(); ++elementIndex )
	{
		//const AvaStaticMeshExportElement& element = ExportElements(elementIndex);
		const AvaStaticMeshExportElement& element = RawExportElements[elementIndex];
		for( INT polyIndex = 0; polyIndex < element.NumPolygons_; ++polyIndex )
		{
			//AvaConvexPolygon& poly = ConvexPolygons( element.PolygonStart_ + polyIndex );
			AvaConvexPolygon& poly = RawPolygons[element.PolygonStart_ + polyIndex];
			
			// calculate lightmap axis
			if( poly.NumTris_ == 0 )
			{
				continue;
			}

			//FStaticMeshTriangle* Triangle = &RawTriangleData[ConvexToTris(poly.TriStart_)];
			FStaticMeshTriangle* Triangle = &RawTriangleData[RawConvexToTris[poly.TriStart_]];

			FVector Vert[3];
			Vert[0] = Triangle->Vertices[0];
			Vert[1] = Triangle->Vertices[1];
			Vert[2] = Triangle->Vertices[2];

			FVector MapX;
			FVector MapY;

			// find a plane parallel to the surface
			// winding 방향을 반대로 해야, normal 방향이 맞다.
			FPlane Plane( Vert[2], Vert[1], Vert[0] );
			Plane.FindBestAxisVectors( MapX, MapY );

			// light map axis
			poly.LightMapXInWorldUnit_ = MapX;
			poly.LightMapYInWorldUnit_ = MapY;

			FVector UVecsInWorldUnit = MapX / (FLOAT) LuxelInWorldUnit;
			FVector VVecsInWorldUnit = MapY / (FLOAT) LuxelInWorldUnit;

			// calculate lightmap extents for polygon
			FVector2D Mins(WORLD_MAX, WORLD_MAX);
			FVector2D Maxs(-WORLD_MAX, -WORLD_MAX);

			for( INT v = 0; v < poly.NumIndex_; ++v )
			{
				INT vertex_index = RawIndices[poly.IndexStart_ + v];
				const FVector& pos = RawVertices[vertex_index].Position;

				FLOAT X = UVecsInWorldUnit | pos;	// U in luxel
				FLOAT Y = VVecsInWorldUnit | pos;	// V in luxel

				Mins.X = Min( X, Mins.X );
				Mins.Y = Min( Y, Mins.Y );
				Maxs.X = Max( X, Maxs.X );
				Maxs.Y = Max( Y, Maxs.Y );
			}

			FVector2D LightMapSize = Maxs - Mins + FVector2D( 1.9999f, 1.9999f );

			INT LightMapWidth = (INT)LightMapSize.X;
			INT LightMapHeight = (INT)LightMapSize.Y;

			// WorldUnit 사이즈에 따라 작을 수 있다.
			check( LightMapWidth > 0);
			check( LightMapHeight > 0);

			UINT BaseX, BaseY;

			UBOOL IsAdded = LightMapLayOut.AddElement( &BaseX, &BaseY, LightMapWidth + Padding * 2, LightMapHeight + Padding * 2 );

			check(IsAdded);	// 항상 성공하도록 만들어야 합니다.....

			// calculate uv coordinate in luxel 나중에 layout size로 나누어 주어야 합니다.
			BaseX += Padding;
			BaseY += Padding;

			// light map alloc 위치
			poly.BaseX_ = BaseX;
			poly.BaseY_ = BaseY;

			for( INT faceIndex = 0; faceIndex < poly.NumTris_; ++faceIndex )
			{
				//FStaticMeshTriangle* Triangle = &RawTriangleData[ConvexToTris(poly.TriStart_ + faceIndex)];
				FStaticMeshTriangle* Triangle = &RawTriangleData[RawConvexToTris[poly.TriStart_ + faceIndex]];

				// channel
				Triangle->NumUVs = Triangle->NumUVs < 2 ? 2 : Triangle->NumUVs;

				Vert[0] = Triangle->Vertices[0];
				Vert[1] = Triangle->Vertices[1];
				Vert[2] = Triangle->Vertices[2];

				

				for( INT vertexIndex = 0; vertexIndex < 3; ++vertexIndex )
				{
					FLOAT X = UVecsInWorldUnit | Vert[vertexIndex];	// U in luxel
					FLOAT Y = VVecsInWorldUnit | Vert[vertexIndex];	// V in luxel

					FVector2D LuxelCoord = FVector2D(X, Y) - Mins;

					check( LuxelCoord.X <= LightMapWidth && LuxelCoord.Y <= LightMapHeight );

					Triangle->UVs[vertexIndex][LightMapCoordChannel] = FVector2D( (FLOAT) BaseX, (FLOAT) BaseY );
					Triangle->UVs[vertexIndex][LightMapCoordChannel] += LuxelCoord;
				}
			}
		}	// loop convex polygon
	} // loop element

	// light map coordinate를 전체 light map size로 나누어야 합니다.
	for( INT TriangleIndex = 0; TriangleIndex < RawTriangles.GetElementCount(); ++TriangleIndex )
	{
		FStaticMeshTriangle* Triangle = &RawTriangleData[TriangleIndex];

		for( INT iVertex = 0; iVertex < 3; ++iVertex )
		{
			Triangle->UVs[iVertex][LightMapCoordChannel] /= FVector2D( (FLOAT)LightMapLayOut.GetSizeX(), (FLOAT) LightMapLayOut.GetSizeY() );
		}
	}

	// unlock bulk data
	BulkExportElements.Unlock();
	BulkConvexPolygons.Unlock();
	BulkConvexToTris.Unlock();
	BulkConvexPolygonIndexBuffer.Unlock();
	BulkConvexPolygonVertexBuffer.Unlock();

	RawTriangles.Unlock();

	LightMapWidth = LightMapLayOut.GetSizeX();
	LightMapHeight = LightMapLayOut.GetSizeY();
}