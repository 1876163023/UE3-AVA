#include "vrad.h"
#include "radial.h"

#undef FastSqrt
#undef ARRAYSIZE
#include "opcode.h"
#include <stack>
#include <vector>

//#pragma optimize("", off)

Opcode::Model s_WorldModel;
Opcode::Model s_WorldShadowCasterModel;

#define MAX_COLLISION_MODELS 4096
Opcode::Model			s_WorldModels[MAX_COLLISION_MODELS];
Opcode::MeshInterface	s_Mesh[MAX_COLLISION_MODELS];

std::vector<IndexedTriangle>	ShadowCasterTriangles;
std::vector<IndexedTriangle>	Triangles;
std::vector<Point>				Vertexes;
std::vector<IndexedTriangle>	DispTriangles;
std::vector<Point>				DispVertexes;
std::vector<int>				DispParentFaceIndices;

std::vector<AVA_StaticMesh>		static_meshes;

CUtlVector<int>	opcodeface_to_dface_and_blackmesh;	// positive = dface index  + 1, negative = - (blackmesh index + 1)
CUtlVector<int> triangle_to_meshid;
CUtlVector<Vector> staticmesh_triangle_normals;

CUtlVector<int> sun_visibility_array;	// 2007. 11. 12 changmin

static DWORD RC_TLS_INDEX = 0;
int skip_id = -1;
int contact_id = -1;
int	self_id = -1;

int AVA_StaticMeshManager::GetNumMeshes()
{
	return (int) static_meshes.size();
}

const AVA_StaticMesh& AVA_StaticMeshManager::GetStaticMesh( int meshid )
{
	return static_meshes[meshid];
}

int AVA_StaticMesh::GetNumTrisnagles() const
{
	return numtriangles_;
}

int AVA_StaticMesh::GetStartVertex() const
{
	return startvertex_;
}

int AVA_StaticMesh::GetNumVertices() const
{
	return numvertices_;
}
void AVA_StaticMesh::GetTriangleIndices( int triangleId, int *i0, int *i1, int *i2 ) const
{
	*i0 = s_pVRadDll2->BlackMeshIndexArray_[startIndex_ + triangleId * 3 + 0];
	*i1 = s_pVRadDll2->BlackMeshIndexArray_[startIndex_ + triangleId * 3 + 1];
	*i2 = s_pVRadDll2->BlackMeshIndexArray_[startIndex_ + triangleId * 3 + 2];
}
const Vector& AVA_StaticMesh::GetVertex( int index ) const
{
	return s_pVRadDll2->BlackMeshVertexArray_[index].Position_;
}
const Vector& AVA_StaticMesh::GetTangentS( int index ) const
{
	return s_pVRadDll2->BlackMeshTangentXArray_[index].Position_;
}
const Vector& AVA_StaticMesh::GetTangentT( int index ) const
{
	return s_pVRadDll2->BlackMeshTangentYArray_[index].Position_;
}
const Vector& AVA_StaticMesh::GetVertexNormal( int index ) const
{
	return s_pVRadDll2->BlackMeshTangentZArray_[index].Position_;
}

static struct RayColliders
{
	RayColliders()
	{
		InitializeCriticalSection( &CS );
		RC_TLS_INDEX = TlsAlloc();
	}
	~RayColliders()
	{
		while (!s.empty())
		{
			delete s.top();
			s.pop();
		}
		DeleteCriticalSection( &CS );
		TlsFree( RC_TLS_INDEX );
	}
	Opcode::RayCollider* Allocate()
	{
		Opcode::RayCollider* p = new Opcode::RayCollider;
		EnterCriticalSection( &CS );
		s.push(p);
		LeaveCriticalSection( &CS );
		TlsSetValue( RC_TLS_INDEX, p );
		return p;
	}
	std::stack<Opcode::RayCollider*> s;
	CRITICAL_SECTION CS;
} _RayCollidersInstance;

void AddDispVertexToOpcodeModel( const Vector& v )
{
	static Point pos;
	pos.Set( v.x, v.y, v.z );
	DispVertexes.push_back( pos );
}

void AddDispTriangleToOpcodeModel( int v1, int v2, int v3, int parentFaceIndex )
{
	static IndexedTriangle Tri;
	Tri.mVRef[0] = v1;
	Tri.mVRef[1] = v2;
	Tri.mVRef[2] = v3;
	DispTriangles.push_back( Tri );
	DispParentFaceIndices.push_back( parentFaceIndex );
}

void CreateOpcodeModels()
{
	Triangles.clear();
	Vertexes.clear();
	ShadowCasterTriangles.clear();

	// create vetex buffer for collision model
	Vertexes.resize( numvertexes + s_pVRadDll2->NumBlackMeshVertices_ );

	// append displacements vertexes
	Vertexes.insert( Vertexes.end(), DispVertexes.begin(), DispVertexes.end() );

	// copy face vertex
	for( int v=0; v < numvertexes; ++ v)
	{
		const Vector& pos = dvertexes[v].point;
		Vertexes[v].Set( pos.x, pos.y, pos.z );
	}

	// black meshes
	const int staticmesh_vertexbase = numvertexes;
	num_blackmesh_vertexes = s_pVRadDll2->NumBlackMeshVertices_;
	blackmesh_vertexes.SetSize(num_blackmesh_vertexes);
	blackmesh_tangent_x.SetSize(num_blackmesh_vertexes);
	blackmesh_tangent_y.SetSize(num_blackmesh_vertexes);
	blackmesh_tangent_z.SetSize(num_blackmesh_vertexes);

	blackmesh_vertexlight_float_vector_data.SetSize( num_blackmesh_vertexes * 4 );	// num bump vector
	memset( blackmesh_vertexlight_float_vector_data.Base(), 0, sizeof(Vector) * num_blackmesh_vertexes * 4 );

	blackmesh_realtime_vertexlighting.SetSize( num_blackmesh_vertexes * 4 );
	memset( blackmesh_realtime_vertexlighting.Base(), 0, sizeof(Vector) * num_blackmesh_vertexes * 4 );

	// bouncing data는 bump가 필요없어요
	blackmesh_bouncing_data.SetSize( num_blackmesh_vertexes );
	memset( blackmesh_bouncing_data.Base(), 0, sizeof(Vector) * num_blackmesh_vertexes );

	blackmesh_vertexlight_weight_data.SetSize( num_blackmesh_vertexes );
	memset( blackmesh_vertexlight_weight_data.Base(), 0, sizeof( float ) * num_blackmesh_vertexes );

	// size = rgbe(=4) * numbumpvect(=4) * num_blackmesh_vertexes
	const int vertexlight_data_size = num_blackmesh_vertexes * 4 * 4;
	blackmesh_vertexlightdata.SetSize(vertexlight_data_size);
	memset(blackmesh_vertexlightdata.Base(), 0, vertexlight_data_size );

	// ray trace option 일때만 필요한 data
	blackmesh_worldvertexlightdata.SetSize(vertexlight_data_size);
	memset(blackmesh_worldvertexlightdata.Base(), 0, vertexlight_data_size);

	for( int v = 0; v < num_blackmesh_vertexes; ++v)
	{
		const Vector& pos = s_pVRadDll2->BlackMeshVertexArray_[v].Position_;
		Vertexes[staticmesh_vertexbase + v].Set( pos.x, pos.y, pos.z );

		blackmesh_vertexes[v] = pos;
		blackmesh_tangent_x[v] = s_pVRadDll2->BlackMeshTangentXArray_[v].Position_;
		blackmesh_tangent_y[v] = s_pVRadDll2->BlackMeshTangentYArray_[v].Position_;
		blackmesh_tangent_z[v] = s_pVRadDll2->BlackMeshTangentZArray_[v].Position_;
	}

	// model을 구성하는 triangle 정보
	opcodeface_to_dface_and_blackmesh.SetSize(0);

	// radiosity triangles
	for( int f=0; f < numfaces; ++f )
	{
		AvaFace* face = &s_pVRadDll2->FaceArray_[f];

		if( face->DispIndex_ != -1 )
			continue;

		// emissive face의 collision을 만들지 않아요~ 2008. 1. 10 changmin
		if ( s_pVRadDll2->TextureInfoArray_[face->TextureInfoIndex_].Flags_ & (SURF_SKIPCOLLISION) )
			continue;

		for( int iTri = 2; iTri < face->NumVertices_ ; ++iTri )
		{
			IndexedTriangle Tri;
			Tri.mVRef[0] = face->VertexIndices_[0];
			Tri.mVRef[1] = face->VertexIndices_[iTri -1];
			Tri.mVRef[2] = face->VertexIndices_[iTri];
			Triangles.push_back( Tri );

			// 1 ~ numfaces
			opcodeface_to_dface_and_blackmesh.AddToTail( f + 1 );

			ShadowCasterTriangles.push_back( Tri );
		}
	}
	
	// create static meshes
	const int staticmesh_count = s_pVRadDll2->NumBlackMeshTriangleCounts_;
	const int staticmesh_trianglebase = (int) Triangles.size();

	// compute static mesh's triangle normals
	if( staticmesh_count > 0 )
	{
		const int all_staticmesh_triangle_counts = s_pVRadDll2->BlackMeshTriangleCountArray_[staticmesh_count-1];
		staticmesh_triangle_normals.SetSize( all_staticmesh_triangle_counts );
		memset( staticmesh_triangle_normals.Base(), 0, sizeof( Vector ) * all_staticmesh_triangle_counts );
		for( int triangleIndex = 0; triangleIndex < all_staticmesh_triangle_counts; ++triangleIndex  )
		{
			const int vi0 = s_pVRadDll2->BlackMeshIndexArray_[triangleIndex * 3 + 0];
			const int vi1 = s_pVRadDll2->BlackMeshIndexArray_[triangleIndex * 3 + 1];
			const int vi2 = s_pVRadDll2->BlackMeshIndexArray_[triangleIndex * 3 + 2];

			const Vector &v0 = s_pVRadDll2->BlackMeshVertexArray_[vi0].Position_;
			const Vector &v1 = s_pVRadDll2->BlackMeshVertexArray_[vi1].Position_;
			const Vector &v2 = s_pVRadDll2->BlackMeshVertexArray_[vi2].Position_;

			Vector e1;
			Vector e2;
			VectorSubtract( v1, v0, e1 );
			VectorSubtract( v2, v0, e2 );
			CrossProduct( e1, e2, staticmesh_triangle_normals[triangleIndex] );
			VectorNormalize( staticmesh_triangle_normals[triangleIndex] );
		}

		// alloc visibility data
		sun_visibility_array.SetSize( staticmesh_count);
		memset(sun_visibility_array.Base(), 0, sizeof(int) * staticmesh_count );
	}
	
	
	// create static mehses
	static_meshes.empty();
	if( staticmesh_count > 0 )
	{
		AVA_StaticMesh staticmesh(
			0,
			0,
			s_pVRadDll2->BlackMeshTriangleCountArray_[0],
			0,
			s_pVRadDll2->BlackMeshVertexCountArray_[0],
			s_pVRadDll2->BlackMeshSampleVerticesFlagArray_[0],
			s_pVRadDll2->BlackMeshSampleToAreaRatioArray_[0] );
		static_meshes.push_back( staticmesh );
	}


	for( int staticmesh_id = 1; staticmesh_id < staticmesh_count; ++staticmesh_id )
	{
		const int triangle_base		= s_pVRadDll2->BlackMeshTriangleCountArray_[staticmesh_id-1];
		const int triangle_count	= s_pVRadDll2->BlackMeshTriangleCountArray_[staticmesh_id] - triangle_base;
		const int vertex_base		= s_pVRadDll2->BlackMeshVertexCountArray_[staticmesh_id-1];
		const int vertex_count		= s_pVRadDll2->BlackMeshVertexCountArray_[staticmesh_id] - vertex_base;
		AVA_StaticMesh staticmesh(
			staticmesh_id,
			triangle_base * 3,
			triangle_count,
			vertex_base,
			vertex_count,
			s_pVRadDll2->BlackMeshSampleVerticesFlagArray_[staticmesh_id],
			s_pVRadDll2->BlackMeshSampleToAreaRatioArray_[staticmesh_id] );
		static_meshes.push_back( staticmesh );
	}
	// create collision triangles from static mesh
	int meshid = 0;
	triangle_to_meshid.SetSize(0);
	for( int f = 0; f < s_pVRadDll2->NumBlackMeshIndices_ / 3; ++f )
	{
		IndexedTriangle Tri;
		Tri.mVRef[0] = numvertexes + s_pVRadDll2->BlackMeshIndexArray_[f * 3 + 0];
		Tri.mVRef[1] = numvertexes + s_pVRadDll2->BlackMeshIndexArray_[f * 3 + 1];
		Tri.mVRef[2] = numvertexes + s_pVRadDll2->BlackMeshIndexArray_[f * 3 + 2];
		Triangles.push_back(Tri);

		// ( -1 ~ - blackTriangleCount )
		opcodeface_to_dface_and_blackmesh.AddToTail( - ( f + 1 ) );

		int meshtrianglecount = s_pVRadDll2->BlackMeshTriangleCountArray_[meshid];
		if( f >= meshtrianglecount )	// f는 zero base
		{
			++meshid;
		}
		assert( meshid < s_pVRadDll2->NumBlackMeshTriangleCounts_  );
		triangle_to_meshid.AddToTail( meshid );

		if( AVA_StaticMeshManager::GetStaticMesh( meshid ).DoCastShadow() )
		{
			ShadowCasterTriangles.push_back( Tri );
		}
	}

	// offsetting displacement triangle's vertexindex
	std::vector<IndexedTriangle>::iterator it;
	for( it = DispTriangles.begin(); it != DispTriangles.end(); ++it )
	{
		it->mVRef[0] += numvertexes + num_blackmesh_vertexes;
		it->mVRef[1] += numvertexes + num_blackmesh_vertexes;
		it->mVRef[2] += numvertexes + num_blackmesh_vertexes;
	}

	// append disp triangles
	Triangles.insert( Triangles.end(), DispTriangles.begin(), DispTriangles.end() );

	// set opcode tri to disp face map
	std::vector<int>::const_iterator dispFaceIt = DispParentFaceIndices.begin();
	for( ; dispFaceIt != DispParentFaceIndices.end(); ++dispFaceIt )
	{
		// face index는 +1 값으로 encoding한다...
		opcodeface_to_dface_and_blackmesh.AddToTail( (*dispFaceIt) + 1 );
	}

	// create opcode model..
	const unsigned int NumModelTriangles = Triangles.size();
	const unsigned int NumModelVertices = Vertexes.size();
	if( NumModelTriangles > 0 )
	{
		Msg("Building World Collision(1/1)...\n");
		static Opcode::MeshInterface Mesh;
		Mesh.SetNbTriangles( NumModelTriangles );
		Mesh.SetNbVertices( NumModelVertices );
		Mesh.SetPointers( &Triangles[0], &Vertexes[0] );
		Opcode::OPCODECREATE Opcc;
		Opcc.mIMesh = &Mesh;
		Opcc.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
		Opcc.mSettings.mLimit = 1;
		Opcc.mNoLeaf = true;
		Opcc.mQuantized = true;
		Opcc.mKeepOriginal = false;
		Opcc.mCanRemap = true;
		s_WorldModel.Build( Opcc );
	}

	// create shadow caster model
	const unsigned int NumShadowCasterTriangles = ShadowCasterTriangles.size();
	if( NumShadowCasterTriangles > 0 )
	{
		Msg("Build World Shadow Caster Collision(1/1)...\n");
		static Opcode::MeshInterface Mesh2;
		Mesh2.SetNbTriangles( NumShadowCasterTriangles );
		Mesh2.SetNbVertices( NumModelVertices );
		Mesh2.SetPointers( &ShadowCasterTriangles[0], &Vertexes[0] );
		Opcode::OPCODECREATE Opcc;
		Opcc.mIMesh = &Mesh2;
		Opcc.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
		Opcc.mSettings.mLimit = 1;
		Opcc.mNoLeaf = true;
		Opcc.mQuantized = true;
		Opcc.mKeepOriginal = false;
		Opcc.mCanRemap = true;
		s_WorldShadowCasterModel.Build( Opcc );
	}

	// create static mesh collision models
	for( int staticmeshIndex = 0; staticmeshIndex < AVA_StaticMeshManager::GetNumMeshes(); ++staticmeshIndex )
	{
		Msg("Building Static Mesh Collision(%d/%d)...\n", staticmeshIndex+1, AVA_StaticMeshManager::GetNumMeshes() );
		const AVA_StaticMesh &staticmesh = AVA_StaticMeshManager::GetStaticMesh(staticmeshIndex);
		s_Mesh[staticmeshIndex].SetNbTriangles( staticmesh.GetNumTrisnagles() );
		// triangle index는 전체 vertex buffer의 index를 가지고 있다. 그래서 전체 model vertex buffer를 걸어준다.
		s_Mesh[staticmeshIndex].SetNbVertices( NumModelVertices );
		s_Mesh[staticmeshIndex].SetPointers( &Triangles[ staticmesh_trianglebase + staticmesh.GetStartTriangleIndex()],
						  &Vertexes[0] );

		Opcode::OPCODECREATE Opcc;
		Opcc.mIMesh = &s_Mesh[staticmeshIndex];
		Opcc.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
		Opcc.mSettings.mLimit = 1;
		Opcc.mNoLeaf = true;
		Opcc.mQuantized = true;
		Opcc.mKeepOriginal = false;
		Opcc.mCanRemap = true;
		s_WorldModels[staticmeshIndex].Build( Opcc );
	}

	// create bsp models
	if( staticmesh_trianglebase > 0 )
	{
		Msg("Building Bsp Collision(1/1)...\n");
		const int bspIndex = AVA_StaticMeshManager::GetNumMeshes();
		s_Mesh[bspIndex].SetNbTriangles( staticmesh_trianglebase );
		s_Mesh[bspIndex].SetNbVertices( numvertexes );
		s_Mesh[bspIndex].SetPointers( &Triangles[0], &Vertexes[0] );
		Opcode::OPCODECREATE Opcc;
		Opcc.mIMesh = &s_Mesh[bspIndex];
		Opcc.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
		Opcc.mSettings.mLimit = 1;
		Opcc.mNoLeaf = true;
		Opcc.mQuantized = true;
		Opcc.mKeepOriginal = false;
		Opcc.mCanRemap = true;
		s_WorldModels[bspIndex].Build( Opcc );
	}

	// to do terrain collision

};

int TestLineWithOpcode( Vector const& start, Vector const& stop )
{
	Vector dir = stop - start;
	float RayLength = dir.Length();
	VectorNormalize( dir );
	Ray ray( Point(  start.x, start.y, start.z ), Point( dir.x, dir.y, dir.z ) );

	static udword Cache;
	Opcode::RayCollider* pRC = (Opcode::RayCollider*)TlsGetValue( RC_TLS_INDEX );
	if (!pRC)
	{
		pRC = _RayCollidersInstance.Allocate();
	}
	Opcode::RayCollider s_RC = *pRC;
	s_RC.SetFirstContact( true  );
	s_RC.SetHitCallback( NULL );
	s_RC.SetTemporalCoherence( true );
	s_RC.SetCulling( false );	
	s_RC.SetMaxDist( RayLength );

	//s_RC.Collide( ray, s_WorldModel, NULL, &Cache );
	s_RC.Collide( ray, s_WorldShadowCasterModel, NULL, &Cache );
	BOOL IsContact = s_RC.GetContactStatus();

	if( IsContact )
	{
		return CONTENTS_SOLID;
	}
	else
	{
		return CONTENTS_EMPTY;
	}
}


int TestLineWithSkipByOpcode( Vector const &start, Vector const &stop )
{
	static udword Cache;
	Opcode::RayCollider* pRC = (Opcode::RayCollider*)TlsGetValue( RC_TLS_INDEX );
	if (!pRC)
	{
		pRC = _RayCollidersInstance.Allocate();
	}
	Opcode::RayCollider s_RC = *pRC;	
	Vector dir = stop - start;
	float RayLength = dir.Length();
	VectorNormalize( dir );
	Ray ray( Point(  start.x, start.y, start.z ), Point( dir.x, dir.y, dir.z ) );

	s_RC.SetFirstContact( true  );
	s_RC.SetHitCallback( NULL );
	s_RC.SetUserData(NULL);
	s_RC.SetTemporalCoherence( false );
	s_RC.SetCulling( false );	
	s_RC.SetMaxDist( RayLength );

	// check collision with bsp model
	s_RC.Collide( ray, s_WorldModels[AVA_StaticMeshManager::GetNumMeshes()], NULL, &Cache );
	if( s_RC.GetContactStatus() )
	{
		return CONTENTS_SOLID;
	}

	// check collision with static meshes
	for( int staticmeshIndex = 0; staticmeshIndex < AVA_StaticMeshManager::GetNumMeshes(); ++staticmeshIndex )
	{
		if(	staticmeshIndex != skip_id
		&&	AVA_StaticMeshManager::GetStaticMesh(staticmeshIndex).DoCastShadow() )
		{
			s_RC.Collide( ray, s_WorldModels[staticmeshIndex], NULL, &Cache );
			if( s_RC.GetContactStatus() )
			{
				return CONTENTS_SOLID;
			}
		}
	}

	// no collision
	return CONTENTS_EMPTY;
}

Vector GetSampleFromHit( const Opcode::CollisionFace& hit, bool isAmbientSample )
{
	Vector sample_color( 0.0f, 0.0f, 0.0f );

	const bool IsBlackMesh = ( opcodeface_to_dface_and_blackmesh[hit.mFaceID] < 0 );
	const bool IsFace = !IsBlackMesh;

	if( IsBlackMesh )
	{
		const int face_index = (-opcodeface_to_dface_and_blackmesh[hit.mFaceID]) - 1;

		// 전등갓등을 제외하기 위한 수단.
		if( AVA_StaticMeshManager::GetStaticMesh(triangle_to_meshid[face_index]).HasEmissive() )
		{
			return sample_color;
		}

		int v0_index = s_pVRadDll2->BlackMeshIndexArray_[face_index * 3 + 0];
		int v1_index = s_pVRadDll2->BlackMeshIndexArray_[face_index * 3 + 1];
		int v2_index = s_pVRadDll2->BlackMeshIndexArray_[face_index * 3 + 2];

		// normal 방향으로 들어오는 빛을 sampling한다
		// 4 = bump 방향들..
		const Vector& vertex0_light = blackmesh_vertexlight_float_vector_data[v0_index * 4];	
		const Vector& vertex1_light = blackmesh_vertexlight_float_vector_data[v1_index * 4];
		const Vector& vertex2_light = blackmesh_vertexlight_float_vector_data[v2_index * 4];

		VectorMA( sample_color, (1.0f - hit.mU - hit.mV), vertex0_light, sample_color );
		VectorMA( sample_color, hit.mU, vertex1_light, sample_color );
		VectorMA( sample_color, hit.mV, vertex2_light, sample_color );

		const int texDataIndex = s_pVRadDll2->BlackMeshTexDataIndexArray_[face_index];
		dtexdata_t* td = &dtexdata[texDataIndex];
		sample_color.x *= td->reflectivity.x;
		sample_color.y *= td->reflectivity.y;
		sample_color.z *= td->reflectivity.z;

		return sample_color;
	}
	else if( IsFace )
	{
		// compute hit point
		// opcode geometry space
		IndexedTriangle& Tri = Triangles[hit.mFaceID];

		Point& Point0 = Vertexes[Tri.mVRef[0]];
		Point& Point1 = Vertexes[Tri.mVRef[1]];
		Point& Point2 = Vertexes[Tri.mVRef[2]];
		Point HitPos = ( 1.0f - hit.mU - hit.mV ) * Point0 + hit.mU * Point1 + hit.mV * Point2; // pbrt 참조했음. 정확~

		// compute lightmap coord
		// vrad geomtry space
		int face_index = opcodeface_to_dface_and_blackmesh[hit.mFaceID] - 1;
		dface_t* face = &dfaces[face_index];
		// do not sample from no light or sky
		texinfo_t *tf = &texinfo[face->texinfo];
		if( tf->flags & TEX_SPECIAL )
		{
			return sample_color;
		}

		// sample luxel
		Vector hit_pt( HitPos.x, HitPos.y, HitPos.z );

		lightinfo_t hitface_lightInfo;
		InitLightinfo( &hitface_lightInfo, face_index );

		Vector2D luxelCoord;
		WorldToLuxelSpace( &hitface_lightInfo, hit_pt, luxelCoord );

		float s = luxelCoord.x;
		float t = luxelCoord.y;

		s = max( 0, s );
		t = max( 0, t );
		s = min( face->m_LightmapTextureSizeInLuxels[0], s );
		t = min( face->m_LightmapTextureSizeInLuxels[1], t );

		// compute lightmap color
		int smax = face->m_LightmapTextureSizeInLuxels[0] + 1;

		colorRGBExp32* lightmap = (colorRGBExp32*)&dlightdata[face->lightofs];
		lightmap += ( (int)t * smax + (int)s ) ;

		sample_color.x = TexLightToLinear( lightmap->r, lightmap->exponent );
		sample_color.y = TexLightToLinear( lightmap->g, lightmap->exponent );
		sample_color.z = TexLightToLinear( lightmap->b, lightmap->exponent );

		texinfo_t *tex = &texinfo[face->texinfo];
		dtexdata_t* td = &dtexdata[tex->texdata];

		sample_color.x *= td->reflectivity.x;
		sample_color.y *= td->reflectivity.y;
		sample_color.z *= td->reflectivity.z;

		if( !isAmbientSample )
		{
			sample_color += td->brightness;
		}
		return sample_color;
	}

	return sample_color;
}


inline colorRGBExp32* dface_AvgLightColor( dface_t *pFace, int nLightStyleIndex ) 
{ 
	return (colorRGBExp32*)&(bouncedlightdata)[pFace->lightofs - (nLightStyleIndex+1) * 4];
}

Vector GetDiffuseReflectionFromHit( const Opcode::CollisionFace& hit, bool isAmbientSample )
{
	Vector sample_color( 0.0f, 0.0f, 0.0f );

	const bool IsBlackMesh = ( opcodeface_to_dface_and_blackmesh[hit.mFaceID] < 0 );
	const bool IsFace = !IsBlackMesh;

	if( IsBlackMesh )
	{
		const int face_index = (-opcodeface_to_dface_and_blackmesh[hit.mFaceID]) - 1;

		// 전등갓등을 제외하기 위한 수단.
		if( AVA_StaticMeshManager::GetStaticMesh(triangle_to_meshid[face_index]).HasEmissive() )
		{
			return sample_color;
		}

		int v0_index = s_pVRadDll2->BlackMeshIndexArray_[face_index * 3 + 0];
		int v1_index = s_pVRadDll2->BlackMeshIndexArray_[face_index * 3 + 1];
		int v2_index = s_pVRadDll2->BlackMeshIndexArray_[face_index * 3 + 2];

		// normal 방향으로 들어오는 빛을 sampling한다
		// 4 = bump 방향들..
		const Vector& vertex0_light = blackmesh_bouncing_data[v0_index];	
		const Vector& vertex1_light = blackmesh_bouncing_data[v1_index];
		const Vector& vertex2_light = blackmesh_bouncing_data[v2_index];

		VectorMA( sample_color, (1.0f - hit.mU - hit.mV), vertex0_light, sample_color );
		VectorMA( sample_color, hit.mU, vertex1_light, sample_color );
		VectorMA( sample_color, hit.mV, vertex2_light, sample_color );

		const int texDataIndex = s_pVRadDll2->BlackMeshTexDataIndexArray_[face_index];
		dtexdata_t* td = &dtexdata[texDataIndex];
		sample_color.x *= td->reflectivity.x;
		sample_color.y *= td->reflectivity.y;
		sample_color.z *= td->reflectivity.z;
		return sample_color;
	}
	else if( IsFace )
	{
		// compute hit point
		// opcode geometry space
		IndexedTriangle& Tri = Triangles[hit.mFaceID];

		Point& Point0 = Vertexes[Tri.mVRef[0]];
		Point& Point1 = Vertexes[Tri.mVRef[1]];
		Point& Point2 = Vertexes[Tri.mVRef[2]];
		Point HitPos = ( 1.0f - hit.mU - hit.mV ) * Point0 + hit.mU * Point1 + hit.mV * Point2;	// pbrt 참조

		// compute lightmap coord
		// vrad geomtry space
		int face_index = opcodeface_to_dface_and_blackmesh[hit.mFaceID] - 1;
		dface_t* face = &dfaces[face_index];
		// do not sample from no light or sky
		texinfo_t *tf = &texinfo[face->texinfo];
		if( tf->flags & TEX_SPECIAL )
		{
			return sample_color;
		}

		// sample luxels
		Vector hit_pt( HitPos.x, HitPos.y, HitPos.z );

		lightinfo_t hitface_lightInfo;
		InitLightinfo( &hitface_lightInfo, face_index );

		Vector2D luxelCoord;
		WorldToLuxelSpace( &hitface_lightInfo, hit_pt,luxelCoord );

		float s = luxelCoord.x;
		float t = luxelCoord.y;

		s = max( 0, s );
		t = max( 0, t );
		s = min( face->m_LightmapTextureSizeInLuxels[0], s );
		t = min( face->m_LightmapTextureSizeInLuxels[1], t );

		// compute lightmap color
		int smax = face->m_LightmapTextureSizeInLuxels[0] + 1;

		colorRGBExp32* lightmap = (colorRGBExp32*)&bouncedlightdata[face->lightofs];
		lightmap += ( (int)t * smax + (int)s ) ;

		sample_color.x = TexLightToLinear( lightmap->r, lightmap->exponent );
		sample_color.y = TexLightToLinear( lightmap->g, lightmap->exponent );
		sample_color.z = TexLightToLinear( lightmap->b, lightmap->exponent );

		texinfo_t* tex = &texinfo[face->texinfo];
		dtexdata_t* td = &dtexdata[tex->texdata];

		sample_color.x *= td->reflectivity.x;
		sample_color.y *= td->reflectivity.y;
		sample_color.z *= td->reflectivity.z;

		if( !isAmbientSample )
		{
			texinfo_t* tex = &texinfo[face->texinfo];
			dtexdata_t* td = &dtexdata[tex->texdata];
			sample_color += td->brightness;
		}
		return sample_color;
	}

	return sample_color;
}


typedef bool (*FrontHitCallback)(udword faceid, void *user_data );

bool IsFrontHit(udword faceid, void *user_data )
{
	Vector *ray = (Vector*) user_data;
	Vector *face_normal = NULL;
	const bool IsBlackMesh = ( opcodeface_to_dface_and_blackmesh[faceid] < 0 );
	if( IsBlackMesh )
	{
		const int face_index = (-opcodeface_to_dface_and_blackmesh[faceid]) - 1;
		face_normal = &staticmesh_triangle_normals[face_index];
	}
	else
	{
		const int face_index = opcodeface_to_dface_and_blackmesh[faceid] - 1;
		face_normal = &dplanes[dfaces[face_index].planenum].normal;
	}
	return DotProduct( *face_normal, *ray ) < 0.0f;
}

struct AvaTrace
{
	struct HitData
	{
		Opcode::CollisionFace	*Closest;
		bool					IsFrontFace;
		FrontHitCallback		IsFrontHitCallback;
		void*					UserData;
	};

	static void ClosestContactWithoutSelfOcclusion( const Opcode::CollisionFace& hit, void* user_data)
	{
		HitData *hitdata = (HitData*) user_data;
		const bool IsBlackMesh = ( opcodeface_to_dface_and_blackmesh[hit.mFaceID] < 0 );
		if( IsBlackMesh )
		{
			const int face_index = (-opcodeface_to_dface_and_blackmesh[hit.mFaceID]) - 1;
			const int meshid = triangle_to_meshid[face_index];
			if(	skip_id != meshid
			&&	AVA_StaticMeshManager::GetStaticMesh(meshid).DoCastShadow() )
			{
				if( hit.mDistance < hitdata->Closest->mDistance )
				{
					*(hitdata->Closest)		= hit;
					hitdata->IsFrontFace	= hitdata->IsFrontHitCallback( hit.mFaceID, hitdata->UserData );
					contact_id				= meshid;
				}
			}
		}
		else
		{
			if( hit.mDistance < hitdata->Closest->mDistance )
			{
				*(hitdata->Closest)		= hit;
				hitdata->IsFrontFace	= hitdata->IsFrontHitCallback( hit.mFaceID, hitdata->UserData );
				contact_id				= 0; // bsp
			}
		}
	}

	static void ClosestContact( const Opcode::CollisionFace& hit, void* user_data)
	{
		// self shaodw가 켜져 있는 상황
		HitData *hitdata = (HitData*) user_data;
		const bool IsBlackMesh = ( opcodeface_to_dface_and_blackmesh[hit.mFaceID] < 0 );
		if( IsBlackMesh )
		{
			const int face_index = (-opcodeface_to_dface_and_blackmesh[hit.mFaceID]) - 1;
			const int meshid = triangle_to_meshid[face_index];
			if(	(meshid != self_id && AVA_StaticMeshManager::GetStaticMesh(meshid).DoCastShadow())
			||	(self_id > 0 && meshid == self_id) )
			{
				if( hit.mDistance < hitdata->Closest->mDistance )
				{
					*(hitdata->Closest)		= hit;
					hitdata->IsFrontFace	= hitdata->IsFrontHitCallback( hit.mFaceID, hitdata->UserData );
					contact_id				= meshid;
				}
			}
		}
		else
		{
			if( hit.mDistance < hitdata->Closest->mDistance )
			{
				*(hitdata->Closest)		= hit;
				hitdata->IsFrontFace	= hitdata->IsFrontHitCallback( hit.mFaceID, hitdata->UserData );
			}
		}
	}
};


bool SampleAmbientWithOpcode( Vector const& start, Vector const& stop, Vector* sample_color )
{
	static udword Cache;

	// get coliider
	Opcode::RayCollider* pRC = (Opcode::RayCollider*)TlsGetValue( RC_TLS_INDEX );
	if (!pRC)
	{
		pRC = _RayCollidersInstance.Allocate();
	}
	Opcode::RayCollider s_RC = *pRC;

	// create trace ray
	Vector dir = stop - start;
	float RayLength = VectorNormalize( dir );
	Ray ray( Point(  start.x, start.y, start.z ), Point( dir.x, dir.y, dir.z ) );

	// initialize trace data and context
	Opcode::CollisionFace	closest_contact;
	closest_contact.mDistance = MAX_FLOAT;

	AvaTrace::HitData hit_data;
	hit_data.Closest			= &closest_contact;
	hit_data.IsFrontFace		= false;
	hit_data.IsFrontHitCallback = IsFrontHit;
	hit_data.UserData			= (void*) &dir;

	s_RC.SetMaxDist( RayLength );
	s_RC.SetFirstContact(false);
	s_RC.SetCulling(false);
	s_RC.SetUserData( &hit_data );
	if( skip_id == -1 )
	{
		s_RC.SetHitCallback( AvaTrace::ClosestContact );
	}
	else
	{
		s_RC.SetHitCallback( AvaTrace::ClosestContactWithoutSelfOcclusion );
		contact_id = -1;
	}

	// trace
	// world를 trace해야 triangle id -> face id가 일치합니다.
	s_RC.Collide( ray, s_WorldModel, NULL, &Cache );

	// get result
	BOOL IsContact = false;
	if( skip_id == -1 )
	{
		IsContact = s_RC.GetContactStatus() && (hit_data.IsFrontFace == true);
	}
	else
	{
		IsContact = s_RC.GetContactStatus() && (hit_data.IsFrontFace == true) && ( contact_id >= 0 );
	}
	if( IsContact )
	{
		*sample_color = GetSampleFromHit( closest_contact, true );
		return true;
	}
	else
	{
		*sample_color = Vector( 0.0f, 0.0f, 0.0f );
		return false;
	}
}


bool SampleDiffuseReflectionWithOpcode( Vector const& start, Vector const& stop, Vector* sample_color )
{
	static udword Cache;

	// get coliider
	Opcode::RayCollider* pRC = (Opcode::RayCollider*)TlsGetValue( RC_TLS_INDEX );
	if (!pRC)
	{
		pRC = _RayCollidersInstance.Allocate();
	}
	Opcode::RayCollider s_RC = *pRC;

	// create trace ray
	Vector dir = stop - start;
	float RayLength = VectorNormalize( dir );
	Ray ray( Point(  start.x, start.y, start.z ), Point( dir.x, dir.y, dir.z ) );

	// initialize trace data and context
	Opcode::CollisionFace	closest_contact;
	closest_contact.mDistance = MAX_FLOAT;

	AvaTrace::HitData hit_data;
	hit_data.Closest			= &closest_contact;
	hit_data.IsFrontFace		= false;
	hit_data.IsFrontHitCallback = IsFrontHit;
	hit_data.UserData			= (void*) &dir;

	s_RC.SetMaxDist( RayLength );
	s_RC.SetFirstContact(false);
	s_RC.SetCulling(false);
	s_RC.SetUserData( &hit_data );
	if( skip_id == -1 )
	{
		s_RC.SetHitCallback( AvaTrace::ClosestContact );
	}
	else
	{
		s_RC.SetHitCallback( AvaTrace::ClosestContactWithoutSelfOcclusion );
		contact_id = -1;
	}

	// trace
	// world를 trace해야 triangle id -> face id가 일치합니다.
	s_RC.Collide( ray, s_WorldModel, NULL, &Cache );

	// get result
	BOOL IsContact = false;
	if( skip_id == -1 )
	{
		IsContact = s_RC.GetContactStatus() && (hit_data.IsFrontFace == true);
	}
	else
	{
		IsContact = s_RC.GetContactStatus() && (hit_data.IsFrontFace == true) && ( contact_id >= 0 );
	}
	if( IsContact )
	{
		*sample_color = GetDiffuseReflectionFromHit( closest_contact, true );
		return true;
	}
	else
	{
		*sample_color = Vector( 0.0f, 0.0f, 0.0f );
		return false;
	}
}

void InitOpcode()
{
	Opcode::InitOpcode();
}

void FinalizeOpcode()
{
	Opcode::CloseOpcode();
}

//#pragma optimize("", on)