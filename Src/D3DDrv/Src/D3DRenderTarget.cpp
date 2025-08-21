/*=============================================================================
	D3DRenderTarget.cpp: D3D render target implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3DDrvPrivate.h"
#include "BatchedElements.h"
#include "ScreenRendering.h"

#if USE_D3D_RHI

#include "D3DPerfSaver.h"

/** An option to emulate platforms which require resolving surfaces to a texture before sampling. */
#define REQUIRE_D3D_RESOLVE 0

/**
* Copies the contents of the given surface to its resolve target texture.
* @param SourceSurface - surface with a resolve texture to copy to
* @param bKeepOriginalSurface - TRUE if the original surface will still be used after this function so must remain valid
* @param ResolveParams - optional resolve params
*/
void RHICopyToResolveTarget(FSurfaceRHIParamRef SourceSurface, UBOOL bKeepOriginalSurface, const FResolveParams& ResolveParams)
{
	if( 
		(REQUIRE_D3D_RESOLVE || 
		 SourceSurface.Texture2D != SourceSurface.ResolveTargetTexture2D ||
		 IsValidRef(SourceSurface.ResolveTargetTextureCube)) )
	{		
		// surface can't be a part of both 2d/cube textures
		check(!(SourceSurface.Texture2D && SourceSurface.TextureCube));
		// surface can't have both 2d/cube resolve target textures
		check(!(IsValidRef(SourceSurface.ResolveTargetTexture2D) && IsValidRef(SourceSurface.ResolveTargetTextureCube)));

		// Get a handle for the destination surface.
		TD3DRef<IDirect3DSurface9> DestinationSurface;
		// resolving to 2d texture
		if( IsValidRef(SourceSurface.ResolveTargetTexture2D) )
		{
			// get level 0 surface from 2d texture
			VERIFYD3DRESULT(SourceSurface.ResolveTargetTexture2D->GetSurfaceLevel(0,DestinationSurface.GetInitReference()));
		}
		// resolving to cube texture
		else
		{
			// get cube face from cube texture
			VERIFYD3DRESULT(SourceSurface.ResolveTargetTextureCube->GetCubeMapSurface(
				GetD3DCubeFace(ResolveParams.CubeFace),0,DestinationSurface.GetInitReference()));
		}		

		// Determine the destination surface size.
		D3DSURFACE_DESC DestinationDesc;
		VERIFYD3DRESULT(DestinationSurface->GetDesc(&DestinationDesc));		

		// 2D 
		if( IsValidRef(SourceSurface.ResolveTargetTexture2D) && !SourceSurface.Texture2D )
		{
			GDirect3DDevice->StretchRect( SourceSurface, NULL, DestinationSurface, NULL, D3DTEXF_NONE );
		}
		else
		{
			// Construct a temporary FTexture to represent the source surface.
			FTexture TempTexture;
			TempTexture.SamplerStateRHI = TStaticSamplerState<SF_Nearest,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
			TempTexture.TextureRHI = (FTextureRHIRef&)SourceSurface.Texture2D;

			// Generate the vertices used to copy from the source surface to the destination surface.
			FLOAT MinX = -1.0f - GPixelCenterOffset / ((FLOAT)DestinationDesc.Width * 0.5f);
			FLOAT MaxX = +1.0f - GPixelCenterOffset / ((FLOAT)DestinationDesc.Width * 0.5f);
			FLOAT MinY = +1.0f + GPixelCenterOffset / ((FLOAT)DestinationDesc.Height * 0.5f);
			FLOAT MaxY = -1.0f + GPixelCenterOffset / ((FLOAT)DestinationDesc.Height * 0.5f);

			FBatchedElements BatchedElements;
			INT V00 = BatchedElements.AddVertex(FVector4(MinX,MinY,0,1),FVector2D(0,0),FColor(255,255,255),FHitProxyId());
			INT V10 = BatchedElements.AddVertex(FVector4(MaxX,MinY,0,1),FVector2D(1,0),FColor(255,255,255),FHitProxyId());
			INT V01 = BatchedElements.AddVertex(FVector4(MinX,MaxY,0,1),FVector2D(0,1),FColor(255,255,255),FHitProxyId());
			INT V11 = BatchedElements.AddVertex(FVector4(MaxX,MaxY,0,1),FVector2D(1,1),FColor(255,255,255),FHitProxyId());

			// Set the destination texture as the render target.
			GDirect3DDevice->SetRenderTarget(0,DestinationSurface);
			GD3DPerfSaver->SetRenderState_SRGBWRITEENABLE(FALSE);

			// No alpha blending, no depth tests or writes.
			RHISetBlendState(NULL,TStaticBlendState<>::GetRHI());
			RHISetDepthState(NULL,TStaticDepthState<FALSE,CF_Always>::GetRHI());

			// Draw a quad using the generated vertices.
			BatchedElements.AddTriangle(V00,V10,V11,&TempTexture,BLEND_Opaque);
			BatchedElements.AddTriangle(V00,V11,V01,&TempTexture,BLEND_Opaque);
			BatchedElements.Draw(NULL,FMatrix::Identity,DestinationDesc.Width,DestinationDesc.Height,FALSE);
		}		
	}
}

void RHICopyFromResolveTargetFast(FSurfaceRHIParamRef DestSurface)
{
	// these need to be referenced in order for the FScreenVertexShader/FScreenPixelShader types to not be compiled out on PC
	TShaderMapRef<FScreenVertexShader> ScreenVertexShader(GetGlobalShaderMap());
	TShaderMapRef<FScreenPixelShader> ScreenPixelShader(GetGlobalShaderMap());
}

void RHICopyToResolveTarget(FSurfaceRHIParamRef SourceSurface, INT SizeX, INT SizeY, ECubeFace CubeFace=CubeFace_PosX )
{
	if( SourceSurface.Texture2D &&
		(REQUIRE_D3D_RESOLVE || 
		SourceSurface.Texture2D != SourceSurface.ResolveTargetTexture2D ||
		IsValidRef(SourceSurface.ResolveTargetTextureCube)) )
	{
		// surface can't be a part of both 2d/cube textures
		check(!(SourceSurface.Texture2D && SourceSurface.TextureCube));
		// surface can't have both 2d/cube resolve target textures
		check(!(IsValidRef(SourceSurface.ResolveTargetTexture2D) && IsValidRef(SourceSurface.ResolveTargetTextureCube)));

		// Get a handle for the destination surface.
		TD3DRef<IDirect3DSurface9> DestinationSurface;
		// resolving to 2d texture
		if( IsValidRef(SourceSurface.ResolveTargetTexture2D) )
		{
			// get level 0 surface from 2d texture
			VERIFYD3DRESULT(SourceSurface.ResolveTargetTexture2D->GetSurfaceLevel(0,DestinationSurface.GetInitReference()));
		}
		// resolving to cube texture
		else
		{
			// get cube face from cube texture
			VERIFYD3DRESULT(SourceSurface.ResolveTargetTextureCube->GetCubeMapSurface(
				GetD3DCubeFace(CubeFace),0,DestinationSurface.GetInitReference()));
		}		

		// Determine the destination surface size.
		D3DSURFACE_DESC DestinationDesc;
		VERIFYD3DRESULT(DestinationSurface->GetDesc(&DestinationDesc));

		// Construct a temporary FTexture to represent the source surface.
		FTexture TempTexture;
		TempTexture.SamplerStateRHI = TStaticSamplerState<SF_Nearest,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
		TempTexture.TextureRHI = (FTextureRHIRef&)SourceSurface.Texture2D;

		// Generate the vertices used to copy from the source surface to the destination surface.
		FLOAT MinX = -1.0f - GPixelCenterOffset / ((FLOAT)SizeX * 0.5f);
		FLOAT MaxX = +1.0f - GPixelCenterOffset / ((FLOAT)SizeX * 0.5f);
		FLOAT MinY = +1.0f + GPixelCenterOffset / ((FLOAT)SizeY * 0.5f);
		FLOAT MaxY = -1.0f + GPixelCenterOffset / ((FLOAT)SizeY * 0.5f);

		FBatchedElements BatchedElements;
		FLOAT U = (FLOAT)SizeX / DestinationDesc.Width;
		FLOAT V = (FLOAT)SizeY / DestinationDesc.Height;
		INT V00 = BatchedElements.AddVertex(FVector4(MinX,MinY,0,1),FVector2D(0,0),FColor(255,255,255),FHitProxyId());
		INT V10 = BatchedElements.AddVertex(FVector4(MaxX,MinY,0,1),FVector2D(U,0),FColor(255,255,255),FHitProxyId());
		INT V01 = BatchedElements.AddVertex(FVector4(MinX,MaxY,0,1),FVector2D(0,V),FColor(255,255,255),FHitProxyId());
		INT V11 = BatchedElements.AddVertex(FVector4(MaxX,MaxY,0,1),FVector2D(U,V),FColor(255,255,255),FHitProxyId());

		// Set the destination texture as the render target.
		GDirect3DDevice->SetRenderTarget(0,DestinationSurface);
		GD3DPerfSaver->SetRenderState_SRGBWRITEENABLE(FALSE);

		// No alpha blending, no depth tests or writes.
		RHISetBlendState(NULL,TStaticBlendState<>::GetRHI());
		RHISetDepthState(NULL,TStaticDepthState<FALSE,CF_Always>::GetRHI());

		// Draw a quad using the generated vertices.
		BatchedElements.AddTriangle(V00,V10,V11,&TempTexture,BLEND_Opaque);
		BatchedElements.AddTriangle(V00,V11,V01,&TempTexture,BLEND_Opaque);
		BatchedElements.Draw(NULL,FMatrix::Identity,SizeX,SizeY,FALSE);
	}
}

void RHIFillSurface(FSurfaceRHIParamRef SourceSurface, INT SizeX, INT SizeY, const FLinearColor& Color, ECubeFace CubeFace=CubeFace_PosX )
{
	if( SourceSurface.Texture2D &&
		(REQUIRE_D3D_RESOLVE || 
		SourceSurface.Texture2D != SourceSurface.ResolveTargetTexture2D ||
		IsValidRef(SourceSurface.ResolveTargetTextureCube)) )
	{
		// surface can't be a part of both 2d/cube textures
		check(!(SourceSurface.Texture2D && SourceSurface.TextureCube));
		// surface can't have both 2d/cube resolve target textures
		check(!(IsValidRef(SourceSurface.ResolveTargetTexture2D) && IsValidRef(SourceSurface.ResolveTargetTextureCube)));

		// Get a handle for the destination surface.
		TD3DRef<IDirect3DSurface9> DestinationSurface;
		// resolving to 2d texture
		if( IsValidRef(SourceSurface.ResolveTargetTexture2D) )
		{
			// get level 0 surface from 2d texture
			VERIFYD3DRESULT(SourceSurface.ResolveTargetTexture2D->GetSurfaceLevel(0,DestinationSurface.GetInitReference()));
		}
		// resolving to cube texture
		else
		{
			// get cube face from cube texture
			VERIFYD3DRESULT(SourceSurface.ResolveTargetTextureCube->GetCubeMapSurface(
				GetD3DCubeFace(CubeFace),0,DestinationSurface.GetInitReference()));
		}		

		// Determine the destination surface size.
		D3DSURFACE_DESC DestinationDesc;
		VERIFYD3DRESULT(DestinationSurface->GetDesc(&DestinationDesc));		

		// Generate the vertices used to copy from the source surface to the destination surface.
		FLOAT MinX = -1.0f - GPixelCenterOffset / ((FLOAT)SizeX * 0.5f);
		FLOAT MaxX = +1.0f - GPixelCenterOffset / ((FLOAT)SizeX * 0.5f);
		FLOAT MinY = +1.0f + GPixelCenterOffset / ((FLOAT)SizeY * 0.5f);
		FLOAT MaxY = -1.0f + GPixelCenterOffset / ((FLOAT)SizeY * 0.5f);

		FBatchedElements BatchedElements;
		FLOAT U = (FLOAT)SizeX / DestinationDesc.Width;
		FLOAT V = (FLOAT)SizeY / DestinationDesc.Height;
		INT V00 = BatchedElements.AddVertex(FVector4(MinX,MinY,0,1),FVector2D(0,0),Color,FHitProxyId());
		INT V10 = BatchedElements.AddVertex(FVector4(MaxX,MinY,0,1),FVector2D(U,0),Color,FHitProxyId());
		INT V01 = BatchedElements.AddVertex(FVector4(MinX,MaxY,0,1),FVector2D(0,V),Color,FHitProxyId());
		INT V11 = BatchedElements.AddVertex(FVector4(MaxX,MaxY,0,1),FVector2D(U,V),Color,FHitProxyId());

		// Set the destination texture as the render target.
		GDirect3DDevice->SetRenderTarget(0,DestinationSurface);
		GD3DPerfSaver->SetRenderState_SRGBWRITEENABLE(FALSE);

		// No alpha blending, no depth tests or writes.
		RHISetBlendState(NULL,TStaticBlendState<>::GetRHI());
		RHISetDepthState(NULL,TStaticDepthState<FALSE,CF_Always>::GetRHI());

		// Draw a quad using the generated vertices.
		BatchedElements.AddTriangle(V00,V10,V11,GWhiteTexture,BLEND_Opaque);
		BatchedElements.AddTriangle(V00,V11,V01,GWhiteTexture,BLEND_Opaque);
		BatchedElements.Draw(NULL,FMatrix::Identity,SizeX,SizeY,FALSE);
	}
}

void RHICopyFromResolveTarget(FSurfaceRHIParamRef DestSurface)
{
}

struct FRHIAntialiasParameters
{
	FString				UserFriendlyName;

	D3DMULTISAMPLE_TYPE	MultisampleType;
	DWORD				MultisampleQuality;
};

static TArray<FRHIAntialiasParameters> GAntialiasingModes;
extern TD3DRef<IDirect3D9> GDirect3D;
extern UBOOL GIsNVidia;

void RHIEnumerateAntialiasing( EPixelFormat SceneColorBufferFormat, EPixelFormat DepthBufferFormat )
{	
	UINT AdapterIndex = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = DEBUG_SHADERS ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL;

	GAntialiasingModes.Empty();

	FRHIAntialiasParameters NewMode;

	NewMode.MultisampleQuality = 0;
	NewMode.MultisampleType = D3DMULTISAMPLE_NONE;
	NewMode.UserFriendlyName = TEXT( "N/A" );
	GAntialiasingModes.AddItem( NewMode );

	// MSAA CSAA SSAA
	for( D3DMULTISAMPLE_TYPE imst = D3DMULTISAMPLE_2_SAMPLES; imst <= D3DMULTISAMPLE_16_SAMPLES; imst = (D3DMULTISAMPLE_TYPE)(imst + 1) )
	{
		DWORD msQuality;
				
		if (SUCCEEDED( GDirect3D->CheckDeviceMultiSampleType( AdapterIndex, DeviceType, (D3DFORMAT)GPixelFormats[SceneColorBufferFormat].PlatformFormat, FALSE, imst, &msQuality )))
		{
			if (SUCCEEDED( GDirect3D->CheckDeviceMultiSampleType( AdapterIndex, DeviceType, (D3DFORMAT)GPixelFormats[DepthBufferFormat].PlatformFormat, FALSE, imst, &msQuality )))
			{
				NewMode.MultisampleType = imst;

				if (GIsNVidia)
				{
					if (imst == D3DMULTISAMPLE_4_SAMPLES)
					{
						// 16x
						if (msQuality > 4)
						{
							NewMode.UserFriendlyName = TEXT( "CSAA 16x" );
							NewMode.MultisampleQuality = 4;
							GAntialiasingModes.AddItem( NewMode );
						}

						// 8x
						if (msQuality > 2)
						{
							NewMode.UserFriendlyName = TEXT( "CSAA 8x" );
							NewMode.MultisampleQuality = 2;
							GAntialiasingModes.AddItem( NewMode );
						}
					}
#define DISABLE_CSAAQ 1

#ifndef DISABLE_CSAAQ
					else if (imst == D3DMULTISAMPLE_8_SAMPLES)
					{
						// 16xQ
						if (msQuality > 2)
						{
							NewMode.UserFriendlyName = TEXT( "CSAA 16xQ" );
							NewMode.MultisampleQuality = 2;
							GAntialiasingModes.AddItem( NewMode );
						}

						// 8xQ
						if (msQuality > 0)
						{
							NewMode.UserFriendlyName = TEXT( "CSAA 8xQ" );
							NewMode.MultisampleQuality = 0;
							GAntialiasingModes.AddItem( NewMode );
						}
					}
#endif
				}
				else
				{
					if (msQuality == 0)
						msQuality = 1;

					for (DWORD q = 0; q < msQuality; ++q)
					{
						NewMode.UserFriendlyName = FString::Printf( TEXT( "MSAA %dx" ), (INT)imst );

						if (q == 1)
						{
							NewMode.UserFriendlyName += TEXT("Q");
						}
						else if (q > 1)
						{
							NewMode.UserFriendlyName += FString::Printf( TEXT( "Q%d" ), q );
						}

						NewMode.MultisampleQuality = q;

						GAntialiasingModes.AddItem( NewMode );
					}
				}				
			}			
		}				
	}	
}

UBOOL RHIGetUserFriendlyAntialiasingName( INT Mode, FString& OutName )
{
	if (Mode >= 0 && Mode < GAntialiasingModes.Num())
	{
		OutName = GAntialiasingModes(Mode).UserFriendlyName;
		return TRUE;
	}

	return FALSE;
}

static INT GMSAAValue = 0, GMSAAQuality = 0;

/**
 * Antialias Mode 값 변경. 적용하고자하면 RecreateDevice해야한다.
 *
 * @param Mode - 적용하고자 하는 Mode값
 * @return		적용된 Mode값. 파라메터로 들어간 Mode가 그대로 적용되었다면 반환값과 같다. 적용에 실패하면 0
 */
INT RHISetAntialiasMode( INT Mode )
{
	INT ResultMode = Mode;

	if (Mode >= 0 && Mode < GAntialiasingModes.Num())
	{
		GMSAAQuality = GAntialiasingModes(Mode).MultisampleQuality;
		GMSAAValue = GAntialiasingModes(Mode).MultisampleType;
	}
	else
	{
		GMSAAQuality = 0;
		GMSAAValue = 0;
		ResultMode = 0;
	}

	return ResultMode;
}

/**
* Creates a RHI surface that can be bound as a render target.
* Note that a surface cannot be created which is both resolvable AND readable.
* @param SizeX - The width of the surface to create.
* @param SizeY - The height of the surface to create.
* @param Format - The surface format to create.
* @param ResolveTargetTexture - The 2d texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
* @param Flags - Surface creation flags
* @param UsageStr - Text describing usage for this surface
* @return The surface that was created.
*/
FSurfaceRHIRef RHICreateTargetableSurface(
	UINT SizeX,
	UINT SizeY,
	BYTE Format,
	FTexture2DRHIParamRef ResolveTargetTexture,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	

	UBOOL bDepthFormat = (Format == PF_DepthStencil || Format == PF_ShadowDepth|| Format == PF_FilteredShadowDepth || Format == PF_D24);

	if(IsValidRef(ResolveTargetTexture))
	{
		checkMsg(!(Flags&TargetSurfCreate_Readable),TEXT("Cannot allocate resolvable surfaces with the readable flag."));

		if((Flags&TargetSurfCreate_Dedicated) || REQUIRE_D3D_RESOLVE)
		{
			// Create a render target surface.
			TD3DRef<IDirect3DSurface9> Surface;

			// Apply MSAA only to default color/depth buffers
			if (appStristr( UsageStr, TEXT("DefaultColor" )) != NULL ||
				appStristr( UsageStr, TEXT("LightAttenuation" )) != NULL ||
				appStristr( UsageStr, TEXT("VelocityBuffer" )) != NULL)
			{
				GDirect3DDevice->CreateRenderTarget(
					SizeX,
					SizeY,
					(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
					(D3DMULTISAMPLE_TYPE)GMSAAValue,
					GMSAAQuality,
					(Flags&TargetSurfCreate_Readable),
					Surface.GetInitReference(),
					NULL
					);
			}

			if (!Surface)
			{
				VERIFYD3DRESULT(GDirect3DDevice->CreateRenderTarget(
					SizeX,
					SizeY,
					(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
					D3DMULTISAMPLE_NONE,
					0,
					(Flags&TargetSurfCreate_Readable),
					Surface.GetInitReference(),
					NULL
					));
			}			

			return FSurfaceRHIRef(ResolveTargetTexture,NULL,Surface);
		}
		else
		{
			// Simply return resolve target texture's surface.
			TD3DRef<IDirect3DSurface9> TargetableSurface;
			VERIFYD3DRESULT(ResolveTargetTexture->GetSurfaceLevel(0,TargetableSurface.GetInitReference()));

			return FSurfaceRHIRef(ResolveTargetTexture,ResolveTargetTexture,TargetableSurface);
		}
	}
	else
	{
		checkMsg((Flags&TargetSurfCreate_Dedicated),TEXT("Cannot allocated non-dedicated unresolvable surfaces."));

		if(bDepthFormat)
		{
			// Apply MSAA only to default color/depth buffers
			TD3DRef<IDirect3DSurface9> Surface;			

			if (appStristr( UsageStr, TEXT("MultisampleDepth" )) != NULL)
			{
				// Create a depth-stencil target surface.				
				VERIFYD3DRESULT(GDirect3DDevice->CreateDepthStencilSurface(
					SizeX,
					SizeY,
					(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
					(D3DMULTISAMPLE_TYPE)GMSAAValue,
					GMSAAQuality,
					TRUE,
					Surface.GetInitReference(),
					NULL
					));
			}
			
			if (!Surface)
			{
				// Create a depth-stencil target surface.				
				VERIFYD3DRESULT(GDirect3DDevice->CreateDepthStencilSurface(
					SizeX,
					SizeY,
					(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
					D3DMULTISAMPLE_NONE,
					0,
					TRUE,
					Surface.GetInitReference(),
					NULL
					));
			}
			
			return FSurfaceRHIRef(FTexture2DRHIRef(),NULL,Surface);
		}
		else
		{
			checkMsg((Flags&TargetSurfCreate_Readable),TEXT("Surface created which isn't readable or resolvable.  Is that intentional?"));

			// Create a render target surface.
			TD3DRef<IDirect3DSurface9> Surface;
			VERIFYD3DRESULT(GDirect3DDevice->CreateRenderTarget(
				SizeX,
				SizeY,
				(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
				D3DMULTISAMPLE_NONE,
				0,
				(Flags&TargetSurfCreate_Readable),
				Surface.GetInitReference(),
				NULL
				));
			return FSurfaceRHIRef(FTexture2DRHIRef(),NULL,Surface);
		}
	}
}

//  [2006/11/20 YTS, temporarily inserted]
// @deprecated - there's no use of drawing surface. volume can't be drawn to a surface
FSurfaceRHIRef RHICreateTargetable3DSurface(
	UINT SizeX,
	UINT SizeY,
	UINT SizeZ,
	BYTE Format,
	FTexture3DRHIParamRef ResolveTargetTexture,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	UBOOL bDepthFormat = (Format == PF_DepthStencil || Format == PF_ShadowDepth);

	if(IsValidRef(ResolveTargetTexture))
	{
		checkMsg(!(Flags&TargetSurfCreate_Readable),TEXT("Cannot allocate resolvable surfaces with the readable flag."));

		if((Flags&TargetSurfCreate_Dedicated) || REQUIRE_D3D_RESOLVE)
		{
			// Create a targetable texture.
			TD3DRef<IDirect3DVolumeTexture9> TargetableTexture;
			VERIFYD3DRESULT(GDirect3DDevice->CreateVolumeTexture(
				SizeX,
				SizeY,
				SizeZ,
				1,
				// Invalid usage field USAGE_RENDERTARGET.
				//bDepthFormat ? D3DUSAGE_DEPTHSTENCIL : D3DUSAGE_RENDERTARGET,
				0,
				(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
				//D3DPOOL_DEFAULT,
				D3DPOOL_MANAGED,
				TargetableTexture.GetInitReference(),
				NULL
				));

			// Retrieve the texture's surface.
			TD3DRef<IDirect3DVolume9> TargetableVolume;
			VERIFYD3DRESULT(TargetableTexture->GetVolumeLevel(0,TargetableVolume.GetInitReference()));

			return FSurfaceRHIRef(ResolveTargetTexture,TargetableTexture,TargetableVolume);
		}
		else
		{
			// Simply return resolve target texture's surface.
			TD3DRef<IDirect3DVolume9> TargetableVolume;
			VERIFYD3DRESULT(ResolveTargetTexture->GetVolumeLevel(0,TargetableVolume.GetInitReference()));

			return FSurfaceRHIRef(ResolveTargetTexture,ResolveTargetTexture,TargetableVolume);
		}
	}
	else
	{
		checkMsg((Flags&TargetSurfCreate_Dedicated),TEXT("Cannot allocated non-dedicated unresolvable surfaces."));

		if(bDepthFormat)
		{
			// Create a depth-stencil target surface.
			TD3DRef<IDirect3DSurface9> Surface;
			VERIFYD3DRESULT(GDirect3DDevice->CreateDepthStencilSurface(
				SizeX,
				SizeY,
				(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
				D3DMULTISAMPLE_NONE,
				0,
				TRUE,
				Surface.GetInitReference(),
				NULL
				));
			return FSurfaceRHIRef(FTexture2DRHIRef(),NULL,Surface);
		}
		else
		{
			checkMsg((Flags&TargetSurfCreate_Readable),TEXT("Surface created which isn't readable or resolvable.  Is that intentional?"));

			// Create a render target surface.
			TD3DRef<IDirect3DSurface9> Surface;
			VERIFYD3DRESULT(GDirect3DDevice->CreateRenderTarget(
				SizeX,
				SizeY,
				(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
				D3DMULTISAMPLE_NONE,
				0,
				(Flags&TargetSurfCreate_Readable),
				Surface.GetInitReference(),
				NULL
				));
			return FSurfaceRHIRef(FTexture2DRHIRef(),NULL,Surface);
		}
	}
}

/**
* Creates a RHI surface that can be bound as a render target and can resolve w/ a cube texture
* Note that a surface cannot be created which is both resolvable AND readable.
* @param SizeX - The width of the surface to create.
* @param Format - The surface format to create.
* @param ResolveTargetTexture - The cube texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
* @param CubeFace - face from resolve texture to use as surface
* @param Flags - Surface creation flags
* @param UsageStr - Text describing usage for this surface
* @return The surface that was created.
*/
FSurfaceRHIRef RHICreateTargetableCubeSurface(
	UINT SizeX,
	BYTE Format,
	FTextureCubeRHIRef ResolveTargetTexture,
	ECubeFace CubeFace,
	DWORD Flags,
	const TCHAR* UsageStr
	)
{
	check(Format != PF_DepthStencil);
	if(!IsValidRef(ResolveTargetTexture))
	{
		checkMsg(FALSE,TEXT("No resolve target cube texture specified.  Just use RHICreateTargetableSurface instead."));
	}
	else
	{
		checkMsg(!(Flags&TargetSurfCreate_Readable),TEXT("Cannot allocate resolvable surfaces with the readable flag."));

		// create a dedicated texture which contains the target surface
		if((Flags&TargetSurfCreate_Dedicated) || REQUIRE_D3D_RESOLVE)
		{
			// Create a targetable texture.
			TD3DRef<IDirect3DTexture9> TargetableTexture;
			VERIFYD3DRESULT(GDirect3DDevice->CreateTexture(
				SizeX,
				SizeX,
				1,
				(Format == PF_ShadowDepth) ? D3DUSAGE_DEPTHSTENCIL : D3DUSAGE_RENDERTARGET,
				(D3DFORMAT)GPixelFormats[Format].PlatformFormat,
				D3DPOOL_DEFAULT,
				TargetableTexture.GetInitReference(),
				NULL
				));

			// Retrieve the texture's surface.
			TD3DRef<IDirect3DSurface9> TargetableSurface;
			VERIFYD3DRESULT(TargetableTexture->GetSurfaceLevel(0,TargetableSurface.GetInitReference()));

			// use a dedicated texture for the target and the cube texture for resolves
			return FSurfaceRHIRef(ResolveTargetTexture,TargetableTexture,TargetableSurface);
		}
		// use a surface from the resolve texture
		else
		{
			// Simply return resolve target texture's surface corresponding to the given CubeFace.
			TD3DRef<IDirect3DSurface9> TargetableSurface;
			VERIFYD3DRESULT(ResolveTargetTexture->GetCubeMapSurface(GetD3DCubeFace(CubeFace),0,TargetableSurface.GetInitReference()));

			// use the same cube texture as the resolve and target textures 
			return FSurfaceRHIRef(ResolveTargetTexture,ResolveTargetTexture,TargetableSurface);
		}
	}

	return FSurfaceRHIRef();
}

UBOOL RHIIsSurfaceAllocated(FSurfaceRHIParamRef Surface)
{
	return TRUE;
}

void RHIDiscardSurface(FSurfaceRHIParamRef Surface)
{
}

void RHIReadSurfaceData(FSurfaceRHIRef &Surface,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<BYTE>& OutData,ECubeFace CubeFace)
{
	UINT SizeX = MaxX - MinX + 1;
	UINT SizeY = MaxY - MinY + 1;

	// Check the format of the surface.
	D3DSURFACE_DESC SurfaceDesc;
	VERIFYD3DRESULT(Surface->GetDesc(&SurfaceDesc));

	check(SurfaceDesc.Format == D3DFMT_A8R8G8B8);

	// Allocate the output buffer.
	OutData.Empty((MaxX - MinX + 1) * (MaxY - MinY + 1) * sizeof(FColor));

	// Read back the surface data from (MinX,MinY) to (MaxX,MaxY)
	D3DLOCKED_RECT	LockedRect;
	RECT			Rect;
	Rect.left	= MinX;
	Rect.top	= MinY;
	Rect.right	= MaxX + 1;
	Rect.bottom	= MaxY + 1;

	TD3DRef<IDirect3DSurface9> DestSurface = Surface;
	if( IsValidRef(Surface.ResolveTargetTexture2D) ||
		IsValidRef(Surface.ResolveTargetTextureCube) )
	{
		// create a temp 2d texture to copy render target to
		TD3DRef<IDirect3DTexture9> Texture2D;
		VERIFYD3DRESULT(GDirect3DDevice->CreateTexture(
			SizeX,
			SizeY,
			1,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_SYSTEMMEM,
			Texture2D.GetInitReference(),
			NULL
			));
		// get its surface 
		DestSurface = NULL;
		VERIFYD3DRESULT(Texture2D->GetSurfaceLevel(0,DestSurface.GetInitReference()));
		// get the render target surface
		TD3DRef<IDirect3DSurface9> SrcSurface;
		if( IsValidRef(Surface.ResolveTargetTextureCube) )
		{
			VERIFYD3DRESULT(Surface.ResolveTargetTextureCube->GetCubeMapSurface(GetD3DCubeFace(CubeFace),0,SrcSurface.GetInitReference()));
		}
		else
		{
			VERIFYD3DRESULT(Surface.ResolveTargetTexture2D->GetSurfaceLevel(0,SrcSurface.GetInitReference()));
		}		
        // copy render target data to memory
		VERIFYD3DRESULT(GDirect3DDevice->GetRenderTargetData(SrcSurface,DestSurface));
	}
	
	VERIFYD3DRESULT(DestSurface->LockRect(&LockedRect,&Rect,D3DLOCK_READONLY));

	for(UINT Y = MinY;Y <= MaxY;Y++)
	{
		BYTE* SrcPtr = (BYTE*)LockedRect.pBits + (Y - MinY) * LockedRect.Pitch;
		BYTE* DestPtr = (BYTE*)&OutData(OutData.Add(SizeX * sizeof(FColor)));
		appMemcpy(DestPtr,SrcPtr,SizeX * sizeof(FColor));
	}

	DestSurface->UnlockRect();
}


//<@ ava specific ; 2007. 4. 12 changmin
#include "..\..\engine\src\Half.h"
void RHIReadHdrSurfaceData(FSurfaceRHIRef &Surface,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<FLOAT>& OutData)
{
	UINT SizeX = MaxX - MinX + 1;
	UINT SizeY = MaxY - MinY + 1;

	// Check the format of the surface.
	D3DSURFACE_DESC SurfaceDesc;
	VERIFYD3DRESULT(Surface->GetDesc(&SurfaceDesc));

	// Allocate the output buffer.
	OutData.Empty();

	// Read back the surface data from (MinX,MinY) to (MaxX,MaxY)
	D3DLOCKED_RECT	LockedRect;
	RECT			Rect;
	Rect.left	= MinX;
	Rect.top	= MinY;
	Rect.right	= MaxX + 1;
	Rect.bottom	= MaxY + 1;

	TD3DRef<IDirect3DSurface9> DestSurface = Surface;
	if( IsValidRef(Surface.ResolveTargetTexture2D) )
	{
		// create a temp 2d texture to copy render target to
		TD3DRef<IDirect3DTexture9> Texture2D;
		VERIFYD3DRESULT(GDirect3DDevice->CreateTexture(
			SizeX,
			SizeY,
			1,
			0,
			D3DFMT_A16B16G16R16F,
			D3DPOOL_SYSTEMMEM,
			Texture2D.GetInitReference(),
			NULL
			));
		// get its surface 
		DestSurface = NULL;
		VERIFYD3DRESULT(Texture2D->GetSurfaceLevel(0,DestSurface.GetInitReference()));
		// get the render target surface
		TD3DRef<IDirect3DSurface9> SrcSurface;
		VERIFYD3DRESULT(Surface.ResolveTargetTexture2D->GetSurfaceLevel(0,SrcSurface.GetInitReference()));

		// copy render target data to memory
		VERIFYD3DRESULT(GDirect3DDevice->GetRenderTargetData(SrcSurface,DestSurface));
	}

	VERIFYD3DRESULT(DestSurface->LockRect(&LockedRect,&Rect,D3DLOCK_READONLY));

	INT PixelCount = SizeX * SizeY;
	WORD *SrcPtr = (WORD*)LockedRect.pBits;
	for( INT PixelIndex = 0; PixelIndex < PixelCount ; ++PixelIndex )
	{
		UINT Channel[4];
		Channel[0] = half_to_float( *SrcPtr++ );
		Channel[1] = half_to_float( *SrcPtr++ );
		Channel[2] = half_to_float( *SrcPtr++ );
		Channel[3] = half_to_float( *SrcPtr++ );
		OutData.AddItem( *((FLOAT*)&Channel[0]) );
		OutData.AddItem( *((FLOAT*)&Channel[1]) );
		OutData.AddItem( *((FLOAT*)&Channel[2]) );
	}

	DestSurface->UnlockRect();
}
//>@ ava

void RHICopyRenderTargetIntoSurface(
									FSurfaceRHIParamRef InSourceSurface, 
									FSurfaceRHIParamRef InTargetSurface, 
									FTextureRHIParamRef LDRTexture,
									FSurfaceRHIParamRef LDRSurface, 
									INT SourceX, INT SourceY, INT SizeX, INT SizeY, INT TargetX, INT TargetY )
{	
	TD3DRef<IDirect3DSurface9> SourceSurface, TargetSurface;

	if(!IsValidRef(InSourceSurface))
	{
		SourceSurface = GD3DBackBuffer;
	}
	else
	{
		SourceSurface = InSourceSurface;
	}

	RECT SourceRect;
	RECT DestRect;

	SourceRect.left = SourceX;
	SourceRect.top = SourceY;

	SourceRect.right = SourceRect.left + SizeX;
	SourceRect.bottom = SourceRect.top + SizeY;

	D3DSURFACE_DESC SourceDesc;
	VERIFYD3DRESULT( SourceSurface->GetDesc( &SourceDesc ) );

	D3DSURFACE_DESC DestinationDesc;
	VERIFYD3DRESULT( InTargetSurface->GetDesc( &DestinationDesc ) );	

	SourceRect.left = Clamp( SourceRect.left, (LONG)0, (LONG)SourceDesc.Width );
	SourceRect.right = Clamp( SourceRect.right, (LONG)0, (LONG)SourceDesc.Width );
	SourceRect.top = Clamp( SourceRect.top, (LONG)0, (LONG)SourceDesc.Height );
	SourceRect.bottom = Clamp( SourceRect.bottom, (LONG)0, (LONG)SourceDesc.Height );		

	DestRect.left = TargetX + SourceRect.left - SourceX;
	DestRect.top = TargetY + SourceRect.top - SourceY;

	DestRect.right = DestRect.left + SourceRect.right - SourceRect.left;
	DestRect.bottom = DestRect.top + SourceRect.bottom - SourceRect.top;

	if (DestRect.left < 0)
	{
		SourceRect.left -= DestRect.left;
		DestRect.left = 0;
	}
	else if (DestRect.right > (LONG)DestinationDesc.Width)
	{
		SourceRect.right -= DestRect.right - DestinationDesc.Width;
		DestRect.right = DestinationDesc.Width;
	}

	if (DestRect.top < 0)
	{
		SourceRect.top -= DestRect.top;
		DestRect.top = 0;
	}
	else if (DestRect.bottom > (LONG)DestinationDesc.Height)
	{
		SourceRect.bottom -= DestRect.bottom - DestinationDesc.Height;
		DestRect.bottom = DestinationDesc.Height;
	}
	
	INT Width = DestRect.right - DestRect.left;
	INT Height = DestRect.bottom - DestRect.top;

	/// 불필요한 경우 :)
	if (Width <= 0 || Height <= 0) 
		return;

	if (SourceDesc.Format == DestinationDesc.Format)
	{
		VERIFYD3DRESULT( GDirect3DDevice->StretchRect( SourceSurface, &SourceRect, InTargetSurface, &DestRect, D3DTEXF_NONE ) );	

		return;
	}

	VERIFYD3DRESULT( GDirect3DDevice->StretchRect( SourceSurface, &SourceRect, LDRSurface, &DestRect, D3DTEXF_NONE ) );	
	
	// Construct a temporary FTexture to represent the source surface.
	FTexture TempTexture;
	TempTexture.SamplerStateRHI = TStaticSamplerState<SF_Nearest,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	TempTexture.TextureRHI = LDRTexture;	

	// Generate the vertices used to copy from the source surface to the destination surface.
	FLOAT MinX = -1.0f + (2.0f*DestRect.left)/DestinationDesc.Width - GPixelCenterOffset / ((FLOAT)DestinationDesc.Width * 0.5f);
	FLOAT MaxX = -1.0f + (2.0f*DestRect.right)/DestinationDesc.Width - GPixelCenterOffset / ((FLOAT)DestinationDesc.Width * 0.5f);
	FLOAT MinY = +1.0f - (2.0f*DestRect.top)/DestinationDesc.Height + GPixelCenterOffset / ((FLOAT)DestinationDesc.Height * 0.5f);
	FLOAT MaxY = +1.0f - (2.0f*DestRect.bottom)/DestinationDesc.Height + GPixelCenterOffset / ((FLOAT)DestinationDesc.Height * 0.5f);

	FBatchedElements BatchedElements;
	INT V00 = BatchedElements.AddVertex(FVector4(MinX,MinY,0,1),FVector2D((FLOAT)DestRect.left / DestinationDesc.Width,(FLOAT)DestRect.top / DestinationDesc.Height),FColor(255,255,255),FHitProxyId());
	INT V10 = BatchedElements.AddVertex(FVector4(MaxX,MinY,0,1),FVector2D((FLOAT)DestRect.right / DestinationDesc.Width,(FLOAT)DestRect.top / DestinationDesc.Height),FColor(255,255,255),FHitProxyId());
	INT V01 = BatchedElements.AddVertex(FVector4(MinX,MaxY,0,1),FVector2D((FLOAT)DestRect.left / DestinationDesc.Width,(FLOAT)DestRect.bottom / DestinationDesc.Height),FColor(255,255,255),FHitProxyId());
	INT V11 = BatchedElements.AddVertex(FVector4(MaxX,MaxY,0,1),FVector2D((FLOAT)DestRect.right / DestinationDesc.Width,(FLOAT)DestRect.bottom / DestinationDesc.Height),FColor(255,255,255),FHitProxyId());

	// Set the destination texture as the render target.
	GDirect3DDevice->SetRenderTarget(0,InTargetSurface);
	GD3DPerfSaver->SetRenderState_SRGBWRITEENABLE(FALSE);

	// No alpha blending, no depth tests or writes.
	RHISetBlendState(NULL,TStaticBlendState<>::GetRHI());
	RHISetDepthState(NULL,TStaticDepthState<FALSE,CF_Always>::GetRHI());

	// Draw a quad using the generated vertices.
	BatchedElements.AddTriangle(V00,V10,V11,&TempTexture,BLEND_Opaque);
	BatchedElements.AddTriangle(V00,V11,V01,&TempTexture,BLEND_Opaque);
	BatchedElements.Draw(NULL,FMatrix::Identity,DestinationDesc.Width,DestinationDesc.Height,FALSE,GEngine->Client->DisplayGamma);

	/*GDirect3DDevice->SetRenderTarget(0,LDRSurface);

	// Set the viewport for the shadow.
	RHISetViewport( NULL, DestRect.left, DestRect.top, 0, DestRect.right, DestRect.bottom, 1.0f );
		
	// Clear color and depth.
	RHIClear( NULL, TRUE,FColor(255,255,255), FALSE,1.0f,FALSE,0 );*/
}

#endif
