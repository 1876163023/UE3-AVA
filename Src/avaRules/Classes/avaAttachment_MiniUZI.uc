class avaAttachment_MiniUZI extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_MiniUZI'
	DeathIconStr="n"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_miniUZI.3P_Pos"
	SocMeshName			=	"Wp_Smg_miniUZI.3P_Soc"
	BasicMeshName		=	"Wp_Smg_miniUZI.Wp_Smg_miniUZI_Basic.MS_miniUZI_Basic_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix = UZI
	SilencerMeshName = "Wp_Silencer.MS_SMG_Uzi_silencer_3p"
}

