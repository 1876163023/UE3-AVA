class avaAttachment_DragunovSVD extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_DragunovSVD'
	DeathIconStr		=	"1"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_sn_SVD.3P_Pos"
	SocMeshName			=	"Wp_sn_SVD.3P_Soc"
	BasicMeshName		=	"Wp_sn_SVD.MS_SVD_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AttachmentBoneName	=	WPBone01
	bRightHandedWeapon	=	true

	AnimPrefix			=	SVD
//발사시 연기 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'

}

