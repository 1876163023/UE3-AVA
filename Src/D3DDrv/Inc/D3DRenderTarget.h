/*=============================================================================
	D3DRenderTarget.h: D3D render target RHI definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_D3D_RHI

/**
 * A D3D surface, and an optional texture handle which can be used to read from the surface.
 */
class FSurfaceRHIRef : public TD3DRef<IDirect3DSurface9>
{
public:
	/** 2d texture to resolve surface to */
	FTexture2DRHIRef ResolveTargetTexture2D;
	/** Cube texture to resolve surface to */
	FTextureCubeRHIRef ResolveTargetTextureCube;
	/** Dedicated texture when not rendering directly to resolve target texture surface */
	TD3DRef<IDirect3DTexture9> Texture2D;
	/** Dedicated texture when not rendering directly to resolve target texture surface */
	TD3DRef<IDirect3DCubeTexture9> TextureCube;

	//  [2006/11/20 YTS, temporarily inserted] some code related with a constructor left below
	FTexture3DRHIRef ResolveTargetTexture3D;
	TD3DRef<IDirect3DVolumeTexture9> Texture3D;
	
	/** default constructor */
	FSurfaceRHIRef()
		:	ResolveTargetTexture2D(NULL)
		,	ResolveTargetTextureCube(NULL)
		,	ResolveTargetTexture3D(NULL)
		,	Texture2D(NULL)
		,	TextureCube(NULL)
		,	Texture3D(NULL)
	{
	}

	/** Initialization constructor. Using 2d resolve texture */
	FSurfaceRHIRef(
		FTexture2DRHIRef InResolveTargetTexture,
		IDirect3DTexture9* InTexture = NULL,
		IDirect3DSurface9* InSurface = NULL,
		UBOOL bInSRGB = FALSE
		)
		:	ResolveTargetTexture2D(InResolveTargetTexture)		
		,	Texture2D(InTexture)
		,	TD3DRef<IDirect3DSurface9>(InSurface)
		,	ResolveTargetTextureCube(NULL)
		,	TextureCube(NULL)
		,	Texture3D(NULL)
		,	ResolveTargetTexture3D(NULL)
	{}

	/** Initialization constructor. Using cube resolve texture */
	FSurfaceRHIRef(
		FTextureCubeRHIRef InResolveTargetTexture,
		IDirect3DCubeTexture9* InTexture = NULL,
		IDirect3DSurface9* InSurface = NULL,
		UBOOL bInSRGB = FALSE
		)
		:	ResolveTargetTextureCube(InResolveTargetTexture)
		,	TextureCube(InTexture)
		,	TD3DRef<IDirect3DSurface9>(InSurface)
		,	ResolveTargetTexture2D(NULL)
		,	Texture2D(NULL)
		,	Texture3D(NULL)
		,	ResolveTargetTexture3D(NULL)
	{}
	
	//  [2006/11/20 YTS, temporarily inserted]
	FSurfaceRHIRef(
		FTexture3DRHIRef InResolveTargetTexture,
		IDirect3DVolumeTexture9* InTexture = NULL,
		IDirect3DVolume9* InVolume = NULL,
		UBOOL bInSRGB = FALSE 
		)
		:	ResolveTargetTexture3D(InResolveTargetTexture)
		,	Texture3D(InTexture)
		// It can't be initialized. the base class of this class is 'TD3DRef<IDrect3DVolume9>.
		//,	TD3DRef<IDirect3DVolume9>(InVolume)
		,	TD3DRef<IDirect3DSurface9>(NULL)
		,	ResolveTargetTexture2D(NULL)
		,	ResolveTargetTextureCube(NULL)
		,	Texture2D(NULL)
		,	TextureCube(NULL)
	{}

	/** 
	* Initialization constructor. Using cube resolve texture 
	* Needed when using a dedicated 2D texture for rendering 
	* with a cube resolve texture
	*/
	FSurfaceRHIRef(
		FTextureCubeRHIRef InResolveTargetTexture,
		IDirect3DTexture9* InTexture = NULL,
		IDirect3DSurface9* InSurface = NULL
		)
		:	ResolveTargetTextureCube(InResolveTargetTexture)
		,	Texture2D(InTexture)
		,	TD3DRef<IDirect3DSurface9>(InSurface)
		,	ResolveTargetTexture2D(NULL)
		,	TextureCube(NULL)
		,	Texture3D(NULL)
		,	ResolveTargetTexture3D(NULL)
	{}

	/** Resets the reference. */
	void Release()
	{
		TD3DRef<IDirect3DSurface9>::Release();
		ResolveTargetTexture2D.Release();
		ResolveTargetTextureCube.Release();
		ResolveTargetTexture3D.Release();
		Texture2D.Release();
		TextureCube.Release();
		Texture3D.Release();
	}
};

typedef const FSurfaceRHIRef& FSurfaceRHIParamRef;

#endif
