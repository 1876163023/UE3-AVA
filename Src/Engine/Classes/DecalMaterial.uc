/**
 * Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
 */
class DecalMaterial extends Material
	native(Decal);

cpptext
{
	// UMaterial interface.
	virtual FMaterialResource* AllocateResource();

	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PreSave();
	virtual void PostLoad();
}
