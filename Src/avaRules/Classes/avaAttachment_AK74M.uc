class avaAttachment_AK74M extends avaAttachment_BaseRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_AK74M'
	DeathIconStr="6"
	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Rif_AK74.3P_Pos"
	SocMeshName			=	"Wp_Rif_AK74.3P_Soc"
	BasicMeshName		=	"Wp_Rif_AK74.MS_74M_rail_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	AK74M
	AttachmentBoneName=WPBone03
	bRightHandedWeapon=false
}

