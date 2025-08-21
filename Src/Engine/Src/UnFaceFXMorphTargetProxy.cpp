/*=============================================================================
	UnFaceFXMorphTargetProxy.cpp: FaceFX Face Graph node proxy to support
	animating Unreal morph targets.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if WITH_FACEFX

#include "UnFaceFXMorphTargetProxy.h"
#include "EngineMaterialClasses.h"

using namespace OC3Ent;
using namespace Face;

//------------------------------------------------------------------------------
// FFaceFXMorphTargetProxy.
//------------------------------------------------------------------------------

#define kCurrentFFaceFXMorphTargetProxyVersion 0

FX_IMPLEMENT_CLASS(FFaceFXMorphTargetProxy, kCurrentFFaceFXMorphTargetProxyVersion, FxGenericTargetProxy)

FFaceFXMorphTargetProxy::FFaceFXMorphTargetProxy()
	: SkeletalMeshComponent(NULL)
	, MorphTargetName(NAME_None)
{
}

FFaceFXMorphTargetProxy::~FFaceFXMorphTargetProxy()
{
}

void FFaceFXMorphTargetProxy::Copy( FxGenericTargetProxy* Other )
{
	FFaceFXMorphTargetProxy* OtherMaterialParameterProxy = FxCast<FFaceFXMorphTargetProxy>(Other);
	if( OtherMaterialParameterProxy )
	{	
		SkeletalMeshComponent = NULL;
		MorphTargetName = OtherMaterialParameterProxy->MorphTargetName;
	}
}

void FFaceFXMorphTargetProxy::Update( FxReal Value )
{
	if( SkeletalMeshComponent )
	{
	    UMorphTarget* MorphTarget = SkeletalMeshComponent->FindMorphTarget(MorphTargetName);
		if( MorphTarget )
		{
			if( Value > 0.0f || Value < 0.0f )
			{
				SkeletalMeshComponent->ActiveMorphs.AddItem(FActiveMorph(MorphTarget, Value));
			}
		}
	}
}

void FFaceFXMorphTargetProxy::SetSkeletalMeshComponent( USkeletalMeshComponent* InSkeletalMeshComponent )
{
	SkeletalMeshComponent = InSkeletalMeshComponent;
}

FxBool FFaceFXMorphTargetProxy::Link( const FxString& InMorphTargetName )
{
	MorphTargetName = FName(ANSI_TO_TCHAR(InMorphTargetName.GetCstr()));
	return FxTrue;
}

void FFaceFXMorphTargetProxy::Destroy( void )
{
	this->~FFaceFXMorphTargetProxy();
}

#endif // WITH_FACEFX
