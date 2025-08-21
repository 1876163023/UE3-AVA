/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class OcclusionGroupComponent extends PrimitiveComponent 
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

cpptext
{
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
}

defaultproperties
{
}
