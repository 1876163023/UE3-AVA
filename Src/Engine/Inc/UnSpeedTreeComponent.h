//         Name: UnSpeedTreeComponent.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2004 IDV, Inc.
//      All Rights Reserved.
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// preprocessor

#ifndef __USPEEDTREECOMPONENT_H__
#define __USPEEDTREECOMPONENT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// includes

#include "UnSpeedTree.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class USpeedTreeComponent

class USpeedTreeComponent : public UPrimitiveComponent
{
						DECLARE_CLASS(USpeedTreeComponent, UPrimitiveComponent, CLASS_NoExport, Engine)
public:			

#ifdef WITH_SPEEDTREE
						// UActorComponent overloads
	virtual void		Tick(FLOAT fDeltaTime);
	virtual	void		PostEditChange(UProperty* puPropertyThatChanged);
	virtual void		UpdateBounds(void);
	virtual void		Render(const FSceneContext& cContext, FPrimitiveRenderInterface* pcPRI);
	virtual	void		RenderShadowVolume(const FSceneContext& cContext, FShadowVolumeRenderInterface* pSVRI, ULightComponent* pLight);
	virtual	void		Precache(void);
	virtual void		CacheLighting(const FLightingBuildOptions& BuildOptions);
	virtual	void		InvalidateLightingCache(void);

						// Collision
	virtual	UBOOL		PointCheck(FCheckResult& cResult, const FVector& cLocation, const FVector& cExtent);
	virtual UBOOL		LineCheck(FCheckResult& cResult, const FVector& cEnd, const FVector& cStart, const FVector& cExtent, DWORD dwTraceFlags);
#ifdef WITH_NOVODEX
	virtual void		InitComponentRBPhys(UBOOL bFixed);
#endif 

						// Lights
			UBOOL		AttachLight(ULightComponent* pLight);
			void		DetachLight(ULightComponent* pLight);
			void		DetachAllLights(void);

#endif

public:
	USpeedTree*						SpeedTree;

	BITFIELD						bUseLeaves		: 1 GCC_PACK(PROPERTY_ALIGNMENT);						
	BITFIELD 						bUseBranches	: 1;					
	BITFIELD 						bUseFronds		: 1;						
	BITFIELD						bUseBillboards	: 1;

	float							LodNearDistance;					
	float							LodFarDistance;					
	float							LodLevelOverride;				

	UTexture2D*						SpeedTreeIcon;
/*	TArray<FMeshElementLightInfo>	Lights;
	TArray<FGuid>					StaticLights;*/
};

#endif	
