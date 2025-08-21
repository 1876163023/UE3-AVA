class SystemSettings extends Object
	native
	transient
	inherits( FExec );

cpptext
{
	/** Constructor, initializing all member variables. */
	USystemSettings();

	///** Empty virtual destructor. */
	//virtual ~FSystemSettings()
	//{}

	/**
	* Initializes system settings and included texture LOD settings.
	*
	* @param bSetupForEditor	Whether to initialize settings for Editor
	*/
	void Initialize( UBOOL bSetupForEditor );

	/**
	* Exec handler implementation.
	*
	* @param Cmd	Command to parse
	* @param Ar	Output device to log to
	*
	* @return TRUE if command was handled, FALSE otherwise
	*/
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

	/**
	* Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
	*
	* @param X - in/out X screen offset
	* @param Y - in/out Y screen offset
	* @param SizeX - in/out X screen size
	* @param SizeY - in/out Y screen size
	*/
	void ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY );

	/**
	* Reverses the scale and offset done by ScaleScreenCoords() 
	* if the screen percentage is not 100% and upscaling is allowed.
	*
	* @param OriginalX - out X screen offset
	* @param OriginalY - out Y screen offset
	* @param OriginalSizeX - out X screen size
	* @param OriginalSizeY - out Y screen size
	* @param InX - in X screen offset
	* @param InY - in Y screen offset
	* @param InSizeX - in X screen size
	* @param InSizeY - in Y screen size
	*/
	void UnScaleScreenCoords( 
		INT &OriginalX, INT &OriginalY, 
		UINT &OriginalSizeX, UINT &OriginalSizeY, 
		FLOAT InX, FLOAT InY, 
		FLOAT InSizeX, FLOAT InSizeY);

	void ApplyPreset( INT DetailLevel );

	/**
	* Getters
	*/

	INT GetPostProcessLevel() { return bAllowBloom ? bUseHighQualityBloom ? 2 : 1 : 0; }

	/** Indicates whether upscaling is needed */
	UBOOL NeedsUpscale()
	{
		return bUpscaleScreenPercentage && ScreenPercentage < 100.0f;
	}

	UBOOL UpdateTextureStreaming();
};


/**
* Misc options.
*/

var bool bUseLoadMapCache; // 20080122 dEAthcURe|FC

/** Global texture LOD settings.									*/
var TextureLODSettings TextureLODSettings;	

/** Global texture LOD settings, initialized at startup. */	
var TextureLODSettings Presets[3];

	/**
	* Scalability options.
	*/

	/** Whether to allow static decals.									*/
var int	bAllowStaticDecals;
	/** Whether to allow dynamic decals.								*/
var int	bAllowDynamicDecals;

	/** Whether to allow dynamic lights.								*/
var int	bAllowDynamicLights;
	/** Whether to allow dynamic shadows.								*/
var int	bAllowDynamicShadows;
	/** Whether to allow dynamic light environments to cast shadows.	*/
var int	bAllowLightEnvironmentShadows;
	/** Whether to composte dynamic lights into light environments.		*/
var int	bUseCompositeDynamicLights;

	/** Whether to allow motion blur.									*/
var int	bAllowMotionBlur;
	/** Whether to allow depth of field.								*/
var int	bAllowDepthOfField;
	/** Whether to allow bloom.											*/
var int	bAllowBloom;
	/** Whether to use high quality bloom or fast versions.				*/
var int	bUseHighQualityBloom;

	/** Whether to allow fog.											*/
var int	bAllowFog;

	/** Whether to allow rendering of SpeedTree leaves.					*/
var int	bAllowSpeedTreeLeaves;
	/** Whether to allow rendering of SpeedTree fronds.					*/
var int	bAllowSpeedTreeFronds;

var int bAllowOneFrameThreadLag;

	/** If enabled, texture will only be streamed in, not out.			*/
var int	bOnlyStreamInTextures;

	/** Whether to upscale at the end of the frame when ScreenPercentage < 100.0f.								*/
var int	bUpscaleScreenPercentage;

	/** LOD bias for skeletal meshes.									*/
var int		SkeletalMeshLODBias;
	/** LOD bias for particle systems.									*/
var int		ParticleLODBias;

	/** Percentage of screen main view should take up.					*/
var	float	ScreenPercentage;	

	/** Whether to use anisotropic filtering							*/
var int		Anisotropy;

	/** Which anti-aliasing mode should be used							*/
	/** User-friendly name can be retrieved via UBOOL RHIGetUserFriendlyAntialiasingName( INT Mode ); */
var int		Antialiasing;

	/** Amount of total system memory in MByte.							*/
var int		CPUMemory;
	/** CPU Frequency in MHz											*/
var int		CPUFrequency;
	/** Number of physical CPUs, disregarding hyper-threading.			*/
var int		CPUCount;
	/** CPU vendor string.												*/
var	init string	CPUVendorString;

	/** Supported pixel shader model * 10, so 30 for 3.0				*/
var int		GPUShaderModel;
	/** On-board GPU memory in MByte.									*/
var int		GPUMemory;
	/** GPU vendor id.													*/
var int		GPUVendorId;
	/** GPU device id.													*/
var int		GPUDeviceId;


/** Migrated from D3DDevice */
var bool	bEnableVSync;
/** Whether to enable framerate smoothing.																		*/
var bool	bSmoothFrameRate;
/** Maximum framerate to smooth. Code will try to not go over via waiting.										*/
var float	MaxSmoothedFrameRate;
/** Minimum framerate smoothing will kick in.																	*/
var float	MinSmoothedFrameRate;

var int		DiffuseCubeResolution;
