class avaProj_FlashBang extends avaProj_Grenade;

defaultproperties
{
	Begin Object Name=ProjectileMesh		
		StaticMesh=StaticMesh'Wp_Grenade_MK3A2.MS_MK3A2_3p'
	End Object

	Physics					= PHYS_Falling
	ExplosionTime				= 1.5	
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
	ReflectOffWallDamping		= 0.35
	ReflectSleepVelocity		= 30
	bSpeedFromOwner			= true

	ProjExplosionTemplate		=	ParticleSystem'avaEffect.Explosion.PS_Grenade_Explosion01'
	ExplosionDecal			=	None
	ExplosionLightClass		=	class'avaLight_Explosion_FlashBang'

	LifeSpan			=	0.0
	ExplosionShake			=	(OffsetMag=(X=-5.0,Y=-5.00,Z=-5.00),OffsetRate=(X=-1000.0,Y=-1000.0,Z=-1000.0),OffsetTime=10,RotMag=(X=0.0,Y=0.0,Z=0.0),RotRate=(X=0.0,Y=0.0,Z=0.0),RotTime=2)	
	ShakeRedius			=	500
	DecalWidth			=	400
	DecalHeight			=	400
	bCheckAutoMessage		=	true

	mydamagetype 		= 	class'avaDMGType_GrenadeFlag'
	Damage			=	0.0
	DamageRadius		=	0
	FullDamageMinRadius	=	0
	MomentumTransfer	=	250000

	FlashRadius = 400	

	// Hot spot
	FlashDamage = 250

	// Value ; absolute luminance 
	// 테스트 해보니 Front값이 뒤에서 터진 것으로 처리됨, 앞뒤를 맞바꿈
	BackFlashParameters=(HoldTime=1.3 ,FadeTime=2 ,Alpha=2.2)
	FrontFlashParameters=(HoldTime=0.1 ,FadeTime=1.5 ,Alpha=1.3)

	ExplosionSound=SoundCue'avaWeaponSounds.Grenade_HE.Grenade_HE_Explosion'	

}