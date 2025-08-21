class avaAttachment_UMP45 extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_UMP45'
	DeathIconStr="N"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_UMP45.3P_Pos"
	SocMeshName			=	"Wp_Smg_UMP45.3P_Soc"
	BasicMeshName		=	"Wp_Smg_UMP45.MS_UMP45_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix = UMP45
	SilencerMeshName = "Wp_Silencer.MS_SMG_UMP45_silencer_3p"
}

