class avaAttachment_Knife2 extends avaWeaponAttachment;

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

	WeaponClass=class'avaWeap_Knife2'
	AttachmentWeaponType = WBT_SMG01				// 3��Ī ��ü Animation ǥ���� ���� Weapon Type
	AnimPrefix = Knife
}