/*=============================================================================
	UnLightMap.cpp: Light-map implementation.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

//!{ 2006-04-21	허 창 민
// compress 때문에 문제가 되는지 확인해 보자
//#define COMPRESS_LIGHTMAPS 1
#define COMPRESS_LIGHTMAPS 0
//!} 2006-04-21	허 창 민

//#pragma DISABLE_OPTIMIZATION


/** Fudge factor to roughly match up lightmaps and regular lighting. */
#define LIGHTMAP_COLOR_FUDGE_FACTOR 1.5f
/** Maximum light intensity stored in vertex/ texture lightmaps. */
#define MAX_LIGHT_INTENSITY	16.f

#if _MSC_VER && !CONSOLE
#pragma pack (push,8)
#include "../../../nvDXT/Inc/dxtlib.h"
#pragma pack (pop)
#endif


#include "UnTextureLayout.h"

IMPLEMENT_CLASS(ULightMapTexture2D);

static FVector LightMapBasis[3] =
{
	FVector(	0.0f,						appSqrt(6.0f) / 3.0f,			1.0f / appSqrt(3.0f)),
	FVector(	-1.0f / appSqrt(2.0f),		-1.0f / appSqrt(6.0f),			1.0f / appSqrt(3.0f)),
	FVector(	+1.0f / appSqrt(2.0f),		-1.0f / appSqrt(6.0f),			1.0f / appSqrt(3.0f)),
};

FLightSample::FLightSample(const FLinearColor& Color,const FVector& Direction)
{
	FLinearColor FudgedColor = Color * LIGHTMAP_COLOR_FUDGE_FACTOR;

	//<@ ava specific ; 2007. 1. 19 changmin
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
	{
		const FLOAT CoefficientScale = (CoefficientIndex == 0) ? Max(0.0f, Direction | FVector(0.0f, 0.0f, 1.0f) ) : Max(0.0f,Direction | LightMapBasis[CoefficientIndex]);
		Coefficients[CoefficientIndex][0] = FudgedColor.R * CoefficientScale;
		Coefficients[CoefficientIndex][1] = FudgedColor.G * CoefficientScale;
		Coefficients[CoefficientIndex][2] = FudgedColor.B * CoefficientScale;
	}
	//>@ ava

	// it's epic's original
	//for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	//{
	//	const FLOAT CoefficientScale = Max(0.0f,Direction | LightMapBasis[CoefficientIndex]);
	//	Coefficients[CoefficientIndex][0] = FudgedColor.R * CoefficientScale;
	//	Coefficients[CoefficientIndex][1] = FudgedColor.G * CoefficientScale;
	//	Coefficients[CoefficientIndex][2] = FudgedColor.B * CoefficientScale;
	//}
}

void FLightSample::AddWeighted(const FLightSample& OtherSample,FLOAT Weight)
{
	//<@ ava specific ; 2007. 1. 19 changmin
	for(INT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
	{
		for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] + OtherSample.Coefficients[CoefficientIndex][ColorIndex] * Weight;
		}
	}
	//>@ ava

	// it's epic's original
	//for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	//{
	//	for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
	//	{
	//		Coefficients[CoefficientIndex][ColorIndex] = Coefficients[CoefficientIndex][ColorIndex] + OtherSample.Coefficients[CoefficientIndex][ColorIndex] * Weight;
	//	}
	//}
}

void FLightMap::Serialize(FArchive& Ar)
{
	// ava specific ; 2007. 12. 21 changmin
	// add cascaded shadow
	if( Ar.LicenseeVer() < VER_AVA_ADD_CASCADED_SHADOWMAP )
	{
		Ar << LightGuids;
		bSupportsCascadedShadow = FALSE;
	}
	else
	{
		Ar << LightGuids;
		Ar << bSupportsCascadedShadow;
	}
}

void FLightMap::FinishCleanup()
{
	delete this;
}


void ULightMapTexture2D::Serialize(FArchive& Ar)
{
	if(Ar.Ver() < VER_RENDERING_REFACTOR)
	{
		// ULightMapTexture2D used to serialize FStaticTexture2D before Super::Serialize, whereas UTexture2D was the other way around.
		LegacySerialize(Ar);
		UTexture::Serialize(Ar);
	}
	else
	{
		Super::Serialize(Ar);
	}
}


/** 
 * Returns a one line description of an object for viewing in the generic browser
 */
FString ULightMapTexture2D::GetDesc()
{
	return FString::Printf( TEXT("Lightmap: %dx%d [%s]"), SizeX, SizeY, GPixelFormats[Format].Name );
}

/** 
 * Returns detailed info to populate listview columns
 */
FString ULightMapTexture2D::GetDetailedDescription( INT Index )
{
	FString Description = TEXT( "" );
	switch( Index )
	{
	case 0:
		Description = FString::Printf( TEXT( "%dx%d" ), SizeX, SizeY );
		break;
	case 1:
		Description = GPixelFormats[Format].Name;
		break;
	}
	return( Description );
}

/**
 * The quantized coefficients for a single light-map texel.
 */
struct FLightMapCoefficients
{
	BYTE Coverage;
	//BYTE Coefficients[NUM_LIGHTMAP_COEFFICIENTS][3];
	//<@ ava specific ; 2007. 1. 19 changmin
	FLOAT Coefficients[NUM_AVA_LIGHTMAPS][3];
	//>@ ava
};

/**
 * An allocation of a region of light-map texture to a specific light-map.
 */

#if _MSC_VER && !CONSOLE
struct FLightMapAllocation
{
	FLightMap2D* LightMap;
	UObject* Outer;
	UINT BaseX;
	UINT BaseY;
	UINT SizeX;
	UINT SizeY;

	//@ADD deif ; RawData를 policy마다 들고 있지 않고 하나에 몰아서 보관한다. 
	// 이에 따라 policy별 raw data의 시작 위치를 기록할 공간이 필요하다.
	INT RawDataOffsetPerPolicy[NUM_LIGHTMAP_POLICES];

	TArray<FLightMapCoefficients> RawData;	

	//FLOAT Scale[NUM_LIGHTMAP_COEFFICIENTS][3];
	//<@ ava specific ; 2007. 1. 19 changmin
	FLOAT Scale[NUM_AVA_LIGHTMAPS][3];
	//>@ ava

	UBOOL bNeedsBumpedLightmap;

	FTextureLayout* TextureLayout;
};

extern NV_ERROR_CODE CompressionCallback(const void *Data, size_t NumBytes, const MIPMapData * MipMapData, void * UserData );

/**
 * A light-map texture which has been partially allocated, but not yet encoded.
 */
struct FLightMapPendingTexture: FTextureLayout
{
	TArray<FLightMapAllocation*> Allocations;
	UBOOL bNeedsBumpedLightmap;
	UObject* Outer;
	UBOOL bNeedsBump;
	UBOOL bSupportsCascadedShadow;	// ava specific ; 2007. 12. 21 changmin ; add cascaded shadow

	FLightMapPendingTexture(UINT InSizeX,UINT InSizeY,UBOOL bNeedsBump, UBOOL bInSupportsCascadedShadow ):
#if COMPRESS_LIGHTMAPS
	FTextureLayout(GPixelFormats[PF_DXT1].BlockSizeX,GPixelFormats[PF_DXT1].BlockSizeY,InSizeX,InSizeY,true)
#else
	FTextureLayout(GPixelFormats[PF_A8R8G8B8].BlockSizeX,GPixelFormats[PF_A8R8G8B8].BlockSizeY,InSizeX,InSizeY,true)
#endif
			, bNeedsBump(bNeedsBump)
			, bSupportsCascadedShadow( bInSupportsCascadedShadow)	 // ava specific ; 2007. 12. 21
	{}	

	FLightMapPendingTexture(const FTextureLayout& LayoutToCopy,UBOOL bNeedsBump, UBOOL bInSupportsCascadedShadow ):
	FTextureLayout(LayoutToCopy)
		, bNeedsBump(bNeedsBump)
		, bSupportsCascadedShadow( bInSupportsCascadedShadow)	 // ava specific ; 2007. 12. 21
	{}	

	//@EDIT deif ; support lightmap policy
	void Encode()
	{
		EncodeLightmaps<FOldLightmapPolicy>();

		if (bSupportsCascadedShadow)
		{
			EncodeLightmaps<FSunExcludedLightmapPolicy>();
		}		

		for(INT AllocationIndex = 0;AllocationIndex < Allocations.Num();AllocationIndex++)
		{
			FLightMapAllocation* Allocation = Allocations(AllocationIndex);

			// Calculate the coordinate scale/biases this light-map.
			Allocation->LightMap->CoordinateScale = FVector2D(
				(FLOAT)Allocation->SizeX / (FLOAT)GetSizeX(),
				(FLOAT)Allocation->SizeY / (FLOAT)GetSizeY()
				);
			Allocation->LightMap->CoordinateBias = FVector2D(
				(FLOAT)(Allocation->BaseX + 0.5f) / (FLOAT)GetSizeX(),
				(FLOAT)(Allocation->BaseY + 0.5f) / (FLOAT)GetSizeY()
				);

			// Free the light-map's raw data.
			Allocation->RawData.Empty();			
		}
	}

	//@EDIT deif ; support lightmap policy
	template <typename LightmapPolicy>
	void EncodeLightmaps()
	{
		// Encode and compress the coefficient textures.		
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)		
		{
			// assign non-bump lightmaps as bumped lightmaps :)
			if (!bNeedsBump && CoefficientIndex > 0)
			{
				for(INT AllocationIndex = 0;AllocationIndex < Allocations.Num();AllocationIndex++)
				{
					FLightMapAllocation* Allocation = Allocations(AllocationIndex);

					// Link the light-map to the texture.
					Allocation->LightMap->Texture<LightmapPolicy>(CoefficientIndex) = Allocation->LightMap->Texture<LightmapPolicy>(0);
				}				

				continue;
			}
		
			// Create the light-map texture for this coefficient.
			ULightMapTexture2D* Texture = new(Outer) ULightMapTexture2D;
			Texture->SizeX		= GetSizeX();
			Texture->SizeY		= GetSizeY();
#if COMPRESS_LIGHTMAPS
			Texture->Format		= PF_DXT1;
#else
			Texture->Format		= PF_A8R8G8B8;
#endif
			Texture->LODGroup	= TEXTUREGROUP_LightAndShadowMap;
			Texture->NeverStream= TRUE;

			// Create the uncompressed top mip-level.
			TArray< TArray<FLinearColor> > MipData;
			TArray<FLinearColor>* TopMipData = new(MipData) TArray<FLinearColor>();
			TopMipData->Empty(GetSizeX() * GetSizeY());
			TopMipData->AddZeroed(GetSizeX() * GetSizeY());
			for(INT AllocationIndex = 0;AllocationIndex < Allocations.Num();AllocationIndex++)
			{
				FLightMapAllocation* Allocation = Allocations(AllocationIndex);
				
				// Link the light-map to the texture.
				Allocation->LightMap->Texture<LightmapPolicy>(CoefficientIndex) = Texture;

				const INT RawDataOffset = Allocation->RawDataOffsetPerPolicy[LightmapPolicy::PolicyIndex];
				
				// Copy the raw data for this light-map into the raw texture data array.
				for(UINT Y = 0;Y < Allocation->SizeY;Y++)
				{
					for(UINT X = 0;X < Allocation->SizeX;X++)
					{
						FLinearColor& DestColor = (*TopMipData)((Y + Allocation->BaseY) * Texture->SizeX + X + Allocation->BaseX);
						//<@ ava specific ; 2007. 12. 3 changmin
						//const FLightMapCoefficients& SourceCoefficients = Allocation->RawData(Y * Allocation->SizeX + X);
						const FLightMapCoefficients& SourceCoefficients = Allocation->RawData( RawDataOffset +  Y * Allocation->SizeX + X);
						//>@ ava
#if VISUALIZE_PACKING						
						if( X == 0 || Y == 0 || X==Allocation->SizeX-1 || Y==Allocation->SizeY-1 
						||	X == 1 || Y == 1 || X==Allocation->SizeX-2 || Y==Allocation->SizeY-2 )
						{
							DestColor = FColor(255,0,0);
						}
						else
						{
							DestColor = FColor(0,255,0);
						}
#else
						DestColor.R = SourceCoefficients.Coefficients[CoefficientIndex][0];
						DestColor.G = SourceCoefficients.Coefficients[CoefficientIndex][1];
						DestColor.B = SourceCoefficients.Coefficients[CoefficientIndex][2];
						DestColor.A = SourceCoefficients.Coverage;
#endif
					}
				}
			}

			UINT NumMips = Max(appCeilLogTwo(Texture->SizeX),appCeilLogTwo(Texture->SizeY)) + 1;
			for(UINT MipIndex = 1;MipIndex < NumMips;MipIndex++)
			{
				UINT SourceMipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> (MipIndex - 1));
				UINT SourceMipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> (MipIndex - 1));
				UINT DestMipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
				UINT DestMipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

				// Downsample the previous mip-level, taking into account which texels are mapped.
				TArray<FLinearColor>* NextMipData = new(MipData) TArray<FLinearColor>();
				NextMipData->Empty(DestMipSizeX * DestMipSizeY);
				NextMipData->AddZeroed(DestMipSizeX * DestMipSizeY);
				UINT MipFactorX = SourceMipSizeX / DestMipSizeX;
				UINT MipFactorY = SourceMipSizeY / DestMipSizeY;
				for(UINT Y = 0;Y < DestMipSizeY;Y++)
				{
					for(UINT X = 0;X < DestMipSizeX;X++)
					{
						FLinearColor AccumulatedColor = FLinearColor::Black;
						UINT Coverage = 0;
						for(UINT SourceY = Y * MipFactorY;SourceY < (Y + 1) * MipFactorY;SourceY++)
						{
							for(UINT SourceX = X * MipFactorX;SourceX < (X + 1) * MipFactorX;SourceX++)
							{
								FLinearColor& SourceColor = MipData(MipIndex - 1)(SourceY * SourceMipSizeX + SourceX);
								if(SourceColor.A)
								{
									AccumulatedColor += SourceColor * SourceColor.A;
									Coverage += SourceColor.A;
								}
							}
						}
						if(Coverage)
						{
							FLinearColor AverageColor(AccumulatedColor / Coverage);
							(*NextMipData)(Y * DestMipSizeX + X) = FLinearColor(AverageColor.R,AverageColor.G,AverageColor.B,Coverage / (MipFactorX * MipFactorY));
						}
						else
						{
							(*NextMipData)(Y * DestMipSizeX + X) = FLinearColor(0,0,0,0);
						}
					}
				}
			}

			// Expand texels which are mapped into adjacent texels which are not mapped to avoid artifacts when using texture filtering.
			for(INT MipIndex = 0;MipIndex < MipData.Num();MipIndex++)
			{
				UINT MipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
				UINT MipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);
				for(UINT DestY = 0;DestY < MipSizeY;DestY++)
				{
					for(UINT DestX = 0;DestX < MipSizeX;DestX++)
					{
						FLinearColor AccumulatedColor = FLinearColor::Black;
						UINT Coverage = 0;
						for(INT SourceY = (INT)DestY - 1;SourceY <= (INT)DestY + 1;SourceY++)
						{
							if(SourceY >= 0 && SourceY < (INT)MipSizeY)
							{
								for(INT SourceX = (INT)DestX - 1;SourceX <= (INT)DestX + 1;SourceX++)
								{
									if(SourceX >= 0 && SourceX < (INT)MipSizeX)
									{
										FLinearColor& SourceColor = MipData(MipIndex)(SourceY * MipSizeX + SourceX);
										if(SourceColor.A)
										{
											static const UINT Weights[3][3] =
											{
												{ 1, 255, 1 },
												{ 255, 0, 255 },
												{ 1, 255, 1 },
											};
											AccumulatedColor += SourceColor * SourceColor.A * Weights[SourceX - DestX + 1][SourceY - DestY + 1];
											Coverage += SourceColor.A * Weights[SourceX - DestX + 1][SourceY - DestY + 1];
										}
									}
								}
							}
						}

						FLinearColor& DestColor = MipData(MipIndex)(DestY * MipSizeX + DestX);
						if(DestColor.A == 0 && Coverage)
						{
							DestColor = AccumulatedColor / Coverage;
							DestColor.A = 0;
						}
					}
				}
			}

			for(INT MipIndex = 0;MipIndex < MipData.Num();MipIndex++)
			{
				UINT MipSizeX = Max(GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
				UINT MipSizeY = Max(GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

				// Compress this mip-level.
				nvCompressionOptions nvOptions; 
				nvOptions.mipMapGeneration		= kNoMipMaps;
				nvOptions.textureType			= kTextureTypeTexture2D;
#if COMPRESS_LIGHTMAPS
				nvOptions.textureFormat			= kDXT1;
#else
				nvOptions.textureFormat 		= k8888;
#endif
				nvOptions.user_data				= Texture;
				nvOptions.bEnableFilterGamma	= TRUE;
				nvOptions.filterGamma			= 2.2f;

				TArray<FColor> GPUMipData;
				GPUMipData.Empty(MipSizeX * MipSizeY);
				GPUMipData.AddZeroed(MipSizeX * MipSizeY);

				FColor* DestColor = &GPUMipData(0);
				const FLinearColor* SourceColor = &MipData(MipIndex)(0);

				for (INT PixelCount=MipSizeX * MipSizeY; PixelCount > 0; --PixelCount)
				{
					PREFETCH( SourceColor + 1 );
					PREFETCH( DestColor + 1 );

					FLOAT MaxComponent = 0;

					for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
					{						
						MaxComponent = Max( MaxComponent, SourceColor->Component( ColorIndex ) );
					}				

					MaxComponent = Clamp( MaxComponent, 1 / 2.0f/*MAX_LIGHT_INTENSITY / 255.0f*/, MAX_LIGHT_INTENSITY );					

					FLOAT InvMaxComponent = MaxComponent > 0 ? 1.0f / MaxComponent : 1;

					BYTE Color[3];

					for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
					{
						Color[ColorIndex] = (BYTE)Clamp<INT>( 
							appTrunc(
							appPow(
							(SourceColor->Component( ColorIndex ) * InvMaxComponent),
							1.0f / 2.2f
							) * 255.0f
							),
							0,
							255);

							/*appTrunc( 
							(SourceColor->Component( ColorIndex ) * InvMaxComponent) * 255.0f ), 
							0, 
							255);*/
					}

					DestColor->R = Color[0];
					DestColor->G = Color[1];
					DestColor->B = Color[2];
					DestColor->A = (BYTE)Clamp<INT>( MaxComponent / MAX_LIGHT_INTENSITY * 255.0f, 0, 255 );

					SourceColor++;
					DestColor++;
				}

				// Compress...
				nvDDS::nvDXTcompress(
					(BYTE*)&GPUMipData(0),	// src
					MipSizeX,						// width
					MipSizeY,						// height
					MipSizeX * 4,					// pitch
					nvBGRA,							// pixel order
					&nvOptions,						// compression options
					CompressionCallback				// callback
					);
							
				/*FTexture2DMipMap* MipMap = new(Texture->Mips)FTexture2DMipMap;

				MipMap->SizeX = Max<UINT>(MipSizeX,4);
				MipMap->SizeY = Max<UINT>(MipSizeY,4);
				MipMap->Data.Lock(LOCK_READ_WRITE);
				appMemcpy( MipMap->Data.Realloc(MipSizeX * MipSizeY * 4), &GPUMipData(0), MipSizeX * MipSizeY * 4 );
				MipMap->Data.Unlock();*/
			}

			// Update the texture resource.
			Texture->UpdateResource();			
		}
	}
};

/** The light-maps which have not yet been encoded into textures. */
static TIndirectArray<FLightMapAllocation> PendingLightMaps;
static UINT PendingLightMapSize = 0;
#endif

//<@ ava specific ; 2007. 12. 3 changmin
// realtime sun light shadow를 위해 확장. RawData = Lightmap / RawData2 = Lightmap without sun
//FLightMap2D* FLightMap2D::AllocateLightMap(UObject* LightMapOuter,FLightMapData2D* RawData,UMaterialInstance* Material,const FBoxSphereBounds& Bounds)
FLightMap2D* FLightMap2D::AllocateLightMap(UObject* LightMapOuter,FLightMapData2D* RawData, FLightMapData2D* RawData2, UBOOL bNeedsBumpedLightmap,const FBoxSphereBounds& Bounds, UBOOL bInSupportsCascadedShadow, FTextureLayout* TextureLayout )
//>@ ava
{
	// If the light-map has no lights in it, return NULL.
	if(!RawData || !RawData->Lights.Num())
	{
		return NULL;
	}

#if _MSC_VER && !CONSOLE
	FLightMapAllocation* Allocation = new(PendingLightMaps) FLightMapAllocation;
	Allocation->bNeedsBumpedLightmap = bNeedsBumpedLightmap;
	Allocation->Outer = LightMapOuter->GetOutermost();	
	Allocation->TextureLayout = TextureLayout;

	// Gather the GUIDs of lights stored in the light-map.
	TArray<FGuid> LightGuids;
	for(INT LightIndex = 0;LightIndex < RawData->Lights.Num();LightIndex++)
	{
		LightGuids.AddItem(RawData->Lights(LightIndex)->LightmapGuid);
	}

	// Create a new light-map.
	//<@ ava specific ; 2007. 12. 21 changmin
	// add cascaded shadow
	//FLightMap2D* LightMap = new FLightMap2D(LightGuids);
	FLightMap2D* LightMap = new FLightMap2D(LightGuids, bInSupportsCascadedShadow );
	//>@ ava
	Allocation->LightMap = LightMap;

	// Quantize the coefficients for this texture.
	Allocation->SizeX = RawData->GetSizeX();
	Allocation->SizeY = RawData->GetSizeY();
	Allocation->RawData.Empty(RawData->GetSizeX() * RawData->GetSizeY() + RawData2->GetSizeX() * RawData2->GetSizeY());

	INT RawDataOffset;
	Allocation->RawDataOffsetPerPolicy[FOldLightmapPolicy::PolicyIndex] = RawDataOffset = Allocation->RawData.Num();

	Allocation->RawData.Add(RawData->GetSizeX() * RawData->GetSizeY());
	for(UINT Y = 0;Y < RawData->GetSizeY();Y++)
	{
		for(UINT X = 0;X < RawData->GetSizeX();X++)
		{
			const FLightSample& SourceSample = (*RawData)(X,Y);
			FLightMapCoefficients& DestCoefficients = Allocation->RawData(RawDataOffset + Y * RawData->GetSizeX() + X);
			DestCoefficients.Coverage = SourceSample.bIsMapped ? 255 : 0;
			//for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			//<@ ava specific ; 2007. 1. 19 changmin
			for(INT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
			//>@ ava
			{
				for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
				{
					DestCoefficients.Coefficients[CoefficientIndex][ColorIndex] = SourceSample.Coefficients[CoefficientIndex][ColorIndex];
				}
			}
		}
	}

	//<@ ava sepcific ; 2007. 12. 3
	// realtime sun shadow		
	Allocation->RawDataOffsetPerPolicy[FSunExcludedLightmapPolicy::PolicyIndex] = RawDataOffset = Allocation->RawData.Num();

	Allocation->RawData.Add(RawData2->GetSizeX() * RawData2->GetSizeY());

	for(UINT Y = 0;Y < RawData2->GetSizeY();Y++)
	{
		for(UINT X = 0;X < RawData2->GetSizeX();X++)
		{
			const FLightSample& SourceSample = (*RawData2)(X,Y);
			FLightMapCoefficients& DestCoefficients = Allocation->RawData(RawDataOffset + Y * RawData2->GetSizeX() + X);
			DestCoefficients.Coverage = SourceSample.bIsMapped ? 255 : 0;
			//for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			//<@ ava specific ; 2007. 1. 19 changmin
			for(INT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				//>@ ava
			{
				for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
				{
					DestCoefficients.Coefficients[CoefficientIndex][ColorIndex] = SourceSample.Coefficients[CoefficientIndex][ColorIndex];
				}
			}
		}
	}
	//>@ ava

	// Track the size of pending light-maps.
	PendingLightMapSize += Allocation->SizeX * Allocation->SizeY;

	// Once there are enough pending light-maps, flush encoding.
	const UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;
	const UINT MaxPendingLightMapSize = Square(PackedLightAndShadowMapTextureSize) * 4;
	if(PendingLightMapSize >= MaxPendingLightMapSize)
	{
		FinishEncoding();
	}

	return LightMap;
#else
	return NULL;
#endif
}

#if _MSC_VER && !CONSOLE
// epic's code
//IMPLEMENT_COMPARE_POINTER(FLightMapAllocation,UnLightMap,{ return Max(B->SizeX,B->SizeY) - Max(A->SizeX,A->SizeY); });

//<@ ava specific ; 2006. 12. 05 changmin
IMPLEMENT_COMPARE_POINTER(FLightMapAllocation,UnLightMap,
							
{ 
	if( A->TextureLayout || B->TextureLayout)
	{
		return B->TextureLayout - A->TextureLayout;
	}

	if( A->bNeedsBumpedLightmap == B->bNeedsBumpedLightmap )
		return (Max(B->SizeX,B->SizeY) - Max(A->SizeX,A->SizeY));
	else
		return (B->bNeedsBumpedLightmap-A->bNeedsBumpedLightmap);
} );

//>@ ava
#endif

void FLightMap2D::FinishEncoding()
{
#if _MSC_VER && !CONSOLE
	GWarn->BeginSlowTask(TEXT("Encoding light-maps"),1);

	UINT PackedLightAndShadowMapTextureSize = GWorld->GetWorldInfo()->PackedLightAndShadowMapTextureSize;

	// Reset the pending light-map size.
	PendingLightMapSize = 0;

	//<@ ava specific ; 2006. 12. 05. changmin
	// Sort the light-maps in descending order by size.
	Sort<USE_COMPARE_POINTER(FLightMapAllocation,UnLightMap)>((FLightMapAllocation**)PendingLightMaps.GetData(),PendingLightMaps.Num());
	// debug result
	for( INT LightMapIndex = 0; LightMapIndex < PendingLightMaps.Num(); ++LightMapIndex )
	{
		FLightMapAllocation& Allocation = PendingLightMaps(LightMapIndex);
		debugf( NAME_Log, TEXT("lightmap allocation(%d), bump(%d), size(%d,%d)"), LightMapIndex, Allocation.bNeedsBumpedLightmap, Allocation.SizeX, Allocation.SizeY );
	}
	//>@ ava

	// Allocate texture space for each light-map.
	TIndirectArray<FLightMapPendingTexture> PendingTextures;
	for(INT LightMapIndex = 0;LightMapIndex < PendingLightMaps.Num();LightMapIndex++)
	{
		FLightMapAllocation& Allocation = PendingLightMaps(LightMapIndex);
		FLightMapPendingTexture* Texture = NULL;
		
		UBOOL bNeedsBump = Allocation.bNeedsBumpedLightmap;

		// Lightmaps will always be 4-pixel aligned...
		UINT PaddingBaseX = 0;
		UINT PaddingBaseY = 0;

		// 큰 texture다! BSP 같은 것. 이것은 바로 pending texture로 가야 함!
		UBOOL bIsHugeAllocation = Allocation.TextureLayout != NULL;
		
		if (!bIsHugeAllocation)
		{
			// Find an existing texture which the light-map can be stored in.		
			for(INT TextureIndex = 0;TextureIndex < PendingTextures.Num();TextureIndex++)
			{
				FLightMapPendingTexture* ExistingTexture = &PendingTextures(TextureIndex);
				if( ExistingTexture->Outer == Allocation.Outer && ExistingTexture->bNeedsBumpedLightmap == Allocation.bNeedsBumpedLightmap )
				{
					if(ExistingTexture->AddElement(&PaddingBaseX,&PaddingBaseY,Allocation.SizeX,Allocation.SizeY))
					{
						Texture = ExistingTexture;
						break;
					}
				}
			}
		}		

		if(!Texture)
		{
			if (bIsHugeAllocation)
			{
				Texture = new(PendingTextures) FLightMapPendingTexture(*Allocation.TextureLayout,bNeedsBump, Allocation.LightMap->bSupportsCascadedShadow);

				delete Allocation.TextureLayout;
				Allocation.TextureLayout = NULL;
			}
			else
			{
				UINT NewTextureSizeX = PackedLightAndShadowMapTextureSize;
				UINT NewTextureSizeY = PackedLightAndShadowMapTextureSize;
				if(Allocation.SizeX > NewTextureSizeX || Allocation.SizeY > NewTextureSizeY)
				{
					NewTextureSizeX = 1 << appCeilLogTwo(Allocation.SizeX);
					NewTextureSizeY = 1 << appCeilLogTwo(Allocation.SizeY);
				}

				Texture = new(PendingTextures) FLightMapPendingTexture(NewTextureSizeX,NewTextureSizeY,bNeedsBump, Allocation.LightMap->bSupportsCascadedShadow);
				
				verify(Texture->AddElement(&PaddingBaseX,&PaddingBaseY,Allocation.SizeX,Allocation.SizeY));
			}					

			//>@ ava
			Texture->bNeedsBumpedLightmap = Allocation.bNeedsBumpedLightmap;
			Texture->Outer = Allocation.Outer;
		}
		// Position the light-maps in the middle of their padded space.
		Allocation.BaseX = PaddingBaseX;
		Allocation.BaseY = PaddingBaseY;

		Texture->Allocations.AddItem(&Allocation);
	}

	// Encode all the pending textures.
	for(INT TextureIndex = 0;TextureIndex < PendingTextures.Num();TextureIndex++)
	{
		GWarn->StatusUpdatef(TextureIndex,PendingTextures.Num(),*LocalizeUnrealEd(TEXT("EncodingLightMapsF")),TextureIndex,PendingTextures.Num());
		PendingTextures(TextureIndex).Encode();
	}

	PendingTextures.Empty();
	PendingLightMaps.Empty();

	GWarn->EndSlowTask();
#endif
}

//<@ ava specific ; 2007. 12. 21 changmin
// add cascaded shadow
//FLightMap2D::FLightMap2D(const TArray<FGuid>& InLightGuids)
FLightMap2D::FLightMap2D(const TArray<FGuid>& InLightGuids, UBOOL bInSupportsCascadedShadow )
: FLightMap( bInSupportsCascadedShadow )
//>@ ava
{
	LightGuids = InLightGuids;

	appMemzero( Textures, sizeof(Textures) );	
}

const UTexture2D* FLightMap2D::GetTexture(UINT BasisIndex) const
{
	//<@ ava specific ; 2007. 1. 19 changmin
	check(BasisIndex < NUM_AVA_LIGHTMAPS);
	//>@ ava
	//check(BasisIndex < NUM_LIGHTMAP_COEFFICIENTS);	

	//<@ ava sepcific ; 2007. 11. 27 changmin
	// add cascaded shadow of sun light
	extern UBOOL GUseCascadedShadow;
	if( GUseCascadedShadow && bSupportsCascadedShadow )
	{		
		return CheckedTexture<FSunExcludedLightmapPolicy>(BasisIndex);
	}
	else
	{
		return CheckedTexture<FOldLightmapPolicy>(BasisIndex);
	}
	//>@ ava
}

struct FLegacyLightMapTextureInfo
{
	ULightMapTexture2D* Texture;
	FLinearColor Scale;
	FLinearColor Bias;

	friend FArchive& operator<<(FArchive& Ar,FLegacyLightMapTextureInfo& I)
	{
		return Ar << I.Texture << I.Scale << I.Bias;
	}
};

void FLightMap2D::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	for (INT TextureIndex=0; TextureIndex<ARRAY_COUNT(Textures); ++TextureIndex)
	{
		UObject::AddReferencedObject(ObjectArray,Textures[TextureIndex]);
	}
}

void FLightMap2D::Serialize(FArchive& Ar)
{
	const UBOOL bCanSkipUnnecessaryLightmaps			= (!GIsEditor && GIsGame);

	FLightMap::Serialize(Ar);

	//for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	//{
	//	Ar << Textures[CoefficientIndex];

	//	//@warning This used to use the FVector serialization (wrong). Now it would have used
	//	//         the FVector4 serialization (correct), but this would cause backwards compability
	//	//         problems.
	//	//         We are now assuming that FVector and the first 12 bytes of FVector4 are binary compatible.
	//	Ar << (FVector&)ScaleVectors[CoefficientIndex];
	//}
	//Ar << CoordinateScale << CoordinateBias;

	//<@ ava specific ; 2007. 11. 27 changmin
	// add cascaded shadow of sun light
	extern UBOOL GUseCascadedShadow;
	if( Ar.LicenseeVer() < VER_AVA_ADD_CASCADED_SHADOWMAP )
	{
		//<@ ava specific ; 2007. 1. 19 changmin
		if( Ar.LicenseeVer() < VER_AVA_NEEDSBUMPEDLIGHTMAP )
		{
			FVector Dummy;
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			{
				Ar << Texture<FOldLightmapPolicy>(CoefficientIndex+1);
				Ar << Dummy;
			}
			Ar << CoordinateScale << CoordinateBias;
			Texture<FOldLightmapPolicy>(0) = NULL;
		}
		else if( Ar.LicenseeVer() < VER_AVA_REMOVE_LIGHTSCALE )
		{
			FVector Dummy;
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
			{
				Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);
				Ar << Dummy;
			}
			Ar << CoordinateScale << CoordinateBias;
		}
		else
		{
			const UBOOL bNeedsBumpedLightmap = !IsSM2Platform(GRHIShaderPlatform);
			const UBOOL bSkipBump = (bCanSkipUnnecessaryLightmaps && !bNeedsBumpedLightmap);
			const UBOOL bSkipNormalLightmap = (bCanSkipUnnecessaryLightmaps && bNeedsBumpedLightmap );

			if( bSkipNormalLightmap )	// 첫번째 Lightmap pass / bump lightmap 3개 읽고, 첫번째 lightmap은 bump 1 번으로 setting
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{	
					if( CoefficientIndex == 0 )
					{
						Ar.SetSkipObjectRef( TRUE );
					}

					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);

					if( CoefficientIndex == 0 )
					{
						Ar.SetSkipObjectRef( FALSE );
					}	
				}
				Texture<FOldLightmapPolicy>(0) = Texture<FOldLightmapPolicy>(1);
			}
			else if( bSkipBump )	// normal lightmap 읽고, 나머지 bump lightmap은 normal lightmap으로 설정.
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);

					if (CoefficientIndex > 0)
					{
						Texture<FOldLightmapPolicy>(CoefficientIndex) = Texture<FOldLightmapPolicy>(0);
					}
					else
					{
						Ar.SetSkipObjectRef( TRUE );
					}
				}
				Ar.SetSkipObjectRef( FALSE );
			}
			else	// 모든 lightmap 읽기
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);
				}
			}

			Ar << CoordinateScale << CoordinateBias;
		}
		//>@ ava

		//// set to null for old data
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
		{
			Texture<FSunExcludedLightmapPolicy>(CoefficientIndex) = NULL;
		}
	}
	else
	{		
		const UBOOL bNeedsBumpedLightmap = !IsSM2Platform(GRHIShaderPlatform);

		// GUseCascadedShadow flag에 따라 선택적으로 필요한 lightmap 을 불러온다.
		const UBOOL bSkipBump			= (bCanSkipUnnecessaryLightmaps && !bNeedsBumpedLightmap);
		const UBOOL bSkipNormalLightmap = (bCanSkipUnnecessaryLightmaps && bNeedsBumpedLightmap);		
		const UBOOL bLoadOnlyLightmapWithSun	= bCanSkipUnnecessaryLightmaps && !(GUseCascadedShadow && bSupportsCascadedShadow);
		const UBOOL bLoadOnlyLightmapWithoutSun	= bCanSkipUnnecessaryLightmaps && (GUseCascadedShadow && bSupportsCascadedShadow);

		if( bSkipNormalLightmap )	// 첫번째 Lightmap pass / bump lightmap 3개 읽고, 첫번째 lightmap은 bump 1 번으로 setting
		{
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
			{	
				UBOOL bIsBump = (CoefficientIndex > 0);

				if( bSkipNormalLightmap && !bIsBump || bLoadOnlyLightmapWithoutSun )
				{
					Ar.SetSkipObjectRef( TRUE );
				}
				Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);
				if( bSkipNormalLightmap && !bIsBump || bLoadOnlyLightmapWithoutSun )
				{
					Ar.SetSkipObjectRef( FALSE );
				}	
			}
			Texture<FOldLightmapPolicy>(0) = Texture<FOldLightmapPolicy>(1);

			if( bSupportsCascadedShadow )
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{	
					UBOOL bIsBump = (CoefficientIndex > 0);
					if( bSkipNormalLightmap && !bIsBump || bLoadOnlyLightmapWithSun )
					{
						Ar.SetSkipObjectRef( TRUE );
					}
					Ar << Texture<FSunExcludedLightmapPolicy>(CoefficientIndex);
					if( bSkipNormalLightmap && !bIsBump || bLoadOnlyLightmapWithSun )
					{
						Ar.SetSkipObjectRef( FALSE );
					}	
				}
				Texture<FSunExcludedLightmapPolicy>(0) = Texture<FSunExcludedLightmapPolicy>(1);
			}

			// copy to another for IsValid() operation
			if( bLoadOnlyLightmapWithoutSun )
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{
					Texture<FOldLightmapPolicy>(CoefficientIndex) = Texture<FSunExcludedLightmapPolicy>(CoefficientIndex);
				}
			}
			
			if( bLoadOnlyLightmapWithSun )
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{
					Texture<FSunExcludedLightmapPolicy>(CoefficientIndex) = Texture<FOldLightmapPolicy>(CoefficientIndex);
				}
			}
		}
		else if( bSkipBump )	// normal lightmap 읽고, 나머지 bump lightmap은 normal lightmap으로 설정.
		{
			if( bLoadOnlyLightmapWithoutSun )
			{
				// serialize normal lightmap - skip all
				Ar.SetSkipObjectRef( TRUE );
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);
					if (CoefficientIndex > 0)
					{
						Texture<FOldLightmapPolicy>(CoefficientIndex) = Texture<FOldLightmapPolicy>(0);
					}
				}
				Ar.SetSkipObjectRef( FALSE );

				// serialize special light map -
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FSunExcludedLightmapPolicy>(CoefficientIndex);
					if (CoefficientIndex > 0)
					{
						Texture<FSunExcludedLightmapPolicy>(CoefficientIndex) = Texture<FSunExcludedLightmapPolicy>(0);
					}
					else
					{
						Ar.SetSkipObjectRef( TRUE );
					}
				}
				Ar.SetSkipObjectRef( FALSE );

				// copy special to normal for IsValid() operation
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{
					Texture<FOldLightmapPolicy>(CoefficientIndex) = Texture<FSunExcludedLightmapPolicy>(CoefficientIndex);
				}
			}
			else
			{
				// serialize normal lightmap
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);

					if (CoefficientIndex > 0)
					{
						Texture<FOldLightmapPolicy>(CoefficientIndex) = Texture<FOldLightmapPolicy>(0);
					}
					else
					{
						Ar.SetSkipObjectRef( TRUE );
					}
				}
				Ar.SetSkipObjectRef( FALSE );

				if( bSupportsCascadedShadow )
				{
					// serialize special lightmap - skip all
					Ar.SetSkipObjectRef( TRUE );
					for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
					{			
						Ar << Texture<FSunExcludedLightmapPolicy>(CoefficientIndex);
						if (CoefficientIndex > 0)
						{
							Texture<FSunExcludedLightmapPolicy>(CoefficientIndex) = Texture<FSunExcludedLightmapPolicy>(0);
						}
					}
					Ar.SetSkipObjectRef( FALSE );
				}

				// copy normal to special for IsValid() operation
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{
					Texture<FSunExcludedLightmapPolicy>(CoefficientIndex) = Texture<FOldLightmapPolicy>(CoefficientIndex);
				}
			}
		}
		else	// 모든 lightmap 읽기
		{
			if( bSupportsCascadedShadow )
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);
				}

				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FSunExcludedLightmapPolicy>(CoefficientIndex);
				}
			}
			else
			{
				for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_AVA_LIGHTMAPS;CoefficientIndex++)
				{			
					Ar << Texture<FOldLightmapPolicy>(CoefficientIndex);
				}
			}
		}

		Ar << CoordinateScale << CoordinateBias;
	}
	//>@ ava
}

FLightMap1D::FLightMap1D(UObject* InOwner,FLightMapData1D& Data):
	Owner(InOwner),
	CachedSampleData(NULL)
{
	// Gather the GUIDs of lights stored in the light-map.	
	for(INT LightIndex = 0;LightIndex < Data.Lights.Num();LightIndex++)
	{
		LightGuids.AddItem(Data.Lights(LightIndex)->LightmapGuid);
	}

	// Calculate the maximum light coefficient for each color component.
	FLOAT MaxCoefficient[NUM_LIGHTMAP_COEFFICIENTS][3];
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	{
		for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			MaxCoefficient[CoefficientIndex][ColorIndex] = 0;
		}
	}

	for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
		{
			for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				MaxCoefficient[CoefficientIndex][ColorIndex] = Clamp(
					Data(SampleIndex).Coefficients[CoefficientIndex][ColorIndex],
					MaxCoefficient[CoefficientIndex][ColorIndex],
					MAX_LIGHT_INTENSITY
					);
			}
		}
	}	

	// Calculate the scale and inverse scale for the quantized coefficients.
	FLOAT InvScale[NUM_LIGHTMAP_COEFFICIENTS][3];
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	{
		for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			ScaleVectors[CoefficientIndex].Component(ColorIndex) = MaxCoefficient[CoefficientIndex][ColorIndex];
			InvScale[CoefficientIndex][ColorIndex] = 1.0f / Max(MaxCoefficient[CoefficientIndex][ColorIndex],DELTA);
		}
	}

	// Rescale and quantize the light samples.
	{
		Samples.Lock( LOCK_READ_WRITE );
		FQuantizedLightSample* SampleData = (FQuantizedLightSample*) Samples.Realloc( Data.GetSize() );
		for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
		{
			FQuantizedLightSample& DestSample = SampleData[SampleIndex];
			const FLightSample& SourceSample = Data(SampleIndex);
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			{
				DestSample.Coefficients[CoefficientIndex] = FColor(
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][0] * InvScale[CoefficientIndex][0],1.0f / 2.2f) * 255.0f),0,255),
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][1] * InvScale[CoefficientIndex][1],1.0f / 2.2f) * 255.0f),0,255),
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][2] * InvScale[CoefficientIndex][2],1.0f / 2.2f) * 255.0f),0,255),
					0);
			}
		}
		Samples.Unlock();
	}
	check( CachedSampleData == NULL );
	Samples.GetCopy( &CachedSampleData, TRUE );
	BeginInitResource(this);
}

//<@ ava specific ; 2007. 11. 28 changmin; add per-pixel sunlight shadow
FLightMap1D::FLightMap1D(UObject* InOwner,FLightMapData1D& Data, FLightMapData1D& Data2, UBOOL bInSupportsCascadedShadow ):
	FLightMap(bInSupportsCascadedShadow),
	Owner(InOwner),
	CachedSampleData(NULL)
{
	//<@ ava specific ; 2007. 11. 28 changmin
	check( Data.GetSize() == Data2.GetSize() );
	//>@ ava

	// Gather the GUIDs of lights stored in the light-map.	
	for(INT LightIndex = 0;LightIndex < Data.Lights.Num();LightIndex++)
	{
		LightGuids.AddItem(Data.Lights(LightIndex)->LightmapGuid);
	}

	// Calculate the maximum light coefficient for each color component.
	FLOAT MaxCoefficient[NUM_LIGHTMAP_COEFFICIENTS][3];
	FLOAT MaxCoefficient2[NUM_LIGHTMAP_COEFFICIENTS][3];
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	{
		for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			MaxCoefficient[CoefficientIndex][ColorIndex] = 0;
			MaxCoefficient2[CoefficientIndex][ColorIndex] = 0;	//<@ ava specific ; 2007. 11. 28
		}
	}

	for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
		{
			for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				MaxCoefficient[CoefficientIndex][ColorIndex] = Clamp(
					Data(SampleIndex).Coefficients[CoefficientIndex][ColorIndex],
					MaxCoefficient[CoefficientIndex][ColorIndex],
					MAX_LIGHT_INTENSITY
					);

				//<@ ava specific ; 2007. 11. 28 changmin
				MaxCoefficient2[CoefficientIndex][ColorIndex] = Clamp(
					Data2(SampleIndex).Coefficients[CoefficientIndex][ColorIndex],
					MaxCoefficient2[CoefficientIndex][ColorIndex],
					MAX_LIGHT_INTENSITY
					);
				//>@ ava
			}
		}
	}	

	// Calculate the scale and inverse scale for the quantized coefficients.
	FLOAT InvScale[NUM_LIGHTMAP_COEFFICIENTS][3];
	FLOAT InvScale2[NUM_LIGHTMAP_COEFFICIENTS][3];
	for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
	{
		for(UINT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
		{
			ScaleVectors[CoefficientIndex].Component(ColorIndex) = MaxCoefficient[CoefficientIndex][ColorIndex];

			//<@ ava specific ; 2007. 11. 28 changmin
			ScaleVectorsWithoutSun[CoefficientIndex].Component(ColorIndex) = MaxCoefficient2[CoefficientIndex][ColorIndex];
			//>@ ava

			InvScale[CoefficientIndex][ColorIndex] = 1.0f / Max(MaxCoefficient[CoefficientIndex][ColorIndex],DELTA);

			//<@ ava
			InvScale2[CoefficientIndex][ColorIndex] = 1.0f / Max(MaxCoefficient2[CoefficientIndex][ColorIndex],DELTA);
		}
	}

    // Rescale and quantize the light samples.
	{
		Samples.Lock( LOCK_READ_WRITE );
		FQuantizedLightSample* SampleData = (FQuantizedLightSample*) Samples.Realloc( Data.GetSize() );
		for(INT SampleIndex = 0;SampleIndex < Data.GetSize();SampleIndex++)
		{
			FQuantizedLightSample& DestSample = SampleData[SampleIndex];
			const FLightSample& SourceSample = Data(SampleIndex);
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			{
				DestSample.Coefficients[CoefficientIndex] = FColor(
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][0] * InvScale[CoefficientIndex][0],1.0f / 2.2f) * 255.0f),0,255),
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][1] * InvScale[CoefficientIndex][1],1.0f / 2.2f) * 255.0f),0,255),
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][2] * InvScale[CoefficientIndex][2],1.0f / 2.2f) * 255.0f),0,255),
					0);
			}
		}
		Samples.Unlock();
	}

	//<@ ava specific ; 2007. 11. 28 changmin
	// add per pixel shadow of sun light
	// Rescale and quantize the light samples.
	if( bSupportsCascadedShadow )
	{
		SamplesWithoutSun.Lock( LOCK_READ_WRITE );
		FQuantizedLightSample* SampleData = (FQuantizedLightSample*) SamplesWithoutSun.Realloc( Data2.GetSize() );
		for(INT SampleIndex = 0;SampleIndex < Data2.GetSize();SampleIndex++)
		{
			FQuantizedLightSample& DestSample = SampleData[SampleIndex];
			const FLightSample& SourceSample = Data2(SampleIndex);
			for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
			{
				DestSample.Coefficients[CoefficientIndex] = FColor(
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][0] * InvScale2[CoefficientIndex][0],1.0f / 2.2f) * 255.0f),0,255),
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][1] * InvScale2[CoefficientIndex][1],1.0f / 2.2f) * 255.0f),0,255),
					(BYTE)Clamp<INT>(appTrunc(appPow(SourceSample.Coefficients[CoefficientIndex][2] * InvScale2[CoefficientIndex][2],1.0f / 2.2f) * 255.0f),0,255),
					0);
			}
		}
		SamplesWithoutSun.Unlock();
	}
	//>@ ava

	check( CachedSampleData == NULL );

	//<@ ava specific ; 2007. 11. 28 changmin
	extern UBOOL GUseCascadedShadow;
	if( GUseCascadedShadow && bSupportsCascadedShadow )
		SamplesWithoutSun.GetCopy( &CachedSampleData, TRUE );
	else
	//> ava
		Samples.GetCopy( &CachedSampleData, TRUE );


	BeginInitResource(this);
}
//>@ ava specific

FLightMap1D::~FLightMap1D()
{
	DEC_DWORD_STAT_BY( STAT_VertexLightingMemory, Samples.GetBulkDataSize() );

	if(CachedSampleData)
	{
		appFree(CachedSampleData);
		CachedSampleData = NULL;
	}
}

void FLightMap1D::Serialize(FArchive& Ar)
{
	FLightMap::Serialize(Ar);
	Ar << Owner;

	//<@ ava specific; 2007. 11. 28 changmin
	// add per-pixel shadow of sun light
	if( Ar.LicenseeVer() < VER_AVA_ADD_CASCADED_SHADOWMAP )
	{
		Samples.Serialize( Ar, Owner );

		//Samples.Lock( LOCK_READ_ONLY );
		//SamplesWithoutSun = Samples;
		//Samples.Unlock();

		for (INT ElementIndex = 0; ElementIndex < NUM_LIGHTMAP_COEFFICIENTS; ElementIndex++)
		{
			Ar << ScaleVectors[ElementIndex].X;
			Ar << ScaleVectors[ElementIndex].Y;
			Ar << ScaleVectors[ElementIndex].Z;

			//ScaleVectorsWithoutSun[ElementIndex] = ScaleVectors[ElementIndex];
		}
	}
	else
	//>@ ava
	{
		// lock 할 때, 올라오니, harddisk reading은 없다..
		Samples.Serialize( Ar, Owner );

		if( bSupportsCascadedShadow )
		{
			SamplesWithoutSun.Serialize( Ar, Owner );
		}

		for (INT ElementIndex = 0; ElementIndex < NUM_LIGHTMAP_COEFFICIENTS; ElementIndex++)
		{
			Ar << ScaleVectors[ElementIndex].X;
			Ar << ScaleVectors[ElementIndex].Y;
			Ar << ScaleVectors[ElementIndex].Z;
		}

		if( bSupportsCascadedShadow )
		{
			for (INT ElementIndex = 0; ElementIndex < NUM_LIGHTMAP_COEFFICIENTS; ElementIndex++)
			{
				Ar << ScaleVectorsWithoutSun[ElementIndex].X;
				Ar << ScaleVectorsWithoutSun[ElementIndex].Y;
				Ar << ScaleVectorsWithoutSun[ElementIndex].Z;
			}
		}
	}

}

void FLightMap1D::InitResources()
{
	if(!CachedSampleData)
	{
		extern UBOOL GUseCascadedShadow;
		//<@ ava specific ; 2007. 11. 28 changmin
		// add per-pixel shadow of sun light
		if(GUseCascadedShadow && bSupportsCascadedShadow)
		{
			SamplesWithoutSun.GetCopy( &CachedSampleData, TRUE );
		}
		else
		//>@ ava
		{
			Samples.GetCopy( &CachedSampleData, TRUE );
		}

		INC_DWORD_STAT_BY( STAT_VertexLightingMemory, Samples.GetBulkDataSize() );

		BeginInitResource(this);
	}
}

void FLightMap1D::InitRHI()
{
	// Compute the vertex buffer size.
	SIZE_T Size = Samples.GetBulkDataSize();

	// Create the light-map vertex buffer.
	VertexBufferRHI = RHICreateVertexBuffer(Size,NULL,FALSE);

	// Write the light-map data to the vertex buffer.
	void* Buffer = RHILockVertexBuffer(VertexBufferRHI,0,Size);
	appMemcpy(Buffer,CachedSampleData,Size);

	//<@ ava specific ; 2006. 10. 17 changmin
	// static mesh decal render data 생성을 위해, CachedSampleData가 필요합니다. 그래서, 이 data를 없애지 않습니다.
	//appFree(CachedSampleData);
	//CachedSampleData = NULL;
	//>@ ava
	RHIUnlockVertexBuffer(VertexBufferRHI);
}

/**
 * Creates a new lightmap that is a copy of this light map, but where the sample data
 * has been remapped according to the specified sample index remapping.
 *
 * @param		SampleRemapping		Sample remapping: Dst[i] = Src[SampleRemapping[i]].
 * @return							The new lightmap.
 */
FDynamicLightMap1D* FLightMap1D::DuplicateWithRemappedVerts(const TArray<INT>& SampleRemapping)
{
	check( SampleRemapping.Num() > 0 );

	// Create a new light map.
	FDynamicLightMap1D* NewLightMap = new FDynamicLightMap1D;

	// Copy over the owner.
	NewLightMap->Owner = Owner;
	NewLightMap->LightGuids = LightGuids;

	//<@ ava specific ; 2008. 1. 4 changmin
	// add cascaded shadow
	NewLightMap->bSupportsCascadedShadow = bSupportsCascadedShadow;
	//>@ ava

	// Copy over coefficient scale vectors.
	for ( INT ElementIndex = 0 ; ElementIndex < NUM_LIGHTMAP_COEFFICIENTS ; ++ElementIndex )
	{
		NewLightMap->ScaleVectors[ElementIndex] = ScaleVectors[ElementIndex];
		if( bSupportsCascadedShadow )
		{
			NewLightMap->ScaleVectorsWithoutSun[ElementIndex] = ScaleVectorsWithoutSun[ElementIndex];	//<@ ava speicifc
		}
	}

	//<@ ava specific ; 2007. 11. 28 changmin
	// add per-pixel shadow of sun light
	const UBOOL bAVA = TRUE;
	if (bAVA)
	{
		// Copy over samples given the index remapping.
		FQuantizedLightSample* SrcSampleData = NULL;
		extern UBOOL GUseCascadedShadow;
		if (GUseCascadedShadow && bSupportsCascadedShadow )
		{
			SrcSampleData = (FQuantizedLightSample*)SamplesWithoutSun.Lock( LOCK_READ_ONLY );
		}
		else
		{
			SrcSampleData = (FQuantizedLightSample*)Samples.Lock( LOCK_READ_ONLY );
		}

		NewLightMap->Samples.Lock( LOCK_READ_WRITE );
		FQuantizedLightSample* DstSampleData = (FQuantizedLightSample*)NewLightMap->Samples.Realloc( SampleRemapping.Num() );

		for( INT SampleIndex = 0 ; SampleIndex < SampleRemapping.Num() ; ++SampleIndex )
		{
			const INT RemappedIndex = SampleRemapping(SampleIndex);
			checkSlow( RemappedIndex >= 0 && RemappedIndex < NumSamples() );
			const FQuantizedLightSample& SrcSample = SrcSampleData[RemappedIndex];
			FQuantizedLightSample& DstSample = DstSampleData[SampleIndex];
			for( UINT CoefficientIndex = 0 ; CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS ; ++CoefficientIndex )
			{
				DstSample.Coefficients[CoefficientIndex] = SrcSample.Coefficients[CoefficientIndex];
			}
		}
		if( GUseCascadedShadow && bSupportsCascadedShadow )
		{
			SamplesWithoutSun.Unlock();
		}
		else
		{
			Samples.Unlock();
		}
		NewLightMap->Samples.Unlock();

		check( NewLightMap->CachedSampleData == NULL );
		NewLightMap->Samples.GetCopy( &NewLightMap->CachedSampleData, TRUE );	
	}
	else
	//>@ ava
	{
		// Copy over samples given the index remapping.
		FQuantizedLightSample* SrcSampleData = (FQuantizedLightSample*)Samples.Lock( LOCK_READ_ONLY );
		NewLightMap->Samples.Lock( LOCK_READ_WRITE );
		FQuantizedLightSample* DstSampleData = (FQuantizedLightSample*)NewLightMap->Samples.Realloc( SampleRemapping.Num() );

		for( INT SampleIndex = 0 ; SampleIndex < SampleRemapping.Num() ; ++SampleIndex )
		{
			const INT RemappedIndex = SampleRemapping(SampleIndex);
			checkSlow( RemappedIndex >= 0 && RemappedIndex < NumSamples() );
			const FQuantizedLightSample& SrcSample = SrcSampleData[RemappedIndex];
			FQuantizedLightSample& DstSample = DstSampleData[SampleIndex];
			for( UINT CoefficientIndex = 0 ; CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS ; ++CoefficientIndex )
			{
				DstSample.Coefficients[CoefficientIndex] = SrcSample.Coefficients[CoefficientIndex];
			}
		}
		Samples.Unlock();
		NewLightMap->Samples.Unlock();

		check( NewLightMap->CachedSampleData == NULL );
		NewLightMap->Samples.GetCopy( &NewLightMap->CachedSampleData, TRUE );	
	}
	



	return NewLightMap;
}

FDynamicLightMap1D* FLightMap1D::DuplicateWithNewSamples(const TArray<FQuantizedLightSample>& SourceSamples)
{
	check( SourceSamples.Num() > 0 );

	// Create a new light map.
	FDynamicLightMap1D* NewLightMap = new FDynamicLightMap1D;

	// Copy over the owner.
	NewLightMap->Owner = Owner;
	NewLightMap->LightGuids = LightGuids;

	//<@ ava specific ; 2008. 1. 4 changmin
	// add cascaded shadow
	NewLightMap->bSupportsCascadedShadow = bSupportsCascadedShadow;
	//>@ ava

	// Copy over coefficient scale vectors.
	for ( INT ElementIndex = 0 ; ElementIndex < NUM_LIGHTMAP_COEFFICIENTS ; ++ElementIndex )
	{
		NewLightMap->ScaleVectors[ElementIndex] = ScaleVectors[ElementIndex];
		if( bSupportsCascadedShadow )
		{
			NewLightMap->ScaleVectorsWithoutSun[ElementIndex] = ScaleVectorsWithoutSun[ElementIndex];	// ava specific
		}
	}
	
	// copy samples
	NewLightMap->Samples.Lock( LOCK_READ_WRITE );

	FQuantizedLightSample* SampleData = (FQuantizedLightSample*)NewLightMap->Samples.Realloc( SourceSamples.Num() );
	for(INT SampleIndex = 0;SampleIndex < SourceSamples.Num();SampleIndex++)
	{
		FQuantizedLightSample& DestSample = SampleData[SampleIndex];
		DestSample = SourceSamples(SampleIndex);
	}

	NewLightMap->Samples.Unlock();
	
	NewLightMap->Samples.GetCopy( &NewLightMap->CachedSampleData, TRUE );	

	return NewLightMap;
}



//<@ ava specific ; 2006. 10. 16 changmin
// 이건 사용 안하네요....2007. 11. 28 changmin
void FLightMap1D::SetSamples( const TArray<FQuantizedLightSample>& SourceSamples )
{
	// copy samples
	Samples.Lock( LOCK_READ_WRITE );

	FQuantizedLightSample* SampleData = (FQuantizedLightSample*) Samples.Realloc( SourceSamples.Num() );
	for(INT SampleIndex = 0;SampleIndex < SourceSamples.Num();SampleIndex++)
	{
		FQuantizedLightSample& DestSample = SampleData[SampleIndex];
		DestSample = SourceSamples(SampleIndex);
	}

	Samples.Unlock();

	check( CachedSampleData == NULL );
	Samples.GetCopy( &CachedSampleData, TRUE );
}
//>@ ava

//<@ ava specific ; 2006. 10. 17 changmin
// 이것도 안쓰이네요... 2007. 11. 28 changmin
void FLightMap1D::CopyScaleVectors( const FLightMap1D* SourceLightmap )
{
	for( INT LightCoeff = 0; LightCoeff < NUM_LIGHTMAP_COEFFICIENTS; ++LightCoeff )
	{
		ScaleVectors[LightCoeff] = SourceLightmap->ScaleVectors[LightCoeff];
		if( bSupportsCascadedShadow )
			ScaleVectorsWithoutSun[LightCoeff] = SourceLightmap->ScaleVectorsWithoutSun[LightCoeff];
	}
}
//>@ ava

//!{ 2006-04-10	허 창 민
void FLightSample::AddLight(const FLinearColor& Color, INT BumpIndex )
{
	const float MAX_HDR_INTENSITY = 1.0e10;

	FLOAT R = Color.R < MAX_HDR_INTENSITY ? Color.R : MAX_HDR_INTENSITY;
	FLOAT G = Color.G < MAX_HDR_INTENSITY ? Color.G : MAX_HDR_INTENSITY;
	FLOAT B = Color.B < MAX_HDR_INTENSITY ? Color.B : MAX_HDR_INTENSITY;

	// check underflow ( wrong simulated data )
	Coefficients[BumpIndex][0] = Max( Coefficients[BumpIndex][0], R); 
	Coefficients[BumpIndex][1] = Max( Coefficients[BumpIndex][1], G);
	Coefficients[BumpIndex][2] = Max( Coefficients[BumpIndex][2], B);
}
//!} 2006-04-10	허 창 민

//!{ 2006-05-09	허 창 민
void FLightSample2::AddLight(const FLinearColor& Color)
{
	Coefficients[0] += Color.R;
	Coefficients[1] += Color.G;
	Coefficients[2] += Color.G;
}
//!} 2006-05-09	허 창 민


/*-----------------------------------------------------------------------------
	FQuantizedLightSample version of bulk data.
-----------------------------------------------------------------------------*/

/**
 * Returns whether single element serialization is required given an archive. This e.g.
 * can be the case if the serialization for an element changes and the single element
 * serialization code handles backward compatibility.
 */
UBOOL FQuantizedLightSampleBulkData::RequiresSingleElementSerialization( FArchive& Ar )
{
#if PS3
	return TRUE;
#else
	return FALSE;
#endif
}

/**
 * Returns size in bytes of single element.
 *
 * @return Size in bytes of single element
 */
INT FQuantizedLightSampleBulkData::GetElementSize() const
{
	return sizeof(FQuantizedLightSample);
}

/**
 * Serializes an element at a time allowing and dealing with endian conversion and backward compatiblity.
 * 
 * @param Ar			Archive to serialize with
 * @param Data			Base pointer to data
 * @param ElementIndex	Element index to serialize
 */
void FQuantizedLightSampleBulkData::SerializeElement( FArchive& Ar, void* Data, INT ElementIndex )
{
	FQuantizedLightSample* QuantizedLightSample = (FQuantizedLightSample*)Data + ElementIndex;
	if( Ar.Ver() < VER_QUANT_LIGHTSAMPLE_BYTE_TO_COLOR )
	{
		for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
		{
			// old lighting coefficient colors stored as byte arrays
			BYTE TempCoefficients[3];
			Ar << TempCoefficients[0];
			Ar << TempCoefficients[1];
			Ar << TempCoefficients[2];				
			// convert to colors
			QuantizedLightSample->Coefficients[CoefficientIndex] = FColor( TempCoefficients[0], TempCoefficients[1], TempCoefficients[2], 0 );
		}			
	}
	else
	{
		// serialize as colors
		for(INT CoefficientIndex = 0;CoefficientIndex < NUM_LIGHTMAP_COEFFICIENTS;CoefficientIndex++)
		{
			DWORD ColorDWORD = QuantizedLightSample->Coefficients[CoefficientIndex].DWColor();
			Ar << ColorDWORD;
#if PS3
			//@TODO: Byte-order for vertex colors are RGBA, not ARGB, on PS3. Do this in the cooker.
			FColor Color(ColorDWORD);
			QuantizedLightSample->Coefficients[CoefficientIndex] = FColor(Color.G, Color.B, Color.A, Color.R);
#else
			QuantizedLightSample->Coefficients[CoefficientIndex] = FColor(ColorDWORD);
#endif
		}            
	}
};

FArchive& operator<<(FArchive& Ar,FLightMap*& R)
{
	enum
	{
		LMT_None = 0,
		LMT_1D = 1,
		LMT_2D = 2,
	};
	DWORD LightMapType = LMT_None;
	if(Ar.IsSaving())
	{
		if(R != NULL)
		{
			if(R->GetLightMap1D())
			{
				LightMapType = LMT_1D;
			}
			else if(R->GetLightMap2D())
			{
				LightMapType = LMT_2D;
			}
		}
	}
	Ar << LightMapType;

	if(Ar.IsLoading())
	{
		check(!R);

		if(LightMapType == LMT_1D)
		{
			R = new FLightMap1D();
		}
		else if(LightMapType == LMT_2D)
		{
			R = new FLightMap2D();
		}
	}

	if(R != NULL)
	{
		R->Serialize(Ar);
	}

	// Discard light-maps older than the light-map gamma correction fix.
	if(LightMapType == LMT_1D && Ar.Ver() < VER_VERTEX_LIGHTMAP_GAMMACORRECTION_FIX)
	{
		delete R;
		R = NULL;
	}

	//<@ ava specific ; 2007. 1. 20 changmin
	// Discard light-maps older than the bump lightmaps
	if(LightMapType == LMT_2D && Ar.Ver() < VER_AVA_NEEDSBUMPEDLIGHTMAP )
	{
		delete R;
		R = NULL;
	}
	//>@ ava

	return Ar;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/