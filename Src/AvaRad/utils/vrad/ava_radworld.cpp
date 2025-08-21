// Redduck Inc, 2007

#include "ava_radworld.h"

#include "vrad.h"
#include "lightmap.h"

// dplanes / dvertexes / texinfo / dtexdata / dfaces / dedges / dsurfaceedges
void AVA_BuildRadWorld()
{
	numfaces	= s_pVRadDll2->NumFaces_;
	numplanes	= s_pVRadDll2->NumPlanes_;
	numtexinfo	= s_pVRadDll2->NumTextureInfos_;
	numtexdata	= s_pVRadDll2->NumTextureDatas_;
	numvertexes = s_pVRadDll2->NumVertices_;
	numedges	= 0;
	numsurfedges= 0;

	// edge 개수 세기 : 나중에 edge 도 export 할 것
	for( int i = 0; i < numfaces; ++ i)
	{
		AvaFace* face = &s_pVRadDll2->FaceArray_[i];
		numedges += face->NumVertices_;
		numsurfedges += face->NumVertices_;
	}

	// allocate memory for calculation
	// 이 데이타들은 초기화된다.
	dfaces.SetSize( numfaces );
	dvertexes.SetSize( numvertexes );
	dplanes.SetSize( numplanes );
	texinfo.SetSize( numtexinfo );
	dtexdata.SetSize( numtexdata );
	dedges.SetSize( numedges );
	dsurfedges.SetSize( numsurfedges );

	// 초기화되지 않음.
	vertexref.SetSize( numvertexes );
	memset( vertexref.Base(), 0, sizeof( vertexref[0] ) * numvertexes );

	// 다른 이들에서 copy 됨
	g_vertnormalindices.SetSize( numvertexes );	// save vertex normal 시 사용됨
	g_vertnormals.SetSize( numvertexes );
	vertexface.SetSize( numvertexes );
	faceneighbor.SetSize( numfaces );
	facelight.SetSize( numfaces );
	face_offset.SetSize( numfaces );
	face_centroids.SetSize( numedges );

	memset( &faceneighbor.Element(0), 0, sizeof(faceneighbor_t) * numfaces );
	memset( &facelight.Element(0), 0, sizeof(facelight_t) * numfaces );
	memset( &face_offset.Element(0), 0, sizeof(Vector) * numfaces );
	memset( &face_centroids.Element(0), 0, sizeof(Vector) * numfaces );	

	// dispinfo
	static ddispinfo_t dispinfo;

	// 사용하지 않는 필드를 초기화한다.
	memset( &dispinfo, 0, sizeof( ddispinfo_t ) );
	for( int neighborIndex = 0; neighborIndex < 4; ++neighborIndex )
	{
		dispinfo.m_EdgeNeighbors[neighborIndex].m_SubNeighbors[1].SetInvalid();
		dispinfo.m_CornerNeighbors[neighborIndex].SetInvalid();
	}

	// copy
	for( int dispIndex = 0; dispIndex < s_pVRadDll2->NumDispInfos_; ++dispIndex )
	{
		AvaDispInfo* srcDispInfo = &s_pVRadDll2->DispInfos_[dispIndex];

		dispinfo.startPosition		= srcDispInfo->StartPosition_;
		dispinfo.m_iDispVertStart	= srcDispInfo->DispVertexStart_;
		dispinfo.m_iDispTriStart	= -1;
		dispinfo.power				= srcDispInfo->Power_;
		dispinfo.contents			= MASK_OPAQUE;

		for( int neighborIndex = 0; neighborIndex < 4; ++neighborIndex )
		{
			CDispSubNeighbor& neighbor = dispinfo.m_EdgeNeighbors[neighborIndex].m_SubNeighbors[0];
			neighbor.m_iNeighbor			= srcDispInfo->Neighbor_[neighborIndex].NeighborIndex_;
			neighbor.m_NeighborOrientation	= srcDispInfo->Neighbor_[neighborIndex].NeighborOrientation_;
			neighbor.m_NeighborSpan			= srcDispInfo->Neighbor_[neighborIndex].NeighborSpan_;
			neighbor.m_Span					= srcDispInfo->Neighbor_[neighborIndex].Span_;
		}

		g_dispinfo.AddToTail(dispinfo);
	}

	// dispvertex
	// copy
	static CDispVert dispvertex;
	for( int dispv = 0; dispv < s_pVRadDll2->NumDispVertices_; ++dispv )
	{
		AvaDispVertex* srcDispV = &s_pVRadDll2->DispVertices_[dispv];

		dispvertex.m_vVector	= srcDispV->DispVector_;
		dispvertex.m_flDist		= srcDispV->DispDistance_;
		dispvertex.m_flAlpha	= srcDispV->Alpha_;

		g_DispVerts.AddToTail(dispvertex);
	}

	// vertex 리스트 복사
	for( int v = 0; v < numvertexes; ++v )
	{
		dvertexes[v].point = s_pVRadDll2->VertexArray_[v].Position_;
	}	

	// 구성면 정보 복사
	int currentEdge = 0;
	for( int i = 0; i < numfaces; ++ i)
	{
		// 구성면 정보 생성
		AvaFace* face = &s_pVRadDll2->FaceArray_[i];
		dface_t* targetFace = &dfaces[i];
		targetFace->smoothingGroups = 0;
		targetFace->dispinfo = face->DispIndex_;

		// 구성면에 평면정보 연결하기
		int PlaneIndex = face->PlaneIndex_;
		targetFace->planenum = PlaneIndex;
		AvaPlane* plane = &s_pVRadDll2->PlaneArray_[PlaneIndex];
		dplanes[PlaneIndex].normal = plane->Normal_;
		dplanes[PlaneIndex].dist = plane->Distance_;

		// 구성면에 텍스쳐 정보 연결하기
		int TexInfoIndex = face->TextureInfoIndex_;
		AvaTextureInfo* TexInfo = &s_pVRadDll2->TextureInfoArray_[TexInfoIndex];
		targetFace->texinfo = TexInfoIndex;

		Vector UVec, VVec;

		VectorMultiply( TexInfo->TexturemapUAxis_, 1.0f / TexInfo->TexelInWorldUnit_, UVec );
		VectorMultiply( TexInfo->TexturemapVAxis_, 1.0f / TexInfo->TexelInWorldUnit_, VVec );

		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[0][0] = UVec.x;
		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[0][1] = UVec.y;
		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[0][2] = UVec.z;
		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[0][3] = 0.0f;

		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[1][0] = VVec.x;
		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[1][1] = VVec.y;
		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[1][2] = VVec.z;
		texinfo[TexInfoIndex].textureVecsTexelsPerWorldUnits[1][3] = 0.0f;

		VectorMultiply( TexInfo->LightmapUAxis_, 1.0f / TexInfo->LuxelInWorldUnit_, UVec );
		VectorMultiply( TexInfo->LightmapVAxis_, 1.0f / TexInfo->LuxelInWorldUnit_, VVec );

		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[0][0] = UVec.x;
		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[0][1] = UVec.y;
		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[0][2] = UVec.z;
		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[0][3] = TexInfo->LightmapUOffset_;

		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[1][0] = VVec.x;
		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[1][1] = VVec.y;
		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[1][2] = VVec.z;
		texinfo[TexInfoIndex].lightmapVecsLuxelsPerWorldUnits[1][3] = TexInfo->LightmapVOffset_;

		texinfo[TexInfoIndex].flags = 0;
		//texinfo[TexInfoIndex].flags |= SURF_BUMPLIGHT;	// 항상 BUMP 을 사용한다.		
		texinfo[TexInfoIndex].flags |= TexInfo->Flags_;

		int TexDataIndex = TexInfo->TextureDataIndex_;
		texinfo[TexInfoIndex].texdata = TexDataIndex;
		AvaTextureData* TexData = &s_pVRadDll2->TextureDataArray_[TexDataIndex];		
		dtexdata[TexDataIndex].reflectivity = TexData->Reflectivity_;
		dtexdata[TexDataIndex].brightness = TexData->Brightness_;
		dtexdata[TexDataIndex].width = max( 32, TexData->Width_ );
		dtexdata[TexDataIndex].height = max( 32, TexData->Height_ );
		dtexdata[TexDataIndex].nameStringTableID = 0;		

		// edge 추가 : 시계방향 edge 만 들어온다고 가정
		dfaces[i].firstedge = currentEdge;
		dfaces[i].numedges = face->NumVertices_;
		for( int v= 0; v < face->NumVertices_; ++v )
		{
			dedges[currentEdge + v].v[0] = face->VertexIndices_[v % face->NumVertices_ ];
			dedges[currentEdge + v].v[1] = face->VertexIndices_[(v+1) % face->NumVertices_];

			dsurfedges[currentEdge + v] = currentEdge + v;
		}
		currentEdge += face->NumVertices_;
	}

	// black mesh texdata
	for( int bm_texdataIndex = 0; bm_texdataIndex < s_pVRadDll2->NumBlackMeshTexDataIndices_;++bm_texdataIndex )
	{
		const int tdIndex = s_pVRadDll2->BlackMeshTexDataIndexArray_[bm_texdataIndex];
		AvaTextureData* TexData = &s_pVRadDll2->TextureDataArray_[tdIndex];		
		dtexdata[tdIndex].reflectivity = TexData->Reflectivity_;
		dtexdata[tdIndex].brightness = TexData->Brightness_;
		dtexdata[tdIndex].width = max( 32, TexData->Width_ );
		dtexdata[tdIndex].height = max( 32, TexData->Height_ );
		dtexdata[tdIndex].nameStringTableID = 0;		
	}


}
