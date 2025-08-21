/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_G36 extends avaAttachment_BaseRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_G36'
	DeathIconStr		=	"b"

	bMeshIsSkeletal		=	true
	MeshName			=	"Wp_Rif_G36.3P_Pos"
	SocMeshName			=	"Wp_Rif_G36.3P_Soc"
	BasicMeshName		=	"Wp_Rif_G36.Wp_Rif_G36_Scope.MS_G36_Scope_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	G36
}

