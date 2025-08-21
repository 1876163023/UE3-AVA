/*=============================================================================
	UnTexCompress.cpp: Unreal texture compression functions.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if _MSC_VER && !CONSOLE
#pragma pack (push,8)
#include "../../../nvDXT/Inc/dxtlib.h"
#pragma pack (pop)
#pragma comment(lib, "nvDXTlibMTDLL.vc8.lib")
#endif

//!{ 2006-04-17	허 창 민
#if !FINAL_RELEASE
#if _MSC_VER && !CONSOLE
#pragma pack (push,8)
#include "../../../External/squish-1.3/squish.h"
#pragma pack (pop)
#endif
#endif

//!} 2006-04-17	허 창 민

/*-----------------------------------------------------------------------------
	DXT functions.
-----------------------------------------------------------------------------*/

// Callbacks required by nvDXT library.
#if _MSC_VER && !CONSOLE

NV_ERROR_CODE CompressionCallback(const void *Data, size_t NumBytes, const MIPMapData * MipMapData, void * UserData )
{
	UTexture2D* Texture2D = Cast<UTexture2D>((UObject*)UserData);
	if( MipMapData && Texture2D )
	{
		FTexture2DMipMap* MipMap = new(Texture2D->Mips)FTexture2DMipMap;
		
		MipMap->SizeX = Max<UINT>(MipMapData->width,4);
		MipMap->SizeY = Max<UINT>(MipMapData->height,4);
		MipMap->Data.Lock(LOCK_READ_WRITE);
		appMemcpy( MipMap->Data.Realloc(NumBytes), Data, NumBytes );
		MipMap->Data.Unlock();
	}
	return NV_OK;
}

#endif

/**
 * Shared compression functionality. 
 */
void UTexture::Compress()
{
	// High dynamic range textures are currently always stored as RGBE (shared exponent) textures.
	RGBE = (CompressionSettings == TC_HighDynamicRange);

	//<@ ava specific; 2006. 9. 11 changmin
	// TEXGROUP_UI는 Mipmap을 생성하지 않도록 한다.
	if( LODGroup == TEXTUREGROUP_UI )
	{
		CompressionNoMipmaps = true;
	}
	//>@ ava
}

void GenerateMips2D(UTexture2D* Texture)
{
	UINT NumMips = Max(appCeilLogTwo(Texture->SizeX),appCeilLogTwo(Texture->SizeY));

	// Remove any existing non-toplevel mipmaps.
	if(Texture->Mips.Num() > 1)
	{
		// We need to flush all rendering commands before we can modify a textures' mip array.
		FlushRenderingCommands();
		// Remove non top-level mipmaps.
		Texture->Mips.Remove(1,Texture->Mips.Num() - 1);
	}

	// Only generate mip-maps for 32-bit RGBA images.
	if(Texture->Format == PF_A8R8G8B8)
	{
		// Allocate the new mipmaps.
		for(UINT MipIndex = 1;MipIndex <= NumMips;MipIndex++)
		{
			INT DestSizeX = Max((INT)GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
			INT DestSizeY = Max((INT)GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

			FTexture2DMipMap* MipMap = new(Texture->Mips) FTexture2DMipMap;

			MipMap->SizeX = DestSizeX;
			MipMap->SizeY = DestSizeY;

			SIZE_T ImageSize = CalculateImageBytes(DestSizeX,DestSizeY,0,(EPixelFormat)Texture->Format);
			
			MipMap->Data.Lock( LOCK_READ_WRITE );
			MipMap->Data.Realloc( ImageSize );
			MipMap->Data.Unlock();

			FTexture2DMipMap* SourceMip = &Texture->Mips(MipIndex - 1);
			FTexture2DMipMap* DestMip = &Texture->Mips(MipIndex);

			INT	SubSizeX = SourceMip->SizeX / DestMip->SizeX;
			INT SubSizeY = SourceMip->SizeY / DestMip->SizeY;
			BYTE SubShift = appCeilLogTwo(SubSizeX) + appCeilLogTwo(SubSizeY);
			FLOAT SubScale = 1.f / (SubSizeX + SubSizeY);

			check(Texture->Format == PF_A8R8G8B8);

			FColor*	DestPtr	= (FColor*) DestMip->Data.Lock( LOCK_READ_WRITE );
			FColor*	SrcPtr	= (FColor*) SourceMip->Data.Lock( LOCK_READ_ONLY );

			if( Texture->RGBE )
			{
				for(INT Y = 0;Y < DestMip->SizeY;Y++)
				{
					for(INT X = 0;X < DestMip->SizeX;X++)
					{
						FLinearColor HDRColor = FLinearColor(0,0,0,0);

						for(INT SubX = 0;SubX < SubSizeX;SubX++)
							for(INT SubY = 0;SubY < SubSizeY;SubY++)
								HDRColor += SrcPtr[SubY * SourceMip->SizeX + SubX].FromRGBE();

						*DestPtr = (HDRColor * SubScale).ToRGBE();

						DestPtr++;
						SrcPtr += 2;
					}

					SrcPtr += SourceMip->SizeX;
				}
			}
			else
			{
				for(INT Y = 0;Y < DestMip->SizeY;Y++)
				{
					for(INT X = 0;X < DestMip->SizeX;X++)
					{
						INT	R = 0,
							G = 0,
							B = 0,
							A = 0;

						for(INT SubX = 0;SubX < SubSizeX;SubX++)
						{
							for(INT SubY = 0;SubY < SubSizeY;SubY++)
							{
								R += SrcPtr[SubY * SourceMip->SizeX + SubX].R;
								G += SrcPtr[SubY * SourceMip->SizeX + SubX].G;
								B += SrcPtr[SubY * SourceMip->SizeX + SubX].B;
								A += SrcPtr[SubY * SourceMip->SizeX + SubX].A;
							}
						}

						DestPtr->R = R >> SubShift;
						DestPtr->G = G >> SubShift;
						DestPtr->B = B >> SubShift;
						DestPtr->A = A >> SubShift;

						DestPtr++;
						SrcPtr += 2;
					}

					SrcPtr += SourceMip->SizeX;
				}
			}

			DestMip->Data.Unlock();
			SourceMip->Data.Unlock();
		}
	}
	// Grayscale mip map generation
	else if ( Texture->Format == PF_G8 )
	{
		// Allocate the new mipmaps.
		for(UINT MipIndex = 1;MipIndex <= NumMips;MipIndex++)
		{
			INT DestSizeX =	Max((INT)GPixelFormats[Texture->Format].BlockSizeX,Texture->SizeX >> MipIndex);
			INT DestSizeY = Max((INT)GPixelFormats[Texture->Format].BlockSizeY,Texture->SizeY >> MipIndex);

			FTexture2DMipMap* MipMap = new(Texture->Mips) FTexture2DMipMap;

			MipMap->SizeX = DestSizeX;
			MipMap->SizeY = DestSizeY;

			SIZE_T ImageSize = CalculateImageBytes(DestSizeX,DestSizeY,0,(EPixelFormat)Texture->Format);

			MipMap->Data.Lock( LOCK_READ_WRITE );
			MipMap->Data.Realloc( ImageSize );
			MipMap->Data.Unlock();

			FTexture2DMipMap* SourceMip = &Texture->Mips(MipIndex - 1);
			FTexture2DMipMap* DestMip	= &Texture->Mips(MipIndex);

			INT	SubSizeX = SourceMip->SizeX / DestMip->SizeX;
			INT SubSizeY = SourceMip->SizeY / DestMip->SizeY;
			BYTE SubShift = appCeilLogTwo(SubSizeX) + appCeilLogTwo(SubSizeY);
			FLOAT SubScale = 1.f / (SubSizeX + SubSizeY);

			// single channel grayscale
			BYTE*	DestPtr	= (BYTE*) DestMip->Data.Lock( LOCK_READ_WRITE );
			BYTE*	SrcPtr	= (BYTE*) SourceMip->Data.Lock( LOCK_READ_ONLY );

			// generate this mip level
			for(INT Y = 0;Y < DestMip->SizeY;Y++)
			{
				for(INT X = 0;X < DestMip->SizeX;X++)
				{
					INT	R = 0;
					for(INT SubX = 0;SubX <	SubSizeX;SubX++)
					{
						for(INT SubY = 0;SubY <	SubSizeY;SubY++)
						{
							R += SrcPtr[SubY * SourceMip->SizeX + SubX];
						}
					}

					*DestPtr = R >> SubShift;
					DestPtr++;
					SrcPtr += 2;
				}
				SrcPtr += SourceMip->SizeX;
			}
			DestMip->Data.Unlock();
			SourceMip->Data.Unlock();
		}
	}
}

void UTexture2D::Compress()
{
#if _MSC_VER && !CONSOLE
	Super::Compress();

	switch( Format )
	{
	case PF_A8R8G8B8:
	case PF_G8:
	case PF_DXT1:
	case PF_DXT3:
	case PF_DXT5:
		// Handled formats, break.
		break;

	case PF_Unknown:
	case PF_A32B32G32R32F:
	case PF_G16:
	default:
		// Unhandled, return.
		return;
	}

	// Return if no source art is present (maybe old package).
	if( SourceArt.GetBulkDataSize() == 0 )
	{
		return;
	}

	// Decompress source art.
	FPNGHelper PNG;
	PNG.InitCompressed( SourceArt.Lock(LOCK_READ_WRITE), SourceArt.GetBulkDataSize(), SizeX, SizeY );
	TArray<BYTE> RawData = PNG.GetRawData();
	SourceArt.Unlock();

	// Don't compress textures smaller than DXT blocksize.
	if( SizeX < 4 || SizeY < 4 )
	{
		CompressionNone = 1;
	}

	// Displacement maps get stored as PF_G8
	if( CompressionSettings == TC_Displacementmap )
	{
		Init(SizeX,SizeY,PF_G8);
		
		FTexture2DMipMap& TopLevelMip = Mips(0);

		FColor* RawColor	= (FColor*) &RawData(0);
		BYTE*	DestColor	= (BYTE*) TopLevelMip.Data.Lock(LOCK_READ_WRITE);

		for( INT i=0; i<SizeX * SizeY; i++ )
		{
			*(DestColor++)	= (RawColor++)->A;
		}

		TopLevelMip.Data.Unlock();
	}
	// Grayscale textures are stored uncompressed.
	else
	if( CompressionSettings == TC_Grayscale )
	{
		Init(SizeX,SizeY,PF_G8);
		
		FTexture2DMipMap& TopLevelMip = Mips(0);

		FColor* RawColor	= (FColor*) &RawData(0);
		BYTE*	DestColor	= (BYTE*) TopLevelMip.Data.Lock(LOCK_READ_WRITE);

		for( INT i=0; i<SizeX * SizeY; i++ )
		{
			*(DestColor++) = (RawColor++)->R;
		}

		TopLevelMip.Data.Unlock();

		// check for mip generation
		if( !CompressionNoMipmaps )
		{
			GenerateMips2D(this);
		}
	}
	// Certain textures (icons in Editor) need to be accessed by code so we can't compress them.
	else
	if( CompressionNone || (CompressionSettings == TC_HighDynamicRange && CompressionFullDynamicRange ) )
	{
		Init(SizeX,SizeY,PF_A8R8G8B8);
		
		FTexture2DMipMap& TopLevelMip = Mips(0);

		check( TopLevelMip.Data.GetBulkDataSize() == RawData.Num() );
		appMemcpy( TopLevelMip.Data.Lock(LOCK_READ_WRITE), RawData.GetData(), RawData.Num() );
		TopLevelMip.Data.Unlock();

		if( !CompressionNoMipmaps )
		{
			GenerateMips2D(this);
		}
	}
	// Regular textures.
	else
	{
		UBOOL	Opaque			= 1,
				FreeSourceData	= 0;
		FColor*	SourceData		= (FColor*) &RawData(0);

		// Artists sometimes have alpha channel in source art though don't want to use it.
		if( ! (CompressionNoAlpha || CompressionSettings == TC_Normalmap) )
		{
			// Figure out whether texture is opaque or not.
			FColor*	Color = SourceData;
			for( INT y=0; y<SizeY; y++ )
			{
				for( INT x=0; x<SizeX; x++ )
				{
					if( (Color++)->A != 255 )
					{
						Opaque = 0;
						break;
					}
				}
			}
		}

		// We need to fiddle with the exponent for RGBE textures.
		if( CompressionSettings == TC_HighDynamicRange && RGBE )
		{
			FreeSourceData	= 1;
			SourceData		= new FColor[SizeY*SizeX];
			appMemcpy( SourceData, &RawData(0), SizeY * SizeX * sizeof(FColor) );

			// Clamp exponent to -8, 7 range, translate into 0..15 and shift into most significant bits so compressor doesn't throw the data away.
			FColor*	Color = SourceData;
			for( INT y=0; y<SizeY; y++ )
			{
				for( INT x=0; x<SizeX; x++ )
				{
					Color->A = (Clamp(Color->A - 128, -8, 7) + 8) * 16;
					Color++;
				}
			}
		}

		// DXT1 if opaque (or override) and DXT5 otherwise. DXT3 is only suited for masked textures though DXT5 works fine for this purpose as well.
		EPixelFormat PixelFormat = Opaque ? PF_DXT1 : PF_DXT5;
		
		// DXT3's explicit 4 bit alpha works well with RGBE textures as we can limit the exponent to 4 bit.
		if( RGBE )
		{
			PixelFormat = PF_DXT3;
		}

		nvTextureFormats	TextureFormat	= PixelFormat == PF_DXT1 ? kDXT1 : PixelFormat == PF_DXT3 ? kDXT3 : kDXT5;
		UBOOL				bIsNormalMap	= (CompressionSettings == TC_Normalmap) || (CompressionSettings == TC_NormalmapAlpha);

		// We need to flush all rendering commands before we can modify a textures' mip array.
		if( Mips.Num() )
		{
			// Flush rendering commands.
			FlushRenderingCommands();
			// Start with a clean plate.
			Mips.Empty();
		}

		// Constructor fills in default data for CompressionOptions.
		nvCompressionOptions nvOptions; 
		nvOptions.mipMapGeneration		= CompressionNoMipmaps || RGBE ? kNoMipMaps : kGenerateMipMaps; //@todo compression: compressor needs to be aware of RGBE format.
		nvOptions.mipFilterType			= kMipFilterBox;
		nvOptions.textureType			= kTextureTypeTexture2D;
		nvOptions.textureFormat			= TextureFormat;
		nvOptions.bEnableFilterGamma	= SRGB;
		nvOptions.filterGamma			= 2.2f;
//		nvOptions.bRGBE					= RGBE; // Clamps exponent to -8, 7 range, translates into 0..15 range and shifts into most significant bits.
		nvOptions.weightType			= bIsNormalMap ? kTangentSpaceNormalMapWeighting : kLuminanceWeighting;
		nvOptions.user_data				= this;
		Format							= PixelFormat;

		// Compress...
		nvDDS::nvDXTcompress(
			(BYTE*)SourceData,		// src
			SizeX,					// width
			SizeY,					// height
			SizeX * 4,				// pitch
			nvBGRA,					// pixel order
			&nvOptions,				// compression options
			CompressionCallback		// callback
			);

		if( FreeSourceData )
		{
			delete [] SourceData;
		}
	}

	// We modified the texture data and potentially even the format so we can't stream it from disk.
	bHasBeenLoadedFromPersistentArchive = FALSE;

	// Create the texture's resource.
	UpdateResource();
#endif
}

//!{ 2006-04-17	허 창 민
FColor UTexture2D::SampleAverageColor()
{
#if FINAL_RELEASE
	appDebugBreak();	
#else
	// 제일 하위 Mip에서 Dxt값 가져오면 값이 정확치가 않다.
	// 4 x 4 pixel 을 가지고 있는 block을 가져오기
	if( Mips.Num() > 0 && (Format == PF_DXT1 || Format == PF_DXT3 || Format == PF_DXT5 ) )
	{
		UINT SizeX = 1; 
		UINT SizeY = 1;
		UINT Mip = Mips.Num() - 1;
		FTexture2DMipMap* LowestMip = NULL;

		while( Mip > 0 )
		{
			LowestMip = &(Mips( Mip ));

			SizeX = LowestMip->SizeX;
			SizeY = LowestMip->SizeY;

			if( SizeX > 4 && SizeY > 4 )
			{
				if( Mip == Mips.Num() -1 )
				{
					Mip = 0;
				}
				else
				{
					Mip++;
					LowestMip = &(Mips( Mip ));
					SizeX = LowestMip->SizeX;
					SizeY = LowestMip->SizeY;
				}

				break;
			}

			Mip--;
		} 


		// 최소 block size 일때만 처리 해보면
		if( SizeX == 4 || SizeY == 4 )
		{
			UINT	USize = Align( LowestMip->SizeX, GPixelFormats[Format].BlockSizeX ),
				VSize = Align( LowestMip->SizeY, GPixelFormats[Format].BlockSizeY ),
				NumColumns	= (USize / GPixelFormats[Format].BlockSizeX),
				NumRows		= (VSize / GPixelFormats[Format].BlockSizeY);

			BYTE pixels[ 4 * 16 ];	// { r1, g1, b1, a1, ....., r16, g16, b16, a16 }

			UINT BlockFormat = 0;
			switch( Format )
			{
			case PF_DXT1:
				BlockFormat = squish::kDxt1;
				break;
			case PF_DXT3:
				BlockFormat = squish::kDxt3;
				break;
			case PF_DXT5:
				BlockFormat = squish::kDxt5;
				break;
			}

			if( BlockFormat )
			{				
				squish::Decompress( pixels, Mips(Mip).Data.Lock( LOCK_READ_ONLY ), BlockFormat );
				Mips(Mip).Data.Unlock();

				FLOAT R = 0.0f;
				FLOAT G = 0.0f;
				FLOAT B = 0.0f;
				FLOAT A = 0.0f;

				for( INT y = 0; y < 4; ++y )
				{
					for( INT x = 0; x < 4; ++x )
					{
						BYTE* Src = &pixels[ ( y * 4 + x ) * 4 ];
						R += Src[0] / 255.0f;
						G += Src[1] / 255.0f;
						B += Src[2] / 255.0f;
						A += Src[3] / 255.0f;
					}
				}

				R /= 16.0f;
				G /= 16.0f;
				B /= 16.0f;
				A /= 16.0f;

				return FLinearColor( R, G, B, A );
			}		
		}
	}

#endif
	return FColor( 0, 0, 0, 255 );
}
//!} 2006-04-17	허 창 민

//!{ 2006-04-11	허 창 민
FColor UTexture2D::SampleTexture( const FVector2D& Start, const FVector2D& End )
{
	check( Start.X >= 0.0f && Start.X <= 1.0f );
	check( Start.Y >= 0.0f && Start.Y <= 1.0f );
	check( End.X >= 0.0f && End.X <= 1.0f );
	check( End.Y >= 0.0f && End.Y <= 1.0f );
	check( Start.X <= End.X && Start.Y <= End.Y );

	// uv to pixel 너무 어렵당~
	INT StartX	= Min( appCeil( Start.X * (FLOAT)SizeX - 0.5f ),	(INT)(SizeX - 1) );
	INT EndX	= Min( appCeil( End.X * (FLOAT)SizeX - 0.5f ),		(INT)(SizeX - 1) );
	INT StartY	= Min( appCeil( Start.Y * (FLOAT)SizeY - 0.5f ),	(INT)(SizeY - 1) );
	INT EndY	= Min( appCeil( End.Y * (FLOAT)SizeY - 0.5f ),		(INT)(SizeY - 1) );

	FColor SampleColor;

	// Return if no source art is present (maybe old package).
	if( SourceArt.GetBulkDataSize() == 0 )
	{
		return SampleColor;
	}

	// Decompress source art.
	FPNGHelper PNG;
	PNG.InitCompressed( SourceArt.Lock(LOCK_READ_WRITE), SourceArt.GetBulkDataSize(), SizeX, SizeY );
	TArray<BYTE> RawData = PNG.GetRawData();
	SourceArt.Unlock();

	FColor*	SourceData		= (FColor*) &RawData(0);

	UINT SampleCount = 0;
	//FLinearColor ColorSum; 이건 너무 느리다... 감마 조정 때문에 Power 연산이 들어가 있어서..
	FLinearColor ColorSum2;
	for( INT Y = StartY; Y <= EndY; ++Y )
	{
		for( INT X = StartX; X <= EndX; ++X)
		{
			FColor& C = SourceData[Y * SizeY + X];
			ColorSum2 += (C);
			++SampleCount;
		}
	}

	ColorSum2 /= (FLOAT)SampleCount;
	
	SampleColor = FColor(ColorSum2);

	return SampleColor;
}
//!} 2006-04-11	허 창 민
