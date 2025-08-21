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
	//AttachmentBoneName;		// Character �� �տ� �ش��ϴ� Bone Name
	CarriedSocketName	=	B3

	AttachmentBoneName	=	WPBone01
	bRightHandedWeapon	=	true

	AnimPrefix			=	SVD
//�߻�� ���� �׽�Ʈ
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'

}

