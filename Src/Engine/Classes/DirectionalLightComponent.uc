/**
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class DirectionalLightComponent extends LightComponent
	native
	hidecategories(Object)
	editinlinenew;
	
//	ava needs categories; 2007. 1. 17 changmin
//	collapsecategories 

/**
 * Trace distance for static lighting. Objects further than TraceDistance away from an object won't be taken into 
 * account for static shadowing applied to said object. This is used to work around floating point consistency
 * issues in the collision code with regard to very long traces. The old default was WORLD_MAX.
 */
var(AdvancedLighting)	float	TraceDistance;

/**
 * AVA specific
 * Light environment에서만 사용 :)
 */
var	transient			float	ShadowFalloffExponent, ShadowFalloffDistance, ShadowOffset;

cpptext
{
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual FVector4 GetPosition() const;
	virtual ELightComponentType GetLightType() const;
}

defaultproperties
{
	TraceDistance=100000
	bCastCompositeShadow=TRUE
}
