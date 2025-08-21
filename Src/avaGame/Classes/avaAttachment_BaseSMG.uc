class avaAttachment_BaseSMG extends avaAttachment_BaseGun;

defaultproperties
{
	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.007
		Intensity	=	0.05
		Size		=	2
		Speed		=	14000		
	End Object
	TrailInterval	=	3

	DamageCode=SMG
	BulletWhip=SoundCue'avaWeaponSounds.Whip.BaseGun_BulletWhipCue'

	AttachmentWeaponType = WBT_SMG01
	AnimPrefix = G36

	DeathIconStr="v"

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_SMG_MuzzleFlash_3P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33

	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'
	SilencerMeshName = "Wp_Silencer.MS_SMG_Silencer"
}