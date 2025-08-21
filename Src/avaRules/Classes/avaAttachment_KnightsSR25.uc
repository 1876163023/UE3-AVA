class avaAttachment_KnightsSR25 extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass=class'avaWeap_KnightsSR25'
	DeathIconStr		=	"P"

	bMeshIsSkeletal		=	true
	MeshName			=	"WP_Sn_MSG90A1.3P_Pos"
	SocMeshName			=	"WP_Sn_MSG90A1.3P_Soc"
	BasicMeshName		=	"WP_Sn_MSG90A1.MS_MSG90A1_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AttachmentBoneName	=	WPBone01
	bRightHandedWeapon	=	true

	AnimPrefix			=	 MSG90A1
}

