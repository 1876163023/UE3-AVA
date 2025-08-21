/**
 * Copyright ?2004-2006 Red duck, Inc. All Rights Reserved.
 */
class SHPointLightComponent extends PointLightComponent
	native
	collapsecategories
	hidecategories(Object);

cpptext
{
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual ELightComponentType GetLightType() const;
	virtual UBOOL IsDepthDrawingLight() const;
}

defaultproperties
{
}
