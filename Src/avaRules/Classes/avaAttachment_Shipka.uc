class avaAttachment_Shipka extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_Shipka'
	DeathIconStr="c"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_MP5K.3P_Pos"
	SocMeshName			=	"Wp_Smg_MP5K.3P_Soc"
	BasicMeshName		=	"Wp_Smg_MP5K.Wp_Smg_MP5K_Basic.MS_MP5K_3P"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix = MP5KPDW
}

