//=============================================================================
//  avaAttachment_C4
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/02/21 by OZ
//	
//=============================================================================

class avaAttachment_C4 extends avaWeaponAttachment;

defaultproperties
{
	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_New_C4.3P_Pos"
	SocMeshName			=	"Wp_New_C4.3P_Soc"
	BasicMeshName		=	"Wp_New_C4.MS_C4_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root

	DeathIconStr		=	"i"
	WeaponClass			=	class'avaWeap_C4'
	AnimPrefix			= C4

	AttachmentWeaponType = WBT_SMG01				// 3인칭 상체 Animation 표현을 위한 Weapon Type
	AttachmentBoneName	=	WPBone03
	bRightHandedWeapon	=	false
	CarriedSocketName	=	M2
}