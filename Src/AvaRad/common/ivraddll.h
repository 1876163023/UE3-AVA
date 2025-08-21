//========= Copyright1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef IVRADDLL_H
#define IVRADDLL_H
#ifdef _WIN32
#pragma once
#endif


#include "interface.h"
#include "bspfile.h"


#define VRAD_INTERFACE_VERSION "vraddll_1"
#define VRAD_INTERFACE_VERSION2	"vraddll_2"

class CBSPInfo
{
public:
	byte			*dlightdata;
	int				lightdatasize;

	dface_t			*dfaces;
	unsigned char	*m_pFacesTouched;	// If non-null, then this has 1 byte for each face and
										// tells which faces had their lightmaps updated.										
	int				numfaces;
	
	dvertex_t		*dvertexes;
	int				numvertexes;

	dedge_t			*dedges;
	int				numedges;

	int				*dsurfedges;
	int				numsurfedges;

	texinfo_t		*texinfo;
	int				numtexinfo;

	dtexdata_t		*dtexdata;
	int				numtexdata;

	ddispinfo_t		*g_dispinfo;
	int				g_numdispinfo;

	char			*texDataStringData;
	int				nTexDataStringData;

	int				*texDataStringTable;
	int				nTexDataStringTable;
};


// This is the DLL interface to VRAD.
class IVRadDLL
{
public:
	// All vrad.exe does is load the VRAD DLL and run this.
	virtual int			main( int argc, char **argv ) = 0;
	
	
	// Load the BSP file into memory.
	virtual bool		Init( char const *pFilename ) = 0;

	// You must call this if you call Init(), to free resources.
	virtual void		Release() = 0;

	// Get some data from the BSP file that's in memory.
	virtual void		GetBSPInfo( CBSPInfo *pInfo ) = 0;

	// Incrementally relight the BSP file in memory given the new entity 
	// descriptions in pVMFFile. pVMFFile should only contain light entities.
	//
	// Returns true only if the lightmaps are updated. If the process is 
	// interrupted or there is an error, false is returned.
	virtual bool		DoIncrementalLight( char const *pVMFFile ) = 0;

	// Calling DoIncrementalLight doesn't actually write anything to disk.
	// Calling this will write the incremental light file out and will write the
	// current in-memory light data into the BSP.
	// NOTE: if DoIncrementalLight never finished, this will do nothing and return false.
	virtual bool		Serialize() = 0;

	// Returns a 0-1 value telling how close it is to completing the task.
	// This can be called from a separate thread than DoIncrementLight.
	virtual float		GetPercentComplete() = 0;

	// This can be called from a separate thread than the DoIncrementalLight thread.
	// It asynchronously tells DoIncrementalLight to stop as soon as possible and exit.
	virtual void		Interrupt() = 0;
};

// 우리는 데이타를 메모리에서 메모리로 전송해야 합니다.
//!{ 2006-03-20	허 창 민
struct AvaFace
{
	int				NumVertices_;
	int*			VertexIndices_;

	unsigned short	PlaneIndex_;
	unsigned short	TextureInfoIndex_;

	int				DispIndex_;

	Vector			Emission_;

	int				LightmapWidth_;
	int				LightmapHeight_;
};

struct AvaTextureInfo
{
	Vector			TexturemapUAxis_;
	Vector			TexturemapVAxis_;
	
	Vector			LightmapUAxis_;
	Vector			LightmapVAxis_;

	float			LightmapUOffset_;
	float			LightmapVOffset_;

	float			TexelInWorldUnit_;
	float			LuxelInWorldUnit_;

	int				TextureDataIndex_;
	int				Flags_;
};

struct AvaTextureData
{
	int				Width_;
	int				Height_;

	Vector			Reflectivity_;	
	Vector			Brightness_;
};

struct AvaPlane
{
	Vector	Normal_;
	float	Distance_;	// N dot P0
};

struct AvaVertex
{
	Vector Position_;
	AvaVertex()
	{}
	AvaVertex( float x, float y, float z ) : Position_(x, y, z)
	{}
};

struct AvaPointLight
{
	Vector Position_;
	Vector Intensity_;
	float Radius_;
	float Exponent_;
	bool bLightMapLight_;
	INT LightId_;
};

struct AvaSpotLight : AvaPointLight
{
	Vector Direction_;
	float InnerConeAngle_;
	float OuterConeAngle_;
};

struct AvaSunLight
{
	Vector Intensity_;
	Vector Ambient_;
	Vector Direction_;
	bool bLightMapLight_;
	INT LightId_;

	char	SkyEnvMapFileName_[1024];
	float	SkyMax_;
	float	SkyScale_;
	float	Yaw_;	
};

struct AvaAmbientCube
{
	float				SHValues[4*3];
	byte				Irradiance[4];	// encoded as rgbe
};

// for terrain
struct AvaDispNeighbor
{
	unsigned short		NeighborIndex_;
	unsigned char		NeighborOrientation_;
	unsigned char		Span_;
	unsigned char		NeighborSpan_;
};

struct AvaDispInfo
{
	Vector			StartPosition_;		// UV base position for lighting map
	int				DispVertexStart_;	// displacement vertex index
	int				Power_;				// indicate size of map ( 2^power + 1 ) * (2^power + 1 )
	AvaDispNeighbor Neighbor_[4];		// edge neighbors
};

struct AvaDispVertex
{
	Vector			DispVector_;
	float			DispDistance_;
	float			Alpha_;
};


class IVRadDLL2 : public IVRadDLL
{
public:
	AvaFace*			FaceArray_;
	AvaPlane*			PlaneArray_;
	AvaVertex*			VertexArray_;
	AvaTextureInfo*		TextureInfoArray_;
	AvaTextureData*		TextureDataArray_;
	AvaPointLight*		PointLightArray_;
	AvaSpotLight*		SpotLightArray_;
	AvaSunLight			SunLight_;

	int					NumFaces_;
	int					NumPlanes_;
	int					NumVertices_;
	int					NumTextureInfos_;
	int					NumTextureDatas_;
	int					NumPointLights_;
	int					NumSpotLights_;

	AvaVertex*			BlackMeshVertexArray_;
	int*				BlackMeshIndexArray_;
	int*				BlackMeshTexDataIndexArray_;
	int*				BlackMeshTriangleCountArray_;
	int*				BlackMeshVertexCountArray_;
	int*				BlackMeshSampleVerticesFlagArray_;
	float*				BlackMeshSampleToAreaRatioArray_;

	AvaVertex*			BlackMeshTangentXArray_;
	AvaVertex*			BlackMeshTangentYArray_;
	AvaVertex*			BlackMeshTangentZArray_;

	int					NumBlackMeshVertices_;
	int					NumBlackMeshIndices_;
	int					NumBlackMeshTexDataIndices_;
	int					NumBlackMeshTriangleCounts_;
	int					NumBlackMeshVertexCounts_;
	int					NumBlackMeshSampleToAreaRatioCounts_;
	int					NumBlackMeshSampleVerticesFlagCounts_;

	int					NumAmbientCubeSamplePoints_;
	Vector*				AmbientCubeSamplePoints_;

	int					NumAmbientCubes_;
	AvaAmbientCube*		AmbientCubes_;

	// for terrain
	int					NumDispInfos_;
	AvaDispInfo*		DispInfos_;
	int					NumDispVertices_;
	AvaDispVertex*		DispVertices_;

	virtual int			FaceLimit() = 0;
	virtual int			PlaneLimit() = 0;
	virtual int			VertexLimit() = 0;
	virtual int			TextureInfoLimit() = 0;
	virtual int			TextureDataLimit() = 0;
	virtual int			LightLimit() = 0;
	virtual int			LightmapSizeLimit() = 0;
	virtual int			EdgeLimit() = 0;
	virtual int			SurfaceEdgeLimit() = 0;

	virtual bool Load( const char* szName ) { return true; }
	virtual bool Save( const char* szName ) { return true; }
};
//!} 2006-03-20	허 창 민


#endif // IVRADDLL_H
