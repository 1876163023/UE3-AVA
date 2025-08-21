//=============================================================================
//  avaProj_Motar
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/06/07 by OZ
//		Motar 용 Projectile
//=============================================================================
class avaProj_Motar extends avaProjectile;


simulated function Explode(vector HitLocation, vector HitNormal)
{
	if (Damage>0 && DamageRadius>0)
		HurtRadius(Damage,DamageRadius, MyDamageType, MomentumTransfer, HitLocation );

	SpawnExplosionEffects(HitLocation, HitNormal);

	if ( Role == ROLE_Authority )
		MakeNoise(1.0);

	ShutDown();
}

defaultproperties
{
	//ProjFlightTemplate=ParticleSystem'avaBazooka.Particles.P_WP_Avril_Smoke_Trail'
	//ProjExplosionTemplate=ParticleSystem'avaBazooka.Particles.P_WP_Avril_Explo'
	//ExplosionLightClass=class'UTGame.UTRocketExplosionLight'

	// 곡사화기 이기 때문에 MaxSpeed 랑 AccelRate 를 주면 안될것 같다.
	speed			=	2400.0
	//MaxSpeed		=	2400.0
	//AccelRate		=	800.0
	TossZ			=	0.0

	Damage			=	125.0
	DamageRadius	=	150.0
	MomentumTransfer=	150000
	
	//MyDamageType=class'UTDmgType_AvrilRocket'
	LifeSpan		=	0.0
	bProjTarget		=	True

	//AmbientSound=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Travel_Cue'
	//ExplosionSound=SoundCue'A_Weapon.AVRiL.Cue.A_Weapon_AVRiL_ImpactCue'

	RotationRate=(Roll=50000)
	DesiredRotation=(Roll=30000)
	bCollideWorld=true

	Begin Object Name=ProjectileMesh
		StaticMesh=StaticMesh'Wp_Heavy_RPG7.MS_Heavy_RPG7_missile'
		Scale=0.75
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=15
		CollisionHeight=10
		AlwaysLoadOnClient=True
		AlwaysLoadOnServer=True
		BlockNonZeroExtent=true
		BlockZeroExtent=true
		BlockActors=true
		CollideActors=true
	End Object

	bUpdateSimulatedPosition=true
	bNetTemporary=false
	bWaitForEffects=true
	bRotationFollowsVelocity=true

	Physics=PHYS_Falling
}