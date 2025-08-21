class avaMod_M82A1_T_Trigger_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Spread_WhenMovingA.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenFallingA.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenDuckingA.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).Spread_WhenSteadyA.param1-=0.00275;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA += 0.02;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18238
	Slot		= WEAPON_SLOT_Trigger
}
