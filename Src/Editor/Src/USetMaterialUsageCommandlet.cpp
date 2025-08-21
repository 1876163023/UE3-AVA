/*=============================================================================
	USetMaterialUsageCommandlet.cpp - Commandlet which finds what types of geometry a material is used on.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineMaterialClasses.h"
#include "EngineParticleClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineAnimClasses.h"

#include "..\..\UnrealEd\Inc\scc.h"
#include "..\..\UnrealEd\Inc\SourceControlIntegration.h"

struct FPackageMaterialInfo
{
	TArray<FString> SkeletalMeshMaterials;
	TArray<FString> ParticleSystemMaterials;
};

static void SetMaterialUsage(
	TDynamicMap<FName,FPackageMaterialInfo>& PackageInfoMap,
	UMaterialInstance* MaterialInstance,
	UBOOL bUsedWithSkeletalMesh,
	UBOOL bUsedWithParticleSystem
	)
{
	UMaterial* Material = MaterialInstance->GetMaterial();
	if(Material)
	{
		FPackageMaterialInfo* PackageInfo = PackageInfoMap.Find(Material->GetOutermost()->GetFName());
		if(!PackageInfo)
		{
			PackageInfo = &PackageInfoMap.Set(Material->GetOutermost()->GetFName(),FPackageMaterialInfo());
		}
		if(bUsedWithSkeletalMesh && !Material->bUsedWithSkeletalMesh)
		{
			PackageInfo->SkeletalMeshMaterials.AddUniqueItem(Material->GetPathName());
		}
		if(bUsedWithParticleSystem && !Material->bUsedWithParticleSystem)
		{
			PackageInfo->ParticleSystemMaterials.AddUniqueItem(Material->GetPathName());
		}
	}
}

INT USetMaterialUsageCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList;

	FString PackageWildcard;
	FString PackagePrefix;
	if(ParseToken(Parms,PackageWildcard,FALSE))
	{
		GFileManager->FindFiles(PackageList,*PackageWildcard,TRUE,FALSE);
		PackagePrefix = FFilename(PackageWildcard).GetPath() * TEXT("");
	}
	else
	{
		PackageList = GPackageFileCache->GetPackageFileList();
	}
	if( !PackageList.Num() )
		return 0;

	FSourceControlIntegration* SCC = new FSourceControlIntegration;

	// Iterate over all packages.
	TDynamicMap<FName,FPackageMaterialInfo> PackageInfoMap;
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename Filename = PackagePrefix * PackageList(PackageIndex);

		warnf(NAME_Log, TEXT("Loading %s"), *Filename);

		UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
		if (Package != NULL)
		{
			// Iterate over all objects in the package.
			for(FObjectIterator ObjectIt;ObjectIt;++ObjectIt)
			{
				if(ObjectIt->IsIn(Package))
				{
					USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(*ObjectIt);
					USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(*ObjectIt);
					UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(*ObjectIt);
					if(SkeletalMeshComponent)
					{
						// Mark all the materials referenced by the skeletal mesh component as being used with a skeletal mesh.
						for(INT MaterialIndex = 0;MaterialIndex < SkeletalMeshComponent->Materials.Num();MaterialIndex++)
						{
							UMaterialInstance* Material = SkeletalMeshComponent->Materials(MaterialIndex);
							if(Material)
							{
								SetMaterialUsage(PackageInfoMap,Material,TRUE,FALSE);
							}
						}
					}
					else if(SkeletalMesh)
					{
						// Mark all the materials referenced by the skeletal mesh as being used with a skeletal mesh.
						for(INT MaterialIndex = 0;MaterialIndex < SkeletalMesh->Materials.Num();MaterialIndex++)
						{
							UMaterialInstance* Material = SkeletalMesh->Materials(MaterialIndex);
							if(Material)
							{
								SetMaterialUsage(PackageInfoMap,Material,TRUE,FALSE);
							}
						}
					}
					else if(SpriteEmitter)
					{
						// Mark the sprite's material as being used with a particle system.
						if(SpriteEmitter->Material)
						{
							SetMaterialUsage(PackageInfoMap,SpriteEmitter->Material,FALSE,TRUE);
						}
					}
					else if(!appStricmp(*ObjectIt->GetClass()->GetName(),TEXT("SeqAct_SetMaterial")))
					{
						// Extract the value of the script NewMaterial property.
						UProperty* MaterialProperty = CastChecked<UProperty>(ObjectIt->FindObjectField(FName(TEXT("NewMaterial"))));
						UMaterialInstance* Material = *(UMaterialInstance**)((BYTE*)*ObjectIt + MaterialProperty->Offset);
						if(Material)
						{
							USequenceAction* SequenceAction = CastChecked<USequenceAction>(*ObjectIt);
							// If the SetMaterial targets include a skeletal mesh, mark the material as being used with a skeletal mesh.
							for(INT TargetIndex = 0;TargetIndex < SequenceAction->Targets.Num();TargetIndex++)
							{
								ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(SequenceAction->Targets(TargetIndex));
								if(SkeletalMeshActor)
								{
									SetMaterialUsage(PackageInfoMap,Material,TRUE,FALSE);
									break;
								}
							}
						}
					}
				}
			}
		}

		UObject::CollectGarbage(RF_Native);
		SaveLocalShaderCaches();
	}

	for(TDynamicMap<FName,FPackageMaterialInfo>::TConstIterator PackageIt(PackageInfoMap);PackageIt;++PackageIt)
	{
		const FPackageMaterialInfo& PackageInfo = PackageIt.Value();
		// Only save dirty packages.
		if(PackageInfo.SkeletalMeshMaterials.Num() || PackageInfo.ParticleSystemMaterials.Num())
		{
			warnf(
				TEXT("Package %s is dirty(%u,%u)"),
				*PackageIt.Key().ToString(),
				PackageInfo.SkeletalMeshMaterials.Num(),
				PackageInfo.ParticleSystemMaterials.Num()
				);

			FString Filename;
			if(GPackageFileCache->FindPackageFile(*PackageIt.Key().ToString(),NULL,Filename))
			{
				UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
				if(Package)
				{
					UObject::ResetLoaders(Package);

					for(INT MaterialIndex = 0;MaterialIndex < PackageInfo.SkeletalMeshMaterials.Num();MaterialIndex++)
					{
						UMaterial* Material = FindObjectChecked<UMaterial>(NULL,*PackageInfo.SkeletalMeshMaterials(MaterialIndex));
						Material->UseWithSkeletalMesh();
					}

					for(INT MaterialIndex = 0;MaterialIndex < PackageInfo.ParticleSystemMaterials.Num();MaterialIndex++)
					{
						UMaterial* Material = FindObjectChecked<UMaterial>(NULL,*PackageInfo.ParticleSystemMaterials(MaterialIndex));
						Material->UseWithParticleSystem();
					}

					if (GFileManager->IsReadOnly(*Filename))
					{
						SCC->CheckOut(Package);
					}
					if (!GFileManager->IsReadOnly(*Filename))
					{
						// resave the package
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
			}

			UObject::CollectGarbage(RF_Native);
			SaveLocalShaderCaches();
		}
	}

	delete SCC; // clean up our allocated SCC

	return 0;
}

IMPLEMENT_CLASS(USetMaterialUsageCommandlet);