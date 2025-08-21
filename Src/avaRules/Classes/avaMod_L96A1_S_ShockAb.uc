class avaMod_L96A1_S_ShockAb extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenDuckingA.param1-=0.005;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param1-=0.005;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA += 0.08;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 13261
	Slot		= WEAPON_SLOT_Stock
}
