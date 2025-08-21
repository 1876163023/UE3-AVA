//=============================================================================
//  avaAttachment_GalilSniper
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/04/05 by OZ
//=============================================================================
class avaAttachment_GalilSniper extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_GalilSniper'
	DeathIconStr		=	"e"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Sn_Galil.3P_Pos"
	SocMeshName			=	"Wp_Sn_Galil.3P_Soc"
	BasicMeshName		=	"Wp_Sn_Galil.Wp_Sn_Galil_Scope.MS_Galil_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character �� �տ� �ش��ϴ� Bone Name
	CarriedSocketName	=	B3

	AttachmentBoneName	=	WPBone01
	bRightHandedWeapon	=	true

	AnimPrefix			=	Galil

//�߻�� ���� �׽�Ʈ
	AbsMuzzleFlashPSCTemplate = ParticleSystem'avaEffect.Particles.P_WP_Sniper_Muzzlesmoke_3P'
}

