class avaAttachment_BasePistol extends avaAttachment_BaseGun;

defaultproperties
{
	DamageCode=Pistol
	BulletWhip=SoundCue'avaWeaponSounds.Whip.BaseGun_BulletWhipCue'
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Pistol_MuzzleFlash_3P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'

	AttachmentWeaponType = WBT_SMG01
	AnimPrefix = P226
}