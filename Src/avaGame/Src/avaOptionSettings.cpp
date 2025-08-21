#include "avaGame.h"

#include "SupportedResolution.h"
#include "avaNetClient.h"
#include "avaTransactions.h"
#include "..\Inc\fileCache.h"
#include "..\..\avaLaunch\Base64.h"

struct FGPUDependentSettings
{
	UBOOL bApplyThisSettings;

	INT StartupResolutionX, StartupResolutionY;
	INT CharacterDetail, TextureDetail;
	INT Anisotropy;
	UBOOL bNoFog;
	UBOOL bNoDynamicShadows;
	UBOOL bNoPostProcess;
	UBOOL bNoWorldShadows;

	INT ShaderModel;
};

FGPUDependentSettings GGPUDependentSettings;

/************************************************************************/
/* some static functions                                                */
/************************************************************************/

FORCEINLINE static AavaPlayerController* GetPlayerOwner()
{
	return GEngine && GEngine->GamePlayers.IsValidIndex(0) && GEngine->GamePlayers(0) ? Cast<AavaPlayerController>(GEngine->GamePlayers(0)->Actor) : NULL;
}

/************************************************************************/
/* OptionSettings	                                                    */
/************************************************************************/

UavaOptionSettings* UavaOptionSettings::GetDefaultObject()
{
	return UavaOptionSettings::GetDefaultOptionSettings();
}

void UavaOptionSettings::StaticInit()
{
	static UBOOL bInitialized = FALSE;
	static UBOOL bViewportInitialized = FALSE;
	static UBOOL bAudioInitialized = FALSE;
	static UBOOL bGameInitialized = FALSE;
	static UBOOL bPlayerInputInitialized = FALSE;
	UGameEngine* GameEngine = NULL;
	UAudioDevice* AudioDevice = NULL;

	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	if( !bViewportInitialized && (GameEngine = Cast<UGameEngine>(GEngine)) != NULL &&
		GEngine->Client && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame )
	{
		OptionSettings->SetDisplayGamma( OptionSettings->DisplayGamma );
		OptionSettings->SetResolution( FVector2D(OptionSettings->StartUpResX, OptionSettings->StartUpResY) );
		// OptionSettings.CurrentAspectRatio값을 얻어옴
		OptionSettings->GetAspectRatio();
		bViewportInitialized = TRUE;
	}

	if( !bInitialized && GSystemSettings != NULL )
	{
		AavaPawn* Pawn = AavaPawn::StaticClass()->GetDefaultObject<AavaPawn>();
		UPlayerInput* PlayerInput = UPlayerInput::StaticClass()->GetDefaultObject<UPlayerInput>();
		check( Pawn != NULL && PlayerInput );

		debugf(TEXT("OptionSettings->StartUpResX = %f"), OptionSettings->StartUpResX);
		debugf(TEXT("OptionSettings->StartUpResY = %f"), OptionSettings->StartUpResY);
		debugf(TEXT("OptionSettings->bDisablePostProcess = %d"), OptionSettings->bDisablePostProcess);
		debugf(TEXT("OptionSettings->bDisableDynamicShadows = %d"), OptionSettings->bDisableDynamicShadows);
		debugf(TEXT("OptionSettings->bDisableFog = %d"), OptionSettings->bDisableFog);
		debugf(TEXT("OptionSettings->CharacterDetail = %d"), OptionSettings->CharacterDetail);
		debugf(TEXT("OptionSettings->TextureDetail = %d"), OptionSettings->TextureDetail);
		debugf(TEXT("OptionSettings->Anisotropy = %d"), OptionSettings->Anisotropy);

		/* GPU default setting 적용 */
		if (GGPUDependentSettings.bApplyThisSettings)
		{
			OptionSettings->StartUpResX = GGPUDependentSettings.StartupResolutionX;
			OptionSettings->StartUpResY = GGPUDependentSettings.StartupResolutionY;
			OptionSettings->bDisablePostProcess = GGPUDependentSettings.bNoPostProcess;
			OptionSettings->bDisableDynamicShadows = GGPUDependentSettings.bNoDynamicShadows;
			OptionSettings->bDisableFog = GGPUDependentSettings.bNoFog;
			OptionSettings->CharacterDetail = GGPUDependentSettings.CharacterDetail;
			OptionSettings->TextureDetail = GGPUDependentSettings.TextureDetail;
			OptionSettings->Anisotropy = GGPUDependentSettings.Anisotropy;
			OptionSettings->ShaderModel = GGPUDependentSettings.ShaderModel;
		}

		//<@ changmin 2007. 5. 21
		// 초기 설정된 값은 GPUSetting이나, Hardware가 지원여부에 따라 변경될 수 있다.
		// 변경된 최종값을 가지고, Option값을 재설정 및 저장합니다.

		if( !IsAvailShaderPlatform( GetIntShaderPlatform(OptionSettings->ShaderModel) ) )
			OptionSettings->ShaderModel = GetExtShaderNum( SP_PCD3D_SM2_POOR );
		//>@

		GSystemSettings->bAllowFog = !OptionSettings->bDisableFog;
		GSystemSettings->bAllowBloom = GSystemSettings->bAllowDepthOfField = !OptionSettings->bDisablePostProcess;
		GSystemSettings->bAllowDynamicShadows = !OptionSettings->bDisableDynamicShadows;		
		GSystemSettings->bAllowDynamicLights = !OptionSettings->bDisableDynamicLights;
		GSystemSettings->bUseHighQualityBloom = OptionSettings->bUseHighQualityBloom;
		GSystemSettings->Anisotropy = OptionSettings->Anisotropy;
		GSystemSettings->bUseLoadMapCache = OptionSettings->bUseLoadMapCache;
		GSystemSettings->bAllowOneFrameThreadLag = OptionSettings->bAllowOneFrameThreadLag;
		
		// Antialiasing Mode 변환
		GSystemSettings->Antialiasing = OptionSettings->Antialiasing;
		GSystemSettings->bUseCompositeDynamicLights = !OptionSettings->bDisableCompositeDynamicLights;

		GSystemSettings->bEnableVSync = OptionSettings->bUseVSync;
		GSystemSettings->MaxSmoothedFrameRate = 100;
		// VSync가 켜져 있을 때만 smooth framerate 작동하도록 수정
		GSystemSettings->bSmoothFrameRate = OptionSettings->bUseSmoothFrameRate;

		/* TextureDetail */
		GSystemSettings->ApplyPreset( Clamp(OptionSettings->TextureDetail,0,2) );

		/* CharacterDetail */
		OptionSettings->SetCharacterDetail( OptionSettings->CharacterDetail );
		Pawn->SaveConfig();

		if( IsSM2Platform( GRHIShaderPlatform ) || GGPUDependentSettings.bNoWorldShadows )
			OptionSettings->bUseWorldShadow = FALSE;

		OptionSettings->SaveConfig();
		bInitialized = TRUE;
	}

	// AudioDevice를 사용하지 않을 수도 있다. 따라서 따로 초기화
	if( ! bAudioInitialized && GEngine && GEngine->Client && (AudioDevice = GEngine->Client->GetAudioDevice()) != NULL )
	{
		// 오디오 초기화
		AudioDevice->SetGroupVolume(FName(TEXT("Game")),Clamp(OptionSettings->GameVolume,0.f,2.f));
		AudioDevice->SetGroupVolume(FName(TEXT("Music")),Clamp(OptionSettings->MusicVolume,0.f,2.f));
		AudioDevice->SetGroupVolume(FName(TEXT("System")),Clamp(OptionSettings->SystemVolume,0.f,2.f));

		// 무슨 연유에서인지 DefaultOptionSettings.ini의 AudioChannel=16이 작동하지 않네. -_-;
		if( OptionSettings->AudioChannel == 0 )
		{
			OptionSettings->AudioChannel = 32;
		}

		if( AudioDevice->GetMaxChannels() != OptionSettings->AudioChannel )
		{
			AudioDevice->SetMaxChannels(OptionSettings->AudioChannel);
			AudioDevice->SaveConfig();
		}
		bAudioInitialized = TRUE;
	}

	UPlayerInput* PlayerInput = NULL;
	AavaPlayerController* PlayerOwner = NULL;
	if( ! bPlayerInputInitialized && (PlayerOwner = GetPlayerOwner()) != NULL &&
		(PlayerInput = PlayerOwner->PlayerInput) != NULL )
	{
		FLOAT CurrentMouseSensitivity = OptionSettings->MouseSensitivity;
		CurrentMouseSensitivity = CurrentMouseSensitivity == 0.f ? 27.f : CurrentMouseSensitivity;
		OptionSettings->SetMouseSensitivity( CurrentMouseSensitivity );
		OptionSettings->SetInvertMouse( OptionSettings->bInvertMouse );
		OptionSettings->SetMouseSmoothing( !OptionSettings->bDisableMouseSmoothing );
		PlayerInput->SaveConfig();

		bPlayerInputInitialized = TRUE;
	}

	AavaGameReplicationInfo* GRI = NULL;
	if( ! bGameInitialized && GWorld && GWorld->GetWorldInfo() 
		&& (GRI = Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI)) != NULL )
	{
		OptionSettings->SetUseLocalSound( OptionSettings->UseLocalSound );
		GRI->SaveConfig();
		bGameInitialized = TRUE;
	}
}

void UavaOptionSettings::StaticSaveConfig()
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	OptionSettings->SaveConfig();

	// @deprecated Engine.TextureDetail
	//INT TextureDetail = -1;
	//GConfig->GetInt( TEXT("Engine.Engine"), TEXT("TextureDetail"), TextureDetail, GEngineIni );
	//if( TextureDetail != OptionSettings->TextureDetail)
	//{
	//	GConfig->SetInt( TEXT("Engine.Engine"), TEXT("TextureDetail"), OptionSettings->TextureDetail, GEngineIni );
	//	GConfig->Flush(FALSE);
	//}
}

/** Common */
void UavaOptionSettings::RecreateDevice( UBOOL bRecreateWhenChanged /*=FALSE*/ )
{
	static INT RecreateDeviceCount = 0;

	extern UBOOL GPendingRecreateDevice;
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( bRecreateWhenChanged )
	{
		if( RecreateDeviceCount > 0 )
		{
			GPendingRecreateDevice = TRUE;
			RecreateDeviceCount = 0;
		}
	}
	else
	{
		GPendingRecreateDevice = TRUE;
		RecreateDeviceCount++;
	}
}

void UavaOptionSettings::MiscIterativeUpdate( UBOOL bUpdateTextureDetail, UBOOL bUpdateCharacterDetail )
{
	if( bUpdateTextureDetail )
	{
		for (TObjectIterator<UTexture> It; It; ++It)
		{
			UTexture* Texture = *It;
			if (Texture->LODGroup != TEXTUREGROUP_UI)
			{
				Texture->CachedCombinedLODBias = GSystemSettings->TextureLODSettings.CalculateLODBias( Texture );
				Texture->UpdateResource();
			}
		}
	}
	
	if( bUpdateCharacterDetail )
	{
		UavaOptionSettings *OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		for( TObjectIterator<AavaPawn> It ; It ; ++It )
		{
			AavaPawn* P = *It;
			P->LODBias = OptionSettings->CharacterDetail;
		}
	}
}

/** Channel List */
FAccountChannelInfo* UavaOptionSettings::FindMyAccountChannelInfo()
{
	for( INT i = 0 ; i < LastChannelGroupData.Num() ; i++ )
		if( LastChannelGroupData(i).AccountID == GetAvaNetHandler()->GetMyAccountID() )
			return &LastChannelGroupData(i);

	return NULL;
}

void UavaOptionSettings::SetLastChannelGroup( INT NewChannelGroup, UBOOL bSaveConfig /*=FALSE*/ )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	INT LastChannelGroup = INDEX_NONE;
	FAccountChannelInfo* AccountChannelInfo = FindMyAccountChannelInfo();
	if( AccountChannelInfo != NULL )
		LastChannelGroup = AccountChannelInfo->LastChannelGroup;
	else
	{
		static const INT MAX_CHANNELGROUP_COUNT = 5;
		if( LastChannelGroupData.Num() >= MAX_CHANNELGROUP_COUNT )
			LastChannelGroupData.Remove(0, (LastChannelGroupData.Num() - MAX_CHANNELGROUP_COUNT) + 1);

		AccountChannelInfo = new(LastChannelGroupData) FAccountChannelInfo( GetAvaNetHandler()->GetMyAccountID(), EChannelFlag_Practice);
	}

	if( LastChannelGroup != NewChannelGroup )
	{
		AccountChannelInfo->LastChannelGroup = NewChannelGroup;
		if( bSaveConfig )
			SaveConfig();

		bLastChannelGroupChanged = TRUE;
	}
}

INT UavaOptionSettings::GetLastChannelGroup()
{
	INT LastChannelGroup = INDEX_NONE;

	FAccountChannelInfo* AccountChannelInfo = FindMyAccountChannelInfo();
	if( AccountChannelInfo != NULL )
		LastChannelGroup = AccountChannelInfo->LastChannelGroup;
	else
		LastChannelGroup = GetAvaNetHandler()->GetMyBestChannelFlag();

	return LastChannelGroup;
}

void UavaOptionSettings::FlushChanged()
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	if( OptionSettings->bLastChannelGroupChanged )
	{
		OptionSettings->SaveConfig();
		OptionSettings->bLastChannelGroupChanged = FALSE;
	}
}


/** Video Settings */

void UavaOptionSettings::SetConfirmedResolution( FVector2D NewRes )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();

	NewRes.X = Max<INT>( NewRes.X, 800 );
	NewRes.Y = Max<INT>( NewRes.Y, 600 );

	OptionSettings->ConfirmedResolution = NewRes;
}

FVector2D UavaOptionSettings::GetConfirmedResolution( UBOOL bFlushOut /*=TRUE*/ )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	FVector2D ResultRes = OptionSettings->ConfirmedResolution;

	if( bFlushOut )
		OptionSettings->ConfirmedResolution = FVector2D(0,0);

	return ResultRes;
}

FVector2D UavaOptionSettings::GetResolution()
{
	FVector2D Resolution;
	if( GEngine->Client )
	{
		Resolution.X = GEngine->Client->StartupResolutionX;
		Resolution.Y = GEngine->Client->StartupResolutionY;
	}
	return Resolution;
}

UBOOL UavaOptionSettings::SetResolution( FVector2D NewRes, UBOOL bDisableWarning )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	FVector2D OldRes = GetResolution();

	NewRes.X = Max<INT>( NewRes.X, 800 );
	NewRes.Y = Max<INT>( NewRes.Y, 600 );
	
	if( GEngine->Client &&
		(NewRes.X != GEngine->Client->StartupResolutionX || NewRes.Y != GEngine->Client->StartupResolutionY ) )
	{
		SaveObjectForTransaction( GEngine->Client );
		SaveObjectForTransaction( OptionSettings );
		OptionSettings->StartUpResX = GEngine->Client->StartupResolutionX = NewRes.X;
		OptionSettings->StartUpResY = GEngine->Client->StartupResolutionY = NewRes.Y;

		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		if( GameEngine )
		{
			UBOOL bImmediately = FALSE;
			GameEngine->GViewportResizer.Resize( NewRes.X, NewRes.Y , TRUE, bImmediately);
			// @TODO : bImmediately action here. ex) PopupMessage("NewResolution applied immediately.");

			if( !bDisableWarning )
			{
				FString NewOldResStr = FString::Printf(TEXT("%d*%d|%d*%d"), appTrunc(NewRes.X), appTrunc(NewRes.Y), appTrunc(OldRes.X), appTrunc(OldRes.Y));
				GetAvaNetHandler()->ProcMessage( EMsg_Option, EMsg_Option_SettingChanged, TEXT("Resolution"), NewOldResStr, (INT)bImmediately, 0 );
			}
		}
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::SetResolutionStr( const FString& NewResolution, UBOOL bDisableWarning )
{
	TArray<FString> ParsedResStr;
	NewResolution.ParseIntoArrayWS(&ParsedResStr, TEXT("*"));

	return ParsedResStr.Num() >= 2 ? SetResolution( FVector2D( appAtof(*ParsedResStr(0)), appAtof(*ParsedResStr(1))), bDisableWarning ) : FALSE;
}


FVector2D UavaOptionSettings::GetAspectRatio()
{
	FVector2D AspectRatio;
	FIntPoint Resolution = GetResolution().IntPoint();
	if( Resolution.X > 0 && Resolution.Y > 0 )
	{
		INT GCD = FScreenResolution::GCD(Resolution.X, Resolution.Y);
		AspectRatio.X = Resolution.X / GCD;
		AspectRatio.Y = Resolution.Y / GCD;
		if( AspectRatio.X == 8 )
		{
			AspectRatio.X *= 2;
			AspectRatio.Y *= 2;
		}
		GetDefaultOptionSettings()->CurrentAspectRatio = AspectRatio;
	}
	return AspectRatio;
}

/** AspectRatio 값은 임시로 사용하며 Config(ini파일)와 관련이 없다. */
UBOOL UavaOptionSettings::SetAspectRatio( FVector2D NewAspectRatio )
{
	if( GetDefaultOptionSettings()->CurrentAspectRatio != NewAspectRatio )
	{
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		CurrentAspectRatio = NewAspectRatio;
		return TRUE;
	}

	return FALSE;
}

FLOAT UavaOptionSettings::GetDisplayGamma()
{
	return GEngine->Client ? GEngine->Client->DisplayGamma : 0.f;
}

UBOOL UavaOptionSettings::SetDisplayGamma( FLOAT NewGamma )
{
	NewGamma = Clamp(NewGamma, 1.6f, 3.f);

	if( GetDisplayGamma() != NewGamma )
	{
		SaveObjectForTransaction( GEngine->Client );
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		GEngine->Client->DisplayGamma = NewGamma;
		GetDefaultOptionSettings()->DisplayGamma = NewGamma;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetFog()
{
	return GSystemSettings->bAllowFog;
}

UBOOL UavaOptionSettings::SetFog( UBOOL bAllowFog )
{
	if( GetFog() != bAllowFog )
	{
		SaveObjectForTransaction( GSystemSettings );
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		GSystemSettings->bAllowFog = bAllowFog;
		GetDefaultOptionSettings()->bDisableFog = (!bAllowFog);
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetPostProcess()
{
	return GSystemSettings->bAllowBloom/* && GSystemSettings->bUseHighQualityBloom */;
}

UBOOL UavaOptionSettings::SetPostProcess( UBOOL bUsePostProcess )
{
	bUsePostProcess = bUsePostProcess ? TRUE : FALSE;
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( bUsePostProcess != GetPostProcess() )
	{
		SaveObjectForTransaction( GSystemSettings );
		SaveObjectForTransaction( OptionSettings );
		GSystemSettings->bUseHighQualityBloom = bUsePostProcess;
		GSystemSettings->bAllowDepthOfField = GSystemSettings->bAllowBloom = bUsePostProcess;
		OptionSettings->bUseHighQualityBloom = bUsePostProcess;
		OptionSettings->bDisablePostProcess = !bUsePostProcess;

		RecreateDevice();
		return TRUE;
	}
	return FALSE;
}

void UavaOptionSettings::GetShadowDetailList(TArray<INT>& DetailList)
{
	DetailList.AddItem(0);
	DetailList.AddItem(1);
	if( !(IsSM2Platform( GRHIShaderPlatform ) || GGPUDependentSettings.bNoWorldShadows) )
		DetailList.AddItem(2);
}

INT UavaOptionSettings::GetShadowDetail()
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();

	UBOOL DynamicShadow = GetDynamicShadows();
	UBOOL WorldShadow = GetWorldShadow();

	return !DynamicShadow ? 0 :
			!WorldShadow ? 1 : 2;
}

UBOOL UavaOptionSettings::SetShadowDetail( INT DetailLevel )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	
	if( GetShadowDetail() != DetailLevel )
	{
		SetDynamicShadow( DetailLevel > 0 );
		SetWorldShadow( DetailLevel > 1 );
		return TRUE;
	}

	return FALSE;
}

UBOOL UavaOptionSettings::GetDynamicShadows()
{
	return GSystemSettings->bAllowDynamicShadows;
}

UBOOL UavaOptionSettings::SetDynamicShadow(UBOOL bSetDynamicShadows)
{
	bSetDynamicShadows = bSetDynamicShadows ? TRUE : FALSE;
	if( GetDynamicShadows() != bSetDynamicShadows )
	{
		UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		SaveObjectForTransaction( GSystemSettings );
		SaveObjectForTransaction( OptionSettings );
		GSystemSettings->bAllowDynamicShadows = bSetDynamicShadows;
		OptionSettings->bDisableDynamicShadows = (!bSetDynamicShadows);
		RecreateDevice();
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetWorldShadow()
{
	return GetDefaultOptionSettings()->bUseWorldShadow;
}

UBOOL UavaOptionSettings::SetWorldShadow( UBOOL bSet )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();

	if( OptionSettings->bUseWorldShadow != bSet )
	{
		SaveObjectForTransaction( OptionSettings );
		OptionSettings->bUseWorldShadow = bSet;
		extern UBOOL GUseCascadedShadow;
		if( bSet != GUseCascadedShadow )
			GetAvaNetHandler()->ProcMessage( EMsg_Option, EMsg_Option_SettingChanged, TEXT("WorldShadow"), TEXT(""), 0,0 );
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetImpactParticle()
{
	return ! GetDefaultOptionSettings()->bDisableImpactParticle;
}

UBOOL UavaOptionSettings::SetImpactParticle( UBOOL bSetImpactParticle )
{
	if( GetImpactParticle() != bSetImpactParticle )
	{
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		GetDefaultOptionSettings()->bDisableImpactParticle = (!bSetImpactParticle);
		return TRUE;
	}
	return FALSE;
}

INT UavaOptionSettings::GetDynamicLightLevel() 
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	return (!OptionSettings->bDisableDynamicLights && !OptionSettings->bDisableCompositeDynamicLights) ? 1 :
		(!OptionSettings->bDisableDynamicLights && OptionSettings->bDisableCompositeDynamicLights) ? 2 : 0;
}

UBOOL UavaOptionSettings::SetDynamicLightLevel( INT Level )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( GetDynamicLightLevel() != Level )
	{
		SaveObjectForTransaction( OptionSettings );
		SaveObjectForTransaction( GSystemSettings );

		switch( Level )
		{
		case 0:	OptionSettings->bDisableDynamicLights = TRUE; OptionSettings->bDisableCompositeDynamicLights = TRUE;	break;
		case 1: OptionSettings->bDisableDynamicLights = FALSE; OptionSettings->bDisableCompositeDynamicLights = FALSE; break;
		case 2: OptionSettings->bDisableDynamicLights = FALSE; OptionSettings->bDisableCompositeDynamicLights = TRUE; break;
		default: break;
		}

		GSystemSettings->bAllowDynamicLights = !OptionSettings->bDisableDynamicLights;
		GSystemSettings->bUseCompositeDynamicLights = !OptionSettings->bDisableCompositeDynamicLights;
		return TRUE;
	}
	return FALSE;
}

INT UavaOptionSettings::GetStableFrameMode()
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();

	return !OptionSettings->bUseVSync ? 0 :
		!OptionSettings->bUseSmoothFrameRate ? 1 : 2;
}

// @TODO : StableFrame 모드 플레그를 GSystemSettings로 이전할것
UBOOL UavaOptionSettings::SetStableFrameMode( INT Level )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();

	if( GetStableFrameMode() != Level )
	{
		SaveObjectForTransaction( OptionSettings );
		SaveObjectForTransaction( GSystemSettings );

		OptionSettings->bUseVSync = Level > 0;
		OptionSettings->bUseSmoothFrameRate = Level > 1;

		GSystemSettings->bSmoothFrameRate = OptionSettings->bUseSmoothFrameRate;

		if( GSystemSettings->bEnableVSync != OptionSettings->bUseVSync )
		{
			RecreateDevice();
			GSystemSettings->bEnableVSync = OptionSettings->bUseVSync;
		}
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetVSync()
{
	return GSystemSettings->bEnableVSync;
}

UBOOL UavaOptionSettings::SetVSync( UBOOL bSet )
{
	if( GetVSync() != bSet )
	{
		UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
		SaveObjectForTransaction(GSystemSettings);
		SaveObjectForTransaction(OptionSettings);
		GSystemSettings->bEnableVSync = OptionSettings->bUseVSync = bSet;
		RecreateDevice();
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetSmoothFrameRate()
{
	return GSystemSettings->bSmoothFrameRate;
}

UBOOL UavaOptionSettings::SetSmoothFrameRate( UBOOL bSet )
{
	if( GetSmoothFrameRate() != bSet )
	{
		UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		SaveObjectForTransaction(GSystemSettings);
		SaveObjectForTransaction(OptionSettings);
		GSystemSettings->bSmoothFrameRate = OptionSettings->bUseSmoothFrameRate = bSet;
		return TRUE;
	}

	return FALSE;
}

INT UavaOptionSettings::GetTextureDetail()
{
	return GetDefaultOptionSettings()->TextureDetail;
}

UBOOL UavaOptionSettings::SetTextureDetail(INT DetailLevel)
{
	if( TextureDetail != DetailLevel )
	{
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		SaveObjectForTransaction( GSystemSettings );
		GetDefaultOptionSettings()->TextureDetail = DetailLevel;
		GSystemSettings->ApplyPreset( DetailLevel );

		MiscIterativeUpdate( TRUE, FALSE );

		RecreateDevice();
		return TRUE;
	}
	return FALSE;
}

INT UavaOptionSettings::GetCharacterDetail()
{
	AavaPawn* Pawn = AavaPawn::StaticClass()->GetDefaultObject<AavaPawn>();
	return Pawn ? Pawn->LODBias : 0;
}

UBOOL UavaOptionSettings::SetCharacterDetail(INT DetailLevel)
{
	AavaPawn* Pawn = AavaPawn::StaticClass()->GetDefaultObject<AavaPawn>();

	if( GetCharacterDetail() != DetailLevel )
	{
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		SaveObjectForTransaction( Pawn );

		GetDefaultOptionSettings()->CharacterDetail = DetailLevel;
		Pawn->LODBias = DetailLevel;

		MiscIterativeUpdate( FALSE, TRUE );

		return TRUE;
	}
	return FALSE;
}

void UavaOptionSettings::GetAnisotropyList(TArray<INT>& AnisotropyList)
{
	extern INT GMaxLogAnisotropy;
	for( INT i = 0 ; i <= GMaxLogAnisotropy ; i++)
		AnisotropyList.AddItem(i);
}

INT UavaOptionSettings::GetAnisotropy()
{
	return GSystemSettings->Anisotropy;
}

UBOOL UavaOptionSettings::SetAnisotropy(INT NewAnisotropyLevel)
{
	if( GetAnisotropy() != NewAnisotropyLevel )
	{
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		SaveObjectForTransaction( GSystemSettings );
		GetDefaultOptionSettings()->Anisotropy = NewAnisotropyLevel;
		GSystemSettings->Anisotropy = NewAnisotropyLevel;
		RecreateDevice();
		return TRUE;
	}
	return FALSE;
}

void UavaOptionSettings::GetAntiAliasingList(TArray<INT>& AntiAliasingList)
{
	INT Mode = 0;
	FString AAModeName;
	while( RHIGetUserFriendlyAntialiasingName( Mode , AAModeName) )
	{
		AntiAliasingList.AddItem(Mode);
		Mode++;
	}
}

INT UavaOptionSettings::GetAntiAliasing()
{
	return GSystemSettings->Antialiasing;
}

UBOOL UavaOptionSettings::SetAntiAliasing(INT NewAA)
{
	if( GetAntiAliasing() != NewAA )
	{
		SaveObjectForTransaction( GetDefaultOptionSettings() );
		SaveObjectForTransaction( GSystemSettings );
		GSystemSettings->Antialiasing = NewAA;
		GetDefaultOptionSettings()->Antialiasing = NewAA;
		
		RecreateDevice();

		return TRUE;
	}
	return FALSE;
}

void UavaOptionSettings::GetShaderModelList(TArray<INT>& ShaderModelList)
{
	TArray<INT> Elements;
	for( INT i = 0 ; i < SP_MaxPlatforms ; i++)
		if( IsAvailShaderPlatform( (EShaderPlatform)i ) )
			Elements.AddItem( GetExtShaderNum((EShaderPlatform)i) );

	// SM2P, SM2, SM3 순으로 나오도록 거꾸로 값을 넣는다.
	for( INT i = Elements.Num() - 1 ; i >= 0 ; i-- )
		ShaderModelList.AddItem( Elements(i) );
}

INT UavaOptionSettings::GetShaderModel()
{
	return GetDefaultOptionSettings()->ShaderModel;
}

UBOOL UavaOptionSettings::SetShaderModel(INT NewExtShaderNum)
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( IsAvailShaderPlatform( GetIntShaderPlatform( NewExtShaderNum ) ) &&
		OptionSettings->ShaderModel != NewExtShaderNum )
	{
		SaveObjectForTransaction(OptionSettings);
		OptionSettings->ShaderModel = NewExtShaderNum;

		GetAvaNetHandler()->ProcMessage( EMsg_Option, EMsg_Option_SettingChanged, TEXT("ShaderModel"), TEXT(""), 0,0 );
		return TRUE;
	}
	return FALSE;
}

INT UavaOptionSettings::GetDecalDetail()
{
	return GetDefaultOptionSettings()->DecalDetail;
}

UBOOL UavaOptionSettings::SetDecalDetail(INT NewDelcalDetail)
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( OptionSettings->DecalDetail != NewDelcalDetail )
	{
		SaveObjectForTransaction(OptionSettings);
		OptionSettings->DecalDetail = NewDelcalDetail;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetLODApply()
{
	return !GetDefaultOptionSettings()->bDisableLOD;
}

UBOOL UavaOptionSettings::SetLODApply( UBOOL bSet )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( bSet == OptionSettings->bDisableLOD )
	{
		SaveObjectForTransaction(OptionSettings);
		OptionSettings->bDisableLOD = !bSet;
		return TRUE;
	}
	return FALSE;
}

/** Audio Settings */
INT UavaOptionSettings::GetAudioChannel()
{
	check(GEngine && GEngine->Client);
	UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();

	return AudioDevice ? AudioDevice->GetMaxChannels() : INDEX_NONE; 
}

UBOOL UavaOptionSettings::SetAudioChannel( INT AudioChannelCount )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	UAudioDevice* AudioDevice = GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
	INT AudioChannelToSet = Clamp( AudioChannelCount, 16, 128 );
	if( AudioDevice && AudioChannelToSet != AudioDevice->GetMaxChannels() )
	{
		SaveObjectForTransaction( OptionSettings );
		SaveObjectForTransaction( AudioDevice );
		OptionSettings->AudioChannel = AudioChannelToSet;
		AudioDevice->SetMaxChannels( AudioChannelToSet );
		GetAvaNetHandler()->ProcMessage( EMsg_Option, EMsg_Option_SettingChanged, TEXT("AudioChannel"), TEXT(""), 0, 0 );
		return TRUE;
	}
	return FALSE;
}

FLOAT UavaOptionSettings::GetAudioVolume( FName GroupName )
{
	check(GEngine && GEngine->Client);
	UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();

	return AudioDevice ? AudioDevice->GetGroupVolume( GroupName ) : NULL;
}

UBOOL UavaOptionSettings::SetAudioVolume( FName GroupName, FLOAT NewVolume )
{
	check(GEngine && GEngine->Client);
	UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();

	if( AudioDevice )
	{
		SaveObjectForTransaction( AudioDevice );
		SaveObjectForTransaction( OptionSettings );
		AudioDevice->SetGroupVolume( GroupName, NewVolume );
		if( GroupName == TEXT("Music") )
			OptionSettings->MusicVolume = NewVolume;
		else if ( GroupName == TEXT("System") )
			OptionSettings->SystemVolume = NewVolume;
		else if ( GroupName == TEXT("Game") )
			OptionSettings->GameVolume = NewVolume;
		return TRUE;
	}
	return FALSE;
}

/** Mouse Settings */
FLOAT UavaOptionSettings::GetMouseSensitivity()
{
	AavaPlayerController* PC = GetPlayerOwner();
	UPlayerInput* PlayerInput = PC ? PC->PlayerInput : NULL;

	return PlayerInput ? PlayerInput->MouseSensitivity : 1.f;
}

UBOOL UavaOptionSettings::SetMouseSensitivity( FLOAT NewMouseSensitivity )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	AavaPlayerController* PC = GetPlayerOwner();
	UPlayerInput* PlayerInput = PC ? PC->PlayerInput : NULL;

	OptionSettings->MouseSensitivity = NewMouseSensitivity;
	OptionSettings->MouseSensitivity = Clamp( OptionSettings->MouseSensitivity, 1.f, 100.f );

	if( PlayerInput != NULL && 
		GetMouseSensitivity() != OptionSettings->MouseSensitivity )
	{
		SaveObjectForTransaction( PlayerInput );
		SaveObjectForTransaction( OptionSettings );
		PlayerInput->MouseSensitivity = OptionSettings->MouseSensitivity;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetInvertMouse()
{
	AavaPlayerController* PC = GetPlayerOwner();
	UPlayerInput* PlayerInput = PC ? PC->PlayerInput : NULL;

	return PlayerInput ? PlayerInput->bInvertMouse : FALSE;
}

UBOOL UavaOptionSettings::SetInvertMouse( UBOOL bInvertMouse )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::StaticClass()->GetDefaultObject<UavaOptionSettings>();
	AavaPlayerController* PC = GetPlayerOwner();
	UPlayerInput* PlayerInput = PC ? PC->PlayerInput : NULL;

	OptionSettings->bInvertMouse = bInvertMouse ? TRUE : FALSE;
	if( PlayerInput != NULL &&
		GetInvertMouse() != OptionSettings->bInvertMouse )
	{
		SaveObjectForTransaction( PlayerInput );
		SaveObjectForTransaction( OptionSettings );
		PlayerInput->bInvertMouse = OptionSettings->bInvertMouse ? TRUE : FALSE;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetMouseSmoothing()
{
	AavaPlayerController* PC = GetPlayerOwner();
	UPlayerInput* PlayerInput = PC ? PC->PlayerInput : NULL;

	return PlayerInput ? PlayerInput->bEnableMouseSmoothing : FALSE;
}

UBOOL UavaOptionSettings::SetMouseSmoothing( UBOOL bSet, UBOOL bSaveConfig )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::StaticClass()->GetDefaultObject<UavaOptionSettings>();
	AavaPlayerController* PC = GetPlayerOwner();
	UPlayerInput* PlayerInput = PC ? PC->PlayerInput : NULL;

	OptionSettings->bDisableMouseSmoothing = !(bSet ? TRUE : FALSE);
	if( PlayerInput != NULL &&
		GetMouseSmoothing() == OptionSettings->bDisableMouseSmoothing )
	{
		SaveObjectForTransaction( PlayerInput );
		SaveObjectForTransaction( OptionSettings );
		PlayerInput->bEnableMouseSmoothing = !(OptionSettings->bDisableMouseSmoothing ? TRUE : FALSE);

		if( bSaveConfig )
		{
			OptionSettings->SaveConfig();
			PlayerInput->SaveConfig();
		}
			
		return TRUE;
	}
	return FALSE;
}

/** Game Settings */
UBOOL UavaOptionSettings::GetUseLocalSound()
{
	AavaGameReplicationInfo* GRI = (GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->GRI) ? Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI) : NULL;
	return GRI ? GRI->UseLocalSound : FALSE;
}

UBOOL UavaOptionSettings::SetUseLocalSound(UBOOL bUseLocalSound)
{
	AavaGameReplicationInfo* GRI = (GWorld->GetWorldInfo() && GWorld->GetWorldInfo()->GRI) ? Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI) : NULL;
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( GRI != NULL && GRI->UseLocalSound != bUseLocalSound )
	{
		SaveObjectForTransaction( GRI );
		SaveObjectForTransaction( OptionSettings );
		GRI->UseLocalSound = bUseLocalSound;
		OptionSettings->UseLocalSound = bUseLocalSound;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetUseHUDCamera()
{
	return GetDefaultOptionSettings()->UseHUDCamera;
}

UBOOL UavaOptionSettings::SetUseHUDCamera( UBOOL bUseHUDCamera )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( OptionSettings->UseHUDCamera != bUseHUDCamera )
	{
		SaveObjectForTransaction( OptionSettings );
		OptionSettings->UseHUDCamera = bUseHUDCamera;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetOneFrameThreadLag()
{
	return GSystemSettings->bAllowOneFrameThreadLag;
}

UBOOL UavaOptionSettings::SetOneFrameThreadLag( UBOOL bSet )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( GSystemSettings->bAllowOneFrameThreadLag != bSet )
	{
		SaveObjectForTransaction( OptionSettings );
		SaveObjectForTransaction( GSystemSettings );
		GSystemSettings->bAllowOneFrameThreadLag = OptionSettings->bAllowOneFrameThreadLag = bSet;
		return TRUE;
	}
	return FALSE;
}

INT UavaOptionSettings::GetMaxRagdollCount()
{
	return GetDefaultOptionSettings()->MaxRagdollCount;
}

UBOOL UavaOptionSettings::SetMaxRagdollCount( INT MaxRagdoll )
{
	UavaOptionSettings* OptionSettings = GetDefaultOptionSettings();
	if( OptionSettings->MaxRagdollCount != MaxRagdoll )
	{
		SaveObjectForTransaction( OptionSettings );
		OptionSettings->MaxRagdollCount = MaxRagdoll;
		return TRUE;
	}
	return FALSE;
}

UBOOL UavaOptionSettings::GetLoadMapCache()
{
	return GSystemSettings->bUseLoadMapCache;
}

UBOOL UavaOptionSettings::SetLoadMapCache( UBOOL bSet )
{
	bSet = bSet > 0 ? TRUE : FALSE;
	if( GSystemSettings->bUseLoadMapCache != bSet )
	{
		UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		SaveObjectForTransaction(GSystemSettings);
		SaveObjectForTransaction(OptionSettings);
		OptionSettings->bUseLoadMapCache = GSystemSettings->bUseLoadMapCache = bSet;

		UBOOL bFileCacheEnabled = GfileCache.bEnabled ? TRUE : FALSE;
		if( bFileCacheEnabled != bSet )
		{
			GetAvaNetHandler()->ProcMessage( EMsg_Option, EMsg_Option_SettingChanged, TEXT("LoadMapCache"), TEXT(""), 0, 0 );
		}
		return TRUE;
	}

	return FALSE;
}

/** NetConfig */
static const TCHAR OffsetChar = TEXT(' ');

static FString GetGameOptionStringOld()
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	FString ResultStr;
	for( INT FieldType = 0 ; FieldType < GAMEOPTIONFIELD_MAX ; FieldType++ )
	{
		TCHAR ch = OffsetChar;
		switch( FieldType )
		{
		case GAMEOPTIONFIELD_MouseSmoothing:
			ch += OptionSettings->GetMouseSmoothing() ? FALSE : TRUE;
			break;
		}
		ResultStr += ch;
	}
	return ResultStr;
}

static void SetGameOptionStringOld( const FString& NewValue )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	if( GetGameOptionStringOld() != NewValue )
	{
		for( INT ch = 0 ; ch < NewValue.Len() && ch < GAMEOPTIONFIELD_MAX ; ch++ )
		{
			BYTE ByteValue = NewValue[ch] - OffsetChar;
			UBOOL bValue = ByteValue > 0;
			switch( ch )
			{
			case GAMEOPTIONFIELD_MouseSmoothing:
				OptionSettings->SetMouseSmoothing( !bValue, TRUE);
				break;
			}
		}
	}
}

TArray<BYTE> GetGameOptionBytes( )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	const INT BitSizeOfStreamUnit = (sizeof(BYTE) * 8);
	TArray<BYTE> ByteStream;
	for( INT FieldType = 0 ; FieldType < GAMEOPTIONFIELD_MAX ; FieldType++ )
	{
		UBOOL bBitField = FALSE;
		switch( FieldType )
		{
		case GAMEOPTIONFIELD_MouseSmoothing:
			bBitField =  OptionSettings->GetMouseSmoothing() ? TRUE : FALSE;
			break;
		}

		INT ArrayIndex = (FieldType / BitSizeOfStreamUnit);
		if( !ByteStream.IsValidIndex(ArrayIndex) && ArrayIndex >= ByteStream.Num() )
			ByteStream.AddZeroed( (ArrayIndex - ByteStream.Num() + 1) );

		check( ByteStream.IsValidIndex(ArrayIndex) );
		if( bBitField )
			ByteStream(ArrayIndex) |= (0x01 << (FieldType % BitSizeOfStreamUnit));
	}

	return ByteStream;
}

void SetGameOptionBytes( TArray<BYTE>& ByteStream )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	const INT BitSizeOfStreamUnit = (sizeof(BYTE) * 8);

	for( INT FieldType = 0 ; FieldType < GAMEOPTIONFIELD_MAX ; FieldType++ )
	{
		UBOOL bBitField = FALSE;

		INT ArrayIndex = (FieldType / BitSizeOfStreamUnit);
		
		if( ByteStream.IsValidIndex(ArrayIndex) )
		{
			UBOOL bFieldValue = ByteStream(ArrayIndex) & (0x01 << (FieldType % BitSizeOfStreamUnit));
			switch( FieldType )
			{
			case GAMEOPTIONFIELD_MouseSmoothing:
				OptionSettings->SetMouseSmoothing( bFieldValue, TRUE );
			}
		}
	}
}

FString UavaOptionSettings::GetGameOptionString()
{
	CBase64	Base64Encoder;
	TArray<BYTE> GameOptionBytes = GetGameOptionBytes();
	Base64Encoder.Encode( GameOptionBytes.GetTypedData(), GameOptionBytes.Num() );

	return FString(Base64Encoder.EncodedMessage());
}

void UavaOptionSettings::SetGameOptionString( const FString& NewValue )
{
	if( NewValue.Len() == 0 )
		return;

	UBOOL bIsOldFormat = FALSE;
	for( INT ch = 0 ; ch < NewValue.Len() ; ch++ )
	{
		TCHAR c = NewValue[ch];
		// an invalid character (base64 encoding)
		if( !( appIsAlnum(c) || c == TEXT('=') || c == TEXT('/') || c == TEXT('+') ) )
		{
			bIsOldFormat = TRUE;
			break;
		}
	}

	if( bIsOldFormat )
	{
		SetGameOptionStringOld(NewValue);
		//GetAvaNetRequest()->OptionSaveUserKey(FString(TEXT("")),GetGameOptionString());
		return;
	}

	CBase64 Base64Decoder;
	Base64Decoder.Decode( TCHAR_TO_ANSI(*NewValue) );
	FString DecodedStr(Base64Decoder.DecodedMessage());
	TArray<BYTE> GameOptionBytes(DecodedStr.Len());
	for( INT ch = 0 ; ch < DecodedStr.Len() ; ch++ )
		GameOptionBytes(ch) = DecodedStr[ch];

	SetGameOptionBytes( GameOptionBytes );
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetOptionSettings
========================================================================================================== */

IMPLEMENT_CLASS( UUIDataProvider_AvaNetOptionSettings );

/* === IUIListElement interface === */

UBOOL UUIDataProvider_AvaNetOptionSettings::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	return FALSE;
}

TScriptInterface<class IUIListElementCellProvider> UUIDataProvider_AvaNetOptionSettings::GetElementCellSchemaProvider( FName FieldName )
{
	return TScriptInterface<class IUIListElementCellProvider>();
}

/* === UIDataProvider interface === */

void UUIDataProvider_AvaNetOptionSettings::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	GetInternalDataProviderSupportedDataFields( out_Fields );
}

void UUIDataProvider_AvaNetOptionSettings::PostUpdateParamters( UBOOL bParmChanged )
{
	if( bParmChanged )
	{
	}
}

UBOOL UUIDataProvider_AvaNetOptionSettings::GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	return TRUE;
}

UBOOL UUIDataProvider_AvaNetOptionSettings::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	return TRUE;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetOptionSettings - UUIDataProvider_AvaNetOptionSettingVideo
========================================================================================================== */

// for SupportedResolutionProvider and SupportedAspectRatioProvider
typedef TMultiMap< FScreenResolution, FScreenResolution > ResMapType;

IMPLEMENT_CLASS(UUIDataProvider_AvaNetOptionSettingVideo);

void UUIDataProvider_AvaNetOptionSettingVideo::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentResolution")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentAspectRatio")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentAnisotropy")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentAntiAliasing")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentPostProcess")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentTextureDetail")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentCharacerDetail")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentDynamicShadow")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentShaderModel")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentDecalDetail")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentImpactParticle")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentDynamicLight")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentStableFrameMode")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentFog")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentImpactParticle")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentGamma")),DATATYPE_Property);

	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentLODApply")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentShadowDetail")),DATATYPE_Property);

	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentVSync")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentSmoothFrameRate")),DATATYPE_Property);

	new(out_Fields) FUIDataProviderField(FName(TEXT("AspectRatioList")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("ResolutionList")),DATATYPE_Collection);

	new(out_Fields) FUIDataProviderField(FName(TEXT("ShadowDetailList")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CommonValueList2")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CommonValueList3")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("LODBiasList")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("ShaderModelList")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("PostProcessList")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("DynamicLightList")),DATATYPE_Collection);

	new(out_Fields) FUIDataProviderField(FName(TEXT("AntiAliasingList")),DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(FName(TEXT("AnisotropyList")),DATATYPE_Collection);
}

UBOOL UUIDataProvider_AvaNetOptionSettingVideo::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );

	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	if( FieldName == TEXT("AspectRatioList"))
	{
		ResMapType ResMap;
		GetSupportedResolutions(ResMap);

		TArray<FScreenResolution> AspectRatios;
		for( ResMapType::TIterator it(ResMap) ; it ; ++it)
			if( AspectRatios.FindItemIndex(it.Key()) < 0 )
				AspectRatios.AddItem(it.Key());

		for( INT Index = 0 ; Index < AspectRatios.Num() ; Index++ )
			out_Elements.AddItem(Index);

		bResult = TRUE;
	}
	else if( FieldName == TEXT("ResolutionList"))
	{
		ResMapType ResMap;
		GetSupportedResolutions(ResMap);
		ResMapType::TIterator it(ResMap);
		UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		FLOAT CurrentScreenRatio = OptionSettings->CurrentAspectRatio.X / OptionSettings->CurrentAspectRatio.Y;

		for( INT Index = 0 ; it ; ++it, ++Index )
		{
			FLOAT ItemScreenRatio = (FLOAT)it.Key().Width / it.Key().Height;
			if( fabsf( ItemScreenRatio -  CurrentScreenRatio )  < KINDA_SMALL_NUMBER ) 
				out_Elements.AddItem(Index);
		}
		bResult = TRUE;
	}
	else if( FieldName == TEXT("ShadowDetailList") )
	{
		TArray<INT> DetailList;
		OptionSettings->GetShadowDetailList( DetailList );
		for( INT i = 0 ; i < DetailList.Num() ; i++ )
			out_Elements.AddItem(DetailList(i));
	}
	else if( FieldName == TEXT("CommonValueList2"))
	{
		for( INT i = 0 ; i < 2 ; i++ )
			out_Elements.AddItem(i);
	}
	else if( FieldName == TEXT("CommonValueList3"))
	{
		for( INT i = 0 ; i < 3 ; i++ )
			out_Elements.AddItem(i);
	}
	else if( FieldName == TEXT("LODBiasList"))
	{
		for( INT i = 0 ; i < 3 ; i++ )
			out_Elements.AddItem(i);
	}
	else if( FieldName == TEXT("ShaderModelList"))
	{
		TArray<INT> ShaderModelList;
		UavaOptionSettings::GetDefaultOptionSettings()->GetShaderModelList( ShaderModelList );
		for( INT i = 0 ; i < ShaderModelList.Num() ; i++ )
			out_Elements.AddItem(ShaderModelList(i));
	}
	else if( FieldName == TEXT("PostProcessList"))
	{
		static INT Level[] = { 0, 2 };
		for( INT i = 0 ; i < ARRAY_COUNT(Level) ; i++)
			out_Elements.AddItem(Level[i]);

		bResult = TRUE;
	}
	else if( FieldName == TEXT("DynamicLightList"))
	{
		static const INT DynamicLightModeNum = 3;
		for( INT i = 0 ; i < DynamicLightModeNum ; i++ )
			out_Elements.AddItem(i);

		bResult = TRUE;
	}
	else if( FieldName == TEXT("AntiAliasingList"))
	{
		TArray<INT> AAList;
		UavaOptionSettings::GetDefaultOptionSettings()->GetAnisotropyList( AAList );
		for( INT i = 0 ; i < AAList.Num() ; i++ )
			out_Elements.Add(AAList(i));

		bResult = TRUE;
	}
	else if( FieldName == TEXT("AnisotropyList"))
	{
		TArray<INT> AnisotropyList;
		UavaOptionSettings::GetDefaultOptionSettings()->GetAnisotropyList( AnisotropyList );
		for( INT i = 0 ; i < AnisotropyList.Num() ; i++ )
			out_Elements.Add(AnisotropyList(i));

		bResult = TRUE;
	}

	return bResult;
}

void UUIDataProvider_AvaNetOptionSettingVideo::GetElementCellTags( TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(FName(TEXT("AspectRatioValue")), TEXT("ShadowDetailValue"));
	out_CellTags.Set(FName(TEXT("ResolutionValue")),TEXT("CommonValue"));

	out_CellTags.Set(FName(TEXT("ShadowDetailValue")), TEXT("ShadowDetailValue"));
	out_CellTags.Set(FName(TEXT("CommonValue2")),TEXT("CommonValue2"));
	out_CellTags.Set(FName(TEXT("CommonValue3")),TEXT("CommonValue3"));
	out_CellTags.Set(FName(TEXT("LODBiasValue")), TEXT("LODBiasValue"));
	out_CellTags.Set(FName(TEXT("ShaderModelValue")), TEXT("ShaderModelValue"));
	out_CellTags.Set(FName(TEXT("PostProcessValue")), TEXT("PostProcessValue"));
	out_CellTags.Set(FName(TEXT("DynamicLightValue")), TEXT("DynamicLightModeValue"));

	out_CellTags.Set(FName(TEXT("AnisotropyValue")), TEXT("AnisotropyValue"));
	out_CellTags.Set(FName(TEXT("AntialiasingValue")), TEXT("AntialiasingValue"));
}

UBOOL UUIDataProvider_AvaNetOptionSettingVideo::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/)
{
	UBOOL bResult = FALSE;

	if( CellTag == TEXT("AspectRatioValue"))
	{
		ResMapType ResMap;
		GetSupportedResolutions(ResMap);

		TArray<FScreenResolution> AspectRatios;
		for( ResMapType::TIterator it(ResMap) ; it ; ++it)
			if( AspectRatios.FindItemIndex(it.Key()) < 0 )
				AspectRatios.AddItem(it.Key());

		if( AspectRatios.IsValidIndex(ListIndex) )
			out_FieldValue.StringValue = appItoa(AspectRatios(ListIndex).Width) + FString(TEXT(" * ")) + appItoa(AspectRatios(ListIndex).Height);

		bResult = TRUE;
	}
	else if( CellTag == TEXT("ResolutionValue"))
	{
		ResMapType ResMap;
		GetSupportedResolutions(ResMap);
		ResMapType::TIterator it(ResMap);

		for(INT Index = 0 ; it ; ++it, ++Index)
		{
			if( Index == ListIndex )
			{
				out_FieldValue.StringValue = appItoa(it.Value().Width) + FString(TEXT(" * ")) + appItoa(it.Value().Height);
				break;
			}
		}
		bResult = TRUE;
	}
	else if( CellTag == TEXT("ShadowDetailValue") )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_ShadowDetail_Level3[%d]")), ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("CommonValue2"))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_CommonValue_Level2[%d]")), ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("CommonValue3"))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_CommonValue_Level3[%d]")), ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("LODBiasValue"))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_LODBias_Level3[%d]")), ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("ShaderModelValue"))
	{
		FString ActiveLabel = (ListIndex == GetExtShaderNum(GRHIShaderPlatform)) ? Localize(TEXT("UIOptionScene"),TEXT("Text_ActiveShaderModel_Prefix"), TEXT("avaNet")) : FString(TEXT(""));
		FString ShaderModelName = FString::Printf( TEXT("<Strings:avaNet.UIOptionScene.Text_ShaderModel[%d]>"), ListIndex );
		out_FieldValue.StringValue = ShaderModelName.Len() > 0 ? ActiveLabel + ShaderModelName : FString(TEXT("UnknownModel"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("PostProcessValue"))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_PostProcess_Level3[%d]"),ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("DynamicLightValue"))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_DynamicLight_Level3[%d]"),ListIndex), TEXT("avaNet"));;
		bResult = TRUE;
	}
	else if( CellTag == TEXT("AnisotropyValue"))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_Anisotropy_Level[%d]"),ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if( CellTag == TEXT("AntialiasingValue"))
	{
		FString AAModeName;
		if( RHIGetUserFriendlyAntialiasingName(ListIndex, AAModeName) )
		{
			UBOOL bFurtherMore = TRUE;
			UBOOL bNotUsed = (AAModeName == TEXT("N/A"));
			if( bNotUsed )
			{
				FString NextAAMode;
				bFurtherMore = RHIGetUserFriendlyAntialiasingName( 1, NextAAMode);
			}

			if( bNotUsed )
				out_FieldValue.StringValue = bFurtherMore ? Localize(TEXT("UIOptionScene"), TEXT("Text_AntiAliasing_Level[0]"), TEXT("avaNet")) : Localize(TEXT("UIOptionScene"),TEXT("Text_AntiAliasing_UnAvail"),TEXT("avaNet"));
			else
				out_FieldValue.StringValue = AAModeName;
		}
		bResult = TRUE;
	}

	return bResult;
}

UBOOL UUIDataProvider_AvaNetOptionSettingVideo::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	FName Field(*FieldName);
	UBOOL bResult = FALSE;

	if ( Field == FName(TEXT("CurrentResolution")) )
	{
		FVector2D CurrRes = OptionSettings->GetResolution();
		OutFieldValue.StringValue = appItoa(CurrRes.X) + FString(TEXT(" * ")) + appItoa(CurrRes.Y);
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentAspectRatio")) )
	{
		FVector2D CurrAspRatio = OptionSettings->CurrentAspectRatio;	
		OutFieldValue.StringValue = appItoa(CurrAspRatio.X) + FString(TEXT(" * ")) + appItoa(CurrAspRatio.Y);
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentAnisotropy")) )
	{
		extern INT GMaxLogAnisotropy;
		if( GMaxLogAnisotropy > 0 )
			OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_Anisotropy_Level[%d]"), OptionSettings->GetAnisotropy()), TEXT("avaNet"));
		else
			OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), TEXT("Text_Anisotropy_UnAvail"), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentAntiAliasing")) )
	{
		FString AAModeName;
		RHIGetUserFriendlyAntialiasingName( OptionSettings->GetAntiAliasing(), AAModeName);

		UBOOL bFurtherMore = TRUE;
		if( GSystemSettings->Antialiasing == 0 )
		{
			FString NextAAMode;
			bFurtherMore = RHIGetUserFriendlyAntialiasingName( OptionSettings->GetAntiAliasing() + 1, NextAAMode);
		}
		if( bFurtherMore )
			OutFieldValue.StringValue = AAModeName != TEXT("N/A") ?  AAModeName : Localize(TEXT("UIOptionScene"),TEXT("Text_AntiAliasing_Level[0]"),TEXT("avaNet"));
		else
			OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"),TEXT("Text_AntiAliasing_UnAvail"),TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentTextureDetail")) )
	{
		INT TexDetail = OptionSettings->GetTextureDetail();
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_LODBias_Level3[%d]"), TexDetail), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentCharacerDetail")) )
	{
		INT CharDetail = OptionSettings->GetCharacterDetail();
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_LODBias_Level3[%d]"), CharDetail), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentPostProcess")) )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_PostProcess_Level3[%d]"), GSystemSettings->GetPostProcessLevel()), TEXT("avaNet") );
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentDynamicShadow")) )
	{
		OutFieldValue.StringValue = OptionSettings->GetDynamicShadows() ? GTrue : GFalse;
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentShaderModel")) )
	{
		INT ShaderModelNum = OptionSettings->GetShaderModel();
		FString SectionName = FString::Printf(TEXT("Text_ShaderModel[%d]"), ShaderModelNum);
		FString ActiveLabel = ShaderModelNum ==  GetExtShaderNum( GRHIShaderPlatform ) ? Localize(TEXT("UIOptionScene"),TEXT("Text_ActiveShaderModel_Prefix"), TEXT("avaNet")) : TEXT("");
		OutFieldValue.StringValue = ActiveLabel + Localize(TEXT("UIOptionScene"), *SectionName, TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentDecalDetail")) )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_CommonValue_Level2[%d]"),OptionSettings->GetDecalDetail() ), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentImpactParticle")) )
	{
		OutFieldValue.StringValue = OptionSettings->GetImpactParticle() ? GTrue : GFalse;
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentDynamicLight")) )
	{
		INT DynLightLevel = OptionSettings->GetDynamicLightLevel();
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf( TEXT("Text_DynamicLight_Level3[%d]"), DynLightLevel ), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == FName(TEXT("CurrentStableFrameMode")) )
	{
		INT StableModeLevel = OptionSettings->GetStableFrameMode();
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_StableFrameMode_Level3[%d]"),StableModeLevel), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentFog") )
	{
		OutFieldValue.StringValue = OptionSettings->GetFog() ? GTrue : GFalse;
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentImpactParticle") )
	{
		OutFieldValue.StringValue = OptionSettings->GetImpactParticle() ? GTrue : GFalse;
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentGamma") )
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%.1f"), OptionSettings->GetDisplayGamma());
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentLODApply") )
	{
		OutFieldValue.StringValue = OptionSettings->GetLODApply() ? GTrue : GFalse;
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentShadowDetail") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_ShadowDetail_Level3[%d]"), OptionSettings->GetShadowDetail()), TEXT("avaNet"));;
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentVSync") )
	{
		OutFieldValue.StringValue = OptionSettings->GetVSync() ? GTrue : GFalse;
		bResult = TRUE;
	}
	else if ( Field == TEXT("CurrentSmoothFrameRate") )
	{
		OutFieldValue.StringValue = OptionSettings->GetSmoothFrameRate() ? GTrue : GFalse;
		bResult = TRUE;
	}

	if( bResult && OutFieldValue.StringValue.Len() == 0)
		OutFieldValue.StringValue = TEXT("N/A");

	return bResult || GetCellFieldValue(Field, INDEX_NONE, OutFieldValue, ArrayIndex );
}

UBOOL UUIDataProvider_AvaNetOptionSettingVideo::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FName Field = *FieldName;
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	FLOAT FloatParm = FieldValue.RangeValue.GetCurrentValue();
	INT IntParm = appTrunc(FloatParm);
	UBOOL BoolParam = (UBOOL)IntParm;
	FString StringValue = FieldValue.StringValue;

	if ( Field == FName(TEXT("CurrentResolution")))
	{
		TArray<FString> NewResolution;
		StringValue.ParseIntoArrayWS( &NewResolution, TEXT("*") );
		if( NewResolution.Num() >= 2 )
		{
			bResult = OptionSettings->SetResolution( FVector2D(appAtoi(*NewResolution(0)), appAtoi(*NewResolution(1))) );
		}
	}
	else if ( Field == FName(TEXT("CurrentAspectRatio")) )
	{
		TArray<FString> NewAspectRatio;
		StringValue.ParseIntoArrayWS( &NewAspectRatio, TEXT("*") );
		if( NewAspectRatio.Num() >= 2 )
		{
			bResult = OptionSettings->SetAspectRatio( FVector2D(appAtoi(*NewAspectRatio(0)), appAtoi(*NewAspectRatio(1))) );
		}
	}
	else if ( Field == FName(TEXT("CurrentAnisotropy")) )
	{
		bResult = OptionSettings->SetAnisotropy( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentAntiAliasing")) )
	{
		bResult = OptionSettings->SetAntiAliasing( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentTextureDetail")) )
	{
		bResult = OptionSettings->SetTextureDetail( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentCharacerDetail")) )
	{
		bResult = OptionSettings->SetCharacterDetail( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentPostProcess")) )
	{
		bResult = OptionSettings->SetPostProcess( BoolParam );
	}
	else if ( Field == FName(TEXT("CurrentDynamicShadow")) )
	{
		bResult = OptionSettings->SetDynamicShadow( BoolParam );
	}
	else if ( Field == FName(TEXT("CurrentShaderModel")) )
	{
		bResult = OptionSettings->SetShaderModel( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentDecalDetail")) )
	{
		bResult = OptionSettings->SetDecalDetail( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentDynamicLight")) )
	{
		bResult = OptionSettings->SetDynamicLightLevel( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentStableFrameMode")) )
	{
		bResult = OptionSettings->SetStableFrameMode( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentFog")) )
	{
		bResult = OptionSettings->SetFog( BoolParam );
	}
	else if ( Field == FName(TEXT("CurrentImpactParticle")) )
	{
		bResult = OptionSettings->SetImpactParticle( BoolParam );
	}
	else if ( Field == FName(TEXT("CurrentGamma")) )
	{
		bResult = OptionSettings->SetDisplayGamma( FloatParm );
	}
	else if ( Field == FName(TEXT("CurrentLODApply")) )
	{
		bResult = OptionSettings->SetLODApply( BoolParam );
	}
	else if ( Field == FName(TEXT("CurrentShadowDetail")) )
	{
		bResult = OptionSettings->SetShadowDetail( IntParm );
	}
	else if ( Field == FName(TEXT("CurrentVSync")) )
	{
		bResult = OptionSettings->SetVSync( BoolParam );
	}
	else if ( Field == TEXT("CurrentSmoothFrameRate") )
	{
		bResult = OptionSettings->SetSmoothFrameRate( BoolParam );
	}

	return bResult;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetOptionSettings - UUIDataProvider_AvaNetOptionSettingAudio
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetOptionSettingAudio);

UBOOL UUIDataProvider_AvaNetOptionSettingAudio::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	FString FieldStr = FieldName.GetName();
	FieldName = ParseDataField( FieldStr );

	if( FieldName == TEXT("AudioChannelList") )
	{
		static const INT AudioChannels[] = { 16, 32, 64, 128 };
		for( INT i = 0 ; i < ARRAY_COUNT(AudioChannels) ; i++ )
			out_Elements.AddItem( AudioChannels[i] );

		bResult = TRUE;
	}

	return bResult;
}

void UUIDataProvider_AvaNetOptionSettingAudio::GetElementCellTags( TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Set(TEXT("Channel"), TEXT("Channel"));
}

UBOOL UUIDataProvider_AvaNetOptionSettingAudio::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if( CellTag == TEXT("Channel") )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_AudioChannel[%d]"),ListIndex), TEXT("avaNet"));
		bResult = TRUE;
	}

	return bResult;
}

void UUIDataProvider_AvaNetOptionSettingAudio::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentAudioChannel")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentMusicVolume")),DATATYPE_RangeProperty);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentGameVolume")),DATATYPE_RangeProperty);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentSystemVolume")),DATATYPE_RangeProperty);

	new(out_Fields) FUIDataProviderField(FName(TEXT("AudioChannelList")),DATATYPE_Collection);
}

UBOOL UUIDataProvider_AvaNetOptionSettingAudio::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	UBOOL bResult = FALSE;
	check(OptionSettings);

	FName InFieldName = FName(*FieldName);
	if ( InFieldName == TEXT("CurrentAudioChannel") )
	{
		OutFieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_AudioChannel[%d]"), OptionSettings->GetAudioChannel()), TEXT("avaNet"));
		bResult = TRUE;
	}
	else if ( InFieldName == TEXT("CurrentMusicVolume") )
	{
		OutFieldValue.StringValue = FString::Printf( TEXT("%.1f"), OptionSettings->GetAudioVolume( TEXT("Music") ) );
		bResult = TRUE;
	}
	else if ( InFieldName == TEXT("CurrentSystemVolume") )
	{
		OutFieldValue.StringValue = FString::Printf( TEXT("%.1f"), OptionSettings->GetAudioVolume( TEXT("System") ) );
		bResult = TRUE;
	}
	else if ( InFieldName == TEXT("CurrentGameVolume") )
	{
		OutFieldValue.StringValue = FString::Printf( TEXT("%.1f"), OptionSettings->GetAudioVolume( TEXT("Game") ) );
		bResult = TRUE;
	}

	if( bResult && OutFieldValue.StringValue.Len() == 0)
		OutFieldValue.StringValue = TEXT("N/A");

	return bResult || GetCellFieldValue( FName(*FieldName), INDEX_NONE, OutFieldValue, ArrayIndex );
}

UBOOL UUIDataProvider_AvaNetOptionSettingAudio::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FLOAT AudioParms = FieldValue.RangeValue.GetCurrentValue();

	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings != NULL);

	FName InFieldName = FName(*FieldName);
	if( InFieldName == TEXT("CurrentAudioChannel") )
	{
		INT AudioChannelToSet = Clamp( appTrunc(AudioParms), 16, 128 );
		bResult = OptionSettings->SetAudioChannel( AudioChannelToSet );
	}
	else if ( InFieldName == TEXT("CurrentMusicVolume") )
	{
		FLOAT VolumeToSet = AudioParms;
		bResult = OptionSettings->SetAudioVolume(TEXT("Music"),VolumeToSet);
	}
	else if ( InFieldName == TEXT("CurrentSystemVolume") )
	{
		FLOAT VolumeToSet = AudioParms;
		bResult = OptionSettings->SetAudioVolume(TEXT("System"),VolumeToSet);
	}
	else if ( InFieldName == TEXT("CurrentGameVolume") )
	{
		FLOAT VolumeToSet = AudioParms;
		bResult = OptionSettings->SetAudioVolume(TEXT("Game"),VolumeToSet);
	}

	return bResult;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetOptionSettings - UUIDataProvider_AvaNetOptionSettingMouse
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetOptionSettingMouse);

void UUIDataProvider_AvaNetOptionSettingMouse::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentMouseSensitivity")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentInvertMouse")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentMouseSmoothing")),DATATYPE_Property);
}


UBOOL UUIDataProvider_AvaNetOptionSettingMouse::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{	
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	if ( FieldName == TEXT("CurrentMouseSensitivity"))
	{
		OutFieldValue.StringValue = FString::Printf(TEXT("%.1f"),OptionSettings->GetMouseSensitivity());
	}
	else if ( FieldName == TEXT("CurrentInvertMouse") )
	{
		OutFieldValue.StringValue = OptionSettings->GetInvertMouse() ? GTrue : GFalse;
	}
	else if ( FieldName == TEXT("CurrentMouseSmoothing") )
	{
		OutFieldValue.StringValue = OptionSettings->GetMouseSmoothing() ? GTrue : GFalse;
	}

	if( OutFieldValue.StringValue.Len() == 0)
		OutFieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UUIDataProvider_AvaNetOptionSettingMouse::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	UBOOL bResult = FALSE;
	FName InFieldName = *FieldName;

	FLOAT FloatParm = FieldValue.RangeValue.GetCurrentValue();
	INT IntParm = appTrunc(FloatParm);
	UBOOL BoolParm = (UBOOL)IntParm;

	if ( InFieldName == TEXT("CurrentMouseSensitivity"))
	{
		bResult = OptionSettings->SetMouseSensitivity( FloatParm );
	}
	else if ( InFieldName == TEXT("CurrentInvertMouse") )
	{
		bResult = OptionSettings->SetInvertMouse( BoolParm );
	}
	else if ( InFieldName == TEXT("CurrentMouseSmoothing") )
	{
		bResult = OptionSettings->SetMouseSmoothing( BoolParm );
	}


	return bResult;
}

/* ==========================================================================================================
	UUIDataProvider_AvaNetOptionSettings - UUIDataProvider_AvaNetOptionSettingGame
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_AvaNetOptionSettingGame);

void UUIDataProvider_AvaNetOptionSettingGame::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentLocalSound")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentLocalSound_Invert")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentHUDCamera")),DATATYPE_Property);

	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentOneFrameThreadLag")),DATATYPE_Property);
	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentMaxRagdollCount")),DATATYPE_Property);

	new(out_Fields) FUIDataProviderField(FName(TEXT("CurrentLoadMapCache")), DATATYPE_Property);
}


UBOOL UUIDataProvider_AvaNetOptionSettingGame::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{	
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	if ( FieldName == TEXT("CurrentLocalSound"))
	{
		OutFieldValue.StringValue = OptionSettings->GetUseLocalSound() ? GTrue : GFalse;
	}
	else if ( FieldName == TEXT("CurrentLocalSound_Invert") )
	{
		OutFieldValue.StringValue = OptionSettings->GetUseLocalSound() ? GFalse : GTrue;
	}
	else if ( FieldName == TEXT("CurrentHUDCamera") )
	{
		OutFieldValue.StringValue = OptionSettings->GetUseHUDCamera() ? GTrue : GFalse;
	}
	else if ( FieldName == TEXT("CurrentOneFrameThreadLag") )
	{
		OutFieldValue.StringValue = OptionSettings->GetOneFrameThreadLag() ? GTrue : GFalse;
	}
	else if ( FieldName == TEXT("CurrentMaxRagdollCount") )
	{
		OutFieldValue.StringValue = appItoa(OptionSettings->GetMaxRagdollCount());
	}
	else if ( FieldName == TEXT("CurrentLoadMapCache") )
	{
		OutFieldValue.StringValue = OptionSettings->GetLoadMapCache() ? GTrue : GFalse;
	}

	if( OutFieldValue.StringValue.Len() == 0)
		OutFieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UUIDataProvider_AvaNetOptionSettingGame::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
	check(OptionSettings);

	UBOOL bResult = FALSE;
	FName InFieldName = *FieldName;
	INT IntParam = appTrunc(FieldValue.RangeValue.GetCurrentValue());
	UBOOL BoolFlag = IntParam > 0 ? TRUE : FALSE;

	if ( InFieldName == TEXT("CurrentLocalSound"))
	{
		bResult = OptionSettings->SetUseLocalSound( BoolFlag );
	}
	else if ( InFieldName == TEXT("CurrentLocalSound_Invert") )
	{
		bResult = OptionSettings->SetUseLocalSound( !BoolFlag );
	}
	else if ( InFieldName == TEXT("CurrentHUDCamera") )
	{
		bResult = OptionSettings->SetUseHUDCamera( BoolFlag );
	}
	else if ( FieldName == TEXT("CurrentOneFrameThreadLag") )
	{
		bResult = OptionSettings->SetOneFrameThreadLag( BoolFlag );
	}
	else if ( FieldName == TEXT("CurrentMaxRagdollCount") )
	{
		bResult = OptionSettings->SetMaxRagdollCount( IntParam );
	}
	else if ( FieldName == TEXT("CurrentLoadMapCache") )
	{
		bResult = OptionSettings->SetLoadMapCache( BoolFlag );
	}

	return bResult;
}