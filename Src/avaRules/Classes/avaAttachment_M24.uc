class avaAttachment_M24 extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_M24'
	DeathIconStr="o"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_sn_M24_Scope.3P_Pos"
	SocMeshName			=	"Wp_sn_M24_Scope.3P_Soc"
	BasicMeshName		=	"Wp_sn_M24_Scope.MS_M24_Scope_3P"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character �� �տ� �ش��ϴ� Bone Name
	CarriedSocketName	=	B3
	
	AnimPrefix = M24

	AttachmentBoneName=WPBone03
	bRightHandedWeapon=false

//�߻�� ���� �׽�Ʈ
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'
}

