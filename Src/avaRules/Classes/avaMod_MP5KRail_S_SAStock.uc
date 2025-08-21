class avaMod_MP5KRail_S_SAStock extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	avaWeap_BaseGun( Weapon ).Kickback_WhenMoving.Lateralbase-=0.25;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenFalling.Lateralbase-=0.25;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenDucking.Lateralbase-=0.25;
//	avaWeap_BaseGun( Weapon ).Kickback_WhenSteady.Lateralbase-=0.25;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id			=	13082
	Slot = WEAPON_SLOT_Stock
}
