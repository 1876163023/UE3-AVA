class avaWeap_MachineGun extends avaWeap_BaseMachineGun;

defaultproperties
{
	AttachmentClass		= class'avaAttachment_MachineGun'
	BulletType			= class'avaBullet_762NATO'

	MuzzleFlashSocket	    = MuzzleFlashSocket
	MuzzleFlashPSCTemplate  = ParticleSystem'avaEffect.Particles.P_WP_Heavy_MuzzleFlash_1P'
	MuzzleFlashDuration		= 0.05	
	InstantHitDamageTypes(0)	=	class'avaDmgType_Rifle'
}