class avaProj_HEGrenade extends avaProj_Grenade;

defaultproperties
{
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_Grenade_MK3A2.MS_MK3A2_3p'
	End Object

	Physics					= PHYS_Falling
	ExplosionTime			= 1.8	
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

	ProjExplosionTemplate		=	ParticleSystem'avaEffect.Explosion.PS_Grenade_Explosion01'
	ExplosionDecal			=	Material'avaDecal.Crater.M_grenade_Crater'
	ExplosionLightClass			=	class'avaLight_Explosion_HE'

	LifeSpan				=	0.0
	ExplosionShake			=	(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=10,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	
	ShakeRedius			=	1000
	DecalWidth			=	400
	DecalHeight			=	400
	bCheckAutoMessage		=	true

	mydamagetype 		= 	class'avaDMGType_GrenadeFlag'
	Damage			=	170.0
	DamageRadius		=	200
	FullDamageMinRadius	=	60
	MomentumTransfer		=	250000

	FlashRadius=600	

	// Hot spot
	FlashDamage=300
	// Value ; absolute luminance 
	// 테스트 해보니 Front값이 뒤에서 터진 것으로 처리됨, 앞뒤를 맞바꿈
	BackFlashParameters=(HoldTime=0.3,FadeTime=1.8,Alpha=2.2)
	FrontFlashParameters=(HoldTime=0.1,FadeTime=1.5,Alpha=1.3)

	ExplosionSound=SoundCue'avaWeaponSounds.Grenade_HE.Grenade_HE_Explosion'	

}