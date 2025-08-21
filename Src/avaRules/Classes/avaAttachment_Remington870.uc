class avaAttachment_Remington870 extends avaAttachment_BaseShotgun;

defaultproperties
{
	WeaponClass=class'avaWeap_Remington870'
	DeathIconStr		=	"4"

	bMeshIsSkeletal		=	true
	MeshName			=	"WP_Shot_remingtonM870.3P_Pos"
	SocMeshName			=	"WP_Shot_remingtonM870.3P_Soc"
	BasicMeshName		=	"WP_Shot_remingtonM870.MS_remington_m870_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	Remington870

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_ShotGun_MuzzleFlash_3P'	
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_ShotGun_Muzzlesmoke_3P'
	MuzzleFlashColor=(R=255,G=120,B=255,A=255)
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'avaGame.avaGunMuzzleFlashLight'
}

