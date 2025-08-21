class avaMod_M4A1_F_Flash extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).bEnableFlashLight = true;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13060
//	CommonAttachedItems[0] = (MeshName="Wp_Flash.Flash01.MS_Flash01",PrimarySocket=Front )
	Slot = WEAPON_SLOT_Front
}