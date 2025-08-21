/*=============================================================================
	UnFaceFXMorphTargetProxy.h: FaceFX Face Graph node proxy to support
	animating Unreal morph targets.
	Copyright 2003-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if WITH_FACEFX

#ifndef UnFaceFXMorphTargetProxy_H__
#define UnFaceFXMorphTargetProxy_H__

#include "../../../External/FaceFX/FxSDK/Inc/FxGenericTargetNode.h"

using OC3Ent::Face::FxBool;
using OC3Ent::Face::FxReal;
using OC3Ent::Face::FxString;
using OC3Ent::Face::FxGenericTargetProxy;
using OC3Ent::Face::FxArchive;

class FFaceFXMorphTargetProxy : public FxGenericTargetProxy
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_SERIALIZE(FFaceFXMorphTargetProxy, FxGenericTargetProxy)
public:
	// Constructor.
	FFaceFXMorphTargetProxy();
	// Destructor.
	virtual ~FFaceFXMorphTargetProxy();

	// Copies the properties from the other generic target proxy into this one.
	virtual void Copy( FxGenericTargetProxy* Other );
	// Updates the generic target.
	virtual void Update( FxReal Value );

	// Sets the skeletal mesh that owns the morph target that the proxy controls.
	void SetSkeletalMeshComponent( USkeletalMeshComponent* InSkeletalMeshComponent );

	// Links the proxy to the morph target that it controls.
	FxBool Link( const FxString& InMorphTargetName );

protected:
	// A pointer to the skeletal mesh component that owns the morph target 
	// that the proxy controls.
	USkeletalMeshComponent* SkeletalMeshComponent; 
	// The name of the morph target that the proxy controls.
	FName MorphTargetName;

	// Destroys the morph target proxy.
	virtual void Destroy( void );
};

#endif

#endif // WITH_FACEFX
