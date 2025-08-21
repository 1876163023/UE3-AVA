/*=============================================================================
	UnFaceFXMaterialParameterProxy.h: FaceFX Face Graph node proxy to support
	animating Unreal material parameters.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if WITH_FACEFX

#ifndef UnFaceFXMaterialParameterProxy_H__
#define UnFaceFXMaterialParameterProxy_H__

#include "../../../External/FaceFX/FxSDK/Inc/FxGenericTargetNode.h"

using OC3Ent::Face::FxBool;
using OC3Ent::Face::FxInt32;
using OC3Ent::Face::FxReal;
using OC3Ent::Face::FxString;
using OC3Ent::Face::FxGenericTargetProxy;
using OC3Ent::Face::FxArchive;

// Forward declarations.
class USkeletalMeshComponent;
class UMaterialInstanceConstant;

class FFaceFXMaterialParameterProxy : public FxGenericTargetProxy
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_SERIALIZE(FFaceFXMaterialParameterProxy, FxGenericTargetProxy)
public:
	// Constructor.
	FFaceFXMaterialParameterProxy();
	// Destructor.
	virtual ~FFaceFXMaterialParameterProxy();

	// Copies the properties from the other generic target proxy into this one.
	virtual void Copy( FxGenericTargetProxy* Other );
	// Updates the generic target.
	virtual void Update( FxReal Value );

	// Sets the skeletal mesh that owns the material parameter that the proxy
	// controls.
	void SetSkeletalMeshComponent( USkeletalMeshComponent* InSkeletalMeshComponent );

	// Links the proxy to the material parameter that it controls.
	FxBool Link( FxInt32 InMaterialSlotID, const FxString& InScalarParameterName );

protected:
	// A pointer to the skeletal mesh component that owns the material parameter 
	// that the proxy controls.
	USkeletalMeshComponent* SkeletalMeshComponent;
	// A pointer to the material instance constant that the proxy controls.
	UMaterialInstanceConstant* MaterialInstanceConstant;
	// The material slot id.
	FxInt32 MaterialSlotID;
	// The name of the scalar parameter that the proxy controls.
	FName ScalarParameterName;

	// Destroys the material parameter proxy.
	virtual void Destroy( void );
};

#endif

#endif // WITH_FACEFX
