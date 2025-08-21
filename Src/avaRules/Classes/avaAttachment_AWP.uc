/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_AWP extends avaAttachment_BaseSniperRifle;

defaultproperties
{
	WeaponClass=class'avaWeap_AWP'
	DeathIconStr="d"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_sn_TPG1.3P_Pos"
	SocMeshName			=	"Wp_sn_TPG1.3P_Soc"
	BasicMeshName		=	"Wp_sn_TPG1.Wp_Sn_TPG1_Rail.MS_TPG1_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3
	
	AnimPrefix = TPG1

	AttachmentBoneName=WPBone03
	bRightHandedWeapon=false
}

