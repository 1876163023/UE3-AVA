class avaAttachment_Binocular extends avaWeaponAttachment;

defaultproperties
{
	WeaponClass=class'avaWeap_Binocular'

	DeathIconStr="z"
	bMeshIsSkeletal		=	true

	MeshName			=	"Wp_Telescope.3P_Pos"
	SocMeshName			=	"Wp_Telescope.3P_Soc"
	BasicMeshName		=	"Wp_Telescope.MS_Telescope_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root

	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	//CarriedSocketName	=	B3
	AttachmentBoneName=WPBone01

	AnimPrefix		=	Telescope
	// bRightHandedWeapon=TRUE
}