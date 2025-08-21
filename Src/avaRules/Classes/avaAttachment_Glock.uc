class avaAttachment_Glock extends avaAttachment_BasePistol;

defaultproperties
{
	WeaponClass=class'avaWeap_Glock'

	DeathIconStr="j"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Pis_Glock21C.Wp_Glock21C_Basic.3P_Pos"
	SocMeshName			=	"Wp_Pis_Glock21C.Wp_Glock21C_Basic.3P_Soc"
	BasicMeshName		=	"Wp_Pis_Glock21C.Wp_Glock21C_Basic.MS_Glock21C_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	//CarriedSocketName	=	Weapon01

	AnimPrefix = Glock
}
