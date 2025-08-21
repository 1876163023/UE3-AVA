/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_BaseSniperRifle extends avaAttachment_BaseGun;

defaultproperties
{
	Begin Object Name=BulletTrailComponent0
		HalfLife	=	0.01
		Intensity	=	0.05
		Size		=	3
		Speed		=	14000		
	End Object
	TrailInterval	=	1

	DamageCode=SniperRifle
	BulletWhip=SoundCue'avaWeaponSounds.Whip.BaseGun_BulletWhipCue'

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_SniperRifle_MuzzleFlash_3P'	
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'

	WeaponClass=class'avaWeap_BaseSniperRifle'

	AttachmentWeaponType = WBT_SMG01

	AttachmentBoneName=WPBone03
	bRightHandedWeapon=false
}
