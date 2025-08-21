/*=============================================================================
	SceneRenderTargets.h: Scene render target definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * Encapsulates the render targets used for scene rendering.
 */
class FSceneRenderTargets : public FRenderResource
{
public:

	/**
	 * Checks that scene render targets are ready for rendering a view family of the given dimensions.
	 * If the allocated render targets are too small, they are reallocated.
	 */
	void Allocate(UINT MinSizeX,UINT MinSizeY);

	void BeginRenderingFilter();
	void FinishRenderingFilter();

	/**
	 * Sets the scene color target and restores its contents if necessary
	 * @param bRestoreContents - if TRUE then copies contents of SceneColorTexture to the SceneColorSurface
	 */
	void BeginRenderingSceneColor(UBOOL bRestoreContents = FALSE);
	/**
	 * Called when finished rendering to the scene color surface
	 * @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the SceneColorTexture
	 */
	void FinishRenderingSceneColor(UBOOL bKeepChanges = TRUE);

    /**
     * Resolve a previously rendered scene color surface.
     */
    void ResolveSceneColor();

    /**
     * Sets the LDR version of the scene color target.
     */
    void BeginRenderingSceneColorLDR();
    /**
     * Called when finished rendering to the LDR version of the scene color surface.
     * @param bKeepChanges - if TRUE then the SceneColorSurface is resolved to the LDR SceneColorTexture
	 * @param ResolveParams - optional resolve params
     */
    void FinishRenderingSceneColorLDR(UBOOL bKeepChanges = TRUE, const FResolveParams& ResolveParams = FResolveParams());

	/**
	 * Saves a previously rendered scene color surface in the raw bit format.
	 */
	void SaveSceneColorRaw();

	/**
	 * Restores a previously saved raw-scene color surface.
	 */
	void RestoreSceneColorRaw();

    /**
     * Sets the raw version of the scene color target.
     */
    void BeginRenderingSceneColorRaw();

    /** Called to start rendering the depth pre-pass. */
	void BeginRenderingPrePass( UBOOL bRequireVelocity );
	/**
	 * Called when finished rendering the depth pre-pass. During this pass, screen-space velocities
	 * may also have been written, if they are required for later post-process effects.
	 * @param bRequiresVelocities	Whether velocities were written and are required to be saved off
	 *								for later use.
	 */
	void FinishRenderingPrePass(UBOOL bRequiresVelocities);

	void BeginRenderingShadowVolumes();
	void FinishRenderingShadowVolumes();

	void BeginRenderingShadowDepth();
	/**
	* Called when finished rendering to the subject shadow depths so the surface can be copied to texture
	* @param ResolveParams - optional resolve params
	*/
	void FinishRenderingShadowDepth(const FResolveParams& ResolveParams = FResolveParams());

	//<@ ava specific ; 2007. 11. 9 changmin
	// add cascaded shadow
	void AVA_BeginRenderingNearShadowDepth();
	void AVA_FinishRenderingNearShadowDepth(const FResolveParams& ResolveParams = FResolveParams());
	const FTexture2DRHIRef& AVA_GetNearShadowDepthZTexture() const { return RenderTargets[AVA_NearShadowDepthZ].Texture; }
	const FSurfaceRHIRef& AVA_GetNearShadowDepthZSurface() const { return RenderTargets[AVA_NearShadowDepthZ].Surface; }
	//>@ ava

	void BeginRenderingLightAttenuation();
	void FinishRenderingLightAttenuation();

	void BeginRenderingDistortionAccumulation();
	void FinishRenderingDistortionAccumulation();

	void BeginRenderingDistortionDepth();
	void FinishRenderingDistortionDepth();

	void ResolveSceneDepthTexture();

	// FRenderResource interface.
	virtual void InitDynamicRHI();
	virtual void ReleaseDynamicRHI();

	// Texture Accessors.
	const FTexture2DRHIRef& GetFilterColorTexture() const { return RenderTargets[FilterColor].Texture; }
	const FTexture2DRHIRef& GetSceneColorRawTexture() const { return RenderTargets[SceneColorRaw].Texture; }
	const FTexture2DRHIRef& GetSceneColorTexture() const { return RenderTargets[SceneColor].Texture; }
	const FTexture2DRHIRef& GetSceneColorLDRTexture() const { return RenderTargets[LightAttenuation].Texture; }
	const FTexture2DRHIRef& GetSceneDepthTexture() const { return RenderTargets[SceneDepthZ].Texture; }	
	const FTexture2DRHIRef& GetShadowDepthZTexture() const { return RenderTargets[ShadowDepthZ].Texture; }
	const FTexture2DRHIRef& GetShadowDepthColorTexture() const { return RenderTargets[ShadowDepthColor].Texture; }
	const FTexture2DRHIRef& GetLightAttenuationTexture() const { return RenderTargets[LightAttenuation].Texture; }
	const FTexture2DRHIRef& GetVelocityTexture() const { return RenderTargets[VelocityBuffer].Texture; }	
	
	/** 
	* Allows substitution of a 1x1 white texture in place of the light attenuation buffer when it is not needed;
	* this improves shader performance and removes the need for redundant Clears
	*/
	void SetLightAttenuationMode(UBOOL bEnabled) { bLightAttenuationEnabled = bEnabled; }
	const FTextureRHIRef& GetEffectiveLightAttenuationTexture() const 
	{
		if( bLightAttenuationEnabled )
		{
			return *(FTextureRHIRef*)&RenderTargets[LightAttenuation].Texture;
		}
		else
		{
			return GWhiteTexture->TextureRHI;
		}
	}
	
	// Surface Accessors.
	const FSurfaceRHIRef& GetFilterColorSurface() const { return RenderTargets[FilterColor].Surface; }
	const FSurfaceRHIRef& GetSceneColorRawSurface() const { return RenderTargets[SceneColorRaw].Surface; }
	const FSurfaceRHIRef& GetSceneColorSurface() const { return RenderTargets[SceneColor].Surface; }
	const FSurfaceRHIRef& GetSceneColorLDRSurface() const { return RenderTargets[LightAttenuation].Surface; }
	const FSurfaceRHIRef& GetSceneDepthSurface() const { return RenderTargets[SceneDepthZ].Surface; }	
	const FSurfaceRHIRef& GetMSAAFriendlySceneDepthSurface() const { return RenderTargets[MultisampleDepthZ].Surface; }	
	const FSurfaceRHIRef& GetShadowDepthZSurface() const { return RenderTargets[ShadowDepthZ].Surface; }
	const FSurfaceRHIRef& GetShadowDepthColorSurface() const { return RenderTargets[ShadowDepthColor].Surface; }
	const FSurfaceRHIRef& GetLightAttenuationSurface() const { return RenderTargets[LightAttenuation].Surface; }
	const FSurfaceRHIRef& GetVelocitySurface() const { return RenderTargets[VelocityBuffer].Surface; }	

	void OverrideSceneColorSurface( FSurfaceRHIRef );

	UINT GetShadowDepthTextureResolution() const;	

	UINT GetBufferSizeX() const { return BufferSizeX; }
	UINT GetBufferSizeY() const { return BufferSizeY; }

	UINT GetFilterDownsampleFactor() const { return FilterDownsampleFactor; }

	UINT GetFilterBufferSizeX() const { return FilterBufferSizeX; }
	UINT GetFilterBufferSizeY() const { return FilterBufferSizeY; }

#if SUPPORTS_VSM
	void BeginRenderingShadowVariance();
	void FinishRenderingShadowVariance(const FResolveParams& ResolveParams);
	const FTexture2DRHIRef& GetShadowVarianceTexture() const { return RenderTargets[ShadowVariance].Texture; }
	const FSurfaceRHIRef& GetShadowVarianceSurface() const { return RenderTargets[ShadowVariance].Surface; }
	UINT GetShadowVarianceTextureResolution() const;
#endif //#if SUPPORTS_VSM

protected:
	// Constructor.
	FSceneRenderTargets(): BufferSizeX(0), BufferSizeY(0), FilterDownsampleFactor(0), FilterBufferSizeX(0), FilterBufferSizeY(0), bLightAttenuationEnabled(TRUE) {}

private:

	UINT BufferSizeX;
	UINT BufferSizeY;

	UINT FilterDownsampleFactor;

	UINT FilterBufferSizeX;
	UINT FilterBufferSizeY;

	enum ESceneRenderTargetTypes
	{
		// Render target for post process filter colors.
        FilterColor=0,
		// Render target for scene colors.
		SceneColor,
		// Render target for scene colors (resolved as raw-bits).
		SceneColorRaw,
		// Render target for scene depths.
		SceneDepthZ,
		// Render target for shadow depths.
		ShadowDepthZ,
		// Render target for shadow depths as color.
		ShadowDepthColor,
		// Rneder target for shadow variance depths
		ShadowVariance,
		// Render target for light attenuation values.
		LightAttenuation,
		// Render target for motion velocity 2D-vectors.
		VelocityBuffer,		
		// Multisample Depth
		MultisampleDepthZ,

		//<@ ava specific ; 2007. 11. 9 changmin
		AVA_NearShadowDepthZ,
		//>@ ava

		// Max scene RTs available
        MAX_SCENE_RENDERTARGETS
	};

	enum ESceneRenderTargetFlags
	{

	};

	/**
	* Single render target item consists of a render surface and its resolve texture
	*/
	struct FSceneRenderTargetItem
	{
		FSceneRenderTargetItem() 
			: Flags(0) 
		{
		}
		/** texture for resolving to */
		FTexture2DRHIRef Texture;
		/** surface to render to */
		FSurfaceRHIRef Surface;
        /** combination of ESceneRenderTargetFlags */
		DWORD Flags;
	};
	/** static array of all the scene render targets */
	FSceneRenderTargetItem RenderTargets[MAX_SCENE_RENDERTARGETS];	
	/** if TRUE we use the light attenuation buffer otherwise the 1x1 white texture is used */
	UBOOL bLightAttenuationEnabled;

	/** Global context to use for RHI calls */
	FCommandContextRHI* GlobalContext;
};

/** The global render targets used for scene rendering. */
extern TGlobalResource<FSceneRenderTargets> GSceneRenderTargets;

/**
* Proxy render target that wraps an existing render target RHI resource
*/
class FSceneRenderTargetProxy : public FRenderTarget
{
public:
	/**
	* Constructor
	*/
	FSceneRenderTargetProxy();

	/**
	* Set SizeX and SizeY of proxy and re-allocate scene targets as needed
	*
	* @param InSizeX - scene render target width requested
	* @param InSizeY - scene render target height requested
	*/
	void SetSizes(UINT InSizeX,UINT InSizeY);

	// FRenderTarget interface

	/**
	* @return width of the scene render target this proxy will render to
	*/
	virtual UINT GetSizeX() const;

	/**
	* @return height of the scene render target this proxy will render to
	*/
	virtual UINT GetSizeY() const;	

	/**
	* @return gamma this render target should be rendered with
	*/
	virtual FLOAT GetDisplayGamma() const;

	/**
	* @return RHI surface for setting the render target
	*/
	virtual const FSurfaceRHIRef& GetRenderTargetSurface() const;

private:

	/** scene render target width requested */
	UINT SizeX;
	/** scene render target height requested */
	UINT SizeY;
};


