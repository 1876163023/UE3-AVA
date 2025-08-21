/*=============================================================================
	UnMorphMesh.cpp: Unreal morph target mesh and blending implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.	
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"

/**
* serialize members
*/
void UMorphTarget::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	Ar << MorphLODModels;
}

/**
* called when object is loaded
*/
void UMorphTarget::PostLoad()
{
	Super::PostLoad();
}

