#include "EnginePrivate.h"
#include "AvaStaticLighting.h"
#include "UnTextureLayout.h"

#if !FINAL_RELEASE

#define SHADOWMAP_MAX_WIDTH			2048
#define SHADOWMAP_MAX_HEIGHT		2048

#define SHADOWMAP_TEXTURE_WIDTH		512
#define SHADOWMAP_TEXTURE_HEIGHT	512

FAvaStaticLighting* GAvaStaticLighting = NULL;

struct FZeroChecker
{
	INT SizeX;
	INT SizeY;

	FLightMapData2D* TempLightmapData;
	FLightMapData2D* TempLightmapData_2;
	FAvaSurfaceStaticLighting* Surface;
	TArray<UBOOL> ZeroTable;	
	UBOOL bHasZero;

	enum
	{
		BLOCKSIZE = 4
	};

	INT Pitch, NumRows;
	FLOAT ZeroCoefficients[NUM_AVA_LIGHTMAPS][3];

	FZeroChecker( INT InSizeX, INT InSizeY )
		: SizeX(InSizeX), SizeY(InSizeY)
	{
		TempLightmapData = new FLightMapData2D( SizeX, SizeY );
		TempLightmapData_2 = new FLightMapData2D( SizeX, SizeY );

		Pitch = (SizeX+BLOCKSIZE-1) / BLOCKSIZE;
		NumRows = (SizeY+BLOCKSIZE-1) / BLOCKSIZE;

		ZeroTable.AddZeroed( Pitch * NumRows );

		appMemzero(ZeroCoefficients,sizeof(ZeroCoefficients));
	}

	~FZeroChecker()
	{	
		delete TempLightmapData;
		delete TempLightmapData_2;
	}

	void SetSurface( FAvaSurfaceStaticLighting* InSurface )
	{
		bHasZero = FALSE;

		Surface = InSurface;

		Surface->BaseX = 0;
		Surface->BaseY = 0;
		Surface->Component->FillLightmap( Surface, TempLightmapData, TempLightmapData_2 );

		INT BlockSizeX = (Surface->SizeX + BLOCKSIZE-1) / BLOCKSIZE;
		INT BlockSizeY = (Surface->SizeY + BLOCKSIZE-1) / BLOCKSIZE;				

		INT NumZero = 0;				

		for (INT Y=0; Y<BlockSizeY; ++Y)
		{
			for (INT X=0; X<BlockSizeX; ++X)
			{
				UBOOL bFound = FALSE;

				for (INT BY=0; BY<BLOCKSIZE && !bFound; ++BY)
				{
					for (INT BX=0; BX<BLOCKSIZE && !bFound; ++BX)
					{
						const FLightSample& Sample = (*TempLightmapData)(X*BLOCKSIZE+BX,Y*BLOCKSIZE+BY);

						if (Sample.bIsMapped && appMemcmp(Sample.Coefficients,ZeroCoefficients,sizeof(ZeroCoefficients)))
						{
							bFound = TRUE;
						}					
					}
				}

				ZeroTable(X+Y*Pitch) = !bFound;

				if (!bFound)
				{							
					bHasZero = TRUE;
				}				
			}
		}

		for (UINT Y=0; Y<Surface->SizeY; ++Y)
		{
			for (UINT X=0; X<Surface->SizeX; ++X)
			{
				(*TempLightmapData)(X,Y).bIsMapped = FALSE;
			}
		}
	}
};

IMPLEMENT_COMPARE_CONSTPOINTER( FAvaSurfaceStaticLighting, UnModelLight, 
{ 
	if (A->Component->OuterMost != B->Component->OuterMost)
	{
		return A->Component->OuterMost - B->Component->OuterMost;
	}

	if (A->bNeedsBumpedLightmap != B->bNeedsBumpedLightmap)
	{
		if (A->bNeedsBumpedLightmap)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	INT DiffSize = B->SizeX * B->SizeY - A->SizeX * A->SizeY; 

	if (DiffSize != 0)
	{
		return DiffSize;
	}

	return B - A;
} )

UBOOL FAvaSurfaceStaticLighting::Matches( const FAvaSurfaceStaticLighting* Surface ) const
{
	return (
		Surface->bNeedsBumpedLightmap == bNeedsBumpedLightmap &&
		Surface->Component->OuterMost == Component->OuterMost
		);
}

FAvaStaticLightmapPage::FAvaStaticLightmapPage(UINT InSizeX,UINT InSizeY, UBOOL InbNeedsBumpedLightmap,UObject* InOuterMost,const FBoxSphereBounds& InBounds)
:	TextureLayout( new FTextureLayout(1,1,InSizeX,InSizeY,true) ),
	LightMap(NULL), bNeedsBumpedLightmap(InbNeedsBumpedLightmap), OuterMost(InOuterMost), Bounds(InBounds)
{}	

FAvaStaticLightmapPage::~FAvaStaticLightmapPage()
{
	delete TextureLayout;
}

struct FAvaStaticLightingImpl : FAvaStaticLighting
{	
	AvaRadiosityAdapter&								RadiosityAdapter;

	INT NumSurfaces;
	TArray< TRefCountPtr<FAvaComponentStaticLighting> >	Components;

	FAvaStaticLightingImpl( AvaRadiosityAdapter& InRadiosityAdapter )
		: RadiosityAdapter(InRadiosityAdapter), NumSurfaces(0)
	{
		GAvaStaticLighting = this;
	}

	~FAvaStaticLightingImpl()
	{
		GAvaStaticLighting = NULL;
	}

	virtual void Render()
	{
		LayoutSurfaces();

		GenerateLightmaps();

		// Group nodes based on their relevant light set.
		for(INT ComponentIndex= 0; ComponentIndex < Components.Num(); ++ComponentIndex)
		{
			FAvaComponentStaticLighting* Component = Components(ComponentIndex);

			Component->RebuildElements( this );
		}
	}

	void LayoutSurfaces()
	{
		TArray<FAvaSurfaceStaticLighting*> Surfaces;

		Surfaces.Reserve(NumSurfaces);

		// Group nodes based on their relevant light set.
		for(INT ComponentIndex= 0; ComponentIndex < Components.Num(); ++ComponentIndex)
		{
			FAvaComponentStaticLighting* Component = Components(ComponentIndex);

			for(INT SurfaceGroupIndex= 0; SurfaceGroupIndex < Component->NumSurfaces; ++SurfaceGroupIndex)
			{	
				FAvaSurfaceStaticLighting* Surface = Component->GetSurface(SurfaceGroupIndex);

				if (Surface->bHasLightmap)
				{	
					Surfaces.AddItem( Surface );
				}
			}
		}

		// Group별로 모읍니다.
		Sort<USE_COMPARE_CONSTPOINTER(FAvaSurfaceStaticLighting, UnModelLight)>(&Surfaces(0), Surfaces.Num());
		for (INT StartIndex=0; StartIndex<Surfaces.Num();)
		{
			FAvaSurfaceStaticLighting* StartSurface = Surfaces(StartIndex);

			INT EndIndex; 
			for (EndIndex=StartIndex+1; EndIndex<Surfaces.Num(); ++EndIndex)
			{
				FAvaSurfaceStaticLighting* EndSurface = Surfaces(EndIndex);

				if (!StartSurface->Matches(EndSurface))
				{
					break;
				}
			}
			
			LayoutSurfaces(&Surfaces(StartIndex), EndIndex-StartIndex);

			StartIndex = EndIndex;
		}		

		// Generate shadowmap coordinates :)
		for(INT SurfaceGroupIndex= 0; SurfaceGroupIndex < Surfaces.Num(); ++SurfaceGroupIndex)
		{		
			FAvaSurfaceStaticLighting* Surface = Surfaces(SurfaceGroupIndex);

			check(Surface->bHasLightmap);

			Surface->Component->GenerateShadowCoordinates( Surface );
		}		
	}

	void LayoutSurfaces( FAvaSurfaceStaticLighting** Surfaces, INT NumSurfaces )
	{
		// Sort the node surface by size
		Sort<USE_COMPARE_CONSTPOINTER(FAvaSurfaceStaticLighting, UnModelLight)>(Surfaces, NumSurfaces);

		INT LightmapStartIndex = Lightmaps.Num();

		for(INT SurfaceGroupIndex= 0; SurfaceGroupIndex < NumSurfaces; ++SurfaceGroupIndex)
		{		
			FAvaSurfaceStaticLighting* Surface = Surfaces[SurfaceGroupIndex];

			check(Surface->bHasLightmap);

			// Find an existing group with the same material.
			for(INT LightGroupIndex = LightmapStartIndex;LightGroupIndex < Lightmaps.Num();LightGroupIndex++)
			{
				FAvaStaticLightmapPage* ExistingPage = Lightmaps(LightGroupIndex);				
				check( ExistingPage->Matches(Surface) );

				if( ExistingPage->TextureLayout->AddElement( &Surface->BaseX, &Surface->BaseY, Surface->SizeX, Surface->SizeY) )
				{
					ExistingPage->AddSurface( Surface );								
					break;
				}				
			}			

			// 새로운 group 생성
			if(Surface->LightmapPage == NULL)
			{
				const UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;

				UINT TextureSizeX = PackedLightAndShadowMapTextureSize;
				UINT TextureSizeY = PackedLightAndShadowMapTextureSize;
				if(Surface->SizeX > TextureSizeX || Surface->SizeY > TextureSizeY)
				{
					TextureSizeX = (Surface->SizeX + 3) & ~3;
					TextureSizeY = (Surface->SizeY + 3) & ~3;
				}
				// The surface didn't fit in any existing group, create a new group.
				FAvaStaticLightmapPage* LightmapPage = ::new FAvaStaticLightmapPage(TextureSizeX,TextureSizeY,Surface->bNeedsBumpedLightmap,Surface->Component->OuterMost,Surface->Component->Bounds);
				Lightmaps.AddItem(LightmapPage);

				verify(LightmapPage->TextureLayout->AddElement(&Surface->BaseX,&Surface->BaseY,Surface->SizeX,Surface->SizeY));			
				LightmapPage->AddSurface( Surface );
			}							
		}	

		// check if there was split
		INT NumLightmaps = Lightmaps.Num() - LightmapStartIndex;		

		INT MaxSizeX = 16, MaxSizeY = 16;

		for(INT LightGroupIndex = LightmapStartIndex;LightGroupIndex < Lightmaps.Num();LightGroupIndex++)
		{
			FAvaStaticLightmapPage* ExistingPage = Lightmaps(LightGroupIndex);				

			debugf( TEXT("Lightmap %d - %d x %d"), LightGroupIndex, ExistingPage->TextureLayout->GetSizeX(), ExistingPage->TextureLayout->GetSizeY() );

			MaxSizeX = Max<INT>( MaxSizeX, ExistingPage->TextureLayout->GetSizeX() );
			MaxSizeY = Max<INT>( MaxSizeY, ExistingPage->TextureLayout->GetSizeY() );
		}

		// Revert all :)
		Lightmaps.Remove(LightmapStartIndex,NumLightmaps);

#if ENABLE_CHEKC_ZEROES
		FZeroChecker ZeroChecker( MaxSizeX, MaxSizeY );		
#endif

		for(INT SurfaceGroupIndex= 0; SurfaceGroupIndex < NumSurfaces; ++SurfaceGroupIndex)
		{		
			FAvaSurfaceStaticLighting* Surface = Surfaces[SurfaceGroupIndex];

			check(Surface->bHasLightmap);

#if ENABLE_CHEKC_ZEROES
			ZeroChecker.SetSurface( Surface );
#endif

			// 다시 넣을 것입니다. ;)
			Surface->LightmapPage = NULL;
			
			INT BestPageIndex = -1;

			// material이 속해 있는 page가 가장 좋은 page
			for(INT LightGroupIndex = LightmapStartIndex;LightGroupIndex < Lightmaps.Num();LightGroupIndex++)
			{
				FAvaStaticLightmapPage* ExistingPage = Lightmaps(LightGroupIndex);				
				check( ExistingPage->Matches(Surface) );

				if (ExistingPage->Materials.ContainsItem(Surface->Material))
				{
					BestPageIndex = LightmapStartIndex;
					break;
				}
			}

			// 없을 때는 그냥 round robin
			if (BestPageIndex < 0)
			{				
				BestPageIndex = (SurfaceGroupIndex % NumLightmaps) + LightmapStartIndex;
			}

			// 이미 갖고 있는 경우에는 넣어욤~
			if (BestPageIndex < Lightmaps.Num())
			{
				// Find an existing group with the same material.
				for(INT LightGroupIndex = 0;LightGroupIndex < Lightmaps.Num() - LightmapStartIndex;LightGroupIndex++)
				{
					INT Index = LightGroupIndex;
					
					if (Index < NumLightmaps)
					{
						Index = (Index + BestPageIndex) % NumLightmaps;
					}

					Index += LightmapStartIndex;

					FAvaStaticLightmapPage* ExistingPage = Lightmaps(Index);
					check( ExistingPage->Matches(Surface) );

					if( ExistingPage->TextureLayout->AddElement( 
						&Surface->BaseX, &Surface->BaseY, Surface->SizeX, Surface->SizeY, 
#if ENABLE_CHEKC_ZEROES
						ZeroChecker.bHasZero ? &ZeroChecker.ZeroTable(0) : NULL, ZeroChecker.bHasZero ? ZeroChecker.Pitch : 0 
#else
						NULL, 0
#endif
						) )
					{
						ExistingPage->AddSurface( Surface );														
						break;
					}				
				}			
			}		


			// 못 넣은 경우에 대한 처리
			if(Surface->LightmapPage == NULL)
			{
				const UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;

				UINT TextureSizeX = PackedLightAndShadowMapTextureSize;
				UINT TextureSizeY = PackedLightAndShadowMapTextureSize;
				if(Surface->SizeX > TextureSizeX || Surface->SizeY > TextureSizeY)
				{
					TextureSizeX = (Surface->SizeX + 3) & ~3;
					TextureSizeY = (Surface->SizeY + 3) & ~3;
				}
				// The surface didn't fit in any existing group, create a new group.
				FAvaStaticLightmapPage* LightmapPage = ::new FAvaStaticLightmapPage(TextureSizeX,TextureSizeY,Surface->bNeedsBumpedLightmap,Surface->Component->OuterMost,Surface->Component->Bounds);
				Lightmaps.AddItem(LightmapPage);

#if ENABLE_CHEKC_ZEROES
				verify(LightmapPage->TextureLayout->AddElement(
					&Surface->BaseX,&Surface->BaseY,Surface->SizeX,Surface->SizeY, 
					ZeroChecker.bHasZero ? &ZeroChecker.ZeroTable(0) : NULL, ZeroChecker.bHasZero ? ZeroChecker.Pitch : 0 )); 					
#else
				verify(LightmapPage->TextureLayout->AddElement(
					&Surface->BaseX,&Surface->BaseY,Surface->SizeX,Surface->SizeY, 					
					NULL, 0 ));			
#endif
					
				LightmapPage->AddSurface( Surface );
			}										
		}	

		for(INT LightGroupIndex = LightmapStartIndex;LightGroupIndex < Lightmaps.Num();LightGroupIndex++)
		{
			FAvaStaticLightmapPage* ExistingPage = Lightmaps(LightGroupIndex);				

			debugf( TEXT("Lightmap %d - %d x %d"), LightGroupIndex, ExistingPage->TextureLayout->GetSizeX(), ExistingPage->TextureLayout->GetSizeY() );
		}		
	}

	void GenerateLightmaps()
	{
		TArray<ULightComponent*>		RadiosityLights;

		// gather baked lights :)
		ULightComponent* SunLight = NULL;
		for(TObjectIterator<ULightComponent> LightIt;LightIt;++LightIt)
		{
			const UBOOL bLightIsInWorld = LightIt->GetOwner() && GWorld->ContainsActor(LightIt->GetOwner());
			if (bLightIsInWorld && LightIt->UseDirectLightMap)
			{
				RadiosityLights.AddItem( *LightIt );
			}

			//<@ 2008. 1. 24 changmin
			// add cascaded shadow
			if( LightIt->bUseCascadedShadowmap )
			{
				SunLight = Cast<UDirectionalLightComponent>(*LightIt);
			}
			//>@ 
		}		

		// lightgroup마다 lightmap 생성
		for(INT LightmapIndex = 0;LightmapIndex < Lightmaps.Num();LightmapIndex++)
		{
			FAvaStaticLightmapPage* LightmapPage = Lightmaps(LightmapIndex);

			FLightMapData2D		*LightMapData	= new FLightMapData2D(LightmapPage->TextureLayout->GetSizeX(), LightmapPage->TextureLayout->GetSizeY());
			FLightMapData2D		*LightMapData_2 = new FLightMapData2D(LightmapPage->TextureLayout->GetSizeX(), LightmapPage->TextureLayout->GetSizeY());	// 2007. 12. 3 changmin ; realtime sun shadow
			LightMapData->Lights.Append( RadiosityLights );

			// Compute the light-map data for each surface in the group.
			for(INT SurfaceIndex = 0; SurfaceIndex < LightmapPage->Surfaces.Num();SurfaceIndex++)
			{			
				FAvaSurfaceStaticLighting* Surface = LightmapPage->Surfaces(SurfaceIndex);				

				Surface->Component->FillLightmap( Surface, LightMapData, LightMapData_2 );
			}			

			LightmapPage->LightMap = FLightMap2D::AllocateLightMap( LightmapPage->OuterMost, LightMapData, LightMapData_2, LightmapPage->bNeedsBumpedLightmap, LightmapPage->Bounds, RadiosityAdapter.bSupportsCascadedShadow_, LightmapPage->TextureLayout );						
			LightmapPage->TextureLayout = NULL;

			delete LightMapData;
			delete LightMapData_2;
			LightMapData = NULL;
			LightMapData_2 = NULL;
		}		
	}

	virtual void AddComponent( FAvaComponentStaticLighting* ComponentLighting ) 
	{
		Components.AddItem( ComponentLighting );		

		NumSurfaces += ComponentLighting->NumSurfaces;
	}
};

void AvaBeginPackingLightmap( AvaRadiosityAdapter& RadiosityAdapter )
{
	new FAvaStaticLightingImpl( RadiosityAdapter );
}

void AvaEndPackingLightmap()
{
	GAvaStaticLighting->Render();		

	delete GAvaStaticLighting;
}

static UBOOL HasZero( const UBOOL* IsMapped, INT Pitch, INT NumRows )
{
	for (INT Y=0; Y<NumRows; ++Y)
	{
		if (!IsMapped[Pitch*Y])
			return TRUE;
	}

	return FALSE;
}

void FTextureLayout::GatherReusableArea( INT NodeIndex, const UBOOL* ZeroTable, INT ZeroPitch)
{
	FTextureLayoutNode& Node = Nodes(NodeIndex);	

	INT BlockSizeX = (Node.SizeX + 3) / 4;
	INT BlockSizeY = (Node.SizeY + 3) / 4;
	
	INT NumZeros = 0;

	for (INT Y=0; Y<BlockSizeY; ++Y)
	{
		for (INT X=0; X<BlockSizeX; ++X)
		{
			if (ZeroTable[X+Y*ZeroPitch])
			{
				NumZeros++;
			}
		}
	}
}

#endif