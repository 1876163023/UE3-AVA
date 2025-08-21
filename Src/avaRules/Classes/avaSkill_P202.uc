class avaSkill_P202 extends avaCharacterModifier;

//51205		잠행 이동		Fast Silent Move
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrWalkSpeedPct += 0.1;
	Pawn.ChrCrouchSpeedPct += 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51205
}
