/*=============================================================================
ScalabilityOptions.cpp: Unreal engine HW compat scalability system.
Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
Globals.
-----------------------------------------------------------------------------*/

/** Global scalability object. */
USystemSettings* GSystemSettings = NULL;

IMPLEMENT_CLASS( USystemSettings );

///*-----------------------------------------------------------------------------
//FSystemSettings
//-----------------------------------------------------------------------------*/
//
///** Constructor, initializing all member variables. */
//FSystemSettings::FSystemSettings()
//// Scalability options
//:	bAllowStaticDecals				( TRUE )
//,	bAllowDynamicDecals				( TRUE )
//,	bAllowDynamicLights				( TRUE )
//,	bAllowDynamicShadows			( TRUE )
//,	bAllowLightEnvironmentShadows	( TRUE )
//,	bUseCompositeDynamicLights		( FALSE )
//,	bAllowMotionBlur				( TRUE )
//,	bAllowDepthOfField				( TRUE )
//,	bAllowBloom						( TRUE )
//,	bUseHighQualityBloom			( TRUE )
//,	bAllowSpeedTreeLeaves			( TRUE )
//,	bAllowSpeedTreeFronds			( TRUE )
//,	bOnlyStreamInTextures			( FALSE )
//,	SkeletalMeshLODBias				( 0 )
//,	ParticleLODBias					( 0 )
//,	ScreenPercentage				( 100.f )
//,	bAllowFog						( TRUE )
//// System configuration
//,	CPUMemory						( 0 )
//,	CPUFrequency					( 0 )
//,	CPUCount						( 0 )
//,	CPUVendorString					( TEXT("Unknown") )
//,	GPUShaderModel					( 0 )
//,	GPUMemory						( 0 )
//,	GPUVendorId						( INDEX_NONE )
//,	GPUDeviceId						( INDEX_NONE )
//,	Anisotropy						( 4 )
//,	Antialiasing					( 0 )
//,	bUpscaleScreenPercentage		( TRUE )
//{}
//
///**
//* Initializes system settings and included texture LOD settings.
//*
//* @param bSetupForEditor	Whether to initialize settings for Editor
//*
//*/
//void FSystemSettings::Initialize( UBOOL bSetupForEditor )
//{
//	// Always stream out textures in the Editor. We don't use GIsEditor as it's not set at this point.
//	bOnlyStreamInTextures = bOnlyStreamInTextures && !bSetupForEditor;
//
//	// Initialize texture LOD settings before any are loaded from disk.
//	TextureLODSettings.Initialize( GEngineIni, TEXT("TextureLODSettings") );
//
//	for (INT PresetIndex=0; PresetIndex<ARRAY_COUNT(Presets); ++PresetIndex)
//	{
//		Presets[PresetIndex].Initialize( GEngineIni, *FString::Printf( TEXT("TextureLODPreset%d"), PresetIndex ) );
//	}	
//}
//
//
///**
//* Exec handler implementation.
//*
//* @param Cmd	Command to parse
//* @param Ar	Output device to log to
//*
//* @return TRUE if command was handled, FALSE otherwise
//*/
//UBOOL FSystemSettings::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
//{
//	struct { const TCHAR* Name; UBOOL* SwitchPtr; } Switches[] =
//	{
//		{ TEXT("STATICDECALS")				, &bAllowStaticDecals },
//		{ TEXT("DYNAMICDECALS")				, &bAllowDynamicDecals },
//		{ TEXT("DYNAMICLIGHTS")				, &bAllowDynamicLights },
//		{ TEXT("DYNAMICSHADOWS")			, &bAllowDynamicShadows },
//		{ TEXT("LIGHTENVIRONMENTSHADOWS")	, &bAllowLightEnvironmentShadows },
//		{ TEXT("MOTIONBLUR")				, &bAllowMotionBlur },
//		{ TEXT("DEPTHOFFIELD")				, &bAllowDepthOfField },
//		{ TEXT("BLOOM")						, &bAllowBloom },
//		{ TEXT("QUALITYBLOOM")				, &bUseHighQualityBloom },
//		{ TEXT("FOG")						, &bAllowFog },
//		{ TEXT("SPEEDTREELEAVES")			, &bAllowSpeedTreeLeaves },
//		{ TEXT("SPEEDTREEFRONDS")			, &bAllowSpeedTreeFronds },
//		{ TEXT("ONLYSTREAMINTEXTURES")		, &bOnlyStreamInTextures },
//		{ TEXT("COMPOSITEDYNAMICLIGHTS")	, &bUseCompositeDynamicLights },
//		{ TEXT("UPSCALESCREENPERCENTAGE")	, &bUpscaleScreenPercentage },			
//	};
//
//	struct { const TCHAR* Name; UBOOL* IntValuePtr; } IntValues[] =
//	{
//		{ TEXT("SKELETALMESHLODBIAS")		, &SkeletalMeshLODBias },
//		{ TEXT("PARTICLELODBIAS")			, &ParticleLODBias },
//	};
//
//	struct { const TCHAR* Name; FLOAT* FloatValuePtr; } FloatValues[] =
//	{
//		{ TEXT("SCREENPERCENTAGE")			, &ScreenPercentage },
//	};
//
//	// Keep track whether the command was handled or not.
//	UBOOL bHandledCommand = FALSE;
//
//	if( ParseCommand(&Cmd,TEXT("SCALE")) )
//	{
//		if( ParseCommand(&Cmd,TEXT("FASTEST")) )
//		{
//			bAllowStaticDecals				= FALSE;
//			bAllowDynamicDecals				= FALSE;
//			bAllowDynamicLights				= FALSE;
//			bAllowDynamicShadows			= FALSE;
//			bAllowLightEnvironmentShadows	= FALSE;
//			bUseCompositeDynamicLights		= TRUE;
//			bAllowMotionBlur				= FALSE;
//			bAllowDepthOfField				= FALSE;
//			bAllowBloom						= FALSE;
//			bAllowFog						= FALSE;
//			bUseHighQualityBloom			= FALSE;
//			bAllowSpeedTreeLeaves			= FALSE;
//			bAllowSpeedTreeFronds			= FALSE;
//			bOnlyStreamInTextures			= /*!CONSOLE && */!GIsEditor;
//			SkeletalMeshLODBias				= INT_MAX;
//			ParticleLODBias					= INT_MAX;
//			ScreenPercentage				= 50.f;			
//			bHandledCommand					= TRUE;
//		}
//		else if( ParseCommand(&Cmd,TEXT("LOWEND")) )
//		{
//			bAllowStaticDecals				= FALSE;
//			bAllowDynamicDecals				= FALSE;
//			bAllowDynamicLights				= FALSE;
//			bAllowDynamicShadows			= FALSE;
//			bAllowLightEnvironmentShadows	= FALSE;
//			bUseCompositeDynamicLights		= TRUE;
//			bAllowMotionBlur				= FALSE;
//			bAllowFog						= FALSE;
//			bAllowDepthOfField				= TRUE;
//			bAllowBloom						= TRUE;
//			bUseHighQualityBloom			= FALSE;
//			bAllowSpeedTreeLeaves			= FALSE;
//			bAllowSpeedTreeFronds			= FALSE;
//			bOnlyStreamInTextures			= /*!CONSOLE && */!GIsEditor;
//			SkeletalMeshLODBias				= 1;
//			ParticleLODBias					= 2;
//			ScreenPercentage				= 75.f;			
//			bHandledCommand					= TRUE;
//		}
//		else if( ParseCommand(&Cmd,TEXT("PRETTIEST")) || ParseCommand(&Cmd,TEXT("HIGHEND")) )
//		{
//			bAllowFog						= TRUE;
//			bAllowStaticDecals				= TRUE;
//			bAllowDynamicDecals				= TRUE;
//			bAllowDynamicLights				= TRUE;
//			bAllowDynamicShadows			= TRUE;
//			bAllowLightEnvironmentShadows	= TRUE;
//			bUseCompositeDynamicLights		= FALSE;
//			bAllowMotionBlur				= TRUE;
//			bAllowDepthOfField				= TRUE;
//			bAllowBloom						= TRUE;
//			bUseHighQualityBloom			= TRUE;
//			bAllowSpeedTreeLeaves			= TRUE;
//			bAllowSpeedTreeFronds			= TRUE;
//			bOnlyStreamInTextures			= /*!CONSOLE && */!GIsEditor;
//			SkeletalMeshLODBias				= 0;
//			ParticleLODBias					= 0;
//			ScreenPercentage				= 100.f;
//			bHandledCommand					= TRUE;
//		}
//		else if( ParseCommand(&Cmd,TEXT("SET")) )
//		{
//			// Search for a specific boolean
//			for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
//			{
//				if( ParseCommand(&Cmd,Switches[SwitchIndex].Name) )
//				{
//					UBOOL bNewValue = ParseCommand(&Cmd,TEXT("TRUE"));
//					*Switches[SwitchIndex].SwitchPtr = bNewValue;
//					bHandledCommand	= TRUE;
//				}
//			}
//
//			// Search for a specific int value.
//			for( INT IntValueIndex=0; IntValueIndex<ARRAY_COUNT(IntValues); IntValueIndex++ )
//			{
//				if( ParseCommand(&Cmd,IntValues[IntValueIndex].Name) )
//				{
//					INT NewValue = appAtoi( Cmd );
//					*IntValues[IntValueIndex].IntValuePtr = NewValue;
//					bHandledCommand	= TRUE;
//				}
//			}
//
//			// Search for a specific float value.
//			for( INT FloatValueIndex=0; FloatValueIndex<ARRAY_COUNT(FloatValues); FloatValueIndex++ )
//			{
//				if( ParseCommand(&Cmd,FloatValues[FloatValueIndex].Name) )
//				{
//					FLOAT NewValue = appAtof( Cmd );
//					*FloatValues[FloatValueIndex].FloatValuePtr = NewValue;
//					bHandledCommand	= TRUE;
//				}
//			}
//		}
//		else if( ParseCommand(&Cmd,TEXT("TOGGLE")) )
//		{
//			// Search for a specific boolean
//			for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
//			{
//				if( ParseCommand(&Cmd,Switches[SwitchIndex].Name) )
//				{
//					*Switches[SwitchIndex].SwitchPtr = !*Switches[SwitchIndex].SwitchPtr;
//					bHandledCommand	= TRUE;
//				}
//			}
//		}
//	}	
//
//	return bHandledCommand;
//}
//
//
///**
//* Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
//*
//* @param X - in/out X screen offset
//* @param Y - in/out Y screen offset
//* @param SizeX - in/out X screen size
//* @param SizeY - in/out Y screen size
//*/
//void FSystemSettings::ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY )
//{
//	// Take screen percentage option into account if percentage != 100.
//	if( GSystemSettings->ScreenPercentage != 100.f )
//	{
//		// Clamp screen percentage to reasonable range.
//		FLOAT ScaleFactor = Clamp( GSystemSettings->ScreenPercentage / 100.f, 0.0f, 1.f );
//
//		INT	OrigX = X;
//		INT OrigY = Y;
//		UINT OrigSizeX = SizeX;
//		UINT OrigSizeY = SizeY;
//
//		// Scale though make sure we're at least covering 1 pixel.
//		SizeX = Max(1,appTrunc(ScaleFactor * OrigSizeX));
//		SizeY = Max(1,appTrunc(ScaleFactor * OrigSizeY));
//
//		// Center scaled view.
//		X = OrigX + (OrigSizeX - SizeX) / 2;
//		Y = OrigY + (OrigSizeY - SizeY) / 2;
//	}
//}
//
///**
//* Reverses the scale and offset done by ScaleScreenCoords() 
//* if the screen percentage is not 100% and upscaling is allowed.
//*
//* @param OriginalX - out X screen offset
//* @param OriginalY - out Y screen offset
//* @param OriginalSizeX - out X screen size
//* @param OriginalSizeY - out Y screen size
//* @param InX - in X screen offset
//* @param InY - in Y screen offset
//* @param InSizeX - in X screen size
//* @param InSizeY - in Y screen size
//*/
//void FSystemSettings::UnScaleScreenCoords( 
//	INT &OriginalX, INT &OriginalY, 
//	UINT &OriginalSizeX, UINT &OriginalSizeY, 
//	FLOAT InX, FLOAT InY, 
//	FLOAT InSizeX, FLOAT InSizeY)
//{
//	if (NeedsUpscale())
//	{
//		FLOAT ScaleFactor = Clamp( GSystemSettings->ScreenPercentage / 100.f, 0.0f, 1.f );
//
//		//undo scaling
//		OriginalSizeX = appTrunc(InSizeX / ScaleFactor);
//		OriginalSizeY = appTrunc(InSizeY / ScaleFactor);
//
//		//undo centering
//		OriginalX = appTrunc(InX - (OriginalSizeX - InSizeX) / 2.0f);
//		OriginalY = appTrunc(InY - (OriginalSizeY - InSizeY) / 2.0f);
//	}
//	else
//	{
//		OriginalSizeX = appTrunc(InSizeX);
//		OriginalSizeY = appTrunc(InSizeY);
//
//		OriginalX = appTrunc(InX);
//		OriginalY = appTrunc(InY);
//	}
//}
//
//void FSystemSettings::ApplyPreset( INT DetailLevel )
//{
//	if (DetailLevel >= 0 && DetailLevel < ARRAY_COUNT(Presets))
//	{
//		TextureLODSettings = Presets[DetailLevel];
//	}
//}

/*-----------------------------------------------------------------------------
USystemSettings
-----------------------------------------------------------------------------*/

/** Constructor, initializing all member variables. */
USystemSettings::USystemSettings()
// Scalability options
:	bAllowStaticDecals				( TRUE )
,	bAllowDynamicDecals				( TRUE )
,	bAllowDynamicLights				( TRUE )
,	bAllowDynamicShadows			( TRUE )
,	bAllowLightEnvironmentShadows	( TRUE )
,	bUseCompositeDynamicLights		( FALSE )
,	bAllowMotionBlur				( TRUE )
,	bAllowDepthOfField				( TRUE )
,	bAllowBloom						( TRUE )
,	bUseHighQualityBloom			( TRUE )
,	bAllowSpeedTreeLeaves			( TRUE )
,	bAllowOneFrameThreadLag			( FALSE )
,	bAllowSpeedTreeFronds			( TRUE )
,	bOnlyStreamInTextures			( FALSE )
,	SkeletalMeshLODBias				( 0 )
,	ParticleLODBias					( 0 )
,	ScreenPercentage				( 100.f )
,	bAllowFog						( TRUE )
// System configuration
,	CPUMemory						( 0 )
,	CPUFrequency					( 0 )
,	CPUCount						( 0 )
,	CPUVendorString					( TEXT("Unknown") )
,	GPUShaderModel					( 0 )
,	GPUMemory						( 0 )
,	GPUVendorId						( INDEX_NONE )
,	GPUDeviceId						( INDEX_NONE )
,	Anisotropy						( 4 )
,	Antialiasing					( 0 )
,	bUpscaleScreenPercentage		( TRUE )
,	DiffuseCubeResolution			( 0 )
{}

/**
* Initializes system settings and included texture LOD settings.
*
* @param bSetupForEditor	Whether to initialize settings for Editor
*
*/
void USystemSettings::Initialize( UBOOL bSetupForEditor )
{
	// Always stream out textures in the Editor. We don't use GIsEditor as it's not set at this point.
	bOnlyStreamInTextures = bOnlyStreamInTextures && !bSetupForEditor;

	// Initialize texture LOD settings before any are loaded from disk.
	TextureLODSettings.Initialize( GEngineIni, TEXT("TextureLODSettings") );

	for (INT PresetIndex=0; PresetIndex<ARRAY_COUNT(Presets); ++PresetIndex)
	{
		Presets[PresetIndex].Initialize( GEngineIni, *FString::Printf( TEXT("TextureLODPreset%d"), PresetIndex ) );
	}	
}


/**
* Exec handler implementation.
*
* @param Cmd	Command to parse
* @param Ar	Output device to log to
*
* @return TRUE if command was handled, FALSE otherwise
*/
UBOOL USystemSettings::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	struct { const TCHAR* Name; INT* SwitchPtr; } Switches[] =
	{
		{ TEXT("STATICDECALS")				, &bAllowStaticDecals },
		{ TEXT("DYNAMICDECALS")				, &bAllowDynamicDecals },
		{ TEXT("DYNAMICLIGHTS")				, &bAllowDynamicLights },
		{ TEXT("DYNAMICSHADOWS")			, &bAllowDynamicShadows },
		{ TEXT("LIGHTENVIRONMENTSHADOWS")	, &bAllowLightEnvironmentShadows },
		{ TEXT("MOTIONBLUR")				, &bAllowMotionBlur },
		{ TEXT("DEPTHOFFIELD")				, &bAllowDepthOfField },
		{ TEXT("BLOOM")						, &bAllowBloom },
		{ TEXT("QUALITYBLOOM")				, &bUseHighQualityBloom },
		{ TEXT("FOG")						, &bAllowFog },
		{ TEXT("SPEEDTREELEAVES")			, &bAllowSpeedTreeLeaves },
		{ TEXT("SPEEDTREEFRONDS")			, &bAllowSpeedTreeFronds },
		{ TEXT("ONEFRAMETHREADLAG")			, &bAllowOneFrameThreadLag },			
		{ TEXT("ONLYSTREAMINTEXTURES")		, &bOnlyStreamInTextures },
		{ TEXT("COMPOSITEDYNAMICLIGHTS")	, &bUseCompositeDynamicLights },
		{ TEXT("UPSCALESCREENPERCENTAGE")	, &bUpscaleScreenPercentage },			
	};

	struct { const TCHAR* Name; INT* IntValuePtr; } IntValues[] =
	{
		{ TEXT("SKELETALMESHLODBIAS")		, &SkeletalMeshLODBias },
		{ TEXT("PARTICLELODBIAS")			, &ParticleLODBias },
	};

	struct { const TCHAR* Name; FLOAT* FloatValuePtr; } FloatValues[] =
	{
		{ TEXT("SCREENPERCENTAGE")			, &ScreenPercentage },
	};

	// Keep track whether the command was handled or not.
	UBOOL bHandledCommand = FALSE;

	if( ParseCommand(&Cmd,TEXT("SCALE")) )
	{
		if( ParseCommand(&Cmd,TEXT("FASTEST")) )
		{
			bAllowStaticDecals				= FALSE;
			bAllowDynamicDecals				= FALSE;
			bAllowDynamicLights				= FALSE;
			bAllowDynamicShadows			= FALSE;
			bAllowLightEnvironmentShadows	= FALSE;
			bUseCompositeDynamicLights		= TRUE;
			bAllowMotionBlur				= FALSE;
			bAllowDepthOfField				= FALSE;
			bAllowBloom						= FALSE;
			bAllowFog						= FALSE;
			bUseHighQualityBloom			= FALSE;
			bAllowSpeedTreeLeaves			= FALSE;
			bAllowSpeedTreeFronds			= FALSE;
			bOnlyStreamInTextures			= FALSE;
			SkeletalMeshLODBias				= INT_MAX;
			ParticleLODBias					= INT_MAX;
			ScreenPercentage				= 50.f;			
			bHandledCommand					= TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("LOWEND")) )
		{
			bAllowStaticDecals				= FALSE;
			bAllowDynamicDecals				= FALSE;
			bAllowDynamicLights				= FALSE;
			bAllowDynamicShadows			= FALSE;
			bAllowLightEnvironmentShadows	= FALSE;
			bUseCompositeDynamicLights		= TRUE;
			bAllowMotionBlur				= FALSE;
			bAllowFog						= FALSE;
			bAllowDepthOfField				= TRUE;
			bAllowBloom						= TRUE;
			bUseHighQualityBloom			= FALSE;
			bAllowSpeedTreeLeaves			= FALSE;
			bAllowSpeedTreeFronds			= FALSE;
			bOnlyStreamInTextures			= FALSE;
			SkeletalMeshLODBias				= 1;
			ParticleLODBias					= 2;
			ScreenPercentage				= 75.f;			
			bHandledCommand					= TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("PRETTIEST")) || ParseCommand(&Cmd,TEXT("HIGHEND")) )
		{
			bAllowFog						= TRUE;
			bAllowStaticDecals				= TRUE;
			bAllowDynamicDecals				= TRUE;
			bAllowDynamicLights				= TRUE;
			bAllowDynamicShadows			= TRUE;
			bAllowLightEnvironmentShadows	= TRUE;
			bUseCompositeDynamicLights		= FALSE;
			bAllowMotionBlur				= TRUE;
			bAllowDepthOfField				= TRUE;
			bAllowBloom						= TRUE;
			bUseHighQualityBloom			= TRUE;
			bAllowSpeedTreeLeaves			= TRUE;
			bAllowSpeedTreeFronds			= TRUE;
			bOnlyStreamInTextures			= FALSE;
			SkeletalMeshLODBias				= 0;
			ParticleLODBias					= 0;
			ScreenPercentage				= 100.f;
			bHandledCommand					= TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("SET")) )
		{
			// Search for a specific boolean
			for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
			{
				if( ParseCommand(&Cmd,Switches[SwitchIndex].Name) )
				{
					UBOOL bNewValue = ParseCommand(&Cmd,TEXT("TRUE"));
					*Switches[SwitchIndex].SwitchPtr = bNewValue;
					bHandledCommand	= TRUE;
				}
			}

			// Search for a specific int value.
			for( INT IntValueIndex=0; IntValueIndex<ARRAY_COUNT(IntValues); IntValueIndex++ )
			{
				if( ParseCommand(&Cmd,IntValues[IntValueIndex].Name) )
				{
					INT NewValue = appAtoi( Cmd );
					*IntValues[IntValueIndex].IntValuePtr = NewValue;
					bHandledCommand	= TRUE;
				}
			}

			// Search for a specific float value.
			for( INT FloatValueIndex=0; FloatValueIndex<ARRAY_COUNT(FloatValues); FloatValueIndex++ )
			{
				if( ParseCommand(&Cmd,FloatValues[FloatValueIndex].Name) )
				{
					FLOAT NewValue = appAtof( Cmd );
					*FloatValues[FloatValueIndex].FloatValuePtr = NewValue;
					bHandledCommand	= TRUE;
				}
			}
		}
		else if( ParseCommand(&Cmd,TEXT("TOGGLE")) )
		{
			// Search for a specific boolean
			for( INT SwitchIndex=0; SwitchIndex<ARRAY_COUNT(Switches); SwitchIndex++ )
			{
				if( ParseCommand(&Cmd,Switches[SwitchIndex].Name) )
				{
					*Switches[SwitchIndex].SwitchPtr = !*Switches[SwitchIndex].SwitchPtr;
					bHandledCommand	= TRUE;
				}
			}
		}
	}	

	return bHandledCommand;
}


/**
* Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
*
* @param X - in/out X screen offset
* @param Y - in/out Y screen offset
* @param SizeX - in/out X screen size
* @param SizeY - in/out Y screen size
*/
void USystemSettings::ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY )
{
	// Take screen percentage option into account if percentage != 100.
	check(GSystemSettings);

	if( GSystemSettings->ScreenPercentage != 100.f )
	{
		// Clamp screen percentage to reasonable range.
		FLOAT ScaleFactor = Clamp( GSystemSettings->ScreenPercentage / 100.f, 0.0f, 1.f );

		INT	OrigX = X;
		INT OrigY = Y;
		UINT OrigSizeX = SizeX;
		UINT OrigSizeY = SizeY;

		// Scale though make sure we're at least covering 1 pixel.
		SizeX = Max(1,appTrunc(ScaleFactor * OrigSizeX));
		SizeY = Max(1,appTrunc(ScaleFactor * OrigSizeY));

		// Center scaled view.
		X = OrigX + (OrigSizeX - SizeX) / 2;
		Y = OrigY + (OrigSizeY - SizeY) / 2;
	}
}

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
void USystemSettings::UnScaleScreenCoords( 
	INT &OriginalX, INT &OriginalY, 
	UINT &OriginalSizeX, UINT &OriginalSizeY, 
	FLOAT InX, FLOAT InY, 
	FLOAT InSizeX, FLOAT InSizeY)
{
	check(GSystemSettings);
	if (NeedsUpscale())
	{
		FLOAT ScaleFactor = Clamp( GSystemSettings->ScreenPercentage / 100.f, 0.0f, 1.f );

		//undo scaling
		OriginalSizeX = appTrunc(InSizeX / ScaleFactor);
		OriginalSizeY = appTrunc(InSizeY / ScaleFactor);

		//undo centering
		OriginalX = appTrunc(InX - (OriginalSizeX - InSizeX) / 2.0f);
		OriginalY = appTrunc(InY - (OriginalSizeY - InSizeY) / 2.0f);
	}
	else
	{
		OriginalSizeX = appTrunc(InSizeX);
		OriginalSizeY = appTrunc(InSizeY);

		OriginalX = appTrunc(InX);
		OriginalY = appTrunc(InY);
	}
}

void USystemSettings::ApplyPreset( INT DetailLevel )
{
	if (DetailLevel >= 0 && DetailLevel < ARRAY_COUNT(Presets))
	{
		TextureLODSettings = Presets[DetailLevel];
	}

	UpdateTextureStreaming();
}

/**
* Recreates texture resources and drops mips.
*
* @return		TRUE if the settings were applied, FALSE if they couldn't be applied immediately.
*/
UBOOL USystemSettings::UpdateTextureStreaming()
{
	if ( GUseSeekFreeLoading )
	{
		debugf(TEXT("Can't change texture detail at run-time when using cooked content."));
		return FALSE;
	}

	if ( GStreamingManager )
	{
		// Make sure textures can be streamed out so that we can unload current mips.
		const UBOOL bOldOnlyStreamInTextures = bOnlyStreamInTextures;
		bOnlyStreamInTextures = FALSE;

		for( TObjectIterator<UTexture2D> It; It; ++It )
		{
			UTexture* Texture = *It;
			if( Texture->LODGroup != TEXTUREGROUP_UI )
			{
				// Update cached LOD bias.
				Texture->CachedCombinedLODBias = TextureLODSettings.CalculateLODBias( Texture );
				// Recreate the texture's resource.
				Texture->UpdateResource();
			}
		}

		// Make sure we iterate over all textures by setting it to high value.
		GStreamingManager->SetNumIterationsForNextFrame( 100 );
		// Update resource streaming with fudged max texture mip count, causing mips to be dropped.
		GStreamingManager->UpdateResourceStreaming( 0 );
		// Block till requests are finished.
		GStreamingManager->BlockTillAllRequestsFinished();

		// Restore streaming out of textures.
		bOnlyStreamInTextures = bOldOnlyStreamInTextures;
	}

	return TRUE;
}


/*-----------------------------------------------------------------------------
The End.
-----------------------------------------------------------------------------*/

