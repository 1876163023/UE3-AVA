/**
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

// @GEMINI_TODO: figure out a good way to use Combine without including FConfigCacheIni.h maybe?
#include "FConfigCacheIni.h"

IMPLEMENT_CLASS(UOnlineSubsystem);
IMPLEMENT_CLASS(UOnlineGameSettings);
IMPLEMENT_CLASS(UOnlineGameSearch);

/**
 * Handle downloaded content in a platform-independent way
 *
 * @param Content The content descriptor that describes the downloaded content files
 *
 * @param return TRUE is successful
 */
UBOOL UOnlineSubsystem::ProcessDownloadedContent(const FOnlineContent& Content)
{
	// tell the package cache about all the new packages
	for (INT PackageIndex = 0; PackageIndex < Content.ContentPackages.Num(); PackageIndex++)
	{
		GPackageFileCache->CacheDownloadedPackage(*Content.ContentPackages(PackageIndex), Content.UserIndex);
	}

	// temporarily allow the GConfigCache to perform file operations if they were off
	UBOOL bWereFileOpsDisabled = GConfig->AreFileOperationsDisabled();
	GConfig->EnableFileOperations();

	// tell ini cache that any new ini sections in the loop below are special downloaded sections
	GConfig->StartUsingDownloadedCache(Content.UserIndex);

	// get a list of all known config files
	TArray<FFilename> ConfigFiles;
	GConfig->GetConfigFilenames(ConfigFiles);

	// tell the ini cache about all the new ini files
	for (INT FileIndex = 0; FileIndex < Content.ContentFiles.Num(); FileIndex++)
	{
		FFilename ContentFile = FFilename(Content.ContentFiles(FileIndex)).GetCleanFilename();
		// get filename extension
		FString Ext = ContentFile.GetExtension();
			
		// skip any non-ini/loc (for current language) files
		if (Ext != TEXT("ini") && Ext != UObject::GetLanguage())
		{
			continue;
		}

		// look for the optional special divider string
		#define DividerString TEXT("__")
		INT Divider = ContentFile.InStr(DividerString);
		// if we found it, just use what's after the divider
		if (Divider != -1)
		{
			ContentFile = ContentFile.Right(ContentFile.Len() - (Divider + appStrlen(DividerString)));
		}

		FConfigFile* NewConfigFile = NULL;

		// look for the filename in the config files
		UBOOL bWasCombined = FALSE;
		for (INT ConfigFileIndex = 0; ConfigFileIndex < ConfigFiles.Num() && !bWasCombined; ConfigFileIndex++)
		{
			// does the config file (without path) match the DLC file?
			if (ConfigFiles(ConfigFileIndex).GetCleanFilename() == ContentFile)
			{
				// get the configfile object
				NewConfigFile = GConfig->FindConfigFile(*ConfigFiles(ConfigFileIndex));
				check(NewConfigFile);

				// merge our ini file into the existing one
				NewConfigFile->Combine(*Content.ContentFiles(FileIndex));

				debugf(TEXT("Merged DLC config file '%s' into existing config '%s'"), *Content.ContentFiles(FileIndex), *ConfigFiles(ConfigFileIndex));

				// mark that we have combined
				bWasCombined = TRUE;
			}
		}

		// if it wasn't combined, add a new file
		if (!bWasCombined)
		{
			// we need to create a usable pathname for the new ini/loc file
			FString NewConfigFilename;
			if (Ext == TEXT("ini"))
			{
				NewConfigFilename = appGameConfigDir() + ContentFile;
			}
			else
			{
				// put this into any localization directory in the proper language sub-directory (..\ExampleGame\Localization\fra\DLCMap.fra)
				NewConfigFilename = GSys->LocalizationPaths(0) * Ext * ContentFile;
			}
			// first we set a value into the config for this filename (since we will be reading from a different
			// path than we want to store the config under, we can't use LoadFile)
			GConfig->SetBool(TEXT("DLCDummy"), TEXT("A"), FALSE, *NewConfigFilename);

			// now get the one we just made
			NewConfigFile = GConfig->FindConfigFile(*NewConfigFilename);
			
			// read in the file
			NewConfigFile->Read(*Content.ContentFiles(FileIndex));

			debugf(TEXT("Read new DLC config file '%s' into the config cache"), *Content.ContentFiles(FileIndex));
		}
		
		check(NewConfigFile);
		// look for packages to load for maps
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		if (GameEngine)
		{
			// look for a section in the config file that tells us to fully load a package
			// this is the format:
			// [Engine.PackagesToFullyLoadForDLC]
			// MapName=Map1
			// Package=PackageA
			// Package=PackageB
			// MapName=Map2
			// Package=PackageC
			// Package=PackageD
			TMultiMap<FString,FString>* PackagesToFullyLoad = NewConfigFile->Find(TEXT("Engine.PackagesToFullyLoadForDLC"));
			FName MapName = NAME_None;
			TArray<FName> PackageList;
			if (PackagesToFullyLoad)
			{
				for( TMultiMap<FString,FString>::TIterator It(*PackagesToFullyLoad); It; ++It )
				{
					if (It.Key() == TEXT("MapName"))
					{
						// submit a previous list now
						if (PackageList.Num())
						{
							check(MapName != NAME_None);
							GameEngine->AddPerMapPackagesToLoad(MapName, PackageList, TRUE);
							// clear out the list for the next map
							PackageList.Empty();
						}

						// remember the current map name
						MapName = FName(*It.Value());
					}
					else if (It.Key() == TEXT("Package"))
					{
						// add this package to the current list of packages
						PackageList.AddItem(FName(*It.Value()));
					}
				}

				// submit the remaining packages
				if (PackageList.Num())
				{
					check(MapName != NAME_None);
					GameEngine->AddPerMapPackagesToLoad(MapName, PackageList, TRUE);
				}
			}
		}
	}

	// put the ini cache back to normal
	GConfig->StopUsingDownloadedCache();
	
	// re-disable file ops if they were before
	if (bWereFileOpsDisabled)
	{
		GConfig->DisableFileOperations();
	}

	// refresh the data provider
	UGameUISceneClient* Client = UUIRoot::GetSceneClient();
	if (Client)
	{
		// @todo: This should be GameResources!!! not WarfareGameResources!
		UUIDataStore_GameResource* DataStore = (UUIDataStore_GameResource*)Client->DataStoreManager->FindDataStore(FName(TEXT("WarfareGameResources")));
		if (DataStore)
		{
			// reparse the .ini files for the PerObjectConfig objects
			DataStore->InitializeListElementProviders();
			// make ay currently active widgets using the data get refreshed
			DataStore->eventRefreshSubscribers();
		}
	}


	return TRUE;
}

/**
 * Flush downloaded content for all users, making the engine stop using the content
 *
 * @param MaxNumUsers Platform specific max number of users to flush (this will iterate over all users from 0 to MaxNumUsers, as well as NO_USER 
 */
void UOnlineSubsystem::FlushAllDownloadedContent(INT MaxNumUsers)
{
	// remove content not associated with a user
	GPackageFileCache->ClearDownloadedPackages();
	GConfig->RemoveDownloadedSections();

	// remove any existing content packages for all users
	for (INT UserIndex = 0; UserIndex < MaxNumUsers; UserIndex++)
	{
		GPackageFileCache->ClearDownloadedPackages(UserIndex);
		GConfig->RemoveDownloadedSections(UserIndex);
	}

	// refresh the data provider to make sure it removes the maps, etc
	UGameUISceneClient* Client = UUIRoot::GetSceneClient();
	if (Client)
	{
		// @todo: This should be GameResources!!! not WarfareGameResources!
		UUIDataStore_GameResource* DataStore = (UUIDataStore_GameResource*)Client->DataStoreManager->FindDataStore(FName(TEXT("WarfareGameResources")));
		if (DataStore)
		{
			// reparse the .ini files for the PerObjectConfig objects
			DataStore->InitializeListElementProviders();
			// make ay currently active widgets using the data get refreshed
			DataStore->eventRefreshSubscribers();
		}
	}

	// cleanup the list of packages to load for maps
	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine)
	{
		// cleanup all of the fully loaded packages for maps (may not free memory until next GC)
		GameEngine->CleanupAllPerMapPackagesToLoad();
	}
}
