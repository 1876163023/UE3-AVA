/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_BizonPP19 extends avaAttachment_BaseSMG;

defaultproperties
{
	WeaponClass=class'avaWeap_BizonPP19'
	DeathIconStr="u"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Smg_bizon.3P_Pos"
	SocMeshName			=	"Wp_Smg_bizon.3P_Soc"
	BasicMeshName		=	"Wp_Smg_bizon.MS_bizon_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3
	AnimPrefix = Bizon
}

