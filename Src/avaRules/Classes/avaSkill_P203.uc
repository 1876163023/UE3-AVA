class avaSkill_P203 extends avaCharacterModifier;

//51206		������Ʈ �ܷ�		FastSprint
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrSprintSpeedPct += 0.1;
	Pawn.ChrCrouchSprintSpeedPct += 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51206
}
