// 사양별 기본 설정값(resolution) 및 bound값(ShaderPlatform)
// 추후에 기본 옵션 설정도 여기서 가져올 수 있게 하면 됨. :)
// 대부분 GOverrideXXXX로 해결봅시다.
#include "D3DDrvPrivate.h"
#include <dxdiag.h>


enum EFillrateQuality
{
	FQ_Highest,
	FQ_High,
	FQ_Medium,
	FQ_Low,
};

FString GetGPUKey( INT VendorId, INT DeviceId )
{
	struct FGPUDeviceInfo
	{
		INT DeviceId;
		char* DeviceName;		
	};

	return FString::Printf( TEXT("%04x%04x"), VendorId, DeviceId );	
}

INT RHICheckGPUMemory()
{
	INT GPUMemory = 0;

	IDxDiagProvider* pDxDiagProvider = NULL;
	IDxDiagContainer* pDxDiagRoot = NULL;
	IDxDiagContainer* pDxDiagDisplay = NULL;
	IDxDiagContainer* pDxDiag0 = NULL;

	UBOOL bCleanupCOM = SUCCEEDED( CoInitialize( NULL ) );		

	if (SUCCEEDED(::CoCreateInstance(CLSID_DxDiagProvider, NULL, CLSCTX_INPROC_SERVER, IID_IDxDiagProvider, (LPVOID*) &pDxDiagProvider)))
	{
		DXDIAG_INIT_PARAMS dxDiagInitParam;
		
		VARIANT vVideoRam;
		ZeroMemory(&dxDiagInitParam, sizeof(DXDIAG_INIT_PARAMS));
		dxDiagInitParam.dwSize                  = sizeof(DXDIAG_INIT_PARAMS);
		dxDiagInitParam.dwDxDiagHeaderVersion   = DXDIAG_DX9_SDK_VERSION;
		dxDiagInitParam.bAllowWHQLChecks        = false;
		dxDiagInitParam.pReserved               = NULL;

		if (SUCCEEDED(pDxDiagProvider->Initialize(&dxDiagInitParam))
			&& SUCCEEDED(pDxDiagProvider->GetRootContainer(&pDxDiagRoot))
			&& SUCCEEDED(pDxDiagRoot->GetChildContainer(L"DxDiag_DisplayDevices", &pDxDiagDisplay))
			&& SUCCEEDED(pDxDiagDisplay->GetChildContainer(L"0", &pDxDiag0))			
			&& SUCCEEDED(pDxDiag0->GetProp(L"szDisplayMemoryEnglish", &vVideoRam))
			&& vVideoRam.vt== VT_BSTR
			&& vVideoRam.bstrVal!= NULL
			)
		{	
			GPUMemory = appAtoi(vVideoRam.bstrVal);

			::VariantClear(&vVideoRam);
		}		
	}

#define SAFE_RELEASE(x) if (x) { (x)->Release(); (x) = NULL; }
	SAFE_RELEASE(pDxDiag0);
	SAFE_RELEASE(pDxDiagDisplay);
	SAFE_RELEASE(pDxDiagRoot);
	SAFE_RELEASE(pDxDiagProvider);

	if (bCleanupCOM)
	{
		CoUninitialize();
	}

	return GPUMemory;
}


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

extern FGPUDependentSettings GGPUDependentSettings;

extern TD3DRef<IDirect3D9> GDirect3D;



UBOOL RHICheckDriverVersion()
{
	// Create the Direct3D object if necessary.
	if ( !GDirect3D )
	{
		*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
	}
	if ( !GDirect3D )
	{
		appErrorf(
			NAME_FriendlyError,
			TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)")
			);

		return FALSE;
	}

	UINT AdapterIndex = D3DADAPTER_DEFAULT;
	D3DADAPTER_IDENTIFIER9	AdapterIdentifier;
	VERIFYD3DRESULT( GDirect3D->GetAdapterIdentifier( AdapterIndex, 0, &AdapterIdentifier ) );

	WORD ProductNumber		= HIWORD(AdapterIdentifier.DriverVersion.HighPart);
	WORD VersionNumber		= LOWORD(AdapterIdentifier.DriverVersion.HighPart);
	WORD SubVersionNumber	= HIWORD(AdapterIdentifier.DriverVersion.LowPart);
	WORD BuildNumber		= LOWORD(AdapterIdentifier.DriverVersion.LowPart);

	FString CurrentDriverVersion;
	CurrentDriverVersion = FString::Printf(TEXT("%d.%d.%d.%d"), ProductNumber, VersionNumber, SubVersionNumber, BuildNumber );
	debugf(NAME_Log, TEXT("Current Driver Version : %s"), *CurrentDriverVersion );

	// get required minimum driver's version number
	//FString DriverVersion;
	//const DWORD ATI_VENDOR_ID = 0x1002;
	//const DWORD NVIDIA_VENDOR_ID = 0x10DE;
	//if( AdapterIdentifier.VendorId == ATI_VENDOR_ID )
	//{
	//	GConfig->GetString( TEXT("GPUDriver"), TEXT("ATI"), DriverVersion, GGPUIni );
	//}
	//else if( AdapterIdentifier.VendorId == NVIDIA_VENDOR_ID )
	//{
	//	GConfig->GetString( TEXT("GPUDriver"), TEXT("NVIDIA"), DriverVersion, GGPUIni );
	//}

	//DriverVersion = DriverVersion.Trim();
	//TArray<FString> VersionNumbers;
	//DriverVersion.ParseIntoArray( &VersionNumbers, TEXT("."), TRUE );
	//if( VersionNumbers.Num() == 4 )
	//{
	//	debugf(NAME_Log, TEXT("Minimum Driver Version : %s.%s.%s.%s"), *VersionNumbers(0), *VersionNumbers(1), *VersionNumbers(2), *VersionNumbers(3) );
	//}
	//else
	//{
	//	debugf(NAME_Warning, TEXT("Redduck : Invalid driver number is used in gpu.ini"));
	//}
	return TRUE;
}



UBOOL RHICheckDeviceCaps()
{
	GGPUDependentSettings.bApplyThisSettings = FALSE;
	GGPUDependentSettings.StartupResolutionX = 1024;
	GGPUDependentSettings.StartupResolutionY = 768;
	GGPUDependentSettings.CharacterDetail = 0;
	GGPUDependentSettings.TextureDetail = 0;
	GGPUDependentSettings.Anisotropy = 0;
	GGPUDependentSettings.bNoFog = FALSE;
	GGPUDependentSettings.bNoDynamicShadows = FALSE;
	GGPUDependentSettings.bNoPostProcess = FALSE;

	// Create the Direct3D object if necessary.
	if ( !GDirect3D )
	{
		*GDirect3D.GetInitReference() = Direct3DCreate9(D3D_SDK_VERSION);
	}
	if ( !GDirect3D )
	{
		appErrorf(
			NAME_FriendlyError,
			TEXT("Please install DirectX 9.0c or later (see Release Notes for instructions on how to obtain it)")
			);

		return FALSE;
	}

	INT GPUMemory = RHICheckGPUMemory();

	UINT AdapterIndex = D3DADAPTER_DEFAULT;

	D3DADAPTER_IDENTIFIER9	AdapterIdentifier, CachedAdapterIdentifier;
	appMemzero( &CachedAdapterIdentifier, sizeof(CachedAdapterIdentifier) );

	VERIFYD3DRESULT(GDirect3D->GetAdapterIdentifier(AdapterIndex,0,&AdapterIdentifier));

	INT INIVersion = 0;
	GConfig->GetInt( TEXT("GPUPreset"), TEXT("GPUPresetVer"), INIVersion, GGPUIni );

	FArchive* Ar = GFileManager->CreateFileReader(*(appGameDir() + TEXT("GPUDeviceId.bin")));
	if (Ar)
	{
		INT Version;
		*Ar << Version;
		Ar->Serialize( &CachedAdapterIdentifier, sizeof(AdapterIdentifier) );
		Ar->Close();

		if (Version != INIVersion)
		{
			appMemzero( &CachedAdapterIdentifier, sizeof(CachedAdapterIdentifier) );
		}
	}			

	GGPUDependentSettings.ShaderModel = 2;

	UBOOL bIsSM3Hardware = FALSE;
	UBOOL bEnableShadow = FALSE;
	UBOOL bHiRes = FALSE;
	INT QC = 0;

	FString Key = GetGPUKey( AdapterIdentifier.VendorId, AdapterIdentifier.DeviceId );
	FString Entry;
	FString GPUName;	
	FString MemFallback;

	if (GConfig->GetString( TEXT("GPUPreset"), *Key, Entry, GGPUIni ))
	{
		// Trim whitespace at the beginning.
		Entry = Entry.Trim();
		// Remove brackets.
		Entry = Entry.Replace( TEXT("("), TEXT("") );
		Entry = Entry.Replace( TEXT(")"), TEXT("") );
		// Parse comma delimited entries into array.

		TArray<FString> Entries;
		Entry.ParseIntoArray( &Entries, TEXT(","), TRUE );

		FString Preset;

		for( INT EntryIndex=0; EntryIndex<Entries.Num(); EntryIndex++ )
		{
			const TCHAR* Entry = *Entries(EntryIndex);
			Parse( Entry, TEXT("Default="), Preset );			
			Parse( Entry, TEXT("Name="), GPUName );	
			Parse( Entry, TEXT("QC="), QC );

			//<@ 2007. 5. 11 changmin
			extern UBOOL GRequireResolve;
			//extern UBOOL GBlackShadowVolume;
			ParseUBOOL( Entry, TEXT("Resolve="),		GRequireResolve );
			//ParseUBOOL( Entry, TEXT("BlackVolume="),	GBlackShadowVolume );
			//>@

			UBOOL bUseFallback = 
				(GPUMemory <= 128) && Parse( Entry, TEXT("128="), MemFallback ) ||
				(GPUMemory > 128 && GPUMemory <= 256) && Parse( Entry, TEXT("256="), MemFallback ) ||
				(GPUMemory > 256) && Parse( Entry, TEXT("512="), MemFallback );

			if (bUseFallback)
			{				
				Preset = MemFallback;
			}
		}		

		if (GConfig->GetString( TEXT("GPUPreset"), *Preset, Entry, GGPUIni ))
		{
			// Trim whitespace at the beginning.
			Entry = Entry.Trim();
			// Remove brackets.
			Entry = Entry.Replace( TEXT("("), TEXT("") );
			Entry = Entry.Replace( TEXT(")"), TEXT("") );
			// Parse comma delimited entries into array.

			TArray<FString> Entries;
			Entry.ParseIntoArray( &Entries, TEXT(","), TRUE );

			FString Preset;

			for( INT EntryIndex=0; EntryIndex<Entries.Num(); EntryIndex++ )
			{
				const TCHAR* Entry = *Entries(EntryIndex);
				ParseUBOOL( Entry, TEXT("SM3="), bIsSM3Hardware );
				ParseUBOOL( Entry, TEXT("NoDynamicShadows="), GGPUDependentSettings.bNoDynamicShadows );
				ParseUBOOL( Entry, TEXT("NoFog="), GGPUDependentSettings.bNoFog );
				ParseUBOOL( Entry, TEXT("NoPostProcess="), GGPUDependentSettings.bNoPostProcess );
				ParseUBOOL( Entry, TEXT("NoWorldShadow="), GGPUDependentSettings.bNoWorldShadows );	
				Parse( Entry, TEXT("ShaderModel="), GGPUDependentSettings.ShaderModel );	
				Parse( Entry, TEXT("ResX="), GGPUDependentSettings.StartupResolutionX );	
				Parse( Entry, TEXT("ResY="), GGPUDependentSettings.StartupResolutionY );	
				Parse( Entry, TEXT("CharacterDetail="), GGPUDependentSettings.CharacterDetail );
				Parse( Entry, TEXT("TextureDetail="), GGPUDependentSettings.TextureDetail );				
				Parse( Entry, TEXT("Anisotropy="), GGPUDependentSettings.Anisotropy );				
			}	
		}		

		debugf( NAME_Log, TEXT("GPU : %s"), *GPUName );
	}		

	if (bIsSM3Hardware)
	{
		GGPUDependentSettings.ShaderModel = 3;
	}

	EShaderPlatform DeviceNativePlatform = GGPUDependentSettings.ShaderModel == 3 ? SP_PCD3D : GGPUDependentSettings.ShaderModel == 2 ? SP_PCD3D_SM2 : SP_PCD3D_SM2_POOR;

	if (GRHIShaderPlatform < DeviceNativePlatform)
	{
		GRHIShaderPlatform = DeviceNativePlatform;
	}


	UBOOL bDeviceChanged = (appMemcmp( &CachedAdapterIdentifier, &AdapterIdentifier, sizeof(AdapterIdentifier) ) != 0);

	if (QC == 0)
	{
		appMsgf( AMT_OK, *Localize(TEXT("Warning"),TEXT("UntestedGPU"),TEXT("Engine")), *GPUName );
	}	
	else if (QC < 0)
	{
		appMsgf( AMT_OK, *Localize(TEXT("Warning"),TEXT("NotSupportedGPU"),TEXT("Engine")), *GPUName );
		return FALSE;
	}

	if (bDeviceChanged)
	{
		Ar = GFileManager->CreateFileWriter(*(appGameDir() + TEXT("GPUDeviceId.bin")));
		if (Ar)
		{
			*Ar << INIVersion;
			Ar->Serialize( &AdapterIdentifier, sizeof(AdapterIdentifier) );
			Ar->Close();
		}

		GGPUDependentSettings.bApplyThisSettings = TRUE;
	}		

	return TRUE;
}
