#define _CRT_SECURE_NO_DEPRECATE

#include <atlbase.h>
#include <atlstr.h>

#include "vrad.h"
#include "lightmap.h"
#include "radial.h"
#include <bumpvects.h>
#include "utlvector.h"
//#include "vmpi.h"

#include "ivraddll.h"

#include "rgbe/rgbe.h"

#include <algorithm>
#include "SHMath.h"

int GSHSunTblSize = 0;
SHVectorRGB SHSunTbl[Pbrt::MAX_SUN_SAMPLES];
int SHSunTblEnvIndex[Pbrt::MAX_SUN_SAMPLES];

#define INV_TWOPI	0.15915494309189533577f
#define INV_PI		0.31830988618379067154f

Vector		GSunDirection;
LinearColor	GSunColor;
bool		gSunVisible;

//#pragma optimize("", off)
/**
 * BuildVertexLights
 */

extern int spy_info[4][3];

#define MAX_SAMPLES_PER_TRIANGLE 32
float RandomFloat();
void AVA_BuildVertexLights( int iThread, int meshid )
{
	int total_sample_count = 0;
	const AVA_StaticMesh &staticmesh = AVA_StaticMeshManager::GetStaticMesh( meshid );

	// set collision context
	if( staticmesh.NeedsSelfShadow() )
	{
		skip_id = -1;
	}
	else
	{
		skip_id = meshid;	// skip self
	}

	gSunVisible = false;

	if( staticmesh.NeedsMultisample() )
	{
		float s[MAX_SAMPLES_PER_TRIANGLE];
		float t[MAX_SAMPLES_PER_TRIANGLE];
		float u[MAX_SAMPLES_PER_TRIANGLE];
		for( int sampleIndex = 0; sampleIndex < MAX_SAMPLES_PER_TRIANGLE; ++sampleIndex )
		{
			float u1 = RandomFloat();
			float u2 = RandomFloat();
			const float su1 = sqrtf(u1);
			s[sampleIndex] = 1.0f - su1;
			t[sampleIndex] = u2 * su1;
			u[sampleIndex] = 1.0f - s[sampleIndex] - t[sampleIndex];
		}

		
		for( int triangleid = 0; triangleid < staticmesh.GetNumTrisnagles(); ++triangleid )
		{
			// get triangle data
			int v_index[3];
			staticmesh.GetTriangleIndices( triangleid, &v_index[0], &v_index[1], &v_index[2] );

			Vector triangle_vertices[3];
			Vector vertex_normals[3];
			Vector vertex_tangent_s[3];
			Vector vertex_tangent_t[3];
			for( int i = 0; i < 3; ++i)
			{
				triangle_vertices[i]	= staticmesh.GetVertex( v_index[i] );
				vertex_normals[i]		= staticmesh.GetVertexNormal( v_index[i] );
				vertex_tangent_s[i]		= staticmesh.GetTangentS( v_index[i] );
				vertex_tangent_t[i]		= staticmesh.GetTangentT( v_index[i] );
			}
			
			// compute triangle area and sample count
			Vector e1;
			Vector e2;
			NVector triangle_normal;
			VectorSubtract( triangle_vertices[1], triangle_vertices[0], e1 );
			VectorSubtract( triangle_vertices[2], triangle_vertices[0], e2 );
			CrossProduct( e1, e2, triangle_normal );
			const float triangle_area = VectorNormalize( triangle_normal ) * 0.5f;
			triangle_normal.getIndex();

			const int num_samples = clamp(Float2Int(triangle_area * staticmesh.GetSampleToAreaRatio()), 0, MAX_SAMPLES_PER_TRIANGLE );
			total_sample_count += num_samples;

			// generate sample and sampling light
			for( int sample_index = 0; sample_index < num_samples; ++sample_index )
			{
				Vector sample_pos(0.0f, 0.0f, 0.0f);
				VectorMA( sample_pos, s[sample_index], triangle_vertices[0], sample_pos );
				VectorMA( sample_pos, t[sample_index], triangle_vertices[1], sample_pos );
				VectorMA( sample_pos, u[sample_index], triangle_vertices[2], sample_pos );

				// interpolate vertex normal
				Vector sample_normal(0.0f, 0.0f, 0.0f);
				VectorMA( sample_normal, s[sample_index], vertex_normals[0], sample_normal );
				VectorMA( sample_normal, t[sample_index], vertex_normals[1], sample_normal );
				VectorMA( sample_normal, u[sample_index], vertex_normals[2], sample_normal );
				VectorNormalize( sample_normal );
				NVector tangent_z = sample_normal;
				tangent_z.getIndex();

				Vector sample_tangent_s(0.0f, 0.0f, 0.0f);
				VectorMA( sample_tangent_s, s[sample_index], vertex_tangent_s[0], sample_tangent_s );
				VectorMA( sample_tangent_s, t[sample_index], vertex_tangent_s[1], sample_tangent_s );
				VectorMA( sample_tangent_s, u[sample_index], vertex_tangent_s[2], sample_tangent_s );

				Vector sample_tangent_t(0.0f, 0.0f, 0.0f);
				VectorMA( sample_tangent_t, s[sample_index], vertex_tangent_t[0], sample_tangent_t );
				VectorMA( sample_tangent_t, t[sample_index], vertex_tangent_t[1], sample_tangent_t );
				VectorMA( sample_tangent_t, u[sample_index], vertex_tangent_t[2], sample_tangent_t );

				NVector world_bump_basis[3];
				GetBumpNormals( sample_tangent_s, sample_tangent_t, triangle_normal, sample_normal, world_bump_basis );
								
				// compute lighting
				// Iterate over all direct lights and sample them
				int light_count = 0;
				for( directlight_t* dl = activelights; dl != NULL; dl = dl->next )
				{
					Vector adjusted_pos = sample_pos;

					const bool bRealtimeLight = (dl->light.style == 1);

					// emit_hdrEnvmapSkylight / emit_skylight / emit_skyambient 가 아닐때
					if( !VectorCompare( dl->light.origin, vec3_origin ) )
					{
						Vector fudge = dl->light.origin - sample_pos;
						VectorNormalize( fudge );
						adjusted_pos += fudge;
					}

					++light_count;
					Vector light_vec;
					float falloff = 1.0f;
					float	bump_dots[3];
					Vector	skyIrradiance[4];
					Vector	skyIrradiance2[4];

					memset( skyIrradiance, 0, sizeof( Vector ) * 4 );
					memset( skyIrradiance2, 0, sizeof( Vector ) * 4 );

					float dot = 0.0f;
					if( bRealtimeLight )
					{
						dot = GatherSampleLight(
							dl,	// light
							-2,	// -2 는 vertex lighting facenumber 내부에서 사용안함.
							adjusted_pos,
							tangent_z,
							light_vec,
							&falloff,
							iThread, world_bump_basis, bump_dots , skyIrradiance,
							skyIrradiance2
							);
					}
					else
					{
						dot = GatherSampleLight(
							dl,	// light
							-2,	// -2 는 vertex lighting facenumber 내부에서 사용안함.
							adjusted_pos,
							tangent_z,
							light_vec,
							&falloff,
							iThread, world_bump_basis, bump_dots , skyIrradiance
							);
					}
					if( dot <= 0 )
					{
						continue;
					}

					const float vertex_weights[3] = {s[sample_index], t[sample_index], u[sample_index] };
					for( int i = 0; i < 3; ++i )
					{
						Vector* vertexlight = &blackmesh_vertexlight_float_vector_data[ v_index[i] * 4 ];
						Vector* vertexlight2 = &blackmesh_realtime_vertexlighting[ v_index[i] * 4 ];

						// Compute the contributions to each of the bumped lightmaps
						// The first sample is for non-bumped lighting.
						// The other sample are for bumpmapping.
						if( dl->light.type == emit_hdrEnvmapSkylight )
						{
							VectorMA( vertexlight[0], vertex_weights[i], skyIrradiance[0], vertexlight[0] );
							if( bRealtimeLight )
							{
								// ( sun + sky ) - sky
								VectorSubtract( skyIrradiance[0], skyIrradiance2[0], skyIrradiance2[0] );
								skyIrradiance2[0].x = (skyIrradiance2[0].x > 0.0f) ? skyIrradiance2[0].x : 0.0f;
								skyIrradiance2[0].y = (skyIrradiance2[0].y > 0.0f) ? skyIrradiance2[0].y : 0.0f;
								skyIrradiance2[0].z = (skyIrradiance2[0].z > 0.0f) ? skyIrradiance2[0].z : 0.0f;
								VectorMA( vertexlight2[0], vertex_weights[i], skyIrradiance2[0], vertexlight2[0]);
							}
						}
						else
						{
							VectorMA( vertexlight[0], falloff * dot * vertex_weights[i], dl->light.intensity, vertexlight[0] );
							if( bRealtimeLight )
							{
								VectorMA( vertexlight2[0], falloff * dot * vertex_weights[i], dl->light.intensity, vertexlight2[0] );
							}
						}			
						// bump case
						for( int bumpIndex = 0; bumpIndex < NUM_BUMP_VECTS; ++bumpIndex )
						{
							const int n = bumpIndex+1;
							if( dl->light.type == emit_hdrEnvmapSkylight )
							{
								VectorMA( vertexlight[bumpIndex+1], vertex_weights[i], skyIrradiance[n], vertexlight[bumpIndex+1] );
								if( bRealtimeLight )
								{
									// ( sun + sky ) - sky
									VectorSubtract( skyIrradiance[n], skyIrradiance2[n], skyIrradiance2[n] );
									skyIrradiance2[n].x = (skyIrradiance2[n].x > 0.0f) ? skyIrradiance2[n].x : 0.0f;
									skyIrradiance2[n].y = (skyIrradiance2[n].y > 0.0f) ? skyIrradiance2[n].y : 0.0f;
									skyIrradiance2[n].z = (skyIrradiance2[n].z > 0.0f) ? skyIrradiance2[n].z : 0.0f;
									VectorMA( vertexlight2[bumpIndex+1], vertex_weights[i], skyIrradiance2[n], vertexlight2[bumpIndex+1] );
								}
							}
							else
							{
								float bump_dot = (dl->light.type == emit_skyambient) ? bump_dots[n-1] : DotProduct( world_bump_basis[n-1], light_vec );
								if ( bump_dot > 0)
								{
									VectorMA( vertexlight[bumpIndex+1], falloff * bump_dot * vertex_weights[i], dl->light.intensity, vertexlight[bumpIndex+1] );
									if( bRealtimeLight )
									{
										VectorMA( vertexlight2[n], falloff * bump_dot * vertex_weights[i], dl->light.intensity, vertexlight2[n] );
									}
								}			
							}
						}
					}
				}
				const float vertex_weights[3] = {s[sample_index], t[sample_index], u[sample_index]};
				for( int i = 0; i < 3; ++i )
				{
					blackmesh_vertexlight_weight_data[v_index[i]] += vertex_weights[i];
				}
			}
		}
		
		// vertex lighting value = vertex / weight
		const int start_vertex = staticmesh.GetStartVertex();
		const int end_vertex = start_vertex + staticmesh.GetNumVertices();
		for( int vertex_index = start_vertex ; vertex_index < end_vertex; ++vertex_index)
		{
			const float weight = blackmesh_vertexlight_weight_data[vertex_index];
			if( weight < 0.001f )
			{
				blackmesh_vertexlight_float_vector_data[vertex_index * 4 + 0] = Vector( 0.0f, 0.0f, 0.0f );
				blackmesh_vertexlight_float_vector_data[vertex_index * 4 + 1] = Vector( 0.0f, 0.0f, 0.0f );
				blackmesh_vertexlight_float_vector_data[vertex_index * 4 + 2] = Vector( 0.0f, 0.0f, 0.0f );
				blackmesh_vertexlight_float_vector_data[vertex_index * 4 + 3] = Vector( 0.0f, 0.0f, 0.0f );

				blackmesh_realtime_vertexlighting[vertex_index * 4 + 0 ] = Vector( 0.0f, 0.0f ,0.0f );
				blackmesh_realtime_vertexlighting[vertex_index * 4 + 1 ] = Vector( 0.0f, 0.0f ,0.0f );
				blackmesh_realtime_vertexlighting[vertex_index * 4 + 2 ] = Vector( 0.0f, 0.0f ,0.0f );
				blackmesh_realtime_vertexlighting[vertex_index * 4 + 3 ] = Vector( 0.0f, 0.0f ,0.0f );
			}
			else
			{
				Vector *sample_value = &blackmesh_vertexlight_float_vector_data[vertex_index * 4];
				VectorScale( sample_value[0], 1.0f / weight, sample_value[0] );
				VectorScale( sample_value[1], 1.0f / weight, sample_value[1] );
				VectorScale( sample_value[2], 1.0f / weight, sample_value[2] );
				VectorScale( sample_value[3], 1.0f / weight, sample_value[3] );

				sample_value = &blackmesh_realtime_vertexlighting[vertex_index * 4];
				VectorScale( sample_value[0], 1.0f / weight, sample_value[0] );
				VectorScale( sample_value[1], 1.0f / weight, sample_value[1] );
				VectorScale( sample_value[2], 1.0f / weight, sample_value[2] );
				VectorScale( sample_value[3], 1.0f / weight, sample_value[3] );

				sun_visibility_array[meshid] = gSunVisible;
			}
		}
	}
	
	// per vertex lighting ( no multisample or smalll triangle )
	const int start_vertex	= staticmesh.GetStartVertex();
	const int end_vertex	= start_vertex + staticmesh.GetNumVertices();
	for( int v = start_vertex; v < end_vertex; ++v )
	{
		if( blackmesh_vertexlight_weight_data[v] == 0.0f )	// check multisampled
		{
			total_sample_count++;

			spy_info[iThread][2] = v;
			Vector& pos			= blackmesh_vertexes[v];
			Vector& tangent_x	= blackmesh_tangent_x[v];
			Vector& tangent_y	= blackmesh_tangent_y[v];
			NVector tangent_z	= blackmesh_tangent_z[v];
			tangent_z.getIndex();

			// offsetting sample position
			NVector world_bump_basis[3];
			GetBumpNormals( tangent_x, tangent_y, tangent_z, tangent_z, world_bump_basis );

			// Iterate over all direct lights and sample them
			for( directlight_t* dl = activelights; dl != NULL; dl = dl->next )
			{
				const bool bRealtimeLight = (dl->light.style == 1);

				Vector light_vec;
				float falloff = 1.0f;
				float	bump_dots[3];

				Vector	skyIrradiance[4];
				Vector	skyIrradiance2[4];
				memset( skyIrradiance, 0, sizeof( Vector ) * 4 );
				memset( skyIrradiance2, 0, sizeof( Vector ) * 4 );

				Vector adjusted_pos = pos;
				// emit_hdrEnvmapSkylight / emit_skylight / emit_skyambient가 아닐때
				if( !VectorCompare( dl->light.origin, vec3_origin ) )
				{
					Vector fudge = dl->light.origin - pos;
					VectorNormalize( fudge );
					adjusted_pos += fudge;
				}

				float dot = 0.0f;

				if( bRealtimeLight )
				{
					dot = GatherSampleLight(
						dl,	// light
						-2,	// -2 는 vertex lightign facenumber 내부에서 사용안함.
						adjusted_pos,
						tangent_z,
						light_vec,
						&falloff,
						iThread, world_bump_basis, bump_dots , skyIrradiance, skyIrradiance2
						);
				}
				else
				{
					dot = GatherSampleLight(
						dl,	// light
						-2,	// -2 는 vertex lightign facenumber 내부에서 사용안함.
						adjusted_pos,
						tangent_z,
						light_vec,
						&falloff,
						iThread, world_bump_basis, bump_dots , skyIrradiance
						);
				}

				if( dot <= 0 )
					continue;

				Vector* vertexlight = &blackmesh_vertexlight_float_vector_data[v*4];
				Vector* vertexlight2 = &blackmesh_realtime_vertexlighting[v*4];

				// Compute the contributions to each of the bumped lightmaps
				// The first sample is for non-bumped lighting.
				// The other sample are for bumpmapping.
				if( dl->light.type == emit_hdrEnvmapSkylight )
				{
					VectorAdd( vertexlight[0], skyIrradiance[0], vertexlight[0] );
					if( bRealtimeLight )
					{
						VectorSubtract( skyIrradiance[0], skyIrradiance2[0], skyIrradiance2[0] );
						skyIrradiance2[0].x = (skyIrradiance2[0].x > 0.0f) ? skyIrradiance2[0].x : 0.0f;
						skyIrradiance2[0].y = (skyIrradiance2[0].y > 0.0f) ? skyIrradiance2[0].y : 0.0f;
						skyIrradiance2[0].z = (skyIrradiance2[0].z > 0.0f) ? skyIrradiance2[0].z : 0.0f;
						VectorAdd( vertexlight2[0], skyIrradiance2[0], vertexlight2[0] );
					}
				}
				else
				{
					VectorMA( vertexlight[0], falloff * dot, dl->light.intensity, vertexlight[0] );
					if( bRealtimeLight )
					{
						VectorMA( vertexlight2[0], falloff * dot, dl->light.intensity, vertexlight2[0] );
					}
				}			

				for( int bumpIndex = 0; bumpIndex < NUM_BUMP_VECTS; ++bumpIndex )
				{
					const int n = bumpIndex+1;

					if( dl->light.type == emit_hdrEnvmapSkylight )
					{
						VectorAdd( vertexlight[n], skyIrradiance[n], vertexlight[n] );
						if( bRealtimeLight )
						{
							VectorSubtract( skyIrradiance[n], skyIrradiance2[n], skyIrradiance2[n] );
							skyIrradiance2[n].x = (skyIrradiance2[n].x > 0.0f) ? skyIrradiance2[n].x : 0.0f;
							skyIrradiance2[n].y = (skyIrradiance2[n].y > 0.0f) ? skyIrradiance2[n].y : 0.0f;
							skyIrradiance2[n].z = (skyIrradiance2[n].z > 0.0f) ? skyIrradiance2[n].z : 0.0f;
							VectorAdd( vertexlight2[n], skyIrradiance2[n], vertexlight2[n] );
						}
					}
					else
					{
						float bump_dot = dl->light.type == emit_skyambient ? bump_dots[n-1] : DotProduct( world_bump_basis[n-1], light_vec );
						if ( bump_dot > 0)
						{
							VectorMA( vertexlight[bumpIndex+1], falloff * bump_dot, dl->light.intensity, vertexlight[bumpIndex+1] );
							if( bRealtimeLight )
							{
								VectorMA( vertexlight2[n], falloff * bump_dot, dl->light.intensity, vertexlight2[n] );
							}
						}
					}
				}
			}

			sun_visibility_array[meshid] = gSunVisible;
		}
	}

	
	// restore collision context
	skip_id = -1;
}


#define NUMVERTEXNORMALS	162
static Vector s_raddir[NUMVERTEXNORMALS]=
{
#include "anorms.h"
};

// unreal engine 3
#define WORLD_MAX			524288.0	/* Maximum size of the world */

extern directlight_t *gAmbient;

void ComputeVertexAmbientLights( int threadnum, int vertex_chunk_number )
{
	int start_vertex = vertex_chunk_number * vertex_chunk_size;
	int end_vertex = min(num_blackmesh_vertexes, start_vertex + vertex_chunk_size);

	// compute first mesh id mesh id
	int meshid = 0;
	for( meshid = 0; meshid < s_pVRadDll2->NumBlackMeshVertexCounts_; ++meshid )
	{
		if( start_vertex < s_pVRadDll2->BlackMeshVertexCountArray_[meshid] )
		{
			break;
		}
	}

	const AVA_StaticMesh &staticmesh = AVA_StaticMeshManager::GetStaticMesh( meshid );
	if( staticmesh.NeedsSelfShadow() )
	{
		skip_id = -1;
	}
	else
	{
		skip_id = meshid;	// skip self
	}

	self_id = meshid;	// ambient sampling시 사용

	for( int v = start_vertex; v < end_vertex; ++v )
	{
		if( v >= s_pVRadDll2->BlackMeshVertexCountArray_[meshid] )
		{
			++meshid;
			const AVA_StaticMesh &staticmesh = AVA_StaticMeshManager::GetStaticMesh( meshid );
			if( staticmesh.NeedsSelfShadow() )
			{
				skip_id = -1;
			}
			else
			{
				skip_id = meshid;	// skip self
			}

			self_id = meshid;	// ambient sampling시 사용
		}

		Vector& pos = blackmesh_vertexes[v];
		Vector& tangent_x = blackmesh_tangent_x[v];
		Vector& tangent_y = blackmesh_tangent_y[v];
		Vector& tangent_z = blackmesh_tangent_z[v];

		Vector world_bump_basis[3];
		GetBumpNormals( tangent_x, tangent_y, tangent_z, tangent_z, world_bump_basis );

		Vector ambient_color[4] =
		{
			Vector( 0.0f, 0.0f, 0.0f ),
			Vector( 0.0f, 0.0f, 0.0f ),
			Vector( 0.0f, 0.0f, 0.0f ),
			Vector( 0.0f, 0.0f, 0.0f )
		};

		float denom[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		for( int i = 0; i < NUMVERTEXNORMALS; ++i )
		{
			Vector sample_pos;
			Vector upend;

			VectorAdd( pos, s_raddir[i], sample_pos );
			VectorMA( sample_pos, WORLD_MAX * 1.74, s_raddir[i], upend );

			float flDot[4];
			flDot[0] = DotProduct( tangent_z, s_raddir[i] );

			if( flDot[0] < 0.0f )
			{
				continue;
			}

			flDot[1] = DotProduct( world_bump_basis[0], s_raddir[i] );
			flDot[2] = DotProduct( world_bump_basis[1], s_raddir[i] );
			flDot[3] = DotProduct( world_bump_basis[2], s_raddir[i] );

			Vector sample_color;
			bool is_hit = SampleAmbientWithOpcode( sample_pos, upend, &sample_color );

			if( is_hit )
			{
				sample_color.x = max( 0.0f, sample_color.x * INV_PI);
				sample_color.y = max( 0.0f, sample_color.y * INV_PI);
				sample_color.z = max( 0.0f, sample_color.z * INV_PI);
				for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1 ; ++ibump )
				{
					if( flDot[ibump] > 0.0f )
					{
						VectorMA( ambient_color[ibump], flDot[ibump], sample_color, ambient_color[ibump] );
						denom[ibump] += flDot[ibump];
					}
				}
			}
		}

		for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1; ++ibump )
		{
			if( denom[ibump] != 0.0f )
			{
				VectorScale( ambient_color[ibump], 1.0f / denom[ibump], ambient_color[ibump] );
			}
		}

		// total_vertex_light = vertex_direct_lighting + vertex_ambient_lighting
		Vector total_vertex_lighting[4];

		VectorAdd( blackmesh_vertexlight_float_vector_data[v*4 + 0], ambient_color[0], total_vertex_lighting[0] );
		VectorAdd( blackmesh_vertexlight_float_vector_data[v*4 + 1], ambient_color[1], total_vertex_lighting[1] );
		VectorAdd( blackmesh_vertexlight_float_vector_data[v*4 + 2], ambient_color[2], total_vertex_lighting[2] );
		VectorAdd( blackmesh_vertexlight_float_vector_data[v*4 + 3], ambient_color[3], total_vertex_lighting[3] );

		// encoding total_vertex_light -> blackmesh_vertexlightdata
		// vertexlight = 16 byte [ (rgbe for normal), (rgbe for bumpvect0),(rgbe for bumpvec1),(rgbe for bumpvec2) ]
		const int color_size = 4; // rgbe = 4 byte
		const int vertex_light_size = 16;

		Vec3toColorRGBExp32( total_vertex_lighting[0], ( colorRGBExp32 *) &blackmesh_worldvertexlightdata[ v * vertex_light_size + color_size * 0 ] );
		Vec3toColorRGBExp32( total_vertex_lighting[1], ( colorRGBExp32 *) &blackmesh_worldvertexlightdata[ v * vertex_light_size + color_size * 1 ] );
		Vec3toColorRGBExp32( total_vertex_lighting[2], ( colorRGBExp32 *) &blackmesh_worldvertexlightdata[ v * vertex_light_size + color_size * 2 ] );
		Vec3toColorRGBExp32( total_vertex_lighting[3], ( colorRGBExp32 *) &blackmesh_worldvertexlightdata[ v * vertex_light_size + color_size * 3 ] );
	}

	// restore skip
	skip_id = -1;
}

////////////////////// GI::AVA_GatherVertexAmbientLights /////////////////
void AVA_GatherVertexAmbientLights( int threadnum, int vertex_chunk_number )
{
	int start_vertex = vertex_chunk_number * vertex_chunk_size;
	int end_vertex = min(num_blackmesh_vertexes, start_vertex + vertex_chunk_size);

	// compute first mesh id mesh id
	int meshid = 0;
	for( meshid = 0; meshid < s_pVRadDll2->NumBlackMeshVertexCounts_; ++meshid )
	{
		if( start_vertex < s_pVRadDll2->BlackMeshVertexCountArray_[meshid] )
		{
			break;
		}
	}

	const AVA_StaticMesh &staticmesh = AVA_StaticMeshManager::GetStaticMesh( meshid );
	if( staticmesh.NeedsSelfShadow() )
	{
		skip_id = -1;
	}
	else
	{
		skip_id = meshid;	// skip self
	}

	self_id = meshid;	// ambient sampling시 사용

	for( int v = start_vertex; v < end_vertex; ++v )
	{
		if( v >= s_pVRadDll2->BlackMeshVertexCountArray_[meshid] )
		{
			++meshid;
			const AVA_StaticMesh &staticmesh = AVA_StaticMeshManager::GetStaticMesh( meshid );
			if( staticmesh.NeedsSelfShadow() )
			{
				skip_id = -1;
			}
			else
			{
				skip_id = meshid;	// skip self
			}

			self_id = meshid;	// ambient sampling시 사용
		}

		Vector& pos = blackmesh_vertexes[v];
		Vector& tangent_x = blackmesh_tangent_x[v];
		Vector& tangent_y = blackmesh_tangent_y[v];
		Vector& tangent_z = blackmesh_tangent_z[v];

		Vector world_bump_basis[3];
		GetBumpNormals( tangent_x, tangent_y, tangent_z, tangent_z, world_bump_basis );

		Vector ambient_color[4] =
		{
			Vector( 0.0f, 0.0f, 0.0f ),
			Vector( 0.0f, 0.0f, 0.0f ),
			Vector( 0.0f, 0.0f, 0.0f ),
			Vector( 0.0f, 0.0f, 0.0f )
		};

		float denom[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		for( int i = 0; i < NUMVERTEXNORMALS; ++i )
		{
			Vector sample_pos;
			Vector upend;

			VectorAdd( pos, s_raddir[i], sample_pos );
			VectorMA( sample_pos, WORLD_MAX * 1.74, s_raddir[i], upend );

			float flDot[4];
			flDot[0] = DotProduct( tangent_z, s_raddir[i] );

			if( flDot[0] < 0.0f )
			{
				continue;
			}

			flDot[1] = DotProduct( world_bump_basis[0], s_raddir[i] );
			flDot[2] = DotProduct( world_bump_basis[1], s_raddir[i] );
			flDot[3] = DotProduct( world_bump_basis[2], s_raddir[i] );

			Vector sample_color;
			bool is_hit = SampleDiffuseReflectionWithOpcode( sample_pos, upend, &sample_color );

			if( is_hit )
			{
				sample_color.x = max( 0.0f, sample_color.x * INV_PI);
				sample_color.y = max( 0.0f, sample_color.y * INV_PI);
				sample_color.z = max( 0.0f, sample_color.z * INV_PI);
				for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1 ; ++ibump )
				{
					if( flDot[ibump] > 0.0f )
					{
						VectorMA( ambient_color[ibump], flDot[ibump], sample_color, ambient_color[ibump] );
						denom[ibump] += flDot[ibump];
					}
				}
			}
		}

		for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1; ++ibump )
		{
			if( denom[ibump] != 0.0f )
			{
				VectorScale( ambient_color[ibump], 1.0f / denom[ibump], ambient_color[ibump] );
			}
		}

		// ambient light를 여기에 저장
		VectorCopy( ambient_color[0], blackmesh_vertexlight_float_vector_data[v*4 + 0] );
		VectorCopy( ambient_color[1], blackmesh_vertexlight_float_vector_data[v*4 + 1] );
		VectorCopy( ambient_color[2], blackmesh_vertexlight_float_vector_data[v*4 + 2] );
		VectorCopy( ambient_color[3], blackmesh_vertexlight_float_vector_data[v*4 + 3] );		
	}

	// restore skip
	skip_id = -1;
}


SHVector UnitSH[NUMVERTEXNORMALS];

int InitSHTables();
int UnitSHInitializer()
{
	InitSHTables();
	for (int i=0; i<NUMVERTEXNORMALS; ++i)
	{
		UnitSH[i] = PointLightSH( s_raddir[i] );
	}

	return 1;
}

static int __dummy_UnitSHInitializer = UnitSHInitializer();

////////////////// Compute Point Irradiance ///////////////////////////////
void ComputeSkyIrradianceAtPoint( const Vector &pos, Vector *irradiance )
{
	const int sampleCount = (sampleskylevel == 1) ? 128 : Pbrt::MAX_SAMPLES;
	Vector normal( 0, 0, 1 );

	const float inv_pdf = 1.0f / Pbrt::uniform_sky_samples[0].pdf;
	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		const Vector &wi = Pbrt::uniform_sky_samples[sampleIndex].wi;

		float dot = DotProduct( normal, wi );

		if( dot <= EQUAL_EPSILON )
			continue;

		// check visibility
		Vector trace_end;
		VectorScale( wi, MAX_TRACE_LENGTH, trace_end );
		VectorAdd( pos, trace_end, trace_end );
		if( TestLine( pos, trace_end, 0, 0 ) != CONTENTS_EMPTY )
		{
			continue;
		}

		// visible
		const Vector &radiance = Pbrt::uniform_sky_samples[sampleIndex].radiance;
		VectorMA( *irradiance, dot * inv_pdf, radiance, *irradiance );
	}
	VectorScale( *irradiance, 1.0f / (float)sampleCount, *irradiance );
}

void ComputeSunAndSkyIrradianceAtPoint( const Vector &pos, Vector *irradiance )
{
	// for sun sampling
	Pbrt::EnvmapSample_t* envSamples = NULL;
	Pbrt::GetEnvironmentMapSamples( &envSamples );
	Vector normal( 0, 0, 1 );

	const int sampleCount = (sampleskylevel == 1) ? 128 : Pbrt::MAX_SAMPLES;
	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		const Vector &wi = envSamples[sampleIndex].wi;
		const float &pdf = envSamples[sampleIndex].pdf;

		float dot = DotProduct( normal, wi );
		if( dot <= EQUAL_EPSILON )
			continue;

		// check visibility
		Vector trace_end;
		VectorScale( wi, MAX_TRACE_LENGTH, trace_end );
		VectorAdd( pos, trace_end, trace_end );
		if( TestLine( pos, trace_end, 0, 0 ) != CONTENTS_EMPTY )
		{
			continue;
		}

		// visible
		const Vector &radiance = envSamples[sampleIndex].scaledRadiance;
		VectorMA( *irradiance, dot / pdf, radiance, *irradiance );
	}
	VectorScale( *irradiance, 1.0f / (float)sampleCount, *irradiance );
}

void ComputeIrradianceAtPointOfLight( directlight_t *dl, const Vector &pos, Vector *irradiance )
{
	Vector normal( 0, 0, 1 );
	if( dl->light.type == emit_hdrEnvmapSkylight )
	{
		if( dl->light.flags || sampleskylevel == 0 )
		{
			return;
		}

		if( sampleskylevel > 0 )
		{
			Vector sunPos(0, 0, 0);
			Vector sunVec(0, 0, 0);

			// check sun visibility
			const Vector &sunDirection = Pbrt::GetSunDirection();

			float sunDot = DotProduct( normal, sunDirection );

			if( sunDot < EQUAL_EPSILON )
			{
				ComputeSkyIrradianceAtPoint( pos, irradiance );
			}
			else
			{
				VectorScale( sunDirection, MAX_TRACE_LENGTH, sunVec );
				VectorAdd( pos, sunVec, sunPos );
				if( TestLine( pos, sunPos, 0, 0) != CONTENTS_EMPTY )
				{
					ComputeSkyIrradianceAtPoint( pos, irradiance );
				}
				else
				{
					ComputeSunAndSkyIrradianceAtPoint( pos, irradiance );
				}
			}
		}
	}
	else if( dl->light.type == emit_skylight )
	{
		// TODO : implement this
	}
	else if( dl->light.type == emit_skyambient )
	{
		// TODO : implement this
	}
	else
	{
		Vector src(0 ,0, 0);
		Vector lightVec;
		float dist;
		if( dl->facenum == -1 )
		{
			VectorCopy( dl->light.origin, src );
		}
		VectorSubtract( src, pos, lightVec );
		dist = VectorNormalize( lightVec );
		if( dist < 1.0f )
		{
			dist = 1.0f;
		}
		float scale = 0.0f;
		switch( dl->light.type )
		{
		case emit_ue3point:
			{
				float distScale = dist / dl->light.radius;
				scale = powf( max( 1.0f - distScale * distScale, 0.0f), dl->light.exponent );
			}
			break;
		case emit_ue3spot:
			{
				float dot = -DotProduct (lightVec, dl->light.normal);
				if ( dot <= dl->light.stopdot2 )
				{
					scale = 0.0f;
				}
				else
				{
					float dist_scale = dist / dl->light.radius;					
					scale = powf( max( 1.0f - dist_scale * dist_scale, 0.0f ), dl->light.exponent );										
					if (dot <= dl->light.stopdot)
					{
						dot = ((dot) - dl->light.stopdot2) / (dl->light.stopdot - dl->light.stopdot2);
						scale = dot * dot;
					}					
				}
			}
			break;
		case emit_point:
			scale = 1.0 / (dl->light.constant_attn + dl->light.linear_attn * dist + dl->light.quadratic_attn * dist * dist);
			break;
		case emit_surface:
			{
				float dot = -DotProduct (lightVec, dl->light.normal);
				if (dot <= EQUAL_EPSILON)
				{
					scale = 0.0f;
				}
				else
				{
					scale = dot / (dist * dist);
				}
			}
			break;
		case emit_spotlight:
			{
				float dot = -DotProduct( lightVec, dl->light.normal );
				if (dot <= dl->light.stopdot2)
				{
					scale = 0.0f; // outside light cone
				}
				else
				{
					scale = dot / (dl->light.constant_attn + dl->light.linear_attn * dist + dl->light.quadratic_attn * dist * dist);
					if (dot <= dl->light.stopdot) // outside inner cone
					{
						if ((dl->light.exponent == 0.0f) || (dl->light.exponent == 1.0f))
						{
							scale *= (dot - dl->light.stopdot2) / (dl->light.stopdot - dl->light.stopdot2);
						}
						else
						{
							scale *= pow((dot - dl->light.stopdot2) / (dl->light.stopdot - dl->light.stopdot2), dl->light.exponent);
						}
					}
				}
			}
			break;
		default:
			scale = 0.0f;
			break;
		}
		if( TestLine(pos, src, 0, 0) == CONTENTS_EMPTY )
		{
			VectorScale( dl->light.intensity, scale / 4.0f * M_PI, *irradiance );
		}
	}
}



void GatherAmbientCube( int threadnum, int ambientcube_chunk_number )
{
	const Vector normal( 0, 0, 1 );

	int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
	int end_ambientcube = min(s_pVRadDll2->NumAmbientCubes_, start_ambientcube + ambientcube_chunk_size);	
	for( int v = start_ambientcube; v < end_ambientcube; ++v )
	{
		SHVectorRGB SH;
		const Vector &pos = s_pVRadDll2->AmbientCubeSamplePoints_[ v ];						

		// encode sky and bounced light to SH
		for( int i = 0; i < NUMVERTEXNORMALS; ++i )
		{
			Vector upend;
			VectorMA( pos, WORLD_MAX * 1.74, s_raddir[i], upend );

			Vector sample_color;
			bool is_hit = SampleAmbientWithOpcode( pos, upend, &sample_color );

			if (!is_hit)
			{
				if (!gAmbient)
				{
					continue;
				}

				if( gAmbient->light.type == emit_hdrEnvmapSkylight )
				{
					Pbrt::SampleSkyFromEnvironmentMap( s_raddir[i], &sample_color );
				}
				else
				{
					sample_color = gAmbient->light.intensity;
				}
			}							
			
			sample_color *= NUMVERTEXNORMALS / (8 * M_PI);						

			sample_color.x = max( 0.0f, sample_color.x );
			sample_color.y = max( 0.0f, sample_color.y );
			sample_color.z = max( 0.0f, sample_color.z );

			SH += UnitSH[i] * LinearColor( sample_color.x, sample_color.y, sample_color.z, 0 );
		}
		SH = SH * ( 1.0f / NUMVERTEXNORMALS );
		memcpy( &s_pVRadDll2->AmbientCubes_[ v ].SHValues, &SH, sizeof(SH) );

		// gather irradiance of lights
		Vector irradiance(0, 0, 0);
		for( directlight_t *dl = activelights; dl != NULL; dl = dl->next )
		{
			Vector lightIrradiance(0, 0, 0);
			ComputeIrradianceAtPointOfLight( dl, pos, &lightIrradiance );
			VectorAdd( irradiance, lightIrradiance, irradiance );
		}

		// gather irradiance of diffuse reflections
		const float inv_pdf = 1.0f / Pbrt::uniform_sphere_samples[0].pdf;
		Vector bouncedIrradiance(0, 0, 0);
		for( int i = 0; i < Pbrt::MAX_SAMPLES; ++i )
		{
			Vector upend;
			VectorMA( pos, WORLD_MAX * 1.74, Pbrt::uniform_sphere_samples[i].wi, upend );

			float dot = DotProduct( normal, Pbrt::uniform_sphere_samples[i].wi );
			if( dot < EQUAL_EPSILON )
				continue;

			Vector sample_color;
			bool is_hit = SampleAmbientWithOpcode( pos, upend, &sample_color );

			if (is_hit)
			{
				sample_color *= inv_pdf;						
				sample_color *= dot;
				sample_color.x = max( 0.0f, sample_color.x * INV_PI );
				sample_color.y = max( 0.0f, sample_color.y * INV_PI );
				sample_color.z = max( 0.0f, sample_color.z * INV_PI );
				VectorAdd(bouncedIrradiance, sample_color, bouncedIrradiance);
			}
		}
		VectorScale( bouncedIrradiance, 1.0f / (float)Pbrt::MAX_SAMPLES, bouncedIrradiance );
		VectorAdd(irradiance, bouncedIrradiance, irradiance);
		Vec3toColorRGBExp32( irradiance, ( colorRGBExp32 *) &s_pVRadDll2->AmbientCubes_[v].Irradiance );
	}
}

void GatherAmbientCubeGI( int threadnum, int ambientcube_chunk_number )
{
	const Vector normal( 0, 0, 1 );

	int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
	int end_ambientcube = min(s_pVRadDll2->NumAmbientCubes_, start_ambientcube + ambientcube_chunk_size);	
	for( int v = start_ambientcube; v < end_ambientcube; ++v )
	{
		SHVectorRGB SH;
		const Vector &pos = s_pVRadDll2->AmbientCubeSamplePoints_[ v ];						

		// encode sky and bounced light to SH
		for( int i = 0; i < NUMVERTEXNORMALS; ++i )
		{
			Vector upend;
			VectorMA( pos, WORLD_MAX * 1.74, s_raddir[i], upend );

			Vector sample_color;
			bool is_hit = SampleDiffuseReflectionWithOpcode( pos, upend, &sample_color );

			if (!is_hit)
			{
				if (!gAmbient)
				{
					continue;
				}

				if( gAmbient->light.type == emit_hdrEnvmapSkylight )
				{
					Pbrt::SampleSkyFromEnvironmentMap( s_raddir[i], &sample_color );
				}
				else
				{
					sample_color = gAmbient->light.intensity;
				}
			}							

			sample_color *= NUMVERTEXNORMALS / (8 * M_PI);						

			sample_color.x = max( 0.0f, sample_color.x );
			sample_color.y = max( 0.0f, sample_color.y );
			sample_color.z = max( 0.0f, sample_color.z );

			SH += UnitSH[i] * LinearColor( sample_color.x, sample_color.y, sample_color.z, 0 );
		}
		SH = SH * ( 1.0f / NUMVERTEXNORMALS );
		memcpy( &s_pVRadDll2->AmbientCubes_[ v ].SHValues, &SH, sizeof(SH) );

		// gather irradiance of lights
		Vector irradiance(0, 0, 0);
		for( directlight_t *dl = activelights; dl != NULL; dl = dl->next )
		{
			Vector lightIrradiance(0, 0, 0);

			//<@ Secondary Light Source : 2007. 8. 20
			if( dl->secondaryfacenum != -1 )
				continue;
			//>@ Secondary Light Source

			ComputeIrradianceAtPointOfLight( dl, pos, &lightIrradiance );
			VectorAdd( irradiance, lightIrradiance, irradiance );
		}

		// gather irradiance of diffuse reflections
		const float inv_pdf = 1.0f / Pbrt::uniform_sphere_samples[0].pdf;
		Vector bouncedIrradiance(0, 0, 0);
		for( int i = 0; i < Pbrt::MAX_SAMPLES; ++i )
		{
			Vector upend;
			VectorMA( pos, WORLD_MAX * 1.74, Pbrt::uniform_sphere_samples[i].wi, upend );

			float dot = DotProduct( normal, Pbrt::uniform_sphere_samples[i].wi );
			if( dot < EQUAL_EPSILON )
				continue;

			Vector sample_color;
			bool is_hit = SampleDiffuseReflectionWithOpcode( pos, upend, &sample_color );

			if (is_hit)
			{
				sample_color *= inv_pdf;						
				sample_color *= dot;
				sample_color.x = max( 0.0f, sample_color.x * INV_PI );
				sample_color.y = max( 0.0f, sample_color.y * INV_PI );
				sample_color.z = max( 0.0f, sample_color.z * INV_PI );
				VectorAdd(bouncedIrradiance, sample_color, bouncedIrradiance);
			}
		}
		VectorScale( bouncedIrradiance, 1.0f / (float)Pbrt::MAX_SAMPLES, bouncedIrradiance );
		VectorAdd(irradiance, bouncedIrradiance, irradiance);
		Vec3toColorRGBExp32( irradiance, ( colorRGBExp32 *) &s_pVRadDll2->AmbientCubes_[v].Irradiance );
	}
}

// Random Number State
/*
Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The names of its contributors may not be used to endorse or promote
products derived from this software without specific prior written
permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */
// Random Number Functions
static void init_genrand(u_long seed) {
	mt[0]= seed & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] =
			(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}
unsigned long genrand_int32(void)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		if (mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* default initial seed */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}
/* generates a random number on [0,1]-real-interval */
float genrand_real1(void)
{
	return genrand_int32()*((float)1.0/(float)4294967295.0);
	/* divided by 2^32-1 */
}
/* generates a random number on [0,1)-real-interval */
float genrand_real2(void)
{
	return genrand_int32()*((float)1.0/(float)4294967296.0);
	/* divided by 2^32 */
}

inline float RandomFloat() {
	return genrand_real2();
}

inline unsigned long RandomUInt() {
	return genrand_int32();
}

//////////////////////////////////////////////////////////////////

void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy)
{
	float r, theta;

	// Map uniform random numbers to $[-1,1]^2$
	float sx = 2 * u1 - 1;
	float sy = 2 * u2 - 1;

	// Map square to $(r,\theta)$
	// Handle degeneracy at the origin
	if (sx == 0.0 && sy == 0.0) {
		*dx = 0.0;
		*dy = 0.0;
		return;
	}

	if (sx >= -sy) {
		if (sx > sy) {
			// Handle first region of disk
			r = sx;
			if (sy > 0.0)
				theta = sy/r;
			else
				theta = 8.0f + sy/r;
		}
		else {
			// Handle second region of disk
			r = sy;
			theta = 2.0f - sx/r;
		}
	}
	else {
		if (sx <= sy) {
			// Handle third region of disk
			r = -sx;
			theta = 4.0f - sy/r;
		}
		else {
			// Handle fourth region of disk
			r = -sy;
			theta = 6.0f + sx/r;
		}
	}

	theta *= M_PI / 4.f;
	*dx = r*cosf(theta);
	*dy = r*sinf(theta);
}

inline Vector Cross(const Vector &v1, const Vector &v2)
{
	return Vector(
		(v1.y * v2.z) - (v1.z * v2.y),
		(v1.z * v2.x) - (v1.x * v2.z),
		(v1.x * v2.y) - (v1.y * v2.x)
		);
}

inline void CoordinateSystem(const Vector &v1, Vector *v2, Vector *v3)
{
	if (fabsf(v1.x) > fabsf(v1.y))
	{
		float invLen = 1.f / sqrtf(v1.x*v1.x + v1.z*v1.z);
		*v2 = Vector(-v1.z * invLen, 0.f, v1.x * invLen);
	}
	else
	{
		float invLen = 1.f / sqrtf(v1.y*v1.y + v1.z*v1.z);
		*v2 = Vector(0.f, v1.z * invLen, -v1.y * invLen);
	}
	*v3 = Cross(v1, *v2);
}

void Pbrt::CosineWeightedSphereSample( const Vector& normal, Vector* wi, float* pdf )
{
	float x, y, z;

	float u1 = RandomFloat();
	float u2 = RandomFloat();

	ConcentricSampleDisk( u1, u2, &x, &y );
	z = sqrtf( max( 0.0f, 1.0f - x*x - y*y) );

	if( RandomFloat() < .5 ) 
	{
		z*= -1;
	}

	*wi = Vector( x, y, z );

	*pdf = fabsf( wi->z ) * INV_TWOPI;

	// transform direction to world space
	Vector v1, v2;
	CoordinateSystem( normal, &v1, &v2 );
	*wi = Vector(
		v1.x * wi->x + v2.x * wi->y + normal.x * wi->z ,
		v1.y * wi->x + v2.y * wi->y + normal.y * wi->z ,
		v1.z * wi->x + v2.z * wi->y + normal.z * wi->z
		);

}

const INT numCachedSamples = 6;
Vector aaNormals[ numCachedSamples ] =
{
	Vector( 1.0f, 0.0f, 0.0f ),
	Vector( -1.0f, 0.0f, 0.0f ),
	Vector( 0.0f, 1.0f, 0.0f ),
	Vector( 0.0f, -1.0f, 0.0f ),
	Vector( 0.0f, 0.0f, 1.0f ),
	Vector( 0.0f, 0.0f, -1.0f )
};

Pbrt::Sample_t cachedSamples[numCachedSamples][Pbrt::MAX_SAMPLES];
Pbrt::Sample_t localspace_samples[Pbrt::MAX_SAMPLES];
Pbrt::Sample_t localspace_uniform_samples[Pbrt::MAX_SAMPLES];
Pbrt::Sample_t Pbrt::uniform_sky_samples[Pbrt::MAX_SAMPLES];
Pbrt::Sample_t Pbrt::uniform_sphere_samples[Pbrt::MAX_SAMPLES];

#define THRESH_NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */

Pbrt::Sample_t* Pbrt::GetCachedSample( const Vector& normal )
{
	for( int dirIndex = 0; dirIndex < numCachedSamples; ++dirIndex )
	{
		Vector dist;
		VectorSubtract( normal, aaNormals[dirIndex], dist );

		if( abs(dist.x) < THRESH_NORMALS_ARE_SAME
			&& abs(dist.y) < THRESH_NORMALS_ARE_SAME
			&& abs(dist.z) < THRESH_NORMALS_ARE_SAME )
		{
			return &cachedSamples[dirIndex][0];
		}
	}

	return NULL;
}

// To generate high-quality samples for the integrators, use low-discrepancy patterns
inline float VanDerCorput(unsigned int n, unsigned int scramble)
{
	n = (n << 16) | (n >> 16);
	n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
	n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
	n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
	n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
	n ^= scramble;

	return (float)n / (float)0x100000000LL;
}

inline float Sobol2(unsigned int n, unsigned int scramble)
{
	for (u_int v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
	{
		if (n & 0x1)
		{
			scramble ^= v;
		}
	}

	return (float)scramble / (float)0x100000000LL;
}

inline void Sample02( unsigned int n, unsigned int scramble[2], float sample[2] )
{
	sample[0] = VanDerCorput(n, scramble[0]);
	sample[1] = Sobol2(n, scramble[1] );
}

///// generate uniform samples /////////////
void GenerateUniformSamplesHemisphere( int sampleCount, Pbrt::Sample_t *samples )
{
	unsigned int scramble[2] = { RandomUInt(), RandomUInt() };
	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		float u[2];
		Sample02( sampleIndex, scramble, u);
		float z = u[0];
		float r = sqrtf(max(0.f, 1.f - z*z));
		float phi = 2 * M_PI * u[1];
		float x = r * cosf(phi);
		float y = r * sinf(phi);
		samples[sampleIndex].wi = Vector( x, y, z );
		samples[sampleIndex].pdf = INV_TWOPI;
	}
}

void Pbrt::GetUniformSamplesHemisphere( const Vector& normal, int sampleCount, Pbrt::Sample_t* samples )
{
	Vector v1, v2;
	CoordinateSystem( normal, &v1, &v2 );

	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		samples[sampleIndex].wi = localspace_uniform_samples[sampleIndex].wi;
		samples[sampleIndex].pdf = localspace_uniform_samples[sampleIndex].pdf;

		// 원하는 방향으로 회전시킨다.
		Vector *wi = &samples[sampleIndex].wi;
		*wi = Vector(
			v1.x * wi->x + v2.x * wi->y + normal.x * wi->z ,
			v1.y * wi->x + v2.y * wi->y + normal.y * wi->z ,
			v1.z * wi->x + v2.z * wi->y + normal.z * wi->z
			);
	}
}

void GenerateUniformSamplesSphere( int sampleCount, Pbrt::Sample_t *samples )
{
	unsigned int scramble[2] = { RandomUInt(), RandomUInt() };
	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		float u[2];
		Sample02( sampleIndex, scramble, u);
		float z = 1.f - 2.f * u[0];
		float r = sqrtf(max(0.f, 1.f - z*z));
		float phi = 2 * M_PI * u[1];
		float x = r * cosf(phi);
		float y = r * sinf(phi);
		samples[sampleIndex].wi = Vector( x, y, z );
		samples[sampleIndex].pdf = 1.f / (4.f * M_PI);
	}
}

////////////////// Cosine weighted hemisphere sampling ////////////////////////

void GenerateCosineWeightedHemiSphereSample( int sampleCount, Pbrt::Sample_t* samples )
{
	float x, y, z;

	unsigned int scramble[2] = { RandomUInt(), RandomUInt() };

	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		float u[2];
		Sample02( sampleIndex, scramble, u);

		ConcentricSampleDisk( u[0], u[1], &x, &y );

		z = sqrtf( max( 0.0f, 1.0f - x*x - y*y) );

		samples[sampleIndex].wi = Vector( x, y, z );

		// set pdf here because pdf = cos value
		samples[sampleIndex].pdf = z * INV_PI;
	}
};

void Pbrt::CosineWeightedHemiSphereSample( const Vector& normal, int sampleCount, Pbrt::Sample_t* samples )
{
	Vector v1, v2;
	CoordinateSystem( normal, &v1, &v2 );

	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		samples[sampleIndex].wi = localspace_samples[sampleIndex].wi;
		samples[sampleIndex].pdf = localspace_samples[sampleIndex].pdf;

		// 원하는 방향으로 회전시킨다.
		Vector *wi = &samples[sampleIndex].wi;
		*wi = Vector(
			v1.x * wi->x + v2.x * wi->y + normal.x * wi->z ,
			v1.y * wi->x + v2.y * wi->y + normal.y * wi->z ,
			v1.z * wi->x + v2.z * wi->y + normal.z * wi->z
			);
	}
}

void Pbrt::Initialize()
{
	GenerateCosineWeightedHemiSphereSample( MAX_SAMPLES, localspace_samples );
	GenerateUniformSamplesHemisphere( MAX_SAMPLES, localspace_uniform_samples );

	GenerateUniformSamplesSphere( MAX_SAMPLES, uniform_sphere_samples );
	GenerateUniformSamplesHemisphere( MAX_SAMPLES, uniform_sky_samples );

	for( int dirIndex = 0; dirIndex < numCachedSamples; ++dirIndex )
	{
		CosineWeightedHemiSphereSample( aaNormals[dirIndex], MAX_SAMPLES, &cachedSamples[dirIndex][0] );
	}
}

__forceinline float SphericalTheta( const Vector& v )
{
	return acosf(v.z);
}

__forceinline float SphericalPhi( const Vector& v )
{
	float p = atan2f( v.y, v.x );
	return ( p < 0.f ) ? p + 2.f * M_PI : p;
}

static CString Root("\\\\avabuild\\avacontext\\rad_mpi\\sky_environment_map\\");

void ComputeStep1dCDF(float *f, int nSteps, float *c, float *cdf)
{
	// Compute integral of step function at $x_i$
	int i;
	cdf[0] = 0.;
	for (i = 1; i < nSteps+1; ++i)
	{
		cdf[i] = cdf[i-1] + f[i-1] / nSteps;
		//cdf[i] = cdf[i-1] + f[i-1];
	}
	// Transform step function integral into cdf
	*c = cdf[nSteps];
	for (i = 1; i < nSteps+1; ++i)
	{
		cdf[i] /= *c;
	}
}

struct Distribution1D
{
	// Distribution1D Methods
	Distribution1D(float *f, int n)
	{
		func = new float[n];
		cdf = new float[n+1];
		count = n;
		memcpy(func, f, n*sizeof(float));
		ComputeStep1dCDF(func, n, &funcInt, cdf);
		invFuncInt = 1.f / funcInt;
		invCount = 1.f / count;
	}

	float Sample(float u, float *pdf)
	{
		// Find surrounding cdf segments
		float *ptr = std::lower_bound(cdf, cdf+count+1, u);
		int offset = (int) (ptr-cdf-1);

		// Return offset along current cdf segment
		u = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);

		*pdf = func[offset] * invFuncInt;

		return offset + u;
	}
	// Distribution1D Data
	float *func, *cdf;
	float funcInt, invFuncInt, invCount;
	int count;
};

namespace Pbrt
{
	float phi, theta;
}

class Sky
{
public:
	Sky() : radianceMap_( NULL ), width_(0), height_(0), uDistrib_( NULL ), vDistribs_( NULL ), exposure_( 0 ), sunScale_( 0.001f ), radianceScale_( 1.0f ), skyMax_( 1.0f ), yaw_( 0.0f ), sunDirection_(0.0f, 0.0f, 0.0f )
	{}

	bool LoadRadianceMap( const char* fileName )
	{
		FILE *fp;

		fp = fopen( fileName, "rb" );

		if( !fp )
		{
			return false;
		}

		rgbe_header_info info;
		int width, height;

		int opState = RGBE_ReadHeader( fp, &width, &height, &info );

		if ( opState == RGBE_RETURN_SUCCESS )
		{
			const int hdrImageSize = 3 * width * height;

			if( radianceMap_ )
			{
				delete[] radianceMap_;
				radianceMap_ = NULL;
			}

			radianceMap_ = new float[hdrImageSize];

			int opStatus = RGBE_ReadPixels_RLE( fp, radianceMap_, width, height );

			if ( opStatus == RGBE_RETURN_FAILURE )
			{
				Error( "Error RGBE Read Pixels\n" );

				delete[] radianceMap_;
				radianceMap_ = NULL;

				fclose( fp );
				return false;
			}

			fclose( fp );

			width_ = width;
			height_ = height;

			float brightness = 0.0f;
			float saturation = 0.1f;

			Tonemap::Initialize( radianceMap_, width, height / 2, brightness, saturation );

			ComputeSamples();

			FindSun();

			return true;
		}
		else
		{
			fclose( fp );
			return false;
		}
	}
	

	float Pdf( const Vector& wi )
	{
		float theta = SphericalTheta( wi ), phi = SphericalPhi( wi );

		int u = clamp( Float2Int( phi * INV_TWOPI * uDistrib_->count ), 0, uDistrib_->count - 1 );
		int v = clamp( Float2Int( theta * INV_PI * vDistribs_[u]->count ), 0, vDistribs_[u]->count - 1 );

		return	( uDistrib_->func[u] * vDistribs_[u]->func[v] ) /
				( uDistrib_->funcInt * vDistribs_[u]->funcInt ) *
				1.0f / ( 2.0f * M_PI * M_PI * sinf(theta) );
	}

	__forceinline void SampleRadianceMap( const Vector& wi, Vector* sample )
	{
		if( !radianceMap_ )
		{
			sample->x = 0.0f;
			sample->y = 0.0f;
			sample->z = 0.0f;
		}
		else
		{
			int x = 0, y = 0;
			GetPixelPosition( wi, &x, &y );
			float* pixel = &radianceMap_[ (y * width_ + x) * 3 ];
			sample->x = pixel[0] * radianceScale_;
			sample->y = pixel[1] * radianceScale_;
			sample->z = pixel[2] * radianceScale_;
		}		
	}

	__forceinline void SampleRadianceMap( int x, int y, Vector* sample  )
	{
		const float* pixel = &radianceMap_[ (y * width_ + x) * 3 ];

		sample->x = pixel[0] * radianceScale_;
		sample->y = pixel[1] * radianceScale_;
		sample->z = pixel[2] * radianceScale_;

	}

	__forceinline void SampleSky( const Vector& wi, Vector* sample )
	{
		SampleRadianceMap( wi, sample );

		if( VectorLength( *sample ) > skyMax_ )
		{
			sample->x = 0.0f;
			sample->y = 0.0f;
			sample->z = 0.0f;
		}
		//<@ 2006. 10. 19 sky light의 색을 살리기 위해서.. sky lightㄹ scaling 하자
		else
		{
			sample->x *= skyScale_;
			sample->y *= skyScale_;
			sample->z *= skyScale_;
		}
		//>@
	}

	void SampleSun( const Vector& wi, Vector* sample )
	{
		SampleRadianceMap( wi, sample );

		sample->x = sample->x > skyMax_ ? sample->x * sunScale_ : sample->x;
		sample->y = sample->y > skyMax_ ? sample->y * sunScale_ : sample->y;
		sample->z = sample->z > skyMax_ ? sample->z * sunScale_ : sample->z;
	}

	int GetSampleCount()
	{
		if( radianceMap_ )
		{
			return Pbrt::MAX_SUN_SAMPLES;
		}
		else
		{
			return 0;
		}
	}

	Pbrt::EnvmapSample_t* GetSamples()
	{
		if( radianceMap_ )
		{
			return samples_;
		}
		else
		{
			return NULL;
		}
	}

	void SetExposure( int exposure )
	{
		exposure_ = exposure;
		radianceScale_ = powf( 2.0f, exposure );
	}

	void SetSunScale( float sunScale )
	{
		sunScale_ = sunScale;
	}

	void SetSkyMax( float skyMax )
	{
		skyMax_ = skyMax;
	}

	void SetSkyScale( float skyScale )
	{
		skyScale_ = skyScale;
	}

	void SetYaw( float yaw )
	{
		yaw_ = yaw;
	}

	const Vector& GetSunDirection()
	{
		return sunDirection_;
	}

	Vector maxSunRadiance;

private:
	__forceinline void GetPixelPosition( const Vector& wi, int* x, int* y )
	{
		float phi = SphericalPhi( wi );
		// user rotation 적용
		// yaw_ 는 degree
		phi += ( yaw_ * ( M_PI / 180.0f ) ) ;
		phi = fmodf( phi, 2.0f * M_PI );
		if( phi < 0.0f )
		{
			phi += 2.0f * M_PI;
		}

		float s = phi / ( M_PI * 2.0f );
		float t = SphericalTheta( wi ) / M_PI;

		s = clamp(s, 0, 1);
		t = clamp(t, 0, 1);

		*x = s * (width_-1);
		*y = t * (height_-1);
	}

	void FindSun()
	{
		float lumWeight[3] = { 0.212671f, 0.715160f, 0.072169f };

		int maxX = 0;
		int maxY = 0;
		float maxLum = radianceMap_[0] * lumWeight[0] + radianceMap_[1] * lumWeight[1] + radianceMap_[2] * lumWeight[2];

		maxSunRadiance = Vector( 0, 0, 0 );

		for( int x = 1; x < width_; ++x )
		{
			for( int y = 1; y < height_; ++y )
			{
				const float* radiance = &radianceMap_[ (y * width_ + x) * 3 ];
				float lum = radiance[0] * lumWeight[0] + radiance[1] * lumWeight[1] + radiance[2] * lumWeight[2];

				if ( lum > maxLum )
				{
					maxX = x;
					maxY = y;
					maxLum = lum;

					VectorMax( maxSunRadiance, Vector( radiance[0], radiance[1], radiance[2] ), maxSunRadiance ); 
				}
			}
		}

		maxSunRadiance *= radianceScale_ * sunScale_;

		Vector sunDir;

		float s = (float) maxX / (width_ - 1);
		float t = (float) maxY / (height_ - 1);

		float phi = s * 2.0f * M_PI;

		// user 회전 적용. yaw 는 degree
		phi += ( yaw_ * M_PI / 180.0f );	

		float theta = t * M_PI;

		float sintheta = sinf( theta );
		float cosphi = cosf( phi );
		float sinphi = sinf( phi );
		float costheta = cosf( theta );

		Pbrt::phi = phi;
		Pbrt::theta = theta;

		sunDirection_ =  Vector( sintheta * cosphi, sintheta * sinphi, costheta );		
	}

	void ComputeSamples()
	{
		if( radianceMap_ == NULL )
			return;

		const float lumWeight[3] = { 0.212671f, 0.715160f, 0.072169f };

		// compute scalar-valued image from environment map
		float* img = new float[ width_ * height_ ];

		const int nu = width_;
		const int nv = height_;

		for( int x = 0; x < width_; ++x )
		{
			int leftX = ( x == 0 ) ? width_ - 1 : x - 1;

			for( int y = 0 ; y < height_; ++y )
			{
				int topY = ( y == 0 ) ? height_ - 1 : y - 1;

				// four sample for average
				//float* leftTop		= &radianceMap_[ (topY * width_ + leftX) * 3 ];
				//float* rightTop		= &radianceMap_[ (topY * width_ + x) * 3 ];
				//float* leftBottom	= &radianceMap_[ (y * width_ + leftX) * 3 ];
				float* rightBottom	= &radianceMap_[ (y * width_ + x) * 3 ];

				float sample[3];
				for( int dim = 0; dim < 3; ++dim )
				{
					//sample[dim] = leftTop[dim] + rightTop[dim] + leftBottom[dim] + rightBottom[dim];
					//sample[dim] *= 0.25f;	// 0.25 = 1/4
					sample[dim] = rightBottom[dim];
				}


				// save luminance
				img[y + x*nv] = sample[0] * lumWeight[0] + sample[1] * lumWeight[1] + sample[2] * lumWeight[2];
			}
		}

		// Initialize sampling PDFs for infinite area light
		float* func = (float*) alloca( max( nu, nv ) * sizeof( float ) );
		float* sinVals = (float*) alloca( nv * sizeof( float ) );
		for( int i = 0; i < nv; ++i )
		{
			sinVals[i] = sinf( M_PI * float(i + .5) / float(nv) );
		}

		vDistribs_ = new Distribution1D *[nu];
		for( int u = 0; u < nu; ++u )
		{
			// Compute sampling distribution for column _u_
			for( int v = 0; v < nv; ++v )
			{
				func[v] = img[u*nv+v] * sinVals[v];
			}

			vDistribs_[u] = new Distribution1D(func, nv);
		}


		// Compute sampling distribution for columns of image
		for( int u = 0; u < nu; ++u )
		{
			func[u] = vDistribs_[u]->funcInt;
		}

		uDistrib_ = new Distribution1D( func, nu );

		delete[] img;

		unsigned int scramble[2] = { RandomUInt(), RandomUInt() };		

		GSHSunTblSize = 0;		
		
		GSunColor = LinearColor(0,0,0,0);
		GSunDirection = Vector( 0, 0, 0 );

		float TotalLuminance = 0;
		float AvgLuminance = 0;
		int NumSamples = 0;

		static float Pdfs = 0;

		for( int sampleIndex = 0; sampleIndex < Pbrt::MAX_SUN_SAMPLES; ++sampleIndex )
		{
			float u[2];
			Sample02( sampleIndex, scramble, u);

			GenerateSunSample( u[0], u[1], &samples_[sampleIndex].wi, &samples_[sampleIndex].pdf, &samples_[sampleIndex].radiance, &samples_[sampleIndex].scaledRadiance );			

			if (VectorLength( samples_[sampleIndex].scaledRadiance ) > skyMax_)
			{
				SHSunTblEnvIndex[GSHSunTblSize] = sampleIndex;
				SHSunTbl[GSHSunTblSize++] = PointLightSH( samples_[sampleIndex].wi ) * LinearColor( samples_[sampleIndex].scaledRadiance.x, samples_[sampleIndex].scaledRadiance.y, samples_[sampleIndex].scaledRadiance.z, 0 );

				LinearColor NewColor( samples_[sampleIndex].scaledRadiance.x, samples_[sampleIndex].scaledRadiance.y, samples_[sampleIndex].scaledRadiance.z, 0 );

				float Luminance = ( NewColor.R + NewColor.G + NewColor.B);

				Pdfs += 1.0f / samples_[sampleIndex].pdf;

				TotalLuminance += Luminance;
				NumSamples ++;
			}
		}		

		for( int sampleIndex = 0; sampleIndex < Pbrt::MAX_SUN_SAMPLES; ++sampleIndex )
		{			
			if (VectorLength( samples_[sampleIndex].scaledRadiance ) > skyMax_)
			{				
				LinearColor NewColor;

				NewColor = LinearColor( samples_[sampleIndex].scaledRadiance.x, samples_[sampleIndex].scaledRadiance.y, samples_[sampleIndex].scaledRadiance.z, 0 );

				float Luminance = ( NewColor.R + NewColor.G + NewColor.B);

				Vector Direction = samples_[sampleIndex].wi * (1.0f / samples_[sampleIndex].wi.Length());

				float pct;
				pct = (Luminance / TotalLuminance);				
				GSunDirection += Direction * pct; 
			}			
		}

		if (GSunDirection.Length()>0)
			GSunDirection /= GSunDirection.Length();

		int RealSamples = 0;

		for( int sampleIndex = 0; sampleIndex < Pbrt::MAX_SUN_SAMPLES; ++sampleIndex )
		{			
			if (VectorLength( samples_[sampleIndex].scaledRadiance ) > skyMax_)
			{				
				LinearColor NewColor;
				
				NewColor = LinearColor( samples_[sampleIndex].scaledRadiance.x, samples_[sampleIndex].scaledRadiance.y, samples_[sampleIndex].scaledRadiance.z, 0 );

				float Luminance = ( NewColor.R + NewColor.G + NewColor.B);

				Vector Direction = samples_[sampleIndex].wi * (1.0f / samples_[sampleIndex].wi.Length());
				float dot = GSunDirection.Dot( Direction );

				if (dot < 0)
					continue;

				float pct;
				pct = (Luminance / TotalLuminance);
				GSunColor += dot * NewColor / samples_[sampleIndex].pdf;

				RealSamples++;
			}			
		}

		GSunColor /= RealSamples * M_PI * 2;

		// generate hemisphere sky samples
		for( int sampleIndex = 0; sampleIndex < Pbrt::MAX_SUN_SAMPLES; ++sampleIndex )
		{
			Pbrt::SampleSkyFromEnvironmentMap( Pbrt::uniform_sky_samples[sampleIndex].wi, &Pbrt::uniform_sky_samples[sampleIndex].radiance );
		}
	}	

	void GenerateSunSample( float u1, float u2, Vector* wi, float* pdf, Vector* radiance, Vector* scaledRadiance )
	{
		float pdfs[2];
		float fu = uDistrib_->Sample( u1, &pdfs[0] );
		int u = clamp( Float2Int(fu), 0, uDistrib_->count-1 );

		float fv = vDistribs_[u]->Sample( u2, &pdfs[1] );

		// Covert sample point to direction on the unit sphere
		float theta = fv * vDistribs_[u]->invCount * M_PI;
		float phi = fu * uDistrib_->invCount * 2.0f * M_PI;

		// user 회전 적용 : yaw는 degree
		phi += ( yaw_ * M_PI / 180.0f );

		float costheta = cosf( theta ), sintheta = sinf( theta );
		float sinphi = sinf( phi ), cosphi = cosf( phi );

		*wi = Vector( sintheta * cosphi, sintheta * sinphi, costheta );
		*pdf = ( pdfs[0] * pdfs[1] ) / ( 2.0f * M_PI * M_PI * sintheta );

		int v = clamp( Float2Int(fv), 0, vDistribs_[u]->count-1 );

		SampleRadianceMap( u, v , radiance );

		VectorScale( *radiance, sunScale_, *scaledRadiance );
	}

private:
	float*	radianceMap_;
	int		width_;
	int		height_;
	Distribution1D *uDistrib_;
	Distribution1D **vDistribs_;

	int		exposure_;
	float	sunScale_;
	float	skyScale_;
	float	radianceScale_;
	float	skyMax_;
	float	yaw_;

	// pre-calculated samples
	Pbrt::EnvmapSample_t	samples_[Pbrt::MAX_SUN_SAMPLES];
	Vector	sunDirection_;
};

Sky gSky;

const Vector& Pbrt::GetMaxSunRadiance()
{
	return gSky.maxSunRadiance;
}

float Pbrt::GetEnvrionmentPdf( const Vector&wi )
{
	return gSky.Pdf( wi );
}

void Pbrt::GetEnvironmentMapSamples( Pbrt::EnvmapSample_t** samples )
{
	*samples = gSky.GetSamples();
}
void Pbrt::SampleEnvironmentMap( const Vector& wi, Vector* radiance )
{
	gSky.SampleRadianceMap( wi, radiance );
}

void Pbrt::SampleSkyFromEnvironmentMap( const Vector& wi, Vector* radiance )
{
	gSky.SampleSky( wi, radiance );
}

void Pbrt::SampleSunFromEnvironmentMap( const Vector& wi, Vector* radiance )
{
	gSky.SampleSun( wi, radiance );
}

const Vector& Pbrt::GetSunDirection()
{
	return gSky.GetSunDirection();
}

bool Pbrt::LoadEnvironmentMap( const char* fileName, int exposure, float sunScale, float skyMax, float skyScale, float Yaw )
{
	gSky.SetExposure( exposure );

	gSky.SetSunScale( sunScale );

	gSky.SetSkyScale( skyScale );

	gSky.SetSkyMax( skyMax );

	gSky.SetYaw( Yaw );

	CString fullFileName = Root + fileName;

	bool result = gSky.LoadRadianceMap( fullFileName );

	if (result == false )
	{
		return false;
	}
	
	return true;
}

//#pragma optimize("", on)