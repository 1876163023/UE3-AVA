class avaSkill_S201 extends avaCharacterModifier;

//51716	고급 스나이핑	크런치 스나이핑	Advanced Sniping	Crouch Sniping
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrWalkSpeedPct	+= 0.1;
	Pawn.ChrCrouchSpeedPct 	+= 0.1;
	Pawn.WeapTypeAdd[WEAPON_SNIPER].SpreadDuckingAdd -=0.001;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51716
}
