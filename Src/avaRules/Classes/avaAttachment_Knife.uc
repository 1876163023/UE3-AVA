//=============================================================================
//  avaAttachment_Knife
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//
//	2006/03/06 by OZ
//	
//=============================================================================

class avaAttachment_Knife extends avaWeaponAttachment;

/// SKIP Cause Muzzle flash ; deif 2006/4/12
simulated event ThirdPersonFireEffects(vector HitLocation)
{
	// Have pawn play firing anim
	if ( Instigator != none && avaPawn(Instigator)!=None )
	{
		// FIXME: Come up with a better way to support the animspeed.

		if ( avaPawn(Instigator).FiringMode == 1 )
			Instigator.PlayFiring(1.0,'1');
		else
			Instigator.PlayFiring(1.0,'0');
	}
}

defaultproperties
{
	DamageCode=Knife

	bMeshIsSkeletal		=	false
	MeshName			=	"Wp_Knife01.MS_Knife01_3p"
	//SocMeshName		=
	//BasicMeshName		=
	//PosRootBoneName	=
	//SocRootBoneName	=

	DeathIconStr="m"

	WeaponClass=class'avaWeap_Knife'
	AttachmentWeaponType = WBT_SMG01				// 3인칭 상체 Animation 표현을 위한 Weapon Type
	AnimPrefix = Knife
}