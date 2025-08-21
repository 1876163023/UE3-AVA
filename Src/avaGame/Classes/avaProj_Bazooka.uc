//=============================================================================
//  avaProj_Bazooka
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/05/29 by OZ
//		Bazooka ¿ë Projectile
//=============================================================================
class avaProj_Bazooka extends avaProjectile;

defaultproperties
{
	ProjFlightTemplate		=	ParticleSystem'avaEffect.Heavyweapons.Ps_RPG7_Smoke'
	ProjExplosionTemplate	=	ParticleSystem'avaEffect.Explosion.PS_Grenade_Explosion01'
	ExplosionDecal			=	Material'avaDecal.Crater.M_grenade_Crater'
	DecalWidth				=	400
	DecalHeight				=	400
	//ExplosionLightClass=class'UTGame.UTRocketExplosionLight'
	speed					=	600.0
	MaxSpeed				=	1600.0
	AccelRate				=	1000.0

	Damage					=	200.0
	DamageRadius			=	256
	FullDamageMinRadius		=	128
	MomentumTransfer		=	250000
	
	MyDamageType			=	class'avaDmgType_Explosion'
	LifeSpan				=	7.0
	bProjTarget				=	True

	//AmbientSound=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Travel_Cue'
	//ExplosionSound=SoundCue'A_Weapon.AVRiL.Cue.A_Weapon_AVRiL_ImpactCue'

	//RotationRate			=	(Roll=50000)
	//DesiredRotation			=	(Roll=30000)
	bCollideWorld			=	true

	Begin Object Name=ProjectileMesh
		//'WP_AVRiL.Mesh.S_WP_AVRiL_Missile'
		StaticMesh=StaticMesh'Wp_Heavy_RPG7.MS_Heavy_RPG7_missile'
		Rotation=(Yaw=49152)
	End Object

	bExplodeWhenHitWall		=	true

	//bUpdateSimulatedPosition	=	true
	//bNetTemporary				=	false
	bWaitForEffects				=	true
	bRotationFollowsVelocity	=	true
}