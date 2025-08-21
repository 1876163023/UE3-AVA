/*=============================================================================
	UCContentCommandlets.cpp: Various commmandlets.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineUIPrivateClasses.h"

#include "EngineAnimClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineSoundClasses.h"
#include "UnOctree.h"
#include "UnTerrain.h"
#include "..\..\UnrealEd\Inc\scc.h"
#include "..\..\UnrealEd\Inc\SourceControlIntegration.h"

/**
* Creates an instance of a StaticMeshCollectorActor.  If a valid World is specified, uses SpawnActor;
* otherwise, uses ConstructObject.
*
* @param	Package		the package to create the new actor in
* @param	World		if Package corresponds to a map package, the reference to the UWorld object for the map.
*/
namespace
{
	template< class T >
	T* CreateComponentCollector( UPackage* Package, UWorld* World )
	{
		T* Result = NULL;

		if ( Package != NULL )
		{
			if ( World != NULL && World->PersistentLevel != NULL )
			{
				Result = Cast<T>(World->SpawnActor(T::StaticClass()));
			}
			else
			{
				Result = ConstructObject<T>(T::StaticClass(), Package);
			}

			if ( Result != NULL )
			{
				Result->SetFlags(RF_Cooked);
			}
		}

		return Result;
	}
};

/** 
 * Helper function to save a package that may or may not be a map package
 *
 * @param Package The package to save
 * @param Filename The location to save the package to
 *
 * @return TRUE if successful
 */
UBOOL SavePackageHelper(UPackage* Package, FString Filename)
{
	// look for a world object in the package (if there is one, there's a map)
	UWorld* World = FindObject<UWorld>(Package, TEXT("TheWorld"));
	UBOOL bSavedCorrectly;
	if (World)
	{	
		bSavedCorrectly = UObject::SavePackage(Package, World, 0, *Filename, GWarn);
	}
	else
	{
		bSavedCorrectly = UObject::SavePackage(Package, NULL, RF_Standalone, *Filename, GWarn);
	}

	// return success
	return bSavedCorrectly;
}

/*
Below is a template commandlet than can be used when you want to perform an operation on all packages.

INT UPerformAnOperationOnEveryPackage::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0 ; FileIndex < FilesInPath.Num() ; ++FileIndex )
	{
		const FFilename& Filename = FilesInPath(FileIndex);
		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		/////////////////
		//
		// Do your thing here
		//
		/////////////////

		TObjectIterator<UStaticMesh> It;...

		UStaticMesh* StaticMesh = *It;
		if( StaticMesh->IsIn( Package )




		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}
*/
/*-----------------------------------------------------------------------------
	UCutDownContentCommandlet commandlet.
-----------------------------------------------------------------------------*/
/**
 * Allows commandlets to override the default behavior and create a custom engine class for the commandlet. If
 * the commandlet implements this function, it should fully initialize the UEngine object as well.  Commandlets
 * should indicate that they have implemented this function by assigning the custom UEngine to GEngine.
 */
void UCutDownContentCommandlet::CreateCustomEngine()
{
	UClass* EditorEngineClass	= UObject::StaticLoadClass( UEditorEngine::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.EditorEngine"), NULL, LOAD_None | LOAD_DisallowFiles, NULL );

	// must do this here so that the engine object that we create on the next line receives the correct property values
	UObject* DefaultEngine		= EditorEngineClass->GetDefaultObject(TRUE);

	// ConditionalLink normally doesn't do anything for non-intrinsic classes (since Link should be called from UClass::Serialize in those cases),
	// but since we loaded the editor class using the LOAD_DisallowFiles file, Serialize won't be called.  Temporarily allow these non-intrinsic classes
	// to be linked so that the commandlet can operate properly
	GUglyHackFlags |= HACK_ClassLoadingDisabled;;
	EditorEngineClass->ConditionalLink();
	GUglyHackFlags &= ~HACK_ClassLoadingDisabled;

	GEngine = GEditor			= ConstructObject<UEditorEngine>( EditorEngineClass );
	GEditor->InitEditor();
}

INT UCutDownContentCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// Create directories for cutdown files.
	GFileManager->MakeDirectory( *FString::Printf(TEXT("..%sCutdownPackages%sPackages"	),PATH_SEPARATOR,PATH_SEPARATOR), 1 );
	GFileManager->MakeDirectory( *FString::Printf(TEXT("..%sCutdownPackages%sMaps"		),PATH_SEPARATOR,PATH_SEPARATOR), 1 );

	// Create a list of packages to base cutdown on.
	FString				PackageName;
	TArray<FString>		BasePackages;
	while( ParseToken( Parms, PackageName, 0 ) )
	{
		new(BasePackages)FString(FFilename(PackageName).GetBaseFilename());
	}

#if 1
	// Load all .u files
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FString &Filename = PackageList(PackageIndex);
		UObject* Package;

		if( appStricmp( *FFilename(Filename).GetExtension(), TEXT("u")) == 0 )
		{
			Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( !Package )
			{
				GWarn->Logf(NAME_Log, TEXT("Error loading %s!"), *Filename);
				continue;
			}
		}
		else
			continue;
	}

	UObject::ResetLoaders( NULL );
#endif

	// Load all specified packages/ maps.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename		Filename			= FFilename(PackageList(PackageIndex));
		INT				BasePackageIndex	= 0;			
		UPackage*		Package;

		// Find package.
		for( BasePackageIndex=0; BasePackageIndex<BasePackages.Num(); BasePackageIndex++ )
		{
			if( appStricmp( *Filename.GetBaseFilename(), *BasePackages(BasePackageIndex)) == 0 )
			{
				break;
			}
		}

		if( BasePackageIndex == BasePackages.Num() && BasePackageIndex )
		{
			continue;
		}

		UObject::CollectGarbage( RF_Native | RF_Standalone );

		Package = UObject::LoadPackage( NULL, *Filename, 0 );

		if( !Package )
		{
			GWarn->Logf(NAME_Log, TEXT("Error loading %s!"), *Filename);
			continue;
		}
		INT Total = PackageList.Num();
		GWarn->Logf(NAME_Log, TEXT("Loaded %s (%d/%d)"), *Filename,PackageIndex,Total);

		// Saving worlds.
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if( World )
		{
			FString NewMapName = FString::Printf(TEXT("..%sCutdownPackages%sMaps%s%s.%s"),PATH_SEPARATOR,PATH_SEPARATOR,PATH_SEPARATOR,*Filename.GetBaseFilename(),*Filename.GetExtension());
			UObject::SavePackage( Package, World, 0, *NewMapName, GWarn );

		}
		else
		{	
			warnf(NAME_Log, TEXT("Error loading %s! (no UWorld)"), *Filename);
		}
	}

	GWarn->Logf(NAME_Log, TEXT("Done loading. Now Saving."));

	// Save cutdown packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename		Filename	= FFilename(PackageList(PackageIndex));
		UPackage*		Package		= FindObject<UPackage>( NULL, *Filename.GetBaseFilename() );

		// strip out editor meta data (source textures, etc) if desired
		if (ParseParam(*Params, TEXT("strip")))
		{
			for (TObjectIterator<UObject> It; It; ++It)
			{
				if (It->IsIn(Package))
				{
					It->StripData(UE3::PLATFORM_Unknown);
				}
			}
		}

		// Don't save EnginePackages.
		if( Filename.ToUpper().InStr( TEXT("ENGINEPACKAGES") ) != INDEX_NONE )
			continue;

		if( Filename.ToUpper().InStr( TEXT("ENGINE\\CONTENT") ) != INDEX_NONE )
			continue;

		if( Filename.ToUpper().InStr( TEXT("AVAGAME\\CONTENT") ) != INDEX_NONE )
			continue;		

		// Only save .upk's that are currently loaded.
		if( !Package || appStricmp( *Filename.GetExtension(), TEXT("upk")) != 0 )
			continue;

		GWarn->Logf(NAME_Log, TEXT("Saving %s"),*Filename);

		FString NewPackageName = FString::Printf(TEXT("..%sCutdownPackages%sPackages%s%s.%s"),PATH_SEPARATOR,PATH_SEPARATOR,PATH_SEPARATOR,*Filename.GetBaseFilename(),*Filename.GetExtension());
		SavePackage( Package, NULL, RF_Standalone, *NewPackageName, NULL );
	}

	return 0;
}

IMPLEMENT_CLASS(UCutDownContentCommandlet);

/**-----------------------------------------------------------------------------
 *	UResavePackages commandlet.
 *
 * This commandlet is meant to resave packages as a default.  We are able to pass in
 * flags to determine which conditions we do NOT want to resave packages. (e.g. not dirty
 * or not older than some version)
 *
 *
----------------------------------------------------------------------------**/

#define CURRENT_PACKAGE_VERSION 0
#define IGNORE_PACKAGE_VERSION INDEX_NONE
INT UResavePackagesCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	FSourceControlIntegration* SCC = new FSourceControlIntegration;


	// skip the assert when a package can not be opened
	UBOOL bCanIgnoreFails		= ParseParam(Parms,TEXT("SKIPFAILS"));
	/** load all packages, and display warnings for those packages which would have been resaved but were read-only */
	UBOOL bVerifyContent			= ParseParam(Parms,TEXT("VERIFY"));
	/** if we should only save dirty pacakges **/
	UBOOL bOnlySaveDirtyPackages	= ParseParam(Parms,TEXT("OnlySaveDirtyPackages"));
	/** if we should auto checkout packages that need to be saved**/
	UBOOL bAutoCheckOut	= ParseParam(Parms,TEXT("AutoCheckOutPackages"));

	UClass* ResaveClass = NULL;
	UBOOL bCheckResaveClass = ParseObject<UClass>(Parms,TEXT("RESAVECLASS="),ResaveClass,ANY_PACKAGE);
	
	/** only packages that have this version or higher will be resaved; a value of IGNORE_PACKAGE_VERSION indicates that there is no minimum package version */
	INT MinResaveVersion=IGNORE_PACKAGE_VERSION;

	/** only packages that have this version or lower will be resaved; a value of IGNORE_PACKAGE_VERSION indicates that there is no maximum package version */
	INT MaxResaveVersion=IGNORE_PACKAGE_VERSION;

	// determine if the resave operation should be constrained to certain package versions
	if ( Parse(Parms,TEXT("-MINVER="),MinResaveVersion) )
	{
		if ( MinResaveVersion == CURRENT_PACKAGE_VERSION )
		{
			MinResaveVersion = GPackageFileVersion;
		}
	}

	if ( Parse(Parms,TEXT("-MAXVER="),MaxResaveVersion) )
	{
		if ( MaxResaveVersion == CURRENT_PACKAGE_VERSION )
		{
			MaxResaveVersion = GPackageFileVersion;
		}
	}
	else if ( ParseParam(Parms,TEXT("CHECKVER")) )
	{
		// only resave packages with old versions
		MaxResaveVersion = GPackageFileVersion - 1;
	}

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	INT GCIndex = 0;

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		UBOOL bIsReadOnly = GFileManager->IsReadOnly( *Filename);
		if( Filename.GetExtension() == TEXT("U") )
		{
			warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
		}
		else if ( bIsReadOnly && !bVerifyContent && !bAutoCheckOut )
		{
			warnf(NAME_Log, TEXT("Skipping read-only file %s"), *Filename);
		}
		else
		{
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);

			INT NumErrorsFromLoading = GWarn->Errors.Num();
			UBOOL bSavePackage = TRUE;

			// Get the package linker.
			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker(NULL,*Filename,LOAD_NoVerify,NULL,NULL);
			UObject::EndLoad();

			INT PackageVersion = Linker->Summary.GetFileVersion();

			// validate that this package meets the minimum requirement
			if ( MinResaveVersion != IGNORE_PACKAGE_VERSION && PackageVersion < MinResaveVersion )
			{
				bSavePackage = FALSE;
			}

			// make sure this package meets the maximum requirement
			if ( MaxResaveVersion != IGNORE_PACKAGE_VERSION && PackageVersion > MaxResaveVersion )
			{
				bSavePackage = FALSE;
			}

			if(bSavePackage)
			{
				// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
				UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
				if (Package == NULL)
				{
					if( bCanIgnoreFails == TRUE )
					{
						continue;
					}
					else
					{
						check(Package);
					}
				}

				// Check if the package contains any instances of the class that needs to be resaved.
				if(bCheckResaveClass)
				{
					bSavePackage = FALSE;
					for(FObjectIterator ObjectIt;ObjectIt;++ObjectIt)
					{
						if(ObjectIt->IsA(ResaveClass) && ObjectIt->IsIn(Package))
						{
							bSavePackage = TRUE;
						}
					}
				}

				// if we are only saving dirty packages and the package is not dirty, then we do not want to save the package (remember the default behavior is to ALWAYS save the package)
				if( ( bOnlySaveDirtyPackages == TRUE ) && ( Package->IsDirty() == FALSE ) )
				{
					bSavePackage = FALSE;
				}

				// here we want to check and see if we have any loading warnings
				// if we do then we want to resave this package
				if( !bSavePackage && ParseParam(appCmdLine(),TEXT("SavePackagesThatHaveFailedLoads")) == TRUE )
				{
					//warnf( TEXT( "NumErrorsFromLoading: %d GWarn->Errors num: %d" ), NumErrorsFromLoading, GWarn->Errors.Num() );

					if( NumErrorsFromLoading != GWarn->Errors.Num() )
					{
						bSavePackage = TRUE;
					}
				}

				// hook to allow performing additional checks without lumping everything into this one function
				PerformAdditionalOperations(Package,bSavePackage);

				// Now based on the computation above we will see if we should actually attempt
				// to save this package
				if( bSavePackage == TRUE )
				{
					if( bIsReadOnly == TRUE && bVerifyContent == TRUE && bAutoCheckOut == FALSE )
					{
						warnf(NAME_Log,TEXT("Package [%s] is read-only but needs to be resaved (Package's Version: %i  Current Version: %i)"), *Filename, PackageVersion, VER_LATEST_ENGINE );
					}
					else
					{
						// check to see if we need to check this package out
						if( bIsReadOnly == TRUE && bAutoCheckOut == TRUE )
						{
							SCC->CheckOut(Package);
						}

						// so now we need to see if we actually were able to check this file out
						// if the file is still read only then we failed and need to emit an error and go to the next package
						if( GFileManager->IsReadOnly( *Filename ) == TRUE )
						{
							warnf( NAME_Error, TEXT("Unable to check out the Package: %s"), *Filename );
							continue;
						}


						warnf(NAME_Log,TEXT("Resaving package [%s] (Package's Version: %i  Saved Version: %i)"), *Filename, PackageVersion, VER_LATEST_ENGINE );
						if( SavePackageHelper(Package, Filename) )
						{
							warnf( NAME_Log, TEXT("Correctly saved:  [%s]."), *Filename );
						}
					}
				}
			}

			UObject::CollectGarbage(RF_Native);
		}

		// Potentially save the local shader cache to include this packages' shaders.
		SaveLocalShaderCaches();
	}

	delete SCC; // clean up our allocated SCC
	SCC = NULL;

	return 0;
}
/**
 * Allows the commandlet to perform any additional operations on the package before it is resaved.
 *
 * @param	Package			the package that is currently being processed
 * @param	bSavePackage	[in]	indicates whether the package is currently going to be saved
 *							[out]	set to TRUE to resave the package
 */
void UResavePackagesCommandlet::PerformAdditionalOperations( UPackage* Package, UBOOL& bSavePackage )
{
	check(Package);
	UBOOL bShouldSavePackage = bSavePackage;
	
	if( ( ParseParam(appCmdLine(), TEXT("CLEANCLASSES")) == TRUE ) && ( CleanClassesFromContentPackages(Package) == TRUE ) )
	{
		bShouldSavePackage = TRUE;
	}
	else if( ( ParseParam(appCmdLine(), TEXT("INSTANCEMISSINGSUBOBJECTS")) == TRUE ) && ( InstanceMissingSubObjects(Package) == TRUE ) )
	{
		bShouldSavePackage = TRUE;
	}

	// add additional operations here

	bSavePackage = bShouldSavePackage;
}

/**
 * Removes any UClass exports from packages which aren't script packages.
 *
 * @param	Package			the package that is currently being processed
 *
 * @return	TRUE to resave the package
 */
UBOOL UResavePackagesCommandlet::CleanClassesFromContentPackages( UPackage* Package )
{
	check(Package);
	UBOOL bResult = FALSE;

	for ( TObjectIterator<UClass> It; It; ++It )
	{
		if ( It->IsIn(Package) )
		{
			warnf(NAME_Log, TEXT("Removing class '%s' from package [%s]"), *It->GetPathName(), *Package->GetName());

			// mark the class as transient so that it won't be saved into the package
			It->SetFlags(RF_Transient);

			// clear the standalone flag just to be sure :)
			It->ClearFlags(RF_Standalone);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Instances subobjects for any existing objects with subobject properties pointing to the default object.
 * This is currently the case when a class has an object property and subobject definition added to it --
 * existing instances of such a class will see the new object property refer to the template object.
 *
 * @param	Package			The package that is currently being processed.
 *
 * @return					TRUE to resave the package.
 */
UBOOL UResavePackagesCommandlet::InstanceMissingSubObjects( UPackage* Package )
{
	check( Package );
	UBOOL bResult = FALSE;

	for ( TObjectIterator<UObject> It; It; ++It )
	{
		if ( It->IsIn( Package ) )
		{
			if ( !It->IsTemplate() )
			{
				for ( TFieldIterator<UProperty> ItP(It->GetClass()) ; ItP ; ++ItP )
				{
					UProperty* Property = *ItP;
					if ( Property->PropertyFlags & CPF_NeedCtorLink )
					{
						UBOOL bAlreadyLogged = FALSE;
						for ( INT i = 0 ; i < Property->ArrayDim ; ++i )
						{
							FString	Value;
							if ( Property->ExportText( i, Value, (BYTE*)*It, (BYTE*)*It, *It, PPF_Localized ) )
							{
								UObject* SubObject = UObject::StaticFindObject( UObject::StaticClass(), ANY_PACKAGE, *Value );
								if ( SubObject )
								{
									if ( SubObject->IsTemplate() )
									{
										// The heap memory allocated at this address is owned the source object, so
										// zero out the existing data so that the UProperty code doesn't attempt to
										// deallocate it before allocating the memory for the new value.
										appMemzero( (BYTE*)*It + Property->Offset, Property->GetSize() );
										Property->CopyCompleteValue( (BYTE*)*It + Property->Offset, &SubObject, *It/*NULL*/, *It );
										bResult = TRUE;
										if ( !bAlreadyLogged )
										{
											bAlreadyLogged = TRUE;
											if ( Property->ArrayDim == 1 )
											{
												warnf( NAME_Log, TEXT("Instancing %s::%s"), *It->GetPathName(), *Property->GetName() );
											}
											else
											{
												warnf( NAME_Log, TEXT("Instancing %s::%s[%i]"), *It->GetPathName(), *Property->GetName(), i );
											}
											
										}
									}
								}
							}
						} // Property->ArrayDim
					} // If property NeedCtorLink
				} // Field iterator
			} // if It->IsTemplate
		} // If in package
	} // Object iterator

	return bResult;
}

IMPLEMENT_CLASS(UResavePackagesCommandlet)

/*-----------------------------------------------------------------------------
	UScaleAudioVolumeCommandlet commandlet.
-----------------------------------------------------------------------------*/

INT UScaleAudioVolumeCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	// Read the scale amount from the commandline
	FLOAT VolumeScale = 1.f;
	if ( !Parse(Parms, TEXT("VolumeScale="), VolumeScale) )
	{
		warnf(NAME_Log, TEXT("Failed to parse volume scale"));
		return 0;
	}

	TArray<FString> SoundPackages;
	FSourceControlIntegration* SCC = new FSourceControlIntegration;

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);

		if( Filename.GetExtension() == TEXT("U") )
		{
			warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
		}
		else
		{
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
			if (Package != NULL)
			{
				// Iterate over all SoundNodeWave objects
				UBOOL bHasSounds = FALSE;
				warnf(NAME_Log, TEXT("Looking for wave nodes..."));
				for (TObjectIterator<USoundNodeWave> NodeIt; NodeIt; ++NodeIt)
				{
					if (NodeIt->IsIn(Package))
					{
						bHasSounds = TRUE;
						// scale the volume
						NodeIt->Volume = NodeIt->Volume * VolumeScale;
					}
				}
				if ( bHasSounds )
				{
					if (GFileManager->IsReadOnly(*Filename))
					{
						SCC->CheckOut(Package);
						if (GFileManager->IsReadOnly(*Filename))
						{
							SoundPackages.AddItem(Package->GetFullName());
							continue;
						}
					}

					// resave the package
					SavePackageHelper(Package, Filename);
				}
			}
		}

		UObject::CollectGarbage(RF_Native);
	}

	for (INT Idx = 0; Idx < SoundPackages.Num(); Idx++)
	{
		warnf(TEXT("Failed to save sound package %s"),*SoundPackages(Idx));
	}

	delete SCC; // clean up our allocated SCC
	SCC = NULL;

	return 0;
}
IMPLEMENT_CLASS(UScaleAudioVolumeCommandlet)

/*-----------------------------------------------------------------------------
	UConformCommandlet commandlet.
-----------------------------------------------------------------------------*/

INT UConformCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	FString Src, Old;
	if( !ParseToken(Parms,Src,0) )
	{
		warnf(NAME_Error, TEXT("Source file not specified"));
		return 1;
	}
	if( !ParseToken(Parms,Old,0) )
	{
		warnf(NAME_Error, (TEXT("Old file not specified")));
		return 1;
	}

	// get the full pathname of the package we're conforming
	FString SrcFilename;
	GPackageFileCache->FindPackageFile(*Src, NULL, SrcFilename);

	// if it's read only, don't bother
	if (GFileManager->IsReadOnly(*SrcFilename))
	{
		warnf(NAME_Warning, TEXT("Package '%s' is read only"), *Src);
		return 0;
	}

	GWarn->Log( TEXT("Loading...") );
	BeginLoad();
	ULinkerLoad* OldLinker = UObject::GetPackageLinker( CreatePackage(NULL,*(Old+FString(TEXT("_OLD")))), *Old, LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
	EndLoad();

	UPackage* NewPackage = LoadPackage( NULL, *Src, LOAD_None );
	
	if( !OldLinker )
	{
		warnf(NAME_Error, TEXT("Old file '%s' load failed"), *Old);
		return 1;
	}
	if( !NewPackage )
	{
		warnf(NAME_Error, TEXT("New file '%s' load failed"), *Src);
		return 1;
	}

	GWarn->Log( TEXT("Saving...") );

	// look for a world object in the package (if there is one, there's a map)
	UWorld* World = FindObject<UWorld>(NewPackage, TEXT("TheWorld"));
	UBOOL bSavedCorrectly=FALSE;
	if (World)
	{	
		bSavedCorrectly = UObject::SavePackage(NewPackage, World, 0, *SrcFilename, GError, OldLinker);
	}
	else
	{
		bSavedCorrectly = UObject::SavePackage(NewPackage, NULL, RF_Standalone, *SrcFilename, GWarn, OldLinker);
	}
	warnf( TEXT("File %s successfully conformed to %s..."), *Src, *Old );
	
	GIsRequestingExit=1;
	return 0;
}
IMPLEMENT_CLASS(UConformCommandlet)

/*-----------------------------------------------------------------------------
	UDumpEmittersCommandlet commandlet.
-----------------------------------------------------------------------------*/
#include "EngineParticleClasses.h"

const TCHAR* GetParticleModuleAcronym(UParticleModule* Module)
{
	if (!Module)
		return TEXT("");

	if (Module->IsA(UParticleModuleAcceleration::StaticClass()))				return TEXT("IA");
	if (Module->IsA(UParticleModuleAccelerationOverLifetime::StaticClass()))	return TEXT("AOL");
	if (Module->IsA(UParticleModuleAttractorLine::StaticClass()))				return TEXT("AL");
	if (Module->IsA(UParticleModuleAttractorParticle::StaticClass()))			return TEXT("AP");
	if (Module->IsA(UParticleModuleAttractorPoint::StaticClass()))				return TEXT("APT");
	if (Module->IsA(UParticleModuleBeamNoise::StaticClass()))					return TEXT("BN");
	if (Module->IsA(UParticleModuleBeamSource::StaticClass()))					return TEXT("BS");
	if (Module->IsA(UParticleModuleBeamTarget::StaticClass()))					return TEXT("BT");
	if (Module->IsA(UParticleModuleCollision::StaticClass()))					return TEXT("CLS");
	if (Module->IsA(UParticleModuleColor::StaticClass()))						return TEXT("IC");
	if (Module->IsA(UParticleModuleColorByParameter::StaticClass()))			return TEXT("CBP");
	if (Module->IsA(UParticleModuleColorOverLife::StaticClass()))				return TEXT("COL");
	if (Module->IsA(UParticleModuleColorScaleOverLife::StaticClass()))			return TEXT("CSL");
	if (Module->IsA(UParticleModuleLifetime::StaticClass()))					return TEXT("LT");
	if (Module->IsA(UParticleModuleLocation::StaticClass()))					return TEXT("IL");
	if (Module->IsA(UParticleModuleLocationDirect::StaticClass()))				return TEXT("LD");
	if (Module->IsA(UParticleModuleLocationEmitter::StaticClass()))				return TEXT("LE");
	if (Module->IsA(UParticleModuleLocationPrimitiveCylinder::StaticClass()))	return TEXT("LPC");
	if (Module->IsA(UParticleModuleLocationPrimitiveSphere::StaticClass()))		return TEXT("LPS");
	if (Module->IsA(UParticleModuleMeshRotation::StaticClass()))				return TEXT("IMR");
	if (Module->IsA(UParticleModuleMeshRotationRate::StaticClass()))			return TEXT("IMRR");
	if (Module->IsA(UParticleModuleOrientationAxisLock::StaticClass()))			return TEXT("OAL");
	if (Module->IsA(UParticleModuleRotation::StaticClass()))					return TEXT("IR");
	if (Module->IsA(UParticleModuleRotationOverLifetime::StaticClass()))		return TEXT("ROL");
	if (Module->IsA(UParticleModuleRotationRate::StaticClass()))				return TEXT("IRR");
	if (Module->IsA(UParticleModuleRotationRateMultiplyLife::StaticClass()))	return TEXT("RRML");
	if (Module->IsA(UParticleModuleSize::StaticClass()))						return TEXT("IS");
	if (Module->IsA(UParticleModuleSizeMultiplyLife::StaticClass()))			return TEXT("SML");
	if (Module->IsA(UParticleModuleSizeMultiplyVelocity::StaticClass()))		return TEXT("SMV");
	if (Module->IsA(UParticleModuleSizeScale::StaticClass()))					return TEXT("SS");
	if (Module->IsA(UParticleModuleSubUV::StaticClass()))						return TEXT("SUV");
	if (Module->IsA(UParticleModuleSubUVDirect::StaticClass()))					return TEXT("SUVD");
	if (Module->IsA(UParticleModuleSubUVSelect::StaticClass()))					return TEXT("SUVS");
	if (Module->IsA(UParticleModuleTrailSource::StaticClass()))					return TEXT("TS");
	if (Module->IsA(UParticleModuleTrailSpawn::StaticClass()))					return TEXT("TSP");
	if (Module->IsA(UParticleModuleTrailTaper::StaticClass()))					return TEXT("TT");
	if (Module->IsA(UParticleModuleTypeDataBeam::StaticClass()))				return TEXT("TDB");
	if (Module->IsA(UParticleModuleTypeDataBeam2::StaticClass()))				return TEXT("TDB2");
	if (Module->IsA(UParticleModuleTypeDataMesh::StaticClass()))				return TEXT("TDM");
	if (Module->IsA(UParticleModuleTypeDataSubUV::StaticClass()))				return TEXT("TDSUV");
	if (Module->IsA(UParticleModuleTypeDataTrail::StaticClass()))				return TEXT("TDT");
	if (Module->IsA(UParticleModuleVelocity::StaticClass()))					return TEXT("IV");
	if (Module->IsA(UParticleModuleVelocityInheritParent::StaticClass()))		return TEXT("VIP");
	if (Module->IsA(UParticleModuleVelocityOverLifetime::StaticClass()))		return TEXT("VOL");

	return TEXT("???");
}

INT UDumpEmittersCommandlet::Main(const FString& Params)
{
	const TCHAR* Parms = *Params;

	FString						PackageWildcard;
	TArray<UParticleSystem*>	ParticleSystems;
	TArray<UPackage*>			ParticlePackages;
	TArray<FString>				UberModuleList;

	const TCHAR*	CmdLine		= appCmdLine();
	UBOOL			bAllRefs	= FALSE;

	if (ParseParam(CmdLine, TEXT("ALLREFS")))
	{
		bAllRefs	= TRUE;
	}

	while (ParseToken(Parms, PackageWildcard, 0))
	{
		TArray<FString> FilesInPath;

		if (PackageWildcard.Left(1) == TEXT("-"))
		{
			// this is a command-line parameter - skip it
			continue;
		}

		GFileManager->FindFiles(FilesInPath, *PackageWildcard, 1, 0);
		if( FilesInPath.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles(FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0);
		}

		if (FilesInPath.Num() == 0)
		{
			warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
			continue;
		}

		for (INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++)
		{
			const FString &Filename = FilesInPath(FileIndex);

			BeginLoad();
			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
			check(Package);
			EndLoad();

			// Check for particle systems
			if (Package)
			{
				debugf(TEXT("Examining Package %s"), *Package->GetName());

				// Dump the particle systems
				for (TObjectIterator<UParticleSystem> It; It; ++It)
				{
					UParticleSystem* PartSys = Cast<UParticleSystem>(*It);
					check(PartSys->GetOuter());

					// Determine the package it is in...
					UObject*	Outer			= PartSys->GetOuter();
					UPackage*	ParticlePackage = Cast<UPackage>(Outer);
					while (ParticlePackage == NULL)
					{
						Outer			= Outer->GetOuter();
						ParticlePackage = Cast<UPackage>(Outer);
						if (Outer == NULL)
						{
							warnf(TEXT("No package??? %s"), *PartSys->GetName());
							break;
						}
					}

					if (bAllRefs)
					{
						// Find the package the particle is in...
						ParticleSystems.AddUniqueItem(PartSys);
						ParticlePackages.AddUniqueItem(ParticlePackage);
					}
					else
					{
						if (It->IsIn(Package))
						{
							ParticleSystems.AddUniqueItem(PartSys);
							ParticlePackages.AddUniqueItem(Package);
						}
					}
				}
			}

			// Now, dump out the particle systems and the package list.
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("                                     PARTICLE SYSTEM LIST                                     \n"));
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("ParticleSystem Count %d"), ParticleSystems.Num());
			for (INT PSysIndex = 0; PSysIndex < ParticleSystems.Num(); PSysIndex++)
			{
				UParticleSystem* PSys = ParticleSystems(PSysIndex);
				if (PSys)
				{
					debugf(TEXT("\tParticleSystem %s"), *PSys->GetFullName());

					for (INT EmitterIndex = 0; EmitterIndex < PSys->Emitters.Num(); EmitterIndex++)
					{
						UParticleEmitter* Emitter = PSys->Emitters(EmitterIndex);
						if (Emitter == NULL)
							continue;

						debugf(TEXT("\t\tParticleEmitter %s"), *Emitter->GetName());

						FString	AcronymCollection;

						UParticleModule* Module;
						for (INT ModuleIndex = 0; ModuleIndex < Emitter->Modules.Num(); ModuleIndex++)
						{
							Module = Emitter->Modules(ModuleIndex);
							if (Module)
							{
								debugf(TEXT("\t\t\tModule %2d - %48s - %s"), ModuleIndex, *Module->GetName(), GetParticleModuleAcronym(Module));
								AcronymCollection += GetParticleModuleAcronym(Module);
							}
						}

						UberModuleList.AddUniqueItem(AcronymCollection);
					}
				}
			}

			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("                                         PACKAGE LIST                                         \n"));
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("Package Count %d"), ParticlePackages.Num());
			for (INT PackageIndex = 0; PackageIndex < ParticlePackages.Num(); PackageIndex++)
			{
				UPackage* OutPackage	= ParticlePackages(PackageIndex);
				debugf(TEXT("\t%s"), *OutPackage->GetName());
			}

			UObject::CollectGarbage(RF_Native);

			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("                                       UBER-MODULE LIST                                       \n"));
			debugf(TEXT("**********************************************************************************************\n"));
			debugf(TEXT("UberModule Count %d"), UberModuleList.Num());
			for (INT UberIndex = 0; UberIndex < UberModuleList.Num(); UberIndex++)
			{
				debugf(TEXT("\t%3d - %s"), UberIndex, *UberModuleList(UberIndex));
			}
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UDumpEmittersCommandlet)


/*-----------------------------------------------------------------------------
UMergePackagesCommandlet
-----------------------------------------------------------------------------*/

/**
 * This function will return all objects that were created after a given object index.
 * The idea is that you load some stuff, remember how many objects are in the engine, 
 * and then load a map where you want to track what was loaded. Any objects you load 
 * after that point will come after the original number of objects.
 *
 * @param NewObjects	The array that receives the new objects loaded beginning with index Start
 * @param Start			The first index of object to copy into NewObjects
 */
void GetAllObjectsAfter(TArray<UObject*>& NewObjects, DWORD Start = 0)
{
	// i wish there was a better way
	DWORD NumObjects = 0;
	for (TObjectIterator<UObject> It; It; ++It)
	{
		NumObjects++;
	}

	DWORD ObjIndex = 0;
	for (TObjectIterator<UObject> It; It; ObjIndex++, ++It)
	{
		UObject* Obj = *It;
		if (ObjIndex >= Start)
		{
			NewObjects.AddItem(Obj);
		}
	}
}

INT UMergePackagesCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	FString	PackageWildcard;

	FString SrcPackageName, DestPackageName;
	ParseToken(Parms,SrcPackageName,0);
	ParseToken(Parms,DestPackageName,1);

	
	// Only moving objects from source package, so find offset of current objects
	INT FirstObjectToMove = 0;
	for (TObjectIterator<UObject> It; It; ++It)
	{
		FirstObjectToMove++;
	}

	UPackage* SrcPackage  = Cast<UPackage>(UObject::LoadPackage( NULL, *SrcPackageName, 0 ));

	// the objects to be moved are the ones in the object array
	TArray<UObject*> DiffObjects;
	GetAllObjectsAfter(DiffObjects, FirstObjectToMove);

	UPackage* DestPackage = Cast<UPackage>(UObject::LoadPackage( NULL, *DestPackageName, 0 ));

	// go through all the new objects and rename all packages that were loaded to their new package name
	for (INT ObjIndex = 0; ObjIndex < DiffObjects.Num(); ObjIndex++)
	{
		UPackage* DiffPackage = Cast<UPackage>(DiffObjects(ObjIndex));

		if (DiffPackage && DiffPackage != UObject::GetTransientPackage() && DiffPackage != DestPackage)
		{
			BeginLoad();
			ULinkerLoad* Linker = GetPackageLinker(DiffPackage, NULL, LOAD_NoWarn, NULL, NULL);
			EndLoad();
			if (Linker && !Linker->ContainsCode() && !Linker->ContainsMap())
			{
				warnf(TEXT("Moving object %s to %s"), *DiffPackage->GetFullName(),*DestPackageName);
				// Only move if won't conflict
				UBOOL Exists = StaticFindObject(DiffPackage->GetClass(), DestPackage, *DiffPackage->GetName(), TRUE)!=0;
				if(Exists)
				{
					warnf(TEXT("Object already exists in package, skipped"));
				}
				else
				{
					DiffPackage->Rename(*DiffPackage->GetName(), DestPackage, REN_ForceNoResetLoaders);
//					warnf(TEXT("Object added to package"));
				}
			}
			else
			{
//				warnf(TEXT("%s didn't match"), *DiffPackage->GetFullName());
			}
		}
	}

	SavePackageHelper(DestPackage, DestPackageName);

	UObject::CollectGarbage(RF_Native);
	return 0;
}
IMPLEMENT_CLASS(UMergePackagesCommandlet)

/* ==========================================================================================================
	UCreateDefaultStyleCommandlet
========================================================================================================== */
INT UCreateDefaultStyleCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// create the UISkin package
	UPackage* SkinPackage = CreatePackage(NULL, TEXT("DefaultUISkin"));

	// load the classes we need, to ensure that their defaults are loaded
	LoadClass<UUISkin>(NULL, TEXT("Engine.UISkin"), NULL, LOAD_None, NULL);
	LoadClass<UUIStyle>(NULL, TEXT("Engine.UIStyle"), NULL, LOAD_None, NULL);
	LoadClass<UUIStyle_Data>(NULL, TEXT("Engine.UIStyle_Combo"), NULL, LOAD_None, NULL);

	// create the UISkin object
	DefaultSkin = ConstructObject<UUISkin>( UUISkin::StaticClass(), SkinPackage, TEXT("DefaultSkin"), RF_Public|RF_Standalone );
	DefaultSkin->Tag = TEXT("DefaultSkin");

	// create the default text style
	UUIStyle_Text* DefaultTextStyle = CreateTextStyle();

	// create the default image style
	UUIStyle_Image* DefaultImageStyle = CreateImageStyle();

	// create the default combo style
	UUIStyle_Combo* DefaultComboStyle = CreateComboStyle(DefaultTextStyle,DefaultImageStyle);

	DefaultSkin->AddStyle(DefaultTextStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(DefaultImageStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(DefaultComboStyle->GetOwnerStyle());

	CreateAdditionalStyles();

	CreateMouseCursors();

	// now save it!
	INT Result = SavePackage(SkinPackage, DefaultSkin, 0, TEXT("../Engine/Content/DefaultUISkin.upk"), GWarn) ? 0 : 1;

	return Result;
}

void UCreateDefaultStyleCommandlet::CreateAdditionalStyles() const
{
	CreateConsoleStyles();

	UUIStyle_Image* CursorImageStyle = CreateImageStyle(TEXT("CursorImageStyle"), FLinearColor(1.f,1.f,1.f,1.f));
	CursorImageStyle->DefaultImage = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.Arrow"), NULL, LOAD_None, NULL);
	DefaultSkin->AddStyle(CursorImageStyle->GetOwnerStyle());

	UUIStyle_Image* CaretImageStyle = CreateImageStyle(TEXT("DefaultCaretStyle"), FLinearColor(1.f,1.f,1.f,1.f));
	CaretImageStyle->DefaultImage = LoadObject<UTexture>(NULL, TEXT("EngineResources.White"), NULL, LOAD_None, NULL);
	DefaultSkin->AddStyle(CaretImageStyle->GetOwnerStyle());
}

void UCreateDefaultStyleCommandlet::CreateMouseCursors() const
{
	FUIMouseCursor ArrowCursor, SplitVCursor, SplitHCursor;
	ArrowCursor.CursorStyle = SplitVCursor.CursorStyle = SplitHCursor.CursorStyle = TEXT("CursorImageStyle");

	UUITexture* ArrowTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), DefaultSkin);
	UUITexture* SplitVTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), DefaultSkin);
	UUITexture* SplitHTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), DefaultSkin);

	ArrowTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.Arrow"), NULL, LOAD_None, NULL);
	SplitVTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.SplitterVert"), NULL, LOAD_None, NULL);
	SplitHTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.SplitterHorz"), NULL, LOAD_None, NULL);

	ArrowCursor.Cursor = ArrowTexture;
	SplitHCursor.Cursor = SplitHTexture;
	SplitVCursor.Cursor = SplitVTexture;

	DefaultSkin->AddCursorResource(TEXT("Arrow"), ArrowCursor);
	DefaultSkin->AddCursorResource(TEXT("SplitterHorz"), SplitHCursor);
	DefaultSkin->AddCursorResource(TEXT("SplitterVert"), SplitVCursor);
}

void UCreateDefaultStyleCommandlet::CreateConsoleStyles() const
{
	// create the style for rendering the text that is being typed into the console
	UUIStyle_Text* ConsoleTextStyle = CreateTextStyle(TEXT("ConsoleTextStyle"), FLinearColor(0.f,1.f,0.f,1.f));

	// create the style for the borders around the console typing region
	UUIStyle_Image* ConsoleImageStyle = CreateImageStyle(TEXT("ConsoleImageStyle"),FLinearColor(0.f,1.f,0.f,1.f));
	ConsoleImageStyle->DefaultImage = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.White"),NULL,0,NULL);

	// create a combo style that will be applied to the console's typing region
	UUIStyle_Combo* ConsoleStyle = CreateComboStyle(ConsoleTextStyle, ConsoleImageStyle, TEXT("ConsoleStyle"));

	// create the style for rendering the console buffer text
	UUIStyle_Text* ConsoleBufferTextStyle = CreateTextStyle(TEXT("ConsoleBufferTextStyle"));

	// create the style for the buffer background
	UUIStyle_Image* ConsoleBufferImageStyle = CreateImageStyle(TEXT("ConsoleBufferImageStyle"),FLinearColor(0.f,0.f,0.f,1.f));
	ConsoleBufferImageStyle->DefaultImage = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.Black"),NULL,0,NULL);

	UUIStyle_Combo* ConsoleBufferStyle = CreateComboStyle(ConsoleBufferTextStyle, ConsoleBufferImageStyle, TEXT("ConsoleBufferStyle"));

	DefaultSkin->AddStyle(ConsoleTextStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleImageStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleBufferTextStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleBufferImageStyle->GetOwnerStyle());
	DefaultSkin->AddStyle(ConsoleBufferStyle->GetOwnerStyle());
}

UUIStyle_Text* UCreateDefaultStyleCommandlet::CreateTextStyle( const TCHAR* StyleName/*=TEXT("DefaultTextStyle")*/, FLinearColor StyleColor/*=FLinerColor(1.f,1.f,1.f,1.f)*/ ) const
{
	UUIStyle* Style = ConstructObject<UUIStyle>( UUIStyle::StaticClass(), DefaultSkin, StyleName, RF_Public|RF_Transactional|RF_PerObjectLocalized );
	Style->StyleDataClass = UUIStyle_Text::StaticClass();
	Style->StyleID = appCreateGuid();
	Style->StyleTag = Style->GetFName();

	UUIStyle_Text* Result = Cast<UUIStyle_Text>(Style->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()));
	Result->StyleFont = GEngine->SmallFont;
	Result->StyleColor = StyleColor;

	return Result;
}

UUIStyle_Image* UCreateDefaultStyleCommandlet::CreateImageStyle( const TCHAR* StyleName/*=TEXT("DefaultImageStyle")*/, FLinearColor StyleColor/*=FLinerColor(1.f,1.f,1.f,1.f)*/ ) const
{
	UUIStyle* Style = ConstructObject<UUIStyle>( UUIStyle::StaticClass(), DefaultSkin, StyleName, RF_Public|RF_Transactional|RF_PerObjectLocalized );
	Style->StyleDataClass = UUIStyle_Image::StaticClass();
	Style->StyleID = appCreateGuid();
	Style->StyleTag = Style->GetFName();

	UUIStyle_Image* Result = Cast<UUIStyle_Image>(Style->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()));
	Result->DefaultImage = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.DefaultTexture"), NULL, 0, NULL);
	Result->StyleColor = StyleColor;

	return Result;
}

UUIStyle_Combo* UCreateDefaultStyleCommandlet::CreateComboStyle( UUIStyle_Text* TextStyle, UUIStyle_Image* ImageStyle, const TCHAR* StyleName/*=TEXT("DefaultComboStyle")*/ ) const
{
	UUIStyle* Style = ConstructObject<UUIStyle>( UUIStyle::StaticClass(), DefaultSkin, StyleName, RF_Public|RF_Transactional|RF_PerObjectLocalized );
	Style->StyleDataClass = UUIStyle_Combo::StaticClass();
	Style->StyleID = appCreateGuid();
	Style->StyleTag = Style->GetFName();

	UUIStyle_Combo* Result = Cast<UUIStyle_Combo>(Style->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()));
	Result->StyleColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
	Result->TextStyle = FStyleDataReference(TextStyle->GetOwnerStyle(),UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>());
	Result->ImageStyle = FStyleDataReference(ImageStyle->GetOwnerStyle(), UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>());

	return Result;
}

IMPLEMENT_CLASS(UCreateDefaultStyleCommandlet);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UExamineOutersCommandlet
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT UExamineOutersCommandlet::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	const FString DefaultObjectPrefix( DEFAULT_OBJECT_PREFIX );

	warnf( NAME_Log, TEXT("%s%-7s%-7s%-7s%-7s%-7s%-7s%-7s%-7s%-7s"),
		*FString("Package").RightPad(72),TEXT("Match"), TEXT("Level"), TEXT("Pkg"), TEXT("Class"), TEXT("NULL"), TEXT("DefObj"), TEXT("PSReq"), TEXT("Other"), TEXT("Total") );

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0 ; FileIndex < FilesInPath.Num() ; ++FileIndex )
	{
		const FFilename& Filename = FilesInPath(FileIndex);

		UObject* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		// Iterate over all the editinline properties of an object and gather outer stats.
		INT NumMatchedOuters = 0;
		INT NumLevelOuters = 0;
		INT NumPackageOuters = 0;
		INT NumClassOuters = 0;
		INT NumNullOuters = 0;
		INT NumDefaultObjectOuters = 0;
		INT NumPSRequiredModule = 0;
		INT NumOther = 0;
		INT NumTotal = 0;

		for ( TObjectIterator<UObject> It ; It ; ++It )
		{
			UObject* Object = *It;
			UClass* ObjectClass = Object->GetClass();

			for ( TFieldIterator<UProperty, CLASS_IsAUProperty> PropIt( ObjectClass ) ; PropIt ; ++PropIt )
			{
				UObjectProperty* Property = Cast<UObjectProperty>( *PropIt );

				// Select out editinline object properties.
				if ( (PropIt->PropertyFlags & (CPF_EditInline | CPF_EditInlineUse)) && Property )
				{
					// Get the property value.
					UObject* EditInlineObject;
					Property->CopySingleValue( &EditInlineObject, ((BYTE*)Object) + Property->Offset );

					// If the property value was non-NULL . . .
					if ( EditInlineObject )
					{
						// Check its outer.
						UObject* EditInlineObjectOuter = EditInlineObject->GetOuter();

						if ( EditInlineObjectOuter == NULL )
						{
							++NumNullOuters;
						}
						else if ( EditInlineObjectOuter == Object )
						{
							++NumMatchedOuters;
						}
						else if ( EditInlineObjectOuter->IsA( ULevel::StaticClass() ) )
						{
							++NumLevelOuters;
						}
						else if ( EditInlineObjectOuter->IsA( UPackage::StaticClass() ) )
						{
							++NumPackageOuters;
						}
						else if ( EditInlineObjectOuter->IsA( UClass::StaticClass() ) )
						{
							++NumClassOuters;
						}
						else if ( FString( *EditInlineObjectOuter->GetName() ).StartsWith( DefaultObjectPrefix ) )
						{
							++NumDefaultObjectOuters;
						}
						else if ( appStricmp( *Property->GetName(), TEXT("RequiredModule") ) == 0 )
						{
							++NumPSRequiredModule;
						}
						else
						{
							++NumOther;
						}
						++NumTotal;
					}
				}
			} // property itor
		} // object itor

		warnf( NAME_Log, TEXT("%s%-7i%-7i%-7i%-7i%-7i%-7i%-7i%-7i%-7i"),
			*Filename.RightPad( 72 ), NumMatchedOuters, NumLevelOuters, NumPackageOuters, NumClassOuters, NumNullOuters, NumDefaultObjectOuters, NumPSRequiredModule, NumOther, NumTotal );

		// Clean up.
		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}
IMPLEMENT_CLASS(UExamineOutersCommandlet);


/*-----------------------------------------------------------------------------
	URebuildMapCommandlet commandlet.
-----------------------------------------------------------------------------*/

INT URebuildMapCommandlet::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;
	ParseCommandLine( *Params, Tokens, Switches);

	// Rebuilding a map is a very slow operation and there are bound to be bugs in the map transition code so we only support
	// a single map (including sublevels) at a time and require multiple maps to be handled at the calling level via e.g. a
	// batch file.

	if( Tokens.Num() == 0 )
	{
		warnf(NAME_Error, TEXT("Missing map name argument!"));
	}
	else
	{
		//Token 1부터 cluster로 사용
		//check(Tokens.Num()==1);

		// See whether filename was found in cache.		
		FFilename Filename;	
		if( GPackageFileCache->FindPackageFile( *Tokens(0), NULL, Filename ) )
		{
			// Load the map file.
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
			if( Package )
			{
				// Find the world object inside the map file.
				UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
				if( World )
				{
					// Set loaded world as global world object and add to root set so the lighting rebuild
					// code doesn't garbage collect it.
					GWorld = World;
					GWorld->AddToRoot();
					GWorld->Init();
				
					// Mark all sublevels as being visible in the Editor so they get rebuilt.
					for( INT LevelIndex=0; LevelIndex<GWorld->GetWorldInfo()->StreamingLevels.Num(); LevelIndex++ )
					{
						ULevelStreaming* LevelStreamingObject = GWorld->GetWorldInfo()->StreamingLevels(LevelIndex);
						if( LevelStreamingObject )
						{
							LevelStreamingObject->bShouldBeVisibleInEditor = TRUE;
						}
					}
			
					// Associate sublevels, loading and making them visible.
					GWorld->UpdateLevelStreaming();
					GWorld->FlushLevelStreaming();

					// Update componets for all levels.
					GWorld->UpdateComponents(FALSE);

					// Rebuild lighting.
					DOUBLE LightingStartTime = appSeconds();
					FLightingBuildOptions Options;

					Options.bUseRadiosityLighting = Switches.ContainsItem( TEXT("radiosity") );
					Options.bUseRayTraceLighting = Switches.ContainsItem( TEXT("raytrace") );
					Options.bUseMultiThread = Switches.ContainsItem( TEXT("multithread") );
					Options.bExtra = TRUE;
					Options.bFast = FALSE;
					Options.bBakeAmbientCubes = TRUE;
					Options.AmbientType = 2;
					Options.NumBounce = 1;

					Options.Clusters.Append( Tokens );

					Options.Clusters.Remove( 0 );

					GEditor->BuildLighting( Options );

					warnf(TEXT("Building lighting took %5.2f minutes."), (appSeconds() - LightingStartTime) / 60 );

#if 0
					// Rebuild pathing.
					DOUBLE PathingStartTime = appSeconds();
					FPathBuilder Builder;
					Builder.DefinePaths( TRUE, FALSE );
					warnf(TEXT("Building paths took    %5.2f minutes."), (appSeconds() - PathingStartTime) / 60 );
#endif

					// Iterate over all levels in the world and save them.
					for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
					{
						ULevel*			Level			= GWorld->Levels(LevelIndex);
						check(Level);					
						UWorld*			World			= CastChecked<UWorld>(Level->GetOuter());
						ULinkerLoad*	Linker			= World->GetLinker();
						check(Linker);
						FString			LevelFilename	= Linker->Filename;

						// Save sublevel.
						UObject::SavePackage( World->GetOutermost(), World, 0, *LevelFilename, GWarn );
					}

					// Remove from root again.
					GWorld->RemoveFromRoot();
				}
				// Couldn't find the world object in the loaded package. Most likely because it's not a map.
				else
				{
					warnf(NAME_Log, TEXT("Cannot find world object in '%s'! Is it a map file?"), *Filename);
				}
			}
			// Couldn't load the package for some reason.
			else
			{
				warnf(NAME_Log, TEXT("Error loading '%s'!"), *Filename);
			}
		}
	}

	return 0;
}
IMPLEMENT_CLASS(URebuildMapCommandlet);

/*-----------------------------------------------------------------------------
	UTestCompressionCommandlet commandlet.
-----------------------------------------------------------------------------*/

extern DOUBLE GTotalUncompressTime;

/**
 * Run a compression/decompress test with the given package and compression options
 *
 * @param PackageName		The package to compress/decompress
 * @param Flags				The options for compression
 * @param UncompressedSize	The size of the uncompressed package file
 * @param CompressedSize	The options for compressed package file
 */
void UTestCompressionCommandlet::RunTest(const FFilename& PackageName, ECompressionFlags Flags, DWORD& UncompressedSize, DWORD& CompressedSize)
{
	SET_WARN_COLOR(COLOR_WHITE);
	warnf(TEXT(""));
	warnf(TEXT("Running Compression Test..."));

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Package:"));
	SET_WARN_COLOR(COLOR_GRAY);
	warnf(TEXT("  %s"), *PackageName.GetCleanFilename());

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Options:"));
	SET_WARN_COLOR(COLOR_GRAY);
	if (Flags & COMPRESS_LZO)
	{
		warnf(TEXT("  LZO"));
	}
	if (Flags & COMPRESS_ZLIB)
	{
		warnf(TEXT("  ZLIB"));
	}
	if (Flags & COMPRESS_BiasMemory)
	{
		warnf(TEXT("  Compressed for memory"));
	}
	if (Flags & COMPRESS_BiasSpeed)
	{
		warnf(TEXT("  Compressed for decompression speed"));
	}
#if _DEBUG
	warnf(TEXT("  Debug build"));
#elif FINAL_RELEASE
	warnf(TEXT("  Final release build"));
#else
	warnf(TEXT("  Release build"));
#endif


	// Generate new filename by swapping out extension.
	FFilename DstPackageFilename = FString::Printf(TEXT("%s%s%s.compressed_%x"), 
		*PackageName.GetPath(),
		PATH_SEPARATOR,
		*PackageName.GetBaseFilename(),
		(DWORD)Flags);

	// Create file reader and writers.
	FArchive* Reader = GFileManager->CreateFileReader( *PackageName );
	FArchive* Writer = GFileManager->CreateFileWriter( *DstPackageFilename );
	check( Reader );
	check( Writer );

	// Figure out filesize.
	INT ReaderTotalSize	= Reader->TotalSize();
	// Creat buffers for serialization (Src) and comparison (Dst).
	BYTE* SrcBuffer			= (BYTE*) appMalloc( ReaderTotalSize );
	BYTE* DstBuffer			= (BYTE*) appMalloc( ReaderTotalSize );
	BYTE* DstBufferAsync	= (BYTE*) appMalloc( ReaderTotalSize );

	DOUBLE StartRead = appSeconds();
	Reader->Serialize( SrcBuffer, ReaderTotalSize );
	DOUBLE StopRead = appSeconds();

	DOUBLE StartCompress = appSeconds();
	Writer->SerializeCompressed( SrcBuffer, ReaderTotalSize, Flags );
	DOUBLE StopCompress = appSeconds();

	// Delete (and implicitly flush) reader and writers.
	delete Reader;
	delete Writer;

	// Figure out compressed size (and propagate uncompressed for completeness)
	CompressedSize		= GFileManager->FileSize( *DstPackageFilename );
	UncompressedSize	= ReaderTotalSize;

	// Load compressed via FArchive
	Reader = GFileManager->CreateFileReader( *DstPackageFilename );

	GTotalUncompressTime = 0;
	Reader->SerializeCompressed( DstBuffer, 0, Flags );
	DOUBLE UncompressTime = GTotalUncompressTime;
	delete Reader;
	
	// Load compressed via async IO system.
	FIOSystem* IO = GIOManager->GetIOSystem( IOSYSTEM_GenericAsync );
	check(GIOManager);
	FThreadSafeCounter Counter(1);
	GTotalUncompressTime = 0;
	IO->LoadCompressedData( DstPackageFilename, 0, CompressedSize, UncompressedSize, DstBufferAsync, Flags, &Counter, AIOP_Normal );
	// Wait till it completed.
	while( Counter.GetValue() > 0 )
	{
		appSleep(0.05f);
	}
	DOUBLE AsyncUncompressTime = GTotalUncompressTime;

	// Compare results and assert if they are different.
	for( DWORD Count=0; Count<UncompressedSize; Count++ )
	{
		BYTE Src		= SrcBuffer[Count];
		BYTE Dst		= DstBuffer[Count];
		BYTE DstAsync	= DstBufferAsync[Count];
		check( Src == Dst );
		check( Src == DstAsync );
	}

	appFree( SrcBuffer );
	appFree( DstBuffer );
	appFree( DstBufferAsync );


	SET_WARN_COLOR(COLOR_WHITE);
	warnf(TEXT(""));
	warnf(TEXT("Complete!"));

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Size information:"));
	SET_WARN_COLOR(COLOR_GRAY);
	warnf(TEXT("  Uncompressed Size:     %.3fMB"), (FLOAT)UncompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Compressed Size:       %.3fMB"), (FLOAT)CompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Ratio:                 %.3f%%"), ((FLOAT)CompressedSize / (FLOAT)UncompressedSize) * 100.0f);
	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT("Time information:"));
	SET_WARN_COLOR(COLOR_GRAY);
	warnf(TEXT("  Read time:             %.3fs"), StopRead - StartRead);
	warnf(TEXT("  Compress time:         %.3fs"), StopCompress - StartCompress);
	warnf(TEXT("  Decompress time:       %.3fs"), UncompressTime);
	warnf(TEXT("  Async Decompress time: %.3fs"), AsyncUncompressTime);
}

INT UTestCompressionCommandlet::Main(const FString& Params)
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;
	ParseCommandLine( *Params, Tokens, Switches);

	// a list of all the different tests to run (as defined by the flags)
	TArray<DWORD> CompressionTests;
	UBOOL bAllTests = ParseParam(*Params, TEXT("alltests"));

	if (bAllTests || ParseParam(*Params, TEXT("zlibtest")))
	{
		CompressionTests.AddItem(COMPRESS_ZLIB);
	}
	if (bAllTests || ParseParam(*Params, TEXT("normaltest")))
	{
		CompressionTests.AddItem(COMPRESS_LZO);
	}
	if (bAllTests || ParseParam(*Params, TEXT("speedtest")))
	{
		CompressionTests.AddItem(COMPRESS_LZO | COMPRESS_BiasSpeed);
	}
	if (bAllTests || ParseParam(*Params, TEXT("sizetest")))
	{
		CompressionTests.AddItem(COMPRESS_LZO | COMPRESS_BiasMemory);
	}

	// list of all files to compress
	TArray<FString> FilesToCompress;

	if (ParseParam(*Params, TEXT("allpackages")))
	{
		FilesToCompress = GPackageFileCache->GetPackageFileList();
	}
	else
	{
		// iterate over all passed in tokens looking for packages
		for (INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			FFilename PackageFilename;
			// See whether filename was found in cache.		
			if (GPackageFileCache->FindPackageFile(*Tokens(TokenIndex), NULL, PackageFilename))
			{
				FilesToCompress.AddItem(*PackageFilename);
			}
		}
	}

	// keep overall stats
	QWORD TotalUncompressedSize = 0;
	QWORD TotalCompressedSize = 0;

	// Iterate over all files doing stuff.
	for (INT FileIndex = 0; FileIndex < FilesToCompress.Num(); FileIndex++)
	{
		for (INT TestIndex = 0; TestIndex < CompressionTests.Num(); TestIndex++)
		{
			DWORD UncompressedSize = 0;
			DWORD CompressedSize = 0;

			// run compression test
			RunTest(FilesToCompress(FileIndex), (ECompressionFlags)CompressionTests(TestIndex), UncompressedSize, CompressedSize);

			// update stats
			TotalUncompressedSize += UncompressedSize;
			TotalCompressedSize += CompressedSize;
		}
	}

	SET_WARN_COLOR(COLOR_YELLOW);
	warnf(TEXT(""));
	warnf(TEXT(""));
	warnf(TEXT("OVERALL STATS"));
	CLEAR_WARN_COLOR();
	warnf(TEXT("  Total Uncompressed:    %.3fMB"), (FLOAT)TotalUncompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Total Compressed:      %.3fMB"), (FLOAT)TotalCompressedSize / (1024.0f * 1024.0f));
	warnf(TEXT("  Total Ratio:           %.3f%%"), ((FLOAT)TotalCompressedSize / (FLOAT)TotalUncompressedSize) * 100.0f);

	return 0;
}
IMPLEMENT_CLASS(UTestCompressionCommandlet);


/*-----------------------------------------------------------------------------
	UFindDuplicateKismetObjectsCommandlet.
-----------------------------------------------------------------------------*/
INT UFindDuplicateKismetObjectsCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// skip the assert when a package can not be opened
	UBOOL bDeleteDuplicates		= ParseParam(Parms,TEXT("clean"));
	UBOOL bMapsOnly				= ParseParam(Parms,TEXT("mapsonly"));
	UBOOL bNonMapsOnly			= ParseParam(Parms,TEXT("nonmapsonly"));

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if( !PackageList.Num() )
		return 0;

	INT GCIndex = 0;

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		const FFilename& Filename = PackageList(PackageIndex);
		// if we only want maps, skip non-maps
		if (bMapsOnly && Filename.GetExtension() != FURL::DefaultMapExt)
		{
			continue;
		}
		if (bNonMapsOnly && Filename.GetExtension() == FURL::DefaultMapExt)
		{
			continue;
		}

		UObject::BeginLoad();
		ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
		UObject::EndLoad();

		// see if we have any sequences
		UBOOL bHasSequences = FALSE;
		FName SequenceName(TEXT("Sequence"));
		for (INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ExportIndex++)
		{
			if (Linker->GetExportClassName(ExportIndex) == SequenceName)
			{
				bHasSequences = TRUE;
				break;
			}
		}

		if (!bHasSequences)
		{
			SET_WARN_COLOR(COLOR_DARK_YELLOW);
			warnf(TEXT("Skipping %s (no sequences)"), *Filename);
			CLEAR_WARN_COLOR();
			continue;
		}

		warnf(TEXT("Processing %s"), *Filename);
		// open the package
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn|LOAD_Quiet );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		UBOOL bDirty = FALSE;
		// look for kismet sequences
		for (TObjectIterator<USequence> It; It; ++It)
		{
			if (!It->IsIn(Package))
			{
				continue;
			}
			TArray<USequenceObject*>& Objects = It->SequenceObjects;

			// n^2 operation looking for duplicate
			for (INT ObjIndex1 = 0; ObjIndex1 < Objects.Num(); ObjIndex1++)
			{
				USequenceObject* Obj1 = Objects(ObjIndex1);
				for (INT ObjIndex2 = ObjIndex1 + 1; ObjIndex2 < Objects.Num(); ObjIndex2++)
				{
					USequenceObject* Obj2 = Objects(ObjIndex2);
					if (Obj1->ObjPosX == Obj2->ObjPosX && Obj1->ObjPosY == Obj2->ObjPosY && 
						Obj1->DrawWidth == Obj2->DrawWidth && Obj1->DrawHeight == Obj2->DrawHeight &&
						Obj1->GetClass() == Obj2->GetClass())
					{
						SET_WARN_COLOR(COLOR_GREEN);
						warnf(TEXT("Two duplicate sequence objects: '%s' and '%s'"), *Obj1->GetFullName(), *Obj2->GetPathName());
						CLEAR_WARN_COLOR();
						if (bDeleteDuplicates)
						{
							Objects.RemoveItem(Obj2);
							bDirty = TRUE;
						}
					}
				}
			}
		}

		if (bDirty)
		{
			SavePackageHelper(Package, Filename);
		}

//		if (bDirty || (PackageIndex % 10) == 0)
		{
			UObject::CollectGarbage(RF_Native);
		}
	}
	return 0;
}
IMPLEMENT_CLASS(UFindDuplicateKismetObjectsCommandlet);

/*-----------------------------------------------------------------------------
	UFixupEmitters commandlet.
-----------------------------------------------------------------------------*/
INT UFixupEmittersCommandlet::Main( const FString& Params )
{
	// 
	FString			Token;
	const TCHAR*	CommandLine		= appCmdLine();
	UBOOL			bForceCheckAll	= FALSE;
	FString			FileToFixup(TEXT(""));

	while (ParseToken(CommandLine, Token, 1))
	{
		if (Token == TEXT("-FORCEALL"))
		{
			bForceCheckAll = TRUE;
		}
		else
		{
			FString PackageName;
			if (GPackageFileCache->FindPackageFile(*Token, NULL, PackageName))
			{
				FileToFixup = FFilename(Token).GetBaseFilename();
			}
			else
			{
				debugf(TEXT("Package %s Not Found."), *Token);
				return -1;
			}
		}
	}


	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();
	if (!PackageList.Num())
		return 0;

	// Determine the list of packages we have to touch...
	TArray<FString> RequiredPackageList;
	RequiredPackageList.Empty(PackageList.Num());

	if (FileToFixup != TEXT(""))
	{
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (appStricmp(*Filename.GetBaseFilename(), *FileToFixup) == 0)
			{
				new(RequiredPackageList)FString(PackageList(PackageIndex));
				break;
			}
		}
	}
	else
	{
		// Iterate over all packages.
		for (INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++)
		{
			const FFilename& Filename = PackageList(PackageIndex);
			if (Filename.GetExtension() == TEXT("U"))
			{
				warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
				continue;
			}

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
			if (Package)
			{
				if (bForceCheckAll)
				{
					new(RequiredPackageList)FString(PackageList(PackageIndex));
				}
				else
				{
					TArray<UPackage*> Packages;
					Packages.AddItem(Package);

					UBOOL bIsDirty = FALSE;
					// Fix up ParticleSystems
					for (TObjectIterator<UParticleSystem> It; It && !bIsDirty; ++It)
					{
						if (It->IsIn(Package))
						{
							UParticleSystem* PartSys = Cast<UParticleSystem>(*It);
							check(PartSys->GetOuter());

							for (INT i = PartSys->Emitters.Num() - 1; (i >= 0) && !bIsDirty; i--)
							{
								UParticleEmitter* Emitter = PartSys->Emitters(i);
								if (Emitter == NULL)
									continue;

								UObject* EmitterOuter = Emitter->GetOuter();
								if (EmitterOuter != PartSys)
								{
									bIsDirty	= TRUE;
								}

								if (!bIsDirty)
								{
									for (INT LODIndex = 0; (LODIndex < Emitter->LODLevels.Num()) && !bIsDirty; LODIndex++)
									{
										UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
										if (LODLevel == NULL)
										{
											continue;
										}

										UParticleModule* Module;
										Module = LODLevel->TypeDataModule;
										if (Module)
										{
											if (Module->GetOuter() != EmitterOuter)
											{
												bIsDirty	= TRUE;
											}
										}

										if (!bIsDirty)
										{
											for (INT i = 0; i < LODLevel->Modules.Num(); i++)
											{
												Module = Emitter->Modules(i);
												if (Module)
												{
													if (Module->GetOuter() != EmitterOuter)
													{
														bIsDirty	= TRUE;
														break;
													}
												}
											}
										}
									}
								}
							}

							if (bIsDirty)
							{
								new(RequiredPackageList)FString(PackageList(PackageIndex));
							}
							else
							{
								warnf(NAME_Log, TEXT("ALREADY CONVERTED? Skipping package %32s"), *Filename);
							}
							break;
						}
					}

					// Fix up ParticleEmitters
					for (TObjectIterator<UParticleEmitter> It; It && !bIsDirty; ++It)
					{
						if (It->IsIn(Package))
						{
							UParticleEmitter* Emitter = *It;
							UParticleSystem* PartSys = Cast<UParticleSystem>(Emitter->GetOuter());
							if (PartSys == NULL)
							{
								bIsDirty	= TRUE;
							}

							if (!bIsDirty)
							{
								UObject* EmitterOuter = Emitter->GetOuter();
								if (EmitterOuter != PartSys)
								{
									bIsDirty	= TRUE;
								}

								if (!bIsDirty)
								{
									for (INT LODIndex = 0; (LODIndex < Emitter->LODLevels.Num()) && !bIsDirty; LODIndex++)
									{
										UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
										if (LODLevel == NULL)
										{
											continue;
										}

										UParticleModule* Module;
										Module = LODLevel->TypeDataModule;
										if (Module)
										{
											if (Module->GetOuter() != EmitterOuter)
											{
												bIsDirty	= TRUE;
											}
										}

										if (!bIsDirty)
										{
											for (INT i = 0; i < LODLevel->Modules.Num(); i++)
											{
												Module = Emitter->Modules(i);
												if (Module)
												{
													if (Module->GetOuter() != EmitterOuter)
													{
														bIsDirty	= TRUE;
														break;
													}
												}
											}
										}
									}
								}
							}
						}
						if (bIsDirty)
						{
							new(RequiredPackageList)FString(PackageList(PackageIndex));
						}
						else
						{
							warnf(NAME_Log, TEXT("ALREADY CONVERTED? Skipping package %32s"), *Filename);
						}
						break;
					}
				}
			}
			UObject::CollectGarbage(RF_Native);
		}
	}

	if (!RequiredPackageList.Num())
	{
		warnf(NAME_Log, TEXT("No emitter fixups required!"));
		return 0;
	}

	warnf(NAME_Log, TEXT("Emitter fixups required for %d packages..."), RequiredPackageList.Num());

	// source control object
	FSourceControlIntegration* SCC = new FSourceControlIntegration;;

	// Iterate over all packages.
	for (INT PackageIndex = 0; PackageIndex < RequiredPackageList.Num(); PackageIndex++)
	{
		const FFilename& Filename = RequiredPackageList(PackageIndex);

		// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
		UPackage* Package = CastChecked<UPackage>(UObject::LoadPackage(NULL, *Filename, 0));
		check(Package);

		TArray<UPackage*> Packages;
		Packages.AddItem(Package);

		if (GFileManager->IsReadOnly( *Filename))
		{
			// update scc status
			SCC->Update(Packages);

			// can we check it out?
			if (Package->SCCCanCheckOut())
			{
				// The package is still available, so do the check out.
				SCC->CheckOut(Package);
			}

			// if the checkout failed for any reason, this will still be readonly, so we can't clean it up
			const INT	FailCount	= 3;
			INT			iCount		= 0;
			while (GFileManager->IsReadOnly( *Filename) && (iCount < FailCount))
			{
				if (iCount == (FailCount - 1))
				{
					// No warning
				}
				else
				if ((iCount + 1) == (FailCount - 1))
				{
					appMsgf(AMT_OK, TEXT("LAST CHANCE - ABOUT TO SKIP!\nCouldn't check out %s, manually do it, sorry!"), *Filename);
				}
				else
				{
					appMsgf(AMT_OK, TEXT("Couldn't check out %s, manually do it, sorry!"), *Filename);
				}
				iCount++;
			}

			if (iCount >= FailCount)
			{
				appMsgf(AMT_OK, TEXT("Skipping %s"), *Filename);
				continue;
			}
		}
		
		warnf(NAME_Log, TEXT("Loading %s"), *Filename);

		if (Package)
		{
			UBOOL	bIsDirty = FALSE;

			// Fix up ParticleSystems
			for (TObjectIterator<UParticleSystem> It; It; ++It)
			{
				if (It->IsIn(Package))
				{
					UParticleSystem* PartSys = Cast<UParticleSystem>(*It);
					check(PartSys->GetOuter());
					
					UBOOL bResetTabs = FALSE;

					for (INT EmitterIndex = PartSys->Emitters.Num() - 1; EmitterIndex >= 0; EmitterIndex--)
					{
						UParticleEmitter* Emitter = PartSys->Emitters(EmitterIndex);
						if (Emitter == NULL)
							continue;

						UObject* EmitterOuter = Emitter->GetOuter();
						if (EmitterOuter != PartSys)
						{
							// Replace the outer...
							Emitter->Rename(NULL, PartSys, REN_ForceNoResetLoaders);
							debugf(TEXT("Renaming particle emitter %32s from Outer %32s to Outer %32s"),
								*(Emitter->GetName()), *(EmitterOuter->GetName()), *(GetName()));
							bResetTabs = TRUE;
							bIsDirty = TRUE;
						}

						UParticleModule* Module;
						UParticleLODLevel* LODLevel;

						for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
						{
							LODLevel = Emitter->LODLevels(LODIndex);
							if (LODLevel == NULL)
							{
								continue;
							}

							Module = LODLevel->TypeDataModule;
							if (Module)
							{
								UObject* ModuleOuter = Module->GetOuter();
								if (ModuleOuter != EmitterOuter)
								{
									// Replace the outer...
									debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
										*(Module->GetName()), *(ModuleOuter->GetName()), *(EmitterOuter->GetName()));
									Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
									bResetTabs = TRUE;
									bIsDirty = TRUE;
								}
							}

							for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
							{
								Module = LODLevel->Modules(ModuleIndex);
								if (Module)
								{
									UObject* ModuleOuter = Module->GetOuter();
									if (ModuleOuter != EmitterOuter)
									{
										// Replace the outer...
										debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
											*(Module->GetName()), *(ModuleOuter->GetName()), 
											*(EmitterOuter->GetName()));
										Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
										bResetTabs = TRUE;
										bIsDirty = TRUE;
									}
								}
							}
						}
					}
					if (bResetTabs)
					{
						if (PartSys->CurveEdSetup)
						{
							PartSys->CurveEdSetup->ResetTabs();
						}
					}
				}
			}

			// Fix up ParticleEmitters
			for (TObjectIterator<UParticleEmitter> It; It; ++It)
			{
				if (It->IsIn(Package))
				{
					UParticleEmitter* Emitter = *It;
					UParticleSystem* PartSys = Cast<UParticleSystem>(Emitter->GetOuter());
					UObject* EmitterOuter = Emitter->GetOuter();

					UParticleModule* Module;
					UParticleLODLevel* LODLevel;
					for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
					{
						LODLevel = Emitter->LODLevels(LODIndex);
						if (LODLevel == NULL)
						{
							continue;
						}

						Module = LODLevel->TypeDataModule;
						if (Module)
						{
							UObject* ModuleOuter = Module->GetOuter();
							if (ModuleOuter != EmitterOuter)
							{
								// Replace the outer...
								debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
									*(Module->GetName()), *(ModuleOuter->GetName()), *(EmitterOuter->GetName()));
								Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
								bIsDirty = TRUE;
							}
						}

						for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
						{
							Module = LODLevel->Modules(ModuleIndex);
							if (Module)
							{
								UObject* ModuleOuter = Module->GetOuter();
								if (ModuleOuter != EmitterOuter)
								{
									// Replace the outer...
									debugf(TEXT("  Renaming particle module %32s from Outer %32s to Outer %32s"),
										*(Module->GetName()), *(ModuleOuter->GetName()), 
										*(EmitterOuter->GetName()));
									Module->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders);
									bIsDirty = TRUE;
								}
							}
						}
					}
				}
			}

			if (bIsDirty)
			{
				UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
				if( World )
				{	
					UObject::SavePackage( Package, World, 0, *Filename, GWarn );
				}
				else
				{
					UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
				}
			}
		}
		UObject::CollectGarbage(RF_Native);
	}

	delete SCC;

	return 0;
}
IMPLEMENT_CLASS(UFixupEmittersCommandlet)

/**
 *	UFixupSourceUVsCommandlet.
 *
 *	Some source UVs in static meshes were corrupted on import - this commandlet attempts to fix them.
 *
 *  The general pattern is the UV coords appear as [0][0*n], [0][1*n] and [0][2*n] when they should be [0][0], [1][0] and [2][0]
 *  I also tried when the UV coords appear as [0][0], [1][1] and [2][0] but this is not fixable
 *
 *  I have tried to write it to not give false positives.
 */
INT UFixupSourceUVsCommandlet::Main( const FString & Params )
{
	// Parse command line args.
	FString			FullyQualified;
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TArray<FString> FilesInPath;
	UBOOL			bResult, bPackageDirty;
	INT				Incorrect, Failed, FixType, i;

	const TCHAR * Parms = *Params;
	ParseCommandLine( Parms, Tokens, Switches );

	if( Tokens.Num() > 0 )
	{
		for( i = 0; i < Tokens.Num(); i++ )
		{
			GPackageFileCache->FindPackageFile( *Tokens( i ), NULL, FullyQualified );
			new( FilesInPath ) FString( FullyQualified );
		}
	}
	else
	{
		FilesInPath = GPackageFileCache->GetPackageFileList();
	}

	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT( "No packages found" ) );
		return 1;
	}

	Incorrect = 0;
	Failed = 0;

	// Iterate over all files checking for incorrect source UVs
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		const FFilename & Filename = FilesInPath( FileIndex );
		warnf( NAME_Log, TEXT( "Loading %s" ), *Filename );
		bPackageDirty = FALSE;

		UPackage * Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT( "Error loading %s!" ), *Filename );
			continue;
		}

		for( TObjectIterator<UStaticMesh>It; It; ++It )
		{
			UStaticMesh * StaticMesh = *It;
			if( StaticMesh->IsIn( Package ) )
			{
				FStaticMeshRenderData * StaticMeshRenderData = &StaticMesh->LODModels( 0 );
				FStaticMeshTriangle * RawTriangleData = ( FStaticMeshTriangle * )StaticMeshRenderData->RawTriangles.Lock( LOCK_READ_ONLY );

				if( RawTriangleData->NumUVs < 0 || RawTriangleData->NumUVs > 8 )
				{
					warnf( NAME_Warning, TEXT( "Bad number of raw UV channels (%d) in %s" ), RawTriangleData->NumUVs, *StaticMesh->GetName() );
					StaticMeshRenderData->RawTriangles.Unlock();
					continue;	
				}

				bResult = CheckUVs( StaticMeshRenderData, RawTriangleData );
				StaticMeshRenderData->RawTriangles.Unlock();

				if( !bResult )
				{
					warnf( NAME_Log, TEXT( "UV source data incorrect in %s - fixing" ), *StaticMesh->GetName() );
					Incorrect++;

					FStaticMeshTriangle* RawTriangleData = ( FStaticMeshTriangle* )StaticMeshRenderData->RawTriangles.Lock( LOCK_READ_WRITE );
					FixType = CheckFixable( RawTriangleData, StaticMeshRenderData->RawTriangles.GetElementCount() );
					if( FixType < 0 )
					{
						warnf( NAME_Warning, TEXT( "UV source data degenerate or invalid in %s - please reimport" ), *StaticMesh->GetName() );
						Failed++;
					}
					else
					{
						FixupUVs( FixType, RawTriangleData, StaticMeshRenderData->RawTriangles.GetElementCount() );
						bPackageDirty = TRUE;
					}

					StaticMeshRenderData->RawTriangles.Unlock();
				}
			}
		}

		// Write out the package if necessary
		if( bPackageDirty )
		{
			bResult = SavePackageHelper(Package, Filename);

			if( !bResult )
			{
				warnf( NAME_Log, TEXT( "Failed to save: %s" ), *Filename );
			}
		}

		UObject::CollectGarbage( RF_Native );
	}

	warnf( NAME_Log, TEXT( "%d source static mesh(es) with incorrect UV data, %d failed to fix up" ), Incorrect, Failed );
	return( 0 );
}

/**
 *	FixupUVs
 *
 *  Actually remap the UV coords
 */
void UFixupSourceUVsCommandlet::FixupUVs( INT step, FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT			i, j, k;

	for( i = 0; i < NumRawTriangles; i++ )
	{
		check( RawTriangleData[i].NumUVs < 3 );

		// Copy in the valid UVs
		for( j = 0; j < RawTriangleData[i].NumUVs; j++ )
		{
			for( k = 0; k < 3; k++ )
			{
				RawTriangleData[i].UVs[k][j] = RawTriangleData[i].UVs[0][k * step];
			}
		}

		// Zero out the remainder
		for( j = RawTriangleData[i].NumUVs; j < 8; j++ )
		{
			for( k = 0; k < 3; k++ )
			{
				RawTriangleData[i].UVs[k][j].X = 0.0f;
				RawTriangleData[i].UVs[k][j].Y = 0.0f;
			}
		}
	}
}

/**
 *	ValidateUVChannels
 *
 *  Make sure there are a valid number of UV channels that can be fixed up
 */
UBOOL UFixupSourceUVsCommandlet::ValidateUVChannels( const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT			i, UVChannelCount;
	FVector2D	UVsBad[8][3];

	UVChannelCount = RawTriangleData[0].NumUVs;
	check( UVChannelCount > 0 );
	if( UVChannelCount > 2 )
	{
		return( FALSE );
	}

	// Check to make sure all source tris have the same number of UV channels
	for( i = 0; i < NumRawTriangles; i++ )
	{
		if( RawTriangleData[i].NumUVs != UVChannelCount )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 *	CheckFixableType
 *
 *  Check to see if the UVs were serialized out incorrectly
 */
UBOOL UFixupSourceUVsCommandlet::CheckFixableType( INT step, const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT			i, j, UVChannelCount;
	INT			NullCounts[6];

	UVChannelCount = RawTriangleData[0].NumUVs;
	for( j = 0; j < UVChannelCount * 3; j++ )
	{
		NullCounts[j] = 0;
	}

	for( i = 0; i < NumRawTriangles; i++ )
	{
		// Check to make sure the UVs are all valid
		for( j = 0; j < UVChannelCount * 3; j++ )
		{
			if( appIsNaN( RawTriangleData[i].UVs[0][j * step].X ) || appIsNaN( RawTriangleData[i].UVs[0][j * step].Y ) )
			{
				return( FALSE );
			}

			if( !appIsFinite( RawTriangleData[i].UVs[0][j * step].X ) && !appIsFinite( RawTriangleData[i].UVs[0][j * step].Y ) )
			{
				return( FALSE );
			}

			// Cull 0.0 and #DEN
			if( fabs( RawTriangleData[i].UVs[0][j * step].X ) > SMALL_NUMBER || fabs( RawTriangleData[i].UVs[0][j * step].Y ) > SMALL_NUMBER )
			{
				continue;
			}

			NullCounts[j]++;
		}
	}

	// If any coord is zeroed out more than half the time, it's likley incorrect
	for( j = 0; j < UVChannelCount * 3; j++ )
	{
		if( NullCounts[j] > NumRawTriangles / 2 )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}

/**
 *	CheckFixable
 */
INT UFixupSourceUVsCommandlet::CheckFixable( const FStaticMeshTriangle * RawTriangleData, INT NumRawTriangles )
{
	INT		i;

	if( !ValidateUVChannels( RawTriangleData, NumRawTriangles ) )
	{
		return( -1 );
	}

	for( i = 1; i < 4; i++ )
	{
		if( CheckFixableType( i, RawTriangleData, NumRawTriangles ) )
		{
			return( i );
		}
	}

	return( -1 );
}

/**
 *	FindUV
 *
 *  Find if a UV coord exists anywhere in the source static mesh data
 */
UBOOL UFixupSourceUVsCommandlet::FindUV( const FStaticMeshTriangle * RawTriangleData, INT UVChannel, INT NumRawTriangles, FVector2D & UV )
{
	INT		i;

	for( i = 0; i < NumRawTriangles; i++ )
	{
		if( RawTriangleData[i].UVs[0][UVChannel] == UV )
		{
			return( TRUE );
		}

		if( RawTriangleData[i].UVs[1][UVChannel] == UV )
		{
			return( TRUE );
		}

		if( RawTriangleData[i].UVs[2][UVChannel] == UV )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

/**
 *	CheckUVs
 *
 *  Iterate over the final mesh data and check to see if every element of UV data exists somewhere in the source static mesh
 */
UBOOL UFixupSourceUVsCommandlet::CheckUVs( FStaticMeshRenderData * StaticMeshRenderData, const FStaticMeshTriangle * RawTriangleData )
{
	// As the conversion from raw triangles to indexed elements is non trivial and we are checking for a transposition error, just make sure
	// every UV pair in the final data exists somewhere in the raw data.
	INT NumRawTriangles = StaticMeshRenderData->RawTriangles.GetElementCount();

	for( UINT i = 0; i < StaticMeshRenderData->VertexBuffer.GetNumTexCoords(); i++ )
	{
		for( UINT j = 0; j < StaticMeshRenderData->NumVertices; j++ )
		{
			if( !FindUV( RawTriangleData, i, NumRawTriangles, StaticMeshRenderData->VertexBuffer.VertexUV(j,i) ) )
			{
				return( FALSE );
			}
		}
	}
	
	return( TRUE );
}

IMPLEMENT_CLASS( UFixupSourceUVsCommandlet );

/*-----------------------------------------------------------------------------
	UWrangleContent.
-----------------------------------------------------------------------------*/

/** 
 * Helper struct to store information about a unreferenced object
 */
struct FUnreferencedObject
{
	/** Name of package this object resides in */
	FString PackageName;
	/** Full name of object */
	FString ObjectName;
	/** Size on disk as recorded in FObjectExport */
	INT SerialSize;

	/**
	 * Constrcutor for easy creation in a TArray
	 */
	FUnreferencedObject(const FString& InPackageName, const FString& InObjectName, INT InSerialSize)
	: PackageName(InPackageName)
	, ObjectName(InObjectName)
	, SerialSize(InSerialSize)
	{
	}
};

/**
 * Helper struct to store information about referenced objects insde
 * a package. Stored in TMap<> by package name, so this doesn't need
 * to store the package name 
 */
struct FPackageObjects
{
	/** All objected referenced in this package, and their class */
	TMap<FString, UClass*> ReferencedObjects;

	/** Was this package a fully loaded package, and saved right after being loaded? */
	UBOOL bIsFullyLoadedPackage;

	FPackageObjects()
	: bIsFullyLoadedPackage(FALSE)
	{
	}

};
	
FArchive& operator<<(FArchive& Ar, FPackageObjects& PackageObjects)
{
	Ar << PackageObjects.bIsFullyLoadedPackage;

	if (Ar.IsLoading())
	{
		INT NumObjects;
		FString ObjectName;
		FString ClassName;

		Ar << NumObjects;
		for (INT ObjIndex = 0; ObjIndex < NumObjects; ObjIndex++)
		{
			Ar << ObjectName << ClassName;
			UClass* Class = UObject::StaticLoadClass(UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_None, NULL);
			PackageObjects.ReferencedObjects.Set(*ObjectName, Class);
		}
	}
	else if (Ar.IsSaving())
	{
		INT NumObjects = PackageObjects.ReferencedObjects.Num();
		Ar << NumObjects;
		for (TMap<FString, UClass*>::TIterator It(PackageObjects.ReferencedObjects); It; ++It)
		{
			FString ObjectName, ClassName;
			ObjectName = It.Key();
			ClassName = It.Value()->GetPathName();

			Ar << ObjectName << ClassName;
		}
		
	}

	return Ar;
}

/**
 * Stores the fact that an object (given just a name) was referenced
 *
 * @param PackageName Name of the package the object lives in
 * @param ObjectName FullName of the object
 * @param ObjectClass Class of the object
 * @param ObjectRefs Map to store the object information in
 * @param bIsFullLoadedPackage TRUE if the packge this object is in was fully loaded
 */
void ReferenceObjectInner(const FString& PackageName, const FString& ObjectName, UClass* ObjectClass, TMap<FString, FPackageObjects>& ObjectRefs, UBOOL bIsFullyLoadedPackage)
{
	// look for an existing FPackageObjects
	FPackageObjects* PackageObjs = ObjectRefs.Find(*PackageName);
	// if it wasn't found make a new entry in the map
	if (PackageObjs == NULL)
	{
		PackageObjs = &ObjectRefs.Set(*PackageName, FPackageObjects());
	}

	// if either the package was already marked as fully loaded or it now is fully loaded, then
	// it will be fully loaded
	PackageObjs->bIsFullyLoadedPackage = PackageObjs->bIsFullyLoadedPackage || bIsFullyLoadedPackage;

	// add this referenced object to the map
	PackageObjs->ReferencedObjects.Set(*ObjectName, ObjectClass);

	// make sure the class is in the root set so it doesn't get GC'd, making the pointer we cached invalid
	ObjectClass->AddToRoot();
}

extern TArray<UObject*> GDLOObjects;
TArray<FString> GFailedPackages;

/**
 * Stores the fact that an object was referenced
 *
 * @param Object The object that was referenced
 * @param ObjectRefs Map to store the object information in
 * @param bIsFullLoadedPackage TRUE if the packge this object is in was fully loaded
 */
UBOOL ReferenceObject(UObject* Object, TMap<FString, FPackageObjects>& ObjectRefs, UBOOL bIsFullyLoadedPackage)
{
	FString PackageName = Object->GetOutermost()->GetName();

	// find the outermost non-upackage object, as it will be loaded later with all its subobjects
	while (Object->GetOuter() && Object->GetOuter()->GetClass() != UPackage::StaticClass())
	{
		Object = Object->GetOuter();
	}

	// make sure this object is valid (it's not in a script or native-only package)
	UBOOL bIsValid = TRUE;
	// can't be in a script packge or be a field/template in a native package, or a top level pacakge, or in the transient package
	if ((Object->GetOutermost()->PackageFlags & PKG_ContainsScript) ||
		Object->IsA(UField::StaticClass()) ||
		//Object->IsTemplate() ||
		Object->GetOuter() == NULL ||
		Object->IsIn(UObject::GetTransientPackage()))
	{
		bIsValid = FALSE;
	}

	if (bIsValid)
	{
		UObjectRedirector* Redirector = Cast<UObjectRedirector>( Object );
		if (Redirector && Redirector->DestinationObject)
		{
			ReferenceObject( Redirector->DestinationObject, ObjectRefs, bIsFullyLoadedPackage );
		}

		// save the reference
		ReferenceObjectInner(PackageName, Object->GetFullName(), Object->GetClass(), ObjectRefs, bIsFullyLoadedPackage);

		// add the other language versions
	    for (INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++)
		{
			// see if a localized package for this package exists
			FString LocPath;
			FString LocPackageName = PackageName + TEXT("_") + GKnownLanguageExtensions[LangIndex];

			INT Index;
			
			// 실패했어!
			if (GFailedPackages.FindItem( LocPackageName, Index ))
				continue;

			if (GPackageFileCache->FindPackageFile(*LocPackageName, NULL, LocPath))
			{
				// make the localized object name (it doesn't even have to exist)
				FString LocObjectName = FString::Printf(TEXT("%s %s.%s"), *Object->GetClass()->GetName(), *LocPackageName, *Object->GetPathName(Object->GetOutermost()));

				// save a reference to the (possibly existing) localized object (these won't have been in a fully loaded package)
				ReferenceObjectInner(LocPackageName, LocObjectName, Object->GetClass(), ObjectRefs, FALSE);
			}
			else
			{
				GFailedPackages.AddItem( LocPackageName );
			}
		}
	}

	return bIsValid;
}

/**
 * Take a package pathname and return a path for where to save the cutdown
 * version of the package. Will create the directory if needed.
 *
 * @param Filename Path to a package file
 * @param CutdownDirectoryName Name of the directory to put this package into
 *
 * @return Location to save the cutdown package
 */
FFilename MakeCutdownFilename(const FFilename& Filename, const TCHAR* CutdownDirectoryName=TEXT("CutdownPackages"))
{
	// replace the .. with ..\GAMENAME\CutdownContent
	FFilename CutdownDirectory = Filename.GetPath();
	CutdownDirectory = CutdownDirectory.Replace(TEXT("..\\"), *FString::Printf(TEXT("..\\%sGame\\%s\\"), appGetGameName(), CutdownDirectoryName));

	// make sure it exists
	GFileManager->MakeDirectory(*CutdownDirectory, TRUE);

	// return the full pathname
	return CutdownDirectory * Filename.GetCleanFilename();
}

/**
 * Removes editor-only data for a package
 *
 * @param Package THe package to remove editor date from all objects
 */
void StripEditorData(UPackage* Package)
{
	for( TObjectIterator<UObject> It; It; ++It )
	{
		if (It->IsIn(Package))
		{
			It->StripData(UE3::PLATFORM_Unknown);
		}
	}
}

void Cook(UPackage* Package, UE3::EPlatformType Platform)
{
	for( TObjectIterator<UObject> It; It; ++It )
	{
		if (It->IsIn(Package))
		{
			UObject* Object = *It;			

			UTexture2D*			Texture2D			= Cast<UTexture2D>(Object);
			UTextureMovie*		TextureMovie		= Cast<UTextureMovie>(Object);
			USoundNodeWave*		SoundNodeWave		= Cast<USoundNodeWave>(Object);
			UTerrainComponent*	TerrainComponent	= Cast<UTerrainComponent>(Object);
			UBrushComponent*	BrushComponent		= Cast<UBrushComponent>(Object);
			UParticleSystem*	ParticleSystem		= Cast<UParticleSystem>(Object);

			// Rebuild Novodex brush data for BrushComponents that are part of Volumes (eg blocking volumes)
			if (BrushComponent && !BrushComponent->IsTemplate() && BrushComponent->GetOwner() && BrushComponent->GetOwner()->IsVolumeBrush())
			{
				BrushComponent->BuildSimpleBrushCollision();
				BrushComponent->BuildPhysBrushData();
			}

			if (ParticleSystem)
			{
				// Cook out the thumbnail image - no reason to store it for gameplay.
				ParticleSystem->ThumbnailImageOutOfDate = TRUE;
				ParticleSystem->ThumbnailImage = NULL;
			}			

			if (SoundNodeWave)
			{
				::CookSoundNodeWave( SoundNodeWave, Platform );
			}

			if (Texture2D)
			{
				if( Texture2D->LODGroup == TEXTUREGROUP_UI )
				{
					if( Texture2D->Mips.Num() > 1 )
					{
						Texture2D->Mips.Remove( 1, Texture2D->Mips.Num() - 1 );
					}
				}
			}

			Object->StripData(Platform);
		}
	}
}

void TestDLOConsistency()
{	
}

extern TArray<UObject*> GDLOObjects;
void LoadDLOObjectsExplicitly()
{
	GDLOObjects.Empty();
	FName DLO( TEXT("LoadDLOs") );

	extern UBOOL GDoingExportDLO;

	GDoingExportDLO = TRUE;

	for ( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* pClass = *It;

		UObject* CDO = pClass->GetDefaultObject();
		if (!CDO)
			continue;

		UFunction* pFunction = CDO->FindFunction( DLO );
		if (pFunction)
		{	
			if (pFunction->HasAllFunctionFlags( FUNC_Static | FUNC_Event ))
			{
				debugf( NAME_Log, TEXT( "%s::LoadDLOs"), *pClass->GetFullName() ); 

				CDO->ProcessEvent( pFunction,NULL );
			}
		}
	}	

	GDoingExportDLO = FALSE;
}

/**
* Cooks out all static mesh actors in the specified package by re-attaching their StaticMeshComponents to
* a StaticMeshCollectionActor referenced by the world.
*
* @param	Package		the package being cooked
*/
void UWrangleContentCommandlet::CookStaticMeshActors( UPackage* Package )
{
	// 'Cook-out' material expressions on consoles
	// In this case, simply don't load them on the client
	// Keep 'parameter' types around so that we can get their defaults.
	//if ( (GCookingTarget & PLATFORM_Console) != 0 )
	{
		// only cook-out StaticMeshActors when cooking for console
		check(Package);

		// find all StaticMeshActors and static Light actors which are referenced by something in the map
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if ( World != NULL )
		{
			TArray<ULevel*> LevelsToSearch = World->Levels;

			// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
			if ( World->PersistentLevel != NULL )
			{
				LevelsToSearch.AddUniqueItem(World->PersistentLevel);
			}
			for ( INT LevelIndex = 0; LevelIndex < LevelsToSearch.Num(); LevelIndex++ )
			{
				ULevel* Level = LevelsToSearch(LevelIndex);

				// we need to remove all StaticMeshActors and static Light actors from the level's Actors array so that we don't
				// get false positives during our reference checking.
				// however, we'll need to restore any actors which don't get cooked out, so keep track of their indices
				TMap<AStaticMeshActor*,INT> StaticActorIndices;
				TLookupMap<AStaticMeshActor*> StaticMeshActors;

				// remove all StaticMeshActors from the level's Actor array so that we don't get false positives.
				for ( INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ActorIndex++ )
				{
					AActor* Actor = Level->Actors(ActorIndex);
					if ( Actor != NULL && Actor->IsA(AStaticMeshActor::StaticClass()) )
					{
						AStaticMeshActor* StaticMeshActor = static_cast<AStaticMeshActor*>(Actor);

						StaticMeshActors.AddItem(StaticMeshActor);
						StaticActorIndices.Set(StaticMeshActor, ActorIndex);
						Level->Actors(ActorIndex) = NULL;
					}
				}

				// now use the object reference collector to find the static mesh actors that are still being referenced
				TArray<AStaticMeshActor*> ReferencedStaticMeshActors;
				TArchiveObjectReferenceCollector<AStaticMeshActor> SMACollector(&ReferencedStaticMeshActors, Package, FALSE, TRUE, TRUE, TRUE);
				Level->Serialize( SMACollector );

				// remove any StaticMeshActors which aren't valid for cooking out
				TFindObjectReferencers<AStaticMeshActor> StaticMeshReferencers(ReferencedStaticMeshActors, Package);
				for ( INT ActorIndex = ReferencedStaticMeshActors.Num() - 1; ActorIndex >= 0; ActorIndex-- )
				{
					AStaticMeshActor* StaticMeshActor = ReferencedStaticMeshActors(ActorIndex);
					UStaticMeshComponent* Component = StaticMeshActor->StaticMeshComponent;

					// for now, we'll ignore StaticMeshActors that are archetypes or instances of archetypes.
					if ( Component == NULL || Component->IsTemplate(RF_ArchetypeObject) || Component->GetArchetype()->IsTemplate(RF_ArchetypeObject) )
					{
						ReferencedStaticMeshActors.Remove(ActorIndex);
					}
					else
					{
						TArray<UObject*> Referencers;
						StaticMeshReferencers.MultiFind(StaticMeshActor, Referencers);
						for ( INT ReferencerIndex = Referencers.Num() - 1; ReferencerIndex >= 0; ReferencerIndex-- )
						{
							UObject* Referencer = Referencers(ReferencerIndex);
							if ( Referencer == StaticMeshActor->GetLevel() )
							{
								// if this is the actor's level, ignore this reference
								Referencers.Remove(ReferencerIndex);
							}
							else if ( Referencer == StaticMeshActor->StaticMeshComponent )
							{
								// if the referencer is the StaticMeshActor's StaticMeshComponent, we can ignore this reference as this means that
								// something else in the level is referencing the StaticMeshComponent (which will still be around even if we cook
								// out the StaticMeshActor)
								Referencers.Remove(ReferencerIndex);
							}
							else if ( Referencer->IsA(ANavigationPoint::StaticClass())
								&&	(static_cast<ANavigationPoint*>(Referencer)->Base == StaticMeshActor) )
							{
								// if this actor references the StaticMeshActor because it's based on the static mesh actor, ignore this reference
								Referencers.Remove(ReferencerIndex);
							}
						}

						// if this StaticMeshActor is still referenced by something, do not cook it out
						if ( Referencers.Num() == 0 )
						{
							ReferencedStaticMeshActors.Remove(ActorIndex);
						}
					}
				}

				// remove the referenced static mesh actors from the list of actors to be cooked-out.
				for ( INT ActorIndex = 0; ActorIndex < ReferencedStaticMeshActors.Num(); ActorIndex++ )
				{
					StaticMeshActors.RemoveItem(ReferencedStaticMeshActors(ActorIndex));
				}

				AStaticMeshCollectionActor* MeshCollector = NULL;
				AStaticMeshCollectionActor* MeshCollector_BlockOnlyProj = NULL;
				for ( INT ActorIndex = 0; ActorIndex < StaticMeshActors.Num(); ActorIndex++ )
				{
					AStaticMeshActor* StaticMeshActor = StaticMeshActors(ActorIndex);
					UStaticMeshComponent* Component = StaticMeshActor->StaticMeshComponent;

					// SMAs without a SMC should be removed from maps. There already is a map warning for this so we simply silently handle this case.
					if( !Component )
					{
						continue;
					}

					// if there is a limit to the number of components that can be added to a StaticMeshCollectorActor, create a new
					// one if we have reached the limit

					AStaticMeshCollectionActor* MeshCollector_ToUse;

					if (StaticMeshActor->bBlockOnlyProj)
					{
						if (MeshCollector_BlockOnlyProj == NULL
							|| (MeshCollector_BlockOnlyProj->MaxStaticMeshComponents > 0
							&&  MeshCollector_BlockOnlyProj->StaticMeshComponents.Num() >= MeshCollector_BlockOnlyProj->MaxStaticMeshComponents) )
						{
							MeshCollector_BlockOnlyProj = CreateComponentCollector<AStaticMeshCollectionActor>(Package, World);

							MeshCollector_BlockOnlyProj->bBlockOnlyProj = TRUE;
						}

						MeshCollector_ToUse = MeshCollector_BlockOnlyProj;
					}
					else
					{
						if (MeshCollector == NULL
							|| (MeshCollector->MaxStaticMeshComponents > 0
							&&  MeshCollector->StaticMeshComponents.Num() >= MeshCollector->MaxStaticMeshComponents) )
						{
							MeshCollector = CreateComponentCollector<AStaticMeshCollectionActor>(Package, World);
						}

						MeshCollector_ToUse = MeshCollector;
					}					 

					// UPrimitiveComponent::Detach() will clear the ShadowParent but it will never be restored, so save the reference and restore it later
					UPrimitiveComponent* ComponentShadowParent = Component->ShadowParent;

					// remove it from the StaticMeshActor.
					StaticMeshActor->DetachComponent(Component);

					// rather than duplicating the StaticMeshComponent into the mesh collector, rename the component into the collector
					// so that we don't lose any outside references to this component (@todo ronp - are external references to components even allowed?)
					const UBOOL bWasPublic = Component->HasAnyFlags(RF_Public);

					// clear the RF_Public flag so that we don't create a redirector
					Component->ClearFlags(RF_Public);

					// try to use the same component name - however, since we're renaming multiple components into the same Outer,
					// it's likely that we'll end up with a name conflict so first test to see if it would work and if not, use a
					// new name
					const FString OriginalComponentName = Component->GetName();
					if ( Component->Rename(*Component->GetName(), MeshCollector_ToUse, REN_ForceNoResetLoaders|REN_Test) )
					{
						Component->Rename(*Component->GetName(), MeshCollector_ToUse, REN_ForceNoResetLoaders);
					}
					else
					{
						Component->Rename(NULL, MeshCollector_ToUse, REN_ForceNoResetLoaders);
					}

					if ( bWasPublic )
					{
						Component->SetFlags(RF_Public);
					}

					// now add it to the mesh collector's StaticMeshComponents array
					MeshCollector_ToUse->StaticMeshComponents.AddItem(Component);

					// it must also exist in the AllComponents array so that the component's physics data can be cooked
					MeshCollector_ToUse->AllComponents.AddItem(Component);

					// copy any properties which are usually pulled from the Owner at runtime
					Component->ShadowParent = ComponentShadowParent;
					Component->CollideActors = StaticMeshActor->bCollideActors && Component->CollideActors;
					Component->HiddenGame = Component->HiddenGame || (!Component->bCastHiddenShadow && StaticMeshActor->bHidden);
					Component->HiddenEditor = Component->HiddenEditor || StaticMeshActor->bHiddenEd;

					// Since UPrimitiveComponent::SetTransformedToWorld() generates a matrix which includes the
					// component's Scale/Scale3D then multiplies that by the actor's LocalToWorld to get the final
					// transform, the component's Scale/Scale3D must include the actor's scale and the LocalToWorld
					// matrix we serialize for the component must NOT include scaling.  This is necessary so that if
					// the component has a value for Scale/Scale3D it is preserved without applying the actor's scaling
					// twice for all cases.
					FMatrix ActorLocalToWorld = StaticMeshActor->LocalToWorld();

					FVector ActorScale3D = StaticMeshActor->DrawScale * StaticMeshActor->DrawScale3D;
					Component->Scale3D *= ActorScale3D;

					FVector RecipScale( 1.f/ActorScale3D.X, 1.f/ActorScale3D.Y, 1.f/ActorScale3D.Z );

					ActorLocalToWorld.M[0][0] *= RecipScale.X;
					ActorLocalToWorld.M[0][1] *= RecipScale.X;
					ActorLocalToWorld.M[0][2] *= RecipScale.X;

					ActorLocalToWorld.M[1][0] *= RecipScale.Y;
					ActorLocalToWorld.M[1][1] *= RecipScale.Y;
					ActorLocalToWorld.M[1][2] *= RecipScale.Y;

					ActorLocalToWorld.M[2][0] *= RecipScale.Z;
					ActorLocalToWorld.M[2][1] *= RecipScale.Z;
					ActorLocalToWorld.M[2][2] *= RecipScale.Z;

					Component->ConditionalUpdateTransform(ActorLocalToWorld);

					// now mark the StaticMeshActor with no-load flags so that it will disappear on save
					StaticMeshActor->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
					for ( INT CompIndex = 0; CompIndex < StaticMeshActor->Components.Num(); CompIndex++ )
					{
						if ( StaticMeshActor->Components(CompIndex) != NULL )
						{
							StaticMeshActor->Components(CompIndex)->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
						}
					}

					debugf(NAME_DevCooking, TEXT("Cooking out StaticMeshActor %s; re-attaching %s to %s as %s"),
						*StaticMeshActor->GetName(), *OriginalComponentName, *MeshCollector_ToUse->GetName(), *Component->GetName());

					StaticActorIndices.Remove(StaticMeshActor);
				}

				// finally, restore the entries in the level's Actors array for the StaticMeshActors not being cooked out
				for ( TMap<AStaticMeshActor*,INT>::TIterator It(StaticActorIndices); It; ++It )
				{
					INT ActorIndex = It.Value();

					// make sure nothing filled in this entry in the array
					checkSlow(Level->Actors(ActorIndex)==NULL);
					Level->Actors(ActorIndex) = It.Key();
				}
			}
		}
	}
}

/**
* Cooks out all static Light actors in the specified package by re-attaching their LightComponents to a 
* StaticLightCollectionActor referenced by the world.
*/
void UWrangleContentCommandlet::CookStaticLightActors( UPackage* Package )
{
	//if ( (GCookingTarget & PLATFORM_Console) != 0 )
	{
		// only cook-out static Lights when cooking for console
		check(Package);

		// find all StaticMeshActors and static Light actors which are referenced by something in the map
		UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
		if ( World != NULL )
		{
			TArray<ULevel*> LevelsToSearch = World->Levels;

			// make sure that the world's PersistentLevel is part of the levels array for the purpose of this test.
			if ( World->PersistentLevel != NULL )
			{
				LevelsToSearch.AddUniqueItem(World->PersistentLevel);
			}
			for ( INT LevelIndex = 0; LevelIndex < LevelsToSearch.Num(); LevelIndex++ )
			{
				ULevel* Level = LevelsToSearch(LevelIndex);

				// we need to remove all static Light actors from the level's Actors array so that we don't
				// get false positives during our reference checking.
				// however, we'll need to restore any actors which don't get cooked out, so keep track of their indices
				TMap<ALight*,INT> StaticActorIndices;
				TLookupMap<ALight*> StaticLightActors;

				// remove all StaticMeshActors from the level's Actor array so that we don't get false positives.
				for ( INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ActorIndex++ )
				{
					AActor* Actor = Level->Actors(ActorIndex);
					if ( Actor != NULL && Actor->IsA(ALight::StaticClass()) && Actor->bStatic )
					{
						ALight* Light = static_cast<ALight*>(Actor);

						StaticLightActors.AddItem(Light);
						StaticActorIndices.Set(Light, ActorIndex);
						Level->Actors(ActorIndex) = NULL;
					}
				}

				// now use the object reference collector to find the static mesh actors that are still being referenced
				TArray<ALight*> ReferencedStaticLightActors;
				{
					TArchiveObjectReferenceCollector<ALight> LightCollector(&ReferencedStaticLightActors, Package, FALSE, TRUE, TRUE, TRUE);
					Level->Serialize( LightCollector );
				}

				// remove any static light actors which aren't valid for cooking out
				for ( INT ActorIndex = ReferencedStaticLightActors.Num() - 1; ActorIndex >= 0; ActorIndex-- )
				{
					ALight* Light = ReferencedStaticLightActors(ActorIndex);
					if ( Light->bStatic )
					{
						ULightComponent* Component = ReferencedStaticLightActors(ActorIndex)->LightComponent;

						// for now, we'll ignore static Lights that are archetypes or instances of archetypes.
						if ( Component != NULL
							&&	(Component->IsTemplate(RF_ArchetypeObject) || Component->GetArchetype()->IsTemplate(RF_ArchetypeObject)) )
						{
							ReferencedStaticLightActors.Remove(ActorIndex);
						}
					}
					else
					{
						ReferencedStaticLightActors.Remove(ActorIndex);
					}
				}

				TFindObjectReferencers<ALight> StaticLightReferencers(ReferencedStaticLightActors, Package);
				for ( INT ActorIndex = ReferencedStaticLightActors.Num() - 1; ActorIndex >= 0; ActorIndex-- )
				{
					ALight* LightActor = ReferencedStaticLightActors(ActorIndex);

					TArray<UObject*> Referencers;
					StaticLightReferencers.MultiFind(LightActor, Referencers);
					for ( INT ReferencerIndex = Referencers.Num() - 1; ReferencerIndex >= 0; ReferencerIndex-- )
					{
						UObject* Referencer = Referencers(ReferencerIndex);
						if ( Referencer == LightActor->GetLevel() )
						{
							// if this is the actor's level, ignore this reference
							Referencers.Remove(ReferencerIndex);
						}
						else if ( Referencer->IsIn(LightActor) && Referencer->IsA(UComponent::StaticClass()) )
						{
							// if the referencer is one of the LightActor's components, we can ignore this reference as this means that
							// something else in the level is referencing the component directly (which will still be around even if we cook
							// out the Light actor)
							Referencers.Remove(ReferencerIndex);
						}
					}

					// if this actor is still referenced by something, do not cook it out
					if ( Referencers.Num() == 0 )
					{
						ReferencedStaticLightActors.Remove(ActorIndex);
					}
				}

				for ( INT ActorIndex = 0; ActorIndex < ReferencedStaticLightActors.Num(); ActorIndex++ )
				{
					StaticLightActors.RemoveItem(ReferencedStaticLightActors(ActorIndex));
				}

				AStaticLightCollectionActor* LightCollector = NULL;
				for ( INT ActorIndex = 0; ActorIndex < StaticLightActors.Num(); ActorIndex++ )
				{
					ALight* LightActor = StaticLightActors(ActorIndex);
					ULightComponent* Component = LightActor->LightComponent;

					// Light actors without a light component should be removed from maps. There already is a map warning for this so we simply silently handle this case.
					if( !Component )
					{
						continue;
					}

					// if there is a limit to the number of components that can be added to a StaticLightCollectorActor, create a new
					// one if we have reached the limit
					if (LightCollector == NULL
						|| (LightCollector->MaxLightComponents > 0
						&&  LightCollector->LightComponents.Num() >= LightCollector->MaxLightComponents) )
					{
						LightCollector = CreateComponentCollector<AStaticLightCollectionActor>(Package, World);
					}

					// remove it from the Light actor.
					LightActor->DetachComponent(Component);

					// rather than duplicating the LightComponent into the light collector, rename the component into the collector
					// so that we don't lose any outside references to this component (@todo ronp - are external references to components even allowed?)
					const UBOOL bWasPublic = Component->HasAnyFlags(RF_Public);

					// clear the RF_Public flag so that we don't create a redirector
					Component->ClearFlags(RF_Public);

					// try to use the same component name - however, since we're renaming multiple components into the same Outer,
					// it's likely that we'll end up with a name conflict so first test to see if it would work and if not, use a
					// new name
					if ( Component->Rename(*Component->GetName(), LightCollector, REN_ForceNoResetLoaders|REN_Test) )
					{
						Component->Rename(*Component->GetName(), LightCollector, REN_ForceNoResetLoaders);
					}
					else
					{
						Component->Rename(NULL, LightCollector, REN_ForceNoResetLoaders);
					}

					if ( bWasPublic )
					{
						Component->SetFlags(RF_Public);
					}

					// now add it to the light collector's LightComponents array
					LightCollector->LightComponents.AddItem(Component);

					// it must also exist in the AllComponents array so that the component's physics data can be cooked
					LightCollector->AllComponents.AddItem(Component);

					// copy any properties which are usually pulled from the Owner at runtime
					// @todo


					// set the component's LightToWorld while we still have a reference to the original owning Light actor.  This matrix
					// will be serialized to disk by the LightCollector

					Component->LightToWorld = LightActor->LocalToWorld();

					// now mark the Light actor with no-load flags so that it will disappear on save
					LightActor->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
					for ( INT CompIndex = 0; CompIndex < LightActor->Components.Num(); CompIndex++ )
					{
						if ( LightActor->Components(CompIndex) != NULL )
						{
							LightActor->Components(CompIndex)->SetFlags(RF_NotForClient|RF_NotForServer|RF_NotForEdit);
						}
					}

					debugf(NAME_DevCooking, TEXT("Cooking out %s %s; re-attaching %s to %s"),
						*LightActor->GetClass()->GetName(), *LightActor->GetName(), *Component->GetName(), *LightCollector->GetName());

					StaticActorIndices.Remove(LightActor);
				}


				// finally, restore the entries in the level's Actors array for the static Lights not being cooked out
				for ( TMap<ALight*,INT>::TIterator It(StaticActorIndices); It; ++It )
				{
					INT ActorIndex = It.Value();

					// make sure nothing filled in this entry in the array
					checkSlow(Level->Actors(ActorIndex)==NULL);
					Level->Actors(ActorIndex) = It.Key();
				}
			}
		}
	}
}

INT UWrangleContentCommandlet::Main( const FString& Params )
{	
	extern UBOOL GIsCookingForAva;
	GIsCookingForAva = TRUE;

	// does the user want to save the stripped down content?
	UBOOL bShouldRestoreFromPreviousRun = ParseParam(*Params, TEXT("restore"));
	UBOOL bShouldSavePackages = !ParseParam(*Params, TEXT("nosave"));
	UBOOL bShouldSaveUnreferencedContent = !ParseParam(*Params, TEXT("nosaveunreferenced"));
	UBOOL bShouldDumpUnreferencedContent = ParseParam(*Params, TEXT("reportunreferenced"));
	UBOOL bShouldRemoveEditorOnlyData = ParseParam(*Params, TEXT("removeeditoronly"));
	UBOOL bCookWindows = ParseParam(*Params, TEXT("cookwindows"));
	UBOOL bNoDefaults = ParseParam(*Params, TEXT("nodefaults"));

	// store all referenced objects
	TMap<FString, FPackageObjects> AllReferencedPublicObjects;

	if (bShouldRestoreFromPreviousRun)
	{
		FArchive* Ar = GFileManager->CreateFileReader(*(appGameDir() + TEXT("Wrangle.bin")));
		*Ar << AllReferencedPublicObjects;
		delete Ar;
	}
	else
	{
		// make name for our ini file to control loading
		FString WrangleContentIniName =	appGameConfigDir() + TEXT("WrangleContent.ini");

		// get a list of packages to load
		TMultiMap<FString,FString>* PackagesToFullyLoad = GConfig->GetSectionPrivate( TEXT("WrangleContent.PackagesToFullyLoad"), 0, 1, *WrangleContentIniName );
		TMultiMap<FString,FString>* PackagesToAlwaysCook = GConfig->GetSectionPrivate( TEXT("Engine.PackagesToAlwaysCook"), 0, 1, GEngineIni );
		TMultiMap<FString,FString>* PackagesToExcludeFromSeekFree = GConfig->GetSectionPrivate( TEXT("Engine.PackagesToExcludeFromSeekFree"), 0, 1, GEngineIni );

		// this ini setting isn't required
		if (!PackagesToFullyLoad)
		{
			warnf(NAME_Error, TEXT("This commandlet needs a WrangleContent.ini in the Config directory with a [WrangleContent.PackagesToFullyLoad] section"));
			return 1;
		}

		// move any always cook packages to list of packages to load
		if (PackagesToAlwaysCook)
		{
			for (TMultiMap<FString,FString>::TIterator It(*PackagesToAlwaysCook); It; ++It)
			{
				PackagesToFullyLoad->Add(*It.Key(), *It.Value());
			}
		}

		// move any exclude from seek free packages to list of packages to load
		if (PackagesToExcludeFromSeekFree)
		{
			for (TMultiMap<FString,FString>::TIterator It(*PackagesToExcludeFromSeekFree); It; ++It)
			{
				PackagesToFullyLoad->Add(*It.Key(), *It.Value());
			}
		}

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

		if (!bNoDefaults)
			LoadDLOObjectsExplicitly();
		
		// all currently loaded public objects were referenced by script code, so mark it as referenced
		for(FObjectIterator ObjectIt;ObjectIt;++ObjectIt)
		{
			UObject* Object = *ObjectIt;

			// record all public referenced objects
//			if (Object->HasAnyFlags(RF_Public))
			{
				ReferenceObject(Object, AllReferencedPublicObjects, FALSE);
			}
		}

		for (INT i=0; i<GDLOObjects.Num(); ++i)
		{
			if (!ReferenceObject(GDLOObjects(i), AllReferencedPublicObjects, FALSE))
			{
				warnf(TEXT("DLO object %s wasn't appended"), *GDLOObjects(i)->GetName() );
			}
		}

		TArray<FString> MapPackageNames;
		void appGetGameMapPackageNames(TArray<FString>& PackageNames, UBOOL bCanIncludeEditorOnlyPackages);

		if (!bNoDefaults)
		{
			appGetGameMapPackageNames(MapPackageNames, TRUE);

			for (INT i=0; i<MapPackageNames.Num(); ++i)
			{
				FString MapPackageName = MapPackageNames(i);
				PackagesToFullyLoad->Add( *MapPackageName, *MapPackageName );
			}
		}		

		// go over all the packages that we want to fully load
		for (TMultiMap<FString,FString>::TIterator It(*PackagesToFullyLoad); It; ++It)
		{
			// there may be multiple sublevels to load if this package is a persistent level with sublevels
			TArray<FString> PackagesToLoad;
			// start off just loding this package (more may be added in the loop)
			PackagesToLoad.AddItem(*It.Value());

			for (INT PackageIndex = 0; PackageIndex < PackagesToLoad.Num(); PackageIndex++)
			{
				// save a copy of the packagename (not a reference incase the PackgesToLoad array gets realloced)
				FString PackageName = PackagesToLoad(PackageIndex);
				FFilename PackageFilename;

				if( GPackageFileCache->FindPackageFile( *PackageName, NULL, PackageFilename ) == TRUE )
				{
					warnf(TEXT("Fully loading %s..."), *PackageFilename);

	// @todo josh: track redirects in this package and then save the package instead of copy it if there were redirects
	// or make sure that the following redirects marks the package dirty (which maybe it shouldn't do in the editor?)

					// load the package fully
					UPackage* Package = UObject::LoadPackage(NULL, *PackageFilename, LOAD_None);

					UObject::BeginLoad();
					ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *PackageFilename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
					UObject::EndLoad();

					// look for special package types
					UBOOL bIsMap = Linker->ContainsMap();
					UBOOL bIsScriptPackage = Linker->ContainsCode();

					// collect all public objects loaded
					for(FObjectIterator ObjectIt; ObjectIt; ++ObjectIt)
					{
						UObject* Object = *ObjectIt;

						// record all public referenced objects (skipping over top level packages)
						if (/*Object->HasAnyFlags(RF_Public) &&*/ Object->GetOuter() != NULL)
						{
							// is this public object in a fully loaded package?
							UBOOL bIsObjectInFullyLoadedPackage = Object->IsIn(Package);

							if (bIsMap && bIsObjectInFullyLoadedPackage && Object->HasAnyFlags(RF_Public))
							{
								warnf(NAME_Warning, TEXT("Clearing public flag on map object %s"), *Object->GetFullName());
								Object->ClearFlags(RF_Public);
								// mark that we need to save the package since we modified it (instead of copying it)
								Object->MarkPackageDirty();
							}
							else
							{
								// record that this object was referenced
								ReferenceObject(Object, AllReferencedPublicObjects, bIsObjectInFullyLoadedPackage);
							}
						}
					}

					UBOOL bCookOutStaticMeshActors = TRUE;
					UBOOL bCookOutStaticLightActors = TRUE;

					if ( bCookOutStaticMeshActors )
					{
						CookStaticMeshActors( Package );
					}

					if ( bCookOutStaticLightActors )
					{
						CookStaticLightActors(Package);
					}

					// remove editor-only data if desired
					if (bShouldRemoveEditorOnlyData)
					{
						StripEditorData(Package);
						Package->SetDirtyFlag(TRUE);
					}

					if (bCookWindows)
					{
						Cook(Package,UE3::PLATFORM_Windows);
						Package->SetDirtyFlag(TRUE);
					}

					// add any sublevels of this world to the list of levels to load
					for( TObjectIterator<UWorld> It; It; ++It )
					{
						UWorld*		World		= *It;
						AWorldInfo* WorldInfo	= World->GetWorldInfo();
						// iterate over streaming level objects loading the levels.
						for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
						{
							ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
							if( StreamingLevel )
							{
								// add this sublevel's package to the list of packages to laod
								PackagesToLoad.AddUniqueItem(*StreamingLevel->PackageName.ToString());
							}
						}
					}

					// save/copy the package if desired, and only if it's not a script package (script code is
					// not cutdown, so we always use original script code)
					if (bShouldSavePackages && !bIsScriptPackage)
					{
						// make the name of the location to put the package
						FString CutdownPackageName = MakeCutdownFilename(PackageFilename);
						
						// if the package was modified by loading it, then we should save the package
						if (Package->IsDirty())
						{
							// save the fully load packages
							warnf(TEXT("Saving fully loaded package %s..."), *CutdownPackageName);
							if (!SavePackageHelper(Package, CutdownPackageName))
							{
								warnf(NAME_Error, TEXT("Failed to save package %s..."), *CutdownPackageName);
							}
						}
						else
						{
							warnf(TEXT("Copying fully loaded package %s..."), *CutdownPackageName);
							// copy the unmodified file (faster than saving) (0 is success)
							if (GFileManager->Copy(*CutdownPackageName, *PackageFilename) != 0)
							{
								warnf(NAME_Error, TEXT("Failed to copy package to %s..."), *CutdownPackageName);
							}
						}
					}

					// close this package
					UObject::CollectGarbage(RF_Native);
				}
			}
		}

		// save out the referenced objects so we can restore
		FArchive* Ar = GFileManager->CreateFileWriter(*(appGameDir() + TEXT("Wrangle.bin")));
		*Ar << AllReferencedPublicObjects;
		delete Ar;
	}

	// list of all objects that aren't needed
	TArray<FUnreferencedObject> UnnecessaryPublicObjects;
	TMap<FFilename, FPackageObjects> UnnecessaryObjectsByPackage;
	TMap<FString, UBOOL> UnnecessaryObjects;
	TArray<FFilename> UnnecessaryPackages;

	// now go over all packages, quickly, looking for public objects NOT in the AllNeeded array
	const TArray<FString> AllPackages(GPackageFileCache->GetPackageFileList());

	if (bShouldDumpUnreferencedContent || bShouldSaveUnreferencedContent)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Looking for unreferenced objects:"));
		CLEAR_WARN_COLOR();

		// Iterate over all files doing stuff.
		for (INT PackageIndex = 0; PackageIndex < AllPackages.Num(); PackageIndex++)
		{
			FFilename PackageFilename(AllPackages(PackageIndex));
			FString PackageName = PackageFilename.GetBaseFilename();

			// we don't care about trying to wrangle the various shader caches so just skipz0r them
			if(	PackageFilename.GetBaseFilename().InStr( TEXT("LocalShaderCache") )	!= INDEX_NONE
				||	PackageFilename.GetBaseFilename().InStr( TEXT("RefShaderCache") )	!= INDEX_NONE )
			{
				continue;
			}

			// the list of objects in this package
			FPackageObjects* PackageObjs = NULL;

			// this will be set to true if every object in the package is unnecessary
			UBOOL bAreAllObjectsUnnecessary = FALSE;

			if (PackageFilename.GetExtension() == FURL::DefaultMapExt)
			{
				warnf(TEXT("Skipping map %s..."), *PackageFilename);
				continue;
			}
			else if (PackageFilename.GetExtension() == TEXT("u"))
			{
				warnf(TEXT("Skipping script package %s..."), *PackageFilename);
				continue;
			}
			else if (PackageFilename.ToUpper().InStr(TEXT("SHADERCACHE")) != -1)
			{
				warnf(TEXT("Skipping shader cache package %s..."), *PackageFilename);
				continue;
			}
			else
			{
				// get the objectes referenced by this package
				PackageObjs = AllReferencedPublicObjects.Find(*PackageName);

				// if the were no objects referenced in this package, we can just skip it, 
				// and mark the whole package as unreferenced
				if (PackageObjs == NULL)
				{
					warnf(TEXT("No objects in %s were referenced..."), *PackageFilename);
					new(UnnecessaryPublicObjects) FUnreferencedObject(PackageName, 
						TEXT("ENTIRE PACKAGE"), GFileManager->FileSize(*PackageFilename));

					// all objects in this package are unnecasstry
					bAreAllObjectsUnnecessary = TRUE;
				}
				else if (PackageObjs->bIsFullyLoadedPackage)
				{
					warnf(TEXT("Skipping fully loaded package %s..."), *PackageFilename);
					continue;
				}
				else
				{
					warnf(TEXT("Scanning %s..."), *PackageFilename);
				}
			}

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *PackageFilename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
			UObject::EndLoad();

			// go through the exports in the package, looking for public objects
			for (INT ExportIndex = 0; ExportIndex < Linker->ExportMap.Num(); ExportIndex++)
			{
				FObjectExport& Export = Linker->ExportMap(ExportIndex);
				FString ExportName = Linker->GetExportFullName(ExportIndex);

				// some packages may have brokeness in them so we want to just continue so we can wrangle
				if( Export.ObjectName == NAME_None )
				{
					warnf( TEXT( "    Export.ObjectName == NAME_None  for Package: %s " ), *PackageFilename );
					continue;
				}

				// make sure its outer is a package, and this isn't a package
				if (Linker->GetExportClassName(ExportIndex) == NAME_Package || 
					(Export.OuterIndex != 0 && Linker->GetExportClassName(Export.OuterIndex - 1) != NAME_Package))
				{
					continue;
				}

				// was it not already referenced?
				// NULL means it wasn't in the reffed public objects map for the package
				if (bAreAllObjectsUnnecessary || PackageObjs->ReferencedObjects.Find(ExportName) == NULL)
				{
					// is it public?
					if ((Export.ObjectFlags & RF_Public) != 0 && !bAreAllObjectsUnnecessary)
					{
						// if so, then add it to list of unused pcreateexportublic items
						new(UnnecessaryPublicObjects) FUnreferencedObject(PackageFilename.GetBaseFilename(), ExportName, Export.SerialSize);
					}

					// look for existing entry
					FPackageObjects* ObjectsInPackage = UnnecessaryObjectsByPackage.Find(*PackageFilename);
					// if not found, make a new one
					if (ObjectsInPackage == NULL)
					{
						ObjectsInPackage = &UnnecessaryObjectsByPackage.Set(*PackageFilename, FPackageObjects());
					}

					// get object's class
					check(IS_IMPORT_INDEX(Export.ClassIndex));
					FString ClassName = Linker->GetImportPathName(- Export.ClassIndex - 1);
					UClass* Class = StaticLoadClass(UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_None, NULL);
					check(Class);

					// make sure it doesn't get GC'd
					Class->AddToRoot();
				
					// add this referenced object to the map
					ObjectsInPackage->ReferencedObjects.Set(*ExportName, Class);

					// add this to the map of all unnecessary objects
					UnnecessaryObjects.Set(*ExportName, TRUE);
				}
			}

			// collect garbage every 20 packages (we aren't fully loading, so it doesn't need to be often)
			if ((PackageIndex % 20) == 0)
			{
				UObject::CollectGarbage(RF_Native);
			}
		}
	}

	if (bShouldSavePackages)
	{
		INT NumPackages = AllReferencedPublicObjects.Num();

		// go through all packages, and save out referenced objects
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Saving referenced objects in %d Packages:"), NumPackages);
		CLEAR_WARN_COLOR();
		INT PackageIndex = 0;
		for (TMap<FString, FPackageObjects>::TIterator It(AllReferencedPublicObjects); It; ++It, PackageIndex++ )
		{
			// we don't care about trying to wrangle the various shader caches so just skipz0r them
			if(	It.Key().InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
				|| It.Key().InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
			{
				continue;
			}

			// if the package was a fully loaded pacakge, than we already saved it
			if (It.Value().bIsFullyLoadedPackage)
			{
				continue;
			}

			// package for all loaded objects
			UPackage* Package = NULL;
			
			// fully load all the referenced objects in the package
			for (TMap<FString, UClass*>::TIterator It2(It.Value().ReferencedObjects); It2; ++It2)
			{
				// get the full object name
				FString ObjectPathName = It2.Key();

				// skip over the class portion (the It2.Value() has the class pointer already)
				INT Space = ObjectPathName.InStr(TEXT(" "));
				check(Space);

				// get everything after the space
				ObjectPathName = ObjectPathName.Right(ObjectPathName.Len() - (Space + 1));

				// load the referenced object

				UObject* Object = UObject::StaticLoadObject(It2.Value(), NULL, *ObjectPathName, NULL, LOAD_NoWarn, NULL);

				// the object may not exist, because of attempting to load localized content
				if (Object)
				{
					check(Object->GetPathName() == ObjectPathName);

					// set the package if needed
					if (Package == NULL)
					{
						Package = Object->GetOutermost();
					}
					else
					{
						// make sure all packages are the same
						check(Package == Object->GetOutermost());
					}
				}
			}

			// make sure we found some objects in here
			if (Package)
			{
				// mark this package as fully loaded so it can be saved, even though we didn't fully load it
				// (which is the point of this commandlet)
				Package->MarkAsFullyLoaded();

				// get original path of package
				FFilename OriginalPackageFilename;

				//warnf( TEXT( "*It.Key(): %s" ), *It.Key() );				
				
				// we need to be able to find the original package
				//verify(GPackageFileCache->FindPackageFile(*It.Key(), NULL, OriginalPackageFilename) == TRUE);
				if( GPackageFileCache->FindPackageFile(*It.Key(), NULL, OriginalPackageFilename) == FALSE )
				{
					continue;

					// HACK 
					appErrorf( TEXT( "Could not find file in file cache: %s"), *It.Key() );
				}

				// any maps need to be fully referenced
				check(OriginalPackageFilename.GetExtension() != FURL::DefaultMapExt);

				// make the filename for the output package
				FString CutdownPackageName = MakeCutdownFilename(OriginalPackageFilename);

				warnf(TEXT("Saving %s... [%d/%d]"), *CutdownPackageName, PackageIndex + 1, NumPackages);

				// remove editor-only data if desired
				if (bShouldRemoveEditorOnlyData)
				{
					StripEditorData(Package);
					Package->SetDirtyFlag(TRUE);
				}

				if (bCookWindows)
				{
					Cook(Package,UE3::PLATFORM_Windows);
					Package->SetDirtyFlag(TRUE);
				}

				// save the package now that all needed objects in it are loaded
				SavePackageHelper(Package, *CutdownPackageName);

				// close up this package
				UObject::CollectGarbage(RF_Native);
			}
		}
	}

	if (bShouldDumpUnreferencedContent)
	{
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Unreferenced Public Objects:"));
		CLEAR_WARN_COLOR();

		// Create string with system time to create a unique filename.
		INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
		appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
		FString	CurrentTime = FString::Printf(TEXT("%i.%02i.%02i-%02i.%02i.%02i"), Year, Month, Day, Hour, Min, Sec );

		// create a csv
		FString CSVFilename = FString::Printf(TEXT("%sUnreferencedObjects-%s.csv"), *appGameLogDir(), *CurrentTime);
		FArchive* CSVFile = GFileManager->CreateFileWriter(*CSVFilename);
		if (!CSVFile)
		{
			warnf(NAME_Error, TEXT("Failed to open output file %s"), *CSVFilename);
		}

		for (INT ObjectIndex = 0; ObjectIndex < UnnecessaryPublicObjects.Num(); ObjectIndex++)
		{
			FUnreferencedObject& Object = UnnecessaryPublicObjects(ObjectIndex);
			warnf(*Object.ObjectName);

			// dump out a line to the .csv file
			// @todo: sort by size to Excel's 65536 limit gets the biggest objects
			FString CSVLine = FString::Printf(TEXT("%s,%s,%d%s"), *Object.PackageName, *Object.ObjectName, Object.SerialSize, LINE_TERMINATOR);
			CSVFile->Serialize(TCHAR_TO_ANSI(*CSVLine), CSVLine.Len());
		}
	}

	// load every unnecessary object by package, rename it and any unnecessary objects if uses, to the 
	// an unnecessary package, and save it
	if (bShouldSaveUnreferencedContent)
	{
		INT NumPackages = UnnecessaryObjectsByPackage.Num();
		SET_WARN_COLOR(COLOR_WHITE);
		warnf(TEXT(""));
		warnf(TEXT("Saving unreferenced objects [%d packages]:"), NumPackages);
		CLEAR_WARN_COLOR();

		// go through each package that has unncessary objects in it
		INT PackageIndex = 0;
		for (TMap<FFilename, FPackageObjects>::TIterator PackageIt(UnnecessaryObjectsByPackage); PackageIt; ++PackageIt, PackageIndex++)
		{
			// we don't care about trying to wrangle the various shader caches so just skipz0r them
			if(	PackageIt.Key().InStr( TEXT("LocalShaderCache") ) != INDEX_NONE
				|| PackageIt.Key().InStr( TEXT("RefShaderCache") ) != INDEX_NONE )
			{
				continue;
			}

			//warnf(TEXT("Processing %s"), *PackageIt.Key());
			UPackage* FullyLoadedPackage = NULL;
			// fully load unnecessary packages with no objects, 
			if (PackageIt.Value().ReferencedObjects.Num() == 0)
			{
				// just load it, and don't need a reference to it
				FullyLoadedPackage = UObject::LoadPackage(NULL, *PackageIt.Key(), LOAD_None);
			}
			else
			{
				// load every unnecessary object in this package
				for (TMap<FString, UClass*>::TIterator ObjectIt(PackageIt.Value().ReferencedObjects); ObjectIt; ++ObjectIt)
				{
					// get the full object name
					FString ObjectPathName = ObjectIt.Key();

					// skip over the class portion (the It2.Value() has the class pointer already)
					INT Space = ObjectPathName.InStr(TEXT(" "));
					check(Space > 0);

					// get everything after the space
					ObjectPathName = ObjectPathName.Right(ObjectPathName.Len() - (Space + 1));

					// load the unnecessary object
					UObject* Object = UObject::StaticLoadObject(ObjectIt.Value(), NULL, *ObjectPathName, NULL, LOAD_NoWarn, NULL);
					
					// this object should exist since it was gotten from a linker
					if (!Object)
					{
						warnf(NAME_Error, TEXT("Failed to load object %s, it will be deleted permanently!"), *ObjectPathName);
					}
				}
			}

			// now find all loaded objects (in any package) that are in marked as unnecessary,
			// and rename them to their destination
			for (TObjectIterator<UObject> It; It; ++It)
			{
				// if was unnecessary...
				if (UnnecessaryObjects.Find(*It->GetFullName()))
				{
					// ... then rename it (its outer needs to be a package, everything else will have to be
					// moved by its outer getting moved)
					if (!It->IsA(UPackage::StaticClass()) &&
						It->GetOuter() &&
						It->GetOuter()->IsA(UPackage::StaticClass()) &&
						It->GetOutermost()->GetName().Left(4) != TEXT("NFS_"))
					{
						UPackage* NewPackage = UObject::CreatePackage(NULL, *(FString(TEXT("NFS_")) + It->GetOuter()->GetPathName()));
						//warnf(TEXT("Renaming object from %s to %s.%s"), *It->GetPathName(), *NewPackage->GetPathName(), *It->GetName());

						// move the object if we can. IF the rename fails, then the objet was already renamed to this spot, but not GC'd.
						// that's okay.
						if (It->Rename(*It->GetName(), NewPackage, REN_Test))
						{
							It->Rename(*It->GetName(), NewPackage, REN_None);
						}
					}

				}
			}

			// find the one we moved this packages objects to
			FFilename PackagePath = PackageIt.Key();
			FString PackageName = PackagePath.GetBaseFilename();
			UPackage* MovedPackage = UObject::FindPackage(NULL, *(FString(TEXT("NFS_")) + PackageName));
			check(MovedPackage);

			// convert the new name to a a NFS directory directory
			FFilename MovedFilename = MakeCutdownFilename(FString::Printf(TEXT("%s\\NFS_%s"), *PackagePath.GetPath(), *PackagePath.GetCleanFilename()), TEXT("NFSContent"));
			warnf(TEXT("Saving package %s [%d/%d]"), *MovedFilename, PackageIndex, NumPackages);
			// finally save it out
			SavePackageHelper(MovedPackage, *MovedFilename);

			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}

IMPLEMENT_CLASS(UWrangleContentCommandlet);

//<@ ava specific ; 2006. 9. 9 changmin
/**
*	URemoveUITextureMips
*
*	UI Texture 는 Mipmap이 필요없습니다. 용량을 절약하기 위해서, Mipmap이 생성되어 있는 UI Texture에 대해, Mipmap을 제거하는 작업을 합니다.
*   Ava의 UI Texture Package Path 는 avaGame/Content/UI입니다
*	TextureLODGroup = TEXTUREGROUP_UI 입니다.
 *	UI Texture Package들 중에, TextureLODGroup이 TEXTUREGROUP_UI이고 Mipmap이 생성되어 있는 경우, 이를 제거합니다.
*
*/
INT URemoveUITextureMipsCommandlet::Main( const FString & Params )
{
	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR * Parms = *Params;
	ParseCommandLine( Parms, Tokens, Switches );

	TArray<FString> PackageList = GPackageFileCache->GetPackageFileList();

	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename		Filename	= FFilename(PackageList(PackageIndex));

		if( Filename.ToUpper().InStr( TEXT("AVAGAME\\CONTENT\\UI") ) != INDEX_NONE )
		{

			warnf(NAME_Log, TEXT("Start Loading... %s"), *Filename);
			warnf(NAME_Log, TEXT("----------------------------------------"));

			UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn );

			if (Package != NULL)
			{
				// Iterate over all SoundNodeWave objects
				UBOOL bRequireSave = FALSE;

				warnf(NAME_Log, TEXT("Load......"));

				for (TObjectIterator<UTexture2D> TextureIt; TextureIt; ++TextureIt)
				{
					if (TextureIt->IsIn(Package))
					{
						
						UTexture2D* Texture2D = *TextureIt;
						if(Texture2D->LODGroup == TEXTUREGROUP_UI)
						{
							if( Texture2D->CompressionNoMipmaps == false )
							{
								const INT PreviousNumMips = Texture2D->Mips.Num();

								bRequireSave = TRUE;
								Texture2D->CompressionNoMipmaps = true;
								Texture2D->Compress();

								const INT PostNumMips = Texture2D->Mips.Num();

								warnf(NAME_Log,TEXT("Remove Mips(%d->%d) in %s"), PreviousNumMips, PostNumMips, *(Texture2D->GetName()));
							}
						}
					}
				}

				if ( bRequireSave )
				{
					if (GFileManager->IsReadOnly(*Filename))
					{
						warnf(NAME_Error, TEXT("Package Is Read Only. Can't Save Package....."));
						continue;
					}

					// resave the package
					SavePackageHelper(Package, Filename);
					warnf(NAME_Log, TEXT("Pakage is Dirty.... Save Compelete...") );
				}
				else
				{
					warnf(NAME_Log, TEXT("Package don't need to save..."), *Filename );
				}
			}
			else
			{
				warnf(NAME_Log, TEXT("Can't Load... %s"), *Filename);
			}
		}
	}

	return 0;
}

IMPLEMENT_CLASS(URemoveUITextureMipsCommandlet);
//>@ ava
