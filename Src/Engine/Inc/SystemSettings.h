/*=============================================================================
ScalabilityOptions.cpp: Unreal engine HW compat scalability system.
Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
Scalability options.
-----------------------------------------------------------------------------*/

//struct FSystemSettings : public FExec
//{
//	/** Constructor, initializing all member variables. */
//	FSystemSettings();
//
//	/** Empty virtual destructor. */
//	virtual ~FSystemSettings()
//	{}
//
//	/**
//	* Initializes system settings and included texture LOD settings.
//	*
//	* @param bSetupForEditor	Whether to initialize settings for Editor
//	*/
//	void Initialize( UBOOL bSetupForEditor );
//
//	/**
//	* Exec handler implementation.
//	*
//	* @param Cmd	Command to parse
//	* @param Ar	Output device to log to
//	*
//	* @return TRUE if command was handled, FALSE otherwise
//	*/
//	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
//
//	/**
//	* Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
//	*
//	* @param X - in/out X screen offset
//	* @param Y - in/out Y screen offset
//	* @param SizeX - in/out X screen size
//	* @param SizeY - in/out Y screen size
//	*/
//	void ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY );
//
//	/**
//	* Reverses the scale and offset done by ScaleScreenCoords() 
//	* if the screen percentage is not 100% and upscaling is allowed.
//	*
//	* @param OriginalX - out X screen offset
//	* @param OriginalY - out Y screen offset
//	* @param OriginalSizeX - out X screen size
//	* @param OriginalSizeY - out Y screen size
//	* @param InX - in X screen offset
//	* @param InY - in Y screen offset
//	* @param InSizeX - in X screen size
//	* @param InSizeY - in Y screen size
//	*/
//	void UnScaleScreenCoords( 
//		INT &OriginalX, INT &OriginalY, 
//		UINT &OriginalSizeX, UINT &OriginalSizeY, 
//		FLOAT InX, FLOAT InY, 
//		FLOAT InSizeX, FLOAT InSizeY);
//
//	void ApplyPreset( INT DetailLevel );
//
//	/**
//	* Misc options.
//	*/
//
//	/** Global texture LOD settings.									*/
//	FTextureLODSettings TextureLODSettings;	
//
//	/** Global texture LOD settings, initialized at startup. */	
//	FTextureLODSettings Presets[3];
//
//	/**
//	* Getters
//	*/
//
//	INT GetPostProcessLevel() { return bAllowBloom ? bUseHighQualityBloom ? 2 : 1 : 0; }
//
//	/** Indicates whether upscaling is needed */
//	UBOOL NeedsUpscale()
//	{
//		return bUpscaleScreenPercentage && ScreenPercentage < 100.0f;
//	}
//
//	/**
//	* Scalability options.
//	*/
//
//	/** Whether to allow static decals.									*/
//	UBOOL	bAllowStaticDecals;
//	/** Whether to allow dynamic decals.								*/
//	UBOOL	bAllowDynamicDecals;
//
//	/** Whether to allow dynamic lights.								*/
//	UBOOL	bAllowDynamicLights;
//	/** Whether to allow dynamic shadows.								*/
//	UBOOL	bAllowDynamicShadows;
//	/** Whether to allow dynamic light environments to cast shadows.	*/
//	UBOOL	bAllowLightEnvironmentShadows;
//	/** Whether to composte dynamic lights into light environments.		*/
//	UBOOL	bUseCompositeDynamicLights;
//
//	/** Whether to allow motion blur.									*/
//	UBOOL	bAllowMotionBlur;
//	/** Whether to allow depth of field.								*/
//	UBOOL	bAllowDepthOfField;
//	/** Whether to allow bloom.											*/
//	UBOOL	bAllowBloom;
//	/** Whether to use high quality bloom or fast versions.				*/
//	UBOOL	bUseHighQualityBloom;
//
//	/** Whether to allow fog.											*/
//	UBOOL	bAllowFog;
//
//	/** Whether to allow rendering of SpeedTree leaves.					*/
//	UBOOL	bAllowSpeedTreeLeaves;
//	/** Whether to allow rendering of SpeedTree fronds.					*/
//	UBOOL	bAllowSpeedTreeFronds;
//
//	/** If enabled, texture will only be streamed in, not out.			*/
//	UBOOL	bOnlyStreamInTextures;
//
//	/** Whether to upscale at the end of the frame when ScreenPercentage < 100.0f.								*/
//	UBOOL	bUpscaleScreenPercentage;
//
//	/** LOD bias for skeletal meshes.									*/
//	INT		SkeletalMeshLODBias;
//	/** LOD bias for particle systems.									*/
//	INT		ParticleLODBias;
//
//	/** Percentage of screen main view should take up.					*/
//	FLOAT	ScreenPercentage;	
//
//	/** Whether to use anisotropic filtering							*/
//	INT		Anisotropy;
//
//	/** Which anti-aliasing mode should be used							*/
//	/** User-friendly name can be retrieved via UBOOL RHIGetUserFriendlyAntialiasingName( INT Mode ); */
//	INT		Antialiasing;
//
//protected:
//	/** Amount of total system memory in MByte.							*/
//	INT		CPUMemory;
//	/** CPU Frequency in MHz											*/
//	INT		CPUFrequency;
//	/** Number of physical CPUs, disregarding hyper-threading.			*/
//	INT		CPUCount;
//	/** CPU vendor string.												*/
//	FString	CPUVendorString;
//
//	/** Supported pixel shader model * 10, so 30 for 3.0				*/
//	INT		GPUShaderModel;
//	/** On-board GPU memory in MByte.									*/
//	INT		GPUMemory;
//	/** GPU vendor id.													*/
//	INT		GPUVendorId;
//	/** GPU device id.													*/
//	INT		GPUDeviceId;
//};
//
///** Global scalability object. */
//extern FSystemSettings GSystemSettings;

class USystemSettings;
extern USystemSettings* GSystemSettings;

/*-----------------------------------------------------------------------------
The End.
-----------------------------------------------------------------------------*/

