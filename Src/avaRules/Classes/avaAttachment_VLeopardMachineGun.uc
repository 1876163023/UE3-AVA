class avaAttachment_VLeopardMachineGun extends avaWeaponAttachment;

defaultproperties
{
	WeaponClass=class'avaVWeap_LeopardMachineGun'
	DeathIconStr="W"
	bMeshIsSkeletal		=	true
	MeshName			=	""
	SocMeshName			=	""
	BasicMeshName		=	""
	PosRootBoneName		=	None
	SocRootBoneName		=	None
	CarriedSocketName	=	B3
	AnimPrefix			=	AK47
	AttachmentBoneName	=	WPBone03
	bRightHandedWeapon	=	false

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_Heavy_MuzzleFlash_3P_Tank'	
	MuzzleFlashDuration=0.33
}
