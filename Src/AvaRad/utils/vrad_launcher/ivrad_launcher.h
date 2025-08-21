// 2006-03-31	허 창 민

#ifndef IVRAD_LAUNCHER_H
#define IVRAD_LAUNCHER_H

#ifdef _WIN32
#pragma once
#endif

#ifdef VRAD_LAUNCHER_EXPORTS
#define VRAD_LAUNCHER_API __declspec(dllexport)
#else
#define VRAD_LAUNCHER_API __declspec(dllimport)
#endif

// VRAD Options
// int로 option을 전달하니, 32 bit 까지 사용가능하다...
#define FORCE_SINGLE_THREAD			( 1 << 0 )

#define	SAMPLE_SKY_NONE			( 1 << 1 )
#define SAMPLE_SKY_SIMPLE		( 1 << 2 )
#define SAMPLE_SKY_FULL			( 1 << 3 )

#define DO_EXTRA				( 1 << 4 )
#define DO_FAST					( 1 << 5 )
#define USE_RAYTRACE			( 1 << 6 )

class IVRadCallback
{
public :
	virtual void Begin( int Total, LPCWSTR szCaption ) = 0;
	virtual void Update( int Cur, int Total, LPCWSTR szCaption ) = 0;
	virtual void End() = 0;
	virtual bool IsCanceled() = 0;
};

struct FHDRSunInfo
{
	float	Theta;
	float	Phi;
	float	Direction[3];
	float	Color[4];
};

class IVRadLauncher
{
public:
	virtual bool Initialize() = 0;

	// set index 
	virtual void SetIndexBuffer( int NumIndices, int* Indices ) = 0;
	virtual void AddIndex( int Index ) = 0;

	// 1 vertex = 3d vector = float x, y, z
	virtual void SetVertexBuffer( int NumVertices, float* Vertices ) = 0;
	virtual void AddVertex( float X, float Y, float Z ) = 0;

	// 1 plane = 4d vector = < plane normal , distance = n dot p0 >
    virtual void SetPlaneBuffer( int NumPlanes, float* Planes ) = 0;
	virtual void AddPlane( float X, float Y, float Z, float D ) = 0;

	virtual void AddFace( int IndexBufferOffset, int NumVertices,
							unsigned short iPlane,
							unsigned short iTextureInfo,
							int iDispInfo,
							float Emissive_X, float Emissive_Y, float Emissive_Z,
							int LightmapWidth, int LightmapHeight ) = 0;

	virtual void AddTextureInfo(
							float TexturemapU_X, float TexturemapU_Y, float TexturemapU_Z,
							float TexturemapV_X, float TexturemapV_Y, float TexturemapV_Z,
							float LightmapU_X, float LightmapU_Y, float LightmapU_Z, float LightmapU_Offset,
							float LightmapV_X, float LightmapV_Y, float LightmapV_Z, float LightmapV_Offset,
							float TexelInWorldUnit,
							float LuxelInWorldUnit,
							int iTextureData,
							int Flags ) = 0;

	virtual void AddTextureData( int Width, int Height,
						 float Reflectivity_X, float Reflectivity_Y, float Reflectivity_Z,
						 float Brightness_X, float Brightness_Y, float Brightness_Z,
						 float ReflectivityScale ) = 0;

	virtual void AddPointLight( float Pos_X, float Pos_Y, float Pos_Z,	// Position
								float Intensity_X, float Intensity_Y, float Intensity_Z,
								float Radius, float Exponent,
								bool bLightMapLight,
								INT LightId ) = 0;

	virtual void AddSpotLight(float Pos_X, float Pos_Y, float Pos_Z,
		float Intensity_X, float Intensity_Y, float Intensity_Z,
		float Radius, float Exponent,
		float Direction_X, float Direction_Y, float Direction_Z, 
		float InnerConeAngle, float OuterConeAngle,
		bool bLightMapLight,
		INT LightId ) = 0;

	virtual bool Run( IVRadCallback* ) = 0;

	virtual void GetLightmap( int iFace, int* Width, int* Height, unsigned char** Lightmap ) = 0;
	virtual void GetLightmapWithoutSun( int iFace, int* Width, int* Height, unsigned char** Lightmap ) = 0;

	virtual int GetFaceSunVisibility( int iFace ) = 0;

	virtual void GetNegativeLightmap( int iFace, int* Width, int* Height, unsigned char** Lightmap ) = 0;

	virtual void GetShadowmapCount( int iFace, int* Count ) = 0;

	virtual void GetShadowmap( int iFace, int ShadowIndex, int* ExportLightId, int* Width, int* Height, unsigned char** Shadowmap ) = 0;

	virtual void Finalize() = 0;

	virtual void SetOptions( int options, int numgrid ) = 0;

	virtual void SetBlackMeshVertexBuffer( int NumVertices, float* Vertices ) = 0;

	virtual void SetBlackMeshIndexBuffer( int NumIndices, int* Indices ) = 0;

	virtual void SetBlackMeshTexDataIndexBuffer( int NumIndices, int* Indices ) = 0;

	virtual void SetBlackMeshTriangleCounts( int NumTriangleCount, int* Counts ) = 0;

	virtual void SetBlackMeshVertexCounts( int NumVetexCount, int *Counts ) = 0;

	virtual void SetBlackMeshSampleToAreaRatios( int NumRatios, float *Ratios ) = 0;

	virtual void SetBlackMeshSampleVerticesFlags( int NumFlags, int *flags ) = 0;

	virtual void SetBlackMeshVertexTangentBasisBuffer( int NumVertices, float* TangentX, float* TangentY, float* TangentZ ) = 0;

	virtual void GetBlackMeshVertexLightMap( unsigned char** VertexLightMap ) = 0;
	virtual void GetBlackMeshVertexLightMapWithoutSun( unsigned char** VertexLightMap ) = 0;

	virtual void GetBlackMeshSunVisibility( int** SunVisibility ) = 0;

	// Envmap file은 지정된 폴더에서만 읽기 시도합니다.
	// 따라서, 파일이름만 주어지면 됩니다.
	virtual void SetSunLight_( float X, float Y, float Z, float Intensity_X, float Intensity_Y, float Intensity_Z, float Ambient_X, float Ambient_Y, float Ambient_Z, bool bLightMapLight, INT LightId,
		const char* SkyEnvMapFileName, float SkyMax, float SkyScale, float Yaw, float HDRTonemapScale ) = 0;

	virtual void AddCluster( const char* szHost ) = 0;

	virtual void SetAmbientCubePoints( int nPoints, const float* ) = 0;

	virtual const float* GetAmbientCubes() = 0;

	virtual FHDRSunInfo* GetHDRSunInfo() = 0;

	virtual void AddDispInfo(
		float startPositionX, float startPositionY, float startPositionZ,
		INT DispVertexStart,
		INT Power,
		unsigned short NeighborIndices[4],
		unsigned char NeighborOrientations[4],
		unsigned char Spans[4],
		unsigned char NeighbrSpans[4] ) = 0;

	virtual void AddDispVertex( float DispVecX, float DispVecY, float DispVecZ, float DispDistance, float Alpha ) = 0;

	// compute pvs
	virtual bool RunPVS( IVRadCallback* ) = 0;
};

extern "C" VRAD_LAUNCHER_API IVRadLauncher* GetLauncher();

#endif // IVRAD_LAUNCHER_H
