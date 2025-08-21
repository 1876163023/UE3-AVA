class avaMod_P90TR_T_LightTrigger_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).SpreadDecayTime += 0.8;
	avaWeap_BaseGun( Weapon ).MaxInaccuracy -=0.3;
	avaWeap_BaseGun( Weapon ).Spread_WhenFalling.param2 -=0.005;
	avaWeap_BaseGun( Weapon ).Spread_WhenMoving.param2 -=0.005;
	avaWeap_BaseGun( Weapon ).Spread_WhenDucking.param2 -=0.005;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteady.param2 -=0.005;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18181
	Slot		= WEAPON_SLOT_Trigger
}
