class avaAttachment_L96A1 extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_L96A1'
	DeathIconStr="o"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_sn_M24_Scope.3P_Pos"
	SocMeshName			=	"Wp_sn_M24_Scope.3P_Soc"
	BasicMeshName		=	"Wp_sn_M24_Scope.MS_M24_Scope_3P"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3
	
	AnimPrefix = M24

	AttachmentBoneName=WPBone03
	bRightHandedWeapon=false
}

