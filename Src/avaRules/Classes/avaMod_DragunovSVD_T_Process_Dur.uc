class avaMod_DragunovSVD_T_Process_Dur extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).Kickback_WhenDuckingA.Lateralbase-=0.8;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Upbase-=1;
	avaWeap_BaseGun( Weapon ).Kickback_WhenSteadyA.Lateralbase-=0.8;

	avaWeap_BaseGun( Weapon ).FireInterval[(0)] += 0.3;

}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id		= 18024
	Slot		= WEAPON_SLOT_Trigger
}
