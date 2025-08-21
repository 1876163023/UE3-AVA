class avaProj_RPGRocket extends avaProj_Bazooka;

defaultproperties
{
	ProjFlightTemplate			=	ParticleSystem'avaEffect.Heavyweapons.Ps_RPG7_Smoke'
	ProjExplosionTemplate		=	ParticleSystem'avaEffect.Explosion.PS_RPG7_Explosion'
	ExplosionDecal			=	Material'avaDecal.Crater.M_grenade_Crater'
	DecalWidth			=	400
	DecalHeight			=	400

	MaxEffectDistance			=	25000.0

	speed					=	50.0 
	MaxSpeed				=	8000.0
	AccelRate				=	2000.0

	Damage				=	250.0
	DamageRadius			=	280
	FullDamageMinRadius		=	80
	MomentumTransfer			=	250000
	
	MyDamageType			=	class'avaDmgType_RPG7'
	LifeSpan				=	15.0
	bProjTarget			=	True

	//AmbientSound=SoundCue'A_Weapon.RocketLauncher.Cue.A_Weapon_RL_Travel_Cue'
	ExplosionSound=SoundCue'avaWeaponSounds.Heavy_RPG7.Heavy_RPG7_Explosion'
	ExplosionLightClass			=	class'avaLight_Explosion_RPG'

	//RotationRate			=	(Roll=50000)
	//DesiredRotation		=	(Roll=30000)
	bCollideWorld			=	true

	Begin Object Name=ProjectileMesh
		StaticMesh=StaticMesh'Wp_Heavy_RPG7.MS_Heavy_RPG7_missile'
		Rotation=(Yaw=49152)
	End Object

	//bUpdateSimulatedPosition	=	true
	//bNetTemporary				=	false
	bWaitForEffects				=	true
	AdjustInstigatorVelFactor	=	0.0
	//bRotationFollowsVelocity	=	true

	ProjFlightSound				=	SoundCue'avaWeaponSounds.Heavy_RPG7.Heavy_RPG7_Fly_by'



}