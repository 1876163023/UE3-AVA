class avaAttachment_P226 extends avaAttachment_BasePistol;

defaultproperties
{
	WeaponClass=class'avaWeap_P226'

	DeathIconStr="f"
	
	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Pis_P226.3P_Pos"
	SocMeshName			=	"Wp_Pis_P226.3P_Soc"
	BasicMeshName		=	"Wp_Pis_P226.Wp_Pis_P226_Basic.MS_P226_Basic_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	//CarriedSocketName	=	Weapon01
	AnimPrefix = P226
	SilencerMeshName = "Wp_Silencer.MS_P226_Silencer_3p"
}
