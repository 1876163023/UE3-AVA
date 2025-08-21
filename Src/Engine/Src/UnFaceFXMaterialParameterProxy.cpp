/*=============================================================================
	UnFaceFXMaterialParameterProxy.cpp: FaceFX Face Graph node proxy to support
	animating Unreal material parameters.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if WITH_FACEFX

#include "UnFaceFXMaterialParameterProxy.h"
#include "EngineMaterialClasses.h"

using namespace OC3Ent;
using namespace Face;

//------------------------------------------------------------------------------
// FFaceFXMaterialParameterProxy.
//------------------------------------------------------------------------------

#define kCurrentFFaceFXMaterialParameterProxyVersion 0

FX_IMPLEMENT_CLASS(FFaceFXMaterialParameterProxy, kCurrentFFaceFXMaterialParameterProxyVersion, FxGenericTargetProxy)

FFaceFXMaterialParameterProxy::FFaceFXMaterialParameterProxy()
	: SkeletalMeshComponent(NULL)
	, MaterialInstanceConstant(NULL)
	, MaterialSlotID(0)
	, ScalarParameterName(NAME_None)
{
}

FFaceFXMaterialParameterProxy::~FFaceFXMaterialParameterProxy()
{
}

void FFaceFXMaterialParameterProxy::Copy( FxGenericTargetProxy* Other )
{
	FFaceFXMaterialParameterProxy* OtherMaterialParameterProxy = FxCast<FFaceFXMaterialParameterProxy>(Other);
	if( OtherMaterialParameterProxy )
	{
		SkeletalMeshComponent = NULL;
		MaterialInstanceConstant = NULL;
		MaterialSlotID = OtherMaterialParameterProxy->MaterialSlotID;
		ScalarParameterName = OtherMaterialParameterProxy->ScalarParameterName;
	}
}

void FFaceFXMaterialParameterProxy::Update( FxReal Value )
{
	if( MaterialInstanceConstant )
	{
		MaterialInstanceConstant->SetScalarParameterValue(ScalarParameterName, Value);
	}
}

void FFaceFXMaterialParameterProxy::SetSkeletalMeshComponent( USkeletalMeshComponent* InSkeletalMeshComponent )
{
	SkeletalMeshComponent = InSkeletalMeshComponent;
	MaterialInstanceConstant = NULL;
	if( SkeletalMeshComponent )
	{
		UMaterialInstance* MaterialInstance = SkeletalMeshComponent->GetMaterial(MaterialSlotID);
		if( MaterialInstance && MaterialInstance->IsA(UMaterialInstanceConstant::StaticClass()) )
		{
			MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInstance);
		}

		if( !MaterialInstanceConstant && SkeletalMeshComponent->SkeletalMesh )
		{
			if( MaterialSlotID < SkeletalMeshComponent->SkeletalMesh->Materials.Num() && 
				SkeletalMeshComponent->SkeletalMesh->Materials(MaterialSlotID) )
			{
				if( SkeletalMeshComponent->bDisableFaceFXMaterialInstanceCreation )
				{
					debugf(TEXT("FaceFX: WARNING Unable to create MaterialInstanceConstant because bDisableFaceFXMaterialInstanceCreation is true!"));
				}
				else
				{
					UMaterialInstanceConstant* NewMaterialInstanceConstant = CastChecked<UMaterialInstanceConstant>( UObject::StaticConstructObject(UMaterialInstanceConstant::StaticClass(), InSkeletalMeshComponent) );
					NewMaterialInstanceConstant->SetParent(SkeletalMeshComponent->SkeletalMesh->Materials(MaterialSlotID));
					INT NumMaterials = SkeletalMeshComponent->Materials.Num();
					if( NumMaterials <= MaterialSlotID )
					{
						SkeletalMeshComponent->Materials.AddZeroed(MaterialSlotID + 1 - NumMaterials);
					}
					SkeletalMeshComponent->Materials(MaterialSlotID) = NewMaterialInstanceConstant;
					MaterialInstanceConstant = NewMaterialInstanceConstant;
				}
			}
		}
	}
}

FxBool FFaceFXMaterialParameterProxy::Link( FxInt32 InMaterialSlotID, const FxString& InScalarParameterName )
{
	MaterialSlotID = InMaterialSlotID;
	ScalarParameterName = FName(ANSI_TO_TCHAR(InScalarParameterName.GetCstr()));
	return FxTrue;
}

void FFaceFXMaterialParameterProxy::Destroy( void )
{
	this->~FFaceFXMaterialParameterProxy();
}

#endif // WITH_FACEFX
