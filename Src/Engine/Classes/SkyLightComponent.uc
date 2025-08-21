/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class SkyLightComponent extends LightComponent
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** The brightness for the lower hemisphere of the sky light. */
var() const float LowerBrightness;

/** The color of the lower hemisphere of the sky light. */
var() const color LowerColor;

var() const AmbientSH IrradianceSH;

cpptext
{
	/**
	 * Called when a property is being changed.
	 *
	 * @param PropertyThatChanged	Property that changed or NULL if unknown or multiple
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	/**
	 * Called after data has been serialized.
	 */
	virtual void PostLoad();

	// ULightComponent interface.
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual FVector4 GetPosition() const;
	virtual ELightComponentType GetLightType() const;
}

defaultproperties
{
	CastShadows=False
	bCastCompositeShadow=TRUE
	LowerColor=(R=255,G=255,B=255)
}
