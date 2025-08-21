class avaAttachment_VSS extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_VSS'
	DeathIconStr		=	"e"

	bMeshIsSkeletal		=	true
	MeshName			=	"WP_sn_vss.3P_Pos"
	SocMeshName			=	"WP_sn_vss.3P_Soc"
	BasicMeshName		=	"Wp_Sn_Galil.Wp_Sn_Galil_Scope.MS_Galil_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AttachmentBoneName	=	WPBone01
	bRightHandedWeapon	=	true

	AnimPrefix			=	VSS
}

