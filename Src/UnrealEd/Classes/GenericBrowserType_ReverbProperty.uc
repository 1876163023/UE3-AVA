/**
 * Copyright ?2005 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_Texture: Textures
//=============================================================================

class GenericBrowserType_ReverbProperty
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );	
	virtual void DoubleClick( UObject* InObject );
}
	
defaultproperties
{
	Description="Reverb Preset"
}
