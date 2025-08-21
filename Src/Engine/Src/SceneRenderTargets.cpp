/*=============================================================================
	SceneRenderTargets.cpp: Scene render target implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/** The global render targets used for scene rendering. */
TGlobalResource<FSceneRenderTargets> GSceneRenderTargets;

void FSceneRenderTargets::Allocate(UINT MinSizeX,UINT MinSizeY)
{
#if CONSOLE
	// force to always use the global screen sizes iot avoid reallocating the scene buffers
	MinSizeX = GScreenWidth;
	MinSizeY = GScreenHeight;
#endif

	if(BufferSizeX < MinSizeX || BufferSizeY < MinSizeY)
	{
		// Reinitialize the render targets for the given size.
		BufferSizeX = Max(BufferSizeX,MinSizeX);
		BufferSizeY = Max(BufferSizeY,MinSizeY);
		FilterDownsampleFactor = 4;
		FilterBufferSizeX = BufferSizeX / FilterDownsampleFactor + 2;
		FilterBufferSizeY = BufferSizeY / FilterDownsampleFactor + 2;
		Release();
		Init();
	}
}

void FSceneRenderTargets::BeginRenderingFilter()
{
	// Set the filter color surface as the render target
	RHISetRenderTarget(GlobalContext, GetFilterColorSurface(), FSurfaceRHIRef());
}

void FSceneRenderTargets::FinishRenderingFilter()
{
	// Resolve the filter color surface 
	RHICopyToResolveTarget(GetFilterColorSurface(), FALSE);
}

/**
* Sets the scene color target and restores its contents if necessary
* @param bRestoreContents - if TRUE then copies contents of SceneColorTexture to the SceneColorSurface
*/
void FSceneRenderTargets::BeginRenderingSceneColor(UBOOL bRestoreContents)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingSceneColor"));

	if(bRestoreContents)
	{
		// Initialize the scene color surface to its previously resolved contents.
		RHICopyFromResolveTarget(GetSceneColorSurface());
	}

	// Set the scene color surface as the render target, and the scene depth surface as the depth-stencil target.
	RHISetRenderTarget(GlobalContext, GetSceneColorSurface(), GetMSAAFriendlySceneDepthSurface());
}

/**
* Called when finished rendering to the scene color surface
* @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the SceneColorTexture
*/
void FSceneRenderTargets::FinishRenderingSceneColor(UBOOL bKeepChanges)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingSceneColor"));

	if(bKeepChanges)
	{
		if (!GSupportsDepthTextures)
		{
			extern UBOOL GSceneDepthDirty;
			GSceneDepthDirty = FALSE;
		}

		// Resolve the scene color surface to the scene color texture.
		RHICopyToResolveTarget(GetSceneColorSurface(), TRUE);
	}
}

/**
* Sets the LDR version of the scene color target.
*/
void FSceneRenderTargets::BeginRenderingSceneColorLDR()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingSceneColorLDR"));

	// Set the light attenuation surface as the render target, and the scene depth buffer as the depth-stencil surface.
	RHISetRenderTarget(GlobalContext,GetSceneColorLDRSurface(),GetSceneDepthSurface());
}

/**
* Called when finished rendering to the LDR version of the scene color surface.
* @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the LDR SceneColorTexture
* @param ResolveParams - optional resolve params
*/
void FSceneRenderTargets::FinishRenderingSceneColorLDR(UBOOL bKeepChanges,const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingSceneColorLDR"));

	if(bKeepChanges)
	{
		// Resolve the scene color surface to the scene color texture.
		RHICopyToResolveTarget(GetSceneColorLDRSurface(), TRUE, ResolveParams);
	}
}


/**
* Saves a previously rendered scene color target
*/
void FSceneRenderTargets::ResolveSceneColor()
{
    SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("ResolveSceneColor"));
    RHICopyToResolveTarget(GetSceneColorSurface(), TRUE);
}

/**
* Sets the raw version of the scene color target.
*/
void FSceneRenderTargets::BeginRenderingSceneColorRaw()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingSceneColorRaw"));

	// Use the raw version of the scene color as the render target, and use the standard scene depth buffer as the depth-stencil surface.
	RHISetRenderTarget(GlobalContext, GetSceneColorRawSurface(), GetSceneDepthSurface());
}

/**
 * Saves a previously rendered scene color surface in the raw bit format.
 */
void FSceneRenderTargets::SaveSceneColorRaw()
{
	RHICopyToResolveTarget(GetSceneColorRawSurface(), TRUE);
}

/**
 * Restores a previously saved raw-scene color surface.
 */
void FSceneRenderTargets::RestoreSceneColorRaw()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("RestoreSceneColorRaw"));

	// Initialize the scene color surface to its previously resolved contents.
	//RHICopyFromResolveTargetFast(GetSceneColorRawSurface());

	// Set the scene color surface as the render target, and the scene depth surface as the depth-stencil target.
	RHISetRenderTarget(GlobalContext, GetSceneColorSurface(), GetMSAAFriendlySceneDepthSurface());
}

void FSceneRenderTargets::BeginRenderingPrePass( UBOOL bRequireVelocity )
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingPrePass"));

	// Set the motion blur velocity buffer as the render target, and the scene depth surface as the depth-stencil target.
	RHISetRenderTarget(GlobalContext, bRequireVelocity ? GetVelocitySurface() : GetLightAttenuationSurface(), GetMSAAFriendlySceneDepthSurface());
}

void FSceneRenderTargets::FinishRenderingPrePass( UBOOL bRequiresVelocities )
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingPrePass"));
	if ( bRequiresVelocities )
	{
		// Resolve the velocity buffer to a texture, so it can be read later.
		RHICopyToResolveTarget(GetVelocitySurface(), TRUE);
	}
}

void FSceneRenderTargets::BeginRenderingShadowVolumes()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingShadowVolumes"));

	// Make sure we are writing to the same depth stencil buffer as
	// BeginRenderingSceneColor and BeginRenderingLightAttenuation.
	//
	// Note that we're not actually writing anything to the color
	// buffer here. It could be anything with the same dimension.
	RHISetRenderTarget(GlobalContext,GetLightAttenuationSurface(),GetMSAAFriendlySceneDepthSurface());
	RHISetColorWriteEnable(GlobalContext,FALSE);
}

void FSceneRenderTargets::FinishRenderingShadowVolumes()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingShadowVolumes"));
	RHISetColorWriteEnable(GlobalContext,TRUE);
}

void FSceneRenderTargets::BeginRenderingShadowDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingShadowDepth"));

	if(GSupportsHardwarePCF || GSupportsFetch4)
	{
		// set the shadow z surface as the depth buffer
		// have to bind a color target that is the same size as the depth texture on platforms that support Hardware PCF
		RHISetRenderTarget(GlobalContext,GetShadowDepthColorSurface(), GetShadowDepthZSurface());   
		// disable color writes since we only want z depths
		RHISetColorWriteEnable(GlobalContext,FALSE);
	}
	else if( GSupportsDepthTextures)
	{
		// set the shadow z surface as the depth buffer
		RHISetRenderTarget(GlobalContext,FSurfaceRHIRef(), GetShadowDepthZSurface());   
		// disable color writes since we only want z depths
		RHISetColorWriteEnable(GlobalContext,FALSE);
	}
	else
	{
		// Set the shadow color surface as the render target, and the shadow z surface as the depth buffer
		RHISetRenderTarget(GlobalContext,GetShadowDepthColorSurface(), GetShadowDepthZSurface());
	}
}

/**
* Called when finished rendering to the subject shadow depths so the surface can be copied to texture
* @param ResolveParams - optional resolve params
*/
void FSceneRenderTargets::FinishRenderingShadowDepth(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingShadowDepth"));

	if( GSupportsDepthTextures || GSupportsHardwarePCF || GSupportsFetch4)
	{
		// Resolve the shadow depth z surface.
		RHICopyToResolveTarget(GetShadowDepthZSurface(), FALSE, ResolveParams);
		// restore color writes
		RHISetColorWriteEnable(GlobalContext,TRUE);
	}
	else
	{
		// Resolve the shadow depth color surface.
		RHICopyToResolveTarget(GetShadowDepthColorSurface(), FALSE, ResolveParams);
	}
}

//<@ ava specific ; 2007. 11. 9 changmin
void FSceneRenderTargets::AVA_BeginRenderingNearShadowDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("AVA_BeginRenderingNearShadowDepth"));

	if(GSupportsHardwarePCF || GSupportsFetch4)
	{
		// set the shadow z surface as the depth buffer
		// have to bind a color target that is the same size as the depth texture on platforms that support Hardware PCF
		RHISetRenderTarget(GlobalContext,GetShadowDepthColorSurface(), AVA_GetNearShadowDepthZSurface());   
		// disable color writes since we only want z depths
		RHISetColorWriteEnable(GlobalContext,FALSE);
	}
	else if( GSupportsDepthTextures)
	{
		// set the shadow z surface as the depth buffer
		RHISetRenderTarget(GlobalContext,FSurfaceRHIRef(), AVA_GetNearShadowDepthZSurface());   
		// disable color writes since we only want z depths
		RHISetColorWriteEnable(GlobalContext,FALSE);
	}
	else
	{
		// Set the shadow color surface as the render target, and the shadow z surface as the depth buffer
		RHISetRenderTarget(GlobalContext,GetShadowDepthColorSurface(), AVA_GetNearShadowDepthZSurface());
	}
}
void FSceneRenderTargets::AVA_FinishRenderingNearShadowDepth(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("AVA_FinishRenderingNearShadowDepth"));

	if( GSupportsDepthTextures || GSupportsHardwarePCF || GSupportsFetch4)
	{
		// Resolve the shadow depth z surface.
		RHICopyToResolveTarget(AVA_GetNearShadowDepthZSurface(), FALSE, ResolveParams);
		// restore color writes
		RHISetColorWriteEnable(GlobalContext,TRUE);
	}
	else
	{
		// Resolve the shadow depth color surface.
		RHICopyToResolveTarget(GetShadowDepthColorSurface(), FALSE, ResolveParams);
	}
}
//>@ ava

#if SUPPORTS_VSM
void FSceneRenderTargets::BeginRenderingShadowVariance()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingShadowVariance"));
	// Set the shadow variance surface
	RHISetRenderTarget(GlobalContext,GetShadowVarianceSurface(), FSurfaceRHIRef());
}

void FSceneRenderTargets::FinishRenderingShadowVariance(const FResolveParams& ResolveParams)
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingShadowVariance"));
	// Resolve the shadow variance surface.
	RHICopyToResolveTarget(GetShadowVarianceSurface(), FALSE,ResolveParams);
}

UINT FSceneRenderTargets::GetShadowVarianceTextureResolution() const
{
	//<@ ava specific ; 2008. 1. 2 changmin
	extern UBOOL GUseCascadedShadow;
	if( GUseCascadedShadow )
		return GAvaShadowDepthBufferSize;
	else
	//>@ ava
	return GShadowDepthBufferSize;
}
#endif //#if SUPPORTS_VSM

void FSceneRenderTargets::BeginRenderingLightAttenuation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingLightAttenuation"));

	// Set the light attenuation surface as the render target, and the scene depth buffer as the depth-stencil surface.
	RHISetRenderTarget(GlobalContext,GetLightAttenuationSurface(),GetSceneDepthSurface());
}

void FSceneRenderTargets::FinishRenderingLightAttenuation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingLightAttenuation"));

	// Resolve the light attenuation surface.
	RHICopyToResolveTarget(GetLightAttenuationSurface(), FALSE);
}

void FSceneRenderTargets::BeginRenderingDistortionAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingDistortionAccumulation"));

	// use RGBA8 light target for accumulating distortion offsets	
	// R = positive X offset
	// G = positive Y offset
	// B = negative X offset
	// A = negative Y offset

	RHISetRenderTarget(GlobalContext,GetLightAttenuationSurface(),GetSceneDepthSurface());
}

void FSceneRenderTargets::FinishRenderingDistortionAccumulation()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingDistortionAccumulation"));

	RHICopyToResolveTarget(GetLightAttenuationSurface(), FALSE);
}

void FSceneRenderTargets::BeginRenderingDistortionDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("BeginRenderingDistortionDepth"));
}

void FSceneRenderTargets::FinishRenderingDistortionDepth()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("FinishRenderingDistortionDepth"));
}

void FSceneRenderTargets::ResolveSceneDepthTexture()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("ResolveSceneDepthTexture"));

	if(GSupportsDepthTextures)
	{
		// Resolve the scene depth surface.
		RHICopyToResolveTarget(GetSceneDepthSurface(), TRUE);
	}	
}

UINT Aling(UINT Size, UINT Alignment)
{
	return (Size + Alignment - 1) & ~(Alignment - 1);
}

void FSceneRenderTargets::InitDynamicRHI()
{
	if(BufferSizeX > 0 && BufferSizeY > 0)
	{
		//<@ ava specific ; 2007. 7. 10 changmin
		if( GRHIShaderPlatform == SP_PCD3D_SM2_POOR )
		{
			check( GIsLowEndHW );
		}
		//>@ ava

		//<@ 2007. 5. 25 changmin
		// sm3에서도 allowbloom, allowdepthoffield가 아니면, filter buffer 가 필요없네요..
		//if (!IsSM2Platform(GRHIShaderPlatform) || GSystemSettings->bAllowBloom || GSystemSettings->bAllowDepthOfField)
		if ( GSystemSettings->bAllowBloom || GSystemSettings->bAllowDepthOfField)
		//>@ ava
		{
			// Create the filter targetable texture and surface.
			RenderTargets[FilterColor].Texture = RHICreateTexture2D(FilterBufferSizeX,FilterBufferSizeY,PF_A16B16G16R16,1,TexCreate_ResolveTargetable);
			RenderTargets[FilterColor].Surface = RHICreateTargetableSurface(
				FilterBufferSizeX,FilterBufferSizeY,PF_A16B16G16R16,RenderTargets[FilterColor].Texture,TargetSurfCreate_Dedicated,TEXT("FilterColor"));
		}

		EPixelFormat SceneColorBufferFormat = PF_FloatRGB;
		if(GSupportsDepthTextures)
		{
			// Create a texture to store the resolved scene depth, and a render-targetable surface to hold the unresolved scene depth.
			RenderTargets[SceneDepthZ].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,PF_DepthStencil,1,TexCreate_DepthStencil);
			RenderTargets[SceneDepthZ].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_DepthStencil,RenderTargets[SceneDepthZ].Texture,0,TEXT("DefaultDepth"));						
		}
		else
		{
			// Create a surface to store the unresolved scene depth.
			RenderTargets[SceneDepthZ].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_DepthStencil,FTexture2DRHIRef(),TargetSurfCreate_Dedicated,TEXT("DefaultDepth"));
			// Allocate an alpha channel in the scene color texture to store the resolved scene depth.
			SceneColorBufferFormat = PF_FloatRGBA;
			
			if (GIsLowEndHW)
			{
				SceneColorBufferFormat = PF_A8R8G8B8;
			}
		}

		void RHIEnumerateAntialiasing( EPixelFormat SceneColorBufferFormat, EPixelFormat DepthBufferFormat );
		UBOOL RHIGetUserFriendlyAntialiasingName( INT Mode, FString& OutName );
		UBOOL RHISetAntialiasMode( INT Mode );

		RHIEnumerateAntialiasing( SceneColorBufferFormat, PF_DepthStencil );		
		
		UBOOL bUsingMSAA = RHISetAntialiasMode( GSystemSettings->Antialiasing );

		if (bUsingMSAA)
		{
			FString AAMode;
			RHIGetUserFriendlyAntialiasingName( GSystemSettings->Antialiasing, AAMode );

			debugf( NAME_Log, TEXT( "Antialiasing mode : %s" ), *AAMode );

			RenderTargets[MultisampleDepthZ].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_DepthStencil,FTexture2DRHIRef(),TargetSurfCreate_Dedicated,TEXT("MultisampleDepth"));			
		}
		else
		{
			RenderTargets[MultisampleDepthZ].Surface = RenderTargets[SceneDepthZ].Surface;
		}

#if !XBOX		
		// Create a texture to store the resolved scene colors, and a dedicated render-targetable surface to hold the unresolved scene colors.
		RenderTargets[SceneColor].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,SceneColorBufferFormat,1,TexCreate_ResolveTargetable);
		//<@ ava specific ; 2007. 5. 11 changmin
		extern UBOOL GRequireResolve;
		RenderTargets[SceneColor].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,SceneColorBufferFormat,RenderTargets[SceneColor].Texture, (bUsingMSAA || GRequireResolve ) ? TargetSurfCreate_Dedicated : 0,TEXT("DefaultColor"));
		RenderTargets[SceneColorRaw].Texture = RenderTargets[SceneColor].Texture;
		RenderTargets[SceneColorRaw].Surface = RenderTargets[SceneColor].Surface;
#else
		// Create a texture to store the resolved scene colors, and a dedicated render-targetable surface to hold the unresolved scene colors.
		const SIZE_T ExpandedSceneColorSize = RHICalculateTextureBytes(BufferSizeX,BufferSizeY,1,SceneColorBufferFormat);
		const SIZE_T RawSceneColorSize = RHICalculateTextureBytes(BufferSizeX,BufferSizeY,1,PF_A2B10G10R10);
		const SIZE_T SharedSceneColorSize = Max(ExpandedSceneColorSize,RawSceneColorSize);
		FSharedMemoryResourceRHIRef MemoryBuffer = RHICreateSharedMemory(SharedSceneColorSize);

		RenderTargets[SceneColor].Texture = RHICreateSharedTexture2D(BufferSizeX,BufferSizeY,SceneColorBufferFormat,1,MemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[SceneColor].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,SceneColorBufferFormat,RenderTargets[SceneColor].Texture,TargetSurfCreate_Dedicated,TEXT("DefaultColor"));

		// Create a version of the scene color textures that represent the raw bits (i.e. that can do the resolves without any format conversion)
		RenderTargets[SceneColorRaw].Texture = RHICreateSharedTexture2D(BufferSizeX,BufferSizeY,PF_A2B10G10R10,1,MemoryBuffer,TexCreate_ResolveTargetable);
		RenderTargets[SceneColorRaw].Surface = RHICreateTargetableSurface(
			BufferSizeX,BufferSizeY,PF_A2B10G10R10,RenderTargets[SceneColorRaw].Texture,TargetSurfCreate_Dedicated,TEXT("DefaultColorRaw"));
#endif		
		if (GSystemSettings->bAllowDynamicShadows)
		{		
			//create the shadow depth color surface
			//platforms with GSupportsDepthTextures don't need a depth color target
			//platforms with GSupportsHardwarePCF still need a color target, due to API restrictions
			if (!GSupportsDepthTextures)
			{
				RenderTargets[ShadowDepthColor].Texture = RHICreateTexture2D(GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_R32F,1,TexCreate_ResolveTargetable);
				RenderTargets[ShadowDepthColor].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_R32F,RenderTargets[ShadowDepthColor].Texture,0,TEXT("ShadowDepthRT"));

			}

			//create the shadow depth texture and/or surface
			if (GSupportsHardwarePCF)
			{
				// Create a depth texture, used to sample PCF values
				RenderTargets[ShadowDepthZ].Texture = RHICreateTexture2D(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_FilteredShadowDepth,1,TexCreate_DepthStencil);

				// Don't create a dedicated surface
				RenderTargets[ShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_FilteredShadowDepth,
					RenderTargets[ShadowDepthZ].Texture,
					0,
					TEXT("ShadowDepthZ")
					);

				//<@ ava specific ; 2007. 11. 9 changmin
				// add cascaded shadow
				// Create a depth texture, used to sample PCF values
				RenderTargets[AVA_NearShadowDepthZ].Texture = RHICreateTexture2D(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_FilteredShadowDepth,1,TexCreate_DepthStencil);

				// Don't create a dedicated surface
				RenderTargets[AVA_NearShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_FilteredShadowDepth,
					RenderTargets[AVA_NearShadowDepthZ].Texture,
					0,
					TEXT("AVA_NearShadowDepthZ")
					);
				//>@ ava specifc
			}
			else if (GSupportsFetch4)
			{
				// Create a D24 depth stencil texture for use with Fetch4 shadows
				RenderTargets[ShadowDepthZ].Texture = RHICreateTexture2D(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_D24,1,TexCreate_DepthStencil);

				// Don't create a dedicated surface
				RenderTargets[ShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_D24,
					RenderTargets[ShadowDepthZ].Texture,
					0,
					TEXT("ShadowDepthZ")
					);

				//<@ ava specific ; 2007. 11. 9 changmin
				// add cascaded shadow
				// Create a D24 depth stencil texture for use with Fetch4 shadows
				RenderTargets[AVA_NearShadowDepthZ].Texture = RHICreateTexture2D(
					GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_D24,1,TexCreate_DepthStencil);

				// Don't create a dedicated surface
				RenderTargets[AVA_NearShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_D24,
					RenderTargets[AVA_NearShadowDepthZ].Texture,
					0,
					TEXT("AVA_NearShadowDepthZ")
					);
				//>@ ava
			}
			else
			{
				if( GSupportsDepthTextures )
				{
					// Create a texture to store the resolved shadow depth
					RenderTargets[ShadowDepthZ].Texture = RHICreateTexture2D(
						GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_ShadowDepth,1,TexCreate_ResolveTargetable);
				}			

				// Create a dedicated depth-stencil target surface for shadow depth rendering.
				RenderTargets[ShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_ShadowDepth,
					RenderTargets[ShadowDepthZ].Texture,
					TargetSurfCreate_Dedicated,
					TEXT("ShadowDepthZ")
					);
				//<@ ava specific ; 2007. 11. 9 changmin
				// add cascaded shadow
				if( GSupportsDepthTextures )
				{
					// Create a texture to store the resolved shadow depth
					RenderTargets[AVA_NearShadowDepthZ].Texture = RHICreateTexture2D(
						GetShadowDepthTextureResolution(),GetShadowDepthTextureResolution(),PF_ShadowDepth,1,TexCreate_ResolveTargetable);
				}			

				// Create a dedicated depth-stencil target surface for shadow depth rendering.
				RenderTargets[AVA_NearShadowDepthZ].Surface = RHICreateTargetableSurface(
					GetShadowDepthTextureResolution(),
					GetShadowDepthTextureResolution(),
					PF_ShadowDepth,
					RenderTargets[AVA_NearShadowDepthZ].Texture,
					TargetSurfCreate_Dedicated,
					TEXT("AVA_NearShadowDepthZ")
					);

				//>@ ava
			}
		}

		// Are dynamic shadows not allowed?
		if ( !GSystemSettings->bAllowDynamicShadows )
		{
			RenderTargets[ShadowDepthColor].Texture.Release();
			RenderTargets[ShadowDepthColor].Surface.Release();
			RenderTargets[ShadowDepthZ].Texture.Release();
			RenderTargets[ShadowDepthZ].Surface.Release();
		}

#if SUPPORTS_VSM
		// We need a 2-channel format to support VSM, and a dedicated surface to work with the filtering
		EPixelFormat ShadowVarianceFmt=PF_G16R16F;
		RenderTargets[ShadowVariance].Texture = RHICreateTexture2D(GetShadowVarianceTextureResolution(),GetShadowVarianceTextureResolution(), ShadowVarianceFmt,1,TexCreate_ResolveTargetable);
		RenderTargets[ShadowVariance].Surface = RHICreateTargetableSurface(
			GetShadowVarianceTextureResolution(),
			GetShadowVarianceTextureResolution(), 
			ShadowVarianceFmt,RenderTargets[ShadowVariance].Texture,
			TargetSurfCreate_Dedicated,TEXT("ShadowVariance")
			);
#endif //#if SUPPORTS_VSM


			// Create a texture to store the resolved light attenuation values, and a render-targetable surface to hold the unresolved light attenuation values.
			RenderTargets[LightAttenuation].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,PF_A8R8G8B8,1,TexCreate_ResolveTargetable);
			RenderTargets[LightAttenuation].Surface = RHICreateTargetableSurface(
				BufferSizeX,BufferSizeY,PF_A8R8G8B8,RenderTargets[LightAttenuation].Texture,bUsingMSAA ? TargetSurfCreate_Dedicated : 0,TEXT("LightAttenuation"));									

		if (!GIsLowEndHW)
		{
			// 일단 ava에서는 motion blur를 안쓸 예정
			if (GIsEditor)
			{
				// Create a texture to store the resolved velocity 2d-vectors, and a render-targetable surface to hold them.
				RenderTargets[VelocityBuffer].Texture = RHICreateTexture2D(BufferSizeX,BufferSizeY,PF_G16R16,1,TexCreate_ResolveTargetable);
				RenderTargets[VelocityBuffer].Surface = RHICreateTargetableSurface(
					BufferSizeX,BufferSizeY,PF_G16R16,RenderTargets[VelocityBuffer].Texture,bUsingMSAA ? TargetSurfCreate_Dedicated : 0,TEXT("VelocityBuffer"));
			}		
		}		
	}
	GlobalContext = RHIGetGlobalContext();
}

void FSceneRenderTargets::ReleaseDynamicRHI()
{
	// make sure no scene render targets and textures are in use before releasing them
	RHISetRenderTarget(GlobalContext,FSurfaceRHIRef(),FSurfaceRHIRef());

	for( INT RTIdx=0; RTIdx < MAX_SCENE_RENDERTARGETS; RTIdx++ )
	{
		RenderTargets[RTIdx].Texture.Release();
		RenderTargets[RTIdx].Surface.Release();
	}
}

UINT FSceneRenderTargets::GetShadowDepthTextureResolution() const
{
	//<@ ava specific ; 2007. 1. 2 changmin
	extern UBOOL GUseCascadedShadow;
	if( GUseCascadedShadow )
		return GAvaShadowDepthBufferSize;
	//>@ ava
	return GShadowDepthBufferSize;
}

//
void FSceneTextureShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	// only used if Material has an expression that requires SceneColorTexture
	SceneColorTextureParameter.Bind(ParameterMap,TEXT("SceneColorTexture"),TRUE);
	// only used if Material has an expression that requires SceneDepthTexture
	SceneDepthTextureParameter.Bind(ParameterMap,TEXT("SceneDepthTexture"),TRUE);
	// only used if Material has an expression that requires SceneDepthTexture
	SceneDepthCalcParameter.Bind(ParameterMap,TEXT("MinZ_MaxZRatio"),TRUE);
	// only used if Material has an expression that requires ScreenPosition biasing
	ScreenPositionScaleBiasParameter.Bind(ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);
}

//
void FSceneTextureShaderParameters::Set(FCommandContextRHI* Context, const FSceneView* View,FShader* PixelShader, ESamplerFilter ColorFilter/*=SF_Nearest*/, UBOOL UseLDRTexture/*=FALSE*/) const
{
	if (SceneColorTextureParameter.IsBound() == TRUE)
	{
		FSamplerStateRHIRef Filter;
		switch ( ColorFilter )
		{
			case SF_Linear:
				Filter = TStaticSamplerState<SF_Linear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
			case SF_AnisotropicLinear:
				Filter = TStaticSamplerState<SF_AnisotropicLinear,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
			case SF_Nearest:
			default:
				Filter = TStaticSamplerState<SF_Nearest,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
				break;
		}

		SetTextureParameter(
			Context,
			PixelShader->GetPixelShader(),
			SceneColorTextureParameter,
			Filter,
			UseLDRTexture ? GSceneRenderTargets.GetSceneColorLDRTexture() : GSceneRenderTargets.GetSceneColorTexture()
			);
	}
	if (SceneDepthTextureParameter.IsBound() == TRUE &&
		IsValidRef(GSceneRenderTargets.GetSceneDepthTexture()) == TRUE)
	{
		SetTextureParameter(
			Context,
			PixelShader->GetPixelShader(),
			SceneDepthTextureParameter,
			TStaticSamplerState<SF_Nearest,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
			GSceneRenderTargets.GetSceneDepthTexture()
			);
	}
	RHISetViewPixelParameters( Context, View, PixelShader->GetPixelShader(), &SceneDepthCalcParameter, &ScreenPositionScaleBiasParameter );
}

//
FArchive& operator<<(FArchive& Ar,FSceneTextureShaderParameters& Parameters)
{
	Ar << Parameters.SceneColorTextureParameter;
	Ar << Parameters.SceneDepthTextureParameter;
	Ar << Parameters.SceneDepthCalcParameter;
	Ar << Parameters.ScreenPositionScaleBiasParameter;
	return Ar;
}

void FSceneRenderTargets::OverrideSceneColorSurface( FSurfaceRHIRef Surface )
{
	RenderTargets[SceneColor].Surface = Surface;
}

/*-----------------------------------------------------------------------------
FSceneRenderTargetProxy
-----------------------------------------------------------------------------*/

/**
* Constructor
*/
FSceneRenderTargetProxy::FSceneRenderTargetProxy()
:	SizeX(0)
,	SizeY(0)
{	
}

/**
* Set SizeX and SizeY of proxy and re-allocate scene targets as needed
*
* @param InSizeX - scene render target width requested
* @param InSizeY - scene render target height requested
*/
void FSceneRenderTargetProxy::SetSizes(UINT InSizeX,UINT InSizeY)
{
	SizeX = InSizeX;
	SizeY = InSizeY;

	if( IsInRenderingThread() )
	{
		GSceneRenderTargets.Allocate(SizeX,SizeY);
	}
	else
	{
		struct FRenderTargetSizeParams
		{
			UINT SizeX;
			UINT SizeY;
		};
		FRenderTargetSizeParams RenderTargetSizeParams = {SizeX,SizeY};
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			RenderTargetAllocProxyCommand,
			FRenderTargetSizeParams,Parameters,RenderTargetSizeParams,
		{
			GSceneRenderTargets.Allocate(Parameters.SizeX, Parameters.SizeY);
		});
	}
}

/**
* @return RHI surface for setting the render target
*/
const FSurfaceRHIRef& FSceneRenderTargetProxy::GetRenderTargetSurface() const
{
	return GSceneRenderTargets.GetSceneColorSurface();
}

/**
* @return width of the scene render target this proxy will render to
*/
UINT FSceneRenderTargetProxy::GetSizeX() const
{
	if( SizeX != 0 )
	{
		return Min<UINT>(SizeX,GSceneRenderTargets.GetBufferSizeX());
	}
	else
	{
		return GSceneRenderTargets.GetBufferSizeX();
	}	
}

/**
* @return height of the scene render target this proxy will render to
*/
UINT FSceneRenderTargetProxy::GetSizeY() const
{
	if( SizeY != 0 )
	{
		return Min<UINT>(SizeY,GSceneRenderTargets.GetBufferSizeY());
	}
	else
	{
		return GSceneRenderTargets.GetBufferSizeY();
	}
}

/**
* @return gamma this render target should be rendered with
*/
FLOAT FSceneRenderTargetProxy::GetDisplayGamma() const
{
	return 1.0f;
}
