/**
 * Copyright © 2005 Epic Games, Inc. All Rights Reserved.
 */

class GenericBrowserType_DecalMaterial
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}
	
defaultproperties
{
	Description="Decal Material"
}
