class avaSkill_R302 extends avaCharacterModifier;

//51463		��� ��ô �Ʒ� ������		PowThrowing
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ProjectileVelAmp += 0.02;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51463
}
