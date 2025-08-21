/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_MP5K extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_MP5K'
	DeathIconStr="c"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_MP5K.3P_Pos"
	SocMeshName			=	"Wp_Smg_MP5K.3P_Soc"
	BasicMeshName		=	"Wp_Smg_MP5K.Wp_Smg_MP5K_Basic.MS_MP5K_3P"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix = MP5KPDW
	SilencerMeshName = "Wp_Silencer.MS_SMG_MP5K_silencer_3p"
}

