/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_M4A1 extends avaAttachment_BaseRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_M4A1'
	DeathIconStr		=	"y"
	
	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Rif_M4.3P_Pos"
	SocMeshName			=	"Wp_Rif_M4.3P_Soc"
	BasicMeshName		=	"Wp_Rif_M4.Wp_Rif_M4_Rail.MS_M4_Rail_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	M4
}

