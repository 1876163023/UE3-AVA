// vrad_launcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <direct.h>
#include "vstdlib/strtools.h"
#include "vstdlib/icommandline.h"

//!{ 2006-03-23	허 창 민
#include "tgawriter.h"
#include "colorspace.h"
#include "vector4d.h"
#include "mathlib.h"
#include "cmdlib.h"

#include "ivrad_launcher.h"
#include <windows.h>
//!} 2006-03-23	허 창 민

static CString DataRoot("c:\\");
static CString Root("\\\\avabuild\\avacontext\\rad_mpi\\");
static CString MPIExec( "\"c:\\program files\\mpich2\\bin\\mpiexec.exe\"" );
static CString VRadExe("c:\\avarad\\VRad.exe");
static CString VisExe("c:\\avarad\\q3map.exe");
static CString VRadDevExe("c:\\avarad\\VRad_dev.exe");

// Get computer name.  NOTE: Only one return value is valid at a time!
const TCHAR* appComputerName()
{
	static TCHAR Result[256]=TEXT("");
	if( !Result[0] )
	{
		DWORD Size=sizeof(Result)/sizeof(Result[0]);
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR ACh[ARRAY_COUNT(Result)];
			GetComputerNameA( ACh, &Size );
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, Result, ARRAY_COUNT(Result) );
		}
		else
#endif
		{
			GetComputerName( Result, &Size );
		}		
	}
	return Result;
}

// Get user name.  NOTE: Only one return value is valid at a time!
const TCHAR* appUserName()
{
	static TCHAR Result[256]=TEXT("");
	if( !Result[0] )
	{
		DWORD Size=sizeof(Result)/sizeof(Result[0]);
#if UNICODE
		if( GUnicode && !GUnicodeOS )
		{
			ANSICHAR ACh[ARRAY_COUNT(Result)];
			GetUserNameA( ACh, &Size );
			MultiByteToWideChar( CP_ACP, 0, ACh, -1, Result, ARRAY_COUNT(Result) );
		}
		else
#endif
		{			
			GetUserName( Result, &Size );
		}				
	}
	return Result;
}

void *appCreateProc( const TCHAR* URL, const TCHAR* Parms )
{
	//debugf( NAME_Log, TEXT("CreateProc %s %s"), URL, Parms );

	TCHAR CommandLine[1024];
	sprintf( CommandLine, TEXT("%s %s"), URL, Parms );

	PROCESS_INFORMATION ProcInfo;
	SECURITY_ATTRIBUTES Attr;
	Attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	Attr.lpSecurityDescriptor = NULL;
	Attr.bInheritHandle = TRUE;

#if UNICODE
	if( GUnicode && !GUnicodeOS )
	{
		STARTUPINFOA StartupInfoA = { sizeof(STARTUPINFO), NULL, NULL, NULL,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL, SW_HIDE, NULL, NULL,
			NULL, NULL, NULL };
		if( !CreateProcessA( NULL, TCHAR_TO_ANSI(CommandLine), &Attr, &Attr, TRUE, DETACHED_PROCESS | REALTIME_PRIORITY_CLASS,
			NULL, NULL, &StartupInfoA, &ProcInfo ) )
			return NULL;
	}
	else
#endif
	{
		STARTUPINFO StartupInfo = { sizeof(STARTUPINFO), NULL, NULL, NULL,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL, SW_HIDE, NULL, NULL,
			NULL, NULL, NULL };
		if( !CreateProcess( NULL, CommandLine, &Attr, &Attr, TRUE, DETACHED_PROCESS | REALTIME_PRIORITY_CLASS,
			NULL, NULL, &StartupInfo, &ProcInfo ) )
			return NULL;
	}
	return (void*)ProcInfo.hProcess;
}

char* GetLastErrorString()
{
	static char err[2048];

	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);

	strncpy( err, (char*)lpMsgBuf, sizeof( err ) );
	LocalFree( lpMsgBuf );

	err[ sizeof( err ) - 1 ] = 0;

	return err;
}


void MakeFullPath( const char *pIn, char *pOut, int outLen )
{
	if ( pIn[0] == '/' || pIn[0] == '\\' || pIn[1] == ':' )
	{
		// It's already a full path.
		Q_strncpy( pOut, pIn, outLen );
	}
	else
	{
		_getcwd( pOut, outLen );
		Q_strncat( pOut, "\\", outLen );
		Q_strncat( pOut, pIn, outLen );
	}
}


void StripFilename( char *pFilename )
{
	char *pLastSlash = 0;
	while ( *pFilename )
	{
		if ( *pFilename == '/' || *pFilename == '\\' )
			pLastSlash = pFilename;

		++pFilename;
	}
	if ( pLastSlash )
		*pLastSlash = 0;
}

//!{ 2006-03-31	허 창 민

#include <vector>

using std::vector;

class VRadLauncher : public IVRadLauncher
{
public:
	INT NumGrid_;
	int bHasHDRSunInfo;
	FHDRSunInfo HDRSunInfo;
	float HdrTonemapScale_;

	VRadLauncher() : Module_( NULL ), VRad_( NULL ), Options_(0), NumGrid_(1), bHasHDRSunInfo( 0 ), HdrTonemapScale_(0)
	{}
	~VRadLauncher()
	{
		/*if( Module_ )
		{
			Sys_UnloadModule( Module_ );
		}*/
	}
	virtual bool Initialize();

	bool WriteInputData( const char* szFilename );
	bool ReadOutputData( const char* szFilename );


	// index buffer
	virtual void SetIndexBuffer( int NumIndices, int* Indices );
	virtual void AddIndex( int Index );

	// 1 vertex = 3d vector = float x, y, z
	virtual void SetVertexBuffer( int NumVertices, float* Vertices );
	virtual void AddVertex( float X, float Y, float Z );

	// 1 plane = 4d vector = < plane normal , distance = n dot p0 >
	virtual void SetPlaneBuffer( int NumPlanes, float* Planes );
	virtual void AddPlane( float X, float Y, float Z, float D );

	virtual void AddFace( int IndexBufferOffset, int NumVertices,
		unsigned short iPlane,
		unsigned short iTextureInfo,
		int iDispInfo,
		float Emissive_X, float Emissive_Y, float Emissive_Z,
		int LightmapWidth, int LightmapHeight );

	virtual void AddTextureInfo(
		float TexturemapU_X, float TexturemapU_Y, float TexturemapU_Z,
		float TexturemapV_X, float TexturemapV_Y, float TexturemapV_Z,

		float LightmapU_X, float LightmapU_Y, float LightmapU_Z, float LightmapU_Offset,
		float LightmapV_X, float LightmapV_Y, float LightmapV_Z, float LightmapV_Offset,

		float TexelInWorldUnit,
		float LuxelInWorldUnit,

		int iTextureData,
		int Flags);

	virtual void AddTextureData( int Width, int Height,
		float Reflectivity_X, float Reflectivity_Y, float Reflectivity_Z,
		float Brightness_X, float Brightness_Y, float Brightness_Z,
		float ReflectivityScale );

	virtual void AddPointLight( float Pos_X, float Pos_Y, float Pos_Z,
								float Intensity_X, float Intensity_Y, float Intensity_Z,
								float Radius, float Exponent,
								bool bLightMapLight, INT LightId);

	virtual void AddSpotLight(float Pos_X, float Pos_Y, float Pos_Z,
		float Intensity_X, float Intensity_Y, float Intensity_Z,
		float Radius, float Exponent,
		float Direction_X, float Direction_Y, float Direction_Z, 
		float InnerConeAngle, float OuterConeAngle,
		bool bLightMapLight, int LightId);

	virtual bool Run( IVRadCallback* );

	virtual void GetLightmap( INT iFace, int* Width, int* Height, unsigned char** Lightmap );

	virtual void GetLightmapWithoutSun( INT iFace, int* Width, int* Height, unsigned char** Lightmap );

	virtual int GetFaceSunVisibility( INT iFace );

	virtual void GetNegativeLightmap( INT iFace, int* Width, int* Height, unsigned char** Lightmap );

	virtual void GetShadowmapCount( INT iFace, int* Count );

	virtual void GetShadowmap( INT iFace, INT ShadowIndex, int* ExportLightId, int* Width, int* Height, unsigned char** Shadowmap );

	virtual void Finalize();

	virtual void SetOptions( int options, int num_grid );
	virtual void AddCluster( const char* szHost )
	{
		Clusters_.push_back( szHost );
	}

	void SetBlackMeshVertexBuffer( int NumVertices, float* Vertices );
	void SetBlackMeshIndexBuffer( int NumIndices, int* Indices );
	void SetBlackMeshTexDataIndexBuffer( int NumIndices, int *Indices );
	void SetBlackMeshTriangleCounts( int NumTriangleCount, int *Counts );
	void SetBlackMeshVertexCounts( int NumVertexCount, int *Counts );
	void SetBlackMeshSampleToAreaRatios( int NumRatios, float *ratios );
	void SetBlackMeshSampleVerticesFlags( int NumFlags, int *flags );
	void SetBlackMeshVertexTangentBasisBuffer( int NumVertices, float* TangentX, float* TangentY, float* TangentZ );

	void GetBlackMeshVertexLightMap( unsigned char** VertexLightMap );
	void GetBlackMeshVertexLightMapWithoutSun( unsigned char** VertexLightMap );

	void GetBlackMeshSunVisibility( int** Visibility );

	// internal use
	/*bool LoadModule();*/
	void InitData();
	void DumpLightmaps();
	bool CheckVRadLimits();

	int Execute( IVRadCallback* callback, LPCTSTR szCmd, LPCTSTR pszCmdLine );

	virtual FHDRSunInfo* GetHDRSunInfo()
	{
		if (bHasHDRSunInfo)
		{
			return &HDRSunInfo;
		}
		else
		{
			return NULL;
		}
	}

	virtual void SetSunLight_( float X, float Y, float Z, float Intensity_X, float Intensity_Y, float Intensity_Z, float Ambient_X, float Ambient_Y, float Ambient_Z, bool bLightMapLight, INT LightId,
		const char* SkyEnvMapFileName, float SkyMax, float SkyScale, float Yaw, float HDRTonemapScale ) 
	{
		SunLight_.Direction_.x = X;
		SunLight_.Direction_.y = Y;
		SunLight_.Direction_.z = Z;

		SunLight_.Intensity_.x = Intensity_X;
		SunLight_.Intensity_.y = Intensity_Y;
		SunLight_.Intensity_.z = Intensity_Z;

		SunLight_.Ambient_.x = Ambient_X;
		SunLight_.Ambient_.y = Ambient_Y;
		SunLight_.Ambient_.z = Ambient_Z;

		SunLight_.bLightMapLight_ = bLightMapLight;
		SunLight_.LightId_ = LightId;

		SunLight_.SkyMax_	= SkyMax;
		SunLight_.SkyScale_ = SkyScale;
		SunLight_.Yaw_		= Yaw;		

		if( SkyEnvMapFileName )
		{
			strcpy( SunLight_.SkyEnvMapFileName_, SkyEnvMapFileName );
		}
		else
		{
			SunLight_.SkyEnvMapFileName_[0] = '\0';
		}

		HdrTonemapScale_ = HDRTonemapScale;
	}

	virtual void SetAmbientCubePoints( int nPoints, const float* data ) 
	{
		if (nPoints)
			AmbientCubeSamplePoints_.assign( (Vector*)data, (Vector*)data + nPoints );
		else
			AmbientCubeSamplePoints_.resize(0);
	}

	virtual const float* GetAmbientCubes() 
	{
		return AmbientCubes_.empty() ? NULL : (float*)&AmbientCubes_[0];
	}

	virtual void AddDispInfo(
		float StartPosX, float StartPosY, float StartPosZ,
		INT DispVertexStart,
		INT Power,
		unsigned short NeighborIndices[4],
		unsigned char NeighborOrientations[4],
		unsigned char Spans[4],
		unsigned char NeighbrSpans[4] );

	virtual void AddDispVertex( float DispVecX, float DispVecY, float DispVecZ, float DispDistance, float Alpha );

	virtual bool RunPVS( IVRadCallback* );

private:
	CSysModule*	Module_;
	IVRadDLL2*	VRad_;

	int			NumVertices_;
	int			NumIndices_;

	int			NumFaces_;
	int			NumPlanes_;
	int			NumTextureInfos_;
	int			NumTextureDatas_;

	int			NumEdges_;

	int			Options_;


	vector<int>				IndexBuffer_;
	vector<AvaVertex>		VertexBuffer_;
	vector<AvaFace>			FaceList_;
	vector<AvaPlane>		PlaneBuffer_;
	vector<AvaTextureInfo>	TextureInfoBuffer_;
	vector<AvaTextureData>	TextureDataBuffer_;

	vector<AvaPointLight>	PointLights_;
	vector<AvaSpotLight>	SpotLights_;

	vector<dface_t>			dfaces;	
	vector<byte>			dlightdata;
	vector<byte>			dlightdata2;
	vector<byte>			dshadowdata;
	vector<int>				sunVisibility_;

	AvaSunLight				SunLight_;

	// black mesh 는 triangle list이다.
	vector<AvaVertex>		BlackMeshVertexBuffer_;
	vector<int>				BlackMeshIndexBuffer_;	
	vector<int>				BlackMeshTexDataIndexBuffer_;
	vector<int>				BlackMeshTriangleCounts_;
	vector<int>				BlackMeshVertexCounts_;
	vector<float>			BlackMeshSampleToAreaRatios_;
	vector<int>				BlackMeshSampleVerticesFlags_;

	vector<AvaVertex>		BlackMeshVertexTangentXBuffer_;
	vector<AvaVertex>		BlackMeshVertexTangentYBuffer_;
	vector<AvaVertex>		BlackMeshVertexTangentZBuffer_;
	
	vector<byte>			blackmesh_vertexlightdata;				
	vector<byte>			blackmesh_vertexlightdata_without_sun;	

	vector<CString>			Clusters_;
	vector<Vector>			AmbientCubeSamplePoints_;
	vector<AvaAmbientCube>	AmbientCubes_;

	vector<AvaDispInfo>		DispInfoBuffer_;
	vector<AvaDispVertex>	DispVertexBuffer_;

	void AcquireClusters( bool bAcquire )
	{
		SYSTEMTIME		now;
		FILETIME		tsUserTime;
		if (bAcquire)
		{
			GetSystemTime(&now);
			if(!SystemTimeToFileTime(&now, &tsUserTime)) 
			{			
				return;
			}
		}
		for (INT i=0; i<Clusters_.size(); ++i)
		{
			CString filename = Root + "occupied_clusters\\" + Clusters_[i];
			if (bAcquire)
			{
				HANDLE h = CreateFile( filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,0 );
				if (h != INVALID_HANDLE_VALUE)
				{
					SetFileTime( h, &tsUserTime, &tsUserTime, &tsUserTime );
					CloseHandle( h );
				}				
			}
			else
			{
				DeleteFile( filename );
			}
		}
	}
};

// singleton object.
VRadLauncher s_launcher;

// export object.
extern "C" VRAD_LAUNCHER_API IVRadLauncher* GetLauncher()
{
	return (IVRadLauncher*) &s_launcher;
}

void PrintMsg( char const* pMsgFormat, ... )
{
	va_list args;
	va_start( args, pMsgFormat );
	char pTempBuffer[1024];
	/* Printf the file and line for warning + assert only... */
	int len = 0;
	/* Create the message.... */
	len = vsprintf( &pTempBuffer[len], pMsgFormat, args );
	OutputDebugString( pTempBuffer );
	va_end(args);
}

void VRadLauncher::InitData()
{
	NumVertices_ = 0;
	NumIndices_ = 0;
	NumFaces_ = 0;
	NumPlanes_ = 0;
	NumTextureInfos_ = 0;
	NumTextureDatas_ = 0;
	NumEdges_ = 0;
	Options_ = 0;
	IndexBuffer_.clear();
	VertexBuffer_.clear();
	FaceList_.clear();
	PlaneBuffer_.clear();
	TextureInfoBuffer_.clear();
	TextureDataBuffer_.clear();
	PointLights_.clear();
	SpotLights_.clear();
	AmbientCubes_.clear();
	AmbientCubeSamplePoints_.clear();
	BlackMeshIndexBuffer_.clear();
	BlackMeshTexDataIndexBuffer_.clear();
	BlackMeshVertexBuffer_.clear();
	BlackMeshTriangleCounts_.clear();
	BlackMeshVertexCounts_.clear();
	memset( &SunLight_, 0, sizeof(SunLight_) );
}

bool VRadLauncher::Initialize()
{
	InitData();
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f, false, false, false, false );
	return true;
}

void VRadLauncher::SetIndexBuffer( int NumIndices, int* Indices )
{
	NumIndices_ = NumIndices;
	IndexBuffer_.resize(NumIndices);

	if (NumIndices)
		memcpy( (void*) &IndexBuffer_[0], (const void*) Indices, sizeof( int ) * NumIndices );
}

void VRadLauncher::AddIndex( int Index )
{
	IndexBuffer_.push_back( Index );
	NumIndices_++;
}

void VRadLauncher::SetVertexBuffer( int NumVertices, float* Vertices )
{
	NumVertices_ = NumVertices;
	VertexBuffer_.resize(NumVertices);

	if (NumVertices)
		memcpy( (void*) &VertexBuffer_[0], (const void*) Vertices, sizeof( float ) * 3 * NumVertices );
}

void VRadLauncher::SetBlackMeshVertexTangentBasisBuffer( int NumVertices, float* TangentX, float* TangentY, float* TangentZ )
{
	NumVertices_ = NumVertices;
	BlackMeshVertexTangentXBuffer_.resize(NumVertices);
	BlackMeshVertexTangentYBuffer_.resize(NumVertices);
	BlackMeshVertexTangentZBuffer_.resize(NumVertices);
	if (NumVertices)
	{
		memcpy( (void*) &BlackMeshVertexTangentXBuffer_[0], (const void*) TangentX, sizeof( float ) * 3 * NumVertices );
		memcpy( (void*) &BlackMeshVertexTangentYBuffer_[0], (const void*) TangentY, sizeof( float ) * 3 * NumVertices );
		memcpy( (void*) &BlackMeshVertexTangentZBuffer_[0], (const void*) TangentZ, sizeof( float ) * 3 * NumVertices );
	}	
}

void VRadLauncher::AddVertex( float X, float Y, float Z )
{
	VertexBuffer_.push_back( AvaVertex( X, Y, Z ) );
	NumVertices_++;
}

void VRadLauncher::SetPlaneBuffer( int NumPlanes, float* Planes )
{
	NumPlanes_ = NumPlanes;
	PlaneBuffer_.resize( NumPlanes );

	if (NumPlanes)
		memcpy( (void*) &PlaneBuffer_[0], (const void*) Planes, sizeof( float ) * 4 * NumPlanes );
}

void VRadLauncher::AddPlane( float X, float Y, float Z, float D )
{
	static AvaPlane plane;
	plane.Distance_ = D;
	plane.Normal_ = Vector( X, Y, Z );
	PlaneBuffer_.push_back( plane );
	++NumPlanes_;
}

void VRadLauncher::AddFace( int IndexBufferOffset, int NumVertices, 
							unsigned short iPlane,
							unsigned short iTextureInfo,
							int iDispInfo,
							float Emissive_X, float Emissive_Y, float Emissive_Z,
							int LightmapWidth, int LightmapHeight )
{
	static AvaFace face;
	face.NumVertices_		= NumVertices;
	face.VertexIndices_		= &IndexBuffer_[IndexBufferOffset];
	face.PlaneIndex_		= iPlane;
	face.TextureInfoIndex_	= iTextureInfo;
	face.DispIndex_			= iDispInfo;
	face.Emission_			= Vector( Emissive_X, Emissive_Y, Emissive_Z );
	face.LightmapWidth_		= LightmapWidth;
	face.LightmapHeight_	= LightmapHeight;
	FaceList_.push_back( face );
	++NumFaces_;
	NumEdges_ += NumVertices;
}

void VRadLauncher::AddTextureInfo( float TexturemapU_X, float TexturemapU_Y, float TexturemapU_Z,
								   float TexturemapV_X, float TexturemapV_Y, float TexturemapV_Z,
								   float LightmapU_X, float LightmapU_Y, float LightmapU_Z, float LightmapU_Offset,
								   float LightmapV_X, float LightmapV_Y, float LightmapV_Z, float LightmapV_Offset,
								   float TexelInWorldUnit,
								   float LuxelInWorldUnit,
								   int iTextureData,
								   int Flags)
{
	static AvaTextureInfo info;
	info.TexturemapUAxis_ = Vector( TexturemapU_X, TexturemapU_Y, TexturemapU_Z );
	info.TexturemapVAxis_ = Vector( TexturemapV_X, TexturemapV_Y, TexturemapV_Z );
	info.LightmapUAxis_ = Vector( LightmapU_X, LightmapU_Y, LightmapU_Z );
	info.LightmapVAxis_ = Vector( LightmapV_X, LightmapV_Y, LightmapV_Z );
	info.LightmapUOffset_ = LightmapU_Offset;
	info.LightmapVOffset_ = LightmapV_Offset;
	info.TexelInWorldUnit_ = TexelInWorldUnit;
	info.LuxelInWorldUnit_ = LuxelInWorldUnit;
	info.TextureDataIndex_ = iTextureData;
	info.Flags_ = Flags;
	TextureInfoBuffer_.push_back( info );
	++NumTextureInfos_;
}

void VRadLauncher::AddTextureData( int Width, int Height,
								   float Reflectivity_X, float Reflectivity_Y, float Reflectivity_Z,
								   float Brightness_X, float Brightness_Y, float Brightness_Z,
								   float ReflectivityScale )
{
	static AvaTextureData texdata;
	texdata.Width_	= Width;
	texdata.Height_	= Height;	
	texdata.Reflectivity_ = Vector( Reflectivity_X, Reflectivity_Y, Reflectivity_Z );
	texdata.Brightness_ = Vector( Brightness_X, Brightness_Y, Brightness_Z );
	VectorScale( texdata.Reflectivity_, ReflectivityScale, texdata.Reflectivity_ );
	TextureDataBuffer_.push_back( texdata );
	++NumTextureDatas_;
}

void VRadLauncher::AddPointLight(float Pos_X, float Pos_Y, float Pos_Z,
								 float Intensity_X, float Intensity_Y, float Intensity_Z,
								float Radius, float Exponent, bool bLightMapLight,
								INT LightId )
{
	AvaPointLight Light;
	Light.Position_		= Vector( Pos_X, Pos_Y, Pos_Z );
	Light.Intensity_	= Vector( Intensity_X, Intensity_Y, Intensity_Z);
	Light.Radius_		= Radius;
	Light.Exponent_		= Exponent;
	Light.bLightMapLight_ = bLightMapLight;
	Light.LightId_ = LightId;
	PointLights_.push_back( Light );
}

void VRadLauncher::AddSpotLight(float Pos_X, float Pos_Y, float Pos_Z,
								 float Intensity_X, float Intensity_Y, float Intensity_Z,
								 float Radius, float Exponent,
								 float Direction_X, float Direction_Y, float Direction_Z, 
								 float InnerConeAngle, float OuterConeAngle, bool bLightMapLight, INT LightId )
{
	AvaSpotLight Light;
	Light.Position_		= Vector( Pos_X, Pos_Y, Pos_Z );
	Light.Intensity_	= Vector( Intensity_X, Intensity_Y, Intensity_Z);
	Light.Radius_		= Radius;
	Light.Exponent_		= Exponent;
	Light.bLightMapLight_ = bLightMapLight;
	Light.LightId_		= LightId;
	Light.Direction_	= Vector( Direction_X, Direction_Y, Direction_Z );
	Light.InnerConeAngle_ = InnerConeAngle;
	Light.OuterConeAngle_ = OuterConeAngle;
	SpotLights_.push_back( Light );
}

void VRadLauncher::AddDispInfo(
						 float StartPosX, float StartPosY, float StartPosZ,
						 INT DispVertexStart,
						 INT Power,
						 unsigned short NeighborIndices[4],
						 unsigned char NeighborOrientations[4],
						 unsigned char Spans[4],
						 unsigned char NeighborSpans[4] )
{
	static AvaDispInfo DispInfo;
	DispInfo.StartPosition_ = Vector( StartPosX, StartPosY, StartPosZ );
	DispInfo.DispVertexStart_ = DispVertexStart;
	DispInfo.Power_ = Power;
	for( int neighborIndex =0; neighborIndex < 4; ++neighborIndex )
	{
		DispInfo.Neighbor_[neighborIndex].NeighborIndex_		= NeighborIndices[neighborIndex];
		DispInfo.Neighbor_[neighborIndex].NeighborOrientation_	= NeighborOrientations[neighborIndex];
		DispInfo.Neighbor_[neighborIndex].Span_					= Spans[neighborIndex];
		DispInfo.Neighbor_[neighborIndex].NeighborSpan_			= NeighborSpans[neighborIndex];
	}
	DispInfoBuffer_.push_back( DispInfo );	
}

void VRadLauncher::AddDispVertex( float DispVecX, float DispVecY, float DispVecZ, float DispDistance, float Alpha )
{
	static AvaDispVertex DispVertex;
	DispVertex.DispVector_ = Vector( DispVecX, DispVecY, DispVecZ );
	DispVertex.DispDistance_ = DispDistance;
	DispVertex.Alpha_ = Alpha;

	DispVertexBuffer_.push_back( DispVertex );
}

void VRadLauncher::SetOptions( int options, int num_grid )
{
	Options_ = options;
	NumGrid_ = num_grid;	
}

bool VRadLauncher::CheckVRadLimits()
{
	return true;
}

template <typename T> void WriteVector( const std::vector<T>& data, FILE* fp )
{
	size_t N = data.size();
	fwrite( &N, sizeof(size_t), 1, fp );
	if (N)
	{
		fwrite( &data.front(), sizeof(T), N, fp );
	}
}

bool VRadLauncher::WriteInputData( const char* szFilename )
{
	FILE* fp = fopen( szFilename, "wb" );
	if (!fp)
	{
		return false;
	}
	WriteVector( FaceList_, fp );
	WriteVector( PlaneBuffer_, fp );
	WriteVector( VertexBuffer_, fp );
	WriteVector( TextureDataBuffer_, fp );
	WriteVector( TextureInfoBuffer_, fp );
	WriteVector( PointLights_, fp );
	WriteVector( SpotLights_, fp );
	fwrite( &SunLight_, sizeof(SunLight_), 1, fp );
	WriteVector( BlackMeshIndexBuffer_, fp );
	WriteVector( BlackMeshTexDataIndexBuffer_, fp );
	WriteVector( BlackMeshTriangleCounts_, fp );
	WriteVector( BlackMeshVertexCounts_, fp );
	WriteVector( BlackMeshSampleToAreaRatios_, fp );
	WriteVector( BlackMeshSampleVerticesFlags_, fp );
	WriteVector( BlackMeshVertexBuffer_, fp );
	WriteVector( BlackMeshVertexTangentXBuffer_, fp );
	WriteVector( BlackMeshVertexTangentYBuffer_, fp );
	WriteVector( BlackMeshVertexTangentZBuffer_, fp );
	for (size_t i=0; i<FaceList_.size(); ++i)
	{
		fwrite( FaceList_[i].VertexIndices_, FaceList_[i].NumVertices_, sizeof(int), fp );
	}	
	WriteVector( AmbientCubeSamplePoints_, fp );
	WriteVector( DispInfoBuffer_, fp );
	WriteVector( DispVertexBuffer_, fp );
	fclose( fp );
	return true;
}

void VRadLauncher::SetBlackMeshVertexBuffer( int NumVertices, float* Vertices )
{
	BlackMeshVertexBuffer_.resize( NumVertices );

	if (NumVertices)
		memcpy( (void*) &BlackMeshVertexBuffer_[0], (const void*) Vertices, sizeof( float ) * 3 * NumVertices );
}

void VRadLauncher::SetBlackMeshIndexBuffer( int NumIndices, int* Indices )
{
	BlackMeshIndexBuffer_.resize( NumIndices );

	if (NumIndices)
		memcpy( (void*) &BlackMeshIndexBuffer_[0], (const void*) Indices, sizeof( int ) * NumIndices );
}

void VRadLauncher::SetBlackMeshTexDataIndexBuffer( int NumIndices, int *Indices )
{
	BlackMeshTexDataIndexBuffer_.resize( NumIndices );
	if( NumIndices )
		memcpy( (void*) &BlackMeshTexDataIndexBuffer_[0], (const void*) Indices, sizeof( int ) * NumIndices );
}

void VRadLauncher::SetBlackMeshTriangleCounts( int NumTriangleCounts, int *Counts )
{
	BlackMeshTriangleCounts_.resize( NumTriangleCounts );
	if( NumTriangleCounts )
	{
		memcpy( (void*) &BlackMeshTriangleCounts_[0], (const void*) Counts, sizeof(int) * NumTriangleCounts );
	}
}

void VRadLauncher::SetBlackMeshVertexCounts( int NumVertexCount, int *Counts )
{
	BlackMeshVertexCounts_.resize( NumVertexCount );
	if( NumVertexCount )
	{
		memcpy( (void*) &BlackMeshVertexCounts_[0], (const void*) Counts, sizeof(int) * NumVertexCount );
	}
}

void VRadLauncher::SetBlackMeshSampleToAreaRatios( int NumRatios, float *ratios )
{
	BlackMeshSampleToAreaRatios_.resize( NumRatios );
	if( NumRatios )
	{
		memcpy( (void*) &BlackMeshSampleToAreaRatios_[0], (const void*) ratios, sizeof(float) * NumRatios );
	}
}

void VRadLauncher::SetBlackMeshSampleVerticesFlags( int NumFlags, int *flags )
{
	BlackMeshSampleVerticesFlags_.resize( NumFlags );
	if( NumFlags )
	{
		memcpy( (void*) &BlackMeshSampleVerticesFlags_[0], (const void*) flags, sizeof(int) * NumFlags );
	}
}

template <typename T>
void ReadVector( vector<T>& data, FILE* fp )
{
	size_t N;
	fread( &N, sizeof(size_t), 1, fp );
	data.resize( N );
	if (N > 0)
		fread( &data.front(), sizeof(typename T), N, fp );
}

bool VRadLauncher::ReadOutputData( const char* szFilename )
{	
	FILE* fp = fopen( szFilename, "rb" );
	if (!fp) return false;
	ReadVector( dlightdata, fp );
	ReadVector( dlightdata2, fp );
	ReadVector( dfaces, fp );
	ReadVector( blackmesh_vertexlightdata, fp );
	ReadVector( blackmesh_vertexlightdata_without_sun, fp );
	ReadVector( AmbientCubes_, fp );
	ReadVector( dshadowdata, fp );

	ReadVector( sunVisibility_, fp );

	fread( &bHasHDRSunInfo, 1, sizeof(int), fp );
	if (bHasHDRSunInfo)
	{
		fread( &HDRSunInfo, 1, sizeof(FHDRSunInfo), fp );
	}
	fclose( fp );
	return true;
}


bool VRadLauncher::RunPVS( IVRadCallback *callback )
{
	AcquireClusters( true );
	// TODO:merge options 은 mpi 실행시, portal 개수에 영향을 끼칩니다.. 안끼치도록 고치면 merge option을 추가할 수 있습니다.
	// 2007. 1. 3
	CString parameters( "-vis -saveprt -nopassage c:\\avavis");
	
	if (Clusters_.empty())		// single computing
	{	
		CString CommandLine = GetCommandLineA();
		if (CommandLine[0] == '"')
		{
			CommandLine = CommandLine.Left( CommandLine.Find( '"', 1 ) );
			CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// binaries
			CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// unrealengine3
			CommandLine.Append( "\\Tool\\avavis\\q3map.exe\"" );
		}
		else
		{
			CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// binaries
			CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// unrealengine3
			CommandLine.Append( "\\Tool\\avavis\\q3map.exe" );
		}
		if (Execute( callback, CommandLine, parameters ) != 0)
		{				
			return false;
		}
	}
	else	// parallel computing
	{
		CString Hosts( "-hosts " );
		Hosts.AppendFormat( "%d localhost localhost ", Clusters_.size() + 2 );			
		// multi-core pc
		for (INT i=0; i<Clusters_.size(); ++i)
		{
			int Delim;
			if ((Delim = Clusters_[i].Find( '@' )) > 0)
			{
				Hosts.AppendFormat( "%s ", Clusters_[i].Left( Delim ) );					
			}
			else
			{
				Hosts.AppendFormat( "%s ", Clusters_[i] );
			}				
		}

		/// lauch process for parallel computing
		if ( Execute( callback, MPIExec, Hosts + " " + VisExe + " " + parameters ) != 0 )
		{
			AcquireClusters( false );
			return false;
		}			
		AcquireClusters( false );
	}
	return true;
}

bool VRadLauncher::Run( IVRadCallback* callback )
{	
	// Check VRAD Boundary
	if( CheckVRadLimits() )
	{
		AcquireClusters( true );
		CString parameters( "-red2black");// -dump" );		
		if( Options_ & FORCE_SINGLE_THREAD )
		{
			parameters.Append( " -threads 1");			
		}
		if( Options_ & DO_EXTRA)
		{
			parameters.Append( " -extra");			
		}	
		else
		{
			parameters.Append( " -noextra");
		}

		if( Options_ & USE_RAYTRACE )
		{
			parameters.Append( " -raytrace" );
		}

		if( Options_ & DO_FAST)
		{
			parameters.Append( " -fast");			
		}			
		if( Options_ & SAMPLE_SKY_NONE )
		{
			parameters.Append( " -samplesky 0");
		}
		else if( Options_ & SAMPLE_SKY_SIMPLE )
		{
			parameters.Append( " -samplesky 1");
		}
		else if( Options_ & SAMPLE_SKY_FULL )
		{
			parameters.Append( " -samplesky 2");
		}
		if (HdrTonemapScale_ != 0)
		{
			parameters.AppendFormat( " -hdrtonemapscale %f", HdrTonemapScale_ );
		}
		parameters.Append( " -bounce 500");
		//!{ 2006-06-05	허 창 민
		// for debugging lighting error
		if (Clusters_.empty())
		{
			parameters.Append( " -dump" );
			parameters.Append( " -dumpnormals" );
		}
		//!} 2006-06-05	허 창 민

		CString filename( DataRoot+appComputerName()+".radcontext" );
		WriteInputData( filename );		
		parameters.AppendFormat( " %s", filename );
		// single computing
		if (Clusters_.empty())
		{	
			CString CommandLine = GetCommandLineA();
			if (CommandLine[0] == '"')
			{
				CommandLine = CommandLine.Left( CommandLine.Find( '"', 1 ) );
				CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// binaries
				CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// unrealengine3
				CommandLine.Append( "\\Tool\\avaRad\\vRad.exe\"" );
			}
			else
			{
				CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// binaries
				CommandLine = CommandLine.Left( CommandLine.ReverseFind( '\\' ) );	/// unrealengine3
				CommandLine.Append( "\\Tool\\avaRad\\vRad.exe" );
			}
			if (Execute( callback, CommandLine, parameters ) != 0)
			{				
				return false;
			}
		}
		else	// parallel computing
		{
			CString Hosts( "-hosts " );
			Hosts.AppendFormat( "%d localhost ", Clusters_.size() + 1 );			
			// multi-core pc
			for (INT i=0; i<Clusters_.size(); ++i)
			{
				int Delim;
				if ((Delim = Clusters_[i].Find( '@' )) > 0)
				{
					Hosts.AppendFormat( "%s ", Clusters_[i].Left( Delim ) );					
				}
				else
				{
					Hosts.AppendFormat( "%s ", Clusters_[i] );
				}				
			}
			
			/// lauch process for parallel computing
			// 배포하기 전에 고칠 것
			if ( Execute( callback, MPIExec, Hosts + " " + VRadExe + " " + parameters ) != 0 )
			//if ( Execute( callback, MPIExec, Hosts + " " + VRadDevExe + " " + parameters ) != 0 ) 
			{
				AcquireClusters( false );
				return false;
			}			
			AcquireClusters( false );
		}		

		// read result of radiosity lighting
		if (!ReadOutputData( filename + ".out" ))
		{
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

int VRadLauncher::Execute(IVRadCallback* callback, LPCTSTR pszCmd, LPCTSTR pszCmdLine)
{
	//MessageBox( NULL, pszCmdLine, pszCmd, MB_OK );
	int rval = -1;
	SECURITY_ATTRIBUTES saAttr; 
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr, hChildStderrWr; 

	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

//	MessageBox( NULL, pszCmdLine, pszCmd, MB_OK | MB_ICONWARNING );

	// Create a pipe for the child's STDOUT. 
	if(CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
	{
		if(CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
		{
			if (DuplicateHandle(GetCurrentProcess(),hChildStdoutWr, GetCurrentProcess(),&hChildStderrWr,0, TRUE,DUPLICATE_SAME_ACCESS))
			{
				/* Now create the child process. */ 
				STARTUPINFO si;
				memset(&si, 0, sizeof si);
				si.cb = sizeof(si);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdInput = hChildStdinRd;
				si.hStdError = hChildStderrWr;
				si.hStdOutput = hChildStdoutWr;
				PROCESS_INFORMATION pi;
				CString str;
				str.Format("%s %s", pszCmd, pszCmdLine);
				CStringW last_desc;
				int last_cur = -1, last_total;
				if(CreateProcess(NULL, (char*) LPCTSTR(str), NULL, NULL, TRUE, 
					DETACHED_PROCESS, NULL, NULL, &si, &pi))
				{
					HANDLE hProcess = pi.hProcess;
					HANDLE hThread = pi.hThread;

#define BUFFER_SIZE 4096
					// read from pipe..
					char buffer[BUFFER_SIZE*2+1];
					int nLines = 0;
					BOOL bDone = FALSE;
					BOOL bOpen = FALSE;
					INT len;
					len = 0;					
					char cur_desc[256];
					CStringW cur_descw;

					DWORD LastAcquiredTime = 0, CurTime;

					strcpy( cur_desc, "" );

					while(1)
					{
						DWORD dwCount = 0;
						DWORD dwRead = 0;

						if (callback->IsCanceled())
						{
							TerminateProcess( pi.hProcess, -1 );							
							break;
						}

						// read from input handle
						PeekNamedPipe(hChildStdoutRd, NULL, NULL, NULL, &dwCount, NULL);
						if (dwCount)
						{
							dwCount = min (dwCount, BUFFER_SIZE - 1);
							ReadFile(hChildStdoutRd, buffer + len, dwCount, &dwRead, NULL);
						}
						if(dwRead)
						{
							buffer[dwRead + len] = 0;

							OutputDebugString( buffer );

							int old_len = len;

							len += dwRead;

							for (;;)
							{
								static char* found;
								static char* delim;
								
								/// prefix를 찾는다.
								found = strstr( buffer, "[[PROGRESS:" );								

								if (found)
								{
									/// skip bytes
									int skip = found - buffer;

									/// 전체 문자열 중 나머지 부분을 옮기고,
									memmove( buffer, found, len - skip +1 );

									/// 길이를 줄인다.
									len -= skip;									
									
									/// suffix를 찾는다.
									delim = strstr( buffer, "]]" );									

									if (delim)
									{
										int contents = delim - buffer;

										// 12345678901
										// [[PROGRESS:

										char temp[256];
										
										memcpy( temp, buffer+11, contents-11 );
										temp[contents-11]=0;

										char desc[256];
										int cur, total;
										sscanf( temp, "%s %d %d", desc, &cur, &total );

										CStringW progress;

										progress.Format( L" (%d of %d)", cur, total );

										if (strcmp( cur_desc, desc ))
										{
											if (bOpen)
											{											
												callback->End();
											}

											strcpy( cur_desc, desc );

											USES_CONVERSION;
											cur_descw = T2W( cur_desc );

											callback->Begin( total, cur_descw + progress );

											bOpen = TRUE;
										}
										
										last_cur = cur;
										last_total = total;
										last_desc = cur_descw + progress;

										if (bOpen)
											callback->Update( cur, total, cur_descw + progress );

										if (cur == total)
										{
											last_cur = -1;

											if (bOpen)
												callback->End();

											bOpen = FALSE;

											cur_desc[0] = 0;
										}

										memmove( buffer, delim, len - contents +1 );
										len -= contents;
									}
									else
										break;									
								}
								else
								{
									/// 못찾았으니 어째~~ 계속 갑시다. 예전 것과의 concat 결과는 무의미하니, 지워버려! (-_- 꼭 그렇지는 않을 수도)
									if (len > BUFFER_SIZE)
									{
										int shrink = len - BUFFER_SIZE;

										len -= shrink;
										memmove( buffer, buffer + shrink, len + 1 );										
									}
									
									break;
								}
							}														
						}
						// check process termination
						else if(WaitForSingleObject(hProcess, 500) != WAIT_TIMEOUT)
						{							
							if (last_cur >= 0 && bOpen)
								callback->Update( last_cur, last_total, last_desc );

							if(bDone)
								break;
							bDone = TRUE;	// next time we get it							

							rval = 0;
						}

						if (last_cur >= 0 && bOpen)
							callback->Update( last_cur, last_total, last_desc );

						CurTime = GetTickCount();
						if (CurTime - LastAcquiredTime > 10000)
						{
							LastAcquiredTime = CurTime;
							AcquireClusters(true);
						}						
					}

					if (bOpen)
					{
						callback->End();
						bOpen = FALSE;
					}					
				}
				else
				{
					//SetForegroundWindow();
					CString strTmp;
					strTmp.Format("* Could not execute the command:\r\n   %s\r\n", str);
					//Append(strTmp);
					strTmp.Format("* Windows gave the error message:\r\n   \"%d\"\r\n", GetLastError());
					//Append(strTmp);
				}

				CloseHandle(hChildStderrWr);
			}
			CloseHandle(hChildStdinRd);
			CloseHandle(hChildStdinWr);
		}
		CloseHandle(hChildStdoutRd);
		CloseHandle(hChildStdoutWr);
	}

	return rval;
}

void VRadLauncher::GetShadowmapCount( int iFace, int* Count )
{
	dface_t* face = &dfaces[iFace];
	unsigned char *shadowmapdata = &dshadowdata[face->lightshadowofs];
	*Count = shadowmapdata[0];

	// shadow map 기능 제거 2007. 5. 3
	*Count = 0;
}

void VRadLauncher::GetShadowmap(INT iFace, int ShadowIndex, int* ExportLightId, int* Width, int* Height, unsigned char** ShadowMap )
{
	dface_t* face = &dfaces[iFace];

	*Width = face->m_LightmapTextureSizeInLuxels[0] + 1;
	*Height = face->m_LightmapTextureSizeInLuxels[1] + 1;

	const int num_luxel = ( *Width ) * ( *Height );

	unsigned char *shadowmap_array = &dshadowdata[face->lightshadowofs + 1];

	const int shadowmap_offset = 1 + num_luxel;

	*ExportLightId	= shadowmap_array[ShadowIndex * shadowmap_offset + 0];
	*ShadowMap		= &shadowmap_array[ShadowIndex * shadowmap_offset + 1];
}

void VRadLauncher::GetLightmap( int iFace, int* Width, int* Height, unsigned char** Lightmap )
{
	//assert( VRad_ );
	assert( iFace < dfaces.size() );

	dface_t* face = &dfaces[iFace];

	/*
		대각선으로 면적이 0인 경우에 한하여 width, height는 valid하나 area가 0인 degen face가 생성될 수 있어서 error handling. :)
	*/
	if (face->lightofs < 0)
	{
		*Width = 0;
		*Height = 0;
		*Lightmap = 0;		
	}
	else
	{
		*Width	= face->m_LightmapTextureSizeInLuxels[0] + 1;
		*Height = face->m_LightmapTextureSizeInLuxels[1] + 1;
		*Lightmap = &dlightdata[face->lightofs];
	}	
}

void VRadLauncher::GetLightmapWithoutSun( int iFace, int* Width, int* Height, unsigned char** Lightmap )
{
	//assert( VRad_ );
	assert( iFace < dfaces.size() );

	dface_t* face = &dfaces[iFace];

	/*
	대각선으로 면적이 0인 경우에 한하여 width, height는 valid하나 area가 0인 degen face가 생성될 수 있어서 error handling. :)
	*/
	if (face->lightofs < 0)
	{
		*Width = 0;
		*Height = 0;
		*Lightmap = 0;		
	}
	else
	{
		*Width	= face->m_LightmapTextureSizeInLuxels[0] + 1;
		*Height = face->m_LightmapTextureSizeInLuxels[1] + 1;
		*Lightmap = &dlightdata2[face->lightofs];
	}
}

int VRadLauncher::GetFaceSunVisibility( int iFace )
{
	return dfaces[iFace].bSunVisibility;
}

void VRadLauncher::GetNegativeLightmap( int iFace, int* Width, int* Height, unsigned char** Lightmap )
{
	//assert( VRad_ );
	assert( iFace < dfaces.size() );

	// TODO : 여기를 고쳐야 한다.

	//dface_t* face = &dfaces[iFace];

	//*Width = face->m_LightmapTextureSizeInLuxels[0] + 1;
	//*Height = face->m_LightmapTextureSizeInLuxels[1] + 1;

	//// light style
	//// 0 = all lights
	//// 1 = not light map light
	//int light_style_index;
	//const int NonLightMapStyle = 1;
	//for (light_style_index=0; light_style_index < MAXLIGHTMAPS; light_style_index++ )
	//{
	//	if ( face->styles[light_style_index] == NonLightMapStyle )
	//		break;
	//}

	//
	//if( light_style_index < MAXLIGHTMAPS )
	//{
	//	// 항상 bump map을 사용한다는 가정이 성립해야 한다.
	//	const int numluxels = (*Width) * (*Height);
	//	const bool needsBumpmap = true;
	//	const int bumpSampleCount = needsBumpmap ? NUM_BUMP_VECTS + 1 : 1;
	//	*Lightmap = &dlightdata[face->lightofs + (light_style_index * bumpSampleCount) * numluxels * 4 ]; 
	//}
	//else
	{
		*Lightmap = NULL;
	}
}

void VRadLauncher::GetBlackMeshVertexLightMap( unsigned char** VertexLightMap )
{
	*VertexLightMap = blackmesh_vertexlightdata.size() > 0 ? &blackmesh_vertexlightdata[0] : NULL;
}

void VRadLauncher::GetBlackMeshVertexLightMapWithoutSun(unsigned char **VertexLightMap)
{
	*VertexLightMap = blackmesh_vertexlightdata_without_sun.size() > 0 ? &blackmesh_vertexlightdata_without_sun[0] : NULL;
}

void VRadLauncher::GetBlackMeshSunVisibility( int** VisibilityMap )
{
	*VisibilityMap = sunVisibility_.size() > 0 ? &sunVisibility_[0] : NULL;
}

void VRadLauncher::Finalize()
{
	/*if( Module_ )
	{
		Sys_UnloadModule( Module_ );
		VRad_ = NULL;
	}*/
}

void VRadLauncher::DumpLightmaps()
{
	assert( VRad_ );

	char Filename[255];
	int LightmapSize[2];
	Vector4D* blocklights = (Vector4D*)malloc( MAX_LIGHTMAP_DIM_INCLUDING_BORDER * MAX_LIGHTMAP_DIM_INCLUDING_BORDER );

	ColorSpace::SetGamma( 2.2f, 2.2f, 1.0f, false, false );
	
	for( int iFace = 0; iFace < dfaces.size(); ++iFace )
	{
		dface_t* CurFace = &dfaces[iFace];		

		LightmapSize[0] = CurFace->m_LightmapTextureSizeInLuxels[0] + 1;
		LightmapSize[1] = CurFace->m_LightmapTextureSizeInLuxels[1] + 1;

		Msg(" %d face : Lightmap size in luxel ( %d x %d )\n", iFace, LightmapSize[0], LightmapSize[1] );

		assert( LightmapSize[0] < MAX_LIGHTMAP_DIM_INCLUDING_BORDER && LightmapSize[1] < MAX_LIGHTMAP_DIM_INCLUDING_BORDER );

		// rgbe -> float
		int nLuxels = LightmapSize[0] * LightmapSize[1];
		
		int iBumpStart = 1;
		int iBumpEnd = 2;		
		for( int iBump = iBumpStart; iBump <  iBumpEnd; ++iBump )
		{
			colorRGBExp32* pLightmap = (colorRGBExp32*)&dlightdata[CurFace->lightofs + iBump * nLuxels * 4 ];

			for( int iLuxel = 0; iLuxel < nLuxels; ++iLuxel )
			{
				blocklights[iLuxel][0] = TexLightToLinear( pLightmap->r, pLightmap->exponent );
				blocklights[iLuxel][1] = TexLightToLinear( pLightmap->g, pLightmap->exponent );
				blocklights[iLuxel][2] = TexLightToLinear( pLightmap->b, pLightmap->exponent );
				blocklights[iLuxel][3] = 1;
				++pLightmap;
			}
			// float -> char
			float* pSrc = (float*)blocklights;
			unsigned char* Color;
			unsigned char* Image = new unsigned char[nLuxels*3];
			Color = Image;

			for( int t = 0; t < LightmapSize[1]; ++t )
			{
				for( int s = 0; s < LightmapSize[0]; ++s, pSrc += (sizeof(Vector4D) / sizeof(*pSrc)) )
				{
					Color[0] = RoundFloatToByte( min(pSrc[0] * 255.0f, 255.0f));
					Color[1] = RoundFloatToByte( min(pSrc[1] * 255.0f, 255.0f));
					Color[2] = RoundFloatToByte( min(pSrc[2] * 255.0f, 255.0f));

					Color += 3;
				}
			}

			sprintf( Filename, "%02d_lightmap_bump_%d.tga", iFace, iBump );

			TGAWriter::Write( Image, Filename, LightmapSize[0], LightmapSize[1], IMAGE_FORMAT_RGB888, IMAGE_FORMAT_RGB888 );

			delete[] Image;
		}
		
	}

	free( blocklights );
}

//!} 2006-03-31	허 창 민
