class avaAttachment_M16A4 extends avaAttachment_BaseRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_M16A4'
	DeathIconStr		=	"a"
	
	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Rif_M4.3P_Pos"
	SocMeshName			=	"Wp_Rif_M4.3P_Soc"
	BasicMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rail_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	M16A2
}

