class avaSkill_R301 extends avaCharacterModifier;

//51462	��ô ����	�⺻ ��ô �Ʒ�	Throwing Mastery	PowRolling
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ProjectileVelAmp += 0.03;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51462
}
