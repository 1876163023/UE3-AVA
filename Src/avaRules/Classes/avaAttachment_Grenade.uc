/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAttachment_Grenade extends avaWeaponAttachment;

defaultproperties
{
	DamageCode				=	Grenade

	bMeshIsSkeletal			=	false
	MeshName				=	"Wp_M67.MS_M67_3p"

	WeaponClass				=	class'avaWeap_Grenade'

	MuzzleFlashSocket		=	None
	MuzzleFlashPSCTemplate	=	None
	MuzzleFlashDuration		=	0

	AttachmentWeaponType	=	WBT_SMG01
	AnimPrefix				=	G

	DeathIconStr			=	"g"
}

