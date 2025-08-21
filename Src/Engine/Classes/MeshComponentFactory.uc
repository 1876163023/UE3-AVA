/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MeshComponentFactory extends PrimitiveComponentFactory
	native
	abstract;

var(Rendering) array<MaterialInstance>	Materials;

cpptext
{
	virtual UPrimitiveComponent* CreatePrimitiveComponent(UObject* InOuter) { return NULL; }
}

defaultproperties
{
	CastShadow=True
}
