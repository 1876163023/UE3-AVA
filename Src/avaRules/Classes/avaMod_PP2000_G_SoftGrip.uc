class avaMod_PP2000_G_SoftGrip extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SpreadDecayTime -= 0.3;
	avaWeap_BaseGun( Weapon ).MaxInaccuracy -=0.2;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1 +=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1 +=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1 +=0.002;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1 +=0.002;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18130
	Slot		= WEAPON_SLOT_Grip
}
