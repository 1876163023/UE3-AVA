/*=============================================================================
	UnPrecompileShaderCommandlet.cpp: Shader precompiler (both local/reference cache).
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "UnTerrain.h"
#include "GPUSkinVertexFactory.h"

extern INT GInstanceId;

/**
 * Helper for loading PS3 or Xenon platform tools dlls
 */
class FPlatformDllLoader
{
public:
	enum EPlatformDllType
	{
		PlatformDll_None,
        PlatformDll_PS3,
		PlatformDll_Xenon,
	};

	/** constructor (default) */
	FPlatformDllLoader()
	: PtrDllLoader(NULL)
	{
	}

	/** destructor unloads any loaded dlls */
	~FPlatformDllLoader()
	{
		Unload();
	}
    
	/** load tool dlls for the platform type */
	UBOOL Load( EPlatformDllType Type )
	{
		if( Unload() )
		{
			switch( Type )
			{
			case PlatformDll_PS3:
				PtrDllLoader = new PS3DllLoader();
				break;
			case PlatformDll_Xenon:
				PtrDllLoader = new XenonDllLoader();
				break;
			};
		}
		return( PtrDllLoader != NULL && PtrDllLoader->IsValid() );
	}

	/** unload any loaded dlls */
	UBOOL Unload()
	{
        delete PtrDllLoader;
		return TRUE;
	}

	/** get handle to the platform tools dll */
	void* GetDllHandle()
	{
		return (PtrDllLoader) ? PtrDllLoader->DllHandle : NULL;
	}

private:

	/** base dll loader */
	class BaseDllLoader
	{
	public:
		BaseDllLoader() : DllHandle(NULL) {}
		virtual ~BaseDllLoader() {};
		virtual UBOOL IsValid() = 0;
		void* DllHandle;
	};

	/** PS3 dll loader */
	class PS3DllLoader : public BaseDllLoader
	{
	public:		
		PS3DllLoader()
		{
			FString DllFilename	= FString(appBaseDir()) + TEXT("\\PS3\\PS3Tools.dll");
			DllHandle = appGetDllHandle( *DllFilename );
			if( !DllHandle )
			{
				appDebugMessagef(TEXT("Couldn't bind to %s."), *DllFilename );
			}
		}

		~PS3DllLoader()
		{
            if( DllHandle )
			{
				appFreeDllHandle( DllHandle );
			}
		}

		virtual UBOOL IsValid()
		{
			return( DllHandle != NULL );
		}		
	};

	/** Xenon dll loader */
	class XenonDllLoader : public BaseDllLoader
	{
	public:
		XenonDllLoader()
			: XbdmDllHandle(NULL)
		{
			// XeToolsDll relies on xbdm.dll so we try to load it first.
			FString	XbdmDllFileName	= TEXT("xbdm.dll");
			XbdmDllHandle = appGetDllHandle( *XbdmDllFileName );
			if( !XbdmDllHandle )
			{
				appDebugMessagef(TEXT("Couldn't bind to %s."), *XbdmDllFileName );
			}
			else
			{
				// Try to load XeToolsDll if loading xbdm.dll succeeded.
				FString DllFilename	= FString(appBaseDir()) + TEXT("\\Xenon\\XeTools.dll");
				DllHandle = appGetDllHandle( *DllFilename );
				if( !DllHandle )
				{					
					appDebugMessagef(TEXT("Couldn't bind to %s."), *DllFilename );
				}
			}
		}

		~XenonDllLoader()
		{
			// unbind dlls that were loaded
			if( DllHandle ) 
			{
				appFreeDllHandle( DllHandle );
			}
			if( XbdmDllHandle )
			{
				appFreeDllHandle( XbdmDllHandle );
			}
		}

		virtual UBOOL IsValid()
		{
			return( DllHandle != NULL );
		}

		void* XbdmDllHandle;
	};

	BaseDllLoader* PtrDllLoader;
};

/*-----------------------------------------------------------------------------
	UPrecompileShadersCommandlet implementation.
-----------------------------------------------------------------------------*/

FShaderType* FindShaderTypeByKeyword(const TCHAR* Keyword, UBOOL bVertexShader)
{
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		if (appStristr(ShaderTypeIt->GetName(), Keyword) && appStristr(ShaderTypeIt->GetName(), bVertexShader ? TEXT("Vertex") : TEXT("Pixel")))
		{
			return *ShaderTypeIt;
		}
	}
	return NULL;
}

INT UPrecompileShadersCommandlet::Main(const FString& Params)
{
	if( ( !GIsFirstInstance ) && ( ParseParam(appCmdLine(),TEXT("ALLOW_PARALLEL_PRECOMPILESHADERS")) == FALSE ) )
	{
		SET_WARN_COLOR_AND_BACKGROUND(COLOR_RED, COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("ANOTHER INSTANCE IS RUNNING, THIS WILL NOT BE ABLE TO SAVE SHADER CACHE... QUITTING!"));
		CLEAR_WARN_COLOR();
		return 1;
	}

	// get some parameter
	FString Platform;
	if (!Parse(*Params, TEXT("PLATFORM="), Platform))
	{
		SET_WARN_COLOR(COLOR_YELLOW);
		warnf(TEXT("Usage: PrecompileShaders platform=<platform> [package=<package> [-script] [-depends]]\n   Platform is ps3 or xbox360 (or xenon)\n   Specifying no package will look in all packages\n   -script will force it to look in all script as well as the packages loaded by <package>\n   -depends will also look in dependencies of the package"));
		CLEAR_WARN_COLOR();
		return 1;
	}

	UBOOL bLoadScript = FALSE;
	UBOOL bProcessDependencies = FALSE;
	EShaderPlatform ShaderPlatform;	

	// are we making the ref shader cache?
	UBOOL bSaveReferenceShaderCache = ParseParam(*Params, TEXT("refcache"));

	// Whether we should skip maps or not.
	UBOOL bShouldSkipMaps = ParseParam(*Params, TEXT("skipmaps"));

	// @GEMINI_TODO: Combine this with the consolesupport container to just find the DLL that matches the platform
	// instead of hardcoding what DLL to load
	FPlatformDllLoader DllLoader;
	if (Platform == TEXT("PS3"))
	{
		DllLoader.Load( FPlatformDllLoader::PlatformDll_PS3 );
		ShaderPlatform = SP_PS3;
	}
	else if (Platform == TEXT("xenon") || Platform == TEXT("xbox360"))
	{	
		DllLoader.Load( FPlatformDllLoader::PlatformDll_Xenon );
		ShaderPlatform = SP_XBOXD3D;
	}
	else if (Platform == TEXT("pc") || Platform == TEXT("win32"))
	{
		ShaderPlatform = SP_PCD3D;
	}
	else if (Platform == TEXT("pc_sm2"))
	{
		ShaderPlatform = SP_PCD3D_SM2;
	}
	else if (Platform == TEXT("pc_sm2_poor"))
	{
		ShaderPlatform = SP_PCD3D_SM2_POOR;
	}
	else
	{
		SET_WARN_COLOR(COLOR_RED);
		warnf(NAME_Error, TEXT("Unknown platform. Run with no parameters for list of known platforms."));
		CLEAR_WARN_COLOR();
		return 1;
	}

	GInstanceId = ShaderPlatform;

	if(!IsPCPlatform( ShaderPlatform ))
	{
		// Bind function entry points.
		FuncCreateShaderPrecompiler CreateShaderPrecompiler	= (FuncCreateShaderPrecompiler)appGetDllExport(DllLoader.GetDllHandle(), TEXT("CreateShaderPrecompiler"));
		check( CreateShaderPrecompiler );

		// create the precompiler in the DLL
		check(!GConsoleShaderPrecompilers[ShaderPlatform]);
		GConsoleShaderPrecompilers[ShaderPlatform] = CreateShaderPrecompiler();
	}

	// figure out what packages to iterate over
	TArray<FString> PackageList;
	FString SinglePackage;
	if (Parse(*Params, TEXT("PACKAGE="), SinglePackage))
	{
		FString PackagePath;
		if (!GPackageFileCache->FindPackageFile(*SinglePackage, NULL, PackagePath))
		{
			SET_WARN_COLOR(COLOR_RED);
			warnf(NAME_Error, TEXT("Failed to find file %s"), *SinglePackage);
			CLEAR_WARN_COLOR();
			return 1;
		}
		else
		{
			PackageList.AddItem(PackagePath);

			bLoadScript = ParseParam(*Params, TEXT("script"));
			bProcessDependencies = ParseParam(*Params, TEXT("depends"));
		}
	}
	else
	{
		// make sure all possible script packages are loaded
		TArray<FString> PackageNames;
		appGetGameScriptPackageNames(PackageNames, TRUE);
		appGetGameNativeScriptPackageNames(PackageNames, TRUE);
		appGetEngineScriptPackageNames(PackageNames, TRUE);
		for (INT PackageIndex = 0; PackageIndex < PackageNames.Num(); PackageIndex++)
		{
			warnf(TEXT("Loading script package %s..."), *PackageNames(PackageIndex));
			UObject::LoadPackage(NULL, *PackageNames(PackageIndex), LOAD_None);
		}

		void LoadDLOObjectsExplicitly();
		LoadDLOObjectsExplicitly();

		PackageList = GPackageFileCache->GetPackageFileList();
	}

	FString ForceName;
	Parse(*Params, TEXT("FORCE="), ForceName);

	if (bLoadScript)
	{
		// @GEMINI_TODO: If -script was specified, load all script packages
		// load up all the script code if we are 
//	void appGetEngineScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
//	void appGetGameScriptPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages)
	}

	// force compile of the global shaders if desired
	if (ForceName == TEXT("GLOBAL"))
	{
		extern TShaderMap<FGlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];	
		delete GGlobalShaderMap[ShaderPlatform];
		GGlobalShaderMap[ShaderPlatform] = NULL;
	}
	FVertexFactoryType* OverrideVFType = NULL;
	FShaderType* OverrideShaderVertexType = NULL;
	FShaderType* OverrideShaderPixelType = NULL;
	if (ForceName == TEXT("GPUSKIN"))
	{
		OverrideVFType = &FGPUSkinVertexFactory::StaticType;
	}
	else if (ForceName == TEXT("LOCAL"))
	{
		OverrideVFType = &FLocalVertexFactory::StaticType;
	}

	else if (ForceName == TEXT("DEPTH"))
	{
		OverrideShaderVertexType = FindShaderTypeByKeyword(TEXT("DepthOnly"), TRUE);
		OverrideShaderPixelType = FindShaderTypeByKeyword(TEXT("DepthOnly"), FALSE);
	}
	else if (ForceName == TEXT("SHADOWDEPTH"))
	{
		OverrideShaderVertexType = FindShaderTypeByKeyword(TEXT("ShadowDepth"), TRUE);
		OverrideShaderPixelType = FindShaderTypeByKeyword(TEXT("ShadowDepth"), FALSE);
	}
	// @todo more

	// make sure we have all global shaders for the platform
	VerifyGlobalShaders(ShaderPlatform);

	for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
	{
		if( FFilename(PackageList(PackageIndex)).GetExtension() == FString(TEXT("u")) )
		{
			continue;
		}

		// we don't care about trying to wrangle the various shader caches so just skipz0r them
		if(	FFilename(PackageList(PackageIndex)).GetBaseFilename().InStr( TEXT("LocalShaderCache") )	!= INDEX_NONE
			||	FFilename(PackageList(PackageIndex)).GetBaseFilename().InStr( TEXT("RefShaderCache") )		!= INDEX_NONE )			
		{
			continue;
		}

		// Skip maps with default map extension if -skipmaps command line option is used.
		if( bShouldSkipMaps && FFilename(PackageList(PackageIndex)).GetExtension() == FURL::DefaultMapExt )
		{
			SET_WARN_COLOR(COLOR_DARK_GREEN);
			warnf(TEXT("Skipping map %s..."), *PackageList(PackageIndex));
			CLEAR_WARN_COLOR();
			continue;
		}

		if (PackageList(PackageIndex). InStr(TEXT("ShaderCache")) != -1)
		{
			SET_WARN_COLOR(COLOR_DARK_GREEN);
			warnf(TEXT("Skipping shader cache %s..."), *PackageList(PackageIndex));
			CLEAR_WARN_COLOR();
			continue;
		}

		SET_WARN_COLOR(COLOR_GREEN);
		warnf(TEXT("Loading %s..."), *PackageList(PackageIndex));
		CLEAR_WARN_COLOR();

		UPackage* Package = Cast<UPackage>(UObject::LoadPackage( NULL, *PackageList(PackageIndex), LOAD_None ));

		// go over all the materials
		for (TObjectIterator<UMaterial> It; It; ++It)
		{
			UMaterial* Material = *It;

			// if we are loading a single package, and we want to process its dependencies, then don't restrict by package

			// @GEMINI_TODO: if loading script, but not dependencies, then this will need to change!
			if (bProcessDependencies || Material->IsIn(Package))
			{
				SET_WARN_COLOR(COLOR_WHITE);
				warnf(TEXT("Processing %s..."), *Material->GetPathName());
				CLEAR_WARN_COLOR();

				UBOOL bForceCompile = FALSE;
				// if the name contains our force string, then flush the shader from the cache
				if (ForceName.Len() && (ForceName == TEXT("*") || Material->GetPathName().ToUpper().InStr(ForceName.ToUpper()) != -1))
				{
					SET_WARN_COLOR(COLOR_DARK_YELLOW);
					warnf(TEXT("Flushing shaders for %s"), *Material->GetPathName());
					CLEAR_WARN_COLOR();

					bForceCompile = TRUE;
				}

				// compile for the platform!
				TRefCountPtr<FMaterialShaderMap> MapRef;
                if (Material->MaterialResource == NULL)
                {
	                SET_WARN_COLOR(COLOR_YELLOW);
	                warnf(NAME_Warning, TEXT("%s has a NULL MaterialResource"), *Material->GetFullName());
	                CLEAR_WARN_COLOR();
	                continue;
                }

				if (OverrideVFType)
				{
					FMaterialShaderMap* ShaderMap = FMaterialShaderMap::FindId(Material->MaterialResource->GetId(), ShaderPlatform);
					ShaderMap->FlushShadersByVertexFactoryType(OverrideVFType);
				}
				if (OverrideShaderVertexType)
				{
					FMaterialShaderMap* ShaderMap = FMaterialShaderMap::FindId(Material->MaterialResource->GetId(), ShaderPlatform);
					ShaderMap->FlushShadersByShaderType(OverrideShaderVertexType);
				}
				if (OverrideShaderPixelType)
				{
					FMaterialShaderMap* ShaderMap = FMaterialShaderMap::FindId(Material->MaterialResource->GetId(), ShaderPlatform);
					ShaderMap->FlushShadersByShaderType(OverrideShaderPixelType);
				}

				if (!Material->MaterialResource->Compile(ShaderPlatform, MapRef, bForceCompile))
				{
					// handle errors
					warnf(NAME_Warning, TEXT("Material failed to be compiled:"));
					for(TMultiMap<UMaterialExpression*,FString>::TConstIterator ErrorIt(Material->MaterialResource->GetCompileErrors());ErrorIt;++ErrorIt)
					{
						warnf(NAME_Warning, TEXT("%s: %s"), *ErrorIt.Key()->GetName(), *ErrorIt.Value());
					}
				}
			}
		}

		// go over all the Actors
		for (TObjectIterator<ATerrain> It; It; ++It)
		{
			ATerrain* Terrain = *It;
			if (Terrain && Terrain->IsIn(Package))
			{
				if (Terrain->CachedMaterials != NULL)
				{
					// Make sure materials are compiled for Xbox 360 and add them to the shader cache embedded into seekfree packages.
					for (INT CachedMatIndex = 0; CachedMatIndex < Terrain->CachedMaterialCount; CachedMatIndex++)
					{
						FTerrainMaterialResource* TMatRes = Terrain->CachedMaterials[CachedMatIndex];
						if (TMatRes)
						{
							SET_WARN_COLOR(COLOR_WHITE);
							warnf(TEXT("Processing %s[%d]..."), *Terrain->GetPathName(), CachedMatIndex);
							CLEAR_WARN_COLOR();

							// Compile the material...
							TRefCountPtr<FMaterialShaderMap> MaterialShaderMapRef;
							if (!TMatRes->Compile(ShaderPlatform, MaterialShaderMapRef, FALSE))
							{
								// handle errors
								warnf(NAME_Warning, TEXT("Terrain material failed to be compiled:"));
								for(TMultiMap<UMaterialExpression*,FString>::TConstIterator ErrorIt(TMatRes->GetCompileErrors());ErrorIt;++ErrorIt)
								{
									warnf(NAME_Warning, TEXT("%s: %s"), *ErrorIt.Key()->GetName(), *ErrorIt.Value());
								}
							}
						}
					}
				}
			}
		}

		// save out the local shader cache
		SaveLocalShaderCaches();

		// close the package
		UObject::CollectGarbage(RF_Native);
	}

	if(!IsPCPlatform( ShaderPlatform ))
	{
		// destroy the precompiler object
		check(GConsoleShaderPrecompilers[ShaderPlatform]);
		FuncDestroyShaderPrecompiler DestroyShaderPrecompiler = (FuncDestroyShaderPrecompiler)appGetDllExport(DllLoader.GetDllHandle(), TEXT("DestroyShaderPrecompiler"));
		DestroyShaderPrecompiler(GConsoleShaderPrecompilers[ShaderPlatform]);
		GConsoleShaderPrecompilers[ShaderPlatform] = NULL;
	}

	// save out the local shader cache
	SaveLocalShaderCaches();

	// if we're saving the ref shader cache, save it now
	if (bSaveReferenceShaderCache)
	{
		FString ReferencePath = FString::Printf(TEXT("%s%s%sReferenceShaderCache.upk"),*appGameDir(),TEXT("Content"),PATH_SEPARATOR);

		// clear out the global shader maps from the reference shader cache
		extern TShaderMap<FGlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];	

		delete GGlobalShaderMap[ShaderPlatform];
		GGlobalShaderMap[ShaderPlatform] = NULL;

		// mark it as dirty so it will save
		GetLocalShaderCache(ShaderPlatform)->MarkDirty();

		// save it as the reference package (what if it's already open?)
		// @todo: tell LoadShaderCaches to not load the ref package somehow, or maybe delete the package at startup?
		SaveLocalShaderCache(ShaderPlatform,*GetReferenceShaderCacheFilename(ShaderPlatform));
	}

	return 0;
}
IMPLEMENT_CLASS(UPrecompileShadersCommandlet)
