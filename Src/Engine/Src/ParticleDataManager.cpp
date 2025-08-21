/*=============================================================================
	ParticleDataManager.cpp: Particle dynamic data manager implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"

IMPLEMENT_CLASS(UParticleDataManager);

/*-----------------------------------------------------------------------------
	ParticleDataManager
-----------------------------------------------------------------------------*/
//
typedef TDynamicMap<UParticleSystemComponent*, UBOOL>::TIterator TParticleDataIterator;

/**
 *	Update the dynamic data for all particle system componets
 */
void FParticleDataManager::UpdateDynamicData()
{
	for (TParticleDataIterator It(PSysComponents); It; ++It)
	{
		UParticleSystemComponent* PSysComp = (UParticleSystemComponent*)(It.Key());
		if (PSysComp)
		{
			PSysComp->UpdateDynamicData();
		}
	}
	Clear();
}
	
//
/**
 *	Add a particle system component to the list.
 *
 *	@param		InPSysComp		The particle system component to add.
 *
 */
void FParticleDataManager::AddParticleSystemComponent(UParticleSystemComponent* InPSysComp)
{
	if ((GIsUCC == FALSE) && (GIsCooking == FALSE))
	{
		if (InPSysComp)
		{
			PSysComponents.Set(InPSysComp, TRUE);
		}
	}
}

/**
 *	Remove a particle system component to the list.
 *
 *	@param		InPSysComp		The particle system component to remove.
 *
 */
void FParticleDataManager::RemoveParticleSystemComponent(UParticleSystemComponent* InPSysComp)
{
	if ((GIsUCC == FALSE) && (GIsCooking == FALSE))
	{
		PSysComponents.Remove(InPSysComp);
	}
}

/**
 *	Clear all pending components from the queue.
 *
 */
void FParticleDataManager::Clear()
{
	//@todo. Move this to 'Reset' when that functionality it added to DynamicMap
	PSysComponents.Empty();
}
