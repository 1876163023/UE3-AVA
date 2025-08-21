class avaAttachment_SV98 extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_SV98'
	DeathIconStr		=	"l"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Sn_SV98.3P_Pos"
	SocMeshName			=	"Wp_Sn_SV98.3P_Soc"
	BasicMeshName		=	"Wp_Sn_SV98.MS_Sn_SV98_3P"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character �� �տ� �ش��ϴ� Bone Name
	CarriedSocketName	=	B3

	AttachmentBoneName	=	WPBone03
	bRightHandedWeapon	=	false

	AnimPrefix			=	SV98
//�߻�� ���� �׽�Ʈ
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'
}

