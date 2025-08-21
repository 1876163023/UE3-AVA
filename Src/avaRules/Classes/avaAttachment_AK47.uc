/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_AK47 extends avaAttachment_BaseRifle;

defaultproperties
{

	WeaponClass=class'avaWeap_AK47'

	DeathIconStr="t"
	bMeshIsSkeletal		=	true
	MeshName			=	"WP_Rif_AK47.3P_Pos"
	SocMeshName			=	"WP_Rif_AK47.3P_Soc"
	BasicMeshName		=	"Wp_Rif_Ak47.MS_AK47_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	AK47
	AttachmentBoneName=WPBone03
	bRightHandedWeapon=false
}

