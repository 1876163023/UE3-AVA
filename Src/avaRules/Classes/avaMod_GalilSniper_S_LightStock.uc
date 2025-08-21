class avaMod_GalilSniper_S_LightStock extends avaMod_Weapon;

static function ApplyToWeapon_Client( avaWeapon Weapon )
{
//	Weapon.BaseSpeed	+= 5;	// 기본속도
}

static function ApplyToWeapon_Server( avaWeapon Weapon )
{
}

defaultproperties
{
	Id = 13073
	Slot = WEAPON_SLOT_Stock
}