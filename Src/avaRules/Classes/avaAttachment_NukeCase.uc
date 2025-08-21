class avaAttachment_NukeCase extends avaWeaponAttachment;

defaultproperties
{

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_NUCpack.3P_Pos"
	SocMeshName			=	"Wp_NUCpack.3P_Soc"
	BasicMeshName		=	"Wp_NUCpack.MS_NUCpack_3p"
	PosRootBoneName	=	root
	SocRootBoneName	=	root

	DeathIconStr			=	"x"
	WeaponClass				=	class'avaWeap_NukeCase'
	AnimPrefix				=	NCPack

	AttachmentWeaponType	=	WBT_SMG01				// 3인칭 상체 Animation 표현을 위한 Weapon Type
	AttachmentBoneName		=	WPBone01
	bRightHandedWeapon		=	true
	CarriedSocketName		=	M1
}
