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

	AttachmentWeaponType	=	WBT_SMG01				// 3��Ī ��ü Animation ǥ���� ���� Weapon Type
	AttachmentBoneName		=	WPBone01
	bRightHandedWeapon		=	true
	CarriedSocketName		=	M1
}
