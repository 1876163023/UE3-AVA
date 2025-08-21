//=============================================================================
// BlockingVolume:  a bounding volume
// used to block certain classes of actors
// primary use is to provide collision for non-zero extent traces around static meshes 
// Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class BlockingVolume extends Volume
	native
	placeable;

var() bool bClampFluid;

var() bool bIgnoreProjectile;		// true 이면 Projectile Blocking 하지 않는다...
var() bool bIgnorePawn;				// true 이면 Pawn 을 Blocking 하지 않는다...

cpptext
{
	UBOOL IgnoreBlockingBy( const AActor *Other ) const;
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=true
		BlockActors=true
		BlockZeroExtent=false
		BlockNonZeroExtent=true
		BlockRigidBody=true
	End Object

	bWorldGeometry=true
    bCollideActors=True
    bBlockActors=True
    bClampFluid=True

	bIgnoreProjectile	=	true
	bIgnorePawn			=	false
}
