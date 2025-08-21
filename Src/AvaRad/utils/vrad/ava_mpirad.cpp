// Redduck inc, 2007

#include <windows.h>
#define MPICH_IGNORE_CXX_SEEK
#include "mpi.h"
#include "mpidist.h"
#include "ava_mpirad.h"

//#include "vmpi.h"
#include "vrad.h"
#include "lightmap.h"

bool g_bUseMPI = false;
bool g_bMPIMaster = true;

extern void BuildPatchLights( int facenum );

///////////////////////////
void BuildFacelightsWorker (UINT iThread, UINT facenum, void*)
{	
	BuildFacelights( iThread, facenum );
}

void AVA_MPI_BuildFacelights()
{
	Threads threads;
	threads.SetNumThreads( numthreads );
	class FaceLightMPIInterface : public MPIInterface
	{
	public :
		FaceLightMPIInterface()
			: buf(0)
		{}
		~FaceLightMPIInterface()
		{
			free(buf);
		}
		virtual UINT GetBufferCount( UINT iWork ) 
		{
			Prepare( iWork );
			return 5 + light_entries + shadow_entries;
		}
		void* buf;
		UINT light_entries;
		UINT shadow_entries;
		UINT update_buffercount;
		std::pair<UINT,UINT> conv1[ MAXLIGHTMAPS * (NUM_BUMP_VECTS+1) ];
		void Prepare( UINT iWork )
		{						
			facelight_t* fl = &facelight[iWork];

			light_entries = 0;

			for (UINT i=0; i<MAXLIGHTMAPS; ++i)
			{
				for (UINT j=0; j<NUM_BUMP_VECTS+1; ++j)
				{
					if (fl->light[i][j])
					{						
						conv1[ light_entries++ ] = std::make_pair(i,j);								
					}
				}
			}
			// shadows
			// lightshadow_t has shadow array
			// buffer 0 = lightshadow_t list
			// buffer 1	= shadow array
			shadow_entries = fl->shadows.Count() * 2;
		}

		virtual void PrepareResultBuffer( UINT iWork, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			dface_t* face = &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];
			UINT iLightmap, iBump;
			switch (iBuffer)
			{
			case 0 :
				*ppBuffer = face;
				*Size = sizeof(dface_t);
				break;
			case 1 :
				*ppBuffer = fl;
				*Size = sizeof(facelight_t);
				break;
			case 2 :
				*ppBuffer = fl->sample;
				*Size = sizeof(sample_t) * fl->numsamples;
				break;					
			case 3 :
				*ppBuffer = fl->luxel;
				*Size = sizeof(Vector) * fl->numluxels;						
				break;
			case 4 :
				*ppBuffer = fl->luxelNormals;
				*Size = fl->luxelNormals ? sizeof(Vector) * fl->numluxels : 0;
				break;
			default :
				if( iBuffer < ( 5 + light_entries ) )
				{
					iLightmap = conv1[iBuffer - 5].first;
					iBump = conv1[iBuffer - 5].second;
					if (fl->light[iLightmap][iBump]) 
					{
						*ppBuffer = fl->light[iLightmap][iBump];
						*Size = sizeof(Vector) * fl->numsamples;
					}
					else
					{
						*ppBuffer = 0;
						*Size = 0;
					}
				}
				else
				{
					// 0 = lightshadow
					// 1 = shadow array in lightshadow
					const int shadow_index = ( iBuffer - ( 5 + light_entries ) ) / 2;
					const int buffer_type = ( iBuffer - ( 5 + light_entries ) ) % 2;
					switch( buffer_type )
					{
					case 0:
						*ppBuffer = &(fl->shadows[shadow_index]);
						*Size = sizeof( lightshadow_t );
						break;

					case 1:
						*ppBuffer = fl->shadows[shadow_index].shadow;
						*Size = sizeof( float ) * fl->numsamples;
						break;

					default:
						break;
					}
				}
				break;
			}										
		}						
		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}
		virtual void UpdateResult( UINT iWork, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{		
			dface_t* face = &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];					
			UINT iLightmap, iBump;
			switch (iBuffer)
			{
			case 0 :												
				//ATLASSERT( Size == sizeof(dface_t) );
				memcpy( face, pBuffer, sizeof(dface_t) );												
				break;
			case 1 :
				{
					//ATLASSERT( Size == sizeof(facelight_t) );
					//memcpy( fl, pBuffer, sizeof(facelight_t) );						
					facelight_t* src_facelight = (facelight_t*)(pBuffer);
					fl->numsamples = src_facelight->numsamples;
					fl->numluxels = src_facelight->numluxels;
					fl->worldAreaPerLuxel = src_facelight->worldAreaPerLuxel;
					memcpy( fl->light, src_facelight->light, sizeof(Vector*) * MAXLIGHTMAPS * ( NUM_BUMP_VECTS + 1 ) );
					fl->sample = NULL;
					fl->luxel = NULL;
					fl->luxelNormals = NULL;
					fl->shadows.RemoveAll();
					Prepare( iWork );
					break;
				}
			case 2 :
				fl->sample = (sample_t*)malloc( Size );
				if (!fl->sample)
				{
					printf( "!!!!Error - alloc sample %d bytes\n", Size );
					break;
				}
				memcpy( fl->sample, pBuffer, Size );
				break;					
			case 3 :
				fl->luxel = (Vector*)malloc( Size );
				if (!fl->luxel)
				{
					printf( "!!!!Error - alloc luxel %d bytes\n", Size );
					break;
				}
				memcpy( fl->luxel, pBuffer, Size );
				break;
			case 4 :
				fl->luxelNormals = (Vector*)malloc( Size );
				if (!fl->luxelNormals)
				{
					printf( "!!!!Error - alloc luxelNormals %d bytes\n", Size );
					break;
				}
				memcpy( fl->luxelNormals, pBuffer, Size );
				break;
			default :	
				if( iBuffer < 5 + light_entries )
				{
					iLightmap = conv1[iBuffer - 5].first;
					iBump = conv1[iBuffer - 5].second;

					//Msg( "update %d, %d, %p - %d %d %p %d", iLightmap, iBump, fl->light[iLightmap][iBump], iWork, iBuffer, pBuffer, Size );

					fl->light[iLightmap][iBump] = (Vector*)malloc( Size );
					memcpy( fl->light[iLightmap][iBump], pBuffer, Size );												
				}
				else
				{
					// 0 = lightshadow
					// 1 = shadow array in lightshadow
					const int shadow_index	= ( iBuffer - ( 5 + light_entries ) ) / 2;
					const int buffer_type	= ( iBuffer - ( 5 + light_entries ) ) % 2;

					while( fl->shadows.Count() <= shadow_index )
					{
						fl->shadows.AddToTail();
					}

					switch( buffer_type )
					{
					case 0:
						memcpy( &fl->shadows[shadow_index], pBuffer, Size );
						break;

					case 1:
						fl->shadows[shadow_index].shadow = (float*)malloc(Size);
						memcpy( fl->shadows[shadow_index].shadow, pBuffer, Size );
						break;

					default:
						break;
					}
				}
				break;
			}
		}
	};
	threads.SetNumThreads( numthreads );
	FaceLightMPIInterface FaceLighting;
	threads.MPI_Run( "FaceLight", numfaces, BuildFacelightsWorker, NULL, &FaceLighting, false );

	/// MPI를 사용하는 경우, Master인 경우
	if( g_bUseMPI && g_bMPIMaster )
	{
		DWORD LastTick, CurTick;
		LastTick = ::GetTickCount();
		Msg( "[[PROGRESS:BuildPatchLight %d %d]]\n", 0, numfaces );		
		for (UINT i=0; i<numfaces; ++i)
		{
			BuildPatchLights( i );						
			CurTick = ::GetTickCount();
			if (CurTick - LastTick > 100 || numfaces-1 == i)
			{
				LastTick = CurTick;				
				Msg( "[[PROGRESS:BuildPatchLight  %d %d]]\n", i+1, numfaces );
			}			
		}
	}
}

/////////////////////////////////////////////////////////////
void AVA_BuildFacelightsWorker (UINT iThread, UINT facenum, void*)
{	
	AVA_BuildFacelights( iThread, facenum );
}

void AVA_MPI_BuildFacelights2()
{
	Threads threads;
	threads.SetNumThreads( numthreads );
	class FaceLightMPIInterface : public MPIInterface
	{
	public :
		FaceLightMPIInterface()
			: buf(0)
		{}
		~FaceLightMPIInterface()
		{
			free(buf);
		}
		virtual UINT GetBufferCount( UINT iWork ) 
		{
			Prepare( iWork );
			return light_entries + shadow_entries + 1;
		}
		void* buf;
		UINT light_entries;
		UINT shadow_entries;
		UINT update_buffercount;
		std::pair<UINT,UINT> conv1[ MAXLIGHTMAPS * (NUM_BUMP_VECTS+1) ];
		void Prepare( UINT iWork )
		{						
			facelight_t* fl = &facelight[iWork];
			light_entries = 0;
			for (UINT i=0; i<MAXLIGHTMAPS; ++i)
			{
				for (UINT j=0; j<NUM_BUMP_VECTS+1; ++j)
				{
					if (fl->light[i][j])
					{						
						conv1[ light_entries++ ] = std::make_pair(i,j);								
					}
				}
			}
			// shadows
			// lightshadow_t has shadow array
			// buffer 0 = lightshadow_t list
			// buffer 1	= shadow array
			shadow_entries = fl->shadows.Count() * 2;
		}

		virtual void PrepareResultBuffer( UINT iWork, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			dface_t* face	= &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];
			UINT iLightmap, iBump;
			if( iBuffer == 0 )
			{
				*ppBuffer = &face->bSunVisibility;
				*Size = sizeof(int);
			}
			else if( iBuffer < (light_entries+1) )
			{
				iLightmap = conv1[iBuffer-1].first;
				iBump = conv1[iBuffer-1].second;
				if (fl->light[iLightmap][iBump]) 
				{
					*ppBuffer = fl->light[iLightmap][iBump];
					*Size = sizeof(Vector) * fl->numsamples;
				}
				else
				{
					*ppBuffer = 0;
					*Size = 0;
				}
			}
			else
			{
				// 0 = lightshadow
				// 1 = shadow array in lightshadow
				const int shadow_index = ( (iBuffer-1) - ( 5 + light_entries ) ) / 2;
				const int buffer_type = ( (iBuffer-1) - ( 5 + light_entries ) ) % 2;
				switch( buffer_type )
				{
				case 0:
					*ppBuffer = &(fl->shadows[shadow_index]);
					*Size = sizeof( lightshadow_t );
					break;

				case 1:
					*ppBuffer = fl->shadows[shadow_index].shadow;
					*Size = sizeof( float ) * fl->numsamples;
					break;

				default:
					break;
				}
			}
		}						
		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}
		virtual void UpdateResult( UINT iWork, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{		
			dface_t* face = &dfaces[iWork];

			facelight_t* fl = &facelight[iWork];			
			UINT iLightmap, iBump;

			if( iBuffer == 0 )
			{
				memcpy( &face->bSunVisibility, pBuffer, sizeof(int) );												
				Prepare( iWork );
			}
			else  if( iBuffer < light_entries + 1)
			{
				iLightmap = conv1[iBuffer-1].first;
				iBump = conv1[iBuffer-1].second;
				if( !fl->light[iLightmap][iBump] )
				{
					printf( "error!!! facelight did not be computed\n" );
				}
				else
					memcpy( fl->light[iLightmap][iBump], pBuffer, Size );												
			}
			else
			{
				// 0 = lightshadow
				// 1 = shadow array in lightshadow
				const int shadow_index	= ( (iBuffer-1) - ( light_entries ) ) / 2;
				const int buffer_type	= ( (iBuffer-1) - ( light_entries ) ) % 2;

				while( fl->shadows.Count() <= shadow_index )
				{
					fl->shadows.AddToTail();
				}

				switch( buffer_type )
				{
				case 0:
					memcpy( &fl->shadows[shadow_index], pBuffer, Size );
					break;

				case 1:
					//fl->shadows[shadow_index].shadow = (float*)malloc(Size);
					if( !fl->shadows[shadow_index].shadow )
					{
						printf( "error!!! facelight did not be computed\n" );
						break;
					}
					memcpy( fl->shadows[shadow_index].shadow, pBuffer, Size );
					break;

				default:
					break;
				}
			}
		}
	};

	FaceLightMPIInterface FaceLighting;
	threads.MPI_Run( "FaceLight", numfaces, AVA_BuildFacelightsWorker, NULL, &FaceLighting, false );
}


/////////////////////////////////////////////////////////////
// Secondary Face lights
void AVA_BuildSecondaryFacelightsWorker (UINT iThread, UINT facenum, void*)
{	
	AVA_BuildSecondaryFacelights( iThread, facenum );
}

void AVA_MPI_BuildSecondaryFacelights()
{
	Threads threads;
	threads.SetNumThreads( numthreads );
	class FaceLightMPIInterface : public MPIInterface
	{
	public :
		FaceLightMPIInterface()
			: buf(0)
		{}
		~FaceLightMPIInterface()
		{
			free(buf);
		}
		virtual UINT GetBufferCount( UINT iWork ) 
		{
			Prepare( iWork );
			return light_entries + shadow_entries;
		}
		void* buf;
		UINT light_entries;
		UINT shadow_entries;
		UINT update_buffercount;
		std::pair<UINT,UINT> conv1[ MAXLIGHTMAPS * (NUM_BUMP_VECTS+1) ];
		void Prepare( UINT iWork )
		{						
			facelight_t* fl = &facelight[iWork];
			light_entries = 0;
			for (UINT i=0; i<MAXLIGHTMAPS; ++i)
			{
				for (UINT j=0; j<NUM_BUMP_VECTS+1; ++j)
				{
					if (fl->light[i][j])
					{						
						conv1[ light_entries++ ] = std::make_pair(i,j);								
					}
				}
			}
			// shadows
			// lightshadow_t has shadow array
			// buffer 0 = lightshadow_t list
			// buffer 1	= shadow array
			shadow_entries = fl->shadows.Count() * 2;
		}

		virtual void PrepareResultBuffer( UINT iWork, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			dface_t* face	= &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];
			UINT iLightmap, iBump;
			if( iBuffer < light_entries )
			{
				iLightmap = conv1[iBuffer].first;
				iBump = conv1[iBuffer].second;
				if (fl->light[iLightmap][iBump]) 
				{
					*ppBuffer = fl->light[iLightmap][iBump];
					*Size = sizeof(Vector) * fl->numsamples;
				}
				else
				{
					*ppBuffer = 0;
					*Size = 0;
				}
			}
			else
			{
				// 0 = lightshadow
				// 1 = shadow array in lightshadow
				const int shadow_index = ( iBuffer - ( 5 + light_entries ) ) / 2;
				const int buffer_type = ( iBuffer - ( 5 + light_entries ) ) % 2;
				switch( buffer_type )
				{
				case 0:
					*ppBuffer = &(fl->shadows[shadow_index]);
					*Size = sizeof( lightshadow_t );
					break;

				case 1:
					*ppBuffer = fl->shadows[shadow_index].shadow;
					*Size = sizeof( float ) * fl->numsamples;
					break;

				default:
					break;
				}
			}
		}						
		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}
		virtual void UpdateResult( UINT iWork, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{		
			dface_t* face = &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];					
			UINT iLightmap, iBump;

			if( iBuffer == 0 )
			{
				Prepare( iWork );
			}

			if( iBuffer < light_entries )
			{
				iLightmap = conv1[iBuffer].first;
				iBump = conv1[iBuffer].second;
				if( !fl->light[iLightmap][iBump] )
				{
					printf( "error!!! facelight did not be computed\n" );
				}
				else
				{
					memcpy( fl->light[iLightmap][iBump], pBuffer, Size );
				}
			}
			else
			{
				// 0 = lightshadow
				// 1 = shadow array in lightshadow
				const int shadow_index	= ( iBuffer - ( light_entries ) ) / 2;
				const int buffer_type	= ( iBuffer - ( light_entries ) ) % 2;

				while( fl->shadows.Count() <= shadow_index )
				{
					fl->shadows.AddToTail();
				}

				switch( buffer_type )
				{
				case 0:
					memcpy( &fl->shadows[shadow_index], pBuffer, Size );
					break;

				case 1:
					//fl->shadows[shadow_index].shadow = (float*)malloc(Size);
					if( !fl->shadows[shadow_index].shadow )
					{
						printf( "error!!! facelight did not be computed\n" );
						break;
					}
					memcpy( fl->shadows[shadow_index].shadow, pBuffer, Size );
					break;

				default:
					break;
				}
			}
		}
	};

	FaceLightMPIInterface FaceLighting;
	threads.MPI_Run( "SecondaryFaceLight", numfaces, AVA_BuildSecondaryFacelightsWorker, NULL, &FaceLighting, false );
}

////////////
void AVA_GatherFaceAmbientlightsWorker( UINT iThread, UINT facenum, void* )
{
	AVA_GatherFaceAmbientlights( iThread, facenum );
}

void AVA_MPI_GatherFaceAmbientLights()
{
	Threads threads;
	threads.SetNumThreads( numthreads );

	class FaceAmbientLightMPIInterface : public MPIInterface
	{
	public :
		FaceAmbientLightMPIInterface()
			: buf(0)
		{}
		~FaceAmbientLightMPIInterface()
		{
			free(buf);
		}
		virtual UINT GetBufferCount( UINT iWork ) 
		{
			Prepare( iWork );
			return light_entries + shadow_entries;
		}
		void* buf;
		UINT light_entries;
		UINT shadow_entries;
		UINT update_buffercount;
		std::pair<UINT,UINT> conv1[ MAXLIGHTMAPS * (NUM_BUMP_VECTS+1) ];
		void Prepare( UINT iWork )
		{						
			facelight_t* fl = &facelight[iWork];

			light_entries = 0;

			for (UINT i=0; i<MAXLIGHTMAPS; ++i)
			{
				for (UINT j=0; j<NUM_BUMP_VECTS+1; ++j)
				{
					if (fl->light[i][j])
					{						
						conv1[ light_entries++ ] = std::make_pair(i,j);								
					}
				}
			}
			// shadows
			// lightshadow_t has shadow array
			// buffer 0 = lightshadow_t list
			// buffer 1	= shadow array
			shadow_entries = fl->shadows.Count() * 2;
		}

		virtual void PrepareResultBuffer( UINT iWork, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			dface_t* face = &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];
			UINT iLightmap, iBump;
			if( iBuffer < ( light_entries ) )
			{
				iLightmap = conv1[iBuffer].first;
				iBump = conv1[iBuffer].second;
				if (fl->light[iLightmap][iBump]) 
				{
					*ppBuffer = fl->light[iLightmap][iBump];
					*Size = sizeof(Vector) * fl->numsamples;
				}
				else
				{
					*ppBuffer = 0;
					*Size = 0;
				}
			}
			else
			{
				// 0 = lightshadow
				// 1 = shadow array in lightshadow
				const int shadow_index = ( iBuffer - ( light_entries ) ) / 2;
				const int buffer_type = ( iBuffer - ( light_entries ) ) % 2;
				switch( buffer_type )
				{
				case 0:
					*ppBuffer = &(fl->shadows[shadow_index]);
					*Size = sizeof( lightshadow_t );
					break;

				case 1:
					*ppBuffer = fl->shadows[shadow_index].shadow;
					*Size = sizeof( float ) * fl->numsamples;
					break;

				default:
					break;
				}
			}
		}						
		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}
		virtual void UpdateResult( UINT iWork, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{		
			dface_t* face = &dfaces[iWork];
			facelight_t* fl = &facelight[iWork];					
			UINT iLightmap, iBump;

			if( iBuffer == 0 )
			{
				Prepare( iWork );
			}

			if( iBuffer < light_entries )
			{
				iLightmap = conv1[iBuffer].first;
				iBump = conv1[iBuffer].second;
				if( iLightmap == 1 )
				{
					// runtime light 저장 용도로 쓰이는 style이기 때문에, ambient가 없습니다.
					memset(fl->light[iLightmap][iBump], 0, Size);
				}	
				else
				{
					memcpy( fl->light[iLightmap][iBump], pBuffer, Size );												
				}
			}
			else
			{
				// 0 = lightshadow
				// 1 = shadow array in lightshadow
				const int shadow_index	= ( iBuffer - ( light_entries ) ) / 2;
				const int buffer_type	= ( iBuffer - ( light_entries ) ) % 2;

				while( fl->shadows.Count() <= shadow_index )
				{
					fl->shadows.AddToTail();
				}

				switch( buffer_type )
				{
				case 0:
					memcpy( &fl->shadows[shadow_index], pBuffer, Size );
					break;

				case 1:
					memcpy( fl->shadows[shadow_index].shadow, pBuffer, Size );
					break;

				default:
					break;
				}
			}
		}
	};

	FaceAmbientLightMPIInterface FaceAmbientLighting;
	threads.MPI_Run( "GatherFaceAmbientLight", numfaces, AVA_GatherFaceAmbientlightsWorker, NULL, &FaceAmbientLighting, false );
}


//////////////////////////////////
//__forceinline void BuildVertexLightsWorker( UINT threadnum, UINT vertex_chunk_number, void* )
__forceinline void BuildVertexLightsWorker( UINT threadnum, UINT meshid, void* )
{
	if ((int)threadnum<0) return;
	AVA_BuildVertexLights(threadnum, meshid);
}

int GetNumVertexLightingJobs()
{
	int num_vertexlighting_job = ( num_blackmesh_vertexes / vertex_chunk_size );
	if( num_blackmesh_vertexes % vertex_chunk_size )
	{
		num_vertexlighting_job++;
	}
	return num_vertexlighting_job;
}

void AVA_MPI_BuildVertexlights()
{
	Threads threads;
	threads.SetNumThreads( numthreads );
	class VertexLightingMPIInterface : public MPIInterface
	{
	public :			
		virtual UINT GetBufferCount( UINT iFace ) 
		{
			return 3;
		}			
		virtual void PrepareResultBuffer( UINT meshid, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			const int start_vertex = AVA_StaticMeshManager::GetStaticMesh( meshid ).GetStartVertex();
			const int numvertices = AVA_StaticMeshManager::GetStaticMesh( meshid ).GetNumVertices();

			switch( iBuffer )
			{
			case 0:
				*ppBuffer = &blackmesh_vertexlight_float_vector_data[start_vertex*4];	// 4 = 1 normal + 3 basis data
				*Size = sizeof(Vector) * 4 * numvertices;
				break;
			case 1:
				*ppBuffer = &blackmesh_realtime_vertexlighting[start_vertex*4];	// 4 = 1 normal + 3 basis data
				*Size = sizeof(Vector) * 4 * numvertices;
				break;
			case 2:
				*ppBuffer = &sun_visibility_array[meshid];
				*Size = sizeof(int);
			default:
				break;
			}

		}						
		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}
		virtual void UpdateResult( UINT meshid, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{
			const int start_vertex = AVA_StaticMeshManager::GetStaticMesh( meshid ).GetStartVertex();
			switch( iBuffer )
			{
			case 0:
				memcpy( &blackmesh_vertexlight_float_vector_data[start_vertex*4], pBuffer, Size );
				break;
			case 1:
				memcpy( &blackmesh_realtime_vertexlighting[start_vertex*4], pBuffer, Size );
				break;
			case 2:
				memcpy( &sun_visibility_array[meshid], pBuffer, Size );
			default:
				break;
			}
			
		}
	};		
	threads.SetNumThreads( numthreads );
	VertexLightingMPIInterface VertexLighting;
	threads.MPI_Run( "VertexLighting", AVA_StaticMeshManager::GetNumMeshes(), BuildVertexLightsWorker, NULL, &VertexLighting, false );		
}

//////////////////////////
__forceinline void ComputeVertexAmbientLightsWorker( UINT threadnum, UINT vertex_chunk_number, void* )
{
	if ((int)threadnum<0) return;
	ComputeVertexAmbientLights( threadnum, vertex_chunk_number );
}

void AVA_MPI_BuildVertexAmbientlights()
{
	Threads threads;
	threads.SetNumThreads( numthreads );
	class VertexAmbientLightingMPIInterface : public MPIInterface
	{
	public :			
		virtual UINT GetBufferCount( UINT iFace ) 
		{
			return 1;
		}			

		virtual void PrepareResultBuffer( UINT vertex_chunk_number, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			int start_vertex = vertex_chunk_number * vertex_chunk_size;
			int end_vertex = min(num_blackmesh_vertexes, start_vertex + vertex_chunk_size);

			*ppBuffer = &blackmesh_worldvertexlightdata[start_vertex*16];
			*Size = 16 * (end_vertex - start_vertex);
		}						

		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}

		virtual void UpdateResult( UINT vertex_chunk_number, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{					
			int start_vertex = vertex_chunk_number * vertex_chunk_size;
			memcpy( &blackmesh_worldvertexlightdata[start_vertex*16], pBuffer, Size );
		}
	};		
	VertexAmbientLightingMPIInterface VertexAmbientLighting;
	threads.MPI_Run( "VertexAmbientLighting", GetNumVertexLightingJobs(), ComputeVertexAmbientLightsWorker, NULL, &VertexAmbientLighting, false );		
}

///////////////// Gather Vertex Ambient Light From Bounced Light /////////////////////////////
__forceinline void GatherVertexAmbientLightsWorker( UINT threadnum, UINT vertex_chunk_number, void* )
{
	if( (int)threadnum<0 ) return;
	AVA_GatherVertexAmbientLights( threadnum, vertex_chunk_number );
}

void AVA_MPI_GatherVertexAmbientLights()
{
	Threads threads;
	threads.SetNumThreads( numthreads );
	class VertexAmbientLightingMPIInterface : public MPIInterface
	{
	public :			
		virtual UINT GetBufferCount( UINT iFace ) 
		{
			return 1;
		}			

		virtual void PrepareResultBuffer( UINT vertex_chunk_number, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			int start_vertex = vertex_chunk_number * vertex_chunk_size;
			int end_vertex = min(num_blackmesh_vertexes, start_vertex + vertex_chunk_size);

			*ppBuffer = &blackmesh_vertexlight_float_vector_data[start_vertex * 4];
			*Size = sizeof( Vector ) * 4 * (end_vertex - start_vertex);
		}						

		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}

		virtual void UpdateResult( UINT vertex_chunk_number, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{					
			int start_vertex = vertex_chunk_number * vertex_chunk_size;
			memcpy( &blackmesh_vertexlight_float_vector_data[start_vertex*4], pBuffer, Size );
		}
	};		
	VertexAmbientLightingMPIInterface VertexAmbientLighting;
	threads.MPI_Run( "GatherVertexAmbientLighting", GetNumVertexLightingJobs(), GatherVertexAmbientLightsWorker, NULL, &VertexAmbientLighting, false );
}


////////////////////////////////



int GetNumAmbientCubeJobs()
{
	int num_ambientcube_job = ( s_pVRadDll2->NumAmbientCubes_ / ambientcube_chunk_size );
	if( num_ambientcube_job % ambientcube_chunk_size )
	{
		num_ambientcube_job++;
	}
	return num_ambientcube_job;
}

void AVA_MPI_BuildAmbientcubes()
{

	Threads threads;
	threads.SetNumThreads( numthreads );
	class AmbientCubeMPIInterface : public MPIInterface
	{
	public :			
		virtual UINT GetBufferCount( UINT iFace ) 
		{
			return 1;
		}			

		virtual void PrepareResultBuffer( UINT ambientcube_chunk_number, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
			int end_ambientcube = min(s_pVRadDll2->NumAmbientCubes_, start_ambientcube + ambientcube_chunk_size);					

			*ppBuffer = &s_pVRadDll2->AmbientCubes_[start_ambientcube];
			*Size = sizeof(AvaAmbientCube) * (end_ambientcube - start_ambientcube);
		}						

		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}

		virtual void UpdateResult( UINT ambientcube_chunk_number, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{					
			int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
			memcpy( &s_pVRadDll2->AmbientCubes_[start_ambientcube], pBuffer, Size );
		}
	};		

	threads.SetNumThreads( numthreads );
	AmbientCubeMPIInterface AmbientCube;
	threads.MPI_Run( "SamplingAmbientCubes", GetNumAmbientCubeJobs(), GatherAmbientCubeWorker, NULL, &AmbientCube, false );		
}

// GI 용.
void AVA_MPI_BuildAmbientcubes_GI()
{

	Threads threads;
	threads.SetNumThreads( numthreads );
	class AmbientCubeMPIInterface : public MPIInterface
	{
	public :			
		virtual UINT GetBufferCount( UINT iFace ) 
		{
			return 1;
		}			

		virtual void PrepareResultBuffer( UINT ambientcube_chunk_number, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
		{
			int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
			int end_ambientcube = min(s_pVRadDll2->NumAmbientCubes_, start_ambientcube + ambientcube_chunk_size);					

			*ppBuffer = &s_pVRadDll2->AmbientCubes_[start_ambientcube];
			*Size = sizeof(AvaAmbientCube) * (end_ambientcube - start_ambientcube);
		}						

		virtual void DiscardResultBuffer( UINT iBuffer ) 
		{
		}

		virtual void UpdateResult( UINT ambientcube_chunk_number, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
		{					
			int start_ambientcube = ambientcube_chunk_number * ambientcube_chunk_size;
			memcpy( &s_pVRadDll2->AmbientCubes_[start_ambientcube], pBuffer, Size );
		}
	};		
	AmbientCubeMPIInterface AmbientCube;
	threads.MPI_Run( "SamplingAmbientCubes", GetNumAmbientCubeJobs(), GatherAmbientCubeGIWorker, NULL, &AmbientCube, false );		
}

////////////////////
void AVA_MPI_DistributeLightingData()
{
	if (g_bUseMPI)
	{			
		MPI_Barrier(MPI_COMM_WORLD);
		char* styles = (char*)malloc(MAXLIGHTMAPS*numfaces);
		if (g_bMPIMaster)
		{
			for (int i=0; i<numfaces; ++i)
			{
				memcpy( styles + i * MAXLIGHTMAPS, dfaces[i].styles, sizeof(MAXLIGHTMAPS) );
			}
		}
		MPI_Bcast( styles, MAXLIGHTMAPS*numfaces, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );			
		if (!g_bMPIMaster)
		{
			for (int i=0; i<numfaces; ++i)
			{
				memcpy( dfaces[i].styles, styles + i * MAXLIGHTMAPS, sizeof(MAXLIGHTMAPS) );
			}
			PrecompLightmapOffsets();
		}
		free( styles );
		MPI_Bcast( dlightdata.Base(), dlightdata.Count(), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );			
		MPI_Bcast( blackmesh_vertexlight_float_vector_data.Base(), blackmesh_vertexlight_float_vector_data.Count() * sizeof(Vector), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );			
	}
}

void AVA_MPI_DistributeBouncedLight()
{
	MPI_Bcast( bouncedlightdata.Base(), bouncedlightdata.Count(), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );
	MPI_Bcast( blackmesh_bouncing_data.Base(), blackmesh_bouncing_data.Count() * sizeof(Vector), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );
}


//<@ 2007. 8. 17 : Secondary Light Source
void AVA_MPI_DistributeSecondaryFacelights()
{
	if( g_bUseMPI )
	{
		MPI_Barrier(MPI_COMM_WORLD);

		int secondaryfacecount = 0;
		if( g_bMPIMaster )
		{
			for( int i = 0; i < numfaces; ++i )
			{
				dface_t* face	= &dfaces[i];
				if( texinfo[face->texinfo].flags & SURF_SECONDARYLIGHTSOURCE )
				{
					++secondaryfacecount;
				}
			}
			Msg( "[[PROGRESS:BroadcastSecondaryFacelights %d %d]]\n", 0, secondaryfacecount );
		}

		int broadcastcount = 0;
		for( int i = 0; i < numfaces; ++i )
		{
			dface_t* face	= &dfaces[i];
			facelight_t* fl = &facelight[i];
			if( texinfo[face->texinfo].flags & SURF_SECONDARYLIGHTSOURCE )
			{
				// light style 은 0번과 1번이 존재합니다.
				// secondary light source를 생성할 때는, 0번만이 필요합니다.
				if( fl->light[0][0] )
				{
					if( g_bMPIMaster )
					{
						Msg( "[[PROGRESS:BroadcastSecondaryFacelights %d %d]]\n", ++broadcastcount, secondaryfacecount );
					}
					MPI_Bcast( fl->light[0][0], sizeof(Vector) * fl->numsamples, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );
				}
				else
				{
					printf( "error!!! Distribute SecondaryFacelights\n" );
				}
			}
		}
	}
}
//>@ 