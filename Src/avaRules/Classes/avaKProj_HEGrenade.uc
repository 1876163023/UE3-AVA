class avaKProj_HEGrenade extends avaKProj_Grenade;

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Wp_M83.MS_M83_3p'
		bNotifyRigidBodyCollision=true
		BlockRigidBody=true
		LightEnvironment=MyLightEnvironment
	End Object

	mydamagetype 		= 	class'avaDMGType_GrenadeFlag'
	Damage			=	170.0
	DamageRadius		=	220
	FullDamageMinRadius	=	80
	MomentumTransfer		=	250000

	ExplodeTime=1.8

	FlashRadius=600
	FlashDamage=10
	FrontFlashParameters=(HoldTime=0.5,FadeTime=0.8,Alpha=2.2)
	BackFlashParameters=(HoldTime=0.2,FadeTime=0.5,Alpha=1.6)

	ExplosionSound=SoundCue'avaWeaponSounds.Grenade_HE.Grenade_HE_Explosion'	
}