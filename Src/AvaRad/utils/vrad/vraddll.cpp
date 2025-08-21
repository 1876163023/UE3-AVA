//========= Copyright1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
//!{ 2006-03-20	허 창 민
//#include <strstrea.h>
#include <strstream>
#include <atlstr.h>
#include <atltime.h>
#include <dbghelp.h>

#pragma comment( lib, "dbghelp.lib" )
//!} 2006-03-20	허 창 민
#define MPICH_IGNORE_CXX_SEEK
#include "mpi.h"
#include "vraddll.h"
#include "bsplib.h"
#include "vrad.h"
//#include "map_shared.h"
#include "lightmap.h"
#include "threads.h"
#include "mpi.h"
#include "WinXPSP2Firewall.h"
#include "SHMath.h"

namespace Tonemap
{
	extern float gScaleFactor;
	extern bool bApplyTonemap;
}

int spy_info[4][3];

static CUtlVector<unsigned char> g_LastGoodLightData;
static CUtlVector<unsigned char> g_FacesTouched;

int mpi_rank, mpi_size;

extern bool g_bUseMPI, g_bMPIMaster;

EXPOSE_SINGLE_INTERFACE( CVRadDLL, IVRadDLL, VRAD_INTERFACE_VERSION );

//!{ 2006-03-20	허 창 민
EXPOSE_SINGLE_INTERFACE( CVRadDLL2, IVRadDLL2, VRAD_INTERFACE_VERSION2 );
//!} 2006-03-20	허 창 민

// Static CRC table
DWORD s_arrdwCrc32Table[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
		0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
		0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
		0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
		0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
		0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
		0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
		0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
		0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
		0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
		0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
		0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
		0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
		0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
		0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
		0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
		0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
		0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
		0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
		0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
		0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
		0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
		0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
		0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
		0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
		0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
		0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
		0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
		0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
		0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
		0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
		0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
		0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

// ---------------------------------------------------------------------------- //
// temporary static array data size tracking
// original data size = 143 megs
// - converting ddispindices, ddispverts, g_dispinfo, and dlightdata to CUtlVector
//		- 51 megs
// ---------------------------------------------------------------------------- //

class dat
{
public:
	char *name;
	int size;
};
#define DATENTRY(name) {#name, sizeof(name)}

dat g_Dats[] =
{
	DATENTRY(dmodels),
	DATENTRY(dvisdata),
	DATENTRY(dlightdata),
	DATENTRY(dentdata),
	DATENTRY(dleafs),
	DATENTRY(dplanes),
	DATENTRY(dvertexes),
	DATENTRY(g_vertnormalindices),
	DATENTRY(g_vertnormals),
	DATENTRY(texinfo),
	DATENTRY(dtexdata),
	DATENTRY(g_dispinfo),
	DATENTRY(dorigfaces),
	DATENTRY(g_primitives),
	DATENTRY(g_primverts),
	DATENTRY(g_primindices),
	DATENTRY(dfaces),
	DATENTRY(dedges),
	DATENTRY(dleaffaces),
	DATENTRY(dleafbrushes),
	DATENTRY(dsurfedges),
	DATENTRY(dbrushes),
	DATENTRY(dbrushsides),
	DATENTRY(dareas),
	DATENTRY(dareaportals),
	DATENTRY(dworldlights),
	DATENTRY(dportals),
	DATENTRY(dclusters),
	DATENTRY(dleafwaterdata),
	DATENTRY(dportalverts),
	DATENTRY(dclusterportals),
	DATENTRY(g_DispLightmapAlpha),
	DATENTRY(g_ClipPortalVerts),
	DATENTRY(g_CubemapSamples),
	DATENTRY(g_TexDataStringData),
	DATENTRY(g_TexDataStringTable),
	DATENTRY(g_Overlays)
};

int CalcDatSize()
{
	int ret = 0;
	int count = sizeof( g_Dats ) / sizeof( g_Dats[0] );
	
	int i;
	for( i=1; i < count; i++ )
	{
		if( g_Dats[i-1].size > g_Dats[i].size )
		{
			dat temp = g_Dats[i-1];
			g_Dats[i-1] = g_Dats[i];
			g_Dats[i] = temp;
			
			if( i > 1 )
				i -= 2;
			else
				i -= 1;
		}
	}

	for( i=0; i < count; i++ )
		ret += g_Dats[i].size;
	
	return ret;
}

int g_TotalDatSize = CalcDatSize();




int CVRadDLL::main( int argc, char **argv )
{
	return 0;
}


bool CVRadDLL::Init( char const *pFilename )
{
	//VRAD_Init();
	//
	//// Set options and run vrad startup code.
	//do_fast = true;
	//g_bLowPriorityThreads = true;
	//g_pIncremental = GetIncremental();

	//VRAD_LoadBSP( pFilename );
	return true;
}


void CVRadDLL::Release()
{
}


void CVRadDLL::GetBSPInfo( CBSPInfo *pInfo )
{
	//!{ 2006-05-03	허 창 민
	pInfo->dlightdata = dlightdata.Base();
	pInfo->lightdatasize = dlightdata.Count();

	//pInfo->dfaces = dfaces;
	pInfo->dfaces = dfaces.Base();
	pInfo->m_pFacesTouched = g_FacesTouched.Base();
	pInfo->numfaces = numfaces;
	
	//pInfo->dvertexes = dvertexes;
	pInfo->dvertexes = dvertexes.Base();
	pInfo->numvertexes = numvertexes;

	//pInfo->dedges = dedges;
	pInfo->dedges = dedges.Base();
	pInfo->numedges = numedges;

	//pInfo->dsurfedges = dsurfedges;
	pInfo->dsurfedges = dsurfedges.Base();
	pInfo->numsurfedges = numsurfedges;

	//pInfo->texinfo = texinfo;
	pInfo->texinfo = texinfo.Base();
	pInfo->numtexinfo = numtexinfo;

	pInfo->g_dispinfo = g_dispinfo.Base();
	pInfo->g_numdispinfo = g_dispinfo.Count();

	//pInfo->dtexdata = dtexdata;
	pInfo->dtexdata = dtexdata.Base();
	pInfo->numtexdata = numtexdata;

	pInfo->texDataStringData = g_TexDataStringData;
	pInfo->nTexDataStringData = g_nTexDataStringData;

	pInfo->texDataStringTable = g_TexDataStringTable;
	pInfo->nTexDataStringTable = g_nTexDataStringTable;
	//!} 2006-05-03	허 창 민
}


bool CVRadDLL::DoIncrementalLight( char const *pVMFFile )
{
	return false;
}


bool CVRadDLL::Serialize()
{
	//if( !g_pIncremental )
	//	return false;

	if( g_LastGoodLightData.Count() > 0 )
	{
		dlightdata.CopyArray( g_LastGoodLightData.Base(), g_LastGoodLightData.Count() );

		//if( g_pIncremental->Serialize() )
		//{
		//	// Delete this so it doesn't keep re-saving it.
		//	g_LastGoodLightData.Purge();
		//	return true;
		//}
	}

	return false;
}


float CVRadDLL::GetPercentComplete()
{
	return (float)g_iCurFace / numfaces;
}


void CVRadDLL::Interrupt()
{
	g_bInterrupt = true;
}

//!{ 2006-03-20	허 창 민
int CVRadDLL2::main( int argc, char **argv )
{
	return AVA_VRAD_Main( argc, argv, this );
}

class TempHolder : public IVRadDLL2
{
public :
	virtual int			FaceLimit() {return 0;}
	virtual int			PlaneLimit() {return 0;}
	virtual int			VertexLimit() {return 0;}
	virtual int			TextureInfoLimit() {return 0;}
	virtual int			TextureDataLimit() {return 0;}
	virtual int			LightLimit() {return 0;}
	virtual int			LightmapSizeLimit() {return 0;}
	virtual int			EdgeLimit() {return 0;}
	virtual int			SurfaceEdgeLimit() {return 0;}
	// All vrad.exe does is load the VRAD DLL and run this.
	virtual int			main( int argc, char **argv ) {return 0;}
	// Load the BSP file into memory.
	virtual bool		Init( char const *pFilename ) {return false;}
	// You must call this if you call Init(), to free resources.
	virtual void		Release() {}
	// Get some data from the BSP file that's in memory.
	virtual void		GetBSPInfo( CBSPInfo *pInfo ) {}	
	virtual bool		DoIncrementalLight( char const *pVMFFile ) {return false;}	
	virtual bool		Serialize() {return false;}	
	virtual float		GetPercentComplete() { return 0; }	
	virtual void		Interrupt() {}

	int* VertexIndices;

	TempHolder()		 
	{
		FaceArray_ = 0;
		PlaneArray_ = 0;
		VertexArray_ = 0;
		TextureInfoArray_ = 0;
		TextureDataArray_ = 0;
		PointLightArray_ = 0;
		SpotLightArray_ = 0;
		VertexIndices = 0;

		BlackMeshIndexArray_ = 0;
		BlackMeshTexDataIndexArray_ = 0;
		BlackMeshTriangleCountArray_ = 0;
		BlackMeshVertexCountArray_ = 0;
		BlackMeshSampleToAreaRatioArray_ = 0;
		BlackMeshSampleVerticesFlagArray_= 0;
		BlackMeshVertexArray_ = 0;
		BlackMeshTangentXArray_ = 0;
		BlackMeshTangentYArray_ = 0;
		BlackMeshTangentZArray_ = 0;

		AmbientCubes_ = 0;
		AmbientCubeSamplePoints_ = 0;

		DispInfos_ = 0;
		DispVertices_ = 0;
	}

	~TempHolder()
	{
		free( VertexIndices );
		free( FaceArray_ );
		free( PlaneArray_ );
		free( VertexArray_ );
		free( TextureInfoArray_ );
		free( TextureDataArray_ );
		free( PointLightArray_ );
		free( SpotLightArray_ );

		free( BlackMeshIndexArray_ );
		free( BlackMeshTexDataIndexArray_ );
		free( BlackMeshTriangleCountArray_ );
		free( BlackMeshVertexCountArray_ );
		free( BlackMeshSampleToAreaRatioArray_ );
		free( BlackMeshSampleVerticesFlagArray_);
		free( BlackMeshVertexArray_ );
		free( BlackMeshTangentXArray_);
		free( BlackMeshTangentYArray_);
		free( BlackMeshTangentZArray_);

		free( AmbientCubes_ );
		free( AmbientCubeSamplePoints_ );
	}

	template <typename T>
		void LoadVector( T*& data, int& N, FILE* fp )
	{
		size_t n;

		fread( &n, sizeof(size_t), 1, fp );
		if (!n)
		{
			data = NULL;
			N = 0;
			return;
		}
		
		N = n;

		data = (typename T*)realloc( data, sizeof(typename T) * n );
		fread( data, sizeof(typename T), n, fp  );
	}

	template <typename T>
		void Broadcast( T*& data, int& N )
	{
		int mpi_rank;		
		MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );		

		MPI_Bcast( &N, 1, MPI_INT, 0, MPI_COMM_WORLD );
		
		if (!N)
		{
			data = NULL;
			return;
		}

		if (mpi_rank != 0)
			data = (typename T*)realloc( data, sizeof(typename T) * N );

		MPI_Bcast( data, sizeof(typename T) * N, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );
	}

	inline void CalcCrc32(const BYTE byte, DWORD &dwCrc32) const
	{
		dwCrc32 = ((dwCrc32) >> 8) ^ s_arrdwCrc32Table[(byte) ^ ((dwCrc32) & 0x000000FF)];
	}

	template <typename T>
	void sum_crc32( typename T* buf, int N, DWORD& crc ) const
	{
		for (int i=0; i<N * sizeof(typename T); ++i)
		{
			CalcCrc32( ((BYTE*)buf)[i], crc );
		}
	}

	virtual bool Load( const char* szFilename )
	{
		extern CString cache_prefix;						

		int nVertices = 0;
		int mpi_rank;
		MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );				

		if (mpi_rank == 0)
		{	
			Msg( "[[PROGRESS:Loading 0 2]]\n" );

			cache_prefix = "c:\\radcache";
			DWORD old_crc = 0;
			extern bool bUseCache;

			bUseCache = false;

			if (mpi_rank == 0)
			{
				FILE* cfp = fopen( cache_prefix + ".crc", "rb" );
				if (cfp)
				{			
					bUseCache = true;
					fread( &old_crc, 4, 1, cfp );
					fclose(cfp);			
				}
			}		

			FILE* fp = fopen( szFilename, "rb" );

			if (!fp) return false;

			try
			{
				LoadVector( FaceArray_, NumFaces_, fp );
				LoadVector( PlaneArray_, NumPlanes_, fp );
				LoadVector( VertexArray_, NumVertices_, fp );			
				LoadVector( TextureDataArray_, NumTextureDatas_, fp );
				LoadVector( TextureInfoArray_, NumTextureInfos_, fp );
				LoadVector( PointLightArray_, NumPointLights_, fp );
				LoadVector( SpotLightArray_, NumSpotLights_, fp );
				fread( &SunLight_, sizeof(SunLight_), 1, fp );
				LoadVector( BlackMeshIndexArray_, NumBlackMeshIndices_, fp );
				LoadVector( BlackMeshTexDataIndexArray_, NumBlackMeshTexDataIndices_, fp );
				LoadVector( BlackMeshTriangleCountArray_, NumBlackMeshTriangleCounts_, fp );
				LoadVector( BlackMeshVertexCountArray_, NumBlackMeshVertexCounts_, fp );
				LoadVector( BlackMeshSampleToAreaRatioArray_, NumBlackMeshSampleToAreaRatioCounts_, fp );
				LoadVector( BlackMeshSampleVerticesFlagArray_, NumBlackMeshSampleVerticesFlagCounts_, fp );
				LoadVector( BlackMeshVertexArray_, NumBlackMeshVertices_, fp );
				LoadVector( BlackMeshTangentXArray_, NumBlackMeshVertices_, fp );
				LoadVector( BlackMeshTangentYArray_, NumBlackMeshVertices_, fp );
				LoadVector( BlackMeshTangentZArray_, NumBlackMeshVertices_, fp );				

				Msg( "Num Static Meshes %d\n", NumBlackMeshVertexCounts_ );

				if (mpi_rank == 0)
				{
					for (int i=0; i<NumFaces_; ++i)
					{
						FaceArray_[i].VertexIndices_ = NULL;
					}

					DWORD crc = 0xffffffff;
					sum_crc32( FaceArray_, NumFaces_, crc );
					sum_crc32( PlaneArray_, NumPlanes_, crc );
					sum_crc32( VertexArray_, NumVertices_, crc );
					sum_crc32( TextureDataArray_, NumTextureDatas_, crc );
					sum_crc32( TextureInfoArray_, NumTextureInfos_, crc );

					//sum_crc32(&SunLight_, 1, crc );

					sum_crc32( BlackMeshIndexArray_, NumBlackMeshIndices_, crc );
					sum_crc32( BlackMeshTexDataIndexArray_, NumBlackMeshTexDataIndices_, crc );
					sum_crc32( BlackMeshVertexArray_, NumBlackMeshVertices_, crc );
					sum_crc32( BlackMeshTangentXArray_, NumBlackMeshVertices_, crc );
					sum_crc32( BlackMeshTangentYArray_, NumBlackMeshVertices_, crc );
					sum_crc32( BlackMeshTangentZArray_, NumBlackMeshVertices_, crc );

					if (bUseCache && old_crc != crc)
					{
						bUseCache = false;
					}

					FILE* cfp = fopen( cache_prefix + ".crc", "wb" );
					if (cfp)
					{
						fwrite( &crc, 4, 1, cfp );
						fclose(cfp);
					}
				}			
				
				for (int i=0; i<NumFaces_; ++i)
				{
					nVertices += FaceArray_[i].NumVertices_;
				}

				VertexIndices = (int*)realloc( VertexIndices, sizeof(int) * nVertices );
				fread( VertexIndices, nVertices, sizeof(int), fp );				

				LoadVector( AmbientCubeSamplePoints_, NumAmbientCubeSamplePoints_, fp );								

				// for terrain
				LoadVector( DispInfos_, NumDispInfos_, fp );
				LoadVector( DispVertices_, NumDispVertices_, fp );
			}
			catch (...)
			{		
				fclose(fp);
				INT fail = 0;
				MPI_Bcast( &fail, 1, MPI_INT, 0, MPI_COMM_WORLD );
				return false;
			}

			fclose(fp);
		}

		INT result = 1;
		MPI_Bcast( &result, 1, MPI_INT, 0, MPI_COMM_WORLD );

		if (result != 1)
			return false;

		if (mpi_rank == 0)
			Msg( "[[PROGRESS:Loading 1 2]]\n" );
		
		Broadcast( FaceArray_, NumFaces_ );

		//Msg( "Broadcasting plane...\n" );
		Broadcast( PlaneArray_, NumPlanes_ );

		//Msg( "Broadcasting vertex...\n" );
		Broadcast( VertexArray_, NumVertices_ );			

		//Msg( "Broadcasting texdata...\n" );
		Broadcast( TextureDataArray_, NumTextureDatas_ );

		//Msg( "Broadcasting texinfo...\n" );
		Broadcast( TextureInfoArray_, NumTextureInfos_ );

		//Msg( "Broadcasting pointlight...\n" );
		Broadcast( PointLightArray_, NumPointLights_ );
		Broadcast( SpotLightArray_, NumSpotLights_ );

		//Msg( "Broadcasting sunlight...\n" );
		MPI_Bcast( &SunLight_, sizeof(SunLight_), MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD );

		//Msg( "Broadcasting bm index...\n" );
		Broadcast( BlackMeshIndexArray_, NumBlackMeshIndices_ );
		Broadcast( BlackMeshTexDataIndexArray_, NumBlackMeshTexDataIndices_ );
		Broadcast( BlackMeshTriangleCountArray_, NumBlackMeshTriangleCounts_);
		Broadcast( BlackMeshVertexCountArray_, NumBlackMeshVertexCounts_ );
		Broadcast( BlackMeshSampleToAreaRatioArray_, NumBlackMeshSampleToAreaRatioCounts_ );
		Broadcast( BlackMeshSampleVerticesFlagArray_, NumBlackMeshSampleVerticesFlagCounts_ );
		Broadcast( BlackMeshVertexArray_, NumBlackMeshVertices_ );
		Broadcast( BlackMeshTangentXArray_, NumBlackMeshVertices_ );		
		Broadcast( BlackMeshTangentYArray_, NumBlackMeshVertices_ );
		Broadcast( BlackMeshTangentZArray_, NumBlackMeshVertices_ );

		Broadcast( VertexIndices, nVertices );
		int* Indices = VertexIndices;
		for (int i=0; i<NumFaces_; ++i)
		{
			FaceArray_[i].VertexIndices_ = Indices;
			Indices += FaceArray_[i].NumVertices_;
		}

		Broadcast( AmbientCubeSamplePoints_, NumAmbientCubeSamplePoints_ );

		NumAmbientCubes_ = NumAmbientCubeSamplePoints_;
		AmbientCubes_ = (AvaAmbientCube*)calloc( NumAmbientCubes_, sizeof(AvaAmbientCube) );

		// for terrains
		Broadcast( DispInfos_, NumDispInfos_ );
		Broadcast( DispVertices_, NumDispVertices_ );

		if (mpi_rank == 0)
		{
			Msg( "[[PROGRESS:Loading 2 2]]\n" );
		}
		return true;
	}

	template <typename T>
		void SaveVector( const CUtlVector<typename T>& data, FILE* fp )
	{
		size_t N = data.Count();

		fwrite( &N, 1, sizeof(size_t), fp );
		if (!N) return;

		fwrite( data.Base(), N, sizeof(typename T), fp );
	}

	template <typename T>
		void SaveVector( const typename T* data, int n, FILE* fp )
	{
		size_t N = n;

		fwrite( &N, 1, sizeof(size_t), fp );
		if (!N) return;

		fwrite( data, N, sizeof(typename T), fp );
	}

	virtual bool Save( const char* szFilename )
	{
		FILE* fp = fopen( szFilename, "wb" );

		if (!fp) return false;

		SaveVector( dlightdata, fp );

		SaveVector( dlightdata2, fp );

		SaveVector( dfaces, fp );

		SaveVector( blackmesh_worldvertexlightdata, fp );	// with sun

		SaveVector( blackmesh_vertexlightdata, fp );		// without sun

		SaveVector( AmbientCubes_, NumAmbientCubes_, fp );

		//!{ 2006-06-30	허 창 민
		SaveVector( dshadowdata, fp );
		//!} 2006-06-30	허 창 민

		//<@ 2007. 11. 12 changmin
		// add sun visibility for static meshes
		SaveVector( sun_visibility_array, fp );
		//>@ 

		int hdrSky = 0;

		extern directlight_t *gAmbient;

		if (gAmbient && gAmbient->light.type == emit_hdrEnvmapSkylight)
		{
			hdrSky = 1;

			Pbrt::theta += M_PI;

			Pbrt::theta *= 180 / M_PI;
			Pbrt::phi *= 180 / M_PI;

			fwrite( &hdrSky, 1, sizeof(int), fp );						
			fwrite( &Pbrt::phi, 1, sizeof(float), fp );
			fwrite( &Pbrt::theta, 1, sizeof(float), fp );

			extern Vector GSunDirection;
			extern LinearColor GSunColor;

			fwrite( &GSunDirection, 1, sizeof(GSunDirection), fp );
			fwrite( &GSunColor, 1, sizeof(GSunColor), fp );
		}
		else
			fwrite( &hdrSky, 1, sizeof(hdrSky), fp );		

		fclose(fp);

		extern CString cache_prefix;

		cache_prefix = "c:\\radcache";

		return true;
	}
};

#include <tlhelp32.h>
//#include "dbghelp.h"

//#define DEBUG_DPRINTF		1	//allow d()
//#include "wfun.h"

#pragma optimize("y", off)		//generate stack frame pointers for all functions - same as /Oy- in the project
#pragma warning(disable: 4200)	//nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4100)	//unreferenced formal parameter

// In case you don't have dbghelp.h.
#ifndef _DBGHELP_

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
	DWORD	ThreadId;
	PEXCEPTION_POINTERS	ExceptionPointers;
	BOOL	ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef enum _MINIDUMP_TYPE {
	MiniDumpNormal =			0x00000000,
	MiniDumpWithDataSegs =		0x00000001,
} MINIDUMP_TYPE;

typedef	BOOL (WINAPI * MINIDUMP_WRITE_DUMP)(
	IN HANDLE			hProcess,
	IN DWORD			ProcessId,
	IN HANDLE			hFile,
	IN MINIDUMP_TYPE	DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
	IN PVOID									UserStreamParam, OPTIONAL
	IN PVOID									CallbackParam OPTIONAL
	);

#else

typedef	BOOL (WINAPI * MINIDUMP_WRITE_DUMP)(
	IN HANDLE			hProcess,
	IN DWORD			ProcessId,
	IN HANDLE			hFile,
	IN MINIDUMP_TYPE	DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
	IN PMINIDUMP_USER_STREAM_INFORMATION		UserStreamParam, OPTIONAL
	IN PMINIDUMP_CALLBACK_INFORMATION			CallbackParam OPTIONAL
	);
#endif //#ifndef _DBGHELP_

HMODULE	hDbgHelp;
MINIDUMP_WRITE_DUMP	MiniDumpWriteDump_;

// Tool Help functions.
typedef	HANDLE (WINAPI * CREATE_TOOL_HELP32_SNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef	BOOL (WINAPI * MODULE32_FIRST)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef	BOOL (WINAPI * MODULE32_NEST)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

CREATE_TOOL_HELP32_SNAPSHOT	CreateToolhelp32Snapshot_;
MODULE32_FIRST	Module32First_;
MODULE32_NEST	Module32Next_;

#define	DUMP_SIZE_MAX	8000	//max size of our dump
#define	CALL_TRACE_MAX	((DUMP_SIZE_MAX - 2000) / (MAX_PATH + 40))	//max number of traced calls
#define	NL				"\r\n"	//new line

//****************************************************************************************
BOOL WINAPI Get_Module_By_Ret_Addr(PBYTE Ret_Addr, PCHAR Module_Name, PBYTE & Module_Addr)
//****************************************************************************************
// Find module by Ret_Addr (address in the module).
// Return Module_Name (full path) and Module_Addr (start address).
// Return TRUE if found.
{
	MODULEENTRY32	M = {sizeof(M)};
	HANDLE	hSnapshot;

	Module_Name[0] = 0;

	if (CreateToolhelp32Snapshot_)
	{
		hSnapshot = CreateToolhelp32Snapshot_(TH32CS_SNAPMODULE, 0);

		if ((hSnapshot != INVALID_HANDLE_VALUE) &&
			Module32First_(hSnapshot, &M))
		{
			do
			{
				if (DWORD(Ret_Addr - M.modBaseAddr) < M.modBaseSize)
				{
					lstrcpyn(Module_Name, M.szExePath, MAX_PATH);
					Module_Addr = M.modBaseAddr;
					break;
				}
			} while (Module32Next_(hSnapshot, &M));
		}

		CloseHandle(hSnapshot);
	}

	return !!Module_Name[0];
} //Get_Module_By_Ret_Addr

//******************************************************************
int WINAPI Get_Call_Stack(PEXCEPTION_POINTERS pException, PCHAR Str)
//******************************************************************
// Fill Str with call stack info.
// pException can be either GetExceptionInformation() or NULL.
// If pException = NULL - get current call stack.
{
	CHAR	Module_Name[MAX_PATH];
	PBYTE	Module_Addr = 0;
	PBYTE	Module_Addr_1;
	int		Str_Len;

	typedef struct STACK
	{
		STACK *	Ebp;
		PBYTE	Ret_Addr;
		DWORD	Param[0];
	} STACK, * PSTACK;

	STACK	Stack = {0, 0};
	PSTACK	Ebp;

	if (pException)		//fake frame for exception address
	{
		Stack.Ebp = (PSTACK)pException->ContextRecord->Ebp;
		Stack.Ret_Addr = (PBYTE)pException->ExceptionRecord->ExceptionAddress;
		Ebp = &Stack;
	}
	else
	{
		Ebp = (PSTACK)&pException - 1;	//frame addr of Get_Call_Stack()

		// Skip frame of Get_Call_Stack().
		if (!IsBadReadPtr(Ebp, sizeof(PSTACK)))
			Ebp = Ebp->Ebp;		//caller ebp
	}

	Str[0] = 0;
	Str_Len = 0;

	// Trace CALL_TRACE_MAX calls maximum - not to exceed DUMP_SIZE_MAX.
	// Break trace on wrong stack frame.
	for (int Ret_Addr_I = 0;
		(Ret_Addr_I < CALL_TRACE_MAX) && !IsBadReadPtr(Ebp, sizeof(PSTACK)) && !IsBadCodePtr(FARPROC(Ebp->Ret_Addr));
		Ret_Addr_I++, Ebp = Ebp->Ebp)
	{
		// If module with Ebp->Ret_Addr found.
		if (Get_Module_By_Ret_Addr(Ebp->Ret_Addr, Module_Name, Module_Addr_1))
		{
			if (Module_Addr_1 != Module_Addr)	//new module
			{
				// Save module's address and full path.
				Module_Addr = Module_Addr_1;
				Str_Len += wsprintf(Str + Str_Len, NL "%08X  %s", Module_Addr, Module_Name);
			}

			// Save call offset.
			Str_Len += wsprintf(Str + Str_Len,
				NL "  +%08X", Ebp->Ret_Addr - Module_Addr);

			// Save 5 params of the call. We don't know the real number of params.
			if (pException && !Ret_Addr_I)	//fake frame for exception address
				Str_Len += wsprintf(Str + Str_Len, "  Exception Offset");
			else if (!IsBadReadPtr(Ebp, sizeof(PSTACK) + 5 * sizeof(DWORD)))
			{
				Str_Len += wsprintf(Str + Str_Len, "  (%X, %X, %X, %X, %X)",
					Ebp->Param[0], Ebp->Param[1], Ebp->Param[2], Ebp->Param[3], Ebp->Param[4]);
			}
		}
		else
			Str_Len += wsprintf(Str + Str_Len, NL "%08X", Ebp->Ret_Addr);
	}

	return Str_Len;
} //Get_Call_Stack

//***********************************
int WINAPI Get_Version_Str(PCHAR Str)
//***********************************
// Fill Str with Windows version.
{
	OSVERSIONINFOEX	V = {sizeof(OSVERSIONINFOEX)};	//EX for NT 5.0 and later

	if (!GetVersionEx((POSVERSIONINFO)&V))
	{
		ZeroMemory(&V, sizeof(V));
		V.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((POSVERSIONINFO)&V);
	}

	if (V.dwPlatformId != VER_PLATFORM_WIN32_NT)
		V.dwBuildNumber = LOWORD(V.dwBuildNumber);	//for 9x HIWORD(dwBuildNumber) = 0x04xx

	return wsprintf(Str,
		NL "Windows:  %d.%d.%d, SP %d.%d, Product Type %d",	//SP - service pack, Product Type - VER_NT_WORKSTATION,...
		V.dwMajorVersion, V.dwMinorVersion, V.dwBuildNumber, V.wServicePackMajor, V.wServicePackMinor, V.wProductType);
} //Get_Version_Str

//*************************************************************
PCHAR WINAPI Get_Exception_Info(PEXCEPTION_POINTERS pException)
//*************************************************************
// Allocate Str[DUMP_SIZE_MAX] and return Str with dump, if !pException - just return call stack in Str.
{
	PCHAR		Str;
	int			Str_Len;
	int			i;
	CHAR		Module_Name[MAX_PATH];
	PBYTE		Module_Addr;
	HANDLE		hFile;
	FILETIME	Last_Write_Time;
	FILETIME	Local_File_Time;
	SYSTEMTIME	T;

	Str = new CHAR[DUMP_SIZE_MAX];

	if (!Str)
		return NULL;

	Str_Len = 0;
	Str_Len += Get_Version_Str(Str + Str_Len);

	Str_Len += wsprintf(Str + Str_Len, NL "Process:  ");
	GetModuleFileName(NULL, Str + Str_Len, MAX_PATH);
	Str_Len = lstrlen(Str);

	// If exception occurred.
	if (pException)
	{
		EXCEPTION_RECORD &	E = *pException->ExceptionRecord;
		CONTEXT &			C = *pException->ContextRecord;

		// If module with E.ExceptionAddress found - save its path and date.
		if (Get_Module_By_Ret_Addr((PBYTE)E.ExceptionAddress, Module_Name, Module_Addr))
		{
			Str_Len += wsprintf(Str + Str_Len,
				NL "Module:  %s", Module_Name);

			if ((hFile = CreateFile(Module_Name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
			{
				if (GetFileTime(hFile, NULL, NULL, &Last_Write_Time))
				{
					FileTimeToLocalFileTime(&Last_Write_Time, &Local_File_Time);
					FileTimeToSystemTime(&Local_File_Time, &T);

					Str_Len += wsprintf(Str + Str_Len,
						NL "Date Modified:  %02d/%02d/%d",
						T.wMonth, T.wDay, T.wYear);
				}
				CloseHandle(hFile);
			}
		}
		else
		{
			Str_Len += wsprintf(Str + Str_Len,
				NL "Exception Addr:  %08X", E.ExceptionAddress);
		}

		Str_Len += wsprintf(Str + Str_Len,
			NL "Exception Code:  %08X", E.ExceptionCode);

		if (E.ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			// Access violation type - Write/Read.
			Str_Len += wsprintf(Str + Str_Len,
				NL "%s Address:  %08X",
				(E.ExceptionInformation[0]) ? "Write" : "Read", E.ExceptionInformation[1]);
		}

		// Save instruction that caused exception.
		Str_Len += wsprintf(Str + Str_Len, NL "Instruction: ");
		for (i = 0; i < 16; i++)
			Str_Len += wsprintf(Str + Str_Len, " %02X", PBYTE(E.ExceptionAddress)[i]);

		// Save registers at exception.
		Str_Len += wsprintf(Str + Str_Len, NL "Registers:");
		Str_Len += wsprintf(Str + Str_Len, NL "EAX: %08X  EBX: %08X  ECX: %08X  EDX: %08X", C.Eax, C.Ebx, C.Ecx, C.Edx);
		Str_Len += wsprintf(Str + Str_Len, NL "ESI: %08X  EDI: %08X  ESP: %08X  EBP: %08X", C.Esi, C.Edi, C.Esp, C.Ebp);
		Str_Len += wsprintf(Str + Str_Len, NL "EIP: %08X  EFlags: %08X", C.Eip, C.EFlags);
	} //if (pException)

	// Save call stack info.
	Str_Len += wsprintf(Str + Str_Len, NL "Call Stack:");
	Get_Call_Stack(pException, Str + Str_Len);

	if (Str[0] == NL[0])
		lstrcpy(Str, Str + sizeof(NL) - 1);

	return Str;
} //Get_Exception_Info


extern "C"
{
	LONG MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS* ExceptionInfo )
	{
		//  계속해서 handler가 실행되는 모양인데.
		//  최초의 한번만 덤프하자.
		static bool already = false;
		if ( !already )
		{
			char computerName[256];
			int len = 256;

			MPI_Get_processor_name( computerName, &len );
			printf( "**** Crash **** %s\n", computerName );

			printf( Get_Exception_Info(ExceptionInfo) );			

			printf( "\n" );

			for (int i=0; i<sizeof(spy_info)/sizeof(spy_info[0]); ++i)
			{
				printf( "thread %d - ", i );
				for (int j=0; j<sizeof(spy_info[0])/sizeof(spy_info[0][0]); ++j)
				{
					printf( "%4d", spy_info[i][j] );
				}

				printf( "\n" );				
			}
			fflush(stdout);

			//  let's mini dump
			CString dt = CTime::GetCurrentTime().Format( TEXT("%Y%m%d_%H%M%S") );
			CString filename;
			filename.Format( TEXT("\\\\avabuild\\avacontext\\rad_mpi\\crash\\styxrad_crash_%s"), dt.GetString() );
			filename.Replace( '.', '_' );
			filename.Append( TEXT(".dmp") );

			HANDLE dumpfile = CreateFile( filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

			MINIDUMP_EXCEPTION_INFORMATION einfo;

			einfo.ThreadId = GetCurrentThreadId();
			einfo.ExceptionPointers = ExceptionInfo;
			einfo.ClientPointers = TRUE;

			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				dumpfile,
				MiniDumpWithDataSegs,
				&einfo, 0, 0 );

			CloseHandle( dumpfile );

			already = true;
		}


		return EXCEPTION_EXECUTE_HANDLER;
	}
}

int main( int argc, char** argv )
{
	HMODULE	hKernel32;
	CHAR	Buf[0x11];
	CHAR	Str[sizeof(Buf)] = "abc";	

	// Try to get MiniDumpWriteDump() address.
	hDbgHelp = LoadLibrary("DBGHELP.DLL");
	MiniDumpWriteDump_ = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	//	d("hDbgHelp=%X, MiniDumpWriteDump_=%X", hDbgHelp, MiniDumpWriteDump_);

	// Try to get Tool Help library functions.
	hKernel32 = GetModuleHandle("KERNEL32");
	CreateToolhelp32Snapshot_ = (CREATE_TOOL_HELP32_SNAPSHOT)GetProcAddress(hKernel32, "CreateToolhelp32Snapshot");
	Module32First_ = (MODULE32_FIRST)GetProcAddress(hKernel32, "Module32First");
	Module32Next_ = (MODULE32_NEST)GetProcAddress(hKernel32, "Module32Next");

	memset( spy_info, 0, sizeof(spy_info) );

	SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER) MyUnhandledExceptionFilter );

	// plugin과 교신할 네트워크 라이브러리를 초기화하기 전에, XP SP2의 경우 Firewall에 등록한다
	{
		CoInitialize( NULL );

		WinXPSP2FireWall fw;
		if( fw.Initialize() == FW_NOERROR )
		{			
			USES_CONVERSION;

			fw.AddApplication( A2W(argv[0]), L"Ava UnrealEngine3 Radiosity simulator");
		}
	}

	MPI::Init(argc,argv);
	
	mpi_size = MPI::COMM_WORLD.Get_size();
	mpi_rank = MPI::COMM_WORLD.Get_rank();

	Msg( "Hello from MPI %d/%d\n", mpi_rank, mpi_size );

	MPI::COMM_WORLD.Barrier();

	g_bUseMPI = (mpi_size > 1);
	g_bMPIMaster = mpi_size > 1 && mpi_rank == 0;		

	TempHolder t;

	int result = AVA_VRAD_Main( argc, argv, &t );	

	MPI::COMM_WORLD.Barrier();

	MPI::Finalize();

	CmdLib_Cleanup();

	SetUnhandledExceptionFilter( 0 );

	CoUninitialize();

	return result;
}

bool CVRadDLL2::Init( char const *pFilename )
{

	return false;
}

void CVRadDLL2::Release()
{
}


void CVRadDLL2::GetBSPInfo( CBSPInfo *pInfo )
{
	//!{ 2006-05-03	허 창 민
	pInfo->dlightdata = dlightdata.Base();
	pInfo->lightdatasize = dlightdata.Count();

	//pInfo->dfaces = dfaces;
	pInfo->dfaces = dfaces.Base();
	pInfo->m_pFacesTouched = g_FacesTouched.Base();
	pInfo->numfaces = numfaces;

	//pInfo->dvertexes = dvertexes;
	pInfo->dvertexes = dvertexes.Base();
	pInfo->numvertexes = numvertexes;

	//pInfo->dedges = dedges;
	pInfo->dedges = dedges.Base();
	pInfo->numedges = numedges;

	//pInfo->dsurfedges = dsurfedges;
	pInfo->dsurfedges = dsurfedges.Base();
	pInfo->numsurfedges = numsurfedges;

	//pInfo->texinfo = texinfo;
	pInfo->texinfo = texinfo.Base();
	pInfo->numtexinfo = numtexinfo;

	pInfo->g_dispinfo = g_dispinfo.Base();
	pInfo->g_numdispinfo = g_dispinfo.Count();

	//pInfo->dtexdata = dtexdata;
	pInfo->dtexdata = dtexdata.Base();
	pInfo->numtexdata = numtexdata;

	pInfo->texDataStringData = g_TexDataStringData;
	pInfo->nTexDataStringData = g_nTexDataStringData;

	pInfo->texDataStringTable = g_TexDataStringTable;
	pInfo->nTexDataStringTable = g_nTexDataStringTable;
	//!} 2006-05-03	허 창 민
}


bool CVRadDLL2::DoIncrementalLight( char const *pVMFFile )
{
	return false;
}


bool CVRadDLL2::Serialize()
{
	//if( !g_pIncremental )
	//	return false;

	if( g_LastGoodLightData.Count() > 0 )
	{
		dlightdata.CopyArray( g_LastGoodLightData.Base(), g_LastGoodLightData.Count() );

		//if( g_pIncremental->Serialize() )
		//{
		//	// Delete this so it doesn't keep re-saving it.
		//	g_LastGoodLightData.Purge();
		//	return true;
		//}
	}

	return false;
}


float CVRadDLL2::GetPercentComplete()
{
	return (float)g_iCurFace / numfaces;
}


void CVRadDLL2::Interrupt()
{
	g_bInterrupt = true;
}

//!{ 2006-04-26	허 창 민
// limit 정보들
int CVRadDLL2::FaceLimit()
{
	return MAX_MAP_FACES;
}

int CVRadDLL2::PlaneLimit()
{
	return MAX_MAP_PLANES;
}

int CVRadDLL2::VertexLimit()
{
	return MAX_MAP_VERTS;
}

int CVRadDLL2::TextureInfoLimit()
{
	return MAX_MAP_TEXINFO;
}

int CVRadDLL2::TextureDataLimit()
{
	return MAX_MAP_TEXDATA;
}

int CVRadDLL2::LightLimit()
{
	return MAX_MAP_WORLDLIGHTS;
}

int CVRadDLL2::LightmapSizeLimit()
{
	return MAX_LIGHTMAP_DIM_WITHOUT_BORDER;
}

int CVRadDLL2::EdgeLimit()
{
	return MAX_MAP_EDGES;
}

int CVRadDLL2::SurfaceEdgeLimit()
{
	return MAX_MAP_SURFEDGES;
}
//!} 2006-03-20	허 창 민