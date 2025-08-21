class avaAttachment_P90 extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_P90'

	DeathIconStr="3"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_P90.3P_Pos"
	SocMeshName			=	"Wp_Smg_P90.3P_Soc"
	BasicMeshName		=	"Wp_Smg_P90.MS_Smg_P90_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3
	AnimPrefix = P90
}

