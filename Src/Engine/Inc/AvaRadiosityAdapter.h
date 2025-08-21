// 2006-04-10	허 창 민

#ifndef _AVA_RADIOSITY_ADAPTER_
#define _AVA_RADIOSITY_ADAPTER_

#include "../../avarad/utils/vrad_launcher/ivrad_launcher.h"


// ava rad의 bspflags.h와 맞춰야 합니다.
#define SURF_NOLIGHT				0x0400	// Don't calculate light
#define SURF_BUMPLIGHT				0x0800	// calculate three lightmaps for the surface for bumpmapping
#define SURF_SECONDARYLIGHTSOURCE	0x1000	// this surface has secondary light source
#define SURF_SKIPCOLLISION			0x2000	// don't create collision model

struct AvaSurfaceMapInfo
{
	FVector			TexturemapUAxis;
	FVector			TexturemapVAxis;

	FVector			LightmapUAxis;
	FVector			LightmapVAxis;

	FLOAT			LightmapUOffset;
	FLOAT			LightmapVOffset;

	FLOAT			ShadowMapScale;
	INT				Flags;

	AvaSurfaceMapInfo()
		: TexturemapUAxis( 1.0f, 0.0f, 0.0f),
		  TexturemapVAxis( 0.0f, 1.0f, 0.0f),
		  LightmapUAxis( 1.0f, 0.0f, 0.0f),
		  LightmapVAxis( 0.0f, 1.0f, 0.0f),
		  LightmapUOffset(0.0f),
		  LightmapVOffset(0.0f),
		  ShadowMapScale(1.0f),
		  Flags(0)
	{}
};

struct AvaLightmapInfo
{
	FVector2D		MinUV;	// surface map 상에서 이 node가 차지하는 면적의 최소 위치
	FVector2D		MaxUV;	// surface map 상에서 이 node가 차지하는 면적의 최대 위치

	INT				Width, Height;		// lightmap 의 크기
	unsigned char*	Lightmap;			// lightmap data
	unsigned char*	LightmapWithoutSun;			// lightmap data
	unsigned char*	NegativeLightmap;	// negative light map data

	// iBump = 0		: Normal
	// iBump 1, 2, 3	: Lightmap Vector Basis
	void GetSample( INT X, INT Y, INT iBump, FLOAT* R, FLOAT* G, FLOAT* B);
	void GetSampleWithoutSun( INT X, INT Y, INT iBump, FLOAT* R, FLOAT* G, FLOAT* B);
	void GetNegativeSample( INT X, INT Y, INT iBump, FLOAT* R, FLOAT* G, FLOAT* B);

	AvaLightmapInfo()
		: MinUV(0.0f, 0.0f),
		  MaxUV(0.0f, 0.0f),
		  Width(0),
		  Height(0),
		  Lightmap(NULL),
		  NegativeLightmap(NULL)
	{}

};

// to export terrain to rad
namespace Rad
{
	typedef enum
	{
		CORNER_TO_CORNER=0,
		CORNER_TO_MIDPOINT=1,
		MIDPOINT_TO_CORNER=2
	} NeighborSpan;


	// These define relative orientations of displacement neighbors.
	typedef enum
	{
		ORIENTATION_CCW_0=0,
		ORIENTATION_CCW_90=1,
		ORIENTATION_CCW_180=2,
		ORIENTATION_CCW_270=3
	} NeighborOrientation;
};


struct AvaDispNeighbor
{
	USHORT			NeighborIndex;
	UCHAR			NeighborOrientation;
	UCHAR			Span;
	UCHAR			NeighborSpan;
	UCHAR			Padding;
};

struct AvaDispInfo
{
	FVector			StartPosition;		// UV base position for lighting map
	INT				DispVertexStart;	// displacement vertex index
	INT				Power;				// indicate size of map ( 2^power + 1 ) * (2^power + 1 )

	// edge neighbors : left / top / right / bottom
	USHORT			NeighborIndex[4];
	UCHAR			NeighborOrientation[4];
	UCHAR			Span[4];
	UCHAR			NeighborSpan[4];

	//AvaDispNeighbor Neighbor[4];		// edge neighbors
};

struct AvaDispVertex
{
	FVector			DispVector;
	FLOAT			DispDistance;
	FLOAT			Alpha;
};


// terrain


struct AvaShadowMapInfo
{
	INT				ExportLightId;
	unsigned char*	ShadowMap;
};

struct AvaFaceInfo
{
	INT				IndexBufferOffset;
	INT				NumVertices;
	unsigned short	iPlane;
	unsigned short	iSurfaceMapInfo;
	unsigned short	iTextureData;
	INT				iDispInfo;

	FVector			Emission;

	AvaLightmapInfo				LightmapInfo;

	TArray<AvaShadowMapInfo>	ShadowMaps;

	TArray<FVector2D>	RadiosityLightmapCoords;

	TArray<FVector2D>	SurfaceMapCoords;
	INT					bSunVisible;
};

struct AvaTextureData
{
	UMaterialInstance* SourceMaterial_;

	INT SizeX_;
	INT SizeY_;

	FVector Reflectivity_;
	FVector Brightness_;
	FLOAT	ReflectivityScale_;	

	DWORD Flags;

	UBOOL operator==( const AvaTextureData& Other ) const
	{
		if( SourceMaterial_ == Other.SourceMaterial_ )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
};



class IVRadLauncher;

class UMaterial;

class FVector;

//!{ 2006-06-05	허 창 민
struct AvaSample
{
	INT NumPoints;
	TArray<FVector>	Positions;
	FVector	Color;
	FVector Center;
	FVector NormalEnd;
};
//!} 2006-06-05	허 창 민
class AvaRadiosityAdapter : public IVRadCallback
{
public:
	virtual void Begin( int Total, LPCTSTR szCaption ) 
	{
		GWarn->BeginSlowTask( szCaption, TRUE );
		GWarn->StatusUpdatef( 0, Total, szCaption, 0, Total );
	}

	virtual void Update( int Cur, int Total, LPCTSTR szCaption ) 
	{
		GWarn->StatusUpdatef( Cur, Total, szCaption, Cur, Total );
	}

	virtual void End() 
	{
		GWarn->EndSlowTask();
	}

	virtual bool IsCanceled();

	AvaRadiosityAdapter() : hDLL_(NULL), VRadLauncher_(NULL), BlackMeshVertexLightMap_(NULL), BlackMeshSunVisibility_(NULL), bSupportsCascadedShadow_(FALSE)
	{}

	bool StartUp();
	void GatherWorldInfo(const FLightingBuildOptions& Options);
	bool Render(const FLightingBuildOptions& Options);
	void ShutDown();

	INT FindOrAddTexData( UMaterialInstance* Material );

	void GetRadiositySamples( TArray<AvaSample>* Samples );

	FORCEINLINE UBOOL GetSunVisibility( INT Id )
	{
		return BlackMeshSunVisibility_[Id];
	}
	

	// Export Data 저장공간
	TArray<AvaSurfaceMapInfo>	SurfaceMaps_;
	TArray<INT>					IndexBuffer_;
	TArray<FVector>				VertexBuffer_;
	TArray<FPlane>				PlaneBuffer_;
	TArray<AvaFaceInfo>			FaceInfos_;
	TArray<AvaTextureData>		TextureDatas_;

	TArray<FVector>				BlackMeshVertexBuffer_;
	TArray<INT>					BlackMeshIndexBuffer_;
	TArray<INT>					BlackMeshTexDataIndex_;	// triangle당 texdata index로 맵핑..

	// per static mesh info
	TArray<INT>					BlackMeshTriangleCounts_;
	TArray<INT>					BlackMeshVertexCounts_;
	TArray<FLOAT>				BlackMeshSampleToAreaRatios_;
	TArray<INT>					BlackMeshSampleVerticesFlags_;

	TArray<FVector>				BlackMeshVertexTangentXBuffer_;
	TArray<FVector>				BlackMeshVertexTangentYBuffer_;
	TArray<FVector>				BlackMeshVertexTangentZBuffer_;

	TArray<AvaDispInfo>			DispInfos_;
	TArray<AvaDispVertex>		DispVertexes_;

	TMap<ULightComponent*, INT>	LightToExportId;

	unsigned char*				BlackMeshVertexLightMap_;
	unsigned char*				BlackMeshVertexLightMapWithoutSun_;

	int*						BlackMeshSunVisibility_;

	UBOOL						bSupportsCascadedShadow_;	

	TArray<FVector2D>			Displacements;
	TArray<FVector2D>			MaxLuxels;

private:
	HMODULE			hDLL_;
	IVRadLauncher*	VRadLauncher_;
};

// 2006. 12. 29 changmin
class AvaPVSCalculator : public IVRadCallback
{
public:
	~AvaPVSCalculator()
	{
		ShutDown();
	}
	virtual void Begin( int Total, LPCTSTR Caption )
	{
		GWarn->BeginSlowTask( Caption, TRUE );
		GWarn->StatusUpdatef( 0, Total, Caption, 0, Total );
	}
	virtual void Update( int Cur, int Total, LPCTSTR Caption )
	{
		GWarn->StatusUpdatef( Cur, Total, Caption, Cur, Total );
	}
	virtual void End()
	{
		GWarn->EndSlowTask();
	}
	virtual bool IsCanceled();

	bool LoadLauncher();
	bool Compute( const FFullBuildGeometryOptions &Options );
	void ShutDown();

private:
	HMODULE			LauncherHandle_;
	IVRadLauncher	*Launcher_;
};

//
//	ULineBatchComponent
//
class USampleComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(USampleComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	TArray<AvaSample> Samples;	

	virtual FPrimitiveSceneProxy* CreateSceneProxy();
};

#endif