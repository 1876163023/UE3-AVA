/**
 * Copyright ?2005 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_UIAnimation: UIWidget Animations
//=============================================================================

class GenericBrowserType_UIAnimation
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}
	
defaultproperties
{
	Description="UI Animation"
}
