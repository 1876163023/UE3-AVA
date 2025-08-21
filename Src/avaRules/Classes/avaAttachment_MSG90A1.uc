class avaAttachment_MSG90A1 extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass=class'avaWeap_MSG90A1'
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
//발사시 연기 테스트
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'
}

