class avaAttachment_BaseRifle extends avaAttachment_BaseGun
	abstract;

defaultproperties
{
	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.007
		Intensity	=	0.05
		Size		=	2
		Speed		=	14000		
	End Object
	TrailInterval	=	3

	DamageCode=Rifle
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Rifle_MuzzleFlash_3P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'

	BulletWhip=SoundCue'avaWeaponSounds.Whip.BaseGun_BulletWhipCue'

	AttachmentWeaponType			=	WBT_SMG01
}
