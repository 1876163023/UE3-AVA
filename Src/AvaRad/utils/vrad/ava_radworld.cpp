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

	// edge ���� ���� : ���߿� edge �� export �� ��
	for( int i = 0; i < numfaces; ++ i)
	{
		AvaFace* face = &s_pVRadDll2->FaceArray_[i];
		numedges += face->NumVertices_;
		numsurfedges += face->NumVertices_;
	}

	// allocate memory for calculation
	// �� ����Ÿ���� �ʱ�ȭ�ȴ�.
	dfaces.SetSize( numfaces );
	dvertexes.SetSize( numvertexes );
	dplanes.SetSize( numplanes );
	texinfo.SetSize( numtexinfo );
	dtexdata.SetSize( numtexdata );
	dedges.SetSize( numedges );
	dsurfedges.SetSize( numsurfedges );

	// �ʱ�ȭ���� ����.
	vertexref.SetSize( numvertexes );
	memset( vertexref.Base(), 0, sizeof( vertexref[0] ) * numvertexes );

	// �ٸ� �̵鿡�� copy ��
	g_vertnormalindices.SetSize( numvertexes );	// save vertex normal �� ����
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

	// ������� �ʴ� �ʵ带 �ʱ�ȭ�Ѵ�.
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

	// vertex ����Ʈ ����
	for( int v = 0; v < numvertexes; ++v )
	{
		dvertexes[v].point = s_pVRadDll2->VertexArray_[v].Position_;
	}	

	// ������ ���� ����
	int currentEdge = 0;
	for( int i = 0; i < numfaces; ++ i)
	{
		// ������ ���� ����
		AvaFace* face = &s_pVRadDll2->FaceArray_[i];
		dface_t* targetFace = &dfaces[i];
		targetFace->smoothingGroups = 0;
		targetFace->dispinfo = face->DispIndex_;

		// �����鿡 ������� �����ϱ�
		int PlaneIndex = face->PlaneIndex_;
		targetFace->planenum = PlaneIndex;
		AvaPlane* plane = &s_pVRadDll2->PlaneArray_[PlaneIndex];
		dplanes[PlaneIndex].normal = plane->Normal_;
		dplanes[PlaneIndex].dist = plane->Distance_;

		// �����鿡 �ؽ��� ���� �����ϱ�
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
		//texinfo[TexInfoIndex].flags |= SURF_BUMPLIGHT;	// �׻� BUMP �� ����Ѵ�.		
		texinfo[TexInfoIndex].flags |= TexInfo->Flags_;

		int TexDataIndex = TexInfo->TextureDataIndex_;
		texinfo[TexInfoIndex].texdata = TexDataIndex;
		AvaTextureData* TexData = &s_pVRadDll2->TextureDataArray_[TexDataIndex];		
		dtexdata[TexDataIndex].reflectivity = TexData->Reflectivity_;
		dtexdata[TexDataIndex].brightness = TexData->Brightness_;
		dtexdata[TexDataIndex].width = max( 32, TexData->Width_ );
		dtexdata[TexDataIndex].height = max( 32, TexData->Height_ );
		dtexdata[TexDataIndex].nameStringTableID = 0;		

		// edge �߰� : �ð���� edge �� ���´ٰ� ����
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
