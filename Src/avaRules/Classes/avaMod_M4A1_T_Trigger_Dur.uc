class avaMod_M4A1_T_Trigger_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).AccuracyOffset += 0.02;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18067
	Slot		= WEAPON_SLOT_Trigger
}
