class avaSkill_P201 extends avaCharacterModifier;

//51204	����̵���	���� �ܷ�	Advanced Move	SafeLanding
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.FallingDamageAmp -= 0.5;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 51204
}
