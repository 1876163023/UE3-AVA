class avaMod_M4A1_F_Laser extends avaMod_Weapon;

/*
//static function ApplyToCharacter_Client( avaPawn Pawn );
//static function ApplyToCharacter_Server( avaPawn Pawn );
*/

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param2-=0.004;
	avaWeap_BaseGun( Weapon ).BaseSpeed-=2;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13061
	CommonAttachedItems[0] = (MeshName="Wp_Laser.Com_Laser.MS_Com_Laser_001",PrimarySocket=Front )
	Slot = WEAPON_SLOT_Front
}