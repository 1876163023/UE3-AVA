/*=============================================================================
	UnPackageUtilities.cpp: Commandlets for viewing information about package files
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineMaterialClasses.h"
#include "UnPropertyTag.h"
#include "EngineUIPrivateClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"
#include "LensFlare.h"

/*-----------------------------------------------------------------------------
	ULoadPackageCommandlet
-----------------------------------------------------------------------------*/

/**
 * If you pass in -ALL this will recursively load all of the packages from the
 * directories listed in the .ini path entries
 **/

INT ULoadPackageCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	const UBOOL bLoadAllPackages = Switches.FindItemIndex(TEXT("ALL")) != INDEX_NONE;
	TArray<FString> FilesInPath;
	if ( bLoadAllPackages )
	{
		FilesInPath = GPackageFileCache->GetPackageFileList();
		ResetLoaders(NULL);
	}
	else
	{
		for ( INT i = 0; i < Tokens.Num(); i++ )
		{
			FString	PackageWildcard = Tokens(i);	

			GFileManager->FindFiles( FilesInPath, *PackageWildcard, TRUE, FALSE );
			if( FilesInPath.Num() == 0 )
			{
				// if no files were found, it might be an unqualified path; try prepending the .u output path
				// if one were going to make it so that you could use unqualified paths for package types other
				// than ".u", here is where you would do it
				GFileManager->FindFiles( FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

				if ( FilesInPath.Num() == 0 )
				{
					TArray<FString> Paths;
					if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
					{
						for ( INT i = 0; i < Paths.Num(); i++ )
						{
							GFileManager->FindFiles( FilesInPath, *(Paths(i) * PackageWildcard), 1, 0 );
						}
					}
				}
				else
				{
					// re-add the path information so that GetPackageLinker finds the correct version of the file.
					FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
					for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
					{
						FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
					}
				}

				// Try finding package in package file cache.
				if ( FilesInPath.Num() == 0 )
				{
					FString Filename;
					if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
					{
						new(FilesInPath)FString(Filename);
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
				{
					FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
				}
			}
		}
	}

	if( FilesInPath.Num() == 0 )
	{
		warnf(NAME_Warning,TEXT("No packages found matching '%s'"), Parms);
		return 1;
	}

	const UBOOL bSimulateClient = Switches.FindItemIndex(TEXT("NOCLIENT")) == INDEX_NONE;
	const UBOOL bSimulateServer = Switches.FindItemIndex(TEXT("NOSERVER")) == INDEX_NONE;
	const UBOOL bSimulateEditor = Switches.FindItemIndex(TEXT("NOEDITOR")) == INDEX_NONE;
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		GIsClient = bSimulateClient;
		GIsServer = bSimulateServer;
		GIsEditor = bSimulateEditor;

		const FFilename& Filename = FilesInPath(FileIndex);
		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		const FString& PackageName = FPackageFileCache::PackageFromPath(*Filename);
		UPackage* Package = FindObject<UPackage>(NULL, *PackageName, TRUE);
		if ( Package != NULL && !bLoadAllPackages )
		{
			ResetLoaders(Package);
		}
		
		Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}

		GIsEditor = GIsServer = GIsClient = TRUE;
		SaveLocalShaderCaches();
		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}
IMPLEMENT_CLASS(ULoadPackageCommandlet)


/*-----------------------------------------------------------------------------
	UShowObjectCountCommandlet.
-----------------------------------------------------------------------------*/

void UShowObjectCountCommandlet::StaticInitialize()
{
}

struct FPackageObjectCount
{
	INT		Count;
	FString	PackageName;
	FString ClassName;

	FPackageObjectCount( const FString& inPackageName, const FString& inClassName, INT inCount )
	: Count(inCount), PackageName(inPackageName), ClassName(inClassName)
	{
	}
};

IMPLEMENT_COMPARE_CONSTREF( FPackageObjectCount, UnPackageUtilities, { int result = appStricmp(*A.ClassName, *B.ClassName); if ( result == 0 ) { result = B.Count - A.Count; } return result; } )

INT UShowObjectCountCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	GIsRequestingExit			= 1;	// so CTRL-C will exit immediately
	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

	if ( Tokens.Num() == 0 )
	{
		warnf(TEXT("No class specified!"));
		return 1;
	}

	UBOOL bIncludeChildren = Switches.FindItemIndex(TEXT("ExactClass")) == INDEX_NONE;
	const UBOOL bIgnoreScriptPackages = Switches.FindItemIndex(TEXT("IgnoreScript")) != INDEX_NONE;

	EObjectFlags ObjectMask = RF_LoadForClient|RF_LoadForServer|RF_LoadForEdit;
	if ( ParseParam(appCmdLine(), TEXT("SkipClientOnly")) )
	{
		ObjectMask &= ~RF_LoadForClient;
	}
	if ( ParseParam(appCmdLine(), TEXT("SkipServerOnly")) )
	{
		ObjectMask &= ~RF_LoadForServer;
	}
	if ( ParseParam(appCmdLine(), TEXT("SkipEditorOnly")) )
	{
		ObjectMask &= ~RF_LoadForEdit;
	}

	TArray<UClass*> SearchClasses;
	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		FString& SearchClassName = Tokens(TokenIndex);
		UClass* SearchClass = LoadClass<UObject>(NULL, *SearchClassName, NULL, 0, NULL);
		if ( SearchClass == NULL )
		{
			warnf(TEXT("Failed to load class specified '%s'"), *SearchClassName);
			return 1;
		}

		SearchClasses.AddUniqueItem(SearchClass);
	}

	TArray<FString> PackageFiles = GPackageFileCache->GetPackageFileList();

	INT GCIndex = 0;
	TArray<FPackageObjectCount> ClassObjectCounts;
	for( INT FileIndex=0; FileIndex<PackageFiles.Num(); FileIndex++ )
	{
		const FString &Filename = PackageFiles(FileIndex);

		warnf(NAME_Progress, TEXT("Checking '%s'..."), *Filename);

		UObject::BeginLoad();
		ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn|LOAD_NoVerify, NULL, NULL );
		UObject::EndLoad();

		if( Linker && (!bIgnoreScriptPackages || (Linker->LinkerRoot != NULL && (Linker->LinkerRoot->PackageFlags&PKG_ContainsScript) == 0)) )
		{
			TArray<INT> ObjectCounts;
			ObjectCounts.AddZeroed(SearchClasses.Num());

			UBOOL bContainsObjects=FALSE;
			for ( INT i = 0; i < Linker->ExportMap.Num(); i++ )
			{
				FObjectExport& Export = Linker->ExportMap(i);
				if ( (Export.ObjectFlags&ObjectMask) == 0 )
				{
					continue;
				}

				FString ClassPathName;


				FName ClassFName = NAME_Class;
				PACKAGE_INDEX ClassPackageIndex = 0;

				// get the path name for this Export's class
				if ( IS_IMPORT_INDEX(Export.ClassIndex) )
				{
					FObjectImport& ClassImport = Linker->ImportMap(-Export.ClassIndex -1);
					ClassFName = ClassImport.ObjectName;
					ClassPackageIndex = ClassImport.OuterIndex;
				}
				else if ( Export.ClassIndex != UCLASS_INDEX )
				{
					FObjectExport& ClassExport = Linker->ExportMap(Export.ClassIndex-1);
					ClassFName = ClassExport.ObjectName;
					ClassPackageIndex = ClassExport.OuterIndex;
				}

				FName OuterName = NAME_Core;
				if ( ClassPackageIndex > 0 )
				{
					FObjectExport& OuterExport = Linker->ExportMap(ClassPackageIndex-1);
					OuterName = OuterExport.ObjectName;
				}
				else if ( ClassPackageIndex < 0 )
				{
					FObjectImport& OuterImport = Linker->ImportMap(-ClassPackageIndex-1);
					OuterName = OuterImport.ObjectName;
				}
				else if ( Export.ClassIndex != UCLASS_INDEX )
				{
					OuterName = Linker->LinkerRoot->GetFName();
				}
				
				ClassPathName = FString::Printf(TEXT("%s.%s"), *OuterName.ToString(), *ClassFName.ToString());
				UClass* ExportClass = FindObject<UClass>(ANY_PACKAGE, *ClassPathName);
				if ( ExportClass == NULL )
				{
					ExportClass = StaticLoadClass(UObject::StaticClass(), NULL, *ClassPathName, NULL, LOAD_NoVerify|LOAD_NoWarn|LOAD_Quiet, NULL);
				}

				if ( ExportClass == NULL )
				{
					continue;
				}

				for ( INT ClassIndex = 0; ClassIndex < SearchClasses.Num(); ClassIndex++ )
				{
					UClass* SearchClass = SearchClasses(ClassIndex);
					if ( bIncludeChildren ? ExportClass->IsChildOf(SearchClass) : ExportClass == SearchClass )
					{
						bContainsObjects = TRUE;
						INT& CurrentObjectCount = ObjectCounts(ClassIndex);
						CurrentObjectCount++;
					}
				}
			}

			if ( bContainsObjects )
			{
				for ( INT ClassIndex = 0; ClassIndex < ObjectCounts.Num(); ClassIndex++ )
				{
					INT ClassObjectCount = ObjectCounts(ClassIndex);
					if ( ClassObjectCount > 0 )
					{
						new(ClassObjectCounts) FPackageObjectCount(Filename, SearchClasses(ClassIndex)->GetName(), ClassObjectCount);
					}
				}
			}
		}

		// only GC every 10 packages (A LOT faster this way, and is safe, since we are not 
		// acting on objects that would need to go away or anything)
		if ((++GCIndex % 10) == 0)
		{
			UObject::CollectGarbage(RF_Native);
		}
	}

	if( ClassObjectCounts.Num() )
	{
		Sort<USE_COMPARE_CONSTREF(FPackageObjectCount,UnPackageUtilities)>( &ClassObjectCounts(0), ClassObjectCounts.Num() );

		INT TotalObjectCount=0;
		INT PerClassObjectCount=0;

		FString LastReportedClass;
		INT IndexPadding=0;
		for ( INT i = 0; i < ClassObjectCounts.Num(); i++ )
		{
			FPackageObjectCount& PackageObjectCount = ClassObjectCounts(i);
			if ( PackageObjectCount.ClassName != LastReportedClass )
			{
				if ( LastReportedClass.Len() > 0 )
				{
					warnf(TEXT("    Total: %i"), PerClassObjectCount);
				}

				PerClassObjectCount = 0;
				LastReportedClass = PackageObjectCount.ClassName;
				warnf(TEXT("\r\nPackages containing objects of class '%s':"), *LastReportedClass);
				IndexPadding = appItoa(PackageObjectCount.Count).Len();
			}

			warnf(TEXT("    Count: %*i    Package: %s"), IndexPadding, PackageObjectCount.Count, *PackageObjectCount.PackageName);
			PerClassObjectCount += PackageObjectCount.Count;
			TotalObjectCount += PackageObjectCount.Count;
		}

		warnf(TEXT("    Total: %i"), PerClassObjectCount);
		warnf(TEXT("\r\nTotal number of object instances: %i"), TotalObjectCount);
	}
	return 0;
}

IMPLEMENT_CLASS(UShowObjectCountCommandlet);

/*-----------------------------------------------------------------------------
	UShowTaggedPropsCommandlet.
-----------------------------------------------------------------------------*/

INT UShowTaggedPropsCommandlet::Main(const FString& Params)
{
	const TCHAR* CmdLine = appCmdLine();

	FString	ClassName, PackageName, PropertyFilter;
	PackageName = ParseToken(CmdLine, FALSE);
	ClassName = ParseToken(CmdLine, FALSE);
	PropertyFilter = ParseToken(CmdLine, FALSE);

	UObject* Pkg = LoadPackage(NULL, *PackageName, LOAD_None);
	UClass* SearchClass = StaticLoadClass(UObject::StaticClass(), NULL, *ClassName, NULL, LOAD_None, NULL);

	if ( SearchClass == NULL && ClassName.Len() > 0 )
	{
		warnf(NAME_Error, TEXT("Failed to load class '%s'"), *ClassName);
		return 1;
	}

	if ( PropertyFilter.Len() > 0 )
	{
		TArray<FString> PropertyNames;
		PropertyFilter.ParseIntoArray(&PropertyNames, TEXT(","), TRUE);

		for ( INT PropertyIndex = 0; PropertyIndex < PropertyNames.Num(); PropertyIndex++ )
		{
			UProperty* Property = FindFieldWithFlag<UProperty,CLASS_IsAUProperty>(SearchClass, FName(*PropertyNames(PropertyIndex)));
			if ( Property != NULL )
			{
				SearchProperties.Set(Property,Property);
			}
		}
	}

	// this is needed in case we end up serializing a script reference which results in VerifyImport being called
	BeginLoad();
	for ( FObjectIterator It; It; ++It )
	{
		UObject* Obj = *It;
		if ( Obj->IsA(SearchClass) && Obj->IsIn(Pkg) )
		{
			ShowSavedProperties(Obj);
		}
	}
	EndLoad();

	return 0;
}

void UShowTaggedPropsCommandlet::ShowSavedProperties( UObject* Object ) const
{
	check(Object);

	ULinkerLoad& Ar = *Object->GetLinker();
	INT LinkerIndex = Object->GetLinkerIndex();
	check(LinkerIndex != INDEX_NONE);

	const UBOOL bIsArchetypeObject = Object->IsTemplate();
	if ( bIsArchetypeObject == TRUE )
	{
		Ar.StartSerializingDefaults();
	}

	FName PropertyName(NAME_None);
	FObjectExport& Export = Ar.ExportMap(LinkerIndex);
	Ar.Loader->Seek(Export.SerialOffset);
	Ar.Loader->Precache(Export.SerialOffset,Export.SerialSize);

	if( Object->HasAnyFlags(RF_HasStack) )
	{
		FStateFrame* DummyStateFrame = new FStateFrame(Object);

		Ar << DummyStateFrame->Node << DummyStateFrame->StateNode;
		Ar << DummyStateFrame->ProbeMask;
		Ar << DummyStateFrame->LatentAction;
		Ar << DummyStateFrame->StateStack;
		if( DummyStateFrame->Node )
		{
			Ar.Preload( DummyStateFrame->Node );
			INT Offset = DummyStateFrame->Code ? DummyStateFrame->Code - &DummyStateFrame->Node->Script(0) : INDEX_NONE;
			Ar << Offset;
			if( Offset!=INDEX_NONE )
			{
				if( Offset<0 || Offset>=DummyStateFrame->Node->Script.Num() )
				{
					appErrorf( TEXT("%s: Offset mismatch: %i %i"), *GetFullName(), Offset, DummyStateFrame->Node->Script.Num() );
				}
			}
			DummyStateFrame->Code = Offset!=INDEX_NONE ? &DummyStateFrame->Node->Script(Offset) : NULL;
		}
		else 
		{
			DummyStateFrame->Code = NULL;
		}

		delete DummyStateFrame;
	}

	if ( Object->IsA(UComponent::StaticClass()) && !Object->HasAnyFlags(RF_ClassDefaultObject) )
		((UComponent*)Object)->PreSerialize(Ar);

	Object->SerializeNetIndex(Ar);

	BYTE* Data = (BYTE*)appAlloca(256*256);
	appMemzero(Data, 256*256);

	// Load tagged properties.
	UClass* ObjClass = Object->GetClass();
	
	// This code assumes that properties are loaded in the same order they are saved in. This removes a n^2 search 
	// and makes it an O(n) when properties are saved in the same order as they are loaded (default case). In the 
	// case that a property was reordered the code falls back to a slower search.
	UProperty*	Property			= ObjClass->PropertyLink;
	UBOOL		AdvanceProperty		= 0;
	INT			RemainingArrayDim	= Property ? Property->ArrayDim : 0;

	UBOOL bDisplayedObjectName = FALSE;

	// Load all stored properties, potentially skipping unknown ones.
	while( 1 )
	{
		FPropertyTag Tag;
		Ar << Tag;
		if( Tag.Name == NAME_None )
			break;
		PropertyName = Tag.Name;

		// Move to the next property to be serialized
		if( AdvanceProperty && --RemainingArrayDim <= 0 )
		{
			Property = Property->PropertyLinkNext;
			// Skip over properties that don't need to be serialized.
			while( Property && !Property->ShouldSerializeValue( Ar ) )
			{
				Property = Property->PropertyLinkNext;
			}
			AdvanceProperty		= 0;
			RemainingArrayDim	= Property ? Property->ArrayDim : 0;
		}

		// If this property is not the one we expect (e.g. skipped as it matches the default value), do the brute force search.
		if( Property == NULL || Property->GetFName() != Tag.Name )
		{
			UProperty* CurrentProperty = Property;
			// Search forward...
			for ( ; Property; Property=Property->PropertyLinkNext )
			{
				if( Property->GetFName() == Tag.Name )
				{
					break;
				}
			}
			// ... and then search from the beginning till we reach the current property if it's not found.
			if( Property == NULL )
			{
				for( Property = ObjClass->PropertyLink; Property && Property != CurrentProperty; Property = Property->PropertyLinkNext )
				{
					if( Property->GetFName() == Tag.Name )
					{
						break;
					}
				}

				if( Property == CurrentProperty )
				{
					// Property wasn't found.
					Property = NULL;
				}
			}

			RemainingArrayDim = Property ? Property->ArrayDim : 0;
		}

		const UBOOL bShowPropertyValue = SearchProperties.Num() == 0
			|| (Property != NULL && SearchProperties.Find(Property) != NULL);

		if ( bShowPropertyValue && !bDisplayedObjectName )
		{
			bDisplayedObjectName = TRUE;
			warnf(TEXT("%s:"), *Object->GetPathName());
		}


		//@{
		//@compatibility
		// are we converting old content from UDistributionXXX properties to FRawDistributionYYY structs?
		UBOOL bDistributionHack = FALSE;
		// if we were looking for an object property, but we found a matching struct property,
		// and the struct is of type FRawDistributionYYY, then we need to do some mojo
		if (Ar.Ver() < VER_FDISTRIBUTIONS && Tag.Type == NAME_ObjectProperty && Property && Cast<UStructProperty>(Property,CLASS_IsAUStructProperty) != NULL )
		{
			FName StructName = ((UStructProperty*)Property)->Struct->GetFName();
			if (StructName == NAME_RawDistributionFloat || StructName == NAME_RawDistributionVector)
			{
				bDistributionHack = TRUE;
			}
		}
		//@}

		if( !Property )
		{
			//@{
			//@compatibility
			if (Ar.IsLoading() && Tag.Name == NAME_InitChild2StartBone)
			{
				UProperty* NewProperty = FindField<UProperty>(ObjClass, TEXT("BranchStartBoneName"));
				if (NewProperty != NULL && NewProperty->IsA(UArrayProperty::StaticClass()) && ((UArrayProperty*)NewProperty)->Inner->IsA(UNameProperty::StaticClass()))
				{
					// we don't have access to ULinkerLoad::operator<<(FName&), so we have to reproduce that code here
					FName OldName;

					// serialize the name index
					INT NameIndex;
					Ar << NameIndex;

					if ( !Ar.NameMap.IsValidIndex(NameIndex) )
					{
						appErrorf( TEXT("Bad name index %i/%i"), NameIndex, Ar.NameMap.Num() );
					}

					if ( Ar.NameMap(NameIndex) == NAME_None )
					{
						if (Ar.Ver() >= VER_FNAME_CHANGE_NAME_SPLIT)
						{
							INT TempNumber;
							Ar << TempNumber;
						}
						OldName = NAME_None;
					}
					else
					{
						if ( Ar.Ver() < VER_FNAME_CHANGE_NAME_SPLIT )
						{
							OldName = Ar.NameMap(NameIndex);
						}
						else if ( Ar.Ver() < VER_NAME_TABLE_LOADING_CHANGE )
						{
							INT Number;
							Ar << Number;

							// if there was a number and the namemap got a number from being split by the namemap serialization
							// then that means we had a name like A_6_2 that was split before to A_6, 2 and now it was split again
							// so we need to reconstruct the full thing
							if (Number && Ar.NameMap(NameIndex).GetNumber())
							{
								OldName = FName(*FString::Printf(TEXT("%s_%d"), Ar.NameMap(NameIndex).GetName(), NAME_INTERNAL_TO_EXTERNAL(Ar.NameMap(NameIndex).GetNumber())), Number);
							}
							else
							{
								// otherwise, we can just add them (since at least 1 is zero) to get the actual number
								OldName = FName((EName)Ar.NameMap(NameIndex).GetIndex(), Number + Ar.NameMap(NameIndex).GetNumber());
							}
						}
						else
						{
							INT Number;
							Ar << Number;
							// simply create the name from the NameMap's name index and the serialized instance number
							OldName = FName((EName)Ar.NameMap(NameIndex).GetIndex(), Number);
						}
					}

					((TArray<FName>*)(Data + NewProperty->Offset))->AddItem(OldName);
					AdvanceProperty = FALSE;
					continue;
				}
			}
			//@}

			debugfSlow( NAME_Warning, TEXT("Property %s of %s not found for package:  %s"), *Tag.Name.ToString(), *ObjClass->GetFullName(), *Ar.GetArchiveName().ToString() );
		}
		else if( Tag.Type==NAME_StrProperty && Property->GetID()==NAME_NameProperty )  
		{ 
			FString str;  
			Ar << str; 
			*(FName*)(Data + Property->Offset + Tag.ArrayIndex * Property->ElementSize ) = FName(*str);  
			AdvanceProperty = TRUE;

			if ( bShowPropertyValue )
			{
				FString PropertyValue;
				Property->ExportText(Tag.ArrayIndex, PropertyValue, Data, Data, NULL, PPF_Localized);

				FString PropertyNameText = *Property->GetName();
				if ( Property->ArrayDim != 1 )
					PropertyNameText += FString::Printf(TEXT("[%s]"), *appItoa(Tag.ArrayIndex));

				warnf(TEXT("\t%s%s"), *PropertyNameText.RightPad(32), *PropertyValue);
			}
			continue; 
		}
		else if( !bDistributionHack && Tag.Type!=Property->GetID() )
		{
			debugf( NAME_Warning, TEXT("Type mismatch in %s of %s - Previous (%s) Current(%s) for package:  %s"), *Tag.Name.ToString(), *GetName(), *Tag.Type.ToString(), *Property->GetID().ToString(), *Ar.GetArchiveName().ToString() );
		}
		else if( Tag.ArrayIndex>=Property->ArrayDim )
		{
			debugf( NAME_Warning, TEXT("Array bounds in %s of %s: %i/%i for package:  %s"), *Tag.Name.ToString(), *GetName(), Tag.ArrayIndex, Property->ArrayDim, *Ar.GetArchiveName().ToString() );
		}
		else if( !bDistributionHack && Tag.Type==NAME_StructProperty && Tag.ItemName!=CastChecked<UStructProperty>(Property)->Struct->GetFName() )
		{
			debugf( NAME_Warning, TEXT("Property %s of %s struct type mismatch %s/%s for package:  %s"), *Tag.Name.ToString(), *GetName(), *Tag.ItemName.ToString(), *CastChecked<UStructProperty>(Property)->Struct->GetName(), *Ar.GetArchiveName().ToString() );
		}
		else if( !Property->ShouldSerializeValue(Ar) )
		{
			if ( bShowPropertyValue )
			{
				//@{
				//@compatibility
				// we need to load any existing values for the SourceStyle property of a StyleDataReference struct so that we can save the referenced style's STYLE_ID into
				// the SourceStyleID property of this struct
				if ( Ar.Ver() < VER_ADDED_SOURCESTYLEID
					&& Ar.IsLoading()
					&& GetFName() == NAME_StyleDataReference
					&& Property->GetFName() == NAME_SourceStyle
					)
				{
					// goto may be evil in OOP land, but it's the simplest way to achieve the desired behavior with minimal impact on load times
					goto LoadPropertyValue;
				}
				//@}
				debugf( NAME_Warning, TEXT("Property %s of %s is not serializable for package:  %s"), *Tag.Name.ToString(), *GetName(), *Ar.GetArchiveName().ToString() );
			}
		}
		else if ( bShowPropertyValue )
		{
LoadPropertyValue:
			// This property is ok.
			BYTE* DestAddress = Data + Property->Offset + Tag.ArrayIndex*Property->ElementSize;

			//@{
			//@compatibility
			// if we are doing the distribution fixup hack, serialize into the FRawDistribtion's Distribution variable
			if (bDistributionHack)
			{
				UScriptStruct* RawDistributionStruct = CastChecked<UStructProperty>(Property)->Struct;

				// find the actual UDistributionXXX property inside the struct, 
				// and use that for serializing
				Property = FindField<UObjectProperty>(RawDistributionStruct, TEXT("Distribution"));

				// offset the DestAddress by the amount inside the struct this property is
				DestAddress += Property->Offset;
			}
			//@}

			Tag.SerializeTaggedProperty( Ar, Property, DestAddress, Tag.Size, NULL );

			//@{
			//@compatibility
			if ( Object->GetClass()->GetFName() == NAME_StyleDataReference 
				&& Ar.Ver() < VER_CHANGED_UISTATES
				&& Tag.Name == NAME_SourceState )
			{
				*(UObject**)DestAddress = (*(UClass**)DestAddress)->GetDefaultObject();
			}
			AdvanceProperty = TRUE;

			FString PropertyValue;
			Property->ExportText(Tag.ArrayIndex, PropertyValue, Data, Data, NULL, PPF_Localized);

			FString PropertyNameText = *Property->GetName();
			if ( Property->ArrayDim != 1 )
				PropertyNameText += FString::Printf(TEXT("[%s]"), *appItoa(Tag.ArrayIndex));

			warnf(TEXT("\t%s%s"), *PropertyNameText.RightPad(32), *PropertyValue);
			continue;
		}

		if ( !bShowPropertyValue )
		{
			// if we're not supposed to show the value for this property, just skip it without logging a warning
			AdvanceProperty = TRUE;
			BYTE B;
			for( INT i=0; i<Tag.Size; i++ )
			{
				Ar << B;
			}

			continue;
		}

		AdvanceProperty = FALSE;

		// Skip unknown or bad property.
		debugfSlow( NAME_Warning, TEXT("Skipping %i bytes of type %s for package:  %s"), Tag.Size, *Tag.Type.ToString(), *Ar.GetArchiveName().ToString() );
			
		BYTE B;
		for( INT i=0; i<Tag.Size; i++ )
		{
			Ar << B;
		}
	}

	if ( bDisplayedObjectName )
		warnf(TEXT(""));

	if ( bIsArchetypeObject == TRUE )
	{
		Ar.StopSerializingDefaults();
	}
}

IMPLEMENT_CLASS(UShowTaggedPropsCommandlet)

/*-----------------------------------------------------------------------------
	UListPackagesReferencing commandlet.
-----------------------------------------------------------------------------*/

/**
 * Contains the linker name and filename for a package which is referencing another package.
 */
struct FReferencingPackageName
{
	/** the name of the linker root (package name) */
	FName LinkerFName;

	/** the complete filename for the package */
	FString Filename;

	/** Constructor */
	FReferencingPackageName( FName InLinkerFName, const FString& InFilename )
	: LinkerFName(InLinkerFName), Filename(InFilename)
	{
	}

	/** Comparison operator */
	inline UBOOL operator==( const FReferencingPackageName& Other )
	{
		return LinkerFName == Other.LinkerFName;
	}
};

inline DWORD GetTypeHash( const FReferencingPackageName& ReferencingPackageStruct )
{
	return GetTypeHash(ReferencingPackageStruct.LinkerFName);
}

IMPLEMENT_COMPARE_CONSTREF(FReferencingPackageName,UnPackageUtilities,{ return appStricmp(*A.LinkerFName.ToString(),*B.LinkerFName.ToString()); });

INT UListPackagesReferencingCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	TLookupMap<FReferencingPackageName>	ReferencingPackages;
	TArray<FString> PackageFiles = GPackageFileCache->GetPackageFileList();


	//@todo ronp - add support for searching for references to multiple packages/resources at once.

	FString SearchName;
	if( ParseToken(Parms, SearchName, 0) )
	{

		// determine whether we're searching references to a package or a specific resource
		INT delimPos = SearchName.InStr(TEXT("."), TRUE);

		// if there's no dots in the search name, or the last part of the name is one of the registered package extensions, we're searching for a package
		const UBOOL bIsPackage = delimPos == INDEX_NONE || GSys->Extensions.FindItemIndex(SearchName.Mid(delimPos+1)) != INDEX_NONE;

		FName SearchPackageFName=NAME_None;
		if ( bIsPackage == TRUE )
		{
			// remove any extensions on the package name
			SearchPackageFName = FName(*FFilename(SearchName).GetBaseFilename());
		}
		else
		{
			// validate that this resource exists
			UObject* SearchObject = StaticLoadObject(UObject::StaticClass(), NULL, *SearchName, NULL, LOAD_NoWarn, NULL);
			if ( SearchObject == NULL )
			{
				warnf(TEXT("Unable to load specified resource: %s"), *SearchName);
				return 1;
			}

			// searching for a particular resource - pull off the package name
			SearchPackageFName = SearchObject->GetOutermost()->GetFName();

			// then change the SearchName to the object's actual path name, in case the name passed on the command-line wasn't a complete path name
			SearchName = SearchObject->GetPathName();
		}

		INT GCIndex = 0;
		for( INT FileIndex=0; FileIndex<PackageFiles.Num(); FileIndex++ )
		{
			const FString &Filename = PackageFiles(FileIndex);

			warnf(NAME_Progress, TEXT("Loading '%s'..."), *Filename);

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_Quiet|LOAD_NoWarn, NULL, NULL );
			UObject::EndLoad();

			if( Linker )
			{
				FName LinkerFName = Linker->LinkerRoot->GetFName();

				// ignore the package if it's the one we're processing
				if( LinkerFName != SearchPackageFName )
				{
					// look for the search package in this package's ImportMap.
					for( INT ImportIndex=0; ImportIndex<Linker->ImportMap.Num(); ImportIndex++ )
					{
						FObjectImport& Import = Linker->ImportMap( ImportIndex );
						UBOOL bImportReferencesSearchPackage = FALSE;

						if ( bIsPackage == TRUE )
						{
							if ( Import.ClassPackage == SearchPackageFName )
							{
								// this import's class is contained in the package we're searching for references to
								bImportReferencesSearchPackage = TRUE;
							}
							else if ( Import.ObjectName == SearchPackageFName && Import.ClassName == NAME_Package && Import.ClassPackage == NAME_Core )
							{
								// this import is the package we're searching for references to
								bImportReferencesSearchPackage = TRUE;
							}
							else if ( Import.OuterIndex != ROOTPACKAGE_INDEX )
							{
								// otherwise, determine if this import's source package is the package we're searching for references to
								// Import.SourceLinker is cleared in UObject::EndLoad, so we can't use that
								PACKAGE_INDEX OutermostLinkerIndex = Import.OuterIndex;
								for ( PACKAGE_INDEX LinkerIndex = Import.OuterIndex; LinkerIndex != ROOTPACKAGE_INDEX; )
								{
									OutermostLinkerIndex = LinkerIndex;

									// this import's outer might be in the export table if the package was saved for seek-free loading
									if ( IS_IMPORT_INDEX(LinkerIndex) )
									{
										LinkerIndex = Linker->ImportMap( -LinkerIndex - 1 ).OuterIndex;
									}
									else
									{
										LinkerIndex = Linker->ExportMap( LinkerIndex - 1 ).OuterIndex;
									}
								}

								// if the OutermostLinkerIndex is ROOTPACKAGE_INDEX, this import corresponds to the root package for this linker
								if ( IS_IMPORT_INDEX(OutermostLinkerIndex) )
								{
									FObjectImport& PackageImport = Linker->ImportMap( -OutermostLinkerIndex - 1 );
									bImportReferencesSearchPackage =	PackageImport.ObjectName	== SearchPackageFName &&
																		PackageImport.ClassName		== NAME_Package &&
																		PackageImport.ClassPackage	== NAME_Core;
								}
								else
								{
									check(OutermostLinkerIndex != ROOTPACKAGE_INDEX);

									FObjectExport& PackageExport = Linker->ExportMap( OutermostLinkerIndex - 1 );
									bImportReferencesSearchPackage =	PackageExport.ObjectName == SearchPackageFName;
								}
							}
						}
						else
						{
							FString ImportPathName = Linker->GetImportPathName(ImportIndex);
							if ( SearchName == ImportPathName )
							{
								// this is the object we're search for
								bImportReferencesSearchPackage = TRUE;
							}
							else
							{
								// see if this import's class is the resource we're searching for
								FString ImportClassPathName = Import.ClassPackage.ToString() + TEXT(".") + Import.ClassName.ToString();
								if ( ImportClassPathName == SearchName )
								{
									bImportReferencesSearchPackage = TRUE;
								}
								else if ( Import.OuterIndex > ROOTPACKAGE_INDEX )
								{
									// and OuterIndex > 0 indicates that the import's Outer is in the package's export map, which would happen
									// if the package was saved for seek-free loading;
									// we need to check the Outer in this case since we are only iterating through the ImportMap
									FString OuterPathName = Linker->GetExportPathName(Import.OuterIndex - 1);
									if ( SearchName == OuterPathName )
									{
										bImportReferencesSearchPackage = TRUE;
									}
								}
							}
						}

						if ( bImportReferencesSearchPackage )
						{
							ReferencingPackages.AddItem( FReferencingPackageName(LinkerFName, Filename) );
							break;
						}
					}
				}
			}

			// only GC every 10 packages (A LOT faster this way, and is safe, since we are not 
			// acting on objects that would need to go away or anything)
			if ((++GCIndex % 10) == 0)
			{
				UObject::CollectGarbage(RF_Native);
			}
		}

		warnf( TEXT("%i packages reference %s:"), ReferencingPackages.Num(), *SearchName );

		// calculate the amount of padding to use when listing the referencing packages
		INT Padding=appStrlen(TEXT("Package Name"));
		for( INT ReferencerIndex=0; ReferencerIndex<ReferencingPackages.Num(); ReferencerIndex++ )
		{
			Padding = Max(Padding, ReferencingPackages(ReferencerIndex).LinkerFName.ToString().Len());
		}

		warnf( TEXT("  %*s  Filename"), Padding, TEXT("Package Name"));
		
		// KeySort shouldn't be used with TLookupMap because then the Value for each pair in the Pairs array (which is the index into the Pairs array for that pair)
		// is no longer correct.  That doesn't matter to use because we don't use the value for anything, so sort away!
		ReferencingPackages.KeySort<COMPARE_CONSTREF_CLASS(FReferencingPackageName,UnPackageUtilities)>();

		// output the list of referencers
		for( INT ReferencerIndex=0; ReferencerIndex<ReferencingPackages.Num(); ReferencerIndex++ )
		{
			warnf( TEXT("  %*s  %s"), Padding, *ReferencingPackages(ReferencerIndex).LinkerFName.ToString(), *ReferencingPackages(ReferencerIndex).Filename );
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UListPackagesReferencingCommandlet)

/*-----------------------------------------------------------------------------
	UPkgInfo commandlet.
-----------------------------------------------------------------------------*/

struct FExportInfo
{
	FName	Name;
	INT		Size;
};

IMPLEMENT_COMPARE_CONSTREF( FExportInfo, UnPackageUtilities, { return B.Size - A.Size; } )

enum EPackageInfoFlags
{
	PKGINFO_None		=0x00,
	PKGINFO_Names		=0x01,
	PKGINFO_Imports		=0x02,
	PKGINFO_Exports		=0x04,
	PKGINFO_Compact		=0x08,
	PKGINFO_Chunks		=0x10,

	PKGINFO_All			= PKGINFO_Names|PKGINFO_Imports|PKGINFO_Exports|PKGINFO_Chunks,
};

INT UPkgInfoCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	TArray<FString> Tokens, Switches;
	ParseCommandLine(Parms, Tokens, Switches);

	// find out which type of info we're looking for
	DWORD InfoFlags = PKGINFO_None;
	if ( Switches.ContainsItem(TEXT("names")) )
	{
		InfoFlags |= PKGINFO_Names;
	}
	if ( Switches.ContainsItem(TEXT("imports")) )
	{
		InfoFlags |= PKGINFO_Imports;
	}
	if ( Switches.ContainsItem(TEXT("exports")) )
	{
		InfoFlags |= PKGINFO_Exports;
	}
	if ( Switches.ContainsItem(TEXT("simple")) )
	{
		InfoFlags |= PKGINFO_Compact;
	}
	if ( Switches.ContainsItem(TEXT("chunks")) )
	{
		InfoFlags |= PKGINFO_Chunks;
	}
	if ( Switches.ContainsItem(TEXT("all")) )
	{
		InfoFlags |= PKGINFO_All;
	}

	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		FString& PackageWildcard = Tokens(TokenIndex);
		
		TArray<FString> FilesInPath;
		GFileManager->FindFiles( FilesInPath, *PackageWildcard, TRUE, FALSE );
		if( FilesInPath.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles( FilesInPath, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

			if ( FilesInPath.Num() == 0 )
			{
				TArray<FString> Paths;
				if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
				{
					for ( INT i = 0; i < Paths.Num(); i++ )
					{
						GFileManager->FindFiles( FilesInPath, *(Paths(i) * PackageWildcard), 1, 0 );
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
				{
					FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
				}
			}

			// Try finding package in package file cache.
			if ( FilesInPath.Num() == 0 )
			{
				FString Filename;
				if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
				{
					new(FilesInPath)FString(Filename);
				}
			}
		}
		else
		{
			// re-add the path information so that GetPackageLinker finds the correct version of the file.
			FFilename WildcardPath = PackageWildcard;
			for ( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
			{
				FilesInPath(FileIndex) = WildcardPath.GetPath() * FilesInPath(FileIndex);
			}
		}

		if ( FilesInPath.Num() == 0 )
		{
			warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
			continue;
		}

		for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
		{
			const FString &Filename = FilesInPath(FileIndex);

			{
				// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
				// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
				const FString& PackageName = FPackageFileCache::PackageFromPath(*Filename);
				UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
				if ( ExistingPackage != NULL )
				{
					ResetLoaders(ExistingPackage);
				}
			}

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_NoVerify, NULL, NULL );
			UObject::EndLoad();

			if( Linker )
			{
				if ( FileIndex > 0 )
				{
					warnf(TEXT(""));
				}

				// Display information about the package.
				FName LinkerName = Linker->LinkerRoot->GetFName();

				// Display summary info.
				GWarn->Log( TEXT("********************************************") );
				GWarn->Logf( TEXT("Package '%s' Summary"), *LinkerName.ToString() );
				GWarn->Log( TEXT("--------------------------------------------") );

				GWarn->Logf( TEXT("\t         Filename: %s"), *Filename);
				GWarn->Logf( TEXT("\t     File Version: %i"), Linker->Ver() );
				GWarn->Logf( TEXT("\t   Engine Version: %d"), Linker->Summary.EngineVersion);
				GWarn->Logf( TEXT("\t   Cooker Version: %d"), Linker->Summary.CookedContentVersion);
				GWarn->Logf( TEXT("\t     PackageFlags: %X"), Linker->Summary.PackageFlags );
				GWarn->Logf( TEXT("\t        NameCount: %d"), Linker->Summary.NameCount );
				GWarn->Logf( TEXT("\t       NameOffset: %d"), Linker->Summary.NameOffset );
				GWarn->Logf( TEXT("\t      ImportCount: %d"), Linker->Summary.ImportCount );
				GWarn->Logf( TEXT("\t     ImportOffset: %d"), Linker->Summary.ImportOffset );
				GWarn->Logf( TEXT("\t      ExportCount: %d"), Linker->Summary.ExportCount );
				GWarn->Logf( TEXT("\t     ExportOffset: %d"), Linker->Summary.ExportOffset );
				GWarn->Logf( TEXT("\tCompression Flags: %X"), Linker->Summary.CompressionFlags);

				FString szGUID = Linker->Summary.Guid.String();
				GWarn->Logf( TEXT("\t             Guid: %s"), *szGUID );
				GWarn->Log ( TEXT("\t      Generations:"));
				for( INT i = 0; i < Linker->Summary.Generations.Num(); ++i )
				{
					const FGenerationInfo& generationInfo = Linker->Summary.Generations( i );
					GWarn->Logf(TEXT("\t\t\t%d) ExportCount=%d, NameCount=%d, NetObjectCount=%d"), i, generationInfo.ExportCount, generationInfo.NameCount, generationInfo.NetObjectCount);
				}


				if( (InfoFlags&PKGINFO_Chunks) != 0 )
				{
					GWarn->Log( TEXT("--------------------------------------------") );
					GWarn->Log ( TEXT("Compression Chunks"));
					GWarn->Log ( TEXT("=========="));
					
					for ( INT ChunkIndex = 0; ChunkIndex < Linker->Summary.CompressedChunks.Num(); ChunkIndex++ )
					{
						FCompressedChunk& Chunk = Linker->Summary.CompressedChunks(ChunkIndex);
						GWarn->Log ( TEXT("\t*************************"));
						GWarn->Logf( TEXT("\tChunk %d:"), ChunkIndex );
						GWarn->Logf( TEXT("\t\tUncompressedOffset: %d"), Chunk.UncompressedOffset);
						GWarn->Logf( TEXT("\t\t  UncompressedSize: %d"), Chunk.UncompressedSize);
						GWarn->Logf( TEXT("\t\t  CompressedOffset: %d"), Chunk.CompressedOffset);
						GWarn->Logf( TEXT("\t\t    CompressedSize: %d"), Chunk.CompressedSize);
					}
				}

				if( (InfoFlags&PKGINFO_Names) != 0 )
				{
					GWarn->Log( TEXT("--------------------------------------------") );
					GWarn->Log ( TEXT("Name Map"));
					GWarn->Log ( TEXT("========"));
					for( INT i = 0; i < Linker->NameMap.Num(); ++i )
					{
						FName& name = Linker->NameMap( i );
						GWarn->Logf( TEXT("\t%d: Name '%s' Index %d [Internal: %s, %d]"), i, *name.ToString(), name.GetIndex(), name.GetName(), name.GetNumber() );
					}
				}

				// if we _only_ want name info, skip this part completely
				if ( InfoFlags != PKGINFO_Names )
				{
					if( (InfoFlags&PKGINFO_Imports) != 0 )
					{
						GWarn->Log( TEXT("--------------------------------------------") );
						GWarn->Log ( TEXT("Import Map"));
						GWarn->Log ( TEXT("=========="));
					}

					TArray<FName> DependentPackages;
					for( INT i = 0; i < Linker->ImportMap.Num(); ++i )
					{
						FObjectImport& import = Linker->ImportMap( i );

						FName PackageName = NAME_None;
						FName OuterName = NAME_None;
						if ( import.OuterIndex != ROOTPACKAGE_INDEX )
						{
							if ( IS_IMPORT_INDEX(import.OuterIndex) )
							{
								FObjectImport& OuterImport = Linker->ImportMap(-import.OuterIndex-1);
								OuterName = OuterImport.ObjectName;
							}
							else if ( import.OuterIndex < 0 )
							{
								FObjectExport& OuterExport = Linker->ExportMap(import.OuterIndex-1);
								OuterName = OuterExport.ObjectName;
							}

							// Find the package which contains this import.  import.SourceLinker is cleared in UObject::EndLoad, so we'll need to do this manually now.
							PACKAGE_INDEX OutermostLinkerIndex = import.OuterIndex;
							for ( PACKAGE_INDEX LinkerIndex = import.OuterIndex; LinkerIndex != ROOTPACKAGE_INDEX; )
							{
								OutermostLinkerIndex = LinkerIndex;

								// this import's outer might be in the export table if the package was saved for seek-free loading
								if ( IS_IMPORT_INDEX(LinkerIndex) )
								{
									LinkerIndex = Linker->ImportMap( -LinkerIndex - 1 ).OuterIndex;
								}
								else
								{
									LinkerIndex = Linker->ExportMap( LinkerIndex - 1 ).OuterIndex;
								}
							}

							// if the OutermostLinkerIndex is ROOTPACKAGE_INDEX, this import corresponds to the root package for this linker
							if ( IS_IMPORT_INDEX(OutermostLinkerIndex) )
							{
								FObjectImport& PackageImport = Linker->ImportMap( -OutermostLinkerIndex - 1 );
								PackageName = PackageImport.ObjectName;
							}
							else
							{
								check(OutermostLinkerIndex != ROOTPACKAGE_INDEX);
								FObjectExport& PackageExport = Linker->ExportMap( OutermostLinkerIndex - 1 );
								PackageName = PackageExport.ObjectName;
							}
						}

						if ( (InfoFlags&PKGINFO_Imports) != 0 )
						{
							GWarn->Log ( TEXT("\t*************************"));
							GWarn->Logf( TEXT("\tImport %d: '%s'"), i, *import.ObjectName.ToString() );
							GWarn->Logf( TEXT("\t\t       Outer: '%s' (%d)"), *OuterName.ToString(), import.OuterIndex);
							GWarn->Logf( TEXT("\t\t     Package: '%s'"), *PackageName.ToString());
							GWarn->Logf( TEXT("\t\t       Class: '%s'"), *import.ClassName.ToString() );
							GWarn->Logf( TEXT("\t\tClassPackage: '%s'"), *import.ClassPackage.ToString() );
							GWarn->Logf( TEXT("\t\t     XObject: %s"), import.XObject ? TEXT("VALID") : TEXT("NULL"));
							GWarn->Logf( TEXT("\t\t SourceIndex: %d"), import.SourceIndex );
						}

						if ( PackageName == NAME_None && import.ClassPackage == NAME_Core && import.ClassName == NAME_Package )
						{
							PackageName = import.ObjectName;
						}

						if ( PackageName != NAME_None && PackageName != LinkerName )
						{
							DependentPackages.AddUniqueItem(PackageName);
						}

						if ( import.ClassPackage != NAME_None && import.ClassPackage != LinkerName )
						{
							DependentPackages.AddUniqueItem(import.ClassPackage);
						}
					}

					if ( DependentPackages.Num() )
					{
						GWarn->Log( TEXT("--------------------------------------------") );
						warnf(TEXT("\tPackages referenced by %s:"), *LinkerName.ToString());
						for ( INT i = 0; i < DependentPackages.Num(); i++ )
						{
							warnf(TEXT("\t\t%i) %s"), i, *DependentPackages(i).ToString());
						}
					}
				}

				if( (InfoFlags&PKGINFO_Exports) != 0 )
				{
					GWarn->Log( TEXT("--------------------------------------------") );
					GWarn->Log ( TEXT("Export Map"));
					GWarn->Log ( TEXT("=========="));

					if ( (InfoFlags&PKGINFO_Compact) == 0 )
					{
						for( INT i = 0; i < Linker->ExportMap.Num(); ++i )
						{
							GWarn->Log ( TEXT("\t*************************"));
							FObjectExport& Export = Linker->ExportMap( i );
							GWarn->Logf( TEXT("\tExport %d: '%s'"), i, *Export.ObjectName.ToString() );

							// find the name of this object's class
							INT ClassIndex = Export.ClassIndex;
							FName ClassName = ClassIndex>0 
								? Linker->ExportMap(ClassIndex-1).ObjectName
								: ClassIndex<0 
									? Linker->ImportMap(-ClassIndex-1).ObjectName
									: FName(NAME_Class);

							// find the name of this object's parent...for UClasses, this will be the parent class
							// for UFunctions, this will be the SuperFunction, if it exists, etc.
							FName ParentName = NAME_None;
							if ( Export.SuperIndex > 0 )
							{
								FObjectExport& ParentExport = Linker->ExportMap(Export.SuperIndex-1);
								ParentName = ParentExport.ObjectName;
							}
							else if ( Export.SuperIndex < 0 )
							{
								FObjectImport& ParentImport = Linker->ImportMap(-Export.SuperIndex-1);
								ParentName = ParentImport.ObjectName;
							}

							// find the name of this object's Outer.  For UClasses, this will generally be the
							// top-level package itself.  For properties, a UClass, etc.
							FName OuterName = NAME_None;
							if ( Export.OuterIndex > 0 )
							{
								FObjectExport& OuterExport = Linker->ExportMap(Export.OuterIndex-1);
								OuterName = OuterExport.ObjectName;
							}
							else if ( Export.OuterIndex < 0 )
							{
								FObjectImport& OuterImport = Linker->ImportMap(-Export.OuterIndex-1);
								OuterName = OuterImport.ObjectName;
							}

							FName TemplateName = NAME_None;
							if ( Export.ArchetypeIndex > 0 )
							{
								FObjectExport& TemplateExport = Linker->ExportMap(Export.ArchetypeIndex-1);
								TemplateName = TemplateExport.ObjectName;
							}
							else if ( Export.ArchetypeIndex < 0 )
							{
								FObjectImport& TemplateImport = Linker->ImportMap(-Export.ArchetypeIndex-1);
								TemplateName = TemplateImport.ObjectName;
							}

							GWarn->Logf( TEXT("\t\t       Class: '%s' (%i)"), *ClassName.ToString(), ClassIndex );
							GWarn->Logf( TEXT("\t\t      Parent: '%s' (%d)"), *ParentName.ToString(), Export.SuperIndex );
							GWarn->Logf( TEXT("\t\t       Outer: '%s' (%d)"), *OuterName.ToString(), Export.OuterIndex );
							GWarn->Logf( TEXT("\t\t   Archetype: '%s' (%d)"), *TemplateName.ToString(), Export.ArchetypeIndex);
							GWarn->Logf( TEXT("\t\t ObjectFlags: 0x%016I64X"), Export.ObjectFlags );
							GWarn->Logf( TEXT("\t\t        Size: %d"), Export.SerialSize );
							GWarn->Logf( TEXT("\t\t      Offset: %d"), Export.SerialOffset );
							GWarn->Logf( TEXT("\t\t     _Object: %s"), Export._Object ? TEXT("VALID") : TEXT("NULL"));
							GWarn->Logf( TEXT("\t\t  _iHashNext: %d"), Export._iHashNext );
							GWarn->Logf( TEXT("\t\t ExportFlags: %x"), Export.ExportFlags );
							if ( Export.ComponentMap.Num() )
							{
								GWarn->Log(TEXT("\t\tComponentMap:"));
								INT Count = 0;
								for ( TMap<FName,INT>::TIterator It(Export.ComponentMap); It; ++It )
								{
									GWarn->Logf(TEXT("\t\t\t%i) %s (%i)"), Count++, *It.Key().ToString(), It.Value());
								}
							}
						}
					}
					else
					{
						for( INT ExportIndex=0; ExportIndex<Linker->ExportMap.Num(); ExportIndex++ )
						{
							const FObjectExport& Export = Linker->ExportMap(ExportIndex);
							warnf(TEXT("  %8i %10i %s"), ExportIndex, Export.SerialSize, *Export.ObjectName.ToString());
						}
					}
				}
			}

			UObject::CollectGarbage(RF_Native);
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UPkgInfoCommandlet)


/*-----------------------------------------------------------------------------
	UListCorruptedComponentsCommandlet
-----------------------------------------------------------------------------*/

/**
 * This commandlet is designed to find (and in the future, possibly fix) content that is affected by the components bug described in
 * TTPRO #15535 and UComponentProperty::InstanceComponents()
 */
INT UListCorruptedComponentsCommandlet::Main(const FString& Params)
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

	UBOOL bCheckVersion = Switches.ContainsItem(TEXT("CHECKVER"));

	// Iterate over all files doing stuff.
	for( INT FileIndex = 0 ; FileIndex < FilesInPath.Num() ; ++FileIndex )
	{
		const FFilename& Filename = FilesInPath(FileIndex);
		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		UObject* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}
		else if( bCheckVersion && Package->GetLinkerVersion() != GPackageFileVersion )
		{
			warnf( NAME_Log, TEXT("Version mismatch. Package [%s] should be resaved."), *Filename );
		}

		UBOOL bInsertNewLine = FALSE;
		for ( TObjectIterator<UComponent> It; It; ++It )
		{
			if ( It->IsIn(Package) && !It->IsTemplate(RF_ClassDefaultObject) )
			{
				UComponent* Component = *It;
				UComponent* ComponentTemplate = Cast<UComponent>(Component->GetArchetype());
				UObject* Owner = Component->GetOuter();
				UObject* TemplateOwner = ComponentTemplate->GetOuter();
				if ( !ComponentTemplate->HasAnyFlags(RF_ClassDefaultObject) )
				{
					if ( TemplateOwner != Owner->GetArchetype() )
					{
						bInsertNewLine = TRUE;

						FString RealArchetypeName;
						if ( Component->TemplateName != NAME_None )
						{
							UComponent* RealArchetype = Owner->GetArchetype()->FindComponent(Component->TemplateName);
							if ( RealArchetype != NULL )
							{
								RealArchetypeName = RealArchetype->GetFullName();
							}
							else
							{
								RealArchetypeName = FString::Printf(TEXT("NULL: no matching components found in Owner Archetype %s"), *Owner->GetArchetype()->GetFullName());
							}
						}
						else
						{
							RealArchetypeName = TEXT("NULL");
						}

						warnf(TEXT("\tPossible corrupted component: '%s'	Archetype: '%s'	TemplateName: '%s'	ResolvedArchetype: '%s'"),
							*Component->GetFullName(), 
							*ComponentTemplate->GetPathName(),
							*Component->TemplateName.ToString(),
							*RealArchetypeName);
					}

					if ( Component->GetClass()->HasAnyClassFlags(CLASS_UniqueComponent) )
					{
						bInsertNewLine = TRUE;
						warnf(TEXT("\tComponent is using unique component class: %s"), *Component->GetFullName());
					}

					if ( ComponentTemplate->GetClass()->HasAnyClassFlags(CLASS_UniqueComponent) )
					{
						bInsertNewLine = TRUE;
						warnf(TEXT("\tComponent archetype has unique component class: '%s'	Archetype: '%s'"), *Component->GetFullName(), *ComponentTemplate->GetPathName());
					}
				}
			}
		}

		if ( bInsertNewLine )
		{
			warnf(TEXT(""));
		}
		UObject::CollectGarbage( RF_Native );
	}

	return 0;
}

IMPLEMENT_CLASS(UListCorruptedComponentsCommandlet);

/*-----------------------------------------------------------------------------
	UAnalyzeCookedPackages commandlet.
-----------------------------------------------------------------------------*/

INT UAnalyzeCookedPackagesCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Tokens on the command line are package wildcards.
	for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
	{
		// Find all files matching current wildcard.
		FFilename		Wildcard = Tokens(TokenIndex);
		TArray<FString> Filenames;
		GFileManager->FindFiles( Filenames, *Wildcard, TRUE, FALSE );
	
		// Iterate over all found files.
		for( INT FileIndex = 0; FileIndex < Filenames.Num(); FileIndex++ )
		{
			const FString& Filename = Wildcard.GetPath() + PATH_SEPARATOR + Filenames(FileIndex);

			UObject::BeginLoad();
			ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_None, NULL, NULL );
			UObject::EndLoad();

			if( Linker )
			{
				check(Linker->LinkerRoot);
				check(Linker->Summary.PackageFlags & PKG_Cooked);

				// Display information about the package.
				FName LinkerName = Linker->LinkerRoot->GetFName();

				// Display summary info.
				GWarn->Logf( TEXT("********************************************") );
				GWarn->Logf( TEXT("Package '%s' Summary"), *LinkerName.ToString() );
				GWarn->Logf( TEXT("--------------------------------------------") );

				GWarn->Logf( TEXT("\t     Version: %i"), Linker->Ver() );
				GWarn->Logf( TEXT("\tPackageFlags: %x"), Linker->Summary.PackageFlags );
				GWarn->Logf( TEXT("\t   NameCount: %d"), Linker->Summary.NameCount );
				GWarn->Logf( TEXT("\t  NameOffset: %d"), Linker->Summary.NameOffset );
				GWarn->Logf( TEXT("\t ImportCount: %d"), Linker->Summary.ImportCount );
				GWarn->Logf( TEXT("\tImportOffset: %d"), Linker->Summary.ImportOffset );
				GWarn->Logf( TEXT("\t ExportCount: %d"), Linker->Summary.ExportCount );
				GWarn->Logf( TEXT("\tExportOffset: %d"), Linker->Summary.ExportOffset );

				FString szGUID = Linker->Summary.Guid.String();
				GWarn->Logf( TEXT("\t        Guid: %s"), *szGUID );
				GWarn->Logf( TEXT("\t Generations:"));
				for( INT i=0; i<Linker->Summary.Generations.Num(); i++ )
				{
					const FGenerationInfo& GenerationInfo = Linker->Summary.Generations( i );
					GWarn->Logf( TEXT("\t\t%d) ExportCount=%d, NameCount=%d"), i, GenerationInfo.ExportCount, GenerationInfo.NameCount );
				}

				GWarn->Logf( TEXT("") );
				GWarn->Logf( TEXT("Exports:") );
				GWarn->Logf( TEXT("Class Outer Name Size Offset ExportFlags ObjectFlags") );

				for( INT i = 0; i < Linker->ExportMap.Num(); ++i )
				{
					FObjectExport& Export = Linker->ExportMap( i );
	
					// Find the name of this object's class.
					INT ClassIndex	= Export.ClassIndex;
					FName ClassName = NAME_Class;
					if( ClassIndex > 0 )
					{
						ClassName = Linker->ExportMap(ClassIndex-1).ObjectName;
					}
					else if( ClassIndex < 0 )
					{
						Linker->ImportMap(-ClassIndex-1).ObjectName;
					}
					
					// Find the name of this object's Outer.  For UClasses, this will generally be the
					// top-level package itself.  For properties, a UClass, etc.
					FName OuterName = NAME_None;
					if ( Export.OuterIndex > 0 )
					{
						FObjectExport& OuterExport = Linker->ExportMap(Export.OuterIndex-1);
						OuterName = OuterExport.ObjectName;
					}
					else if ( Export.OuterIndex < 0 )
					{
						FObjectImport& OuterImport = Linker->ImportMap(-Export.OuterIndex-1);
						OuterName = OuterImport.ObjectName;
					}

					//GWarn->Logf( TEXT("Class Outer Name Size Offset ExportFlags ObjectFlags") );
					GWarn->Logf( TEXT("%s %s %s %i %i %x 0x%016I64X"), 
									*ClassName.ToString(), 
									*OuterName.ToString(), 
									*Export.ObjectName.ToString(), 
									Export.SerialSize, 
									Export.SerialOffset, 
									Export.ExportFlags, 
									Export.ObjectFlags );
				}
			}

			UObject::CollectGarbage(RF_Native);
		}
	}
	return 0;
}
IMPLEMENT_CLASS(UAnalyzeCookedPackagesCommandlet)



/**
 * This will look for Textures which have "SPEC" in their name and see if they have an LODBias of two
 **/
struct LODBiasFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* Texture2D = *It;

			if( Texture2D->IsIn( Package ) == FALSE )
			{
				continue;
			}

			const FString&  TextureName = Texture2D->GetPathName();
			const INT		LODBias     = Texture2D->LODBias;

			//warnf( TEXT( " checking %s" ), *TextureName );

			// if this has been named as a specular texture
			if( ( TextureName.ToUpper().InStr( TEXT("SPEC" )) != INDEX_NONE )
				&& ( LODBias != 2 )
				)
			{
				warnf( TEXT("Specular LODBias of 2 Not Set for:  %s ( Currently has %d )"), *TextureName, LODBias );
				UObject::CollectGarbage(RF_Native);
			}

			// if this has a negative LOD Bias that might be suspicious :-) (artists will often bump up the LODBias on their pet textures to make certain they are full full res)
			if( ( LODBias < 0 )
				)
			{
				warnf( TEXT("LODBias is negative for: %s ( Currently has %d )"), *TextureName, LODBias );
				UObject::CollectGarbage(RF_Native);
			}

			if( ( Texture2D->NeverStream == TRUE )
				)
			{
				warnf( TEXT("NeverStream is set to true for: %s"), *TextureName );
				UObject::CollectGarbage(RF_Native);
			}

		}
	}
};


/**
 * This will find materials which are missing Physical Materials
 **/
struct MaterialMissingPhysMaterialFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* Material = *It;

			if( Material->IsIn( Package ) == FALSE )
			{
				continue;
			}

			UBOOL bHasPhysicalMaterial = FALSE;

			if( Material->PhysMaterial != NULL )
			{
				// if we want to do some other logic such as looking at the textures
				// used and seeing if they are all of one type (e.g. all effects textures probably
				// imply that the material is used in an effect and probably doesn't need a physical material)
				//TArray<UTexture*> OutTextures;
				//Material->GetTextures( OutTextures, TRUE );
				//const FTextureLODGroup& LODGroup = TextureLODGroups[Texture->LODGroup];

				bHasPhysicalMaterial = TRUE;
			}

			if( bHasPhysicalMaterial == FALSE )
			{
				const FString& MaterialName = Material->GetPathName();
				warnf( TEXT("Lacking PhysicalMaterial:  %s"), *MaterialName  );
				UObject::CollectGarbage(RF_Native);
			}
		}
	}
};


/**
 * This will find SoundCues which are missing Sound Groups.
 **/
struct SoundCueMissingGroupsFunctor
{
	template< typename OBJECTYPE >
	void DoIt( UPackage* Package )
	{
		for( TObjectIterator<OBJECTYPE> It; It; ++It )
		{
			OBJECTYPE* TheSoundCue = *It;

			if( TheSoundCue->IsIn( Package ) == FALSE )
			{
				continue;
			}

			UBOOL bHasAGroup = FALSE;

			if( TheSoundCue->SoundGroup != NAME_None )
			{
				// if we want to do some other logic such as looking at the textures
				// used and seeing if they are all of one type (e.g. all effects textures probably
				// imply that the material is used in an effect and probably doesn't need a physical material)
				//TArray<UTexture*> OutTextures;
				//Material->GetTextures( OutTextures, TRUE );
				//const FTextureLODGroup& LODGroup = TextureLODGroups[Texture->LODGroup];

				bHasAGroup = TRUE;
			}

			if( bHasAGroup == FALSE )
			{
				const FString& TheSoundCueName = TheSoundCue->GetPathName();
				warnf( TEXT("Lacking a Group:  %s"), *TheSoundCueName );
				UObject::CollectGarbage(RF_Native);
			}
		}
	}
};


/**
 * This is our Functional "Do an Action to all Packages" Template.  Basically it handles all
 * of the boilerplate code which normally gets copy pasted around.  So now we just pass in
 * the OBJECTYPE  (e.g. Texture2D ) and then the Functor which will do the actual work.
 *
 * @see UFindMissingPhysicalMaterialsCommandlet
 * @see UFindTexturesWhichLackLODBiasOfTwoCommandlet
 **/
template< typename OBJECTYPE, typename FUNCTOR >
void DoActionToAllPackages( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	UCommandlet::ParseCommandLine(Parms, Tokens, Switches);

	TArray<FString> FilesInPath;
	FilesInPath = GPackageFileCache->GetPackageFileList();

	INT GCIndex = 0;
	for( INT FileIndex = 0; FileIndex < FilesInPath.Num(); FileIndex++ )
	{
		const FFilename& Filename = FilesInPath(FileIndex);
		warnf( NAME_Log, TEXT("Loading %s"), *Filename );

		// we don't care about trying to wrangle the various shader caches so just skipz0r them
		if(	Filename.GetBaseFilename().InStr( TEXT("LocalShaderCache") )	!= INDEX_NONE
			||	Filename.GetBaseFilename().InStr( TEXT("RefShaderCache") )		!= INDEX_NONE )
		{
			continue;
		}

		// don't die out when we have a few bad packages, just keep on going so we get most of the data
		try
		{
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( Package != NULL )
			{
				FUNCTOR TheFunctor;
				TheFunctor.DoIt<OBJECTYPE>( Package );
			}
			else
			{
				warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			}
		}
		catch ( ... )
		{
			warnf( NAME_Log, TEXT("Exception %s"), *Filename.GetBaseFilename() );
		}



		if( (++GCIndex % 10) == 0 )
		{
			UObject::CollectGarbage(RF_Native);
		}
	}
}



/*-----------------------------------------------------------------------------
FindMissingPhysicalMaterials commandlet.
-----------------------------------------------------------------------------*/

INT UFindTexturesWithMissingPhysicalMaterialsCommandlet::Main( const FString& Params )
{
	DoActionToAllPackages<UMaterial, MaterialMissingPhysMaterialFunctor>(Params);

	return 0;
}
IMPLEMENT_CLASS(UFindTexturesWithMissingPhysicalMaterialsCommandlet)


/*-----------------------------------------------------------------------------
 FindSpecTexturesWithoutLODBiasOfTwo Commandlet
-----------------------------------------------------------------------------*/

INT UFindSpecTexturesWithoutLODBiasOfTwoCommandlet::Main( const FString& Params )
{
	DoActionToAllPackages<UTexture2D, LODBiasFunctor>(Params);

	return 0;
}
IMPLEMENT_CLASS(UFindSpecTexturesWithoutLODBiasOfTwoCommandlet)


/*-----------------------------------------------------------------------------
 FindSoundCuesWithMissingGroups commandlet.
-----------------------------------------------------------------------------*/
INT UFindSoundCuesWithMissingGroupsCommandlet::Main( const FString& Params )
{
	DoActionToAllPackages<USoundCue, SoundCueMissingGroupsFunctor>(Params);

	return 0;
}
IMPLEMENT_CLASS(UFindSoundCuesWithMissingGroupsCommandlet)




/*-----------------------------------------------------------------------------
	UListScriptReferencedContentCommandlet.
-----------------------------------------------------------------------------*/
/**
 * Processes a value found by ListReferencedContent(), possibly recursing for inline objects
 *
 * @param	Value			the object to be processed
 * @param	Property		the property where Value was found (for a dynamic array, this is the Inner property)
 * @param	PropertyDesc	string printed as the property Value was assigned to (usually *Property->GetName(), except for dynamic arrays, where it's the array name and index)
 * @param	Tab				string with a number of tabs for the current tab level of the output
 */
void UListScriptReferencedContentCommandlet::ProcessObjectValue(UObject* Value, UProperty* Property, const FString& PropertyDesc, const FString& Tab)
{
	if (Value != NULL)
	{
		// if it's an inline object, recurse over its properties
		if ((Property->PropertyFlags & CPF_NeedCtorLink) || (Property->PropertyFlags & CPF_Component))
		{
			ListReferencedContent(Value->GetClass(), (BYTE*)Value, FString(*Value->GetName()), Tab + TEXT("\t"));
		}
		else
		{
			// otherwise, print it as content that's being referenced
			warnf(TEXT("%s\t%s=%s'%s'"), *Tab, *PropertyDesc, *Value->GetClass()->GetName(), *Value->GetPathName());
		}
	}
}

/**
 * Lists content referenced by the given data
 *
 * @param	Struct		the type of the Default data
 * @param	Default		the data to look for referenced objects in
 * @param	HeaderName	string printed before any content references found (only if the data might contain content references)
 * @param	Tab			string with a number of tabs for the current tab level of the output
 */
void UListScriptReferencedContentCommandlet::ListReferencedContent(UStruct* Struct, BYTE* Default, const FString& HeaderName, const FString& Tab)
{
	UBOOL bPrintedHeader = FALSE;

	// iterate over all its properties
	for (UProperty* Property = Struct->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
	{
		if ( !bPrintedHeader &&
			 (Property->IsA(UObjectProperty::StaticClass()) || Property->IsA(UStructProperty::StaticClass()) || Property->IsA(UArrayProperty::StaticClass())) &&
			 Property->ContainsObjectReference() )
		{
			// this class may contain content references, so print header with class/struct name
			warnf(TEXT("%s%s"), *Tab, *HeaderName);
			bPrintedHeader = TRUE;
		}
		// skip class properties and object properties of class Object
		UObjectProperty* ObjectProp = Cast<UObjectProperty>(Property);
		if (ObjectProp != NULL && ObjectProp->PropertyClass != UObject::StaticClass() && ObjectProp->PropertyClass != UClass::StaticClass())
		{
			if (ObjectProp->ArrayDim > 1)
			{
				for (INT i = 0; i < ObjectProp->ArrayDim; i++)
				{
					ProcessObjectValue(*(UObject**)(Default + Property->Offset + i * Property->ElementSize), Property, FString::Printf(TEXT("%s[%d]"), *Property->GetName(), i), Tab);
				}
			}
			else
			{
				ProcessObjectValue(*(UObject**)(Default + Property->Offset), Property, FString(*Property->GetName()), Tab);
			}
		}
		else if (Property->IsA(UStructProperty::StaticClass()))
		{
			if (Property->ArrayDim > 1)
			{
				for (INT i = 0; i < Property->ArrayDim; i++)
				{
					ListReferencedContent(((UStructProperty*)Property)->Struct, (Default + Property->Offset + i * Property->ElementSize), FString::Printf(TEXT("%s[%d]"), *Property->GetName(), i), Tab + TEXT("\t"));
				}
			}
			else
			{
				ListReferencedContent(((UStructProperty*)Property)->Struct, (Default + Property->Offset), FString(*Property->GetName()), Tab + TEXT("\t"));
			}
		}
		else if (Property->IsA(UArrayProperty::StaticClass()))
		{
			UArrayProperty* ArrayProp = (UArrayProperty*)Property;
			FArray* Array = (FArray*)(Default + Property->Offset);
			UObjectProperty* ObjectProp = Cast<UObjectProperty>(ArrayProp->Inner);
			if (ObjectProp != NULL && ObjectProp->PropertyClass != UObject::StaticClass() && ObjectProp->PropertyClass != UClass::StaticClass())
			{
				for (INT i = 0; i < Array->Num(); i++)
				{
					ProcessObjectValue(*(UObject**)((BYTE*)Array->GetData() + (i * ArrayProp->Inner->ElementSize)), ObjectProp, FString::Printf(TEXT("%s[%d]"), *ArrayProp->GetName(), i), Tab);
				}
			}
			else if (ArrayProp->Inner->IsA(UStructProperty::StaticClass()))
			{
				UStruct* InnerStruct = ((UStructProperty*)ArrayProp->Inner)->Struct;
				INT PropertiesSize = InnerStruct->GetPropertiesSize();
				for (INT i = 0; i < Array->Num(); i++)
				{
					ListReferencedContent(InnerStruct, (BYTE*)Array->GetData() + (i * ArrayProp->Inner->ElementSize), FString(*Property->GetName()), Tab + TEXT("\t"));
				}
			}
		}
	}
}

/** lists all content referenced in the default properties of script classes */
INT UListScriptReferencedContentCommandlet::Main(const FString& Params)
{
	warnf(TEXT("Loading EditPackages..."));
	// load all the packages in the EditPackages list
	BeginLoad();
	for (INT i = 0; i < GEditor->EditPackages.Num(); i++)
	{
		LoadPackage(NULL, *GEditor->EditPackages(i), 0);
	}
	EndLoad();

	// iterate over all classes
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (!(It->ClassFlags & CLASS_Intrinsic))
		{
			ListReferencedContent(*It, (BYTE*)It->GetDefaultObject(), FString::Printf(TEXT("%s %s"), *It->GetFullName(), It->HasAnyFlags(RF_Native) ? TEXT("(native)") : TEXT("")));
		}
	}

	return 0;
}
IMPLEMENT_CLASS(UListScriptReferencedContentCommandlet);


/*-----------------------------------------------------------------------------
	UAnalyzeContent commandlet.
-----------------------------------------------------------------------------*/

/**
 * Helper structure to hold level/ usage count information.
 */
struct FLevelResourceStat
{
	/** Level package.						*/
	UObject*	LevelPackage;
	/** Usage count in above level package.	*/
	INT			Count;

	/**
	 * Constructor, initializing Count to 1 and setting package.
	 *
	 * @param InLevelPackage	Level package to use
	 */
	FLevelResourceStat( UObject* InLevelPackage )
	: LevelPackage( InLevelPackage )
	, Count( 1 )
	{}
};
/**
 * Helper structure containing usage information for a single resource and multiple levels.
 */
struct FResourceStat
{
	/** Resource object.											*/
	UObject*					Resource;
	/** Total number this resource is used across all levels.		*/
	INT							TotalCount;
	/** Array of detailed per level resource usage breakdown.		*/
	TArray<FLevelResourceStat>	LevelResourceStats;

	/**
	 * Constructor
	 *
	 * @param	InResource		Resource to use
	 * @param	LevelPackage	Level package to use
	 */
	FResourceStat( UObject* InResource, UObject* LevelPackage )
	:	Resource( InResource )
	,	TotalCount( 1 )
	{
		// Create initial stat entry.
		LevelResourceStats.AddItem( FLevelResourceStat( LevelPackage ) );
	}

	/**
	 * Increment usage count by one
	 *
	 * @param	LevelPackage	Level package using the resource.
	 */
	void IncrementUsage( UObject* LevelPackage )
	{
		// Iterate over all level resource stats to find existing entry.
		UBOOL bFoundExisting = FALSE;
		for( INT LevelIndex=0; LevelIndex<LevelResourceStats.Num(); LevelIndex++ )
		{
			FLevelResourceStat& LevelResourceStat = LevelResourceStats(LevelIndex);
			// We found a match.
			if( LevelResourceStat.LevelPackage == LevelPackage )
			{
				// Increase its count and exit loop.
				LevelResourceStat.Count++;
				bFoundExisting = TRUE;
				break;
			}
		}
		// No existing entry has been found, add new one.
		if( !bFoundExisting )
		{
			LevelResourceStats.AddItem( FLevelResourceStat( LevelPackage ) );
		}
		// Increase total count.
		TotalCount++;
	}
};
/** Compare function used by sort. Sorts in descending order. */
IMPLEMENT_COMPARE_CONSTREF( FResourceStat, UnPackageUtilities, { return B.TotalCount - A.TotalCount; } );

/**
 * Class encapsulating stats functionality.
 */
class FResourceStatContainer
{
public:
	/**
	 * Constructor
	 *
	 * @param	InDescription	Description used for dumping stats.
	 */
	FResourceStatContainer( const TCHAR* InDescription )
	:	Description( InDescription )
	{}

	/** 
	 * Function called when a resource is encountered in a level to increment count.
	 *
	 * @param	Resource		Encountered resource.
	 * @param	LevelPackage	Level package resource was encountered in.
	 */
	void EncounteredResource( UObject* Resource, UObject* LevelPackage )
	{
		FResourceStat* ResourceStatPtr = ResourceToStatMap.Find( Resource );
		// Resource has existing stat associated.
		if( ResourceStatPtr != NULL )
		{
			ResourceStatPtr->IncrementUsage( LevelPackage );
		}
		// Associate resource with new stat.
		else
		{
			FResourceStat ResourceStat( Resource, LevelPackage );
			ResourceToStatMap.Set( Resource, ResourceStat );
		}
	}

	/**
	 * Dumps all the stats information sorted to the log.
	 */
	void DumpStats()
	{
		// Copy TMap data into TArray so it can be sorted.
		TArray<FResourceStat> SortedList;
		for( TMap<UObject*,FResourceStat>::TIterator It(ResourceToStatMap); It; ++It )
		{
			SortedList.AddItem( It.Value() );
		}
		// Sort the list in descending order by total count.
		Sort<USE_COMPARE_CONSTREF(FResourceStat,UnPackageUtilities)>( SortedList.GetTypedData(), SortedList.Num() );

		warnf( NAME_Log, TEXT("") ); 
		warnf( NAME_Log, TEXT("") ); 
		warnf( NAME_Log, TEXT("Stats for %s."), *Description ); 
		warnf( NAME_Log, TEXT("") ); 

		// Iterate over all entries and dump info.
		for( INT i=0; i<SortedList.Num(); i++ )
		{
			const FResourceStat& ResourceStat = SortedList(i);
			warnf( NAME_Log, TEXT("%4i use%s%4i level%s for%s   %s"), 
				ResourceStat.TotalCount,
				ResourceStat.TotalCount > 1 ? TEXT("s in") : TEXT(" in "), 
				ResourceStat.LevelResourceStats.Num(), 
				ResourceStat.LevelResourceStats.Num() > 1 ? TEXT("s") : TEXT(""),
				ResourceStat.LevelResourceStats.Num() > 1 ? TEXT("") : TEXT(" "),
				*ResourceStat.Resource->GetFullName() );

			for( INT LevelIndex=0; LevelIndex<ResourceStat.LevelResourceStats.Num(); LevelIndex++ )
			{
				const FLevelResourceStat& LevelResourceStat = ResourceStat.LevelResourceStats(LevelIndex);
				warnf( NAME_Log, TEXT("    %4i use%s: %s"), 
					LevelResourceStat.Count, 
					LevelResourceStat.Count > 1 ? TEXT("s in") : TEXT("  in"), 
					*LevelResourceStat.LevelPackage->GetName() );
			}
		}
	}
private:
	/** Map from resource to stat helper structure. */
	TMap<UObject*,FResourceStat>	ResourceToStatMap;
	/** Description used for dumping stats.			*/
	FString							Description;
};

void UAnalyzeContentCommandlet::StaticInitialize()
{
	ShowErrorCount = FALSE;
}

INT UAnalyzeContentCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Retrieve all package file names and iterate over them, comparing them to tokens.
	TArray<FString> PackageFileList = GPackageFileCache->GetPackageFileList();		
	for( INT PackageIndex=0; PackageIndex<PackageFileList.Num(); PackageIndex++ )
	{
		// Tokens on the command line are package names.
		for( INT TokenIndex=0; TokenIndex<Tokens.Num(); TokenIndex++ )
		{
			// Compare the two and see whether we want to include this package in the analysis.
			FFilename PackageName = PackageFileList( PackageIndex );
			if( Tokens(TokenIndex) == PackageName.GetBaseFilename() )
			{
				UPackage* Package = UObject::LoadPackage( NULL, *PackageName, LOAD_None );
				if( Package != NULL )
				{
					warnf( NAME_Log, TEXT("Loading %s"), *PackageName );
					// Find the world and load all referenced levels.
					UWorld* World = FindObjectChecked<UWorld>( Package, TEXT("TheWorld") );
					if( World )
					{
						AWorldInfo* WorldInfo	= World->GetWorldInfo();
						// Iterate over streaming level objects loading the levels.
						for( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
						{
							ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
							if( StreamingLevel )
							{
								// Load package if found.
								FString Filename;
								if( GPackageFileCache->FindPackageFile( *StreamingLevel->PackageName.ToString(), NULL, Filename ) )
								{
									warnf(NAME_Log, TEXT("Loading sub-level %s"), *Filename);
									LoadPackage( NULL, *Filename, LOAD_None );
								}
							}
						}
					}
				}
				else
				{
					warnf( NAME_Error, TEXT("Error loading %s!"), *PackageName );
				}
			}
		}
	}

	// By now all objects are in memory.

	FResourceStatContainer StaticMeshMaterialStats( TEXT("materials applied to static meshes") );
	FResourceStatContainer StaticMeshStats( TEXT("static meshes placed in levels") );
	FResourceStatContainer BSPMaterialStats( TEXT("materials applied to BSP surfaces") );

	// Iterate over all static mesh components and add their materials and static meshes.
	for( TObjectIterator<UStaticMeshComponent> It; It; ++It )
	{
		UStaticMeshComponent*	StaticMeshComponent = *It;
		UPackage*				LevelPackage		= StaticMeshComponent->GetOutermost();
		
		// Only add if the outer is a map package.
		if( LevelPackage->ContainsMap() )
		{
			if( StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() )
			{
				// Populate materials array, avoiding duplicate entries.
				TArray<UMaterial*> Materials;
				INT MaterialCount = StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num();
				for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
				{
					UMaterialInstance* MaterialInstance = StaticMeshComponent->GetMaterial( MaterialIndex );
					if( MaterialInstance && MaterialInstance->GetMaterial() )
					{
						Materials.AddUniqueItem( MaterialInstance->GetMaterial() );
					}
				}

				// Iterate over materials and create/ update associated stats.
				for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
				{
					UMaterial* Material = Materials(MaterialIndex);
					// Track materials applied to static meshes.			
					StaticMeshMaterialStats.EncounteredResource( Material, LevelPackage );
				}
			}

			// Track static meshes used by static mesh components.
			if( StaticMeshComponent->StaticMesh )
			{
				StaticMeshStats.EncounteredResource( StaticMeshComponent->StaticMesh, LevelPackage );
			}
		}
	}

	for( TObjectIterator<ABrush> It; It; ++It )
	{
		ABrush*		BrushActor		= *It;
		UPackage*	LevelPackage	= BrushActor->GetOutermost();
		
		// Only add if the outer is a map package.
		if( LevelPackage->ContainsMap() )
		{
			if( BrushActor->Brush && BrushActor->Brush->Polys )
			{
				UPolys* Polys = BrushActor->Brush->Polys;

				// Populate materials array, avoiding duplicate entries.
				TArray<UMaterial*> Materials;
				for( INT ElementIndex=0; ElementIndex<Polys->Element.Num(); ElementIndex++ )
				{
					const FPoly& Poly = Polys->Element(ElementIndex);
					if( Poly.Material && Poly.Material->GetMaterial() )
					{
						Materials.AddUniqueItem( Poly.Material->GetMaterial() );
					}
				}

				// Iterate over materials and create/ update associated stats.
				for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
				{
					UMaterial* Material = Materials(MaterialIndex);
					// Track materials applied to BSP.
					BSPMaterialStats.EncounteredResource( Material, LevelPackage );
				}
			}
		}
	}

	// Dump stat summaries.
	StaticMeshMaterialStats.DumpStats();
	StaticMeshStats.DumpStats();
	BSPMaterialStats.DumpStats();

	return 0;
}
IMPLEMENT_CLASS(UAnalyzeContentCommandlet)


/*-----------------------------------------------------------------------------
	UAnalyzeReferencedContentCommandlet
-----------------------------------------------------------------------------*/

/** Constructor, initializing all members. */
UAnalyzeReferencedContentCommandlet::FStaticMeshStats::FStaticMeshStats( UStaticMesh* StaticMesh )
:	ResourceType(StaticMesh->GetClass()->GetName())
,	ResourceName(StaticMesh->GetPathName())
,	NumInstances(0)
,	NumTriangles(0)
,	NumSections(0)
,	bIsReferencedByScript(FALSE)
,	NumMapsUsedIn(0)
,	ResourceSize(StaticMesh->GetResourceSize())
{
	// Update triangle and section counts.
	for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
	{
		const FStaticMeshElement& StaticMeshElement = StaticMesh->LODModels(0).Elements(ElementIndex);
		NumTriangles += StaticMeshElement.NumTriangles;
		NumSections++;
	}

	NumPrimitives = 0;
	if(StaticMesh->BodySetup)
	{
		NumPrimitives = StaticMesh->BodySetup->AggGeom.ConvexElems.Num();
	}
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FStaticMeshStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i%s"),
								*ResourceType,
								*ResourceName,
								NumInstances,
								NumTriangles,
								NumSections,
								NumPrimitives,
								NumMapsUsedIn,
								ResourceSize,
								LINE_TERMINATOR);
}

/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FStaticMeshStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumInstances,NumTriangles,NumSections,CollisionPrims,NumMapsUsedIn,ResourceSize") LINE_TERMINATOR;
}


/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FTextureStats::FTextureStats( UTexture* Texture )
:	ResourceType(Texture->GetClass()->GetName())
,	ResourceName(Texture->GetPathName())
,	bIsReferencedByScript(FALSE)
,	NumMapsUsedIn(0)
,	ResourceSize(Texture->GetResourceSize())
,	LODBias(Texture->LODBias)
,	LODGroup(Texture->LODGroup)
,	Format(PF_Unknown)
{
	// Update format.
	UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
	if( Texture2D )
	{
		Format = Texture2D->Format;
	}
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FTextureStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i%s"),
								*ResourceType,
								*ResourceName,						
								MaterialsUsedBy.Num(),
								bIsReferencedByScript,
								NumMapsUsedIn,
								ResourceSize,
								LODBias,
								LODGroup,
								Format,
								LINE_TERMINATOR);
}

/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FTextureStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumMaterialsUsedBy,ScriptReferenced,NumMapsUsedIn,ResourceSize,LODBias,LODGroup,Format") LINE_TERMINATOR;
}

/**
 * Static helper to return instructions used by shader type.
 *
 * @param	MeshShaderMap	Shader map to use to find shader of passed in type
 * @param	ShaderType		Type of shader to query instruction count for
 * @return	Instruction count if found, 0 otherwise
 */
static INT GetNumInstructionsForShaderType( const FMeshMaterialShaderMap* MeshShaderMap, FShaderType* ShaderType )
{
	INT NumInstructions = 0;
	const FShader* Shader = MeshShaderMap->GetShader(ShaderType);
	if( Shader )
	{
		NumInstructions = Shader->GetNumInstructions();
	}
	return NumInstructions;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FMaterialStats::FMaterialStats( UMaterial* Material, UAnalyzeReferencedContentCommandlet* Commandlet )
:	ResourceType(Material->GetClass()->GetName())
,	ResourceName(Material->GetPathName())
,	NumBrushesAppliedTo(0)
,	NumStaticMeshInstancesAppliedTo(0)
,	bIsReferencedByScript(FALSE)
,	NumMapsUsedIn(0)
,	ResourceSizeOfReferencedTextures(0)
{
	// Keep track of unique textures and texture sample count.
	TArray<UTexture*> UniqueTextures;
	TArray<UTexture*> SampledTextures;
	Material->GetTextures( UniqueTextures );
	Material->GetTextures( SampledTextures, FALSE );
	
	// Update texture samplers count.
	NumTextureSamples = SampledTextures.Num();

	// Update dependency chain stats.
	check( Material->MaterialResource);
	MaxTextureDependencyLength = Material->MaterialResource->GetMaxTextureDependencyLength();

	// Update instruction counts.
	const FMaterialShaderMap* MaterialShaderMap = Material->MaterialResource->GetShaderMap();
	if(MaterialShaderMap)
	{
		// Use the local vertex factory shaders.
		const FMeshMaterialShaderMap* MeshShaderMap = MaterialShaderMap->GetMeshShaderMap(&FLocalVertexFactory::StaticType);
		check(MeshShaderMap);

		// Get intrustion counts.
		NumInstructionsTranslucent				= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeTranslucent );
		NumInstructionsAdditive					= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeAdditive );
		NumInstructionsModulate					= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeModulate );
		NumInstructionsEmissiveNoLightmap		= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeEmissiveNoLightmap	);
		NumInstructionsEmissiveAndLightmap		= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypeEmissiveAndLightmap );
		NumInstructionsPointLightWithShadowMap	= GetNumInstructionsForShaderType( MeshShaderMap, Commandlet->ShaderTypePointLightWithShadowMap );
	}

	// Iterate over unique texture refs and update resource size.
	for( INT TextureIndex=0; TextureIndex<UniqueTextures.Num(); TextureIndex++ )
	{
		UTexture* Texture = UniqueTextures(TextureIndex);
		ResourceSizeOfReferencedTextures += Texture->GetResourceSize();
		TexturesUsed.AddItem( Texture->GetFullName() );
	}
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FMaterialStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i%s"),
								*ResourceType,
								*ResourceName,
								NumBrushesAppliedTo,
								NumStaticMeshInstancesAppliedTo,
								StaticMeshesAppliedTo.Num(),
								bIsReferencedByScript,
								NumMapsUsedIn,
								TexturesUsed.Num(),
								NumTextureSamples,
								MaxTextureDependencyLength,
								Max3( NumInstructionsTranslucent, NumInstructionsAdditive, NumInstructionsModulate ),
								NumInstructionsEmissiveNoLightmap,
								NumInstructionsEmissiveAndLightmap,
								NumInstructionsPointLightWithShadowMap,
								ResourceSizeOfReferencedTextures,
								LINE_TERMINATOR);
}

/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FMaterialStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,NumBrushesAppliedTo,NumStaticMeshInstancesAppliedTo,NumStaticMeshesAppliedTo,ScriptReferenced,NumMapsUsedIn,NumTextures,NumTextureSamples,MaxTextureDependencyLength,Translucent,Emissive,EmissiveLightmap,PointLightShadowMap,ResourceSizeOfReferencedTextures") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FParticleStats::FParticleStats( UParticleSystem* ParticleSystem )
:	ResourceType(ParticleSystem->GetClass()->GetName())
,	ResourceName(ParticleSystem->GetPathName())
,	bIsReferencedByScript(FALSE)
,	NumMapsUsedIn(0)
,	NumEmitters(0)
,	NumModules(0)
,	NumPeakActiveParticles(0)
{
	// Iterate over all sub- emitters and update stats.
	for( INT EmitterIndex=0; EmitterIndex<ParticleSystem->Emitters.Num(); EmitterIndex++ )
	{
		UParticleEmitter* ParticleEmitter = ParticleSystem->Emitters(EmitterIndex);
		if( ParticleEmitter )
		{
			NumEmitters++;
			NumModules				+= ParticleEmitter->Modules.Num();
			// Get peak active particles from LOD 0.
			UParticleLODLevel* LODLevel = ParticleEmitter->GetLODLevel(0);
			if( LODLevel )
			{	
				NumPeakActiveParticles += LODLevel->PeakActiveParticles;
			}
		}
	}
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FParticleStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%i,%i,%i%s"),
								*ResourceType,
								*ResourceName,	
								bIsReferencedByScript,
								NumMapsUsedIn,
								NumEmitters,
								NumModules,
								NumPeakActiveParticles,
								LINE_TERMINATOR);
}




/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FParticleStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,NumEmitters,NumModules,NumPeakActiveParticles") LINE_TERMINATOR;
}

/** Constructor, initializing all members */
UAnalyzeReferencedContentCommandlet::FAnimSequenceStats::FAnimSequenceStats( UAnimSequence* Sequence )
:	ResourceType(Sequence->GetClass()->GetName())
,	ResourceName(Sequence->GetPathName())
,	bIsReferencedByScript(FALSE)
,	NumMapsUsedIn(0)
,	CompressionScheme(FString(TEXT("")))
,	TranslationFormat(ACF_None)
,	RotationFormat(ACF_None)
,	AnimationSize(0)
{
	// The sequence object name is not very useful - strip and add the friendly name.
	FString Left, Right;
	ResourceName.Split(TEXT("."), &Left, &Right, TRUE);
	ResourceName = Left + TEXT(".") + Sequence->SequenceName.ToString();

	if(Sequence->CompressionScheme)
	{
		CompressionScheme = Sequence->CompressionScheme->GetClass()->GetName();
	}

	TranslationFormat = static_cast<AnimationCompressionFormat>(Sequence->TranslationCompressionFormat);
	RotationFormat = static_cast<AnimationCompressionFormat>(Sequence->RotationCompressionFormat);
	AnimationSize = Sequence->GetResourceSize();
}


static FString GetCompressionFormatString(AnimationCompressionFormat InFormat)
{
	switch(InFormat)
	{
	case ACF_None:
		return FString(TEXT("ACF_None"));
	case ACF_Float96NoW:
		return FString(TEXT("ACF_Float96NoW"));
	case ACF_Fixed48NoW:
		return FString(TEXT("ACF_Fixed48NoW"));
	case ACF_IntervalFixed32NoW:
		return FString(TEXT("ACF_IntervalFixed32NoW"));
	case ACF_Fixed32NoW:
		return FString(TEXT("ACF_Fixed32NoW"));
	case ACF_Float32NoW:
		return FString(TEXT("ACF_Float32NoW"));
	}

	return FString(TEXT("Unknown"));
}

/**
 * Stringifies gathered stats in CSV format.
 *
 * @return comma separated list of stats
 */
FString UAnalyzeReferencedContentCommandlet::FAnimSequenceStats::ToCSV() const
{
	return FString::Printf(TEXT("%s,%s,%i,%i,%s,%s,%s,%i%s"),
		*ResourceType,
		*ResourceName,	
		bIsReferencedByScript,
		NumMapsUsedIn,
		*GetCompressionFormatString(TranslationFormat),
		*GetCompressionFormatString(RotationFormat),
		*CompressionScheme,
		AnimationSize,
		LINE_TERMINATOR);
}




/**
 * Returns a header row for CSV
 *
 * @return comma separated header row
 */
FString UAnalyzeReferencedContentCommandlet::FAnimSequenceStats::GetCSVHeaderRow()
{
	return TEXT("ResourceType,ResourceName,ScriptReferenced,NumMapsUsedIn,TransFormat,RotFormat,CompressionScheme,AnimSize") LINE_TERMINATOR;
}

/**
 * Retrieves/ creates material stats associated with passed in material.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	Material	Material to retrieve/ create material stats for
 * @return	pointer to material stats associated with material
 */
UAnalyzeReferencedContentCommandlet::FMaterialStats* UAnalyzeReferencedContentCommandlet::GetMaterialStats( UMaterial* Material )
{
	UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = ResourceNameToMaterialStats.Find( *Material->GetFullName() );
	if( MaterialStats == NULL )
	{
		MaterialStats =	&ResourceNameToMaterialStats.Set( *Material->GetFullName(), UAnalyzeReferencedContentCommandlet::FMaterialStats( Material, this ) );
	}
	return MaterialStats;
}

/**
 * Retrieves/ creates texture stats associated with passed in texture.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	Texture		Texture to retrieve/ create texture stats for
 * @return	pointer to texture stats associated with texture
 */
UAnalyzeReferencedContentCommandlet::FTextureStats* UAnalyzeReferencedContentCommandlet::GetTextureStats( UTexture* Texture )
{
	UAnalyzeReferencedContentCommandlet::FTextureStats* TextureStats = ResourceNameToTextureStats.Find( *Texture->GetFullName() );
	if( TextureStats == NULL )
	{
		TextureStats = &ResourceNameToTextureStats.Set( *Texture->GetFullName(), UAnalyzeReferencedContentCommandlet::FTextureStats( Texture ) );
	}
	return TextureStats;
}

/**
 * Retrieves/ creates static mesh stats associated with passed in static mesh.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	StaticMesh	Static mesh to retrieve/ create static mesh stats for
 * @return	pointer to static mesh stats associated with static mesh
 */
UAnalyzeReferencedContentCommandlet::FStaticMeshStats* UAnalyzeReferencedContentCommandlet::GetStaticMeshStats( UStaticMesh* StaticMesh )
{
	UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = ResourceNameToStaticMeshStats.Find( *StaticMesh->GetFullName() );
	if( StaticMeshStats == NULL )
	{
		StaticMeshStats = &ResourceNameToStaticMeshStats.Set( *StaticMesh->GetFullName(), UAnalyzeReferencedContentCommandlet::FStaticMeshStats( StaticMesh ) );
	}
	return StaticMeshStats;
}

/**
 * Retrieves/ creates particle stats associated with passed in particle system.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	ParticleSystem	Particle system to retrieve/ create static mesh stats for
 * @return	pointer to particle system stats associated with static mesh
 */
UAnalyzeReferencedContentCommandlet::FParticleStats* UAnalyzeReferencedContentCommandlet::GetParticleStats( UParticleSystem* ParticleSystem )
{
	UAnalyzeReferencedContentCommandlet::FParticleStats* ParticleStats = ResourceNameToParticleStats.Find( *ParticleSystem->GetFullName() );
	if( ParticleStats == NULL )
	{
		ParticleStats = &ResourceNameToParticleStats.Set( *ParticleSystem->GetFullName(), UAnalyzeReferencedContentCommandlet::FParticleStats( ParticleSystem ) );
	}
	return ParticleStats;
}

/**
 * Retrieves/ creates animation sequence stats associated with passed in animation sequence.
 *
 * @warning: returns pointer into TMap, only valid till next time Set is called
 *
 * @param	AnimSequence	Anim sequence to retrieve/ create anim sequence stats for
 * @return	pointer to particle system stats associated with anim sequence
 */
UAnalyzeReferencedContentCommandlet::FAnimSequenceStats* UAnalyzeReferencedContentCommandlet::GetAnimSequenceStats( UAnimSequence* AnimSequence )
{
	UAnalyzeReferencedContentCommandlet::FAnimSequenceStats* AnimStats = ResourceNameToAnimStats.Find( *AnimSequence->GetFullName() );
	if( AnimStats == NULL )
	{
		AnimStats = &ResourceNameToAnimStats.Set( *AnimSequence->GetFullName(), UAnalyzeReferencedContentCommandlet::FAnimSequenceStats( AnimSequence ) );
	}
	return AnimStats;
}

void UAnalyzeReferencedContentCommandlet::StaticInitialize()
{
	ShowErrorCount = FALSE;
}

/**
 * Handles encountered object, routing to various sub handlers.
 *
 * @param	Object			Object to handle
 * @param	LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param	bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleObject( UObject* Object, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	// Disregard marked objects as they won't go away with GC.
	if( !Object->HasAnyFlags( RF_Marked ) )
	{
		// Whether the object is the passed in level package if it is != NULL.
		UBOOL bIsInALevelPackage = LevelPackage && Object->IsIn( LevelPackage );

		// Handle static mesh.
		if( Object->IsA(UStaticMesh::StaticClass()) )
		{
			HandleStaticMesh( (UStaticMesh*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles static mesh component if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(UStaticMeshComponent::StaticClass()) && bIsInALevelPackage )
		{
			HandleStaticMeshComponent( (UStaticMeshComponent*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle material.
		else if( Object->IsA(UMaterial::StaticClass()) )
		{
			HandleMaterial( (UMaterial*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle texture.
		else if( Object->IsA(UTexture::StaticClass()) )
		{
			HandleTexture( (UTexture*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handles brush actor if it's residing in the map package. LevelPackage == NULL for non map packages.
		else if( Object->IsA(ABrush::StaticClass()) && bIsInALevelPackage )
		{
			HandleBrush( (ABrush*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle particle system.
		else if( Object->IsA(UParticleSystem::StaticClass()) )
		{
			HandleParticleSystem( (UParticleSystem*) Object, LevelPackage, bIsScriptReferenced );
		}
		// Handle anim sequence.
		else if( Object->IsA(UAnimSequence::StaticClass()) )
		{
			HandleAnimSequence( (UAnimSequence*) Object, LevelPackage, bIsScriptReferenced );
		}
	}
}

/**
 * Handles gathering stats for passed in static mesh.
 *
 * @param StaticMesh	StaticMesh to gather stats for.
 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleStaticMesh( UStaticMesh* StaticMesh, UPackage* LevelPackage, UBOOL bIsScriptReferenced  )
{
	UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( StaticMesh );
	
	if( LevelPackage )
	{
		StaticMeshStats->NumMapsUsedIn++;
	}

	if( bIsScriptReferenced )
	{
		StaticMeshStats->bIsReferencedByScript = TRUE;
	}

	// Populate materials array, avoiding duplicate entries.
	TArray<UMaterial*> Materials;
	INT MaterialCount = StaticMesh->LODModels(0).Elements.Num();
	for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
	{
		UMaterialInstance* MaterialInstance = StaticMesh->LODModels(0).Elements(MaterialIndex).Material;
		if( MaterialInstance && MaterialInstance->GetMaterial() )
		{
			Materials.AddUniqueItem( MaterialInstance->GetMaterial() );
		}
	}

	// Iterate over materials and create/ update associated stats.
	for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
	{
		UMaterial* Material	= Materials(MaterialIndex);	
		UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );
		MaterialStats->StaticMeshesAppliedTo.Set( *StaticMesh->GetFullName(), TRUE );
	}
}

/**
 * Handles gathering stats for passed in static mesh component.
 *
 * @param StaticMeshComponent	StaticMeshComponent to gather stats for
 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleStaticMeshComponent( UStaticMeshComponent* StaticMeshComponent, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( StaticMeshComponent->StaticMesh && StaticMeshComponent->StaticMesh->LODModels.Num() )
	{
		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		INT MaterialCount = StaticMeshComponent->StaticMesh->LODModels(0).Elements.Num();
		for( INT MaterialIndex=0; MaterialIndex<MaterialCount; MaterialIndex++ )
		{
			UMaterialInstance* MaterialInstance = StaticMeshComponent->GetMaterial( MaterialIndex );
			if( MaterialInstance && MaterialInstance->GetMaterial() )
			{
				Materials.AddUniqueItem( MaterialInstance->GetMaterial() );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
		{
			UMaterial* Material	= Materials(MaterialIndex);	
			UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );
			MaterialStats->NumStaticMeshInstancesAppliedTo++;
		}
		
		// Track static meshes used by static mesh components.
		if( StaticMeshComponent->StaticMesh )
		{
			UAnalyzeReferencedContentCommandlet::FStaticMeshStats* StaticMeshStats = GetStaticMeshStats( StaticMeshComponent->StaticMesh );
			StaticMeshStats->NumInstances++;
		}
	}
}

/**
 * Handles gathering stats for passed in material.
 *
 * @param Material	Material to gather stats for
 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleMaterial( UMaterial* Material, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );	
	
	if( LevelPackage )
	{
		MaterialStats->NumMapsUsedIn++;
	}

	if( bIsScriptReferenced )
	{
		MaterialStats->bIsReferencedByScript = TRUE;
	}

	// Array of textures used by this material. No duplicates.
	TArray<UTexture*> TexturesUsed;
	Material->GetTextures(TexturesUsed);

	// Update textures used by this material.
	for( INT TextureIndex=0; TextureIndex<TexturesUsed.Num(); TextureIndex++ )
	{
		UTexture* Texture = TexturesUsed(TextureIndex);
		UAnalyzeReferencedContentCommandlet::FTextureStats* TextureStats = GetTextureStats(Texture);
		TextureStats->MaterialsUsedBy.Set( *Material->GetFullName(), TRUE );
	}
}

/**
 * Handles gathering stats for passed in texture.
 *
 * @paramTexture	Texture to gather stats for
 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleTexture( UTexture* Texture, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FTextureStats* TextureStats = GetTextureStats( Texture );
	
	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		TextureStats->NumMapsUsedIn++;
	}

	// Mark as being referenced by script.
	if( bIsScriptReferenced )
	{
		TextureStats->bIsReferencedByScript = TRUE;
	}
}

/**
 * Handles gathering stats for passed in brush.
 *
 * @param BrushActor Brush actor to gather stats for
 * @param LevelPackage	Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleBrush( ABrush* BrushActor, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	if( BrushActor->Brush && BrushActor->Brush->Polys )
	{
		UPolys* Polys = BrushActor->Brush->Polys;

		// Populate materials array, avoiding duplicate entries.
		TArray<UMaterial*> Materials;
		for( INT ElementIndex=0; ElementIndex<Polys->Element.Num(); ElementIndex++ )
		{
			const FPoly& Poly = Polys->Element(ElementIndex);
			if( Poly.Material && Poly.Material->GetMaterial() )
			{
				Materials.AddUniqueItem( Poly.Material->GetMaterial() );
			}
		}

		// Iterate over materials and create/ update associated stats.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num(); MaterialIndex++ )
		{
			UMaterial* Material = Materials(MaterialIndex);
			UAnalyzeReferencedContentCommandlet::FMaterialStats* MaterialStats = GetMaterialStats( Material );
			MaterialStats->NumBrushesAppliedTo++;
		}
	}
}

/**
 * Handles gathering stats for passed in particle system.
 *
 * @param ParticleSystem	Particle system to gather stats for
 * @param LevelPackage		Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleParticleSystem( UParticleSystem* ParticleSystem, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FParticleStats* ParticleStats = GetParticleStats( ParticleSystem );
	
	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		ParticleStats->NumMapsUsedIn++;
	}

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		ParticleStats->bIsReferencedByScript = TRUE;
	}
}

/**
 * Handles gathering stats for passed in animation sequence.
 *
 * @param AnimSequence		AnimSequence to gather stats for
 * @param LevelPackage		Currently loaded level package, can be NULL if not a level
 * @param bIsScriptReferenced Whether object is handled because there is a script reference
 */
void UAnalyzeReferencedContentCommandlet::HandleAnimSequence( UAnimSequence* AnimSequence, UPackage* LevelPackage, UBOOL bIsScriptReferenced )
{
	UAnalyzeReferencedContentCommandlet::FAnimSequenceStats* AnimStats = GetAnimSequenceStats( AnimSequence );

	// Only handle further if we have a level package.
	if( LevelPackage )
	{
		AnimStats->NumMapsUsedIn++;
	}

	// Mark object as being referenced by script.
	if( bIsScriptReferenced )
	{
		AnimStats->bIsReferencedByScript = TRUE;
	}
}

INT UAnalyzeReferencedContentCommandlet::Main( const FString& Params )
{
	// Parse command line.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	// Whether to only deal with map files.
	UBOOL bShouldOnlyLoadMaps	= Switches.FindItemIndex(TEXT("MAPSONLY")) != INDEX_NONE;
	// Whether to exclude script references.
	UBOOL bExcludeScript		= Switches.FindItemIndex(TEXT("EXCLUDESCRIPT")) != INDEX_NONE;

	// Load up all script files in EditPackages.
	UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
	for( INT i=0; i<EditorEngine->EditPackages.Num(); i++ )
	{
		LoadPackage( NULL, *EditorEngine->EditPackages(i), LOAD_NoWarn );
	}

	// Mark loaded objects as they are part of the always loaded set and are not taken into account for stats.
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		// Script referenced asset.
		if( !bExcludeScript )
		{
			HandleObject( Object, NULL, TRUE );
		}
		// Mark object as always loaded so it doesn't get counted multiple times.
		Object->SetFlags( RF_Marked );
	}

	// Build package file list.
	const TArray<FString> FilesInPath( GPackageFileCache->GetPackageFileList() );
	if( FilesInPath.Num() == 0 )
	{
		warnf( NAME_Warning, TEXT("No packages found") );
		return 1;
	}

	// Find shader types.
	ShaderTypeTranslucent				= FindShaderTypeByName(TEXT("TTranslucencyPixelShaderFBlendTranslucentPolicyFNoLightMapPolicy"));
	ShaderTypeAdditive					= FindShaderTypeByName(TEXT("TTranslucencyPixelShaderFBlendAdditivePolicyFNoLightMapPolicy"));
	ShaderTypeModulate					= FindShaderTypeByName(TEXT("TTranslucencyPixelShaderFBlendModulatePolicyFNoLightMapPolicy"));
	ShaderTypeEmissiveNoLightmap		= FindShaderTypeByName(TEXT("TEmissivePixelShader<FNoLightMapPolicy>"));
	ShaderTypeEmissiveAndLightmap		= FindShaderTypeByName(TEXT("TEmissivePixelShader<FVertexLightMapPolicy>"));
	ShaderTypePointLightWithShadowMap	= FindShaderTypeByName(TEXT("TLightPixelShaderFPointLightPolicyFShadowTexturePolicy"));

	check( ShaderTypeTranslucent );
	check( ShaderTypeAdditive );
	check( ShaderTypeModulate );
	check( ShaderTypeEmissiveNoLightmap	);
	check( ShaderTypeEmissiveAndLightmap );
	check( ShaderTypePointLightWithShadowMap );

	// Iterate over all files, loading up ones that have the map extension..
	for( INT FileIndex=0; FileIndex<FilesInPath.Num(); FileIndex++ )
	{
		const FFilename& Filename = FilesInPath(FileIndex);		

		// Disregard filenames that don't have the map extension if we're in MAPSONLY mode.
		if( bShouldOnlyLoadMaps && (Filename.GetExtension() != FURL::DefaultMapExt) )
		{
			continue;
		}
			
		// Skip filenames with the script extension. @todo: don't hardcode .u as the script file extension
		if( (Filename.GetExtension() == TEXT("u")) )
		{
			continue;
		}

		warnf( NAME_Log, TEXT("Loading %s"), *Filename );
		UPackage* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
		if( Package == NULL )
		{
			warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
		}
		else
		{
			// Figure out whether package is a map or content package.
			UBOOL bIsAMapPackage = FindObject<UWorld>(Package, TEXT("TheWorld")) != NULL;

			// Handle currently loaded objects.
			for( TObjectIterator<UObject> It; It; ++It )
			{
				UObject* Object = *It;
				HandleObject( Object, bIsAMapPackage ? Package : NULL, FALSE );
			}
		}

		// Collect garbage, going back to a clean slate.
		UObject::CollectGarbage( RF_Native );

		// Verify that everything we cared about got cleaned up correctly.
		UBOOL bEncounteredUnmarkedObject = FALSE;
		for( TObjectIterator<UObject> It; It; ++It )
		{
			UObject* Object = *It;
			if( !Object->HasAllFlags( RF_Marked ) && !Object->IsIn(UObject::GetTransientPackage()) )
			{
				bEncounteredUnmarkedObject = TRUE;
				debugf(TEXT("----------------------------------------------------------------------------------------------------"));
				debugf(TEXT("%s didn't get cleaned up!"),*Object->GetFullName());
				UObject::StaticExec(*FString::Printf(TEXT("OBJ REFS CLASS=%s NAME=%s"),*Object->GetClass()->GetName(),*Object->GetPathName()));
				TMap<UObject*,UProperty*>	Route		= FArchiveTraceRoute::FindShortestRootPath( Object, TRUE, RF_Native  );
				FString						ErrorString	= FArchiveTraceRoute::PrintRootPath( Route, Object );
				debugf(TEXT("%s"),*ErrorString);
			}
		}
		check(!bEncounteredUnmarkedObject);
	}


	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	FString	CurrentTime = FString::Printf(TEXT("%i.%02i.%02i-%02i.%02i.%02i"), Year, Month, Day, Hour, Min, Sec );

	// Re-used helper variables for writing to CSV file.
	FString		CSVDirectory	= appGameLogDir() + TEXT("AssetStatsCSVs") PATH_SEPARATOR;
	FString		CSVFilename		= TEXT("");
	FArchive*	CSVFile			= NULL;
	
	// Create CSV folder in case it doesn't exist yet.
	GFileManager->MakeDirectory( *CSVDirectory );

	// CSV: Human-readable spreadsheet format.
	CSVFilename	= FString::Printf(TEXT("%sStaticMeshStats-%s-%i-%s.csv"), *CSVDirectory, GGameName, GEngineVersion, *CurrentTime);
	CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{	
		// Write out header row.
		const FString& HeaderRow = FStaticMeshStats::GetCSVHeaderRow();
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write out each individual stats row.
		for( TMap<FString,FStaticMeshStats>::TIterator It(ResourceNameToStaticMeshStats); It; ++ It )
		{
			const FStaticMeshStats& StatsEntry = It.Value();
			const FString& Row = StatsEntry.ToCSV();
			CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}

	CSVFilename	= FString::Printf(TEXT("%sTextureStats-%s-%i-%s.csv"), *CSVDirectory, GGameName, GEngineVersion, *CurrentTime);
	CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{	
		// Write out header row.
		const FString& HeaderRow = FTextureStats::GetCSVHeaderRow();
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write out each individual stats row.
		for( TMap<FString,FTextureStats>::TIterator It(ResourceNameToTextureStats); It; ++ It )
		{
			const FTextureStats& StatsEntry = It.Value();
			const FString& Row = StatsEntry.ToCSV();
			CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}

	CSVFilename	= FString::Printf(TEXT("%sMaterialStats-%s-%i-%s.csv"), *CSVDirectory, GGameName, GEngineVersion, *CurrentTime);
	CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{	
		// Write out header row.
		const FString& HeaderRow = FMaterialStats::GetCSVHeaderRow();
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write out each individual stats row.
		for( TMap<FString,FMaterialStats>::TIterator It(ResourceNameToMaterialStats); It; ++ It )
		{
			const FMaterialStats& StatsEntry = It.Value();
			const FString& Row = StatsEntry.ToCSV();
			CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}

	CSVFilename	= FString::Printf(TEXT("%sParticleStats-%s-%i-%s.csv"), *CSVDirectory, GGameName, GEngineVersion, *CurrentTime);
	CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{	
		// Write out header row.
		const FString& HeaderRow = FParticleStats::GetCSVHeaderRow();
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write out each individual stats row.
		for( TMap<FString,FParticleStats>::TIterator It(ResourceNameToParticleStats); It; ++ It )
		{
			const FParticleStats& StatsEntry = It.Value();
			const FString& Row = StatsEntry.ToCSV();
			CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}

	CSVFilename	= FString::Printf(TEXT("%sAnimStats-%s-%i-%s.csv"), *CSVDirectory, GGameName, GEngineVersion, *CurrentTime);
	CSVFile		= GFileManager->CreateFileWriter( *CSVFilename );
	if( CSVFile )
	{	
		// Write out header row.
		const FString& HeaderRow = FAnimSequenceStats::GetCSVHeaderRow();
		CSVFile->Serialize( TCHAR_TO_ANSI( *HeaderRow ), HeaderRow.Len() );

		// Write out each individual stats row.
		for( TMap<FString,FAnimSequenceStats>::TIterator It(ResourceNameToAnimStats); It; ++ It )
		{
			const FAnimSequenceStats& StatsEntry = It.Value();
			const FString& Row = StatsEntry.ToCSV();
			CSVFile->Serialize( TCHAR_TO_ANSI( *Row ), Row.Len() );
		}

		// Close and delete archive.
		CSVFile->Close();
		delete CSVFile;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Could not create CSV file %s for writing."), *CSVFilename);
	}

#if 0
	debugf(TEXT("%s"),*FStaticMeshStats::GetCSVHeaderRow());
	for( TMap<FString,FStaticMeshStats>::TIterator It(ResourceNameToStaticMeshStats); It; ++ It )
	{
		const FStaticMeshStats& StatsEntry = It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}

	debugf(TEXT("%s"),*FTextureStats::GetCSVHeaderRow());
	for( TMap<FString,FTextureStats>::TIterator It(ResourceNameToTextureStats); It; ++ It )
	{
		const FTextureStats& StatsEntry	= It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}

	debugf(TEXT("%s"),*FMaterialStats::GetCSVHeaderRow());
	for( TMap<FString,FMaterialStats>::TIterator It(ResourceNameToMaterialStats); It; ++ It )
	{
		const FMaterialStats& StatsEntry = It.Value();
		debugf(TEXT("%s"),*StatsEntry.ToCSV());
	}
#endif

	return 0;
}
IMPLEMENT_CLASS(UAnalyzeReferencedContentCommandlet)



/*-----------------------------------------------------------------------------
	UShowStylesCommandlet
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UShowStylesCommandlet);

INT UShowStylesCommandlet::Main(const FString& Params)
{
	INT Result = 0;

	// Parse command line args.
	TArray<FString> Tokens;
	TArray<FString> Switches;

	const TCHAR* Parms = *Params;
	ParseCommandLine(Parms, Tokens, Switches);

	for ( INT TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++ )
	{
		FString& PackageWildcard = Tokens(TokenIndex);

		TArray<FString> PackageFileNames;
		GFileManager->FindFiles( PackageFileNames, *PackageWildcard, 1, 0 );
		if( PackageFileNames.Num() == 0 )
		{
			// if no files were found, it might be an unqualified path; try prepending the .u output path
			// if one were going to make it so that you could use unqualified paths for package types other
			// than ".u", here is where you would do it
			GFileManager->FindFiles( PackageFileNames, *(appScriptOutputDir() * PackageWildcard), 1, 0 );

			if ( PackageFileNames.Num() == 0 )
			{
				TArray<FString> Paths;
				if ( GConfig->GetArray( TEXT("Core.System"), TEXT("Paths"), Paths, GEngineIni ) > 0 )
				{
					for ( INT i = 0; i < Paths.Num(); i++ )
					{
						GFileManager->FindFiles( PackageFileNames, *(Paths(i) * PackageWildcard), 1, 0 );
					}
				}
			}
			else
			{
				// re-add the path information so that GetPackageLinker finds the correct version of the file.
				FFilename WildcardPath = appScriptOutputDir() * PackageWildcard;
				for ( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
				{
					PackageFileNames(FileIndex) = WildcardPath.GetPath() * PackageFileNames(FileIndex);
				}
			}

			// Try finding package in package file cache.
			if ( PackageFileNames.Num() == 0 )
			{
				FString Filename;
				if( GPackageFileCache->FindPackageFile( *PackageWildcard, NULL, Filename ) )
				{
					new(PackageFileNames)FString(Filename);
				}
			}
		}
		else
		{
			// re-add the path information so that GetPackageLinker finds the correct version of the file.
			FFilename WildcardPath = PackageWildcard;
			for ( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
			{
				PackageFileNames(FileIndex) = WildcardPath.GetPath() * PackageFileNames(FileIndex);
			}
		}

		if ( PackageFileNames.Num() == 0 )
		{
			warnf(TEXT("No packages found using '%s'!"), *PackageWildcard);
			continue;
		}

		// reset the loaders for the packages we want to load so that we don't find the wrong version of the file
		// (otherwise, attempting to run pkginfo on e.g. Engine.xxx will always return results for Engine.u instead)
		for ( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
		{
			const FString& PackageName = FPackageFileCache::PackageFromPath(*PackageFileNames(FileIndex));
			UPackage* ExistingPackage = FindObject<UPackage>(NULL, *PackageName, TRUE);
			if ( ExistingPackage != NULL )
			{
				ResetLoaders(ExistingPackage);
			}
		}

		for( INT FileIndex = 0; FileIndex < PackageFileNames.Num(); FileIndex++ )
		{
			const FString &Filename = PackageFileNames(FileIndex);

			warnf( NAME_Log, TEXT("Loading %s"), *Filename );

			UObject* Package = UObject::LoadPackage( NULL, *Filename, LOAD_None );
			if( Package == NULL )
			{
				warnf( NAME_Error, TEXT("Error loading %s!"), *Filename );
			}
			else
			{
				for ( FObjectIterator It; It; ++It )
				{
					if ( It->IsIn(Package) && It->IsA(UUIStyle::StaticClass()) )
					{
						DisplayStyleInfo(Cast<UUIStyle>(*It));
					}
				}
			}
		}
	}

	return Result;
}

void UShowStylesCommandlet::DisplayStyleInfo( UUIStyle* Style )
{
	// display the info about this style
	GWarn->Log(TEXT("*************************"));
	GWarn->Logf(TEXT("%s"), *Style->GetPathName());
	GWarn->Logf(TEXT("\t Archetype: %s"), *Style->GetArchetype()->GetPathName());
	GWarn->Logf(TEXT("\t       Tag: %s"), *Style->GetStyleName());
	GWarn->Logf(TEXT("\tStyleClass: %s"), Style->StyleDataClass != NULL ? *Style->StyleDataClass->GetName() : TEXT("NULL"));
	GWarn->Log(TEXT("\tStyle Data:"));

	const INT Indent = GetStyleDataIndent(Style);
	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(Style->StateDataMap); It; ++It )
	{
		UUIState* State = It.Key();
		UUIStyle_Data* StyleData = It.Value();

		GWarn->Logf(TEXT("\t%*s: %s"), Indent, *State->GetClass()->GetName(), *StyleData->GetPathName());
		UUIStyle_Combo* ComboStyleData = Cast<UUIStyle_Combo>(StyleData);
		if ( ComboStyleData != NULL )
		{
			UUIStyle_Data* CustomTextStyle = ComboStyleData->TextStyle.GetCustomStyleData();
			GWarn->Logf(TEXT("\t\t TextStyle: %s"), CustomTextStyle ? *CustomTextStyle->GetPathName() : TEXT("NULL"));

			UUIStyle_Data* CustomImageStyle = ComboStyleData->ImageStyle.GetCustomStyleData();
			GWarn->Logf(TEXT("\t\tImageStyle: %s"), CustomImageStyle ? *CustomImageStyle->GetPathName() : TEXT("NULL"));
		}
	}
}

INT UShowStylesCommandlet::GetStyleDataIndent( UUIStyle* Style )
{
	INT Result = 0;

	check(Style);
	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(Style->StateDataMap); It; ++It )
	{
		FString StateClassName = It.Key()->GetClass()->GetName();
		Result = Max(StateClassName.Len(), Result);
	}

	return Result;
}


IMPLEMENT_CLASS(UTestWordWrapCommandlet);
INT UTestWordWrapCommandlet::Main(const FString& Params)
{
	INT Result = 0;

	// replace any \n strings with the real character code
	FString MyParams = Params.Replace(TEXT("\\n"), TEXT("\n"));
	const TCHAR* Parms = *MyParams;

	INT WrapWidth = 0;
	FString WrapWidthString;
	ParseToken(Parms, WrapWidthString, FALSE);
	WrapWidth = appAtoi(*WrapWidthString);

	// advance past the space between the width and the test string
	Parms++;
	warnf(TEXT("WrapWidth: %i  WrapText: '%s'"), WrapWidth, Parms);
	UFont* DrawFont = GEngine->GetTinyFont();

	FRenderParameters Parameters(0, 0, WrapWidth, 0, DrawFont);
	TArray<FWrappedStringElement> Lines;

	UUIString::WrapString(Parameters, 0, Parms, Lines, TEXT("\n"));

	warnf(TEXT("Result: %i lines"), Lines.Num());
	for ( INT LineIndex = 0; LineIndex < Lines.Num(); LineIndex++ )
	{
		FWrappedStringElement& Line = Lines(LineIndex);
		warnf(TEXT("Line %i): (X=%.2f,Y=%.2f) '%s'"), LineIndex, Line.LineExtent.X, Line.LineExtent.Y, *Line.Value);
	}

	return Result;
}


































