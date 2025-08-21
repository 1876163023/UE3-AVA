/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_MachineGun extends avaAttachment_BaseRifle;

defaultproperties
{
	WeaponClass			=	class'avaWeap_MachineGun'
	DeathIconStr		=	"a"
	
	bMeshIsSkeletal		=	true
	MeshName			=	"WP_Heavy_M249.3P_Pos"
	SocMeshName			=	"WP_Heavy_M249.3P_Soc"
	BasicMeshName		=	"WP_Heavy_M249.Wp_M249_3p"
	PosRootBoneName		=	root
	SocRootBoneName		=	root
	//AttachmentBoneName;		// Character 의 손에 해당하는 Bone Name
	CarriedSocketName	=	B3

	AnimPrefix			=	M249
}

