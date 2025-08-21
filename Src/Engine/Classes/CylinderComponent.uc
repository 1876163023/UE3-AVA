/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class CylinderComponent extends PrimitiveComponent
	native
	noexport
	collapsecategories
	editinlinenew;

var() const export float	CollisionHeight;
var() const export float	CollisionRadius;

native final function SetSize(float NewHeight, float NewRadius);

// The rotation part of the local-to-world transformation has no effect on the cylinder; it is always
// assumed to be aligned with the z-axis. The translation part is however taken into consideration.

defaultproperties
{
	HiddenGame=TRUE
	BlockZeroExtent=true
	BlockNonZeroExtent=true
	CollisionRadius=+00022.000000
	CollisionHeight=+00022.000000
	bAcceptsLights=false
	bCastDynamicShadow=false
}
