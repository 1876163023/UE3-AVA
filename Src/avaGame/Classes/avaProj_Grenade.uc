class avaProj_Grenade extends avaProjectile;

defaultproperties
{
	// Add the Meshk
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_M67.MS_M67_3p'
	End Object

	Physics					= PHYS_Falling
	ExplosionTime			= 3.0
	
	speed					= 400
	MaxSpeed				= 1000.0
	TossZ					= 245.0
	CustomGravityScale		= 0.5;

	bCollideWorld			= true
	bBounce					= true
	bNetTemporary			= false
	bWaitForEffects			= false
	
	bCheckProjectileLight	= false

	bExplodeWhenHitWall		= false
	bReflectOffWall			= true
	ReflectOffWallDamping	= 0.5
	ReflectSleepVelocity	= 20
	bSpeedFromOwner			= true

	ProjExplosionTemplate	=	ParticleSystem'avaEffect.Explosion.PS_Grenade_Explosion01'
	ExplosionDecal			=	Material'avaDecal.Crater.M_grenade_Crater'
	Damage					=	140.0
	DamageRadius			=	250
	FullDamageMinRadius		=	100
	MomentumTransfer		=	250000
	MyDamageType			=	class'avaDmgType_Explosion'
	LifeSpan				=	0.0
	ExplosionSound			=	SoundCue'avaWeaponSounds.Grenade_HE.Grenade_HE_Explosion'
	ExplosionShake			=	(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=10,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	
	ShakeRedius				=	800
	DecalWidth				=	400
	DecalHeight				=	400
	bCheckAutoMessage		=	true
	bIndicating				=	true
}
