//=============================================================================
//  avaAttachment_SmokeBomb.
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//	2006/02/09 by OZ
//		1.	avaAttachment_Grenade �� �������� ���۵Ǿ���.
//=============================================================================
class avaAttachment_SmokeBomb extends avaWeaponAttachment;

defaultproperties
{
	bMeshIsSkeletal			=	false
	MeshName				=	"Wp_M83.MS_M83_3p"

	WeaponClass				=	class'avaWeap_SmokeBomb'

	MuzzleFlashSocket		=	None
	MuzzleFlashPSCTemplate	=	None
	MuzzleFlashDuration		=	0

	AttachmentWeaponType	=	WBT_SMG01
	AnimPrefix				=	G

	DeathIconStr			=	"h"
}

