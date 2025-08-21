#ifndef AVA_STATIC_LIGHTING_H
#define AVA_STATIC_LIGHTING_H

#if !(CONSOLE || FINAL_RELEASE)

class FTextureLayout;
struct FAvaStaticLighting;
struct FAvaComponentStaticLighting;
struct FAvaStaticLightmapPage;
struct FAvaSurfaceStaticLighting;

struct FAvaSurfaceStaticLighting : FRefCountedObject
{
	FAvaStaticLightmapPage*			LightmapPage;
	FAvaComponentStaticLighting*	Component;

	FVector TangentX;
	FVector TangentY;
	FVector TangentZ;

	UINT BaseX, BaseY;
	UINT SizeX, SizeY;	

	UBOOL bHasLightmap;

	UMaterialInstance* Material;
	UBOOL bNeedsBumpedLightmap;

	FAvaSurfaceStaticLighting( FAvaComponentStaticLighting* InComponent, UMaterialInstance* InMaterial, const FVector& InTangentX, const FVector& InTangentY, const FVector& InTangentZ )
		: BaseX(0), BaseY(0), SizeX(0), SizeY(0), Component(InComponent), bHasLightmap(FALSE), Material(InMaterial), bNeedsBumpedLightmap(FLightMap2D::NeedsBumpedLightmap( InMaterial )),
		TangentX(InTangentX), TangentY(InTangentY), TangentZ(InTangentZ), LightmapPage(NULL)
	{
	}

	UBOOL Matches( const FAvaSurfaceStaticLighting* Surface ) const;
};

struct FAvaComponentStaticLighting : FRefCountedObject
{
	UObject*					OuterMost;
	FBoxSphereBounds			Bounds;
	INT							NumSurfaces;

	FAvaComponentStaticLighting( UObject* InOuterMost, const FBoxSphereBounds& InBounds )
		: OuterMost(InOuterMost), Bounds(InBounds), NumSurfaces(0)
	{
	}

	virtual void GenerateShadowCoordinates( FAvaSurfaceStaticLighting* Surface ) = 0;
	virtual void FillLightmap( FAvaSurfaceStaticLighting* InSurface, FLightMapData2D* LightMapData, FLightMapData2D *LightMapData_2 ) = 0;
	virtual void RebuildElements( FAvaStaticLighting* StaticLighting ) = 0;
	virtual FAvaSurfaceStaticLighting* GetSurface( INT SurfaceIndex ) = 0;
};

struct FAvaStaticLightmapPage : FRefCountedObject
{		
	FLightMapRef				LightMap;
	UBOOL						bNeedsBumpedLightmap;

	UObject*					OuterMost;
	FBoxSphereBounds			Bounds;

	TArray<FAvaSurfaceStaticLighting*>	Surfaces;
	TArray<UMaterialInstance*>	Materials;

	FTextureLayout*				TextureLayout;

	/**
	* Minimal initialization constructor.
	*/
	FAvaStaticLightmapPage(UINT InSizeX,UINT InSizeY, UBOOL InbNeedsBumpedLightmap,UObject* InOuterMost,const FBoxSphereBounds& InBounds);

	~FAvaStaticLightmapPage();

	void AddSurface( FAvaSurfaceStaticLighting* Surface )
	{
		Surface->LightmapPage = this;

		Bounds = Bounds + Surface->Component->Bounds;

		Surfaces.AddItem( Surface );
		Materials.AddUniqueItem( Surface->Material );
	}

	UBOOL Matches( FAvaSurfaceStaticLighting* Surface )
	{
		return (
			Surface->bNeedsBumpedLightmap == bNeedsBumpedLightmap &&
			Surface->Component->OuterMost == OuterMost
			);
	}
};

struct FAvaStaticLighting
{
	TArray< TRefCountPtr<FAvaStaticLightmapPage> >		Lightmaps;

	virtual void Render() = 0;

	virtual void AddComponent( FAvaComponentStaticLighting* ComponentLighting ) = 0;
	virtual void GenerateLightmaps() = 0;
};

extern FAvaStaticLighting* GAvaStaticLighting;

class AvaRadiosityAdapter;
extern void AvaBeginPackingLightmap( AvaRadiosityAdapter& RadiosityAdapter );
extern void AvaEndPackingLightmap();

#endif

#endif