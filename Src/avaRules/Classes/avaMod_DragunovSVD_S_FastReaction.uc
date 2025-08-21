class avaMod_DragunovSVD_S_FastReaction extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
	avaWeap_BaseGun( Weapon ).RangeModifier -= 0.02;
	avaWeap_BaseGun( Weapon ).BaseSpeed -= 5;
	avaWeap_BaseGun( Weapon ).SpreadDecayTime -= 0.05;
	avaWeap_BaseGun( Weapon ).MaxInaccuracyA -= 0.4;
	avaWeap_BaseGun( Weapon ).AccuracyOffsetA -= 0.05;
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13192
	Slot = WEAPON_SLOT_Stock
}