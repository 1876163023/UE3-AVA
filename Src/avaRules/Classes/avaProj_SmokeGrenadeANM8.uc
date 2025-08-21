class avaProj_SmokeGrenadeANM8 extends avaProj_SmokeGrenade;
defaultproperties
{
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_M83.MS_M83_3p'
	End Object

	Physics					= PHYS_Falling
	ExplosionTime				= 0.5
	
	speed					= 400
	MaxSpeed				= 1200.0
	TossZ					= 245.0
	CustomGravityScale			= 0.6;

	bCollideWorld			= true
	bBounce				= true
	bNetTemporary			= false
	bWaitForEffects			= false
	
	bCheckProjectileLight		= false

	bExplodeWhenHitWall		= false
	bReflectOffWall			= true
	ReflectOffWallDamping		= 0.3
	ReflectSleepVelocity			= 30
	bSpeedFromOwner			= true

	SmokeTemplate			=	ParticleSystem'avaEffect.Smoke.Smoke_Screen01_PS'	
	ProjExplosionTemplate		=	None	
	MomentumTransfer			=	250000
	LifeSpan				=	0.0
	Damage				=	0
	DamageRadius			=	0
	SmokeDuration			=	21.0
}