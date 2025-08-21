class avaAttachment_MP5 extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_MP5'
	DeathIconStr="r"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_MP5A3.3P_Pos"
	SocMeshName			=	"Wp_Smg_MP5A3.3P_Soc"
	BasicMeshName		=	"Wp_Smg_MP5A3.MS_Smg_MP5A3_3P"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix = MP5A3
	SilencerMeshName = "Wp_Silencer.MS_SMG_MP5A3_silencer_3p"
}

