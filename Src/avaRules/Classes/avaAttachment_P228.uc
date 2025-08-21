/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_P228 extends avaAttachment_BasePistol;


defaultproperties
{
	WeaponClass=class'avaWeap_P228'

	DeathIconStr="y"

/* FIXME: this doesn't work because enforcer is using a static mesh
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Rifle_MuzzleFlash_1P'
	MuzzleFlashDuration=0.33
*/
	AnimPrefix = P226
}
